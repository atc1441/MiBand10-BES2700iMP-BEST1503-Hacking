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
#ifndef __L2CAP_API_H__
#define __L2CAP_API_H__
#include "bluetooth.h"
#include "me_api.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct l2cap_conn l2cap_conn_t;

bt_status_t btif_l2cap_create_port(uint16_t local_l2cap_psm, bt_l2cap_callback_t l2cap_callback);
bt_status_t btif_l2cap_remove_port(uint16_t local_l2cap_psm);
bt_status_t btif_l2cap_set_rx_credits(uint16_t local_l2cap_psm, uint16_t expected_rx_mtu, uint16_t initial_credits, uint16_t credit_give_step);
bt_status_t btif_l2cap_listen(uint16_t local_l2cap_psm);
bt_status_t btif_l2cap_remove_listen(uint16_t local_l2cap_psm);
bt_status_t btif_l2cap_send_packet(uint16_t connhdl, uint32_t l2cap_handle, const uint8_t *data, uint16_t len, void *context);
bt_status_t btif_l2cap_disconnect(uint32_t l2cap_handle, uint8_t reason);

uint32_t btif_l2cap_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target);
#ifdef BT_L2CAP_ENRE_MODE_SUPPORT
uint32_t btif_l2cap_enre_connect(const bt_bdaddr_t *remote, uint16_t local_l2cap_psm, l2cap_psm_target_profile_t target, uint16_t remote_l2cap_psm);
#endif
#if (EATT_CHAN_SUPPORT == 1)
bt_status_t btif_l2cap_enfc_connect(hci_conn_type_t conn_type, uint16_t connhdl, uint16_t local_spsm);
#endif

typedef void (*btif_l2cap_process_echo_req_callback_func)(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data);
typedef void (*btif_l2cap_process_echo_res_callback_func)(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxlen);
typedef void (*btif_l2cap_fill_in_echo_req_data_callback_func)(uint8_t device_id, struct l2cap_conn *conn, uint8_t *data, uint16_t data_len);

void btif_l2cap_echo_init(btif_l2cap_process_echo_req_callback_func req_func,btif_l2cap_process_echo_res_callback_func res_func,btif_l2cap_fill_in_echo_req_data_callback_func data_func);
void btif_l2cap_process_echo_req_rewrite_rsp_data(uint8_t device_id, uint16_t conhdl, uint8_t id, uint16_t len, uint8_t *data);
void btif_l2cap_process_echo_res_analyze_data(uint8_t device_id, uint16_t conhdl, uint8_t *rxdata, uint16_t rxlen);
void btif_l2cap_fill_in_echo_req_data(uint8_t device_id, struct l2cap_conn *conn, uint8_t *data, uint16_t data_len);
void btif_l2cap_fill_in_enco_rsp_data(uint8_t device_id, uint16 conn_handle, uint8 sigid, uint16 len, const uint8* data);
void btif_custom_l2cap_echo_init(btif_l2cap_process_echo_req_callback_func req_func,btif_l2cap_process_echo_res_callback_func res_func);

void btif_l2cap_register_sdp_disconnect_callback(void (*cb)(const bt_bdaddr_t* addr));
void btif_l2cap_register_get_ibrt_role_callback(uint8_t (*cb)(const bt_bdaddr_t* addr));
void btif_l2cap_register_get_ui_role_callback(uint8_t (*cb)(void));
void btif_l2cap_register_get_tss_state_callback(uint8_t (*cb)(const bt_bdaddr_t* addr));

uint16_t btif_l2cap_get_tx_mtu(uint32_t l2cap_handle);
void btif_create_besaud_extra_channel(void* remote_addr, bt_l2cap_callback_t l2cap_callback);
void btif_besaud_extra_channel_send_data(uint32_t l2cap_handle, const uint8_t *data, uint16_t len, void *context);
void btif_l2cap_reset_conn_sigid(const bt_bdaddr_t *addr);
bool btif_l2cap_is_profile_channel_connected(const bt_bdaddr_t *remote, uint8_t psm_context_mask);
bool btif_l2cap_is_dlci_channel_connected(uint32_t session_l2c_handle, uint64_t app_id);
bt_status_t btif_l2cap_send_bredr_security_manager_rsp(uint8 device_id, uint16 conn_handle, uint16 len, uint8 *data);

#ifdef __cplusplus
}
#endif
#endif /* __L2CAP_API_H__ */
