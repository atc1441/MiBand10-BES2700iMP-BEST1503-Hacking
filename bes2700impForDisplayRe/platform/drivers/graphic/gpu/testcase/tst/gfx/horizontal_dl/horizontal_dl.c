#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   1920
static int   fb_width = 1920, fb_height = 1080;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t raw;


static int32_t path_data[] = {
    2, 0, 0,
    4, DEFAULT_SIZE, 0,
    4, DEFAULT_SIZE, 1,
    4, 0, 1,
    0,
};

static vg_lite_path_t path = {
    {0, 0, /* left,top */
    DEFAULT_SIZE, 1}, /* right,bottom */
    VG_LITE_HIGH, /* quality */
    VG_LITE_S32, /* -128 to 127 coordinate range */
    {0}, /* uploaded */
    sizeof(path_data), /* path length */
    path_data, /* path data */
    1
};

void cleanup(void)
{

    if (buffer.handle != NULL) {
        /* Free the offscreen framebuffer memory. */
        vg_lite_free(&buffer);
    }
    if (raw.handle != NULL) {
        /* Free the raw memory. */
        vg_lite_free(&raw);
    }
    vg_lite_clear_path(&path);
}

int main(int argc, const char * argv[])
{
    vg_lite_matrix_t matrix;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    /* Initialize the draw. */
    error = vg_lite_init(fb_width, fb_height);
    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    filter = VG_LITE_FILTER_POINT;

    /* Try to open the framebuffer. */
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;
    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, 0xFFFF0000);

    /* Setup a 10x10 scale at center of buffer. */
    vg_lite_identity(&matrix);
    vg_lite_translate(0, fb_height / 2.0f, &matrix);

    /* Draw the path using the matrix. */
    error = vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF);
    if (error) {
        printf("vg_lite_draw() returned error %d\n", error);
        cleanup();
        return -1;
    }
    vg_lite_finish();
    /* Save PNG file. */
    vg_lite_save_png("horizontal.png", fb);
    /* Cleanup. */
    cleanup();
    return 0;
}
