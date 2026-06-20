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

#ifndef __BECO_CONVOLVE_1XN_FAST_H__
#define __BECO_CONVOLVE_1XN_FAST_H__

#include <assert.h>
#include "plat_types.h"

#include "beco.h"
#include "beco_l1.h"
#include "beco_bias.hpp"
#include "beco_read_acc_slice.hpp"


template <typename in_t> constexpr auto in_t_mask = BECO_CONF_ATYPE_INT8;
template <>  constexpr auto in_t_mask<q7_t>   = BECO_CONF_ATYPE_INT8;
template <>  constexpr auto in_t_mask<q15_t>  = BECO_CONF_ATYPE_INT16;

template <typename in_t, typename wt_t>  constexpr auto localrotate_mask = 0;
template <>  constexpr auto localrotate_mask<q7_t, q7_t>   = BECO_CONF_RD_8x8;
template <>  constexpr auto localrotate_mask<q7_t, q15_t>   = BECO_CONF_RD_16x8_ROT90;
template <>  constexpr auto localrotate_mask<q15_t, q7_t>   = BECO_CONF_RD_16x8;
template <>  constexpr auto localrotate_mask<q15_t, q15_t>   = BECO_CONF_RD_16x8_ROT90;

template <typename wt_t, typename out_t>  constexpr auto out_t_mask = BECO_CONF_PACK_INT8;
template <>  constexpr auto out_t_mask<q7_t, q7_t>   = BECO_CONF_PACK_INT8;
template <>  constexpr auto out_t_mask<q7_t, q15_t>  = BECO_CONF_PACK_INT16;
template <>  constexpr auto out_t_mask<q15_t, q15_t>  = BECO_CONF_PACK_INT32;
template <>  constexpr auto out_t_mask<q15_t, q31_t>  = BECO_CONF_PACK_INT32;

//
// set config function
//
template<typename in_t, typename wt_t, typename out_t>
void beco_conv1xn_set_config(int scale)
{
    if (sizeof(out_t) == sizeof(q31_t))
        scale = 0;
    uint32_t config;
    config = BECO_CONF_AMODE_REP32   | BECO_CONF_BMODE_REP32  |
             in_t_mask<in_t>         | BECO_CONF_BTYPE_INT8   |
             BECO_CONF_RSHIFT(scale) |
             out_t_mask<wt_t,out_t>  | localrotate_mask<in_t,wt_t>;
    beco_write_config(config);
}

//
// Set bias function
//
void beco_conv1xn_fast_add_bias(const beco_vec64_in_t *pb, const beco_vec64_in_t *q)
{
    beco_write_reg(BECO_REG5, *q);
    beco_write_reg(BECO_REG6, pb[0]);
    beco_mmacrr(BECO_REG5, BECO_REG6);
}

//
// Read out function
//
template <typename in_t, typename wt_t, typename out_t>
    static void beco_conv1xn_fast_read_tile(beco_vec32_out_t out[], int stride, int scale) {};

#define BECO_GEN_TILE_READ_CONV1XN(a, b, o, read_fn) \
    template <>                               \
    void beco_conv1xn_fast_read_tile<a, b, o>(beco_vec32_out_t out[], int stride, int scale){\
        read_fn(out, stride, scale);                                                \
    }

BECO_GEN_TILE_READ_CONV1XN(q7_t, q7_t, q7_t, beco_mmacrr_read_8x8_8)
BECO_GEN_TILE_READ_CONV1XN(q7_t, q7_t, q15_t, beco_mmacrr_read_8x8_16)
BECO_GEN_TILE_READ_CONV1XN(q15_t, q15_t, q15_t, beco_mmacrr_read_16x16_16)
BECO_GEN_TILE_READ_CONV1XN(q15_t, q15_t, q31_t, beco_mmacrr_read_16x16_32)

// //
// // Set preload function
// //
template <typename in_t,typename wt_t> void beco_conv1xn_fast_set_preload_acc(int32_t round) {};
template <> void beco_conv1xn_fast_set_preload_acc<q7_t, q7_t>(int32_t round)\
                 { beco_set_preload_8x8_4ACC(round); }
template <> void beco_conv1xn_fast_set_preload_acc<q15_t, q15_t>(int32_t round)\
                 { beco_set_preload_16x16_4ACC(round);}

//
// Accumulation function
//
template <typename in_t, size_t size_t_in>
void beco_conv1xn_fast_impl(const beco_vec64_in_t *p_im_in,
                            const beco_vec64_in_t *pk,
                            const int inx_stride,
                            const int ch_im_in,
                            const int dim_kernel_x,
                            const int stride_wgt,
                            const int simd_width)
{
    for (int i = 0; i < ch_im_in; i++) {
        beco_write_reg(BECO_REG0, p_im_in[0]);
        beco_write_reg(BECO_REG1, p_im_in[1]);
        int temp = 0;
        int t_kx = 1;
        for (int kx = 1; kx <= dim_kernel_x; kx++, t_kx++) {
            beco_write_reg(BECO_REG5, pk[0]);
            beco_mmacrr(BECO_REG0, BECO_REG5);

            pk += stride_wgt;

            beco_shift_block5(BECO_REG0, size_t_in);
            if (UNLIKELY(t_kx == simd_width)) {
                t_kx = 1;
                beco_write_reg(BECO_REG1, p_im_in[2+temp]);
                temp++;
            }
        }
        p_im_in += inx_stride;
    }
}

