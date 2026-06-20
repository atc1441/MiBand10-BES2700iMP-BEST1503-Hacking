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


#ifndef _BECO_NNFUNCTIONS_H
#define _BECO_NNFUNCTIONS_H

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

/**
 * @brief Q7 fast fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 32!
 *           if not, please use beco_fully_connected_q7.
 */

beco_state beco_fully_connected_q7_fast(const q7_t *pV,
                                        const q7_t *pM,
                                        const uint16_t dim_vec,
                                        const uint16_t num_of_cols,
                                        const uint16_t bias_shift,
                                        const uint16_t out_shift,
                                        const q7_t *bias,
                                        q7_t *pOut);

/**
 * @brief Q7 fast fully-connected layer function, the output type is Q15
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 32!
 *           if not, please use beco_fully_connected_q7_out_q15.
 */

beco_state beco_fully_connected_q7_out_q15_fast(const q7_t *pV,
                                                const q7_t *pM,
                                                const uint16_t dim_vec,
                                                const uint16_t num_of_cols,
                                                const uint16_t bias_shift,
                                                const uint16_t out_shift,
                                                const q7_t *bias,
                                                q15_t *pOut);

/**
 * @brief Q15 fast fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 16!
 *           if not, please use beco_fully_connected_q15.
 */

beco_state beco_fully_connected_q15_fast(const q15_t *pV,
                                        const q15_t *pM,
                                        const uint16_t dim_vec,
                                        const uint16_t num_of_cols,
                                        const uint16_t bias_shift,
                                        const uint16_t out_shift,
                                        const q15_t *bias,
                                        q15_t *pOut);

/**
 * @brief Q15 fast fully-connected layer function, the output type is Q31
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 16!
 *           if not, please use beco_fully_connected_q15_out_q31.
 */

beco_state beco_fully_connected_q15_out_q31_fast(const q15_t *pV,
                                                const q15_t *pM,
                                                const uint16_t dim_vec,
                                                const uint16_t num_of_cols,
                                                const uint16_t bias_shift,
                                                const uint16_t out_shift,
                                                const q15_t *bias,
                                                q31_t *pOut);

/**
 * @brief Mixed Q15-Q7 fast fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 32!
 *           if not, please use beco_fully_connected_mat_q7_vec_q15
 */

beco_state beco_fully_connected_mat_q7_vec_q15_fast(const q15_t *pV,
                                                    const q7_t *pM,
                                                    const uint16_t dim_vec,
                                                    const uint16_t num_of_cols,
                                                    const uint16_t bias_shift,
                                                    const uint16_t out_shift,
                                                    const q7_t *bias,
                                                    q15_t *pOut);

/**
 * @brief Mixed Q15-Q7 fast fully-connected layer function, the output type is Q31
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 * @details  The num_of_cols must be a multiple of 32!
 *           if not, please use beco_fully_connected_mat_q7_vec_q15_out_q31
 */

beco_state beco_fully_connected_mat_q7_vec_q15_out_q31_fast(const q15_t *pV,
                                                            const q7_t *pM,
                                                            const uint16_t dim_vec,
                                                            const uint16_t num_of_cols,
                                                            const uint16_t bias_shift,
                                                            const uint16_t out_shift,
                                                            const q7_t *bias,
                                                            q31_t *pOut);

/**
 * @brief Q7 basic fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_q7(const q7_t *pV,
                                   const q7_t *pM,
                                   const uint16_t dim_vec,
                                   const uint16_t num_of_cols,
                                   const uint16_t bias_shift,
                                   const uint16_t out_shift,
                                   const q7_t *bias,
                                   q7_t *pOut);

/**
 * @brief Q7 basic fully-connected layer function, the output type is Q15
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_q7_out_q15(const q7_t *pV,
                                           const q7_t *pM,
                                           const uint16_t dim_vec,
                                           const uint16_t num_of_cols,
                                           const uint16_t bias_shift,
                                           const uint16_t out_shift,
                                           const q7_t *bias,
                                           q15_t *pOut);

/**
 * @brief Q15 basic fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_q15(const q15_t *pV,
                                    const q15_t *pM,
                                    const uint16_t dim_vec,
                                    const uint16_t num_of_cols,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    const q15_t *bias,
                                    q15_t *pOut);

/**
 * @brief Q15 basic fully-connected layer function, the output type is Q31
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_q15_out_q31(const q15_t *pV,
                                            const q15_t *pM,
                                            const uint16_t dim_vec,
                                            const uint16_t num_of_cols,
                                            const uint16_t bias_shift,
                                            const uint16_t out_shift,
                                            const q15_t *bias,
                                            q31_t *pOut);

/**
 * @brief Mixed Q15-Q7 basic fully-connected layer function
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_mat_q7_vec_q15(const q15_t *pV,
                                               const q7_t *pM,
                                               const uint16_t dim_vec,
                                               const uint16_t num_of_cols,
                                               const uint16_t bias_shift,
                                               const uint16_t out_shift,
                                               const q7_t *bias,
                                               q15_t *pOut);

/**
 * @brief Mixed Q15-Q7 basic fully-connected layer function, the output type is Q31
 * @param[in]       pV          pointer to input vector
 * @param[in]       pM          pointer to matrix weights
 * @param[in]       dim_vec     length of the vector
 * @param[in]       num_of_clos number of cols in weight matrix
 * @param[in]       bias_shift  amount of left-shift for bias
 * @param[in]       out_shift   amount of right-shift for output
 * @param[in]       bias        pointer to bias
 * @param[in,out]   pOut        pointer to output vector
 * @return     The function returns state
 *
 */

