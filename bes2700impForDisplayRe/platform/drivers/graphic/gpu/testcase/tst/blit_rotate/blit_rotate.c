#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "landscape.h"

#define DEFAULT_SIZE   256.0f;
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

static int   fb_width = 640, fb_height = 960;

vg_lite_buffer_t renderTarget;
vg_lite_buffer_t offscreenBuf;
vg_lite_matrix_t matrix;


void cleanup(void)
{
    vg_lite_free(&renderTarget);
    vg_lite_free(&offscreenBuf);

    vg_lite_close();
}

static vg_lite_error_t vg_lite_load_image_src (vg_lite_buffer_t *buffer)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    unsigned char *pval8;
    int i;
    buffer->width = 304;
    buffer->height = 300;
    buffer->stride = 608;
    buffer->format = VG_LITE_BGR565;
    buffer->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    buffer->transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(buffer));
    pval8 = (unsigned char *)buffer->memory;
    memset(buffer->memory,0xff,buffer->stride * buffer->height);
    for(i = 0;i < buffer->height;i++){
        memcpy(pval8, &landscape_raw[i * 600], 600);
        pval8 += buffer->stride;
    }

    return error;
ErrorHandler:
    cleanup();
    return error;
}
int main(int argc, const char * argv[])
 {
    vg_lite_rectangle_t rect = { 64, 64, 64, 64 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    char filename[20];
    int i;
    float angle = 0;

    CHECK_ERROR(vg_lite_init(16, 16));

    vg_lite_load_image_src(&offscreenBuf);

    renderTarget.width = 720;
    renderTarget.height = 1280;
    renderTarget.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&renderTarget));

    for(i = 0 ; i < 360; i++){
        sprintf(filename,"blit_rotate%d.png",i);
        CHECK_ERROR(vg_lite_clear(&renderTarget,NULL,0xffff00ff));
        vg_lite_identity(&matrix);
        vg_lite_translate(350, 400, &matrix);
        vg_lite_rotate(angle, &matrix);
        CHECK_ERROR(vg_lite_blit( &renderTarget, &offscreenBuf, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF, VG_LITE_FILTER_BI_LINEAR));
        CHECK_ERROR(vg_lite_finish());
        vg_lite_save_png(filename,&renderTarget);
        angle++;
    }

ErrorHandler:

    cleanup();
    return 0;
}
