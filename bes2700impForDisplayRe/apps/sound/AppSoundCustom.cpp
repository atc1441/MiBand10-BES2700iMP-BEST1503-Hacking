/***************************************************************************
 *
 * @copyright 2015-2023 BES.
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
 * @author wangzhuli
 * @date 2023.02.22         v1.0
 *
 ****************************************************************************/
#include "AppSoundCustom.h"

#include "SoundApi.h"
#include "SoundDebug.h"
#include "app_ui_api.h"
#include "app_vendor_cmd_evt.h"
#include "app_a2dp.h"

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif

#ifdef APP_SOUND_UI_ENABLE
#include "app_sound.h"
#endif
void soundModeChanged(SoundMode mode)
{
    LOG_D("Custom:Mode[%s] Ready!!!", soundMode2str(mode));
#ifdef APP_SOUND_UI_ENABLE
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if ((mode == MODE_TWS) && (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_FORMATION)) {
        app_sound_ui_event_handler(SOUND_ACTION_TWS_PREHANDLE, SOUND_UI_NO_ERROR);
    }
    else if ((mode == MODE_FREEMAN) && (app_sound_ctrl->super_state == SOUND_UI_W4_ENTER_FREEMAN)) {
        app_sound_ui_event_handler(SOUND_ACTION_ENTER_FREEMAN, SOUND_UI_NO_ERROR);
    }
#ifdef BLE_BIS_TRANSPORT
    else if ((mode == MODE_BIS) && (app_sound_ctrl->super_state == SOUND_UI_W4_ENTER_BIS))
    {
        app_sound_ui_event_handler(SOUND_ACTION_ENTER_BIS, SOUND_UI_NO_ERROR);
    }
#endif
#endif
}

void soundEvtRunComplete(SoundEvtPkt *evtPkt)
{
    LOG_D("Custom:Evt[%s] Complete", soundEvt2str(evtPkt->evt));
#ifdef APP_SOUND_UI_ENABLE
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    SOUND_ACTION_E action = SOUND_ACTION_END;

    switch (evtPkt->evt) {
        case SoundEvBtEnable:
            if (app_sound_ctrl->business_device_type == BT_AND_WIFI_DEVICE_TYPE)
            {
                action = SOUND_ACTION_W4_ALL_DEVICE_READY;
            }
            else
            {
                action = SOUND_ACTION_BT_ENABLE_EVENT;
            }
            break;
        case SoundEvBtDisable:
            action = SOUND_ACTION_BT_DISABLE_EVENT;
            break;
        case SoundEvBtPairing:
            action = SOUND_ACTION_BT_ENTERPAIRING_EVENT;
            break;

        default:
            break;
    }
    app_sound_ui_event_handler(action, SOUND_UI_NO_ERROR);
#endif
}

void soundBisSetParam(SoundBisParam *param)
{
    uint16_t company_id       = 0x02B0;
    const char *broadcast_name = "BES_BCAST";
    uint8_t sync_policy = SOUND_BIS_SYNC_POLICY_BCAST_ID | SOUND_BIS_SYNC_POLICY_COMPANY_ID | SOUND_BIS_SYNC_POLICY_BCAST_NAME;
    static uint8_t adv_mfrs_data[5] = {0x11, 0x22, 0x33, 0x44, 0x55};

    memcpy(param->broadcast_id, bis_tran_broadcast_id, 3);
    memcpy(param->encrypt_key, bis_tran_broadcast_key, 16);
    param->encrypt_enable    = true;
    param->adv_mfrs_data     = adv_mfrs_data;
    param->adv_mfrs_data_len = sizeof(adv_mfrs_data);
    param->bcast_name        = broadcast_name;
    param->company_id        = company_id;
    param->sync_policy       = sync_policy;

    LOG_D("Custom:Bis Param Set Complete");
}

void soundBisStreamStateChangeFunc(bool state, uint8_t role)
{
    LOG_I("[%s][%d]: stream state=%d role=%d", __FUNCTION__, __LINE__, state, role);
}

void soundBisSinkReceiveManufacturerDataFunc(uint8_t *data, uint8_t len)
{
    LOG_I("[%s][%d]: receive User adv data:", __FUNCTION__, __LINE__);
    DUMP8("%02x ", data, len);
}

void soundBtGlobalStateCallback(ibrt_global_state_change_event *state)
{
    LOG_D("Custom:global_status changed = %d", state->state);

    switch (state->state) {
        case IBRT_BLUETOOTH_ENABLED:
#ifdef APP_SOUND_UI_ENABLE
            {
                app_sound_init();
#ifndef WIFI_SOUND_UI_ENABLE
                app_sound_ui_test_key_init();
#endif
                struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
                if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
                {
                    app_sound_ui_event_entry(SOUND_UI_POWERON_EVENT);
                }
            }
#endif
            break;
        case IBRT_BLUETOOTH_DISABLED:
            break;
        default:
            break;
    }
}

void soundBtA2dpCallback(const bt_bdaddr_t *addr, ibrt_conn_a2dp_state_change *state)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x]A2dpStatus changed = %d",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->a2dp_state);
    } else {
        LOG_D("Custom:[d%x]A2dp_status changed = %d addr is NULL",
              state->device_id, state->a2dp_state);
    }

    switch (state->a2dp_state) {
        case IBRT_CONN_A2DP_IDLE:
            break;
        case IBRT_CONN_A2DP_OPEN:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_a2dp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_A2DP_CONNECTED);
