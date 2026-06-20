
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)



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

static int   fb_width = 480, fb_height = 480;
//static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;


static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;
static int frames = 1; //Frames to render

void gpu_cache_invalid(void);
void cpu_gpu_data_cache_invalid(uint32_t start, uint32_t length);

static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image);
    }

    vg_lite_close();
}


static void test_draw_gradient(struct fb_state_s* state,
                           vg_lite_buffer_t* vg_lite_buffer)
{
    vg_lite_linear_gradient_t grad;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    memset(&grad, 0, sizeof(grad));
    if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {

        //printf("Linear gradient is not supported.\n");
        TRACE(3,"Linear gradient is not supported.\n");
        return 0;
    }

    uint32_t ramps[] = {0xff000000, 0xffff0000, 0xff00ff00, 0xff0000ff,
                        0xffffffff};
    uint32_t stops[] = {0, 32, 64, 96, 128};

    vg_lite_set_grad(&grad, 5, ramps, stops);
    vg_lite_update_grad(&grad);

    vg_lite_matrix_t* matGrad = vg_lite_get_grad_matrix(&grad);
    vg_lite_identity(matGrad);
    vg_lite_translate(0, 128, matGrad);

    vg_lite_path_t vpath;
    memset(&vpath, 0, sizeof(vg_lite_path_t));

    int16_t path_data_S16[] = {
        VLC_OP_MOVE,  0,    128,
        VLC_OP_LINE,  0,    256,
        VLC_OP_LINE,  128,  256,
        VLC_OP_LINE,  128,  128,
        VLC_OP_LINE,  0,  128,
        VLC_OP_END
    };


    CHECK_ERROR(vg_lite_init_path(&vpath, VG_LITE_S16, VG_LITE_HIGH,
                                  sizeof(path_data_S16), path_data_S16, 0, 128,
                                  128, 256));


    vg_lite_matrix_t matPath;
    vg_lite_identity(&matPath);

    vg_lite_draw_gradient(vg_lite_buffer, &vpath, VG_LITE_FILL_EVEN_ODD,
                          &matPath, &grad, VG_LITE_BLEND_NONE);


    vg_lite_finish();
    cpu_gpu_data_cache_invalid(vg_lite_buffer->memory, vg_lite_buffer->height
                               * vg_lite_buffer->stride);


    GPU_REP("%s  fb_mem:%p    height :%d stride:%d  size:%d " , __func__,  vg_lite_buffer->memory, vg_lite_buffer->height,
                            vg_lite_buffer->stride, (  vg_lite_buffer->height * vg_lite_buffer->stride )    );


    vg_lite_clear_grad(&grad);

    ErrorHandler:
    return;
}


static void test_draw_gradient_01(struct fb_state_s* state,
                           vg_lite_buffer_t* vg_lite_buffer)
{


    vg_lite_linear_gradient_t grad;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    memset(&grad, 0, sizeof(grad));
    if (VG_LITE_SUCCESS != vg_lite_init_grad(&grad)) {
        //printf("Linear gradient is not supported.\n");
        TRACE(3,"Linear gradient is not supported.\n");
        return 0;
    }


    uint32_t ramps[] = {0xff000000,  0xff00ff00, 0xffff0000, 0xff0000ff,
                        0xffffffff};


    uint32_t stops[] = {0, 32, 64, 96, 128};


    vg_lite_set_grad(&grad, 5, ramps, stops);

    
    vg_lite_update_grad(&grad);

    vg_lite_matrix_t* matGrad = vg_lite_get_grad_matrix(&grad);
    vg_lite_identity(matGrad);

    vg_lite_translate(256, 128, matGrad);


    vg_lite_path_t vpath;
    memset(&vpath, 0, sizeof(vg_lite_path_t));

    int16_t path_data_S16[] = {
        VLC_OP_MOVE,  256,    128,
        VLC_OP_LINE,  256,    256,
        VLC_OP_LINE,  384,  256,
        VLC_OP_LINE,  384,  128,
        VLC_OP_LINE,  256,  128,
        VLC_OP_END
    };

   // CHECK_ERROR(vg_lite_clear(vg_lite_buffer, NULL, 0xFFFF0000));


    CHECK_ERROR(vg_lite_init_path(&vpath, VG_LITE_S16, VG_LITE_HIGH,
                                  sizeof(path_data_S16), path_data_S16, 256, 128,
                                  384, 256));

    vg_lite_matrix_t matPath;
    vg_lite_identity(&matPath);


    vg_lite_draw_gradient(vg_lite_buffer, &vpath, VG_LITE_FILL_EVEN_ODD,
                          &matPath, &grad, VG_LITE_BLEND_NONE);



    vg_lite_finish();
    cpu_gpu_data_cache_invalid(vg_lite_buffer->memory, vg_lite_buffer->height
                               * vg_lite_buffer->stride);


    GPU_REP("%s  fb_mem:%p    height :%d stride:%d  size:%d " , __func__,  vg_lite_buffer->memory, vg_lite_buffer->height,
                            vg_lite_buffer->stride, (  vg_lite_buffer->height * vg_lite_buffer->stride ) );
while(1);

#if 0
#ifdef CONFIG_FB_UPDATE
    ioctl(state->fd, FBIO_UPDATE, (unsigned long)((uintptr_t)&fb_area));
#endif
#endif

    vg_lite_clear_grad(&grad);

    ErrorHandler:
    return;
}



int rect_linergrad_test_main_entry(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_rectangle_t rect;
 
    // Initialize vg_lite engine.
    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s", __func__);

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;

    buffer.format = VG_LITE_RGBA8888;
    //buffer.format = GPU_FB_FORMAT;
    // Clear the buffer with blue.
    CHECK_ERROR(vg_lite_allocate(&buffer));

    GPU_REP("%s  fb_mem:%p    height :%d stride:%d  size:%d " , __func__,  buffer.memory, buffer.height,
                            buffer.stride, (  buffer.height * buffer.stride )    );

  CHECK_ERROR(vg_lite_clear(&buffer, NULL, 0xFFFF0000));
   
#if 0
  //rect.x = 128;
  //rect.y = 128;
  rect.x = 0;
  rect.y = 0;
  rect.width = 128;
  rect.height = 128;
  CHECK_ERROR(vg_lite_clear(&buffer, &rect, 0xFF00ff00));
   
  CHECK_ERROR(vg_lite_finish());

 uint32_t *ptr = (uint32_t *)buffer.memory;

 TRACE(3,"%s 127:%x",__func__, ptr[127]);
 TRACE(3,"%s 128:%x",__func__, ptr[128]);
 TRACE(3,"%s 129:%x",__func__, ptr[129]);
 TRACE(3,"%s 130:%x",__func__, ptr[130]);

  while(1);
#endif
   test_draw_gradient(0,  &buffer);
   test_draw_gradient_01(0,  &buffer);
   while(1);
  

ErrorHandler:
    cleanup();
    GPU_REP("%s test pass", __func__);

    return 0;
}


