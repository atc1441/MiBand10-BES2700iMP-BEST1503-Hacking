


#include <stdio.h>
#include "vg_lite.h"

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#include "cue_index1.img.h"
#include "menu_index2.img.h"
#include "car_index8.img.h"

const int32_t countd = 18;      /* 18 dashes per curve */
const int32_t countc = 2;       /* 2 dashed curves */
static int num = 1;

/* Size of fb that render into. */
static const int   render_width = 1920, render_height = 1080;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

/*Paths */
vg_lite_path_t trap0, trap1, trap2;
vg_lite_path_t track_blue0, track_blue1, track_yellow0, track_yellow1;
vg_lite_rectangle_t rect_alert, rect_trap0, rect_trap1;

#define tx 45.0f
#define ty 20.0f
#define step 12.0f
#define stepy 20.0f
#define unit_size 100.0f
#define lwidth 0.5f
#define SX (1920.0f / 100.0f)
#define SY (1080.0f / 100.0f)
/* Path commands for a 4-edge polygon. */
uint8_t poly4_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};
float trap0_data[] = {
    tx * SX, ty * SY,
    (unit_size - tx) * SX, ty * SY,
    (unit_size - tx + step) * SX, 430,/*(ty + stepy) * SY, */
    (tx - step) * SX, 430/*(ty + stepy) * SY */
};
float trap1_data[] = {
    (tx - step) * SX - 5, 438,/*(ty + stepy) * SY, */
    (unit_size - tx + step) * SX + 5, 438,/*(ty + stepy) * SY, */
    (unit_size - tx + step * 2) * SX, 645,/*(ty + stepy * 2) * SY, */
    (tx - step * 2) * SX, 645/*(ty + stepy * 2) * SY */
};
float trap2_data[] = {
    (tx - step * 2) * SX -3, 653,/*(ty + stepy * 2) * SY, */
    (unit_size - tx + step * 2) * SX+3, 653,/*(ty + stepy * 2) * SY, */
    (unit_size - tx + step * 3) * SX, (ty + stepy * 3) * SY,
    (tx - step * 3) * SX, (ty + stepy * 3) * SY
};
float track_alert_data[] = {
    (tx - step * 3) * 1, (ty + stepy * 3) * 1,
    (unit_size - tx + step * 3) * 1, (ty + stepy * 3) * 1,
    (unit_size - tx + step * 3) * 1, (ty + stepy * 3 + lwidth) * 1,
    (tx - step * 3) * 1, (ty + stepy * 3 + lwidth) * 1,
};

uint8_t track_blue0_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};

uint8_t track_blue1_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_LINE,
    VLC_OP_END
};

float track_blue0_data[] = {
    (tx - step * 3 - lwidth) * SX, (ty + stepy * 3 + lwidth) * SY,
    (tx - step * 3) * SX, (ty + stepy * 3 + lwidth) * SY,
    (tx) * SX, (ty) * SY,
    (tx - lwidth) * SX, (ty) * SY,
};

float track_blue1_data[] = {
    (unit_size - tx + step * 3) * SX, (ty + stepy * 3 + lwidth) * SY,
    (unit_size - tx + step * 3 + lwidth) * SX, (ty + stepy * 3 + lwidth) * SY,
    (unit_size - tx + lwidth) * SX, (ty) * SY,
    (unit_size - tx) * SX, (ty) * SY,
};

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

uint8_t track_yellow0_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_END
};

uint8_t track_yellow1_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_LINE,
    VLC_OP_CUBIC,
    VLC_OP_END
};

float track_yellow0_data[] = {
    (track0_cp0_x - 6) * SX, (track0_cp0_y + 10) * SY,
    (track0_cp0_x - lwidth - 6) * SX, (track0_cp0_y + 10) * SY,
    (track0_cp1_x - lwidth) * SX, (track0_cp1_y) * SY,
    (track0_cp2_x - lwidth) * SX, (track0_cp2_y) * SY,
    (track0_cp3_x - lwidth) * SX, (track0_cp3_y) * SY,
    (track0_cp3_x) * SX, (track0_cp3_y) * SY,
    (track0_cp2_x) * SX, (track0_cp2_y) * SY,
    (track0_cp1_x) * SX, (track0_cp1_y) * SY,
    (track0_cp0_x - 6) * SX, (track0_cp0_y + 10) * SY,
};

