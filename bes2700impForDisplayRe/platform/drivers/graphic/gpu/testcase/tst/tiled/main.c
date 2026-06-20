#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

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
static int   fb_width = 256, fb_height = 256;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

static vg_lite_buffer_t tiled_buffer;
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

    if (tiled_buffer.handle != NULL) {
        vg_lite_free(&tiled_buffer);
    }
    
    vg_lite_clear_path(&path);
    vg_lite_close();
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;

    /* Initialize vg_lite engine. */
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

    /* Setup the tiled buffer. */
    memset(&tiled_buffer, 0, sizeof(tiled_buffer));
    tiled_buffer.format = VG_LITE_RGBA8888;
    tiled_buffer.width  = buffer.width;
    tiled_buffer.height = buffer.height;
    tiled_buffer.tiled  = 1;
    CHECK_ERROR(vg_lite_allocate(&tiled_buffer));
    
    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(&buffer, NULL, 0xFFFF0000));
    CHECK_ERROR(vg_lite_clear(&tiled_buffer, NULL, 0xFFFF0000));
    
    // *** DRAW ***
    // Setup a 10x10 scale at center of buffer.
    vg_lite_identity(&matrix);
    vg_lite_translate(tiled_buffer.width / 2, tiled_buffer.height / 2, &matrix);
    vg_lite_scale(10, 10, &matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);
    
    // Draw the path to the tiled buffer using the matrix.
    CHECK_ERROR(vg_lite_draw(&tiled_buffer, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
    CHECK_ERROR(vg_lite_finish());

    // Save tiled_buffer.
    vg_lite_save_png("tiled_temp.png", &tiled_buffer);
    
    // Render the tiled buffer to target.
    vg_lite_identity(&matrix);
    vg_lite_translate((fb_width - tiled_buffer.width) / 2, (fb_height - tiled_buffer.height) / 2, &matrix);
    CHECK_ERROR(vg_lite_blit(fb, &tiled_buffer, &matrix, VG_LITE_BLEND_NONE, 0, filter));
    CHECK_ERROR(vg_lite_finish());
    // Save the result.
    vg_lite_save_png("tiled.png", &buffer);

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