#endif
            break;
        case IBRT_CONN_A2DP_CODEC_CONFIGURED:
            LOG_D("Custom:delay report support %d", state->delay_report_support);
            break;
        case IBRT_CONN_A2DP_STREAMING:
            LOG_D("Custom:A2dp Streaming");
#ifdef APP_SOUND_UI_ENABLE
            app_sound_a2dp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_A2DP_STREAM_STARTED);
#endif
            break;
        case IBRT_CONN_A2DP_SUSPENED:
            LOG_D("Custom:A2dp Suspened");
#ifdef APP_SOUND_UI_ENABLE
            app_sound_a2dp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_A2DP_STREAM_CLOSE);
#endif
            break;
        case IBRT_CONN_A2DP_CLOSED:
            LOG_D("Custom:A2dp Closed");
#ifdef APP_SOUND_UI_ENABLE
            app_sound_a2dp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_A2DP_DISCONNECTED);
#endif
            break;
        default:
            break;
    }
}

void soundBtHfpCallback(const bt_bdaddr_t *addr, ibrt_conn_hfp_state_change *state)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x]HfpStatus changed = %d",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->hfp_state);
    } else {
        LOG_D("Custom:[d%x]Hfp_status changed = %d addr is NULL",
              state->device_id, state->hfp_state);
    }

    switch (state->hfp_state) {
        case IBRT_CONN_HFP_SLC_DISCONNECTED:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_hfp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_HFP_DISCONNECTED);
#endif
            break;
        case IBRT_CONN_HFP_SLC_OPEN:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_hfp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_HFP_CONNECTED);
#endif
            break;
        case IBRT_CONN_HFP_SCO_OPEN:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_sco_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_SCO_CONNECTED);
#endif
            break;
        case IBRT_CONN_HFP_SCO_CLOSED:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_sco_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_SCO_DISCONNECTED);
#endif
            break;
        case IBRT_CONN_HFP_RING_IND:
            break;
        case IBRT_CONN_HFP_CALL_IND:
            break;
        case IBRT_CONN_HFP_CALLSETUP_IND:
#ifdef APP_SOUND_UI_ENABLE
            if (state->ciev_status == 1) {
                app_sound_hfp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_HFP_INCOMING_CALL);
            } else if (state->ciev_status == 2) {
                app_sound_hfp_state_handle(state->device_id, (device_addr_t *)addr, SOUND_BT_HFP_OUTGOING_CALL);
            }
#endif
            break;
        case IBRT_CONN_HFP_CALLHELD_IND:
            break;
        case IBRT_CONN_HFP_CIEV_SERVICE_IND:
            break;
        case IBRT_CONN_HFP_CIEV_SIGNAL_IND:
            break;
        case IBRT_CONN_HFP_CIEV_ROAM_IND:
            break;
        case IBRT_CONN_HFP_CIEV_BATTCHG_IND:
            break;
        case IBRT_CONN_HFP_SPK_VOLUME_IND:
            break;
        case IBRT_CONN_HFP_MIC_VOLUME_IND:
            break;
        case IBRT_CONN_HFP_IN_BAND_RING_IND:
            LOG_D("Custom:is in-band ring %d", state->in_band_ring_enable);
            // app_ibrt_if_BSIR_command_event(state->in_band_ring_enable);
            break;
        case IBRT_CONN_HFP_VR_STATE_IND:
            break;
        case IBRT_CONN_HFP_AT_CMD_COMPLETE:
            break;
        default:
            break;
    }
}

void soundBtAvrcpCallback(const bt_bdaddr_t *addr, ibrt_conn_avrcp_state_change *state)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x]AvrcpStatus changed = %d",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->avrcp_state);
    } else {
        LOG_D("Custom:[d%x]Avrcp_status changed = %d addr is NULL",
              state->device_id, state->avrcp_state);
    }

    switch (state->avrcp_state) {
        case IBRT_CONN_AVRCP_DISCONNECTED:
            break;
        case IBRT_CONN_AVRCP_CONNECTED:
            break;
        case IBRT_CONN_AVRCP_VOLUME_UPDATED:
            LOG_D("Custom:volume %d %d%%", state->volume, state->volume * 100 / 128);
            break;
        case IBRT_CONN_AVRCP_REMOTE_CT_0104:
            LOG_D("Custom:remote ct 1.4 support %d volume %d", state->support, state->volume);
            break;
        case IBRT_CONN_AVRCP_REMOTE_SUPPORT_PLAYBACK_STATUS_CHANGED_EVENT:
            LOG_D("Custom:playback status support %d", state->support);
            break;
        case IBRT_CONN_AVRCP_PLAYBACK_STATUS_CHANGED:
            LOG_D("Custom:playback status %d", state->playback_status);
            break;
        case IBRT_CONN_AVRCP_PLAY_STATUS_CHANGED:
            LOG_D("Custom:playStatus=%d, pos/totle=[%d/%d]", state->playback_status, state->play_position, state->play_length);
            break;
        case IBRT_CONN_AVRCP_PLAY_POS_CHANGED:
            LOG_D("Custom:playPos=%d", state->play_position);
        default:
            break;
    }
}

void soundBtAvrcpMediaInfoCallback(const bt_bdaddr_t *addr, const avrcp_adv_rsp_parms_t *mediainforsp)
{
    LOG_D("Custom:[%02x:%02x:*:*:*:%02x] Media info update.",
          addr->address[0], addr->address[1], addr->address[5]);

    uint8_t num_of_elements = mediainforsp->element.numIds;
    for (int i = 0; i < num_of_elements && i < BTIF_AVRCP_NUM_MEDIA_ATTRIBUTES; i++) {
        if (mediainforsp->element.txt[i].length > 0) {
            LOG_D("Custom:MediaInfo:[%d] %s %d: %s", i,
                  avrcp_get_track_element_name(mediainforsp->element.txt[i].attrId),
                  mediainforsp->element.txt[i].length, mediainforsp->element.txt[i].string);
        }
    }
}