beco_state beco_fully_connected_mat_q7_vec_q15_out_q31(const q15_t *pV,
                                                       const q7_t *pM,
                                                       const uint16_t dim_vec,
                                                       const uint16_t num_of_cols,
                                                       const uint16_t bias_shift,
                                                       const uint16_t out_shift,
                                                       const q7_t *bias,
                                                       q31_t *pOut);

/**
 * @brief Q7 depthwise separable convolution function
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_q7(const q7_t *Im_in,
                                      const uint16_t dim_im_in_x,
                                      const uint16_t dim_im_in_y,
                                      const uint16_t ch_im_in,
                                      const q7_t *wt,
                                      const uint16_t ch_im_out,
                                      const uint16_t dim_kernel_x,
                                      const uint16_t dim_kernel_y,
                                      const uint16_t padding_x,
                                      const uint16_t padding_y,
                                      const uint16_t stride_x,
                                      const uint16_t stride_y,
                                      const q7_t *bias,
                                      const uint16_t bias_shift,
                                      const uint16_t out_shift,
                                      q7_t *Im_out,
                                      const uint16_t dim_im_out_x,
                                      const uint16_t dim_im_out_y);

/**
 * @brief Q7 depthwise separable convolution function, the output type is Q15
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_q7_out_q15(const q7_t *Im_in,
                                              const uint16_t dim_im_in_x,
                                              const uint16_t dim_im_in_y,
                                              const uint16_t ch_im_in,
                                              const q7_t *wt,
                                              const uint16_t ch_im_out,
                                              const uint16_t dim_kernel_x,
                                              const uint16_t dim_kernel_y,
                                              const uint16_t padding_x,
                                              const uint16_t padding_y,
                                              const uint16_t stride_x,
                                              const uint16_t stride_y,
                                              const q7_t *bias,
                                              const uint16_t bias_shift,
                                              const uint16_t out_shift,
                                              q15_t *Im_out,
                                              const uint16_t dim_im_out_x,
                                              const uint16_t dim_im_out_y);

/**
 * @brief Mixed Q7-Q15 depthwise separable convolution function, the output type is Q15
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_wt_q7_im_q15(const q15_t *Im_in,
                                                const uint16_t dim_im_in_x,
                                                const uint16_t dim_im_in_y,
                                                const uint16_t ch_im_in,
                                                const q7_t *wt,
                                                const uint16_t ch_im_out,
                                                const uint16_t dim_kernel_x,
                                                const uint16_t dim_kernel_y,
                                                const uint16_t padding_x,
                                                const uint16_t padding_y,
                                                const uint16_t stride_x,
                                                const uint16_t stride_y,
                                                const q15_t *bias,
                                                const uint16_t bias_shift,
                                                const uint16_t out_shift,
                                                q15_t *Im_out,
                                                const uint16_t dim_im_out_x,
                                                const uint16_t dim_im_out_y);

/**
 * @brief Mixed Q7-Q15 depthwise separable convolution function, the output type is Q31
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_wt_q7_im_q15_out_q31(const q15_t *Im_in,
                                                        const uint16_t dim_im_in_x,
                                                        const uint16_t dim_im_in_y,
                                                        const uint16_t ch_im_in,
                                                        const q7_t *wt,
                                                        const uint16_t ch_im_out,
                                                        const uint16_t dim_kernel_x,
                                                        const uint16_t dim_kernel_y,
                                                        const uint16_t padding_x,
                                                        const uint16_t padding_y,
                                                        const uint16_t stride_x,
                                                        const uint16_t stride_y,
                                                        const q15_t *bias,
                                                        const uint16_t bias_shift,
                                                        const uint16_t out_shift,
                                                        q31_t *Im_out,
                                                        const uint16_t dim_im_out_x,
                                                        const uint16_t dim_im_out_y);

/**
 * @brief Q15 depthwise separable convolution function
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_q15(const q15_t *Im_in,
                                       const uint16_t dim_im_in_x,
                                       const uint16_t dim_im_in_y,
                                       const uint16_t ch_im_in,
                                       const q15_t *wt,
                                       const uint16_t ch_im_out,
                                       const uint16_t dim_kernel_x,
                                       const uint16_t dim_kernel_y,
                                       const uint16_t padding_x,
                                       const uint16_t padding_y,
                                       const uint16_t stride_x,
                                       const uint16_t stride_y,
                                       const q15_t *bias,
                                       const uint16_t bias_shift,
                                       const uint16_t out_shift,
                                       q15_t *Im_out,
                                       const uint16_t dim_im_out_x,
                                       const uint16_t dim_im_out_y);

/**
 * @brief Q15 depthwise separable convolution function, the output type is Q31
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y
 * @param[in]       bias          pointer to bias. Format: [C_OUT]
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state
 *
 * @details This function is with some constraints:
 *              1. dim_im_in_x == dim_im_out_x
 *              2. dim_im_out_x % 8 == 0
 *              3. dim_kernel_x == dim_kernel_y == 3
 *              4. padding_x == padding_y == 1
 *              5. stride_x == stride_y == 1
 */

