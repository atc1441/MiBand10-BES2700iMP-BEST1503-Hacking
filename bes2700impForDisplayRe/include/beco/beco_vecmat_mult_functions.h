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
#ifndef __BECO_VECMAT_MULT_FUNCTIONS_H__
#define __BECO_VECMAT_MULT_FUNCTIONS_H__

#include "beco_common.h"

BECO_C_DECLARATIONS_START

beco_state beco_vec_q7_mat_q7_out_q15( const int8_t * pV, const int8_t * pM, int16_t * out, int dim_vec, int num_of_column);
beco_state beco_vec_q7_mat_q15_out_q31( const int8_t * pV, const int16_t * pM, int32_t * out,int dim_vec, int num_of_column);
beco_state beco_vec_q15_mat_q7_out_q15( const int16_t * pV, const int8_t * pM, int16_t * out, int dim_vec, int num_of_column);
beco_state beco_vec_q15_mat_q15_out_q31( const int16_t * pV, const int16_t * pM, int32_t * out, int dim_vec, int num_of_column);


BECO_C_DECLARATIONS_END
#endif /*__BECO_VECMAT_MULT_FUNCTIONS_H__ */