void soundBtNewlyPairedTwsCallback(const bt_bdaddr_t *addr)
{
    uint8_t *p = (uint8_t *)addr;
    LOG_D("Custom:newly tws paired.");
    LOG_D("tws_addr:  %02x:%02x:%02x:%02x:%02x:%02x",
          p[0], p[1], p[2], p[3], p[4], p[5]);
}

void soundBtNewlyPairedMobileCallback(const bt_bdaddr_t *addr)
{
    uint8_t *p = (uint8_t *)addr;
    LOG_D("Custom:newly mobile paired.");
    LOG_D("mob_addr:  %02x:%02x:%02x:%02x:%02x:%02x",
          p[0], p[1], p[2], p[3], p[4], p[5]);
}

void soundBtTwsParingStateChanged(ibrt_conn_pairing_state state, uint8_t reason_code)
{
    LOG_D("Custom:tws_pairing_state changed = %d with reason 0x%x", state, reason_code);

    switch (state) {
        case IBRT_CONN_PAIRING_IDLE:
            break;
        case IBRT_CONN_PAIRING_IN_PROGRESS:
            break;
        case IBRT_CONN_PAIRING_COMPLETE:
            break;
        case IBRT_CONN_PAIRING_TIMEOUT:
            break;
        default:
            break;
    }
}

void soundBtTwsAclStateChanged(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code)
{
#ifdef APP_SOUND_UI_ENABLE
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    struct app_sound_tws_info_t *p_tws_info = &app_sound_ctrl->bt_device_info.tws_info;
#endif
    LOG_D("Custom:tws_acl_state changed = %d with reason 0x%x role %d", state->state.acl_state, reason_code, state->current_role);

    switch (state->state.acl_state) {
        case IBRT_CONN_ACL_CONNECTED:
#ifdef APP_SOUND_UI_ENABLE
            if (state->current_role < SOUND_ROLE_UNKNOW) {
                p_tws_info->role = (SOUND_ROLE_E)state->current_role;
            } else {
                p_tws_info->role = SOUND_ROLE_UNKNOW;
            }

            if ((reason_code > 0x47) || (reason_code == 0)) {
                p_tws_info->connect_status = SOUND_ACL_CONNECTED;
                app_sound_ui_event_handler(SOUND_ACTION_TWS_CONNECTE_SIG, SOUND_UI_NO_ERROR);
                LOG_D("Custom:tws_acl_state changed CONNECTED connect_status=%d reason 0x%x role %d",app_sound_ctrl->bt_device_info.tws_info.connect_status, reason_code, state->current_role);
            } else  {
                p_tws_info->connect_status = SOUND_ACL_DISCONNECTED;
                LOG_D("Custom:tws_acl_state changed CONNECTE fail connect_status=%d; with reason 0x%x role %d",app_sound_ctrl->bt_device_info.tws_info.connect_status, reason_code, state->current_role);
            }
#endif
            break;
        case IBRT_CONN_ACL_DISCONNECTED:
#ifdef APP_SOUND_UI_ENABLE
            p_tws_info->connect_status = SOUND_ACL_DISCONNECTED;
            app_sound_ui_event_handler(SOUND_ACTION_TWS_DISCONNECTED, SOUND_UI_NO_ERROR);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING_CANCELED:
            break;
        case IBRT_CONN_ACL_CONNECTING_FAILURE:
#ifdef APP_SOUND_UI_ENABLE
            p_tws_info->connect_status = SOUND_ACL_DISCONNECTED;
            app_sound_ui_event_handler(SOUND_ACTION_TWS_CONNECTE_SIG, SOUND_UI_CONNECT_FAILED_PAGE_TIMEOUT);
#endif
            break;
        default:
            break;
    }
}

void soundBtMobileAclStateChanged(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x]AclStatus changed = %d reason 0x%x",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->state.acl_state, reason_code);
    } else {
        LOG_D("Custom:[d%x]Acl state changed = %d reason 0x%x, addr is NULL",
              state->device_id, state->state.acl_state, reason_code);
    }

    switch (state->state.acl_state) {
        case IBRT_CONN_ACL_DISCONNECTED:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_bredr_acl_state_handle(state->device_id, (device_addr_t *)addr, SOUND_ACL_DISCONNECTED, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_bredr_acl_state_handle(state->device_id, (device_addr_t *)addr, SOUND_ACL_CONNECTING, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING_CANCELED:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_bredr_acl_state_handle(state->device_id, (device_addr_t *)addr, SOUND_ACL_DISCONNECTED, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTING_FAILURE:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_bredr_acl_state_handle(state->device_id, (device_addr_t *)addr, SOUND_ACL_DISCONNECTED, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_CONNECTED:
            break;
        case IBRT_CONN_ACL_PROFILES_CONNECTED:
            break;
        case IBRT_CONN_ACL_AUTH_COMPLETE:
#ifdef APP_SOUND_UI_ENABLE
            app_sound_bredr_acl_state_handle(state->device_id, (device_addr_t *)addr, SOUND_ACL_CONNECTED, reason_code);
#endif
            break;
        case IBRT_CONN_ACL_DISCONNECTING:
            break;
        case IBRT_CONN_ACL_UNKNOWN:
            break;
        default:
            break;
    }
}

void soundBtScoStateChanged(const bt_bdaddr_t *addr, ibrt_sco_conn_state_event *state, uint8_t reason_code)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x]ScoStatus changed = %d with reason 0x%x",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->state.sco_state, reason_code);
    } else {
        LOG_D("Custom:ScoState changed = %d with reason 0x%x", state->state.sco_state, reason_code);
    }

    switch (state->state.sco_state) {
        case IBRT_CONN_SCO_CONNECTED:
            break;
        case IBRT_CONN_SCO_DISCONNECTED:
            break;
        default:
            break;
    }
}

