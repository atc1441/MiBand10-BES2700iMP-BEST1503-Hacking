#include <stdio.h>

#include <stdlib.h>
#include <math.h>

#include "vg_lite.h"
#include "vg_lite_util.h"

#define RATE(x) (320.0 * (x) / 640)
static vg_lite_buffer_t buffer,buffer1, image_buffer, image_buffer2, render_buffer, render_buffer2;
static vg_lite_buffer_t * fb;

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

static vg_lite_path_t path_circle;
static vg_lite_path_t path_hand;
static vg_lite_path_t path_button;

void build_paths()
{
    char    *pchar;
    float   *pfloat;
    int32_t data_size;

    /* Path circle */
    /* MOV_TO
     CUBIC_TO
     CUBIC_TO
     CUBIC_TO
     CUBIC_TO
     */
    data_size = 4 * 5 + 1 + 8 + 8 * 3 * 4;

    memset(&path_circle, 0, sizeof(path_circle));
    path_circle.quality = VG_LITE_LOW;
    path_circle.format = VG_LITE_FP32;
    path_circle.bounding_box[0] = -CLOCK_RADIUS;
    path_circle.bounding_box[1] = -CLOCK_RADIUS;
    path_circle.bounding_box[2] = CLOCK_RADIUS;
    path_circle.bounding_box[3] = CLOCK_RADIUS;
    path_circle.path_length = data_size;
    path_circle.path = malloc(data_size);

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

    /* Path hands. */
    /*
     MOV_TO
     VLINE
     HLINE
     VLINE
     CLOSE
     */
    data_size = 4 * 4 + 1 + 4 * 2 * 4;/*+ 4 * 3; */

    memset(&path_hand, 0, sizeof(path_hand));
    path_hand.quality = VG_LITE_LOW;
    path_hand.format = VG_LITE_FP32;
    path_hand.bounding_box[0] = 0.0f;
    path_hand.bounding_box[1] = 0.0f;
    path_hand.bounding_box[2] = 1.0f;
    path_hand.bounding_box[3] = 1.0f;
    path_hand.path_length = data_size;
    path_hand.path = malloc(data_size);

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

    /* Path button. */
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
    data_size = 4 * 9 + 1 + 4 * 2 * 5 + 4 * 4 * 4;/*+ 4 * 3; */

    memset(&path_button, 0, sizeof(path_button));
    path_button.quality = VG_LITE_LOW;
    path_button.format = VG_LITE_FP32;
    path_button.bounding_box[0] = 0.0f;
    path_button.bounding_box[1] = 0.0f;
    path_button.bounding_box[2] = BUTTON_WIDTH;
    path_button.bounding_box[3] = BUTTON_HEIGHT;
    path_button.path_length = data_size;
    path_button.path = malloc(data_size);

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

void render_clock(vg_lite_buffer_t * buf, int xoffset, int yoffset,
                  float houroffset, float minoffset, float secoffset)
{
    vg_lite_matrix_t mat;
    int32_t i;
    float   x, y;
    float   angle;

    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
    vg_lite_scale(10.0f, 10.0f, &mat);
    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff000000);

    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
    vg_lite_scale(9.5f, 9.5f, &mat);

    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xffcccccc);

    /* Draw ticks. */
    for (i = 0; i < 12; i++)
    {
        angle = i * 3.14159265f / 6;
        x = 80.0f * cos(angle);
        y = 80.0f * sin(angle);
        vg_lite_identity(&mat);
        vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
        vg_lite_translate(x, y, &mat);
        vg_lite_rotate(i * 30, &mat);
        vg_lite_scale(2.0f, 1.0f, &mat);
        vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0x78cc78ff);
    }

    /* Draw hands. */
    /* Hour */
    x = -0.1f;/* * cos(angle); */
    y = -0.5f;/* * sin(angle); */
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
    vg_lite_rotate(houroffset * 30, &mat);
    vg_lite_translate(x * 40.0f, y * 8.0f, &mat);
    vg_lite_scale(40.0f, 8.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff0000ff);

    /* Minute */
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
    vg_lite_rotate(minoffset * 6, &mat);
    vg_lite_translate(x * 60.0f, y * 6.0f, &mat);
    vg_lite_scale(60.0f, 6.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff00ff00);

    /* Second */
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);
    vg_lite_rotate(secoffset * 6, &mat);
    vg_lite_translate(x * 75.0f, y * 2.0f, &mat);
    vg_lite_scale(75.0f, 2.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xffff0000);

    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 + xoffset, buf->height / 2 + yoffset, &mat);

    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff453340);
}

void render_buttons(vg_lite_buffer_t * buf, int xoffset, int yoffset)
{
    vg_lite_matrix_t mat;
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 - BUTTON_WIDTH * 10.0f + xoffset, buf->height / 2 + 150 + yoffset, &mat);
    vg_lite_scale(10.0f, 10.0f, &mat);
    vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff44ff44);

    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 +  10.0f + xoffset, buf->height / 2 + 150 + yoffset, &mat);
    vg_lite_scale(10.0f, 10.0f, &mat);
    vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff4444ff);
}

