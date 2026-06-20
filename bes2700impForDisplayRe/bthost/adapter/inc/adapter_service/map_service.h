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
#ifndef __BT_MAP_SERVICE_H__
#define __BT_MAP_SERVICE_H__
#include "adapter_service.h"
#ifdef BT_MAP_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define MAP_DEFAULT_MAS_INSTANCE 0x00

#define MAP_FOLDER_NAME_MAX_LEN         (32)
#define MAP_VCARD_ELEM_MAX_LEN          (64)
#define MAP_DATETIME_DEFAULT_FMT_LEN    (15)
#define MAP_DATETIME_UTC_OFFSET_FMT_LEN (20)
#define MAP_CONVERSATION_ID_STR_LEN     (32)
#define MAP_PRESENCE_TEXT_MAX_LEN       (32)
#define MAP_HANDLE_VALUE_STR_LEN        (16)
#define MAP_FILTER_RECIPIENT_MAX_LEN    (32)
#define MAP_FILTER_ORIGINATOR_MAX_LEN   (32)

#if (MAP_FOLDER_NAME_MAX_LEN < MAP_CONVERSATION_ID_STR_LEN)
#error "MAP_FOLDER_NAME_MAX_LEN shall not less than MAP_CONVERSATION_ID_STR_LEN"
#endif

#define MAP_NOTIFY_MASK_NEW_MESSAGE               0x0001
#define MAP_NOTIFY_MASK_MESSAGE_DELETED           0x0002
#define MAP_NOTIFY_MASK_MESSAGE_SHIFT             0x0004
#define MAP_NOTIFY_MASK_SENDING_SUCCESS           0x0008
#define MAP_NOTIFY_MASK_SENDING_FAILURE           0x0010
#define MAP_NOTIFY_MASK_DELIVERY_SUCCESS          0x0020
#define MAP_NOTIFY_MASK_DELIVERY_FAILURE          0x0040
#define MAP_NOTIFY_MASK_MEMORY_FULL               0x0080
#define MAP_NOTIFY_MASK_MEMORY_AVAILABLE          0x0100
#define MAP_NOTIFY_MASK_READ_STATUS_CHANGED       0x0200
#define MAP_NOTIFY_MASK_CONVO_CHANGED             0x0400
#define MAP_NOTIFY_MASK_PARTI_PRESENCE_CHANGED    0x0800
#define MAP_NOTIFY_MASK_PARTI_CHATSTAT_CHANGED    0x1000
#define MAP_NOTIFY_MASK_MESSAGE_EXT_DATA_CHANGED  0x2000
#define MAP_NOTIFY_MASK_MESSAGE_REMOVED           0x4000

typedef enum {
    MAP_MSG_TYPE_UNKNOWN,
    MAP_MSG_TYPE_EMAIL,     // email RFC2822 or MIME type basis
    MAP_MSG_TYPE_SMS_GSM,   // GSM short message
    MAP_MSG_TYPE_SMS_CDMA,  // CDMA short message
    MAP_MSG_TYPE_MMS,       // 3GPP MMS
    MAP_MSG_TYPE_IM,        // Instant Message on MIME type basis
} map_msgtype_t;

struct bt_map_sms_t {
    const char *tel;
    const char *msg;
    uint16_t tel_len;
    uint16_t msg_len;
};

struct map_vcard_info_t {
    char name[MAP_VCARD_ELEM_MAX_LEN+1];
    char tel[MAP_VCARD_ELEM_MAX_LEN+1];
    char email[MAP_VCARD_ELEM_MAX_LEN+1];
};

struct map_msg_originator_t {
    bool has_originator;
    struct map_vcard_info_t vcard;
};

struct map_msg_recipient_t {
    uint16_t recipient_num;
    struct map_vcard_info_t *vcard;
};

