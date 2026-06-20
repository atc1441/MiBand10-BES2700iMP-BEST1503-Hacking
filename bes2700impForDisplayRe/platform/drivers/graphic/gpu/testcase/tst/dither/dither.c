/*
 Resolution: 320 x 480
 Format: VG_LITE_RGB565/VG_LITE_ABGR1555/VG_LITE_ARGB4444
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_blit
 Description: Test dither function.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
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

static int   fb_width = 320, fb_height = 480;
static float fb_scale = 1.0f;
static vg_lite_buffer_t buffer[10];
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;

static vg_lite_buffer_format_t formats[] = {
    VG_LITE_RGB565,
    VG_LITE_BGR565,
    VG_LITE_ABGR1555,
    VG_LITE_ARGB1555,
    VG_LITE_BGRA5551,
    VG_LITE_RGBA5551,
    VG_LITE_RGBA4444,
    VG_LITE_BGRA4444,
    VG_LITE_ABGR4444,
    VG_LITE_ARGB4444,
};

void cleanup(void)
{
    int i;
    if (image.handle != NULL) {
        vg_lite_free(&image);
    }
    for (i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
        if (buffer[i].handle != NULL) {
            vg_lite_free(&buffer[i]);
        }
    }
    vg_lite_close();
}


int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    char fname[64];
    vg_lite_error_t error;
    int i;
    filter = VG_LITE_FILTER_POINT;
    error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_init(320, 480));

    if (vg_lite_load_raw(&image, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    /* enable dither function */
    vg_lite_set_dither(1);

    for(i = 0; i < sizeof(formats)/sizeof(formats[0]); i++) {
        buffer[i].width  = fb_width;
        buffer[i].height = fb_height;
        buffer[i].format = formats[i];

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));
        fb = &buffer[i];

        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        vg_lite_identity(&matrix);

        CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));
        CHECK_ERROR(vg_lite_finish());

        sprintf(fname, "dither_%d.png", i);
        vg_lite_save_png(fname, &buffer[i]);
    }
    vg_lite_set_dither(0);

ErrorHandler:
    cleanup();
    return 0;
}
