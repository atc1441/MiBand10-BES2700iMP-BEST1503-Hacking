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
#ifndef __BT_SPP_SERVICE_H__
#define __BT_SPP_SERVICE_H__
#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifndef SPP_SERVICE_NUM
#define SPP_SERVICE_NUM 5
#endif

#define SPP_DEVICE_NUM (SPP_SERVICE_NUM*BT_DEVICE_NUM)

#define BT_SPP_MAX_TX_MTU (L2CAP_CFG_MTU-RFCOMM_PPB_RESERVE)

#define BT_SPP_SERVER_PORT 0x01
#define BT_SPP_CLIENT_PORT 0x02

#define BT_SPP_MAX_CREDIT_STEP (4)
#define BT_SPP_CHANNEL_NUM_START (10)

typedef enum { // rfcomm channel number should be from 1 to 30, lower number 1 ~ 9 is reserved by bt stack
    RFCOMM_CHANNEL_10 = BT_SPP_CHANNEL_NUM_START,
    RFCOMM_CHANNEL_11,
    RFCOMM_CHANNEL_12,
    RFCOMM_CHANNEL_13,
    RFCOMM_CHANNEL_14,
    RFCOMM_CHANNEL_15,
    RFCOMM_CHANNEL_16,
    RFCOMM_CHANNEL_17,
    RFCOMM_CHANNEL_18,
    RFCOMM_CHANNEL_19,
    RFCOMM_CHANNEL_20,
    RFCOMM_CHANNEL_21,
    RFCOMM_CHANNEL_22,
    RFCOMM_CHANNEL_23,
    RFCOMM_CHANNEL_24,
    RFCOMM_CHANNEL_25,
    RFCOMM_CHANNEL_26,
    RFCOMM_CHANNEL_27,
    RFCOMM_CHANNEL_28,
    RFCOMM_CHANNEL_29,
    RFCOMM_CHANNEL_30,
} RFCOMM_CHANNEL_NUM_;

typedef enum {
    RFCOMM_CHANNEL_GS_CONTROL       = RFCOMM_CHANNEL_10,    // 0x00001000   => BTIF_APP_SPP_SERVER_START_ID
    RFCOMM_CHANNEL_GS_AUDIO         = RFCOMM_CHANNEL_11,    // 0x00002000
    RFCOMM_CHANNEL_TOTA             = RFCOMM_CHANNEL_12,    // 0x00004000
    RFCOMM_CHANNEL_BES_OTA          = RFCOMM_CHANNEL_13,    // 0x00008000
    RFCOMM_CHANNEL_AI_VOICE         = RFCOMM_CHANNEL_14,    // 0x00010000
    RFCOMM_CHANNEL_GREEN            = RFCOMM_CHANNEL_15,    // 0x00020000
    RFCOMM_CHANNEL_RED              = RFCOMM_CHANNEL_16,    // 0x00040000
    RFCOMM_CHANNEL_FP               = RFCOMM_CHANNEL_17,    // 0x00080000
    RFCOMM_CHANNEL_AMA              = RFCOMM_CHANNEL_18,    // 0x00100000
    RFCOMM_CHANNEL_BES              = RFCOMM_CHANNEL_19,    // 0x00200000
    RFCOMM_CHANNEL_BAIDU            = RFCOMM_CHANNEL_20,    // 0x00400000
    RFCOMM_CHANNEL_TENCENT          = RFCOMM_CHANNEL_21,    // 0x00800000
    RFCOMM_CHANNEL_ALI              = RFCOMM_CHANNEL_22,    // 0x01000000
    RFCOMM_CHANNEL_COMMON           = RFCOMM_CHANNEL_23,    // 0x02000000
    RFCOMM_CHANNEL_COMMON_RECORD    = RFCOMM_CHANNEL_24,    // 0x04000000
    RFCOMM_CHANNEL_BIXBY_CONTROL    = RFCOMM_CHANNEL_25,    // 0x08000000
    RFCOMM_CHANNEL_BIXBY_VOICE      = RFCOMM_CHANNEL_26,    // 0x10000000
    RFCOMM_CHANNEL_CCMP             = RFCOMM_CHANNEL_27,    // 0x20000000
    RFCOMM_CHANNEL_MMA              = RFCOMM_CHANNEL_28,    // 0x40000000

    RFCOMM_CHANNEL_CUSTOM_3         = RFCOMM_CHANNEL_28,
    RFCOMM_CHANNEL_CUSTOM_2         = RFCOMM_CHANNEL_29,
    RFCOMM_CHANNEL_CUSTOM_1         = RFCOMM_CHANNEL_30,
} RFCOMM_CHANNEL_NUM;

