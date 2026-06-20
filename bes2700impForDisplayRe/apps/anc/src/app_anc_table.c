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
#include "plat_types.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_anc_table.h"

#if defined(XXXX)
#undef XXXX
#endif
#define XXXX    (ANC_INVALID_COEF_INDEX)

static const app_anc_coef_index_cfg_t app_anc_coef_index_cfg[APP_ANC_MODE_QTY] = {
  /*  FF ,  FB , PSAP, SPK ,  TT , SW PSAP, Custom */
    #ifdef VOICE_ASSIST_ADA_IIR
    #ifndef ONLY_FF_IN_IIR
    {XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX},   // APP_ANC_MODE_OFF
    {0x00, XXXX, XXXX, XXXX, 0x00, XXXX, XXXX},   // basic ff
    #ifdef DEBUG_TEST_ADA_IIR
    {0x00, 0x00, XXXX, XXXX, 0x00, XXXX, XXXX},   // basic ff + fb
    #endif
    {0x00, 0x00, XXXX, XXXX, 0x01, XXXX, XXXX},   // adaptive ff + fb
    #else
    {XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX},   // APP_ANC_MODE_OFF
    {0x00, XXXX, XXXX, XXXX, XXXX, XXXX, XXXX},   // basic ff
    #ifdef DEBUG_TEST_ADA_IIR
    {0x00, 0x00, XXXX, XXXX, XXXX, XXXX, XXXX},   // basic ff + fb
    #endif
    {0x01, 0x01, XXXX, XXXX, XXXX, XXXX, XXXX},   // adaptive ff + fb
    #endif
    #else
    {XXXX, XXXX, XXXX, XXXX, XXXX, XXXX, ANC_CUSTOM_MODE_OFF},        // APP_ANC_MODE_OFF
    {0x00, 0x00, XXXX, XXXX, 0x00, XXXX, ANC_CUSTOM_MODE_ANC},        // 1, Production calibration
    {0x01, 0x01, XXXX, XXXX, 0x01, XXXX, ANC_CUSTOM_MODE_FIR_ANC},    // 2, Default Depth
    {XXXX, 0x02, XXXX, XXXX, 0x02, XXXX, ANC_CUSTOM_MODE_ANC},        // 3, Default Moderate
    {XXXX, 0x03, XXXX, XXXX, 0x03, XXXX, ANC_CUSTOM_MODE_ANC},        // 4, Default Mild
    {XXXX, 0x04, XXXX, XXXX, 0x04, XXXX, ANC_CUSTOM_MODE_FIR_TT},     // 5, TRANSPARENCY
    {XXXX, 0x05, XXXX, XXXX, 0x05, XXXX, ANC_CUSTOM_MODE_TT},         // 6, Weak TRANSPARENCY
    {XXXX, 0x06, XXXX, XXXX, 0x06, XXXX, ANC_CUSTOM_MODE_ANC},        // 7, APP_ANC_MODE1
    {XXXX, 0x07, XXXX, XXXX, 0x07, XXXX, ANC_CUSTOM_MODE_ANC},        // 8, APP_ANC_MODE2
    {XXXX, 0x08, XXXX, XXXX, 0x08, XXXX, ANC_CUSTOM_MODE_ANC},        // 9, APP_ANC_MODE3
    {XXXX, 0x09, XXXX, XXXX, 0x09, XXXX, ANC_CUSTOM_MODE_ANC},        // 10, APP_ANC_MODE4
    #endif
};

int32_t app_anc_table_init(void)
{
    // Check whether app_anc_coef_index_cfg is not initialized completely.
    uint32_t index_cfg_num = sizeof(app_anc_coef_index_cfg_t) / sizeof(uint32_t);

    TRACE(0, "[%s] index_cfg_num: %d", __func__, index_cfg_num);

    for (uint32_t mode = 0; mode < APP_ANC_MODE_QTY; mode++) {
        uint32_t valid_index_num = 0;
        uint32_t *index_arry = (uint32_t *)&app_anc_coef_index_cfg[mode];
        for (uint32_t pos = 0; pos <index_cfg_num; pos++) {
            if (*(index_arry + pos) == 0) {
                valid_index_num++;
            }
        }

        ASSERT(valid_index_num < index_cfg_num, "[%s] Found app_anc_coef_index_cfg is not initialized completely!", __func__);
    }

    return 0;
}

int32_t app_anc_table_deinit(void)
{
    return 0;
}

app_anc_coef_index_cfg_t app_anc_table_get_cfg(app_anc_mode_t mode)
{
    ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);

    return app_anc_coef_index_cfg[mode];
}

uint32_t app_anc_table_get_types(app_anc_mode_t mode)
{
    ASSERT(mode < APP_ANC_MODE_QTY, "[%s] mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);

    uint32_t anc_types = 0;
    app_anc_coef_index_cfg_t cfg = app_anc_coef_index_cfg[mode];

    if (cfg.ff != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_FEEDFORWARD;
    }

    if (cfg.fb != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_FEEDBACK;
    }

    if (cfg.tt != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_TALKTHRU;
    }

    if (cfg.psap != ANC_INVALID_COEF_INDEX) {
        anc_types |= PSAP_FEEDFORWARD;
    }

    if (cfg.spk_calib != ANC_INVALID_COEF_INDEX) {
        anc_types |= ANC_SPKCALIB;
    }

    return anc_types;
}

uint32_t app_anc_table_get_custom_mode(app_anc_mode_t mode)
{
    return app_anc_coef_index_cfg[mode].custom;
}
