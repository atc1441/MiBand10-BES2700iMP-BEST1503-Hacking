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

#ifndef _NATIVE_CONVOLVE_H
#define _NATIVE_CONVOLVE_H

#include "beco_common.h"
#include "beco_types.h"
#include "beco_asm.h"


// pM - [dim_vec, num_of_rows]
template <typename Tv, typename Tm, typename Tb, typename To>
void naive_fully_connected(
        const Tv * pV,
        const Tm * pM,
        const Tb * bias,
        const uint16_t dim_vec,
        const uint16_t num_of_rows,
        const uint16_t bias_shift,
        const uint16_t out_shift,
        To * pOut)
{
    for (int i = 0; i < num_of_rows; i++) {
        int acc = ((q31_t)(bias[i]) << bias_shift) + q_round(out_shift);

        for (int j = 0; j < dim_vec; j++) {
            acc += pV[j] * pM[j * num_of_rows + i]; // Note: "M" is in natural order.
        }

        const int pix_size = 8 * sizeof(To);
        pOut[i] = (To)BECO_SSAT((acc >> out_shift), pix_size);
    }
}

template <
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY>
int ref_convolve_HWC_q15(const q15_t *in,
                                   const q15_t *wt,
                                   const q15_t *bias,
                                   const uint16_t bias_shift,
                                   const uint16_t out_shift,
                                   q15_t *out)
{
    int i, j, k, l, m, n;
    int acc, iy, ix;

    for (j = 0; j < OH; j++) {
        for (k = 0; k < OW; k++) {
            for (i = 0; i < OCH; i++) {

                acc = ((q31_t)bias[i] << bias_shift) + q_round(out_shift);
                for (m = 0; m < KY; m++) {
                    iy = SY * j + m - PY;
                    if (iy >= 0 && iy < IH) {
                        for (n = 0; n < KX; n++) {
                            ix = SX * k + n - PX;
                            if (ix >= 0 && ix < IW) {

                            	// Optimize for ICH % 2 == 0:
                                if (ICH % 2 != 0) {
                                    for (l = 0; l < ICH; l++) {
                                        acc += in[ iy*IW*ICH + ix*ICH + l] *
                                            wt[ i*ICH*KX*KY + m*KX*ICH + n*ICH + l];
                                    }
                                }
                                else {
                                	const uint32_t *pin = (const uint32_t *)&in[ iy*IW*ICH + ix*ICH ];
                                	const uint32_t *pwt = (const uint32_t *)&wt[ i*ICH*KX*KY + m*KX*ICH + n*ICH ];
                                    for (l = 0; l < ICH; l += 2)
                                        acc = BECO_SMLAD(*pin++, *pwt++, acc);
                                }
                            }
                        }
                    }
                }
                *out++ = (q15_t)BECO_SSAT((acc >> out_shift), 16);
            }

        }
    }
    return BECO_SUCCESS;
}

template <
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY>
int ref_convolve_HWC_q7(const q7_t *in,
                                   const q7_t *wt,
                                   const q7_t *bias,
                                   const uint16_t bias_shift,
                                   const uint16_t out_shift,
                                   q7_t *out)
{
    int i, j, k, l, m, n;
    int acc, iy, ix;

    for (j = 0; j < OH; j++) {
        for (k = 0; k < OW; k++) {
            for (i = 0; i < OCH; i++) {

                acc = ((q31_t)bias[i] << bias_shift) + q_round(out_shift);
                for (m = 0; m < KY; m++) {
                    iy = SY * j + m - PY;
                    if (iy >= 0 && iy < IH) {
                        for (n = 0; n < KX; n++) {
                            ix = SX * k + n - PX;
                            if (ix >= 0 && ix < IW) {

                            	// Optimize for ICH % 2 == 0:
                                if (ICH % 4 != 0) {
                                    for (l = 0; l < ICH; l++) {
                                        acc += in[ iy*IW*ICH + ix*ICH + l] *
                                            wt[ i*ICH*KX*KY + m*KX*ICH + n*ICH + l];
                                    }
                                }
                                else {
                                	const uint32_t *pin = (const uint32_t *)&in[ iy*IW*ICH + ix*ICH ];
                                	const uint32_t *pwt = (const uint32_t *)&wt[ i*ICH*KX*KY + m*KX*ICH + n*ICH ];
                                    for (l = 0; l < ICH; l += 4) {
                                        uint32_t a = *pin++;
                                        uint32_t b = *pwt++;
                                        acc = BECO_SMLAD(BECO_SXTB16(a, 0), BECO_SXTB16(b, 0), acc);
                                        acc = BECO_SMLAD(BECO_SXTB16(a, 8), BECO_SXTB16(b, 8), acc);
                                    }
                                }
                            }
                        }
                    }
                }
                *out++ = (q7_t)BECO_SSAT((acc >> out_shift), 8);
            }

        }
    }
    return BECO_SUCCESS;
}

