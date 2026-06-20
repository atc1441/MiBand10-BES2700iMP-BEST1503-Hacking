//-----------------------------------------------------------------------------
//Description: The test cases test the different size and position of Clear-Rectangle
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"
//YUYV can't be used as target, so only 10 formats for test.
#define NUM_FORMATS 10
/* Format array. */
static vg_lite_buffer_format_t formats[] =
{
    VG_LITE_RGBA8888,
    VG_LITE_BGRA8888,
    VG_LITE_RGBX8888,
    VG_LITE_BGRX8888,
    VG_LITE_RGB565,
    VG_LITE_BGR565,
    VG_LITE_RGBA4444,
    VG_LITE_BGRA4444,
    VG_LITE_A8,
    VG_LITE_L8,
    VG_LITE_YUYV
};

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}
//-----------------------------------------------------------------------------
//Name: Init
//Parameters: None
//Returned Value: None
//Description: 
//-----------------------------------------------------------------------------
static vg_lite_error_t Init()
{
    return VG_LITE_SUCCESS;
}

//-----------------------------------------------------------------------------
//Name: Exit
//Parameters: None
//Returned Value: None
//Description: 
//-----------------------------------------------------------------------------
static void Exit()
{
}

//-----------------------------------------------------------------------------
//Name: SFT_Clear_Exe
//Parameters: 
//x: The x-axis coordinate of clear-rectangle.
//y: The y-axis coordinate of clear-rectangle.
//w: The width of clear-rectangle
//h: The height of clear-rectangle
//c: The pointer point to the clear color
//Returned Value: None
//Description: The function clear color for the specified position with the 
//specified value. Then create random shape path and draw path
//-----------------------------------------------------------------------------
void SFT_Clear_Exe(int32_t x, int32_t y, int32_t w, int32_t h, vg_lite_color_t c)
{
    vg_lite_rectangle_t rect;
    rect.x = x;
    rect.y = y;
    rect.width = w;
    rect.height = h;

    if (VG_LITE_SUCCESS != Init())
    {
        printf("Init failed.\n");
        return;
    }

    vg_lite_clear(g_fb, &rect, c);

    Exit();
}

//-----------------------------------------------------------------------------
//Name: SFT_Clear_001
//Parameters: None
//Returned Value: None
//Description: Test buffer clear with different color & different area of the buffer.
//-----------------------------------------------------------------------------
vg_lite_error_t    SFT_Clear_001()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int x, y, w, h;
    int i;

    vg_lite_color_t cc;
    vg_lite_buffer_t buffer;
    vg_lite_rectangle_t rect;

    memset(&buffer,0,sizeof(vg_lite_buffer_t));
    buffer.width = ALIGN((int)WINDSIZEX,32);
    buffer.height = (int)WINDSIZEY;
    buffer.format = formats[0];
    buffer.stride = 0;
    buffer.handle = NULL;
    buffer.memory = NULL;
    buffer.address = 0;
    buffer.tiled  = 0;

    CHECK_ERROR(vg_lite_allocate(&buffer));


    for(i = 0; i < 16; i++)
    {
        x = (int)Random_r(0, WINDSIZEX);
        y = (int)Random_r(0, WINDSIZEY);
        w = (int)Random_r(1.0f, WINDSIZEX);
        h = (int)Random_r(1.0f, WINDSIZEY);
        cc = GenColor_r();

        rect.x = x;
        rect.y = y;
        rect.width = w;
        rect.height = h;
        printf("Clearing color: 0x%x, area: %d, %d, %d, %d \n", cc, x, y, w, h);
        CHECK_ERROR(vg_lite_clear(&buffer,NULL,0xffffffff));
        CHECK_ERROR(vg_lite_clear(&buffer,&rect,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("SFT_Clear_001_",&buffer);
        vg_lite_blit(g_fb, &buffer, NULL, 0, 0, filter);
    }

ErrorHandler:
    vg_lite_free(&buffer);
    return error;
}

//different buffer format
vg_lite_error_t    SFT_Clear_002()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i,j;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[NUM_FORMATS];

    for(i = 0; i < NUM_FORMATS; i++)
    {
        cc = GenColor_r();

        buffer[i].width = ALIGN((int)WINDSIZEX,128);
        buffer[i].height = (int)WINDSIZEY;
        buffer[i].format = formats[i];
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("clear buffer with color: 0x%x, format: 0x%x\n", cc, formats[i]);
        CHECK_ERROR(vg_lite_clear(&buffer[i],NULL,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("SFT_Clear_002_",&buffer[i]);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    vg_lite_free(&buffer[i]);
    return error;
}

// different buffer size
vg_lite_error_t    SFT_Clear_003()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int i,j;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[10];

    for(i = 0; i < 10; i++)
    {
        cc = GenColor_r();

        buffer[i].width = ALIGN((int32_t)Random_r(1.0f, WINDSIZEX),32);
        buffer[i].height = (int32_t)Random_r(1.0f, WINDSIZEY);
        buffer[i].format = VG_LITE_RGBA8888;
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("clear buffer of size %d x %d, color: 0x%x\n", buffer[i].width, buffer[i].height, cc);
        CHECK_ERROR(vg_lite_clear(&buffer[i],NULL,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("SFT_Clear_003_",&buffer[i]);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    vg_lite_free(&buffer[i]);
    return error;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t SFT_Clear()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: SFT_Clear_001:::::::::::::::::::::Started\n");
    CHECK_ERROR(SFT_Clear_001());
    output_string("\nCase: SFT_Clear_001:::::::::::::::::::::Ended\n");
    output_string("\nCase: SFT_Clear_002:::::::::::::::::::::Started\n");
    CHECK_ERROR(SFT_Clear_002());
    output_string("\nCase: SFT_Clear_002:::::::::::::::::::::Ended\n");
    output_string("\nCase: SFT_Clear_003:::::::::::::::::::::Started\n");
    CHECK_ERROR(SFT_Clear_003());
    output_string("\nCase: SFT_Clear_003:::::::::::::::::::::Ended\n");

ErrorHandler:
    return error;
}


/* ***
* Logging entry
*/
void SFT_Clear_Log()
{
}

