/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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


#ifndef __BECO_FULLY_CONNECTED_H__
#define __BECO_FULLY_CONNECTED_H__

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "plat_types.h"
#include "beco_read_acc_slice.hpp"

#include "string.h"

#include <limits>


// Create a template to automatically insert correct optimized read-out function.
template <size_t m_elem_sz, size_t o_elem_sz>
    static void beco_read_tile(beco_vec32_out_t out[], int scale);

#define BECO_GEN_TILE_READ(m, o, read_fn) \
    template <>                 \
    void beco_read_tile<m, o>(beco_vec32_out_t out[], int scale) { \
        read_fn(out, scale);                                    \
    }

// Instantiate legal variations
//  Element Size:  M   O   ACCnum
BECO_GEN_TILE_READ(1,  1,  beco_mmacgr4_read_8_8bit)
BECO_GEN_TILE_READ(1,  2,  beco_mmacgr4_read_8_16bit)
BECO_GEN_TILE_READ(2,  2,  beco_mmacgr4_read_16_16bit)
BECO_GEN_TILE_READ(2,  4,  beco_mmacgr4_read_16_32bit)
BECO_GEN_TILE_READ(1,  4,  beco_mmacgr4_read_8_32bit)

template <size_t m_elem_sz>
    static void beco_set_preload(int32_t q_bias);

#define BECO_SET_PRELOAD(m, set_preload_fn) \
    template <>                 \
    void beco_set_preload<m>(int32_t q_bias) { \
        set_preload_fn(q_bias);                \
    }

BECO_SET_PRELOAD(1, beco_set_preload_16x8_4ACC)
BECO_SET_PRELOAD(2, beco_set_preload_16x16_4ACC)

static void beco_add_bias_4ACC(const beco_vec64_in_t *bias, p31_t shift)
{
    beco_write_reg(BECO_REG0, bias[0]);
    beco_write_reg(BECO_REG1, bias[1]);
    beco_write_reg(BECO_REG2, bias[2]);
    beco_write_reg(BECO_REG3, bias[3]);
    beco_mmacgr4(((beco_vec32_in_t){.u32 = shift}), BECO_REG0);
}

static inline void matirx_row_shift(int i)
{
    switch (i) {
    case 1:
        beco_shift_block5(BECO_REG0, 1);
        break;
    case 2:
        beco_shift_block5(BECO_REG0, 2);
        break;
    case 3:
        beco_shift_block5(BECO_REG0, 3);
        break;
    case 4:
        beco_shift_block5(BECO_REG0, 4);
        break;
    case 5:
        beco_shift_block5(BECO_REG0, 5);
        break;
    case 6:
        beco_shift_block5(BECO_REG0, 6);
        break;
    case 7:
        beco_shift_block5(BECO_REG0, 7);
        break;
    default:
        break;
    }
}

