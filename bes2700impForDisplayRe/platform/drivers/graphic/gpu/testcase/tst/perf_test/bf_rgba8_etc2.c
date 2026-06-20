/*
 Resolution: 320 x 480
 Format: VG_LITE_RGB565
 Transformation: Rotate/Scale/Translate
 Alpha Blending: None
 Related APIs: vg_lite_clear/vg_lite_translate/vg_lite_scale/vg_lite_rotate/vg_lite_blit
 Description: Load outside landscape image, then blit it to blue dest buffer with rotate/scale/translate.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "gpu_port.h"
#include "gpu_common.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define printf GPU_ERR


#undef malloc
#define malloc  gpu_malloc
#undef free
#define free  gpu_free



#define DEFAULT_SIZE   320.0f;
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
//static int   fb_width = 480, fb_height = 480;
static int   fb_width = 512, fb_height = 512;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t gamubuffer;
static vg_lite_buffer_t * fb;

static vg_lite_buffer_t image;

extern uint32_t __landscape_start_flash[];
extern uint32_t __landscape_end_flash[];


extern int vg_lite_bf_load_pkm_info_to_buffer(vg_lite_buffer_t * buffer, const char * fdata);
extern int vg_lite_bf_load_pkm(vg_lite_buffer_t * buffer, const char * fdata);

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        vg_lite_free(&image);
    }

   // vg_lite_close();
}




static void* fb_malloc(uint32_t size, uint8_t aligns)
{
    return gamubuffer.memory;
}



#ifdef GAMU_BASE
#include "hal_gamu.h"

/*
 * gamu_fb layout
            width = 512
    |---------------------|
    |(xoffset, yoffset)   |
    | -|----------------| |
    |  |                | |
    |  |                | |
    |  |     2r         | |
    |  |                | |
    |  |                | |
    |  |----------------| |
    |                     |
    |---------------------|
*/

static gamu_fb_info_t  gamu_fb = {
    .id = 0,
    .width = 512,  // must be 512 pixels for 1502x
    .height = 512,
    .bpp = 32,
    .align_malloc = fb_malloc,
    .sqrtf = sqrtf,
    // width = xoffset + 2* radius
    // height = yoffset + 2* radius
    .radius = 240,//256,//,128,//48,
    .xoffset = 16, // colu offset
    .yoffset = 16, // line offset
    .used_vblocks = 0,
};

#endif

int rgba8etc2_test(void)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;

    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;
    TC_INIT
    if (vg_lite_bf_load_pkm_info_to_buffer(&image, __landscape_start_flash) != 0) {
        printf("load pkm file error\n");
        cleanup();
        return -1;
    }
    printf("%s w:%d h:%d f:%d", __func__,image.width, image.height, image.format );
    CHECK_ERROR(vg_lite_allocate(&image));
    if (vg_lite_bf_load_pkm(&image, __landscape_start_flash) != 0) {
        printf("load pkm file error\n");
        cleanup();
        return -1;
    }

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;//VG_LITE_RGB565;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    vg_lite_identity(&matrix);
    #if 0
    vg_lite_translate(fb_width / 2.0f, fb_height / 2.0f, &matrix);
    vg_lite_rotate(33.0f, &matrix);
    vg_lite_translate(fb_width / -2.0f, fb_height / -2.0f, &matrix);
    vg_lite_scale((vg_lite_float_t) fb_width / (vg_lite_float_t) image.width,
                  (vg_lite_float_t) fb_height / (vg_lite_float_t) image.height, &matrix);
    #endif
    CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));

    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    TC_REP("etc2 blit cost");
    cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);


#ifdef GAMU_BASE

    gamubuffer.width  = fb_width;
    gamubuffer.height = fb_height;
    gamubuffer.format = VG_LITE_RGBA8888;//VG_LITE_RGB565;
    CHECK_ERROR(vg_lite_allocate(&gamubuffer));

    hal_gamu_setup(&gamu_fb);
    gamubuffer.memory = gamu_fb.vmem;
    gamubuffer.address = gamubuffer.memory;

    vg_lite_identity(&matrix);


    CHECK_ERROR(vg_lite_blit(&gamubuffer,fb, &matrix, VG_LITE_BLEND_NONE, 0, filter));


    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    TC_REP("etc2 gamu buffer blit cost");
    cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);


    printf("%s Fb memory:%p",__func__ ,gamu_fb.phymem);
    printf("%s Fb vmemory:%p",__func__ ,gamubuffer.memory);
    
   // while(1);
    vg_lite_free(&gamubuffer);
#endif

ErrorHandler:
    
    cleanup();
    return 0;
}
