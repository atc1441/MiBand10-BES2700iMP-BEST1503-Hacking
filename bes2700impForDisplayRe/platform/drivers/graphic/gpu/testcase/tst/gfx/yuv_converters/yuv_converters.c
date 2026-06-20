
#include <stdio.h>
#include "vg_lite.h"

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DDRLess 0
// Size of fb that render into.
static int   render_width = 1920, render_height = 1080;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the offscreen framebuffer memory.
        vg_lite_free(&buffer);
    }   
    vg_lite_close();
    
}

int main(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    error = vg_lite_init(render_width, render_height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    // Allocate the off-screen buffer.
    buffer.width  = render_width;
    buffer.height = render_height;
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    vg_lite_clear(&buffer, NULL, 0xFFFF0000);
    fb = &buffer;     
    printf("Render size: %d x %d\n", buffer.width, buffer.height);   
    vg_lite_finish();
    // Save PNG file.
    vg_lite_save_png("yuv_converters_dl.png", fb);
    // Cleanup.
    cleanup();
    return 0;
}
