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
#include "beco_matrix_mult_aTb.hpp"
#include "beco_matrix_functions.h"

//matrix_multiplication: A[M*K] * B[K*N] = C[M*N]
//matrix A is transposed, B is in natural order

//case1: a_rows and b_cols are all the multiple of x (if it is 16bits, x is 4; if it is 8bits, x is 8)
//       it is fastest.
beco_state beco_mat_mult_i16xi16_i32_fastest( const int16_t * mat_a, const int16_t * mat_b, int32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i16xi8_i16_fastest( const int16_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi16_i16_fastest( const int8_t * mat_a, const int16_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i16_fastest( const int8_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i8_fastest( const int8_t * mat_a, const int8_t * mat_b, int8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu16_u32_fastest( const uint16_t * mat_a, const uint16_t * mat_b, uint32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu8_u16_fastest( const uint16_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu16_u16_fastest( const uint8_t * mat_a, const uint16_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u16_fastest( const uint8_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u8_fastest( const uint8_t * mat_a, const uint8_t * mat_b, uint8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_fastest(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}


//case2: a_rows is the multiple of x (if it is 16bits, x is 4; if it is 8bits, x is 8)
beco_state beco_mat_mult_i16xi16_i32_arows_opt( const int16_t * mat_a, const int16_t * mat_b, int32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i16xi8_i16_arows_opt( const int16_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi16_i16_arows_opt( const int8_t * mat_a, const int16_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i16_arows_opt( const int8_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i8_arows_opt( const int8_t * mat_a, const int8_t * mat_b, int8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu16_u32_arows_opt( const uint16_t * mat_a, const uint16_t * mat_b, uint32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu8_u16_arows_opt( const uint16_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu16_u16_arows_opt( const uint8_t * mat_a, const uint16_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u16_arows_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u8_arows_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_arows_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}



//case3: b_cols is the multiple of x (if it is 16bits, x is 4; if it is 8bits, x is 8)
beco_state beco_mat_mult_i16xi16_i32_bcols_opt( const int16_t * mat_a, const int16_t * mat_b, int32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i16xi8_i16_bcols_opt( const int16_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi16_i16_bcols_opt( const int8_t * mat_a, const int16_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i16_bcols_opt( const int8_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i8_bcols_opt( const int8_t * mat_a, const int8_t * mat_b, int8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu16_u32_bcols_opt( const uint16_t * mat_a, const uint16_t * mat_b, uint32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu8_u16_bcols_opt( const uint16_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu16_u16_bcols_opt( const uint8_t * mat_a, const uint16_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u16_bcols_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u8_bcols_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_bcols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}



//case4: c_cols is the multiple of x (if it is 8bits, x is 4; if it is 16bits, x is 2; if it is 32bits, x is 1)
beco_state beco_mat_mult_i16xi16_i32_ccols_opt( const int16_t * mat_a, const int16_t * mat_b, int32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i16xi8_i16_ccols_opt( const int16_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi16_i16_ccols_opt( const int8_t * mat_a, const int16_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i16_ccols_opt( const int8_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i8_ccols_opt( const int8_t * mat_a, const int8_t * mat_b, int8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu16_u32_ccols_opt( const uint16_t * mat_a, const uint16_t * mat_b, uint32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu8_u16_ccols_opt( const uint16_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu16_u16_ccols_opt( const uint8_t * mat_a, const uint16_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u16_ccols_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u8_ccols_opt( const uint8_t * mat_a, const uint8_t * mat_b, uint8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_ccols_opt(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

//case5: basic functions. a_rows and b_cols can be any value
beco_state beco_mat_mult_i16xi16_i32_basic( const int16_t * mat_a, const int16_t * mat_b, int32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i16xi8_i16_basic( const int16_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi16_i16_basic( const int8_t * mat_a, const int16_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i16_basic( const int8_t * mat_a, const int8_t * mat_b, int16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_i8xi8_i8_basic( const int8_t * mat_a, const int8_t * mat_b, int8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu16_u32_basic( const uint16_t * mat_a, const uint16_t * mat_b, uint32_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u16xu8_u16_basic( const uint16_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu16_u16_basic( const uint8_t * mat_a, const uint16_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u16_basic( const uint8_t * mat_a, const uint8_t * mat_b, uint16_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

beco_state beco_mat_mult_u8xu8_u8_basic( const uint8_t * mat_a, const uint8_t * mat_b, uint8_t * mat_c,
                                            int a_rows, int b_cols, int a_cols )
{
    beco_matrix_mult_aTb_basic(mat_a, mat_b, mat_c,
                    a_rows, b_cols, a_cols);
    return BECO_OK;
}

