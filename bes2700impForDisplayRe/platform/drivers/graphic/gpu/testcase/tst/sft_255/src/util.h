#ifndef __SFT_UTIL_H__
#define __SFT_UTIL_H__

#include "vg_lite.h"
#include "vg_lite_util.h"

void Get_Dirs();
extern char *MAIN_DIR;
extern char *BMP_DIR;
extern char *LOG_DIR;
extern char *MAIN_LOG;
extern char *INP_DIR;

void SavePNG(vg_lite_buffer_t *buf, char *file_name, int i);

unsigned char*    Load_BMP(char *file_name, int *height, int *width);
void        Clear_Screen(vg_lite_color_t color);
float       Random_r(float, float);
unsigned long int Random_i(unsigned long int, unsigned long int);
void        random_srand(unsigned int seed);

vg_lite_color_t GenColor_r();

uint32_t get_bpp(vg_lite_buffer_format_t format);
void * gen_image(int type, vg_lite_buffer_format_t format, uint32_t width, uint32_t height);
void * gen_checker(vg_lite_buffer_format_t format, uint32_t width, uint32_t height);
void * gen_gradient(vg_lite_buffer_format_t format, uint32_t width, uint32_t height);
void * gen_solid(vg_lite_buffer_format_t format, uint32_t width, uint32_t height);
uint32_t pack_pixel(vg_lite_buffer_format_t format, uint32_t r, uint32_t g, uint32_t b, uint32_t a);


// BMP/Log files operations
int InitBMP(void);
void DestroyBMP(void);
int            SaveBMP(char *image_name, unsigned char* p, int width, int height, vg_lite_buffer_format_t format, int stride);
void        SaveBMP_SFT(char * name, vg_lite_buffer_t *buffer);

// Random operations
float        Random_r(float, float);
unsigned long int Random_i(unsigned long int, unsigned long int);
void        random_srand(unsigned int seed);

extern float    WINDSIZEX;
extern float    WINDSIZEY;

typedef struct {
    int x;
    int y;
    int w;
    int h;
} rect_s;

// Divide the screen to 15 copy areas.
extern rect_s c_rect[15];

#endif //__SFT_UTIL_H__
