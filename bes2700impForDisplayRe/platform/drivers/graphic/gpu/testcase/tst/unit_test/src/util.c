#include <stdio.h>
#include <math.h>
#include "util.h"
#include "SFT.h"

extern vg_lite_buffer_t *g_fb;
extern vg_lite_rectangle_t g_fb_rect;

#ifndef __rvct__
#include "string.h"
#endif

#if ! ( defined __LINUX__ || defined __NUCLEUS__  || defined(__QNXNTO__) || defined(__APPLE__))
#include <windows.h>
#include <cassert>
#else
#define BI_RGB        0L
#define BI_RLE8       1L
#define BI_RLE4       2L
#define BI_BITFIELDS  3L

typedef struct tagBITMAPINFOHEADER{
    DWORD      biSize;
    LONG       biWidth;
    LONG       biHeight;
    WORD       biPlanes;
    WORD       biBitCount;
    DWORD      biCompression;
    DWORD      biSizeImage;
    LONG       biXPelsPerMeter;
    LONG       biYPelsPerMeter;
    DWORD      biClrUsed;
    DWORD      biClrImportant;
} __attribute((packed)) BITMAPINFOHEADER;

typedef struct tagBITMAPFILEHEADER {
    WORD    bfType;
    DWORD   bfSize;
    WORD    bfReserved1;
    WORD    bfReserved2;
    DWORD   bfOffBits;
} __attribute((packed)) BITMAPFILEHEADER;

#ifdef __NUCLEUS__
HTRANSINTERFACE input_file = NULL;
HTRANSINTERFACE output_file = NULL;
#endif
#endif

int DATA_STRIDE = 0;
#define LEN 2048
unsigned char *image_data = NULL;     
unsigned char *readpixel_data = NULL;

int DIR_Initialized = FALSE;
char *MAIN_DIR = NULL;
char *BMP_DIR = NULL;
char *LOG_DIR = NULL;
char *MAIN_LOG = NULL;
char *INP_DIR = NULL;

BYTE destbuf[LEN + 16];
char image_bmc_name[256];

unsigned char * ReadBMPFile (char * file_name, int * width, int* height);
void WriteBMPFile (char * file_name, unsigned char * pbuf, unsigned int size);
void AppendLogFile (char * file_name, char * log, int mainlog);
void Clear_Screen(vg_lite_color_t color);

uint32_t get_bpp(vg_lite_buffer_format_t format)
{
    uint32_t bpp;

    switch (format)
    {
    case VG_LITE_RGBA8888:
    case VG_LITE_BGRA8888:
    case VG_LITE_ABGR8888:
    case VG_LITE_ARGB8888:
    case VG_LITE_RGBX8888:
    case VG_LITE_BGRX8888:
    case VG_LITE_XBGR8888:
    case VG_LITE_XRGB8888:
        bpp = 32;
        break;

    case VG_LITE_ABGR1555:
    case VG_LITE_ARGB1555:
    case VG_LITE_BGRA5551:
    case VG_LITE_RGBA5551:
    case VG_LITE_RGBA4444:
    case VG_LITE_BGRA4444:
    case VG_LITE_ABGR4444:
    case VG_LITE_ARGB4444:
    case VG_LITE_RGB565:
    case VG_LITE_BGR565:
    case VG_LITE_YUYV:
        bpp = 16;
        break;

    case VG_LITE_A8:
    case VG_LITE_L8:
    case VG_LITE_RGBA2222:
    case VG_LITE_BGRA2222:
    case VG_LITE_ABGR2222:
    case VG_LITE_ARGB2222:
        bpp = 8;
        break;

    case VG_LITE_A4:
        bpp = 4;
        break;

    default:
        bpp = 32;
        break;
    }

    return bpp;
}

void * gen_image(int type, vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    void *data = NULL;
    switch (type)
    {
    case 0:
        data = gen_checker(format, width, height);
        break;

    case 1:
        data = gen_gradient(format, width, height);
        break;

    case 2:
        data = gen_solid(format, width, height);
        break;

    default:
        break;
    }

    return data;
}

