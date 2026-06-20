#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "gpu_common.h"

//#define GPU_INFO(...)
#define GPU_INFO(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)




//#define DEFAULT_SIZE   320.0f;
#define DEFAULT_SIZE   160.0f;

#define TEST_ALIGMENT  16
#define __func__ __FUNCTION__

//static int   fb_width = 320, fb_height = 480;
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
//static vg_lite_buffer_t * sys_fb;   //system framebuffer object to show the rendering result.
static vg_lite_buffer_t * fb;
//static int has_fb = 0;

static vg_lite_buffer_t image;
static char *error_type[] =
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

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image);
    }
    vg_lite_close();
}

void create_imgA4(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
    };


    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % TEST_ALIGMENT], buffer->stride);
        p += buffer->stride;
    }
}
int imgA4_test_main_entry(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    //uint32_t colors[256] = {0};
    vg_lite_matrix_t matrix;

    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s", __func__);
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_BI_LINEAR;

    image.format = VG_LITE_A4;
    image.width = fb_width;//256;
    image.height = fb_height;//256;
    image.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    image.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(&image));
    memset(image.memory, 0xc0, image.width * image.height/2);
    create_imgA4(&image);

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    //printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;
    // Build a 33 degree matrix from the center of the buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb_width / -2.0f, fb_height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image.width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image.height, &matrix);

    // Clear the buffer with red.
    CHECK_ERROR(vg_lite_clear( &buffer, NULL, 0xFF0000FF));
    // Blit the image using the matrix.
    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_SRC_OVER, 0xFF00FF00, filter));
    CHECK_ERROR(vg_lite_finish());

    gpu_lcdc_display(&buffer);

    // Save PNG file.
    vg_lite_save_png("imgA4.png", &buffer);

    gpu_set_soft_break_point();

ErrorHandler:
    // Cleanup.
    cleanup();
    GPU_REP("%s test pass", __func__);
    return 0;
}

#if 0 // testcase with different ts buffer size
static uint32_t imgA4_cmd_i_256_ts_32_32_fb_160_240_clear[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00001f80,
0x30010acb,	0x200521c0,	0x30010acc,	0x00000080,
0x30010a10,	0x00010123,	0x30010a11,	0x20092240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a34,	0x00000000,	0x30010a00,	0x10000001,
0x30010a02,	0xff0000ff,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20052240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000400,	0x30010a2d,	0x00000000,
0x30010a2f,	0x01000100,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,
};


static uint32_t imgA4_cmd_i_128_ts_32_32_fb_160_240_clear[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00001f80,
0x30010acb,	0x200521c0,	0x30010acc,	0x00000080,
0x30010a10,	0x00010123,	0x30010a11,	0x20062240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a34,	0x00000000,	0x30010a00,	0x10000001,
0x30010a02,	0xff0000ff,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20052240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000200,	0x30010a2d,	0x00000000,
0x30010a2f,	0x00800080,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,
};

static uint32_t imgA4_cmd_i_128_ts_32_32_fb_160_240_clear[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00001f80,
0x30010acb,	0x200521c0,	0x30010acc,	0x00000080,
0x30010a10,	0x00010123,	0x30010a11,	0x20062240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a34,	0x00000000,	0x30010a00,	0x10000001,
0x30010a02,	0xff0000ff,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20052240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000200,	0x30010a2d,	0x00000000,
0x30010a2f,	0x00800080,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,
};

static uint32_t imgA4_cmd_i_128_ts_32_32_fb_160_240[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00001f80,
0x30010acb,	0x200521c0,	0x30010acc,	0x00000080,
0x30010a10,	0x00010123,	0x30010a11,	0x20062240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20052240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000200,	0x30010a2d,	0x00000000,
0x30010a2f,	0x00800080,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,

};

static uint32_t imgA4_cmd_i_128_ts_64_64_fb_160_240[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00001f40,
0x30010acb,	0x20052180,	0x30010acc,	0x000000c0,
0x30010a10,	0x00010123,	0x30010a11,	0x20062240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20052240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000200,	0x30010a2d,	0x00000000,
0x30010a2f,	0x00800080,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,
};

static uint32_t imgA4_cmd_i_128_ts_128_128_fb_160_240[]={
0x30010a35,	0x20050240,	0x30010ac8,	0x00003e80,
0x30010acb,	0x200540c0,	0x30010acc,	0x00000180,
0x30010a10,	0x00010123,	0x30010a11,	0x20064240,
0x30010a12,	0x00000280,	0x30010a13,	0x00f000a0,
0x30010a00,	0x0000a101,	0x30010a02,	0xff00ff00,
0x30010a18,	0xbea5a0b5,	0x30010a19,	0x3e8690d3,
0x30010a1a,	0x3f800000,	0x30010a1c,	0x3babc27e,
0x30010a1d,	0xbb14b908,	0x30010a1e,	0x00000000,
0x30010a1f,	0x00000001,	0x30010a20,	0x3b5f158c,
0x30010a21,	0x3b650352,	0x30010a22,	0x00000000,
0x30010a25,	0x00020027,	0x30010a27,	0x00000000,
0x30010a29,	0x20054240,	0x30010a34,	0x00000000,
0x30010a2b,	0x00000200,	0x30010a2d,	0x00000000,
0x30010a2f,	0x00800080,	0x40000001,	0x00000000,
0x00000000,	0x00f000a0,	0x30010a1b,	0x00000001,
0x10000007,	0x00000000,	0x20000007,	0x00000000,
0x30010a1b,	0x00000001,	0x10000007,	0x00000000,
0x20000007,	0x00000000,	0x00000000,	0x00000000,

};
#endif