#ifndef __SFT_H__
#define __SFT_H__

/* VGLite headers. */
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "vg_lite_tst_platform.h"
#if (defined (VGLITE_TST_FIRMWARE) && !defined (__LINUX__))
#include "vg_lite_hal.h"
#endif
#include "gfx_sft_util.h"

#define VGL_TST_UNUSED(x) (void)(x)

#if !defined (VGLITE_TST_FIRMWARE)
#include <stdio.h>
#endif
#include <math.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <stdint.h>
#endif

#define _SFT_MEM_CHK_ 0

#if ( defined WINCE || defined WCE )
#define itoa _itoa
#endif

#ifdef _WIN32
typedef float float_t;
#endif

#if ( defined _WIN32 || defined __LINUX__ || defined __NUCLEUS__  || defined(__QNXNTO__) || defined(__APPLE__) || defined(VGLITE_TST_FIRMWARE))
    #ifndef TRUE
        #define TRUE 1
        #define FALSE 0
    #endif
    typedef unsigned char    BYTE;
#if (defined(__LINUX__) || defined(__QNXNTO__) || defined(__APPLE__) || defined(VGLITE_TST_FIRMWARE))
    typedef int                BOOL;
    #define assert(exp) { if(! (exp)) VGLITE_TST_PRINTF("%s: line %d: assertion.\r\n", __FILE__, __LINE__); }
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

#if defined (VGLITE_TST_FIRMWARE)
#   define MALLOC(x) vg_lite_malloc(x)
#   define FREE(x) vg_lite_free2(x)
#else
#if _SFT_MEM_CHK_
void * sft_malloc(int line, char *file, unsigned int size);
void sft_free (int line, char *file, void *p);
#    define MALLOC(x) sft_malloc(__LINE__, __FILE__, x)
#    define FREE(x) sft_free(__LINE__, __FILE__, x)
#else
#    define MALLOC(x) malloc(x)
#    define FREE(x) free(x)
#endif
#endif

#define VG_LITE_ONERROR(func) \
if ((ret = func) != TB_OK) \
return ret

/* Returning status of a function. */
#define TB_OK       0
#define TB_BAD     -1
#define TB_BAD_RAW_MEMCMP     -2
#define TB_BAD_API_FAIL     -3
#define TB_BAD_GENERIC     -4

/* Quick names for raw files. */
#define GFX_1_GOLDEN ("gfx_1_golden.raw")
#define GFX_2_3_GOLDEN ("gfx_2_3_golden.raw")
#define GFX_4_GOLDEN_1 ("gfx_4_golden_1.raw")
#define GFX_4_GOLDEN_2 ("gfx_4_golden_2.raw")
#define GFX_5_GOLDEN_1 ("gfx_5_golden_1.raw")
#define GFX_5_GOLDEN_2 ("gfx_5_golden_2.raw")
#define GFX_6_GOLDEN_1 ("gfx_6_golden_1.raw")
#define GFX_6_GOLDEN_2 ("gfx_6_golden_2.raw")
#define GFX_6_GOLDEN_3 ("gfx_6_golden_3.raw")
#define GFX_6_GOLDEN_4 ("gfx_6_golden_4.raw")
#define GFX_7_GOLDEN_1 ("gfx_7_golden_1.raw")
#define GFX_7_GOLDEN_2 ("gfx_7_golden_2.raw")
#define GFX_7_GOLDEN_3 ("gfx_7_golden_3.raw")
#define GFX_8_GOLDEN ("gfx_8_golden.raw")
#define GFX_9_10_GOLDEN ("gfx_9_10_golden.raw")
#define GFX_11_GOLDEN_1 ("gfx_11_golden_1.raw")
#define GFX_11_GOLDEN_2 ("gfx_11_golden_2.raw")

/* Quick names for path commands. */
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

/* Case files. */
int exe_clear(int user_fb_width, int user_fb_height, vg_lite_buffer_t * raw_fb);
int exe_ui(int user_fb_width, int user_fb_height, vg_lite_buffer_t * raw_fb);

#define SFT_BLIT_COLOR_FORMAT_TEST_CODE  (1)
#define SFT_BLIT_RESOLUTION_TEST_CODE    (2)
#define SFT_BLIT_STRIDE_TEST_CODE        (3)
int SFT_Blit(int tst_code, vg_lite_buffer_t * raw_fb, int user_fb_width, int user_fb_height);

#define SFT_DRAW_FILL_RULE_TEST_CODE     (1)
#define SFT_DRAW_PATH_COORD_TEST_CODE    (2)
#define SFT_DRAW_ANTI_ALIAS_TEST_CODE    (3)
#define SFT_DRAW_GENERIC_TEST_CODE       (4)
int SFT_Path_Draw(int tst_code, vg_lite_buffer_t * raw_fb, int user_fb_width, int user_fb_height);

void clear_screen(vg_lite_color_t color);
void sys_fb_blit(vg_lite_buffer_t * source, vg_lite_blend_t blend);

#endif //__SFT_H__
