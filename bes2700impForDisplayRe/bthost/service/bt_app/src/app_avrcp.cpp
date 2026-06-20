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
#ifdef BT_AVRCP_SUPPORT
#undef MOUDLE
#define MOUDLE AVRCP_APP
#include <stdio.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "bluetooth.h"

#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "app_a2dp.h"

#include "avrcp_api.h"
#include "btapp.h"
#include "app_bt.h"

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_tws_besaud.h"
#endif

#include "audio_policy.h"
#include "hal_codec.h"
#include "audio_player_adapter.h"
#include "app_bt_stream.h"
#include "app_audio_active_device_manager.h"
#include "system_utils.h"


typedef enum
{
    SET_ABSOLUTE_VOLUME_OP,
    /* Reserve operation codes */
} app_avrcp_operation_code_t;

typedef struct
{
    app_avrcp_operation_code_t opcode;
    /* Parameters structure determined by different opcodes */
    void *parameters;
} app_avrcp_operation_t;

typedef struct
{
    uint8_t latest_abs_vol;
    uint8_t set_abs_vol_timer;
} app_avrcp_set_absolute_volume_operation_context_t;

static app_avrcp_set_absolute_volume_operation_context_t set_abs_vol_ctx[BT_DEVICE_NUM] = {0};

static void app_avrcp_set_absolute_volume_operation_timer_callback(void *param)
{
    uint8_t device_id = (uint8_t)(uint32_t)param;

    if (device_id >= BT_DEVICE_NUM)
    {
        DEBUG_ERROR(0, "%s invalid parameters", __func__);
        return;
    }

    a2dp_volume_set(device_id, set_abs_vol_ctx[device_id].latest_abs_vol);
}

/**
 * @brief       App avrcp operation handler
 * @param[in]   Device index
 * @param[in]   Opreation
 */
static void app_avrcp_handle_operation(uint8_t device_id, app_avrcp_operation_t *opreation)
{
    if ((device_id >= BT_DEVICE_NUM) || !opreation)
    {
        DEBUG_ERROR(0, "%s invalid parameters", __func__);
        return;
    }

    DEBUG_INFO(0, "(d%d)%s opcode %d", device_id, __func__, opreation->opcode);

    switch (opreation->opcode)
    {
        case SET_ABSOLUTE_VOLUME_OP:
        {
            uint8_t curr_abs_volume = (uint8_t)(uint32_t)opreation->parameters;
            uint8_t curr_local_vol = a2dp_convert_bt_vol_to_local_vol(curr_abs_volume);
            uint8_t latest_local_vol = a2dp_convert_bt_vol_to_local_vol(set_abs_vol_ctx[device_id].latest_abs_vol);

            if (set_abs_vol_ctx[device_id].set_abs_vol_timer &&
                co_timer_is_running(&(set_abs_vol_ctx[device_id].set_abs_vol_timer)))
            {
                if (curr_local_vol != latest_local_vol)
                {
                    co_timer_stop(&(set_abs_vol_ctx[device_id].set_abs_vol_timer));
                    a2dp_volume_set(device_id, curr_abs_volume);
                }
                else
                {
                    co_timer_restart(&(set_abs_vol_ctx[device_id].set_abs_vol_timer));
                }
            }
            else
            {
                if (set_abs_vol_ctx[device_id].set_abs_vol_timer == 0)
                {
                    co_timer_new(&(set_abs_vol_ctx[device_id].set_abs_vol_timer), 200,
                        app_avrcp_set_absolute_volume_operation_timer_callback, (void*)(uint32_t)device_id, 1);
                }
                if (set_abs_vol_ctx[device_id].set_abs_vol_timer)
                {
                    co_timer_start(&(set_abs_vol_ctx[device_id].set_abs_vol_timer));
                }
                else
                {
                    DEBUG_ERROR(0, "(d%d) set_abs_vol_timer create fail!", device_id);
                }
            }

            set_abs_vol_ctx[device_id].latest_abs_vol = curr_abs_volume;
        } break;
        default:
        {
            DEBUG_ERROR(0, "Invalid opcode %d", opreation->opcode);
        } break;
    }
}

static void app_avrcp_operation_context_clear(uint8_t device_id)
{
    if (device_id >= BT_DEVICE_NUM)
    {
        DEBUG_ERROR(0, "%s invalid parameters", __func__);
        return;
    }

    if (set_abs_vol_ctx[device_id].set_abs_vol_timer)
    {
        co_timer_stop(&(set_abs_vol_ctx[device_id].set_abs_vol_timer));
        co_timer_del(&(set_abs_vol_ctx[device_id].set_abs_vol_timer));
        set_abs_vol_ctx[device_id].set_abs_vol_timer = 0;
    }
}

static void app_AVRCP_sendCustomCmdRsp(uint8_t device_id, btif_avrcp_channel_t *chnl, uint8_t isAccept, uint8_t transId)
{
    btif_avrcp_tg_accept_custom_cmd(chnl, isAccept, transId);
}

static void app_AVRCP_CustomCmd_Received(uint8_t* ptrData, uint32_t len)
{
    DEBUG_INFO(1,"AVRCP Custom Command Received %d bytes data:", len);
    DUMP8("0x%02x ", ptrData, len);
}

static media_info_report_t media_info_report_cb = NULL;
void app_avrcp_reg_media_info_report_callback(media_info_report_t cb)
{
    media_info_report_cb = cb;
}

#ifdef AVRCP_TRACK_CHANGED
static void avrcp_media_info_report(const bt_bdaddr_t *addr, const avrcp_adv_rsp_parms_t *mediaPkt)
{
    if (media_info_report_cb) {
        media_info_report_cb(addr, mediaPkt);
    }
}
#endif

bool is_bd_addr_valid(bt_bdaddr_t * addr)
{
    uint8_t addr_empty[6];
    memset(addr_empty, 0, sizeof(addr_empty));
    if (memcmp(addr,addr_empty,6)){
        return TRUE;
    }else{
        return FALSE;
   }
}

#ifndef IBRT
static void app_avrcp_reconnect_timeout_timer_handler(struct BT_DEVICE_T *curr_device)
{
    uint8_t state;

    state = btif_get_avrcp_state(curr_device->avrcp_channel);
    DEBUG_INFO(4,"(d%x) %s a2dp state %d avrcp state %d", curr_device->device_id,
            __func__, curr_device->a2dp_conn_flag, state);
    if ((!app_is_disconnecting_all_bt_connections()) && curr_device->a2dp_conn_flag &&
        (state != BTIF_AVRCP_STATE_CONNECTED))
    {
        btif_remote_device_t *rdev = NULL;
        rdev = btif_a2dp_get_remote_device(curr_device->a2dp_connected_stream);
        if (curr_device->a2dp_connected_stream && rdev)
        {
            bt_bdaddr_t *bd_addr = btif_me_get_remote_device_bdaddr(rdev);
            if (is_bd_addr_valid(bd_addr)) {
                if (besbt_cfg.mark_some_code_for_fuzz_test)
                {
                    return;
                }
                app_bt_reconnect_avrcp_profile(bd_addr);
            } else {
                DEBUG_INFO(1,"%s bd_addr is empty ",__func__);
            }
        }
    }
}

