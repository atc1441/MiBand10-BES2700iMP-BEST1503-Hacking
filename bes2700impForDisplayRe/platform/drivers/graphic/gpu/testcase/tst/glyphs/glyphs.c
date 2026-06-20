#define DC 0
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vg_lite_util.h"
#include "Elm.h"
#include <stdlib.h>
#if DC
#include "viv_dc_util.h"
#include "viv_dc_setting.h"
#endif

#include "hal_trace.h"
#include "gpu_port.h"
#include "hal_cache.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#define printf  GPU_REP

typedef  struct appAttribs
{
    int winWidth;
    int winHeight;
    int frameCount;
};

static ElmBuffer buffer;

#if DC
static ElmBuffer buffer_dc;
#else
static vg_lite_buffer_t *sys_fb;
static ElmBuffer fb;
#endif

static ElmHandle ego_handle_bg = ELM_NULL_HANDLE;

//static struct appAttribs cmdAttrib = {1080,760, 1/*100*/};
static struct appAttribs cmdAttrib = {480,480, 1/*100*/};

static void cleanup(void)
{
#if !DC
    if (sys_fb != NULL)
    {
        ElmDestroyBuffer(fb);
        vg_lite_fb_close(sys_fb);
    }
#endif
    ElmDestroyBuffer(buffer);
    ElmDestroyObject(ego_handle_bg);
    ElmTerminate();
}

static int render(ElmBuffer buffer, ElmHandle object)
{
    int status = 0;
#if 0
	//status = ElmScale(object, 5,5);
	 if (!status) {
        printf("ElmScale() failed:");
        cleanup();
        status = -1;
        return status;
    }
 #endif
    status = ElmDraw(buffer, object);
    if (!status) {
        printf("ElmDraw() failed:");
        cleanup();
        status = -1;
        return status;
    }
    //ElmFinish();
    return status;

}

/*
* get value of cmd argument.
*/
static void getArgument(const char *arguments, char *dest, int *i, int len)
{
    int j = 0;
    while (arguments[*i] == ' ')
    {
        ++(*i);
    }

    while (arguments[*i] != ' ' && (*i) < len)
    {
        dest[j] = arguments[*i];
        ++j;
        ++(*i);
    }
    dest[j] = '\0';
}

/*
* Parse the command line.
*/
static int ParseArgs(const char *arguments)
{
    int     len = 0;
    int     i = 0;
    int     result = 1;
    char strTemp[256];

    len = (int)strlen(arguments);

    if (len > 0)
    {
        while (i < len)
        {
            if (arguments[i] == ' ')
            {
                ++i;
                continue;
            }

            if ((arguments[i] == '-'))
            {
                ++i;
                getArgument(arguments, strTemp, &i, len);
                if (strcmp(strTemp, "w") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.winWidth = atoi(strTemp);
                }else if (strcmp(strTemp, "h") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.winHeight = atoi(strTemp);
                }else if (strcmp(strTemp, "frameCount") == 0)
                {
                    ++i;
                    getArgument(arguments, strTemp, &i, len);
                    cmdAttrib.frameCount = atoi(strTemp);
                }else
                {
                    printf("can't recognise command parameter");
                    result = 0;
                }
            }
            else
            {
                result = 0;
                break;
            }
        }
    }

    return result;
}

static ELM_BUFFER_FORMAT _buffer_format_to_Elm(vg_lite_buffer_format_t format)
{
    switch (format) {
        case VG_LITE_RGBA8888:
            return  ELM_BUFFER_FORMAT_RGBA8888;
            break;

        case VG_LITE_RGBX8888:
            return ELM_BUFFER_FORMAT_RGBX8888;
            break;

        case VG_LITE_BGRA8888:
            return ELM_BUFFER_FORMAT_BGRA8888;
            break;

        case VG_LITE_BGRX8888:
            return ELM_BUFFER_FORMAT_BGRX8888;
            break;

        case VG_LITE_RGB565:
            return ELM_BUFFER_FORMAT_RGB565;
            break;

        case VG_LITE_BGR565:
            return ELM_BUFFER_FORMAT_BGR565;
            break;

        case VG_LITE_RGBA4444:
            return ELM_BUFFER_FORMAT_RGBA4444;
            break;

        case VG_LITE_BGRA4444:
            return ELM_BUFFER_FORMAT_BGRA4444;
            break;

        default:
            return  ELM_BUFFER_FORMAT_RGBA8888;
            break;
    }
}
extern uint32_t __evo_fontsdata_start_flash[];
extern uint32_t __evo_fontsdata_end_flash[];
static char param[512] = {'\0'};

