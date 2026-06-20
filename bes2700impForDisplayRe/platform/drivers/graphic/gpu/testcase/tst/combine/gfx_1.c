#include "SFT.h"

static vg_lite_buffer_t * raw = NULL;
static vg_lite_buffer_t clear_buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * clear_fb;

static void clear_cleanup(void)
{
    if (clear_buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&clear_buffer);
    }
    
    if (raw != NULL && raw->handle != NULL) {
        // Free the raw memory.
        vg_lite_free(raw);
    }
    vg_lite_close();
}

// Initializations.
int clear_init(int user_fb_width, int user_fb_height)
{
    vg_lite_error_t error;
    
    // Initialize the blitter.
    error = vg_lite_init(user_fb_width, user_fb_height);
    if (error != VG_LITE_SUCCESS)
    {
        VGLITE_TST_PRINTF("vg_lite engine initialization failed @ %s, ln%d.\r\n", __FILE__, __LINE__);
        return TB_BAD;
    }
    else
    {
        VGLITE_TST_PRINTF("vg_lite engine initialization OK.\r\n");
    }
    
    // Allocate the buffer.
    clear_buffer.width  = user_fb_width;
    clear_buffer.height = user_fb_height;
    clear_buffer.format = VG_LITE_RGBA8888;
    clear_buffer.tiled  = 0;
    error = vg_lite_allocate(&clear_buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        clear_cleanup();
        return TB_BAD;
    }
    clear_fb = &clear_buffer;
    
    if (clear_fb != NULL) {
        return TB_OK;
    } else {
        printf("clear_fb initialization failed.\n");
        return TB_BAD;
    }
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear
 Description: Clear whole buffer with blue first, then clear a sub-rectangle of the buffer with red.
 */
int exe_clear(int user_fb_width, int user_fb_height, vg_lite_buffer_t * raw_fb)
{
    vg_lite_rectangle_t rect = { 64, 64, 64, 64 };
    
    uint32_t feature_check = 0;
    raw = raw_fb;
    if (clear_init(user_fb_width, user_fb_height) != TB_OK) {
        return TB_BAD;
    }
    
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 1) {
        printf("gfx_1 is not supported.\n");
        return TB_BAD;
    }
    
    // Clear the buffer with blue.
    vg_lite_clear(clear_fb, NULL, 0xFFFF0000);
    
    // Clear a sub-rectangle of the buffer with red.
    vg_lite_clear(clear_fb, &rect, 0xFF0000FF);
    
    vg_lite_finish();

#if !defined (VGLITE_TST_FIRMWARE)
    // Save PNG file.
    vg_lite_save_png("gfx_1.png", clear_fb);
#endif
 
    // Cleanup.
    clear_cleanup();
    
    return TB_OK;
}
