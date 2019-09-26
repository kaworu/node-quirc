/*
 * eci.c - Generate one QR code on stdout containing a ECI segment (UTF-8).
 *
 * Compile with:
 * cc -std=c11 qrcodegen.c eci.c $(pkg-config --cflags --libs libpng) -o eci
 */
#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qrcodegen.h"
#include <png.h>


int
main(int argc, char **argv)
{
	const int dot_size = 3, margin = 4;
	const char white = 0xFF, black = 0x00;
	/* taken from https://www.w3.org/2001/06/utf-8-test/UTF-8-demo.html */
	const char *data = u8"ᚻᛖ ᚳᚹᚫᚦ ᚦᚫᛏ ᚻᛖ ᛒᚢᛞᛖ ᚩᚾ ᚦᚫᛗ ᛚᚪᚾᛞᛖ ᚾᚩᚱᚦᚹᛖᚪᚱᛞᚢᛗ ᚹᛁᚦ ᚦᚪ ᚹᛖᛥᚫ";
	const size_t datalen = strlen(data);
	uint8_t qrcode[qrcodegen_BUFFER_LEN_MAX], tmp[qrcodegen_BUFFER_LEN_MAX];
	png_structp png_ptr;
	png_infop info_ptr;

	uint8_t *eci = calloc(qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_ECI, 0), sizeof(uint8_t));
	uint8_t *buf = calloc(qrcodegen_calcSegmentBufferSize(qrcodegen_Mode_BYTE, datalen), sizeof(uint8_t));
	uint8_t *bytes = calloc(datalen, sizeof(uint8_t));
	if (eci == NULL || buf == NULL || bytes == NULL)
		err(1, "calloc");

	for (size_t i = 0; i < datalen; i++)
		bytes[i] = (uint8_t)data[i];

	struct qrcodegen_Segment segs[] = {
		qrcodegen_makeEci(/* ECI_UTF_8 */26, eci),
		qrcodegen_makeBytes(bytes, datalen, buf),
	};
	bool ret = qrcodegen_encodeSegments(segs, 2, qrcodegen_Ecc_LOW, tmp, qrcode);
	if (!ret)
		err(1, "qrcodegen_encodeSegments");
	const int width = qrcodegen_getSize(qrcode);
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if (png_ptr == NULL)
		err(1, "png_create_write_struct");
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
		err(1, "png_create_info_struct");
	if (setjmp(png_jmpbuf(png_ptr)))
		err(1, "setjmp");
	png_init_io(png_ptr, stdout);
	png_set_IHDR(png_ptr, info_ptr,
	    (width + margin * 2) * dot_size,
	    (width + margin * 2) * dot_size,
	    1 /* depth */, PNG_COLOR_TYPE_GRAY, PNG_INTERLACE_NONE,
	    PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
	png_write_info(png_ptr, info_ptr);
	png_set_packing(png_ptr);
	unsigned char *row = calloc((width + margin * 2), dot_size);
	if (row == NULL)
		err(1, "calloc");
	for (int y = -margin; y < width + margin; y++) {
		for (int x = -margin; x < width + margin; x++) {
			unsigned char dot;
			if (y < 0 || y > width - 1 || x < 0 || x > width - 1)
				dot = white;
			else
				dot = qrcodegen_getModule(qrcode, x, y) ? black : white;
			for (int d = 0; d < dot_size; d++)
				row[(x + margin) * dot_size + d] = dot;
		}
		for (int d = 0; d < dot_size; d++)
			png_write_row(png_ptr, row);
	}

	png_write_end(png_ptr, info_ptr);
	free(row);

	/* cleanup */
	free(bytes);
	free(buf);
	free(eci);
	png_destroy_write_struct(&png_ptr, &info_ptr);

	return (EXIT_SUCCESS);
}