struct bt_map_message_t {
    bool is_draft_msg;
    bool push_to_inbox_convo;
    bool is_native_sms_pdu;
    map_msgtype_t msg_type;
    struct map_vcard_info_t originator;
    struct map_msg_recipient_t recipient;
    const char *content_data;
    uint16_t content_len;
};

struct bt_map_send_param_t {
    bool transparent_sent;
    bool dont_retry_sent;
    const char *conversation_id;
    bool forward_attachment;    // for forward message
    bool prepend_to_origin_msg; // for forward message
    uint64_t origin_msg_handle; // for forward message
};

struct map_get_folder_listing_param_t {
    uint16_t max_list_count; // max num of entries shall be 1024 if this header is not specified
    uint16_t list_start_offset; // the offset shall be 0 if this header is not specified
};

#define MAP_MSG_LISTING_PARM_MASK_SUBJECT           0x00000001
#define MAP_MSG_LISTING_PARM_MASK_DATETIME          0x00000002
#define MAP_MSG_LISTING_PARM_MASK_SENDER_NAME       0x00000004
#define MAP_MSG_LISTING_PARM_MASK_SENDER_ADDRESS    0x00000008
#define MAP_MSG_LISTING_PARM_MASK_RECIPIENT_NAME    0x00000010
#define MAP_MSG_LISTING_PARM_MASK_RECIPIENT_ADDRESS 0x00000020
#define MAP_MSG_LISTING_PARM_MASK_TYPE              0x00000040
#define MAP_MSG_LISTING_PARM_MASK_SIZE              0x00000080
#define MAP_MSG_LISTING_PARM_MASK_RECEPTION_STATUS  0x00000100
#define MAP_MSG_LISTING_PARM_MASK_TEXT              0x00000200
#define MAP_MSG_LISTING_PARM_MASK_ATTACHMENT_SIZE   0x00000400
#define MAP_MSG_LISTING_PARM_MASK_PRIORITY          0x00000800
#define MAP_MSG_LISTING_PARM_MASK_READ              0x00001000
#define MAP_MSG_LISTING_PARM_MASK_SENT              0x00002000
#define MAP_MSG_LISTING_PARM_MASK_PROTECTED         0x00004000
#define MAP_MSG_LISTING_PARM_MASK_REPLYTO_ADDRESS   0x00008000
#define MAP_MSG_LISTING_PARM_MASK_DELIVERY_STATUS   0x00010000
#define MAP_MSG_LISTING_PARM_MASK_CONVO_ID          0x00020000
#define MAP_MSG_LISTING_PARM_MASK_CONVO_NAME        0x00040000
#define MAP_MSG_LISTING_PARM_MASK_DIRECTION         0x00080000
#define MAP_MSG_LISTING_PARM_MASK_ATTACHMENT_MIME   0x00100000

#define MAP_FILTER_MSG_TYPE_EXCEPT_SMS_GSM  0x01
#define MAP_FILTER_MSG_TYPE_EXCEPT_SMS_CDMA 0x02
#define MAP_FILTER_MSG_TYPE_EXCEPT_EMAIL    0x04
#define MAP_FILTER_MSG_TYPE_EXCEPT_MMS      0x08
#define MAP_FILTER_MSG_TYPE_EXCEPT_IM       0x10
#define MAP_FILTER_MSG_TYPE_GET_SMS_GSM     0x1E
#define MAP_FILTER_MSG_TYPE_GET_SMS_CDMA    0x1D
#define MAP_FILTER_MSG_TYPE_GET_EMAIL       0x1B
#define MAP_FILTER_MSG_TYPE_GET_MMS         0x17
#define MAP_FILTER_MSG_TYPE_GET_IM          0x0F

#define MAP_FILTER_READ_STATUS_GET_ALL_MSGS    0x00
#define MAP_FILTER_READ_STATUS_GET_UNREAD_MSGS 0x01
#define MAP_FILTER_READ_STATUS_GET_READED_MSGS 0x02

