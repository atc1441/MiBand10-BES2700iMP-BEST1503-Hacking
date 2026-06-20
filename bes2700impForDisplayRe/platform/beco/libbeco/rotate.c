//
// Copyright 2020 BES Technic
//
// Description: Support functions to rotate (transpose) small tiles and strips.

#include "beco_common.h"
#include "beco_rotate.h"





/*
        ==========
         0  1   2 ...
Strip0  |00|22 |44|66
        |11|33 |55|77
        ----------
Strip1  |00|22 ...
        |11|33
        ----------

Rotate to StripOrder =>

        ========================
Strip0  0 |00|11| |44|55
        1 |22|33| |66|77
        ------------------------
Strip0  ...
*/
extern inline void beco_rotate_2x2x16bit_dest(uint32_t *dest, const uint32_t *p, size_t stride, size_t stride_d)
{
    uint32_t v0,v1;

    v0 = p[0*stride];
    v1 = p[1*stride];

    BECO_ROT(v0,v1, 0x0000ffff, 16);

    dest[0*stride_d] = v0;
    dest[1*stride_d] = v1;
}


extern inline void beco_rotate_4x2x16bit_to_dest(uint32_t *dest, const uint32_t *p, size_t stride, size_t stride_d)
{
    uint32_t v0,v1,v2,v3;

    v0 = p[0];
    v1 = p[1*stride];
    v2 = p[2*stride];
    v3 = p[3*stride];

    BECO_ROT(v0,v1, 0x0000ffff, 16);
    BECO_ROT(v2,v3, 0x0000ffff, 16);

    dest[0*stride_d] = v0;
    dest[1*stride_d] = v2;
    dest[2*stride_d] = v1;
    dest[3*stride_d] = v3;
}


extern inline void beco_rotate_2x2x16bit(uint32_t *p, size_t stride)
{
    beco_rotate_2x2x16bit_dest(p, p, stride, stride);
}


extern inline void rotate_4x16_tileorder(uint32_t *d, const uint32_t *s, int width, size_t stride_s)
{
    int i;
    const size_t stride_d = 1;

    for (i = 0; i < width; i++) {
        beco_rotate_4x2x16bit_to_dest(d, s + i, stride_s, stride_d);
        d += 4; // number of rows converted to linear elements by the rotate call.
    }
}


/*
        ==========
         0 1 2 3 ...
Strip0  |0|0|0|0
        |1|1|1|1
        |2|2|2|...
        |3|3|3|
        ----------
Strip1  |0|0|0|
        |1|1|1|
        |2|2|2|...
        |3|3|3|
        ----------
Rotate to StripOrder =>

        ========================
Strip0  0 |0|1|2|3| 4 |0|1|2|3|
        1 |0|1|2|3| 5 |0|1|2|3|
        2 |0|1|2|3| . ....
        3 |0|1|2|3|
        ------------------------
Strip0  |0|1|2|3||
        ...
*/
extern inline void beco_rotate_4x4x8bit_dest(uint32_t *dest, const uint32_t *p, size_t stride, size_t stride_d)
{
    uint32_t v0,v1,v2,v3;

    v0 = p[0];
    v1 = p[stride];
    v2 = p[2*stride];
    v3 = p[3*stride];

    BECO_ROT(v0,v1, 0x00ff00ff,  8);
    BECO_ROT(v2,v3, 0x00ff00ff,  8);
    BECO_ROT(v0,v2, 0x0000ffff, 16);
    BECO_ROT(v1,v3, 0x0000ffff, 16);

    dest[0*stride_d] = v0;
    dest[1*stride_d] = v1;
    dest[2*stride_d] = v2;
    dest[3*stride_d] = v3;
}


extern inline void beco_rotate_4x4x8bit(uint32_t *p, size_t stride)
{
    beco_rotate_4x4x8bit_dest(p, p, stride, stride);
}


static inline void rotate_4x8_tileorder(uint32_t *d, const uint32_t *s, int width, size_t stride_s, size_t stride_d)
{
    int i;

    for (i = 0; i < width; i++) {
        beco_rotate_4x4x8bit_dest(d, s + i, stride_s, stride_d);
        d += 4*stride_d;
    }
}



// Rotate 4x8bit col vectors to 4x8bit row vectors
// Operation is inplace. Output it in "StripOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 4 rows.
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_4x8bit_col_to_row_inplace(uint8_t *p, int width, size_t stride)
{
    const size_t elements_per_word = (sizeof(uint32_t)/sizeof(*p));
    
    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;

    rotate_4x8_tileorder((uint32_t*)p, (const uint32_t*)p, width, stride, stride);
}



// Rotate 2x16bit col vectors to 2x16bit row vectors
// Operation is inplace. Output it in "StripOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_2x16bit_col_to_row_inplace(uint16_t *p, int width, size_t stride)
{
    int i;
    const size_t elements_per_word = (sizeof(uint32_t)/sizeof(*p));
    uint32_t *q = (uint32_t *)p;

    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;

    for (i = 0; i < width; i++) {
//        beco_rotate_2x2x16bit(q, stride);
        beco_rotate_2x2x16bit_dest(q, q, stride, stride);
        q += 1;
    }
}



