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

#ifndef __BECO_DEPTHWISE_CONV_H__
#define __BECO_DEPTHWISE_CONV_H__

#include <assert.h>

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "plat_types.h"

#include <limits>
#include "beco_read_acc_slice.hpp"


// Create a template to automatically insert correct optimized read-out function
template <size_t in_elem_sz, size_t out_elem_sz, bool flag>
    static void beco_read_tile(beco_vec32_out_t out[], int scale);

#define BECO_GEN_TILE_READ(i, o, f, read_fn) \
    template <>                 \
    void beco_read_tile<i, o, f>(beco_vec32_out_t out[], int scale) { \
        read_fn(out, scale);                                    \
    }

// Instantiate legal variations
// Element Size:   In  Out Read4Accs  ACCnum
BECO_GEN_TILE_READ(1,  1,  false,     beco_mmacgr_read_8_8bit)
BECO_GEN_TILE_READ(1,  2,  false,     beco_mmacgr_read_8_16bit)
BECO_GEN_TILE_READ(2,  2,  false,     beco_mmacgr_read_16_16bit)
BECO_GEN_TILE_READ(2,  4,  false,     beco_mmacgr_read_16_32bit)

BECO_GEN_TILE_READ(1,  1,  true,      beco_mmacgr4_read_8_8bit)
BECO_GEN_TILE_READ(1,  2,  true,      beco_mmacgr4_read_8_16bit)
BECO_GEN_TILE_READ(2,  2,  true,      beco_mmacgr4_read_16_16bit)
BECO_GEN_TILE_READ(2,  4,  true,      beco_mmacgr4_read_16_32bit)

static void beco_set_bias(const q7_t *p_bias, const uint32_t factor)
{
    beco_vec64_in_t B = (beco_vec64_in_t){.i8 = {*p_bias, *p_bias, *p_bias, *p_bias,
                                                *p_bias, *p_bias, *p_bias, *p_bias}};
    beco_write_reg(BECO_REG0, B);
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC1, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC2, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC3, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
}

static void beco_set_bias(const q15_t *p_bias, const uint32_t factor)
{
    beco_vec64_in_t B = (beco_vec64_in_t){.i16 = {*p_bias, *p_bias, *p_bias, *p_bias}};
    beco_write_reg(BECO_REG0, B);
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC1, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC2, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
    beco_mmacgr(BECO_ACC3, ((beco_vec32_in_t){.u32 = factor}), BECO_REG0);
}

template <size_t w_elem_sz>
    static void beco_set_preload(int32_t q_bias);

#define BECO_SET_PRELOAD(m, set_preload_fn) \
    template <>                      \
    void beco_set_preload<m>(int32_t q_bias) {  \
        set_preload_fn(q_bias);                 \
    }

BECO_SET_PRELOAD(1, beco_set_preload_16x8_4ACC)
BECO_SET_PRELOAD(2, beco_set_preload_16x16_4ACC)

static inline void write_data_to_reg4_q15(beco_vec64_in_t *p_w, int i_stride_w, int in_row,
                                          int i_ker_w, int stride_w4, int stride_w, int out_w)
{
    p_w += i_stride_w * 4;
    p_w += in_row * (out_w / 4);
    switch (i_ker_w) {
    case 0:
        beco_write_reg(BECO_REG1, p_w[0]);
        beco_write_reg(BECO_REG2, p_w[1]);
        beco_write_reg(BECO_REG3, p_w[2]);
        beco_write_reg(BECO_REG4, p_w[3]);
        if (LIKELY(i_stride_w == 0))
            beco_move(BECO_REG0, BECO_REG5);    // 0,1,...,14,15
        else
            beco_write_reg(BECO_REG0, *(p_w-1));    // -1,1,...,14,15
        beco_shift_block5(BECO_REG0, 6);
        break;
    case 1:
        beco_shift_block5(BECO_REG0, 2);    // 1,2,...,15,16
        break;
    case 2:
        if (LIKELY((i_stride_w+1) == stride_w4) && LIKELY(stride_w == 0))
            beco_move(BECO_REG4, BECO_REG5);    // 2,3,...,16,0
        else
            beco_write_reg(BECO_REG4, p_w[4]);  // 2,3,...,16,17
        beco_shift_block5(BECO_REG0, 2);
        break;
    }
}

