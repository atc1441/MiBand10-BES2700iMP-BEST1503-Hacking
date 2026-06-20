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

#ifndef __BECO_VECMAT_MULT_H__
#define __BECO_VECMAT_MULT_H__

/* ----------------------------------------------------------------------
 * Project:      BECO MATH Library
 * Title:        beco_vec_mat_mult.c
 * Description:  VEC * MAT
 *
 * $Date:        17. March 2021
 * $Revision:    V.1.0.0
 * $Copyright    2021 BES Technic
 *
 * Target Processor:  Cortex-M cores
 *
 * -------------------------------------------------------------------- */
#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

#include <stdlib.h>
#include <limits>
using namespace std;

#define likely(x) __builtin_expect(!!(x), 1)

//read-out function
static inline void beco_read_acc0_16x8_16bit(beco_vec32_out_t *out)
{
    out[0] = beco_read_acc(BECO_ACC0, 0);
    out[1] = beco_read_acc(BECO_ACC0, 8);
    out[2] = beco_read_acc(BECO_ACC0, 4);
    out[3] = beco_read_acc(BECO_ACC0, 12);
}

static inline void beco_read_acc0_16x16_32bit(beco_vec32_out_t *out)
{
    //beco_read_acc(BECO_ACC0, -4); // reset beco lane pointer
    out[0] = beco_read_acc(BECO_ACC0, 0);
    out[1] = beco_read_acc(BECO_ACC0, 4);
    out[2] = beco_read_acc(BECO_ACC0, 8);
    out[3] = beco_read_acc(BECO_ACC0, 12);
}

static inline void beco_read_all_16x8_16bit(beco_vec32_out_t *out)
{
    out[0] = beco_read_acc(BECO_ACC0, 0);
    out[1] = beco_read_acc(BECO_ACC0, 8);
    out[2] = beco_read_acc(BECO_ACC0, 4);
    out[3] = beco_read_acc(BECO_ACC0, 12);

    out[4] = beco_read_acc(BECO_ACC1, 0);
    out[5] = beco_read_acc(BECO_ACC1, 8);
    out[6] = beco_read_acc(BECO_ACC1, 4);
    out[7] = beco_read_acc(BECO_ACC1, 12);

    out[8] = beco_read_acc(BECO_ACC2, 0);
    out[9] = beco_read_acc(BECO_ACC2, 8);
    out[10] = beco_read_acc(BECO_ACC2, 4);
    out[11] = beco_read_acc(BECO_ACC2, 12);

    out[12] = beco_read_acc(BECO_ACC3, 0);
    out[13] = beco_read_acc(BECO_ACC3, 8);
    out[14] = beco_read_acc(BECO_ACC3, 4);
    out[15] = beco_read_acc(BECO_ACC3, 12);

}

static inline void beco_read_all_16x16_32bit(beco_vec32_out_t *out)
{
    //beco_read_acc(BECO_ACC0, -4); // reset beco lane pointer
    out[0] = beco_read_acc(BECO_ACC0, 0);
    out[4] = beco_read_acc(BECO_ACC1, 0);
    out[8] = beco_read_acc(BECO_ACC2, 0);
    out[12] = beco_read_acc(BECO_ACC3, 0);
    out[1] = beco_read_acc(BECO_ACC0, 4);
    out[5] = beco_read_acc(BECO_ACC1, 4);
    out[9] = beco_read_acc(BECO_ACC2, 4);
    out[13] = beco_read_acc(BECO_ACC3, 4);
    out[2] = beco_read_acc(BECO_ACC0, 8);
    out[6] = beco_read_acc(BECO_ACC1, 8);
    out[10] = beco_read_acc(BECO_ACC2, 8);
    out[14] = beco_read_acc(BECO_ACC3, 8);
    out[3] = beco_read_acc(BECO_ACC0, 12);
    out[7] = beco_read_acc(BECO_ACC1, 12);
    out[11] = beco_read_acc(BECO_ACC2, 12);
    out[15] = beco_read_acc(BECO_ACC3, 12);
}

// Create a template to automatically insert correct optimized read-out function.
template <size_t m_elem_sz, size_t o_elem_sz, size_t n>
    static void beco_read_tile(beco_vec32_out_t out[]);

#define BECO_GEN_TILE_READ(m, o, n, read_fn) \
    template <> inline                 \
    void beco_read_tile<m, o , n>(beco_vec32_out_t out[]) { \
        read_fn(out);                                    \
    }

