
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
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

#undef malloc
#define malloc  gpu_malloc
#undef free
#define free  gpu_free

static  uint32_t  sram_buffer[480*240] ALIGNED(64) SRAM_BSS_LOC;
//static  uint32_t  sram_buffer[160*160] ALIGNED(64);
//static  uint32_t  sram_buffer_dest[160*160] ALIGNED(64);
static  uint32_t  sram_buffer_dest[480*240] ALIGNED(64) SRAM_BSS_LOC;
static uint32_t fb_width = 480;//160;//480;
static uint32_t fb_height = 240;// 160; //480;

extern int evo_path_test_main(int argc, const char * argv[]);
extern int draw_chiness_char_test(vg_lite_buffer_t *fb);
extern int tiger_test_main_entry(int argc, const char *argv[]);
extern int gpu_draw_tiger_test(vg_lite_buffer_t* fb);
extern int rgba8etc2_test(void);

int gpu_open(void)
{
   // gpu_memory_setup();
   // gpu_hw_reset();
   // set_gpu_done_interrupt_handler();
    vg_lite_init(fb_width, fb_height);
    return 0;
}


void gpu_close(void)
{
   vg_lite_close();
}


int gpu_allocate_buffer(vg_lite_buffer_t *buffer)
{
    return vg_lite_allocate(buffer);
}

int gpu_free_buffer(vg_lite_buffer_t *buffer)
{
    return vg_lite_free(buffer);
}

int gpu_clear(vg_lite_buffer_t *buffer, uint32_t rgba8888)
{
    //vg_lite_rectangle_t rect = { 0, 0, 160, 160 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s %d buffer:%p width :%d height: %d  stride:%d", __func__, __LINE__, buffer->memory, buffer->width, buffer->height
           ,buffer->stride );

#if 1
    error = vg_lite_clear(buffer, 0, rgba8888);
    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    //vg_lite_clear(buffer, &rect, rgba8888);
    TC_INIT
    TC_START
    vg_lite_finish();
    cpu_gpu_data_cache_invalid(buffer->memory, buffer->height * buffer->stride);

    TC_END
    TC_REP(clear_gcost);
#else

    memset(buffer->memory,0xff,buffer->height * buffer->stride );

#endif


    while(0)
    {
        uint32_t  *p = (uint32_t *)buffer->memory;
        for(int i =0 ; i < buffer->width; i += 4 ){
           GPU_REP("0x%x 0x%x 0x%x 0x%x \n", *p, *(p+1), *(p+2), *(p+3) );
           p += 4;
        }
    }

    return 0;
}


int gpu_blit(vg_lite_buffer_t *src, vg_lite_buffer_t *dest, vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);

    error = vg_lite_blit(dest, src, 0, blend, 0, filter);
    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_gcost);
    while(0)
     {
        uint32_t  *p = (uint32_t *)dest->memory;
        for(int i =0 ; i < dest->width; i += 4 ){
           GPU_REP("0x%x 0x%x 0x%x 0x%x \n", *p, *(p+1), *(p+2), *(p+3) );
           p += 4;
        }
    }

    return error;
}

int gpu_blit_rect(vg_lite_buffer_t *src, vg_lite_buffer_t *dest, vg_lite_blend_t blend,
                  vg_lite_rectangle_t *srect, vg_lite_rectangle_t *drect )
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_float_t scale_x =1.0;
    vg_lite_float_t scale_y =1.0;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);

    vg_lite_identity(&matrix);

    if (drect){
        if ((drect->width != srect->width)  || (drect->height != srect->height)){ //need scaling
            scale_x = (vg_lite_float_t) drect->width/(vg_lite_float_t) srect->width;
            scale_y = (vg_lite_float_t) drect->height/(vg_lite_float_t) srect->height;
            vg_lite_scale(scale_x, scale_y, &matrix);
        }

        if (drect->x * drect->y != 0){// need trans offset position
           vg_lite_translate(drect->x,drect->y, &matrix);
        }
    }


    error = vg_lite_blit_rect(dest, src, srect, &matrix, blend, 0, filter);
    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_gcost);

    return error;
}






