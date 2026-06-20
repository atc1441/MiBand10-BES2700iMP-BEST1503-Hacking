
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
#include "cmsis_dsp/arm_math.h"

beco_state beco_mat_trans_q15(
        int16_t * pSrc,
        int16_t * pDst,
        uint16_t rows,
        uint16_t cols)

{
    arm_matrix_instance_q15 mat_src;
    arm_matrix_instance_q15 mat_dst;

    arm_mat_init_q15(&mat_src, rows, cols, pSrc);
    arm_mat_init_q15(&mat_dst, cols, rows, pDst);

    arm_status ret = arm_mat_trans_q15(&mat_src, &mat_dst);

    return beco_state(ret);
}

