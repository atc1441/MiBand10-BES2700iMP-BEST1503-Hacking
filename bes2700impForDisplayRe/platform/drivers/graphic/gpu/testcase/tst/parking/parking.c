#include <memory.h>
#if !_WIN32
#include <sys/time.h>
#endif

#include "parking.h"

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "Resource_6/6_1/alert1_index1.img.h"
#include "Resource_6/6_1/close1_index1.img.h"
#include "Resource_6/6_3/car_index8.img.h"
#include "Resource_6/6_4/0_25_index1.img.h"
#include "Resource_6/6_4/0_5_index1.img.h"
#include "Resource_6/6_4/1_5_index1.img.h"
#include "Resource_6/6_4/3_0_index1.img.h"
#include "Resource_6/6_4/alert3_index1.img.h"
#include "Resource_6/6_4/stop3_index1.img.h"
#include "Resource_6/6_5/alert4_index1.img.h"
#include "Resource_6/6_5/button_upper_index8.img.h"
#include "Resource_6/6_5/button_left_index8.img.h"
#include "Resource_6/6_5/button_right_index8.img.h"
#include "Resource_6/6_6/alert5_index1.img.h"
#include "Resource_6/6_6/icon_a_index1.img.h"
#include "Resource_6/6_6/icon_b_index1.img.h"
#include "Resource_6/6_6/icon_c_index1.img.h"

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
// Size of fb that render into.
static int   render_width = BASE_DIM_X, render_height = BASE_DIM_Y;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t buffer;
static int has_blitter = 0;
static vg_lite_buffer_t * sys_fb;
static int has_fb = 0;
int show = 3;   //Frame to show.
int frames = 1; //Frames to render

// Path and colors for frame 0 (garage).
vg_lite_path_t path_wall_lighter;
vg_lite_path_t path_wall_darker;
vg_lite_path_t path_base_lighter;
vg_lite_path_t path_base_darker;
vg_lite_path_t path_garage_cross;
vg_lite_path_t path_garage_bottom;
uint32_t color_wall_lighter = 0x8833aa33;
uint32_t color_wall_darker = 0x88115511;
uint32_t color_base_lighter = 0xffaa5555;
uint32_t color_base_darker = 0xff552222;
uint32_t color_garage_cross = 0xffaa8800;
uint32_t color_garage_bottom = 0xff222255;

// Path and colors for frame 1 (confirm).
vg_lite_path_t path_confirm_notify;
vg_lite_path_t path_confirm_frameline;
vg_lite_path_t path_confirm_lineupper;
vg_lite_path_t path_confirm_linelower;
uint32_t color_confirm_notify = 0xFF222222;;
uint32_t color_confirm_frame = 0xFF2222FF;
uint32_t color_confirm_upper = 0xFF22AABB;
uint32_t color_confirm_lower = 0xFF22CC33;

// Path and colors for frame 2 (steering).
vg_lite_path_t path_steer_cross;
vg_lite_path_t path_steer_center;
vg_lite_path_t path_steer_top;
uint32_t color_steer_cross = 0xFFbb7777;
uint32_t color_steer_center = 0xFFBB66BB;
uint32_t color_steer_top = 0xFF6666AA;
uint32_t color_semi_black = 0xffaaaaaa;

// Path and colors for frame 3 (Display1).
vg_lite_path_t path_display1_mark;
vg_lite_path_t path_display1_tick;
vg_lite_buffer_t buf_display1_alert;
vg_lite_buffer_t buf_display1_close;
uint32_t color_display1_top = 0xFF22FF22;
uint32_t color_display1_center = 0xFF22FFFF;
uint32_t color_display1_bottom = 0xFF2222FF;
uint32_t color_display1_side = 0xFFFFFFFF;
uint32_t display1_clut1[2];

// Path and colors for frame 4 (Display2).
vg_lite_path_t path_display2_shadow;
vg_lite_path_t path_display2_line;
vg_lite_path_t path_display2_mark;
uint32_t color_display2_shadow = 0xFF00DD00;
uint32_t color_display2_line = 0xFF2222DD;
uint32_t color_display2_mark = 0xFF229922;

// Path and colors for frame 5 (Display3).
vg_lite_path_t path_display3_side1_left;
vg_lite_path_t path_display3_side1_right;
vg_lite_path_t path_display3_side2_left;
vg_lite_path_t path_display3_side2_right;
vg_lite_path_t path_display3_side3_left;
vg_lite_path_t path_display3_side3_right;
vg_lite_rectangle_t rect_display3_line;
vg_lite_buffer_t buf_display3_alert;
vg_lite_buffer_t buf_display3_stop;
vg_lite_buffer_t buf_display3_025;
vg_lite_buffer_t buf_display3_05;
vg_lite_buffer_t buf_display3_15;
vg_lite_buffer_t buf_display3_30;
uint32_t color_display3_side1 = 0xFF22DD22;
uint32_t color_display3_side2 = 0xFF22DDDD;
uint32_t color_display3_side3 = 0xFF2222DD;
uint32_t color_display3_line1 = 0xFF22DD22;
uint32_t color_display3_line2 = 0xFF22DDDD;
uint32_t color_display3_line3 = 0xFF2222DD;
uint32_t display3_clut1[2];

// Path and colors for frame 6 (Display4).
vg_lite_path_t path_display4_side_left;
vg_lite_path_t path_display4_side_right;
vg_lite_rectangle_t rect_display4_line;
vg_lite_buffer_t buf_display4_alert;
vg_lite_buffer_t buf_display4_upper;
uint32_t color_display4_side = 0xFF22DDDD;
uint32_t color_display4_line1 = 0xFF22DD22;
uint32_t color_display4_line2 = 0xFF1177EE;
uint32_t display4_clut1[2], display4_clut8[256];

// Path and colors for frame 7 (Display5).
vg_lite_path_t path_display5_frame;
vg_lite_rectangle_t rect_display5_line;
vg_lite_buffer_t buf_display5_alert;
vg_lite_buffer_t buf_display5_a;
vg_lite_buffer_t buf_display5_b;
vg_lite_buffer_t buf_display5_c;
uint32_t color_display5_frame = 0xFF22DDDD;
uint32_t color_display5_line = 0xFF22DDDD;
uint32_t display5_clut1[2];

// Path and colors for frame 8 (Display6).
vg_lite_buffer_t buf_display6_a;
vg_lite_buffer_t buf_display6_b;
vg_lite_buffer_t buf_display6_c;
vg_lite_buffer_t buf_display6_right;
vg_lite_buffer_t buf_display6_025;
vg_lite_buffer_t buf_display6_05;
vg_lite_buffer_t buf_display6_15;
vg_lite_buffer_t buf_display6_30;
vg_lite_buffer_t buf_display6_close;
vg_lite_buffer_t buf_display6_stop;

// Path and colors for frame 9 (Display7).
vg_lite_buffer_t buf_display7_a;
vg_lite_buffer_t buf_display7_b;
vg_lite_buffer_t buf_display7_c;
vg_lite_buffer_t buf_display7_left;
vg_lite_buffer_t buf_display7_stop;

// Path and colors for frame 10 (Display8).
/**************** Data for case Tire_Track ************************************/
/*** Tire track size and gaps. */
float tire_w0, tire_h0, tire_gap0;
float tire_w1, tire_h1, tire_gap1;
#define TIRES_PER_TRACK     30  // How many tire track rectangles per track.
/* Tire track control curves: we take 1080p as base solution. */
#define TCL0X   1100
#define TCL0Y   820
#define TCR0X   1200
#define TCR0Y   820
#define TCL1X   1550
#define TCL1Y   810
#define TCR1X   1650
#define TCR1Y   810
#define TCL2X   500
#define TCL2Y   650
#define TCR2X   550
#define TCR2Y   650
#define TCL3X   100
#define TCL3Y   1100
#define TCR3X   150
#define TCR3Y   1130
#define TCL4X   330
#define TCL4Y   1100
#define TCR4X   380
#define TCR4Y   1130
float tire_curves_left[5][8] = {
    {
        TCL0X, TCL0Y,
        TCL0X + 50, TCL0Y - 180,
        TCL0X + 70, TCL0Y - 250,
        TCL0X + 20, TCL0Y - 300
    },
    {
        TCL1X, TCL1Y,
        TCL1X - 90, TCL1Y - 200,
        TCL1X - 160, TCL1Y - 270,
        TCL1X - 230, TCL1Y - 330
    },
    {
        TCL2X, TCL2Y,
        TCL2X - 0, TCL2Y - 100,
        TCL2X - 25, TCL2Y - 210,
        TCL2X - 65, TCL2Y - 280
    },
    {
        TCL3X, TCL3Y,
        TCL3X + 60, TCL3Y - 100,
        TCL3X + 90, TCL3Y - 220,
        TCL3X + 105, TCL3Y - 330
    },
    {
        TCL4X, TCL4Y,
        TCL4X + 60, TCL4Y - 110,
        TCL4X + 90, TCL4Y - 220,
        TCL4X + 105, TCL4Y - 330
    }
};
float tire_curves_right[5][8] = {
    {
        TCR0X, TCR0Y,
        TCR0X + 20, TCR0Y - 200,
        TCR0X + 20, TCR0Y - 260,
        TCR0X - 35, TCR0Y - 310
    },
    {
        TCR1X, TCR1Y,
        TCR1X - 110, TCR1Y - 200,
        TCR1X - 190, TCR1Y - 290,
        TCR1X - 290, TCR1Y - 340
    },
    {
        TCR2X, TCR2Y,
        TCR2X - 0, TCR2Y - 100,
        TCR2X - 30, TCR2Y - 220,
        TCR2X - 70, TCR2Y - 300
    },
    {
        TCR3X, TCR3Y,
        TCR3X + 45, TCR3Y - 115,
        TCR3X + 85, TCR3Y - 235,
        TCR3X + 100, TCR3Y - 360
    },
    {
        TCR4X, TCR4Y,
        TCR4X + 45, TCR4Y - 115,
        TCR4X + 85, TCR4Y - 235,
        TCR4X + 100, TCR4Y - 360
    }
};
/* Tire track Paths. */
#define PATHS_PER_TRACK  3      // How many paths for a track.
#define TIRE_TRACK_PER_PATH (TIRES_PER_TRACK / PATHS_PER_TRACK) // How many sub-paths per path
vg_lite_path_t tt_paths[5][PATHS_PER_TRACK];

