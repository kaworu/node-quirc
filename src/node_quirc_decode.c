/*
 * node_quirc_decode.c - node-quirc decoding stuff
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>
#define	PNG_BYTES_TO_CHECK	4
#include <jpeglib.h>

#include "node_quirc_decode.h"
#include "quirc.h"


/* a nq_code list */
struct nq_code_list {
	const char	*err; /* global error */
	struct nq_code	*codes;
	unsigned int	 size;
};

struct nq_code {
	const char		*err;
	struct quirc_code	 qcode;
	struct quirc_data	 qdata;
};

static int	nq_load_image(struct quirc *q, const uint8_t *img, size_t img_len, size_t img_width, size_t img_height);
static int	nq_load_png(struct quirc *q, const uint8_t *img, size_t img_len);
static int	nq_load_jpeg(struct quirc *q, const uint8_t *img, size_t img_len);
static int	nq_load_raw(struct quirc *q, const uint8_t *img, size_t img_len, size_t img_width, size_t img_height);


struct nq_code_list *
nq_decode(const uint8_t *img, size_t img_len, size_t img_width, size_t img_height)
{
	struct nq_code_list *list = NULL;
	struct quirc *q = NULL;

	list = calloc(1, sizeof(struct nq_code_list));
	if (list == NULL)
		goto out;

	q = quirc_new();
	if (q == NULL) {
		list->err = "quirc_new()";
		goto out;
	}

	if (nq_load_image(q, img, img_len, img_width, img_height) == -1) {
		// FIXME: more descriptive error here?
		list->err = "failed to load image";
		goto out;
	}

	quirc_end(q);

	int count = quirc_count(q);
	if (count < 0) {
		list->err = "quirc_count()";
		goto out;
	}

	list->size  = (unsigned int)count;
	list->codes = calloc((size_t)list->size, sizeof(struct nq_code));
	if (list->codes == NULL) {
		nq_code_list_free(list);
		list = NULL;
		goto out;
	}

	for (int i = 0; i < count; i++) {
		struct nq_code *nqcode = list->codes + i;
		quirc_decode_error_t err;

		quirc_extract(q, i, &nqcode->qcode);
		err = quirc_decode(&nqcode->qcode, &nqcode->qdata);
		if (err == QUIRC_ERROR_DATA_ECC) {
			quirc_flip(&nqcode->qcode);
			err = quirc_decode(&nqcode->qcode, &nqcode->qdata);
		}

		if (err)
			nqcode->err = quirc_strerror(err);
	}

	/* FALLTHROUGH */
out:
	/* cleanup */
	if (q != NULL)
		quirc_destroy(q);

	return (list);
}


const char *
nq_code_list_err(const struct nq_code_list *list)
{
	return (list->err);
}


unsigned int
nq_code_list_size(const struct nq_code_list *list)
{
	return (list->size);
}


const struct nq_code *
nq_code_at(const struct nq_code_list *list, unsigned int index)
{
	const struct nq_code *target = NULL;

	if (index < nq_code_list_size(list))
		target = list->codes + index;

	return (target);

}


void
nq_code_list_free(struct nq_code_list *list)
{
	if (list != NULL)
		free(list->codes);
	free(list);
}


const char *
nq_code_err(const struct nq_code *code)
{
	return (code->err);
}


int
nq_code_version(const struct nq_code *code)
{
	return (code->qdata.version);
}


const char *
nq_code_ecc_level_str(const struct nq_code *code)
{
	switch (code->qdata.ecc_level) {
	case  QUIRC_ECC_LEVEL_M: return "M";
	case  QUIRC_ECC_LEVEL_L: return "L";
	case  QUIRC_ECC_LEVEL_H: return "H";
	case  QUIRC_ECC_LEVEL_Q: return "Q";
	}

	return "?";
}


int
nq_code_mask(const struct nq_code *code)
{
	return (code->qdata.mask);
}


const char *
nq_code_mode_str(const struct nq_code *code)
{
	switch (code->qdata.data_type) {
	case QUIRC_DATA_TYPE_NUMERIC: return "NUMERIC";
	case QUIRC_DATA_TYPE_ALPHA:   return "ALNUM";
	case QUIRC_DATA_TYPE_BYTE:    return "BYTE";
	case QUIRC_DATA_TYPE_KANJI:   return "KANJI";
	}

	return "unknown";
}


