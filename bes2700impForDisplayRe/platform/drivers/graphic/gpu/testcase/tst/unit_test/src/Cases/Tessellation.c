/*-----------------------------------------------------------------------------
**Description: The test cases test the path filling functions.
-----------------------------------------------------------------------------*/
#include "../SFT.h"
#include "../Common.h"

#define COUNT 12

/* Buffers for Tessellation tests. */
vg_lite_buffer_t dst_buf[2];

/* Format array. */
static vg_lite_buffer_format_t dst_formats[] =
{
    VG_LITE_RGB565,
    VG_LITE_RGBA8888
};
int32_t path_data1[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
int32_t path_data2[] =
{
    2, 155, 155,
    5,  80,   0,
    5,   0,  80,
    5, -80,   0,
    5,   0, -80,
    2, 165, 165,
    5,  60,   0,
    5,   0,  60,
    5, -60,   0,
    5,   0, -60,
    1,
    0,
};
/* A simple triangle */
int32_t path_data3[] = {
    5, 170,  0,
    5, -85, 85,
    1
};
/* An hourglass shape */
int32_t path_data4[] = {
    5, 100,   0,
    5, -50, 100,
    5, -50, 100,
    5, 100,   0,
    1
};
int16_t path_data5[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
int16_t path_data6[] =
{
    2, 155, 155,
    5,  80,   0,
    5,   0,  80,
    5, -80,   0,
    5,   0, -80,
    2, 165, 165,
    5,  60,   0,
    5,   0,  60,
    5, -60,   0,
    5,   0, -60,
    1,
    0,
};
/* A simple triangle */
int16_t path_data7[] = {
    5, 170,  0,
    5, -85, 85,
    1
};
/* An hourglass shape */
int16_t path_data8[] = {
    5, 100,   0,
    5, -50, 100,
    5, -50, 100,
    5, 100,   0,
    1
};
int8_t path_data9[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
/* An hourglass shape */
int8_t path_data10[] = {
    2, -5, -10,
    4,  5, -10,
    4, 10,  -5,
    4,  0,   0,
    4, 10,   5,
    4,  5,  10,
    4, -5,  10,
    4,-10,   5,
    4,-10,  -5,
    0,
};
void build_paths(vg_lite_path_t * path)
{
    char    *pchar;
    float   *pfloat;
    int32_t data_size;
    data_size = 4 * 18 + 1;

    path->path = malloc(data_size);
    vg_lite_init_path(path, VG_LITE_FP32, VG_LITE_HIGH, data_size, path->path, 0.0f, 0.0f, 110.0f, 110.0f);
    pchar = (char*)path->path;
    pfloat = (float*)path->path;
    *pchar = 2;
    pfloat++;
    *pfloat++ = 35.0f;
    *pfloat++ = 50.0f;

    pchar = (char*)pfloat;
    *pchar = 4;
    pfloat++;
    *pfloat++ = 75.0f;
    *pfloat++ = 15.0f;

    pchar = (char*)pfloat;
    *pchar = 4;
    pfloat++;
    *pfloat++ = 110.0f;
    *pfloat++ = 35.0f;

    pchar = (char*)pfloat;
    *pchar = 4;
    pfloat++;
    *pfloat++ = 100.0f;
    *pfloat++ = 50.0f;

    pchar = (char*)pfloat;
    *pchar = 4;
    pfloat++;
    *pfloat++ = 110.0f;
    *pfloat++ = 65.0f;

    pchar = (char*)pfloat;
    *pchar = 4;
    pfloat++;
    *pfloat++ = 85.0f;
    *pfloat++ = 75.0f;

    pchar = (char*)pfloat;
    *pchar = 0;
}
int32_t *test_path1[4] = 
{
    path_data1,
    path_data2,
    path_data3,
    path_data4,
};
int32_t length1[]=
{
    sizeof(path_data1),
    sizeof(path_data2),
    sizeof(path_data3),
    sizeof(path_data4),
};
int16_t *test_path2[4] = 
{
    path_data5,
    path_data6,
    path_data7,
    path_data8,
};
int32_t length2[]=
{
    sizeof(path_data5),
    sizeof(path_data6),
    sizeof(path_data7),
    sizeof(path_data8),
};
int8_t *test_path3[2] = 
{
    path_data9,
    path_data10,
};
int32_t length3[]=
{
    sizeof(path_data9),
    sizeof(path_data10),
};

static void output_string(char *str)
{
    strcat (LogString, str);
    printf("%s", str);
}
static vg_lite_error_t Init()
{
    int i;
    vg_lite_error_t error;
    for (i = 0; i < 2; i++) {
        memset(&dst_buf[i],0,sizeof(vg_lite_buffer_t));
        dst_buf[i].width = (int)WINDSIZEX;
        dst_buf[i].height = (int)WINDSIZEY;
        dst_buf[i].format = dst_formats[i];
        dst_buf[i].stride = 0;
        dst_buf[i].handle = NULL;
        dst_buf[i].memory = NULL;
        dst_buf[i].address = 0;
        dst_buf[i].tiled = 0;

        CHECK_ERROR(vg_lite_allocate(&dst_buf[i]));
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(dst_buf[0].handle != NULL)
        vg_lite_free(&dst_buf[0]);
    if(dst_buf[1].handle != NULL)
        vg_lite_free(&dst_buf[1]);
    return error;
}

/*-----------------------------------------------------------------------------
**Name: Exit
**Parameters: None
**Returned Value: None
**Description: Destroy the buffer objects created in Init() function
-----------------------------------------------------------------------------*/
static void Exit()
{
    int i;
    for (i = 0; i < 2; i++) {
        vg_lite_free(&dst_buf[i]);
    }
}
vg_lite_error_t Tessellation_001()
{
    int i, j;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_color_t color = 0xff88ffff;
    uint32_t    r, g, b, a;
    vg_lite_path_t path;
    vg_lite_matrix_t matrix;
    vg_lite_quality_t qualities[QUALITY_COUNT] = {
        VG_LITE_LOW,
        VG_LITE_MEDIUM,
        VG_LITE_HIGH,
        VG_LITE_UPPER,
    };
    vg_lite_float_t degrees [] = {0, 15.0f, 30.0f, 90.0f, 180.0f, 270.0f, 0, 15.0f, 30.0f, 90.0f, 180.0f, 270.0f};
    vg_lite_format_t formats[FORMATS_COUNT] = {
        VG_LITE_S8,
        VG_LITE_S16,
        VG_LITE_S32,
        VG_LITE_FP32,
    };
    
    vg_lite_identity(&matrix);
    vg_lite_translate(WINDSIZEX/2, WINDSIZEY/2, &matrix);
    for (j = 0; vg_lite_query_feature(gcFEATURE_BIT_VG_QUALITY_8X) ? j < QUALITY_COUNT : j < QUALITY_COUNT - 1; j++) {
        for (i = 0; i < FORMATS_COUNT; i++)
        {
            r = (uint32_t)Random_r(0.0f, 255.0f);
            g = (uint32_t)Random_r(0.0f, 255.0f);
            b = (uint32_t)Random_r(0.0f, 255.0f);
            a = (uint32_t)Random_r(0.0f, 255.0f);
            color = r | (g << 8) | (b << 16) | (a << 24);
            if(formats[i] == VG_LITE_FP32) 
                build_paths(&path);
            else if(formats[i] == VG_LITE_S16)
                vg_lite_init_path(&path, formats[i], qualities[j], length2[i%4], test_path2[i%4], -WINDSIZEX, -WINDSIZEY, WINDSIZEX, WINDSIZEY);
            else if(formats[i] == VG_LITE_S8)
                vg_lite_init_path(&path, formats[i], qualities[j], length3[i%2], test_path3[i%2], -10, -10, 110, 110);
            else
                vg_lite_init_path(&path, formats[i], qualities[j], length1[i%4], test_path1[i%4], -WINDSIZEX, -WINDSIZEY, WINDSIZEX, WINDSIZEY);

            CHECK_ERROR(vg_lite_clear(&dst_buf[i % 2],NULL,0xffffffff));
            vg_lite_rotate(degrees[i], &matrix);
            CHECK_ERROR(vg_lite_draw(&dst_buf[i % 2],
                         &path,
                         i < COUNT /2 ? VG_LITE_FILL_NON_ZERO : VG_LITE_FILL_EVEN_ODD,
                         &matrix,
                         0,
                         color));
            CHECK_ERROR(vg_lite_finish());
            SaveBMP_SFT("Tessellation_", &dst_buf[i % 2],TRUE);
            if(formats[i] == VG_LITE_FP32) {
                free(path.path);
                path.path = NULL;
            }
        }
    }
    return VG_LITE_SUCCESS;

ErrorHandler:
    if(formats[i] == VG_LITE_FP32) {
        if(path.path != NULL) {
            free(path.path);
            path.path = NULL;
        }
    }
    return error;
}

/********************************************************************************
*     \brief
*         entry-Function
******************************************************************************/
vg_lite_error_t Tessellation()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    if (VG_LITE_SUCCESS != Init())
    {
        printf("Draw engine initialization failed.\n");
        Exit();
        return error;
    }
    output_string("\nCase: Tessellation:::::::::::::::::::::Started\n");

    output_string("\nCase: Tessellation_001:::::::::Started\n");
    CHECK_ERROR(Tessellation_001());
    output_string("\nCase: Tessellation_001:::::::::Ended\n");

ErrorHandler:
    Exit();
    return error;
}


/* ***
* Logging entry
*/
void Tessellation_Log()
{
}