/* Other paths. */
vg_lite_path_t tt_path_bottom_curve;    //The red bottom curve on the track
vg_lite_path_t tt_path_center_line;    //The center yellow line on the track
vg_lite_path_t tt_path_top_line;       //The top yellow line on the track
vg_lite_path_t tt_path_track_curves[5];    //The curves besides the tracks.
vg_lite_path_t tt_path_corner_curve[2];          //The left and right curves on the right screen
vg_lite_path_t tt_path_intrack_curve;     //The curve between the left screen tracks
/* Images. */
vg_lite_buffer_t tt_image_navi; //The bottom navigation bar on the right screen
vg_lite_buffer_t tt_image_alert;    //The "Check entire surroundings!" message
vg_lite_buffer_t tt_image_icon_radar;   //The radar icon on the top
vg_lite_buffer_t tt_image_icon_zoom;    //The zoom icon on the center top
vg_lite_buffer_t tt_image_icon_triangle;    //The right triangle  icon on the top bar
vg_lite_buffer_t tt_image_car;              //The car image on the left screen.
/* Matrix. */
vg_lite_matrix_t tt_mat_car;
vg_lite_matrix_t tt_mat_alert;
vg_lite_matrix_t tt_mat_navi;
vg_lite_matrix_t tt_mat_radar;
vg_lite_matrix_t tt_mat_zoom;
vg_lite_matrix_t tt_mat_triangle;

