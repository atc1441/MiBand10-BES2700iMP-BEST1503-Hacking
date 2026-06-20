/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include "beco.h"
#include "beco_l1.h"
#include "beco_vecmat_mult.hpp"
#include "beco_vecmat_mult_functions.h"

//vector _matrix_multiplication: V[K] * M[K*N] = O[N]
beco_state beco_vec_q7_mat_q7_out_q15( const int8_t * pV, const int8_t * pM, int16_t * out,
                                            int dim_vec, int num_of_column)
{
    beco_vecmat_mult(pV, pM, out, dim_vec, num_of_column);
    return BECO_OK;
}

beco_state beco_vec_q7_mat_q15_out_q31( const int8_t * pV, const int16_t * pM, int32_t * out,
                                            int dim_vec, int num_of_column)
{
    beco_vecmat_mult(pV, pM, out, dim_vec, num_of_column);
    return BECO_OK;
}

beco_state beco_vec_q15_mat_q7_out_q15( const int16_t * pV, const int8_t * pM, int16_t * out,
                                            int dim_vec, int num_of_column)
{
    beco_vecmat_mult(pV, pM, out, dim_vec, num_of_column);
    return BECO_OK;
}

beco_state beco_vec_q15_mat_q15_out_q31( const int16_t * pV, const int16_t * pM, int32_t * out,
                                            int dim_vec, int num_of_column)
{
    beco_vecmat_mult(pV, pM, out, dim_vec, num_of_column);
    return BECO_OK;
}