beco_state beco_depthwise_conv_3x3_q15_out_q31(const q15_t *Im_in,
                                               const uint16_t dim_im_in_x,
                                               const uint16_t dim_im_in_y,
                                               const uint16_t ch_im_in,
                                               const q15_t *wt,
                                               const uint16_t ch_im_out,
                                               const uint16_t dim_kernel_x,
                                               const uint16_t dim_kernel_y,
                                               const uint16_t padding_x,
                                               const uint16_t padding_y,
                                               const uint16_t stride_x,
                                               const uint16_t stride_y,
                                               const q15_t *bias,
                                               const uint16_t bias_shift,
                                               const uint16_t out_shift,
                                               q31_t *Im_out,
                                               const uint16_t dim_im_out_x,
                                               const uint16_t dim_im_out_y);


/**
 * @brief Fast Q7 (dilated)convolution function
 * @param[in]          Im_in            pointer to input tensor (Note: Input tensor shape should be
 *                                      aarranged as Cin, inputY,inputX)
 * @param[in]          dim_im_in_x      input tensor dimension x
 * @param[in]          dim_im_in_y      input tensor dimension y
 * @param[in]          ch_im_in         number of input tensor channels
 * @param[in]          wt               pointer to kernel weights (Note：Kernel weight should be
 *                                      arranged as Cin, kernelY, kernelX, Cout)
 * @param[in]          ch_im_out        number of output tensor channels
 * @param[in]          dim_kernel_x     filter kernel size x
 * @param[in]          dim_kernel_y     filter kernel size y
 * @param[in]          padding_x        padding size x
 * @param[in]          padding_y        padding size y
 * @param[in]          stride_x         convolution stride x
 * @param[in]          stride_y         convolution stride y
 * @param[in]          bias             pointer to bias
 * @param[in]          bias_shift       amount of left-shift for bias
 * @param[in]          out_shift        amount of right-shift for output
 * @param[in,out]      Im_out           pointer to output tensor
 * @param[in]          dim_im_out_x     output tensor dimension x
 * @param[in]          dim_im_out_y     output tensor dimension y
 * @param[in]          d_coeff          convolve dilated coefficient
 * @return     The function returns beco status
 *
 * @details  The Im_in must be a multiple of 8.
 *           The dim_im_out_x must be a multiple of 8.
 *
 */

beco_state beco_convolve_q7_fast(const q7_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q7_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q7_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q7_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff);

