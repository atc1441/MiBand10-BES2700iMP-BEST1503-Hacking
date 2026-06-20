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
#ifndef __BES_AVRCP_API_H__
#define __BES_AVRCP_API_H__
#include "a2dp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef U16 bes_avrcp_panel_operation_t;

#define BES_BT_AVRCP_POP_SELECT            0x0000
#define BES_BT_AVRCP_POP_UP                0x0001
#define BES_BT_AVRCP_POP_DOWN              0x0002
#define BES_BT_AVRCP_POP_LEFT              0x0003
#define BES_BT_AVRCP_POP_RIGHT             0x0004
#define BES_BT_AVRCP_POP_RIGHT_UP          0x0005
#define BES_BT_AVRCP_POP_RIGHT_DOWN        0x0006
#define BES_BT_AVRCP_POP_LEFT_UP           0x0007
#define BES_BT_AVRCP_POP_LEFT_DOWN         0x0008
#define BES_BT_AVRCP_POP_ROOT_MENU         0x0009
#define BES_BT_AVRCP_POP_SETUP_MENU        0x000A
#define BES_BT_AVRCP_POP_CONTENTS_MENU     0x000B
#define BES_BT_AVRCP_POP_FAVORITE_MENU     0x000C
#define BES_BT_AVRCP_POP_EXIT              0x000D

#define BES_BT_AVRCP_POP_0                 0x0020
#define BES_BT_AVRCP_POP_1                 0x0021
#define BES_BT_AVRCP_POP_2                 0x0022
#define BES_BT_AVRCP_POP_3                 0x0023
#define BES_BT_AVRCP_POP_4                 0x0024
#define BES_BT_AVRCP_POP_5                 0x0025
#define BES_BT_AVRCP_POP_6                 0x0026
#define BES_BT_AVRCP_POP_7                 0x0027
#define BES_BT_AVRCP_POP_8                 0x0028
#define BES_BT_AVRCP_POP_9                 0x0029
#define BES_BT_AVRCP_POP_DOT               0x002A
#define BES_BT_AVRCP_POP_ENTER             0x002B
#define BES_BT_AVRCP_POP_CLEAR             0x002C

#define BES_BT_AVRCP_POP_CHANNEL_UP        0x0030
#define BES_BT_AVRCP_POP_CHANNEL_DOWN      0x0031
#define BES_BT_AVRCP_POP_PREVIOUS_CHANNEL  0x0032
#define BES_BT_AVRCP_POP_SOUND_SELECT      0x0033
#define BES_BT_AVRCP_POP_INPUT_SELECT      0x0034
#define BES_BT_AVRCP_POP_DISPLAY_INFO      0x0035
#define BES_BT_AVRCP_POP_HELP              0x0036
#define BES_BT_AVRCP_POP_PAGE_UP           0x0037
#define BES_BT_AVRCP_POP_PAGE_DOWN         0x0038

#define BES_BT_AVRCP_POP_POWER             0x0040
#define BES_BT_AVRCP_POP_VOLUME_UP         0x0041
#define BES_BT_AVRCP_POP_VOLUME_DOWN       0x0042
#define BES_BT_AVRCP_POP_MUTE              0x0043
#define BES_BT_AVRCP_POP_PLAY              0x0044
#define BES_BT_AVRCP_POP_STOP              0x0045
#define BES_BT_AVRCP_POP_PAUSE             0x0046
#define BES_BT_AVRCP_POP_RECORD            0x0047
#define BES_BT_AVRCP_POP_REWIND            0x0048
#define BES_BT_AVRCP_POP_FAST_FORWARD      0x0049
#define BES_BT_AVRCP_POP_EJECT             0x004A
#define BES_BT_AVRCP_POP_FORWARD           0x004B
#define BES_BT_AVRCP_POP_BACKWARD          0x004C

#define BES_BT_AVRCP_POP_ANGLE             0x0050
#define BES_BT_AVRCP_POP_SUBPICTURE        0x0051

#define BES_BT_AVRCP_POP_F1                0x0071
#define BES_BT_AVRCP_POP_F2                0x0072
#define BES_BT_AVRCP_POP_F3                0x0073
#define BES_BT_AVRCP_POP_F4                0x0074
#define BES_BT_AVRCP_POP_F5                0x0075

#define BES_BT_AVRCP_POP_VENDOR_UNIQUE     0x007E

#define BES_BT_AVRCP_POP_NEXT_GROUP        0x017E
#define BES_BT_AVRCP_POP_PREV_GROUP        0x027E

#define BES_BT_AVRCP_POP_RESERVED          0x007F

#define BES_BT_AVRCP_NUM_EVENTS            13
#define BES_BT_AVRCP_MAX_NOTIFICATIONS     15

typedef uint8_t bes_bt_avrcp_capabilityId;

#define BES_BT_AVRCP_CAPABILITY_COMPANY_ID        2
#define BES_BT_AVRCP_CAPABILITY_EVENTS_SUPPORTED  3

typedef enum {
    BES_BT_AVRCP_ATTR_MASK_TITLE            = 0x01,
    BES_BT_AVRCP_ATTR_MASK_ARTIST           = 0x02,
    BES_BT_AVRCP_ATTR_MASK_ALBUM            = 0x04,
    BES_BT_AVRCP_ATTR_MASK_TRACK            = 0x08,
    BES_BT_AVRCP_ATTR_MASK_NUM_TRACKS       = 0x10,
    BES_BT_AVRCP_ATTR_MASK_GENRE            = 0x20,
    BES_BT_AVRCP_ATTR_MASK_DURATION         = 0x40,
    BES_BT_AVRCP_ATTR_MASK_COVER_HANDLE     = 0x80,
} bes_bt_avrcp_attr_masks_t;

