
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

    //摄像头通用接口
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
using namespace cv;
    /* Video 通用接口 */
typedef struct {
    int idx;
    VideoCapture *stream;
    Mat frame;
} WebcamContext;
static WebcamContext gCtx[3] = { 0 };    //目前最大支持 3 个摄像头.

    //打开摄像头
static int InitCameraViaID(int idx, int camera_idx, int w, int h)
{
    //参数检查
    gCtx[idx].stream = new VideoCapture(camera_idx);
    if (!gCtx[idx].stream) {
        printf("Cannot open Camera %d\n", camera_idx);
        exit(-1);
        return -1;
    }
    gCtx[idx].stream->set(CV_CAP_PROP_FRAME_WIDTH, w);
    gCtx[idx].stream->set(CV_CAP_PROP_FRAME_HEIGHT, h);
    return 0;
}

static void FinalCamera(void)
{
    for (int i = 0; i < sizeof(gCtx) / sizeof(gCtx[0]); i++) {
        if (gCtx[i].stream) {
            gCtx[i].stream->release();
            gCtx[i].stream = NULL;
            if (!gCtx[i].frame.empty())
                gCtx[i].frame.release();
        }
    }
}

    //获取摄像头的数据
static Mat Get_Picture(int idx)
{
    gCtx[idx].stream->read(gCtx[idx].frame);
    return gCtx[idx].frame;
}

static void save_bmp_to_file(int idx, uint8_t * bmp, int bmp_size)
{
    char filename[128];
    sprintf(filename, "/tmp/jpg/%d.bmp", idx);
    int fd = open(filename, O_CREAT | O_WRONLY | O_TRUNC, 0666);
    if (fd > 0) {
        write(fd, bmp, bmp_size);
        close(fd);
    }
}

#define dbg()        printf("%d.\n", __LINE__)
static uint8_t *convert_Mat_to_BMP(Mat * img, int *bmp_size);

int main(int argc, char **argv)
{
    int idx = 0;

    InitCameraViaID(0, 1, 800, 600);

    int bmp_size, fd;
    uint8_t *bmp = NULL;

    while (idx++ < 10) {
        Mat frame = Get_Picture(0);
        bmp = convert_Mat_to_BMP(&frame, &bmp_size);
        save_bmp_to_file(idx, bmp, bmp_size);

        usleep(1000000);
    }

    FinalCamera();
    return 0;
}

    //convert Mat-object to bmp-struct.

    //https://www.cnblogs.com/lzlsky/archive/2012/08/16/2641698.html
typedef uint32_t DWORD;
typedef uint16_t WORD;
typedef int32_t LONG;
typedef uint8_t BYTE;

typedef struct tagBITMAPFILEHEADER {
    WORD bfType;        //2
    DWORD bfSize;        //6
    WORD bfReserved1;    //8
    WORD bfReserved2;    //10
    DWORD bfOffBits;    //14
} __attribute__ ((packed)) BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
    DWORD biSize;        //4
    LONG biWidth;        //8
    LONG biHeight;        //12
    WORD biPlanes;        //14
    WORD biBitCount;    //16
    DWORD biCompression;    //20
    DWORD biSizeImage;    //24
    LONG biXPelsPerMeter;    //28
    LONG biYPelsPerMeter;    //32
    DWORD biClrUsed;    //36
    DWORD biClrImportant;    //40
} __attribute__ ((packed)) BITMAPINFOHEADER;

typedef struct tagRGBQUAD {
    BYTE rgbBlue;
    BYTE rgbGreen;
    BYTE rgbRed;
    BYTE rgbReserved;
} __attribute__ ((packed)) RGBQUAD;

    //return the pointer of BMP-in-memeory.
    /*
       input: Mat -> img
       output: bmp_size
       return: bmp_buffer pointer. (need to free by USER)
     */