/* Prototypes. */
int SetupFrame_Display1();
int SetupFrame_Display2();
int SetupFrame_Display3();
int SetupFrame_Display4();
int SetupFrame_Display5();
int SetupFrame_Display8();
vg_lite_error_t RenderFrame_Display1(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display2(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display3(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display4(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display5(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display6(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display7(int32_t width, int32_t height);
vg_lite_error_t RenderFrame_Display8(int32_t width, int32_t height);
void DestroyFrame_Display1();
void DestroyFrame_Display2();
void DestroyFrame_Display3();
void DestroyFrame_Display4();
void DestroyFrame_Display5();
void DestroyFrame_Display6();
void DestroyFrame_Display7();
void DestroyFrame_Display8();

#define INIT_PATH(path, size) \
vg_lite_init_path(&(path), VG_LITE_S32, VG_LITE_HIGH, size, NULL, 0, 0, 1280, 720)

/* Automatically append path data and commands into a path object.
 Assuming floating only.
 */
#define CDALIGN(value) (((value) + (4) - 1) & ~((4) - 1))
#define DATA_SIZE 4
#define CDMIN(x, y) ((x) > (y) ? (y) : (x))
#define CDMAX(x, y) ((x) > (y) ? (x) : (y))

static int32_t get_data_count(uint8_t cmd)
{
    static int32_t count[] = {
        0,
        0,
        2,
        2,
        2,
        2,
        4,
        4,
        6,
        6
    };
    
    if ((cmd < VLC_OP_END) || (cmd > VLC_OP_CUBIC_REL)) {
        return -1;
    }
    else {
        return count[cmd];
    }
}

static int32_t calc_path_size(uint8_t *cmd, uint32_t count)
{
    int32_t size = 0;
    int32_t dCount = 0;
    uint32_t i = 0;
    
    for (i = 0; i < count; i++) {
        size++;     //OP CODE.
        
        dCount = get_data_count(cmd[i]);
        if (dCount > 0) {
            size = CDALIGN(size);
            size += dCount * DATA_SIZE;
        }
    }
    
    return size;
}

static void byte_copy(void *dst, void *src, int size)
{
    int i;
    if ((size & 3) != 0)
    {
        uint8_t *dst8 = (uint8_t *)dst;
        uint8_t *src8 = (uint8_t *)src;
        for (i = 0; i < size; i++)
        {
            dst8[i] = src8[i];
        };
    }
    else
    {
        uint32_t *dst32 = (uint32_t *)dst;
        uint32_t *src32 = (uint32_t *)src;
        size /= 4;
        for (i = 0; i < size; i++)
        {
            dst32[i] = src32[i];
        }
    }
}

long gettick()
{
#if _WIN32
    return 0;
#else
    long time;
    struct timeval t;
    gettimeofday(&t, NULL);
    
    time = t.tv_sec * 1000000 + t.tv_usec;
    
    return time;
#endif
}

int SetupFrame_Display1()
{
    uint8_t *data8;
    int32_t *data32;
    int32_t data_size;

    static const int32_t thickness = 8;
    static const int32_t cx = BASE_DIM_X / 2;
    static const int32_t cy = BASE_DIM_Y / 8;
    static const int32_t shw0 = BASE_DIM_X * 5 / 12;
    const int32_t shx0 = shw0 * 0.25f;
    static const int32_t shy0 = BASE_DIM_Y / 8;
    const int32_t cross_hw = shy0 / 2;
    const int32_t cross_hh = shy0 / 2;
    static const int32_t notify_w  = BASE_DIM_X - 100;
    static const int32_t notify_h  = BASE_DIM_Y / 3;
    static const int32_t notify_x  = 50;

    /* Path notify area. */
    data_size = 4 + 8 +         /* Move */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 + 8 +     /* Quad */
    1;              /* End */
    INIT_PATH(path_confirm_notify, data_size);
    path_confirm_notify.path = malloc(data_size);
    data8 = (uint8_t*) path_confirm_notify.path;
    data32= (int32_t*) data8;

    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = (BASE_DIM_X - notify_w) / 2;
    *data32++ = BASE_DIM_Y - notify_x;
    data8 = (uint8_t*)data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = notify_w;
    *data32++ = 0;
    data8 = (uint8_t*)data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -notify_h;
    data8 = (uint8_t*)data32;

    *data8 = OPCODE_QUADRATIC_REL;
    data32++;
    *data32++ = -notify_w / 2;
    *data32++ = notify_h;
    *data32++ = -notify_w;
    *data32++ = 0;
    data8 = (uint8_t*)data32;

    *data8 = OPCODE_END;

    /*Path mark.*/
    data_size = 4 + 8 +         /* Move */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Close, Move */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Close, Move */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    1;              /* End. */
    INIT_PATH(path_display1_mark, data_size);
    path_display1_mark.path = malloc(data_size);
    data8 = (uint8_t*) path_display1_mark.path;
    data32 = (int32_t*) data8;

    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = cx - thickness / 2 - shw0 + shx0;
    *data32++ = cy - thickness / 2;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = cross_hw;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -cross_hw * 0.98f;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = -cross_hh;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness * 2;
    *data32++ = cross_hh * 2 + thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = cx - thickness / 2 - cross_hw;
    *data32++ = cy - thickness / 2;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = cross_hw * 2;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -cross_hw * 2;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = cx + thickness / 2 + shw0 - shx0;
    *data32++ = cy - thickness / 2;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -cross_hw;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = cross_hw * 0.98f;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = -cross_hh;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness * 2;
    *data32++ = cross_hh * 2 + thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_END;

    /*Path tick.*/
    data_size = 4 + 8 +         /* Move */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    4 + 8 +         /* Line */
    1;              /* End. */
    INIT_PATH(path_display1_tick, data_size);
    path_display1_tick.path = malloc(data_size);
    data8 = (uint8_t*) path_display1_tick.path;
    data32 = (int32_t*) data8;

    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = cx - thickness / 2 - thickness / 2;
    *data32++ = cy - thickness / 2;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = cross_hh;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -cross_hh * 2 - thickness;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_END;

    buf_display1_alert.handle = (void *)0;
    buf_display1_alert.width = img_alert1_width;
    buf_display1_alert.height = img_alert1_height;
    buf_display1_alert.format = (vg_lite_buffer_format_t)img_alert1_format;
    vg_lite_allocate(&buf_display1_alert);
    byte_copy(buf_display1_alert.memory, img_alert1_data, img_alert1_stride * img_alert1_height);

    buf_display1_close.handle = buf_display1_alert.handle;
    buf_display1_close.width = img_close1_width;
    buf_display1_close.height = img_close1_height;
    buf_display1_close.format = (vg_lite_buffer_format_t)img_close1_format;
    vg_lite_allocate(&buf_display1_close);
    byte_copy(buf_display1_close.memory, img_close1_data, img_close1_stride * img_close1_height);

    return 0;
}

int SetupFrame_Display2()
{
    uint8_t *data8;
    int32_t *data32;
    int32_t data_size;
    static const int32_t thickness = 40;
    static const int32_t bottomx = BASE_DIM_X / 10;
    static const int32_t bottomy = BASE_DIM_Y * 7 / 8;
    
    //Path line.
    data_size = 4 + 8 +         // Move
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    1;              // End.
    INIT_PATH(path_display2_line, data_size);
    path_display2_line.path = malloc(data_size);
    data8 = (uint8_t*) path_display2_line.path;
    data32 = (int32_t*) data8;
    
    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = bottomx * 1.1f;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE;
    data32++;
    *data32++ = BASE_DIM_X - bottomx * 1.1f;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = 0;
    *data32++ = -thickness / 3;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE;
    data32++;
    *data32++ = bottomx * 1.1f;
    *data32++ = bottomy - thickness / 3;
    data8 = (uint8_t*) data32;
    *data8 = OPCODE_END;

    //Path mark.
    data_size = 4 + 8 +         // Move
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +            // Close, Move
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    1;              // End.
    INIT_PATH(path_display2_mark, data_size);
    path_display2_mark.path = malloc(data_size);
    data8 = (uint8_t*) path_display2_mark.path;
    data32 = (int32_t*) data8;
    
    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = bottomx;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = thickness;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = -BASE_DIM_Y * 5 / 16;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness / 10;
    *data32++ = -thickness / 2;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = -BASE_DIM_Y * 5 / 16;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness / 10;
    *data32++ = -thickness / 2;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness / 4;
    *data32++ = -BASE_DIM_Y * 5 / 64;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_CLOSE;
    
    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = BASE_DIM_X - bottomx;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = thickness;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = -BASE_DIM_Y * 5 / 16;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness / 10;
    *data32++ = -thickness / 2;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = -BASE_DIM_Y * 5 / 16;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness / 10;
    *data32++ = -thickness / 2;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness / 4;
    *data32++ = -BASE_DIM_Y * 5 / 64;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_END;
    
    //Path shadow.
    data_size = 4 + 8 +         // Move
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +            // Close, Move
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    4 + 8 +         // Line
    1;              // End.
    INIT_PATH(path_display2_shadow, data_size);
    path_display2_shadow.path = malloc(data_size);
    data8 = (uint8_t*) path_display2_shadow.path;
    data32 = (int32_t*) data8;
    
    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = bottomx;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness / 2;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness / 3;
    *data32++ = thickness / 3;
    data8 = (uint8_t*) data32;

    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness * 2.3;
    *data32++ = -BASE_DIM_Y * 5 / 8 - BASE_DIM_Y * 5 / 64 - thickness + thickness * 4 / 6;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness * 2 / 3;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_CLOSE;
    
    *data8 = OPCODE_MOVE;
    data32++;
    *data32++ = BASE_DIM_X - bottomx;
    *data32++ = bottomy;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness / 2;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness / 3;
    *data32++ = thickness / 3;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = -thickness * 2.3;
    *data32++ = -BASE_DIM_Y * 5 / 8 - BASE_DIM_Y * 5 / 64 - thickness + thickness * 4 / 6;
    data8 = (uint8_t*) data32;
    
    *data8 = OPCODE_LINE_REL;
    data32++;
    *data32++ = thickness * 2 / 3;
    *data32++ = 0;
    data8 = (uint8_t*) data32;
        
    *data8 = OPCODE_END;
    
    return 0;
}

int SetupFrame_Display3()
{
    static const int32_t side_thickness = 20;
    static const int32_t topy = BASE_DIM_Y * 7 / 8;
    static const int32_t bottomx = BASE_DIM_X / 10;
    
    //Command of side.
    uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        VLC_OP_END
    };
    
    float sides_data1_left[] = {
        bottomx + side_thickness * 6.85f, BASE_DIM_Y - topy + 200,
        side_thickness, 0,
        side_thickness * 3.15f, -200,
        -side_thickness, 0,
    };
    
    float sides_data1_right[] = {
        BASE_DIM_X - side_thickness - bottomx - side_thickness * 6.85f, BASE_DIM_Y - topy + 200,
        side_thickness, 0,
        -side_thickness * 3.15f, -200,
        -side_thickness, 0,
    };
    
    float sides_data2_left[] = {
        bottomx + side_thickness * 4.73f, BASE_DIM_Y - topy + 333,
        side_thickness, 0,
        side_thickness * 2.12f, -133,
        -side_thickness, 0,
    };
    
    float sides_data2_right[] = {
        BASE_DIM_X - side_thickness - bottomx - side_thickness * 4.73f, BASE_DIM_Y - topy + 333,
        side_thickness, 0,
        -side_thickness * 2.12f, -133,
        -side_thickness, 0,
    };
    
    float sides_data3_left[] = {
        bottomx, BASE_DIM_Y,
        side_thickness, 0,
        side_thickness * 4.73f, -topy + 333,
        -side_thickness, 0,
    };
    
    float sides_data3_right[] = {
        BASE_DIM_X - side_thickness - bottomx, BASE_DIM_Y,
        side_thickness, 0,
        -side_thickness * 4.73f, -topy + 333,
        -side_thickness, 0,
    };
    int data_size;

    //Path sides 1.
    data_size = vg_lite_path_calc_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path_display3_side1_left, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side1_left.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side1_left, sides_cmd, sides_data1_left, sizeof(sides_cmd));
    vg_lite_init_path(&path_display3_side1_right, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side1_right.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side1_right, sides_cmd, sides_data1_right, sizeof(sides_cmd));
    
    //Path sides 2.
    vg_lite_init_path(&path_display3_side2_left, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side2_left.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side2_left, sides_cmd, sides_data2_left, sizeof(sides_cmd));
    vg_lite_init_path(&path_display3_side2_right, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side2_right.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side2_right, sides_cmd, sides_data2_right, sizeof(sides_cmd));
    
    //Path sides 3.
    vg_lite_init_path(&path_display3_side3_left, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side3_left.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side3_left, sides_cmd, sides_data3_left, sizeof(sides_cmd));
    vg_lite_init_path(&path_display3_side3_right, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display3_side3_right.path = malloc(data_size);
    vg_lite_path_append(&path_display3_side3_right, sides_cmd, sides_data3_right, sizeof(sides_cmd));
    
    buf_display3_alert.handle = (void *)0;
    buf_display3_alert.width = img_alert3_width;
    buf_display3_alert.height = img_alert3_height;
    buf_display3_alert.format = (vg_lite_buffer_format_t)img_alert3_format;
    vg_lite_allocate(&buf_display3_alert);
    byte_copy(buf_display3_alert.memory, img_alert3_data, img_alert3_stride * img_alert3_height);
    buf_display3_alert.height--;
    
    buf_display3_stop.handle = buf_display3_alert.handle;
    buf_display3_stop.width = img_stop3_width;
    buf_display3_stop.height = img_stop3_height;
    buf_display3_stop.format = (vg_lite_buffer_format_t)img_stop3_format;
    vg_lite_allocate(&buf_display3_stop);
    byte_copy(buf_display3_stop.memory, img_stop3_data, img_stop3_stride * img_stop3_height);
    
    buf_display3_025.width = buf_display3_05.width = buf_display3_15.width = buf_display3_30.width = img_025_width;
    buf_display3_025.height = buf_display3_05.height = buf_display3_15.height = buf_display3_30.height = img_025_height;
    buf_display3_025.format = buf_display3_05.format = buf_display3_15.format = buf_display3_30.format = img_025_format;
    
    buf_display3_025.handle = buf_display3_stop.handle;
    vg_lite_allocate(&buf_display3_025);
    byte_copy(buf_display3_025.memory, img_025_data, img_025_stride * img_025_height);
    buf_display3_05.height--;
    
    buf_display3_05.handle = buf_display3_025.handle;
    vg_lite_allocate(&buf_display3_05);
    byte_copy(buf_display3_05.memory, img_05_data, img_025_stride * img_025_height);
    
    buf_display3_15.handle = buf_display3_05.handle;
    vg_lite_allocate(&buf_display3_15);
    byte_copy(buf_display3_15.memory, img_15_data, img_025_stride * img_025_height);
    buf_display3_15.height--;
    
    buf_display3_30.handle = buf_display3_15.handle;
    vg_lite_allocate(&buf_display3_30);
    byte_copy(buf_display3_30.memory, img_30_data, img_025_stride * img_025_height);
    buf_display3_30.height--;
    
    return 0;
}

int SetupFrame_Display4()
{
    static const int32_t side_thickness = 10;
    static const int32_t topx = BASE_DIM_X / 4;
    static const int32_t topy = BASE_DIM_Y / 8;
    static const int32_t bottomx = BASE_DIM_X / 8;
    static const int32_t bottomy = BASE_DIM_Y * 7 / 8;
    int data_size = 0;
    
    //Path side.
    uint8_t sides_cmd[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE_REL,
        VLC_OP_LINE,
        VLC_OP_LINE_REL,
        VLC_OP_END
    };
    
    float sides_data_left[] = {
        bottomx, bottomy,
        side_thickness, 0,
        topx + side_thickness, topy,
        -side_thickness, 0,
    };
    float sides_data_right[] = {
        BASE_DIM_X - side_thickness - bottomx, bottomy,
        side_thickness, 0,
        BASE_DIM_X - topx, topy,
        -side_thickness, 0,
    };
    
    data_size = vg_lite_path_calc_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path_display4_side_left, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display4_side_left.path = malloc(data_size);
    vg_lite_path_append(&path_display4_side_left, sides_cmd, sides_data_left, sizeof(sides_cmd));
    vg_lite_init_path(&path_display4_side_right, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display4_side_right.path = malloc(data_size);
    vg_lite_path_append(&path_display4_side_right, sides_cmd, sides_data_right, sizeof(sides_cmd));
    
    buf_display4_alert.handle = (void *)0;
    buf_display4_alert.width = img_alert3_width;
    buf_display4_alert.height = img_alert3_height;
    buf_display4_alert.format = (vg_lite_buffer_format_t)img_alert3_format;
    vg_lite_allocate(&buf_display4_alert);
    byte_copy(buf_display4_alert.memory, img_alert3_data, img_alert3_stride * img_alert3_height);
    buf_display4_alert.height--;
    
    buf_display4_upper.handle = buf_display4_alert.handle;
    buf_display4_upper.width = img_upper_width;
    buf_display4_upper.height = img_upper_height;
    buf_display4_upper.format = (vg_lite_buffer_format_t)img_upper_format;
    vg_lite_allocate(&buf_display4_upper);
    byte_copy(buf_display4_upper.memory, img_upper_data, img_upper_stride * img_upper_height);

    return 0;
}

int SetupFrame_Display5()
{
    static const int32_t side_thickness = 10;
    static const int32_t topx = BASE_DIM_X / 4;
    static const int32_t topy = BASE_DIM_Y / 8;
    static const int32_t bottomx = BASE_DIM_X / 8;
    static const int32_t bottomy = BASE_DIM_Y * 7 / 8;
    int data_size = 0;
    
    //Path frame.
    uint8_t frame_cmd[] = {
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE,
        OPCODE_LINE,
        OPCODE_LINE,
        OPCODE_LINE_REL,
        OPCODE_LINE,
        OPCODE_LINE,
        VLC_OP_CLOSE,
        
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        VLC_OP_CLOSE,
        
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        VLC_OP_CLOSE,
        
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        VLC_OP_CLOSE,
        
        VLC_OP_MOVE,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        OPCODE_LINE_REL,
        VLC_OP_CLOSE,
        
        VLC_OP_END
    };
    
    float frame_data[] = {
        bottomx, bottomy,
        side_thickness, 0,
        topx + side_thickness, topy,
        BASE_DIM_X - topx - side_thickness, topy,
        BASE_DIM_X - side_thickness - bottomx, bottomy,
        side_thickness, 0,
        BASE_DIM_X - topx, topy - side_thickness,
        topx, topy - side_thickness,
        
        bottomx * 1.5 + side_thickness, BASE_DIM_Y / 2,
        side_thickness * 3, 0,
        side_thickness * 0.4, -side_thickness,
        -side_thickness * 3.4, 0,
        
        bottomx * 7 / 4 + side_thickness, BASE_DIM_Y * 5 / 16,
        side_thickness * 3, 0,
        side_thickness * 0.4, -side_thickness,
        -side_thickness * 3.4, 0,
        
        BASE_DIM_X - bottomx * 1.5 - side_thickness, BASE_DIM_Y / 2,
        0, -side_thickness,
        -side_thickness * 3.4, 0,
        side_thickness * 0.4, side_thickness,
        
        BASE_DIM_X - bottomx * 7 / 4 - side_thickness, BASE_DIM_Y * 5 / 16,
        0, -side_thickness,
        -side_thickness * 3.4, 0,
        side_thickness * 0.4, side_thickness,
    };
    
    data_size = vg_lite_path_calc_length(frame_cmd, sizeof(frame_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path_display5_frame, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    path_display5_frame.path = malloc(data_size);
    vg_lite_path_append(&path_display5_frame, frame_cmd, frame_data, sizeof(frame_cmd));
    
    buf_display5_alert.handle = (void *)0;
    buf_display5_alert.width = img_alert5_width;
    buf_display5_alert.height = img_alert5_height;
    buf_display5_alert.format = (vg_lite_buffer_format_t)img_alert5_format;
    vg_lite_allocate(&buf_display5_alert);
    byte_copy(buf_display5_alert.memory, img_alert5_data, img_alert5_stride * img_alert5_height);
    buf_display5_alert.height--;
    
    buf_display5_a.handle = buf_display5_alert.handle;
    buf_display5_a.width = img_icona_width;
    buf_display5_a.height = img_icona_height;
    buf_display5_a.format = (vg_lite_buffer_format_t)img_icona_format;
    vg_lite_allocate(&buf_display5_a);
    byte_copy(buf_display5_a.memory, img_icona_data, img_icona_stride * img_icona_height);
    
    buf_display5_b.handle = buf_display5_a.handle;
    buf_display5_b.width = img_iconb_width;
    buf_display5_b.height = img_iconb_height;
    buf_display5_b.format = (vg_lite_buffer_format_t)img_iconb_format;
    vg_lite_allocate(&buf_display5_b);
    byte_copy(buf_display5_b.memory, img_iconb_data, img_iconb_stride * img_iconb_height);
    buf_display5_b.height--;
    
    buf_display5_c.handle = buf_display5_b.handle;
    buf_display5_c.width = img_iconc_width;
    buf_display5_c.height = img_iconc_height;
    buf_display5_c.format = (vg_lite_buffer_format_t)img_iconc_format;
    vg_lite_allocate(&buf_display5_c);
    byte_copy(buf_display5_c.memory, img_iconc_data, img_iconc_stride * img_iconc_height);
    buf_display5_c.height--;
    
    return 0;
}

int SetupFrame_Display6()
{
    buf_display6_a.handle = (void *)0;
    buf_display6_a.width = img_icona_width;
    buf_display6_a.height = img_icona_height;
    buf_display6_a.format = (vg_lite_buffer_format_t)img_icona_format;
    vg_lite_allocate(&buf_display6_a);
    byte_copy(buf_display6_a.memory, img_icona_data, img_icona_stride * img_icona_height);
    buf_display6_a.height--;
    
    buf_display6_b.handle = buf_display6_a.handle;
    buf_display6_b.width = img_iconb_width;
    buf_display6_b.height = img_iconb_height;
    buf_display6_b.format = (vg_lite_buffer_format_t)img_iconb_format;
    vg_lite_allocate(&buf_display6_b);
    byte_copy(buf_display6_b.memory, img_iconb_data, img_iconb_stride * img_iconb_height);
    buf_display6_b.height--;
    
    buf_display6_c.handle = buf_display6_b.handle;
    buf_display6_c.width = img_iconc_width;
    buf_display6_c.height = img_iconc_height;
    buf_display6_c.format = (vg_lite_buffer_format_t)img_iconc_format;
    vg_lite_allocate(&buf_display6_c);
    byte_copy(buf_display6_c.memory, img_iconc_data, img_iconc_stride * img_iconc_height);
    buf_display6_c.height--;
    
    buf_display6_right.handle = buf_display6_c.handle;
    buf_display6_right.width = img_right_width;
    buf_display6_right.height = img_right_height;
    buf_display6_right.format = (vg_lite_buffer_format_t)img_right_format;
    vg_lite_allocate(&buf_display6_right);
    byte_copy(buf_display6_right.memory, img_right_data, img_right_stride * img_right_height);
    buf_display6_right.height--;
    
    buf_display6_025.width = buf_display6_05.width = buf_display6_15.width = buf_display6_30.width = img_025_width;
    buf_display6_025.height = buf_display6_05.height = buf_display6_15.height = buf_display6_30.height = img_025_height;
    buf_display6_025.format = buf_display6_05.format = buf_display6_15.format = buf_display6_30.format = img_025_format;
    
    buf_display6_025.handle = buf_display6_right.handle;
    vg_lite_allocate(&buf_display6_025);
    byte_copy(buf_display6_025.memory, img_025_data, img_025_stride * img_025_height);
    buf_display6_025.height--;
    
    buf_display6_05.handle = buf_display6_025.handle;
    vg_lite_allocate(&buf_display6_05);
    byte_copy(buf_display6_05.memory, img_05_data, img_025_stride * img_025_height);
    buf_display6_05.height--;
    
    buf_display6_15.handle = buf_display6_05.handle;
    vg_lite_allocate(&buf_display6_15);
    byte_copy(buf_display6_15.memory, img_15_data, img_025_stride * img_025_height);
    buf_display6_15.height--;
    
    buf_display6_30.handle = buf_display6_15.handle;
    vg_lite_allocate(&buf_display6_30);
    byte_copy(buf_display6_30.memory, img_30_data, img_025_stride * img_025_height);
    buf_display6_30.height--;
    
    buf_display6_close.handle = buf_display6_30.handle;
    buf_display6_close.width = img_close1_width;
    buf_display6_close.height = img_close1_height;
    buf_display6_close.format = (vg_lite_buffer_format_t)img_close1_format;
    vg_lite_allocate(&buf_display6_close);
    byte_copy(buf_display6_close.memory, img_close1_data, img_close1_stride * img_close1_height);
    
    buf_display6_stop.handle = buf_display6_close.handle;
    buf_display6_stop.width = img_stop3_width;
    buf_display6_stop.height = img_stop3_height;
    buf_display6_stop.format = (vg_lite_buffer_format_t)img_stop3_format;
    vg_lite_allocate(&buf_display6_stop);
    byte_copy(buf_display6_stop.memory, img_stop3_data, img_stop3_stride * img_stop3_height);
 
    return 0;
}

int SetupFrame_Display7()
{
    buf_display7_a.handle = (void *)0;
    buf_display7_a.width = img_icona_width;
    buf_display7_a.height = img_icona_height;
    buf_display7_a.format = (vg_lite_buffer_format_t)img_icona_format;
    vg_lite_allocate(&buf_display7_a);
    byte_copy(buf_display7_a.memory, img_icona_data, img_icona_stride * img_icona_height);
    buf_display7_a.height--;
    
    buf_display7_b.handle = buf_display7_a.handle;
    buf_display7_b.width = img_iconb_width;
    buf_display7_b.height = img_iconb_height;
    buf_display7_b.format = (vg_lite_buffer_format_t)img_iconb_format;
    vg_lite_allocate(&buf_display7_b);
    byte_copy(buf_display7_b.memory, img_iconb_data, img_iconb_stride * img_iconb_height);
    buf_display7_b.height--;
    
    buf_display7_c.handle = buf_display7_b.handle;
    buf_display7_c.width = img_iconc_width;
    buf_display7_c.height = img_iconc_height;
    buf_display7_c.format = (vg_lite_buffer_format_t)img_iconc_format;
    vg_lite_allocate(&buf_display7_c);
    byte_copy(buf_display7_c.memory, img_iconc_data, img_iconc_stride * img_iconc_height);
    buf_display7_c.height--;
    
    buf_display7_left.handle = buf_display7_c.handle;
    buf_display7_left.width = img_left_width;
    buf_display7_left.height = img_left_height;
    buf_display7_left.format = (vg_lite_buffer_format_t)img_left_format;
    vg_lite_allocate(&buf_display7_left);
    byte_copy(buf_display7_left.memory, img_left_data, img_left_stride * img_left_height);
    buf_display7_left.height--;
    
    buf_display7_stop.handle = buf_display7_left.handle;
    buf_display7_stop.width = img_stop3_width;
    buf_display7_stop.height = img_stop3_height;
    buf_display7_stop.format = (vg_lite_buffer_format_t)img_stop3_format;
    vg_lite_allocate(&buf_display7_stop);
    byte_copy(buf_display7_stop.memory, img_stop3_data, img_stop3_stride * img_stop3_height);
    
    return  0;
}

/* Evaluate a quad-bezier. ****************************** */
static void eval_quad(float x0, float y0, float x1, float y1, float x2, float y2,
                      float x3, float y3, float t, float *outx, float *outy)
{
    *outx = x0 * (1.0f - t) * (1.0 - t) * (1.0f - t) +
    x1 * 3.0f * (1.0f - t) * (1.0f - t) * t  +
    x2 * 3.0f * (1.0f - t) * t * t +
    x3 * t * t * t;
    
    *outy = y0 * (1.0f - t) * (1.0f - t) * (1.0f - t) +
    y1 * 3.0f * (1.0f - t) * (1.0f - t) * t  +
    y2 * 3.0f * (1.0f - t) * t * t +
    y3 * t * t * t;
}

/* Setup tire track paths. */
static void tt_setup_tire_tracks()
{
    int i, j;
    float h_eval;
    float tire_step, tire_eval;
    int data_size = 0;
    
    static uint8_t cmd_poly4[] = {
        VLC_OP_MOVE,
        OPCODE_LINE,
        OPCODE_LINE,
        OPCODE_LINE,
        VLC_OP_CLOSE,
    };
    uint8_t *cmd_tire;

    /* Tire track vertices. */
    float tire_coords[5][8 * TIRES_PER_TRACK];
    float *coords;
    
    /* Compute curve eval param. */
    tire_step = 1.0f / (TIRES_PER_TRACK - 1);
    h_eval = tire_step * 0.3f;
    
    /* Evaluate the vertices. */
    for (j = 0; j < 5; j++) {
        coords = &tire_coords[j][0];
        for (i = 0; i < TIRES_PER_TRACK; i++) {
            tire_eval = i * tire_step;
            /* Evalulate the left point: 1st of the rectangle. */
            eval_quad(tire_curves_left[j][0], tire_curves_left[j][1],
                      tire_curves_left[j][2], tire_curves_left[j][3],
                      tire_curves_left[j][4], tire_curves_left[j][5],
                      tire_curves_left[j][6], tire_curves_left[j][7],
                      tire_eval, coords, coords + 1);
            coords += 2;

            /* Evalulate the left point: 1st of the rectangle. */
            eval_quad(tire_curves_right[j][0], tire_curves_right[j][1],
                      tire_curves_right[j][2], tire_curves_right[j][3],
                      tire_curves_right[j][4], tire_curves_right[j][5],
                      tire_curves_right[j][6], tire_curves_right[j][7],
                      tire_eval, coords, coords + 1);
            coords += 2;

            /* Evalulate the left point: 1st of the rectangle. */
            eval_quad(tire_curves_right[j][0], tire_curves_right[j][1],
                      tire_curves_right[j][2], tire_curves_right[j][3],
                      tire_curves_right[j][4], tire_curves_right[j][5],
                      tire_curves_right[j][6], tire_curves_right[j][7],
                      tire_eval + h_eval, coords, coords + 1);
            coords += 2;

            /* Evalulate the left point: 1st of the rectangle. */
            eval_quad(tire_curves_left[j][0], tire_curves_left[j][1],
                      tire_curves_left[j][2], tire_curves_left[j][3],
                      tire_curves_left[j][4], tire_curves_left[j][5],
                      tire_curves_left[j][6], tire_curves_left[j][7],
                      tire_eval + h_eval, coords, coords + 1);
            coords += 2;
        }
    }
    
    /* Setup the command array for tire_track paths. */
    cmd_tire = (uint8_t *)malloc(sizeof(cmd_poly4) * TIRE_TRACK_PER_PATH);
    for (i = 0; i < TIRE_TRACK_PER_PATH; i++) {
        memcpy(cmd_tire + i * sizeof(cmd_poly4), cmd_poly4, sizeof(cmd_poly4));
    }
    cmd_tire[i * sizeof(cmd_poly4) - 1] = VLC_OP_END;   //end with an END.
    
    /* Steup tire track paths. */
    for (i = 0; i < 5; i++) {
        for (j = 0; j < PATHS_PER_TRACK; j++) {
            data_size = vg_lite_path_calc_length(cmd_tire, sizeof(cmd_poly4) * TIRE_TRACK_PER_PATH, VG_LITE_FP32);
            vg_lite_init_path(&tt_paths[i][j], VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
            tt_paths[i][j].path = malloc(data_size);
            vg_lite_path_append(&tt_paths[i][j], cmd_tire, &tire_coords[i][TIRE_TRACK_PER_PATH * j * 8],
                             sizeof(cmd_poly4) * TIRE_TRACK_PER_PATH);
        }
    }
    
    /* Free resource. */
    free(cmd_tire);
};

static void setup_curves()
{
    int i;
    int data_size = 0;

    static float cp_track_curves[5][16] = {
        {
            TCL0X - 20, TCL0Y,
                
            TCL0X + 50 - 20, TCL0Y - 180,
            TCL0X + 70 - 20, TCL0Y - 250,
            TCL0X + 20 - 20, TCL0Y - 300 + 2,

            TCL0X + 20 - 15, TCL0Y - 300,
                
            TCL0X + 70 - 15, TCL0Y - 250,
            TCL0X + 50 - 15, TCL0Y - 180,
            TCL0X - 15, TCL0Y,
        },
        
        {
            TCR1X + 15, TCR1Y,
            TCR1X - 110 + 15, TCR1Y - 200,
            TCR1X - 190 + 15, TCR1Y - 290,
            TCR1X - 290 + 15, TCR1Y - 340 - 4,

            TCR1X - 290 + 20, TCR1Y - 340 - 6,
            TCR1X - 190 + 20, TCR1Y - 290,
            TCR1X - 110 + 20, TCR1Y - 200,
            TCR1X + 20, TCR1Y,
        },
        
        {
            TCR2X + 15, TCR2Y + 2,
            TCR2X - 0 + 15, TCR2Y - 100,
            TCR2X - 30 + 15, TCR2Y - 220,
            TCR2X - 70 + 15, TCR2Y - 300,
            
            TCR2X - 70 + 20, TCR2Y - 300,
            TCR2X - 30 + 20, TCR2Y - 220,
            TCR2X - 0 + 20, TCR2Y - 100,
            TCR2X + 20, TCR2Y + 3,
        },
        
        {
            TCL3X - 20, TCL3Y,
            TCL3X + 60 - 20, TCL3Y - 100,
            TCL3X + 90 - 20, TCL3Y - 220,
            TCL3X + 105 - 20, TCL3Y - 330,

            TCL3X + 105 - 15, TCL3Y - 330,
            TCL3X + 90 - 15, TCL3Y - 220,
            TCL3X + 60 - 15, TCL3Y - 100,
            TCL3X - 15, TCL3Y,
        },
        
        {
            TCR4X + 15, TCR4Y,
            TCR4X + 45 + 15, TCR4Y - 115,
            TCR4X + 85 + 15, TCR4Y - 235,
            TCR4X + 100 + 15, TCR4Y - 360,

            TCR4X + 100 + 20, TCR4Y - 360,
            TCR4X + 85 + 20, TCR4Y - 235,
            TCR4X + 45 + 20, TCR4Y - 115,
            TCR4X + 20, TCR4Y,
        },
    };
 
    static float cp_corner_curve[2][16] = {
        {
            TCL0X - 50, TCL0Y,
            TCL0X - 50 + 5, TCL0Y - 150,
            TCL0X - 50 - 80, TCL0Y - 190,
            TCL0X - 50 - 120, TCL0Y - 160,
            TCL0X - 50 - 120, TCL0Y - 155,
            TCL0X - 50 - 78, TCL0Y - 185,
            TCL0X - 50  , TCL0Y - 150,
            TCL0X - 50 - 5, TCL0Y,
        },
        {
            TCR1X + 50, TCR1Y,
            TCR1X + 50 - 5, TCR1Y - 150,
            TCR1X + 50 + 80, TCR1Y - 190,
            TCR1X + 50 + 120, TCR1Y - 160,
            TCR1X + 50 + 120, TCR1Y - 155,
            TCR1X + 50 + 78, TCR1Y - 185,
            TCR1X + 50  , TCR1Y - 150,
            TCR1X + 50 + 5, TCR1Y,
        }
    };
    
    static float cp_bottom_curve[16] = {
        TCL0X, TCL0Y,
        TCL0X + 160, TCL0Y + 1,
        TCL0X + 320, TCL0Y - 1,
        TCL0X + 550, TCL0Y,

        TCL0X + 550, TCL0Y - 5,
        TCL0X + 320, TCL0Y - 1 - 5,
        TCL0X + 160, TCL0Y + 1 -  5,
        TCL0X, TCL0Y - 5,
    };
    
    static float cp_center_line_mark[16] = {
        TCR0X + 20, TCR0Y - 100,
        250, -25,
        2, 5,
        -120, 12,
        2, 7,
        -5, 1,
        -1, -6,
        -128, 12
        
    };
    
    static float cp_top_line_mark[16] = {
        TCR0X - 35, TCR0Y - 310,
        150, -35,
        2, 5,
        -70, 15,
        2, 7,
        -5, 1,
        -1, -6,
        -70, 17
    };
    
    static float cp_intrack_curve[16] = {
        TCR3X + 85, TCR3Y - 235,
        TCR3X + 85 + 50, TCR3Y - 235 + 10,
        TCR3X + 85 + 100, TCR3Y - 235 + 20,
        TCR3X + 85 + 160, TCR3Y - 235 + 30,
        
        TCR3X + 85 + 160 + 3, TCR3Y - 235 + 30 - 3,
        TCR3X + 85 + 100 + 3, TCR3Y - 235 + 20 - 2 - 3,
        TCR3X + 85 + 50 + 3, TCR3Y - 235 + 10 + 5 - 3,
        TCR3X + 85 + 3, TCR3Y - 235 - 3,
    };
    /* Path commands for a "stroke" curve. */
    static uint8_t curve_line_cmds[] = {
        VLC_OP_MOVE,
        VLC_OP_CUBIC,
        VLC_OP_LINE,
        VLC_OP_CUBIC,
        VLC_OP_CLOSE
    };
    
    /* Path commands for the lines with center mark: 8 vertices */
    static uint8_t line_mark_cmds[] = {
        VLC_OP_MOVE,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_LINE_REL,
        VLC_OP_END
    };
    
    /* Setup the 5 track curves. */
    data_size = vg_lite_path_calc_length(curve_line_cmds, sizeof(curve_line_cmds), VG_LITE_FP32);
    for (i = 0; i < 5; i++) {
        vg_lite_init_path(&tt_path_track_curves[i], VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
        tt_path_track_curves[i].path = malloc(data_size);
        vg_lite_path_append(&tt_path_track_curves[i], curve_line_cmds, &cp_track_curves[i][0], sizeof(curve_line_cmds));
    }
    
    /* Setup the Bottom curve. */
    vg_lite_init_path(&tt_path_bottom_curve, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_bottom_curve.path = malloc(data_size);
    vg_lite_path_append(&tt_path_bottom_curve, curve_line_cmds, &cp_bottom_curve[0], sizeof(curve_line_cmds));
    
    /* Setup the in-track curve. */
    vg_lite_init_path(&tt_path_intrack_curve, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_intrack_curve.path = malloc(data_size);
    vg_lite_path_append(&tt_path_intrack_curve, curve_line_cmds, &cp_intrack_curve[0], sizeof(curve_line_cmds));
  
    /* Setup the Corner curves. */
    vg_lite_init_path(&tt_path_corner_curve[0], VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_corner_curve[0].path = malloc(data_size);
    vg_lite_path_append(&tt_path_corner_curve[0], curve_line_cmds, &cp_corner_curve[0][0], sizeof(curve_line_cmds));
    vg_lite_init_path(&tt_path_corner_curve[1], VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_corner_curve[1].path = malloc(data_size);
    vg_lite_path_append(&tt_path_corner_curve[1], curve_line_cmds, &cp_corner_curve[1][0], sizeof(curve_line_cmds));
    
    /* Setup the lines in the track with marks: center and top line. */
    data_size = vg_lite_path_calc_length(line_mark_cmds, sizeof(line_mark_cmds), VG_LITE_FP32);
    vg_lite_init_path(&tt_path_top_line, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_top_line.path = malloc(data_size);
    vg_lite_path_append(&tt_path_top_line, line_mark_cmds, &cp_top_line_mark[0], sizeof(line_mark_cmds));
    vg_lite_init_path(&tt_path_center_line, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    tt_path_center_line.path = malloc(data_size);
    vg_lite_path_append(&tt_path_center_line, line_mark_cmds, &cp_center_line_mark[0], sizeof(line_mark_cmds));

}

static void setup_images()
{
    tt_image_car.width = img_car_width;
    tt_image_car.height = img_car_height;
    tt_image_car.format = img_car_format;
    tt_image_car.handle = NULL;
    vg_lite_allocate(&tt_image_car);
    tt_image_car.height--;
    memcpy(tt_image_car.memory, img_car_data, img_car_height * img_car_stride);
    vg_lite_identity(&tt_mat_car);
    vg_lite_translate(200, 250, &tt_mat_car);
    vg_lite_scale(3.5f, 3.5f, &tt_mat_car);
    
    tt_image_navi.width = img_icona_width;
    tt_image_navi.height = img_icona_height;
    tt_image_navi.format = img_icona_format;
    tt_image_navi.handle = tt_image_car.handle;
    vg_lite_allocate(&tt_image_navi);
    tt_image_navi.height--;
    memcpy(tt_image_navi.memory, img_icona_data, img_icona_height * img_icona_stride);
    vg_lite_identity(&tt_mat_navi);
    vg_lite_translate(900, 950, &tt_mat_navi);
    vg_lite_scale(6.f, 1.f, &tt_mat_navi);
    
    vg_lite_identity(&tt_mat_alert);
    vg_lite_translate(1000, 150, &tt_mat_alert);
    vg_lite_scale(4.f, 0.7f, &tt_mat_alert);
    
    vg_lite_identity(&tt_mat_radar);
    vg_lite_translate(900, 0, &tt_mat_radar);
    vg_lite_scale(6.f, 0.8f, &tt_mat_radar);

}

int SetupFrame_Display8()
{
    /*
     1. Setup tire track paths;
     2. Setup curves;
     3. Setup the rectangle for line render
     4. Setup images.
     */
    
    tt_setup_tire_tracks();
    
    setup_curves();
    setup_images();
    
    return 0;
}

/* Callback for error recovery. */
vg_lite_error_t error_call1(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(2, alert1_clut));
    CHECK_ERROR(vg_lite_set_CLUT(2, close1_clut));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display1(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    float scale_x = (float)width / BASE_DIM_X;
    float scale_y = (float)height / BASE_DIM_Y;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call1());
    //top.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display1_mark, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display1_top));
    
    //center.
    vg_lite_identity(&mat);
    vg_lite_translate(-75 * ref_scale_x, 250 * ref_scale_y, &mat);
    vg_lite_scale(scale_x * 1.08f, scale_y * 1.08f, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display1_mark, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display1_center));
    
    //bottom.
    vg_lite_identity(&mat);
    vg_lite_translate(-150 * ref_scale_x, 500 * ref_scale_y, &mat);
    vg_lite_scale(scale_x * 1.16f, scale_y * 1.16f, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display1_mark, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display1_bottom));
    CHECK_ERROR(vg_lite_draw(fb, &path_display1_tick, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display1_bottom));
    
    //Notify bar.
    vg_lite_identity(&mat);
    vg_lite_translate(0, 70 * ref_scale_y, &mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_confirm_notify, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xffffffff));

    vg_lite_identity(&mat);
    vg_lite_translate(320 * ref_scale_x, 950 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display1_alert, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(1700 * ref_scale_x, 50 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display1_close, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display2(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    float scale_x = (float)width / BASE_DIM_X;
    float scale_y = (float)height / BASE_DIM_Y;
    vg_lite_matrix_t mat;
    
    //line.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display2_line, VG_LITE_FILL_NON_ZERO, &mat, VG_LITE_BLEND_NONE, color_display2_line));
    
    //mark.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display2_mark, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display2_mark));
    
    //shadow.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display2_shadow, VG_LITE_FILL_NON_ZERO, &mat, VG_LITE_BLEND_NONE, color_display2_shadow));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call3(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_alert3));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_stop3));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_025));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_05));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_15));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_30));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display3(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i, count = 6;
    float scale_x = (float)width / BASE_DIM_X;
    float scale_y = (float)height / BASE_DIM_Y;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call3());   
    //line 1.
    for (i = 0; i < count; i++) {
        rect_display3_line.x = (470 + 990 * i / count) * ref_scale_x;
        rect_display3_line.y = 235 * ref_scale_y;
        rect_display3_line.width = (990 / count - 10) * ref_scale_x;
        rect_display3_line.height = 30 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display3_line, color_display3_line1));
    }
    
    //line 2.
    for (i = 0; i < count; i++) {
        rect_display3_line.x = (415 + 1120 * i / count) * ref_scale_x;
        rect_display3_line.y = 435 * ref_scale_y;
        rect_display3_line.width = (1120 / count - 10) * ref_scale_x;
        rect_display3_line.height = 30 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display3_line, color_display3_line2));
    }
    
    //line 3.
    for (i = 0; i < count; i++) {
        rect_display3_line.x = (350 + 1235 * i / count) * ref_scale_x;
        rect_display3_line.y = 635 * ref_scale_y;
        rect_display3_line.width = (1235 / count - 10) * ref_scale_x;
        rect_display3_line.height = 30 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display3_line, color_display3_line3));
    }
    
    //line 4.
    for (i = 0; i < count; i++) {
        rect_display3_line.x = (280 + 1380 * i / count) * ref_scale_x;
        rect_display3_line.y = 835 * ref_scale_y;
        rect_display3_line.width = (1380 / count - 10) * ref_scale_x;
        rect_display3_line.height = 30 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display3_line, color_display3_line3));
    }

    //side 1.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side1_left, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side1));
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side1_right, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side1));
    //side 2.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side2_left, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side2));
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side2_right, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side2));
    //side 3.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side3_left, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side3));
    CHECK_ERROR(vg_lite_draw(fb, &path_display3_side3_right, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display3_side3));

    vg_lite_identity(&mat);
    vg_lite_translate(800 * ref_scale_x, 770 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_translate(-670, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_025, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_translate(1530, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_025, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    
    vg_lite_identity(&mat);
    vg_lite_translate(210 * ref_scale_x, 570 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_05, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_translate(1400, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_05, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_identity(&mat);
    vg_lite_translate(280 * ref_scale_x, 370 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_15, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_translate(1260, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_15, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(350 * ref_scale_x, 170 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_30, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_translate(1120, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_30, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(0, 40 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_alert, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &buf_display3_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call4(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_alert3));
    CHECK_ERROR(vg_lite_set_CLUT(256, clut_upper));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display4(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i, count = 20;
    float scale_x = (float)width / BASE_DIM_X;
    float scale_y = (float)height / BASE_DIM_Y;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call4());

    //line 1.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    for (i = 0; i < count; i++) {
        rect_display4_line.x = (435 + 1065 * i / count) * ref_scale_x;
        rect_display4_line.y = 322 * ref_scale_y;
        rect_display4_line.width = (1065 / count - 10) * ref_scale_x;
        rect_display4_line.height = 15 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display4_line, color_display4_line1));
    }
    
    //line 2.
    vg_lite_identity(&mat);
    vg_lite_translate(-288 * ref_scale_x, 500 * ref_scale_y, &mat);
    vg_lite_scale(scale_x * 1.3f, scale_y, &mat);
    for (i = 0; i < count; i++) {
        rect_display4_line.x = (286 + 1365 * i / count) * ref_scale_x;
        rect_display4_line.y = 822 * ref_scale_y;
        rect_display4_line.width = (1365 / count - 10) * ref_scale_x;
        rect_display4_line.height = 15 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display4_line, color_display4_line2));
    }

    //side.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display4_side_left, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display4_side));
    CHECK_ERROR(vg_lite_draw(fb, &path_display4_side_right, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, color_display4_side));

    vg_lite_identity(&mat);
    vg_lite_translate(170 * ref_scale_x, 1000 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display4_upper, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(532, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display4_upper, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(532, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display4_upper, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

    vg_lite_identity(&mat);
    vg_lite_translate(702 * ref_scale_x, 916 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display4_upper, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(0, 40 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display4_alert, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call5(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_alert5));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_icona));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_iconb));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_iconc));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display5(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i, count = 25;
    float scale_x = (float)width / BASE_DIM_X;
    float scale_y = (float)height / BASE_DIM_Y;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call5());
    //side.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    CHECK_ERROR(vg_lite_draw(fb, &path_display5_frame, VG_LITE_FILL_NON_ZERO, &mat, VG_LITE_BLEND_NONE, color_display5_frame));
    
    //line 1.
    vg_lite_identity(&mat);
    vg_lite_scale(scale_x, scale_y, &mat);
    for (i = 0; i < count; i++) {
        rect_display5_line.x = (435 + 1055 * i / count) * ref_scale_x;
        rect_display5_line.y = 727 * ref_scale_y;
        rect_display5_line.width = (1055 / count - 10) * ref_scale_x;
        rect_display5_line.height = 15 * ref_scale_y;
        CHECK_ERROR(vg_lite_clear(fb, &rect_display5_line, color_display5_line));
    }
    
    vg_lite_identity(&mat);
    vg_lite_translate(20 * ref_scale_x, 950 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display5_a, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(206, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display5_b, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(206, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display5_c, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(638 * ref_scale_x, 950 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display5_alert, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call6(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_iconc));
    CHECK_ERROR(vg_lite_set_CLUT(256, clut_right));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display6(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call6());
    vg_lite_identity(&mat);
    vg_lite_translate(800 * ref_scale_x, 100 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    vg_lite_scale(4.0f, 1.5f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_a, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 150, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_b, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 150, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_c, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 150, &mat);
    vg_lite_scale(0.375f, 2.0f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_right, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(150 * ref_scale_x, 120 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    vg_lite_scale(4.0f, 1.5f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_025, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 100, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_05, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 100, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_15, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(0, 100, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_30, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(160 * ref_scale_x, 750 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    vg_lite_scale(2.5f, 2.0f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_close, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    
    vg_lite_identity(&mat);
    vg_lite_translate(1600 * ref_scale_x, 0, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    vg_lite_scale(1.2f, (float)height / buf_display6_stop.height / ref_scale_y, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display6_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call7(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(256, clut_left));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_iconc));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display7(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    float ref_scale_x = (float)width / REF_DIM_X;
    float ref_scale_y = (float)height / REF_DIM_Y;
    
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call7());   
    vg_lite_identity(&mat);
    vg_lite_translate(120 * ref_scale_x, 800 * ref_scale_y, &mat);
    vg_lite_scale(ref_scale_x, ref_scale_y, &mat);
    vg_lite_scale(0.5f, 2.0f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display7_left, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(670, 0, &mat);
    vg_lite_scale(2.667f, 0.5f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display7_a, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(250, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display7_b, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(250, 0, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display7_c, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));
    vg_lite_translate(250, 0, &mat);
    vg_lite_scale(0.75f, 2.0f, &mat);
    CHECK_ERROR(vg_lite_blit(fb, &buf_display7_stop, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}
vg_lite_error_t error_call8(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_set_CLUT(256, clut_car));
    CHECK_ERROR(vg_lite_set_CLUT(2, clut_icona));

ErrorHandler:
    return error;
}
vg_lite_error_t RenderFrame_Display8(int32_t width, int32_t height)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i, j;
    vg_lite_matrix_t mat;
    
    CHECK_ERROR(error_call8());
    vg_lite_identity(&mat);
    
    /* Draw the tire_tracks and the curves besides them. */
    for (i = 0; i < 5; i ++) {
        for (j = 0; j < PATHS_PER_TRACK; j++) {
            CHECK_ERROR(vg_lite_draw(fb, &tt_paths[i][j], VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
        }
    }
    
    for (i = 0; i < 5; i ++) {
        CHECK_ERROR(vg_lite_draw(fb, &tt_path_track_curves[i], VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
    }
    /* Draw the curves/lines between the tire_tracks. */
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_bottom_curve, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff0000ff));
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_intrack_curve, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_top_line, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_center_line, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
 
    /* Draw the 2 corner lines. */
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_corner_curve[0], VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));
    CHECK_ERROR(vg_lite_draw(fb, &tt_path_corner_curve[1], VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, 0xff00ffff));


    /* Draw car. */
    CHECK_ERROR(vg_lite_blit(fb, &tt_image_car, &tt_mat_car, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &tt_image_navi, &tt_mat_navi, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &tt_image_navi, &tt_mat_alert, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_POINT));
    CHECK_ERROR(vg_lite_blit(fb, &tt_image_navi, &tt_mat_radar, VG_LITE_BLEND_SRC_OVER, 0, VG_LITE_FILTER_POINT));

ErrorHandler:
    return error;
}

void DestroyFrame_Display1()
{
    if (path_display1_mark.path != NULL)
    {
        free(path_display1_mark.path);
        path_display1_mark.path = NULL;
    }
    if (path_display1_tick.path != NULL)
    {
        free(path_display1_tick.path);
        path_display1_tick.path = NULL;
    }
    if (buf_display1_alert.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display1_alert);
    }
    if (buf_display1_close.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display1_close);
    }
}
void DestroyFrame_Display2()
{
    if (path_display2_shadow.path != NULL)
    {
        free(path_display2_shadow.path);
        path_display2_shadow.path = NULL;
    }
    if (path_display2_line.path != NULL)
    {
        free(path_display2_line.path);
        path_display2_line.path = NULL;
    }
    if (path_display2_mark.path != NULL)
    {
        free(path_display2_mark.path);
        path_display2_mark.path = NULL;
    }
}
void DestroyFrame_Display3()
{
    if (path_display3_side1_left.path != NULL)
    {
        free(path_display3_side1_left.path);
        path_display3_side1_left.path = NULL;
    }
    if (path_display3_side1_right.path != NULL)
    {
        free(path_display3_side1_right.path);
        path_display3_side1_right.path = NULL;
    }
    if (path_display3_side2_left.path != NULL)
    {
        free(path_display3_side2_left.path);
        path_display3_side2_left.path = NULL;
    }
    if (path_display3_side2_right.path != NULL)
    {
        free(path_display3_side2_right.path);
        path_display3_side2_right.path = NULL;
    }
    if (path_display3_side3_left.path != NULL)
    {
        free(path_display3_side3_left.path);
        path_display3_side3_left.path = NULL;
    }
    if (path_display3_side3_right.path != NULL)
    {
        free(path_display3_side3_right.path);
        path_display3_side3_right.path = NULL;
    }
    if (buf_display3_alert.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_alert);
    }
    if (buf_display3_stop.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_stop);
    }
    if (buf_display3_025.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_025);
    }
    if (buf_display3_05.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_05);
    }
    if (buf_display3_15.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_15);
    }
    if (buf_display3_30.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display3_30);
    }
}
void DestroyFrame_Display4()
{
    if (path_display4_side_left.path != NULL)
    {
        free(path_display4_side_left.path);
        path_display4_side_left.path = NULL;
    }
    if (path_display4_side_right.path != NULL)
    {
        free(path_display4_side_right.path);
        path_display4_side_right.path = NULL;
    }
    if (buf_display4_alert.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display4_alert);
    }
    if (buf_display4_upper.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display4_upper);
    }
}
void DestroyFrame_Display5()
{
    if (path_display5_frame.path != NULL)
    {
        free(path_display5_frame.path);
        path_display5_frame.path = NULL;
    }
    if (buf_display5_alert.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display5_alert);
    }
    if (buf_display5_a.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display5_a);
    }
    if (buf_display5_b.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display5_b);
    }
    if (buf_display5_c.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display5_c);
    }
}
void DestroyFrame_Display6()
{
    if (buf_display6_right.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_right);
    }
    if (buf_display6_a.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_a);
    }
    if (buf_display6_b.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_b);
    }
    if (buf_display6_c.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_c);
    }
    if (buf_display6_025.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_025);
    }
    if (buf_display6_05.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_05);
    }
    if (buf_display6_15.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_15);
    }
    if (buf_display6_30.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_30);
    }
    if (buf_display6_stop.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_stop);
    }
    if (buf_display6_close.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display6_close);
    }
}
void DestroyFrame_Display7()
{
    if (buf_display7_left.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display7_left);
    }
    if (buf_display7_a.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display7_a);
    }
    if (buf_display7_b.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display7_b);
    }
    if (buf_display7_c.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display7_c);
    }
    if (buf_display7_stop.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&buf_display7_stop);
    }
}

