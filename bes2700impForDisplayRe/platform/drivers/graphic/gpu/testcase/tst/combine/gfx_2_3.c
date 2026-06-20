#include "SFT.h"

#define ICON_COUNT 6
static vg_lite_filter_t ui_filter;
static vg_lite_buffer_t * raw = NULL;
static vg_lite_buffer_t ui_buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * ui_fb;
static vg_lite_buffer_t icons[ICON_COUNT];
static vg_lite_matrix_t icon_matrix, highlight_matrix;
static int icon_pos[6][2];
static int icon_size = 128;

/*
 A rectangle path with original size 10x10 @ (0, 0)
 */
static char path_data[] = {
    2,  0,  0, // moveto   -5,-10
    4, 10,  0,  // lineto    5,-10
    4, 10, 10,  // lineto   10, -5
    4, 0,  10,    // lineto    0,  0
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

static void ui_cleanup(void)
{
    int i;
    for (i = 0; i < ICON_COUNT; i++)
    {
        if (icons[i].handle != NULL)
        {
            vg_lite_free(&icons[i]);
        }
    }
    if (ui_buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&ui_buffer);
    }
    if (raw != NULL && raw->handle != NULL) {
        // Free the raw memory.
        vg_lite_free(raw);
    }
    vg_lite_close();
}

// Initializations.
int ui_init(int user_fb_width, int user_fb_height)
{
    uint32_t feature_check = 0;
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
    
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 1) {
        printf("gfx_2_3 is not supported.\n");
        return TB_BAD;
    }
    
    // Set image filter type according to hardware feature.
    ui_filter = VG_LITE_FILTER_POINT;
    
    // Allocate the buffer.
    ui_buffer.width  = user_fb_width;
    ui_buffer.height = user_fb_height;
    ui_buffer.format = VG_LITE_RGBA8888;
    ui_buffer.tiled  = 0;
    error = vg_lite_allocate(&ui_buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        ui_cleanup();
        return TB_BAD;
    }
    ui_fb = &ui_buffer;
    
    if (ui_fb != NULL) {
        return TB_OK;
    } else {
        printf("ui_fb initialization failed.\n");
        return TB_BAD;
    }
}

static void loadImages()
{
    int i;
    static const char *img_file_names[ICON_COUNT] =
    {
        "icons/1.raw",
        "icons/2.raw",
        "icons/3.raw",
        "icons/4.raw",
        "icons/5.raw",
        "icons/6.raw"
    };
    
    for (i = 0; i < ICON_COUNT; i++)
    {
        if (vg_lite_load_raw(&icons[i], img_file_names[i]) != 0)
        {
            VGLITE_TST_PRINTF("Can't load image file %s\r\n", img_file_names[i]);
        }
    }
}

/*
 Resolution: 320 x 480
 Format: VG_LITE_RGBA8888
 Transformation: Rotate/Scale/Translate/Perspective
 Alpha Blending: VG_LITE_BLEND_SRC_OVER
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_rotate/vg_lite_perspective/vg_lite_blit
 Description: Blit 6 images into blue buffer and sort them into two rows, then draw the highlighted rectangle into buffer.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
int exe_ui(int user_fb_width, int user_fb_height, vg_lite_buffer_t * raw_fb)
{
    int i, j, icon_id;
    vg_lite_float_t w0, w1;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int gap_x, gap_y;
    raw = raw_fb;
    
    if (ui_init(user_fb_width, user_fb_height) != TB_OK) {
        return TB_BAD;
    }
    
    memset(icons, 0, sizeof(icons));
    memset(&icon_matrix, 0, sizeof(icon_matrix));
    memset(&highlight_matrix, 0, sizeof(highlight_matrix));
    memset(icon_pos, 0, sizeof(icon_pos));

    loadImages();
    
    // Clear the buffer with blue.
    vg_lite_clear(ui_fb, NULL, 0xFFFF0000);
    
    // *** DRAW ***
    /* Draw the 6 icons (3 x 2) */
    gap_x = (ui_fb->width - icon_size * 3) / 4;
    gap_y = (ui_fb->height - icon_size * 2) / 3;
    icon_id = 0;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 3; j++)
        {
            icon_pos[i * 3 + j][0] = gap_x * (j + 1) + j * icon_size;
            icon_pos[i * 3 + j][1] = gap_y * (i + 1) + i * icon_size;
            w0 = 0.0003f * (i + 1) * (j + 1);
            w1 = 0.0003f * (i + 1) * (j + 1);
            
            /* Setup the matrix. */
            vg_lite_identity(&icon_matrix);
            vg_lite_translate(icon_pos[icon_id][0], icon_pos[icon_id][1], &icon_matrix);
            vg_lite_scale((float)icon_size / icons[icon_id].width, (float)icon_size / icons[icon_id].height, &icon_matrix);
            vg_lite_rotate(-10.0f * (i + 1) * (j + 1), &icon_matrix);
            vg_lite_perspective(w0, w1, &icon_matrix);
            error = vg_lite_blit(ui_fb, &icons[icon_id], &icon_matrix, VG_LITE_BLEND_SRC_OVER, 0, ui_filter);
            if (error) {
                VGLITE_TST_PRINTF("vg_lite_blit() returned error %d\r\n", error);
                ui_cleanup();
                return TB_BAD_API_FAIL;
            }
            
            icon_id++;
        }
    }
    
    /* Draw the highlighted rectangle. */
    
    // Setup a 10x10 scale at center of buffer.
    vg_lite_identity(&highlight_matrix);
    vg_lite_translate(icon_pos[2][0], icon_pos[2][1], &highlight_matrix);
    vg_lite_scale(icon_size / 10.0f, icon_size / 10.0f, &highlight_matrix);
    
    // Draw the path using the matrix.
    error = vg_lite_draw(ui_fb, &path, VG_LITE_FILL_EVEN_ODD, &highlight_matrix, VG_LITE_BLEND_SRC_OVER, 0x22444488);
    if (error) {
        VGLITE_TST_PRINTF("vg_lite_draw() returned error %d\r\n", error);
        ui_cleanup();
        return TB_BAD_API_FAIL;
    }
    
    vg_lite_finish();
    
#if !defined (VGLITE_TST_FIRMWARE)
    // Save PNG file.
    vg_lite_save_png("gfx_2_3.png", ui_fb);
#endif

    // Cleanup.
    ui_cleanup();

    return TB_OK;
}
