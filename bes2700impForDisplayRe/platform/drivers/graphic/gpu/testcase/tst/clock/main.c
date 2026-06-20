#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

#include "vg_lite.h"
#include "vg_lite_util.h"

static int   fb_width = 320, fb_height = 480;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

/*
 *-----*
 /       \
 /         \
 *           *
 |          /
 |         X
 |          \
 *           *
 \         /
 \       /
 *-----*
 */
#define CLOCK_RADIUS    10.0f
#define SEC_HAND_LEN    10.0f
#define MIN_HAND_LEN    8
#define HOU_HAND_LEN    6
#define SEC_HAND_WIDTH  2
#define MIN_HAND_WIDTH  3
#define HOU_HAND_WIDTH  4
#define BUTTON_WIDTH    6
#define BUTTON_HEIGHT   3

#define   OPCODE_END                                               0x00
#define   OPCODE_CLOSE                                             0x01
#define   OPCODE_MOVE                                              0x02
#define   OPCODE_MOVE_REL                                          0x03
#define   OPCODE_LINE                                              0x04
#define   OPCODE_LINE_REL                                          0x05
#define   OPCODE_QUADRATIC                                         0x06
#define   OPCODE_QUADRATIC_REL                                     0x07
#define   OPCODE_CUBIC                                             0x08
#define   OPCODE_CUBIC_REL                                         0x09
#define   OPCODE_BREAK                                             0x0A
#define   OPCODE_HLINE                                             0x0B
#define   OPCODE_HLINE_REL                                         0x0C
#define   OPCODE_VLINE                                             0x0D
#define   OPCODE_VLINE_REL                                         0x0E
#define   OPCODE_SQUAD                                             0x0F
#define   OPCODE_SQUAD_REL                                         0x10
#define   OPCODE_SCUBIC                                            0x11
#define   OPCODE_SCUBIC_REL                                        0x12

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
static vg_lite_path_t path_circle;
static vg_lite_path_t path_hand;
static vg_lite_path_t path_button;

static vg_lite_matrix_t matrix_tick[12];
static vg_lite_matrix_t matrix_hour;
static vg_lite_matrix_t matrix_min;
static vg_lite_matrix_t matrix_sec;
static vg_lite_matrix_t matrix_face[2];
static vg_lite_matrix_t matrix_button[2];
static vg_lite_matrix_t matrix_cap;

