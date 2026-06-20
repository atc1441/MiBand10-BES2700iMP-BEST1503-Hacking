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
#ifndef __MAP_API_H__
#define __MAP_API_H__

#ifdef BT_MAP_SUPPORT

#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

void btif_map_init(void);

bt_map_state_t btif_map_get_state(uint16_t connhdl);

bt_status_t btif_map_connect(const bt_bdaddr_t *remote);

void btif_map_disconnect(const bt_bdaddr_t *remote);

void btif_map_set_obex_over_rfcomm(bool over_rfcomm);

void btif_map_send_obex_disconnect_req(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_send_obex_connect_req(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_dont_auto_send_obex_conn_req(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_connect_mas(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_open_mas_channel(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_open_all_mas_channel(uint16_t connhdl);

void btif_map_close_mas_channel(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_send_mns_obex_disc_req(uint16_t connhdl);

void btif_map_close_mns_channel(uint16_t connhdl);

void btif_map_enter_to_root_folder(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_enter_to_parent_folder(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_enter_to_child_folder(uint16_t connhdl, uint8_t mas_instance_id, const char* folder_name);

void btif_map_enter_to_msg_folder(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_send_message(uint16_t connhdl, uint8_t mas_instance_id, struct bt_map_message_t *msg, struct bt_map_send_param_t *param);

void btif_map_put_email_start(uint16_t connhdl, uint8_t mas_instance_id, struct bt_map_message_t *email, uint16_t email_count);

void btif_map_put_email_continue(uint16_t connhdl, uint8_t mas_instance_id, const char *content_data);

void btif_map_put_email_end(uint16_t connhdl, uint8_t mas_instance_id, const char *content_data);

void btif_map_notify_register(uint16_t connhdl, uint8_t mas_instance_id, bool on);

void btif_map_notify_filter(uint16_t connhdl, uint8_t mas_instance_id, uint32_t notify_masks);

void btif_map_get_instance_info(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_get_object(uint16_t connhdl, uint8_t mas_instance_id, const char *type, const char *name);

void btif_map_update_inbox(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_set_msg_read_status(uint16_t connhdl, uint8_t mas_instance_id, uint64_t message_handle, bool set_read);

void btif_map_set_msg_delete_status(uint16_t connhdl, uint8_t mas_instance_id, uint64_t message_handle, bool set_deleted);

void btif_map_set_msg_extended_data(uint16_t connhdl, uint8_t mas_instance_id, uint64_t message_handle, const char* data);

void btif_map_get_folder_listing_size(uint16_t connhdl, uint8_t mas_instance_id);

void btif_map_get_folder_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_folder_listing_param_t *param);

void btif_map_set_srm_in_wait(uint16_t connhdl, uint8_t mas_instance_id, bool wait);

void btif_map_get_message(uint16_t connhdl, uint8_t mas_instance_id, uint64_t message_handle);

void btif_map_get_msg_listing_size(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_msg_listing_param_t *param);

void btif_map_get_message_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_msg_listing_param_t *param);

void btif_map_set_owner_status(uint16_t connhdl, uint8_t mas_instance_id, struct map_set_owner_status_param_t *param);

void btif_map_get_owner_status(uint16_t connhdl, uint8_t mas_instance_id, const char *conversation_id);

void btif_map_get_convo_listing_size(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_convo_listing_param_t *param);

void btif_map_get_convo_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_convo_listing_param_t *param);

#if defined(IBRT)
uint32_t btif_map_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_map_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BT_MAP_SUPPORT */

#endif /* __MAP_API_H__ */
