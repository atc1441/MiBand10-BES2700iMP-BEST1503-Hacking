#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "gpu_common.h"

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
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

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

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);

    vg_lite_close();
}

int test_vector_upload_main_entry(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;

    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    // *** DRAW ***
    // Try uploading the path data.
    CHECK_ERROR(vg_lite_upload_path(&path));

    // Setup a 2x2 scale at top-left of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 4, fb_height / 4, &matrix);
    vg_lite_scale(2, 2, &matrix);
    vg_lite_rotate(45, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    // Setup a 2x2 scale at top-right of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(3 * fb_width / 4, fb_height / 4, &matrix);
    vg_lite_scale(2, 2, &matrix);
    vg_lite_rotate(135, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
    
    // Setup a 2x2 scale at center of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix);
    vg_lite_scale(5, 5, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    // Setup a 2x2 scale at bottom-left of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(fb_width / 4, 3 * fb_height / 4, &matrix);
    vg_lite_scale(2, 2, &matrix);
    vg_lite_rotate(-45, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    // Setup a 2x2 scale at bottom-left of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(3 * fb_width / 4, 3 * fb_height / 4, &matrix);
    vg_lite_scale(2, 2, &matrix);
    vg_lite_rotate(-135, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
    CHECK_ERROR(vg_lite_finish());

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("vector_upload.png", fb);

    gpu_set_soft_break_point();

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
