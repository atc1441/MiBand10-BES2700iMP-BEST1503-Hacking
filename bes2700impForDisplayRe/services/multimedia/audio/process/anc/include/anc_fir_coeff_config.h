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
#ifndef __ANC_FIR_COEFF_CONFIG_H__
#define __ANC_FIR_COEFF_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ANC_FIR_MAX_ORDER       (472)

typedef enum {
    ANC_FIR_CHANNEL_FF_L = 0,
    ANC_FIR_CHANNEL_FF_R,
    ANC_FIR_CHANNEL_MC_L,
    ANC_FIR_CHANNEL_TT_L,

    ANC_FIR_CHANNEL_QTY
} ANC_FIR_CHANNEL_T;

int32_t anc_fir_coeff_config_init();
int32_t anc_fir_coeff_config_deinit();
int32_t anc_fir_get_len(ANC_FIR_CHANNEL_T ch);
int32_t anc_fir_set_coeff(ANC_FIR_CHANNEL_T ch, const float *coeff, uint32_t num);
int32_t anc_fir_set_coeff_q23(ANC_FIR_CHANNEL_T ch, const int32_t *coeff, uint32_t num);
//Test the fir pathway
int32_t anc_fir_set_gain_test(uint32_t gain, ANC_FIR_CHANNEL_T ch);

#ifdef __cplusplus
}
#endif

#endif
