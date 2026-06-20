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
#include "apps.h"
#include "hal_trace.h"
#include "me_api.h"
#include "bt_if.h"
#include "hfp_api.h"
#include "a2dp_api.h"
#include "spp_api.h"
#include "nvrecord_bt.h"
#include "app_bt.h"
#include "btm_i.h"
#include "app_trace_rx.h"
#include "app_media_player.h"
#include "audio_player_adapter.h"

static void app_pair_handler_func(enum pair_event evt, const btif_event_t *event)
{
    switch(evt) {
    case PAIR_EVENT_NUMERIC_REQ:
        break;
    case PAIR_EVENT_COMPLETE:
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("Pairing ok.");
#endif

#ifndef FPGA
#ifdef MEDIA_PLAYER_SUPPORT
        if (btif_me_get_callback_event_err_code(event) == BTIF_BEC_NO_ERROR) {
        } else {
            audio_player_play_prompt(AUD_ID_BT_PAIRING_FAIL, 0);
        }
#endif
#endif
#if defined(IBRT)
        nv_record_execute_async_flush();
#elif !defined(__BT_ONE_BRING_TWO__)
        nv_record_flash_flush();
#endif
        break;
    default:
        break;
    }
}

static void pair_handler_func(enum pair_event event, void *data)
{
    event_t _event;
    int err_code = 0;
    enum pair_event app_pair_evt = PAIR_EVENT_COMPLETE;
    bt_pair_state_change_cb_t cb = NULL;

    TRACE(1,"!!!pair_handler_func event:%d\n", event);

    switch(event) {
    case PAIRING_OK:
        app_pair_evt = PAIR_EVENT_COMPLETE;
        err_code = 0;

        cb = app_bt_get_pair_state_callback();
        if (cb)
        {
            cb((bt_bdaddr_t *)data, APP_BT_PAIRED);
        }

        break;
    case PAIRING_TIMEOUT:
        break;
    case PAIRING_FAILED:
        err_code = 1;
        app_pair_evt = PAIR_EVENT_COMPLETE;
        break;
    default:
        break;
    }

    _event.errCode = err_code;

    app_pair_handler_func(app_pair_evt, (btif_event_t*)&_event);

    return;
}

int bt_pairing_init(void)
{
    btif_pairing_register_callback(pair_handler_func);
    return 0;
}

