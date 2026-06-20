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
#include "hal_trace.h"
#include "audio_process_sync.h"
#include "audio_process_vol.h"

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_customif_cmd.h"
#include "app_tws_ctrl_thread.h"
#endif

int32_t audio_process_tws_sync_change(uint8_t *buf, uint32_t len)
{
    audio_process_sync_t sync_type = *((uint32_t *)buf);
    buf += sizeof(uint32_t);

    TRACE(0, "[%s] sync_type: %d, len: %d", __func__, sync_type, len);

    switch (sync_type) {
        case AUDIO_PROCESS_SYNC_VOL: {
            float gain = 1.0;
            uint32_t ms = 0;

            gain = *((float *)buf);
            buf += sizeof(float);
            ms = *((uint32_t *)buf);
            buf += sizeof(uint32_t);

            audio_process_vol_start_impl(gain, ms);
            break;
        }
        default:
            break;
    }

    return 0;
}

int32_t audio_process_sync_send(uint8_t *buf, uint32_t len)
{
#if defined(IBRT)
    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_AUDIO_PROCESS, buf, len);
#endif

    return 0;
}