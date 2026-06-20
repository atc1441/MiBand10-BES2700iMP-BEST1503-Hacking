#include    <stdio.h>
#include    <string.h>

#include    "SFT.h"
#include    "Common.h"

typedef vg_lite_error_t (*SFT_CASE)();

char *error_type[] = 
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

/*   pattern blending    */
vg_lite_blend_t blend_mode[]=
{
    VG_LITE_BLEND_NONE,
    VG_LITE_BLEND_SRC_OVER,
    VG_LITE_BLEND_DST_OVER,
    VG_LITE_BLEND_SRC_IN,
    VG_LITE_BLEND_DST_IN,
    VG_LITE_BLEND_MULTIPLY,
    VG_LITE_BLEND_SCREEN,
    VG_LITE_BLEND_ADDITIVE,
    VG_LITE_BLEND_SUBTRACT,
};
/*   pattern filters    */
vg_lite_filter_t filter_modes[] =
{
    VG_LITE_FILTER_POINT,
    VG_LITE_FILTER_LINEAR,
    VG_LITE_FILTER_BI_LINEAR,
};
/*   path format    */
vg_lite_format_t formats[] = {
    VG_LITE_S8,
    VG_LITE_S16,
    VG_LITE_S32,
    VG_LITE_FP32,
};
/*  path qualities  */
vg_lite_quality_t qualities[] = {
    VG_LITE_LOW,
    VG_LITE_MEDIUM,
    VG_LITE_HIGH,
    VG_LITE_UPPER,
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
    0,
};
/* A simple triangle */
int32_t path_data3[] = {
    5, 170,  0,
    5, -85, 85,
    0
};
/* An hourglass shape */
int32_t path_data4[] = {
    5, 100,   0,
    5, -50, 100,
    5, -50, 100,
    5, 100,   0,
    0
};
/* A simple square */
int32_t path_data5[] = {
    5, 100,   0,
    5,   0, 100,
    5,-100,   0,
    0
};
int32_t path_data6[] = {
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
/* A square shape with triangular holes */
int32_t path_data7[] = {
    4,  80,  40,
    4,  40,  80,
    4,   0,   0,
    4, 200,   0,
    4, 160,  80,
    4, 120,  40,
    4, 200,   0,
    4, 200, 200,
    4, 120, 160,
    4, 160, 120,
    4, 200, 200,
    4,   0, 200,
    4,  40, 120,
    4,  80, 160,
    4,   0, 200,
    0
};

/* A spheric triangle */
int32_t path_data8[] = {
    7,   85, -60, 170,-40,
    7,  -50,  50, -70,210,
    7,  -90, -30,-100,-170,
    0
};

/* A distorsioned shape with a square hole */
int32_t path_data9[] = {
    VLC_OP_QUAD,     8,     22,     45,     0,
    VLC_OP_QUAD,    37,     26,     45,     53,
    VLC_OP_QUAD,    30,     45,     15,     53,
    VLC_OP_LINE,    15,     18,
    VLC_OP_LINE,    35,     18,
    VLC_OP_LINE,    35,     40,
    VLC_OP_LINE,    0,      40,
    VLC_OP_QUAD,    7,      20,     0,      0,
    VLC_OP_END
};

int8_t path_data10[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
int16_t path_data11[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
int32_t path_data12[] = {
    2,  35, 50, 
    4,  75, 15, 
    4, 110, 35,
    4, 100, 50,
    4, 110, 65,
    4,  75, 85,
    0, 
};
int32_t *test_path[9] = 
{
    path_data1,
    path_data2,
    path_data3,
    path_data4,
    path_data5,
    path_data6,
    path_data7,
    path_data8,
    path_data9,
};
int32_t length[]=
{
    sizeof(path_data1),
    sizeof(path_data2),
    sizeof(path_data3),
    sizeof(path_data4),
    sizeof(path_data5),
    sizeof(path_data6),
    sizeof(path_data7),
    sizeof(path_data8),
    sizeof(path_data9),
};
void build_paths(vg_lite_path_t * path,vg_lite_quality_t qualities,vg_lite_filter_t filter_mode,vg_lite_blend_t blend_mode)
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

void output_string(char *str)
{
    printf("%s", str);
}
/*   generate buffer   */

vg_lite_error_t Allocate_Buffer(vg_lite_buffer_t *buffer,
                                       vg_lite_buffer_format_t format,
                                       int32_t width, int32_t height)
{
    vg_lite_error_t error;

    buffer->width = ALIGN(width, 128);
    buffer->height = height;
    buffer->format = format;
    buffer->stride = 0;
    buffer->handle = NULL;
    buffer->memory = NULL;
    buffer->address = 0;
    buffer->tiled   = (vg_lite_buffer_layout_t)0;

    CHECK_ERROR(vg_lite_allocate(buffer));

ErrorHandler:
    return error;
}


void Free_Buffer(vg_lite_buffer_t *buffer)
{
    vg_lite_free(buffer);
}

vg_lite_error_t gen_buffer(int type, vg_lite_buffer_t *buf, vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    void *data = NULL;
    uint8_t *pdata8, *pdest8;
    uint32_t bpp = get_bpp(format);
    int i;

    /* Randomly generate some images. */
    data = gen_image(type, format, width, height);
    pdata8 = (uint8_t*)data;
    pdest8 = (uint8_t*)buf->memory;
    if (data != NULL)
    {
        for (i = 0; i < height; i++)
        {
            memcpy (pdest8, pdata8, bpp / 8 * width);
            pdest8 += buf->stride;
            pdata8 += bpp / 8 * width;
        }

        free(data);
    }
    else
    {
        printf("Image data generating failed.\n");
    }

    return VG_LITE_SUCCESS;
}

void _memcpy(void *dst, void *src, int size) 
{
    int i;
    for (i = 0; i < size; i++) {
        ((unsigned char*)dst)[i] = ((unsigned char*)src)[i];
    }
}

SFT_CASE SFT_CASES[] = 
{
    Pattern_Test,
    Gradient_Test,
    Matrix_Test,
    API_Test,
    Memory_Test,
    NULL
};
vg_lite_error_t Render()
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    int8_t i = 0;
    InitBMP(1920, 1080);
    while(SFT_CASES[i] != NULL) {
        error = SFT_CASES[i++]();
        if(error) {
            printf("[%s]Pattern_Draw_Test %d failed.error type is %s\n", __func__, __LINE__,error_type[error]);
            return error;
        }
    }
    DestroyBMP();
    return error;
}

