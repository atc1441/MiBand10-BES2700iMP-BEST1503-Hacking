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
#include <string.h>
#include "app_anc.h"
#include "app_anc_sync.h"
#include "hal_trace.h"

#ifdef IBRT
#include "app_tws_ibrt.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ctrl_thread.h"
#endif

int32_t app_anc_sync_init(void)
{
    return 0;
}

int32_t app_anc_sync_send(uint8_t *buf, uint32_t len)
{
#if defined(IBRT)
    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_ANC_STATUS, buf, len);
#endif

    return 0;
}

int32_t app_anc_sync_mode(uint32_t mode)
{
    uint8_t buf[8];

    *((uint32_t *)&buf[0]) = APP_ANC_SYNC_MODE;
    *((uint32_t *)&buf[4]) = mode;

    app_anc_sync_send(buf, sizeof(buf));

    return 0;
}

int32_t app_anc_sync_psap_bands_gain(float gain_l, float gain_r, uint32_t index_start, uint32_t index_end)
{
    uint8_t buf[20];

    *((uint32_t *)&buf[0]) = APP_ANC_SYNC_PSAP_BANDS_GAIN;
    *((float *)&buf[4]) = gain_l;
    *((float *)&buf[8]) = gain_r;
    *((uint32_t *)&buf[12]) = index_start;
    *((uint32_t *)&buf[16]) = index_end;

    app_anc_sync_send(buf, sizeof(buf));

    return 0;
}

int32_t app_anc_sync_psap_bands_gain_array(float *gain)
{
    uint8_t buf[4 + 17 * 4];

    *((uint32_t *)&buf[0]) = APP_ANC_SYNC_PSAP_BANDS_GAIN_ARRAY;
    memcpy(buf + sizeof(uint32_t), gain, 17 * sizeof(float));

    app_anc_sync_send(buf, sizeof(buf));

    return 0;
}

#if defined(PSAP_APP)
extern void psap_set_bands_same_gain_f32(float gain_l, float gain_r, uint32_t index_start, uint32_t index_end);
extern void psap_set_bands_gain_f32(float *gain_l, float *gain_r);
#endif
int32_t app_anc_tws_sync_change(uint8_t *buf, uint32_t len)
{
    app_anc_sync_t sync_type = *((uint32_t *)buf);
    buf += sizeof(uint32_t);

    TRACE(0, "[%s] sync_type: %d, len: %d", __func__, sync_type, len);

    switch (sync_type) {
        case APP_ANC_SYNC_MODE: {
            uint32_t mode = *((uint32_t *)buf);

            TRACE(0, "[%s] sync mode: %d", __func__, mode);
            app_anc_switch_locally(mode);
            break;
        }
#if defined(PSAP_APP)
        case APP_ANC_SYNC_PSAP_BANDS_GAIN: {
            float gain_l;
            float gain_r;
            uint32_t index_start;
            uint32_t index_end;

            gain_l = *((float *)buf);
            buf += sizeof(float);
            gain_r = *((float *)buf);
            buf += sizeof(float);
            index_start = *((uint32_t *)buf);
            buf += sizeof(uint32_t);
            index_end = *((uint32_t *)buf);
            buf += sizeof(uint32_t);

            psap_set_bands_same_gain_f32(gain_l, gain_r, index_start, index_end);
            break;
        }
        case APP_ANC_SYNC_PSAP_BANDS_GAIN_ARRAY: {
            ASSERT(len == 4 + 17 * 4, "[%s] Invalid len: %d", __func__, len);
            psap_set_bands_gain_f32((float *)buf, NULL);
            break;
        }
#endif
        default:
            break;
    }

    return 0;
}
