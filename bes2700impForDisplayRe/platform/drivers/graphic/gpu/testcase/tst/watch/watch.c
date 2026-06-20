//
//  watch.c
//  watch
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "watch_paths.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define IS_ERROR(status)         (status > 0)

#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }



static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static float fb_scale = 1.0f;
static void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}


int watch_main_entry(int argc, const char * argv[])
{
    int i;
    char casename[20];
    uint32_t feature_check = 0;
    vg_lite_matrix_t matrix;
    TC_INIT
    GPU_REP("%s", __func__);

#ifdef GPU_TEST_LOOP
 __test_again:
#endif

    buffer.width  = GPU_FB_WIDTH;
    buffer.height = GPU_FB_HEIGHT;

    // Initialize vglite.
    vg_lite_error_t error = vg_lite_init(buffer.width, buffer.height);

    if (error) {
        printf("vg_lite engine init failed: vg_lite_init() returned error %d\n", error);
        cleanup();
        return -1;
    }
    feature_check = vg_lite_query_feature(gcFEATURE_BIT_VG_IM_INDEX_FORMAT);
    if (feature_check == 0) {
        printf("watch is not supported.\n");
        cleanup();
        return -1;
    }

    buffer.format = GPU_FB_FORMAT;

    // Allocate the buffer.
    error = vg_lite_allocate(&buffer);
    if (error) {
        printf("vg_lite_allocate() returned error %d\n", error);
        cleanup();
        return -1;
    }
    fb = &buffer;
    fb_scale = buffer.width/640.f;
    // Setup a scale at center of buffer.
    vg_lite_identity(&matrix);
    vg_lite_scale(fb_scale, fb_scale, &matrix);
    // Draw the watchs' path using the matrix.
    vg_lite_clear(fb, NULL, 0xFFFF0000);
    for (i = 0; i < pathCount; i++)
    {
        error = vg_lite_draw(fb, &path[i], VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, color_data[i]);
        if (error) {
            printf("vg_lite_draw() returned error %d\n", error);
            cleanup();
            return -1;
        }
    }
    

    sprintf(casename, "watch.cb");
    gpu_save_cmdbuffer(casename);

    TC_START
    vg_lite_finish();
    TC_END
    TC_REP(watch_gcost);

    sprintf(casename, "watch.raw");
    gpu_save_raw(casename, fb);

    gpu_set_soft_break_point();

    gpu_lcdc_display(fb);


ErrorHandler:
    // Cleanup.
    cleanup();
    GPU_REP("%s test pass", __func__);
#ifdef GPU_TEST_LOOP
    goto __test_again;
#endif
    return 0;
}
