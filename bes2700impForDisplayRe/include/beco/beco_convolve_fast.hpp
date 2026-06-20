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

#ifndef __BECO_CONVOLVE_FAST_H__
#define __BECO_CONVOLVE_FAST_H__

#include <cstring>
#include <assert.h>

#include "beco.h"
#include "beco_asm.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "beco_read_acc_slice.hpp"

template <int n_acc_bundle, typename T> struct acc_output_t;
template <typename T> struct acc_output_t< 1, T > {
    static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
    typedef T type;
};
template <typename T> class acc_output_t< 2, T > {
    static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
    typedef typename std::conditional<std::is_signed<T>::value, int16_t, uint16_t>::type xint16_t;
    static constexpr bool is_single { sizeof(T) == 1 };
public:
    typedef typename std::conditional<is_single, xint16_t, T>::type type;
};
template <typename T> class acc_output_t< 4, T > {
    static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
    typedef typename std::conditional<std::is_signed<T>::value, int32_t, uint32_t>::type xint32_t;
    static constexpr bool is_32bit { sizeof(T) == 4 };
public:
    // handle float:
    typedef typename std::conditional<is_32bit, T, xint32_t>::type type;
};

template <typename a_t, typename b_t> struct BundledAccs {
    enum { value = sizeof(a_t) * sizeof(b_t) };
};

template <typename in_t, typename wt_t, typename out_t> struct AccType {
    typedef typename acc_output_t< BundledAccs<wt_t, in_t>::value, out_t >::type type;
};


void setTruncate(bool trunc,int& round,uint16_t out_rshift) {
    round = (trunc)? 0: q_round(out_rshift);
}

template <typename PType>  constexpr auto atype_mask = BECO_CONF_ATYPE_INT8;
template <>  constexpr auto atype_mask<q7_t>   = BECO_CONF_ATYPE_INT8;
template <>  constexpr auto atype_mask<q15_t>  = BECO_CONF_ATYPE_INT16;

template <typename MType, typename OType>  constexpr auto ctype_mask = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<q7_t, q7_t>   = BECO_CONF_PACK_INT8;
template <>  constexpr auto ctype_mask<q7_t, q15_t>  = BECO_CONF_PACK_INT16;
template <>  constexpr auto ctype_mask<q15_t, q15_t>  = BECO_CONF_PACK_INT32;
template <>  constexpr auto ctype_mask<q15_t, q31_t>  = BECO_CONF_PACK_INT32;

template <typename PType, typename MType>  constexpr auto localrotate_mask = 0;
template <>  constexpr auto localrotate_mask<q7_t, q7_t>   = BECO_CONF_RD_8x8;
template <>  constexpr auto localrotate_mask<q7_t, q15_t>   = BECO_CONF_RD_16x8_ROT90;
template <>  constexpr auto localrotate_mask<q15_t, q7_t>   = BECO_CONF_RD_16x8;
template <>  constexpr auto localrotate_mask<q15_t, q15_t>   = BECO_CONF_RD_16x8_ROT90;

template <typename in_t>  constexpr int SIMDWidthWith = 8;
template <>  constexpr int SIMDWidthWith<q7_t>   = 8;
template <>  constexpr int SIMDWidthWith<q15_t>  = 4;

template <typename wt_t>  constexpr int chanPerBatchWith = 8;
template <>  constexpr int chanPerBatchWith<q7_t>   = 8;
template <>  constexpr int chanPerBatchWith<q15_t>  = 4;

template <typename out_t>  constexpr auto SSAT_V = 8;
template <>  constexpr auto SSAT_V<q7_t>   = 8;
template <>  constexpr auto SSAT_V<q15_t>  = 16;
template <>  constexpr auto SSAT_V<q31_t>  = 32;

template <typename PType, typename MType, typename OType>
static void beco_set_conv_config(int scale)
{
    if (sizeof(OType) == sizeof(q31_t))
        scale = 0;
    uint32_t config;
    config =
            BECO_CONF_BMODE_REP32 | BECO_CONF_AMODE_REP32 |
            atype_mask<PType>     | BECO_CONF_BTYPE_INT8  |
            BECO_CONF_RSHIFT(scale)  |
            ctype_mask<MType, OType> | localrotate_mask<PType,MType>;
    beco_write_config(config);
}


