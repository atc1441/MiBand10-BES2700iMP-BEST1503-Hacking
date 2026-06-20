#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"


#define GPU_REP(...)   //TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)


#define __func__ __FUNCTION__
static char *error_type[] =
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

#define DEFAULT_SIZE   454.0f

static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;


static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t raw;

#define ICON_COUNT 6
static vg_lite_buffer_t icons[ICON_COUNT];
static vg_lite_matrix_t icon_matrix, highlight_matrix;
static int icon_pos[6][2];

#if 0
static int icon_size = 128;
#else
static int icon_size = 32;

#endif
/*
 A rectangle path with original size 10x10 @ (0, 0)
 */
static char path_data[] = {
    2,  0,  0,          // moveto    0,  0
    4, 10,  0,          // lineto   10,  0
    4, 10, 10,          // lineto   10, 10
    4,  0, 10,          // lineto    0, 10
    0,                  // end
};

static vg_lite_path_t path = {
    {-10, -10, 10, 10}, // bounding box left, top, right, bottom
    VG_LITE_HIGH,       // quality
    VG_LITE_S8,         // -128 to 127 coordinate range
    {0},                // uploaded
    sizeof(path_data),  // path length
    path_data,          // path data
    1                   // path changed
};





static int vg_lite_load_raw_(vg_lite_buffer_t * iconb, int index)
{

    // Set status.
    int status = 0;
    uint32_t * fbr;
    int x, y;
    unsigned char red = 0xff;
    unsigned char green = 0;
    unsigned char blue  = 0;
    // Get width, height, stride and format info.
    iconb->width  = 32;
    iconb->height = 32;
    iconb->format = VG_LITE_RGBA8888;
    iconb->stride = 128;
    // Allocate the VGLite buffer memory.
    if (vg_lite_allocate(iconb) != VG_LITE_SUCCESS)
    {
            return -1;
    }
    for (y = 0; y< iconb->height; y++) {
         fbr = (uint32_t *)((uint32_t)iconb->memory + iconb->width*4*y);
        for (x = 0; x < iconb->width; x++) {
            *fbr++ =  0x2f<<24|blue<<16| green << 8 | red;
         }
     }

    // Return the status.
    return status;
}




void loadImages()
{
    int i;
    static const char *img_file_names[ICON_COUNT] =
    {
    #if 0
        "icons/1.raw",
        "icons/2.raw",
        "icons/3.raw",
        "icons/4.raw",
        "icons/5.raw",
        "icons/6.raw"
    #else   // test for less memory
        "icons/1_4.raw",
        "icons/2_4.raw",
        "icons/3_4.raw",
        "icons/4_4.raw",
        "icons/5_4.raw",
        "icons/6_4.raw"
    #endif
    };

    for (i = 0; i < ICON_COUNT; i++)
    {
#if 0
        if (vg_lite_load_raw(&icons[i], img_file_names[i]) != 0)
#else   // use dummy file
        if (vg_lite_load_raw_(&icons[i], i) != 0)

#endif
        {
            printf("Can't load image file %s\n", img_file_names[i]);
        }
    }
}

static void cleanup(void)
{
    int i;
    for (i = 0; i < ICON_COUNT; i++)
    {
        if (icons[i].handle != NULL)
        {
            vg_lite_free(&icons[i]);
        }
    }

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);
    vg_lite_close();
}

int ui_test_main_entry(int argc, const char * argv[])
{
    int i, j, icon_id;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int gap_x, gap_y;
    TC_INIT
    GPU_REP("%s", __func__);

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    //printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;

    buffer.format = GPU_FB_FORMAT;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    loadImages();

    // Clear the buffer with blue.

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));


     // *** DRAW ***
    /* Draw the 6 icons (3 x 2) */
    gap_x = (fb_width - icon_size * 3) / 4;   //16
    gap_y = (fb_height - icon_size * 2) / 3;  //32
    icon_id = 0;
    for (i = 0; i < 2; i++)
    {
        for (j = 0; j < 3; j++)
        {
            icon_pos[i * 3 + j][0] = gap_x * (j + 1) + j * icon_size;
            icon_pos[i * 3 + j][1] = gap_y * (i + 1) + i * icon_size;
            #if 1
            /* Setup the matrix. */
            vg_lite_identity(&icon_matrix);
            vg_lite_translate(icon_pos[icon_id][0], icon_pos[icon_id][1], &icon_matrix);
            vg_lite_scale((float)icon_size / icons[icon_id].width, (float)icon_size / icons[icon_id].height, &icon_matrix);
            vg_lite_scale(fb_scale, fb_scale, &icon_matrix);
            CHECK_ERROR(vg_lite_blit(fb, &icons[icon_id], &icon_matrix, VG_LITE_BLEND_SRC_OVER, 0, filter));
            #endif
            icon_id++;
        }
    }

    /* Draw the highlighted rectangle. */
#if 1
    // Setup a 10x10 scale at center of buffer.
    vg_lite_identity(&highlight_matrix);
    vg_lite_translate(icon_pos[2][0], icon_pos[2][1], &highlight_matrix);     //112,58
    vg_lite_scale(icon_size / 10.0f, icon_size / 10.0f, &highlight_matrix);
    vg_lite_scale(fb_scale, fb_scale, &highlight_matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &highlight_matrix, VG_LITE_BLEND_SRC_OVER, 0x22444488));
#endif

    gpu_save_cmdbuffer("ui.cb");

    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    TC_REP(ui_test_gcost);

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("ui.raw", fb);

    gpu_set_soft_break_point();

ErrorHandler:
    // Cleanup.
    cleanup();
    GPU_REP("%s test pass", __func__);
    return 0;
}