static void app_avrcp_reconnect_timeout_timer_callback(void *param)
{
    app_bt_start_custom_function_in_bt_thread((uint32_t)param ,0,
        (uint32_t)app_avrcp_reconnect_timeout_timer_handler);
}
#endif

#define MAX_AVRCP_CONNECT_TRY_TIME 3

extern void app_bt_audio_avrcp_status_quick_switch_filter_timer_callback(void const *param);
extern void app_bt_audio_avrcp_play_status_wait_timer_callback(void const *param);

static void app_avrcp_quick_switch_timer_callback(void *param)
{
    app_bt_audio_avrcp_status_quick_switch_filter_timer_callback(param);
}

static void app_avrcp_play_status_wait_timer_callback(void *param)
{
    app_bt_audio_avrcp_play_status_wait_timer_callback(param);
}

void app_avrcp_get_capabilities_start(int device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(0,"::AVRCP_GET_CAPABILITY\n");

    btif_avrcp_ct_get_capabilities(curr_device->avrcp_channel, BTIF_AVRCP_CAPABILITY_EVENTS_SUPPORTED);
}

void avrcp_get_current_media_status(int device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if(curr_device->avrcp_conn_flag == 0)
        return;

    btif_avrcp_ct_get_play_status(curr_device->avrcp_channel);
}

const char* avrcp_get_track_element_name(uint32_t element_id)
{
    switch (element_id)
    {
        case BTIF_AVRCP_MEDIA_ATTR_TITLE: return "TITLE";
        case BTIF_AVRCP_MEDIA_ATTR_ARTIST: return "ARTIST";
        case BTIF_AVRCP_MEDIA_ATTR_ALBUM: return "ALBUM";
        case BTIF_AVRCP_MEDIA_ATTR_TRACK: return "TRACK";
        case BTIF_AVRCP_MEDIA_ATTR_NUM_TRACKS: return "NUM_TRACKS";
        case BTIF_AVRCP_MEDIA_ATTR_GENRE: return "GENRE";
        case BTIF_AVRCP_MEDIA_ATTR_DURATION: return "DURATION";
        case BTIF_AVRCP_MEDIA_ATTR_COVER_ART: return "Cover Art";
        default: break;
    }
    return "N/A";
}

static void avrcp_event_cmd(uint8_t device_id, btif_avrcp_channel_t *channel,
                                    const avrcp_callback_parms_t * parms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    avctp_cmd_frame_t *cmd_frame;

    cmd_frame = btif_get_avrcp_cmd_frame(parms);

    if (cmd_frame == NULL)
    {
        DEBUG_INFO(0, "avrcp cmd frame wrong");
        return;
    }

    DEBUG_INFO(8,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_COMMAND role%d"
            " ctype%02x subtp%02x subid%02x op%02x len%d more%d",
            device_id,
            btif_get_avrcp_channel_role(channel),
            cmd_frame->ctype,
            cmd_frame->subunitType,
            cmd_frame->subunitId,
            cmd_frame->opcode,
            cmd_frame->operandLen,
            cmd_frame->more);

    switch(cmd_frame->ctype) {
    case BTIF_AVRCP_CTYPE_STATUS:
        {
            uint32_t company_id = *(cmd_frame->operands+2)
                + ((uint32_t)(*(cmd_frame->operands+1))<<8)
                + ((uint32_t)(*(cmd_frame->operands))<<16);

            if(company_id == 0x001958)  //bt sig
            {
                avrcp_operation_t op = *(cmd_frame->operands+3);
                POSSIBLY_UNUSED uint8_t oplen =  *(cmd_frame->operands+6)+ ((uint32_t)(*(cmd_frame->operands+5))<<8);
                switch(op)
                {
                case BTIF_AVRCP_OP_GET_CAPABILITIES:
                    {
                        uint8_t event = *(cmd_frame->operands+7);
                        if(event==BTIF_AVRCP_CAPABILITY_COMPANY_ID)
                        {
                            DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT receive "
                                    "get_capabilities support compay id",
                                    device_id);
                            btif_avrcp_tg_send_capability_company_id_rsp(channel, cmd_frame->transId);
                        }
                        else if(event == BTIF_AVRCP_CAPABILITY_EVENTS_SUPPORTED)
                        {
                            DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive get_capabilities"
                                    " support event transId:%d",
                                    device_id, cmd_frame->transId);
#ifdef __AVRCP_EVENT_COMMAND_VOLUME_SKIP__
                            btif_set_avrcp_adv_rem_event_mask(channel, 0);
#else
                            btif_set_avrcp_adv_rem_event_mask(channel,
                                BTIF_AVRCP_ENABLE_BATT_STATUS_CHANGED|BTIF_AVRCP_ENABLE_VOLUME_CHANGED);
#endif
                            btif_avrcp_tg_send_capability_rsp(channel, BTIF_AVRCP_CAPABILITY_EVENTS_SUPPORTED,
                                    btif_get_avrcp_adv_rem_event_mask(channel), cmd_frame->transId);
                        }
                        else
                        {
                            DEBUG_WARNING(1,"(d%x) ::avrcp_callback_CT receive get_capabilities error event value %d", device_id, op);
                            btif_avrcp_tg_send_error_rsp_for_specific_avc_commands(channel, cmd_frame->transId, BTIF_AVRCP_OP_GET_CAPABILITIES, BTIF_AVRCP_ERR_INVALID_PARM);
                        }
                    }
                    break;
                default:
                    {
                        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive "
                                "AVRCP_EVENT_COMMAND unhandled op=%x,oplen=%x",
                                device_id, op,oplen);
                    }
                    break;
                }

            }
            else
            {
                DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive AVRCP_EVENT_COMMAND"
                        " unknown company_id=%x",
                        device_id, company_id);
            }
        }
        break;
    case BTIF_AVCTP_CTYPE_CONTROL:
        {
            if (cmd_frame->operands[3] == BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME){
                uint8_t volume = 0;
                volume = cmd_frame->operands[7] & 0x7f;
                DEBUG_INFO(4,"(d%x) ::avrcp_callback_CT receive CONTROL"
                        " set_absolute_volume %d %d%% transId:%d",
                        device_id, volume, ((int)volume)*100/128, cmd_frame->transId);

                if((cmd_frame->operandLen != 8))//it works for BQB
                {
                    DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT reject invalid volume", device_id);
                    btif_avrcp_tg_send_error_rsp_for_specific_avc_commands(channel, cmd_frame->transId,
                        BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME,
                        BTIF_AVRCP_ERR_INVALID_PARM);
                    break;
                }
                if(cmd_frame->operands[5] != 0x00 || cmd_frame->operands[6] != 0x01)//it also works for BQB
                {
                    DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT reject error length", device_id);
                    btif_avrcp_tg_send_error_rsp_for_specific_avc_commands(channel, cmd_frame->transId,
                        BTIF_AVRCP_OP_SET_ABSOLUTE_VOLUME,
                        BTIF_AVRCP_ERR_PARM_CONTENT_ERR);
                    break;
                }

                app_avrcp_operation_t set_abs_vol;
                set_abs_vol.opcode = SET_ABSOLUTE_VOLUME_OP;
                set_abs_vol.parameters = (void *)(uint32_t)volume;
                app_avrcp_handle_operation(device_id, &set_abs_vol);

                btif_avrcp_tg_send_absolute_volume_rsp(channel, cmd_frame->operands[7], cmd_frame->transId);
            } else if (BTIF_AVRCP_OP_CUSTOM_CMD == cmd_frame->operands[3]) {
                DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive CONTROL CUSTOM_CMD transId:%d",
                                                device_id, cmd_frame->transId);
                app_AVRCP_CustomCmd_Received(&cmd_frame->operands[7], cmd_frame->operandLen - 7);
                app_AVRCP_sendCustomCmdRsp(device_id, channel, true,cmd_frame->transId);
            }
        }
        break;
    case BTIF_AVCTP_CTYPE_NOTIFY:
        {
            POSSIBLY_UNUSED bt_status_t status;
            if (cmd_frame->operands[7] == BTIF_AVRCP_EID_VOLUME_CHANGED){
                DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive REGISTER "
                        "volume_changed_status NOTIFY transId:%d",
                         device_id, cmd_frame->transId);
                curr_device->volume_report = BTIF_AVCTP_RESPONSE_INTERIM;
                status = btif_avrcp_tg_send_volume_change_interim_rsp(channel, curr_device->a2dp_current_abs_volume);
            }
            else if (cmd_frame->operands[7] == BTIF_AVRCP_EID_BATT_STATUS_CHANGED)
            {
                DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT receive REGISTER "
                        "battery_changed_status NOTIFY transId:%d",
                         device_id, cmd_frame->transId);
                status = btif_avrcp_tg_send_battery_change_interim_rsp(channel, 0x00, cmd_frame->transId);
            }
            else if(cmd_frame->operands[7] == 0xff)//it works for BQB
            {
                status = btif_avrcp_tg_send_error_rsp_for_specific_avc_commands(channel, cmd_frame->transId, BTIF_AVRCP_OP_REGISTER_NOTIFY, BTIF_AVRCP_ERR_INVALID_PARM);
                DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT receive invalid op=0xff tran id:%d status:%d",
                                        device_id, cmd_frame->transId, status);
            }
        }
        break;
    default:
        break;
    }
}

