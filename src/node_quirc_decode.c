/*
 * node_quirc_decode.c - node-quirc decoding stuff
 */

#include <stdlib.h>
#include <string.h>

#include <png.h>

#include "node_quirc_decode.h"
#include "quirc/quirc.h"


/* a nq_code list */
struct nq_code_list {
	const char	*err; /* global error */
	struct nq_code	*codes;
	size_t		 size;
};

struct nq_code {
	const char		*err;
	struct quirc_data	 qdata;
};

static int	load_png(struct quirc *q, const uint8_t *img, size_t img_len)


struct nq_code_list *
nq_decode(const uint8_t *img, size_t img_len)
{
	struct nq_code_list *list = NULL;
	struct quirc *q = NULL;
	char *buf = NULL;

	list = calloc(1, sizeof(struct nq_code_list));
	if (list == NULL)
		goto out;

	/* arguments sanity check */
	if (img == NULL) {
		list->err = "img cannot be NULL";
		goto out;
	}

	q = quirc_new();
	if (q == NULL) {
		list->err = "quirc_new()";
		goto out;
	}

	if (load_png(q, img) == -1) {
		list->err = "failed to load image";
		goto out;
	}

	quirc_end(q);

	int count = quirc_count(q);
	if (count < 0) {
		list->err = "quirc_count()";
		goto out;
	}
	list->codes = calloc((size_t)count, sizeof(struct nq_code));
	if (list->codes == NULL) {
		list->err = "calloc(3)";
		goto out;
	}

	for (int i = 0; i < count; i++) {
		struct quirc_code code;
		struct quirc_data data;
		quirc_decode_error_t err;

		quirc_extract(q, i, &code);
		err = quirc_decode(&code, &list->codes[i].data);

		if (err)
			list->codes[i].err = quirc_strerror(err);
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


size_t
nq_code_list_size(const struct nq_code_list *list)
{
	return (list->size);
}


const struct nq_code *
nq_code_at(const struct nq_code_list *list, size_t index)
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
nq_code_data_type_str(const struct nq_code *code)
{
	switch (code->qdata.data_type) {
	case QUIRC_DATA_TYPE_NUMERIC: return "NUMERIC";
	case QUIRC_DATA_TYPE_ALPHA:   return "ALPHA";
	case QUIRC_DATA_TYPE_BYTE:    return "BYTE";
	case QUIRC_DATA_TYPE_KANJI:   return "KANJI";
	}

	return "unknown";
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

/* hacked from quirc/tests/dbgutil.c */
static int
load_png(struct quirc *q, const uint8_t *img, size_t img_len)
{
	int width, height, rowbytes, interlace_type, number_passes = 1;
	png_uint_32 trns;
	png_byte color_type, bit_depth;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	uint8_t *image;
	int success = 0;
	int pass;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (!png_ptr)
		goto out;

	info_ptr = png_create_info_struct(png_ptr);
	if (!info_ptr)
		goto out;

	if (setjmp(png_jmpbuf(png_ptr)))
		goto out;

	// TODO png_set_read_fn() here
	// see http://pulsarengine.com/2009/01/reading-png-images-from-memory/

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

	for (pass = 0; pass < number_passes; pass++) {
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
	if (png_ptr) {
		if (info_ptr)
			png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		else
			png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
	}
	return (success ? 0 : -1);
}