/**
 * @brief Fast Q7 (dilated)convolution function
 * @param[in]          Im_in            pointer to input tensor (Note: Input tensor shape should be
 *                                      aarranged as Cin, inputY,inputX)
 * @param[in]          dim_im_in_x      input tensor dimension x
 * @param[in]          dim_im_in_y      input tensor dimension y
 * @param[in]          ch_im_in         number of input tensor channels
 * @param[in]          wt               pointer to kernel weights (Note：Kernel weight should be
 *                                      arranged as Cin, kernelY, kernelX, Cout)
 * @param[in]          ch_im_out        number of output tensor channels
 * @param[in]          dim_kernel_x     filter kernel size x
 * @param[in]          dim_kernel_y     filter kernel size y
 * @param[in]          padding_x        padding size x
 * @param[in]          padding_y        padding size y
 * @param[in]          stride_x         convolution stride x
 * @param[in]          stride_y         convolution stride y
 * @param[in]          bias             pointer to bias
 * @param[in]          bias_shift       amount of left-shift for bias
 * @param[in]          out_shift        amount of right-shift for output
 * @param[in,out]      Im_out           pointer to output tensor
 * @param[in]          dim_im_out_x     output tensor dimension x
 * @param[in]          dim_im_out_y     output tensor dimension y
 * @param[in]          d_coeff          convolve dilated coefficient
 * @return     The function returns beco status
 *
 * @details  The Im_in must be a multiple of 8.
 *           The dim_im_out_x must be a multiple of 8.
 *
 */

beco_state beco_convolve_q7_out_q15_fast(const q7_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q7_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q7_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q15_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff);


/**
 * @brief Fast Q15 (dilated)convolution function
 * @param[in]          Im_in            pointer to input tensor (Note: Input tensor shape should be
 *                                      aarranged as Cin, inputY,inputX)
 * @param[in]          dim_im_in_x      input tensor dimension x
 * @param[in]          dim_im_in_y      input tensor dimension y
 * @param[in]          ch_im_in         number of input tensor channels
 * @param[in]          wt               pointer to kernel weights (Note：Kernel weight should be
 *                                      arranged as Cin, kernelY, kernelX, Cout)
 * @param[in]          ch_im_out        number of output tensor channels
 * @param[in]          dim_kernel_x     filter kernel size x
 * @param[in]          dim_kernel_y     filter kernel size y
 * @param[in]          padding_x        padding size x
 * @param[in]          padding_y        padding size y
 * @param[in]          stride_x         convolution stride x
 * @param[in]          stride_y         convolution stride y
 * @param[in]          bias             pointer to bias
 * @param[in]          bias_shift       amount of left-shift for bias
 * @param[in]          out_shift        amount of right-shift for output
 * @param[in,out]      Im_out           pointer to output tensor
 * @param[in]          dim_im_out_x     output tensor dimension x
 * @param[in]          dim_im_out_y     output tensor dimension y
 * @param[in]          d_coeff          convolve dilated coefficient
 * @return     The function returns beco status
 *
 * @details  The Im_in must be a multiple of 4.
 *           The dim_im_out_x must be a multiple of 4.
 *
 */

beco_state beco_convolve_q15_fast(const q15_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q15_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q15_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q15_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff);

/**
 * @brief Fast Q15 (dilated)convolution function
 * @param[in]          Im_in            pointer to input tensor (Note: Input tensor shape should be
 *                                      aarranged as Cin, inputY,inputX)
 * @param[in]          dim_im_in_x      input tensor dimension x
 * @param[in]          dim_im_in_y      input tensor dimension y
 * @param[in]          ch_im_in         number of input tensor channels
 * @param[in]          wt               pointer to kernel weights (Note：Kernel weight should be
 *                                      arranged as Cin, kernelY, kernelX, Cout)
 * @param[in]          ch_im_out        number of output tensor channels
 * @param[in]          dim_kernel_x     filter kernel size x
 * @param[in]          dim_kernel_y     filter kernel size y
 * @param[in]          padding_x        padding size x
 * @param[in]          padding_y        padding size y
 * @param[in]          stride_x         convolution stride x
 * @param[in]          stride_y         convolution stride y
 * @param[in]          bias             pointer to bias
 * @param[in]          bias_shift       amount of left-shift for bias
 * @param[in]          out_shift        amount of right-shift for output
 * @param[in,out]      Im_out           pointer to output tensor
 * @param[in]          dim_im_out_x     output tensor dimension x
 * @param[in]          dim_im_out_y     output tensor dimension y
 * @param[in]          d_coeff          convolve dilated coefficient
 * @return     The function returns beco status
 *
 * @details  The Im_in must be a multiple of 4.
 *           The dim_im_out_x must be a multiple of 4.
 *
 */