#define BTIF_APP_INVALID_ID              ((uint64_t)(0x0000000000000000))
#define BTIF_APP_HFP_PROFILE_ID          ((uint64_t)(0x0000000000000001))
#define BTIF_APP_A2DP_PROFILE_ID         ((uint64_t)(0x0000000000000002))
#define BTIF_APP_AVRCP_PROFILE_ID        ((uint64_t)(0x0000000000000004))
#define BTIF_APP_MAP_PROFILE_ID          ((uint64_t)(0X0000000000000008))
#define BTIF_APP_BTGATT_PROFILE_ID       ((uint64_t)(0x0000000000000010))
#define BTIF_APP_HID_PROFILE_ID          ((uint64_t)(0x0000000000000020))
#define BTIF_APP_BASIC_PROFILE_ID        (BTIF_APP_HFP_PROFILE_ID|BTIF_APP_A2DP_PROFILE_ID|BTIF_APP_AVRCP_PROFILE_ID)

#define BTIF_APP_SPP_SERVER_START_ID     ((uint64_t)(0x0000000000001000)) // reserve enough id for rfcomm channel 1 ~ 9
#define BTIF_APP_SPP_SERVER_NUM          (30)
#define BTIF_APP_SPP_SERVER_END_ID       ((uint64_t)(BTIF_APP_SPP_SERVER_START_ID << (BTIF_APP_SPP_SERVER_NUM - 1)))
#define BTIF_APP_SPP_SERVER_OFFSET       (12)
#define BTIF_APP_SPP_SERVER_MASK         ((uint64_t)(0x000000003FFFFFFF << BTIF_APP_SPP_SERVER_OFFSET))

#define BTIF_APP_SPP_CLIENT_START_ID     ((uint64_t)(BTIF_APP_SPP_SERVER_END_ID << 1))
#define BTIF_APP_SPP_CLIENT_NUM          (10)
#define BTIF_APP_SPP_CLIENT_END_ID       ((uint64_t)(BTIF_APP_SPP_CLIENT_START_ID << (BTIF_APP_SPP_CLIENT_NUM - 1)))
#define BTIF_APP_SPP_OFFSET              (12)
#define BTIF_APP_SPP_NUM                 (BTIF_APP_SPP_SERVER_NUM+BTIF_APP_SPP_CLIENT_NUM)
#define BTIF_APP_SPP_MASK                ((uint64_t)(0x000000FFFFFFFFFF<<BTIF_APP_SPP_OFFSET))

#define BT_SPP_GET_APP_ID_FROM_PORT(n) (((n) < BT_SPP_CHANNEL_NUM_START) ? 0 : (BTIF_APP_SPP_SERVER_START_ID << ((n)-BT_SPP_CHANNEL_NUM_START)))
#define BT_SPP_GET_APP_ID_FROM_INDEX(i) (BTIF_APP_SPP_SERVER_START_ID << (i))