template <typename in_t>
void flush(in_t **imcache, in_t *im_kernel, const int dim_kernel_dila_y, const int stripSize)
{
    in_t *p = im_kernel;

    for (int i = 0; i < dim_kernel_dila_y; i++) {
        imcache[i] = p;
        p += stripSize/dim_kernel_dila_y;
    }
}

inline void clearCacheValid(bool *cache_valid, const int dim_kernel_dila_y)
{
    memset(cache_valid,false,sizeof(*cache_valid)*dim_kernel_dila_y);
}

template <typename T>
void swap(T &a, T &b) {T c; c = a; a = b; b = c; }

template <typename in_t>
void copy_channels(in_t *_pout,
                   const in_t *_pin,
                   const int ch_im_in,
                   const int stride_x,
                   const int SIMDWidth,
                   const int dim_im_in_x,
                   const int dim_im_in_y)
{
    if (stride_x == 1) {
        for (int c = 0; c < ch_im_in; c++) {
            memcpy(_pout, _pin, sizeof(*_pout) * SIMDWidth);
            _pin += dim_im_in_x * dim_im_in_y;
            _pout += SIMDWidth;
        }
        return;
    }

    const in_t *_pin2 = _pin;
    for (int c = 0; c < ch_im_in; c++) {
        _pin2 = _pin;
        for (int x = 0; x < SIMDWidth; x++) {
            *_pout++ = *_pin2;
            _pin2 += stride_x;
        }
        _pin += dim_im_in_x * dim_im_in_y;
    }

}

inline int obtainStep(int a, int b, int c, int d){
    return ((a - b * d) % c == 0)?((a - b * d) / c):((a - b * d) / c + 1);
}

template <typename in_t>
void getStrip(int y,
              int x,
              const in_t *pim,
              in_t *pout,
              const int ch_im_in,
              const int dim_im_in_x,
              const int dim_im_in_y,
              const int dim_kernel_x,
              const int padding_start_x,
              const int padding_end_x,
              const int stride_x,
              const int maxStepX,
              const int stepX,
              const int SIMDWidth,
              const int d_coeff)
{
    int kx;
    int currentStep;

    in_t *po = pout;

    pim +=  dim_im_in_x * y - padding_start_x + x;

    for (kx = 0; kx < dim_kernel_x; kx++) {
        copy_channels(pout, pim, ch_im_in, stride_x, SIMDWidth, dim_im_in_x, dim_im_in_y);
        pim += d_coeff;
        pout += ch_im_in * SIMDWidth;
    }

    // Clear values outside input matrix
    if (stepX <= padding_start_x / SIMDWidth) {
        for (int px = 0; px < padding_start_x; px++) {
            if (px / SIMDWidth == stepX) {
                currentStep = obtainStep(padding_start_x, px, d_coeff, stride_x);
                for (int c = 0; c < currentStep * ch_im_in; c++) {
                    po[c * SIMDWidth + px % SIMDWidth] = 0;
                }
            }
        }
    }

    if (maxStepX - stepX <= padding_end_x / SIMDWidth) {
        for (int px = 0; px < padding_end_x; px++) {
            if (maxStepX - stepX == px / SIMDWidth) {
                currentStep = obtainStep(padding_end_x, px, d_coeff, stride_x);
                for (int c = 0; c < currentStep * ch_im_in; c++) {
                    pout[-(c * SIMDWidth + px % SIMDWidth) - 1] = 0;
                }
            }
        }
    }

}

template <typename in_t>
void updateImcache(int iy,
                   int ix,
                   const in_t *Im_in,
                   in_t **imcache,
                   bool *cache_valid,
                   const int ch_im_in,
                   const int dim_im_in_x,
                   const int dim_im_in_y,
                   const int dim_kernel_x,
                   const int dim_kernel_dila_y,
                   const int padding_start_x,
                   const int padding_end_x,
                   const int padding_y,
                   const int stride_x,
                   const int stride_y,
                   const int maxStepX,
                   const int stepX,
                   const int SIMDWidth,
                   const int d_coeff)
{
    // Discard expired entries.
    if (iy != -padding_y) {
        for (int i = 0; i < dim_kernel_dila_y - stride_y; i++) {
            swap(imcache[i], imcache[i + stride_y]);
            cache_valid[i] = cache_valid[i + stride_y];
            cache_valid[i + stride_y] = false;
        }
        for (int i = dim_kernel_dila_y - stride_y; i < dim_kernel_dila_y; i++) {
            cache_valid[i] = false;
        }
    }

    // Load new entries.
    for (int i = 0; i < dim_kernel_dila_y; i += d_coeff) { // liu add
        if (iy + i >= dim_im_in_y) // liu add
            cache_valid[i] = false;
        else if (iy + i >= 0 && cache_valid[i] == false) {
            assert(iy + i < dim_im_in_y); // liu add
            getStrip(iy + i, ix, Im_in, imcache[i], ch_im_in, dim_im_in_x, dim_im_in_y,
                    dim_kernel_x, padding_start_x, padding_end_x, stride_x, maxStepX,
                    stepX, SIMDWidth, d_coeff);
            cache_valid[i] = true;
        }
    }

}