beco_state beco_convolve_q15_out_q31_fast(const q15_t *Im_in,
                                    const uint16_t dim_im_in_x,
                                    const uint16_t dim_im_in_y,
                                    const uint16_t ch_im_in,
                                    const q15_t *wt,
                                    const uint16_t ch_im_out,
                                    const uint16_t dim_kernel_x,
                                    const uint16_t dim_kernel_y,
                                    const uint16_t padding_x,
                                    const uint16_t padding_y,
                                    const uint16_t stride_x,
                                    const uint16_t stride_y,
                                    const q15_t *bias,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    q31_t  *Im_out,
                                    const uint16_t dim_im_out_x,
                                    const uint16_t dim_im_out_y,
                                    const uint16_t d_coeff);

/**
 * @brief Q7 fast convolve-1x1 layer function, the output type is Q7
 * @param[in]       Im_in          pointer to input tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_in_x    input tensor dimension x
 * @param[in]       dim_im_in_y    input tensor dimension y
 * @param[in]       ch_im_in       number of input tensor channels
 * @param[in]       wt             pointer to kernel weights, the format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out      number of filters, i.e., output tensor channels
 * @param[in]       bias           pointer to bias
 * @param[in]       bias_shift     amount of left-shift for bias
 * @param[in]       out_shift      amount of right-shift for output
 * @param[in,out]   Im_out         pointer to output tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_out_x   output tensor dimension x
 * @param[in]       dim_im_out_y   output tensor dimension y
 * @return     The function returns state
 *
 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *               the product of ch_im_in_x and ch_im_in_y is multiple of 8
 *               ch_im_out is multiple of 8
 */

beco_state beco_convolve_1x1_q7_fast(const q7_t *Im_in,
                                     const uint16_t dim_im_in_x,
                                     const uint16_t dim_im_in_y,
                                     const uint16_t ch_im_in,
                                     const q7_t *wt,
                                     const uint16_t ch_im_out,
                                     const q7_t *bias,
                                     const uint16_t bias_shift,
                                     const uint16_t out_shift,
                                     q7_t *Im_out,
                                     const uint16_t dim_im_out_x,
                                     const uint16_t dim_im_out_y);

/**
 * @brief Q7 fast convolve-1x1 layer function, the output type is Q15
 * @param[in]       Im_in          pointer to input tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_in_x    input tensor dimension x
 * @param[in]       dim_im_in_y    input tensor dimension y
 * @param[in]       ch_im_in       number of input tensor channels
 * @param[in]       wt             pointer to kernel weights, the format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out      number of filters, i.e., output tensor channels
 * @param[in]       bias           pointer to bias
 * @param[in]       bias_shift     amount of left-shift for bias
 * @param[in]       out_shift      amount of right-shift for output
 * @param[in,out]   Im_out         pointer to output tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_out_x   output tensor dimension x
 * @param[in]       dim_im_out_y   output tensor dimension y
 * @return     The function returns state
 *
 * This function implement convolution with 1x1 kernel size.
 *
 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *               the product of ch_im_in_x and ch_im_in_y is multiple of 8
 *               ch_im_out is multiple of 8
 */

beco_state beco_convolve_1x1_q7_out_q15_fast(const q7_t *Im_in,
                                             const uint16_t dim_im_in_x,
                                             const uint16_t dim_im_in_y,
                                             const uint16_t ch_im_in,
                                             const q7_t *wt,
                                             const uint16_t ch_im_out,
                                             const q7_t *bias,
                                             const uint16_t bias_shift,
                                             const uint16_t out_shift,
                                             q15_t *Im_out,
                                             const uint16_t dim_im_out_x,
                                             const uint16_t dim_im_out_y);

/**
 * @brief Q15 fast convolve-1x1 layer function, the output type is Q15
 * @param[in]       Im_in          pointer to input tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_in_x    input tensor dimension x
 * @param[in]       dim_im_in_y    input tensor dimension y
 * @param[in]       ch_im_in       number of input tensor channels
 * @param[in]       wt             pointer to kernel weights, the format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out      number of filters, i.e., output tensor channels
 * @param[in]       bias           pointer to bias
 * @param[in]       bias_shift     amount of left-shift for bias
 * @param[in]       out_shift      amount of right-shift for output
 * @param[in,out]   Im_out         pointer to output tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_out_x   output tensor dimension x
 * @param[in]       dim_im_out_y   output tensor dimension y
 * @return     The function returns state
 *
 * This function implement convolution with 1x1 kernel size.
 *
 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *               the product of ch_im_in_x and ch_im_in_y is multiple of 4
 *               ch_im_out is multiple of 4
 */