void DestroyFrame_Display8()
{
    int i;
    if (tt_path_bottom_curve.path != NULL) {
        free(tt_path_bottom_curve.path);
    }
    if (tt_path_center_line.path != NULL) {
        free(tt_path_center_line.path);
    }
    if (tt_path_top_line.path != NULL) {
        free(tt_path_top_line.path);
    }
    if (tt_path_intrack_curve.path != NULL) {
        free(tt_path_intrack_curve.path);
    }
    
    for (i = 0; i < 5; i++) {
        if (tt_path_track_curves[i].path != NULL) {
            free(tt_path_track_curves[i].path);
        }
    }
    
    for (i = 0; i < 2; i++) {
        if (tt_path_corner_curve[i].path != NULL) {
            free(tt_path_corner_curve[i].path);
        }
    }
}

int SetupFrames(int frame)
{
    int result = 0;
    switch (frame)
    {
        case 3:
            result = SetupFrame_Display1();
            break;
            
        case 4:
            result = SetupFrame_Display2();
            break;
            
        case 5:
            result = SetupFrame_Display3();
            break;
            
        case 6:
            result = SetupFrame_Display4();
            break;
            
        case 7:
            result = SetupFrame_Display5();
            break;
            
        case 8:
            result = SetupFrame_Display6();
            break;
            
        case 9:
            result = SetupFrame_Display7();
            break;
            
        case 10:
            result = SetupFrame_Display8();
            break;
            
        default:
            break;
    }
    
    return result;
}

