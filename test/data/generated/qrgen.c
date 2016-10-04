/*
 * qrgen.c - Generate *a lot* of QR code files.
 *
 * Compile with:
 *   cc -std=c99 qrgen.c -lpng -lqrencode -o qrgen
 */

#define	_GNU_SOURCE /* lol */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>

#include <qrencode.h>
#include <png.h>


static const char *mode_str[] = {
	[QR_MODE_NUM]   = "NUMERIC",
	[QR_MODE_AN]    = "ALNUM",
	[QR_MODE_8]     = "BYTE",
	[QR_MODE_KANJI] = "KANJI",
};

static const char *ecc_level_str[] = {
	[QR_ECLEVEL_L]   = "L",
	[QR_ECLEVEL_M]   = "M",
	[QR_ECLEVEL_Q]   = "Q",
	[QR_ECLEVEL_H]   = "H",
};


void
generate(int version, QRencodeMode mode, QRecLevel level)
{
	const char *num_data = "42";
	int num_datalen = strlen(num_data);
	const char *an_data = "AC-42";
	int an_datalen = strlen(an_data);
	const char *bin_data = "aA1234";
	int bin_datalen = strlen(bin_data);
	/* taken from https://github.com/fukuchi/libqrencode/blob/master/tests/test_estimatebit.c#L109 */
	const unsigned char kanji_data[4] = {0x93, 0x5f,0xe4, 0xaa};
	int kanji_datalen = sizeof(kanji_data);
	const unsigned char *data;
	int datalen;
	QRinput *input;
	QRcode *code;
	png_structp png_ptr;
	png_infop info_ptr;
	int dot_size = 3, margin = 4;
	char white = 0xFF, black = 0x00;
	char *fn;
	FILE *fh;
	int x, y, d;

	switch (mode) {
	case QR_MODE_NUM:
		data    = (const unsigned char *)num_data;
		datalen = num_datalen;
		break;
	case QR_MODE_AN:
		data    = (const unsigned char *)an_data;
		datalen = an_datalen;
		break;
	case QR_MODE_8:
		data    = (const unsigned char *)bin_data;
		datalen = bin_datalen;
		break;
	case QR_MODE_KANJI:
		data    = (const unsigned char *)kanji_data;
		datalen = kanji_datalen;
		break;
	default:
		errx(1, "%d: unsupported mode", mode);
		/* NOTREACHED */
	};

	input = QRinput_new2(version, level);
	if (input == NULL)
		err(1, "QRinput_new2");
	if (QRinput_append(input, mode, datalen, data) == -1)
		err(1, "QRinput_append");
	code = QRcode_encodeInput(input);
	QRinput_free(input);

	int ret = asprintf(&fn, "version=%02d,level=%s,mode=%s.png",
		    version, ecc_level_str[level], mode_str[mode]);
	if (ret == -1)
		err(1, "asprintf");
	if ((fh = fopen(fn, "wb")) == NULL)
		err(1, "fopen");

	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == png_structp_NULL)
		err(1, "png_create_write_struct");
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		err(1, "png_create_info_struct");
	if (setjmp(png_jmpbuf(png_ptr)))
		err(1, "setjmp");
	png_init_io(png_ptr, fh);
	png_set_IHDR(png_ptr, info_ptr,
	    (code->width  + margin * 2) * dot_size,
	    (code->width + margin * 2) * dot_size,
	    1 /* depth */, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
	    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);
	unsigned char *row = calloc((code->width  + margin * 2), dot_size);
	if (row == NULL)
		err(1, "calloc");
	for (y = -margin; y < code->width + margin; y++) {
		for (x = -margin; x < code->width + margin; x++) {
			unsigned char dot;
			if (y < 0 || y > code->width - 1 || x < 0 || x > code->width - 1)
				dot = white;
			else
				dot = (code->data[y * code->width + x] & 0x1 ? black : white);
			for (d = 0; d < dot_size; d++)
				row[(x + margin) * dot_size + d] = dot;
		}
		for (d = 0; d < dot_size; d++)
			png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	free(row);

	/* cleanup */
	png_destroy_write_struct(&png_ptr, &info_ptr);
	fclose(fh);
	QRcode_free(code);
}


int
main(int argc, char **argv)
{
	for (int version = 1; version <= 40; version++) {
		for (QRencodeMode mode = QR_MODE_NUM; mode <= QR_MODE_KANJI; mode++) {
			for (QRecLevel level = QR_ECLEVEL_L; level <= QR_ECLEVEL_H; level++) {
				(void)printf("version=%02d,level=%s,mode=%s\n",
				    version, ecc_level_str[level], mode_str[mode]);
				generate(version, mode, level);
			}
		}
	}
	return (EXIT_SUCCESS);
}
