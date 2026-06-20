/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __APP_TWS_IBRT__MOBILE_CONN_SM__
#define __APP_TWS_IBRT__MOBILE_CONN_SM__

#if defined(IBRT) && defined(IBRT_UI)

#include "stdint.h"
#include "cmsis_os.h"
#include "hsm.h"

#define MOIBLE_EVENT_BASE    0x2000

typedef enum
{
    MOBILE_CONN_DISCONNECTED,
    MOBILE_CONN_CONNECTING,
    MOBILE_CONN_CONNECTED,
    MOBILE_CONN_DISCONNECTING
} ibrt_mobile_connection_state;

typedef enum
{
    //Messages for handling connect/disconnect requests.
    SIG_CONNECT_MOBILE = MOIBLE_EVENT_BASE,
    SIG_DISCONNECT_MOBILE,
    SIG_CANCEL_MOBILE_CONNECT_REQUEST,
    SIG_START_ROLE_SWITCH,
    SIG_PASSIVE_ROLE_SWITCH,

    // Messages for handling error conditions.
    EVT_MOBILE_CONNECT_FAILED,
    EVT_MOBILE_CONNECT_TIMEOUT,
    EVT_MOBILE_DISCONNECT_TIMEOUT,
    EVT_MOBILE_CONNECTING_CANCELED,
    EVT_TWS_ROLE_SIWTCH_FAILED,
    EVT_TWS_ROLE_SWITCH_TIMEOUT,

    EVT_MOBILE_RAW_CONNECTED,
    EVT_MOBILE_CONNECTED,
    EVT_MOBILE_SIMPLE_PAIRING_COMPLETED,
    EVT_MOBILE_DISCONNECTED,
    EVT_MOBILE_ACL_AUTH_COMPLETE,
    EVT_MOBILE_PROFILE_CHANGED,
    EVT_IBRT_LINK_CONNECTED,
    EVT_IBRT_LINK_DISCONNECTED,
    EVT_ROLE_SWITCH_PREPARATION_COMPLETED,
    EVT_ROLE_SWITCH_COMPLTED,

    EVT_MOBILE_MAX_NUM,
} ibrt_mobile_message_e;

typedef void (*on_host_paring_state_changed_notify)(const bt_bdaddr_t *addr,ibrt_conn_pairing_state state,uint8_t reason_code);
typedef void (*on_host_acl_state_changed_notify)(const bt_bdaddr_t *addr,ibrt_conn_acl_state state,uint8_t reason_code);
typedef void (*on_host_cancel_connect_changed)(int8_t state);
typedef void (*on_ibrt_role_changed)(const bt_bdaddr_t *addr,ibrt_conn_role_change_state state,ibrt_role_e role);
typedef void (*on_phone_connect_indication)(const bt_bdaddr_t *addr);
typedef bool (*role_switch_prepare_needed_func)(uint32_t *wait_ms);
typedef void (*role_swtich_prepare_func)(uint32_t timeoutMs);
typedef void (*on_phone_unpaired_indication)(const bt_bdaddr_t *addr);

typedef void (*mobile_sm_post_func)(void);
/*add timer to fix controller exception*/
void app_ibrt_mobile_conn_error_handle_timer_cb(void const *ctx);

typedef struct
{
    Hsm mobile_sm;
    State disconnected;
    State disconnecting;
    State connecting;
    State connected;
    State ibrt_connected;
        State prepare_roleswitch;
        State execute_roleswitch;

    bool already_startup;
    bool safe_entry_disconnect;
    uint8_t used;
    uint8_t             device_id;            /* used for log prefix */
    bt_bdaddr_t         current_addr;
    ibrt_conn_acl_state disconnect_evt;
    uint8_t             disc_reason;
    uint8_t             wait_role_switch_complete;

    osTimerId           exp_handler_timer_id;
    osTimerDefEx_t      exp_handler_timer_def;
    osTimerId           roleswitch_timeout_timer_id;
    osTimerDefEx_t      roleswitch_timeout_timer_def;

    mobile_sm_post_func dis_mobile_post_func;
    bool is_connect_initiated_locally;
    bool initiate_connect;
    bool acl_up;
    bool is_acl_auth_success;
    bool disc_for_entription_fail;
    bool disc_for_key_missing;
    bool upper_custom_switch_prepare;
    bool cancel_page_ongoing;
    unsigned int disconnect_reason;
    bool entry_connected;
    uint32_t page_to;
    uint32_t roleSwitchPreparingTimeoutInMs;
    uint32_t page_stamp;
    uint32_t page_cmp_stamp;
    uint32_t prof_cmp_stamp;
    on_ibrt_role_changed role_changed_cb;
    on_host_paring_state_changed_notify host_pairing_state_changed_cb;
    on_host_acl_state_changed_notify mobile_acl_state_changed_cb;
    on_phone_connect_indication      phone_connect_ind_cb;
    on_host_cancel_connect_changed   host_cancel_connect_changed_cb;
    role_switch_prepare_needed_func  role_switch_prepare_needed;
    role_swtich_prepare_func         role_switch_prepare;

    ibrt_mobile_info_t mobile_dev_info;
} mobile_statemachine_t;

#ifdef __cplusplus
extern "C" {
#endif

void app_tws_ibrt_fast_free_mobile_link_sm(mobile_statemachine_t *p_mobile_link,uint8_t reason);
void app_tws_ibrt_mobile_start(mobile_statemachine_t* p_mobile_sm);
int app_tws_ibrt_mobile_sm_send_msg(uint32_t mobile_sm_ptr,ibrt_mobile_message_e evt,uint32_t param0 = 0,uint32_t param1=0);
int app_tws_ibrt_mobile_sm_on_event(uint32_t event, uint32_t mobile_ptr = 0,uint32_t param0 = 0,uint32_t param1 = 0);
ibrt_conn_acl_state app_ibrt_get_mobile_connection_state(mobile_statemachine_t* p_mobile_sm);
bool app_ibrt_mobile_conn_in_ibrt_connected(mobile_statemachine_t* p_mobile_sm);
bool app_ibrt_is_tws_role_switching(mobile_statemachine_t *rs_sm);

#ifdef __cplusplus
}
#endif

#endif

#endif /*__APP_TWS_IBRT__MOBILE_CONN_SM__*/