void soundBtIbrtRoleSwitchStatusChanged(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role)
{
    if (addr != NULL) {
        LOG_D("Custom:[%02x:%02x:*:*:*:%02x]ibrt role switch changed = %d with role 0x%x",
              addr->address[0], addr->address[1], addr->address[5], state, role);
    } else {
        LOG_D("Custom:ibrt role switch changed = %d with role 0x%x addr is NULL", state, role);
    }

    switch (state) {
        // this msg will be notified if initiate role switch
        case IBRT_CONN_ROLE_SWAP_INITIATED:
            break;
        // role switch complete,both sides will receice this msg
        case IBRT_CONN_ROLE_SWAP_COMPLETE:
            break;
        // nv role changed
        case IBRT_CONN_ROLE_CHANGE_COMPLETE:
            break;
        default:
            break;
    }
}

void soundBtIbrtStateChanged(const bt_bdaddr_t *addr, ibrt_connection_state_event *state, ibrt_role_e role, uint8_t reason_code)
{
    if (addr != NULL) {
        LOG_D("Custom:[d%x][%02x:%02x:*:*:*:%02x] ibrt_state_changed = %d with reason 0x%x role=%d",
              state->device_id, addr->address[0], addr->address[1], addr->address[5], state->state.ibrt_state, reason_code, role);
    } else {
        LOG_D("Custom:[d%x] ibrt_state_changed = %d with reason 0x%x role=%d addr is NULL",
              state->device_id, state->state.ibrt_state, reason_code, role);
    }

    switch (state->state.ibrt_state) {
        case IBRT_CONN_IBRT_DISCONNECTED:
            break;
        case IBRT_CONN_IBRT_CONNECTED:
            break;
        case IBRT_CONN_IBRT_START_FAIL:
            break;
        case IBRT_CONN_IBRT_ACL_CONNECTED:
            break;
        default:
            break;
    }
}

void soundBtAccessModeChanged(btif_accessible_mode_t accessMode)
{
    LOG_D("Custom:AccessMode changed=%d", accessMode);

    switch (accessMode) {
        case BTIF_BAM_NOT_ACCESSIBLE:
            break;
        case BTIF_BAM_GENERAL_ACCESSIBLE:
            break;
        case BTIF_BAM_LIMITED_ACCESSIBLE:
            break;
        case BTIF_BAM_CONNECTABLE_ONLY:
            break;
        case BTIF_BAM_DISCOVERABLE_ONLY:
            break;
        default:
            break;
    }
}

bool soundBtIncomingConnReqCallback(const bt_bdaddr_t *addr)
{
    if (addr == NULL) {
        LOG_D("Custom:Incoming_conn_req addr is NULL, reject");
        return false;
    }
    LOG_D("Custom:[%02x:%02x:*:*:*:%02x]Incoming_conn_req",
          addr->address[0], addr->address[1], addr->address[5]);
    return true;
}

bool soundBtExtraIncomingConnReqCallback(const bt_bdaddr_t *incoming_addr, bt_bdaddr_t *steal_addr)
{
    if (incoming_addr == NULL) {
        LOG_D("Custom:Extra_incoming_conn_req addr is NULL, reject");
        return false;
    }
    LOG_D("Custom:[%02x:%02x:*:*:*:%02x]Extra_incoming_conn_req",
          incoming_addr->address[0], incoming_addr->address[1], incoming_addr->address[5]);
    return true;
}

bool soundDisallowReconnectCallback(const bt_bdaddr_t* addr, const uint16_t active_event)
{
    LOG_D("Custom:Reconnect dev addr: %02x:%02x:*:*:*:%02x, event:0x%02x",
          addr->address[0], addr->address[1], addr->address[5], active_event);
    return false;
}

bool soundDisallowTwsRoleSwitchCallback()
{
    LOG_D("Custom:tws role switch ext-policy");
    return false;
}

static void soundEarbudEvtRunComplete(app_ui_evt_t evt)
{
    LOG_D("Earbud:evt=[%s] complete", app_ui_event_to_string(evt));

    switch (evt) {
        case APP_UI_EV_CASE_OPEN:
            break;
        case APP_UI_EV_CASE_CLOSE:
            break;
        case APP_UI_EV_TWS_PAIRING:
            break;
        case BEST_ALL_DISCONNECT_SIG:
            break;
        case APP_UI_EV_KEEP_ONLY_MOB_LINK:
        default:
            break;
    }
}

void soundBtPairingModeEntry()
{
    LOG_D("Custom:Pairing Entry");
}

void soundBtPairingModeExit()
{
    LOG_D("Custom:Pairing Exit");
}

void soundBtPairingModeTimeoutPre()
{
    LOG_D("Custom:Pairing timeout pre");
}

void soundBtPairingModeTimeoutPost()
{
    LOG_D("Custom:Pairing timeout post");
}

void soundBtTwsRoleSwitchCompleteCallback(TWS_UI_ROLE_E current_role, uint8_t errCode)
{
    LOG_D("Custom:TWS Role switch complete, role=%d, errorCode=%d", current_role, errCode);
}

