
#include <stdio.h>
#include "vg_lite.h"

#include <stdio.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#define DDRLESS 0
#include "car_index8.img.h"
#include "note_index2.img.h"
#include "3m_index1.img.h"
#include "5m_index1.img.h"
#include "7m_index1.img.h"

const int32_t countd = 18;      /* 18 dashes per curve */
const int32_t countc = 1;       /* 1 dashed curves */

#if 1
static int num = 1;
#endif

#define DEFAULT_WIDTH 1920
#define DEFAULT_HEIGHT 1080

/* Size of fb that render into. */
static int   render_width = DEFAULT_WIDTH, render_height = DEFAULT_HEIGHT;
static float scaleX = 1.0f, scaleY = 1.0f;

static vg_lite_buffer_t buffer;     /*offscreen framebuffer object for rendering. */
static vg_lite_buffer_t * fb;

#define CURVE_COUNT 180
#define FRAME_COUNT 360

/*Paths */
vg_lite_path_t curves0[CURVE_COUNT], curves1[CURVE_COUNT], track0, track1, trap0, trap1, trap2, trap3, hcurves, hline;
/*Images */
vg_lite_buffer_t buf_3m, buf_5m, buf_7m, buf_car, buf_note;
uint32_t clut1[2], clut2[4], clut8[256];
/*Matrices */
vg_lite_matrix_t mat_curve, mat_curve_sh, mat_track, mat_trap0,  mat_trap1, mat_trap2, mat_trap3, mat_hcurve, mat_hline;
vg_lite_matrix_t mat_hcurve2, mat_3m, mat_5m, mat_7m, mat_car, mat_note;

void cleanup(void)
{
    int i;
    if (buffer.handle != NULL) {
        /* Free the offscreen framebuffer memory. */
        vg_lite_free(&buffer);
    }

    for (i = 0; i < CURVE_COUNT;i ++) {
        if (curves0[i].path != NULL) {
            free(curves0[i].path);
        }
        if (curves1[i].path != NULL) {
            free(curves1[i].path);
        }
    }
    if (trap0.path != NULL) {
        free (trap0.path);
    }
    if (trap1.path != NULL) {
        free (trap1.path);
    }
    if (trap2.path != NULL) {
        free (trap2.path);
    }
    if (trap3.path != NULL) {
        free (trap3.path);
    }
    if (track0.path != NULL) {
        free (track0.path);
    }
    if (track1.path != NULL) {
        free (track1.path);
    }
    if (hcurves.path != NULL) {
        free (hcurves.path);
    }
    if (hline.path != NULL) {
        free (hline.path);
    }
    if (buf_car.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&buf_car);
    }
    if (buf_note.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&buf_note);
    }
    if (buf_3m.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&buf_3m);
    }
    if (buf_5m.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&buf_5m);
    }
    if (buf_7m.handle != NULL) {
        /* Free the image memory. */
        vg_lite_free(&buf_7m);
    }

    vg_lite_close();
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