void * gen_checker(vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    static int checker = 20;
    int color0[] = {0, 0, 255, 255};
    int color1[] = {255, 255, 255, 255};
    int bpp = get_bpp(format);
    uint32_t * pdata32;
    uint16_t * pdata16;
    uint8_t  * pdata8;
    void     * pdata;
    uint32_t   pixel[2];

    int x, y, idx;

    color0[0] = (int)Random_r(0, 255);
    color0[1] = (int)Random_r(0, 255);
    color0[2] = (int)Random_r(0, 255);
    color0[3] = (int)Random_r(0, 255);
    color1[0] = (int)Random_r(0, 255);
    color1[1] = (int)Random_r(0, 255);
    color1[2] = (int)Random_r(0, 255);
    color1[3] = (int)Random_r(0, 255);

    pdata = malloc(bpp / 8 * width * height);
    pdata32 = (uint32_t*)pdata;
    pdata16 = (uint16_t*)pdata;
    pdata8  = (uint8_t* )pdata;

    pixel[0] = pack_pixel(format, color0[0], color0[1], color0[2], color0[3]);
    pixel[1] = pack_pixel(format, color1[0], color1[1], color1[2], color1[3]);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            idx = ((x/checker) + (y/checker)) % 2;
            switch (bpp)
            {
            case 32: *pdata32++ = pixel[idx];           break;
            case 16: *pdata16++ = (uint16_t)pixel[idx]; break;
            case 8 : *pdata8++  = (uint8_t)pixel[idx];  break;
            default: break;
            }
        }
    }

    return pdata;
}

void * gen_gradient(vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    int orient;
    int color0[4] = {0, 0, 255, 255};
    int color1[4] = {255, 0, 0, 0};
    int color[4];
    int x, y;
    float ratio;
    uint32_t pixel;
    uint32_t * pdata32;
    uint16_t * pdata16;
    uint8_t  * pdata8;
    uint8_t  temp_pixel;
    void     * pdata;
    int        bpp;

    bpp = get_bpp(format);

    pdata = malloc(bpp * width * height / 8);
    pdata32 = (uint32_t*) pdata;
    pdata16 = (uint16_t*) pdata;
    pdata8  = (uint8_t *) pdata;

    orient = (int)Random_r(0, 1);

    color0[0] = (int)Random_r(0, 255);
    color0[1] = (int)Random_r(0, 255);
    color0[2] = (int)Random_r(0, 255);
    color0[3] = (int)Random_r(0, 255);
    color1[0] = (int)Random_r(0, 255);
    color1[1] = (int)Random_r(0, 255);
    color1[2] = (int)Random_r(0, 255);
    color1[3] = (int)Random_r(0, 255);

    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            if (orient == 0)
            {
                ratio = (float)x / (width - 1);
            }
            else
            {
                ratio = (float)y / (height - 1);
            }

            color[0] = (int)(ratio * color1[0] + (1.0f - ratio) * color0[0]);
            color[1] = (int)(ratio * color1[1] + (1.0f - ratio) * color0[1]);
            color[2] = (int)(ratio * color1[2] + (1.0f - ratio) * color0[2]);
            color[3] = (int)(ratio * color1[3] + (1.0f - ratio) * color0[3]);

            pixel = pack_pixel(format, color[0], color[1], color[2], color[3]);
            switch (bpp)
            {
            case 32: *pdata32++ = (uint32_t)pixel;  break;
            case 16: *pdata16++ = (uint16_t)pixel;  break;
            case 8 : *pdata8++  = (uint8_t) pixel;  break;
            case 4 :
                if(x % 2 ==0) {
                    temp_pixel = ((uint8_t) pixel) << 4;
                }
                else {
                    *pdata8++ = ((uint8_t) pixel) >> 4 | temp_pixel;
                }
                break;
            default: break;
            }
        }
    }

    return pdata;
}

