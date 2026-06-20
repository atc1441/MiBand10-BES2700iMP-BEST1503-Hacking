#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "tiger_paths.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

extern uint32_t __tiger_fbdata_start_flash[];
extern uint32_t __tiger_fbdata_end_flash[];

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
static int fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;

static void cleanup(void)
{
    int32_t i;

    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }

    vg_lite_close();
}

#ifdef GPU_CPU_PSRAM_PARALL_TEST
#define GPU_TEST_LOOP
#endif

extern int32_t gpu_test_wait_interrupt(void);

int tiger_test_main_entry(int argc, const char * argv[])
{
    int i;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    TC_INIT
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s", __func__);
#ifdef GPU_CPU_PSRAM_PARALL_TEST
    vg_lite_buffer_t sram_fb;
    uint32_t count = 454 * 454;
//    uint32_t * psram_test_addr = (uint32_t *)0x38000020;
    uint32_t * psram_test_addr = (uint32_t *)0x3c000020;
//    uint32_t * psram_test_addr = (uint32_t *)(SYS_RAM_BASE );
    volatile  uint32_t wait = 1;
    volatile  uint32_t *err_flag =  (uint32_t *)0x3c000000;
#endif
#ifdef GPU_TEST_LOOP
 __test_again:
#endif
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    buffer.memory = 0;

    filter = VG_LITE_FILTER_POINT;
    //filter = VG_LITE_FILTER_LINEAR;
    //printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
    TC_START

    // Setup a scale at center of buffer.
    vg_lite_identity(&matrix);

#if 1
    vg_lite_translate(fb_width / 2 - 10 * fb_width / 454.0f,
                      fb_height / 2 - 80 * fb_height / 454.0f, &matrix);
    vg_lite_scale(2, 2, &matrix);
    vg_lite_scale(fb_width / 454.0f, fb_height / 454.0f, &matrix);
#endif
#if 0//rotate test
    static vg_lite_float_t rote_degree = 0;

    vg_lite_rotate(rote_degree, &matrix);

    rote_degree+=45.0f;

    if(rote_degree>=360){
        rote_degree = 0;
    }
#endif
    // Draw the path using the matrix.
    for (i = 0; i < pathCount; i++)
    {
        CHECK_ERROR(vg_lite_draw(fb, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]));
    }

    gpu_save_cmdbuffer("tiger.cb");

    //TC_START
    CHECK_ERROR(vg_lite_finish());
    cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);
#ifdef GPU_CPU_PSRAM_PARALL_TEST
while(0)
{
    memcpy(&sram_fb,fb, sizeof(vg_lite_buffer_t));
    sram_fb.memory = *(uint32_t *)0x20400000;
    sram_fb.address = *(uint32_t *)0x20400000;

    vg_lite_identity(&matrix);
    vg_lite_translate(0, 0, &matrix);
    CHECK_ERROR(vg_lite_blit(&sram_fb, fb, &matrix, VG_LITE_BLEND_NONE, 0, filter));

    CHECK_ERROR(vg_lite_finish());
    gpu_test_wait_interrupt();

//    TRACE(3, "%s blit", __func__);
}
#endif

 #ifdef GPU_CPU_PSRAM_PARALL_TEST
    {
       // while(0)
        {
            TRACE(3, "%s %d", __func__, __LINE__);

            memset(psram_test_addr, 0x5a,count*4);
            #if 1
            for (int i  = 0; i < count; i++){
                if(*(psram_test_addr + i) != 0x5a5a5a5a){

                        *err_flag = psram_test_addr + i;
                        TRACE(3, "%s fail addr: %p ", __func__, *err_flag);
                        TRACE(3, "%s fail addr: %p ", __func__, (psram_test_addr + i));
                        TRACE(3, "%s fail 0x5a5a5a5a: 0x%8x ", __func__, *(psram_test_addr + i));
                      //  while(wait);
                }
            }
            #endif
        }
    }

    gpu_test_wait_interrupt();


#endif
    TC_END
    TC_REP(tiger_gcost);

    gpu_lcdc_display(fb);

    // Save PNG file.
    gpu_save_raw("tiger.raw", fb);

#ifdef GPU_CPU_PSRAM_PARALL_TEST
    {
    int ccount = (uint32_t)__tiger_fbdata_end_flash - (uint32_t)__tiger_fbdata_start_flash ;
    volatile uint32_t * ps = __tiger_fbdata_start_flash;
    volatile uint32_t * pd = (uint32_t *)fb->memory;
    ccount /=4;

    for(int i = 0; i < ccount; i++){
        if(*ps != *pd){
           GPU_REP("%s test result fail i:%d source:%p - dest:%p  ", __func__,i,ps, pd);
           GPU_REP("%s test result fail i:%d 0x%x - 0x%x  ", __func__,i,*ps, *pd);
           while(wait);
           //break;
        }
        ps +=1 ;
        pd +=1 ;
    }
    }
#endif//psram+gpu test
    GPU_REP("%s %d tfbd: %p %p", __func__, __LINE__, __tiger_fbdata_start_flash, __tiger_fbdata_end_flash);
#if 0
    if (!memcmp(__tiger_fbdata_start_flash,fb->memory, ccount))
    {
        GPU_REP("%s test result pass ", __func__);
    }else{
        GPU_REP("%s test result fail ", __func__);
    }
#endif
    gpu_set_soft_break_point();

ErrorHandler:
    // Cleanup.
    cleanup();
    GPU_REP("%s test pass", __func__);
#ifdef GPU_TEST_LOOP
    goto __test_again;
#endif
    return 0;
}

int gpu_draw_tiger_test(vg_lite_buffer_t* fb)
{
    int i;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    TC_INIT
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s", __func__);

    filter = VG_LITE_FILTER_POINT;


    // Setup a scale at center of buffer.
    vg_lite_identity(&matrix);

    vg_lite_translate(fb->width/ 2 + 16, fb->height/2 + 16 , &matrix);
    vg_lite_scale(fb->width / 160.0f, fb_height / 130.0f, &matrix);
    cpu_cache_flush(fb->memory, fb->height * fb->stride);
    TC_START

    // Draw the path using the matrix.
    for (i = 0; i < pathCount; i++)
    {
        CHECK_ERROR(vg_lite_draw(fb, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]));
    }

    //TC_START
    CHECK_ERROR(vg_lite_finish());

    TC_END
    cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);
    TC_REP(tiger_gcost);

    for (i = 0; i < pathCount; i++)
    {
        vg_lite_clear_path(&path[i]);
    }

ErrorHandler:
    GPU_REP("%s test pass", __func__);
    return 0;
}