bool bt_export_avrcp_channel_is_open(int device_id);
bt_status_t bes_bt_avrcp_profile_connect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_profile_disconnect(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_play_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_pause_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_volume_up_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_volume_down_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_forward_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_backward_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_fast_forward_press_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_fast_forward_release_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_rewind_press_cmd(const bt_bdaddr_t *remote);
bt_status_t bes_bt_avrcp_send_rewind_release_cmd(const bt_bdaddr_t *remote);
bt_avrcp_state_t bes_bt_avrcp_get_state(int device_id);
bool bes_bt_avrcp_channel_is_open(int device_id);
uint8_t bes_bt_avrcp_cb_get_event(const avrcp_callback_parms_t * parms);
bt_status_t bes_bt_avrcp_cb_get_channel_state(const avrcp_callback_parms_t * parms);
uint8_t bes_bt_avrcp_cb_get_adv_op(const avrcp_callback_parms_t * parms);
avrcp_adv_notify_parms_t *bes_bt_avrcp_cb_get_adv_notify(const avrcp_callback_parms_t * parms);
avctp_cmd_frame_t *bes_bt_avrcp_cb_get_cmd_frame(const avrcp_callback_parms_t * parms);
struct avrcp_remote_sdp_info bes_bt_avrcp_get_remote_sdp_info(btif_avrcp_channel_t * channel, bool is_target);
bool bes_bt_avrcp_is_initiator(const bt_bdaddr_t* remote);
void bes_bt_avrcp_ct_register_notification_event(uint8_t device_id, uint8_t event);
bool bes_bt_avrcp_is_remote_ct_info_got(const bt_bdaddr_t *remote);
uint8_t bes_bt_avrcp_get_volume_change_trans_id(uint8_t device_id);
void bes_bt_avrcp_set_volume_change_trans_id(uint8_t device_id, uint8_t trans_id);
uint8_t bes_bt_avrcp_get_ctl_trans_id(uint8_t device_id);
void bes_bt_avrcp_set_ctl_trans_id(uint8_t device_id, uint8_t trans_id);
bt_bdaddr_t *bes_bt_avrcp_get_remote_device_addr(btif_avrcp_channel_t* handle);
btif_avrcp_channel_t *bes_bt_get_avrcp_channel_by_addr(uint8_t* addr);
bt_status_t bes_bt_avrcp_send_volume_notify_rsp(const bt_bdaddr_t *bd_addr, uint8_t volume);
uint8_t bes_bt_avcp_get_connect_status(uint8_t* addr);
bt_status_t bes_bt_avrcp_key_operation(const bt_bdaddr_t *remote, bes_avrcp_panel_operation_t key, bool is_press);
bt_status_t bes_bt_avrcp_ct_get_capabilities(const bt_bdaddr_t *remote, bes_bt_avrcp_capabilityId capabilityId);
void bes_bt_avrcp_ct_callback(uint8_t device_id, btif_avrcp_channel_t* btif_avrcp, const avrcp_callback_parms_t* parms);
bt_status_t bes_bt_avrcp_ct_register_play_pos_notification(const bt_bdaddr_t *remote, uint32_t interval);
bt_status_t bes_bt_avrcp_ct_register_media_status_notification(const bt_bdaddr_t *remote, uint32_t interval);
bt_status_t bes_bt_avrcp_ct_register_track_change_notification(const bt_bdaddr_t *remote, uint32_t interval);
bt_status_t bes_bt_avrcp_ct_register_volume_change_notification(const bt_bdaddr_t *remote, uint32_t interval);
bt_status_t bta_avrcp_send_get_media_status(const bt_bdaddr_t *bd_addr, uint32_t attr_masks);
bt_status_t bta_avrcp_set_addressed_player(const bt_bdaddr_t *remote, uint16_t player_id);
bt_status_t bta_avrcp_set_browsed_player(const bt_bdaddr_t *remote, uint16_t player_id);
bt_status_t bta_avrcp_ct_get_folder_items(const bt_bdaddr_t *remote, void *param);
bt_status_t bta_avrcp_ct_send_change_patch(const bt_bdaddr_t *remote, void *param);
bt_status_t bta_avrcp_ct_send_get_item_attributes(const bt_bdaddr_t *remote, void *param);
bt_status_t bta_avrcp_ct_send_search(const bt_bdaddr_t *remote, const char * string, uint16_t string_len);
bt_status_t bta_avrcp_ct_get_total_number_of_items(const bt_bdaddr_t *remote, uint8_t scope);
bt_status_t bta_avrcp_send_cover_art_get_image_properties(const bt_bdaddr_t *remote, const char *image_handle);
bt_status_t bta_avrcp_send_cover_art_get_image(const bt_bdaddr_t *remote, const char *image_handle, const char *descriptor, uint16_t descriptor_len);
bt_status_t bta_avrcp_send_cover_art_get_linked_thumbnail(const bt_bdaddr_t *remote, const char *image_handle);
#ifdef __cplusplus
}
#endif
#endif /* __BES_AVRCP_API_H__ */