beco_state beco_convolve_1x1_q15_fast(const q15_t *Im_in,
                                      const uint16_t dim_im_in_x,
                                      const uint16_t dim_im_in_y,
                                      const uint16_t ch_im_in,
                                      const q15_t *wt,
                                      const uint16_t ch_im_out,
                                      const q15_t *bias,
                                      const uint16_t bias_shift,
                                      const uint16_t out_shift,
                                      q15_t *Im_out,
                                      const uint16_t dim_im_out_x,
                                      const uint16_t dim_im_out_y);

/**
 * @brief Q15 fast convolve-1x1 layer function, the output type is Q31
 * @param[in]       Im_in          pointer to input tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_in_x    input tensor dimension x
 * @param[in]       dim_im_in_y    input tensor dimension y
 * @param[in]       ch_im_in       number of input tensor channels
 * @param[in]       wt             pointer to kernel weights, the format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out      number of filters, i.e., output tensor channels
 * @param[in]       bias           pointer to bias
 * @param[in]       bias_shift     amount of left-shift for bias
 * @param[in]       out_shift      amount of right-shift for output
 * @param[in,out]   Im_out         pointer to output tensor, the format: [C_IN, H, W]
 * @param[in]       dim_im_out_x   output tensor dimension x
 * @param[in]       dim_im_out_y   output tensor dimension y
 * @return     The function returns state
 *
 * This function implement convolution with 1x1 kernel size.
 *
 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *               the product of ch_im_in_x and ch_im_in_y is multiple of 4
 *               ch_im_out is multiple of 4
 */

beco_state beco_convolve_1x1_q15_out_q31_fast(const q15_t *Im_in,
                                              const uint16_t dim_im_in_x,
                                              const uint16_t dim_im_in_y,
                                              const uint16_t ch_im_in,
                                              const q15_t *wt,
                                              const uint16_t ch_im_out,
                                              const q15_t *bias,
                                              const uint16_t bias_shift,
                                              const uint16_t out_shift,
                                              q31_t *Im_out,
                                              const uint16_t dim_im_out_x,
                                              const uint16_t dim_im_out_y);

/**
 * @brief Fast Q7 version convolve-1xn layer function, the output type is Q7
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y, it is not used
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y, it is not used
 * @param[in]       dilation_x    dilation sizes x
 * @param[in]       dilation_y    dilation sizes y, it is not used
 * @param[in]       bias          pointer to bias
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state

 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *                 -# dim_im_in_x is a multiple of 8
 *                 -# dim_im_out_x is a multiple of 8
 *                 -# ch_im_out is a multiple of 8
 *                 -# Explicit constraints(since it is for 1xn convolution)
 *                 -## dim_im_in_y equals 1
 *                 -## dim_im_out_y equals 1
 *                 -## dim_kernel_y equals 1
 *                 -## padding_x = 0
 *                 -## stride_x = 1
 *                 -## dilation_x = 1
 */

beco_state beco_convolve_1xn_q7_fast(const q7_t *Im_in,
                                     const uint16_t dim_im_in_x,
                                     const uint16_t dim_im_in_y,
                                     const uint16_t ch_im_in,
                                     const q7_t *wt,
                                     const uint16_t ch_im_out,
                                     const uint16_t dim_kernel_x,
                                     const uint16_t dim_kernel_y,
                                     const uint16_t padding_x,
                                     const uint16_t padding_y,
                                     const uint16_t stride_x,
                                     const uint16_t stride_y,
                                     const uint16_t dilation_x,
                                     const uint16_t dilation_y,
                                     const q7_t *bias,
                                     const uint16_t bias_shift,
                                     const uint16_t out_shift,
                                     q7_t *Im_out,
                                     const uint16_t dim_im_out_x,
                                     const uint16_t dim_im_out_y);

/**
 * @brief Fast Q7 version convolve-1xn layer function, the output type is Q15
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y, it is not used
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y, it is not used
 * @param[in]       dilation_x    dilation sizes x
 * @param[in]       dilation_y    dilation sizes y, it is not used
 * @param[in]       bias          pointer to bias
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state

 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *                 -# dim_im_in_x is a multiple of 8
 *                 -# dim_im_out_x is a multiple of 8
 *                 -# ch_im_out is a multiple of 8
 *                 -# Explicit constraints(since it is for 1xn convolution)
 *                 -## dim_im_in_y equals 1
 *                 -## dim_im_out_y equals 1
 *                 -## dim_kernel_y equals 1
 *                 -## padding_x = 0
 *                 -## stride_x = 1
 *                 -## dilation_x = 1
 */