static uint8_t *convert_Mat_to_BMP(Mat * img, int *bmp_size)
{
    uint8_t *pBmpArray = NULL;
    //======建立位图信息 ===========
    int width, height, depth, channel;
    width = img->cols;
    height = img->rows;
    depth = img->depth();
    channel = img->channels();

    printf("w=%d h=%d d=%d c=%d\n", width, height, depth, channel);

    int bits, colors;
    uint8_t idx;
    bits = (8 << (depth / 2)) * channel;

#if 0
    if (bits > 8)
        colors = 0;
    else
        colors = 1 << bits;    //now, bits must be 8; so color = 256
#endif

    if (bits == 24)
        bits = 32;

    //待存储图像数据每行字节数为4的倍数
    int lineByte = (width * bits / 8 + 3) / 4 * 4;

    int colorTablesize = 0;
    if (bits == 8) {    // 256 色位图, // 设置灰阶调色板
        colorTablesize = 256 * sizeof(RGBQUAD);
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    DWORD bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize + lineByte * height;
    //bitmap 文件 buffer
    pBmpArray = (uint8_t *) malloc(bfSize);
    memset(pBmpArray, 0, bfSize);
    *bmp_size = bfSize;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    BITMAPFILEHEADER *fileHead = (BITMAPFILEHEADER *) pBmpArray;    // sizeof(BITMAPFILEHEADER) = 14;
    fileHead->bfType = 0x4D42;    //bmp类型
    //bfSize是图像文件4个组成部分之和
    fileHead->bfSize = bfSize;
    //bfOffBits是图像文件前3个部分所需空间之和
    fileHead->bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + colorTablesize;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //位图的头
    BITMAPINFOHEADER *head = (BITMAPINFOHEADER *) (&pBmpArray[sizeof(BITMAPFILEHEADER)]);    // sizeof(BITMAPINFOHEADER) = 40;
    head->biSize = 40;
    head->biWidth = width;
    head->biHeight = height;
    head->biPlanes = 1;
    head->biBitCount = bits;    //表示颜色用到的位数
    head->biCompression = 0;    //0:不压缩(用BI_RGB表示)
    head->biSizeImage = lineByte * height;    //图像数据站的字节数
    head->biXPelsPerMeter = 0;
    head->biYPelsPerMeter = 0;
    head->biClrUsed = 0;
    head->biClrImportant = 0;
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    RGBQUAD *VgaColorTab = (RGBQUAD *) (&pBmpArray[sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)]);
    if (bits == 8) {    // 256 色位图, // 设置灰阶调色板
        for (idx = 0; idx < 256; idx++) {
            VgaColorTab[idx].rgbRed = VgaColorTab[idx].rgbGreen = VgaColorTab[idx].rgbBlue = idx;
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    //======颠倒数据
    //======Mat 中从上往下存,而 bitmap 中从下往上存。 都是从左往右,并 且 bitmap 每一个点多占一个保留字节,默认 255
    unsigned char *m_pDibBits = pBmpArray + fileHead->bfOffBits;    //存储图像中的数据,从下向上,从左向右 //x行 * Y列
    int x, y;
    unsigned char *bmpdata;
    unsigned char *imgData = img->data;

    if (bits == 8) {
        //把 imgData 中的第一行复制到 m_pDibBits 的最后一行 , 依次颠倒
        for (x = 0; x < height; x++) {
            bmpdata = m_pDibBits + (height - 1 - x) * width;
            memcpy(bmpdata, imgData, width);
            imgData = imgData + width;
        }
    } else if (bits == 32) {
        //把 imgData 中的第一行复制到 m_pDibBits 的最后一行 , 依次颠倒
        for (x = 0; x < height; x++) {
            bmpdata = m_pDibBits + (height - 1 - x) * width * 4;
            for (y = 0; y < width; y++) {
                memcpy(bmpdata, imgData, 3);
                bmpdata[3] = 255;
                bmpdata = bmpdata + 4;
                imgData = imgData + 3;
            }
        }
    }
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    return pBmpArray;
}