const char *
nq_code_eci_str(const struct nq_code *code)
{
	switch (code->qdata.eci) {
		case QUIRC_ECI_ISO_8859_1:  return "ISO_8859_1";
		case QUIRC_ECI_IBM437:      return "IBM437";
		case QUIRC_ECI_ISO_8859_2:  return "ISO_8859_2";
		case QUIRC_ECI_ISO_8859_3:  return "ISO_8859_3";
		case QUIRC_ECI_ISO_8859_4:  return "ISO_8859_4";
		case QUIRC_ECI_ISO_8859_5:  return "ISO_8859_5";
		case QUIRC_ECI_ISO_8859_6:  return "ISO_8859_6";
		case QUIRC_ECI_ISO_8859_7:  return "ISO_8859_7";
		case QUIRC_ECI_ISO_8859_8:  return "ISO_8859_8";
		case QUIRC_ECI_ISO_8859_9:  return "ISO_8859_9";
		case QUIRC_ECI_WINDOWS_874: return "WINDOWS_874";
		case QUIRC_ECI_ISO_8859_13: return "ISO_8859_13";
		case QUIRC_ECI_ISO_8859_15: return "ISO_8859_15";
		case QUIRC_ECI_SHIFT_JIS:   return "SHIFT_JIS";
		case QUIRC_ECI_UTF_8:       return "UTF_8";
	}
	return NULL;
}


const uint8_t *
nq_code_payload(const struct nq_code *code)
{
	return (code->qdata.payload);
}


size_t
nq_code_payload_len(const struct nq_code *code)
{
	return (code->qdata.payload_len);
}


/* returns 0 on success, -1 on error */
static int
nq_load_image(struct quirc *q, const uint8_t *img, size_t img_len, size_t img_width, size_t img_height)
{
	if (img_width > 0 && img_height > 0) {
		return nq_load_raw(q, img, img_len, img_width, img_height);
	}

	int ret = -1; /* error */

	if (img_len >= PNG_BYTES_TO_CHECK) {
		if (png_sig_cmp((uint8_t *)img, (png_size_t)0, PNG_BYTES_TO_CHECK) == 0)
			ret = nq_load_png(q, img, img_len);
	}

	if (ret != 0) {
			ret = nq_load_jpeg(q, img, img_len);
	}

	return (ret);
}


/* hacked from quirc/tests/dbgutil.c */
static int
nq_load_png(struct quirc *q, const uint8_t *img, size_t img_len)
{
	int width, height, rowbytes, interlace_type, number_passes = 1;
	png_uint_32 trns;
	png_byte color_type, bit_depth;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	uint8_t *image;
	FILE *infile = NULL;
	volatile int success = 0;

	infile = fmemopen((uint8_t *)img, img_len, "r");
	if (infile == NULL)
		goto out;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		goto out;

	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		goto out;

	if (setjmp(png_jmpbuf(png_ptr)))
		goto out;

	png_init_io(png_ptr, infile);

	png_read_info(png_ptr, info_ptr);

	color_type     = png_get_color_type(png_ptr, info_ptr);
	bit_depth      = png_get_bit_depth(png_ptr, info_ptr);
	interlace_type = png_get_interlace_type(png_ptr, info_ptr);

	// Read any color_type into 8bit depth, Grayscale format.
	// See http://www.libpng.org/pub/png/libpng-manual.txt

	// PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
	if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
		png_set_expand_gray_1_2_4_to_8(png_ptr);

	if ((trns = png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)))
		png_set_tRNS_to_alpha(png_ptr);

	if (bit_depth == 16)
#if PNG_LIBPNG_VER >= 10504
		png_set_scale_16(png_ptr);
#else
		png_set_strip_16(png_ptr);