static void avrcp_event_adv_rsp(uint8_t device_id, btif_avrcp_channel_t *channel,
                                    const avrcp_callback_parms_t * parms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    POSSIBLY_UNUSED avrcp_role_t role;
    btif_avrcp_operation_t op;
    bt_status_t state;
    avrcp_adv_rsp_parms_t * adv_rsp;

    role = btif_get_avrcp_channel_role(channel);
    op = btif_get_avrcp_cb_channel_advOp(parms);
    state = btif_get_avrcp_cb_channel_state(parms);

    DEBUG_INFO(4,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_ADV_RESPONSE role=%d op=%02x status=%d",
            device_id, role, op, state);

    if (state != BT_STS_SUCCESS)
        return;

    if(op == BTIF_AVRCP_OP_GET_CAPABILITIES)
    {
        avrcp_event_mask_t mask;

        adv_rsp = btif_get_avrcp_adv_rsp(parms);
        ASSERT(adv_rsp != NULL, "avrdp adv rsp wrong");
        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT get_capabilities_rsp eventmask=%x",
                                device_id, adv_rsp->capability.info.eventMask);

        btif_set_avrcp_adv_rem_event_mask(channel, adv_rsp->capability.info.eventMask);
        mask = btif_get_avrcp_adv_rem_event_mask(channel);
        if(mask & BTIF_AVRCP_ENABLE_PLAY_STATUS_CHANGED)
        {
            DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT get_capabilities_rsp support"
                    " PLAY_STATUS_CHANGED", device_id);
            btif_avrcp_ct_register_media_status_notification(channel, 0);
            curr_device->avrcp_remote_support_playback_status_change_event = true;
        }
        if(mask & BTIF_AVRCP_ENABLE_PLAY_POS_CHANGED)
        {
#ifdef AVRCP_TRACK_CHANGED
            DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT get_capabilities_rsp support PLAY_POS_CHANGED", device_id);
            btif_avrcp_ct_register_play_pos_notification(channel, 1);
#endif
        }
#ifdef AVRCP_TRACK_CHANGED
        if (mask & BTIF_AVRCP_ENABLE_TRACK_CHANGED)
        {
            DEBUG_INFO(1,"(d%x) ::avrcp_callback_CT get_capabilities_rsp support TRACK_CHANGED", device_id);
            btif_avrcp_ct_register_track_change_notification(channel, 0);
        }
#endif
    }
    else if(op == BTIF_AVRCP_OP_REGISTER_NOTIFY)
    {
        avrcp_adv_notify_parms_t *notify;
        notify = btif_get_avrcp_adv_notify(parms);
        ASSERT(notify != NULL, "avrdp notify wrong");

        bt_callback_avrcp_register_notify_response_callback(notify->event);
        if(notify->event == BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED)
        {
            DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT NOTIFY RSP playback_changed_status =%x",
                                                    device_id, notify->p.mediaStatus);

            bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_NOTIFY_RSP,
                                                    notify->p.mediaStatus);
            // keep this line after bt_audio_event_handler
            if(curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_AVRCP_STATE)
            {
                curr_device->a2dp_status_recheck &= ~(A2DP_STATUS_SYNC_AVRCP_STATE);
            }
            curr_device->avrcp_playback_status = notify->p.mediaStatus;
            curr_device->a2dp_play_pause_flag =
                    (curr_device->avrcp_playback_status == 0x01) ? 1 : 0;
        }
        else if(notify->event == BTIF_AVRCP_EID_PLAY_POS_CHANGED)
        {
            DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT NOTIFY RSP playpos_changed_status =%x",
                                                device_id, notify->p.position);
        }
