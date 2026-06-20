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
#ifndef __BLE_AI_H__
#define __BLE_AI_H__
#ifdef __AI_VOICE_BLE_ENABLE__
#define CFG_AI_VOICE
#include "gatt_service.h"
#include "ble_core_common.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BLE_AI_CMD_TYPE 0
#define BLE_AI_DATA_TYPE 1
#define BLE_AI_ANY_TYPE 2

void ble_ai_mtu_exchanged(uint8_t conidx, uint16_t connhdl, uint16_t mtu);
void ble_ai_disconnected(uint8_t conidx, uint16_t connhdl);
void ble_ai_report_connected(uint8_t ai_type, uint8_t conidx, uint16_t connhdl);
void ble_ai_report_disconnected(uint8_t ai_type, uint8_t conidx, uint16_t connhdl);
void ble_ai_report_data_tx_done(uint8_t ai_type, uint8_t conidx, uint16_t connhdl, uint8_t data_type);
void ble_ai_report_data_received(uint8_t ai_type, uint32_t connhdl_conidx, uint8_t data_type, gatt_server_char_write_t *p);
void ble_ai_report_cccd_config_changed(uint8_t ai_type, uint32_t connhdl_conidx, uint8_t data_type, uint8_t ntf_ind_flag);

typedef enum{
    /// Server initiated notification
    APP_BLE_AI_GATT_NOTIFY     = 0x00,
    /// Server initiated indication
    APP_BLE_AI_GATT_INDICATE   = 0x01,
} APP_BLE_AI_SEND_EVENT_TYPE_E;

typedef enum{
    /// Server initiated notification
    APP_BLE_AI_CMD     = 0x00,
    /// Server initiated indication
    APP_BLE_AI_DATA    = 0x01,
} APP_BLE_AI_DATA_TYPE_E;

typedef enum{
    APP_BLE_AI_SPEC_AMA     = 0x00,
    APP_BLE_AI_SPEC_DMA,
    APP_BLE_AI_SPEC_GMA,
    APP_BLE_AI_SPEC_SMART,
    APP_BLE_AI_SPEC_TENCENT,
    APP_BLE_AI_SPEC_RECORDING,
    APP_BLE_AI_SPEC_CUSTOMIZE,

    APP_BLE_AI_SPEC_MAX,
} APP_BLE_AI_TYPE_E;

typedef enum{
    APP_BLE_AI_SVC_ADD_DONE = 0x00,
    APP_BLE_AI_CONN,
    APP_BLE_AI_DISCONN,
    APP_BLE_AI_MTU_CHANGE,
    APP_BLE_AI_TX_DONE_EVENT,
    APP_BLE_AI_RECEIVED_EVENT,
    APP_BLE_AI_CHANGE_CCC_EVENT,
} APP_BLE_AI_EVENT_TYPE_E;

typedef struct app_ble_ai_data_send_param
{
    /// ai type see@APP_BLE_AI_TYPE_E
    uint8_t  ai_type;
    /// Connection index
    uint8_t  conidx;
    /// Connection handle
    uint16_t connhdl;
    /// gatt event type, see@APP_BLE_AI_SEND_EVENT_TYPE_E
    uint8_t  gatt_event_type;
    /// send data type. see@APP_BLE_AI_DATA_TYPE_E
    uint8_t  data_type;
    uint32_t data_len;
    uint8_t  *data;
}app_ble_ai_data_send_param_t;

typedef struct app_ble_ai_event_param
{
    /// ai type see@APP_BLE_AI_TYPE_E
    uint8_t  ai_type;
    /// gatt event type, see@APP_BLE_AI_EVENT_TYPE_E
    uint8_t  event_type;
    /// connection index
    uint8_t  conidx;
    /// connection handle
    uint16_t connhdl;
    union {
         /// APP_BLE_AI_MTU_CHANGE
        uint16_t mtu;
        /// APP_BLE_AI_SVC_ADD_DONE
        struct {
            uint16_t star_hdl;
            uint16_t att_num;
        } svc_add_done;
        /// APP_BLE_AI_RECEIVED_EVENT
        struct {
            uint8_t  data_type;
            uint32_t data_len;
            uint8_t  *data;
        } received;
        /// APP_BLE_AI_CHANGE_CCC_EVENT
        struct {
            uint8_t data_type;
            uint8_t ntf_ind_flag;
        } change_ccc;
    }data;
} app_ble_ai_event_param_t;

typedef void(* app_ble_ai_event_cb)(app_ble_ai_event_param_t *param);
void app_ai_data_send(app_ble_ai_data_send_param_t *param);
void app_ai_event_reg(app_ble_ai_event_cb cb);

#if defined(__AMA_VOICE__)
void ble_ai_ama_init(void);
void ble_ai_ama_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(__DMA_VOICE__)
void ble_ai_dma_init(void);
void ble_ai_dma_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(__GMA_VOICE__)
void ble_ai_gma_init(void);
void ble_ai_gma_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(__SMART_VOICE__)
void ble_ai_smart_voice_init(void);
void ble_ai_smart_voice_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(__TENCENT_VOICE__)
void ble_ai_tencent_voice_init(void);
void ble_ai_tencent_voice_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(DUAL_MIC_RECORDING)
void ble_ai_recording_init(void);
void ble_ai_recording_send_data(app_ble_ai_data_send_param_t *param);
#endif

#if defined(__CUSTOMIZE_VOICE__)
void ble_ai_customize_init(void);
void ble_ai_customize_send_data(app_ble_ai_data_send_param_t *sparam);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __AI_VOICE_BLE_ENABLE__ */
#endif /* __BLE_AI_H__ */