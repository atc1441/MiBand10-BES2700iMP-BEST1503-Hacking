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
#ifndef __BT_IF_H_
#define __BT_IF_H_
#include <stdint.h>
#include "bluetooth.h"
#include "me_api.h"
#include "besbt.h"
#include "me_common_define.h"
#include "source_common_define.h"

#include "bt_callback_func.h"

//Application ID,indentify profle app context

#ifdef __cplusplus
extern "C" {
#endif

#if defined(IBRT)
uint64_t btif_app_get_app_id_from_spp_flag(uint8_t spp_flag);
uint8_t btif_app_get_spp_flag_from_app_id(uint64_t app_id);
void btif_ibrt_stack_clean_slave_status(const bt_bdaddr_t* remote);
#endif

// should greater than (sizeof(rfcomm_sync_data_item_t) * RFCOMM_MAX_SYNC_DATA_ITEMS) + 1
#define RFCOMM_MAX_SYNC_TXRX_CREDIT_DATA_SIZE (30)
struct btif_sync_txrx_credit_t {
    bt_bdaddr_t remote_bdaddr;
    uint8_t rfcomm_sync_txrx_credit_data[RFCOMM_MAX_SYNC_TXRX_CREDIT_DATA_SIZE];
} __attribute__ ((packed));

typedef struct{
    unsigned char   *dev_name;
    uint8_t         name_str_len;
    uint8_t         *uuid_buf;
    uint16_t        uuid_buf_len;
} btif_eir_raw_data_t;

bool btif_ibrt_master_wait_remote_new_master_ready(const bt_bdaddr_t *remote);

void btif_ibrt_sync_txrx_credit(struct btif_sync_txrx_credit_t *sync_data, const bt_bdaddr_t *remote);

void btif_ibrt_sync_txrx_credit_handler(struct btif_sync_txrx_credit_t *sync_data, uint16_t length);

enum pair_event
{
    PAIR_EVENT_NUMERIC_REQ,
    PAIR_EVENT_COMPLETE,
    PAIR_EVENT_FAILED,
};

typedef void (*btif_pairing_callback_t)(enum pair_event evt, void *data);

void btif_pairing_register_callback(btif_pairing_callback_t callback);
void btif_confirmation_register_callback(btif_confirmation_req_callback_t callback);

typedef void (*stack_ready_callback_t) (int status);
typedef void (*stack_start_init_callback_t) (void);

int bt_stack_register_ready_callback(stack_ready_callback_t ready_cb);
int bt_stack_register_init_start_callback(stack_start_init_callback_t cb);


int bt_stack_initilize(void);
void bt_chip_common_init(bool ble_only_enabled);

int bt_stack_config(const unsigned char *dev_name, uint8_t len);
void btif_update_bt_name(const unsigned char *dev_name, uint8_t len);
int bt_set_local_dev_name(const unsigned char *device_name, uint8_t len);
int bt_set_local_clock(uint32_t clock);
void bt_process_stack_events(void);
bool btif_is_gatt_over_br_edr_enabled(void);
#if defined(__GATT_OVER_BR_EDR__)
bool btif_is_gatt_over_br_edr_allowed_send(uint8_t conidx);
#endif
bool btif_is_ctkd_over_br_edr_enabled(void);

void btif_set_btstack_chip_config(void *config);
void btif_set_extended_inquiry_response(uint8_t* eir, uint32_t len);
void btif_avrcp_ct_register_notification_event(uint8_t device_id, uint8_t event);
int btif_me_send_hci_cmd(uint16_t opcode, uint8_t *param_data_ptr, uint8_t param_len);
void btif_confirmation_resp(struct bdaddr_t *bdaddr, bool accept);
#if defined(IBRT)
uint32_t btif_save_app_bt_device_ctx(uint8_t *ctx_buffer,uint8_t psm_context_mask);
uint32_t btif_set_app_bt_device_ctx(uint8_t *ctx_buffer,uint8_t psm_context_mask,uint8_t bt_devices_idx, uint8_t rm_detbl_idx, uint8_t avd_ctx_device_idx);
bool btif_ibrt_master_wait_remote_new_master_ready(const bt_bdaddr_t *remote);
void btif_ibrt_master_tws_switch_set_start(const bt_bdaddr_t* remote);
void btif_ibrt_slave_tws_switch_set_start(const bt_bdaddr_t* remote);
void btif_ibrt_master_become_slave(const bt_bdaddr_t* remote);
void btif_ibrt_slave_become_master(const bt_bdaddr_t* remote);
void btif_ibrt_clear_side_save_credit_bit(const bt_bdaddr_t* remote);
#endif
uint32_t btif_get_class_of_device(void);
uint32_t btif_set_class_of_device(uint8 *cod);
void btif_osapi_lock_stack(void);
void btif_osapi_unlock_stack(void);
int btif_osapi_lock_is_exist(void);
void btif_osapi_notify_evm(void);
struct btm_conn_item_t *btif_ibrt_create_new_snoop_link(bt_bdaddr_t *remote, uint16 conn_handle);
void btif_conn_ibrt_disconnected_handle(struct btm_conn_item_t *btm_conn);
int8 btif_l2cap_send_data( uint32 l2cap_handle, uint8 *data, uint32 datalen, void *context);
void btif_create_besaud_extra_channel(void* remote_addr, bt_l2cap_callback_t l2cap_callback);
int btif_check_l2cap_mtu_buffer_available(void);
void btif_colist_addto_head(struct list_node *n, struct list_node *head);
void btif_colist_addto_tail(struct list_node *n, struct list_node *head);
void btif_colist_insert_after(struct list_node *n, struct list_node *head);
void btif_colist_delete(struct list_node *entry);
int btif_colist_is_node_on_list(struct list_node *list, struct list_node *node);
int btif_colist_item_count(struct list_node *list);
struct list_node *btif_colist_get_head(struct list_node *head);
int btif_colist_is_list_empty(struct list_node *head);
BOOL btif_is_list_circular(list_entry_t * list);
void btif_insert_tail_list(list_entry_t * head, list_entry_t * entry);
list_entry_t *btif_remove_head_list(list_entry_t * head);
void btif_remove_entry_list(list_entry_t * entry);
void btif_ecc_gen_new_secret_key_192(uint8_t* secret_key192);
void btif_ecc_gen_new_public_key_192(uint8_t* secret_key,uint8_t* out_public_key);
void btif_hci_enable_cmd_evt_debug(bool enable);
void btif_hci_enable_tx_flow_debug(bool enable);
void btif_hci_enable_tx_0c35_without_alloc(bool enable);
void btif_hci_register_controller_state_check(void (*cb)(void));
void btif_hci_register_pending_too_many_rx_acl_packets(void (*cb)(void));
#ifdef IBRT
void btif_hci_set_start_ibrt_reserve_buff(bool reserve);
void btif_hci_register_acl_tx_buff_tss_process(void (*cb)(void));
#endif
uint8_t *btif_hci_get_curr_pending_cmd(uint16_t cmd_opcode);
uint16_t btif_hci_get_curr_cmd_opcode(void);
int btif_hci_count_free_bt_tx_buff(void);

void btif_pts_av_create_channel(bt_bdaddr_t *btaddr);
void btif_pts_av_set_sink_delay(void);
void btif_pts_rfc_register_channel(void);
void btif_pts_rfc_close(void);
void btif_pts_rfc_close_dlci_0(void);
void btif_pts_rfc_send_data(void);
void btif_pts_av_send_discover(void);
void btif_pts_av_send_getcap(void);
void btif_pts_av_send_setconf(void);
void btif_pts_av_send_getconf(void);
void btif_pts_av_send_reconf(void);
void btif_pts_av_send_open(void);
void btif_pts_av_send_close(void);
void btif_pts_av_send_abort(void);
void btif_pts_av_send_getallcap(void);
void btif_pts_av_send_suspend(void);
void btif_pts_av_send_start(void);
void btif_pts_av_send_security_control(void);
void btif_pts_av_create_media_channel(void);
void btif_pts_l2c_disc_channel(void);
void btif_pts_l2c_send_data(void);
void btif_register_mhdt_mode_change_callback(void (*cb)(struct bdaddr_t remote, bool isIn_mhdt_mode));
void btif_register_is_pts_address_check_callback(bool (*cb)(void *remote));
btif_remote_device_t *btif_cmgr_pts_get_remDev(btif_cmgr_handler_t *cmgr_handler);

#if defined(BT_SOURCE)
void btif_pts_source_cretae_media_channel(void);
void btif_pts_source_send_close_cmd(void);
void btif_pts_source_send_discover_cmd(void);
void btif_pts_source_send_get_capability_cmd(void);
void btif_pts_source_send_set_configuration_cmd(void);
void btif_pts_source_send_get_configuration_cmd(void);
void btif_pts_source_send_reconfigure_cmd(void);
void btif_pts_source_send_open_cmd(void);
void btif_pts_source_send_abort_cmd(void);
void btif_pts_source_send_get_all_capability_cmd(void);
void btif_pts_source_send_suspend_cmd(void);
void btif_pts_source_send_start_cmd(void);
void btif_update_name_and_uuid(btif_eir_raw_data_t *eir_raw_data);
#endif

#ifdef BQB_PROFILE_TEST
void btif_pts_reject_INVALID_OBJECT_TYPE(void);
void btif_pts_reject_INVALID_CHANNELS(void);
void btif_pts_reject_INVALID_SAMPLING_FREQUENCY(void);
void btif_pts_reject_INVALID_DRC(void);
void btif_pts_reject_NOT_SUPPORTED_OBJECT_TYPE(void);
void btif_pts_reject_NOT_SUPPORTED_CHANNELS(void);
void btif_pts_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY(void);
void btif_pts_reject_NOT_SUPPORTED_DRC(void);
void btif_pts_reject_INVALID_CODEC_TYPE(void);
void btif_pts_reject_INVALID_CHANNEL_MODE(void);
void btif_pts_reject_INVALID_SUBBANDS(void);
void btif_pts_reject_INVALID_ALLOCATION_METHOD(void);
void btif_pts_reject_INVALID_MINIMUM_BITPOOL_VALUE(void);
void btif_pts_reject_INVALID_MAXIMUM_BITPOOL_VALUE(void);
void btif_pts_reject_INVALID_BLOCK_LENGTH(void);
void btif_pts_reject_INVALID_CP_TYPE(void);
void btif_pts_reject_INVALID_CP_FORMAT(void);
void btif_pts_reject_NOT_SUPPORTED_CODEC_TYPE(void);
#endif

void btif_config_ctkd_over_br_edr(bool isEnable);

#ifdef __cplusplus
}
#endif /*  */

#endif /*__BT_IF_H_*/