static inline void write_data_to_reg_q15(beco_vec64_in_t *p_w, int i_stride_w, int in_row,
                                         int i_ker_w, int stride_w4, int stride_w, int out_w)
{
    p_w += i_stride_w;
    p_w += in_row * (out_w / 4);
    switch (i_ker_w) {
    case 0:
        beco_write_reg(BECO_REG1, p_w[0]);
        if (LIKELY(i_stride_w == 0))
            beco_move(BECO_REG0, BECO_REG5);
        else
            beco_write_reg(BECO_REG0, *(p_w-1));
        beco_shift_block5(BECO_REG0, 6);
        break;
    case 1:
        beco_shift_block5(BECO_REG0, 2);
        break;
    case 2:
        if (LIKELY((i_stride_w + 1) == (stride_w4 * 4 + stride_w)))
            beco_move(BECO_REG1, BECO_REG5);
        else
            beco_write_reg(BECO_REG1, p_w[1]);
        beco_shift_block5(BECO_REG0, 2);
        break;
    }
}

static inline void write_data_to_reg4_q7(beco_vec64_in_t *p_w, int i_stride_w, int in_row,
                                         int i_ker_w, int stride_w4, int stride_w, int out_w)
{
    p_w += i_stride_w * 4;
    p_w += in_row * (out_w / 8);
    switch (i_ker_w) {
    case 0:
        beco_write_reg(BECO_REG1, p_w[0]);
        beco_write_reg(BECO_REG2, p_w[1]);
        beco_write_reg(BECO_REG3, p_w[2]);
        beco_write_reg(BECO_REG4, p_w[3]);
        if (LIKELY(i_stride_w == 0))
            beco_move(BECO_REG0, BECO_REG5);
        else
            beco_write_reg(BECO_REG0, *(p_w-1));
        beco_shift_block5(BECO_REG0, 7);
        break;
    case 1:
        beco_shift_block5(BECO_REG0, 1);
        break;
    case 2:
        if (LIKELY((i_stride_w+1) == stride_w4) && LIKELY(stride_w == 0))
            beco_move(BECO_REG4, BECO_REG5);
        else
            beco_write_reg(BECO_REG4, p_w[4]);
        beco_shift_block5(BECO_REG0, 1);
        break;
    }
}

static inline void write_data_to_reg_q7(beco_vec64_in_t *p_w, int i_stride_w, int in_row,
                                        int i_ker_w, int stride_w4, int stride_w, int out_w)
{
    p_w += i_stride_w;
    p_w += in_row * (out_w / 8);
    switch (i_ker_w) {
    case 0:
        beco_write_reg(BECO_REG1, p_w[0]);
        if (LIKELY(i_stride_w == 0))
            beco_move(BECO_REG0, BECO_REG5);
        else
            beco_write_reg(BECO_REG0, *(p_w-1));
        beco_shift_block5(BECO_REG0, 7);
        break;
    case 1:
        beco_shift_block5(BECO_REG0, 1);
        break;
    case 2:
        if (LIKELY((i_stride_w + 1) == (stride_w4 * 4 + stride_w)))
            beco_move(BECO_REG1, BECO_REG5);
        else
            beco_write_reg(BECO_REG1, p_w[1]);
        beco_shift_block5(BECO_REG0, 1);
        break;
    }
}

template <size_t w_elem_sz, size_t flag>
    static void beco_write_to_reg(beco_vec64_in_t *p_w, int i_stride_w, int in_row,
                                  int i_ker_w, int stride_w4, int stride_w, int out_w);

