/*
 Double image sample.
 This case draws a path to a buffer, then combine the path buffer with the image together into the framebuffer.
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define FB_WIDTH    640
#define FB_HEIGHT   480
#define COMPRESSION 0

static int has_blitter = 0;
static int has_draw = 0;
static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static int img_size = 128;
static int has_fb = 0;
static uint32_t clear_color = 0XFFFF0000;
static vg_lite_buffer_format_t  formats[4] = {
    VG_LITE_YUY2,
    VG_LITE_NV12,
    VG_LITE_ANV12,
    VG_LITE_AYUY2,
};

static vg_lite_buffer_t img_buffer, paint_buffer, yuv_buffers[4];
/*
 A rectangle path with original size 10x10 @ (0, 0)
 */

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

    paint_buffer.format  = VG_LITE_BGRA8888;
    paint_buffer.width = img_size;
    paint_buffer.height = img_size;
    paint_buffer.stride = 0;
    error = vg_lite_allocate(&paint_buffer);

    yuv_buffers[0].format = formats[0];
    yuv_buffers[0].width = img_size;
    yuv_buffers[0].height = img_size;
    yuv_buffers[0].stride = 0;

    error = vg_lite_allocate(&yuv_buffers[0]);

    return error;
}
void cleanup(void)
{
    if (yuv_buffers[0].handle != NULL) {
        vg_lite_free(&yuv_buffers[0]);
    }
    if (img_buffer.handle != NULL) {
        vg_lite_free(&img_buffer);
    }
    if (paint_buffer.handle != NULL) {
        vg_lite_free(&paint_buffer);
    }
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_matrix_t    mat_src, mat_yuv[4], mat_bg;
    int i, j = 0;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    /* Try to open the framebuffer. */
    buffer.width  = FB_WIDTH;
    buffer.height = FB_HEIGHT;
    buffer.format = VG_LITE_BGRA8888;
    filter = VG_LITE_FILTER_POINT;
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
    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, clear_color);
    vg_lite_clear(&paint_buffer, NULL, 0x22222288);

    /* Render the src image into different yuv buffers. */
    vg_lite_identity(&mat_bg);
    vg_lite_scale(img_size / (float) img_buffer.width,
                  img_size / (float) img_buffer.height, &mat_bg);
    vg_lite_identity(&mat_src);
    /* Test for default UV swizzle. */
    vg_lite_blit2(&yuv_buffers[0], &img_buffer, &paint_buffer,&mat_bg, &mat_src, VG_LITE_BLEND_SRC_OVER,filter);
    /* Then render source and the 4 yuvs into framebuffer. */
    vg_lite_identity(&mat_src);
    vg_lite_identity(&mat_yuv[0]);
    vg_lite_translate(img_size + 10, 0, &mat_yuv[0]);

    for (i = 0; i < 1; i++)
    {
        vg_lite_blit(fb, &yuv_buffers[0], &mat_yuv[0], VG_LITE_BLEND_NONE, 0, filter);
        vg_lite_finish();
        vg_lite_save_png("lossy1.png", fb);
    }

    /* Cleanup. */
    cleanup();
    return 0;
}