#ifdef AVRCP_TRACK_CHANGED
        else if(notify->event == BTIF_AVRCP_EID_TRACK_CHANGED)
        {
            DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT NOTIFY RSP "
                    "track_changed_status msU32=%x, lsU32=%x",
                    device_id, notify->p.track.msU32, notify->p.track.lsU32);
            curr_device->track_changed = BTIF_AVCTP_RESPONSE_INTERIM;
            btif_avrcp_ct_get_media_Info(channel, BTIF_AVRCP_ENABLE_MEDIA_ATTR_TITLE | BTIF_AVRCP_ENABLE_MEDIA_ATTR_ARTIST | BTIF_AVRCP_ENABLE_MEDIA_ATTR_DURATION|BTIF_AVRCP_ENABLE_MEDIA_ATTR_COVER_ART);
        }
#endif
    }
    else if(op == BTIF_AVRCP_OP_GET_PLAY_STATUS)
    {
        adv_play_status_t * play_status;

        play_status = btif_get_avrcp_adv_rsp_play_status(parms);
        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT get_play_status_rsp playback_changed_status =%d",
                        device_id, play_status->mediaStatus);
        curr_device->avrcp_playback_status = play_status->mediaStatus;
        curr_device->a2dp_play_pause_flag =
                    (curr_device->avrcp_playback_status == 0x01) ? 1 : 0;
    }
#ifdef AVRCP_TRACK_CHANGED
    else if (op == BTIF_AVRCP_OP_GET_MEDIA_INFO)
    {
        uint8_t num_of_elements = 0;
        adv_rsp = btif_get_avrcp_adv_rsp(parms);
        ASSERT(adv_rsp != NULL, "avrdp adv rsp wrong");

        num_of_elements = adv_rsp->element.numIds;

        DEBUG_INFO(2, "(d%x) ::avrcp_callback_CT get_media_info_rsp track_changed_status num=%d", device_id, num_of_elements);

        for (int i = 0; i < num_of_elements && i < BTIF_AVRCP_NUM_MEDIA_ATTRIBUTES; i++)
        {
            if (adv_rsp->element.txt[i].length > 0)
            {
                /**
                 * KuGou Music APP support send lyrics in TITLE attribute
                 */
                DEBUG_INFO(4, "::avrcp_callback_CT [%d] %s %d: %s\n", i,
                    avrcp_get_track_element_name(adv_rsp->element.txt[i].attrId),
                    adv_rsp->element.txt[i].length, adv_rsp->element.txt[i].string);
                if (adv_rsp->element.txt[i].attrId == BTIF_AVRCP_MEDIA_ATTR_COVER_ART)
                {
                    btif_avrcp_record_cover_art_handle(device_id, adv_rsp->element.txt[i].string, adv_rsp->element.txt[i].length);
                    btif_avrcp_send_obex_get_link_thumbnail_by_art_handle(&curr_device->remote, adv_rsp->element.txt[i].string);
                }
            }
        }
        avrcp_media_info_report(&curr_device->remote, adv_rsp);
    }
#endif
}

static void avrcp_event_adv_notify(uint8_t device_id, btif_avrcp_channel_t *channel,
                                    const avrcp_callback_parms_t * parms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    avrcp_adv_notify_parms_t *notify;

    notify = btif_get_avrcp_adv_notify(parms);
    ASSERT(notify != NULL, "avrdp notify wrong");

    if(notify->event == BTIF_AVRCP_EID_VOLUME_CHANGED)
    {
        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT NOTIFY volume_changed_status =%x",
                                    device_id, notify->p.volume);
        btif_avrcp_ct_register_volume_change_notification(channel, 0);
    }
    else if(notify->event == BTIF_AVRCP_EID_MEDIA_STATUS_CHANGED)
    {
        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT NOTIFY playback_changed_status =%x",
                                    device_id, notify->p.mediaStatus);
        if(curr_device->a2dp_status_recheck & A2DP_STATUS_SYNC_AVRCP_STATE)
        {
            curr_device->a2dp_status_recheck &= ~(A2DP_STATUS_SYNC_AVRCP_STATE);
        }
        curr_device->avrcp_playback_status = notify->p.mediaStatus;
        curr_device->a2dp_play_pause_flag = (curr_device->avrcp_playback_status == 0x01) ? 1 : 0;

        btif_avrcp_ct_register_media_status_notification(channel, 0);
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED,
                                    curr_device->avrcp_playback_status);
        bt_avrcp_play_status_change_t param;
        param.event = BT_AVRCP_PLAY_STATUS_CHANGED;
        param.device_id = device_id;
        param.interim = 0;
        param.play_status = (bt_avrcp_play_status_t)curr_device->avrcp_playback_status;
        btif_report_bt_event(&curr_device->remote, BT_EVENT_AVRCP_PLAY_STATUS_CHANGE, &param);
    }
    else if(notify->event == BTIF_AVRCP_EID_PLAY_POS_CHANGED)
    {
        DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT NOTIFY playpos_changed_status =%x",
                                    device_id, notify->p.position);
        btif_avrcp_ct_register_play_pos_notification(channel, 1);
    }
#ifdef AVRCP_TRACK_CHANGED
    else if(notify->event == BTIF_AVRCP_EID_TRACK_CHANGED)
    {
        DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT NOTIFY track_changed_status "
                "msU32=%x, lsU32=%x", device_id,
                notify->p.track.msU32, notify->p.track.lsU32);
        btif_avrcp_ct_register_track_change_notification(channel, 0);
    }
#endif
    return;
}

void avrcp_callback_CT(uint8_t device_id, btif_avrcp_channel_t* btif_avrcp,
                                        const avrcp_callback_parms_t* parms)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    btif_avrcp_channel_t *channel = NULL;
    btif_avctp_event_t event = btif_avrcp_get_callback_event(parms);
    uint8_t init_local_volume = 0;
    uint8_t init_abs_volume = 0;

    if (device_id == BT_DEVICE_INVALID_ID && event == BTIF_AVRCP_EVENT_DISCONNECT)
    {
        // avrcp profile is closed due to acl created fail
        DEBUG_INFO(1,"::AVRCP_EVENT_DISCONNECT acl created error=%x", btif_get_avrcp_cb_channel_error_code(parms));
        return;
    }

    channel = curr_device->avrcp_channel;

    DEBUG_INFO(5,"(d%x) ::%s channel %p event %d%s", device_id,
                __func__, btif_avrcp, event, btif_avrcp_event_str(event));
    ASSERT(device_id < BT_DEVICE_NUM && curr_device->avrcp_channel == btif_avrcp,
                                    "avrcp device channel must match");

    switch(event)
    {
        case BTIF_AVRCP_EVENT_CONNECT_IND:
#if defined(_AUTO_TEST_)
            AUTO_TEST_SEND("Connect ok.");
#endif
            DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_CONNECT_IND %d",
                                    device_id, event);
            break;
        case BTIF_AVRCP_EVENT_CONNECT:
            app_bt_profile_connect_manager_avrcp(device_id, BTIF_AVRCP_EVENT_CONNECT);