#define MAP_FILTER_PRIORITY_NO_FILTERING          0x00
#define MAP_FILTER_PRIORITY_GET_HIGH_PRIO_MSG     0x01
#define MAP_FILTER_PRIORITY_GET_NON_HIGH_PRIO_MSG 0x02

struct map_get_msg_listing_param_t {
    char folder_name[MAP_FOLDER_NAME_MAX_LEN+1]; // store conversation id when it is used
    bool use_conversation_id; // use convo id instead of folder_name
    bool use_message_handle; // use message handle instead of folder_name
    uint64_t message_handle; // only ParameterMask and SubjectLength are valid when use message handle
    uint16_t max_list_count;    // max number of entries returned
    uint16_t list_start_offset; // ignored when get msg listing size
    uint32_t parameter_masks;   // ignored when get msg listing size
    uint8_t subject_length;     // ignored when get msg listing size
    bool has_message_type_filter;
    uint8_t filter_msg_type_masks;
    uint8_t filter_read_status; // 0 "no filtering" get all read and unread messages
    uint8_t filter_priority; // 0 "no-filtering" get all high and non-high priority messages
    bool has_period_begin_filter;
    bool has_period_end_filter;
    char filter_period_begin[MAP_DATETIME_UTC_OFFSET_FMT_LEN+1];
    char filter_period_end[MAP_DATETIME_UTC_OFFSET_FMT_LEN+1];
    bool has_recipient_filter;
    bool has_originator_fileter;
    char recipient[MAP_FILTER_RECIPIENT_MAX_LEN+1];
    char originator[MAP_FILTER_ORIGINATOR_MAX_LEN+1];
};

typedef enum {
    MAP_RECIPI_STATUS_UNKNOWN,
    MAP_RECIPI_STATUS_COMPLETED,    // complete message has been received by the MSE
    MAP_RECIPI_STATUS_FRACTIONED,   // only a prt of the message has been received
    MAP_RECIPI_STATUS_NOTIFICATION, // only a notification about the message has been received
} map_recipient_status_t;

typedef enum {
    MAP_DELIVER_STATUS_UNKNOWN,
    MAP_DELIVER_STATUS_DELIVERED,   // the message has been delivered to the recipient
    MAP_DELIVER_STATUS_SENT,        // the message has been sent from the MSE to the network
} map_deliver_status_t;

typedef enum {
    MAP_DIRECTION_UNKNOWN,
    MAP_DIRECTION_INCOMING,
    MAP_DIRECTION_OUTGOING,
    MAP_DIRECTION_OUTGOINGDRAFT,
    MAP_DIRECTION_OUTGOINGPENDING,
} map_msg_direction_t;

struct map_msgtype_value_t {
    char* msg_type;
    uint16_t msg_type_len;
    map_msgtype_t msg_type_value;
    uint8_t padding;
} __attribute__ ((packed));

struct map_str_len_value_t {
    char *str;
    uint16_t len;
};

struct map_msg_listing_item_t {
    uint64_t handle;
    uint32_t size;
    uint32_t attachment_size;
    map_recipient_status_t recipient_status;
    map_deliver_status_t deliver_status;
    map_msg_direction_t direction;
    bool text;  // the message includes textual content or not
    bool priority;  // the message is of high priority or not
    bool read;  // the message has already been read on the MSE
    bool sent;  // the message has already been sent to the recipient
    bool is_protected; // the message or a part of the message is protected by a DRM scheme
    struct map_msgtype_value_t msg_type;
    struct map_str_len_value_t subject; // max 256
    struct map_str_len_value_t datetime; // max 20
    struct map_str_len_value_t sender_name; // max 256
    struct map_str_len_value_t sender_addressing; // max 256
    struct map_str_len_value_t replyto_addressing; // max 256
    struct map_str_len_value_t recipient_name; // max 256
    struct map_str_len_value_t recipient_addressing; // max 256
    struct map_str_len_value_t conversation_id; // max 32
    struct map_str_len_value_t conversation_name; // max 256
    struct map_str_len_value_t attachment_mime_types; // separated by comma
};