void build_paths()
{
    char    *pchar;
    float   *pfloat;
    int32_t data_size;

    // Path circle
    /* MOV_TO
       CUBIC_TO
       CUBIC_TO
       CUBIC_TO
       CUBIC_TO
     */
    data_size = 4 * 5 + 1 + 8 + 8 * 3 * 4;

    path_circle.path = malloc(data_size);
    vg_lite_init_path(&path_circle, VG_LITE_FP32, VG_LITE_HIGH, data_size, path_circle.path, -CLOCK_RADIUS, -CLOCK_RADIUS, CLOCK_RADIUS, CLOCK_RADIUS);

    pchar = (char*)path_circle.path;
    pfloat = (float*)path_circle.path;
    *pchar = OPCODE_MOVE;
    pfloat++;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_CUBIC;
    pfloat++;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = CLOCK_RADIUS * 0.6f;
    *pfloat++ = CLOCK_RADIUS * 0.6f;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = 0.0f;
    *pfloat++ = CLOCK_RADIUS;

    pchar = (char*)pfloat;
    *pchar = OPCODE_CUBIC;
    pfloat++;
    *pfloat++ = -CLOCK_RADIUS * 0.6f;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = -CLOCK_RADIUS;
    *pfloat++ = CLOCK_RADIUS * 0.6f;
    *pfloat++ = -CLOCK_RADIUS;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_CUBIC;
    pfloat++;
    *pfloat++ = -CLOCK_RADIUS;
    *pfloat++ = -CLOCK_RADIUS * 0.6f;
    *pfloat++ = -CLOCK_RADIUS * 0.6f;
    *pfloat++ = -CLOCK_RADIUS;
    *pfloat++ = 0.0f;
    *pfloat++ = -CLOCK_RADIUS;

    pchar = (char*)pfloat;
    *pchar = OPCODE_CUBIC;
    pfloat++;
    *pfloat++ = CLOCK_RADIUS * 0.6f;
    *pfloat++ = -CLOCK_RADIUS;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = -CLOCK_RADIUS * 0.6f;
    *pfloat++ = CLOCK_RADIUS;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_END;

    // Path hands.
    /*
     MOV_TO
     VLINE
     HLINE
     VLINE
     CLOSE
     */
    data_size = 4 * 4 + 1 + 4 * 2 * 4;//+ 4 * 3;

    path_hand.path = malloc(data_size);
    vg_lite_init_path(&path_hand, VG_LITE_FP32, VG_LITE_HIGH, data_size, path_hand.path, 0.0f, 0.0f, 1.0f, 1.0f);

    pchar = (char*)path_hand.path;
    pfloat = (float*)path_hand.path;
    *pchar = OPCODE_MOVE;
    pfloat++;
    *pfloat++ = 0.0f;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = 1.0f;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = 1.0f;
    *pfloat++ = 1.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = 0.0f;
    *pfloat++ = 1.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_END;

    // Path button.
    /*
     MOV_TO
     LINE
     QUAD
     LINE
     QUAD
     LINE
     QUAD
     LINE
     QUAD
     CLOSE
     */
    data_size = 4 * 9 + 1 + 4 * 2 * 5 + 4 * 4 * 4;//+ 4 * 3;

    path_button.path = malloc(data_size);
    vg_lite_init_path(&path_button, VG_LITE_FP32, VG_LITE_HIGH, data_size, path_button.path, 0.0f, 0.0f, BUTTON_WIDTH, BUTTON_HEIGHT);

    pchar = (char*)path_button.path;
    pfloat = (float*)path_button.path;
    *pchar = OPCODE_MOVE;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH / 8.0f;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH * 7.0f / 8.0f;
    *pfloat++ = 0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_QUADRATIC;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH;
    *pfloat++ = 0.0f;
    *pfloat++ = BUTTON_WIDTH;
    *pfloat++ = BUTTON_HEIGHT / 4.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH;
    *pfloat++ = BUTTON_HEIGHT * 3.0f / 4.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_QUADRATIC;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH;
    *pfloat++ = BUTTON_HEIGHT;
    *pfloat++ = BUTTON_WIDTH * 7.0f / 8.0f;
    *pfloat++ = BUTTON_HEIGHT;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = BUTTON_WIDTH / 8.0f;
    *pfloat++ = BUTTON_HEIGHT;

    pchar = (char*)pfloat;
    *pchar = OPCODE_QUADRATIC;
    pfloat++;
    *pfloat++ = 0.0f;
    *pfloat++ = BUTTON_HEIGHT;
    *pfloat++ = 0.0f;
    *pfloat++ = BUTTON_HEIGHT * 3.0f / 4.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_LINE;
    pfloat++;
    *pfloat++ = 0.0f;
    *pfloat++ = BUTTON_HEIGHT / 4.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_QUADRATIC;
    pfloat++;
    *pfloat++ = 0.0f;
    *pfloat++ = 0.0f;
    *pfloat++ = BUTTON_WIDTH / 8.0f;
    *pfloat++ =0.0f;

    pchar = (char*)pfloat;
    *pchar = OPCODE_END;
}

void build_matrices()
{
    int32_t i;
    float angle;
    float x, y;

    /* Precalculate the matrices for ticks on the clock face. */
    for (i = 0; i < 12; i += 3 )
    {
        angle = i * 3.14159265f / 6;
        x = 80.0f * cos(angle);
        y = 80.0f * sin(angle);
        vg_lite_identity(&matrix_tick[i]);
        vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_tick[i]);
        vg_lite_translate(x, y, &matrix_tick[i]);
        vg_lite_rotate(i * 30, &matrix_tick[i]);
        vg_lite_scale(2.0f, 1.0f, &matrix_tick[i]);
    }

    x = -0.1f;
    y = -0.5f;
    vg_lite_identity(&matrix_hour);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_hour);
    vg_lite_rotate(180, &matrix_hour);
    vg_lite_translate(x * 40.0f, y * 8.0f, &matrix_hour);
    vg_lite_scale(40.0f, 8.0f, &matrix_hour);

    // Minute
    vg_lite_identity(&matrix_min);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_min);
    vg_lite_rotate(12 * 6, &matrix_min);
    vg_lite_translate(x * 60.0f, y * 6.0f, &matrix_min);
    vg_lite_scale(60.0f, 6.0f, &matrix_min);

    // Second
    vg_lite_identity(&matrix_sec);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_sec);
    vg_lite_rotate(22 * 6, &matrix_sec);
    vg_lite_translate(x * 75.0f, y * 2.0f, &matrix_sec);
    vg_lite_scale(75.0f, 2.0f, &matrix_sec);

    vg_lite_identity(&matrix_face[0]);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_face[0]);
    vg_lite_scale(10.0f, 10.0f, &matrix_face[0]);

    vg_lite_identity(&matrix_face[1]);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_face[1]);
    vg_lite_scale(9.5f, 9.5f, &matrix_face[1]);

    vg_lite_identity(&matrix_cap);
    vg_lite_translate(fb_width / 2, fb_height / 2, &matrix_cap);

    vg_lite_identity(&matrix_button[0]);
    vg_lite_translate(fb_width / 2 - BUTTON_WIDTH * 10.0f, fb_height / 2 + 150, &matrix_button[0]);
    vg_lite_scale(10.0f, 10.0f, &matrix_button[0]);

    vg_lite_identity(&matrix_button[1]);
    vg_lite_translate(fb_width / 2 +  10.0f, fb_height / 2 + 150, &matrix_button[1]);
    vg_lite_scale(10.0f, 10.0f, &matrix_button[1]);
}

