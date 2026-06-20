#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "gpu_port.h"
#include "gpu_common.h"

#undef malloc
#define malloc  gpu_malloc
#undef free
#define free  gpu_free

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#define printf  GPU_REP



#define DEFAULT_SIZE   480.0f;
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
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
#if 0   // O
static uint8_t cmd[] = {
 2, 5,
 5, 5, 5, 5,
 //5, 5, 5, 1,
 5, 5, 5,
 2, 5, 5, 5,
 5, 0,
};
/*
static float path_data[] = {
 39.000000f,     35.000000f,
 7.000000f,      -5.000000f,     0.000000f,      -24.000000f,
 -7.000000f,     -6.000000f,     -22.000000f,    0.000000f,
 -8.000000f,     6.000000f,      0.000000f,      24.000000f,
 8.000000f,      5.000000f,      22.000000f,     0.000000f,

 33.000000f,     27.000000f,
 -10.000000f,    0.000000f, 0.000000f,      -17.000000f,
  10.000000f,     0.000000f, 0.000000f,      17.000000f,
};
*/
static float path_data[] = {
 39.000000f,     35.000000f,
 7.000000f,      -5.000000f,     0.000000f,      -24.000000f,
 -7.000000f,     -6.000000f,     -22.000000f,    0.000000f,
 -8.000000f,     6.000000f,      0.000000f,      24.000000f,
 8.000000f,      5.000000f,      22.000000f,     0.000000f,

 33.000000f,     27.000000f,
 -10.000000f,    0.000000f, 0.000000f,      -17.000000f,
  10.000000f,     0.000000f, 0.000000f,      17.000000f,
};
#endif



#if 1  // 龙

static uint8_t cmd[] = {
 2, 5,
 9, 9,  2,
 5, 5, 5, 5,
  2, 5, 5,
 5, 5,  2,
 5, 5, 9, 5,
 9, 9, 9, 5,
 7, 9, 5, 5,
 5, 9, 5, 5,
 5, 5,  2,
 5, 7, 9, 5,
 5, 5,  2,
 5, 7, 9, 5,
 5, 5, 9, 5,
 9, 9, 5,
 2, 5, 9, 9,
  2, 5, 9,
 9, 5, 9, 5,
 5, 5, 5, 5,
 5, 5, 9, 5,
 5, 9, 5, 5,
 5, 7, 9, 5,
 5, 5, 5, 5,
 9, 5, 9, 5,
 5, 5, 5, 5,
 5, 7, 9, 5,
 5, 5, 5, 7,
 9, 5, 5, 5,
 5, 7, 9, 5,
 5, 9, 5, 9,
 9, 9, 5, 0,
};

