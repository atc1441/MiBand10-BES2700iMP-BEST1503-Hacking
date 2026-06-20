/***************************************************************************
 *
 * Copyright 2024-2034 BES.
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
#ifndef __APP_UI_DEVICES_MGR_H__
#define __APP_UI_DEVICES_MGR_H__
#include "app_ibrt_conn_evt.h"
#include "app_ui_evt.h"

#define  DEVICE_MGR_MAX_NUM                 (BT_DEVICE_NUM + 2)
#define  DEVICE_HAVE_THREE_NUM              (3)

typedef enum {
    APP_DEV_NONE = 0,
    APP_DEV_BT = 0x01,
    APP_DEV_LEA = 0x02, //CIS connection link
    APP_DEV_BT_AND_LEA = 0x03, //BT connection link + CIS connection link
    APP_DEV_LINK_MAX,
} app_dev_mgr_link_mode_t;

typedef enum {
    APP_DEV_BIS_USE_NO_USE = 0,
    APP_DEV_BIS_USE,
    APP_DEV_BIS_USE_MAX,
} app_dev_mgr_bis_use_mode_t;

typedef enum {
    APP_DEV_LEA_DISCONNECTED = 0,
    APP_DEV_LEA_DISCONNECTING,
    APP_DEV_LEA_RELOAD_INSTANCE,
    APP_DEV_LEA_ADD_WHITE_LIST,
    APP_DEV_LEA_CONNECTED,
    APP_DEV_LEA_MAX_STATE,
} app_dev_mgr_lea_state_t;

typedef enum {
    APP_DEV_BIS_IDLE= 0,
    APP_DEV_BIS_ADD_SRC,
    APP_DEV_W4_BIS_SYNC,
    APP_DEV_BIS_SYNCING,
    APP_DEV_BIS_STREAM_START,
    APP_DEV_BIS_STREAM_STOP,
    APP_DEV_BIS_MAX_STATE
} app_dev_mgr_bis_state_t;

typedef enum {
    APP_DEV_BT_DISCONNECTED = 0,
    APP_DEV_BT_DISCONNECTING,
    APP_DEV_BT_RELOAD_INSTANCE,
    APP_DEV_BT_CONNECTING,
    APP_DEV_BT_PHONE_CONN_REQ,
    APP_DEV_BT_CONNECTED,
    APP_DEV_BT_ACL_CONNECTED,
    APP_DEV_BT_MAX_STATE,
} app_dev_mgr_bt_state_t;


typedef enum {
    APP_DEV_POLICY_NONE = 0,
    APP_DEV_POLICY_LEA_START,
    APP_DEV_POLICY_LEA_RELOAD_INSTANCE = APP_DEV_POLICY_LEA_START,
    APP_DEV_POLICY_LEA_ADD_WHITE_LIST,
    APP_DEV_POLICY_LEA_CONNECTED,
    APP_DEV_POLICY_LEA_DISCONNECTED,
    APP_DEV_POLICY_LEA_END,

    APP_DEV_POLICY_BT_START = 0x10,
    APP_DEV_POLICY_BT_RELOAD_INSTANCE = APP_DEV_POLICY_BT_START,
    APP_DEV_POLICY_BT_CONN_REQ,
    APP_DEV_POLICY_BT_RECONNECTING,
    APP_DEV_POLICY_BT_CONNECTING_FAIL,
    APP_DEV_POLICY_BT_CONNECTED,
    APP_DEV_POLICY_BT_DISCONNECTED,
    APP_DEV_POLICY_BT_END,

    APP_DEV_POLICY_IBRT_START = 0x20,
    APP_DEV_POLICY_IBRT_CONNECTED = APP_DEV_POLICY_IBRT_START,
    APP_DEV_POLICY_IBRT_ACL_CONNECTED,
    APP_DEV_POLICY_IBRT_DISCONNECTED,
    APP_DEV_POLICY_IBRT_END,

    APP_DEV_POLICY_BIS_START   = 0x30,
    APP_DEV_POLICY_BIS_ADD_SRC = APP_DEV_POLICY_BIS_START,
    APP_DEV_POLICY_BIS_SYNC_PA,
    APP_DEV_POLICY_BIS_STREAM_CHANGE,
    APP_DEV_POLICY_BIS_RM_SRC,
    APP_DEV_POLICY_BIS_END,

    APP_DEV_POLICY_OTHER_START   = 0x40,
    APP_DEV_POLICY_RELOAD_COMPLETE = APP_DEV_POLICY_OTHER_START,
    APP_DEV_POLICY_TWS_CONNECTED,
    APP_DEV_POLICY_OTHER_END
} app_dev_mgr_policy_type_t;

typedef enum {
    APP_DEV_SYNC_CONN_STATE = 0,
    APP_DEV_SYNC_DISCONNECT,
    APP_DEV_SYNC_ADV_LIST,
    APP_DEV_SYNC_TRIGGER_ADV_EVT,
    APP_DEV_SYNC_RSP,
} app_dev_mgr_cmd_type_t;

typedef enum {
    APP_DEV_NSP_TWS_CONN = 0,
    APP_DEV_NSP_BT_CONN,
    APP_DEV_NSP_LEA_CONN,
    APP_DEV_NSP_BT_START,
    APP_DEV_NSP_LEA_START,
    APP_DEV_NSP_BIS_SYNC,
} app_dev_mgr_nonsupport_tirgger_t;

typedef enum {
    APP_DEV_SET_PEER_BT_STATE = 0,
    APP_DEV_CLEAR_PEER_BT_STATE,
    APP_DEV_SET_PEER_BLE_STATE,
    APP_DEV_CLEAR_PEER_BLE_STATE,
} app_dev_mgr_updta_peer_state_t;

typedef struct
{
    struct list_node    node;
    bool                used;
    bool                is_lea_device;
    uint8_t             device_id;
    uint8_t             le_conidx;
    uint8_t             mobile_state;
    uint8_t             ibrt_state;
    uint8_t             lea_state;
    uint8_t             local_conn_state;
    uint8_t             peer_conn_state;
    bt_bdaddr_t         address;
    uint8_t             addr_type;
} app_dev_mgr_dev_ctx_t;

typedef struct
{
    bool                valid;
    uint8_t             connect_state;
    bt_bdaddr_t         address;
    uint8_t             addr_type;
} app_dev_mgr_send_dev_t;

typedef struct
{
    uint8_t                    cmd_type;
    uint8_t                    device_num;
    bool                       master_req;
    bool                       bis_mode;
    app_dev_mgr_policy_type_t  evt_type;
    bt_bdaddr_t                address;
    app_dev_mgr_send_dev_t     device[BT_DEVICE_NUM + 1];
} dev_send_mgr_t;

#ifdef __cplusplus
extern "C" {
#endif

void app_dev_mgr_init(multipoint_mode_t multipoint_mode, bool enable_leaudio);

void app_dev_mgr_ux_reload_mobile_instance(bt_bdaddr_t *bt_addr, uint8_t num);

void app_dev_mgr_ux_change_mode(multipoint_mode_t multipoint_mode);

bool app_dev_mgr_bt_conn_request_handle(bt_bdaddr_t *addr, uint8_t *cod);

void app_dev_mgr_event_entry(app_ui_evt_t evt);

void app_dev_mgr_send_box_msg(app_ui_evt_t box_evt);

void app_dev_mgr_bt_event_handler(ibrt_conn_evt_header *pkt, int pkt_len);

void app_dev_mgr_le_audio_event_handler(void *event, int event_len);

void app_dev_mgr_recv_req_dev_mgr_handle(dev_send_mgr_t *dev_mgr, uint16_t rsp_seq);

void app_dev_mgr_recv_dev_mgr_rsp_handle(dev_send_mgr_t *dev_mgr);

void app_dev_mgr_recv_dev_mgr_rsp_timeout_handle(dev_send_mgr_t *dev_mgr);

bool app_dev_mgr_user_trigger_paring_adv(bool notify_peer);

void app_dev_mgr_user_disconnect_dev(const bt_bdaddr_t *addr);

void app_dev_mgr_register_devices_limite_callback(bt_bdaddr_t *(*cb)(bt_bdaddr_t *incomming_addr));

uint8_t app_dev_mgr_check_lea_connected_by_addr(const bt_bdaddr_t *addr);

bool app_dev_mgr_update_bt_state(uint8_t state, bt_bdaddr_t *addr, uint8_t device_id);

bool app_dev_mgr_reconfig_lea_max_links(uint8_t lea_max_num, uint8_t dul_max_num);
#ifdef __cplusplus
}
#endif

#endif /* __APP_UI_DEVICES_MGR_H__ */