void soundBtPeerBoxStateUpdateCallback(bud_box_state box_state)
{
    LOG_D("Custom:peer box state update=%s", app_ui_box_state_to_string(box_state));
}

void soundBtVenderEventCallback(uint8_t evt_type, uint8_t *buffer, uint8_t length)
{
    uint8_t subcode = evt_type;

    LOG_D("Custom:Vender event, subcode=%d", subcode);

    switch (subcode) {
        case HCI_DBG_SNIFFER_INIT_CMP_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE:
            break;

        case HCI_DBG_IBRT_SWITCH_COMPLETE_EVT_SUBCODE:
            break;

        case HCI_NOTIFY_CURRENT_ADDR_EVT_CODE:
            break;

        case HCI_DBG_TRACE_WARNING_EVT_CODE:
            break;

        case HCI_SCO_SNIFFER_STATUS_EVT_CODE:
            break;

        case HCI_DBG_RX_SEQ_ERROR_EVT_SUBCODE:
            break;

        case HCI_LL_MONITOR_EVT_CODE:
            break;

        case HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE:
            break;

        default:
            break;
    }
}

static ibrt_link_status_changed_cb_t link_status_cbs = {
    .ibrt_global_state_changed                  = soundBtGlobalStateCallback,
    .ibrt_a2dp_state_changed                    = soundBtA2dpCallback,
    .ibrt_hfp_state_changed                     = soundBtHfpCallback,
    .ibrt_avrcp_state_changed                   = soundBtAvrcpCallback,
    .ibrt_tws_pairing_changed                   = soundBtTwsParingStateChanged,
    .ibrt_tws_acl_state_changed                 = soundBtTwsAclStateChanged,
    .ibrt_mobile_acl_state_changed              = soundBtMobileAclStateChanged,
    .ibrt_sco_state_changed                     = soundBtScoStateChanged,
    .ibrt_tws_role_switch_status_ind            = soundBtIbrtRoleSwitchStatusChanged,
    .ibrt_ibrt_state_changed                    = soundBtIbrtStateChanged,
    .ibrt_access_mode_changed                   = soundBtAccessModeChanged,
};

static ibrt_mgr_status_changed_cb_t mgr_status_cbs = {
    .ibrt_mgr_event_run_comp_hook               = soundEarbudEvtRunComplete,
    .ibrt_mgr_pairing_mode_entry_hook           = soundBtPairingModeEntry,
    .ibrt_mgr_pairing_mode_exit_hook            = soundBtPairingModeExit,
    .ibrt_mgr_pairing_mode_timeout_pre_hook     = soundBtPairingModeTimeoutPre,
    .ibrt_mgr_pairing_mode_timeout_post_hook    = soundBtPairingModeTimeoutPost,
    .ibrt_mgr_tws_role_switch_comp_hook         = soundBtTwsRoleSwitchCompleteCallback,
    .peer_box_state_update_hook                 = soundBtPeerBoxStateUpdateCallback,
};

static ibrt_ext_conn_policy_cb_t conn_policy_cbs = {
    .accept_incoming_conn_req_hook              = soundBtIncomingConnReqCallback,
    .accept_extra_incoming_conn_req_hook        = soundBtExtraIncomingConnReqCallback,
    .disallow_start_reconnect_hook              = soundDisallowReconnectCallback,
    .disallow_tws_role_switch_hook              = soundDisallowTwsRoleSwitchCallback,
};

#if BLE_AUDIO_ENABLED
static void soundLeaAdvStateChanged(AOB_ADV_STATE_T state, uint8_t err_code)
{
    LOG_D("Custom:LEA:adv state changed=%d", state);
    switch (state) {
        case AOB_ADV_START:
            break;
        case AOB_ADV_STOP:
            break;
        case AOB_ADV_FAILED:
            break;
        default:
            break;
    }
}

static void soundLeaMobAclStateChanged(uint8_t conidx, const ble_bdaddr_t *addr, AOB_ACL_STATE_T state, uint8_t errCode)
{
    LOG_D("Custom:LEA:[%d][%02x:%02x:*:*:*:%02x]Acl state changed=%d, errcode=0x%x",
          conidx, addr->addr[0], addr->addr[1], addr->addr[5], state, errCode);
    static bool first_bond = true;

    switch (state) {
        case AOB_ACL_DISCONNECTED:
            first_bond = true;
            break;
        case AOB_ACL_CONNECTING:
            break;
        case AOB_ACL_FAILED:
            break;
        case AOB_ACL_CONNECTED:
            break;
        /* report bond success after encrypt success, so will not report encrypt success later */
        case AOB_ACL_BOND_SUCCESS:
            break;
        case AOB_ACL_BOND_FAILURE:
            break;
        case AOB_ACL_ENCRYPT:
            break;
        case AOB_ACL_ATTR_BOND:
            if (first_bond) {
                TRACE(0, "LE AUDIO ATTR BOND!!!");
                first_bond = false;
                app_ui_keep_only_the_leaudio_device(conidx);
            }
            break;
        case AOB_ACL_DISCONNECTING:
            break;
    }
}

static void soundLeaVolChanged(uint8_t con_lid, uint8_t volume, uint8_t mute)
{
    LOG_D("Custom:LEA:[%d] vol=%d, mute=%d", con_lid, volume, mute);
}

static void soundLeaVocsOffsetChanged(int16_t offset, uint8_t output_lid)
{
    LOG_D("Custom:LEA:vocs offset=%d, output_lid=%d", offset, output_lid);
}

static void soundLeaVocsBondDataChanged(uint8_t output_lid, uint8_t cli_cfg_bf)
{
    LOG_D("Custom:LEA:vocs bond data=%d, output_lid=%d", cli_cfg_bf, output_lid);
}

