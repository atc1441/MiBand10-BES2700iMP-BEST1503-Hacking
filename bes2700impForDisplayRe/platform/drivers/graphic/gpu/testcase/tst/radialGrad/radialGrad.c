
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DEFAULT_SIZE   480.0f;
#define DEFAULT_WIDTH 480
#define DEFAULT_HEIGHT 640

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
static int   fb_width = DEFAULT_WIDTH, fb_height = DEFAULT_HEIGHT;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
int frames = 4;
vg_lite_radial_gradient_spreadmode_t spreadmode[] = {
    VG_LITE_RADIAL_GRADIENT_SPREAD_FILL,
    VG_LITE_RADIAL_GRADIENT_SPREAD_PAD,
    VG_LITE_RADIAL_GRADIENT_SPREAD_REPEAT,
    VG_LITE_RADIAL_GRADIENT_SPREAD_REFLECT,
};
/*
*-----*
/       \
/         \
*           *
|          /
|         X
|          \
*           *
\         /
\       /
*-----*
*/
static char path_data[] = {
    2, 0, 0,
    4, 120, 0,
    4, 120, 120,
    4, 0, 120,
    0,
};

static vg_lite_path_t path = {
    { 0,   0,
      120, 120 },
    VG_LITE_HIGH,
    VG_LITE_S8,
    {0},
    sizeof(path_data),
    path_data,
    1
};

void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_radial_gradient_t grad;

    vg_lite_color_ramp_t vgColorRamp[] =
    {
        {
            0.0f,
            0.4f, 0.0f, 0.6f, 1.0f
        },
        {
            0.25f,
            0.9f, 0.5f, 0.1f, 1.0f
        },
        {
            0.5f,
            0.8f, 0.8f, 0.0f, 1.0f
        },
        {
            0.75f,
            0.0f, 0.3f, 0.5f, 1.0f
        },
        {
            1.00f,
            0.4f, 0.0f, 0.6f, 1.0f
        }
    };
    vg_lite_radial_gradient_parameter_t radialGradient = {256.0f ,256.0f ,256.0f ,256.0f,223.0f};
    vg_lite_matrix_t *matGrad;
    vg_lite_matrix_t matPath;
    int fcount = 0;
    char filename[20];
    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_BGRX8888;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    printf("Render size: %d x %d\n", fb_width, fb_height);
    while (frames > 0)
    {
        sprintf(filename,"radialGrad_%d.png",frames+1);
        memset(&grad, 0, sizeof(grad));
        vg_lite_set_rad_grad(&grad, 5,vgColorRamp,radialGradient,spreadmode[fcount],1);
        vg_lite_update_rad_grad(&grad);
        matGrad = vg_lite_get_rad_grad_matrix(&grad);
        vg_lite_identity(matGrad);

        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xffffffff));
        vg_lite_identity(&matPath);
        vg_lite_scale(2.0f, 2.0f, &matPath);
        vg_lite_translate(100.0f,100.0f,&matPath);

        CHECK_ERROR(vg_lite_draw_radial_gradient(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &grad,0,VG_LITE_BLEND_NONE,VG_LITE_FILTER_LINEAR));
        CHECK_ERROR(vg_lite_finish());
        vg_lite_clear_rad_grad(&grad);
        frames--;
        printf("frame %d done\n", fcount++);
        vg_lite_save_png(filename, fb);
    }

ErrorHandler:

    cleanup();
    return 0;
}
