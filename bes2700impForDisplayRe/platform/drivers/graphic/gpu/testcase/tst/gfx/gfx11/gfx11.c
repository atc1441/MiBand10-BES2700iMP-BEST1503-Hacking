/*
    Double image sample.
    This case draws a path to a buffer, then combine the path buffer with the image together into the framebuffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define ICON_COUNT 6
#define FB_WIDTH    640
#define FB_HEIGHT   480

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t path_buffer, image_buffer;
static vg_lite_matrix_t pb_matrix, ib_matrix;
static int img_size = 128;
static uint32_t clear_color = 0XFFFF0000;

void loadImages()
{
    if (!vg_lite_load_png(&image_buffer, "landscape1.png"))
    {
        printf("Can't load image file %s\n", "double_image_src1.png");
    }
    
    if (!vg_lite_load_png(&path_buffer, "drop.png"))
    {
        printf("Can't load image file %s\n", "double_image_src0.png");
    }

}

void cleanup(void)
{
    if (image_buffer.handle != NULL)
    {
        vg_lite_free(&image_buffer);
    }
    if (path_buffer.handle != NULL)
    {
        vg_lite_free(&path_buffer);
    }
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    filter = VG_LITE_FILTER_POINT;

    /* Allocate the buffer. */
    if (argc == 3) {
            buffer.width = atoi(argv[1]);
            buffer.height = atoi(argv[2]);
        } else {
            buffer.width  = FB_WIDTH;
            buffer.height = FB_HEIGHT;
    }
    buffer.format = VG_LITE_BGRA8888;
    /* Initialize the draw. */
    error = vg_lite_init(buffer.width, buffer.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;  
    loadImages();
    
    /* Clear the buffer with blue. */
    vg_lite_clear(fb, NULL, clear_color);

    /* Combine image buffer & bg_buffer. */
    vg_lite_identity(&ib_matrix);
    vg_lite_identity(&pb_matrix);
    vg_lite_scale((vg_lite_float_t) fb->width / FB_WIDTH,
                  (vg_lite_float_t) fb->height / FB_HEIGHT, &pb_matrix);
    vg_lite_translate((FB_WIDTH - img_size) / 2, (FB_HEIGHT - img_size) / 2, &pb_matrix);

    /* Combine the path buffer & image_buffer into framebuffer. */
    vg_lite_blit2(fb, &path_buffer, &image_buffer, &pb_matrix, &ib_matrix, VG_LITE_BLEND_ADDITIVE, filter);
    vg_lite_finish();
    /* Save PNG file. */
    vg_lite_save_png("gfx11.png", fb);

    /* Cleanup. */
    cleanup();
    return 0;
}
