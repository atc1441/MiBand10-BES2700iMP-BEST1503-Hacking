#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Beco acc read-out functions
//

#ifndef _BECO_READ_H
#define _BECO_READ_H

#include <stdint.h>
#include "beco_common.h"

BECO_C_DECLARATIONS_START

void beco_read_acc0_1x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc0_2x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc0_2x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc0_4x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc0_4x4(beco_vec32_out_t rd[], size_t stride);

void beco_read_acc1_1x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc1_2x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc1_2x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc1_4x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc1_4x4(beco_vec32_out_t rd[], size_t stride);

void beco_read_acc2_1x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc2_2x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc2_2x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc2_4x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc2_4x4(beco_vec32_out_t rd[], size_t stride);

void beco_read_acc3_1x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc3_2x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc3_2x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc3_4x2(beco_vec32_out_t rd[], size_t stride);
void beco_read_acc3_4x4(beco_vec32_out_t rd[], size_t stride);

// Create aliases for common read-out types.

#define BECO_REF_ALIAS(fn, wafn) void wafn(beco_vec32_out_t rd[], size_t stride)


// single acc, 8x8 bit
BECO_REF_ALIAS(beco_read_acc0_1x4, beco_read_acc0_8x8_8bit);
BECO_REF_ALIAS(beco_read_acc1_1x4, beco_read_acc1_8x8_8bit);
BECO_REF_ALIAS(beco_read_acc2_1x4, beco_read_acc2_8x8_8bit);
BECO_REF_ALIAS(beco_read_acc3_1x4, beco_read_acc3_8x8_8bit);

BECO_REF_ALIAS(beco_read_acc0_2x4, beco_read_acc0_8x8_16bit);
BECO_REF_ALIAS(beco_read_acc1_2x4, beco_read_acc1_8x8_16bit);
BECO_REF_ALIAS(beco_read_acc2_2x4, beco_read_acc2_8x8_16bit);
BECO_REF_ALIAS(beco_read_acc3_2x4, beco_read_acc3_8x8_16bit);

BECO_REF_ALIAS(beco_read_acc0_4x4, beco_read_acc0_8x8_32bit);
BECO_REF_ALIAS(beco_read_acc1_4x4, beco_read_acc1_8x8_32bit);
BECO_REF_ALIAS(beco_read_acc2_4x4, beco_read_acc2_8x8_32bit);
BECO_REF_ALIAS(beco_read_acc3_4x4, beco_read_acc3_8x8_32bit);


// single acc, 16x16 bit
BECO_REF_ALIAS(beco_read_acc0_2x2, beco_read_acc0_16x16_32bit);
BECO_REF_ALIAS(beco_read_acc1_2x2, beco_read_acc1_16x16_32bit);
BECO_REF_ALIAS(beco_read_acc2_2x2, beco_read_acc2_16x16_32bit);
BECO_REF_ALIAS(beco_read_acc3_2x2, beco_read_acc3_16x16_32bit);


// single acc, 16x8 bit
BECO_REF_ALIAS(beco_read_acc0_2x4, beco_read_acc0_16x8_32bit);
BECO_REF_ALIAS(beco_read_acc1_2x4, beco_read_acc1_16x8_32bit);
BECO_REF_ALIAS(beco_read_acc2_2x4, beco_read_acc2_16x8_32bit);
BECO_REF_ALIAS(beco_read_acc3_2x4, beco_read_acc3_16x8_32bit);

BECO_REF_ALIAS(beco_read_acc0_4x2, beco_read_acc0_16x8_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc1_4x2, beco_read_acc1_16x8_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc2_4x2, beco_read_acc2_16x8_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc3_4x2, beco_read_acc3_16x8_32bit_rot90);

BECO_REF_ALIAS(beco_read_acc0_1x4, beco_read_acc0_16x8_16bit);
BECO_REF_ALIAS(beco_read_acc1_1x4, beco_read_acc1_16x8_16bit);
BECO_REF_ALIAS(beco_read_acc2_1x4, beco_read_acc2_16x8_16bit);
BECO_REF_ALIAS(beco_read_acc3_1x4, beco_read_acc3_16x8_16bit);

BECO_REF_ALIAS(beco_read_acc0_2x2, beco_read_acc0_16x8_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc1_2x2, beco_read_acc1_16x8_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc2_2x2, beco_read_acc2_16x8_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc3_2x2, beco_read_acc3_16x8_16bit_rot90);

// single acc, 8x16 bit
BECO_REF_ALIAS(beco_read_acc0_4x2, beco_read_acc0_8x16_32bit);
BECO_REF_ALIAS(beco_read_acc1_4x2, beco_read_acc1_8x16_32bit);
BECO_REF_ALIAS(beco_read_acc2_4x2, beco_read_acc2_8x16_32bit);
BECO_REF_ALIAS(beco_read_acc3_4x2, beco_read_acc3_8x16_32bit);

BECO_REF_ALIAS(beco_read_acc0_2x4, beco_read_acc0_8x16_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc1_2x4, beco_read_acc1_8x16_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc2_2x4, beco_read_acc2_8x16_32bit_rot90);
BECO_REF_ALIAS(beco_read_acc3_2x4, beco_read_acc3_8x16_32bit_rot90);

BECO_REF_ALIAS(beco_read_acc0_2x2, beco_read_acc0_8x16_16bit);
BECO_REF_ALIAS(beco_read_acc1_2x2, beco_read_acc1_8x16_16bit);
BECO_REF_ALIAS(beco_read_acc2_2x2, beco_read_acc2_8x16_16bit);
BECO_REF_ALIAS(beco_read_acc3_2x2, beco_read_acc3_8x16_16bit);

BECO_REF_ALIAS(beco_read_acc0_1x4, beco_read_acc0_8x16_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc1_1x4, beco_read_acc1_8x16_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc2_1x4, beco_read_acc2_8x16_16bit_rot90);
BECO_REF_ALIAS(beco_read_acc3_1x4, beco_read_acc3_8x16_16bit_rot90);


//
// 64 bit input, 4 acc read-out
//
void beco_read_all_8x8(beco_vec32_out_t rd[], size_t stride);
void beco_read_all_4x8(beco_vec32_out_t rd[], size_t stride);
void beco_read_all_2x8(beco_vec32_out_t rd[], size_t stride);
void beco_read_all_8x4(beco_vec32_out_t rd[], size_t stride);
void beco_read_all_4x4(beco_vec32_out_t rd[], size_t stride);

BECO_REF_ALIAS(beco_read_all_8x8, beco_read_all_8x8_32bit);
BECO_REF_ALIAS(beco_read_all_4x8, beco_read_all_8x8_16bit);
BECO_REF_ALIAS(beco_read_all_2x8, beco_read_all_8x8_8bit);

BECO_REF_ALIAS(beco_read_all_8x4, beco_read_all_16x8_32bit);
BECO_REF_ALIAS(beco_read_all_4x4, beco_read_all_16x8_16bit);

BECO_REF_ALIAS(beco_read_all_4x8, beco_read_all_8x16_32bit);
BECO_REF_ALIAS(beco_read_all_2x8, beco_read_all_8x16_16bit);

BECO_REF_ALIAS(beco_read_all_4x4, beco_read_all_16x16_32bit);

BECO_C_DECLARATIONS_END

#ifdef __cplusplus
#endif


#endif

