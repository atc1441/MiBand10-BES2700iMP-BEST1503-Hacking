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
#ifndef __BT_AVRCP_SERVICE_H__
#define __BT_AVRCP_SERVICE_H__
#include "adapter_service.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * standard avrcp (controller/target role) interface
 *
 */

#define AVRCP_KEY_PLAY          BT_AVRCP_KEY_PLAY
#define AVRCP_KEY_STOP          BT_AVRCP_KEY_STOP
#define AVRCP_KEY_PAUSE         BT_AVRCP_KEY_PAUSE
#define AVRCP_KEY_FORWARD       BT_AVRCP_KEY_FORWARD
#define AVRCP_KEY_BACKWARD      BT_AVRCP_KEY_BACKWARD
#define AVRCP_KEY_VOLUME_UP     BT_AVRCP_KEY_VOLUME_UP
#define AVRCP_KEY_VOLUME_DOWN   BT_AVRCP_KEY_VOLUME_DOWN
#define AVRCP_KEY_FAST_FORWARD_START BT_AVRCP_KEY_FAST_FORWARD_START
#define AVRCP_KEY_FAST_FORWARD_STOP  BT_AVRCP_KEY_FAST_FORWARD_STOP
#define AVRCP_KEY_REWIND_START BT_AVRCP_KEY_REWIND_START
#define AVRCP_KEY_REWIND_STOP  BT_AVRCP_KEY_REWIND_STOP

#define AVRCP_COMPANY_ID_MAX_NUM        12

typedef enum {
    BT_AVRCP_KEY_NULL           = 0,
    BT_AVRCP_KEY_VOLUME_UP      = 0x41,
    BT_AVRCP_KEY_VOLUME_DOWN    = 0x42,
    BT_AVRCP_KEY_MUTE           = 0x43,
    BT_AVRCP_KEY_PLAY           = 0x44,
    BT_AVRCP_KEY_STOP           = 0x45,
    BT_AVRCP_KEY_PAUSE          = 0x46,
    BT_AVRCP_KEY_RECORD         = 0x47,
    BT_AVRCP_KEY_REWIND         = 0x48,
    BT_AVRCP_KEY_FAST_FORWARD   = 0x49,
    BT_AVRCP_KEY_EJECT          = 0x4A,
    BT_AVRCP_KEY_FORWARD        = 0x4B,
    BT_AVRCP_KEY_BACKWARD       = 0x4C,
    BT_AVRCP_KEY_FAST_FORWARD_START     = 0X4D,
    BT_AVRCP_KEY_FAST_FORWARD_STOP      = 0X4E,
    BT_AVRCP_KEY_REWIND_START           = 0X4F,
    BT_AVRCP_KEY_REWIND_STOP            = 0X50
} bt_avrcp_key_code_t;

typedef enum {
    BT_AVRCP_KEY_STATE_PRESS,
    BT_AVRCP_KEY_STATE_RELEASE,
} bt_avrcp_key_state_t;

typedef enum {
    BT_AVRCP_PLAY_STATUS_CHANGED            = 0x01,
    BT_AVRCP_TRACK_CHANGED                  = 0x02,
    BT_AVRCP_TRACE_REACHED_END              = 0x03,
    BT_AVRCP_TRACK_REACHED_START            = 0x04,
    BT_AVRCP_PLAY_POS_CHANGED               = 0x05,
    BT_AVRCP_BATT_STATUS_CHANGED            = 0x06,
    BT_AVRCP_SYSTEM_STATUS_CHANGED          = 0x07,
    BT_AVRCP_APP_SETTING_CHANGED            = 0x08,
    BT_AVRCP_NOW_PLAYING_CONTENT_CHANGED    = 0x09,
    BT_AVRCP_AVAL_PLAYER_CHANGED            = 0x0a,
    BT_AVRCP_ADDR_PLAYER_CHANGED            = 0x0b,
    BT_AVRCP_UIDS_CHANGED                   = 0x0c,
    BT_AVRCP_VOLUME_CHANGED                 = 0x0d,
} bt_avrcp_status_change_event_t;

typedef enum {
    BT_AVRCP_PLAY_STATUS_STOPPED    = 0x00,
    BT_AVRCP_PLAY_STATUS_PLAYING    = 0x01,
    BT_AVRCP_PLAY_STATUS_PAUSED     = 0x02,
    BT_AVRCP_PLAY_STATUS_FWD_SEEK   = 0x03,
    BT_AVRCP_PLAY_STATUS_REV_SEEK   = 0x04,
    BT_AVRCP_PLAY_STATUS_ERROR      = 0xff,
} bt_avrcp_play_status_t;

typedef enum {
    BT_AVRCP_BATTERY_STATUS_NORMAL      = 0x00,
    BT_AVRCP_BATTERY_STATUS_WARNING     = 0x01,
    BT_AVRCP_BATTERY_STATUS_CRITICAL    = 0x02,
    BT_AVRCP_BATTERY_STATUS_EXTERNAL    = 0x03,
    BT_AVRCP_BATTERY_STATUS_FULLCHARGE  = 0x04,
} bt_avrcp_battery_status_t;

