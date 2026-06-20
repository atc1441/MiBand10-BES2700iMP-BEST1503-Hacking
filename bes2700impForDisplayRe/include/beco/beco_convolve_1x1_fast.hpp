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

#ifndef __BECO_CONVOLVE_1X1_FAST_H__
#define __BECO_CONVOLVE_1X1_FAST_H__

#include <assert.h>

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
void beco_conv1x1_set_config(int scale)
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
// Accumulation function
//
static void beco_conv1x1_fast_impl(const beco_vec64_in_t *pIn, const beco_vec64_in_t *pWt,
                                   const uint16_t in_ch, const int beco_in_stride,
                                   const int beco_wt_stride)
{
    for (int i_in_ch = 0; i_in_ch < in_ch; i_in_ch++) {
        beco_write_reg(BECO_REG3, pIn[0]);
        beco_write_reg(BECO_REG4, pWt[0]);
        beco_mmacrr(BECO_REG3, BECO_REG4);
        pIn += beco_in_stride;
        pWt += beco_wt_stride;
    }
}

//
// Read out function
//
template <typename in_t, typename wt_t, typename out_t>
    static void beco_conv1x1_read_tile(beco_vec32_out_t out[], int stride, int scale) {};

#define BECO_GEN_TILE_READ_CONV1X1(a, b, o, read_fn)  \
	template <>                                \
	void beco_conv1x1_read_tile<a, b, o>(beco_vec32_out_t out[], int stride, int scale) { \
		read_fn(out, stride, scale);                         \
	}

BECO_GEN_TILE_READ_CONV1X1(q7_t, q7_t, q7_t, beco_mmacrr_read_8x8_8)
BECO_GEN_TILE_READ_CONV1X1(q7_t, q7_t, q15_t, beco_mmacrr_read_8x8_16)
BECO_GEN_TILE_READ_CONV1X1(q15_t, q15_t, q15_t, beco_mmacrr_read_16x16_16)
BECO_GEN_TILE_READ_CONV1X1(q15_t, q15_t, q31_t, beco_mmacrr_read_16x16_32)

//
// Set bias function
//
void beco_conv1x1_add_bias(const beco_vec64_in_t *pb, const beco_vec64_in_t *q)
{
    beco_write_reg(BECO_REG2, *q);
    beco_write_reg(BECO_REG3, pb[0]);
    beco_mmacrr(BECO_REG2, BECO_REG3);
}

//
// set preload function
//
template <typename in_t, typename wt_t>
    static void beco_conv1x1_set_preload(int32_t q_bias);

#define BECO_GEN_SET_PRELOAD_CONV1X1(a, b, set_preload_fn)  \
    template <>                                          \
    void beco_conv1x1_set_preload<a, b>(int32_t q_bias) {\
        set_preload_fn(q_bias);                              \
    }

BECO_GEN_SET_PRELOAD_CONV1X1(q7_t, q7_t, beco_set_preload_8x8_4ACC)
BECO_GEN_SET_PRELOAD_CONV1X1(q15_t, q15_t, beco_set_preload_16x16_4ACC)

//
// beco convolve 1x1 fast function
//
template<typename in_t, typename wt_t, typename b_t, typename out_t>
void beco_conv1x1_fast_process(const in_t *Im_in,
                               const uint16_t dim_im_in_x,
                               const uint16_t dim_im_in_y,
                               const uint16_t ch_im_in,
                               const wt_t *wt,
                               const uint16_t ch_im_out,
                               const b_t *bias,
                               const uint16_t bias_shift,
                               const uint16_t out_shift,
                               out_t *Im_out,
                               const uint16_t dim_im_out_x,
                               const uint16_t dim_im_out_y)
{
    const int im_size = dim_im_in_y * dim_im_in_x;
    constexpr int size_t_o = sizeof(out_t);
    constexpr int SIMDInSize = 8 / sizeof(in_t);
    constexpr int SIMDWtSize = 8 / sizeof(wt_t);

    assert(im_size % SIMDInSize == 0 && ch_im_out % SIMDWtSize == 0);

    in_t bias_factor[SIMDWtSize];
    for (int i = 0; i < SIMDWtSize; i++) {
        bias_factor[i] = 1 << bias_shift;
    }
    const beco_vec64_in_t *q = (beco_vec64_in_t *)bias_factor;

    const int32_t round = q_round(out_shift);

    const int outsize_stride = im_size / (4 / size_t_o);
    const int loopcnt_insize = im_size / SIMDInSize;
    const int loopcnt_outch = ch_im_out / SIMDWtSize;

    const beco_vec64_in_t *beco_wt = (beco_vec64_in_t *)wt;
    const beco_vec64_in_t *beco_im_in = (beco_vec64_in_t *)Im_in;
    const beco_vec64_in_t *beco_bias = (beco_vec64_in_t *)bias;
    beco_vec32_out_t *beco_im_out =  (beco_vec32_out_t *)Im_out;

    for (int i = 0; i < loopcnt_outch; i++) {
        const beco_vec64_in_t *cur_beco_im_in = beco_im_in;
        beco_vec32_out_t *cur_beco_im_out = beco_im_out;
        for (int j = 0; j < loopcnt_insize; j++) {
            beco_conv1x1_set_preload<in_t, wt_t>(round);
            if (bias)
                beco_conv1x1_add_bias(beco_bias, q);
            beco_conv1x1_fast_impl(cur_beco_im_in, beco_wt, ch_im_in,
                                   loopcnt_insize, loopcnt_outch);
            beco_conv1x1_read_tile<in_t, wt_t, out_t>(cur_beco_im_out, outsize_stride, out_shift);
            cur_beco_im_out += SIMDInSize / (4 / size_t_o);
            cur_beco_im_in++;
        }
        beco_bias++;
        beco_im_out += SIMDWtSize * outsize_stride;
        beco_wt++;
    }
}

template<typename in_t, typename wt_t, typename b_t, typename out_t>
void beco_convolve_1x1_fast(const in_t *Im_in,
                            const uint16_t dim_im_in_x,
                            const uint16_t dim_im_in_y,
                            const uint16_t ch_im_in,
                            const wt_t *wt,
                            const uint16_t ch_im_out,
                            const b_t *bias,
                            const uint16_t bias_shift,
                            const uint16_t out_shift,
                            out_t *Im_out,
                            const uint16_t dim_im_out_x,
                            const uint16_t dim_im_out_y)
{
    beco_conv1x1_set_config<in_t, wt_t, out_t>(out_shift);

    beco_conv1x1_fast_process(Im_in, dim_im_in_x, dim_im_in_y, ch_im_in,
                              wt, ch_im_out, bias, bias_shift, out_shift,
                              Im_out, dim_im_out_x, dim_im_out_y);
}

#endif