// input - CHW, kernel - Cin,H,W,Cout, Output - CHW
// input - CHW, kernel - Cin,H,W,Cout, Output - CHW
template <
    typename OUT_T,
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY,
    int DC>
int ref_convolve_CHW_q15(const q15_t *in,
                        const q15_t *wt,
                        const q15_t *bias,
                        const uint16_t bias_shift,
                        const uint16_t out_shift,
                        OUT_T *out)
{
    int i, j, k, l, m, n;
    int acc, iy, ix;

    for (j = 0; j < OCH; j++) {
        for (k = 0; k < OH; k++) {
            for (i = 0; i < OW; i++) {

                acc = ((q31_t)bias[j] << bias_shift) + q_round(out_shift);
                for (m = 0; m < KY; m++) {
                    iy = SY * k + m * DC - PY;
                    if (iy >= 0 && iy < IH) {
                        for (n = 0; n < KX; n++) {
                            ix = SX * i + n * DC - PX;
                            if (ix >= 0 && ix < IW) {
                                for (l = 0; l < ICH; l++) {
                                    acc += in[l * IW * IH + iy * IW + ix] *
                                        wt[ j + n * OCH + m * KX * OCH + l * OCH * KX * KY];
                                }
                            }
                        }
                    }
                }
                if (sizeof(OUT_T) == sizeof(q15_t)) {
                    *out++ = (OUT_T)BECO_SSAT((acc >> out_shift), 16);
                } else {
                    *out++ = (OUT_T)BECO_SSAT((acc >> out_shift), 32);
                }
            }

        }
    }
    return BECO_SUCCESS;
}

template <
    typename OUT_T,
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY,
    int DC>
int ref_convolve_CHW_q7(const q7_t *in,
                        const q7_t *wt,
                        const q7_t *bias,
                        const uint16_t bias_shift,
                        const uint16_t out_shift,
                        OUT_T *out)
{
    int i, j, k, l, m, n;
    int acc, iy, ix;

    for (j = 0; j < OCH; j++) {
        for (k = 0; k < OH; k++) {
            for (i = 0; i < OW; i++) {

                acc = ((q31_t)bias[j] << bias_shift) + q_round(out_shift);
                for (m = 0; m < KY; m++) {
                    iy = SY * k + m * DC - PY;
                    if (iy >= 0 && iy < IH) {
                        for (n = 0; n < KX; n++) {
                            ix = SX * i + n * DC - PX;
                            if (ix >= 0 && ix < IW) {
                                for (l = 0; l < ICH; l++) {
                                    acc += in[l * IW * IH + iy * IW + ix] *
                                        wt[ j + n * OCH + m * KX * OCH + l * OCH * KX * KY];
                                }
                            }
                        }
                    }
                }
                if (sizeof(OUT_T) == sizeof(q31_t)) {
                    *out++ = (OUT_T)BECO_SSAT((acc >> out_shift), 32);
                } else if (sizeof(OUT_T) == sizeof(q15_t)) {
                    *out++ = (OUT_T)BECO_SSAT((acc >> out_shift), 16);
                } else {
                    *out++ = (OUT_T)BECO_SSAT((acc >> out_shift), 8);
                }
            }

        }
    }
    return BECO_SUCCESS;
}

