#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <png.h>
#include <stdlib.h>

//raw file format:
//header:
	//width  = read_long(fp);
	//height = read_long(fp);
	//stride = read_long(fp);
	//format = read_long(fp);

// Read a 32-bit signed integer.
static int read_long(FILE * fp)
{
	unsigned char b0, b1, b2, b3;	/* Bytes from file */

	b0 = getc(fp);
	b1 = getc(fp);
	b2 = getc(fp);
	b3 = getc(fp);

	return ((int)(((((b3 << 8) | b2) << 8) | b1) << 8) | b0);
}

/**
*/
int writeImage(char *filename, int width, int height, char *fbbuf, int scale_rate, char *title)
{
	int code = 0;
	FILE *fp = NULL;
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	png_bytep row = NULL;
	char *fbd = (char *)malloc(width * height * 4 / scale_rate / scale_rate);
	// Open file for writing (binary mode)
	fp = fopen(filename, "wb");
	if (fp == NULL) {
		fprintf(stderr, "Could not open file %s for writing\n", filename);
		code = 1;
		return code;
	}
	char *cl = fbbuf;
	char *cr = fbd;
	for (int i = 0; i < height / scale_rate; i++) {
		cl = fbbuf + width * 4 * i * scale_rate;
		for (int c = 0; c < width / scale_rate; c++) {
			*(int32_t *) (cr) = *(int32_t *) (cl + c * 4 * scale_rate);
			cr += 4;
		}
	}

	fwrite(fbd, width * height * 4 / scale_rate / scale_rate, 1, fp);

	if (fp != NULL)
		fclose(fp);
	return code;
}

int main(int argc, char *argv[])
{
	// Make sure that the output filename argument has been provided
	char *fbbuf = NULL;

	int fbbufsize = 0;

	int32_t raw_width;	/*! Width of the buffer in pixels. */
	int32_t raw_height;	/*! Height of the buffer in pixels. */
	int32_t raw_stride;
	int32_t format;
	int scale_rate = 1;
	FILE *fp = NULL;
	if (argc != 4) {
		fprintf(stderr, "usage: \r\n   raw2fb fbfilename rawfile  scale_rate\n");
		return 1;
	}
	// Specify an output image size

	scale_rate = atoi(argv[3]);
	fp = fopen(argv[2], "rb");

	raw_width = read_long(fp);
	raw_height = read_long(fp);
	raw_stride = read_long(fp);
	format = read_long(fp);
	printf("raw_width:%d", raw_width);
	printf("raw_height:%d", raw_height);
	printf("raw_stride:%d", raw_stride);
	printf("format:%d", format);
	fbbufsize = raw_stride * raw_height;

	fbbuf = (char *)malloc(fbbufsize);
	fread(fbbuf, fbbufsize, 1, fp);
	fclose(fp);
#if 0				//test
	char *fbr;
	int x, y;
	unsigned char red = 0xff;
	unsigned char green = 0xff;
	unsigned char blue = 0xff;
	for (y = 0; y < height; y++) {
		fbr = fbbuf + width * 4 * y;
		for (x = 0; x < width; x++) {
			if (y % 4 == 0) {

				fbr[x * 4 + 0] = red;
				fbr[x * 4 + 1] = 0x00;
				fbr[x * 4 + 2] = 0x00;
				fbr[x * 4 + 3] = 0xff;
			} else {
				fbr[x * 4 + 0] = 0x00;
				fbr[x * 4 + 1] = green;
				fbr[x * 4 + 2] = 0x00;
				fbr[x * 4 + 3] = 0xff;
			}
		}
	}
#endif

	printf("Saving fbfile\n");
	int result = writeImage(argv[1], raw_width, raw_height, fbbuf, scale_rate, "This is my test image");
	if (result) {
		printf("Saving err\n");
	}

	return result;
}
