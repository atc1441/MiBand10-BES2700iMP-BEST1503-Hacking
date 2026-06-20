/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: Rotate/Scale/Translate
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_rotate/vg_lite_blit
 Description: Load outside landscape image, then blit it to blue dest buffer with rotate/scale/translate.
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

vg_lite_color_key4_t colorkey=
{
    {1,0,0,0,0x55,100,100,100},
    {1,0,0,0,0x55,100,100,100},
    {1,0,0,0,0x55,0,0,0},
    {1,0,0,0,0x55,0,0,0},
};

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;

    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    if(!vg_lite_query_feature(gcFEATURE_BIT_VG_COLOR_KEY))
        goto ErrorHandler;

    filter = VG_LITE_FILTER_POINT;

    if (vg_lite_load_raw(&image, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;
    vg_lite_set_color_key(colorkey);

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFF323232));
    vg_lite_identity(&matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image.width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image.height, &matrix);

    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));
    CHECK_ERROR(vg_lite_finish());

    vg_lite_save_raw("colorkey.raw", fb);

ErrorHandler:

    cleanup();
    return 0;
}
