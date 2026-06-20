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
#ifndef __BES_AI_API_H__
#define __BES_AI_API_H__

#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#ifdef __AI_VOICE_BLE_ENABLE__
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
typedef enum{
    /// Server initiated notification
    BES_BLE_AI_GATT_NOTIFY     = 0x00,
    /// Server initiated indication
    BES_BLE_AI_GATT_INDICATE   = 0x01,
} BES_BLE_AI_SEND_EVENT_TYPE_E;

typedef enum{
    /// Server initiated notification
    BES_BLE_AI_CMD     = 0x00,
    /// Server initiated indication
    BES_BLE_AI_DATA    = 0x01,
    /// Does not distinguish the data types of CMD and DATA
    BES_BLE_AI_ANY     = 0x02,
} BES_BLE_AI_DATA_TYPE_E;

typedef enum{
    BES_BLE_AI_SPEC_AMA     = 0x00,
    BES_BLE_AI_SPEC_DMA,
    BES_BLE_AI_SPEC_GMA,
    BES_BLE_AI_SPEC_SMART,
    BES_BLE_AI_SPEC_TENCENT,
    BES_BLE_AI_SPEC_RECORDING,
    BES_BLE_AI_SPEC_COMMON,
} BES_BLE_AI_TYPE_E;

typedef enum{
    BES_BLE_AI_SVC_ADD_DONE = 0x00,
    BES_BLE_AI_CONN,
    BES_BLE_AI_DISCONN,
    BES_BLE_AI_MTU_CHANGE,
    BES_BLE_AI_TX_DONE_EVENT,
    BES_BLE_AI_RECEIVED_EVENT,
    BES_BLE_AI_CHANGE_CCC_EVENT,
} BES_BLE_AI_EVENT_TYPE_E;

/*
 * DEFINES
 ****************************************************************************************
 */
typedef struct bes_ble_ai_data_send_param
{
    /// ai type see@BES_BLE_AI_TYPE_E
    uint8_t  ai_type;
    /// Connection index
    uint8_t  conidx;
    /// Connection handle
    uint16_t connhdl;
    /// gatt event type, see@BES_BLE_AI_SEND_EVENT_TYPE_E
    uint8_t  gatt_event_type;
    /// send data type. see@BES_BLE_AI_DATA_TYPE_E
    uint8_t  data_type;
    /// data lenth
    uint32_t data_len;
    /// data pointer
    uint8_t  *data;
}bes_ble_ai_data_send_param_t;

typedef struct bes_ble_ai_event_param
{
    /// ai type see@BES_BLE_AI_TYPE_E
    uint8_t  ai_type;
    /// gatt event type, see@BES_BLE_AI_EVENT_TYPE_E
    uint8_t  event_type;
    /// Connection index
    uint8_t  conidx;
    /// Connection handle
    uint16_t connhdl;
    union {
        /// BES_BLE_AI_MTU_CHANGE
        uint16_t mtu;
        /// BES_BLE_AI_SVC_ADD_DONE
        struct {
            uint16_t start_hdl;
            uint16_t att_num;
        } svc_add_done;
        /// BES_BLE_AI_RECEIVED_EVENT
        struct {
            uint8_t  data_type;
            uint32_t data_len;
            uint8_t  *data;
        } received;
        /// BES_BLE_AI_CHANGE_CCC_EVENT
        struct {
            uint8_t data_type;
            uint8_t ntf_ind_flag;
        } chnage_ccc;
    }data;
}bes_ble_ai_event_param_t;

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
typedef void(* bes_ble_ai_event_cb)(bes_ble_ai_event_param_t *param);

/**
 ****************************************************************************************
 *
 * AI Send Data Application Functions
 *
 ****************************************************************************************
 */
void bes_ble_ai_gatt_event_reg(bes_ble_ai_event_cb cb);

void bes_ble_ai_gatt_data_send(bes_ble_ai_data_send_param_t *param);

#endif /* BLE_AUDIO_ENABLED */

#ifdef __cplusplus
}
#endif
#endif /* BLE_HOST_SUPPORT */
#endif /* __BES_AOB_API_H__ */

