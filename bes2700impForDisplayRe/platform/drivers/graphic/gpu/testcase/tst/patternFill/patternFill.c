
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#if DDRLESS_FPGA
#include "grad.h"
#endif

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
int frames = 1; //Frames to render

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;

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
    2, -5, -10, // moveto   -5,-10
    4, 5, -10,  // lineto    5,-10
    4, 10, -5,  // lineto   10, -5
    4, 0, 0,    // lineto    0,  0
    4, 10, 5,   // lineto   10,  5
    4, 5, 10,   // lineto    5, 10
    4, -5, 10,  // lineto   -5, 10
    4, -10, 5,  // lineto  -10,  5
    4, -10, -5, // lineto  -10, -5
    0, // end
};

static vg_lite_path_t path = {
    {-10, -10, // left,top
        10, 10}, // right,bottom
    VG_LITE_HIGH, // quality
    VG_LITE_S8, // -128 to 127 coordinate range
    {0}, // uploaded
    sizeof(path_data), // path length
    path_data, // path data
    1
};

void cleanup(void)
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

int main(int argc, const char * argv[])
{
    int i = 0;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix, matPath;

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    /* Load the image. */
    if (0 != vg_lite_load_raw(&image, "landscape.raw")) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);
   
    while (i < frames)
    {
        // Clear the buffer with blue.
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        // Build a 33 degree rotation matrix from the center of the buffer.
        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -2.0f, fb_height / -4.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matPath);
        vg_lite_scale(10, 10, &matPath);

        //vg_lite_blit(&image, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        // Fill the path using an image.
        CHECK_ERROR(vg_lite_draw_pattern(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter));

        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 1.2f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -1.5f, fb_height / -2.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 1.3f, &matPath);
        vg_lite_scale(10, 10, &matPath);
        CHECK_ERROR(vg_lite_draw_pattern(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_PAD, 0xffaabbcc, filter));
        CHECK_ERROR(vg_lite_finish());
        printf("frame:%d Done!\n",i);
        i++;
    }
    // Save PNG file.
    vg_lite_save_png("patternFill.png", fb);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