// input - CHW, kernel - Cin,H,W,Cout, Output - CHW
template <typename in_t, typename wt_t, typename b_t, typename out_t>
static void ref_convolve_CHW(const in_t *Im_in,
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
                             const b_t *bias,
                             const uint16_t bias_shift,
                             const uint16_t out_shift,
                             out_t  *Im_out,
                             const uint16_t dim_im_out_x,
                             const uint16_t dim_im_out_y,
                             const uint16_t d_coeff)
{
    int i, j, k, l, m, n;
    int acc, iy, ix;

    for (j = 0; j < ch_im_out; j++) {
        for (k = 0; k < dim_im_out_y; k++) {
            for (i = 0; i < dim_im_out_x; i++) {

                acc = ((q31_t)bias[j] << bias_shift) + q_round(out_shift);
                for (m = 0; m < dim_kernel_y; m++) {
                    iy = stride_y * k + m * d_coeff - padding_y;
                    if (iy >= 0 && iy < dim_im_in_y) {
                        for (n = 0; n < dim_kernel_x; n++) {
                            ix = stride_x * i + n * d_coeff - padding_x;
                            if (ix >= 0 && ix < dim_im_in_x) {
                                for (l = 0; l < ch_im_in; l++) {
                                    acc += Im_in[l*dim_im_in_x*dim_im_in_y + iy*dim_im_in_x + ix] *
                                    wt[j + n*ch_im_out + m*dim_kernel_x*ch_im_out + \
                                     l*ch_im_out*dim_kernel_x*dim_kernel_y];
                                }
                            }
                        }
                    }
                }

                const int pixsize = 8 * sizeof(out_t);
                *Im_out++ = (out_t)BECO_SSAT((acc >> out_shift), pixsize);
            }

        }
    }
}

// input - CHW, kernel - Cin,H,W,Cout, Output - CHW
template <typename in_t, typename wt_t, typename b_t, typename out_t>
void ref_convolve_1x1_CHW(const in_t *Im_in,
                          const uint16_t dim_im_x,
                          const uint16_t dim_im_y,
                          const uint16_t ch_im_in,
                          const wt_t *wt,
                          const uint16_t ch_im_out,
                          const b_t *bias,
                          const uint16_t bias_shift,
                          const uint16_t out_shift,
                          out_t *Im_out)
{
    int im_size = dim_im_x*dim_im_y;
    int conv_out;

    for (int j = 0; j < ch_im_out; j++) {
        for (int i = 0; i < im_size; i++) {

            conv_out = ((q31_t)bias[j] << bias_shift) + q_round(out_shift);

            for (int l = 0; l < ch_im_in; l++) {
                conv_out += Im_in[i + l*im_size] * wt[j + l*ch_im_out];
            }

            const int pix_size = 8 * sizeof(out_t);
            Im_out[i + j*im_size] = (out_t)BECO_SSAT((conv_out >> out_shift), pix_size);
        }
    }
}

// input - CHW, kernel - CHW, Output - CHW
template <typename in_t, typename wt_t, typename b_t, typename out_t>
static void ref_depthwise_conv_CHW(const in_t *Im_in,
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
                            const b_t *bias,
                            const uint16_t bias_shift,
                            const uint16_t out_shift,
                            out_t *Im_out,
                            const uint16_t dim_im_out_x,
                            const uint16_t dim_im_out_y)
{
    int i_out_y, i_out_x, i_out_ch;
    int i_ker_y, i_ker_x;

    for (i_out_y = 0; i_out_y < dim_im_out_y; i_out_y++) {
        for (i_out_x = 0; i_out_x < dim_im_out_x; i_out_x++) {
            for (i_out_ch = 0; i_out_ch < ch_im_out; i_out_ch++) {

                int conv_out = ((q31_t)bias[i_out_ch] << bias_shift) + q_round(out_shift);
                for (i_ker_y = 0; i_ker_y < dim_kernel_y; i_ker_y++) {
                    for (i_ker_x = 0; i_ker_x < dim_kernel_x; i_ker_x++) {

                        int in_row = stride_y * i_out_y + i_ker_y - padding_y;
                        int in_col = stride_x * i_out_x + i_ker_x - padding_x;

                        if (in_row >= 0 && in_col >= 0 && \
                            in_row < dim_im_in_y && in_col < dim_im_in_x) {
                            conv_out += Im_in[(i_out_ch*dim_im_in_y+in_row) * dim_im_in_x+in_col]*\
                                        wt[(i_out_ch*dim_kernel_y+i_ker_y) * dim_kernel_x+i_ker_x];
                        }
                    }
                }

                const int pix_size = 8 * sizeof(out_t);
                Im_out[(i_out_ch * dim_im_out_y + i_out_y) * dim_im_out_x + i_out_x] = \
                    (out_t)BECO_SSAT((conv_out >> out_shift), pix_size);

            }
        }
    }
}

#endif