static void build_dashes(float *data, int32_t *count, float sx, float sy, float ratio, int n)
{
    static float *p;
    static float s0[4][2] = {{0.0f, 0.0f}, {0.7f, 1.0f}, {0.5f, 2.0f}, {-0.5f, 3.0f}};
    static float t0[4][2] = {{0.0f, 0.0f}, {0.5f, 1.0f}, {1.2f, 2.0f}, {3.5f, 3.0f}};
    static float s1[4][2] = {{4.0f, 0.0f}, {3.5f, 1.0f}, {2.8f, 2.0f}, {0.5f, 3.0f}};
    static float t1[4][2] = {{4.0f, 0.0f}, {3.3f, 1.0f}, {3.5f, 2.0f}, {4.5f, 3.0f}};

    float c0[4][2];
    float c1[4][2];
    const float seg_step = 1.0f / countd;
    float thickness= 0.05f;
    int32_t i;
    float ex, ey;
    float t;

    if (count != NULL)
    {
        *count = 2 * 4 * countd * countc;
    }

    if (data == NULL)
    {
        return;
    }

    for (i = 0; i < 4; i++) {
        c0[i][0] = s0[i][0] + (t0[i][0] - s0[i][0])  * ratio;
        c0[i][1] = s0[i][1] + (t0[i][1] - s0[i][1])  * ratio;
        c1[i][0] = s1[i][0] + (t1[i][0] - s1[i][0])  * ratio;
        c1[i][1] = s1[i][1] + (t1[i][1] - s1[i][1])  * ratio;
    }

    i = 0;
    p = data;
    thickness *= sx;

    if (0 == n) {
        for (i = 0; i < countd; i++)
        {
            t = seg_step * i;
            eval_quad(c0[0][0], c0[0][1], c0[1][0], c0[1][1], c0[2][0], c0[2][1], c0[3][0], c0[3][1], t, &ex, &ey);
            ex *= sx;
            ey *= sy;
            *p++ = ex - thickness;
            *p++ = ey;
            *p++ = ex;
            *p++ = ey;
            t = seg_step * (i + 1) - (seg_step * 0.1);
            eval_quad(c0[0][0], c0[0][1], c0[1][0], c0[1][1], c0[2][0], c0[2][1], c0[3][0], c0[3][1], t, &ex, &ey);
            ex *= sx;
            ey *= sy;
            *p++ = ex;
            *p++ = ey;
            *p++ = ex - thickness;
            *p++ = ey;
        }
    } else {
        for (i = 0; i < countd; i++)
        {
            t = seg_step * i;
            eval_quad(c1[0][0], c1[0][1], c1[1][0], c1[1][1], c1[2][0], c1[2][1], c1[3][0], c1[3][1], t, &ex, &ey);

            ex *= sx;
            ey *= sy;
            *p++ = ex - thickness;
            *p++ = ey;
            *p++ = ex;
            *p++ = ey;
            t = seg_step * (i + 1) - (seg_step * 0.1);
            eval_quad(c1[0][0], c1[0][1], c1[1][0], c1[1][1], c1[2][0], c1[2][1], c1[3][0], c1[3][1], t, &ex, &ey);
            ex *= sx;
            ey *= sy;
            *p++ = ex;
            *p++ = ey;
            *p++ = ex - thickness;
            *p++ = ey;
        }
    }
    /* Finish up the last dash. */
}

static void build_dash_path(vg_lite_path_t *path, float ratio, int n)
{
    int32_t data_size;
    uint8_t *p8;
    float   *pf;
    int32_t i;
    float   *data;
    int32_t data_count;

    build_dashes(NULL, &data_count, 1.0f, 1.0f, 0, n);
    data = (float*) malloc(data_count * 4);
    build_dashes(data, &data_count, 1.0f, 1.0f, ratio, n);

    /* coordinates + 3 line_to per dash + (close + move_to) between per 2 dash + */
    /* first move_to + last close + last end. */
    data_size = 8 * 4 * countd * countc + 4 * 3 * countd * countc +
    4 * (countd * countc - 1) + 4 + 2;

    /*data_size = 50; */
    vg_lite_init_path(path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);
    path->path = malloc(data_size);

    p8 = (uint8_t*) path->path;
    pf = (float *) path->path;
    for (i = 0; i < countd * countc; i++)
    {
        *p8 = VLC_OP_MOVE;
        pf++;

        *pf++ = *data++;
        *pf++ = *data++;

        p8 = (uint8_t*) pf;
        *p8 = VLC_OP_LINE;
        pf++;

        *pf++ = *data++;
        *pf++ = *data++;

        p8 = (uint8_t*) pf;
        *p8 = VLC_OP_LINE;
        pf++;

        *pf++ = *data++;
        *pf++ = *data++;

        p8 = (uint8_t*) pf;
        *p8 = VLC_OP_LINE;
        pf++;

        *pf++ = *data++;
        *pf++ = *data++;

        p8 = (uint8_t*) pf;
        *p8++ = VLC_OP_CLOSE;
    }

    *p8 = VLC_OP_END;
}

