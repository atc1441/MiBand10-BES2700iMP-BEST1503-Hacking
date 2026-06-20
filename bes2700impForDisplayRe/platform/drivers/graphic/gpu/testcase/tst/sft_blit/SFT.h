#ifndef __SFT_H__
#define __SFT_H__

// VGLite headers.
#include "vg_lite.h"
#include "vg_lite_util.h"

#include <stdio.h>
#include "util.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>
#define _SFT_MEM_CHK_ 0

extern vg_lite_buffer_t * sys_fb;
extern int   fb_width;
extern int   fb_height;
extern vg_lite_filter_t filter;

#if ( defined WINCE || defined WCE )
#define itoa _itoa
#endif

#ifdef _WIN32
typedef float float_t;
#define __func__ __FUNCTION__
#endif

#if (defined _WIN32 || defined __LINUX__ || defined __NUCLEUS__  || defined(__QNXNTO__) || defined(__APPLE__))
    #ifndef TRUE
        #define TRUE 1
        #define FALSE 0
    #endif
    typedef unsigned char    BYTE;
#if (defined(_WIN32) || defined(__LINUX__) || defined(__QNXNTO__) || defined(__APPLE__))
    typedef int                BOOL;
    #define assert(exp) { if(! (exp)) printf("%s: line %d: assertion.\n", __FILE__, __LINE__); }
#else
    #define assert(exp) 
#endif
    typedef unsigned long    DWORD;
    typedef unsigned short    WORD;
    typedef long            LONG;
    //typedef int bool;
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
#    define MALLOC(x) sft_malloc(__LINE__, __FILE__, x)
#    define FREE(x) sft_free(__LINE__, __FILE__, x)
#else
#    define MALLOC(x) malloc(x)
#    define FREE(x) free(x)
#endif

// Quick names for path commands
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

#if 0
static int data_count[] = 
{
0, 
0, 
2,
2,
2,
2,
4,
4,
6,
6
};
#endif
extern int SFT_REFER;
void StatesReset(void);

//$$Case files:
void SFT_Clear(void);
void SFT_Clear_Log(void);
void SFT_Blit(void);
void SFT_Path_Draw(void);

#endif //__SFT_H__
