#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
#define __func__ __FUNCTION__
char *error_type[] = 
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }
static int   fb_width = 256, fb_height = 256;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;

static vg_lite_buffer_t image;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        vg_lite_free(&image);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    uint8_t *ptr;
    int i = 0;

    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(0, 0));

    filter = VG_LITE_FILTER_POINT;

    image.width = 256;
    image.height = 256;
    image.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&image));
    ptr = (uint8_t *) image.memory;
    for(i = 0; i < image.height; i++)
    {
        memset(ptr,i,image.stride);
        ptr += image.stride;
    }

    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA2222;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFFFFFF));
    /* Build a 33 degree rotation matrix from the center of the buffer. */
    vg_lite_identity(&matrix);

    /* Blit the image using the matrix. */
    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));
    CHECK_ERROR(vg_lite_finish());
    /* Save PNG file. */
    vg_lite_save_png("ARGB2222_output.png", fb);

    CHECK_ERROR(vg_lite_clear(&image, NULL, 0xFFFFFFFF));
    /* Blit the image using the matrix. */
    CHECK_ERROR(vg_lite_blit( &image,fb, &matrix, VG_LITE_BLEND_NONE, 0, filter));
    CHECK_ERROR(vg_lite_finish());
    /* Save PNG file. */
    vg_lite_save_png("ARGB2222_input.png", &image);

ErrorHandler:
    cleanup();
    return 0;
}
