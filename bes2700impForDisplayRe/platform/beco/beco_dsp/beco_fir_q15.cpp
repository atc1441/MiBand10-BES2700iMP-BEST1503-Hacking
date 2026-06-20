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
 * @file beco_fir_q15.cpp
 * @brief beco_fir_q15 source file for beco_dsp lib
 * @version V.2.0
 * @date 2022-11-16
 * 
 * 
 */
#include <assert.h>
#include <string.h>
#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"
#include "beco_bias.hpp"
#include "beco_dspfunctions.h"
template <size_t m_elem_sz>
    static void beco_set_preload(int32_t q_bias);
#define BECO_SET_PRELOAD(m, set_preload_fn) \
    template <> inline                 \
    void beco_set_preload<m>(int32_t q_bias) { \
        set_preload_fn(q_bias);                 \
    }
BECO_SET_PRELOAD(2, beco_set_preload_16x16_4ACC)
static inline void beco_fir_q15_config(int scale)
{
    uint32_t config;
    config =
            BECO_CONF_BMODE_REP64 | BECO_CONF_AMODE_REP16 |
            BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT16 |
            BECO_OUTCNF_RSHIFT(scale) |
#if defined(CHIP_BEST1306)
            BECO_CONF_PACK_INT32  | BECO_CONF_RD_16x16;
#else
            BECO_CONF_PACK_INT16  | BECO_CONF_RD_16x16;
#endif
    beco_write_config(config);
}
static inline void beco_read_all_16_16bit(beco_vec32_out_t *out)
{
    out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    out[0].i16[1] = beco_read_acc(BECO_ACC0, 4).i16[0];
    out[1].i16[0] = beco_read_acc(BECO_ACC0, 8).i16[0];
    out[1].i16[1] = beco_read_acc(BECO_ACC0, 12).i16[0];
    out[2].i16[0] = beco_read_acc(BECO_ACC1, 0).i16[0];
    out[2].i16[1] = beco_read_acc(BECO_ACC1, 4).i16[0];
    out[3].i16[0] = beco_read_acc(BECO_ACC1, 8).i16[0];
    out[3].i16[1] = beco_read_acc(BECO_ACC1, 12).i16[0];
    out[4].i16[0] = beco_read_acc(BECO_ACC2, 0).i16[0];
    out[4].i16[1] = beco_read_acc(BECO_ACC2, 4).i16[0];
    out[5].i16[0] = beco_read_acc(BECO_ACC2, 8).i16[0];
    out[5].i16[1] = beco_read_acc(BECO_ACC2, 12).i16[0];
    out[6].i16[0] = beco_read_acc(BECO_ACC3, 0).i16[0];
    out[6].i16[1] = beco_read_acc(BECO_ACC3, 4).i16[0];
    out[7].i16[0] = beco_read_acc(BECO_ACC3, 8).i16[0];
    out[7].i16[1] = beco_read_acc(BECO_ACC3, 12).i16[0];
}
static void beco_fir_q15_impl(const beco_vec64_in_t *beco_pSrc,
                              const q15_t *pCoeffs,
                              const uint16_t numTaps)
{
    beco_write_reg(BECO_REG0, *beco_pSrc++);
    beco_write_reg(BECO_REG1, *beco_pSrc++);
    beco_write_reg(BECO_REG2, *beco_pSrc++);
    beco_write_reg(BECO_REG3, *beco_pSrc++);
    beco_write_reg(BECO_REG4, *beco_pSrc++);
    int k = 0;
    for (int i = numTaps - 1; i >= 0; i--) {
        if (k == 4) {
            beco_write_reg(BECO_REG4, *beco_pSrc++);
            k = 0;
        }
        uint32_t pcoeff_32 = pCoeffs[i];
        beco_mmacgr4(((beco_vec32_in_t){.u32 = pcoeff_32}), BECO_REG0);
        beco_shift_block5(BECO_REG0, sizeof(q15_t));
        k++;
    }
}
static void beco_fir_q15_process(const q15_t *pSrc,
                                 const uint16_t pSrcLen,
                                 const q15_t *pCoeffs,
                                 const uint16_t numTaps,
                                 const uint16_t out_shift,
                                 q15_t *pDst,
                                 const uint16_t pDstLen)
{
     assert(pSrcLen == pDstLen + numTaps - 1);
    const beco_vec64_in_t *beco_pSrc = (const beco_vec64_in_t *) pSrc;
    const unsigned beco_len_4accs = pDstLen / 16;
    const unsigned remainder = pDstLen - beco_len_4accs * 16;
    beco_vec32_out_t *beco_pDst = (beco_vec32_out_t *)pDst;
    int round = q_round(out_shift);
    unsigned j;
    for (j = 0; j < beco_len_4accs; j++) {
        beco_set_preload<2>(round);
        beco_fir_q15_impl(&beco_pSrc[4*j], pCoeffs, numTaps);
        beco_read_all_16_16bit(beco_pDst);
        beco_pDst += 8;
    }
    if (remainder != 0) {
        beco_set_preload<2>(round);
        beco_fir_q15_impl(&beco_pSrc[4*beco_len_4accs], pCoeffs, numTaps);
        beco_vec32_out_t p_out_tmp[8] = {0};
        beco_read_all_16_16bit(p_out_tmp);
        memcpy(beco_pDst, p_out_tmp, remainder * sizeof(int16_t));
    }
}
beco_state beco_fir_q15(const q15_t *pSrc,
                        const uint16_t pSrcLen,
                        const q15_t *pCoeffs,
                        const uint16_t numTaps,
                        const uint16_t out_shift,
                        q15_t *pDst,
                        const uint16_t pDstLen)
{
    beco_fir_q15_config(out_shift);
    beco_fir_q15_process(
        pSrc, pSrcLen, pCoeffs, numTaps, out_shift, pDst, pDstLen);
    return BECO_OK;
}