//PSRAM_DATA_LOC static float path_data[] = {
const static float path_data[] = {
 8.120120f,	 2.480470f,
 0.439453f,	 -0.280273f,	 6.240230f,	 2.200200f,
 3.720700f,	 6.240230f,	 1.600590f,	 4.439450f,
 -0.240234f,	 -1.319340f,	 -1.280270f,	 -3.159180f,
 -2.040040f,	 -4.159180f,	 6.320310f,	 28.879900f,
 9.160160f,	 0.000000f,	 0.000000f,	 -3.879880f,
 -9.160160f,	 0.000000f,	 0.000000f,	 3.879880f,
 15.480500f,	 20.280300f,	 -9.160160f,	 0.000000f,
 0.000000f,	 3.519530f,	 9.160160f,	 0.000000f,
 0.000000f,	 -3.519530f,	 15.080100f,	 19.120100f,
 1.320310f,	 -1.560550f,	 3.279300f,	 2.480470f,
 -0.239258f,	 0.280273f,	 -0.879883f,	 0.639648f,
 -1.679690f,	 0.759766f,	 0.000000f,	 14.919900f,
 0.000000f,	 2.000000f,	 -0.400391f,	 3.120120f,
 -3.599610f,	 3.400390f,	 -0.080078f,	 -0.719727f,
 -0.240234f,	 -1.280270f,	 -0.600586f,	 -1.639650f,
 -0.479492f,	 -0.360352f,	 -1.160160f,	 -0.640625f,
 -2.559570f,	 -0.840820f,	 0.000000f,	 -0.599609f,
 2.799800f,	 0.200195f,	 3.479490f,	 0.200195f,
 0.600586f,	 0.000000f,	 0.760742f,	 -0.240234f,
 0.760742f,	 -0.720703f,	 0.000000f,	 -5.479490f,
 -9.160160f,	 0.000000f,	 0.000000f,	 8.160160f,
 0.000000f,	 0.319336f,	 -1.000000f,	 0.919922f,
 -2.080080f,	 0.919922f,	 -0.400391f,	 0.000000f,
 0.000000f,	 -21.280300f,	 2.679690f,	 1.280270f,
 8.560550f,	 0.000000f,	 15.759800f,	 7.360350f,
 1.720700f,	 -2.240230f,	 1.959960f,	 1.599610f,
 3.239260f,	 2.759770f,	 -0.120117f,	 0.440430f,
 -0.479492f,	 0.639648f,	 -1.040040f,	 0.639648f,
 -17.239300f,	 0.000000f,	 -0.280273f,	 -1.159180f,
 13.599600f,	 0.000000f,	 16.320299f,	 14.799800f,
 1.719730f,	 -2.200200f,	 1.959960f,	 1.560550f,
 3.200200f,	 2.760740f,	 -0.120117f,	 0.439453f,
 -0.480469f,	 0.639648f,	 -1.040040f,	 0.639648f,
 -18.200199f,	 0.000000f,	 -0.320312f,	 -1.200200f,
 10.640600f,	 0.000000f,	 0.639648f,	 -1.839840f,
 1.399410f,	 -4.439450f,	 1.839840f,	 -6.280270f,
 3.759770f,	 1.120120f,	 -0.160156f,	 0.440430f,
 -0.679688f,	 0.760742f,	 -1.479490f,	 0.720703f,
 -0.800781f,	 1.319340f,	 -2.000000f,	 3.000000f,
 -3.080080f,	 4.439450f,	 2.959960f,	 0.000000f,
 5.120120f,	 9.000000f,	 0.439453f,	 -0.200195f,
 5.920900f,	 3.040040f,	 3.400390f,	 7.320310f,
 1.360350f,	 5.320310f,	 -0.080078f,	 -1.639650f,
 -1.000000f,	 -3.759770f,	 -1.799800f,	 -5.120120f,
 37.360401f,	 32.679699f,	 0.120117f,	 3.240230f,
 0.959961f,	 0.320312f,	 1.279300f,	 0.560547f,
 1.279300f,	 1.080080f,	 0.000000f,	 1.240230f,
 -1.399410f,	 1.639650f,	 -8.279300f,	 1.639650f,
 -4.720700f,	 0.000000f,	 -3.399410f,	 0.000000f,
 -4.160160f,	 -0.639648f,	 -4.160160f,	 -2.799800f,
 0.000000f,	 -19.639601f,	 3.000000f,	 1.280270f,
 8.360350f,	 0.000000f,	 0.000000f,	 -4.160160f,
 -8.400390f,	 0.000000f,	 -0.959961f,	 1.080080f,
 -2.879880f,	 -1.600590f,	 0.240234f,	 -0.240234f,
 0.640625f,	 -0.559570f,	 1.040040f,	 -0.799805f,
 0.000000f,	 -9.559570f,	 4.160160f,	 0.559570f,
 -0.120117f,	 0.400391f,	 -0.439453f,	 0.759766f,
 -1.639650f,	 0.919922f,	 0.000000f,	 3.520510f,
 8.359380f,	 0.000000f,	 1.800780f,	 -2.360350f,
 2.159180f,	 1.679690f,	 3.439450f,	 2.879880f,
 -0.120117f,	 0.440430f,	 -0.519531f,	 0.639648f,
 -1.040040f,	 0.639648f,	 -12.559600f,	 0.000000f,
 0.000000f,	 3.520510f,	 8.279300f,	 0.000000f,
 1.400390f,	 -1.520510f,	 3.160160f,	 2.400390f,
 -0.240234f,	 0.320312f,	 -0.919922f,	 0.639648f,
 -1.679690f,	 0.799805f,	 0.000000f,	 4.959960f,
 -0.040039f,	 0.240234f,	 -1.280270f,	 0.799805f,
 -2.080080f,	 0.799805f,	 -0.400391f,	 0.000000f,
 0.000000f,	 -0.919922f,	 -8.879880f,	 0.000000f,
 0.000000f,	 3.400390f,	 7.879880f,	 0.000000f,
 1.559570f,	 -1.799800f,	 1.800780f,	 1.279300f,
 2.960940f,	 2.359380f,	 -0.120117f,	 0.440430f,
 -0.480469f,	 0.640625f,	 -1.040040f,	 0.640625f,
 -11.360400f,	 0.000000f,	 0.000000f,	 3.240230f,
 7.679690f,	 0.000000f,	 1.560550f,	 -1.800780f,
 1.719730f,	 1.280270f,	 2.919920f,	 2.320310f,
 -0.120117f,	 0.440430f,	 -0.520508f,	 0.639648f,
 -1.040040f,	 0.639648f,	 -11.120100f,	 0.000000f,
 0.000000f,	 3.280270f,	 8.439450f,	 0.000000f,
 1.600590f,	 -1.799800f,	 1.759770f,	 1.280270f,
 2.959960f,	 2.360350f,	 -0.120117f,	 0.439453f,
 -0.480469f,	 0.639648f,	 -1.040040f,	 0.639648f,
 -11.960000f,	 0.000000f,	 0.000000f,	 3.160160f,
 0.000000f,	 0.759766f,	 0.280273f,	 1.040040f,
 2.040040f,	 1.040040f,	 4.519530f,	 0.000000f,
 1.959960f,	 0.000000f,	 3.560550f,	 -0.040039f,
 4.200200f,	 -0.080078f,	 0.440430f,	 -0.040039f,
 0.679688f,	 -0.120117f,	 0.879883f,	 -0.360352f,
 0.320312f,	 -0.399414f,	 0.760742f,	 -1.719730f,
 1.160160f,	 -3.200200f,	 0.480469f,	 0.000000f,
} ;