// Instantiate legal variations
//  Element Size:  M   O   ACCnum
BECO_GEN_TILE_READ(1,  2,  1, beco_read_acc0_16x8_16bit)
BECO_GEN_TILE_READ(2,  4,  1, beco_read_acc0_16x16_32bit)
BECO_GEN_TILE_READ(1,  2,  4, beco_read_all_16x8_16bit)
BECO_GEN_TILE_READ(2,  4,  4, beco_read_all_16x16_32bit)


template <typename Tv, typename Tm>
static void beco_vecmat_mult_remained_impl(const Tv *p_v, const Tm *p_m,
                int loop_index, int elem_per_calculate, const int dim_vec, const int num_of_column)
{
    Tm *p_mr = (Tm *)malloc(elem_per_calculate * sizeof(Tm));
    int stride_loop = elem_per_calculate * loop_index;

    for (int i = 0; i < dim_vec; i++) {
        int stride_m = i * num_of_column + stride_loop;
        for(int j = 0; j < elem_per_calculate; j++) {
            p_mr[j] = p_m[stride_m + j]; // fill_matrix_tile
        }

        uint32_t pv_32 = *p_v++;
        beco_vec64_in_t *beco_mr = (beco_vec64_in_t *)p_mr;

        beco_write_reg(BECO_REG0, beco_mr[0]);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);
    }
    free(p_mr);
}

// vecmat_mult_impl
//case1: if num_of_column is the multiple of x, use BECO_ACC0~BECO_ACC4
template <typename Tv>
static inline void beco_vecmat_mult_4macc_impl(const Tv *p_v, const beco_vec64_in_t *beco_m,
                const int dim_vec, const unsigned column_for_64)
{

    for (int i = 0; i < dim_vec; i++){
        uint32_t pv_32 = *p_v++;
        //beco_vec64_in_t *beco_w = (beco_vec64_in_t *)p_w;

        beco_write_reg(BECO_REG0, beco_m[0]);
        beco_write_reg(BECO_REG1, beco_m[1]);
        beco_write_reg(BECO_REG2, beco_m[2]);
        beco_write_reg(BECO_REG3, beco_m[3]);

        //beco_mmacgr4(((beco_vec32_in_t){.i16 = {p_v[i], 0}}), BECO_REG0);
        beco_mmacgr4(((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);
        beco_m += column_for_64;
    }
}

//case2: if num_of_column is the multiple of x, only use BECO_ACC0
template <typename Tv>
static inline void beco_vecmat_mult_macc_impl(const Tv *p_v, const beco_vec64_in_t *beco_m,
                const int dim_vec, const unsigned column_for_64) // if num_of_column is the multiple of 4
{
    for (int i = 0; i < dim_vec; i++){
        uint32_t pv_32 = *p_v++;
        //beco_vec64_in_t *beco_w = (beco_vec64_in_t *)p_w;

        beco_write_reg(BECO_REG0, beco_m[0]);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);

        beco_m += column_for_64;
    }
}

//case3: if num_of_column is not the multiple of x, use four 16 bit p_w to make a 64 bit beco_w.
//       it is slower than Pointer
template <typename Tv, typename Tm>
static inline void beco_vecmat_mult_nomultiple_impl(const Tv *p_v, const Tm *p_m,
                const int dim_vec, const int num_of_column) // if num_of_column is not the multiple of 4
{
    int64_t pm_64[1];
    beco_vec64_in_t *beco_m;

    for (int i = 0; i < dim_vec; i++){
        uint32_t pv_32 = *p_v++;
        memcpy(pm_64, p_m, 8);
        beco_m = (beco_vec64_in_t *)pm_64;
        //const beco_vec64_in_t beco_m = (const beco_vec64_in_t){.i16={p_m[0],p_m[1],p_m[2],p_m[3]}};

        beco_write_reg(BECO_REG0, beco_m[0]);
        beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);

        p_m += num_of_column;
    }
}