static int gpu_blit_scale_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                vg_lite_float_t scale_x, vg_lite_float_t scale_y,
                                vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);
    vg_lite_identity(&matrix);
    vg_lite_scale(scale_x, scale_x, &matrix);
    error = vg_lite_blit(dest, src, &matrix, blend, 0, filter);
    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_scale_gcost);

    return error;
}
/*

 c2(move) (x,y)   c5(line to) (+w,0)
    x------------>--------X
    |                     |
 c5 |                     |c5(line to (+0,+h))
 (line to (+0,-h))        |
    |                     |
    ^                     V
    |                     |
    x-------<-------------x
    (0,h)       (w,h)
       c5(line to (-w,+0))
*/


static float rect_path_data[] = {
    0.000000f,  0.000000f,
    32.000000f, 0.000000f,
    0.000000f,  200.000000f,
    -32.000000f,0.000000f,
    0.000000f,  -200.000000f,
};


static uint8_t cmd[] = {
 2,
 5,
 5,
 5,
 5,
 0,
};



static void gpu_set_img_rectpath_data(vg_lite_buffer_t *image, float *data)
{
    data[0] = 0.000000f;
    data[1] = 0.000000f;
    data[2] = (float)image->width;
    data[3] = 0.000000f;
    data[4] = 0.000000f;
    data[5] = (float)image->height;
    data[6] = -1.0f *(float)image->width;
    data[7] = 0.000000f;
    data[8] = -1.0f * (float)image->height;
    data[9] = 0.000000f;
}



static int gpu_draw_scale_blend_pattern(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                vg_lite_float_t scale_x, vg_lite_float_t scale_y,
                                vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t matPath;
    vg_lite_path_t   path;
    uint32_t data_size;

    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);
    vg_lite_identity(&matrix);
    vg_lite_scale(scale_x, scale_x, &matrix);

    vg_lite_identity(&matPath);
    vg_lite_scale(scale_x, scale_x, &matPath);

    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    gpu_set_img_rectpath_data(src,rect_path_data);
    error = vg_lite_path_append(&path, cmd, rect_path_data, sizeof(cmd));

    if (error){
        free(path.path);
        GPU_REP("%s path append error:%d", __func__, error);
        return error;
    }

    error = vg_lite_draw_pattern(dest, &path, VG_LITE_FILL_EVEN_ODD, &matPath, src, &matrix, blend, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter);

    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(gpu_draw_scale_blend_pattern_gcost);

    return error;
}




static int gpu_blit_rotation_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                   vg_lite_float_t degree, vg_lite_blend_t blend )
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_identity(&matrix);
    vg_lite_translate(dest->width/2 ,dest->height/2,&matrix);
    vg_lite_rotate(degree, &matrix);
    error = vg_lite_blit(dest, src, &matrix, blend, 0, filter);
    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_gcost);

    return error;
}



static int gpu_draw_pattern(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                   vg_lite_float_t degree, vg_lite_blend_t blend )
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t matPath;
    vg_lite_path_t   path;
    uint32_t data_size;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START

    vg_lite_identity(&matrix);
    vg_lite_translate(dest->width/2 ,dest->height/2,&matrix);
    vg_lite_rotate(degree, &matrix);

    vg_lite_identity(&matPath);
    vg_lite_translate(dest->width/2 ,dest->height/2,&matPath);
    vg_lite_rotate(degree, &matPath);

    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    gpu_set_img_rectpath_data(src,rect_path_data);
    error = vg_lite_path_append(&path, cmd, rect_path_data, sizeof(cmd));

    if (error){
        free(path.path);
        GPU_REP("%s path append error:%d", __func__, error);
        return error;
    }

    error = vg_lite_draw_pattern(dest, &path, VG_LITE_FILL_EVEN_ODD, &matPath, src, &matrix, blend, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter);

    if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_draw_pattern_gcost);
    free(path.path);
    return error;
}



static int gpu_blit_transall_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                   vg_lite_matrix_t *matrix, vg_lite_blend_t blend )
{
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__,   src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__,  src->memory, src->width, src->height);

    error = vg_lite_blit(dest, src, matrix, blend, 0, filter);
     if (error){
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_gcost);
    return error;

}

