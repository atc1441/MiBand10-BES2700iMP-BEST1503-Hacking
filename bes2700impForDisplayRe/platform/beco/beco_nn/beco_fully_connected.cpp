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

/**
 * @file beco_fully_connected_fast.cpp
 * @brief beco_fully_connected_fast source file for beco_nn lib
 * @version V.2.0
 * @date 2022-08-26
 *
 * @example
 *
 *                 [b0 ,b1 ,b2 ,b3 ]
 * [a0,a1,a2,a3] * [b4 ,b5 ,b6 ,b7 ] = [a0*b0+a1*b4+a2*b8+a3*b12,...]
 *                 [b8 ,b9 ,b10,b11]
 *                 [b12,b13,b14,b15]
 */

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

#include "beco_fully_connected.hpp"
#include "beco_nnfunctions.h"

beco_state beco_fully_connected_q7(const q7_t *pV,
                                   const q7_t *pM,
                                   const uint16_t dim_vec,
                                   const uint16_t num_of_cols,
                                   const uint16_t bias_shift,
                                   const uint16_t out_shift,
                                   const q7_t *bias,
                                   q7_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}

beco_state beco_fully_connected_q7_out_q15(const q7_t *pV,
                                           const q7_t *pM,
                                           const uint16_t dim_vec,
                                           const uint16_t num_of_cols,
                                           const uint16_t bias_shift,
                                           const uint16_t out_shift,
                                           const q7_t *bias,
                                           q15_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}

beco_state beco_fully_connected_q15(const q15_t *pV,
                                    const q15_t *pM,
                                    const uint16_t dim_vec,
                                    const uint16_t num_of_cols,
                                    const uint16_t bias_shift,
                                    const uint16_t out_shift,
                                    const q15_t *bias,
                                    q15_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}

beco_state beco_fully_connected_q15_out_q31(const q15_t *pV,
                                            const q15_t *pM,
                                            const uint16_t dim_vec,
                                            const uint16_t num_of_cols,
                                            const uint16_t bias_shift,
                                            const uint16_t out_shift,
                                            const q15_t *bias,
                                            q31_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}

beco_state beco_fully_connected_mat_q7_vec_q15(const q15_t *pV,
                                               const q7_t *pM,
                                               const uint16_t dim_vec,
                                               const uint16_t num_of_cols,
                                               const uint16_t bias_shift,
                                               const uint16_t out_shift,
                                               const q7_t *bias,
                                               q15_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}

beco_state beco_fully_connected_mat_q7_vec_q15_out_q31(const q15_t *pV,
                                                       const q7_t *pM,
                                                       const uint16_t dim_vec,
                                                       const uint16_t num_of_cols,
                                                       const uint16_t bias_shift,
                                                       const uint16_t out_shift,
                                                       const q7_t *bias,
                                                       q31_t *pOut)
{
    beco_fully_connected(pV, pM, dim_vec, num_of_cols, bias_shift, out_shift, bias, pOut);
    return BECO_OK;
}
