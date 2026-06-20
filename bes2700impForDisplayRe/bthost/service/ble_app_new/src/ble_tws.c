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
#include "app_ble.h"
#include "nvrecord_ble.h"
#include "bt_drv_interface.h"
#include "app_bt_sync.h"
#include "bluetooth_nv_mgr.h"

#if defined(IBRT)
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_custom_cmd.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"

#if IBRT_UI
#include "app_ui_evt_thread.h"
#endif

#if BLE_AUDIO_ENABLED
#include "aob_csip_api.h"
#include "aob_bis_api.h"
#include "aob_conn_api.h"
#include "aob_service_sync.h"
#include "gaf_media_sync.h"
#endif

#ifdef TWS_SYSTEM_ENABLED
static void ble_sync_info_prepare_handler(uint8_t *buf, uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
    uint16_t sent_len = 0;
    *totalLen = sizeof(NV_RECORD_PAIRED_BLE_DEV_INFO_T);
    uint8_t *pBleInfo = (uint8_t *)nv_record_blerec_get_ptr();
    uint16_t validLen = (uint16_t)TWS_SYNC_BUF_SIZE - OFFSETOF(TWS_SYNC_ENTRY_T, info) - OFFSETOF(TWS_SYNC_DATA_T, content) - 2;

    if (expectLen) {
        // contiune packet
        *len = expectLen < validLen ? expectLen : validLen;
    } else if (*totalLen <= validLen) {
        // send all info in one packet
        *len = *totalLen;
    } else {
        // the first packet 
        *len = validLen;
        expectLen = *totalLen;
    }
 
    sent_len = (*totalLen - expectLen);
    memcpy(buf, pBleInfo + sent_len, *len);
}

static void ble_sync_info_receive_continue_process(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    NV_RECORD_PAIRED_BLE_DEV_INFO_T *pReceivedBleInfo = (NV_RECORD_PAIRED_BLE_DEV_INFO_T *)buf;
    // Refresh basic info once
    nv_record_extension_update_tws_ble_info(pReceivedBleInfo);

    // for le audio case, no need to exchange ble pairing info
#if (BLE_AUDIO_ENABLED == 0)
    for (uint32_t index = 0; index < sizeof(pReceivedBleInfo->ble_nv)/sizeof(BleDeviceinfo); index++)
    {
        bluetooth_nv_mgr_ble_record_add(BLE_NV_REC_ADD_LE_SYNC_INFO_RECV, &pReceivedBleInfo->ble_nv[index].pairingInfo);
    }

    gap_get_local_irk();

    if (TWS_UI_SLAVE == app_ibrt_if_get_ui_role())
    {
        app_ble_add_devices_info_to_resolving();
    }
#endif
}

static void ble_sync_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    TRACE(2, "%s length:%d %d", __func__, length, isContinueInfo);
    ble_sync_info_receive_continue_process(buf, length, isContinueInfo);
}

static void ble_sync_info_rsp_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    TRACE(2, "%s length:%d %d", __func__, length, isContinueInfo);
    ble_sync_info_receive_continue_process(buf, length, isContinueInfo);
}

void app_ble_mode_tws_sync_init(void)
{
    TWS_SYNC_USER_T userBle =
    {
        ble_sync_info_prepare_handler,
        ble_sync_info_received_handler,
        ble_sync_info_prepare_handler,
        ble_sync_info_rsp_received_handler,
        NULL,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_BLE_INFO, &userBle);
}

void app_ble_sync_ble_info(void)
{
    app_ibrt_if_prepare_sync_info();
    app_ibrt_if_sync_info(TWS_SYNC_USER_BLE_INFO);
    app_ibrt_if_flush_sync_info();
}
#endif

void ble_roleswitch_start(void)
{
    TRACE(0, "%s", __func__);
#if (BLE_AUDIO_ENABLED == 0) && defined(IBRT)
    // disable adv after role switch start
    app_ble_force_switch_adv(BLE_SWITCH_USER_RS, false);
    if (TWS_UI_SLAVE != app_ibrt_if_get_ui_role())
    {
        app_ble_sync_ble_info();
    }
#endif
}