#define TEST_ALIGMENT  16
static void create_imgA4(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t*)buffer->memory;
    uint8_t values[] = {
    0x00, 0x11, 0x22, 0x33,
    0x44, 0x55, 0x66, 0x77,
    0x88, 0x99, 0xaa, 0xbb,
    0xcc, 0xdd, 0xee, 0xff
    };


    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % TEST_ALIGMENT], buffer->stride);
        p += buffer->stride;
    }
}

static void create_imgA8(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint8_t *p = (uint8_t*)buffer->memory;

    for (i = 0; i < buffer->height; i++)
    {
        memset(p, i, buffer->stride);
        p += buffer->stride;
    }
}

static vg_lite_buffer_t src;
static vg_lite_buffer_t dest;
static vg_lite_buffer_t sram_src;
static vg_lite_buffer_t sram_dest;

void gpu_performance_test(void)
{

   //tiger_test_main_entry(1,0);
   evo_path_test_main(1,0);
   //GPU_REP("after glyphs test");
   gpu_open();

   rgba8etc2_test();

   src.width  = fb_width;
   src.height = fb_height;
   src.format = VG_LITE_RGBA8888;

   dest.width  = fb_width;
   dest.height = fb_height;
   dest.format = VG_LITE_RGBA8888;

   sram_src.width  = fb_width;
   sram_src.height = fb_height;
   sram_src.format = VG_LITE_RGBA8888;

   sram_dest.width  = fb_width;
   sram_dest.height = fb_height;
   sram_dest.format = VG_LITE_RGBA8888;

   gpu_allocate_buffer(&src);
   gpu_allocate_buffer(&dest);

   memcpy(&sram_src,&src, sizeof(vg_lite_buffer_t));
   sram_src.memory = sram_buffer;
   sram_src.address = sram_src.memory;
   memcpy(&sram_dest,&src, sizeof(vg_lite_buffer_t));
   sram_dest.memory = sram_buffer_dest;
   sram_dest.address = sram_dest.memory;

   gpu_clear(&src, 0xffff0000);
   gpu_clear(&dest, 0xffff0000);

   /*
   对目标矩形区域设置特定颜色#0A59F7
   */

   hal_sys_timer_delay_us(200000);


   TRACE(3,"set color to sram buffer 0x0A59F7");

   gpu_clear(&sram_src, 0x0A59F7);


   /*
   将矢量汉字"龍"渲染到目标区域，颜色值#0A59F7
   */
   TRACE(3,"draw chiness char  to sram buffer ");

   draw_chiness_char_test(&sram_dest);


   TRACE(3,"draw chiness char  to psram buffer ");

   draw_chiness_char_test(&dest);

/*
   draw tiger in psram
*/

   TRACE(3,"draw tiger to psram buffer w*d: %d * %d", dest.width , dest.height);
   gpu_draw_tiger_test(&dest);

/*
   draw tiger in sram
*/

   TRACE(3,"draw tiger to sram buffer w*d: %d * %d", sram_dest.width , sram_dest.height);
   gpu_draw_tiger_test(&sram_dest);

   #if 0
   gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);
   gpu_blit(&sram_src, &dest, VG_LITE_BLEND_NONE);
   gpu_blit(&sram_src, &src, VG_LITE_BLEND_NONE);
   gpu_blit(&src, &sram_src, VG_LITE_BLEND_NONE);
   #endif
   /*
   将源矩形区域copy到目标区域
   */
   sram_src.format = VG_LITE_RGB565;
   sram_src.stride /= 2;
   TRACE(3,"blit rgb565->rgba8888 sram->sram");
   gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);



   /*
   将源矩形区域copy到目标区域
   */

   src.format = VG_LITE_RGB565;
   src.stride /= 2;
   TRACE(3,"blit rgb565->rgba8888 psram->sram");
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

   /*
   将源区域混合到目标区域
   */
   sram_src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   sram_src.format = VG_LITE_RGBA8888;
   sram_src.stride *= 2;
   TRACE(3,"blit blend rgba8888->rgba8888 sram->sram");
   gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    /*
   将源区域混合到目标区域
   */
   src.width  = fb_width;
   src.height = fb_height;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   src.format = VG_LITE_RGBA8888;
   src.stride = src.width * 4;
   TRACE(3,"blit blend rgba8888->rgba8888 psram->sram");
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   /*
   将源区域混合到目标区域
   */
   sram_src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   sram_src.width  = 48;
   sram_src.height = 48;
   sram_src.format = VG_LITE_A4;
   sram_src.stride = 48/2;
   create_imgA4(&sram_src);
   TRACE(3,"blit blend A4->rgba8888 sram->sram");
   gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   /*
   将源区域混合到目标区域
   */
   src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   src.width  = 48;
   src.height = 48;
   src.format = VG_LITE_A4;
   src.stride = 48/2;
   create_imgA4(&src);
   TRACE(3,"blit blend A4->rgba8888 psram->sram");
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   /*
   将源区域混合到目标区域
   */
   src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   src.width  = 48;
   src.height = 48;
   src.format = VG_LITE_A8;
   src.stride = 48;
   create_imgA8(&src);
   TRACE(3,"blit blend A8->rgba8888 psram->sram");
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   hal_sys_timer_delay_us(200000);

