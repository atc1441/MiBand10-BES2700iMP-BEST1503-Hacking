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
#ifndef __SPP_API_H__
#define __SPP_API_H__
#include "bluetooth.h"
#ifdef __cplusplus
extern "C" {
#endif

#define BT_SPP_MAX_SERVER_LISTEN_INSTANCES 5
#define BTIF_SPP_MAX_DEVICE_NUM 5

struct btif_spp_list_t {
    struct list_node spp_list;
};

struct btif_spp_global_t {
    struct list_node port_list;
    bool spp_is_initialized;
    uint8_t bt_spp_server_listen_used_instance_cnt;
    bt_spp_callback_t bt_spp_server_listen_callbacks[BT_SPP_MAX_SERVER_LISTEN_INSTANCES];
    struct btif_spp_list_t sink_device[BTIF_SPP_MAX_DEVICE_NUM];
    struct btif_spp_list_t source_device[BTIF_SPP_MAX_DEVICE_NUM];
    struct btif_spp_list_t tws_device;
};

void btif_spp_init(void);
struct btif_spp_global_t *btif_spp_get_global(void);
uint32_t btif_rfc_get_session_l2c_handle(const bt_bdaddr_t *remote);
bool btif_rfc_is_dlci_channel_connected(uint32_t session_handle, uint64_t app_id);
bt_status_t btif_spp_get_record_uuid(const bt_spp_channel_t *spp_chan, bt_spp_uuid_t *out);
bt_status_t btif_spp_create_port(uint8_t local_server_channel, const bt_sdp_record_attr_t *attr_list, uint16_t attr_count);
bt_status_t btif_spp_set_callback(uint8_t local_server_channel, uint16_t rx_buff_size, bt_spp_callback_t spp_callback, bt_spp_callback_t client_callback);
bt_status_t btif_spp_set_app_layer_give_credit(uint8_t local_server_channel, bool app_layer_give_credit);
bt_status_t btif_spp_listen(uint8_t local_server_channel, bool support_multi_device, bt_socket_accept_callback_t accept_callback);
bt_status_t btif_spp_remove_listen(uint8_t local_server_channel);
bt_status_t btif_spp_connect(const bt_bdaddr_t *remote, uint8_t local_server_channel, const uint8_t *uuid, uint16_t uuid_len);
bt_status_t btif_spp_connect_server_channel(const bt_bdaddr_t *remote, uint8_t local_server_channel, uint8_t remote_server_channel);
bt_status_t btif_spp_disconnect(uint32_t rfcomm_handle, uint8_t reason);
bt_status_t btif_spp_write(uint32_t rfcomm_handle, const uint8_t *data, uint16_t size);
bt_status_t btif_spp_server_listen(const bt_spp_server_config_t* config);
bt_status_t btif_spp_server_remove_listen(uint8_t local_server_channel);
bt_status_t btif_spp_give_handled_credits(uint32_t rfcomm_handle, uint16_t handled_credits);
bt_spp_channel_t *btif_spp_create_channel(uint8_t device_id, uint8_t local_server_channel);
bool btif_spp_is_connected(uint32_t rfcomm_handle);
bool btif_spp_is_port_connected(const bt_bdaddr_t *remote, uint8_t local_server_channel);
uint16_t btif_spp_count_free_tx_buf_cnt(uint32_t rfcomm_handle);
uint64_t btif_spp_get_app_id_from_port(uint8_t local_server_channel);
uint8_t btif_spp_get_port_from_app_id(uint64_t spp_app_id);

#if defined(IBRT)
void btif_spp_ibrt_slave_release_dlc_connection(uint32_t device_id, uint32_t dlci);
void btif_spp_force_disconnect_spp_profile(const bt_bdaddr_t *addr, uint64_t app_id, uint8_t reason);
uint32_t btif_spp_profile_save_ctx(uint64_t app_id, const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_spp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __SPP_API_H__ */