void ble_roleswitch_complete(uint8_t newRole)
{
#ifdef BLE_ADV_RPA_ENABLED
#else
    btif_me_set_ble_bd_address(bt_get_ble_local_address());
#endif

#if (BLE_AUDIO_ENABLED == 0) && defined(IBRT)
    TRACE(0, "%s newRole %d", __func__, newRole);
    app_ble_force_switch_adv(BLE_SWITCH_USER_RS, true);
    if (newRole == IBRT_SLAVE)
    {
        gap_terminate_all_ble_connection();
    }
#endif
}

void ble_role_update(uint8_t newRole)
{
#if defined(IBRT)
    TRACE(0, "%s newRole %d", __func__, newRole);
#if (BLE_AUDIO_ENABLED == 0)
    if (newRole == IBRT_SLAVE)
    {
        gap_terminate_all_ble_connection();
    }
    app_ble_refresh_adv_state_generic();
#endif
#endif
}

void ble_ibrt_event_entry(uint8_t ibrt_evt_type)
{
#if defined(IBRT) && defined(IBRT_UI)
    TRACE(0, "%s evt_type %d", __func__, ibrt_evt_type);
    if (APP_UI_EV_CASE_OPEN == ibrt_evt_type)
    {
        app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, true);
    }
    else if (APP_UI_EV_UNDOCK == ibrt_evt_type)
    {
        app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, true);
    }
    else if (APP_UI_EV_CASE_CLOSE == ibrt_evt_type)
    {
        // disconnect all of the BLE connections when box closed
        app_ble_disconnect_all();
        app_ble_force_switch_adv(BLE_SWITCH_USER_BOX, false);
    }
#endif
}

#if defined(BLE_USE_TWS_SYNC)

#if BLE_AUDIO_ENABLED && defined(IBRT_UI)
static void ble_audio_info_prepare_handler(uint8_t *buf, uint16_t *totalLen, uint16_t *len, uint16_t expectLen)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *info = nv_record_bleaudio_get_ptr();
    if (!info)
    {
        return;
    }

    *totalLen = *len = sizeof(NV_RECORD_BLE_AUDIO_DEV_INFO_T);
    memcpy(buf, (uint8_t *)info, *totalLen);

}

static void ble_audio_info_received_handler(uint8_t *buf, uint16_t length, bool isContinueInfo)
{
    NV_RECORD_BLE_AUDIO_DEV_INFO_T *pInfo = (NV_RECORD_BLE_AUDIO_DEV_INFO_T *)buf;
    nv_record_bleaudio_update_devinfo((uint8_t *)pInfo);
}

void ble_audio_tws_sync_init(void)
{
    TRACE(0, "ble_audio_tws_sync_init");
    TWS_SYNC_USER_T userLeAudio =
    {
        ble_audio_info_prepare_handler,
        ble_audio_info_received_handler,
        NULL,
        NULL,
        NULL,
    };

    app_ibrt_if_register_sync_user(TWS_SYNC_USER_LE_AUDIO, &userLeAudio);
}

int app_ble_tws_sync_send_cmd(APP_BLE_TWS_SYNC_CMD_CODE_E code, uint8_t high_priority, uint8_t *data, uint16_t data_len)
{
    int ret = 0;
    app_tws_cmd_code_e tws_cmd;

    tws_cmd = APP_TWS_CMD_EXCH_BLE_AUDIO_INFO + (code - BLE_TWS_SYNC_EXCH_BLE_AUDIO_INFO);
    ret = tws_ctrl_send_cmd(tws_cmd, data, data_len);
    return ret;
}

int app_ble_tws_sync_send_rsp(APP_BLE_TWS_SYNC_CMD_CODE_E code, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    int ret = 0;
    app_tws_cmd_code_e tws_cmd;

    tws_cmd = APP_TWS_CMD_EXCH_BLE_AUDIO_INFO + (code - BLE_TWS_SYNC_EXCH_BLE_AUDIO_INFO);
    ret = tws_ctrl_send_rsp(tws_cmd, rsp_seq, p_buff, length);
    return ret;
}
#endif

app_ble_tws_sync_volume_rec_callback volume_sync_receive_handler = NULL;
app_ble_tws_sync_volume_callback volume_sync_trigger_handler = NULL;
app_ble_tws_sync_volume_callback volume_sync_offset_trigger_handler = NULL;;