#define BECO_WRITE_REG(m, f, write_reg_fn)      \
    template <> inline          \
    void beco_write_to_reg<m, f>(beco_vec64_in_t *p_w, int i_stride_w, int in_row,      \
                                 int i_ker_w, int stride_w4, int stride_w, int out_w) {     \
        write_reg_fn(p_w, i_stride_w, in_row, i_ker_w, stride_w4, stride_w, out_w);     \
    }

BECO_WRITE_REG(2, 1, write_data_to_reg4_q15)
BECO_WRITE_REG(2, 0, write_data_to_reg_q15)
BECO_WRITE_REG(1, 1, write_data_to_reg4_q7)
BECO_WRITE_REG(1, 0, write_data_to_reg_q7)

template <typename InType, typename WtType, typename OutType>
static void beco_dep_conv_process(const InType *Im_in,
                                  const uint16_t dim_im_in_x,
                                  const uint16_t dim_im_in_y,
                                  const uint16_t ch_im_in,
                                  const WtType *wt,
                                  const uint16_t ch_im_out,
                                  const uint16_t dim_kernel_x,
                                  const uint16_t dim_kernel_y,
                                  const uint16_t padding_x,
                                  const uint16_t padding_y,
                                  const uint16_t stride_x,
                                  const uint16_t stride_y,
                                  const InType *bias,
                                  const uint16_t bias_shift,
                                  const uint16_t out_shift,
                                  OutType *Im_out,
                                  const uint16_t dim_im_out_x,
                                  const uint16_t dim_im_out_y)
{
    unsigned ch, i, j, k, l;
    beco_vec32_out_t    *p_out  = (beco_vec32_out_t *)Im_out;
    beco_vec64_in_t     *p_w    = (beco_vec64_in_t *)Im_in;
    const unsigned      Accs    = 4;

    constexpr size_t w_elem_sz = sizeof(InType);
    constexpr size_t o_elem_sz = sizeof(OutType);

    constexpr size_t w_elem_per_vec = sizeof(beco_vec64_in_t)/w_elem_sz;
    constexpr size_t o_elem_per_vec = sizeof(*p_out)/o_elem_sz;
    constexpr size_t tile_w         = w_elem_per_vec / o_elem_per_vec;

    const uint32_t bias_factor      = 1 <<  bias_shift;
    const unsigned temp = w_elem_per_vec * Accs;
    const unsigned beco_4acc_cols   = dim_im_out_x / temp;
    const unsigned beco_acc_cols    = (dim_im_out_x % temp) / w_elem_per_vec;

    assert(ch_im_in == ch_im_out);
    assert(dim_im_in_x == dim_im_out_x);
    assert(dim_im_out_x % w_elem_per_vec == 0);

    for (ch = 0; ch < ch_im_in; ch++) {
        for (i = 0; i < dim_im_out_y; i++) {
            // cal by beco acc 0-3
            for (j = 0; j < beco_4acc_cols; j++) {
                beco_clear_acc(BECO_ACC0);
                beco_clear_acc(BECO_ACC1);
                beco_clear_acc(BECO_ACC2);
                beco_clear_acc(BECO_ACC3);
                beco_set_preload<w_elem_sz>(q_round(out_shift));
                beco_set_bias(bias, bias_factor);

                for (k = 0; k < dim_kernel_y; k++) {
                    int in_row = stride_y * i + k - padding_y;
                    for (l = 0; l < dim_kernel_x; l++) {
                        uint32_t v0 = wt[k * ch_im_in * dim_kernel_x + l * ch_im_in + ch];
                        if (LIKELY(in_row >= 0) && LIKELY(in_row < dim_im_in_y)) {
                            beco_write_to_reg<w_elem_sz, 1>(p_w, j, in_row, l, beco_4acc_cols,
                                                            beco_acc_cols, dim_im_out_x);
                            beco_mmacgr4(((beco_vec32_in_t){.u32 = v0}), BECO_REG0);
                        }
                    }
                }

                beco_read_tile<w_elem_sz, o_elem_sz, true>(p_out, out_shift);
                p_out += Accs * tile_w;
            }

            // cal by beco acc0
            int col_start = beco_4acc_cols * Accs;
            for (j = col_start; j < beco_acc_cols + col_start; j++) {
                beco_clear_acc(BECO_ACC0);
                beco_set_preload<w_elem_sz>(q_round(out_shift));
                beco_set_bias(bias, bias_factor);

                for (k = 0; k < dim_kernel_y; k++) {
                    int in_row = stride_y * i + k - padding_y;
                    for (l = 0; l < dim_kernel_x; l++) {
                        uint32_t v0 = wt[k * ch_im_in * dim_kernel_x + l * ch_im_in + ch];
                        if (LIKELY(in_row >= 0) && LIKELY(in_row < dim_im_in_y)) {
                            beco_write_to_reg<w_elem_sz, 0>(p_w, j, in_row, l, beco_4acc_cols,
                                                            beco_acc_cols, dim_im_out_x);
                            beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u32 = v0}), BECO_REG0);
                        }
                    }
                }

                beco_read_tile<w_elem_sz, o_elem_sz, false>(p_out, out_shift);
                p_out +=  tile_w;
            }
        }

        bias += 1;
        p_w += (dim_im_in_x * dim_im_in_y) / w_elem_per_vec;
    }
}