#endif


static vg_lite_path_t path;
static void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);

    vg_lite_close();
}

int evo_path_test_main(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    TC_INIT
    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = VG_LITE_RGBA8888;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    vg_lite_identity(&matrix);
    vg_lite_translate(200,200, &matrix);
    vg_lite_scale(5, 5, &matrix);
    vg_lite_rotate(80.9, &matrix);
    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 48, 48);
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_path_append(&path, cmd, path_data, sizeof(cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
    TC_START
    CHECK_ERROR(vg_lite_finish());
    TC_END
    printf(" %s fb: %p", __func__, fb->memory);
    hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
    TC_REP("draw long character cost");
    printf("fb: %p", fb->memory);

    //while(1);

ErrorHandler:
    cleanup();
    return 0;
}

int draw_chiness_char_test(vg_lite_buffer_t *fb)
{
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    TC_INIT

    vg_lite_identity(&matrix);
//    vg_lite_translate(200,200, &matrix);
//    vg_lite_scale(5, 5, &matrix);
    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 48, 48);
    path.path = malloc(data_size);
    vg_lite_path_append(&path, cmd, path_data, sizeof(cmd));

    vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF);
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
    TC_START
    vg_lite_finish();
    TC_END
    hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
    printf(" %s fb: %p", __func__, fb->memory);
    TC_REP("draw long character cost");
    vg_lite_clear_path(&path);
    return 0;
}