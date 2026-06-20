#include <stdio.h>

#include "vg_lite.h"
#include "vg_lite_util.h"

#include "util.h"
#include "Common.h"
#include "SFT.h"

extern vg_lite_buffer_t *g_fb;
extern vg_lite_rectangle_t g_fb_rect;
extern vg_lite_buffer_t g_buffer;
extern vg_lite_filter_t filter;
extern int has_fb;

unsigned int    Run_Width = 320;
unsigned int    Run_Height = 240;
int            Run_Red = 8;
int            Run_Green = 8;
int            Run_Blue = 8;
int            Run_Alpha = 8;

float    WINDSIZEX = 320.f;
float   WINDSIZEY = 240.f;


/// initialization
int init()
{
    vg_lite_error_t error;
    CHECK_ERROR(vg_lite_init((int32_t)WINDSIZEX, (int32_t)WINDSIZEY));

    filter = VG_LITE_FILTER_POINT;

    // Allocate a buffer.
    g_buffer.width = Run_Width;
    g_buffer.height = Run_Height;
    g_buffer.format = VG_LITE_RGB565;
    g_buffer.tiled = 0;
    CHECK_ERROR(vg_lite_allocate(&g_buffer));
    g_fb = &g_buffer;

    g_fb_rect.x = 0;
    g_fb_rect.y = 0;
    g_fb_rect.width = Run_Width;
    g_fb_rect.height = Run_Height;

    if (g_fb != NULL)
    {
        return 0;
    }
    else
    {
        printf("fb initialization failed.\n");
        return -1;
    }

ErrorHandler:
    return error;
}

// destroy VGLite resources
void destroy()
{

    vg_lite_free(g_fb);

    vg_lite_close();
}

int main (int argc, char** argv)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int ret;

    SFT_REFER = 0;

    if (argc > 1)
        Cmd = argv[1];

    //initialize
    ret = init();
    if (ret != 0)
        return -1;

    //Draw
    CHECK_ERROR(Render());
    //destroy
    destroy();

ErrorHandler:
    return -1;
}


