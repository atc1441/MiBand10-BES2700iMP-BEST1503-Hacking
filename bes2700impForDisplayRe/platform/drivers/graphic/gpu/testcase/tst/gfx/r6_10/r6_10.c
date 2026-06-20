


#include <stdio.h>
#include "vg_lite.h"

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

const int32_t countd = 18;      /* 18 dashes per curve */
const int32_t countc = 2;       /* 2 dashed curves */
static int num = 1;

/* Size of fb that render into. */
static const int   render_width = 1920, render_height = 1080;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

/*Paths */
vg_lite_path_t trap0, trap1, trap2;
vg_lite_path_t track_blue, track_yellow;
vg_lite_path_t track_alert;
#if 0
const float tx = 45.0f;
const float ty = 20.0f;
const float step =12.0f;
const float stepy = 20.0f;
const float unit_size = 100.0f;
const float lwidth = 0.5f;
#endif
#define tx 45.0f
#define ty 20.0f
#define step 12.0f
#define stepy 20.0f
#define unit_size 100.0f
#define lwidth 0.5f

/* Path commands for a 4-edge polygon. */
uint8_t poly4_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};
float trap0_data[] = {
    tx, ty,
    unit_size - tx, ty,
    unit_size - tx + step, ty + stepy,
    tx - step, ty + stepy
};
float trap1_data[] = {
    tx - step, ty + stepy,
    unit_size - tx + step, ty + stepy,
    unit_size - tx + step * 2, ty + stepy * 2,
    tx - step * 2, ty + stepy * 2
};
float trap2_data[] = {
    tx - step * 2, ty + stepy * 2,
    unit_size - tx + step * 2, ty + stepy * 2,
    unit_size - tx + step * 3, ty + stepy * 3,
    tx - step * 3, ty + stepy * 3
};
float track_alert_data[] = {
    tx - step * 3, ty + stepy * 3,
    unit_size - tx + step * 3, ty + stepy * 3,
    unit_size - tx + step * 3, ty + stepy * 3 + lwidth,
    tx - step * 3, ty + stepy * 3 + lwidth,
};

uint8_t track_blue_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,

    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};
float track_blue_data[] = {
    tx - step * 3 - lwidth, ty + stepy * 3 + lwidth,
    tx - step * 3, ty + stepy * 3 + lwidth,
    tx, ty,
    tx - lwidth, ty,

    unit_size - tx + step * 3, ty + stepy * 3 + lwidth,
    unit_size - tx + step * 3 + lwidth, ty + stepy * 3 + lwidth,
    unit_size - tx + lwidth, ty,
    unit_size - tx, ty,

};

/*const float track0_cp0_x = tx - step * 3; */
/*const float track0_cp0_y = ty + stepy * 3; */
#define track0_cp0_x (tx - step * 3)
#define track0_cp0_y (ty + stepy * 3)
#define track0_cp1_x (track0_cp0_x + 20)
#define track0_cp1_y (track0_cp0_y - 35)
#define track0_cp2_x (track0_cp0_x + 40)
#define track0_cp2_y (track0_cp0_y - 60)
#define track0_cp3_x (track0_cp0_x + 55)
#define track0_cp3_y (track0_cp0_y - 62)
#define track1_cp0_x (unit_size - tx + step * 3)
#define track1_cp0_y (ty + stepy * 3)
#define track1_cp1_x (track1_cp0_x - 20)
#define track1_cp1_y (track1_cp0_y - 40)
#define track1_cp2_x (track1_cp0_x - 30)
#define track1_cp2_y (track1_cp0_y - 50)
#define track1_cp3_x (track1_cp0_x - 15)
#define track1_cp3_y (track1_cp0_y - 55)
/*const float track0_cp1_x = track0_cp0_x + 20; */
/*const float track0_cp1_y = track0_cp0_y - 35; */
/*const float track0_cp2_x = track0_cp0_x + 40; */
/*const float track0_cp2_y = track0_cp0_y - 60; */
/*const float track0_cp3_x = track0_cp0_x + 55; */
/*const float track0_cp3_y = track0_cp0_y - 62; */
/*const float track1_cp0_x = unit_size - tx + step * 3; */
/*const float track1_cp0_y = ty + stepy * 3; */
/*const float track1_cp1_x = track1_cp0_x - 20; */
/*const float track1_cp1_y = track1_cp0_y - 40; */
/*const float track1_cp2_x = track1_cp0_x - 30; */
/*const float track1_cp2_y = track1_cp0_y - 50; */
/*const float track1_cp3_x = track1_cp0_x - 15; */
/*const float track1_cp3_y = track1_cp0_y - 55; */

