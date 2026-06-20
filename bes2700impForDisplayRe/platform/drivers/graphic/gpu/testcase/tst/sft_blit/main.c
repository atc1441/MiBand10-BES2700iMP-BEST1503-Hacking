#include "SFT.h"

int fb_width = 320, fb_height = 480;

vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
vg_lite_buffer_t * fb;
vg_lite_filter_t filter;

vg_lite_buffer_t raw;

// Test function.
extern void SFT_Blit();

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }
    
    if (raw.handle != NULL) {
        // Free the raw memory.
        vg_lite_free(&raw);
    }
    
    vg_lite_close();
}

// Initializations.
int init()
{
    uint32_t feature_check = 0;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    
    // Initialize the blitter.
    error = vg_lite_init(0, 0);
    if (error != VG_LITE_SUCCESS)
    {
        printf("vg_lite engine initialization failed.\n");
        return -1;
    }
    
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 1) {
        printf("sft_blit is not supported.\n");
        return -1;
    }

    filter = VG_LITE_FILTER_POINT;
    
    // Allocate the buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;
    buffer.tiled  = 0;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
        fb= &buffer;

    return 0;
}

int render()
{
    int ret2 = 0;
    
    InitBMP();
    SFT_Blit();
    DestroyBMP();
    
    return ret2;
}

int main(int argc, const char * argv[])
{
    int ret;

    /* Initialize. */
    ret = init();
    if (ret != 0)
    {
        cleanup();
        return -1;
    }

    /* Draw. */
    ret = render();
    if (ret != 0)
    {
        cleanup();
        return -1;
    }

    /* Cleanup. */
    cleanup();
    return 0;
}
