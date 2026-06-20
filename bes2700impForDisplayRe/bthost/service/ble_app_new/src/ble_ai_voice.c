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
#ifdef __AI_VOICE_BLE_ENABLE__
#include "ble_ai_voice.h"

static app_ble_ai_event_cb event_callback = NULL;

void app_ai_data_send(app_ble_ai_data_send_param_t *param)
{
    switch (param->ai_type) {
#if defined(__AMA_VOICE__)
    case APP_BLE_AI_SPEC_AMA:
        ble_ai_ama_send_data(param);
        break;
#endif
#if defined(__DMA_VOICE__)
    case APP_BLE_AI_SPEC_DMA:
        ble_ai_dma_send_data(param);
        break;
#endif
#if defined(__GMA_VOICE__)
    case APP_BLE_AI_SPEC_GMA:
        ble_ai_gma_send_data(param);
        break;
#endif
#if defined(__SMART_VOICE__)
    case APP_BLE_AI_SPEC_SMART:
        ble_ai_smart_voice_send_data(param);
        break;
#endif
#if defined(__TENCENT_VOICE__)
    case APP_BLE_AI_SPEC_TENCENT:
        ble_ai_tencent_voice_send_data(param);
        break;
#endif
#if defined(DUAL_MIC_RECORDING)
    case APP_BLE_AI_SPEC_RECORDING:
        ble_ai_recording_send_data(param);
        break;
#endif
#if defined(__CUSTOMIZE_VOICE__)
    case APP_BLE_AI_SPEC_CUSTOMIZE:
        ble_ai_customize_send_data(param);
        break;
#endif
    default:
        TRACE(0, "%s[ERROR]: not find ai type=%d", __func__, param->ai_type);
        return;
    }
}

void app_ai_event_reg(app_ble_ai_event_cb cb)
{
    event_callback = cb;
}

void ble_ai_mtu_exchanged(uint8_t conidx, uint16_t connhdl, uint16_t mtu)
{
    if (event_callback)
    {
        app_ble_ai_event_param_t param;
        param.ai_type      = APP_BLE_AI_SPEC_MAX;
        param.event_type   = APP_BLE_AI_MTU_CHANGE;
        param.conidx       = conidx;
        param.connhdl    = connhdl;
        param.data.mtu     = mtu;
        event_callback(&param);
    }
}

void ble_ai_disconnected(uint8_t conidx, uint16_t connhdl)
{
    if(event_callback)
    {
        app_ble_ai_event_param_t param;
        param.ai_type    = APP_BLE_AI_SPEC_MAX;
        param.event_type = APP_BLE_AI_DISCONN;
        param.conidx     = conidx;
        param.connhdl    = connhdl;
        event_callback(&param);
    }
}

void ble_ai_report_connected(uint8_t ai_type, uint8_t conidx, uint16_t connhdl)
{
   if (event_callback)
   {
       app_ble_ai_event_param_t event_param = {0};
       event_param.ai_type    = ai_type;
       event_param.conidx     = conidx;
       event_param.connhdl    = connhdl;
       event_param.event_type = APP_BLE_AI_CONN;
       event_callback(&event_param);
   }
}

void ble_ai_report_disconnected(uint8_t ai_type, uint8_t conidx, uint16_t connhdl)
{
    if (event_callback)
    {
        app_ble_ai_event_param_t event_param = {0};
        event_param.ai_type    = ai_type;
        event_param.conidx     = conidx;
        event_param.connhdl    = connhdl;
        event_param.event_type = APP_BLE_AI_DISCONN;
        event_callback(&event_param);
    }
}

void ble_ai_report_data_tx_done(uint8_t ai_type, uint8_t conidx, uint16_t connhdl, uint8_t data_type)
{
    if (event_callback)
    {
        app_ble_ai_event_param_t event_param = {0};
        event_param.ai_type = ai_type;
        event_param.conidx  = conidx;
        event_param.connhdl  = connhdl;
        event_param.event_type = APP_BLE_AI_TX_DONE_EVENT;
        event_param.data.received.data_type = data_type;
        event_callback(&event_param);
    }
}

void ble_ai_report_data_received(uint8_t ai_type, uint32_t connhdl_conidx, uint8_t data_type, gatt_server_char_write_t *p)
{
    if (event_callback)
    {
        app_ble_ai_event_param_t event_param = {0};
        event_param.ai_type = ai_type;
        event_param.conidx  = (uint8_t)(connhdl_conidx & 0xff);
        event_param.connhdl  = (uint16_t)(connhdl_conidx >> 16);
        event_param.event_type = APP_BLE_AI_RECEIVED_EVENT;
        event_param.data.received.data_type = data_type ? APP_BLE_AI_DATA : APP_BLE_AI_CMD;
        event_param.data.received.data_len  = p->value_len;
        event_param.data.received.data      = (uint8_t *)p->value;
        event_callback(&event_param);
    }
}

void ble_ai_report_cccd_config_changed(uint8_t ai_type, uint32_t connhdl_conidx, uint8_t data_type, uint8_t ntf_ind_flag)
{
    if (event_callback)
    {
        app_ble_ai_event_param_t event_param = {0};
        event_param.ai_type = ai_type;
        event_param.conidx  = (uint8_t)(connhdl_conidx & 0xff);
        event_param.connhdl  = (uint16_t)(connhdl_conidx >> 16);
        event_param.event_type = APP_BLE_AI_CHANGE_CCC_EVENT;
        event_param.data.change_ccc.data_type    = data_type ? APP_BLE_AI_DATA : APP_BLE_AI_CMD;
        event_param.data.change_ccc.ntf_ind_flag = ntf_ind_flag;
        event_callback(&event_param);
    }
}

#endif /* __AI_VOICE_BLE_ENABLE__ */
