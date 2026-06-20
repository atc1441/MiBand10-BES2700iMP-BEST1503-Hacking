#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&image);
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    filter = VG_LITE_FILTER_POINT;

    /* Allocate the buffer. */
    if (argc == 3) {
        buffer.width = atoi(argv[1]);
        buffer.height = atoi(argv[2]);
    } else {
        buffer.width  = 640;
        buffer.height = 480;
    }
    /* Initialize the draw. */
    error = vg_lite_init(buffer.width, buffer.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    /* Load the image. */
    if (!vg_lite_load_png(&image, "image.png")) {
        printf("vg_lite_load_png() coud not load the image 'landscape.png'\n");
        cleanup();
        return -1;
    }
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, 0xFFFF0000);

    /* Build a X degree rotation matrix from the center of the buffer. */
    vg_lite_identity(&matrix);
    vg_lite_translate(fb->width / 2.0f, fb->height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb->width / -2.0f, fb->height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb->width / (vg_lite_float_t) image.width,
                  (vg_lite_float_t) fb->height / (vg_lite_float_t) image.height, &matrix);

    /* Blit the image using the matrix. */
    vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter);
    vg_lite_finish();
    /* Save PNG file. */
    vg_lite_save_png("gfx3.png", fb);

    /* Cleanup. */
    cleanup();
    return 0;
}
