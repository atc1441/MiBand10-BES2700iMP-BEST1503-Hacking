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
#ifdef AOB_MOBILE_ENABLED
#ifdef BLE_USB_AUDIO_SUPPORT
#include "bluetooth_bt_api.h"
#include "app_bt_func.h"
#include "ble_audio_dbg.h"
#include "ble_audio_core.h"
#include "ble_audio_mobile_info.h"
#include "app_ble_usb_stream_stm.h"
#include "app_ble_usb_audio.h"
#include "aob_media_api.h"
#include "list.h"

#define INVALID_LID         (0xFF)

static const AOB_BAP_CIG_PARAM_T cig_param =
{
    .c2p_bn = 1, .c2p_nse = 2, .c2p_ft = 1,
    .p2c_bn = 1, .p2c_nse = 2, .p2c_ft = 1,
    .frame_cnt_per_sdu = 1,

#ifdef AOB_LOW_LATENCY_MODE
    .iso_interval_1_25ms = 4,  // 5ms = 4 * 1.25ms
#else
#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    .iso_interval_1_25ms = 6,  // 7.5ms = 6 * 1.25ms
#else
    .iso_interval_1_25ms = 8,  // 5ms = 8 * 1.25ms
#endif
#endif
};

static usb_stream_stm_t ble_usb_stream_stm;

static const char* usb_stream_sm_event_to_string(int event)
{
    #define CASES(s) case s:return "["#s"]";

    switch(event)
    {
        CASES(USB_REQ_STREAM_PLAYBACK_START)
        CASES(USB_REQ_STREAM_CAPTURE_START)
        CASES(USB_REQ_STREAM_PLAYBACK_STOP)
        CASES(USB_REQ_STREAM_CAPTURE_STOP)
        CASES(EVT_PLAYBACK_STREAM_STARTED)
        CASES(EVT_CAPTURE_STREAM_STARTED)
        CASES(START_EVT)
        CASES(ENTRY_EVT)
        CASES(EXIT_EVT)
    }
    return "[UNDEFINE EVENT]";
}

POSSIBLY_UNUSED static void ble_audio_capture_start_handler(uint8_t con_lid)
{
    LOG_I("%s", __func__);

    // Default cis count in cig
    bes_ble_bap_ascc_set_cis_count_in_cig(bes_ble_gap_connection_count());

#ifdef AOB_LOW_LATENCY_MODE
    bes_ble_bap_ascc_set_sdu_interval(5000, 5000);
#endif /* AOB_LOW_LATENCY_MODE */

#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    bes_ble_bap_ascc_set_sdu_interval(7500, 7500);
#endif

    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start[] =
    {
        {APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 40, APP_GAF_DIRECTION_SRC, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_GAME},
    };
    aob_media_mobile_start_stream(ase_to_start, con_lid, false);
}

static void ble_audio_playback_start_handler(uint8_t con_lid)
{
    LOG_I("%s", __func__);

    // Default cis count in cig
    bes_ble_bap_ascc_set_cis_count_in_cig(bes_ble_gap_connection_count());

#ifdef AOB_LOW_LATENCY_MODE
    bes_ble_bap_ascc_set_sdu_interval(5000, 5000);
#endif /* AOB_LOW_LATENCY_MODE */

#ifdef BLE_AUDIO_FRAME_DUR_7_5MS
    bes_ble_bap_ascc_set_sdu_interval(7500, 7500);
#endif

    AOB_MEDIA_ASE_CFG_INFO_T ase_to_start[] =
    {
        {APP_GAF_BAP_SAMPLING_FREQ_48000HZ, 40, APP_GAF_DIRECTION_SINK, AOB_CODEC_ID_LC3,  APP_GAF_BAP_CONTEXT_TYPE_GAME},
    };
    aob_media_mobile_start_stream(ase_to_start, con_lid, false);
}

void ble_usb_stream_resume_ble_audio(uint8_t con_lid)
{
    BLE_USB_STREAM_STATE_E stream_state = ble_usb_stream_stm_get_cur_state();
    LOG_I("%s stream_state=%d", __func__,stream_state);
    ble_audio_capture_start_handler(con_lid);
    ble_audio_playback_start_handler(con_lid);
}

