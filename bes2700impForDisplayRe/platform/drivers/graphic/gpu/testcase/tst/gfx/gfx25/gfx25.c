#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

static vg_lite_buffer_t buffer, tiled_buffer;
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

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }
    if (tiled_buffer.handle != NULL) {
        vg_lite_free(&tiled_buffer);
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    filter = VG_LITE_FILTER_POINT;

    /* Allocate the buffer. */
    if (argc == 3) {
        buffer.width = atoi(argv[1]);
        buffer.height = atoi(argv[2]);
    } else {
        buffer.width  = 640;
        buffer.height = 480;
    }
    /* Initialize the draw. */
    error = vg_lite_init(buffer.width, buffer.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    /* Setup the tiled buffer. */
    tiled_buffer.format = VG_LITE_BGRA8888;
    if (argc == 3) {
        tiled_buffer.width = atoi(argv[1]);
        tiled_buffer.height = atoi(argv[2]);
    } else {
        tiled_buffer.width  = 640;
        tiled_buffer.height = 480;
    }
    tiled_buffer.tiled  = 1;
    error = vg_lite_allocate(&tiled_buffer);
    if (error) {
        printf("temp buffer allocation failed: %d\n", error);
        cleanup();
        return -1;
    }

    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, 0xFFFF0000);
    vg_lite_clear(&tiled_buffer, NULL, 0xFFFF0000);

    /* Setup a 10x10 scale at center of buffer. */
    vg_lite_identity(&matrix);
    vg_lite_translate(tiled_buffer.width / 2, tiled_buffer.height / 2, &matrix);
    vg_lite_scale(10, 10, &matrix);
    vg_lite_scale((vg_lite_float_t) tiled_buffer.width / 640,
                  (vg_lite_float_t) tiled_buffer.height / 480, &matrix);

    /* Draw the path to the tiled buffer using the matrix. */
    error = vg_lite_draw(&tiled_buffer, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF);
    if (error) {
        printf("vg_lite_draw() returned error %d\n", error);
        cleanup();
        return -1;
    }

    /* Save tiled_buffer. */
    vg_lite_save_png("tiled_temp.png", &tiled_buffer);
    /* Render the tiled buffer to target. */
    vg_lite_identity(&matrix);
    error = vg_lite_blit(fb, &tiled_buffer, &matrix, VG_LITE_BLEND_NONE, 0, filter);
    if (error) {
        printf("vg_lite_blit() returned error %d\n", error);
        cleanup();
        return -1;
    }
    vg_lite_finish();
    /* Save the result. */
    vg_lite_save_png("gfx25.png", fb);

    /* Cleanup. */
    cleanup();
    return 0;
}