#if defined(_AUTO_TEST_)
            AUTO_TEST_SEND("Connect ok.");
#endif

#if defined(IBRT) && defined(IBRT_UI)
            app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
            app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
#endif

            curr_device->avrcp_conn_flag = 1;
            curr_device->waiting_pause_suspend = false;
            curr_device->avrcp_remote_support_playback_status_change_event = false;
            curr_device->mock_avrcp_after_force_disc = false;
            curr_device->ibrt_slave_force_disc_avrcp = false;

            app_bt_query_hfp_sdp_when_mobile_reconnect(device_id);

            app_avrcp_get_capabilities_start(device_id);

            curr_device->a2dp_default_abs_volume = app_bt_manager.config.a2dp_default_abs_volume;

            init_abs_volume = a2dp_abs_volume_get(device_id);

            init_local_volume = a2dp_volume_local_get(device_id);

            DEBUG_INFO(3, "(d%x) initial volume %d %d", device_id, init_local_volume, init_abs_volume);

            if (init_abs_volume == 0)
            {
                init_abs_volume = a2dp_convert_local_vol_to_bt_vol(init_local_volume);
            }

            curr_device->a2dp_initial_volume = init_abs_volume;

            app_bt_a2dp_current_abs_volume_just_set(device_id, init_abs_volume);

            curr_device->avrcp_playback_status = 0;

#ifdef RESUME_MUSIC_AFTER_CRASH_REBOOT
            app_bt_resume_music_after_crash_reboot(device_id);
#endif
            curr_device->avrcp_connect_try_times = 0;
#ifndef IBRT
#ifdef BT_A2DP_SUPPORT
            osTimerStop(curr_device->avrcp_reconnect_timer);
#endif
#endif
            bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_AVRCP_CONNECTED, 0);
            break;
        case BTIF_AVRCP_EVENT_CONNECT_MOCK:
            curr_device->waiting_pause_suspend = false;
            curr_device->avrcp_conn_flag = 1;
            bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_AVRCP_CONNECTED, 0);
            break;
        case BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGE_EVENT_SUPPORT:
            break;
        case BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED:
            bt_audio_event_handler(device_id,APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED_MOCK,0);
            break;
        case BTIF_AVRCP_EVENT_CT_SDP_INFO:
            /* avrcp_state IBRT_CONN_AVRCP_REMOTE_CT_0104 */
            break;
        case BTIF_AVRCP_EVENT_DISCONNECT:
            app_bt_profile_connect_manager_avrcp(device_id, BTIF_AVRCP_EVENT_DISCONNECT);
#if defined(IBRT) && defined(IBRT_UI)
            app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
            app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_AVRCP_PROFILE_ID);
#endif
            app_avrcp_operation_context_clear(device_id);

            curr_device->avrcp_conn_flag = 0;
            curr_device->a2dp_need_resume_flag = 0;
            DEBUG_INFO(1,"(d%x) ::AVRCP_EVENT_DISCONNECT", device_id);

            curr_device->avrcp_playback_status = 0;
            curr_device->volume_report = 0;
            curr_device->waiting_pause_suspend = false;
#ifdef AVRCP_TRACK_CHANGED
            curr_device->track_changed = 0;
#endif

#ifndef IBRT
#ifdef BT_A2DP_SUPPORT 
            curr_device->avrcp_connect_try_times = 0;
            osTimerStop(curr_device->avrcp_reconnect_timer);
            if (besbt_cfg.mark_some_code_for_fuzz_test)
            {
                break;
            }

            if(!app_is_disconnecting_all_bt_connections())
            {
                curr_device->avrcp_connect_try_times++;
                if (curr_device->avrcp_connect_try_times < MAX_AVRCP_CONNECT_TRY_TIME)
                {
                    osTimerStart(curr_device->avrcp_reconnect_timer, 3000);
                }
            }
#endif
#endif
            break;
        case BTIF_AVRCP_EVENT_RESPONSE:
            {
                btif_avrcp_operation_t op;
                adv_play_status_t * play_status;

                op = btif_get_avrcp_cb_channel_advOp(parms);
                play_status = btif_get_avrcp_adv_rsp_play_status(parms);

                DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_RESPONSE "
                        "op=0x%x[0x30=GET_PLAY_STATUS],status=%x %x",
                        device_id, op, btif_get_avrcp_cb_channel_state(parms),
                        play_status->mediaStatus);

                if(op == BTIF_AVRCP_OP_GET_PLAY_STATUS)
                {
                    curr_device->avrcp_playback_status = play_status->mediaStatus;
                    curr_device->a2dp_play_pause_flag =
                                    (curr_device->avrcp_playback_status == 0x01) ? 1 : 0;
                }
            }

            break;
         /*For Sony Compability Consideration*/
        case BTIF_AVRCP_EVENT_PANEL_PRESS:
            {
                avrcp_panel_cnf_t * cnf;
                cnf = btif_get_avrcp_panel_cnf(parms);
                ASSERT(cnf != NULL, "avrcp panel cnf wrong");
                DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_PANEL_PRESS %02x %02x",
                        device_id, cnf->operation, btif_get_avrcp_panel_ind(parms)->operation);
                if (besbt_cfg.mark_some_code_for_fuzz_test)
                {
                    break; // dont set volume duplicated for fuzz test to avoid mailbox overflow
                }
                switch(cnf->operation)
                {
                    case BTIF_AVRCP_POP_VOLUME_UP:
                        audio_player_volume_up();
                        break;
                    case BTIF_AVRCP_POP_VOLUME_DOWN:
                        audio_player_volume_down();
                        break;
                    default :
                        break;
                    }
            }
            break;
        case BTIF_AVRCP_EVENT_PANEL_HOLD:
            DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_PANEL_HOLD %02x %02x",
                            device_id, btif_get_avrcp_panel_cnf (parms)->operation,
                            btif_get_avrcp_panel_ind(parms)->operation);
            break;
        case BTIF_AVRCP_EVENT_PANEL_RELEASE:
            DEBUG_INFO(3,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_PANEL_RELEASE %02x %02x",
                            device_id, btif_get_avrcp_panel_cnf (parms)->operation,
                            btif_get_avrcp_panel_ind(parms)->operation);
            break;
         /*For Sony Compability Consideration End*/
        case BTIF_AVRCP_EVENT_PANEL_CNF:
            {
                avrcp_panel_cnf_t * cnf;

                cnf = btif_get_avrcp_panel_cnf(parms);
                ASSERT(cnf != NULL, "avrcp panel cnf wrong");
                DEBUG_INFO(4,"(d%x) ::AVRCP_EVENT_PANEL_CNF %02x %02x %02x",
                        device_id, cnf->operation, cnf->press, cnf->response);
            }
            break;
        case BTIF_AVRCP_EVENT_ADV_TX_DONE://20
            DEBUG_INFO(4,"(d%x) ::AVRCP_EVENT_ADV_TX_DONE op:%02x err_code:%d state:%d",
                                device_id, btif_get_avrcp_cb_txPdu_Op(parms),
                                btif_get_avrcp_cb_channel_error_code(parms),
                                btif_get_avrcp_cb_channel_state(parms));
            break;
        case BTIF_AVRCP_EVENT_ADV_RESPONSE://18
            avrcp_event_adv_rsp(device_id, channel, parms);
            break;
        case BTIF_AVRCP_EVENT_COMMAND:
            avrcp_event_cmd(device_id, channel, parms);
            break;
        case BTIF_AVRCP_EVENT_ADV_NOTIFY://17
            avrcp_event_adv_notify(device_id, channel, parms);
            break;
        case BTIF_AVRCP_EVENT_ADV_CMD_TIMEOUT:
            DEBUG_INFO(2,"(d%x) ::avrcp_callback_CT AVRCP_EVENT_ADV_CMD_TIMEOUT role=%d",
                                device_id, btif_get_avrcp_channel_role(channel));
            break;

        case BTIF_AVRCP_EVENT_BROWSE_RESPONSE:
            break;

        case BTIF_AVRCP_EVENT_BROWSE_CMD:
            break;

        case BTIF_AVRCP_OBEX_EVENT_OPEN:
            break;

        case BTIF_AVRCP_OBEX_EVENT_CHANNEL_CONNECTED:
            break;

        case BTIF_AVRCP_OBEX_EVENT_DISCONNECT:
            break;

        case BTIF_AVRCP_OBEX_GET_RSP:
            break;
    }

    if (curr_device->avrcp_conn_flag)
    {
        btif_avrcp_connect_browsing_channel(&curr_device->remote);
        btif_avrcp_connect_obex_channel(&curr_device->remote);
    }