template <typename InType>  constexpr auto btype_mask = BECO_CONF_BTYPE_INT8;
template <>  constexpr auto btype_mask<int8_t>   = BECO_CONF_BTYPE_INT8;
template <>  constexpr auto btype_mask<int16_t>  = BECO_CONF_BTYPE_INT16;

template <typename OutType>  constexpr auto ctype_mask = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<int8_t>   = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<int16_t>  = BECO_CONF_PACK_INT16;
template <>  constexpr auto ctype_mask<int32_t>  = BECO_CONF_PACK_INT32;

template <typename InType>  constexpr auto localrotate_mask = 0;
template <>  constexpr auto localrotate_mask<int8_t>    = BECO_CONF_RD_16x8_ROT90;
template <>  constexpr auto localrotate_mask<int16_t>   = BECO_CONF_RD_16x16;

template <typename OutType>
static void beco_set_dep_conv_config(int scale)
{
    uint32_t config;
    if (sizeof(OutType) == sizeof(q31_t))
        scale = 0;

    config =
            BECO_CONF_BMODE_REP64 | BECO_CONF_AMODE_REP16 |
            BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8  |
            BECO_CONF_RSHIFT(scale) |
            BECO_CONF_PACK_INT32   | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);
}

template <typename InType, typename WtType, typename OutType>
void beco_depthwise_conv(const InType *Im_in,
                         const uint16_t dim_im_in_x,
                         const uint16_t dim_im_in_y,
                         const uint16_t ch_im_in,
                         const WtType *wt,
                         const uint16_t ch_im_out,
                         const uint16_t dim_kernel_x,
                         const uint16_t dim_kernel_y,
                         const uint16_t padding_x,
                         const uint16_t padding_y,
                         const uint16_t stride_x,
                         const uint16_t stride_y,
                         const InType *bias,
                         const uint16_t bias_shift,
                         const uint16_t out_shift,
                         OutType *Im_out,
                         const uint16_t dim_im_out_x,
                         const uint16_t dim_im_out_y)
{
    beco_set_dep_conv_config<OutType>(out_shift);

    beco_dep_conv_process<InType, WtType, OutType>(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in,
                                                   wt, ch_im_out, dim_kernel_x, dim_kernel_y,
                                                   padding_x, padding_y, stride_x, stride_y,
                                                   bias, bias_shift, out_shift, Im_out,
                                                   dim_im_out_x, dim_im_out_y);
}

#endif
