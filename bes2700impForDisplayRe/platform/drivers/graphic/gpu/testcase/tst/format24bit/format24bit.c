/*
 Resolution: 320 x 480
 Format: VG_LITE_ABGR8565,VG_LITE_BGRA5658,VG_LITE_ARGB8565,VG_LITE_RGBA5658,VG_LITE_RGB888,VG_LITE_BGR888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_draw_pattern/vg_lite_blit
 Description: Load outside landscape image, drawpattern it to blue dest buffer with 24bit format, then blit it to blue dest buffer with 24bit format.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   320.0f;

#define DEFAULT_WIDTH 320
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
static vg_lite_buffer_t buffer[6];
static vg_lite_buffer_t buffer1[6];
static vg_lite_buffer_t image;
static vg_lite_buffer_t * fb;

static char path_data[] = {
    2, -5, -10, /* moveto   -5,-10 */
    4, 5, -10,  /* lineto    5,-10 */
    4, 10, -5,  /* lineto   10, -5 */
    4, 0, 0,    /* lineto    0,  0 */
    4, 10, 5,   /* lineto   10,  5 */
    4, 5, 10,   /* lineto    5, 10 */
    4, -5, 10,  /* lineto   -5, 10 */
    4, -10, 5,  /* lineto  -10,  5 */
    4, -10, -5, /* lineto  -10, -5 */
    0, /* end */
};

static vg_lite_path_t path = {
    {-10, -10, /* left,top */
        10, 10}, /* right,bottom */
    VG_LITE_HIGH, /* quality */
    VG_LITE_S8, /* -128 to 127 coordinate range */
    {0}, /* uploaded */
    sizeof(path_data), /* path length */
    path_data, /* path data */
    1
};

static vg_lite_buffer_format_t  formats[6] = {  
    VG_LITE_ABGR8565,
    VG_LITE_BGRA5658,
    VG_LITE_ARGB8565,
    VG_LITE_RGBA5658,
    VG_LITE_RGB888,
    VG_LITE_BGR888,
};

void cleanup(void)
{
    int i;
    if (image.handle != NULL) {
        vg_lite_free(&image);
    }
    for (i = 0; i < 6; i++) {
        if (buffer[i].handle != NULL) {
            vg_lite_free(&buffer[i]);
        }
        if (buffer1[i].handle != NULL) {
            vg_lite_free(&buffer1[i]);
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

    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_24BIT);
    if (!feature_check) {
        printf("24bit format is not supported.\n");
        cleanup();
        return -1;
    }
    filter = VG_LITE_FILTER_POINT;

    /* Load the image. */
    if (0 != vg_lite_load_raw(&image, "landscape.raw")) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    /* Allocate the off-screen buffer. */
    for (i = 0; i < 6; i++)
    {
        buffer1[i].format = buffer[i].format = formats[i];
        buffer1[i].width = buffer[i].width = fb_width;
        buffer1[i].height = buffer[i].height = fb_height;

        error = vg_lite_allocate(&buffer[i]);
        error = vg_lite_allocate(&buffer1[i]);
        if (error != VG_LITE_SUCCESS) {
            return error;
        }
    }

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    for (i = 0; i < 6; i++)
    {
        /* Clear the buffer with blue. */
        CHECK_ERROR(vg_lite_clear(&buffer[i], NULL, 0xFFFF0000));
        CHECK_ERROR(vg_lite_clear(&buffer1[i], NULL, 0xFFFF0000));
        /* Build a 33 degree rotation matrix from the center of the buffer. */
        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -2.0f, fb_height / -4.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matPath);
        vg_lite_scale(10, 10, &matPath);

        CHECK_ERROR(vg_lite_draw_pattern(&buffer1[i], &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter));
        vg_lite_identity(&matrix);
        CHECK_ERROR(vg_lite_blit(&buffer[i], &buffer1[i], &matrix, VG_LITE_BLEND_NONE, 0, filter));
        CHECK_ERROR(vg_lite_finish());
        sprintf(fname, "24bit_%d.png", i);
        vg_lite_save_png(fname, &buffer[i]);
    }

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}