//
// beco convolve 1xn fast function
//
template <typename in_t, typename wt_t, typename b_t, typename out_t>
void beco_convolve_1xn_fast_process(const in_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const wt_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const uint16_t dilation_x,
                                    const uint16_t dilation_y,
                                    const b_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    out_t *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y)
{
    assert(dim_im_in_y == 1 && dim_kernel_y == 1 && dim_im_out_y == 1);
    assert(padding_x == 0 && dilation_x == 1 && bias_shift < 8*sizeof(b_t));
    assert(stride_x == 1);

    constexpr int size_t_in = sizeof(in_t);
    constexpr int SIMDHeight = 8 / size_t_in;  // B mode input Im_in number
    constexpr int SIMDWidth = 8 / sizeof(wt_t);    // A mode input wt number

    assert(ch_im_out % SIMDWidth == 0 && dim_im_in_x % SIMDHeight == 0);
    assert((dim_im_out_x * stride_x) % SIMDHeight == 0);

    in_t bias_factor[SIMDHeight];
    for (int i = 0; i < SIMDHeight; i++) {
        bias_factor[i] = 1 << bias_shift;
    }
    const beco_vec64_in_t *q = (beco_vec64_in_t *)bias_factor;

    int32_t round = q_round(out_shift);

    const int inw_stride = dim_im_in_x / SIMDHeight;
    const int outch_strip_64bit = ch_im_out / SIMDWidth;
    const int outStride = sizeof(beco_vec32_out_t) / sizeof(out_t);
    int ow_strip_64bit =  (dim_im_out_x * stride_x) / SIMDHeight;

    const beco_vec64_in_t *beco_im_in = (beco_vec64_in_t *)Im_in;
    const beco_vec64_in_t *beco_wt = (beco_vec64_in_t *)wt;
    const beco_vec64_in_t *p_bias = (beco_vec64_in_t *)bias;
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)Im_out;

    for (int i = 0; i < outch_strip_64bit; i++) {
        beco_vec32_out_t *current_beco_out = (beco_vec32_out_t *)beco_out;
        for (int j = 0; j < ow_strip_64bit; j++) {
            beco_conv1xn_fast_set_preload_acc<in_t, wt_t>(round);

            if (bias)
                beco_conv1xn_fast_add_bias(p_bias, q);

            beco_conv1xn_fast_impl<in_t, size_t_in>(&beco_im_in[j], beco_wt, inw_stride, ch_im_in,
                                            dim_kernel_x, outch_strip_64bit, SIMDHeight);

            beco_conv1xn_fast_read_tile<in_t, wt_t, out_t>(
                current_beco_out, dim_im_out_x/outStride, out_shift);

            current_beco_out += SIMDHeight / outStride;
        }

        beco_wt++;
        p_bias++;
        beco_out += SIMDWidth * dim_im_out_x / outStride;
    }
}

//
// beco convolve 1xn fast function
//
template <typename in_t, typename wt_t, typename b_t, typename out_t>
void beco_convolve_1xn_fast(const in_t *Im_in,
                       const uint16_t dim_im_in_x,
                       const uint16_t dim_im_in_y,
                       const uint16_t ch_im_in,
                       const wt_t *wt,
                       const uint16_t ch_im_out,
                       const uint16_t dim_kernel_x,
                       const uint16_t dim_kernel_y,
                       const uint16_t padding_x,
                       const uint16_t padding_y,
                       const uint16_t stride_x,
                       const uint16_t stride_y,
                       const uint16_t dilation_x,
                       const uint16_t dilation_y,
                       const b_t *bias,
                       const uint16_t bias_shift,
                       const uint16_t out_shift,
                       out_t *Im_out,
                       const uint16_t dim_im_out_x,
                       const uint16_t dim_im_out_y)
{
    beco_conv1xn_set_config<in_t, wt_t, out_t>(out_shift);

    beco_convolve_1xn_fast_process<in_t, wt_t, b_t, out_t>(Im_in, dim_im_in_x, dim_im_in_y,
                ch_im_in, wt, ch_im_out, dim_kernel_x, dim_kernel_y, padding_x, padding_y,
                stride_x, stride_y, dilation_x, dilation_y, bias, bias_shift, out_shift,
                Im_out, dim_im_out_x, dim_im_out_y);
}

#endif


