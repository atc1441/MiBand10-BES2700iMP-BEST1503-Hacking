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
#ifndef __APP_TWS_IBRT_CORE_H__
#define __APP_TWS_IBRT_CORE_H__

#include "app_tws_ibrt_core_type.h"
#include "app_tws_ibrt_mobile_conn_sm.h"

#ifdef __cplusplus
 extern "C" {
#endif

/* TWS enhanced switch implementation */
typedef struct {
    uint8_t initiator_role;
    uint8_t req;
    uint8_t responsor_role;
    uint8_t rsp;
    uint8_t master_initiate_rs;
    uint16_t master_rs_delay;
} ERS_PKT_T;

typedef void (*role_switch_prepare_handler)(void);

typedef uint8_t (*app_sbm_status_callback_handler_t)(void);
typedef void (*app_tws_ui_role_update_callback_handler_t)(uint8_t newRole);

// app_ibrt_conn_xxx        api for inner

// app_ibrt_connif_xxx      api for extern

/**************************************API FOR INNER CALL**************************************/
bool app_ibrt_conn_is_w4_data_exchange(const bt_bdaddr_t *addr);
bool app_ibrt_conn_is_w4_ibrt(const bt_bdaddr_t *addr);
bool app_ibrt_conn_is_dev_role_switch_on(const bt_bdaddr_t *mobile_addr);
bool app_ibrt_conn_get_tws_use_same_addr_enable();
bool app_ibrt_conn_is_tws_link_connecting();
bool app_ibrt_conn_any_mobile_connecting();
bool app_ibrt_conn_get_tws_connected_dev_addr(bt_bdaddr_t* pAddr);
bool app_ibrt_conn_tws_is_role_switch_allowed(const bt_bdaddr_t *addr);

void app_ibrt_conn_set_ibrt_mode(ibrt_mode_e mode);
void app_ibrt_conn_reset_peer_tws_info(void);
void app_ibrt_conn_cmd_status_callback(const void *para);
void app_ibrt_conn_cmd_complete_callback(const uint8_t *para);
void app_ibrt_conn_tws_switch_callback(ibrt_mobile_info_t *mobile_info, uint8_t new_role);
void app_ibrt_conn_cmd_send_fail_error_handler(uint16_t cmd_code,uint8_t *p_buff);
void app_ibrt_conn_global_handler(btif_remote_device_t *remote_dev,ibrt_link_type_e link_type, uint8_t evt_type, uint8_t status);
void app_ibrt_conn_ibrt_profile_callback(uint8_t device_id, uint64_t profile,void *param1,void *param2,void* param3);
void app_ibrt_conn_vender_event_handler(uint8_t evt_type, uint8_t * buffer, uint32_t length);
void app_ibrt_conn_link_connected_callback(bt_bdaddr_t *addr);
void app_ibrt_conn_mobile_connected_callback(bt_bdaddr_t *addr);
void app_ibrt_conn_tws_connected_callback(bt_bdaddr_t *adrd);
void app_ibrt_conn_set_local_volume_info(uint8_t * p_buffer);
void app_ibrt_conn_profile_state_change_ind(uint64_t profile,uint8_t connected);
void app_ibrt_conn_ibrt_sm_quit(const bt_bdaddr_t *addr);
void app_ibrt_conn_free_mobile_sm(const bt_bdaddr_t *addr);
void app_ibrt_conn_profile_data_exchange(ibrt_mobile_info_t* mobile_info);
void app_ibrt_conn_controller_error_handler(ibrt_controller_error_type error_type, bool reset_controler,const bt_bdaddr_t* mobile_addr = NULL);
mobile_statemachine_t* app_ibrt_conn_get_mobile_sm_by_addr(const bt_bdaddr_t* addr);
void app_ibrt_conn_stack_report_create_acl_failed(const bt_bdaddr_t *addr);

uint8_t app_ibrt_conn_prepare_local_mobile_info(mobile_info_exchange_t *buffer);

void app_ibrt_conn_rs_mgr_init(void);
bool app_ibrt_conn_rs_task_set(uint8_t task_id, uint16_t life_cycle);
void app_ibrt_conn_rs_task_clr(uint8_t task_id);
bool app_ibrt_conn_enable_rs_delay(void);

uint8_t app_ibrt_conn_get_all_valid_mobile_info(ibrt_mobile_info_t *p_mobile_info_array[],uint8_t max_size);
uint8_t app_ibrt_load_conn_default_config(void);

ibrt_status_t app_ibrt_conn_send_ibrt_msg(const bt_bdaddr_t *addr, ibrt_sm_message_e evt,uint32_t param0=0,uint32_t param1=0);
ibrt_status_t app_ibrt_conn_send_mobile_msg(const bt_bdaddr_t *addr, ibrt_mobile_message_e evt,uint32_t param0 = 0,uint32_t param1 = 0);
ibrt_status_t app_ibrt_conn_slave_startup_ibrt(const bt_bdaddr_t *addr, ibrt_sm_message_e param);

void app_ibrt_conn_set_exchange_profile_complete(const bt_bdaddr_t *addr);

mobile_statemachine_t* app_ibrt_conn_slave_mock_mobile_link(const bt_bdaddr_t *addr,uint32_t param0);
ibrt_mobile_info_t *app_ibrt_conn_get_mobile_info(const bt_bdaddr_t* addr);
ibrt_mobile_info_t* app_ibrt_conn_get_mobile_info_by_ibrt_handle(uint16_t ibrt_handle);
ibrt_mobile_info_t* app_ibrt_conn_get_mobile_info_by_hci_handle(uint16_t hci_handle);
ibrt_conn_config_t* app_ibrt_conn_get_config();

const app_ibrt_conn_event_cb* app_ibrt_conn_get_notify_callback();

const app_ibrt_cmd_ui_handler_cb* app_ibrt_conn_get_cmd_handler_cbs(void);

app_tws_info_t* app_ibrt_conn_get_peer_tws_info();

const app_ibrt_if_cbs_t* app_ibrt_conn_get_reg_ibrt_if_cb();

const app_ibrt_get_peer_rssi_cb* app_ibrt_conn_get_reg_rssi_cmd_cb();

uint8_t app_ibrt_conn_get_mobile_connecting_dev_count();

bool app_ibrt_conn_super_handle_connect_request(bt_bdaddr_t *addr, uint8_t *cod);

void app_ibrt_conn_mobile_ssp_failed_handler(bt_bdaddr_t *addr);

void app_ibrt_conn_delay_rs_register_cb(role_switch_prepare_handler cb);

void app_ibrt_conn_enhanced_rs_cmd_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);
void app_ibrt_conn_enhanced_rs_rsp_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

void app_ibrt_conn_sync_set_tws_link_id(bool local_has_mobile);

void app_ibrt_send_set_tws_link_id_req(bool local_has_mobile);

void app_ibrt_conn_set_tws_link_id(void const *arg);

void app_ibrt_conn_delay_set_tws_link_id(void);

void app_ibrt_conn_mobile_sync_no_profile_info(const bt_bdaddr_t *addr);

#ifdef SASS_ENABLED
void app_ibrt_conn_notify_sass_info(bool enable_multipoint, const bt_bdaddr_t *addr);
#endif

void app_ibrt_register_sbm_status(app_sbm_status_callback_handler_t func);

void app_ibrt_register_tws_ui_role_update_handle(app_tws_ui_role_update_callback_handler_t func);

#ifdef __cplusplus
}
#endif

#endif