Msg const* ble_usb_stream_sm_super_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: SUPER state on event: %s", usb_stream_sm_event_to_string(msg->evt));
    switch (msg->evt)
    {
        case START_EVT:
            STATE_START(me, &me->idle);
            return 0;
        case ENTRY_EVT:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_idle_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: IDLE State on event: %s", usb_stream_sm_event_to_string(msg->evt));
    uint32_t i = 0;
    uint8_t ase_lid = INVALID_LID;

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
        {
            for (i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++)
            {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED, APP_GAF_DIRECTION_SINK);
                if (ase_lid != INVALID_LID)
                {
                    if (ble_audio_have_ase_sm_by_ase_lid(ase_lid))
                    {
                        aob_media_mobile_enable_stream(ase_lid);
                    }
                }
                else
                {
                    LOG_I("USB-STREAM link(%d) ase is not ready", i);
                }
            }
            STATE_TRAN(me, &me->playback_start);
        }
        return 0;
        case USB_REQ_STREAM_CAPTURE_START:
        {
            for (i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED, APP_GAF_DIRECTION_SRC);
                if (ase_lid != INVALID_LID) {
                    if (ble_audio_have_ase_sm_by_ase_lid(ase_lid))
                    {
                        aob_media_mobile_enable_stream(ase_lid);
                    }
                }
            }
            STATE_TRAN(me, &me->capture_start);
        }
        return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_playback_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: playback state on event: %s", usb_stream_sm_event_to_string(msg->evt));
    uint8_t ase_lid = INVALID_LID;

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case EVT_PLAYBACK_STREAM_STARTED:
            gap_mobile_start_usb_audio_receiving_dma();
            return 0;
        case USB_REQ_STREAM_PLAYBACK_STOP:
            for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
                if (ase_lid != INVALID_LID) {
                    aob_media_mobile_disable_stream(ase_lid);
                }
            }
            STATE_TRAN(me, &me->idle);
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
            return 0;
        case USB_REQ_STREAM_CAPTURE_START:
            {
                for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                    ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED, APP_GAF_DIRECTION_SRC);
                    if (ase_lid != INVALID_LID) {
                        if (ble_audio_have_ase_sm_by_ase_lid(ase_lid)) {
                            aob_media_mobile_enable_stream(ase_lid);
                        }
                    }
                }
            }

            STATE_TRAN(me, &me->both_start);
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_capture_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: CAPTURE state on event: %s", usb_stream_sm_event_to_string(msg->evt));
    uint8_t ase_lid = INVALID_LID;

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case EVT_CAPTURE_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_transmission_dma();
        }
            return 0;
        case USB_REQ_STREAM_CAPTURE_STOP:
        {
            for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SRC);
                if (ase_lid != INVALID_LID) {
                    aob_media_mobile_disable_stream(ase_lid);
                }
            }
            STATE_TRAN(me, &me->idle);
        }
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
        {
            for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED, APP_GAF_DIRECTION_SINK);
                if (ase_lid != INVALID_LID) {
                    if (ble_audio_have_ase_sm_by_ase_lid(ase_lid)) {
                        aob_media_mobile_enable_stream(ase_lid);
                    }
                }
            }

            STATE_TRAN(me, &me->both_start);
        }
        return 0;
        case USB_REQ_STREAM_CAPTURE_START:
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

