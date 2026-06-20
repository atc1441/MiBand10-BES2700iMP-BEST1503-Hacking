#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DDRLESS 0
static int   fb_width = 1920, fb_height = 1080;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t raw;

void cleanup(void)
{

    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    if (raw.handle != NULL) {
        /* Free the raw memory. */
        vg_lite_free(&raw);
    }
}

int main(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;

    /* Initialize the blitter. */
#if DDRLESS
    vg_lite_error_t error = vg_lite_init(fb_width, fb_height);
#else
    vg_lite_error_t error = vg_lite_init(0, 0);
#endif
    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        return -1;
    }

    filter = VG_LITE_FILTER_POINT;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
#if DDRLESS
    buffer.format = VG_LITE_RGBA8888;
#else
    buffer.format = VG_LITE_RGB565;
#endif
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    /* Clear the buffer with blue. */
    error = vg_lite_clear(fb, NULL, 0xFFFF0000);
    if (error) {
        printf("vg_lite_clear() returned error %d\n", error);
        cleanup();
        return -1;
    }

    vg_lite_finish();
    /* Save PNG file. */
#if DDRLESS
    vg_lite_save_png("clear_dl.png", fb);
#else
    vg_lite_save_png("clear_gfx.png", fb);
#endif

    /* Cleanup. */
    cleanup();
    return 0;
}