// Rotate 4x8bit col vectors to 4x8bit row vectors
// Operation is to second buffer of width x 32bit.
// Output it in "TileOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 4 rows.
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_4x8bit_col_to_row_tileorder(uint32_t *d, const uint8_t *p, int width, size_t stride)
{
    int i;
    const size_t elements_per_word = (sizeof(*d)/sizeof(*p));
    
    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;

    for (i = 0; i < width; i++) {
        beco_rotate_4x4x8bit_dest(d, (uint32_t*)p, stride, 1);
        d += elements_per_word;
        p += elements_per_word;
    }
}


// Rotate 2x16bit col vectors to 2x16bit row vectors
// Operation is to second buffer of width x 32bit.
// Output it in "TileOrder".
//
// Parameter:
//    p    - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_2x16bit_col_to_row_tileorder(uint32_t *d, const uint16_t *p, int width, size_t stride)
{
    int i;
    const size_t elements_per_word = (sizeof(*d)/sizeof(*p));
    
    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;

    for (i = 0; i < width; i++) {
        beco_rotate_2x2x16bit_dest(d, (uint32_t*)p, stride, 1);
        d += elements_per_word;
        p += elements_per_word;
    }
}


// Rotate 8x8bit col vectors to 8x8bit row vectors
// Operation is to second buffer of width x 64bit (2x32bit).
// Output it in "TileOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 8 rows. (width*2 uint32_t's)
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_8x8bit_col_to_row_tileorder(uint32_t *d, const uint8_t *p, int width, size_t stride)
{
    int i;
    const size_t elements_per_word = (sizeof(*d)/sizeof(*p));
    const uint32_t *q = (const uint32_t*)p;

    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;
#if 1
    for (i = 0; i < 2; i++) {
        rotate_4x8_tileorder(d, q, width, stride, 2);
        d += 1;
        q += stride*4;
    }
#else    
    rotate_4x8_tileorder(d,     q,            width, stride, 2);
    rotate_4x8_tileorder(d + 1, q + stride*4, width, stride, 2);
#endif

#if 0
    q = (uint32_t*)p;
    for (i = 0; i < width; i++) {
        beco_rotate_4x4x8bit_destx(d + elements_per_word*i*2, q + i, stride, 2);
    }

    uint32_t *q += stride * 4;
    d += 1;
    for (i = 0; i < width; i++) {
        beco_rotate_4x4x8bit_destx(d + elements_per_word*i*2+1, q + i, stride, 2);
    }
#endif
}


// Rotate 4x16bit col vectors to 4x16bit row vectors
// Operation is to second buffer of width x 64bit (2x32bit).
// Output it in "TileOrder".
//
// Parameter:
//    p    - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_4x16bit_col_to_row_tileorder(uint32_t *d, const uint16_t *p, int width, size_t stride)
{
    const size_t elements_per_word = (sizeof(*d)/sizeof(*p));
    
    // Calculate stride width in 32bit words
    stride = stride / elements_per_word;
    width = width / elements_per_word;

    rotate_4x16_tileorder(d, (const uint32_t *)p, width, stride);
}

#ifdef MAIN

// This part require C++
// Compile with 
//     g++ -o rotate-test -Wall -DMAIN -O2 libbeco/rotate.c -Iinclude

#include <stdio.h>
#include "beco_print_matrix.hpp"


int main()
{
    uint32_t xx[] = {0x13121110, 0x23222120, 0x33323130, 0x43424140};
    uint8_t  strip_8x16[] = {
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,
        20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
        40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,
        60,61,62,63,64,65,66,67,68,69,70,71,72,73,74,75,

        80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,
        100,101,102,103,104,105,106,107,108,109,110,111,112,113,114,115,
        120,121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,
        140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155
    };
    uint16_t  strip_4x16[] = {
        140,141,142,143,144,145,146,147,148,149,150,151,152,153,154,155,
        160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,

        240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255,
        260,261,262,263,264,265,266,267,268,269,270,271,272,273,274,275
    };
    uint32_t  strip_to[32];
        
    printf("Rotate input 4x4 matrix of 8bit\n");
    print_matrix((uint8_t*)xx, 4, 4, 4);

    printf("\nRotate output 4x4 matrix of 8bit\n");
    beco_rotate_4x4x8bit(xx, 1);
    print_matrix((uint8_t*)xx, 4, 4, 4);

    printf("\nRotate input 2x2 matrix of 16bit\n");
    print_matrix((uint16_t*)xx, 2, 2, 2);

    printf("\nRotate output 2x2 matrix of 16bit\n");
    beco_rotate_2x2x16bit(xx, 1);
    print_matrix((uint16_t*)xx, 2, 2, 2);


    printf("\nRotate 4x16 strip 8bit to tile-order\n");
    beco_rotate_4x8bit_col_to_row_tileorder(strip_to, strip_8x16, 16, 16);
    print_matrix((uint8_t*)strip_to, 4, 16, 4);

    printf("\nRotate 2x16 strip 16bit to tile-order\n");
    beco_rotate_2x16bit_col_to_row_tileorder(strip_to, strip_4x16, 16, 16);
    print_matrix((uint16_t*)strip_to, 2, 16, 2);


    printf("\nRotate 8x8 strip 8bit to tile-order\n");
    beco_rotate_8x8bit_col_to_row_tileorder(strip_to, strip_8x16, 16, 16);
    print_matrix((uint8_t*)strip_to, 8, 16, 8);

    printf("\nRotate 4x16 strip 16bit to tile-order\n");
    beco_rotate_4x16bit_col_to_row_tileorder(strip_to, strip_4x16, 16, 16);
    print_matrix((uint16_t*)strip_to, 4, 16, 4);
}
#endif