typedef enum {
    MAP_PRESENCE_UNKNOWN = 0,
    MAP_PRESENCE_OFFLINE,
    MAP_PRESENCE_ONLINE,
    MAP_PRESENCE_AWAY,
    MAP_PRESENCE_DONOT_DISTURB,
    MAP_PRESENCE_BUSY,
    MAP_PRESENCE_IN_A_MEETING,
    MAP_PRESENCE_MAX_VALUE,
} map_presence_t;

typedef enum {
    MAP_CHATSTATE_UNKNOWN = 0,
    MAP_CHATSTATE_INACTIVE,
    MAP_CHATSTATE_ACTIVE,
    MAP_CHATSTATE_COMPOSING,
    MAP_CHATSTATE_PAUSED_COMPOSING,
    MAP_CHATSTATE_GONE,
    MAP_CHATSTATE_MAX_VALUE,
} map_chatstate_t;

struct map_set_owner_status_param_t {
    bool has_presence_avail;
    bool has_presence_text;
    bool has_last_activity;
    bool has_chat_state;
    map_presence_t presence_avail;
    map_chatstate_t chat_state;
    char convo_id[MAP_CONVERSATION_ID_STR_LEN+1];
    char presence_text[MAP_PRESENCE_TEXT_MAX_LEN+1];
    char last_activity[MAP_DATETIME_UTC_OFFSET_FMT_LEN+1];
};

#define MAP_CONVO_PARAM_MASK_CONVO_NAME             0x00000001
#define MAP_CONVO_PARAM_MASK_LAST_ACTIVITY          0x00000002
#define MAP_CONVO_PARAM_MASK_READ_STATUS            0x00000004
#define MAP_CONVO_PARAM_MASK_VERSION_COUNTER        0x00000008
#define MAP_CONVO_PARAM_MASK_SUMMARY                0x00000010
#define MAP_CONVO_PARAM_MASK_PARTICIPANTS           0x00000020
#define MAP_CONVO_PARAM_MASK_PARTI_UCI              0x00000040
#define MAP_CONVO_PARAM_MASK_PARTI_DISPLAY_NAME     0x00000080
#define MAP_CONVO_PARAM_MASK_PARTI_CHAT_STATE       0x00000100
#define MAP_CONVO_PARAM_MASK_PARTI_LAST_ACTIVITY    0x00000200
#define MAP_CONVO_PARAM_MASK_PARTI_X_BT_UID         0x00000400
#define MAP_CONVO_PARAM_MASK_PARTI_NAME             0x00000800
#define MAP_CONVO_PARAM_MASK_PARTI_PRESENCE_AVAIL   0x00001000
#define MAP_CONVO_PARAM_MASK_PARTI_PRESENCE_TEXT    0x00002000
#define MAP_CONVO_PARAM_MASK_PARTI_PRIORITY         0x00004000

struct map_get_convo_listing_param_t {
    uint16_t max_list_count;    // max number of entries returned
    uint16_t list_start_offset;
    bool has_last_activity_begin_filter;
    bool has_last_activity_end_filter;
    bool has_recipient_filter;
    uint8_t filter_read_status; // 0 "no filtering" get all read and unread messages
    uint32_t convo_param_masks;
    char recipient[MAP_FILTER_RECIPIENT_MAX_LEN+1];
    char filter_last_activity_begin[MAP_DATETIME_UTC_OFFSET_FMT_LEN+1];
    char filter_last_activity_end[MAP_DATETIME_UTC_OFFSET_FMT_LEN+1];
    char convo_id[MAP_CONVERSATION_ID_STR_LEN+1];
};