template <typename Tv>
static inline void beco_fc_impl(
                const Tv *p_v, const beco_vec64_in_t *beco_w,
                const uint16_t dim_vec, const uint32_t beco_cols,
                int remain_num, int m_elem_sz)
{
    int a = remain_num;
    int c = m_elem_sz >> 1;
    int shift_num, offset_num, index;
    for (int i = 0; i < dim_vec; i++) {
        uint32_t pv_32 = *p_v++;
        if (m_elem_sz == 1) {
            shift_num = (a*i) / 8;
            offset_num = (a*i) % 8;
        } else {
            shift_num = (a*i) / 4;
            offset_num = (a*i) % 4;
        }

        index = beco_cols * i + shift_num;

        if (offset_num == 0) {
            beco_write_reg(BECO_REG0, beco_w[index]);
            beco_write_reg(BECO_REG1, beco_w[index+1]);
            beco_write_reg(BECO_REG2, beco_w[index+2]);
            beco_write_reg(BECO_REG3, beco_w[index+3]);
            beco_mmacgr4(((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);
        } else {
            beco_write_reg(BECO_REG0, beco_w[index]);
            beco_write_reg(BECO_REG1, beco_w[index+1]);
            beco_write_reg(BECO_REG2, beco_w[index+2]);
            beco_write_reg(BECO_REG3, beco_w[index+3]);
            beco_write_reg(BECO_REG4, beco_w[index+4]);
            matirx_row_shift(offset_num << c);
            beco_mmacgr4(((beco_vec32_in_t){.u32 = pv_32}), BECO_REG0);
        }
    }
}

template <typename Tv, typename Tm, typename To>
static void beco_fc_process(const Tv *pV,
                                 const Tm *pM,
                                 const uint16_t dim_vec,
                                 const uint16_t num_of_cols,
                                 const uint16_t bias_shift,
                                 const uint16_t out_shift,
                                 const Tm *bias,
                                 To *pOut)
{
    constexpr size_t m_elem_sz = sizeof(Tm);
    constexpr size_t m_elem_per_vec64 = sizeof(beco_vec64_in_t)/m_elem_sz;
    constexpr size_t m_elem_per_vec64_4accs = 4 * m_elem_per_vec64;
    const size_t remainder = num_of_cols % m_elem_per_vec64_4accs;

    /* bias */
    const uint32_t bias_factor = 1 << bias_shift;
    const beco_vec64_in_t *beco_bias = (const beco_vec64_in_t *) bias;

    /* matrix */
    const unsigned beco_cols = (num_of_cols - remainder) / m_elem_per_vec64;
    const unsigned beco_cols_4accs = num_of_cols / m_elem_per_vec64_4accs;
    const beco_vec64_in_t *beco_m = (const beco_vec64_in_t *) pM;

    /* out */
    constexpr size_t o_elem_sz = sizeof(To);
    beco_vec32_out_t *p_out = (beco_vec32_out_t *)pOut;
    constexpr size_t o_stride = o_elem_sz * 8 / m_elem_sz; // beco_vec32_out_t

    int round = q_round(out_shift);
    unsigned j;

    for (j = 0; j < beco_cols_4accs; j++) {
        beco_set_preload<m_elem_sz>(round);

        if (bias) {
            beco_add_bias_4ACC(&beco_bias[4*j], bias_factor);
        }

        beco_fc_impl(pV, &beco_m[4*j], dim_vec, beco_cols, remainder, m_elem_sz);
        beco_read_tile<m_elem_sz, o_elem_sz>(p_out, out_shift);
        p_out += o_stride;
    }

    if (LIKELY(remainder != 0)) {
        beco_set_preload<m_elem_sz>(round);

        if (bias) {
            beco_add_bias_4ACC(&beco_bias[beco_cols], bias_factor);
        }

        beco_fc_impl(pV, &beco_m[beco_cols], dim_vec, beco_cols, remainder, m_elem_sz);
        beco_vec32_out_t p_out_tmp[m_elem_per_vec64_4accs] = {0};
        beco_read_tile<m_elem_sz, o_elem_sz>(p_out_tmp, out_shift);
        memcpy(p_out, p_out_tmp, remainder * o_elem_sz);
    }
}


template <typename OType>
    static void beco_set_fc_config(int scale)
{
    uint32_t config;
    if (sizeof(OType) == sizeof(q31_t))
        scale = 0;

    config =
            BECO_CONF_BMODE_REP64 | BECO_CONF_AMODE_REP16 |
            BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8  |
            BECO_CONF_RSHIFT(scale) |
            BECO_CONF_PACK_INT32   | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);
}

template <typename VType, typename MType, typename OType>
     void beco_fully_connected(const VType *pV,
                                const MType *pM,
                                const uint16_t dim_vec,
                                const uint16_t num_of_cols,
                                const uint16_t bias_shift,
                                const uint16_t out_shift,
                                const MType *bias,
                                OType *pOut)
{
    beco_set_fc_config<OType>(out_shift);

    beco_fc_process<VType, MType, OType>(
        pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
}

#endif


