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
#ifndef __BLE_DP_COMMON_H__
#define __BLE_DP_COMMON_H__
#include "ble_common_define.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

/// health thermometer application environment structure
typedef enum {
    /// datapath connect, para_p: uint8_t *conidx
    DP_CONN_DONE,
    /// datapath disconnect, para_p: uint8_t *conidx
    DP_DISCONN_DONE,
    /// change mtu done, para_p: app_dp_mtu_exchange_msg_t *
    DP_MTU_CHANGE_DONE,
    /// receive data , para_p:app_dp_rec_data_msg_t *
    DP_DATA_RECEIVED,
    /// tx done , para_p: NULL
    DP_TX_DONE,
}DP_EVENT_TYPE_E;

typedef struct {
    uint8_t conidx;
    uint16_t mtu;
}app_dp_mtu_exchange_msg_t;

typedef struct {
    uint8_t  *data;
    uint16_t data_len;
    uint8_t  conidx;
}app_dp_rec_data_msg_t;

typedef union{
    uint8_t connect_index;
    uint8_t disconnect_index;
    app_dp_mtu_exchange_msg_t dp_mtu_exchange;
    app_dp_rec_data_msg_t dp_recv_data;
}ble_if_app_dp_param_u;

typedef void(*app_datapath_event_cb)(DP_EVENT_TYPE_E event_type, ble_if_app_dp_param_u *para_p);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_DP_COMMON_H__ */