typedef enum {
    BT_MAP_STATE_CLOSED,
    BT_MAP_STATE_STANDBY = 1, // ready
    BT_MAP_STATE_QUERING,
    BT_MAP_STATE_CONNECTING,
    BT_MAP_STATE_OPEN,
    BT_MAP_STATE_DISCONNECTING,
} bt_map_state_t;

typedef enum {
    BT_MAP_MSG_STATUS_UNREAD,
    BT_MAP_MSG_STATUS_READ,
    BT_MAP_MSG_STATUS_UNDELETED,
    BT_MAP_MSG_STATUS_DELETED,
} bt_map_msg_status_t;

typedef struct {
    uint8_t error_code;
} bt_map_opened_param_t;

typedef struct {
    uint8_t error_code;
} bt_map_closed_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint16_t folder_listing_size;
    struct map_str_len_value_t body;
} bt_map_folder_listing_rsp_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t mas_instance_id;
    struct map_msg_listing_item_t *item;
} bt_map_msg_listing_item_rsp_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t mas_instance_id;
    struct map_str_len_value_t msg_body;
} bt_map_get_msg_rsp_param_t;

typedef struct {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint64_t message_handle;
} bt_map_put_msg_rsp_param_t;

typedef struct {
    uint8_t mas_instance_id;
    map_msgtype_t msg_type;
    bool msg_is_read;
    const char *msg_folder;
    uint64_t message_handle;
} bt_map_read_status_change_param_t;

typedef union {
    bt_map_opened_param_t *opened;
    bt_map_closed_param_t *closed;
    bt_map_folder_listing_rsp_param_t *fl_rsp;
    bt_map_msg_listing_item_rsp_param_t *ml_item;
    bt_map_get_msg_rsp_param_t *get_msg_rsp;
    bt_map_put_msg_rsp_param_t *put_msg_rsp;
    bt_map_read_status_change_param_t *read_status_change;
} bt_map_callback_param_t;

typedef enum {
    BT_MAP_EVENT_OPENED = BT_EVENT_MAP_OPENED,
    BT_MAP_EVENT_CLOSED,
    BT_MAP_EVENT_FOLDER_LISTING_RSP,
    BT_MAP_EVENT_MSG_LISTING_ITEM_RSP,
    BT_MAP_EVENT_GET_MSG_RSP,
    BT_MAP_EVENT_READ_STATUS_CHANGED,
    BT_MAP_EVENT_END,
} bt_map_event_t;

#if BT_MAP_EVENT_END != BT_EVENT_MAP_END
#error "bt_map_event_t error define"
#endif

typedef int (*bt_map_callback_t)(const bt_bdaddr_t *bd_addr, bt_map_event_t event, bt_map_callback_param_t param);

bt_status_t bt_map_init(bt_map_callback_t callback);
bt_status_t bt_map_cleanup(void);
bt_status_t bt_map_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_get_folder_listing(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_enter_to_root_folder(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_enter_to_parent_folder(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_enter_to_msg_folder(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_enter_to_child_folder(const bt_bdaddr_t *bd_addr, const char *folder);
bt_status_t bt_map_send_sms(const bt_bdaddr_t *bd_addr, struct bt_map_sms_t *sms);
bt_status_t bt_map_notify_register(const bt_bdaddr_t *bd_addr, bool enable);
bt_status_t bt_map_get_msg_listing(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_get_unread_message(const bt_bdaddr_t *bd_addr);
bt_status_t bt_map_get_message(const bt_bdaddr_t *bd_addr, uint64_t handle);
bt_status_t bt_map_set_message_status(const bt_bdaddr_t *bd_addr, uint64_t handle, bt_map_msg_status_t status);
int bt_map_is_connected(const bt_bdaddr_t *bd_addr);

#if defined(IBRT)
uint32_t btif_map_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);
uint32_t btif_map_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);
#endif

#ifdef __cplusplus
}
#endif
#endif /* BT_MAP_SUPPORT */
#endif /* __BT_MAP_SERVICE_H__ */

