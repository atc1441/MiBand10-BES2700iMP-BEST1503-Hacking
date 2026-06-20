/*
 Resolution: 256 x 256
 Format: VG_LITE_RGB565
 Transformation: Translate/Scale
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_draw
 Description: Draw a polygon vector graphic with high render quality on blue buffer.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define DEFAULT_SIZE   454.0f;
#define __func__ __FUNCTION__
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
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

vg_lite_path_t path1;
static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the offscreen framebuffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path1);
    vg_lite_close();
}

int arcpath_main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    char    *pchar;
    float   *pfloat;
    int32_t data_size;
    vg_lite_linear_gradient_t gradient;
    vg_lite_matrix_t *gradMatrix;
    vg_lite_matrix_t primaryMatrix;
    uint32_t gradColors[] = {0xAA10ff10,0xAAff10ff};
    uint32_t gradStops[] = {0, 200};

    TC_INIT

    data_size = 4 * 9 + 1 + 4 * 6+ 4 * 6+ 4 * 6;
    path1.path = gpu_malloc(data_size);

    pchar = (char*)path1.path;
    pfloat = (float*)path1.path;
    *pchar = 0x02;
    pfloat++;
    *pfloat++ = 230;
    *pfloat++ = 0;

    pchar = (char*)pfloat;
    *pchar = 0x0E;
    pfloat++;
    *pfloat++ = 230;
    *pfloat++ = 230;
    *pfloat++ = 0;
    *pfloat++ = 0;
    *pfloat++ = 230;

    pchar = (char*)pfloat;
    *pchar = 0x0E;
    pfloat++;
    *pfloat++ = 5;
    *pfloat++ = 5;
    *pfloat++ = 0;
    *pfloat++ = 10;
    *pfloat++ = 230;

    pchar = (char*)pfloat;
    *pchar = 0x10;
    pfloat++;
    *pfloat++ = 220;
    *pfloat++ = 220;
    *pfloat++ = 0;
    *pfloat++ = 230;
    *pfloat++ = 10;

    pchar = (char*)pfloat;
    *pchar = 0x0E;
    pfloat++;
    *pfloat++ = 5;
    *pfloat++ = 5;
    *pfloat++ = 0;
    *pfloat++ = 230;
    *pfloat++ = 0;

    pchar = (char*)pfloat;
    *pchar = 0;

    vg_lite_init_arc_path(&path1,
                       VG_LITE_FP32,
                       VG_LITE_HIGH,
                       data_size,
                       path1.path,
                       0, 0,
                       100, 100);

    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    memset(&gradient, 0, sizeof(gradient));
    CHECK_ERROR(vg_lite_init_grad(&gradient));
    CHECK_ERROR(vg_lite_set_grad(&gradient, 2, gradColors, gradStops));
    CHECK_ERROR(vg_lite_update_grad(&gradient));

    vg_lite_identity(&primaryMatrix);
    // Set up grad matrix to translate to the edge of the square path
    gradMatrix = vg_lite_get_grad_matrix(&gradient);
    *gradMatrix = primaryMatrix;
    vg_lite_scale(2, 2, gradMatrix);
    fb_scale = (float)fb_width / DEFAULT_SIZE;

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;

    buffer.format = GPU_FB_FORMAT;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFF101010));
    // *** DRAW ***
    vg_lite_identity(&matrix);
    CHECK_ERROR(vg_lite_draw_gradient(fb, &path1, VG_LITE_FILL_EVEN_ODD, &matrix, &gradient, VG_LITE_BLEND_SRC_OVER));
    CHECK_ERROR(vg_lite_finish());

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("clear.raw", fb);

    gpu_set_soft_break_point();


ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
