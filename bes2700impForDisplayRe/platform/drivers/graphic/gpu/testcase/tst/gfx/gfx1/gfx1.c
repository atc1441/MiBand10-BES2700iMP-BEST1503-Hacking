#include <stdio.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
    }
}

int main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    /* Try to open the framebuffer. */
    vg_lite_init(640, 480);
    /* Allocate a buffer. */
    buffer.width = 640;
    buffer.height = 480;
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    /* Clear the buffer with blue. */
    error = vg_lite_clear(fb, NULL, 0xFFFF0000);
    if (error) {
        printf("vg_lite_clear() returned error %d\n", error);
        cleanup();
        return -1;
    }
    vg_lite_finish();
    /* Save PNG file. */
    vg_lite_save_png("gfx1.png", fb);

    /* Cleanup. */
    cleanup();
    return 0;
}