template <typename Tv, typename Tm, typename To>
static void beco_vecmat_mult_process(const Tv * pV,const Tm * pM, To *pOut,
                        const int dim_vec, const int num_of_column)
{
    beco_vec32_out_t *out = (beco_vec32_out_t *)pOut;

    unsigned j;
    //constexpr size_t v_elem_sz = sizeof(Tv);
    constexpr size_t m_elem_sz = sizeof(Tm);
    constexpr size_t o_elem_sz = sizeof(To);

    constexpr size_t m_elem_per_vec64 = sizeof(beco_vec64_in_t)/m_elem_sz;
    constexpr size_t o_elem_per_vec32 = sizeof(beco_vec32_out_t)/o_elem_sz;
    constexpr size_t tile_per_calculation = m_elem_per_vec64 / o_elem_per_vec32;//it is equal to m_elem_per_vec64
    constexpr size_t tile_4macc_calculation = tile_per_calculation * 4;

    const unsigned loopcnt = num_of_column / m_elem_per_vec64;
    const unsigned remainder = num_of_column % m_elem_per_vec64;

    const unsigned loopcnt_4macc = num_of_column / (m_elem_per_vec64 * 4);
    const unsigned remainder_4macc = num_of_column % (m_elem_per_vec64 * 4);

    if(likely(0 == remainder_4macc)) {
        const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;
        for (j = 0; j < loopcnt_4macc; j++) {
            beco_clear_acc(BECO_ACC0);       // clear accumulators
            beco_clear_acc(BECO_ACC1);
            beco_clear_acc(BECO_ACC2);
            beco_clear_acc(BECO_ACC3);

            beco_vecmat_mult_4macc_impl(pV, beco_m + j*4, dim_vec, loopcnt);
            beco_read_tile<m_elem_sz, o_elem_sz, 4>(out);

            out += tile_4macc_calculation;
        }
    } else if(0 == remainder){

        const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;
        for (j = 0; j < loopcnt; j++) {
            beco_clear_acc(BECO_ACC0);

            beco_vecmat_mult_macc_impl(pV, beco_m + j, dim_vec, loopcnt);
            beco_read_tile<m_elem_sz, o_elem_sz, 1>(out);

            out += tile_per_calculation;
        }
    } else {
        for (j = 0; j < loopcnt; j++) {
            beco_clear_acc(BECO_ACC0);

            beco_vecmat_mult_nomultiple_impl(pV, pM + j * m_elem_per_vec64, dim_vec, num_of_column);
            beco_read_tile<m_elem_sz, o_elem_sz, 1>(out);

            out += tile_per_calculation;
        }

        //fill the matrix
        beco_vec32_out_t *p_out = (beco_vec32_out_t *)malloc(m_elem_per_vec64 * sizeof(beco_vec32_out_t));

        beco_clear_acc(BECO_ACC0);

        beco_vecmat_mult_remained_impl(pV, pM, j, m_elem_per_vec64, dim_vec, num_of_column);
        beco_read_tile<m_elem_sz, o_elem_sz, 1>(p_out);

        for(uint16_t i = 0; i < remainder; i++) {
            out[i] = p_out[i];
        }
        free(p_out);
    }
}

template <typename MType>  constexpr auto btype_mask = BECO_CONF_BTYPE_INT8;
template <>  constexpr auto btype_mask<int8_t>   = BECO_CONF_BTYPE_INT8;
template <>  constexpr auto btype_mask<uint8_t>  = BECO_CONF_BTYPE_UINT8;
template <>  constexpr auto btype_mask<int16_t>  = BECO_CONF_BTYPE_INT16;
template <>  constexpr auto btype_mask<uint16_t> = BECO_CONF_BTYPE_UINT16;

template <typename OType>  constexpr auto ctype_mask = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<int8_t>   = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<uint8_t>  = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<int16_t>  = BECO_CONF_PACK_INT16;
template <>  constexpr auto ctype_mask<uint16_t> = BECO_CONF_PACK_INT16;
template <>  constexpr auto ctype_mask<int32_t>  = BECO_CONF_PACK_INT32;
template <>  constexpr auto ctype_mask<uint32_t> = BECO_CONF_PACK_INT32;
template <>  constexpr auto ctype_mask<float>    = BECO_CONF_PACK_FLOAT32;

template <typename MType>  constexpr auto localrotate_mask = 0;
template <>  constexpr auto localrotate_mask<int8_t>   = BECO_CONF_RD_16x8_ROT90;
template <>  constexpr auto localrotate_mask<uint8_t>  = BECO_CONF_RD_16x8_ROT90;
template <>  constexpr auto localrotate_mask<int16_t>   = BECO_CONF_RD_16x16;
template <>  constexpr auto localrotate_mask<uint16_t>  = BECO_CONF_RD_16x16;


template <typename MType, typename OType>
    static inline void beco_set_vecmat_mult_config(int scale)
{
    uint32_t config;

    config =
            BECO_CONF_BMODE_REP64 | BECO_CONF_AMODE_REP16 |
            BECO_CONF_ATYPE_INT16 | btype_mask<MType>     |
            BECO_OUTCNF_RSHIFT(scale) |
            ctype_mask<OType>   | localrotate_mask<MType>;
    beco_write_config(config);
}

template <typename VType, typename MType, typename OType>
     void beco_vecmat_mult(const VType *pV, const MType *pM, OType *O,
                               int K, int N)
{
    beco_set_vecmat_mult_config<MType, OType>(0);

    beco_vecmat_mult_process<VType, MType, OType>(pV, pM, O, K, N);
}

#endif //__BECO_VECMAT_MULT_H__


