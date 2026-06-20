/*
 Double image sample.
 This case draws a path to a buffer, then combine the path buffer with the image together into the framebuffer.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;
static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;    /* offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t raw;

static int img_size = 128;
static uint32_t clear_color = 0XFFFF0000;
static vg_lite_buffer_format_t  formats[4] = {
    VG_LITE_YUY2_TILED,
    VG_LITE_NV12_TILED,
    VG_LITE_ANV12_TILED,
    VG_LITE_AYUY2_TILED,
};

static vg_lite_buffer_t img_buffer, paint_buffer, src_buffer, yuv_buffers[4];

void loadImages()
{
    if (vg_lite_load_raw(&img_buffer, "folder.raw") != 0) {
        printf("load raw file error\n");
    }
}

vg_lite_error_t createBuffers()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i;

    paint_buffer.format = src_buffer.format = VG_LITE_RGBA8888;
    paint_buffer.width = src_buffer.width   = img_size;
    paint_buffer.height = src_buffer.height = img_size;
    paint_buffer.stride = src_buffer.stride = 0;
    error = vg_lite_allocate(&paint_buffer);
    error = vg_lite_allocate(&src_buffer);

    for (i = 0; i < 4; i++)
    {
        yuv_buffers[i].format = formats[i];
        yuv_buffers[i].width = img_size;
        yuv_buffers[i].height = img_size;
        yuv_buffers[i].stride = 0;

        error = vg_lite_allocate(&yuv_buffers[i]);

        if (error != VG_LITE_SUCCESS)
            break;
    }
    return error;
}
void cleanup(void)
{
    int i;

    if (src_buffer.handle != NULL) {
        vg_lite_free(&src_buffer);
    }
    for (i = 0; i < 4; i++) {
        if (yuv_buffers[i].handle != NULL) {
            vg_lite_free(&yuv_buffers[i]);
        }
    }
    if (img_buffer.handle != NULL)
    {
        vg_lite_free(&img_buffer);
    }
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }
    if (paint_buffer.handle != NULL) {
        vg_lite_free(&paint_buffer);
    }
    if (raw.handle != NULL) {
        /* Free the raw memory. */
        vg_lite_free(&raw);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_matrix_t mat_src, mat_yuv[4], mat_bg;
    vg_lite_filter_t filter;
    char fname[64];
    int i;

    vg_lite_error_t error = vg_lite_init(320, 320);
    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return -1;
    }

    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_DOUBLE_IMAGE);
    if (!feature_check) {
        printf("double image is not supported.\n");
        cleanup();
        return -1;
    }
    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;

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

    createBuffers();
    loadImages();

    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, clear_color);

    vg_lite_clear(&paint_buffer, NULL, 0x66222288);
    vg_lite_clear(&src_buffer, NULL, 0);

    /* Render the src image into different yuv buffers. */
    vg_lite_identity(&mat_bg);
    vg_lite_scale(img_size / (float) img_buffer.width,
                  img_size / (float) img_buffer.height, &mat_bg);
    vg_lite_identity(&mat_src);
    vg_lite_blit2(&src_buffer, &paint_buffer, &img_buffer, &mat_src, &mat_bg, VG_LITE_BLEND_SRC_OVER, filter);
    vg_lite_finish();
    vg_lite_save_png("double_src.png", &src_buffer);

    /* Test for default UV swizzle. */
    for (i = 0; i < 4; i++)
    {
        vg_lite_blit2(&yuv_buffers[i], &paint_buffer,
                      &img_buffer, &mat_src, &mat_bg, VG_LITE_BLEND_SRC_OVER, filter);
    }

    /* Then render source and the 4 yuvs into framebuffer. */
    vg_lite_identity(&mat_src);
    vg_lite_scale(fb_scale, fb_scale, &mat_src);
    vg_lite_identity(&mat_yuv[0]);
    vg_lite_translate(img_size + 10, 0, &mat_yuv[0]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[0]);
    vg_lite_identity(&mat_yuv[1]);
    vg_lite_translate((img_size + 10) * 2, 0, &mat_yuv[1]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[1]);
    vg_lite_identity(&mat_yuv[2]);
    vg_lite_translate(img_size + 10, img_size + 10, &mat_yuv[2]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[2]);
    vg_lite_identity(&mat_yuv[3]);
    vg_lite_translate((img_size + 10) * 2, img_size + 10, &mat_yuv[3]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[3]);
    
    for (i = 0; i < 4; i++)
    {
        vg_lite_blit(fb, &yuv_buffers[i], &mat_yuv[0], VG_LITE_BLEND_NONE, 0, filter);
        sprintf(fname, "double_uv_%d.png", i);
        vg_lite_finish();
        vg_lite_save_png(fname, fb);
    }

    /* Test for VU swizzle. */
    for (i = 0; i < 4; i++)
    {
        yuv_buffers[i].yuv.swizzle = VG_LITE_SWIZZLE_VU;
        vg_lite_blit2(&yuv_buffers[i], &paint_buffer,
                      &img_buffer, &mat_src, &mat_bg, VG_LITE_BLEND_SRC_OVER, filter);
    }

    /* Then render source and the 4 yuvs into framebuffer. */
    vg_lite_identity(&mat_src);
    vg_lite_scale(fb_scale, fb_scale, &mat_src);
    vg_lite_identity(&mat_yuv[0]);
    vg_lite_translate(img_size + 10, 0, &mat_yuv[0]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[0]);
    vg_lite_identity(&mat_yuv[1]);
    vg_lite_translate((img_size + 10) * 2, 0, &mat_yuv[1]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[1]);
    vg_lite_identity(&mat_yuv[2]);
    vg_lite_translate(img_size + 10, img_size + 10, &mat_yuv[2]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[2]);
    vg_lite_identity(&mat_yuv[3]);
    vg_lite_translate((img_size + 10) * 2, img_size + 10, &mat_yuv[3]);
    vg_lite_scale(fb_scale, fb_scale, &mat_yuv[3]);

    for (i = 0; i < 4; i++)
    {
        vg_lite_blit(fb, &yuv_buffers[i], &mat_yuv[0], VG_LITE_BLEND_NONE, 0, filter);
        sprintf(fname, "double_vu_%d.png", i);
        vg_lite_finish();
        vg_lite_save_png(fname, fb);
    }

    /* Cleanup. */
    cleanup();
    return 0;
}