static void soundLeaMediaTrackChanged(uint8_t con_lid)
{
    LOG_D("Custom:LEA:mob[%d] media trace changed", con_lid);
}

static void soundLeaMediaStreamStatusChanged(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state)
{
    LOG_D("Custom:LEA:mob[%d] ase_id=%d, stream_state=%d", con_lid, ase_lid, state);
}

static void soundLeaMediaPlaybackStatusChanged(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state)
{
    LOG_D("Custom:LEA:mob[%d] media_pb_status_changed=%d", con_lid, state);

    switch (state) {
        case AOB_MGR_PLAYBACK_STATE_INACTIVE:
            break;
        case AOB_MGR_PLAYBACK_STATE_PLAYING:
            break;
        case AOB_MGR_PLAYBACK_STATE_PAUSED:
            break;
        case AOB_MGR_PLAYBACK_STATE_SEEKING:
            break;
        default:
            break;
    }
}

static void soundLeaMediaMicStateChanged(uint8_t mute)
{
    LOG_D("Custom:LEA:media mic mute=%d", mute);
}

static void soundLeaMediaIsoLinkQualityChanged(AOB_ISO_LINK_QUALITY_INFO_T param)
{
    LOG_D("Custom:LEA:Iso link quality");
}

static void soundLeaMediaPacsCccdWrittenChanged(uint8_t con_lid)
{
    LOG_D("Custom:LEA:mob[%d] pacs cccd writen", con_lid);
}

static void soundLeaCallStateChanged(uint8_t con_lid, void *param)
{
    AOB_SINGLE_CALL_INFO_T *call_state_changed = (AOB_SINGLE_CALL_INFO_T *)param;

    LOG_D("Custom:LEA:mob[%d] call state changed=%d", con_lid, call_state_changed->state);

    switch (call_state_changed->state) {
        case AOB_CALL_STATE_INCOMING:
            break;
        case AOB_CALL_STATE_DIALING:
            break;
        case AOB_CALL_STATE_ALERTING:
            break;
        case AOB_CALL_STATE_ACTIVE:
            break;
        case AOB_CALL_STATE_LOCAL_HELD:
            break;
        case AOB_CALL_STATE_REMOTE_HELD:
            break;
        case AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD:
            break;
        case AOB_CALL_STATE_IDLE:
            break;
        default:
            break;
    }
}

static void soundLeaCallSrvSignalStrengthValueChanged(uint8_t con_lid, uint8_t value)
{
    LOG_D("Custom:LEA:mob[%d] call srv signal strength value=%d", con_lid, value);
}

static void soundLeaCallStatusFlagsChanged(uint8_t con_lid, bool inband_ring, bool silent_mode)
{
    LOG_D("Custom:LEA:mob[%d] call status, inband_ring=%d, slient_mode=%d", con_lid, inband_ring, silent_mode);
}

static void soundLea_call_ccp_opt_supported_opcode_ind_cb(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported)
{
    LOG_D("Custom:LEA:mob[%d] call ccp opt local_hold_op_supported=%d, join_op_supported=%d", con_lid, local_hold_op_supported, join_op_supported);
}

static void soundLeaCallTerminateReasonChanged(uint8_t con_lid, uint8_t call_id, uint8_t reason)
{
    LOG_D("Custom:LEA:mob[%d] call[%d] terminate reason=%d", con_lid, call_id, reason);
}

static void soundLeaCallIncomingNumberInfChanged(uint8_t con_lid, uint8_t url_len, uint8_t *url)
{
    LOG_D("Custom:LEA:mob[%d] call incoming number inf", con_lid);
}

static void soundLeaAobCallSvcChanged(uint8_t con_lid)
{
    LOG_D("Custom:LEA:mob[%d] AOB call svc changed", con_lid);
}

static void soundLeaCallActionResultChanged(uint8_t con_lid, void *param)
{
    LOG_D("Custom:LEA:mob[%d] call action result changed", con_lid);
}

static app_ble_audio_event_cb_t custom_ui_le_audio_event_cb = {
    .ble_audio_adv_state_changed                = soundLeaAdvStateChanged,
    .mob_acl_state_changed                      = soundLeaMobAclStateChanged,
    .vol_changed_cb                             = soundLeaVolChanged,
    .vocs_offset_changed_cb                     = soundLeaVocsOffsetChanged,
    .vocs_bond_data_changed_cb                  = soundLeaVocsBondDataChanged,
    .media_track_change_cb                      = soundLeaMediaTrackChanged,
    .media_stream_status_change_cb              = soundLeaMediaStreamStatusChanged,
    .media_playback_status_change_cb            = soundLeaMediaPlaybackStatusChanged,
    .media_mic_state_cb                         = soundLeaMediaMicStateChanged,
    .media_iso_link_quality_cb                  = soundLeaMediaIsoLinkQualityChanged,
    .media_pacs_cccd_written_cb                 = soundLeaMediaPacsCccdWrittenChanged,
    .call_state_change_cb                       = soundLeaCallStateChanged,
    .call_srv_signal_strength_value_ind_cb      = soundLeaCallSrvSignalStrengthValueChanged,
    .call_status_flags_ind_cb                   = soundLeaCallStatusFlagsChanged,
    .call_ccp_opt_supported_opcode_ind_cb       = soundLea_call_ccp_opt_supported_opcode_ind_cb,
    .call_terminate_reason_ind_cb               = soundLeaCallTerminateReasonChanged,
    .call_incoming_number_inf_ind_cb            = soundLeaCallIncomingNumberInfChanged,
    .call_svc_changed_ind_cb                    = soundLeaAobCallSvcChanged,
    .call_action_result_ind_cb                  = soundLeaCallActionResultChanged,
};

