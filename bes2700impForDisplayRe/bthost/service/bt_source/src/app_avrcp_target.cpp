/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef BT_AVRCP_SUPPORT
#if defined(BT_AVRCP_TG_ROLE)
#include "bt_source.h"
#include "app_a2dp.h"
#include "app_a2dp_source.h"
#include "app_avrcp_target.h"
#include "hal_trace.h"
#include "app_bt_media_manager.h"
#include "audio_player_adapter.h"

static uint8_t avrcp_target_get_play_status(struct BT_SOURCE_DEVICE_T *curr_device)
{
    if (curr_device->base_device->avrcp_playback_status == 0)
    {
        return a2dp_source_get_play_status(&curr_device->base_device->remote);
    }
    else
    {
        return curr_device->base_device->avrcp_playback_status;
    }
}

void avrcp_target_send_play_status_change_notify(uint8_t device_id)
{
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    static uint8_t play_status = 0xff;
    uint8_t curr_play_status = 0;

    curr_device = app_bt_source_get_device(device_id);

    if (curr_device == NULL)
    {
        return;
    }

    if (curr_device->base_device->avrcp_conn_flag && curr_device->base_device->play_status_notify_registered)
    {
        curr_play_status = avrcp_target_get_play_status(curr_device);

        if (curr_play_status != play_status)
        {
            btif_avrcp_send_play_status_change_actual_rsp(curr_device->base_device->avrcp_channel, curr_play_status);

            play_status = curr_play_status;
        }
    }
}

static void avrcp_event_cmd_handle(uint8_t device_id,
                                    const avrcp_callback_parms_t *params,
                                    struct BT_SOURCE_DEVICE_T *curr_device,
                                    btif_avrcp_channel_t * channel)
{
    avctp_cmd_frame_t *cmd_frame = btif_get_avrcp_cmd_frame(params);
    uint8_t *operands;

    ASSERT(cmd_frame, "get frame wrong");

    operands = cmd_frame->operands;
    if(cmd_frame->ctype == BTIF_AVRCP_CTYPE_STATUS)
    {
        uint32_t company_id = operands[2] +
            (uint32_t(operands[1]) << 8) +
            (uint32_t(operands[0]) << 16);
        TRACE(2,"%s ::AVRCP_EVENT_COMMAND company_id=%x", __func__, company_id);
        if(company_id == 0x001958)  //bt sig
        {
            avrcp_operation_t op = operands[3];
            uint8_t oplen =  operands[6]+ (uint32_t(operands[5]) << 8);
            TRACE(3,"%s ::AVRCP_EVENT_COMMAND op=%x,oplen=%x", __func__, op,oplen);
            switch(op) {
            case BTIF_AVRCP_OP_GET_CAPABILITIES:
                {
                    uint8_t event = operands[7];
                    avrcp_event_mask_t mask;

                    if(event == BTIF_AVRCP_CAPABILITY_COMPANY_ID)
                    {
                        TRACE(1,"%s ::AVRCP_EVENT_COMMAND send support compay id", __func__);
                    }
                    else if(event == BTIF_AVRCP_CAPABILITY_EVENTS_SUPPORTED)
                    {
                        TRACE(2,"%s ::AVRCP_EVENT_COMMAND send support event transId:%d",
                                                            __func__, cmd_frame->transId);

                        btif_avrcp_set_channel_adv_event_mask(channel, BTIF_AVRCP_ENABLE_MEDIA_STATUS_CHANGED|BTIF_AVRCP_ENABLE_VOLUME_CHANGED);

                        mask = btif_get_avrcp_adv_rem_event_mask(channel);
                        btif_avrcp_tg_send_capability_rsp(channel, BTIF_AVRCP_CAPABILITY_EVENTS_SUPPORTED,
                                                            mask, cmd_frame->transId);
                    }
                    else
                    {
                        TRACE(1,"%s ::AVRCP_EVENT_COMMAND send error event value", __func__);
                    }
                }
                break;
            case BTIF_AVRCP_OP_GET_PLAY_STATUS:
                {
                    uint8_t status = avrcp_target_get_play_status(curr_device);
                    //Send fake length and position information, otherwise some cars will not play music
                    uint32_t len_ms = 103000;
                    static uint32_t pos_ms = 4000;
                    TRACE(0, "BTIF_AVRCP_OP_GET_PLAY_STATUS len_ms %d pos_ms %d status %d", len_ms, pos_ms, status);
                    btif_avrcp_send_play_status_rsp(channel, len_ms, pos_ms, status);
                    pos_ms += 1000;
                    if (pos_ms >= len_ms)
                    {
                        pos_ms = 1000;
                    }
                }
                break;
            }
        }
    }
    else if(cmd_frame->ctype == BTIF_AVCTP_CTYPE_CONTROL)
    {
        TRACE(1,"%s ::AVRCP_EVENT_COMMAND AVCTP_CTYPE_CONTROL", __func__);
        DUMP8("%02x ", operands, cmd_frame->operandLen);
        if (operands[3] == BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME)
        {
            TRACE(2,"%s ::AVRCP_EID_VOLUME_CHANGED transId:%d", __func__, cmd_frame->transId);
            //a2dp_volume_set(curr_device->device_id, (operands[7])+1);
            DUMP8("%02x ", operands,  cmd_frame->operandLen);
            btif_avrcp_tg_send_absolute_volume_rsp(channel, operands[7], cmd_frame->transId);
        }
        else if (BTIF_AVRCP_OP_CUSTOM_CMD == operands[3])
        {
            btif_avrcp_tg_accept_custom_cmd(channel, true, cmd_frame->transId);
        }
    }
    else if (cmd_frame->ctype == BTIF_AVCTP_CTYPE_NOTIFY)
    {
        uint8_t vol = 0;
        uint8_t status;
        TRACE(1,"%s ::AVRCP_EVENT_COMMAND AVCTP_CTYPE_NOTIFY", __func__);
        DUMP8("%02x ", operands, cmd_frame->operandLen);
        if (operands[7] == BTIF_AVRCP_EID_VOLUME_CHANGED)
        {
            TRACE(2,"%s ::AVRCP_EID_VOLUME_CHANGED transId:%d", __func__, cmd_frame->transId);
            vol = a2dp_volume_get((enum BT_DEVICE_ID_T)device_id);
            btif_avrcp_tg_send_volume_change_interim_rsp(channel, vol);
        }
        else if (operands[7] == BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED)
        {
            TRACE(2,"%s ::AVRCP_EID_MEDIA_STATUS_CHANGED transId:%d", __func__, cmd_frame->transId);
            curr_device->base_device->play_status_notify_registered = true;
            status = avrcp_target_get_play_status(curr_device);
            btif_avrcp_send_play_status_change_interim_rsp(channel, status);
        }
    }
}