int glyphs_test_entry_main(int argc, const char * argv[])
{
    int status = 0;
    int count = 0;
    int i, count_s = 0;
    float angle_s = 0.f, angle_m = 0.f, angle_h = 0.f;
    //TC_INIT
#if !DC
    ELM_BUFFER_FORMAT format = 0;
#endif

#if 0
    for (i=1; i < argc; i++)
    {
        strcat(param, argv[i]);
        strcat(param, " ");
    }

    if (!ParseArgs(param))
    {
        printf("%s","input parameter error\n");
        return -1;
    }
#endif
    //status = ElmInitialize(1080,760);
    status = ElmInitialize(480,480);
    if (!status)
    {
        printf("Elm init failed:");
        cleanup();
        return -1;
    }
#if DC
    buffer = ElmCreateBuffer(cmdAttrib.winWidth, cmdAttrib.winHeight, ELM_BUFFER_FORMAT_BGRA8888);
    buffer_dc = ElmCreateBuffer(cmdAttrib.winWidth, cmdAttrib.winHeight, ELM_BUFFER_FORMAT_BGRA8888);
#else
    buffer = ElmCreateBuffer(cmdAttrib.winWidth, cmdAttrib.winHeight, ELM_BUFFER_FORMAT_RGBA8888);
    sys_fb = vg_lite_fb_open();

    if (sys_fb != NULL)
    {
        format = _buffer_format_to_Elm(sys_fb->format);
        fb = ElmWrapBuffer(sys_fb->width, sys_fb->height, sys_fb->stride,
                           sys_fb->memory, sys_fb->address, format);
    }
#endif

	//ego_handle_bg = ElmCreateObjectFromFile(ELM_OBJECT_TYPE_EGO, "glyphs_complex_noaa.evo");

    #if 0
	ego_handle_bg = ElmCreateObjectFromFile(ELM_OBJECT_TYPE_EGO, "glyphs_complex_msaa4x4.evo");
    #else
	ego_handle_bg = ElmCreateObjectFromData(ELM_OBJECT_TYPE_EGO, __evo_fontsdata_start_flash
                        ,__evo_fontsdata_end_flash - __evo_fontsdata_start_flash);
    #endif

    //ElmCreateObjectFromData(ELM_OBJECT_TYPE type, void *data, int size)

    printf("RS: %dx%d frameCount:%d\n",cmdAttrib.winWidth, cmdAttrib.winHeight, cmdAttrib.frameCount);
#if DC
    {
        Config config = {0};

        uint32_t address = ElmGetBufferAddress(buffer_dc);
        /* Initialize DC */
        if (viv_dc_device_init() < 0)
        {
            printf("init_dc() failed\n");
            cleanup();
            return -1;
        }

        if (viv_util_prepare_framebuffer_config(
            &config.framebuffer,
            cmdAttrib.winWidth, cmdAttrib.winHeight,
            vivARGB8888,
            vivLINEAR,
            address) != vivSTATUS_OK)
        {
            printf("viv_util_prepare_framebuffer_config() failed\n");
            viv_dc_device_deinit();
            cleanup();
            return -1;
        }

        printf("wyh====%s:%d address:%d\n",__FUNCTION__,__LINE__,address);
        config.output_type = vivDPI;
        config.output_format = vivD24;
        config.framebuffer.visible_width = config.framebuffer.width;
        config.framebuffer.total_width = config.framebuffer.width + 160;
        config.framebuffer.hsync_start = config.framebuffer.width + 16;
        config.framebuffer.hsync_end = config.framebuffer.hsync_start + 96;
        config.framebuffer.visible_height = config.framebuffer.height;
        config.framebuffer.total_height = config.framebuffer.height + 45;
        config.framebuffer.vsync_start = config.framebuffer.height + 10;
        config.framebuffer.vsync_end = config.framebuffer.vsync_start + 2;

        printf("wyh====%s:%d\n",__FUNCTION__,__LINE__);
        viv_reset_framebuffer(&config);
        viv_set_framebuffer(&config);
        viv_set_display(&config);
        viv_set_panel(&config);
        viv_set_output(&config, vivTRUE);
        viv_set_gamma(&config, vivFALSE, 0, 0, 0, 0);
        viv_set_dither(&config, vivFALSE, vivARGB8888, 0, 0);
        viv_set_cursor(&config, vivFALSE);
        viv_set_commit(&config);

        /* If setting will not be changed. Yon can deinit DC first. */
        viv_dc_device_deinit();
    }
#endif
     //printf("%s %d", __func__, __LINE__);

    for (count = 0; count < cmdAttrib.frameCount; count++)
    {
        ElmClear(buffer, 0xFFFFFFFF, 0, 0, 0, 0, 1);
        ElmFinish(); //
        hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
       //  printf("%s %d", __func__, __LINE__);

       // TC_START
        hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
        status = render(buffer, ego_handle_bg);
        ElmFinish(); //

        hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
        printf("%s %d", __func__, __LINE__);
        {
             uint32_t * p= (uint32_t *)0x3c02f000;
             int c ;
            for (c= 0; c < 230400; c++){
                 if (*(p+c) != 0xffffffff){
                   //  printf("i %d: 0x%x", c,*(p+c));
                 }
            }
            printf("i %d:", c);

        }

     while(1);
        if (status == -1)
        {
            printf("%s :%d render failed!\n",__FUNCTION__,__LINE__);
            return -1;
        }
        hal_cache_invalidate_all(HAL_CACHE_ID_D_CACHE);
		 //ElmSaveBuffer(buffer, "glyphs_complex_noaa.png");
#if DC
        {
            status = ElmDraw(buffer_dc, buffer);
            if (!status)
            {
                printf("ElmDraw failed:");
                cleanup();
                return -1;
            }
        }
#else
        if (sys_fb != NULL)
        {
            status = ElmDraw(fb, buffer);
            if (!status)
            {
                printf("ElmDraw failed:");
                cleanup();
                return -1;
            }
        }
#endif
    }
    // printf("%s %d", __func__, __LINE__);
    //TC_END
    //TC_REP("gyphs test");
    cleanup();
    return 0;
}