#if defined(IBRT)
    app_tws_ibrt_profile_callback(device_id, BTIF_APP_AVRCP_PROFILE_ID,
                                (void *)btif_avrcp, (void *)parms,&curr_device->remote);
#endif

#if defined(IBRT)
    if (event == BTIF_AVRCP_EVENT_CONNECT_MOCK)
    {
        bt_avrcp_opened_t param;
        param.error_code = 0;
        param.device_id = device_id;
        btif_report_bt_event(&curr_device->remote, BT_EVENT_AVRCP_OPENED, &param);
    }
    if(event == BTIF_AVRCP_EVENT_PLAYBACK_STATUS_CHANGED)
    {
        bt_avrcp_play_status_change_t param;
        param.event = BT_AVRCP_PLAY_STATUS_CHANGED;
        param.device_id = device_id;
        param.interim = 0;
        param.play_status = (bt_avrcp_play_status_t)curr_device->avrcp_playback_status;
        btif_report_bt_event(&curr_device->remote, BT_EVENT_AVRCP_PLAY_STATUS_CHANGE, &param);
    }
#endif
}

void btapp_send_pause_key(int stream_id)
{
    DEBUG_INFO(1,"btapp_send_pause_key id = %x",stream_id);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(stream_id);
    btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PAUSE,TRUE);
    btif_avrcp_set_panel_key(curr_device->avrcp_channel,BTIF_AVRCP_POP_PAUSE,FALSE);
}

void btapp_a2dp_suspend_music(int stream_id)
{
    DEBUG_INFO(1,"btapp_a2dp_suspend_music id = %x",stream_id);
    btapp_send_pause_key(stream_id);
}

void app_pts_ar_connect(bt_bdaddr_t *btaddr)
{
    btif_avrcp_connect(btaddr);
}

#ifdef __TENCENT_VOICE__
avrcp_media_status_t    media_status = 0xff;
uint8_t avrcp_get_media_status(void)
{
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(app_bt_audio_judge_is_pc_bydevid(device_id))
    {
        DEBUG_INFO(1,"current device is pc");
        if(!curr_device->a2dp_streamming)
        {
            return BTIF_AVRCP_MEDIA_PAUSED;
        }
        else
        {
            if(a2dp_audio_is_mute_frame(device_id))
            {
                return BTIF_AVRCP_MEDIA_PAUSED;
            }
            return BTIF_AVRCP_MEDIA_PLAYING;
        }
    }
    else
    {
        return curr_device->avrcp_playback_status;
    }
    DEBUG_INFO(2,"%s %d",__func__, media_status);
}
 uint8_t avrcp_ctrl_music_flag;
void avrcp_set_media_status(uint8_t status)
{
    DEBUG_INFO(2,"%s %d",__func__, status);
    if ((status == 1 && avrcp_ctrl_music_flag == 2) || (status == 2 && avrcp_ctrl_music_flag == 1))
        avrcp_ctrl_music_flag = 0;

#if defined(__CONNECTIVITY_LOG_REPORT__)
    if ((status == 2) || (status == 1)) {  //BTIF_AVRCP_MEDIA_PLAYING, BTIF_AVRCP_MEDIA_PAUSED
        app_ibrt_if_reset_acl_data_packet_check();
    }
#endif

    media_status = status;
}
#endif

void bes_bt_avrcp_ct_callback(uint8_t device_id, btif_avrcp_channel_t* btif_avrcp, const avrcp_callback_parms_t* parms)
    __attribute__((alias("avrcp_callback_CT")));

#endif

#if defined(BT_AVRCP_SUPPORT) || defined(BT_A2DP_SUPPORT)

void a2dp_volume_local_set(int id, uint8_t vol)
{
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && curr_device->acl_is_connected)
    {
        if (!nv_record_btdevicerecord_find(&curr_device->remote, &record) && record->device_vol.a2dp_vol != vol)
        {
            nv_record_btdevicerecord_set_a2dp_vol(record, vol);
#ifndef FPGA
            nv_record_touch_cause_flush();
#endif
        }
    }
}

void a2dp_abs_volume_set(int id, uint8_t vol)
{
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && curr_device->acl_is_connected)
    {
        if (!nv_record_btdevicerecord_find(&curr_device->remote, &record) && record->device_plf.a2dp_abs_vol != vol)
        {
            nv_record_btdevicerecord_set_a2dp_abs_vol(record, vol);
#ifndef FPGA
            nv_record_touch_cause_flush();
#endif
        }
    }
}

uint8_t a2dp_abs_volume_get(int id)
{
    uint8_t local_abs_vol = app_bt_manager.config.a2dp_default_abs_volume;
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && !nv_record_btdevicerecord_find(&curr_device->remote, &record))
    {
        local_abs_vol = record->device_plf.a2dp_abs_vol;
        DEBUG_INFO(3,"(d%x)%s,abs vol %d",id,__func__,local_abs_vol);
    }
    else
    {
        DEBUG_INFO(2,"(d%x)%s,not find remDev",id,__func__);
    }

    return local_abs_vol;
}