uint8_t track_yellow_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_LINE,
    VLC_OP_CUBIC,

    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_END
};
float track_yellow_data[] = {
    track0_cp0_x - 6, track0_cp0_y + 10,
    track0_cp0_x - lwidth - 6, track0_cp0_y + 10,
    track0_cp1_x - lwidth, track0_cp1_y,
    track0_cp2_x - lwidth, track0_cp2_y,
    track0_cp3_x - lwidth, track0_cp3_y,
    track0_cp3_x, track0_cp3_y,
    track0_cp2_x, track0_cp2_y,
    track0_cp1_x, track0_cp1_y,
    track0_cp0_x - 6, track0_cp0_y + 10,

    track1_cp0_x + 6, track1_cp0_y + 10,
    track1_cp0_x - lwidth + 6, track1_cp0_y + 10,
    track1_cp1_x - lwidth, track1_cp1_y,
    track1_cp2_x - lwidth, track1_cp2_y,
    track1_cp3_x - lwidth, track1_cp3_y,
    track1_cp3_x, track1_cp3_y,
    track1_cp2_x, track1_cp2_y,
    track1_cp1_x, track1_cp1_y,
    track1_cp0_x + 6, track1_cp0_y + 10,
};

uint32_t color_trap0 = 0x440000ff;
uint32_t color_trap1 = 0x880000ff;
uint32_t color_trap2 = 0xff0000ff;
uint32_t color_track_alert = 0xffff0000;
uint32_t color_track_blue = 0xff0000ff;
uint32_t color_track_yellow = 0xffbbaa00;

/*Images */
vg_lite_buffer_t img_car, img_cue, img_menu;

void cleanup(void)
{
    vg_lite_clear_path(&trap0);
    vg_lite_clear_path(&trap1);
    vg_lite_clear_path(&trap2);
    vg_lite_clear_path(&track_alert);
    vg_lite_clear_path(&track_yellow);
    vg_lite_clear_path(&track_blue);

    if (img_car.memory != NULL) {
        vg_lite_free(&img_car);
    }
    if (img_cue.memory != NULL) {
        vg_lite_free(&img_cue);
    }
    if (img_menu.memory != NULL) {
        vg_lite_free(&img_menu);
    }

    vg_lite_close();

}

/* Automatically append path data and commands into a path object.
 Assuming floating only.
 */
#define CDALIGN(value) (((value) + (4) - 1) & ~((4) - 1))
#define DATA_SIZE 4
#define CDMIN(x, y) ((x) > (y) ? (y) : (x))
#define CDMAX(x, y) ((x) > (y) ? (x) : (y))

/*
#define VLC_OP_END          0x00
#define VLC_OP_CLOSE        0x01
#define VLC_OP_MOVE         0x02
#define VLC_OP_MOVE_REL     0x03
#define VLC_OP_LINE         0x04
#define VLC_OP_LINE_REL     0x05
#define VLC_OP_QUAD         0x06
#define VLC_OP_QUAD_REL     0x07
#define VLC_OP_CUBIC        0x08
#define VLC_OP_CUBIC_REL    0x09
*/
static inline int32_t get_data_count(uint8_t cmd)
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
        size++;     /*OP CODE. */
        dCount = get_data_count(cmd[i]);
        if (dCount > 0) {
            size = CDALIGN(size);
            size += dCount * DATA_SIZE;
        }
    }

    return size;
}

