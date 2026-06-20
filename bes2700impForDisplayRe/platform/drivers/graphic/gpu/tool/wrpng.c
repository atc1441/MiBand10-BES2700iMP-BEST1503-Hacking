#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <malloc.h>
#include <png.h>
// This function actually writes out the PNG image file. The string 'title' is
// also written into the image file
int writeImage(char *filename, int width, int height, char *title);

/*
fun:    根据宽,高,标题将每个像素的RGB值在指定路径生成png文件
return: int型,0表示正确,1表示出错
arg[0]:    filename,生成的文件名字
arg[1]: width,图片宽
arg[2]: height,图片高
arg[3]: title,标题
*/
int writeImage(char *filename, int width, int height, char *title)
{
    int code = 0;
    FILE *fp = NULL;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    png_bytep row = NULL;

    // Open file for writing (binary mode)
    fp = fopen(filename, "wb");
    if (fp == NULL) {
        fprintf(stderr, "Could not open file %s for writing\n", filename);
        code = 1;
        goto finalise;
    }
    // Initialize write structure
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fprintf(stderr, "Could not allocate write struct\n");
        code = 1;
        goto finalise;
    }
    // Initialize info structure
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fprintf(stderr, "Could not allocate info struct\n");
        code = 1;
        goto finalise;
    }
    // Setup Exception handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        fprintf(stderr, "Error during png creation\n");
        code = 1;
        goto finalise;
    }

    png_init_io(png_ptr, fp);

    // Write header (8 bit colour depth)
    png_set_IHDR(png_ptr, info_ptr, width, height, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

    // Set title
    if (title != NULL) {
        png_text title_text;
        title_text.compression = PNG_TEXT_COMPRESSION_NONE;
        title_text.key = "Title";
        title_text.text = title;
        png_set_text(png_ptr, info_ptr, &title_text, 1);
    }
    png_write_info(png_ptr, info_ptr);

    // Allocate memory for one row (4 bytes per pixel - RGBA)
    row = (png_bytep) malloc(4 * width * sizeof(png_byte));

    // Write image data
    int x, y;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            if (x == 0 || x == (width - 1) || y == 0 || y == (height - 1)) {
                row[x * 4 + 0] = 0x00;
                row[x * 4 + 1] = 0x00;
                row[x * 4 + 2] = 0x00;
                row[x * 4 + 3] = 0xff;
            } else {
                row[x * 4 + 0] = 0x00;
                row[x * 4 + 1] = 0x00;
                row[x * 4 + 2] = 0xff;
                row[x * 4 + 3] = 0xff;
            }
        }
        png_write_row(png_ptr, row);
    }

    // End write
    png_write_end(png_ptr, NULL);

finalise:
    if (fp != NULL)
        fclose(fp);
    if (info_ptr != NULL)
        png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    if (png_ptr != NULL)
        png_destroy_write_struct(&png_ptr, (png_infopp) NULL);
    if (row != NULL)
        free(row);

    return code;
}

int main(int argc, char *argv[])
{
    // Make sure that the output filename argument has been provided
    if (argc < 2) {
        fprintf(stderr, "Please specify output file\n");
        return 1;
    }
    // Specify an output image size
    int width = 50;
    int height = 50;

    if (argc == 4) {
        width = atoi(argv[2]);
        height = atoi(argv[3]);
    }
    // Save the image to a PNG file
    // The 'title' string is stored as part of the PNG file
    printf("Saving PNG\n");
    int result = writeImage(argv[1], width, height, "This is my test image");
    if (result) {
        printf("Saving err\n");
    }
    //// Free up the memorty used to store the image
    //free(buffer);

    return result;
}