static void  app_ble_tws_sync_volume_handler(void)
{

    if (volume_sync_trigger_handler)
    {
        volume_sync_trigger_handler();
    }
}

static void  app_ble_tws_sync_volume_offset_handler(void)
{

    if (volume_sync_offset_trigger_handler)
    {
        volume_sync_offset_trigger_handler();
    }
}

APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_VOLUME, app_ble_tws_sync_volume_handler, NULL);
APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_VOLUME_OFFSET, app_ble_tws_sync_volume_offset_handler, NULL);

app_ble_tws_switch_focus_recv_callback switch_focus_recv_handler = NULL;
app_ble_tws_switch_focus_trigger_cb switch_focus_trigger_handler = NULL;
static void app_ble_tws_sync_switch_focus_handler(void)
{
    if(switch_focus_trigger_handler)
    {
        switch_focus_trigger_handler();
    }
}

static void app_ble_tws_sync_info_report(uint32_t opCode, uint8_t *buf, uint8_t len)
{
    switch (opCode)
    {
        case APP_BT_SYNC_OP_RETRIGGER:
            break;
        case APP_BT_SYNC_OP_VOLUME:
            if (volume_sync_receive_handler)
            {
                volume_sync_receive_handler(buf, len);
            }
            break;
        case APP_BT_SYNC_OP_VOLUME_OFFSET:
            break;
        case APP_BT_SYNC_OP_SWITCH:
            //if data need,restore data and when time arived use it.
            if(switch_focus_recv_handler)
            {
                switch_focus_recv_handler(buf, len);
            }
            break;
        default:
            break;
    }
}

APP_BT_SYNC_COMMAND_TO_ADD(APP_BT_SYNC_OP_SWITCH, app_ble_tws_sync_switch_focus_handler, NULL);
void app_ble_sync_focus_switch_register(app_ble_tws_switch_focus_recv_callback recv_cb,
                                             app_ble_tws_switch_focus_trigger_cb trigger_cb)
{
    switch_focus_recv_handler = recv_cb;
    switch_focus_trigger_handler = trigger_cb;
    app_bt_sync_register_report_info_callback(app_ble_tws_sync_info_report);
}

bool app_ble_sync_switch_focus_info(uint8_t *data, uint8_t data_len)
{
    return app_bt_sync_enable(APP_BT_SYNC_OP_SWITCH, data_len, data, false);
}


bool app_ble_tws_sync_volume_register(app_ble_tws_sync_volume_rec_callback receive_cb,
                                      app_ble_tws_sync_volume_callback trigger_cb,
                                      app_ble_tws_sync_volume_callback offset_trigger_cb)
{
    volume_sync_receive_handler        = receive_cb;
    volume_sync_trigger_handler        = trigger_cb;
    volume_sync_offset_trigger_handler = offset_trigger_cb;
    app_bt_sync_register_report_info_callback(app_ble_tws_sync_info_report);
    return 1;
}

bool app_ble_tws_sync_volume(uint8_t *data, uint8_t data_len)
{
    return app_bt_sync_enable(APP_BT_SYNC_OP_VOLUME, data_len, data, false);
}
#else

void ble_audio_tws_sync_init(void)
{

}

bool app_ble_tws_sync_volume_register(app_ble_tws_sync_volume_rec_callback receive_cb,
                                      app_ble_tws_sync_volume_callback trigger_cb,
                                      app_ble_tws_sync_volume_callback offset_trigger_cb)
{
    return 0;
}

bool app_ble_tws_sync_volume(uint8_t *data, uint8_t data_len)
{
    return 0;
}

int app_ble_tws_sync_send_cmd(APP_BLE_TWS_SYNC_CMD_CODE_E code, uint8_t high_priority, uint8_t *data, uint16_t data_len)
{
    return -1;
}

int app_ble_tws_sync_send_rsp(APP_BLE_TWS_SYNC_CMD_CODE_E code, uint16_t rsp_seq, uint8_t *p_buff, uint16_t length)
{
    return -1;
}

int app_ble_tws_sync_send_cmd_via_ble_register(app_ble_tws_cmd_send_via_ble_t cb)
{
    return -1;
}

#endif
#endif /* IBRT */