typedef enum {
    BT_AVRCP_MEDIA_ATTR_INVALID         = 0x00,
    BT_AVRCP_MEDIA_ATTR_TITLE           = 0x01, // (M) text representing the title, song name or content description
    BT_AVRCP_MEDIA_ATTR_ARTIST          = 0x02, // text representing artist(s), performer(s), or group
    BT_AVRCP_MEDIA_ATTR_ALBUM           = 0x03, // text representing the title of the recording
    BT_AVRCP_MEDIA_ATTR_TRACK_NUM       = 0x04, // numeric ascii string containing the order number of the recording
    BT_AVRCP_MEDIA_ATTR_NUM_TRACKS      = 0x05, // numeric ascii string containing the total number of trackes
    BT_AVRCP_MEDIA_ATTR_GENRE           = 0x06, // text representing the category of the composition by a style
    BT_AVRCP_MEDIA_ATTR_DURATION        = 0x07, // numeric ascii string contianing the length of audio file in ms
    BT_AVRCP_MEDIA_ATTR_COVER_HANDLE    = 0x08, // default cover art, bip image handle
} bt_avrcp_media_attr_t;

typedef enum {
    BT_AVRCP_ATTR_MASK_TITLE            = 0x01,
    BT_AVRCP_ATTR_MASK_ARTIST           = 0x02,
    BT_AVRCP_ATTR_MASK_ALBUM            = 0x04,
    BT_AVRCP_ATTR_MASK_TRACK            = 0x08,
    BT_AVRCP_ATTR_MASK_NUM_TRACKS       = 0x10,
    BT_AVRCP_ATTR_MASK_GENRE            = 0x20,
    BT_AVRCP_ATTR_MASK_DURATION         = 0x40,
    BT_AVRCP_ATTR_MASK_COVER_HANDLE     = 0x80,
} bt_avrcp_attr_masks_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
} bt_avrcp_opened_t;

typedef struct {
    uint8_t error_code;
    uint8_t device_id;
} bt_avrcp_closed_t;

typedef struct {
    uint8_t key_code;
    uint8_t is_pressed;
} bt_avrcp_key_info_t;

typedef struct
{
    unsigned char company_num;
    unsigned int companyid[AVRCP_COMPANY_ID_MAX_NUM];
} bt_avrcp_get_companyid_t;

typedef struct {
    uint16_t eventMask;
} bt_avrcp_support_mask_t;

typedef avrcp_adv_notify_parms_t bt_avrcp_adv_notify_parms_t;

typedef struct {
    uint8_t device_id;
    uint8_t data_len;
    uint8_t* data;
} bt_avrcp_browsing_rsp_t;

typedef struct {
    uint8_t device_id;
    uint8_t cmd_trans_id;
    uint8_t data_len;
    uint8_t* data;
} bt_avrcp_browsing_cmd_t;

typedef struct {
    uint8_t error_code;
    bool is_final_packet;
    uint16_t body_data_length;
    uint8_t *body_data;
} bt_avrcp_obex_get_info_t;

typedef struct {
    uint8_t subevt;
    uint8_t error_code;
    uint8_t opcode;
    bool is_final_packet;
    uint16_t body_data_length;
    uint8_t *body_data;
} bt_avrcp_obex_user_callback_parm_t;

typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
} bt_avrcp_status_header_t;

typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
    uint8_t volume;
    bool is_set_abs_vol; // the volume is set by SetAbsVolume
    uint8_t trans_id; // for SetAbsVolume cmd
} bt_avrcp_volume_change_t;

typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
    bt_avrcp_play_status_t play_status;
} bt_avrcp_play_status_change_t;

typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
    uint32_t position;
} bt_avrcp_play_pos_change_t;

typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
    bt_avrcp_battery_status_t battery;
} bt_avrcp_battery_change_t;

/**
 * track_id: if no track is currently selected, then return 0xFFFFFFFFFFFFFFFF in
 * the INTERIM response. if browsing is not supported and a track is selected,
 * then return 0x0 in the response. if browsing is supported and a track is selected,
 * the identifier shall correspond to the currently played media element as listed
 * in the NowPlaying folder.
 *
 */
typedef struct {
    bt_avrcp_status_change_event_t event;
    uint8_t device_id;  // dont change field order
    uint8_t interim;    // dont change field order
    uint64_t track_id;
} bt_avrcp_track_change_t;

typedef union {
    bt_avrcp_status_header_t header;
    bt_avrcp_volume_change_t volume_change;
    bt_avrcp_play_status_change_t play_status_change;
    bt_avrcp_play_pos_change_t play_pos_change;
    bt_avrcp_track_change_t track_change;
    bt_avrcp_battery_change_t battery_change;
} bt_avrcp_status_change_t;

