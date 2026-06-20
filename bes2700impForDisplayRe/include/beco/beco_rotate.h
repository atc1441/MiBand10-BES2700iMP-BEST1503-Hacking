#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Support functions to rotate (transpose) small tiles and strips.

#ifndef _BECO_ROTATE_H
#define _BECO_ROTATE_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

#include <stddef.h>
#include "beco_asm.h"

/*
Definitions.

Matrix order:

    1 Normal:
    -----------
    |  0 10 20
    |  1 11 21
    |  2 12 22
    |  3 13 23
    |  4 14 24
    |  :  :  :
    -----------

    2 Transposed:
    --------------------
    |  0  1  2  3  4 ..
    | 10 11 12 13 ..
    | 20 21 22 23 ..
    --------------------

Tile order:
    3 Tile-order
    ---------------
    |  0  1  2  3 |
    | 10 11 12 13 |
    | 20 21 22 23 |
    ---------------
    |  4 ..
    | 14 ..
    | 24 ..
    \/\/\/\/\/\/\

    4 Strip-order (strip 2 high)
    ---------------------------------\
    |  0  1  2  3 | 20 21 22 23 | .. /
    | 10 11 12 13 | ..          | .. \
    ---------------------------------/
    |  4 ..       | ..          | .. /
    | 14 ..       | ..          | .. \
    ---------------------------------/
*/


#define BECO_ROT16(a,b, mask, shift)   do {\
        uint32_t ta;                       \
        ta = BECO_PKHBT(a,b, shift);       \
        b = BECO_PKHTB(b,a, shift);        \
        a = ta;                            \
    } while(0)

#define BECO_ROTX(a,b, mask, shift)   do { \
        uint32_t xm;                       \
        xm = ((a >> shift) ^ b) & mask;    \
        a ^= xm << shift;                  \
        b ^= xm;                           \
    } while(0)

#define BECO_ROT(a,b, mask, shift)   do {  \
    if (shift == 16) {                     \
        BECO_ROT16(a,b,mask,shift);        \
    }                                      \
    else {                                 \
        BECO_ROTX(a,b,mask,shift);         \
    }                                      \
    } while(0)



// Rotate 4x8bit col vectors to 4x8bit row vectors
// Operation is inplace. Output it in "StripOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 4 rows.
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_4x8bit_col_to_row_inplace(uint8_t *p, int width, size_t stride);


// Rotate 2x16bit col vectors to 2x16bit row vectors
// Operation is inplace. Output it in "StripOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_2x16bit_col_to_row_inplace(uint16_t *p, int width, size_t stride);


// Rotate 4x8bit col vectors to 4x8bit row vectors
// Operation is to second buffer of width x 32bit.
// Output it in "TileOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 4 rows.
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_4x8bit_col_to_row_tileorder(uint32_t *d, const uint8_t *p, int width, size_t stride);


// Rotate 2x16bit col vectors to 2x16bit row vectors
// Operation is to second buffer of width x 32bit.
// Output it in "TileOrder".
//
// Parameter:
//    p    - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_2x16bit_col_to_row_tileorder(uint32_t *d, const uint16_t *p, int width, size_t stride);


// Rotate 4x16bit col vectors to 4x16bit row vectors
// Operation is to second buffer of width x 64bit (2x32bit).
// Output it in "TileOrder".
//
// Parameter:
//    p    - pointer to strip of 'width' cols and 2 rows.
//    width  - number of 16 bit elements per row
//    stride - number of 16 bit elements to next row.
void beco_rotate_4x16bit_col_to_row_tileorder(uint32_t *d, const uint16_t *p, int width, size_t stride);


// Rotate 8x8bit col vectors to 8x8bit row vectors
// Operation is to second buffer of width x 64bit (2x32bit).
// Output it in "TileOrder".
//
// Parameter:
//    p      - pointer to strip of 'width' cols and 8 rows. (width*2 uint32_t's)
//    width  - number of 8 bit elements per row
//    stride - number of 8 bit elements to next row.
void beco_rotate_8x8bit_col_to_row_tileorder(uint32_t *d, const uint8_t *p, int width, size_t stride);


BECO_C_DECLARATIONS_END

#endif
