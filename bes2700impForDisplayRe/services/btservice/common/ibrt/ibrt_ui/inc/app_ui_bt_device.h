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
#ifndef __APP_UI_BT_DEVICE__
#define __APP_UI_BT_DEVICE__


#include "stdint.h"
#include "cmsis_os.h"
#include "hsm.h"

typedef void (*notify_mobile_status_changed)(uint16_t link_id,app_ui_evt_t status,uint8_t reason);
typedef void (*notify_ibrt_status_changed)(uint16_t link_id,app_ui_evt_t status,tws_role_e role);
typedef bool (*unify_tws_role)(bt_bdaddr_t *addr,tws_role_e curr_role);

typedef enum {
    MOB_SUPER,
    MOB_DISCONNECT,
    MOB_CONNECTING,
    MOB_CONNECTED,
    MOB_IBRT_CONNECTED,
    IBRT_ROLE_SWITCH,
} btmob_sm_state_e;

typedef enum {
    ROLE_UNCONFIRMED,
    ROLE_CONFIRMING,
    ROLE_CONFIRMED,
} mobile_link_role_status_e;

typedef struct
{
    notify_ibrt_status_changed notify_ibrt_status_changed_func;
    notify_mobile_status_changed on_mobile_status_changed_cb;
} app_ui_btmob_sm_cbs_t;

typedef struct
{
    app_ui_evt_t complete_evt;
    app_ui_evt_t latest_evt;
} link_evt_run_result_t;

typedef enum {
    RECONNECT_NONE,
    LINK_LOSS,
    TRY_RECONNECT,
    TRY_RECONNECT_EXT,
} reconnect_reason_t;

typedef enum {
    PRIO_DISCONNECT,
    PRIO_CONENCTING,
    PRIO_CONENCTED,
} prio_state_t;

typedef struct
{
    Hsm mobile_sm;
    State mobile_disconnected;
    State mobile_connecting;
    State mobile_connected;
    State ibrt_connected;
    State role_switching;

    osTimerId reconnect_delay_timer_id;
    osTimerDefEx_t evt_delay_handler_timer_def;

    bool is_valid;
    bool destroying;
    bool hold_run_permission;
    bool backup_run_permission;
    bool reconnect_failed;
    bt_bdaddr_t current_addr;
    uint16_t link_id;
    uint16_t prio_value;
    tws_role_e current_role;
    mobile_link_role_status_e role_status;
    app_ui_evt_t active_evt;
    bool last_evt_run_complete;
    app_ui_evt_queue_t local_link_evt_que;
    app_ui_evt_queue_t peer_link_evt_que;
    notify_mobile_status_changed notify_mobile_status_changed_cb;
    notify_ibrt_status_changed notify_ibrt_status_cb;
    ibrt_ui_req_hdr_t   local_req_hdr;
    ibrt_ui_share_link_info  peer_share_info;
    ibrt_ui_req_info_type_t  notify_type;

    app_ui_evt_t latest_evt;                /* inner use */
    uint16_t max_try_reconnect_times;       /* ext max try reconnect times */
    uint16_t mobile_try_reconnect_times;    /* mobile reconnect counter    */
    reconnect_reason_t reconnect_reason;    /* record reason for distinguish from open try or link loss try */
    uint8_t disconnect_reason;              /* disconnect reason from ctl   */
    uint8_t latest_disc_reason;             /* latest disconnect reason */
    bool exchangeinfo_req;

    link_evt_run_result_t evt_run_result;
} app_ui_btmob_sm_t;

#ifdef __cplusplus
extern "C" {
#endif

void app_ui_btmob_sm_init(uint16_t link_id, app_ui_btmob_sm_t* mobile_link_sm);
void app_ui_btmob_sm_destroy(app_ui_btmob_sm_t* mobile_link_sm);
void app_ui_btmob_sm_reset_link_info(app_ui_btmob_sm_t* mobile_link_sm);
int app_ui_btmob_sm_on_event(app_ui_btmob_sm_t* sm_ptr, uint32_t event, uint32_t param0 = 0, uint32_t param1 = 0);
void app_ui_btmob_sm_register_cbs(app_ui_btmob_sm_t* mobile_link_sm, const app_ui_btmob_sm_cbs_t* cbs);
void app_ui_btmob_sm_release_hold_permission(app_ui_btmob_sm_t* mobile_link_sm);
bool app_ui_btmob_sm_is_idle(app_ui_btmob_sm_t* mobile_link_sm);
bool app_ui_btmob_sm_is_hold_run_permission(app_ui_btmob_sm_t* mobile_link_sm);
void app_ui_btmob_sm_pop_link_queue_event(app_ui_btmob_sm_t* mobile_link_sm, bool is_local_queue);
void app_ui_btmob_sm_push_event_to_link_queue(app_ui_btmob_sm_t* mobile_link_sm, app_ui_event_t* event, bool is_local_queue);
bool app_ui_btmob_sm_get_evt_run_status(app_ui_btmob_sm_t* me);
void app_ui_btmob_sm_get_evt_run_result(app_ui_btmob_sm_t* mobile_link_sm, link_evt_run_result_t* run_result);
void app_ui_btmob_sm_catch_hold_permission(app_ui_btmob_sm_t* mobile_link_sm);
bool app_ui_btmob_sm_reconnect_failed(app_ui_btmob_sm_t* me);
bool app_ui_btmob_sm_state_is_connecting(app_ui_btmob_sm_t* mobile_link_sm);
bool app_ui_btmob_sm_state_is_connected(app_ui_btmob_sm_t* mobile_link_sm);
bool app_ui_btmob_sm_state_is_disconnected(app_ui_btmob_sm_t* mobile_link_sm);
void app_ui_btmob_sm_update_priority(app_ui_btmob_sm_t* device, prio_state_t prio_state);
void app_ui_btmob_sm_update_reconnect_param(app_ui_btmob_sm_t* mobile_link_sm, uint8_t try_count);
const char* btmob_sm_state_to_string(btmob_sm_state_e state);


bool app_ui_notify_switch_ui_role(bool switch2master);

#ifdef __cplusplus
}
#endif

#endif /* __APP_UI_BT_DEVICE__ */