uint8_t a2dp_convert_bt_vol_to_local_vol(uint8_t btVol)
{
    return unsigned_range_value_map(btVol, 0, MAX_A2DP_VOL, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX);
}

void a2dp_update_local_volume(int id, uint8_t localVol)
{
    a2dp_volume_local_set(id, localVol);

    if (app_audio_adm_music_stream_is_active(id, AUDIO_TYPE_BT))
    {
        audio_player_set_volume(localVol);
    }
}

void a2dp_volume_set(int id, uint8_t bt_volume)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);
    uint8_t local_vol = 0;

    bt_volume = bt_volume > 127 ? 127 : bt_volume;

    local_vol = a2dp_convert_bt_vol_to_local_vol(bt_volume);

    if (curr_device)
    {
        DEBUG_INFO(4, "(d%x) %s a2dp_vol bt %d local %d", id, __func__, bt_volume, local_vol);

        curr_device->a2dp_current_abs_volume = bt_volume;

        a2dp_abs_volume_set(id, bt_volume);

        a2dp_update_local_volume(id, local_vol);
    }
}

void a2dp_volume_set_local_vol(int id, uint8_t local_vol)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);
    uint8_t bt_volume = a2dp_convert_local_vol_to_bt_vol(local_vol);

    if (curr_device)
    {
        DEBUG_INFO(4, "(d%x) %s a2dp_vol bt %d local %d", id, __func__, bt_volume, local_vol);

        curr_device->a2dp_current_abs_volume = bt_volume;

        a2dp_abs_volume_set(id, bt_volume);

        a2dp_update_local_volume(id, local_vol);
    }
}

static void app_bt_a2dp_send_volume_change_handler(int device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

#if defined(IBRT)
    if (!curr_device ||
        (tws_besaud_is_connected() &&
        (IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote))))
#else
    if (!curr_device)
#endif
    {
        return;
    }

    DEBUG_INFO(4, "(d%x) %s volume_report %02x vol %d", device_id, __func__, curr_device->volume_report, curr_device->a2dp_current_abs_volume);

    if (curr_device->volume_report == BTIF_AVCTP_RESPONSE_INTERIM)
    {
        bt_status_t ret = btif_avrcp_tg_send_volume_change_actual_rsp(curr_device->avrcp_channel,
            curr_device->a2dp_current_abs_volume);
        if (BT_STS_FAILED != ret)
        {
            curr_device->volume_report = BTIF_AVCTP_RESPONSE_CHANGED;
        }
        else
        {
            DEBUG_INFO(1, "set abs vol failed.");
        }
    }
}

bool app_bt_a2dp_send_volume_change(int device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return false;
    }

    app_bt_start_custom_function_in_bt_thread((uint32_t)device_id,
            0, (uint32_t)app_bt_a2dp_send_volume_change_handler);

    return true;
}

bool app_bt_a2dp_report_current_volume(int device_id)
{
#if defined(BT_A2DP_SUPPORT)
    btif_remote_device_t *remDev = NULL;
    btif_link_mode_t mode = BTIF_BLM_SNIFF_MODE;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    if (!curr_device)
    {
        return false;
    }

    remDev = btif_a2dp_get_remote_device(curr_device->a2dp_connected_stream);

    if (remDev)
    {
        mode = btif_me_get_current_mode(remDev);
    }
    else
    {
        mode = BTIF_BLM_SNIFF_MODE;
    }

    if (mode != BTIF_BLM_ACTIVE_MODE || device_id != app_bt_audio_get_curr_playing_a2dp())
    {
        BT_AUDIO_DEVICE_T *active_device = app_audio_adm_get_active_device();
        device_id = active_device->device_id;
    }
#ifdef ONLY_REPORT_VOLUME_WHEN_STREAMING
    if(device_id == app_bt_audio_get_curr_playing_a2dp())
    {
        return app_bt_a2dp_send_volume_change(device_id);
    }
    return false;
#else
    BT_AUDIO_DEVICE_T *active_device = app_audio_adm_get_active_device();
    if(active_device && active_device->device_id == device_id
        && curr_device->avrcp_conn_flag)
    {
        return app_bt_a2dp_send_volume_change(active_device->device_id);
    }
    return false;
#endif
#else
    return false;
#endif
}

void btapp_a2dp_report_speak_gain(void)
{
    for (int i = 0; i < BT_DEVICE_NUM; i++)
    {
        app_bt_a2dp_report_current_volume(i);
    }
}

uint8_t a2dp_volume_local_get(int id)
{
    uint8_t localVol = hal_codec_get_default_dac_volume_index();
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && !nv_record_btdevicerecord_find(&curr_device->remote, &record))
    {
        localVol = record->device_vol.a2dp_vol;
        DEBUG_INFO(3,"(d%x)%s,vol %d",id,__func__,localVol);
    }
    else
    {
        DEBUG_INFO(2,"(d%x)%s,not find remDev",id,__func__);
    }

    return localVol;
}

uint8_t a2dp_volume_get(int id)
{
    uint8_t localVol = a2dp_volume_local_get(id);

    return a2dp_convert_local_vol_to_bt_vol(localVol);
}

#if defined(BT_AVRCP_TEST_SUPPORT)
#include "app_bt_cmd.h"
static void app_pts_ar_connect_test(const char* param, uint32_t param_len)
{
    app_pts_ar_connect(app_bt_get_pts_address());
}

static void app_pts_ar_disconnect(const char* param, uint32_t param_len)
{
    btif_avrcp_disconnect(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel);
}

static void app_pts_ar_panel_play(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_PLAY,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_PLAY,FALSE);
}

static void app_pts_ar_panel_pause(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_PAUSE,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_PAUSE,FALSE);
}

static void app_pts_ar_panel_stop(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_STOP,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_STOP,FALSE);
}

static void app_pts_ar_panel_forward(const char* param, uint32_t param_le)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_FORWARD,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_FORWARD,FALSE);
}

static void app_pts_ar_panel_backward(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_BACKWARD,FALSE);
}

static void app_pts_ar_volume_up(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_VOLUME_UP,FALSE);
}

static void app_pts_ar_volume_down(const char* param, uint32_t param_len)
{
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,TRUE);
    btif_avrcp_set_panel_key(app_bt_get_device(BT_SOURCE_DEVICE_ID_1)->avrcp_channel,BTIF_AVRCP_POP_VOLUME_DOWN,FALSE);
}

static void app_pts_avrcp_play_test(const char* param, uint32_t param_len)
{
    a2dp_handleKey(AVRCP_KEY_PLAY);
}