static void eval_line(float x0, float y0, float x1, float y1, float t, float *outx, float *outy)
{
    *outx = x0 + (x1 - x0) * t;
    *outy = y0 + (y1 - y0) * t;
}

static void build_track(int n)
{
    uint32_t data_size;
    uint8_t  *p8;
    float    *pf;
    float l0[2][2] = {{0.0f, 0.0f}, {40.0f, 60.0f}};
    float l1[2][2] = {{120.0f, 0.0f}, {80.0f, 60.0f}};
    float t[] = {0.25f, 0.5f, 0.8f};
    float mark_width = 2.0f;
    float mark_height = 1.5f / 70.0f;
    float x, y;

    data_size = 17 * 8 + 17 * 4 + 1;
    if (0 == n) {
        vg_lite_init_path(&track0, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);
        track0.path = malloc(data_size);

        p8 = (uint8_t *)track0.path;
        pf = (float   *)track0.path;

        /*Left track. */
        *p8 = VLC_OP_MOVE;
        pf++;
        *pf++ = l0[0][0];
        *pf++ = l0[0][1];

        p8 = (uint8_t *)pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[0], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *)pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[0] + mark_height, &x, &y);
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[1], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[1] + mark_height, &x, &y);
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[2], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l0[0][0], l0[0][1], l0[1][0], l0[1][1], t[2] + mark_height, &x, &y);
        *pf++ = x + mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l0[1][0];
        *pf++ = l0[1][1];

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l0[1][0] - mark_width;
        *pf++ = l0[1][1];

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l0[0][0] - mark_width;
        *pf++ = l0[0][1];

        p8 = (uint8_t *) pf;
        *p8++ = VLC_OP_END;
    } else {
        vg_lite_init_path(&track1, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);
        track1.path = malloc(data_size);

        p8 = (uint8_t *)track1.path;
        pf = (float   *)track1.path;
        /*Right track. */
        *p8 = VLC_OP_MOVE;
        pf++;
        *pf++ = l1[0][0];
        *pf++ = l1[0][1];

        p8 = (uint8_t *)pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[0], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *)pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[0] + mark_height, &x, &y);
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[1], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[1] + mark_height, &x, &y);
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[2], &x, &y);
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        eval_line(l1[0][0], l1[0][1], l1[1][0], l1[1][1], t[2] + mark_height, &x, &y);
        *pf++ = x - mark_width;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = x;
        *pf++ = y;

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l1[1][0];
        *pf++ = l1[1][1];

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l1[1][0] + mark_width;
        *pf++ = l1[1][1];

        p8 = (uint8_t *) pf;
        *p8 = VLC_OP_LINE;
        pf++;
        *pf++ = l1[0][0] + mark_width;
        *pf++ = l1[0][1];

        p8 = (uint8_t *) pf;
        *p8++ = VLC_OP_END;
    }
}

