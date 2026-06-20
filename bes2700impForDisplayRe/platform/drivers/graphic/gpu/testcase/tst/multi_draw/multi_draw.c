
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * sys_fb;   //system framebuffer object to show the rendering result.
static int has_fb = 0;
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


#define UI_WIDTH        480
#define UI_HEIGHT       480
static int   fb_width = UI_WIDTH, fb_height = UI_HEIGHT;
#define SQUARE_SIZE     80
#define SQUARE_W        10
#define SQUARE_IN       (SQUARE_SIZE - 2*SQUARE_W)
#define SQUARE_X        ((UI_WIDTH - SQUARE_SIZE)/2.0f)   //155
#define SQUARE_Y        ((UI_HEIGHT - SQUARE_SIZE)/2.0f)   //155
#define ALIGN(value,base)   ((value + base - 1) & ~(base-1))

static vg_lite_buffer_t * fb;

static int16_t squarePathData[] =
{
    VLC_OP_MOVE,        SQUARE_X,       SQUARE_Y,
    VLC_OP_LINE_REL,    SQUARE_SIZE,        0,
    VLC_OP_LINE_REL,    0,          SQUARE_SIZE,
    VLC_OP_LINE_REL,    -1*SQUARE_SIZE,    0,
    VLC_OP_LINE_REL,    0,          -1*SQUARE_SIZE,
    VLC_OP_MOVE,        SQUARE_X+SQUARE_W,  SQUARE_Y+SQUARE_W,
    VLC_OP_LINE_REL,    SQUARE_IN,      0,
    VLC_OP_LINE_REL,    0,          SQUARE_IN,
    VLC_OP_LINE_REL,    -1*SQUARE_IN,       0,
    VLC_OP_LINE_REL,    0,          -1*SQUARE_IN,
    VLC_OP_CLOSE,
    0,
};
static vg_lite_path_t squarePath = 
{
    {
        SQUARE_X, SQUARE_Y, // left,top
        SQUARE_X + SQUARE_SIZE, SQUARE_Y + SQUARE_SIZE // right,bottom
    },
    VG_LITE_HIGH, // quality
    VG_LITE_S16, // 16-bit coordinates
    {0}, // uploaded
    sizeof(squarePathData), // path length
    squarePathData, // path data
    1
};

void cleanup(void)
{
    if (has_fb) {
        // Close the framebuffer.
        vg_lite_fb_close(sys_fb);
    }

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}
vg_lite_error_t multi_draw(int tsbuffer_size,int i)
{
    char name[20];
    vg_lite_filter_t filter;
    uint32_t gradColors[] = {0xAAFF0000, 0xAA00FF00}; 
    uint32_t gradStops[] = {0, SQUARE_SIZE};

    vg_lite_linear_gradient_t gradient;

    vg_lite_matrix_t matrix;
    vg_lite_matrix_t primaryMatrix;
    vg_lite_matrix_t *gradMatrix;

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    sprintf(name,"multi_draw_%d.png",i);
    /* Allocate the off-screen buffer. */
    buffer.width  = ALIGN(fb_width,64);
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    printf("Render size: %d x %d\n", fb_width, fb_height);
    memset(&gradient, 0, sizeof(gradient));
    CHECK_ERROR(vg_lite_init_grad(&gradient));
    CHECK_ERROR(vg_lite_set_grad(&gradient, 2, gradColors, gradStops));
    CHECK_ERROR(vg_lite_update_grad(&gradient));
    /* Set up primary matrix to center in 480x800 space */
    vg_lite_identity(&primaryMatrix);
    vg_lite_identity(&matrix);
    // Set up grad matrix to translate to the edge of the square path
    gradMatrix = vg_lite_get_grad_matrix(&gradient);
    *gradMatrix = primaryMatrix;
    vg_lite_translate( SQUARE_X, 0, gradMatrix );

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xffFFFF00));
    //// Fill the path using an image.
    CHECK_ERROR(vg_lite_draw_gradient(fb, &squarePath, VG_LITE_FILL_EVEN_ODD, &primaryMatrix, &gradient, VG_LITE_BLEND_SRC_OVER));
    vg_lite_translate( 100.0f, 0.0f, &primaryMatrix );
    CHECK_ERROR(vg_lite_draw(fb, &squarePath, VG_LITE_FILL_EVEN_ODD, &primaryMatrix, VG_LITE_BLEND_SRC_OVER, 0xAA0000AA));
    CHECK_ERROR(vg_lite_finish());
    CHECK_ERROR(vg_lite_clear_grad(&gradient));
    vg_lite_save_png(name, fb);

    return VG_LITE_SUCCESS;
ErrorHandler:
    vg_lite_clear_grad(&gradient);
    cleanup();
    return error;
}
int main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int size[] = {16,32,64,128,256,512};
    int i;
    for(i = 0;i < 6; i++) {
        CHECK_ERROR(multi_draw(size[i],i));
    }
ErrorHandler:
    cleanup();
    return 0;
}
