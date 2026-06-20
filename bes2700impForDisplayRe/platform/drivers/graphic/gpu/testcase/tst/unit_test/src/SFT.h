#ifndef __SFT_H__
#define __SFT_H__

#include "vg_lite.h"
#include "vg_lite_util.h"

#include <stdio.h>
#include "util.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define _SFT_MEM_CHK_ 0

extern vg_lite_buffer_t *g_fb;
extern float WINDSIZEX;
extern float WINDSIZEY;
extern vg_lite_filter_t filter;

#if ( defined WINCE || defined WCE )
#define itoa _itoa
#endif

#ifdef _WIN32
typedef float float_t;
#define __func__ __FUNCTION__
#endif

#if ( defined(_WIN32) || defined __LINUX__ || defined __NUCLEUS__  || defined(__QNXNTO__) || defined(__APPLE__))
    #ifndef TRUE
        #define TRUE 1
        #define FALSE 0
    #endif
    typedef unsigned char   BYTE;
#if (defined(_WIN32) || defined(__LINUX__) || defined(__QNXNTO__) || defined(__APPLE__))
    typedef int BOOL;
    #define assert(exp) { if(! (exp)) printf("%s: line %d: assertion.\n", __FILE__, __LINE__); }
#else
    #define assert(exp) 
#endif
    typedef unsigned long   DWORD;
    typedef unsigned short  WORD;
    typedef long            LONG;

    #ifndef min
    #define min(a,b)            (((a) < (b)) ? (a) : (b))
    #endif
    #ifndef max
    #define max(a,b)            (((a) > (b)) ? (a) : (b))
    #endif

    char * itoa (int num, char* str, int t);
#endif

extern char LogString[];

#if _SFT_MEM_CHK_
void * sft_malloc(int line, char *file, unsigned int size);
void sft_free (int line, char *file, void *p);
#   define MALLOC(x) sft_malloc(__LINE__, __FILE__, x)
#   define FREE(x) sft_free(__LINE__, __FILE__, x)
#else
#   define MALLOC(x) malloc(x)
#   define FREE(x) free(x)
#endif

#define ALIGN(value,base)   ((value + base - 1) & ~(base-1))

/* Quick names for path commands */
#define PATH_DONE       0x00
#define PATH_CLOSE      0x01
#define MOVE_TO         0x02
#define MOVE_TO_REL     0x03
#define LINE_TO         0x04
#define LINE_TO_REL     0x05
#define QUAD_TO         0x06
#define QUAD_TO_REL     0x07
#define CUBI_TO         0x08
#define CUBI_TO_REL     0x09
#define NUM_PATH_CMD    10

#define BLEND_COUNT                9
#define IMAGEMODEL_COUNT           3
#define FILTER_COUNT               3
#define FORMATS_COUNT              4
#define SRC_FORMATS_CAT_COUNT      9
#define DST_FORMATS_CAT_COUNT      8
#define SRC_FORMATS_COUNT          25
#define DST_FORMATS_COUNT          24
#define QUALITY_COUNT              4

extern int SFT_REFER;
void StatesReset(void);

vg_lite_error_t Clear(void);
void Clear_Log(void);
vg_lite_error_t Draw_Image(void);
void Draw_Image_Log(void);
vg_lite_error_t Tessellation(void);
void Tessellation_Log(void);
vg_lite_error_t Combination(void);
void Combination_Log(void);

#endif
