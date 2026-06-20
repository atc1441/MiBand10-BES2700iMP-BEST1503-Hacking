/*
 Resolution: 256 x 256
 Format: VG_LITE_RGB565
 Transformation: None
 Alpha Blending: None
 Related APIs: vg_lite_clear
 Description: Clear whole buffer with blue first, then clear a sub-rectangle of the buffer with red.
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "cmsis_nvic.h"
#include "gpu_common.h"

//#define SIM_GPU


#ifdef SIM_GPU

#define GPU_REP(...)   //TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   //TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#define TRACE(...)
#else
#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#endif


#define DEFAULT_SIZE   454.0f;
//#define DEFAULT_SIZE   256.0f;

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


#ifdef SIM_GPU
static int   fb_width = 32, fb_height = 32;
#else
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
#endif
static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}

#if 0
__aligned(64)  uint32_t  test_cmd_buffer[]={
#if 0 //ts is 0
0x30010AC8,
0x00040000,
0x30010ACB,
0x09032DC0,
0x30010ACC,
0x00004000,
#endif
#if 0
0x30010A11,   //fb
0x00000000,   //fb addr
0x30010A12,   //stride
0x00000280,
0x30010A13,   // h,w
0x00a000a0,
0x30010A02,   //color
0xFF33CCFF,
0x30010A34,   //clear
0x00000000,
0x30010A00,   //
0x00000041,
0x30010A10,  //pix format
0x00011013,
0x30010A01,  //rect
0x00000000,
0x40000001,  //data
0x00000000,
0x00000000,  //rect
0x00a000a0,
0x30010A1B,
0x00000001,
0x30010A1B,
0x00000001,
0x30010A1B,
0x00011001,
0x30010A1B,
0x00011001,
0x00000000,
0xDEADDEAD,
#else
 0x30010a11,
 0x20028280,
 0x30010a12,
 0x00000280,
 0x30010a13,
 0x00a000a0,

 0x30010a02,
 0xffff0000,
 0x30010a34,
 0x00000000,
 0x30010a00,
 0x10000001,
 0x30010a10,
 0x00011013,//0x00010123,

 0x30010A01,  //rect
 0x00000000,
 0x40000001,
 0x00000000,
 0x00000000,
 0x00a000a0,
 0x30010a1b,
 0x00000001,
#if 1

0x30010a1b,
0x00000001,
0x30010A1B,
0x00011001,
0x30010A1B,
0x00011001,
#endif


 0x30010a34,
 0x00000000,
 0x30010a00,
 0x10000001,
 0x30010a02,
 0xff0000ff,
 0x30010A01,  //rect
 0x00000000,
 0x40000001,
 0x00000000,
 0x00400040,
 0x00a000a0,
 0x30010a1b,
 0x00000001,
 0x30010a1b,
 0x00000001,
#if 1
0x30010A1B,
0x00011001,
0x30010A1B,
0x00011001,
#endif
 #if 0
 0x10000007,
 0x00000000,
 0x20000007,
 0x00000000,
 0x00000000,
 #endif

 0x00000000,
 0xDEADDEAD,

#endif

} ;


#define VG_LITE_HW_CMDBUF_ADDRESS   0x500
#define VG_LITE_HW_CMDBUF_SIZE      0x504
int clear_test_main_entry(int argc, const char * argv[])
{
   // Initialize the blitter.
    vg_lite_init(0, 0);  //ts =0


    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_ARGB8888;

    vg_lite_allocate(&buffer);
    fb = &buffer;
    test_cmd_buffer[1] = (uint32_t)fb->memory;

        /* Write the registers to kick off the command execution (CMDBUF_SIZE). */
     *(uint32_t *) (uint8_t *) (0x401d0000 + VG_LITE_HW_CMDBUF_ADDRESS) = (uint32_t)test_cmd_buffer;

     *(uint32_t *) (uint8_t *) (0x401d0000 + VG_LITE_HW_CMDBUF_SIZE) = sizeof(test_cmd_buffer)/ 8;

    while(1){
        ;
    }


    return 0;
}


#else
/*
 0x30010a10,
 0x00010123,
 0x30010a11,
 0x20028280,
 0x30010a12,
 0x00000280,
 0x30010a13,
 0x00a000a0,
 0x30010a34,
 0x00000000,
 0x30010a00,
 0x10000001,
 0x30010a02,
 0xffff0000,
 0x40000001,
 0x00000000,
 0x00000000,
 0x00a000a0,
 0x30010a1b,
 0x00000001,
 0x30010a34,
 0x00000000,
 0x30010a00,
 0x10000001,
 0x30010a02,
 0xff0000ff,
 0x40000001,
 0x00000000,
 0x00000040,
 0x00000040,
 0x30010a1b,
 0x00000001,
 0x30010a1b,
 0x00000001,
 0x10000007,
 0x00000000,
 0x20000007,
 0x00000000,
 0x00000000,
 0x00000000,


*/

int clear_test_main_entry(int argc, const char * argv[])
{
    vg_lite_rectangle_t rect = { 100, 100, 64, 64 };
    vg_lite_error_t error = VG_LITE_SUCCESS;

    TC_INIT
    GPU_REP("%s", __func__);
    // Initialize the blitter.
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;

    CHECK_ERROR(vg_lite_allocate(&buffer));

    fb = &buffer;


    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    GPU_REP("%s %d fb: %p", __func__, __LINE__, fb->memory);

    // Clear a sub-rectangle of the buffer with red.
    CHECK_ERROR(vg_lite_clear(fb, &rect, 0xFF0000FF));

    gpu_save_cmdbuffer("clear.cb");

    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    TC_REP(clear_gcost);

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("clear.raw", fb);

    gpu_set_soft_break_point();


ErrorHandler:
    // Cleanup.
    cleanup();
    GPU_REP("%s test pass", __func__);

    return 0;
}
#endif