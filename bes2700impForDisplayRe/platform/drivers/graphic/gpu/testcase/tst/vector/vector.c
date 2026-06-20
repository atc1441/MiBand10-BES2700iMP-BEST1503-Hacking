/*
 Resolution: 256 x 256
 Format: VG_LITE_RGB565
 Transformation: Translate/Scale
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_draw
 Description: Draw a polygon vector graphic with high render quality on blue buffer.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "golden.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define DEFAULT_SIZE   454.0f;
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
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.

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
static char path_data[] = {
    2, -5, -10, // moveto   -5,-10
    4, 5, -10,  // lineto    5,-10
    4, 10, -5,  // lineto   10, -5
    4, 0, 0,    // lineto    0,  0
    4, 10, 5,   // lineto   10,  5
    4, 5, 10,   // lineto    5, 10
    4, -5, 10,  // lineto   -5, 10
    4, -10, 5,  // lineto  -10,  5
    4, -10, -5, // lineto  -10, -5
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

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the offscreen framebuffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);
    vg_lite_close();
}

//#define strb_test

#ifdef strb_test
static void test_strb(void)
{
int bv= 0x7a;
int val= 0x5a5a5a5a;
int addr= 0x20092680;
asm  volatile (
"mov r2,%0\n"
"mov r3,%1\n"
"mov r4,%2\n"
"strd    r3, r3, [r4]\n"
"strb    r2,  [r4 , #13]\n"
:
: "r"(bv), "r"(val) , "r"(addr)
: "r2" , "r3" , "r4"
);


while(1){
    ;
}

}
#endif

#if 0 // tss0 cmd buffer dumped
__aligned(64) uint32_t cmd_buffer[]={
0x30010ac8,
0x00040000,
0x30010acb,
0x20090400,
0x30010acc,
0x00004000,
0x30010a11,
0x20092800,
0x30010a12,
0x00000500,
0x30010a13,
0x00f00140,
0x30010a02,
0xffff0000,
0x30010a34,
0x00000000,
0x30010a00,
0x00000001,
0x30010a10,
0x00011013,
0x30010a01,
0x00000000,
0x40000001,
0x00000000,
0x00000000,
0x00f00140,
0x30010a1b,
0x00000001,
0x30010a11,
0x20092800,
0x30010a12,
0x00000500,
0x30010a13,
0x00f00140,
0x30010a02,
0xff0000ff,
0x30010a34,
0x01000000,
0x30010a00,
0x00000003,
0x30010a10,
0x00011013,
0x30010a01,
0x00000000,
0x30010a35,
0x20050400,
0x30010a1b,
0x00010000,
0x30010a30,
0x20050400,
0x30010a33,
0x00000c00,
0x30010ab2,
0x00000c00,
0x30010a38,
0x00000c00,
0x30010ab1,
0x00000c00,
0x30010a39,
0x00000000,
0x30010a3a,
0x00f00140,
0x30060a40,
0x41f00000,
0x00000000,
0x41700000,
0x00000000,
0x41b40000,
0x41340000,
0xdeaddead,
0x30010acd,
0x00000000,
0x30010ace,
0x00000000,
0x30010a34,
0x01308513,
0x30010a1b,
0x00001000,
0x30010a3d,
0x00000001,
0x40000009,
0x00000000,
0x00000002,
0x40500000,
0x41180000,
0x00000004,
0x41140000,
0x40000000,
0x00000004,
0xbfa00000,
0x3f400000,
0x00000004,
0x41180000,
0x40d80000,
0x00000004,
0x40a80000,
0xbfc00000,
0x00000001,
0x00000000,
0x00000000,
0x30010a1b,
0x00000001,
0x30010a1b,
0x00011001,
0x30010a1b,
0x00011001,
0x00000000,
0xdeaddead,
}

#endif

#ifdef  testcase_tss0 //  1 // tss0 cmd buffer test


__aligned(64) uint32_t cmd_buffer[]={
0x30010AC8,
0x00040000,
0x30010ACB,//tsbuffer.tessellation_buffer_gpu + tessellation_size
0xc0000000,   // 3
0x30010ACC,//vg_count_buffer_size
0x00004000,
0x30010A11,// fb addr
0x60000000,   // 7
0x30010A12,
0x00000500,
0x30010A13,
0x00f00140,
0x30010A02,
0xFFFF0000,
0x30010A34,
0x00000000,
0x30010A00,
0x00000001,
0x30010A10,
0x00011013,
0x30010A01,
0x00000000,
0x40000001,
0x00000000,
0x00000000,
0x00F00140,
0x30010A1B,
0x00000001,
0x30010A11,// fb addr
0x60000000, //29
0x30010A12,
0x00000500,
0x30010A13,
0x00f00140,
0x30010A02,
0xFF0000FF,
0x30010A34,
0x01000000,
0x30010A00,
0x00000003,
0x30010A10,
0x00011013,
0x30010A01,
0x00000000,
0x30010A35, // tsbuffer.tessellation_buffer_gpu
0xb0000000, //  45
0x30010A1B,
0x00010000,
0x30010A30, //tsbuffer.tessellation_buffer_gpu + 0x00000000
0xb0000000, //  49
0x30010A33,
0x00000C00,
0x30010AB2,
0x00000C00,
0x30010A38,
0x00000C00,
0x30010AB1,
0x00000C00,
0x30010A39,
0x00000000,
0x30010A3A,
0x00F00140,
0x30060A40,
0x41F00000,
0x00000000,
0x41700000,
0x00000000,
0x41B40000,
0x41340000,
0xDEADDEAD,
0x30010ACD,
0x00000000,
0x30010ACE,
0x00000000,
0x30010A34,
0x01308513,
0x30010A1B,
0x00001000,
0x30010A3D,
0x00000001,
0x40000009,
0x00000000,
0x00000002,
0x40500000,
0x41180000,
0x00000004,
0x41140000,
0x40000000,
0x00000004,
0xBFA00000,
0x3F400000,
0x00000004,
0x41180000,
0x40D80000,
0x00000004,
0x40A80000,
0xBFC00000,
0x00000001,
0x00000000,
0x00000000,
0x30010A1B,
0x00000001,
0x30010A1B,
0x00011001,
0x30010A1B,
0x00011001,
0x00000000,
0xDEADDEAD,
};

extern void * vg_lite_get_tsbuffer(void);
extern int32_t vg_lite_get_tsbuffer_size(void);
extern uint32_t vg_lite_get_vg_count_buffer_size(void);

static void cmd_test(void)
{

 // Initialize the draw.

 vg_lite_init(32, 32);

 buffer.width  = 454;
 buffer.height = 454;
 buffer.format = VG_LITE_RGBA8888;
 vg_lite_allocate(&buffer);
 uint32_t *fb = (uint32_t *)buffer.memory;
 uint32_t *tsb = (uint32_t*) vg_lite_get_tsbuffer();

 cmd_buffer[7] = (uint32_t)fb;
 cmd_buffer[29] = (uint32_t)fb;
 cmd_buffer[45] = (uint32_t)tsb;
 cmd_buffer[49] = (uint32_t)tsb;
 cmd_buffer[3] = (uint32_t)tsb + vg_lite_get_tsbuffer_size();
 cmd_buffer[5] =vg_lite_get_vg_count_buffer_size();

 *(volatile uint32_t *)0x401d0500 = (uint32_t)cmd_buffer;
 *(volatile uint32_t *)0x401d0504 = (sizeof(cmd_buffer) + 7) / 8;

#endif


int vector_test_main_entry(int argc, const char * argv[])
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    TC_INIT
    GPU_REP("%s", __func__);

    #ifdef strb_test
    test_strb();
    #endif

    #ifdef testcase_tss0
    cmd_test();
    #endif

__again:

    // Initialize the draw.
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    //printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;

    CHECK_ERROR(vg_lite_allocate(&buffer));

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(&buffer, NULL, 0xFFFF0000));
    // *** DRAW ***
    // Setup a 10x10 scale at center of buffer.
    vg_lite_identity(&matrix);
    // Translate the matrix to the center of the buffer.
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    // Scale up matrix with 10 times.
    vg_lite_scale(10, 10, &matrix);
//    vg_lite_scale(5, 5, &matrix);
    // Scale matrix according to buffer size.
    vg_lite_scale(fb_scale, fb_scale, &matrix);

    // Draw the path using the matrix.
    CHECK_ERROR(vg_lite_draw(&buffer, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));


    gpu_save_cmdbuffer("vector.cb");

    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    TC_REP(vector_gcost);

    gpu_lcdc_display(&buffer);

    // Save PNG file.
    gpu_save_raw("vector.raw", &buffer);

    gpu_set_soft_break_point();


ErrorHandler:
    // Cleanup.
    cleanup();
#ifdef GPU_LOOP_TEST
    goto __again;
#endif
    GPU_REP("%s test pass", __func__);
    return 0;
}
