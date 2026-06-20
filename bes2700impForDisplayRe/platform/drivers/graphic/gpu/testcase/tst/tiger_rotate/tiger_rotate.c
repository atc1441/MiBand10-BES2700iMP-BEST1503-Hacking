/*
 Resolution: 480 x 800
 Format: VG_LITE_ARGB8888
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_draw/vg_lite_scale/vg_lite_rotate
 Description: Draw a rotate and scale tiger picture .
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "tiger_paths.h"

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
static int fb_width = 480, fb_height = 800;
static vg_lite_buffer_t buffer;    /* offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

static int zoomOut    = 0;
static int scaleCount = 0;
static vg_lite_matrix_t matrix;
vg_lite_float_t rotate_degrees = 5.0;
vg_lite_float_t amplification_factor = 1.25;
vg_lite_float_t reduction_factor = 0.8;
void cleanup(void)
{
    int32_t i;

    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }
    
    vg_lite_close();
}

void animateTiger()
{
    if (zoomOut)
    {
        vg_lite_scale(amplification_factor, amplification_factor, &matrix);
        if (0 == --scaleCount)
            zoomOut = 0;
    }
    else
    {
        vg_lite_scale(reduction_factor, reduction_factor, &matrix);
        if (5 == ++scaleCount)
            zoomOut = 1;
    }

    vg_lite_rotate(rotate_degrees, &matrix);
}

int main(int argc, const char * argv[])
{
    int i,j;
    char name[20];
    vg_lite_filter_t filter;
    vg_lite_error_t error;
    if (argc == 4) {
        rotate_degrees = atof(argv[1]);
        amplification_factor = atof(argv[2]);
        reduction_factor = atof(argv[3]);
    }

    /* Initialize vglite. */
    error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    /* Clear the buffer with blue. */
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    // Setup a scale at center of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2 - 20 * fb_width / 480.0f, fb_height / 2 - 100 * fb_height / 800.0f, &matrix);
    vg_lite_scale(4, 4, &matrix);
    vg_lite_scale(fb_width / 640.0f, fb_height / 480.0f, &matrix);

    for(j = 0; j < 15; j++)
    {
        /* Clear the buffer with blue. */
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        sprintf(name,"tiger_%d.png",j);
        /* Draw the path using the matrix. */
        for (i = 0; i < pathCount; i++)
        {
            CHECK_ERROR(vg_lite_draw(fb, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]));
        }
        CHECK_ERROR(vg_lite_finish());
        animateTiger();
        /* Save PNG file. */
        vg_lite_save_png(name, fb);
    }

ErrorHandler:
    /* Cleanup. */
    cleanup();
    return 0;
}