void * gen_solid(vg_lite_buffer_format_t format, uint32_t width, uint32_t height)
{
    int i, j;
    uint32_t r, g, b, a, pixel;
    void *data = NULL;
    uint8_t  *pdata8;
    uint32_t *pdata32;
    uint16_t *pdata16;
    uint32_t bpp = get_bpp(format);

    r = Random_r(0, 255);
    g = Random_r(0, 255);
    b = Random_r(0, 255);
    a = Random_r(0, 255);

    data = malloc(width * height * bpp / 8);
    pdata8 = (uint8_t  *)data;
    pdata32 = (uint32_t*)data;
    pdata16 = (uint16_t*)data;

    pixel = pack_pixel(format, r, g, b, a);
    for (i = 0; i < height; i ++)
    {
        for (j = 0; j < width; j++)
        {
            switch(bpp)
            {
            case 32:
                *pdata32 = pixel;
                pdata32++;
                break;

            case 16:
                *pdata16 = (uint16_t)pixel;
                pdata16++;
                break;

            case 8:
                *pdata8++ = (uint8_t)pixel;
                break;

            default:
                break;
            }
        }
    }

    return data;
}

uint32_t pack_pixel(vg_lite_buffer_format_t format,
                    uint32_t r, uint32_t g, uint32_t b, uint32_t a)
{
    uint32_t pixel = 0;
    switch (format)
    {
    case VG_LITE_RGBA8888:
        pixel = r | (g << 8) | (b << 16) | (a << 24);
        break;

    case VG_LITE_BGRA8888:
        pixel = b | (g << 8) | (r << 16) | (a << 24);
        break;
    
    case VG_LITE_ARGB8888:
        pixel = a | (r << 8) | (g << 16) | (b << 24);
        break;

    case VG_LITE_ABGR8888:
        pixel = a | (b << 8) | (g << 16) | (r << 24);
        break;

    case VG_LITE_RGB565:
        pixel = ((r & 0xf8) >> 3) | ((g & 0xfc) << 3) | ((b & 0xf8) << 8);
        break;

    case VG_LITE_BGR565:
        pixel = ((b & 0xf8) >> 3) | ((g & 0xfc) << 3) | ((r & 0xf8) << 8);
        break;

    case VG_LITE_RGBA4444:
        pixel = ((r & 0xf0) >> 4) | (g & 0xf0) | ((b & 0xf0) << 4) | ((a & 0xf0) << 8);
        break;

    case VG_LITE_BGRA4444:
        pixel = ((b & 0xf0) >> 4) | (g & 0xf0) | ((r & 0xf0) << 4) | ((a & 0xf0) << 8);
        break;

    case VG_LITE_ARGB4444:
        pixel = ((a & 0xf0) >> 4) | (r & 0xf0) | ((g & 0xf0) << 4) | ((b & 0xf0) << 8);
        break;

    case VG_LITE_ABGR4444:
        pixel = ((a & 0xf0) >> 4) | (b & 0xf0) | ((g & 0xf0) << 4) | ((r & 0xf0) << 8);
        break;
    
    case VG_LITE_RGBA2222:
        pixel = ((r & 0xc0) >> 6) | (g & 0xc0) >> 4 | ((b & 0xc0) >> 2) | (a & 0xc0);
        break;

    case VG_LITE_BGRA2222:
        pixel = ((b & 0xc0) >> 6) | (g & 0xc0) >> 4 | ((r & 0xc0) >> 2) | (a & 0xc0);
        break;

    case VG_LITE_ARGB2222:
        pixel = ((a & 0xc0) >> 6) | (r & 0xc0) >> 4 | ((g & 0xc0) >> 2) | (b & 0xc0);
        break;

    case VG_LITE_ABGR2222:
        pixel = ((a & 0xc0) >> 6) | (b & 0xc0) >> 4 | ((g & 0xc0) >> 2) | (r & 0xc0);
        break;

    case VG_LITE_A8:
        pixel = a;
        break;

    case VG_LITE_A4:
        pixel = (a & 0x0f) | ((a & 0x0f) << 4);
        break;

    case VG_LITE_L8:
        pixel = (int)(0.2126f * r + 0.7152f * g + 0.0722f * b);
        break;

    case VG_LITE_YUYV:
        break;

    case VG_LITE_RGBX8888:
        pixel = r | (g << 8) | (b << 16);
        break;
        
    case VG_LITE_BGRX8888:
        pixel = b | (g << 8) | (r << 16);
        break;

    case VG_LITE_XRGB8888:
        pixel = r | (g << 8) | (b << 16);
        break;
        
    case VG_LITE_XBGR8888:
        pixel = b | (g << 8) | (r << 16);
        break;

    case VG_LITE_BGRA5551:
        pixel = ((b & 0xf8) >> 3) | ((g & 0xf8) << 2) | ((r & 0xf8) << 7) | ((a & 0x80) << 8);
        break;

    case VG_LITE_RGBA5551:
        pixel = ((r & 0xf8) >> 3) | ((g & 0xf8) << 2) | ((b & 0xf8) << 7) | ((a & 0x80) << 8);
        break;
    
    case VG_LITE_ABGR1555:
        pixel = ((a & 0x80) >> 7) | ((b & 0xf8) << 1) | ((g & 0xf8) << 6) | ((r & 0xf8) << 11);
        break;

    case VG_LITE_ARGB1555:
        pixel = ((a & 0x80) >> 7) | ((r & 0xf8) << 1) | ((g & 0xf8) << 6) | ((b & 0xf8) << 11);
        break;

    default:
        assert(FALSE);
        break;
    }

    return pixel;
}