#endif

	if ((trns) || color_type & PNG_COLOR_MASK_ALPHA)
		png_set_strip_alpha(png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE)
		png_set_palette_to_rgb(png_ptr);

	if (color_type == PNG_COLOR_TYPE_PALETTE ||
	    color_type == PNG_COLOR_TYPE_RGB ||
	    color_type == PNG_COLOR_TYPE_RGB_ALPHA) {
		png_set_rgb_to_gray_fixed(png_ptr, 1, -1, -1);
	}

	if (interlace_type != PNG_INTERLACE_NONE)
		number_passes = png_set_interlace_handling(png_ptr);

	png_read_update_info(png_ptr, info_ptr);

	width    = png_get_image_width(png_ptr, info_ptr);
	height   = png_get_image_height(png_ptr, info_ptr);
	rowbytes = png_get_rowbytes(png_ptr, info_ptr);
	if (rowbytes != width) {
		goto out;
	}

	if (quirc_resize(q, width, height) < 0)
		goto out;

	image = quirc_begin(q, NULL, NULL);

	for (int pass = 0; pass < number_passes; pass++) {
		int y;

		for (y = 0; y < height; y++) {
			png_bytep row_pointer = image + y * width;
			png_read_rows(png_ptr, &row_pointer, NULL, 1);
		}
	}

	png_read_end(png_ptr, info_ptr);

	success = 1;
	/* FALLTHROUGH */
out:
	/* cleanup */
	if (png_ptr != NULL) {
		if (info_ptr != NULL)
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		else
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	}
	if (infile != NULL)
		fclose(infile);
	return (success ? 0 : -1);
}


/* hacked from quirc/tests/dbgutil.c */
struct nq_jpeg_error {
	struct jpeg_error_mgr base;
	jmp_buf env;
};


static void
nq_error_exit(struct jpeg_common_struct *com)
{
	struct nq_jpeg_error *err = (struct nq_jpeg_error *)com->err;

	longjmp(err->env, 0);
}


static struct jpeg_error_mgr *
nq_error_mgr(struct nq_jpeg_error *err)
{
	jpeg_std_error(&err->base);
	err->base.error_exit = nq_error_exit;
	return &err->base;
}


static int
nq_load_jpeg(struct quirc *q, const uint8_t *img, size_t img_len)
{
	struct jpeg_decompress_struct dinfo;
	struct nq_jpeg_error err;
	uint8_t *image;
	JDIMENSION y;

	memset(&dinfo, 0, sizeof(dinfo));
	dinfo.err = nq_error_mgr(&err);

	if (setjmp(err.env))
		goto fail;

	jpeg_create_decompress(&dinfo);
	jpeg_mem_src(&dinfo, img, img_len);

	jpeg_read_header(&dinfo, TRUE);
	dinfo.output_components = 1;
	dinfo.out_color_space = JCS_GRAYSCALE;
	jpeg_start_decompress(&dinfo);

	if (dinfo.output_components != 1)
		goto fail;

	if (quirc_resize(q, dinfo.output_width, dinfo.output_height) < 0)
		goto fail;

	image = quirc_begin(q, NULL, NULL);

	for (y = 0; y < dinfo.output_height; y++)
	{
		JSAMPROW row_pointer = image + y * dinfo.output_width;

		jpeg_read_scanlines(&dinfo, &row_pointer, 1);
	}

	jpeg_finish_decompress(&dinfo);
	jpeg_destroy_decompress(&dinfo);
	return 0;

fail:
	jpeg_destroy_decompress(&dinfo);
	return -1;
}

static int
nq_load_raw(struct quirc *q, const uint8_t *img, size_t img_len, size_t img_width, size_t img_height)
{
	if (quirc_resize(q, img_width, img_height) < 0)
		goto fail;

	uint8_t *image = quirc_begin(q, NULL, NULL);

	const size_t len = img_width * img_height;
	const int channels = len == img_len ? 1 : /* grayscale */
			3 * len == img_len ? 3 : /* rgb */
			4 * len == img_len ? 4 : /* rgba */
			/* default */ -1;

	if (channels == 1) {
		memcpy(image, img, img_len);
	} else if (channels == 3 || channels == 4) {
		for (
			size_t dst_offset = 0, src_offset = 0;
			dst_offset < img_width * img_height;
			dst_offset++, src_offset += channels
		) {
			uint8_t r = img[src_offset];
			uint8_t g = img[src_offset + 1];
			uint8_t b = img[src_offset + 2];
			// convert RGB to grayscale, ignoring alpha channel if present, using this:
			// https://en.wikipedia.org/wiki/Grayscale#Colorimetric_(perceptual_luminance-preserving)_conversion_to_grayscale
			image[dst_offset] = (uint8_t)(0.2126 * (float)r + 0.7152 * (float)g + 0.0722 * (float)b);
		}
	} else {
		goto fail;
	}

	return 0;

fail:
	return -1;
}