beco_state beco_convolve_1xn_q7_out_q15_fast(const q7_t *Im_in,
                                             const uint16_t dim_im_in_x,
                                             const uint16_t dim_im_in_y,
                                             const uint16_t ch_im_in,
                                             const q7_t *wt,
                                             const uint16_t ch_im_out,
                                             const uint16_t dim_kernel_x,
                                             const uint16_t dim_kernel_y,
                                             const uint16_t padding_x,
                                             const uint16_t padding_y,
                                             const uint16_t stride_x,
                                             const uint16_t stride_y,
                                             const uint16_t dilation_x,
                                             const uint16_t dilation_y,
                                             const q7_t *bias,
                                             const uint16_t bias_shift,
                                             const uint16_t out_shift,
                                             q15_t *Im_out,
                                             const uint16_t dim_im_out_x,
                                             const uint16_t dim_im_out_y);

/**
 * @brief Fast Q15 version convolve-1xn layer function, the output type is Q15
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y, it is not used
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y, it is not used
 * @param[in]       dilation_x    dilation sizes x
 * @param[in]       dilation_y    dilation sizes y, it is not used
 * @param[in]       bias          pointer to bias
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state

 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *                 -# dim_im_in_x is a multiple of 4
 *                 -# dim_im_out_x is a multiple of 4
 *                 -# ch_im_out is a multiple of 4
 *                 -# Explicit constraints(since it is for 1xn convolution)
 *                 -## dim_im_in_y equals 1
 *                 -## dim_im_out_y equals 1
 *                 -## dim_kernel_y equals 1
 *                 -## padding_x = 0
 *                 -## stride_x = 1
 *                 -## dilation_x = 1
 */

beco_state beco_convolve_1xn_q15_fast(const q15_t *Im_in,
                                      const uint16_t dim_im_in_x,
                                      const uint16_t dim_im_in_y,
                                      const uint16_t ch_im_in,
                                      const q15_t *wt,
                                      const uint16_t ch_im_out,
                                      const uint16_t dim_kernel_x,
                                      const uint16_t dim_kernel_y,
                                      const uint16_t padding_x,
                                      const uint16_t padding_y,
                                      const uint16_t stride_x,
                                      const uint16_t stride_y,
                                      const uint16_t dilation_x,
                                      const uint16_t dilation_y,
                                      const q15_t *bias,
                                      const uint16_t bias_shift,
                                      const uint16_t out_shift,
                                      q15_t *Im_out,
                                      const uint16_t dim_im_out_x,
                                      const uint16_t dim_im_out_y);

/**
 * @brief Fast Q15 version convolve-1xn layer function, the output type is Q31
 * @param[in]       Im_in         pointer to input tensor. Format: [C_IN, H, W]
 * @param[in]       dim_im_in_x   input tensor dimension x
 * @param[in]       dim_im_in_y   input tensor dimension y
 * @param[in]       ch_im_in      number of input tensor channels
 * @param[in]       wt            pointer to kernel weights. Format: [C_IN, H, W, C_OUT]
 * @param[in]       ch_im_out     number of filters, i.e., output tensor channels
 * @param[in]       dim_kernel_x  filter kernel size x
 * @param[in]       dim_kernel_y  filter kernel size y
 * @param[in]       padding_x     padding sizes x
 * @param[in]       padding_y     padding sizes y, it is not used
 * @param[in]       stride_x      convolution stride x
 * @param[in]       stride_y      convolution stride y, it is not used
 * @param[in]       dilation_x    dilation sizes x
 * @param[in]       dilation_y    dilation sizes y, it is not used
 * @param[in]       bias          pointer to bias
 * @param[in]       bias_shift    amount of left-shift for bias
 * @param[in]       out_shift     amount of right-shift for output
 * @param[in,out]   Im_out        pointer to output tensor. Format: [C_OUT, H, W]
 * @param[in]       dim_im_out_x  output tensor dimension x
 * @param[in]       dim_im_out_y  output tensor dimension y
 * @return     The function returns state

 * @details    This function is the version with full list of optimization tricks, but with
 *             some contraints:
 *                 -# dim_im_in_x is a multiple of 4
 *                 -# dim_im_out_x is a multiple of 4
 *                 -# ch_im_out is a multiple of 4
 *                 -# Explicit constraints(since it is for 1xn convolution)
 *                 -## dim_im_in_y equals 1
 *                 -## dim_im_out_y equals 1
 *                 -## dim_kernel_y equals 1
 *                 -## padding_x = 0
 *                 -## stride_x = 1
 *                 -## dilation_x = 1
 */