#if ( defined __LINUX__ || defined __NUCLEUS__ || defined(__QNXNTO__))
char * itoa (int num, char* str, int t)
{
    int i = 0;
    int temp = num;
    int cur = 0;

    while (temp > 0){
        i++;
        temp = temp / 10;
    }

    temp = num;
    while (temp > 0){
        cur = temp % 10;
        str[--i] = '0' + cur;
        temp = temp / 10;
    }

    return str;
}
#endif

#define MAX_FILE_LENGTH 1024
char CurrentDir[MAX_FILE_LENGTH];
void GetCurrentDir()
{
    return ;
}


void Get_Dirs()
{
    if(DIR_Initialized == TRUE)
        return;

    GetCurrentDir();
    MAIN_DIR = "./";
    BMP_DIR = "result_bmp/";
    LOG_DIR = "result_Log/";
    MAIN_LOG = "SFT.txt";
    INP_DIR = "input/";
    DIR_Initialized = TRUE;
}

#if 1
/* Devide the full screen into 15 rects */
rect_s c_rect[15];

void InitRects()
{
    int i, j;
    int x, y, w, h;
    rect_s * rect;

    h = (int)WINDSIZEY / 4 - 20;
    w = (int)WINDSIZEX / 4 - 10;
    rect = c_rect;

    for (j = 0; j < 4; j++){
        y = 10 + j * ( h + 20 );
        if ( j != 3 ){
            for (i = 0; i < 4; i++){
                x = 5 + i * ( w + 10 );
                rect->x = x;
                rect->y = y;
                rect->w = w;
                rect->h = h;
                rect++;
            }
        } else {
            w = (int)WINDSIZEX / 3 - 20;
            for (i = 0; i < 3; i++){
                x = 10 + i * ( w + 20 );
                rect->x = x;
                rect->y = y;
                rect->w = w;
                rect->h = h;
                rect++;
            }
        }
    }
}


int InitBMP(int width, int height)
{
    InitRects();

    /* only support read from screen as RGBA8888 format. */
    DATA_STRIDE = width * 4;

    image_data = (unsigned char *)MALLOC (width * height *4 + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + 3);
    memset(image_data, 0, width * height * 4 + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + 3);

    if(image_data == NULL)
        return 1;

    readpixel_data = (unsigned char *)(((unsigned int)image_data + sizeof(BITMAPINFOHEADER) + sizeof(BITMAPFILEHEADER) + 3) & (~0x3));

    return 0;
}