#define BT_SPP_APP_ID_GS_CONTROL    BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_GS_CONTROL)
#define BT_SPP_APP_ID_GS_AUDIO      BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_GS_AUDIO)
#define BT_SPP_APP_ID_TOTA          BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_TOTA)
#define BT_SPP_APP_ID_BES_OTA       BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_BES_OTA)
#define BT_SPP_APP_ID_AI_VOICE      BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_AI_VOICE)
#define BT_SPP_APP_ID_GREEN         BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_GREEN)
#define BT_SPP_APP_ID_RED           BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_RED)
#define BT_SPP_APP_ID_FP            BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_FP)
#define BT_SPP_APP_ID_AMA           BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_AMA)
#define BT_SPP_APP_ID_BES           BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_BES)
#define BT_SPP_APP_ID_BAIDU         BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_BAIDU)
#define BT_SPP_APP_ID_TENCENT       BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_TENCENT)
#define BT_SPP_APP_ID_ALI           BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_ALI)
#define BT_SPP_APP_ID_COMMON        BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_COMMON)
#define BT_SPP_APP_ID_COMMON_RECORD BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_COMMON_RECORD)
#define BT_SPP_APP_ID_BIXBY_CONTROL BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_BIXBY_CONTROL)
#define BT_SPP_APP_ID_BIXBY_VOICE   BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_BIXBY_VOICE)
#define BT_SPP_APP_ID_CCMP          BT_SPP_GET_APP_ID_FROM_PORT(RFCOMM_CHANNEL_CCMP)

typedef enum {
    BT_SPP_EVENT_OPENED = BT_EVENT_SPP_OPENED,
    BT_SPP_EVENT_CLOSED,
    BT_SPP_EVENT_TX_DONE,
    BT_SPP_EVENT_RX_DATA,
    BT_SPP_EVENT_ACCEPT,
    BT_SPP_EVENT_END,
} bt_spp_event_t;

#if BT_SPP_EVENT_END != BT_EVENT_SPP_END
#error "bt_spp_event_t error define"
#endif

#if BT_SPP_EVENT_ACCEPT != BT_SOCKET_EVENT_ACCEPT
#error "BT_SPP_EVENT_ACCEPT error define"
#endif

typedef struct bt_spp_channel_t bt_spp_channel_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t rx_data_len;
    const uint8_t *rx_data_ptr;
    bt_spp_channel_t *spp_chan;
} bt_spp_callback_param_t;

typedef int (*bt_spp_callback_t)(const bt_bdaddr_t *remote, bt_spp_event_t event, bt_spp_callback_param_t *param);

struct bt_spp_dlc_info_t
{
    bt_bdaddr_t remote;
    uint8_t dlci;
} __attribute__ ((packed));

typedef struct {
    uint8_t uuid_len;
    uint8_t uuid[16];
} bt_spp_uuid_t;

typedef struct {
    uint8_t local_server_channel;
    const uint8_t* local_spp_128bit_uuid;
    bt_spp_callback_t app_spp_callback;
    bt_socket_accept_callback_t accept_callback;
} bt_spp_server_config_t;

typedef struct {
    struct list_node node;
    uint16_t local_server_channel;
    bool app_layer_give_credit;
    uint8_t initial_credits;
    uint8_t credit_give_step;
    uint8_t server_instance_id;
    uint16_t rx_buff_size;
    uint16_t attr_count;
    const bt_sdp_record_attr_t *attr_list;
    bt_spp_callback_t app_spp_callback;
    bt_spp_callback_t client_spp_callback;
} bt_spp_port_t;

typedef struct bt_spp_channel_t {
    struct list_node node;
    uint8_t device_id;
    uint8_t rfcomm_dlci;
    bt_bdaddr_t remote;
    uint32_t rfcomm_handle;
    bt_socket_state_t spp_state;
    uint8_t remote_server_channel;
    uint16_t connhdl;
    uint16_t tx_mtu;
    uint16_t rx_credits;
    uint16_t free_tx_buf_cnt;
    bt_spp_port_t *port;
    void *btm_conn;
    uint64_t spp_app_id;
} bt_spp_channel_t;