beco_state beco_convolve_1xn_q15_out_q31_fast(const q15_t *Im_in,
                                              const uint16_t dim_im_in_x,
                                              const uint16_t dim_im_in_y,
                                              const uint16_t ch_im_in,
                                              const q15_t *wt,
                                              const uint16_t ch_im_out,
                                              const uint16_t dim_kernel_x,
                                              const uint16_t dim_kernel_y,
                                              const uint16_t padding_x,
                                              const uint16_t padding_y,
                                              const uint16_t stride_x,
                                              const uint16_t stride_y,
                                              const uint16_t dilation_x,
                                              const uint16_t dilation_y,
                                              const q15_t *bias,
                                              const uint16_t bias_shift,
                                              const uint16_t out_shift,
                                              q31_t *Im_out,
                                              const uint16_t dim_im_out_x,
                                              const uint16_t dim_im_out_y);

/**
 * @defgroup Perform pooling functions, including average pooling.
 *
 *   Average pooling uses Beco's advantage of parallel computation.
 *   We recommend user to choose beco average pooling in all cases.
 *
 *   Max pooling dose not involve addition and multiplication, and can not use the beco.
 *   For max pooling, in m33, naive method is a good choice. Cmsis is the best in m55.
 *
 */


/**
 * @brief Average pooling function for Q7(int8) data.
 *
 *
 * @details   For the purpose of taking full use of the beco coprocessor, the channel should be the
 *            multiple of 8.
 *            If the Height/WIDTH is odd, beco will padding 1 row/column in top/left automatically.
 *            Other padding case is not considered here.
 *            Note that the format is HWC, user can transform CHW to HWC format
 *            by API beco_imgCHW2HWC_q7;
 *
 * @param[in]      src          Pointer to the Input. Format is HWC. Data type: Q7
 * @param[in]      dim_in_h     Image's height. No further limitation.
 * @param[in]      dim_in_w     Image's width. No further limitation.
 * @param[in]      dim_in_c     Image's channel. It must be the multiple of 8.
 * @param[out]     dest         Pointer to the Output. Format is HWC. Data type: Q7
 *
 * @return                      None
 *
 */
void beco_average_pooling_2x2_HWC_q7(const q7_t* src,
                                     const int dim_in_h,
                                     const int dim_in_w,
                                     const int dim_in_c,
                                     q7_t *dest);

/**
 * @brief Average pooling function for Q15(int16) data.
 *
 * @details   For the purpose of taking full use of the beco coprocessor, the channel should be the
 *            multiple of 4.
 *            If the Height/WIDTH is odd, beco will padding 1 row/column in top/left automatically.
 *            Other padding case is not considered here.
 *            Note that the format is HWC, user can transform CHW to HWC format
 *            by API beco_imgCHW2HWC_q15;
 *
 * @param[in]      src          Pointer to the Input. Format is HWC. Data type: Q15
 * @param[in]      dim_in_h     Image's height. No further limitation.
 * @param[in]      dim_in_w     Image's width. No further limitation.
 * @param[in]      dim_in_c     Image's channel. It must be the multiple of 4.
 * @param[out]     dest         Pointer to the Output. Format is HWC. Data type: Q15
 *
 * @return                      None
 *
 */
void beco_average_pooling_2x2_HWC_q15(const q15_t* src,
                                      const int dim_in_h,
                                      const int dim_in_w,
                                      const int dim_in_c,
                                      q15_t *dest);


/**
 * @brief Transfom CHW format to HWC format for q7 data
 *
 * @param[in]      src          Pointer to the Input. Format is CHW.
 * @param[in]      dim_in_h     Image's height. No further limitation.
 * @param[in]      dim_in_w     Image's width. No further limitation.
 * @param[in]      dim_in_c     Image's channel. No further limitation.
 * @param[out]     src_hwc      Pointer to the transformed src. Format is HWC.
 *
 * @return                      None
 *
 */
void beco_imgCHW2HWC_q7(const q7_t *src,
                        const int dim_in_h,
                        const int dim_in_w,
                        const int dim_in_c,
                        q7_t *src_hwc);

/**
 * @brief Transfom CHW format to HWC format for q15 data
 *
 * @param[in]      src          Pointer to the Input. Format is CHW.
 * @param[in]      dim_in_h     Image's height. No further limitation.
 * @param[in]      dim_in_w     Image's width. No further limitation.
 * @param[in]      dim_in_c     Image's channel. No further limitation.
 * @param[out]     src_hwc      Pointer to the transformed src. Format is HWC.
 *
 * @return                      None
 */
void beco_imgCHW2HWC_q15(const q15_t *src,
                         const int dim_in_h,
                         const int dim_in_w,
                         const int dim_in_c,
                         q15_t *src_hwc);

#endif