void avrcp_target_callback(uint8_t device_id, btif_avrcp_channel_t *btif_avrcp, const avrcp_callback_parms_t* parms)
{
    btif_avctp_event_t event = btif_avrcp_get_callback_event(parms);
    struct BT_SOURCE_DEVICE_T *curr_device = NULL;
    btif_avrcp_operation_t avrcp_op;
    bt_status_t avrcp_status;
    avrcp_adv_notify_parms_t *avrcp_notify_params = NULL;

    if (device_id == BT_DEVICE_INVALID_ID && event == BTIF_AVRCP_EVENT_DISCONNECT)
    {
        // avrcp profile is closed due to acl created fail
        TRACE(2,"%s ::AVRCP_EVENT_DISCONNECT acl created error=%x", __func__,
                                    btif_get_avrcp_cb_channel_error_code(parms));
        return;
    }

    curr_device = app_bt_source_get_device(device_id);

    ASSERT(device_id >= BT_SOURCE_DEVICE_ID_BASE && device_id < (BT_SOURCE_DEVICE_ID_BASE + BT_SOURCE_DEVICE_NUM) &&
       curr_device->base_device->avrcp_channel == btif_avrcp, "avrcp target device channel must match");

    TRACE(5,"(d%x) %s channel %p event %d%s", device_id, __func__, btif_avrcp, event, btif_avrcp_event_str(event));

    switch (event) {
        case BTIF_AVRCP_EVENT_CONNECT_IND:
        case BTIF_AVRCP_EVENT_CONNECT:
            TRACE(1,"%s ::AVRCP_EVENT_CONNECT", __func__);
            curr_device->base_device->play_status_notify_registered = false;
            curr_device->base_device->avrcp_conn_flag = true;
            curr_device->base_device->avrcp_playback_status = 0;
            app_avrcp_get_capabilities_start(device_id);
            break;
        case BTIF_AVRCP_EVENT_DISCONNECT:
            TRACE(1,"%s ::AVRCP_EVENT_DISCONNECT", __func__);
            curr_device->base_device->avrcp_conn_flag = false;
            break;
        case BTIF_AVRCP_EVENT_PANEL_PRESS:
            switch(btif_get_avrcp_panel_ind(parms)->operation)
            {
            case BTIF_AVRCP_POP_VOLUME_UP:
                TRACE(1,"%s ::AVRCP_KEY_VOLUME_UP", __func__);
                audio_player_volume_up();
                break;
            case BTIF_AVRCP_POP_VOLUME_DOWN:
                TRACE(1,"%s ::AVRCP_KEY_VOLUME_DOWN", __func__);
                audio_player_volume_down();
                break;
            case BTIF_AVRCP_POP_PLAY:
                TRACE(1,"%s ::AVRCP_KEY_PLAY", __func__);
                curr_device->base_device->avrcp_playback_status = 1;
                app_a2dp_source_start_stream(device_id);
                break;
            case BTIF_AVRCP_POP_PAUSE:
                TRACE(1,"%s ::AVRCP_KEY_PAUSE", __func__);
                curr_device->base_device->avrcp_playback_status = 2;
                app_a2dp_source_suspend_stream(device_id);
                break;
            default:
                break;
            }
            break;
        case BTIF_AVRCP_EVENT_PANEL_RELEASE:
            switch(btif_get_avrcp_panel_ind(parms)->operation)
            {
            case BTIF_AVRCP_POP_PLAY:
                TRACE(1,"%s ::AVRCP_KEY_PLAY released", __func__);
                avrcp_target_send_play_status_change_notify(device_id);
                break;
            case BTIF_AVRCP_POP_PAUSE:
                TRACE(1,"%s ::AVRCP_KEY_PAUSE released", __func__);
                avrcp_target_send_play_status_change_notify(device_id);
                break;
            default:
                break;
            }
            break;
        case BTIF_AVRCP_EVENT_COMMAND:
            avrcp_event_cmd_handle(device_id, parms, curr_device, btif_avrcp);
            break;
        case BTIF_AVRCP_EVENT_ADV_RESPONSE:
            avrcp_op = btif_get_avrcp_cb_channel_advOp(parms);
            avrcp_status = btif_get_avrcp_cb_channel_state(parms);
            TRACE(5,"(d%x) %s ::AVRCP_EVENT_ADV_RESPONSE role=%d op=%02x status=%d",
                device_id, __func__, btif_get_avrcp_channel_role(btif_avrcp), avrcp_op, avrcp_status);
            if (avrcp_status != BT_STS_SUCCESS)
                break;

            if(avrcp_op == BTIF_AVRCP_OP_GET_CAPABILITIES)
            {
                avrcp_adv_rsp_parms_t * rsp;
                rsp = btif_get_avrcp_adv_rsp(parms);
                btif_set_avrcp_adv_rem_event_mask(btif_avrcp, rsp->capability.info.eventMask);
                if(btif_get_avrcp_adv_rem_event_mask(btif_avrcp) & BTIF_AVRCP_ENABLE_VOLUME_CHANGED)
                {
                    TRACE(2,"(d%x) %s ::VOLUME_CHANGED_EVENT remote support", device_id, __func__);
                    btif_avrcp_ct_register_volume_change_notification(btif_avrcp, 0);
                }
            }
            else if(avrcp_op == BTIF_AVRCP_OP_REGISTER_NOTIFY)
            {
                avrcp_notify_params = btif_get_avrcp_adv_notify(parms);
                bt_callback_avrcp_register_notify_response_callback(avrcp_notify_params->event);
                if (avrcp_notify_params->event == BTIF_AVRCP_EID_VOLUME_CHANGED)
                {
                    TRACE(2,"(d%x) %s ::VOLUME_NOTIFY RSP volume_changed_status %d",
                                    device_id, __func__, avrcp_notify_params->p.volume);
                    a2dp_volume_set((enum BT_DEVICE_ID_T)device_id, avrcp_notify_params->p.volume);
                }
            }
            break;
        case BTIF_AVRCP_EVENT_ADV_NOTIFY:
            avrcp_notify_params = btif_get_avrcp_adv_notify(parms);
            if(avrcp_notify_params->event == BTIF_AVRCP_EID_VOLUME_CHANGED)
            {
                 TRACE(3,"(d%x) %s ::AVRCP_EVENT_ADV_NOTIFY volume_changed_status %d",
                                    device_id, __func__, avrcp_notify_params->p.volume);
                 btif_avrcp_ct_register_volume_change_notification(btif_avrcp, 0);
            }
            else
            {
                TRACE(3,"(d%x) %s ::AVRCP_EVENT_ADV_NOTIFY event %d", device_id, __func__, avrcp_notify_params->event);
            }
            break;
        default:
            break;
    }
}

void app_avrcp_target_init(void)
{
    int i = 0;
    struct BT_SOURCE_DEVICE_T *device = NULL;

    if (bt_source_manager.config.av_enable)
    {
        btif_avrcp_init(NULL);

        for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            device = app_bt_source_get_device(i);

            device->base_device->avrcp_channel = btif_alloc_avrcp_tg_channel();

            device->base_device->avrcp_conn_flag = 0;

            device->base_device->avrcp_playback_status = 0;

            btif_avrcp_register(device->base_device->avrcp_channel, avrcp_target_callback);
        }
    }
}

#endif /* BT_AVRCP_TG_ROLE */
#endif /* BT_AVRCP_SUPPORT */