vg_lite_error_t render_clock()
{
    int32_t i;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_draw(fb, &path_circle, VG_LITE_FILL_EVEN_ODD, &matrix_face[0], VG_LITE_BLEND_NONE, 0xff000000));
    CHECK_ERROR(vg_lite_draw(fb, &path_circle, VG_LITE_FILL_EVEN_ODD, &matrix_face[1], VG_LITE_BLEND_NONE, 0xffcccccc));
    // Draw ticks.
    for (i = 0; i < 12; i++)
    {
        CHECK_ERROR(vg_lite_draw(fb, &path_button, VG_LITE_FILL_EVEN_ODD, &matrix_tick[i], VG_LITE_BLEND_NONE, 0x78cc78ff));
    }

    // Draw hands.
    // Hour
    CHECK_ERROR(vg_lite_draw(fb, &path_hand, VG_LITE_FILL_EVEN_ODD, &matrix_hour, VG_LITE_BLEND_NONE, 0xff0000ff));
    // Minute
    CHECK_ERROR(vg_lite_draw(fb, &path_hand, VG_LITE_FILL_EVEN_ODD, &matrix_min, VG_LITE_BLEND_NONE, 0xff00ff00));
    // Second
    CHECK_ERROR(vg_lite_draw(fb, &path_hand, VG_LITE_FILL_EVEN_ODD, &matrix_sec, VG_LITE_BLEND_NONE, 0xffff0000));

    CHECK_ERROR(vg_lite_draw(fb, &path_circle, VG_LITE_FILL_EVEN_ODD, &matrix_cap, VG_LITE_BLEND_NONE, 0xff453340));


ErrorHandler:
    return error;
}

vg_lite_error_t render_buttons()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    CHECK_ERROR(vg_lite_draw(fb, &path_button, VG_LITE_FILL_EVEN_ODD, &matrix_button[0], VG_LITE_BLEND_NONE, 0xff44ff44));
    CHECK_ERROR(vg_lite_draw(fb, &path_button, VG_LITE_FILL_EVEN_ODD, &matrix_button[1], VG_LITE_BLEND_NONE, 0xff4444ff));

ErrorHandler:
    return error;
}

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path_button);
    vg_lite_clear_path(&path_circle);
    vg_lite_clear_path(&path_hand);
    vg_lite_close();

    if (path_button.path != NULL) {
        free(path_button.path);
    }
    if (path_circle.path != NULL) {
        free(path_circle.path);
    }
    if (path_hand.path != NULL) {
        free(path_hand.path);
    }
}

#ifndef _WIN32
long gettick()
{
    long time;
    struct timeval t;
    gettimeofday(&t, NULL);

    time = t.tv_sec * 1000000 + t.tv_usec;

    return time;
}
#endif

int main(int argc, const char * argv[])
{
    long time;
    int32_t frames = 1;
    int32_t i;
    vg_lite_filter_t filter;
    vg_lite_error_t error;

    /* Initialize the vg_lite engine. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGB565;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    // *** DRAW ***
    build_paths();
    build_matrices();

#ifndef _WIN32
    time = gettick();
#endif

    for (i = 0; i < frames; i++)
    {
        CHECK_ERROR(render_clock());
        CHECK_ERROR(render_buttons());
    }
#ifndef _WIN32
    time = gettick() - time;

    printf("Render %d frames (%d x %d), time used %ld us, FPS %lf.\n", frames, fb_width, fb_height, time, frames / ((float)time / 1000000));
#endif

    CHECK_ERROR(vg_lite_finish());
    // Save PNG file.
    vg_lite_save_png("clock.png", fb);

    // Cleanup.
ErrorHandler:
    cleanup();
    return 0;
}
