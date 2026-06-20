/***************************************************************************
 *
 * Copyright 2015-2024 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

#pragma once

// Description: Beco matrix multiply

#ifndef _BECO_MATRIX_MULT_ATB_H
#define _BECO_MATRIX_MULT_ATB_H

#include <stdlib.h>
#include "beco_common.h"
#include "beco_read.h"

#include "cmsis.h"
#include "hal_trace.h"

#define likely(x) __builtin_expect(!!(x), 1)


// Create a template to automatically insert correct optimized read-out function.
template <size_t a_elem_sz, size_t b_elem_sz, size_t c_elem_sz>
    static void beco_read_tile64(beco_vec32_out_t out[], size_t stride);

#define BECO_GEN_TILE64_READ(a,b,c, read_fn) \
    template <> inline                 \
    void beco_read_tile64<a,b,c>(beco_vec32_out_t out[], size_t stride) { \
        read_fn(out, stride);                                    \
    }

// Instantiate legal variations
//  Element Size:  A   B   C
BECO_GEN_TILE64_READ(1,  1,  1, beco_read_all_8x8_8bit)
BECO_GEN_TILE64_READ(1,  1,  2, beco_read_all_8x8_16bit)
BECO_GEN_TILE64_READ(1,  1,  4, beco_read_all_8x8_32bit)
BECO_GEN_TILE64_READ(1,  2,  2, beco_read_all_8x16_16bit)
BECO_GEN_TILE64_READ(1,  2,  4, beco_read_all_8x16_32bit)
BECO_GEN_TILE64_READ(2,  1,  2, beco_read_all_16x8_16bit)
BECO_GEN_TILE64_READ(2,  1,  4, beco_read_all_16x8_32bit)
BECO_GEN_TILE64_READ(2,  2,  4, beco_read_all_16x16_32bit)


#ifndef MMAC_64x64
#define MMAC_64x64(a, b) \
        beco_write_reg(BECO_REG0, a);      \
        beco_write_reg(BECO_REG1, b);      \
        beco_mmacrr(BECO_REG0, BECO_REG1);
#endif

#define MMACSTEP_64x64_S_S(stride_beco_a, stride_beco_b) {  \
        beco_vec64_in_t a, b;              \
        a = *p_a;                          \
        b = *p_b;                          \
        p_a += stride_beco_a;                   \
        p_b += stride_beco_b;                   \
        MMAC_64x64(a, b);                  \
    }

// matrix-multiply
// Inputs A and B, 64bit elements. A is transposed, B is in natural order
//case1: a_rows is the multiple of (sizeof(beco_vec64_in_t)/sizeof(aType))
//   and b_cols is the multiple of (sizeof(beco_vec64_in_t)/sizeof(bType))
extern inline void mmac_tile_aTb(const beco_vec64_in_t *p_a, const beco_vec64_in_t *p_b,
               size_t stride_beco_a, size_t stride_beco_b, int n)
{
#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (; n != 0; n--) {
        MMACSTEP_64x64_S_S(stride_beco_a, stride_beco_b)
    }
}

//case2: a_rows is not the multiple of (sizeof(beco_vec64_in_t)/sizeof(aType))
template <typename Ta>
static void mmac_tile_aTb_a_nomultiple(const Ta *At, const beco_vec64_in_t *beco_b,
               size_t stride_beco_b, int a_rows, int K)
{
    //beco_vec64_in_t a, b;
    int64_t a_64[1];
    beco_vec64_in_t *beco_a;

    int k;
    k = K & 7;
    K &= ~7;
#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (; K != 0; K--) {
        //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At;
        memcpy(a_64, At, 8);
        beco_a = (beco_vec64_in_t *)a_64;

        MMAC_64x64(beco_a[0], beco_b[0]);

        beco_b += stride_beco_b;
        At += a_rows;
    }
    for (; k != 0; k--) {
        //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At;
        memcpy(a_64, At, 8);
        beco_a = (beco_vec64_in_t *)a_64;

        MMAC_64x64(beco_a[0], beco_b[0]);

        beco_b += stride_beco_b;
        At += a_rows;
    }

}

//case3: b_cols is not the multiple of (sizeof(beco_vec64_in_t)/sizeof(bType))
template <typename Tb>
static void mmac_tile_aTb_b_nomultiple(const beco_vec64_in_t *beco_a, const Tb *B,
               size_t stride_beco_a, int b_cols, int K)
{
    //beco_vec64_in_t a, b;
    int64_t b_64[1];
    beco_vec64_in_t *beco_b;
    //bool unaligned_en = config_unaligned_access(true);
    int k;
    k = K & 7;
    K &= ~7;
#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (; K != 0; K--) {
        //beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B;
        memcpy(b_64, B, 8);
        beco_b = (beco_vec64_in_t *)b_64;

        //beco_vec64_in_t beco_b = (beco_vec64_in_t){.i8={B[0],B[1],B[2],B[3],B[4],B[5],B[6],B[7]}};

        MMAC_64x64(beco_a[0], beco_b[0]);
        //MMAC_64x64(beco_a[0], beco_b);

        beco_a += stride_beco_a;
        B += b_cols;
    }
    for (; k != 0; k--) {
        //beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B;
        memcpy(b_64, B, 8);
        beco_b = (beco_vec64_in_t *)b_64;

        //beco_vec64_in_t beco_b = (beco_vec64_in_t){.i8={B[0],B[1],B[2],B[3],B[4],B[5],B[6],B[7]}};

        MMAC_64x64(beco_a[0], beco_b[0]);
        //MMAC_64x64(beco_a[0], beco_b);

        beco_a += stride_beco_a;
        B += b_cols;
    }

    //config_unaligned_access(unaligned_en);
}

//case4:a_rows is not the multiple of (sizeof(beco_vec64_in_t)/sizeof(aType))
//  and b_cols is not the multiple of (sizeof(beco_vec64_in_t)/sizeof(bType))
template <typename Ta, typename Tb>
static void mmac_tile_aTb_ab_nomultiple(const Ta *At, const Tb *B,
               int a_rows, int b_cols, int K)
{
    //beco_vec64_in_t a, b;
    int64_t a_64[1];
    int64_t b_64[1];
    beco_vec64_in_t *beco_a;
    beco_vec64_in_t *beco_b;

    int k;
    k = K & 7;
    K &= ~7;
#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (; K != 0; K--) {
        //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At;
        //beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B;
        memcpy(a_64, At, 8);
        beco_a = (beco_vec64_in_t *)a_64;
        memcpy(b_64, B, 8);
        beco_b = (beco_vec64_in_t *)b_64;

        MMAC_64x64(beco_a[0], beco_b[0]);

        At += a_rows;
        B += b_cols;
    }
    for (; k != 0; k--) {
        //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At;
        //beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B;
        memcpy(a_64, At, 8);
        beco_a = (beco_vec64_in_t *)a_64;
        memcpy(b_64, B, 8);
        beco_b = (beco_vec64_in_t *)b_64;

        MMAC_64x64(beco_a[0], beco_b[0]);

        At += a_rows;
        B += b_cols;
    }
}

template <typename Ta>
static void mmac_tile_aTb_a_remained(const Ta *At, const beco_vec64_in_t *beco_b,
               size_t a_elem_per_vec64, size_t stride_beco_b, int mod_a, int loop_index, int a_rows, int K)
{
    Ta *At_r = (Ta *)malloc(a_elem_per_vec64 * sizeof(Ta));

    //beco_vec64_in_t a, b;
    int stride_rows;

#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (int i = 0; i < K; i++) {
        stride_rows = i * a_rows;
        for(int j = 0; j < mod_a; j++) {
            //At_r[j] = At[loop_index * a_elem_per_vec64 + i * a_rows + j];
            At_r[j] = At[loop_index + stride_rows + j];
        }

        beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At_r;

        MMAC_64x64(beco_a[0], beco_b[0]);

        //At += a_rows;
        beco_b += stride_beco_b;
    }
    free(At_r);
}

template <typename Tb>
static void mmac_tile_aTb_b_remained(const beco_vec64_in_t *beco_a, const Tb *B,
               size_t stride_beco_a, size_t b_elem_per_vec64, int mod_b, int loop_index, int b_cols, int K)
{
    Tb *B_r = (Tb *)malloc(b_elem_per_vec64 * sizeof(Tb));

    //beco_vec64_in_t a, b;
    int stride_cols;

#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (int i = 0; i < K; i++) {
        stride_cols = i * b_cols;
        for(int j = 0; j < mod_b; j++) {
            B_r[j] = B[loop_index + stride_cols + j];
            //B_r[j] = B[loop_index * b_elem_per_vec64 + i * b_cols + j];
            //printf("\nindex = %d, B[%d]=%d\n",(loop_index * b_elem_per_vec64 + i * b_cols + j), j,B[loop_index * b_elem_per_vec64 + i * b_cols + j]);
        }

        beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B_r;

        MMAC_64x64(beco_a[0], beco_b[0]);
        beco_a += stride_beco_a;
        //B += b_cols;
    }
    free(B_r);
}

template <typename Ta, typename Tb>
static void mmac_tile_aTb_ab_a_remained(const Ta *At, const Tb *B,
               size_t a_elem_per_vec64, int mod_a,
               int loop_index, int a_rows, int b_cols, int K)
{
    //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At,
    Ta *At_r = (Ta *)malloc(a_elem_per_vec64 * sizeof(Ta));
    int64_t b_64[1];
    beco_vec64_in_t *beco_b;
    //beco_vec64_in_t a, b;
    int stride_rows;

#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (int k = 0; k < K; k++) {
        stride_rows = k * a_rows;
        for(int j = 0; j < mod_a; j++) {
            //At_r[j] = At[loop_index * a_elem_per_vec64 + k * a_rows + j];
            At_r[j] = At[loop_index + stride_rows + j];
        }
        memcpy(b_64, B, 8);
        beco_b = (beco_vec64_in_t *)b_64;

        beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At_r;
        //beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B;

        MMAC_64x64(beco_a[0], beco_b[0]);
        //At += a_rows;
        B += b_cols;
    }
    free(At_r);
}

template <typename Ta, typename Tb>
static void mmac_tile_aTb_ab_b_remained(const Ta *At, const Tb *B,
               size_t b_elem_per_vec64, int mod_b,
               int loop_index, int a_rows, int b_cols, int K)
{
    //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At,
    Tb *B_r = (Tb *)malloc(b_elem_per_vec64 * sizeof(Tb));

    int64_t a_64[1];
    beco_vec64_in_t *beco_a;
    //beco_vec64_in_t a, b;
    int stride_cols;

#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (int k = 0; k < K; k++) {
        stride_cols = k * b_cols;
        for(int j = 0; j < mod_b; j++) {
            //B_r[j] = B[loop_index * b_elem_per_vec64 + k * b_cols + j];
            B_r[j] = B[loop_index + stride_cols + j];
        }
        memcpy(a_64, At, 8);
        beco_a = (beco_vec64_in_t *)a_64;

        //beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At;
        beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B_r;
        MMAC_64x64(beco_a[0], beco_b[0]);
        At += a_rows;
        //B += b_cols;
    }
    free(B_r);
}

template <typename Ta, typename Tb>
static void mmac_tile_aTb_ab_remained(const Ta *At, const Tb *B,
               size_t a_elem_per_vec64, size_t b_elem_per_vec64,
               int mod_a, int mod_b, int a_rows, int b_cols, int K)
{
    Ta *At_r = (Ta *)malloc(a_elem_per_vec64 * sizeof(Ta));
    Tb *B_r = (Tb *)malloc(b_elem_per_vec64 * sizeof(Tb));
    //beco_vec64_in_t a, b;
    int stride_rows;
    int stride_cols;

#pragma GCC unroll 8
#ifndef __clang__
#pragma GCC ivdep
#endif
    for (int k = 1; k <= K; k++) {
        stride_rows = k * a_rows;
        stride_cols = k * b_cols;
        for(int i = 1; i <= mod_a; i++) {
            At_r[mod_a - i] = At[stride_rows - i];
        }
        for(int j = 1; j <= mod_b; j++) {
            B_r[mod_b - j] = B[stride_cols - j];
        }

        beco_vec64_in_t *beco_a = (beco_vec64_in_t *)At_r;
        beco_vec64_in_t *beco_b = (beco_vec64_in_t *)B_r;

        MMAC_64x64(beco_a[0], beco_b[0]);
        //At += a_rows;
        //B += b_cols;
    }
    free(At_r);
    free(B_r);
}


static inline void mmac_tile_set_offset_zero(void)
{
    beco_clear_acc(BECO_ACC0);
    beco_clear_acc(BECO_ACC1);
    beco_clear_acc(BECO_ACC2);
    beco_clear_acc(BECO_ACC3);
}


// Calculate:
//   C[M][N] = (A[M][K])^T * B[K][N] = At[K][M] * B[K][N]
template <typename Ta, typename Tb, typename Tc>
void beco_mmult_aTb_basic_impl(const Ta *At, const Tb *B, Tc *C,
                    int M, int N, int K)
{
    beco_vec32_out_t *p_c;
    //const beco_vec64_in_t *p_at = At,
    //                      *p_b  = B;
    //TRACE(0,"enter basic");

    constexpr size_t a_elem_sz = sizeof(Ta);
    constexpr size_t b_elem_sz = sizeof(Tb);
    constexpr size_t c_elem_sz = sizeof(Tc);

    constexpr size_t a_elem_per_vec64 = sizeof(beco_vec64_in_t)/a_elem_sz;
    //constexpr size_t b_elem_per_vec64 = sizeof(beco_vec64_in_t)/b_elem_sz;//it is equal to tile_cw
    constexpr size_t c_elem_per_vec32 = sizeof(beco_vec32_out_t)/c_elem_sz;
    constexpr size_t tile_w_per_calculation = sizeof(beco_vec64_in_t)/b_elem_sz;//it is equal to b_elem_per_vec64

    //TRACE(0,"b_elem_per_vec64=%d",tile_w_per_calculation);

    constexpr size_t tile_beco_cw = tile_w_per_calculation / c_elem_per_vec32;
    //int tile_cw = c_elem_per_vec32 * tile_beco_cw;
    int tile_c = a_elem_per_vec64 * N;

    int mod_a = M % a_elem_per_vec64;
    int mod_b = N % tile_w_per_calculation;
    int mod_c = N % c_elem_per_vec32;

    int stride_beco_a = M / a_elem_per_vec64;
    int stride_beco_b = N / tile_w_per_calculation;
    int stride_beco_c = N / c_elem_per_vec32;
    //config_unaligned_access(true);

    if (likely(0 != mod_a && 0 != mod_b)) {
        int stride_a = M - mod_a;
        int stride_b = N - mod_b;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));

        if (likely(0 != mod_c)) {
            int stride_c = tile_c - N +mod_b;
            for (int i = 0; i < stride_beco_a; i++) {
                p_c = (beco_vec32_out_t *)C;
                for (int j = 0; j < stride_beco_b; j++) {

                    mmac_tile_set_offset_zero();
                    mmac_tile_aTb_ab_nomultiple(At + i * a_elem_per_vec64, B + j * tile_w_per_calculation, M, N, K);

                    beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                    Tc *pc_out = (Tc *)pc_remained;
                    //int index_pc = 0;
                    for(unsigned c_rows = 0; c_rows < a_elem_per_vec64; c_rows++){
                        unsigned loop_stride_c = c_rows * N;
                        for(unsigned c_cols = 0; c_cols < (tile_w_per_calculation); c_cols++) {
                            //C[c_rows * N + c_cols] = pc_out[index_pc];
                            C[loop_stride_c + c_cols] = *pc_out++;
                            //index_pc++;
                        }
                    }

                    C +=(tile_w_per_calculation);
                    //p_c += tile_w;
                }

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_ab_b_remained(At + i * a_elem_per_vec64, B, tile_w_per_calculation, mod_b, stride_b, M, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                Tc *pc_remained_out = (Tc *)pc_remained;
                for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                    unsigned loop_stride_c = k * N;
                    unsigned loop_stride_cr = k * tile_w_per_calculation;
                    for (int j = 0; j < mod_b; j++) {
                        //C[j + k * N] = pc_remained_out[j + k * tile_w_per_calculation];
                        C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                    }
                }

                //C += (tile_c - N +mod_b);
                C += stride_c;
                //C += tile_h;
            }

            //p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_ab_a_remained(At, B + j * tile_w_per_calculation, a_elem_per_vec64, mod_a, stride_a, M, N, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                Tc *pc_remained_out = (Tc *)pc_remained;
                //int index_C = 0;
                for(int i = 0; i < (mod_a); i++) {
                    int loop_stride_c = i * N;
                    for(unsigned k = 0; k < (tile_w_per_calculation); k++) {
                        C[loop_stride_c + k] = *pc_remained_out++;
                        //C[i * N + k] = pc_remained_out[index_C];
                        //index_C++;
                        //C[i * N + k + tile_w * c_elem_per_vec32 * j] = pc_remained_out[index_C];
                        //p_c[i * stride_c + tile_w * j + k] = pc_remained[index_C];

                    }
                }

                C +=(tile_w_per_calculation);
            }

            mmac_tile_set_offset_zero();
            mmac_tile_aTb_ab_remained(At, B, a_elem_per_vec64, tile_w_per_calculation, mod_a, mod_b, M, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            Tc *pc_remained_out = (Tc *)pc_remained;
            for(int i = 0; i < mod_a; i++) {
                int loop_stride_c = i * N;
                int loop_stride_cr = i * tile_w_per_calculation;
                for (int j = 0; j < mod_b; j++) {
                    C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                    //C[j + i * N] = pc_remained_out[j + i * tile_w_per_calculation];
                    //p_c[i * stride_c + j + stride_b * tile_w] = pc_remained[j + i * tile_w];
                }
            }
        } else {
            for (int i = 0; i < stride_beco_a; i++) {
                p_c = (beco_vec32_out_t *)C;
                for (int j = 0; j < stride_beco_b; j++) {

                    mmac_tile_set_offset_zero();
                    mmac_tile_aTb_ab_nomultiple(At + i * a_elem_per_vec64, B + j * tile_w_per_calculation, M, N, K);

                    beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
                    p_c += tile_beco_cw;
                }

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_ab_b_remained(At + i * a_elem_per_vec64, B, tile_w_per_calculation, mod_b, stride_b, M, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                    unsigned loop_stride_c = k * stride_beco_c;
                    unsigned loop_stride_cr = k * tile_beco_cw;
                    for (unsigned j = 0; j < (mod_b / c_elem_per_vec32); j++) {
                        p_c[j + loop_stride_c] = pc_remained[j + loop_stride_cr];
                        //p_c[j + i * stride_beco_c] = pc_remained[j + i * tile_beco_cw];
                    }
                }

                C += tile_c;
            }

            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_ab_a_remained(At, B + j * tile_w_per_calculation, a_elem_per_vec64, mod_a, stride_a, M, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                int index_C = 0;
                for(int i = 0; i < (mod_a); i++) {
                    int loop_stride_c = i * stride_beco_c;
                    for(unsigned k = 0; k < tile_beco_cw; k++) {
                        //p_c[i * stride_beco_c + k] = pc_remained[index_C];
                        p_c[loop_stride_c + k] = pc_remained[index_C];
                        index_C++;
                        //p_c[loop_stride_c + k] = *pc_remained++;
                    }
                }

                p_c +=tile_beco_cw;
            }

            mmac_tile_set_offset_zero();
            mmac_tile_aTb_ab_remained(At, B, a_elem_per_vec64, tile_w_per_calculation, mod_a, mod_b, M, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            for(int i = 0; i < mod_a; i++) {
                int loop_stride_c = i * stride_beco_c;
                int loop_stride_cr = i * tile_beco_cw;
                for (unsigned j = 0; j < (mod_b / c_elem_per_vec32); j++) {
                    //p_c[i * stride_beco_c + j] = pc_remained[j + i * tile_beco_cw];
                    p_c[loop_stride_c + j] = pc_remained[j + loop_stride_cr];
                }
            }
        }
        free(pc_remained);
    } else if((0 == mod_a) && (0 != mod_b)) {
        const beco_vec64_in_t *beco_a = (const beco_vec64_in_t *)At;
        int stride_b = N - mod_b;
        int stride_c = tile_c - N +mod_b;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));

        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;

            if (0 == mod_c) {
                for (int j = 0; j < stride_beco_b; j++) {

                    mmac_tile_set_offset_zero();
                    mmac_tile_aTb_b_nomultiple(beco_a + i, B + j * tile_w_per_calculation, stride_beco_a, N, K);

                    beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);

                    p_c += tile_beco_cw;
                    //C +=(tile_w * c_elem_per_vec32);
                }

                mmac_tile_set_offset_zero();

                mmac_tile_aTb_b_remained(beco_a + i, B, stride_beco_a, tile_w_per_calculation, mod_b, stride_b, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                Tc *pc_remained_out = (Tc *)pc_remained;
                for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                    unsigned loop_stride_c = k * N + stride_b;
                    unsigned loop_stride_cr = k * tile_w_per_calculation;
                    for (int j = 0; j < mod_b; j++) {
                        //C[j + k * N + tile_w_per_calculation * stride_beco_b] = pc_remained_out[j + k * tile_w_per_calculation];
                        C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                    }
                }
                C += tile_c;
            } else {
                for (int j = 0; j < stride_beco_b; j++) {

                    mmac_tile_set_offset_zero();
                    mmac_tile_aTb_b_nomultiple(beco_a + i, B + j * tile_w_per_calculation, stride_beco_a, N, K);

                    beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);


                    Tc *pc_out = (Tc *)pc_remained;
                    int index_pc = 0;
                    for(unsigned c_rows = 0; c_rows < a_elem_per_vec64; c_rows++){
                        unsigned loop_stride_c = c_rows * N;
                        for(unsigned c_cols = 0; c_cols < tile_w_per_calculation; c_cols++) {
                            //C[c_rows * N + c_cols] = pc_out[index_pc];
                            C[loop_stride_c + c_cols] = pc_out[index_pc];
                            index_pc++;
                            //C[loop_stride_c + c_cols] = *pc_out++;
                        }
                    }

                    C += tile_w_per_calculation;
                    //p_c += tile_w;
                }

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_b_remained(beco_a + i, B, stride_beco_a, tile_w_per_calculation, mod_b, stride_b, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

                Tc *pc_remained_out = (Tc *)pc_remained;
                for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                    unsigned loop_stride_c = k * N;
                    unsigned loop_stride_cr = k * tile_w_per_calculation;
                    for (int j = 0; j < mod_b; j++) {
                       // C[j + k * N] = pc_remained_out[j + k * tile_w_per_calculation];
                       C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                    }
                }
                C += stride_c;
            }
        }
        free(pc_remained);
    } else if((0 != mod_a) && (0 == mod_b)) {
        const beco_vec64_in_t *beco_b = (const beco_vec64_in_t *)B;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        int stride_a = M - mod_a;

        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));

        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {
                mmac_tile_set_offset_zero();
                mmac_tile_aTb_a_nomultiple(At + i * a_elem_per_vec64, beco_b + j, stride_beco_b, M, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
                p_c += tile_beco_cw;
            }

            C += tile_c;
        }

        p_c = (beco_vec32_out_t *)C;
        for (int j = 0; j < stride_beco_b; j++) {
            mmac_tile_set_offset_zero();
            mmac_tile_aTb_a_remained(At, beco_b + j, a_elem_per_vec64, stride_beco_b, mod_a, stride_a, M, K);

            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            int index_C = 0;
            int stride_cw = tile_beco_cw * j;
            for(int i = 0; i < (mod_a); i++) {
                int loop_stride_c = stride_cw + i * stride_beco_c;
                for(unsigned k = 0; k < tile_beco_cw; k++) {
                    //p_c[i * stride_beco_c + tile_beco_cw * j + k] = pc_remained[index_C];
                    p_c[loop_stride_c + k] = pc_remained[index_C];
                    index_C++;
                    //p_c[loop_stride_c + k] = *pc_remained++;
                }
            }
        }
        free(pc_remained);
    } else {
        //beco_vec32_out_t *p_c;
        const beco_vec64_in_t *p_at = (const beco_vec64_in_t *)At,
                              *p_b  = (const beco_vec64_in_t *)B;
        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb(p_at + i, p_b + j, stride_beco_a, stride_beco_b, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
                p_c += tile_beco_cw;
            }

            // Increment C to point to next tile
            C += tile_c;
        }
    }
}

template <typename Ta, typename Tb, typename Tc>
static void beco_mmult_aTb_arows_opt_impl(const Ta *At, const Tb *B, Tc *C,
                    int M, int N, int K)
{
    beco_vec32_out_t *p_c;
    //const beco_vec64_in_t *p_at = At,
    //                      *p_b  = B;

    constexpr size_t a_elem_sz = sizeof(Ta);
    constexpr size_t b_elem_sz = sizeof(Tb);
    constexpr size_t c_elem_sz = sizeof(Tc);

    constexpr size_t a_elem_per_vec64 = sizeof(beco_vec64_in_t)/a_elem_sz;
    //constexpr size_t b_elem_per_vec64 = sizeof(beco_vec64_in_t)/b_elem_sz;
    constexpr size_t c_elem_per_vec32 = sizeof(beco_vec32_out_t)/c_elem_sz;
    constexpr size_t tile_w_per_calculation = sizeof(beco_vec64_in_t)/b_elem_sz; //it is equal to b_elem_per_vec64


    constexpr size_t tile_beco_cw = tile_w_per_calculation / c_elem_per_vec32;
    int tile_c = a_elem_per_vec64 * N;

    int stride_beco_a = M / a_elem_per_vec64;
    int stride_beco_b = N / tile_w_per_calculation;
    int stride_beco_c = N / c_elem_per_vec32;

    int mod_a = M % a_elem_per_vec64;
    int mod_b = N % tile_w_per_calculation;
    int mod_c = N % c_elem_per_vec32;

    if(0 == mod_a && 0 == mod_b) {
        //printf("please use beco_matrix_mult_aTb_fastest");
        //assert(mod_a == 0 && mod_b == 0);
        ASSERT(0, "please use beco_matrix_mult_aTb_fastest");
    }

    if(0!=mod_a) {
        //printf("A_rows must be multiple of %d",a_elem_per_vec64);
        //assert(mod_a == 0);
        ASSERT(0, "A_rows must be multiple of %d", a_elem_per_vec64);
    }

    const beco_vec64_in_t *beco_a = (const beco_vec64_in_t *)At;
    int stride_b = N - mod_b;
    int stride_c = tile_c - N +mod_b;
    constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
    beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));
    //config_unaligned_access(true);

    for (int i = 0; i < stride_beco_a; i++) {
        p_c = (beco_vec32_out_t *)C;

        if (likely(0 == mod_c)) {
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_b_nomultiple(beco_a + i, B + j * tile_w_per_calculation, stride_beco_a, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);

                p_c += tile_beco_cw;
                //C +=(tile_w * c_elem_per_vec32);
            }

            mmac_tile_set_offset_zero();

            mmac_tile_aTb_b_remained(beco_a + i, B, stride_beco_a, tile_w_per_calculation, mod_b, stride_b, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            Tc *pc_remained_out = (Tc *)pc_remained;
            for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                unsigned loop_stride_c = k * N + stride_b;
                unsigned loop_stride_cr = k * tile_w_per_calculation;
                for (int j = 0; j < mod_b; j++) {
                    //C[j + k * N + tile_w_per_calculation * stride_beco_b] = pc_remained_out[j + k * tile_w_per_calculation];
                    C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                }
            }
            C += tile_c;
        } else {
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_b_nomultiple(beco_a + i, B + j * tile_w_per_calculation, stride_beco_a, N, K);
                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);


                Tc *pc_out = (Tc *)pc_remained;
                //int index_pc = 0;
                for(unsigned c_rows = 0; c_rows < a_elem_per_vec64; c_rows++){
                    unsigned loop_stride_c = c_rows * N;
                    for(unsigned c_cols = 0; c_cols < (tile_w_per_calculation); c_cols++) {
                        //C[c_rows * N + c_cols] = pc_out[index_pc];
                        //index_pc++;
                        C[loop_stride_c + c_cols] = *pc_out++;
                    }
                }

                C +=(tile_w_per_calculation);
                //p_c += tile_beco_cw;
            }

            mmac_tile_set_offset_zero();
            mmac_tile_aTb_b_remained(beco_a + i, B, stride_beco_a, tile_w_per_calculation, mod_b, stride_b, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            Tc *pc_remained_out = (Tc *)pc_remained;
            for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                unsigned loop_stride_c = k * N;
                unsigned loop_stride_cr = k * tile_w_per_calculation;
                for (int j = 0; j < mod_b; j++) {
                    //C[j + k * N] = pc_remained_out[j + k * tile_w_per_calculation];
                    C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                }
            }
            //C += (tile_c - N +mod_b);
            C += stride_c;
        }
    }
    free(pc_remained);
}

template <typename Ta, typename Tb, typename Tc>
static void beco_mmult_aTb_bcols_opt_impl(const Ta *At, const Tb *B, Tc *C,
                    int M, int N, int K)
{
    beco_vec32_out_t *p_c;
    //const beco_vec64_in_t *p_at = At,
    //                      *p_b  = B;

    constexpr size_t a_elem_sz = sizeof(Ta);
    constexpr size_t b_elem_sz = sizeof(Tb);
    constexpr size_t c_elem_sz = sizeof(Tc);

    constexpr size_t a_elem_per_vec64 = sizeof(beco_vec64_in_t)/a_elem_sz;
    //constexpr size_t b_elem_per_vec64 = sizeof(beco_vec64_in_t)/b_elem_sz;
    constexpr size_t c_elem_per_vec32 = sizeof(beco_vec32_out_t)/c_elem_sz;
    constexpr size_t tile_w_per_calculation = sizeof(beco_vec64_in_t)/b_elem_sz;//it is equal to b_elem_per_vec64

    // Calculate output increment:
    //   for example: 8 elements output per tile, 1 elements per vec32_out:
    constexpr size_t tile_beco_cw = tile_w_per_calculation / c_elem_per_vec32;

    // Calculate output tile h increment:
    //   for example: 8 lines of elements output per tile, 'stride_c' vec32_out's per line:
    int tile_c = a_elem_per_vec64 * N;


    int mod_a = M % a_elem_per_vec64;
    int mod_b = N % tile_w_per_calculation;
    //int mod_c = N % c_elem_per_vec32;

    if(0 == mod_a && 0 == mod_b) {
        //printf("please use beco_matrix_mult_aTb_fastest");
        //assert(mod_a == 0 && mod_b == 0);
        ASSERT(0, "please use beco_matrix_mult_aTb_fastest");
    }

    if(0!=mod_b) {
        //printf("B_cols must be multiple of %d",tile_w_per_calculation);
        //assert(mod_b == 0);
        ASSERT(0, "B_cols must be multiple of %d",tile_w_per_calculation);
    }

    int stride_beco_a = M / a_elem_per_vec64;
    int stride_beco_b = N / tile_w_per_calculation;
    int stride_beco_c = N / c_elem_per_vec32;

    int stride_a = M - mod_a;
    const beco_vec64_in_t *beco_b = (const beco_vec64_in_t *)B;
    constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
    beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));
    //config_unaligned_access(true);

    for (int i = 0; i < stride_beco_a; i++) {
        p_c = (beco_vec32_out_t *)C;
        for (int j = 0; j < stride_beco_b; j++) {
            mmac_tile_set_offset_zero();
            mmac_tile_aTb_a_nomultiple(At + i * a_elem_per_vec64, beco_b + j, stride_beco_b, M, K);

            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
            p_c += tile_beco_cw;
        }

        C += tile_c;
    }

    p_c = (beco_vec32_out_t *)C;

    for (int j = 0; j < stride_beco_b; j++) {
        mmac_tile_set_offset_zero();
        mmac_tile_aTb_a_remained(At, beco_b + j, a_elem_per_vec64, stride_beco_b, mod_a, stride_a, M, K);

        beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

        int index_C = 0;
        int stride_w = tile_beco_cw * j;
        for(int i = 0; i < (mod_a); i++) {
            int loop_stride_c = i * stride_beco_c + stride_w;
            for(unsigned k = 0; k < tile_beco_cw; k++) {
                p_c[loop_stride_c + k] = pc_remained[index_C];
                //p_c[i * stride_beco_c + tile_beco_cw * j + k] = pc_remained[index_C];
                index_C++;
            }
        }
    }
    free(pc_remained);
}

template <typename Ta, typename Tb, typename Tc>
void beco_mmult_aTb_ccols_opt_impl(const Ta *At, const Tb *B, Tc *C,
                    int M, int N, int K)
{
    beco_vec32_out_t *p_c;
    //const beco_vec64_in_t *p_at = At,
    //                      *p_b  = B;

    constexpr size_t a_elem_sz = sizeof(Ta);
    constexpr size_t b_elem_sz = sizeof(Tb);
    constexpr size_t c_elem_sz = sizeof(Tc);

    constexpr size_t a_elem_per_vec64 = sizeof(beco_vec64_in_t)/a_elem_sz;
    //constexpr size_t b_elem_per_vec64 = sizeof(beco_vec64_in_t)/b_elem_sz;
    constexpr size_t c_elem_per_vec32 = sizeof(beco_vec32_out_t)/c_elem_sz;
    constexpr size_t tile_w_per_calculation = sizeof(beco_vec64_in_t)/b_elem_sz;//it is equal to b_elem_per_vec64

    // Calculate output increment:
    //   for example: 8 elements output per tile, 1 elements per vec32_out:
    constexpr size_t tile_beco_cw = tile_w_per_calculation / c_elem_per_vec32;

    // Calculate output tile h increment:
    //   for example: 8 lines of elements output per tile, 'stride_c' vec32_out's per line:
    int tile_c = a_elem_per_vec64 * N;


    int mod_a = M % a_elem_per_vec64;
    int mod_b = N % tile_w_per_calculation;
    int mod_c = N % c_elem_per_vec32;

    if(0 == mod_a && 0 == mod_b) {
        //printf("please use beco_matrix_mult_aTb_fastest");
        //assert(mod_a != 0 || mod_b != 0);
        ASSERT(0, "please use beco_matrix_mult_aTb_fastest");
    }

    if(0!=mod_c) {
        //printf("C_cols must be multiple of %d",c_elem_per_vec32);
        //assert(mod_c == 0);
        ASSERT(0, "C_cols must be multiple of %d",c_elem_per_vec32);
    }

    int stride_beco_a = M / a_elem_per_vec64;
    int stride_beco_b = N / tile_w_per_calculation;
    int stride_beco_c = N / c_elem_per_vec32;
    //config_unaligned_access(true);

    if(likely((0 != mod_a) && (0 != mod_b))) {
        int stride_a = M - mod_a;
        int stride_b = N - mod_b;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));
        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_ab_nomultiple(At + i * a_elem_per_vec64, B + j * tile_w_per_calculation, M, N, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
                p_c += tile_beco_cw;
            }

            mmac_tile_set_offset_zero();

            mmac_tile_aTb_ab_b_remained(At + i * a_elem_per_vec64, B, tile_w_per_calculation, mod_b, stride_b, M, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                unsigned loop_stride_c = k * stride_beco_c;
                unsigned loop_stride_cr = k * tile_beco_cw;
                for (unsigned j = 0; j < (mod_b / c_elem_per_vec32); j++) {
                    p_c[j + loop_stride_c] = pc_remained[j + loop_stride_cr];
                    //p_c[j + i * stride_beco_c] = pc_remained[j + i * tile_beco_cw];
                }
            }

            C += tile_c;
        }

        p_c = (beco_vec32_out_t *)C;
        for (int j = 0; j < stride_beco_b; j++) {

            mmac_tile_set_offset_zero();
            mmac_tile_aTb_ab_a_remained(At, B + j * tile_w_per_calculation, a_elem_per_vec64, mod_a, stride_a, M, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            int index_C = 0;
            for(int i = 0; i < (mod_a); i++) {
                int loop_stride_c = i * stride_beco_c;
                for(unsigned k = 0; k < tile_beco_cw; k++) {
                    p_c[loop_stride_c + k] = pc_remained[index_C];
                    //p_c[i * stride_beco_c + k] = pc_remained[index_C];
                    index_C++;
                }
            }
            p_c +=tile_beco_cw;
        }

        mmac_tile_set_offset_zero();
        mmac_tile_aTb_ab_remained(At, B, a_elem_per_vec64, tile_w_per_calculation, mod_a, mod_b, M, N, K);
        beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

        for(int i = 0; i < mod_a; i++) {
            int loop_stride_c = i * stride_beco_c;
            int loop_stride_cr = i * tile_beco_cw;
            for (unsigned j = 0; j < (mod_b / c_elem_per_vec32); j++) {
                p_c[loop_stride_c + j] = pc_remained[j + loop_stride_cr];
                //p_c[i * stride_beco_c + j] = pc_remained[j + i * tile_beco_cw];
            }
        }
        free(pc_remained);
    } else if((0 != mod_a) && (0 == mod_b)) {
        int stride_a = M - mod_a;
        const beco_vec64_in_t *beco_b = (const beco_vec64_in_t *)B;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));

        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {
                mmac_tile_set_offset_zero();
                mmac_tile_aTb_a_nomultiple(At + i * a_elem_per_vec64, beco_b + j, stride_beco_b, M, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
                p_c += tile_beco_cw;
            }

            C += tile_c;
        }

        p_c = (beco_vec32_out_t *)C;

        for (int j = 0; j < stride_beco_b; j++) {
            mmac_tile_set_offset_zero();
            mmac_tile_aTb_a_remained(At, beco_b + j, a_elem_per_vec64, stride_beco_b, mod_a, stride_a, M, K);

            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            int index_C = 0;
            int stride_w = tile_beco_cw * j;
            for(int i = 0; i < (mod_a); i++) {
                int loop_stride_c = i * stride_beco_c + stride_w;
                for(unsigned k = 0; k < tile_beco_cw; k++) {
                    p_c[loop_stride_c + k] = pc_remained[index_C];
                    //p_c[i * stride_beco_c + tile_beco_cw * j + k] = pc_remained[index_C];
                    index_C++;
                }
            }
        }
        free(pc_remained);
    } else {
        const beco_vec64_in_t *beco_a = (const beco_vec64_in_t *)At;
        int stride_b = N - mod_b;
        constexpr size_t tile_beco_c = a_elem_per_vec64 * tile_beco_cw;
        beco_vec32_out_t *pc_remained = (beco_vec32_out_t *)malloc(tile_beco_c * sizeof(beco_vec32_out_t));

        for (int i = 0; i < stride_beco_a; i++) {
            p_c = (beco_vec32_out_t *)C;
            for (int j = 0; j < stride_beco_b; j++) {

                mmac_tile_set_offset_zero();
                mmac_tile_aTb_b_nomultiple(beco_a + i, B + j * tile_w_per_calculation, stride_beco_a, N, K);

                beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);

                p_c += tile_beco_cw;
                //C +=(tile_w * c_elem_per_vec32);
            }

            mmac_tile_set_offset_zero();

            mmac_tile_aTb_b_remained(beco_a + i, B, stride_beco_a, tile_w_per_calculation, mod_b, stride_b, N, K);
            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(pc_remained, tile_beco_cw);

            Tc *pc_remained_out = (Tc *)pc_remained;
            for(unsigned k = 0; k < a_elem_per_vec64; k++) {
                unsigned loop_stride_c = k * N + stride_b;
                unsigned loop_stride_cr = k * tile_w_per_calculation;
                for (int j = 0; j < mod_b; j++) {
                    //C[j + k * N + tile_w_per_calculation * stride_beco_b] = pc_remained_out[j + k * tile_w_per_calculation];
                    C[j + loop_stride_c] = pc_remained_out[j + loop_stride_cr];
                }
            }
            C += tile_c;
        }
        free(pc_remained);
    }
}

template <typename Ta, typename Tb, typename Tc>
static void beco_mmult_aTb_fastest_impl(const beco_vec64_in_t *At, const beco_vec64_in_t *B,
                    beco_vec32_out_t *C,
                    int M, int N, int K,
                    size_t stride_a, size_t stride_b, size_t stride_c)
{
    beco_vec32_out_t *p_c;
    const beco_vec64_in_t *p_at = At,
                          *p_b  = B;

    constexpr size_t a_elem_sz = sizeof(Ta);
    constexpr size_t b_elem_sz = sizeof(Tb);
    constexpr size_t c_elem_sz = sizeof(Tc);

    constexpr size_t a_elem_per_vec64 = sizeof(beco_vec64_in_t)/a_elem_sz;
    constexpr size_t b_elem_per_vec64 = sizeof(beco_vec64_in_t)/b_elem_sz;
    constexpr size_t c_elem_per_vec32 = sizeof(beco_vec32_out_t)/c_elem_sz;

    // assert stride_* is divideable by 64bits
    // static_assert(stride_c % 8 == 0, "Output stride must be multiple of 64bits");
    // assert M*a_elem_sz <= stride_a
    // assert N*b_elem_sz <= stride_b
    // assert N*c_elem_sz <= stride_c
    size_t mod_a = stride_a % a_elem_per_vec64;
    size_t mod_b = stride_b % b_elem_per_vec64;
    if(0!=mod_a) {
        //printf("A_rows must be multiple of %d",a_elem_per_vec64);
        //assert(mod_a == 0);
        ASSERT(0, "A_rows must be multiple of %d",a_elem_per_vec64);
    }
    if(0!=mod_b) {
        //printf("B_cols must be multiple of %d",b_elem_per_vec64);
        //assert(mod_b == 0);
        ASSERT(0, "B_cols must be multiple of %d",b_elem_per_vec64);
    }

    size_t stride_beco_a = stride_a / sizeof(beco_vec64_in_t);
    size_t stride_beco_b = stride_b / sizeof(beco_vec64_in_t);
    size_t stride_beco_c = stride_c / sizeof(beco_vec32_out_t);

    // Calculate output increment:
    //   for example: 8 elements output per tile, 1 elements per vec32_out:
    constexpr size_t tile_w = b_elem_per_vec64 / c_elem_per_vec32;

    // Calculate output tile h increment:
    //   for example: 8 lines of elements output per tile, 'stride_c' vec32_out's per line:
    size_t tile_h = a_elem_per_vec64 * stride_beco_c;

    for (unsigned i = 0; i < stride_beco_a; i++) {
        p_c = C;
        for (unsigned j = 0; j < stride_beco_b; j++) {

            mmac_tile_set_offset_zero();
            mmac_tile_aTb(p_at + i, p_b + j, stride_beco_a, stride_beco_b, K);

            beco_read_tile64<a_elem_sz, b_elem_sz, c_elem_sz>(p_c, stride_beco_c);
#if BECO_DEBUG
            printf("tile (%d,%d)\n", j, i);
            print_matrix((Tc *)p_c, b_elem_per_vec64, a_elem_per_vec64, N);
#endif
            p_c += tile_w;
        }

        // Increment C to point to next tile
        C += tile_h;
    }
}

// Create a template to automatically generate input type configuration
template <typename aType>  constexpr auto atype_mask = BECO_ALUCNF_ATYPE_INT8;
template <>  constexpr auto atype_mask<int8_t>   = BECO_ALUCNF_ATYPE_INT8;
template <>  constexpr auto atype_mask<uint8_t>  = BECO_ALUCNF_ATYPE_UINT8;
template <>  constexpr auto atype_mask<int16_t>  = BECO_ALUCNF_ATYPE_INT16;
template <>  constexpr auto atype_mask<uint16_t> = BECO_ALUCNF_ATYPE_UINT16;

template <typename bType>  constexpr auto btype_mask = BECO_ALUCNF_BTYPE_INT8;
template <>  constexpr auto btype_mask<int8_t>   = BECO_ALUCNF_BTYPE_INT8;
template <>  constexpr auto btype_mask<uint8_t>  = BECO_ALUCNF_BTYPE_UINT8;
template <>  constexpr auto btype_mask<int16_t>  = BECO_ALUCNF_BTYPE_INT16;
template <>  constexpr auto btype_mask<uint16_t> = BECO_ALUCNF_BTYPE_UINT16;

template <typename cType>  constexpr auto ctype_mask = BECO_OUTCNF_PACK_INT8;
template <>  constexpr auto ctype_mask<int8_t>   = BECO_OUTCNF_PACK_INT8;
template <>  constexpr auto ctype_mask<uint8_t>  = BECO_OUTCNF_PACK_INT8;
template <>  constexpr auto ctype_mask<int16_t>  = BECO_OUTCNF_PACK_INT16;
template <>  constexpr auto ctype_mask<uint16_t> = BECO_OUTCNF_PACK_INT16;
template <>  constexpr auto ctype_mask<int32_t>  = BECO_OUTCNF_PACK_INT32;
template <>  constexpr auto ctype_mask<uint32_t> = BECO_OUTCNF_PACK_INT32;
template <>  constexpr auto ctype_mask<float>    = BECO_OUTCNF_PACK_FLOAT32;

template <typename aType, typename bType>  constexpr auto localrotate_mask = 0;
template <>  constexpr auto localrotate_mask<int16_t,int8_t>   = BECO_OUTCNF_LOCALROTATE;
template <>  constexpr auto localrotate_mask<uint16_t,int8_t>  = BECO_OUTCNF_LOCALROTATE;
template <>  constexpr auto localrotate_mask<int16_t,uint8_t>  = BECO_OUTCNF_LOCALROTATE;
template <>  constexpr auto localrotate_mask<uint16_t,uint8_t> = BECO_OUTCNF_LOCALROTATE;

template <typename aType, typename bType, typename cType>
    static inline void beco_set_matrix_mult_config(int scale)
{
    uint32_t config;

    config = (BECO_ALUCNF_BMODE_MAT     |
                BECO_ALUCNF_AMODE_REP32 |
                atype_mask<aType>       |
                btype_mask<bType> ) << BECO_ALUCNF_N_SHIFT;
    config |= BECO_OUTCNF_RSHIFT(scale)  |
                BECO_OUTCNF_GLOBALROTATE |
                ctype_mask<cType>        |
                localrotate_mask<aType, bType>;
    beco_write_config(config);
}

// stride_{a,b,c}: width of respective matrix in elements
// case1: basic function, meet any situation
template <typename aType, typename bType, typename cType>
     void beco_matrix_mult_aTb_basic(const aType *At, const bType *B, cType *C,
                               int M, int N, int K)
{
    beco_set_matrix_mult_config<aType, bType, cType>(0);

    beco_mmult_aTb_basic_impl<aType, bType, cType>(At, B, C,
                    M, N, K);
}

//case2: a_rows is the multiple of (sizeof(beco_vec64_in_t)/sizeof(aType))
template <typename aType, typename bType, typename cType>
     void beco_matrix_mult_aTb_arows_opt(const aType *At, const bType *B, cType *C,
                               int M, int N, int K)
{
    beco_set_matrix_mult_config<aType, bType, cType>(0);

    beco_mmult_aTb_arows_opt_impl<aType, bType, cType>(At, B, C,
                    M, N, K);
}

//case3: b_cols is the multiple of (sizeof(beco_vec64_in_t)/sizeof(bType))
template <typename aType, typename bType, typename cType>
     void beco_matrix_mult_aTb_bcols_opt(const aType *At, const bType *B, cType *C,
                               int M, int N, int K)
{
    beco_set_matrix_mult_config<aType, bType, cType>(0);

    beco_mmult_aTb_bcols_opt_impl<aType, bType, cType>(At, B, C,
                    M, N, K);
}

//case4: c_cols is the multiple of (sizeof(beco_vec32_out_t)/sizeof(cType))
template <typename aType, typename bType, typename cType>
     void beco_matrix_mult_aTb_ccols_opt(const aType *At, const bType *B, cType *C,
                               int M, int N, int K)
{
    beco_set_matrix_mult_config<aType, bType, cType>(0);

    beco_mmult_aTb_ccols_opt_impl<aType, bType, cType>(At, B, C,
                    M, N, K);
}

//case5: a_rows is the multiple of (sizeof(beco_vec64_in_t)/sizeof(aType)),
//       and b_cols is the multiple of (sizeof(beco_vec64_in_t)/sizeof(bType))
//       it is fastest
template <typename aType, typename bType, typename cType>
     void beco_matrix_mult_aTb_fastest(const aType *At, const bType *B, cType *C,
                               int M, int N, int K)
{
    beco_vec64_in_t *pA = (beco_vec64_in_t*)At,
                    *pB = (beco_vec64_in_t*)B;
    beco_vec32_out_t *pCb = (beco_vec32_out_t*)C;

    beco_set_matrix_mult_config<aType, bType, cType>(0);

    beco_mmult_aTb_fastest_impl<aType, bType, cType>(pA, pB, pCb,
                    M, N, K,  // Dimensions
                    M*sizeof(*At), N*sizeof(*B), N*sizeof(*C)); // Stride
}

#endif