#ifdef BLE_ONLY_ENABLED
#define bt_spp_create_port(local_server_channel, attr_list, attr_count) ((bt_status_t)0)
#define bt_spp_set_callback(local_server_channel, rx_buff_size, spp_callback, client_callback) ((bt_status_t)0)
#define bt_spp_set_app_layer_give_credit(local_server_channel,app_layer_give_credit) ((bt_status_t)0)
#define bt_spp_listen(local_server_channel, support_multi_device, accept_callback) ((bt_status_t)0)
#define bt_spp_remove_listen(local_server_channel) ((bt_status_t)0)
#define bt_spp_server_listen(config) ((bt_status_t)0)
#define bt_spp_server_remove_listen(local_server_channel) ((bt_status_t)0)
#define bt_spp_connect(remote, local_server_channel, uuid, uuid_len) ((bt_status_t)0)
#define bt_spp_connect_server_channel(remote, local_server_channel, remote_server_channel) ((bt_status_t)0)
#define bt_spp_disconnect(rfcomm_handle, reason) ((bt_status_t)0)
#define bt_spp_write(rfcomm_handle, data, size) ((bt_status_t)0)
#define bt_spp_get_record_uuid(spp_chan, out) ((bt_status_t)0)
#define bt_spp_give_handled_credits(rfcomm_handle, handled_credits) ((bt_status_t)0)
#define bt_spp_create_channel(device_id, local_server_channel) ((bt_spp_channel_t *)NULL)
#define bt_spp_count_free_tx_buf_cnt(rfcomm_handle) ((uint16_t)0)
#define bt_spp_is_connected(rfcomm_handle) ((bool)0)
#define bt_spp_is_port_connected(remote, local_server_channel) ((bool)0)
#define bt_spp_get_app_id_from_port(local_server_channel) ((uint64_t)0)
#define bt_spp_get_port_from_app_id(spp_app_id) ((uint8_t)0)
#else
bt_status_t bt_spp_create_port(uint8_t local_server_channel, const bt_sdp_record_attr_t *attr_list, uint16_t attr_count);
bt_status_t bt_spp_set_callback(uint8_t local_server_channel, uint16_t rx_buff_size, bt_spp_callback_t spp_callback, bt_spp_callback_t client_callback);
bt_status_t bt_spp_set_app_layer_give_credit(uint8_t local_server_channel, bool app_layer_give_credit);
bt_status_t bt_spp_listen(uint8_t local_server_channel, bool support_multi_device, bt_socket_accept_callback_t accept_callback);
bt_status_t bt_spp_remove_listen(uint8_t local_server_channel);
bt_status_t bt_spp_server_listen(const bt_spp_server_config_t* config);
bt_status_t bt_spp_server_remove_listen(uint8_t local_server_channel);
bt_status_t bt_spp_connect(const bt_bdaddr_t *remote, uint8_t local_server_channel, const uint8_t *uuid, uint16_t uuid_len);
bt_status_t bt_spp_connect_server_channel(const bt_bdaddr_t *remote, uint8_t local_server_channel, uint8_t remote_server_channel);
bt_status_t bt_spp_disconnect(uint32_t rfcomm_handle, uint8_t reason);
bt_status_t bt_spp_write(uint32_t rfcomm_handle, const uint8_t *data, uint16_t size);
bt_status_t bt_spp_get_record_uuid(const bt_spp_channel_t *spp_chan, bt_spp_uuid_t *out);
bt_status_t bt_spp_give_handled_credits(uint32_t rfcomm_handle, uint16_t handled_credits);
bt_spp_channel_t *bt_spp_create_channel(uint8_t device_id, uint8_t local_server_channel);
uint16_t bt_spp_count_free_tx_buf_cnt(uint32_t rfcomm_handle);
bool bt_spp_is_connected(uint32_t rfcomm_handle);
bool bt_spp_is_port_connected(const bt_bdaddr_t *remote, uint8_t local_server_channel);
uint64_t bt_spp_get_app_id_from_port(uint8_t local_server_channel);
uint8_t bt_spp_get_port_from_app_id(uint64_t spp_app_id);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __BT_SPP_SERVICE_H__ */