static int32_t append_path_data(vg_lite_path_t *path,
                                uint8_t        *cmd,
                                void           *data,
                                uint32_t        seg_count)
{
    int32_t size = 0;
    uint32_t i, j;
    int32_t offset = 0;
    int32_t dataCount = 0;
    float *dataf = (float*) data;
    float *pathf = NULL;
    uint8_t *pathc = NULL;

    size = calc_path_size(cmd, seg_count);
    path->path = malloc(size);
    path->path_length = size;
    path->path_changed= 1;

    pathf = (float *)path->path;
    pathc = (uint8_t *)path->path;

    /* Loop to fill path data. */
    for (i = 0; i < seg_count; i++) {
        *(pathc + offset) = cmd[i];
        offset++;
        dataCount = get_data_count(cmd[i]);
        if (dataCount > 0) {
            offset = CDALIGN(offset);
            pathf = (float *) (pathc + offset);
            for (j = 0; j < dataCount / 2; j++) {
                pathf[j * 2] = *dataf++;
                pathf[j * 2 + 1] = *dataf++;

                /* Update path bounds. */
                path->bounding_box[0] = CDMIN(path->bounding_box[0], pathf[j * 2]);
                path->bounding_box[2] = CDMAX(path->bounding_box[2], pathf[j * 2]);
                path->bounding_box[1] = CDMIN(path->bounding_box[1], pathf[j * 2 + 1]);
                path->bounding_box[3] = CDMAX(path->bounding_box[3], pathf[j * 2 + 1]);
            }

            offset += dataCount * DATA_SIZE;
        }
    }

    return size;
}

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

static void eval_line(float x0, float y0, float x1, float y1, float t, float *outx, float *outy)
{
    *outx = x0 + (x1 - x0) * t;
    *outy = y0 + (y1 - y0) * t;
}

static void build_paths()
{
    vg_lite_init_path(&trap0, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap0, poly4_cmd, trap0_data, sizeof(poly4_cmd));

    vg_lite_init_path(&trap1, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap1, poly4_cmd, trap1_data, sizeof(poly4_cmd));

    vg_lite_init_path(&trap2, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap2, poly4_cmd, trap2_data, sizeof(poly4_cmd));

    vg_lite_init_path(&track_alert, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_alert, poly4_cmd, track_alert_data, sizeof(poly4_cmd));

    vg_lite_init_path(&track_blue, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_blue, track_blue_cmd, track_blue_data, sizeof(track_blue_cmd));

    vg_lite_init_path(&track_yellow, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_yellow, track_yellow_cmd, track_yellow_data, sizeof(track_yellow_cmd));
}


static void byte_copy(void *dst, void *src, int size)
{
    uint8_t *dst8 = (uint8_t *)dst;
    uint8_t *src8 = (uint8_t *)src;
    int i;
    for (i = 0; i < size; i++)
    {
        dst8[i] = src8[i];
    };
}

int main(int argc, const char * argv[])
{
    uint32_t i, loops;
    vg_lite_error_t error = vg_lite_init(render_width, render_height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    buffer.width  = render_width;
    buffer.height = render_height;
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    loops = 1;
    printf("Render size: %d x %d\n", buffer.width, buffer.height);

    build_paths();
    vg_lite_clear(fb, NULL, 0xffaabbcc);

    for(i = 0; i < loops; i++) {
        vg_lite_matrix_t mat;
        vg_lite_identity(&mat);
        vg_lite_scale(render_width / 100.0f, render_height / 100.0f, &mat);
        vg_lite_draw(fb, &trap0, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap0);
        vg_lite_draw(fb, &trap1, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap1);
        vg_lite_draw(fb, &trap2, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap2);
        vg_lite_draw(fb, &track_alert, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_alert);
        vg_lite_draw(fb, &track_blue, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_blue);
        vg_lite_draw(fb, &track_yellow, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_yellow);

        vg_lite_finish();
        printf("frame %d done\n", i);
    }
    /* Save PNG file. */
    vg_lite_save_png("r6_10.png", fb);
    /* Cleanup. */
    cleanup();
    return 0;
}
