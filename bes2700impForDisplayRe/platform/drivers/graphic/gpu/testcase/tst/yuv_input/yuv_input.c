/*
 Resolution: 640 x 480
 Format: VG_LITE_NV12,VG_LITE_NV16,VG_LITE_YV12,VG_LITE_YV16,VG_LITE_BGRA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_blit/
 Description: Load outside yuv source, blit it to blue dest buffer with BGRA8888 format.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(
 ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE 640.0f;
#define DEFAULT_WIDTH 640
#define DEFAULT_HEIGHT 480

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

static int fb_width = DEFAULT_WIDTH, fb_height = DEFAULT_HEIGHT;
static float fb_scale = 1.0f;
static vg_lite_buffer_t srcbuffer[4];
static vg_lite_buffer_t dstbuffer;
static vg_lite_buffer_t image;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_format_t formats[] = {
    VG_LITE_NV12,
    VG_LITE_NV16,
    VG_LITE_YV12,
    VG_LITE_YV16,
};
void cleanup(void)
{
    int i;
    for (i = 0; i < sizeof(srcbuffer)/sizeof(srcbuffer[0]); i++) {
        if (srcbuffer[i].handle != NULL) {
            vg_lite_free(&srcbuffer[i]);
        }
        if (dstbuffer.handle != NULL) {
            vg_lite_free(&dstbuffer);
        }
    }
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    int i = 0;
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix, matPath;
    char fname[64];
    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_YUV_INPUT);
    if (!feature_check) {
        printf("yuv format input is not supported.\n");
        cleanup();
        return -1;
    }
    filter = VG_LITE_FILTER_POINT;

    for(i=0;i < sizeof(srcbuffer)/sizeof(srcbuffer[0]);i++) {
        srcbuffer[i].width = 640;
        srcbuffer[i].height = 480;
        srcbuffer[i].format = formats[i];
    }
    dstbuffer.width  = fb_width;
    dstbuffer.height = fb_height;
    dstbuffer.format = VG_LITE_BGRA8888;

    CHECK_ERROR(vg_lite_allocate(&dstbuffer));
    if (0 != vg_lite_load_raw_yuv(&srcbuffer[0], "nv12.raw")) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }
    if (0 != vg_lite_load_raw_yuv(&srcbuffer[1], "nv16.raw")) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }
    if (0 != vg_lite_load_raw_yuv(&srcbuffer[2], "yv12.raw")) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    vg_lite_yv12toyv16(&srcbuffer[3],&srcbuffer[2]);

    for (i = 0; i < sizeof(srcbuffer)/sizeof(srcbuffer[0]); i++) {
        CHECK_ERROR(vg_lite_clear(&dstbuffer, NULL, 0xFFFF0000));
        vg_lite_identity(&matrix);
        CHECK_ERROR(vg_lite_blit(&dstbuffer, &srcbuffer[i], &matrix, VG_LITE_BLEND_NONE, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "yuv_input_%d.png", i);
        vg_lite_save_png(fname, &dstbuffer);
    }

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}