typedef struct {
    uint8_t device_id;
    bt_avrcp_status_change_event_t event;
} bt_avrcp_recv_register_notify_t;

typedef struct {
    uint8_t device_id;
    uint8_t trans_id;
    bt_avrcp_key_code_t key_code;
    bt_avrcp_key_state_t key_state;
} bt_avrcp_recv_passthrough_cmd_t;

/**
 * If TG does not support SongLength and SongPosition on TG, then TG shall
 * return 0xFFFFFFFF.
 *
 */
typedef struct {
    uint8_t device_id;
    bt_avrcp_play_status_t play_status;
    uint32_t song_length; // total length of the playing song in ms
    uint32_t song_position; // current position of the playing in ms elapsed
} bt_avrcp_recv_play_status_t;

typedef struct {
    bt_avrcp_media_attr_t attr_id;
    uint16_t charset;
    uint16_t length;
    const char *attr_value;
} bt_avrcp_elem_attr_value_t;

typedef struct {
    uint8_t device_id;
    uint8_t num_attrs;
    bt_avrcp_elem_attr_value_t *attrs;
} bt_avrcp_recv_media_status_t;

typedef struct
{
    uint8_t scope;
    uint32_t start_item;
    uint32_t end_item;
    uint32_t mediaMask;
} bt_avrcp_get_folder_items_t;

typedef struct
{
    uint8_t scope;
    uint16_t UID_Counter;
    uint32_t UID;
    uint32_t mediaMask;
} bt_avrcp_get_item_att_t;

typedef struct
{
    uint16_t UID_Counter;
    uint32_t direction;
    uint32_t UID;
} bt_avrcp_change_patch_t;

typedef union {
    bt_avrcp_opened_t *opened;
    bt_avrcp_closed_t *closed;
    bt_avrcp_volume_change_t *volume_change;
    bt_avrcp_play_status_change_t *play_status_change;
    bt_avrcp_play_pos_change_t *play_pos_change;
    bt_avrcp_track_change_t *track_change;
    bt_avrcp_battery_change_t *battery_change;
    bt_avrcp_recv_register_notify_t *register_notify;
    bt_avrcp_recv_passthrough_cmd_t *passthrough_cmd;
    bt_avrcp_recv_play_status_t *recv_play_status;
    bt_avrcp_recv_media_status_t *recv_media_status;
    bt_avrcp_key_info_t *key_info;
    bt_avrcp_get_companyid_t *companyid;
    bt_avrcp_support_mask_t *support_mask;
    bt_avrcp_adv_notify_parms_t *notify_info;
    bt_avrcp_browsing_rsp_t *browsing_rsp;
    bt_avrcp_browsing_cmd_t *browsing_cmd;
    bt_avrcp_obex_get_info_t *get_context;
} bt_avrcp_callback_param_t;

typedef enum {
    BT_AVRCP_EVENT_OPENED = BT_EVENT_AVRCP_OPENED,
    BT_AVRCP_EVENT_CLOSED,
    BT_AVRCP_EVENT_VOLUME_CHANGE,
    BT_AVRCP_EVENT_PLAY_STATUS_CHANGE,
    BT_AVRCP_EVENT_PLAY_POS_CHANGE,
    BT_AVRCP_EVENT_TRACK_CHANGE,
    BT_AVRCP_EVENT_BATTERY_CHANGE,
    BT_AVRCP_EVENT_RECV_PASSTHROUGH_CMD,
    BT_AVRCP_EVENT_RECV_PLAY_STATUS,
    BT_AVRCP_EVENT_RECV_MEDIA_STATUS,
    BT_AVRCP_EVENT_END,
} bt_avrcp_event_t;

#if BT_AVRCP_EVENT_END != BT_EVENT_AVRCP_END
#error "bt_avrcp_event_t error define"
#endif

typedef int (*bt_avrcp_callback_t)(const bt_bdaddr_t *bd_addr, bt_avrcp_event_t event, bt_avrcp_callback_param_t param);

bt_status_t bt_avrcp_init(bt_avrcp_callback_t callback);
bt_status_t bt_avrcp_cleanup(void);
bt_status_t bt_avrcp_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_avrcp_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_avrcp_send_passthrough_cmd(const bt_bdaddr_t *bd_addr, bt_avrcp_key_code_t key);
bt_status_t bt_avrcp_send_get_play_status(const bt_bdaddr_t *bd_addr);
bt_status_t bt_avrcp_send_get_media_status(const bt_bdaddr_t *bd_addr, uint32_t attr_masks);
bt_status_t bt_avrcp_send_set_abs_volume(const bt_bdaddr_t *bd_addr, uint8_t volume);
bt_status_t bt_avrcp_report_status_change(const bt_bdaddr_t *bd_addr, bt_avrcp_status_change_event_t event, uint32_t param);

#ifdef __cplusplus
}
#endif
#endif /* __BT_AVRCP_SERVICE_H__ */