void cleanup(void)
{
    DestroyFrame_Display1();
    DestroyFrame_Display2();
    DestroyFrame_Display3();
    DestroyFrame_Display4();
    DestroyFrame_Display5();
    DestroyFrame_Display6();
    DestroyFrame_Display7();
    DestroyFrame_Display8();
    if (buffer.handle != NULL) {
        // Free the offscreen framebuffer memory.
        vg_lite_free(&buffer);
    }
    
    vg_lite_close();
    
}

int parse_args(int argc, const char * argv[])
{
    int i;
    int result = 1;
    
    for (i = 1; i < argc; i++)
    {
        if (i == argc - 1)
        {
            result = 0;
            break;
        }
        
        if (argv[i][0] == '-')
        {
            switch (argv[i][1]) {
                case 'w':
                    render_width = atoi(argv[++i]);
                    break;
                    
                case 'h':
                    render_height = atoi(argv[++i]);
                    break;
                    
                case 'f':
                    frames = atoi(argv[++i]);
                    break;
                    
                case 's':
                    show = atoi(argv[++i]);
                    break;
                default:
                    result = 0;
                    break;
            }
            
            // Invalid input parameters.
            if (result == 0)
            {
                break;
            }
        }
        else
        {
            // Invalid input parameters.
            result = 0;
            break;
        }
    }
    
    return result;
}