Msg const* ble_usb_stream_sm_both_start_state(usb_stream_stm_t *me, Msg *msg)
{
    LOG_I("USB-STREAM: BOTH stream state on event: %s", usb_stream_sm_event_to_string(msg->evt));
    uint8_t ase_lid = INVALID_LID;

    switch (msg->evt)
    {
        case START_EVT:
            return 0;
        case ENTRY_EVT:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_START:
        case USB_REQ_STREAM_CAPTURE_START:
            return 0;
        case USB_REQ_STREAM_PLAYBACK_STOP:
        {
            for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SINK);
                if (ase_lid != INVALID_LID) {
                    aob_media_mobile_disable_stream(ase_lid);
                }
            }
            STATE_TRAN(me, &me->capture_start);
        }
            return 0;
        case USB_REQ_STREAM_CAPTURE_STOP:
        {
            for (uint32_t i = 0; i < BLE_AUDIO_CONNECTION_CNT; i++) {
                ase_lid = app_bap_uc_cli_get_ase_lid(i, APP_GAF_BAP_UC_ASE_STATE_STREAMING, APP_GAF_DIRECTION_SRC);
                if (ase_lid != INVALID_LID) {
                    aob_media_mobile_disable_stream(ase_lid);
                }
            }
            STATE_TRAN(me, &me->playback_start);
        }
            return 0;
        case EVT_CAPTURE_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_transmission_dma();
        }
            return 0;
        case EVT_PLAYBACK_STREAM_STARTED:
        {
            gap_mobile_start_usb_audio_receiving_dma();
        }
            return 0;
        case EXIT_EVT:
            return 0;
    }

    return msg;
}

int ble_usb_stream_sm_on_event(uint32_t stream_stm_ptr,uint32_t event,uint32_t param0,uint32_t param1)
{
    Msg message;
    usb_stream_stm_t *p_stream_sm = NULL;

    message.evt = event;
    message.param0 = param0;
    message.param1 = param1;
    message.param2 = 0;

    p_stream_sm = (usb_stream_stm_t*)stream_stm_ptr;
    HsmOnEvent((Hsm *)p_stream_sm, &message);

    return 0;
}

void ble_usb_stream_send_message(usb_stream_stm_t *p_stream_sm, BLE_USB_STREAM_EVENT_E event, uint32_t param0, uint32_t param1)
{
    app_bt_call_func_in_bt_thread((uint32_t)p_stream_sm,(uint32_t)event,param0,param1,(uint32_t)ble_usb_stream_sm_on_event);
}

usb_stream_stm_t* ble_usb_get_stream_sm(void)
{
    usb_stream_stm_t* iter = &ble_usb_stream_stm;

    return iter;

}

BLE_USB_STREAM_STATE_E ble_usb_stream_stm_get_cur_state(void)
{
    usb_stream_stm_t *p_stream_sm = &ble_usb_stream_stm;

    BLE_USB_STREAM_STATE_E state = BLE_USB_STREAM_STATE_UNKNOW;
    if (!p_stream_sm->used) {
        return state;
    }

    if (p_stream_sm->super.curr == &p_stream_sm->idle) {
        state = BLE_USB_STREAM_STATE_IDLE;
    } else if (p_stream_sm->super.curr == &p_stream_sm->playback_start) {
        state = BLE_USB_STREAM_STATE_PLAYBACK_STARTED;
    } else if (p_stream_sm->super.curr == &p_stream_sm->capture_start) {
        state = BLE_USB_STREAM_STATE_CAPTURE_STARTED;
    } else if (p_stream_sm->super.curr == &p_stream_sm->both_start) {
        state = BLE_USB_STREAM_STATE_BOTH_STARTED;
    }

    return state;
}

void ble_usb_stream_stm_startup(usb_stream_stm_t *ble_usb_stream_sm)
{
    LOG_I("LE-USB-STREAM: %s", __func__);
    ble_usb_stream_stm.used = true;

    HsmCtor((Hsm *)ble_usb_stream_sm, "stream_super", (EvtHndlr)ble_usb_stream_sm_super_state);
    AddState(&ble_usb_stream_sm->idle, "idle",  &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_idle_state);
    AddState(&ble_usb_stream_sm->playback_start, "playback_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_playback_start_state);
    AddState(&ble_usb_stream_sm->capture_start, "capture_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_capture_start_state);
    AddState(&ble_usb_stream_sm->both_start, "both_start", &((Hsm *)ble_usb_stream_sm)->top, (EvtHndlr)ble_usb_stream_sm_both_start_state);

    HsmOnStart((Hsm *)ble_usb_stream_sm);
}

void ble_usb_stream_stm_init(void)
{
    memset(&ble_usb_stream_stm, 0xFF, sizeof(usb_stream_stm_t));
    ble_usb_stream_stm.used = false;
    bes_ble_bap_ascc_prepare_cig_param(&cig_param);
}

#endif
#endif //AOB_MOBILE_ENABLED