float track_yellow1_data[] = {
    (track1_cp0_x + 6) * SX, (track1_cp0_y + 10) * SY,
    (track1_cp0_x - lwidth + 6) * SX, (track1_cp0_y + 10) * SY,
    (track1_cp1_x - lwidth) * SX, (track1_cp1_y) * SY,
    (track1_cp2_x - lwidth) * SX, (track1_cp2_y) * SY,
    (track1_cp3_x - lwidth) * SX, (track1_cp3_y) * SY,
    (track1_cp3_x) * SX, (track1_cp3_y) * SY,
    (track1_cp2_x) * SX, (track1_cp2_y) * SY,
    (track1_cp1_x) * SX, (track1_cp1_y) * SY,
    (track1_cp0_x + 6) * SX, (track1_cp0_y + 10) * SY,
};

uint32_t color_trap0 = 0x440000ff;
uint32_t color_trap1 = 0x880000ff;
uint32_t color_trap2 = 0xff0000ff;
uint32_t color_track_alert = 0xffff0000;
uint32_t color_track_blue = 0xff0000ff;
uint32_t color_track_yellow = 0xffbbaa00;

/*Images */
vg_lite_buffer_t buf_car, buf_cue, buf_menu;

void cleanup(void)
{
    vg_lite_clear_path(&trap0);
    vg_lite_clear_path(&trap1);
    vg_lite_clear_path(&trap2);
    vg_lite_clear_path(&track_yellow0);
    vg_lite_clear_path(&track_yellow1);
    vg_lite_clear_path(&track_blue0);
    vg_lite_clear_path(&track_blue1);

    if (buf_car.memory != NULL) {
        vg_lite_free(&buf_car);
    }
    if (buf_cue.memory != NULL) {
        vg_lite_free(&buf_cue);
    }
    if (buf_menu.memory != NULL) {
        vg_lite_free(&buf_menu);
    }

    vg_lite_close();

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
void dump_img(void * memory, int width, int height, vg_lite_buffer_format_t format)
{
    FILE * fp1;
    char imgname[300];
    int mm;
    sprintf(imgname, "img_%d_dump.txt", num++);
    fp1 = fopen(imgname, "w");
    if (fp1 == NULL)
        printf("error!\n");
    unsigned int* pt = (unsigned int*) memory;
    switch (format) {
        case VG_LITE_INDEX_1:
            for(mm = 0; mm < width * height / 32; ++mm)
            {
                fprintf(fp1, "0x%08x\n",pt[mm]);
            }
            break;
        case VG_LITE_INDEX_2:
            for(mm = 0; mm < width * height / 16; ++mm)
            {
                fprintf(fp1, "0x%08x\n",pt[mm]);
            }
            break;
        case VG_LITE_INDEX_4:
            for(mm = 0; mm < width * height / 8; ++mm)
            {
                fprintf(fp1, "0x%08x\n",pt[mm]);
            }
            break;
        case VG_LITE_INDEX_8:
            for(mm = 0; mm < width * height / 4; ++mm)
            {
                fprintf(fp1, "0x%08x\n",pt[mm]);
            }
            break;
        default:
            break;
    }
    fclose(fp1);
    fp1 = NULL;
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

static void build_paths()
{
    vg_lite_init_path(&trap0, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap0, poly4_cmd, trap0_data, sizeof(poly4_cmd));

    vg_lite_init_path(&trap1, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap1, poly4_cmd, trap1_data, sizeof(poly4_cmd));

    vg_lite_init_path(&trap2, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&trap2, poly4_cmd, trap2_data, sizeof(poly4_cmd));

    vg_lite_init_path(&track_blue0, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_blue0, track_blue0_cmd, track_blue0_data, sizeof(track_blue0_cmd));

    vg_lite_init_path(&track_blue1, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_blue1, track_blue1_cmd, track_blue1_data, sizeof(track_blue1_cmd));

    vg_lite_init_path(&track_yellow0, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_yellow0, track_yellow0_cmd, track_yellow0_data, sizeof(track_yellow0_cmd));

    vg_lite_init_path(&track_yellow1, VG_LITE_FP32, VG_LITE_HIGH, 0, NULL, 0.0f, 0.0f, 0.0f, 0.0f);
    append_path_data(&track_yellow1, track_yellow1_cmd, track_yellow1_data, sizeof(track_yellow1_cmd));
}

static void build_images()
{
    buf_cue.handle = (void *)0;
    buf_cue.width = img_cue_width;
    buf_cue.height = img_cue_height;
    buf_cue.format = (vg_lite_buffer_format_t)img_cue_format;
    vg_lite_allocate(&buf_cue);
    byte_copy(buf_cue.memory, img_cue_data, img_cue_stride * img_cue_height);
    buf_cue.height--;
    vg_lite_set_CLUT(2, clut_cue);
    dump_img(buf_cue.memory, img_cue_width, img_cue_height, img_cue_format);

    buf_menu.handle = buf_cue.handle;
    buf_menu.width = img_menu_width;
    buf_menu.height = img_menu_height;
    buf_menu.format = (vg_lite_buffer_format_t)img_menu_format;
    vg_lite_allocate(&buf_menu);
    byte_copy(buf_menu.memory, img_menu_data, img_menu_stride * img_menu_height);
    buf_menu.height--;
    vg_lite_set_CLUT(4, clut_menu);
    dump_img(buf_menu.memory, img_menu_width, img_menu_height, img_menu_format);

    buf_car.handle = buf_menu.handle;
    buf_car.width = img_car_width;
    buf_car.height = img_car_height;
    buf_car.format = (vg_lite_buffer_format_t)img_car_format;
    vg_lite_allocate(&buf_car);
    byte_copy(buf_car.memory, img_car_data, img_car_stride * img_car_height);
    buf_car.height--;
    vg_lite_set_CLUT(256, clut_car);
    dump_img(buf_car.memory, img_car_width, img_car_height, img_car_format);
}

int main(int argc, const char * argv[])
{
    vg_lite_error_t error;

    /* Initialize the blitter. */
#if DDRLESS
    error = vg_lite_init(render_width, render_height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
#endif

#if DDRLESS_FPGA
    fb = NULL;
#else
    /* Allocate the off-screen buffer. */
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
#endif

#if DDRLESS_CMODEL
/*    vg_lite_set_compose_buffer(fb); */
#endif

    printf("Render size: %d x %d\n", buffer.width, buffer.height);

    build_paths();
    build_images();

#if !DDRLESS_FPGA
    vg_lite_clear(fb, NULL, 0xffaabbcc);
#endif

    vg_lite_matrix_t mat;
    vg_lite_identity(&mat);
    /* vg_lite_scale(render_width / 100.0f, render_height / 100.0f, &mat); */

    rect_alert.x = track_alert_data[0] * render_width / 100.0f;;
    rect_alert.y = track_alert_data[1] * render_height / 100.0f;
    rect_alert.width = (track_alert_data[2] - track_alert_data[0]) * render_width / 100.0f;;
    rect_alert.height = (track_alert_data[5] - track_alert_data[1]) * render_height / 100.0f;

    rect_trap0.x = 630;
    rect_trap0.y = 430;
    rect_trap0.width = 660;
    rect_trap0.height = 8;

    rect_trap1.x = 400;
    rect_trap1.y = 645;
    rect_trap1.width = 1120;
    rect_trap1.height = 8;

    vg_lite_draw(fb, &trap0, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap0);
    vg_lite_clear(fb, &rect_trap0, color_trap0);
    vg_lite_draw(fb, &trap1, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap1);
    vg_lite_clear(fb, &rect_trap1, color_trap2);
    vg_lite_draw(fb, &trap2, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_trap2);
    vg_lite_clear(fb, &rect_alert, color_track_alert);
    vg_lite_draw(fb, &track_blue0, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_blue);
    vg_lite_draw(fb, &track_blue1, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_blue);
    vg_lite_draw(fb, &track_yellow0, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_yellow);
    vg_lite_draw(fb, &track_yellow1, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_SRC_OVER, color_track_yellow);

    vg_lite_identity(&mat);
    vg_lite_translate(55.0f, 80.0f, &mat);
    vg_lite_scale(2.0f, 2.0f, &mat);
    vg_lite_blit(fb, &buf_car, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

    vg_lite_identity(&mat);
    vg_lite_translate(render_width - buf_cue.width - 50, 80.0f, &mat);
    vg_lite_blit(fb, &buf_cue, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

    vg_lite_identity(&mat);
    vg_lite_translate(0.0f, render_height - buf_menu.height, &mat);
    vg_lite_blit(fb, &buf_menu, &mat, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

    vg_lite_finish();
#if !DDRLESS_FPGA
    /* Save PNG file. */
    vg_lite_save_png("r6_03_dl.png", fb);
#endif

    /* Cleanup. */
    cleanup();
    return 0;
}
