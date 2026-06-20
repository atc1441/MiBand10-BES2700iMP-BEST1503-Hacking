//-----------------------------------------------------------------------------
//Description: The test cases test the different size and position of Clear-Rectangle
//-----------------------------------------------------------------------------
#include "../SFT.h"
#include "../Common.h"

#define FORMAT_COUNT 2
/* Format array. */
static vg_lite_buffer_format_t formats[] =
{
    VG_LITE_RGB565,
    VG_LITE_RGBA8888,
};

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}

/* different buffer formats, different clear modes, and part region. */
vg_lite_error_t    Clear_001()
{
    int i;
    int x, y, w, h;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[FORMAT_COUNT];
    vg_lite_rectangle_t rect;

    for(i = 0; i < FORMAT_COUNT; i++)
    {
        cc = GenColor_r();
        memset(&buffer[i],0,sizeof(vg_lite_buffer_t));
        buffer[i].width = (int)WINDSIZEX;
        buffer[i].height = (int)WINDSIZEY;
        buffer[i].format = formats[i];
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        x = (int)Random_r(0, WINDSIZEX);
        y = (int)Random_r(0, WINDSIZEY);
        w = (int)Random_r(1.0f, WINDSIZEX);
        h = (int)Random_r(1.0f, WINDSIZEY);
        rect.x = x;
        rect.y = y;
        rect.width = w;
        rect.height = h;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("color: 0x%x, dst format: 0x%x\n", cc, formats[i]);
        printf("clear with rectangle mode.\n");
        CHECK_ERROR(vg_lite_clear(&buffer[i], NULL, 0xffffffff));
        CHECK_ERROR(vg_lite_clear(&buffer[i],&rect,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Clear_001_",&buffer[i],TRUE);
        printf("clear with scanline mode.\n");
        CHECK_ERROR(vg_lite_clear(&buffer[i], NULL, 0xffffffff));
        CHECK_ERROR(vg_lite_clear(&buffer[i],&rect,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Clear_001_",&buffer[i],TRUE);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(buffer[i].handle != NULL)
        vg_lite_free(&buffer[i]);
    return error;
}

/* different buffer formats, different clear modes, and full screen. */
vg_lite_error_t    Clear_002()
{
    int i;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[FORMAT_COUNT];

    for(i = 0; i < FORMAT_COUNT; i++)
    {
        cc = GenColor_r();
        memset(&buffer[i],0,sizeof(vg_lite_buffer_t));
        buffer[i].width = (int)WINDSIZEX;
        buffer[i].height = (int)WINDSIZEY;
        buffer[i].format = formats[i];
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("color: 0x%x, dst format: 0x%x\n", cc, formats[i]);
        printf("clear with rectangle mode.\n");
        CHECK_ERROR(vg_lite_clear(&buffer[i],NULL,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Clear_002_",&buffer[i],TRUE);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(buffer[i].handle != NULL)
        vg_lite_free(&buffer[i]);
    return error;
}

/* different buffer formats, different clear modes, and part region for 1080p resolution. */
vg_lite_error_t    Clear_003()
{
    int i;
    int x, y, w, h;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[FORMAT_COUNT];
    vg_lite_rectangle_t rect;

    for(i = 0; i < FORMAT_COUNT; i++)
    {
        cc = GenColor_r();
        memset(&buffer[i],0,sizeof(vg_lite_buffer_t));
        buffer[i].width = 1920;
        buffer[i].height = 1080;
        buffer[i].format = formats[i];
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        x = (int)Random_r(0, WINDSIZEX);
        y = (int)Random_r(0, WINDSIZEY);
        w = (int)Random_r(1.0f, WINDSIZEX);
        h = (int)Random_r(1.0f, WINDSIZEY);
        rect.x = x;
        rect.y = y;
        rect.width = w;
        rect.height = h;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("color: 0x%x, dst format: 0x%x\n", cc, formats[i]);
        printf("clear with rectangle mode.\n");
        CHECK_ERROR(vg_lite_clear(&buffer[i], NULL, 0xffffffff));
        CHECK_ERROR(vg_lite_clear(&buffer[i],&rect,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Clear_003_",&buffer[i],TRUE);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(buffer[i].handle != NULL)
        vg_lite_free(&buffer[i]);
    return error;
}

/* different buffer formats, different clear modes, and full screen for 1080p resolution. */
vg_lite_error_t    Clear_004()
{
    int i,j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t cc;
    vg_lite_buffer_t buffer[FORMAT_COUNT];

    for(i = 0; i < FORMAT_COUNT; i++)
    {
        cc = GenColor_r();
        memset(&buffer[i],0,sizeof(vg_lite_buffer_t));
        buffer[i].width = 1920;
        buffer[i].height = 1080;
        buffer[i].format = formats[i];
        buffer[i].stride = 0;
        buffer[i].handle = NULL;
        buffer[i].memory = NULL;
        buffer[i].address = 0;
        buffer[i].tiled = 0;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));

        printf("color: 0x%x, dst format: 0x%x\n", cc, formats[i]);
        printf("clear with rectangle mode.\n");
        CHECK_ERROR(vg_lite_clear(&buffer[i],NULL,cc));
        CHECK_ERROR(vg_lite_finish());
        SaveBMP_SFT("Clear_004_",&buffer[i],TRUE);
        vg_lite_free(&buffer[i]);
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(buffer[i].handle != NULL)
        vg_lite_free(&buffer[i]);
    return error;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Clear()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    output_string("\nCase: Clear_001:::::::::::::::::::::Started\n");
    CHECK_ERROR(Clear_001());
    output_string("\nCase: Clear_001:::::::::::::::::::::Ended\n");
    output_string("\nCase: Clear_002:::::::::::::::::::::Started\n");
    CHECK_ERROR(Clear_002());
    output_string("\nCase: Clear_002:::::::::::::::::::::Ended\n");
    output_string("\nCase: Clear_003:::::::::::::::::::::Started\n");
    CHECK_ERROR(Clear_003());
    output_string("\nCase: Clear_003:::::::::::::::::::::Ended\n");
    output_string("\nCase: Clear_004:::::::::::::::::::::Started\n");
    CHECK_ERROR(Clear_004());
    output_string("\nCase: Clear_004:::::::::::::::::::::Ended\n");

ErrorHandler:
    return error;
}


/* ***
* Logging entry
*/
void Clear_Log()
{
}