#endif /* #if BLE_AUDIO_ENABLED  */

void soundBtNewlyPairedDeviceCallback(const uint8_t *addr)
{
    bt_bdaddr_t btAddr;
    memcpy(&btAddr, addr, sizeof(btAddr));

    if (soundIsTwsAddress(&btAddr)) {
        soundBtNewlyPairedTwsCallback(&btAddr);
    } else {
        soundBtNewlyPairedMobileCallback(&btAddr);
    }
}

static void soundRegisterEarbudCalbacks(void)
{
    app_avrcp_reg_media_info_report_callback(soundBtAvrcpMediaInfoCallback);

    app_ui_register_link_status_callback(&link_status_cbs);

    app_ui_register_mgr_status_callback(&mgr_status_cbs);

    app_ui_register_ext_conn_policy_callback(&conn_policy_cbs);

    nv_record_bt_device_register_newly_paired_device_callback(NEW_DEVICE_CB_USER1, soundBtNewlyPairedDeviceCallback);
}

void soundCustomConfig(void)
{
    app_ui_config_t config;
    SoundEventCallback event_callback = {0};

    // zero init the config
    memset(&config, 0, sizeof(app_ui_config_t));

    // freeman mode config, default should be false
    config.freeman_enable                                   = true;

    // enable sdk lea adv strategy, default should be true
    config.sdk_lea_adv_enable                               = false;

    // pairing mode timeout value config
    config.pairing_timeout_value                            = IBRT_UI_DISABLE_BT_SCAN_TIMEOUT;

    // SDK pairing enable, the default should be true
    config.sdk_pairing_enable                               = true;

    // passive enter pairing when no mobile record, the default should be false
    config.enter_pairing_on_empty_record                    = false;

    // passive enter pairing when reconnect failed, the default should be false
    config.enter_pairing_on_reconnect_mobile_failed         = false;

    // passive enter pairing when mobile disconnect, the default should be false
    config.enter_pairing_on_mobile_disconnect               = false;

    // exit pairing when peer close, the default should be true
    config.exit_pairing_on_peer_close                       = true;

    // exit pairing when a2dp or hfp streaming, the default should be true
    config.exit_pairing_on_streaming                        = true;

    // disconnect remote devices when entering pairing mode actively
    config.paring_with_disc_mob_num                         = IBRT_PAIRING_DISC_ALL_MOB;

    // disconnect SCO or not when accepting phone connection in pairing mode
    config.pairing_with_disc_hf_cfg                         = IBRT_PAIRING_DISC_SCO;

    // not start ibrt in pairing mode, the default should be false
    config.pairing_without_start_ibrt                       = false;

    // disconnect tws immediately when single bud closed in box
    config.disc_tws_imm_on_single_bud_closed                = false;

    // reverse the bt device priority, the latest connected device has high priority by default
    config.bt_device_priority_reverse                       = false;

    // do tws switch when RSII value change, default should be true
    config.tws_switch_according_to_rssi_value               = false;

    // do tws switch when RSII value change, timer threshold
    config.role_switch_timer_threshold                      = IBRT_UI_ROLE_SWITCH_TIME_THRESHOLD;

    // do tws switch when rssi value change over threshold
    config.rssi_threshold                                   = IBRT_UI_ROLE_SWITCH_THRESHOLD_WITH_RSSI;

    // close box delay disconnect tws timeout
    config.close_box_delay_tws_disc_timeout                 = 0;

    // close box debounce time config
    config.close_box_event_wait_response_timeout            = IBRT_UI_CLOSE_BOX_EVENT_WAIT_RESPONSE_TIMEOUT;

    // wait time before launch reconnect event
    config.reconnect_mobile_wait_response_timeout           = IBRT_UI_RECONNECT_MOBILE_WAIT_RESPONSE_TIMEOUT;

    // reconnect event internal config wait timer when tws disconnect
    config.reconnect_wait_ready_timeout                     = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;

    config.tws_conn_failed_wait_time                        = TWS_CONN_FAILED_WAIT_TIME;

    config.reconnect_mobile_wait_ready_timeout              = IBRT_UI_MOBILE_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_tws_wait_ready_timeout                 = IBRT_UI_TWS_RECONNECT_WAIT_READY_TIMEOUT;
    config.reconnect_ibrt_wait_response_timeout             = IBRT_UI_RECONNECT_IBRT_WAIT_RESPONSE_TIMEOUT;
    config.nv_master_reconnect_tws_wait_response_timeout    = IBRT_UI_NV_MASTER_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;
    config.nv_slave_reconnect_tws_wait_response_timeout     = IBRT_UI_NV_SLAVE_RECONNECT_TWS_WAIT_RESPONSE_TIMEOUT;

    config.check_plugin_excute_closedbox_event              = true;
    config.ibrt_with_ai                                     = false;
    config.giveup_reconn_when_peer_unpaired                 = true;
    config.no_profile_stop_ibrt                             = true;

    // if tws&mobile disc, reconnect tws first until tws connected then reconn mobile
    config.delay_reconn_mob_until_tws_connected             = false;

    config.delay_reconn_mob_max_times                       = IBRT_UI_DELAY_RECONN_MOBILE_MAX_TIMES;

    // open box reconnect mobile times config
    config.open_reconnect_mobile_max_times                  = IBRT_UI_OPEN_RECONNECT_MOBILE_MAX_TIMES;

    // open box reconnect tws times config
    config.open_reconnect_tws_max_times                     = IBRT_UI_OPEN_RECONNECT_TWS_MAX_TIMES;

    // connection timeout reconnect mobile times config
    config.reconnect_mobile_max_times                       = IBRT_UI_RECONNECT_MOBILE_MAX_TIMES;

    // connection timeout reconnect tws times config
    config.reconnect_tws_max_times                          = IBRT_UI_RECONNECT_TWS_MAX_TIMES;

    // connection timeout reconnect ibrt times config
    config.reconnect_ibrt_max_times                         = IBRT_UI_RECONNECT_IBRT_MAX_TIMES;

    // controller basband monitor
    config.lowlayer_monitor_enable                          = true;
    config.llmonitor_report_format                          = REP_FORMAT_PACKET;
    config.llmonitor_report_count                           = 1000;
    //  config.llmonitor_report_format                      = REP_FORMAT_TIME;
    //  config.llmonitor_report_count                       = 1600;////625us uint

    config.mobile_page_timeout                              = IBRT_MOBILE_PAGE_TIMEOUT;
    // tws connection supervision timeout
    config.tws_connection_timeout                           = IBRT_UI_TWS_CONNECTION_TIMEOUT;

    config.rx_seq_error_timeout                             = IBRT_UI_RX_SEQ_ERROR_TIMEOUT;
    config.rx_seq_error_threshold                           = IBRT_UI_RX_SEQ_ERROR_THRESHOLD;
    config.rx_seq_recover_wait_timeout                      = IBRT_UI_RX_SEQ_ERROR_RECOVER_TIMEOUT;

    config.rssi_monitor_timeout                             = IBRT_UI_RSSI_MONITOR_TIMEOUT;

    config.radical_scan_interval_nv_slave                   = IBRT_UI_RADICAL_SAN_INTERVAL_NV_SLAVE;
    config.radical_scan_interval_nv_master                  = IBRT_UI_RADICAL_SAN_INTERVAL_NV_MASTER;

    config.scan_interval_in_sco_tws_disconnected            = IBRT_UI_SCAN_INTERVAL_IN_SCO_TWS_DISCONNECTED;
    config.scan_window_in_sco_tws_disconnected              = IBRT_UI_SCAN_WINDOW_IN_SCO_TWS_DISCONNECTED;

    config.scan_interval_in_sco_tws_connected               = IBRT_UI_SCAN_INTERVAL_IN_SCO_TWS_CONNECTED;
    config.scan_window_in_sco_tws_connected                 = IBRT_UI_SCAN_WINDOW_IN_SCO_TWS_CONNECTED;

    config.scan_interval_in_a2dp_tws_disconnected           = IBRT_UI_SCAN_INTERVAL_IN_A2DP_TWS_DISCONNECTED;
    config.scan_window_in_a2dp_tws_disconnected             = IBRT_UI_SCAN_WINDOW_IN_A2DP_TWS_DISCONNECTED;

    config.scan_interval_in_a2dp_tws_connected              = IBRT_UI_SCAN_INTERVAL_IN_A2DP_TWS_CONNECTED;
    config.scan_window_in_a2dp_tws_connected                = IBRT_UI_SCAN_WINDOW_IN_A2DP_TWS_CONNECTED;

    config.connect_no_03_timeout                            = CONNECT_NO_03_TIMEOUT;
    config.disconnect_no_05_timeout                         = DISCONNECT_NO_05_TIMEOUT;

    config.tws_switch_tx_data_protect                       = true;

    config.tws_cmd_send_timeout                             = IBRT_UI_TWS_CMD_SEND_TIMEOUT;
    config.tws_cmd_send_counter_threshold                   = IBRT_UI_TWS_COUNTER_THRESHOLD;

    config.profile_concurrency_supported                    = true;

    config.audio_sync_mismatch_resume_version               = 2;

    config.support_steal_connection                         = true;
    config.support_steal_connection_in_sco                  = false;
    config.support_steal_connection_in_a2dp_steaming        = true;
    config.steal_audio_inactive_device                      = false;

    config.allow_sniff_in_sco                               = false;

    config.always_interlaced_scan                           = true;

    config.disallow_reconnect_in_streaming_state            = false;

    config.open_without_auto_reconnect                      = false;

    config.without_reconnect_when_fetch_out_wear_up         = false;

    config.disallow_rs_by_box_state                         = true;

    config.only_rs_when_sco_active                          = false;

    config.reject_same_cod_device_conn_req                  = true;

    config.is_changed_to_ui_master_on_tws_disconnected      = false;

    config.connected_max_device_num_allow_new_connect       = true;

    config.lea_connected_allow_new_connect                  = true;

    config.dev_idle_allow_nonsupport_stay_connected       = true;
    app_ui_register_vender_event_update_ind(soundBtVenderEventCallback);
    app_ui_reconfig_env(&config);

    event_callback.modeReadyFunc      = soundModeChanged;
    event_callback.evtRunCompleteFunc = soundEvtRunComplete;
    event_callback.bisSetParamFunc    = soundBisSetParam;
    event_callback.bisStreamStateChangeFunc   = soundBisStreamStateChangeFunc;
    event_callback.bisSinkReceiveMfrsDataFunc = soundBisSinkReceiveManufacturerDataFunc;
    soundRegisterEvtCallback(event_callback);
#if BLE_AUDIO_ENABLED
    app_ui_register_custom_ui_le_aud_callback(&custom_ui_le_audio_event_cb);
#endif
}

INIT_APP_SOUND_EXPORT(soundCustomConfig);
