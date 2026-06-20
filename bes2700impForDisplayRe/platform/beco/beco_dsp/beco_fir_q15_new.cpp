/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

#include "beco_dspfunctions.h"
#include "arm_nnsupportfunctions.h"
#include "beco_bias.hpp"
#include "assert.h"

static void beco_fir_q15_readout(beco_vec32_out_t *beco_out)
{
    beco_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    beco_out[0].i16[1] = beco_read_acc(BECO_ACC0, 4).i16[0];
    beco_out[1].i16[0] = beco_read_acc(BECO_ACC0, 8).i16[0];
    beco_out[1].i16[1] = beco_read_acc(BECO_ACC0, 12).i16[0];

    beco_out[2].i16[0] = beco_read_acc(BECO_ACC1, 0).i16[0];
    beco_out[2].i16[1] = beco_read_acc(BECO_ACC1, 4).i16[0];
    beco_out[3].i16[0] = beco_read_acc(BECO_ACC1, 8).i16[0];
    beco_out[3].i16[1] = beco_read_acc(BECO_ACC1, 12).i16[0];

    beco_out[4].i16[0] = beco_read_acc(BECO_ACC2, 0).i16[0];
    beco_out[4].i16[1] = beco_read_acc(BECO_ACC2, 4).i16[0];
    beco_out[5].i16[0] = beco_read_acc(BECO_ACC2, 8).i16[0];
    beco_out[5].i16[1] = beco_read_acc(BECO_ACC2, 12).i16[0];

    beco_out[6].i16[0] = beco_read_acc(BECO_ACC3, 0).i16[0];
    beco_out[6].i16[1] = beco_read_acc(BECO_ACC3, 4).i16[0];
    beco_out[7].i16[0] = beco_read_acc(BECO_ACC3, 8).i16[0];
    beco_out[7].i16[1] = beco_read_acc(BECO_ACC3, 12).i16[0];
}

static void beco_fir_q15_impl(beco_vec64_in_t *beco_in,
                              const int16_t* pCoeffs,
                              const uint16_t numTaps)
{
    beco_write_reg(BECO_REG0, *beco_in++);
    beco_write_reg(BECO_REG1, *beco_in++);
    beco_write_reg(BECO_REG2, *beco_in++);
    beco_write_reg(BECO_REG3, *beco_in++);
    beco_write_reg(BECO_REG4, *beco_in++);
    int j = 1;
    for (int i = 0; i < numTaps; i++, j++) {
        beco_shift_block5(BECO_REG0, 2);
        beco_mmacgr4(((beco_vec32_in_t){.i16 = {*pCoeffs++, 0}}), BECO_REG0);
        if (j == 4) {
            beco_write_reg(BECO_REG4, *beco_in++);
            j = 0;
        }
    }
}

void beco_fir_q15_process_new(const int16_t* pSrc,
                              int16_t *pState,
                              int16_t *pDst,
                              const int16_t* pCoeffs,
                              const uint16_t numTaps,
                              const uint16_t blockSize,
                              const uint16_t shift)
{
    assert(blockSize % 16 == 0 && numTaps % 4 == 0);
    memcpy(&pState[numTaps], pSrc, blockSize*sizeof(int16_t));

    const int beco_cal_nums = blockSize >> 4;
    beco_vec64_in_t *beco_in = (beco_vec64_in_t *)pState;
    beco_vec32_out_t *beco_out = (beco_vec32_out_t *)pDst;
    
    for (int i = 0; i < beco_cal_nums; i++) {
        beco_clear_acc(BECO_ACC0);
        beco_clear_acc(BECO_ACC1);
        beco_clear_acc(BECO_ACC2);
        beco_clear_acc(BECO_ACC3);

        beco_fir_q15_impl(beco_in, pCoeffs, numTaps);
        beco_fir_q15_readout(beco_out);

        beco_in += 4;
        beco_out += 8;
    }

    memcpy(pState, &pState[blockSize], numTaps*sizeof(int16_t));
}

void beco_fir_q15_new(const int16_t* pSrc,
                      int16_t *pState,
                      int16_t *pDst,
                      const int16_t* pCoeffs,
                      const uint16_t numTaps,
                      const uint16_t blockSize,
                      const uint16_t shift)
{
    uint32_t config =
        BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
        BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT16 |
        BECO_CONF_RSHIFT(shift) |
        BECO_CONF_PACK_INT16 | BECO_CONF_RD_16x16;
    beco_write_config(config);

    beco_fir_q15_process_new(pSrc, pState, pDst, pCoeffs, numTaps, blockSize, shift);
}
