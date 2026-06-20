#ifndef __SFT_H__
#define __SFT_H__

#include "vg_lite.h"
#include "vg_lite_util.h"

#include <stdio.h>
#include "util.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

extern float WINDSIZEX;
extern float WINDSIZEY;

#define __func__ __FUNCTION__

#ifndef TRUE
    #define TRUE 1
    #define FALSE 0
#endif
#define assert(exp) { if(! (exp)) printf("%s: line %d: assertion.\n", __FILE__, __LINE__); }

#define MALLOC(x) malloc(x)
#define FREE(x) free(x)

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

#define COLOR_COUNT         16
#define BLEND_COUNT         9
#define IMAGEMODEL_COUNT    3
#define FILTER_COUNT       3
#define FORMATS_COUNT       4
#define QUALITY_COUNT       4
#define OPERATE_COUNT       9

vg_lite_error_t Pattern_Test(void);
vg_lite_error_t Gradient_Test(void);
vg_lite_error_t Matrix_Test(void);
vg_lite_error_t API_Test(void);
vg_lite_error_t Memory_Test(void);

#endif
