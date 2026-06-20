/*
 GrayRidge sample.
 This case render a image into a A8 buffer, then render this A8 buffer as input into a A8 framebuffer(output).
 part1(COMPRESSION == 1): render a image into a A8 buffer.
 part2(COMPRESSION == 0): render an input A8 buffer into a framebuffer
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   640.0f;
#define COMPRESSION 0
static int   fb_width = 640, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

static int img_size = 128;

static vg_lite_buffer_t img_buffer, paint_buffer, a8_buffer;

void loadImages()
{
    if (!vg_lite_load_png(&img_buffer, "landscape1.png"))
    {
        printf("Can't load image file %s\n", "landscape1.png");
    }
}

vg_lite_error_t createBuffers()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    paint_buffer.format = VG_LITE_RGBA8888;
    paint_buffer.width  = img_size;
    paint_buffer.height = img_size;
    paint_buffer.stride = 0;
    error = vg_lite_allocate(&paint_buffer);

    a8_buffer.format = VG_LITE_A8;
    a8_buffer.width  = img_size;
    a8_buffer.height = img_size;
    a8_buffer.stride = 0;
    error = vg_lite_allocate(&a8_buffer);

    return error;
}
void cleanup(void)
{
    if (a8_buffer.handle != NULL) {
        vg_lite_free(&a8_buffer);
    }
    if (paint_buffer.handle != NULL) {
        vg_lite_free(&paint_buffer);
    }
    if (img_buffer.handle != NULL) {
        vg_lite_free(&img_buffer);
    }
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

}

int main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_filter_t filter;
    vg_lite_matrix_t    mat_src, mat_a8, mat_bg;

    filter = VG_LITE_FILTER_POINT;
    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
    
    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_A8;
    /* Initialize the draw. */
    error = vg_lite_init(buffer.width, buffer.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;
    createBuffers();
    loadImages();
    /* Clear the buffer. */
    vg_lite_clear(fb, NULL, 0xffa0a0a0);

    vg_lite_clear(&paint_buffer, NULL, 0x70434321);
    /* Render the src image into a8 buffers. */
    vg_lite_identity(&mat_bg);
    vg_lite_scale(img_size / (float) img_buffer.width,
                  img_size / (float) img_buffer.height, &mat_bg);
    vg_lite_identity(&mat_src);

    /* Test for a8 swizzle. */
    vg_lite_blit2(&a8_buffer, &paint_buffer,
                  &img_buffer, &mat_src, &mat_bg, VG_LITE_BLEND_SRC_OVER, filter);
    /* Then render source and a8 into framebuffer. */
    vg_lite_identity(&mat_a8);
    vg_lite_translate(img_size + 10, 0, &mat_a8);
    vg_lite_scale(fb_scale, fb_scale, &mat_a8);

    vg_lite_blit(fb, &a8_buffer, &mat_a8, VG_LITE_BLEND_NONE, 0, filter);
    vg_lite_finish();
    vg_lite_save_png("greyridge.png", fb);

    /* Cleanup. */
    cleanup();
    return 0;
}