vg_lite_error_t createBuffers()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    render_buffer.format = render_buffer2.format = VG_LITE_YUY2_TILED;
    render_buffer.width  = render_buffer2.width  = 640;
    render_buffer.height = render_buffer2.height = 480;
    render_buffer.stride = render_buffer2.stride = 0;
    error = vg_lite_allocate(&render_buffer);
    if (error != VG_LITE_SUCCESS)
    {
        return error;
    }

    error = vg_lite_allocate(&render_buffer2);
    if (error != VG_LITE_SUCCESS)
    {
        return error;
    }

    return error;
}

void cleanup(void)
{
    if (render_buffer.handle != NULL)
    {
        vg_lite_free(&render_buffer);
    }
    if (render_buffer2.handle != NULL)
    {
        vg_lite_free(&render_buffer2);
    }
    if (image_buffer.handle != NULL)
    {
        vg_lite_free(&image_buffer);
    }
    if (image_buffer2.handle != NULL)
    {
        vg_lite_free(&image_buffer2);
    }
    if (buffer.handle != NULL) {
        /* Free the offscreen framebuffer memory. */
        vg_lite_free(&buffer);
    }
    if (buffer1.handle != NULL) {
        vg_lite_free(&buffer1);
    }
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

int main(int argc, const char * argv[])
{

    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t matrix2;
    int loops;
    int32_t frames = 1;

    vg_lite_error_t error = VG_LITE_SUCCESS;
    filter = VG_LITE_FILTER_POINT;

    /* Allocate the buffer. */
    if (argc == 3) {
        buffer1.width = atoi(argv[1]);
        buffer1.height = atoi(argv[2]);
    } else if (argc == 4){
        buffer1.width = atoi(argv[1]);
        buffer1.height = atoi(argv[2]);
        frames = atoi(argv[3]);
    } else {
        buffer1.width  = 640;
        buffer1.height = 480;
    }
    /* Initialize the draw. */
    error = vg_lite_init(buffer1.width, buffer1.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    buffer1.format = VG_LITE_YUY2_TILED;
    buffer.width = buffer1.width;
    buffer.height = buffer1.height;
    buffer.format = VG_LITE_RGBA8888;
    error = vg_lite_allocate(&buffer);
    error = vg_lite_allocate(&buffer1);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    createBuffers();

    /* *** DRAW *** */
    build_paths();

    if (!vg_lite_load_png(&image_buffer, "landscape1.png")) {
        printf("vg_lite_load_png() coud not load the image 'landscape1.png'\n");
        cleanup();
        return -1;
    }

    if (!vg_lite_load_png(&image_buffer2, "drop2.png")) {
        printf("vg_lite_load_png() coud not load the image 'drop2.png'\n");
        cleanup();
        return -1;
    }
    /* Draw the path using the matrix. */

    for (loops = 0; loops < frames; loops++)
    {
        /* Clear the buffer. */
        vg_lite_clear(&render_buffer, NULL, 0x22222288);
        vg_lite_clear(&render_buffer2, NULL, 0x22222288);
        /* Draw 1st part */
        vg_lite_identity(&matrix);
        vg_lite_scale((vg_lite_float_t) buffer1.width / (vg_lite_float_t) image_buffer.width,
                      (vg_lite_float_t) buffer1.height / (vg_lite_float_t) image_buffer.height, &matrix);

        /* Blit the image using the matrix. */
        vg_lite_blit(&render_buffer, &image_buffer, &matrix, VG_LITE_BLEND_SRC_OVER, 0, filter);
        render_clock(&render_buffer, -160, 0, 0, 15, 8);
        render_buttons(&render_buffer, -160, 0);

        /* Draw 2nd part */
        vg_lite_identity(&matrix2);
        vg_lite_scale((vg_lite_float_t) buffer1.width / (vg_lite_float_t) image_buffer2.width,
                      (vg_lite_float_t) buffer1.height / (vg_lite_float_t) image_buffer2.height, &matrix2);

        /* Blit the image using the matrix. */
        vg_lite_blit(&render_buffer2, &image_buffer2, &matrix2, VG_LITE_BLEND_SRC_OVER, 0, filter);
        render_clock(&render_buffer2, 160, 0, 6, 15, 22);
        render_buttons(&render_buffer2, 160, 0);

        /* Compose 2 parts to framebuffer */
        vg_lite_blit2(&buffer1, &render_buffer, &render_buffer2, &matrix, &matrix2, VG_LITE_BLEND_MULTIPLY, filter);
    }
    vg_lite_identity(&matrix2);
    vg_lite_blit(fb, &buffer1, &matrix, VG_LITE_BLEND_NONE, 0, filter);
    vg_lite_finish();
    vg_lite_save_png("worst2_comp.png", fb);
    /* Cleanup. */
    cleanup();
    return 0;
}