template <typename in_t,typename wt_t> void beco_set_preload_acc(int32_t round) {};
template <> void beco_set_preload_acc<q7_t,q7_t>(int32_t round)
{
    beco_set_preload_8x8_4ACC( round );
}

template <> void beco_set_preload_acc<q15_t,q15_t>(int32_t round)
{
    beco_set_preload_16x16_4ACC( round );
}

template <typename bias_t, typename in_t>
void matmul_set_bias(const bias_t *bias, const in_t *factor)
{

    const uint32_t *w = (const uint32_t *)factor;
    const uint32_t *p = (const uint32_t *)bias;

    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = { p[0], p[1] } }));
    beco_write_reg(BECO_REG3, ((beco_vec64_in_t){.u32 = { w[0], w[1] } }));
    beco_mmacrr(BECO_REG3, BECO_REG2);

}

inline bool valid(const int iy, const int ky, bool *cache_valid)
{
    return (iy + ky >= 0 && cache_valid[ky]);
}

template <typename in_t>
inline bool getArrangedInput(const int iy, const int ky, in_t **imcache,
                            bool *cache_valid, in_t *&col)
{

    if (!valid(iy, ky, cache_valid)) return false;
    col = imcache[ky];

    return true;
}

template <typename in_t, typename wt_t>
void matmul_kernel(const wt_t *w,
                   const in_t *p,
                   const int ch_im_in,
                   const int ch_im_out,
                   const int dim_kernel_x,
                   const int dim_kernel_y,
                   const int SIMDWidth,
                   const int wt_stride)
{

    wt_t *iw = (wt_t*)w;
    int dim_kernel_x2 = dim_kernel_x;

    while (dim_kernel_x2 > 0) {

        wt_t *iw2 = iw;
        int ch_im_in2 = ch_im_in;

        while (ch_im_in2 > 0) {

            const uint32_t *w = (const uint32_t *)p;
            const uint32_t *p1 = (const uint32_t *)iw2;

            beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = { p1[0], p1[1] } }));
            beco_write_reg(BECO_REG3, ((beco_vec64_in_t){.u32 = { w[0], w[1] } }));
            beco_mmacrr(BECO_REG3, BECO_REG2);

            p += SIMDWidth;
            iw2 += wt_stride; // wt_stride = dim_kernel_y * dim_kernel_x * ch_im_out
            --ch_im_in2;
        }

        iw += ch_im_out;
        --dim_kernel_x2;
    }
}


// read out function
template <typename in_t, typename wt_t, typename out_t>
    static void beco_convolve_read_tile(beco_vec32_out_t out[], int stride, int scale) {};

#define BECO_GEN_TILE_READ_CONVOLVE(a, b, o, read_fn)  \
	template <>                                \
	void beco_convolve_read_tile<a, b, o>(beco_vec32_out_t out[], int stride, int scale) { \
		read_fn(out, stride, scale);                         \
	}

BECO_GEN_TILE_READ_CONVOLVE(q7_t, q7_t, q7_t, beco_mmacrr_read_8x8_8)
BECO_GEN_TILE_READ_CONVOLVE(q7_t, q7_t, q15_t, beco_mmacrr_read_8x8_16)
BECO_GEN_TILE_READ_CONVOLVE(q15_t, q15_t, q15_t, beco_mmacrr_read_16x16_16)
BECO_GEN_TILE_READ_CONVOLVE(q15_t, q15_t, q31_t, beco_mmacrr_read_16x16_32)