static void build_trapezoid()
{
    uint32_t data_size;
    uint8_t *p8;
    float   *pf;
    float   pt0[4][2] = {{0.0f, 0.0f},
        {9.8f, 25.0f},
        {90.2f, 25.0f},
        {100.0f, 0.0f}};
    float   pt1[4][2] = {{0.0f, 0.0f},
        {10.0f, 25.0f},
        {90.0f, 25.0f},
        {100.0f, 0.0f}};

    float   pt2[4][2] = {{0.0f, 0.0f},
        {14.0f, 25.0f},
        {86.0f, 25.0f},
        {100.0f, 0.0f}};

    float   pt3[4][2] = {{0.0f, 0.0f},
        {13.3f, 25.0f},
        {86.7f, 25.0f},
        {100.0f, 0.0f}};

    data_size = 12 * 4 + 1;
    vg_lite_init_path(&trap0, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    trap0.path = malloc(data_size);

    p8 = (uint8_t *)trap0.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt0[0][0];
    *pf++ = pt0[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt0[1][0];
    *pf++ = pt0[1][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt0[2][0];
    *pf++ = pt0[2][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt0[3][0];
    *pf++ = pt0[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;
    vg_lite_init_path(&trap1, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    trap1.path = malloc(data_size);

    p8 = (uint8_t *)trap1.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt1[0][0];
    *pf++ = pt1[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt1[1][0];
    *pf++ = pt1[1][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt1[2][0];
    *pf++ = pt1[2][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt1[3][0];
    *pf++ = pt1[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;

    vg_lite_init_path(&trap2, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    trap2.path = malloc(data_size);

    p8 = (uint8_t *)trap2.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt2[0][0];
    *pf++ = pt2[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt2[1][0];
    *pf++ = pt2[1][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt2[2][0];
    *pf++ = pt2[2][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt2[3][0];
    *pf++ = pt2[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;

    vg_lite_init_path(&trap3, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    trap3.path = malloc(data_size);

    p8 = (uint8_t *)trap3.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt3[0][0];
    *pf++ = pt3[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt3[1][0];
    *pf++ = pt3[1][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt3[2][0];
    *pf++ = pt3[2][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt3[3][0];
    *pf++ = pt3[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;
}

static void build_hcurve()
{
    uint32_t data_size;
    uint8_t *p8;
    float   *pf;
    float   pt0[8][2] = {{0.0f, 0.0f},
        {30.0f, 10.0f},
        {70.0f, 10.0f},
        {100.0f, 0.0f},
        {101.0f, 2.0f},
        {71.0f, 12.0f},
        {29.0f, 12.0f},
        {-1.0f, 2.0f},
    };

    data_size = 4 + 8 + 4 + 8 * 3 + 4 + 8 + 4 + 8 * 3 + 1;
    vg_lite_init_path(&hcurves, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    hcurves.path = malloc(data_size);

    p8 = (uint8_t *)hcurves.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt0[0][0];
    *pf++ = pt0[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_CUBIC;
    pf++;
    *pf++ = pt0[1][0];
    *pf++ = pt0[1][1];
    *pf++ = pt0[2][0];
    *pf++ = pt0[2][1];
    *pf++ = pt0[3][0];
    *pf++ = pt0[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt0[4][0];
    *pf++ = pt0[4][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_CUBIC;
    pf++;
    *pf++ = pt0[5][0];
    *pf++ = pt0[5][1];
    *pf++ = pt0[6][0];
    *pf++ = pt0[6][1];
    *pf++ = pt0[7][0];
    *pf++ = pt0[7][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;
}

static void build_hline()
{
    uint32_t data_size;
    uint8_t *p8;
    float   *pf;
    float   pt[][2] = {{0.0f, 0.0f},
        {100.0f, 0.0f},
        {100.0f, 5.0f},
        {0.0f, 5.0f},
    };

    data_size = 12 * 4 + 1;
    vg_lite_init_path(&hline, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, render_width, render_height);

    hline.path = malloc(data_size);

    p8 = (uint8_t *)hline.path;
    pf = (float   *)p8;

    *p8 = VLC_OP_MOVE;
    pf++;
    *pf++ = pt[0][0];
    *pf++ = pt[0][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt[1][0];
    *pf++ = pt[1][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt[2][0];
    *pf++ = pt[2][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_LINE;
    pf++;
    *pf++ = pt[3][0];
    *pf++ = pt[3][1];
    p8 = (uint8_t *) pf;

    *p8 = VLC_OP_END;
}

static void build_paths()
{
    int i;
    for (i = 0;  i < CURVE_COUNT; i++) {
        build_dash_path(&curves0[i], ((float)i / (CURVE_COUNT - 1)), 0);
    }
    for (i = 0;  i < CURVE_COUNT; i++) {
        build_dash_path(&curves1[i], ((float)i / (CURVE_COUNT - 1)), 1);
    }

    build_track(0);
    build_track(1);
    build_trapezoid();
    build_hcurve();
    build_hline();

    vg_lite_identity(&mat_curve);
    vg_lite_scale(scaleX, scaleY, &mat_curve);
    vg_lite_translate(560.0f, 1080.0f, &mat_curve);
    vg_lite_scale(250.0f, -250.0f, &mat_curve);

    vg_lite_identity(&mat_curve_sh);
    vg_lite_scale(scaleX, scaleY, &mat_curve_sh);
    vg_lite_translate(520.0f, 1090.0f, &mat_curve_sh);
    vg_lite_scale(250.0f, -250.0f, &mat_curve_sh);

    vg_lite_identity(&mat_track);
    vg_lite_scale(scaleX, scaleY, &mat_track);
    vg_lite_translate(550.0f, 1090.0f, &mat_track);
    vg_lite_scale(8.0f, -12.0f, &mat_track);

    vg_lite_identity(&mat_trap0);
    vg_lite_scale(scaleX, scaleY, &mat_trap0);
    vg_lite_translate(530.0f, 1090.0f, &mat_trap0);
    vg_lite_scale(9.9f, -8.2f, &mat_trap0);

    vg_lite_identity(&mat_trap1);
    vg_lite_scale(scaleX, scaleY, &mat_trap1);
    vg_lite_translate(625.0f, 900.0f, &mat_trap1);
    vg_lite_scale(8.1f, -7.2f, &mat_trap1);

    vg_lite_identity(&mat_trap2);
    vg_lite_scale(scaleX, scaleY, &mat_trap2);
    vg_lite_translate(710.0f, 720.0f, &mat_trap2);
    vg_lite_scale(6.4f, -8.3f, &mat_trap2);

    vg_lite_identity(&mat_trap3);
    vg_lite_scale(scaleX, scaleY, &mat_trap3);
    vg_lite_translate(795.0f, 520.0f, &mat_trap3);
    vg_lite_scale(4.7f, -6.0f, &mat_trap3);

    vg_lite_identity(&mat_hcurve);
    vg_lite_scale(scaleX, scaleY, &mat_hcurve);
    vg_lite_translate(680.0f, 410.0f, &mat_hcurve);
    vg_lite_scale(7.0f, -6.0f, &mat_hcurve);

    vg_lite_identity(&mat_hcurve2);
    vg_lite_scale(scaleX, scaleY, &mat_hcurve2);
    vg_lite_translate(425.0f, 800.0f, &mat_hcurve2);
    vg_lite_scale(12.3f, 6.0f, &mat_hcurve2);

    vg_lite_identity(&mat_hline);
    vg_lite_scale(scaleX, scaleY, &mat_hline);
    vg_lite_translate(780.0f, 560.0f, &mat_hline);
    vg_lite_scale(5.0f, 2.0f, &mat_hline);

    vg_lite_identity(&mat_3m);
    vg_lite_scale(scaleX, scaleY, &mat_3m);
    vg_lite_translate(1445.0f, 850.0f, &mat_3m);
    vg_lite_scale(1.5f, 1.5f, &mat_3m);

    vg_lite_identity(&mat_5m);
    vg_lite_scale(scaleX, scaleY, &mat_5m);
    vg_lite_translate(1360.0f, 670.0f, &mat_5m);
    vg_lite_scale(1.5f, 1.5f, &mat_5m);

    vg_lite_identity(&mat_7m);
    vg_lite_scale(scaleX, scaleY, &mat_7m);
    vg_lite_translate(1270.0f, 450.0f, &mat_7m);
    vg_lite_scale(1.5f, 1.5f, &mat_7m);

    vg_lite_identity(&mat_car);
    vg_lite_scale(scaleX, scaleY, &mat_car);
    vg_lite_translate(100.0f, 100.0f, &mat_car);
    vg_lite_scale(1.5f, 1.5f, &mat_car);

    vg_lite_identity(&mat_note);
    vg_lite_scale(scaleX, scaleY, &mat_note);
    vg_lite_translate(250.0f, 960.0f, &mat_note);
    vg_lite_scale(1.5f, 1.5f, &mat_note);
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

static void build_images()
{
    buf_3m.handle = (void *)0;
    buf_3m.width = buf_5m.width = buf_7m.width = img_3m_width;
    buf_3m.height = buf_5m.height = buf_7m.height = img_3m_height;
    buf_3m.format = buf_5m.format = buf_7m.format = (vg_lite_buffer_format_t)img_3m_format;
    vg_lite_allocate(&buf_3m);
    byte_copy(buf_3m.memory, img_3m_data, img_3m_stride * img_3m_height);
    buf_3m.height--;
    dump_img(buf_3m.memory, img_3m_width, img_3m_height, img_3m_format);

    buf_5m.handle = buf_3m.handle;
    vg_lite_allocate(&buf_5m);
    byte_copy(buf_5m.memory, img_5m_data, img_3m_stride * img_3m_height);
    dump_img(buf_5m.memory, img_3m_width, img_3m_height, img_3m_format);

    buf_7m.handle = buf_5m.handle;
    vg_lite_allocate(&buf_7m);
    byte_copy(buf_7m.memory, img_7m_data, img_3m_stride * img_3m_height);
    vg_lite_set_CLUT(2, clut_3m);
    dump_img(buf_7m.memory, img_3m_width, img_3m_height, img_3m_format);

    buf_car.handle = buf_7m.handle;
    buf_car.width = img_car_width;
    buf_car.height = img_car_height;
    buf_car.format = (vg_lite_buffer_format_t)img_car_format;
    buf_car.stride = img_car_stride;
    vg_lite_allocate(&buf_car);
    byte_copy(buf_car.memory, img_car_data, buf_car.stride * buf_car.height);
    buf_car.height--;
    vg_lite_set_CLUT(256, clut_car);
    dump_img(buf_car.memory, img_car_width, img_car_height, img_car_format);

    buf_note.handle = buf_car.handle;
    buf_note.width = img_note_width;
    buf_note.height = img_note_height;
    buf_note.format = (vg_lite_buffer_format_t)img_note_format;
    vg_lite_allocate(&buf_note);
    byte_copy(buf_note.memory, img_note_data, buf_note.stride * buf_note.height);
    vg_lite_set_CLUT(4, clut_note);
    dump_img(buf_note.memory, img_note_width, img_note_height, img_note_format);

    return;
}

int main(int argc, const char * argv[])
{
    int32_t i;
    char name[255] = {'\0'};
    int32_t start_frame, frames;
    int32_t fixed_frame = 0, path_id = 0;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    if (argc < 4) {
        printf("Usage: renesas_perf_607 <start_frame> <frames> <fixed frame:0/1> <screen width> <screen height>\n");
        printf("Example: renesas_perf_607 0 60 0\n");
        start_frame = 0;
        frames = FRAME_COUNT;
        fixed_frame = 0;
    }
    else{
        start_frame = atoi(argv[1]);
        frames = atoi(argv[2]);
        fixed_frame = atoi(argv[3]);
        if ((start_frame < 0) || (frames <= 0))
            return -1;

        if (argc >= 6){
            render_width = atoi(argv[4]);
            render_height = atoi(argv[5]);
            scaleX = (float)render_width / DEFAULT_WIDTH;
            scaleY = (float)render_height / DEFAULT_HEIGHT;
        }
    }
    printf("Render %d frames from %d with fixed frame: %d.\n", frames, start_frame, fixed_frame);

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

    printf("Render size: %d x %d\n", render_width, render_height);

    build_paths();
    build_images();

    for (i = start_frame; i < start_frame + frames; i++) {
#if !DDRLESS_FPGA
        vg_lite_clear(fb, NULL, 0xffaabbcc);
#endif
        if (fixed_frame == 0)
            path_id = i;
        else
            path_id = 0;

        if (path_id >= CURVE_COUNT * 2) {
            path_id %= CURVE_COUNT * 2;
        }

        if (path_id >= CURVE_COUNT){
            path_id = CURVE_COUNT * 2 - path_id - 1;
        }

        vg_lite_draw(fb, &curves0[path_id], VG_LITE_FILL_EVEN_ODD, &mat_curve, VG_LITE_BLEND_NONE, 0xff000000);
        vg_lite_draw(fb, &curves0[path_id], VG_LITE_FILL_EVEN_ODD, &mat_curve_sh, VG_LITE_BLEND_NONE, 0xff0000bb);
        vg_lite_draw(fb, &curves1[path_id], VG_LITE_FILL_EVEN_ODD, &mat_curve, VG_LITE_BLEND_NONE, 0xff000000);
        vg_lite_draw(fb, &curves1[path_id], VG_LITE_FILL_EVEN_ODD, &mat_curve_sh, VG_LITE_BLEND_NONE, 0xff0000bb);

        vg_lite_draw(fb, &track0, VG_LITE_FILL_EVEN_ODD, &mat_track, VG_LITE_BLEND_SRC_OVER, 0xaaaabb22);
        vg_lite_draw(fb, &track1, VG_LITE_FILL_EVEN_ODD, &mat_track, VG_LITE_BLEND_SRC_OVER, 0xaaaabb22);

        vg_lite_draw(fb, &trap0, VG_LITE_FILL_EVEN_ODD, &mat_trap0, VG_LITE_BLEND_SRC_OVER, 0xbb7788ff);

        vg_lite_draw(fb, &trap1, VG_LITE_FILL_EVEN_ODD, &mat_trap1, VG_LITE_BLEND_SRC_OVER, 0x997788ff);

        vg_lite_draw(fb, &trap2, VG_LITE_FILL_EVEN_ODD, &mat_trap2, VG_LITE_BLEND_SRC_OVER, 0x775566cc);

        vg_lite_draw(fb, &trap3, VG_LITE_FILL_EVEN_ODD, &mat_trap3, VG_LITE_BLEND_SRC_OVER, 0x55556699);

        vg_lite_draw(fb, &hcurves, VG_LITE_FILL_EVEN_ODD, &mat_hcurve, VG_LITE_BLEND_SRC_OVER, 0xbbbb6644);

        vg_lite_draw(fb, &hcurves, VG_LITE_FILL_EVEN_ODD, &mat_hcurve2, VG_LITE_BLEND_SRC_OVER, 0xbbbb6644);

        vg_lite_draw(fb, &hline, VG_LITE_FILL_EVEN_ODD, &mat_hline, VG_LITE_BLEND_SRC_OVER, 0xaa44aa44);

        vg_lite_blit(fb, &buf_3m, &mat_3m, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);
        vg_lite_blit(fb, &buf_5m, &mat_5m, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);
        vg_lite_blit(fb, &buf_7m, &mat_7m, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

        vg_lite_blit(fb, &buf_car, &mat_car, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

        vg_lite_blit(fb, &buf_note, &mat_note, VG_LITE_BLEND_SRC_OVER, 0x0, VG_LITE_FILTER_POINT);

        vg_lite_finish();
        /*printf("Frame %d command buffer data ready...\n", i); */
        printf("Frame %d done...\n", i);
#if !DDRLESS_FPGA
        /* Save PNG file. */
        sprintf(name, "renesas_perf_607_%d.png", i);
        vg_lite_save_png(name, fb);
#endif
    }

#if 0
    for (i = start_frame; i < start_frame + frames; i++) {
        printf("Rendering frame %d.\n", i);
        vg_lite_kick_off_command();
    }
#endif
    /* Cleanup. */
    cleanup();
    return 0;
}