void DestroyBMP()
{
    if(image_data != NULL)
        FREE (image_data);
}

int SaveBMP(char *image_name, unsigned char* p, int width, int height,
            vg_lite_buffer_format_t format, int stride)
{
    BITMAPINFOHEADER *infoHeader;
    BITMAPFILEHEADER *fileHeader;
    uint8_t *l_image_data = NULL;
    unsigned char *l_readpixel_data = NULL;
    unsigned char *l_data_load;
    int data_size;
    int index, index2;
    int file_len;
    uint16_t color;

    l_image_data = image_data;

    fileHeader =(BITMAPFILEHEADER *) l_image_data;
    infoHeader =(BITMAPINFOHEADER *)(l_image_data + sizeof(BITMAPFILEHEADER));

    /* file header. */
    fileHeader->bfType       = 0x4D42;
    fileHeader->bfSize       = sizeof(BITMAPFILEHEADER);
    fileHeader->bfReserved1  = 0;
    fileHeader->bfReserved2  = 0;
    fileHeader->bfOffBits    = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    /* infoHeader. */
    *(char *)&infoHeader->biSize = sizeof(BITMAPINFOHEADER);
    *(short *)&infoHeader->biWidth = (short)width;
    *(short *)&infoHeader->biHeight = (short)height;
    infoHeader->biPlanes = 1;
    infoHeader->biBitCount = 24;
    *(short *)&infoHeader->biCompression = BI_RGB;

    data_size = width * height * 3;

    for(index = 0; index < height; index++){
        l_readpixel_data = (uint8_t *)p + index*stride;
        l_data_load = l_image_data + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (height-1-index) * width *3;
        for (index2 = 0;index2 < width; index2++, l_data_load+=3)
        {
            switch (format)
            {
            case VG_LITE_RGBA8888:
            case VG_LITE_RGBX8888:
                l_data_load[0] = l_readpixel_data[2];
                l_data_load[1] = l_readpixel_data[1];
                l_data_load[2] = l_readpixel_data[0];
                l_readpixel_data +=4;
                break;
            case VG_LITE_BGRA8888:
            case VG_LITE_BGRX8888:
                l_data_load[0] = l_readpixel_data[0];
                l_data_load[1] = l_readpixel_data[1];
                l_data_load[2] = l_readpixel_data[2];
                l_readpixel_data +=4;
                break;
            case VG_LITE_ARGB8888:
            case VG_LITE_XRGB8888:
                l_data_load[0] = l_readpixel_data[3];
                l_data_load[1] = l_readpixel_data[2];
                l_data_load[2] = l_readpixel_data[1];
                l_readpixel_data +=4;
                break;
            case VG_LITE_ABGR8888:
            case VG_LITE_XBGR8888:
                l_data_load[0] = l_readpixel_data[1];
                l_data_load[1] = l_readpixel_data[2];
                l_data_load[2] = l_readpixel_data[3];
                l_readpixel_data +=4;
                break;

            case VG_LITE_RGB565:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                l_data_load[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                l_data_load[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                break;
            case VG_LITE_BGR565:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 13);
                l_data_load[1] = ((color & 0x07E0) >> 3) | ((color & 0x0600) >> 9);
                l_data_load[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                break;

            case  VG_LITE_RGBA4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = (color & 0x000F) << 4;
                l_data_load[1] = (color & 0x00F0);
                l_data_load[0] = (color & 0x0F00) >> 4;
                break;
            case VG_LITE_BGRA4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[0] = (color & 0x000F) << 4;
                l_data_load[1] = (color & 0x00F0);
                l_data_load[2] = (color & 0x0F00) >> 4;
                break;
            case  VG_LITE_ARGB4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = (color & 0x00F0);
                l_data_load[1] = (color & 0x0F00) >> 4;
                l_data_load[0] = (color & 0xF000) >> 8;
                break;
            case  VG_LITE_ABGR4444:
                color = *(uint16_t*)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[0] = (color & 0x00F0);
                l_data_load[1] = (color & 0x0F00) >> 4;
                l_data_load[2] = (color & 0xF000) >> 8;
                break;

            case VG_LITE_BGRA5551:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                l_data_load[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                l_data_load[0] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                break;
            case VG_LITE_RGBA5551:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[0] = ((color & 0x7C00) >> 7) | ((color & 0x7000) >> 12);
                l_data_load[1] = ((color & 0x03E0) >> 2) | ((color & 0x0380) >> 7);
                l_data_load[2] = ((color & 0x001F) << 3) | ((color & 0x001C) >> 2);
                break;
            case VG_LITE_ABGR1555:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[2] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 11);
                l_data_load[1] = ((color & 0x07C0) >> 3) | ((color & 0x0700) >> 6);
                l_data_load[0] = ((color & 0x003E) << 2) | ((color & 0x0038) >> 3);
                break;
            case VG_LITE_ARGB1555:
                color = *(uint16_t *)l_readpixel_data;
                l_readpixel_data += 2;
                l_data_load[0] = ((color & 0xF800) >> 8) | ((color & 0xE000) >> 11);
                l_data_load[1] = ((color & 0x07C0) >> 3) | ((color & 0x0700) >> 6);
                l_data_load[2] = ((color & 0x003E) << 2) | ((color & 0x0038) >> 3);
                break;

            case VG_LITE_RGBA2222:
                l_data_load[2] = (l_readpixel_data[0] & 0x03) << 6;
                l_data_load[1] = (l_readpixel_data[0] & 0x0c) << 4;
                l_data_load[0] = (l_readpixel_data[0] & 0x30) << 2;
                l_readpixel_data++;
                break;
            case VG_LITE_BGRA2222:
                l_data_load[0] = (l_readpixel_data[0] & 0x03) << 6;
                l_data_load[1] = (l_readpixel_data[0] & 0x0c) << 4;
                l_data_load[2] = (l_readpixel_data[0] & 0x30) << 2;
                l_readpixel_data++;
                break;
            case VG_LITE_ARGB2222:
                l_data_load[2] = (l_readpixel_data[0] & 0x0c) << 4;
                l_data_load[1] = (l_readpixel_data[0] & 0x30) << 2;
                l_data_load[0] = (l_readpixel_data[0] & 0xc0);
                l_readpixel_data++;
                break;
            case VG_LITE_ABGR2222:
                l_data_load[2] = (l_readpixel_data[0] & 0x0c) << 4;
                l_data_load[1] = (l_readpixel_data[0] & 0x30) << 2;
                l_data_load[0] = (l_readpixel_data[0] & 0xc0);
                l_readpixel_data++;
                break;

            case VG_LITE_A8:
            case VG_LITE_L8:
                l_data_load[0] = l_data_load[1] = l_data_load[2] = l_readpixel_data[0];
                l_readpixel_data++;
                break;
            case VG_LITE_YUYV:
                printf("not support format!");
                break;
            default:
                printf("unkonwn format!");
                break;
            }
        }
    }
        file_len = data_size + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

        WriteBMPFile(image_name, l_image_data, file_len);

        return 0;
    }

    /* generate BMP file name automatically. */
    void BMP_File_Name(char *case_name, char *new_name)
    {
        static char surf[] = {'_', '0', '0', '1', '.', 'b', 'm', 'p', '\0' };
        static char before_name[64];

        new_name[0] = '\0';
        strcpy(new_name, case_name);

        if(strcmp(before_name, case_name) != 0) {
            before_name[0] = '\0';
            surf[1] = '0';
            surf[2] = '0';
            surf[3] = '1';
            strcpy(before_name, case_name);
        }else{
            if(surf[3] == '9'){
                surf[3] -= 9;
                if(surf[2] == '9'){
                    surf[2] -= 9;
                    if (surf[1] == '9') {
                        surf[1] -= 9;
                        surf[0] = '1';
                    } else {
                        surf[1] += 1;
                    }
                }else{
                    surf[2] += 1;
                }
            }else{
                surf[3] += 1; 
            }
        }

        strcat(new_name, surf);
        return;
    }

    /* Save Full Screen BMP files for SFT cases. */
    void SaveBMP_SFT(char * name,vg_lite_buffer_t *buffer,BOOL save)
    {
        char        new_name[64];
        unsigned char * pData = readpixel_data;

        pData = (unsigned char*)buffer->memory;
        BMP_File_Name(name, new_name);
        if(save)
        {
            SaveBMP(new_name, pData, buffer->width, buffer->height, buffer->format, buffer->stride);
        }
    }

    /*-----------------------------------------------------------------
    **Name: Load_BMP
    **Parameters:
    **        file_name: The char* type indicate the name of bitmap
    **        height: The height of required bitmap
    **        width: The width of required bitmap
    **Return Value: Pointer point to the data of bitmap image
    **Description: Call ReadBMPFile() function to read the 24Bpp RGB bitmap data
    **                to a buffer, then read data from this buffer and change the 
    **                24Bpp RGB data to 32Bpp RGBA data(A = 0xFF), then return it.
    -----------------------------------------------------------------*/
    unsigned char* Load_BMP(char *file_name, int *height, int *width)
    {
        unsigned char *pData, *pSrcData;
        uint32_t imgHeight, imgWidth;
        uint32_t yInx, xInx;
        unsigned char * pSrc, *pDst;

        pSrcData = ReadBMPFile (file_name, (int*)&imgWidth, (int*)&imgHeight);
        pData = (unsigned char *)MALLOC (sizeof(unsigned char)*4*imgHeight*imgWidth);
        assert(pData != NULL);

        pSrc = (unsigned char *)pSrcData;
        pDst = (unsigned char *)pData;

        /* change the bitmap data from 24Bpp RGB pixel format to 32Bpp RGBA pixel format */
        for (yInx=0; yInx < imgHeight; yInx++){
            for (xInx=0; xInx < imgWidth; xInx++){
                *(pDst+3) = *(pSrc+2);        /* r */
                *(pDst+2) = *(pSrc+1);        /* g */
                *(pDst+1) = *(pSrc);        /* b */
                *(pDst+0) = 0xFF;            /* a */

                pDst += 4;
                pSrc += 3;
            }
        }

        FREE (pSrcData);

        *width = imgWidth;
        *height = imgHeight;
        return (pData);
    }


    /*------------------------------------------------------------------------------
    **Name: ReadBMPFile
    **Parameters:
    **        file_name: the char* type indicate the file name of bitmap
    **        width: the width of required bitmap
    **        Height: the height of required bitmap
    **Returned Value: Pointer point to the data of bitmap
    **Description: For Nucleus, read from cache srcfile to pbuf;
    **               For Win32/WM/Linux, read form file to pbuf.
    **               First read BMP file header BITMAPFILEHEADER and info header 
    **               BITMAPINFOHEADER,get the width and height of the BMP,
    **             then read the RGB data from BITMAPFILEHEADER.bfOffBits of the file.
    ------------------------------------------------------------------------------*/
    unsigned char * ReadBMPFile (char * file_name, int * width, int* height)
    {
        BITMAPFILEHEADER bHeader;
        BITMAPINFOHEADER bInfo;
        unsigned char *pSrcData;
        uint32_t imgHeight = 0, imgWidth = 0;
        char full_file_name [256];

        Get_Dirs();
        full_file_name[0] = '\0';
        strcat(full_file_name, MAIN_DIR);
        strcat(full_file_name, INP_DIR);
        strcat(full_file_name, file_name);

        {
            FILE* fd;

            fd = fopen(full_file_name, "rb");
            assert(fd != NULL);

            /* Read the bitmap header and bitmap info header
               The bitmap image must 24Bpp and RGB pixel format */
            fread(&bHeader, 1, sizeof(BITMAPFILEHEADER), fd);
            assert(bHeader.bfType == (DWORD)0x4D42);
            fread(&bInfo, 1, sizeof(BITMAPINFOHEADER), fd);
            assert(bInfo.biPlanes == 1 && bInfo.biBitCount == 24 && bInfo.biCompression == BI_RGB && bInfo.biWidth%4 == 0 );

            fseek(fd, bHeader.bfOffBits, SEEK_SET);
            imgHeight = bInfo.biHeight;
            imgWidth = bInfo.biWidth;

            /* Read the data of bitmap into a buffer, then return the pointer point to the buffer */
            pSrcData = (unsigned char *)MALLOC (sizeof(unsigned char)*3*imgHeight*imgWidth);
            assert(pSrcData != NULL);
            fread( pSrcData, 1, imgWidth * imgHeight * 3, fd);

            fclose(fd);
        }

        *width = imgWidth;
        *height = imgHeight;
        return (pSrcData);
    }
#endif

    void SavePNG(vg_lite_buffer_t *buf, char *file_name, int i)
    {
        char full_file_name [256] = {0};

        Get_Dirs();
        full_file_name[0] = '\0';
        sprintf(full_file_name, "%s%s%s%03d.png", MAIN_DIR, BMP_DIR, file_name, i);

        vg_lite_save_png(full_file_name, buf);
    }

    vg_lite_color_t GenColor_r()
    {
        int r, g, b, a;
        vg_lite_color_t result;

        r = (int)Random_i(0, 255);
        g = (int)Random_i(0, 255);
        b = (int)Random_i(0, 255);
        a = (int)Random_i(0, 255);

        result = r | (g << 8) | (b << 16) | (a << 24);

        return result;
    }
    /*----------------------------------------------
    **----------------------------------------------
    ** For Nucleus, write to cache srcfile from pbuf;
    **  for Win32/WM/Linux, write to file from pbuf */
    void WriteBMPFile (char * file_name, unsigned char * pbuf, unsigned int size)
    {
        unsigned int result = 0;
        char full_file_name [256];

        Get_Dirs();
        full_file_name[0] = '\0';
        strcat(full_file_name, MAIN_DIR);
        strcat(full_file_name, BMP_DIR);
        strcat(full_file_name, file_name);

        {
            FILE* fd;
            fd = fopen(full_file_name, "wb");

            /* assert (fd != NULL); */
            if(fd != NULL)
            {
                result = fwrite(pbuf, 1, size, fd);
                assert (result == size);

                fclose(fd);
            }
            else
            {
                printf("errno:%d %s(%d)open file: %s failed!\n",
                    ferror(fd),__FUNCTION__,__LINE__,full_file_name);
            }
        }
    }

    /* Random functins. */
    static unsigned long int random_value = 32557;
#define RANDOM_MAX 32767

    void random_srand(unsigned int seed)
    {
        random_value = seed;
    }

    int random_rand(void)
    {
        random_value = random_value * 1103515245 + 12345;
        return (unsigned int)(random_value / 65536) % 32768;
    }

    float Random_r(float Random_low, float Random_hi)
    {
        float x;
        float y, z;

        x = (float)random_rand()/(RANDOM_MAX);
        y = (1 - x)*Random_low;
        z = x * Random_hi;
        return y + z ;
    }

    unsigned long int Random_i(unsigned long int Random_low, unsigned long int Random_hi)
    {
        unsigned long int x, y;
        x = random_rand();
        if ( (y = (Random_hi - Random_low) ) > RANDOM_MAX)
            y = Random_low + y / (RANDOM_MAX) * x;
        else
            y = Random_low + (y * x + (RANDOM_MAX)/y) / (RANDOM_MAX);
        return y;
    }

    /*---------------------------------------------------------------------
    **Name: Clear_Screen
    **Parameters: None
    **Returned Value: None
    **Description: The function clear the screen with specified color.
    ---------------------------------------------------------------------*/
    void Clear_Screen(vg_lite_color_t color)
    {
        vg_lite_clear(g_fb, NULL, color);
    }