/*
将源区域的一部分和目标区域的一部分进行混合
源区域：起始位置 48,48，结束位置 368,368
目标区域：起始位置32,32，结束位置352,352
*/
   vg_lite_rectangle_t srect;
   vg_lite_rectangle_t drect;
   srect.x = 48;
   srect.y = 48;
   srect.width = 320;
   srect.height = 320;
   drect.x = 32;
   drect.y = 32;
   drect.width = 320;
   drect.height =320;

   src.width  = fb_width;
   src.height = fb_height;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   src.stride = src.width * 4;

   sram_dest.width  = fb_width;
   sram_dest.height = fb_height;
   sram_dest.format = VG_LITE_RGBA8888;
   sram_dest.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
   sram_dest.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   sram_dest.stride = src.width * 4;

   TRACE(3,"blit rect  psram->sram");

   gpu_blit_rect(&src, &sram_dest,VG_LITE_BLEND_SRC_OVER,&srect,&drect );

/*
将源区域设置全局透明度后混合到目标区域(1600 不支持)
*/
/*
将源区域缩放后混合到目标区域(低质量) 0.7倍
*/



   src.width  = fb_width;
   src.height = fb_height;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 0.7 0.7 blend  psram->sram");
   gpu_blit_scale_blend(&src, &sram_dest, 0.7, 0.7,VG_LITE_BLEND_SRC_OVER);


   src.width  = fb_width;
   src.height = fb_height;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 0.7 0.7 blend  psram->psram");
   gpu_blit_scale_blend(&src, &dest, 0.7, 0.7,VG_LITE_BLEND_SRC_OVER);

/*
将源区域缩放后混合到目标区域(高质量) 0.7倍
*/

   src.width  = fb_width;
   src.height = fb_height;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 0.7 0.7 blend  psram->sram");
   gpu_draw_scale_blend_pattern(&src, &sram_dest, 0.7, 0.7,VG_LITE_BLEND_SRC_OVER);


   src.width  = fb_width;
   src.height = fb_height;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 0.7 0.7 blend  psram->psram");
   gpu_draw_scale_blend_pattern(&src, &dest, 0.7, 0.7,VG_LITE_BLEND_SRC_OVER);



/*
将源区域缩放后混合到目标区域(低质量) 1.7倍
*/
   src.width  = 240;
   src.height = 120;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 1.7 1.7 blend psram->sram");
   gpu_blit_scale_blend(&src, &sram_dest, 1.7, 1.7,VG_LITE_BLEND_SRC_OVER);

   src.width  = 240;
   src.height = 120;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 1.7 1.7 blend psram->psram");
   gpu_blit_scale_blend(&src, &dest, 1.7, 1.7,VG_LITE_BLEND_SRC_OVER);
