#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <math.h>

#include "vg_lite.h"
#include "vg_lite_util.h"

static vg_lite_buffer_t buffer, image_buffer;
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
    path_circle.quality = VG_LITE_HIGH;
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
    path_hand.quality = VG_LITE_HIGH;
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
    path_button.quality = VG_LITE_HIGH;
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

void cleanup(void)
{
    if (buffer.handle != NULL) {
        /* Free the buffer memory. */
        vg_lite_free(&buffer);
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
    if (image_buffer.handle != NULL)
    {
        vg_lite_free(&image_buffer);
    }
}
void render_clock(vg_lite_buffer_t * buf)
{
    vg_lite_matrix_t mat;
    int32_t i;
    float   x, y;
    float   angle;
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_scale(5.0f, 5.0f, &mat);
    
    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff000000);
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_scale(4.75f, 4.75f, &mat);
    
    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xffcccccc);
  
    for (i = 0; i < 12; i++)
    {
        angle = i * 3.14159265f / 6;
        x = 80.0f * cos(angle);
        y = 80.0f * sin(angle);
        vg_lite_identity(&mat);
        vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
        vg_lite_translate(x * 0.5f, y * 0.5f, &mat);
        vg_lite_rotate(i * 30, &mat);
        vg_lite_scale(1.0f, 0.5f, &mat);
        vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0x78cc78ff);
    }
    
    x = -0.1f;
    y = -0.5f;
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_rotate(7 * 30, &mat);
    vg_lite_translate(x * 40.0f, y * 8.0f, &mat);
    vg_lite_scale(20.0f, 4.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff0000ff);
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_rotate(12 * 6, &mat);
    vg_lite_translate(x * 60.0f, y * 6.0f, &mat);
    vg_lite_scale(30.0f, 3.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff00ff00);
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_rotate(22 * 6, &mat);
    vg_lite_translate(x * 75.0f, y * 2.0f, &mat);
    vg_lite_scale(37.5f, 1.0f, &mat);
    vg_lite_draw(buf, &path_hand, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xffff0000);
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2, buf->height / 2, &mat);
    vg_lite_scale(0.7f, 0.7f, &mat);
    
    vg_lite_draw(buf, &path_circle, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff453340);
}

void render_buttons(vg_lite_buffer_t * buf)
{
    vg_lite_matrix_t mat;
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 - BUTTON_WIDTH * 5.5f, buf->height / 2 + 70, &mat);
    vg_lite_scale(5.0f, 5.0f, &mat);
    vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff44ff44);
    
    vg_lite_identity(&mat);
    vg_lite_translate(buf->width / 2 +  6.0f, buf->height / 2 + 70, &mat);
    vg_lite_scale(5.0f, 5.0f, &mat);
    vg_lite_draw(buf, &path_button, VG_LITE_FILL_EVEN_ODD, &mat, VG_LITE_BLEND_NONE, 0xff4444ff);
}
int main(int argc, const char * argv[])
{
    int32_t frames = 1;
    int32_t i;
    vg_lite_matrix_t matrix;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_filter_t filter = VG_LITE_FILTER_POINT;
    /* Try to open the framebuffer. */
    if (argc == 3) {
        buffer.width = atoi(argv[1]);
        buffer.height = atoi(argv[2]);
    } else if (argc == 4){
        buffer.width = atoi(argv[1]);
        buffer.height = atoi(argv[2]);
        frames = atoi(argv[3]);
    } else {
        buffer.width  = 640;
        buffer.height = 480;
    }
    /* Initialize the draw. */
    error = vg_lite_init(buffer.width, buffer.height);
    if (error) {
        printf("vg_lite_draw_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    buffer.format = VG_LITE_BGRA8888;
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;

    build_paths();

    if (!vg_lite_load_png(&image_buffer, "drop.png")) {
        printf("vg_lite_load_png() coud not load the image 'drop.png'\n");
        cleanup();
        return -1;
    }
    /* Draw the path using the matrix. */
    for (i = 0; i < frames; i++)
    {
        vg_lite_identity(&matrix);
        vg_lite_scale((vg_lite_float_t) fb->width / (vg_lite_float_t) image_buffer.width,
                      (vg_lite_float_t) fb->height / (vg_lite_float_t) image_buffer.height, &matrix);
        render_clock(&image_buffer);
        render_buttons(&image_buffer);
        /* Blit the image using the matrix. */
        vg_lite_blit(fb, &image_buffer, &matrix, VG_LITE_BLEND_NONE, 0, filter);
    }
    if (error) {
        printf("vg_lite_draw() returned error %d\n", error);
        cleanup();
        return -1;
    }
    vg_lite_finish();
    /* Save PNG file. */
    vg_lite_save_png("worst_4.png", fb);
    /* Cleanup. */
    cleanup();
    return 0;
}