template <typename in_t, typename wt_t, typename out_t>
void beco_convolve_fast(const in_t *Im_in,
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
                        const wt_t *bias,
                        const uint16_t bias_shift,
                        const uint16_t out_shift,
                        out_t  *Im_out,
                        const uint16_t dim_im_out_x,
                        const uint16_t dim_im_out_y,
                        const uint16_t d_coeff)
{

    // Input check
    if (sizeof(wt_t) == sizeof(q7_t)) {
        assert(ch_im_out % 8 == 0);
    } else if (sizeof(wt_t) == sizeof(q15_t)) {
        assert(ch_im_out % 4 == 0);
    }

    if (sizeof(in_t) == sizeof(q7_t)) {
        assert(dim_im_out_x % 8 == 0);
    } else if(sizeof(in_t) == sizeof(q15_t)) {
        assert(dim_im_out_x % 4 == 0);
    }
    assert(bias_shift < sizeof(in_t) * 8);

    int round;
    const int SIMDWidth = SIMDWidthWith<in_t>;
    const int chanPerBatch = chanPerBatchWith<wt_t>;
    const int outStride = sizeof(beco_vec32_out_t) / sizeof(out_t);

    wt_t *weights = (wt_t*)wt;
    const int outStridePerChannel = dim_im_out_x * dim_im_out_y;
    in_t bias_factor[SIMDWidth];

    // replicate bias multiply factor
    for (int i = 0; i < SIMDWidth; i++) {
        bias_factor[i] = 1 << bias_shift;
    }

    //Prepare config  before running beco convolve function
    setTruncate(false,round,out_shift);

    int x,y; //coord in out[]
    int ix,iy; //coord in input[]

    const int dim_kernel_dila_y = dim_kernel_y + (dim_kernel_y - 1) * (d_coeff - 1);

    const int padding_start_x = padding_x;
    const int padding_end_x = (dim_im_out_x - 1) * stride_x + dim_kernel_x +
                              (dim_kernel_x - 1) * (d_coeff - 1) - dim_im_in_x - padding_x;

    assert((padding_start_x - padding_end_x) == 1 || (padding_start_x - padding_end_x) == 0);

    const int maxStepX = dim_im_out_x / SIMDWidth - 1;

    const int wt_stride = ch_im_out * dim_kernel_x * dim_kernel_y;

    in_t *imcache[dim_kernel_dila_y];
    bool cache_valid[dim_kernel_dila_y];

    const int kernelSize = ch_im_in * dim_kernel_dila_y * dim_kernel_x;
    const int stripSize = SIMDWidth * kernelSize;

    in_t im_kernel[stripSize];
    flush(imcache, im_kernel, dim_kernel_dila_y, stripSize);
    beco_set_conv_config<in_t, wt_t, out_t>(out_shift);

    ix = 0;
    int stepX = 0;

    for (x = 0; x < dim_im_out_x; x += SIMDWidth, stepX++) {

        iy = - padding_y;
        beco_vec32_out_t *beco_out = (beco_vec32_out_t *)Im_out;
        clearCacheValid(cache_valid, dim_kernel_dila_y);

        for (y = 0; y < dim_im_out_y; y++, iy += stride_y) {

            updateImcache(iy, ix, Im_in, imcache, cache_valid, ch_im_in, dim_im_in_x, dim_im_in_y,
                           dim_kernel_x, dim_kernel_dila_y, padding_start_x, padding_end_x,
                           padding_y, stride_x, stride_y, maxStepX, stepX, SIMDWidth, d_coeff);

            for (int ch = 0; ch < ch_im_out; ch += chanPerBatch) {

                wt_t *w = weights + ch;
                beco_set_preload_acc<in_t, wt_t>(round);

                if (bias)
                matmul_set_bias(bias + ch, bias_factor);

                for (int ky = 0; ky < dim_kernel_dila_y; ky += d_coeff) {

                    in_t *col;
                    if (getArrangedInput(iy, ky, imcache, cache_valid, col))
                        matmul_kernel(w, col, ch_im_in, ch_im_out, dim_kernel_x, dim_kernel_y,
                                      SIMDWidth, wt_stride);

                    w += ch_im_out * dim_kernel_x;
                }

                beco_convolve_read_tile<in_t, wt_t, out_t>(
                    &beco_out[ch*outStridePerChannel/outStride],
                    outStridePerChannel/outStride, out_shift);
            }

            beco_out += dim_im_out_x / outStride;
        }

        Im_out += SIMDWidth;
        ix += SIMDWidth * stride_x;
    }
}

#endif