/*
将源区域缩放后混合到目标区域(高质量) 1.7倍
*/
   src.width  = 240;
   src.height = 120;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 1.7 1.7 blend psram->sram");
   gpu_draw_scale_blend_pattern(&src, &sram_dest, 1.7, 1.7,VG_LITE_BLEND_SRC_OVER);

   src.width  = 240;
   src.height = 120;
   src.stride = src.width * 4;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit scale 1.7 1.7 blend psram->sram");
   gpu_draw_scale_blend_pattern(&src, &dest, 1.7, 1.7,VG_LITE_BLEND_SRC_OVER);

/*
将源区域旋转后混合到目标区域(低质量) 89度
*/
   vg_lite_matrix_t matrix;

   src.width  = 160;//128;//160;
   src.height = 160;//128;//160;
   src.stride = src.width * 4 ;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit rotate 89.0 blend psram->sram");
   gpu_blit_rotation_blend(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   src.width  = 32;
   src.height = 240;
   src.stride = src.width*4 ;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit rotate 89.0 blend psram->sram");
   gpu_blit_rotation_blend(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
   gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

   sram_src.width  = 32;
   sram_src.height = 240;
   sram_src.stride = src.width*4 ;
   sram_src.format = VG_LITE_RGBA8888;
   sram_src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"blit rotate 89.0 blend sram->sram");
   gpu_blit_rotation_blend(&sram_src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
   gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

/*
将源区域旋转后混合到目标区域(高质量) 89度
*/
   src.width  = 32;
   src.height = 240;
   src.stride = src.width*4 ;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   TRACE(3,"draw pattern 89.0 psram->sram");
   gpu_draw_pattern(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
/*
将源区域采用3*3矩阵变换后混合到目标区域(高质量)
变换矩阵要求除a33外都有值。
*/
   src.width  = 240;
   src.height = 240;
   src.stride = 240*4 ;
   src.format = VG_LITE_RGBA8888;
   src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
   src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
   //vg_lite_matrix_t matrix;
   vg_lite_identity(&matrix);
   vg_lite_translate(64,32,&matrix);
   vg_lite_scale(0.5, 0.5, &matrix);
   vg_lite_rotate(30.0, &matrix);

   TRACE(3,"blit translate scale rotate blend psram->sram");
   gpu_blit_transall_blend(&src, &sram_dest,&matrix,VG_LITE_BLEND_SRC_OVER);

   hal_sys_timer_delay_us(200000);

/*
  rgba8_etc2 test
*/

// rgba8etc2_test();



#if 1

   src.width  = 240;
   src.height = 240;
   src.stride = 240*4 ;

   sram_src.width  = 240;
   sram_src.height = 240;
   sram_src.stride = 240*4 ;

   dest.width  = 240;
   dest.height = 240;
   dest.stride = 240*4 ;

   sram_dest.width  = 240;
   sram_dest.height = 240;
   sram_dest.stride = 240*4 ;


  #if 0
   vg_lite_float_t rscale = 0.1;
   for (int i = 0; i < 10; i++){
     rscale *= i;
     gpu_blit_scale(&src, &dest, rscale, rscale);
     gpu_blit_scale(&sram_src, &dest, rscale, rscale);
     gpu_blit_scale(&sram_src, &src, rscale, rscale);
     gpu_blit_scale(&sram_src, &sram_dest, rscale, rscale);
     gpu_blit_scale(&src, &sram_src, rscale, rscale);
   }

  #endif


#if 0
   vg_lite_float_t degree = 1.0;

   for (int i = 0; i <= 90; i++){
     degree = 1.0 * i;

     TRACE(3,"rotate degree: %d /10", (int)degree * 10);
     gpu_blit_rotation_blend(&src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
     gpu_blit_rotation_blend(&sram_src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
     gpu_blit_rotation_blend(&sram_src, &sram_dest, degree,VG_LITE_BLEND_SRC_OVER);
     gpu_blit_rotation_blend(&src, &sram_dest, degree,VG_LITE_BLEND_SRC_OVER);
     hal_sys_timer_delay_us(200000);
   }
#endif

#endif
   gpu_free_buffer(&src);
   gpu_free_buffer(&dest);
   //gpu_free_buffer(&srambuf);

   gpu_close();

   TRACE(3,"test pass");
}