static void app_pts_avrcp_pause_test(const char* param, uint32_t param_len)
{
    a2dp_handleKey(AVRCP_KEY_PAUSE);
}

static void app_pts_avrcp_stop_test(const char* param, uint32_t param_len)
{
    a2dp_handleKey(AVRCP_KEY_STOP);
}

static void app_pts_avrcp_forward_test(const char* param, uint32_t param_len)
{
    a2dp_handleKey(AVRCP_KEY_FORWARD);
}

static void app_pts_avrcp_backward_test(const char* param, uint32_t param_len)
{
    a2dp_handleKey(AVRCP_KEY_BACKWARD);
}

static void app_pts_ar_volume_notify(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    btif_avrcp_ct_register_volume_change_notification(curr_device->avrcp_channel, 0);
}

static void app_pts_ar_volume_change(const char* param, uint32_t param_len)
{
    static uint8_t volume = a2dp_volume_get(BT_DEVICE_ID_1);
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    volume += 1;
    curr_device->volume_report = BTIF_AVCTP_RESPONSE_CHANGED;
    btif_avrcp_tg_send_volume_change_actual_rsp(curr_device->avrcp_channel, volume);
}

#ifndef BTH_SUBSYS_ONLY
extern "C" struct btdevice_volume * app_bt_stream_volume_get_ptr(void);
#endif
static void app_pts_ar_set_absolute_volume(const char* param, uint32_t param_len)
{
    int vol = 10;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
#ifndef BTH_SUBSYS_ONLY
    vol = app_bt_stream_volume_get_ptr()->a2dp_vol;
#endif
    vol = 8*vol-1;
    if (vol > (0x7f-1))
        vol = 0x7f;
    btif_avrcp_ct_set_absolute_volume(curr_device->avrcp_channel, vol);
}

static void app_pts_ar_send_thumbnail_cmd(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        btif_avrcp_send_obex_get_link_thumbnail(&curr_device->remote);
    }
}

static void app_pts_ar_get_cover_art_att(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        btif_avrcp_ct_get_media_Info(curr_device->avrcp_channel, BTIF_AVRCP_ENABLE_MEDIA_ATTR_COVER_ART);
    }
}

static void app_pts_ar_connect_obex_channel(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        btif_avrcp_connect_obex_channel(&curr_device->remote);
    }
}

static void app_pts_ar_connect_browsing_channel(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        btif_avrcp_connect_browsing_channel(&curr_device->remote);
    }
}

static void app_pts_ar_send_folder_cmd(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        return;
    }

    bt_avrcp_get_folder_items_t folder_item = {0};
    folder_item.scope = 0;
    folder_item.start_item = 0x00000000;
    folder_item.end_item = 0x00000006;
    folder_item.mediaMask = 0x83;

    btif_avrcp_ct_get_folder_items(&curr_device->remote, &folder_item);
}

static app_bt_host_cmd_table_t app_avrcp_test_cmd_table[] =
{

    {"ar_connect",              app_pts_ar_connect_test},
     /* a2dp source cmd */
    {"ar_disconnect",           app_pts_ar_disconnect},
    {"ar_panel_play",           app_pts_ar_panel_play},
    {"ar_panel_pause",          app_pts_ar_panel_pause},
    {"ar_panel_stop",           app_pts_ar_panel_stop},
    {"ar_panel_forward",        app_pts_ar_panel_forward},
    {"ar_panel_backward",       app_pts_ar_panel_backward},
    {"ar_volume_up",            app_pts_ar_volume_up},
    {"ar_volume_down",          app_pts_ar_volume_down},
    /* a2dp sink cmd */
    {"ar_sink_panel_play",      app_pts_avrcp_play_test},
    {"ar_sink_panel_pause",     app_pts_avrcp_pause_test},
    {"ar_sink_panel_stop",      app_pts_avrcp_stop_test},
    {"ar_sink_panel_forward",   app_pts_avrcp_forward_test},
    {"ar_sink_panel_backward",  app_pts_avrcp_backward_test},
    {"ar_volume_notify",        app_pts_ar_volume_notify},
    {"ar_volume_change",        app_pts_ar_volume_change},
    {"ar_set_absolute_volume",  app_pts_ar_set_absolute_volume},
    {"ar_send_thumbnail_cmd",   app_pts_ar_send_thumbnail_cmd},
    {"ar_get_cover_art_att",    app_pts_ar_get_cover_art_att},
    {"ar_obex_connect",         app_pts_ar_connect_obex_channel},
    {"ar_browsing_connect",     app_pts_ar_connect_browsing_channel},
    {"ar_send_folder_cmd",      app_pts_ar_send_folder_cmd},
};
#endif /* BT_AVRCP_TEST_SUPPORT */

void avrcp_init(void)
{
#ifndef IBRT
    osTimerAttr_t timerattr;
    memset(&timerattr, 0, sizeof(timerattr));
    timerattr.name = "APP_AVRCP_RECONNECT_TIMER";
#endif
    osTimerAttr_t timerattra;
    memset(&timerattra, 0, sizeof(osTimerAttr_t));
    timerattra.name = "APP_AVRCP_QUICK_SWITCH_FILTER_TIMER";
    osTimerAttr_t timerattrb;
    memset(&timerattrb, 0, sizeof(osTimerAttr_t));
    timerattrb.name = "APP_AVRCP_PLAY_STATUS_WAIT_TIMER";

    for(int i =0; i< BT_DEVICE_NUM; i++ )
    {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(i);
        #ifndef IBRT
        curr_device->avrcp_reconnect_timer = osTimerNew(app_avrcp_reconnect_timeout_timer_callback, osTimerOnce, curr_device, &timerattr);
        if (curr_device->avrcp_reconnect_timer == NULL)
        {
            TRACE(0, "avrcp reconnect timer %d create fail!", i);
        }
        #endif

        curr_device->avrcp_pause_play_quick_switch_filter_timer =
            osTimerNew(app_avrcp_quick_switch_timer_callback, osTimerOnce, curr_device, &timerattra);
        curr_device->avrcp_play_status_wait_timer =
            osTimerNew(app_avrcp_play_status_wait_timer_callback, osTimerOnce, curr_device, &timerattrb);
        if (curr_device->avrcp_pause_play_quick_switch_filter_timer == NULL)
        {
            TRACE(0, "avrcp pause play quick switch filter timer %d create fail!", i);
        }

        if (curr_device->avrcp_play_status_wait_timer == NULL)
        {
            TRACE(0, "avrcp play status wait timer %d create fail!", i);
        }
    }

#if defined(BT_AVRCP_TEST_SUPPORT)
    app_bt_host_add_cmd_table(sizeof(app_avrcp_test_cmd_table)/sizeof(app_avrcp_test_cmd_table[0]),
        app_avrcp_test_cmd_table);
#endif /* BT_AVRCP_TEST_SUPPORT */

}
#endif