#if _WIN32
static void save_buffer(char *name, void *memory, int size)
{
    FILE *file = fopen((const char*)name, "wb+");
    if (file) {
        int written;
        written = fwrite(memory, size, 1, file);
        fclose(file);
        if (written == size) {
            printf("Buffer dump OK to file %s\n", name);
        }
    }
}
#endif

int main(int argc, const char * argv[])
{
    uint32_t fcount = 0;
    uint32_t feature_check = 0;
    long time;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    printf("Usage: -w <width> -h <height> -f <count> -s < /3/4/5/6/7/8/9/10: frame to show>\n");

    if (0 == parse_args(argc, argv)) {
        printf("Invalid arguments.\n");
    }

    CHECK_ERROR(vg_lite_init(render_width, render_height));

    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (!feature_check) {
        printf("index format is not supported.\n");
        return -1;
    }
    has_blitter = 1;
    /* Allocate the off-screen buffer. */
    buffer.width  = render_width;
    buffer.height = render_height;
    buffer.format = VG_LITE_BGRA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    printf("Render size: %d x %d\n", render_width, render_height);

    /* Clear the buffer with blue. */
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xffaabbcc));
    /**** DRAW ****/
    SetupFrames(show);

    while (frames > 0) {
        switch (show) {
            case 3:
                CHECK_ERROR(RenderFrame_Display1(render_width, render_height));
                break;
                
            case 4:
                CHECK_ERROR(RenderFrame_Display2(render_width, render_height));
                break;
                
            case 5:
                time = gettick();
                CHECK_ERROR(RenderFrame_Display3(render_width, render_height));
                time = gettick() - time;
                printf("CPU frame time is %ld us.\n", time);
                break;
                
            case 6:
                time = gettick();
                CHECK_ERROR(RenderFrame_Display4(render_width, render_height));
                time = gettick() - time;
                printf("CPU frame time is %ld us.\n", time);
                break;
                
            case 7:
                CHECK_ERROR(RenderFrame_Display5(render_width, render_height));
                break;
                
            case 8:
                CHECK_ERROR(RenderFrame_Display6(render_width, render_height));
                break;
                
            case 9:
                CHECK_ERROR(RenderFrame_Display7(render_width, render_height));
                break;
                
            case 10:
                CHECK_ERROR(RenderFrame_Display8(render_width, render_height));
                break;
                
            default:
                break;
        }
        CHECK_ERROR(vg_lite_finish());
        frames--;
        printf("frame %d done\n", fcount++);
    }

    // Save PNG file.
    {
        char png_name[20] = {'\0'};
        sprintf(png_name, "parking_s%d.png", show);
        vg_lite_save_png(png_name, fb);
#if _WIN32
        sprintf(png_name, "parking_s%d.fb", show);
        save_buffer(png_name, buffer.memory, buffer.stride * buffer.height);
#endif
    }

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
