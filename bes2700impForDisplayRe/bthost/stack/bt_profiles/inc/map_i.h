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
#ifndef __STACK_MAP_I_H__
#define __STACK_MAP_I_H__

#ifdef BT_MAP_SUPPORT

#include "obex_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAP_DEVICE_MAX_NUM (BT_DEVICE_NUM)

typedef enum {
    MAP_ERROR_INVALID_INSTANCE_ID = 0x01,
    MAP_ERROR_INVALID_PARAMETER,
    MAP_ERROR_OBEX_SEND_FAIL,
    MAP_ERROR_OPERATION_ABORTED,
    MAP_ERROR_INVALID_REMOTE_MOPL,
    MAP_ERROR_BMSG_ALREADY_SENT,
    // obex rsp error code
    MAP_OBEX_RSP_CONTINUE = 0x90,                      // 100 http status code
    MAP_OBEX_RSP_SUCCESS = 0xa0,                       // 200
    MAP_OBEX_RSP_CREATED = 0xa1,                       // 201
    MAP_OBEX_RSP_ACCEPTED = 0xa2,                      // 202
    MAP_OBEX_RSP_NOT_AUTHORITATIVE = 0xa3,             // 203
    MAP_OBEX_RSP_NO_CONTENT = 0xa4,                    // 204
    MAP_OBEX_RSP_RESET_CONTENT = 0xa5,                 // 205
    MAP_OBEX_RSP_PARTIAL_CONTENT = 0xa6,               // 206
    MAP_OBEX_RSP_MULT_CHOICE = 0xb0,                   // 300
    MAP_OBEX_RSP_MOVED_PERMANENTLY = 0xb1,             // 301
    MAP_OBEX_RSP_MOVED_TEMPORARILY = 0xb2,             // 302
    MAP_OBEX_RSP_SEE_OTHER = 0xb3,                     // 303
    MAP_OBEX_RSP_NOT_MODIFIED = 0xb4,                  // 304
    MAP_OBEX_RSP_USE_PROXY = 0xb5,                     // 305
    MAP_OBEX_RSP_BAD_REQUEST = 0xc0,                   // 400
    MAP_OBEX_RSP_UNAUTHORIZED = 0xc1,                  // 401
    MAP_OBEX_RSP_PAYMENT_REQUIRED = 0xc2,              // 402
    MAP_OBEX_RSP_FORBIDDEN = 0xc3,                     // 403
    MAP_OBEX_RSP_NOT_FOUND = 0xc4,                     // 404
    MAP_OBEX_RSP_BAD_METHOD = 0xc5,                    // 405
    MAP_OBEX_RSP_NOT_ACCEPTABLE = 0xc6,                // 406
    MAP_OBEX_RSP_PROXY_AUTH_REQUIRED = 0xc7,           // 407
    MAP_OBEX_RSP_REQUEST_TIMEOUT = 0xc8,               // 408
    MAP_OBEX_RSP_CONFLICT = 0xc9,                      // 409
    MAP_OBEX_RSP_GONE = 0xca,                          // 410
    MAP_OBEX_RSP_LENGTH_REQUIRED = 0xcb,               // 411
    MAP_OBEX_RSP_PRECONDITION_FAILED = 0xcc,           // 412
    MAP_OBEX_RSP_REQUESTED_ENTIRY_TOO_LARGE = 0xcd,    // 413
    MAP_OBEX_RSP_REQUESTED_URL_TOO_LARGE = 0xce,       // 414
    MAP_OBEX_RSP_UNSUPPORTED_MEDIA_TYPE = 0xcf,        // 415
    MAP_OBEX_RSP_INTERNAL_ERROR = 0xd0,                // 500
    MAP_OBEX_RSP_NOT_IMPLEMENTED = 0xd1,               // 501
    MAP_OBEX_RSP_BAD_GATEWAY = 0xd2,                   // 502
    MAP_OBEX_RSP_SERVICE_UNAVAILABLE = 0xd3,           // 503
    MAP_OBEX_RSP_GATEWAY_TIMEOUT = 0xd4,               // 504
    MAP_OBEX_RSP_HTTP_VERSION_NOT_SUPPORT = 0xd5,      // 505
    MAP_OBEX_RSP_DATABASE_FULL = 0xe0,                 // ---
    MAP_OBEX_RSP_DATABASE_LOCKED = 0xe1,               // ---
} map_error_enum_t;

typedef enum {
    // map conn and channel event
    MAP_EVENT_CONN_OPENED,
    MAP_EVENT_CONN_CLOSED,
    MAP_EVENT_CONN_STATE,
    MAP_EVENT_MAS_CHANNEL_OPENED,
    MAP_EVENT_MAS_CHANNEL_CLOSED,
    MAP_EVENT_MNS_CHANNEL_OPENED,
    MAP_EVENT_MNS_CHANNEL_CLOSED,
    // map procedure response event
    MAP_EVENT_NOTIFY_REGISTER_RSP,
    MAP_EVENT_NOTIFY_FILTER_RSP,
    MAP_EVENT_INSTANCE_INFO_RSP,
    MAP_EVENT_GET_OBJECT_TEST_RSP,
    MAP_EVENT_SET_MSG_STATUS_RSP,
    MAP_EVENT_SET_FOLDER_RSP,
    MAP_EVENT_GET_FOLDER_LISTING_SIZE_RSP,
    MAP_EVENT_GET_FOLDER_LISTING_RSP,
    MAP_EVENT_PUSH_MESSAGE_RSP,
    MAP_EVENT_UPDATE_INBOX_RSP,
    MAP_EVENT_GET_MESSAGE_RSP,
    MAP_EVENT_GET_MSG_LISTING_SIZE_RSP,
    MAP_EVENT_GET_MSG_LISTING_RSP,
    MAP_EVENT_SET_OWNER_STATUS_RSP,
    MAP_EVENT_GET_OWNER_STATUS_RSP,
    MAP_EVENT_GET_CONVO_LISTING_SIZE_RSP,
    MAP_EVENT_GET_CONVO_LISTING_RSP,
    // map notify event
    MAP_EVENT_NEW_MESSAGE,
    MAP_EVENT_DELIVERY_SUCCESS,
    MAP_EVENT_SENDING_SUCCESS,
    MAP_EVENT_DELIVERY_FAILURE,
    MAP_EVENT_SENDING_FAILURE,
    MAP_EVENT_MEMORY_FULL,
    MAP_EVENT_MEMORY_AVAILABLE,
    MAP_EVENT_MESSAGE_DELETED,
    MAP_EVENT_MESSAGE_SHIFT,
    MAP_EVENT_READ_STATUS_CHANGED,
    MAP_EVENT_MSG_EXT_DATA_CHANGED,
    MAP_EVENT_MESSAGE_REMOVED,
    MAP_EVENT_CONVO_CHANGED,
    MAP_EVENT_PARTI_PRESENCE_CHANGED,
    MAP_EVENT_PARTI_CHATSTAT_CHANGED,
} map_event_t;

typedef enum {
    MAP_NOTIFY_NEW_MESSAGE, // v1.0
    MAP_NOTIFY_DELIVERY_SUCCESS,
    MAP_NOTIFY_SENDING_SUCCESS,
    MAP_NOTIFY_DELIVERY_FAILURE,
    MAP_NOTIFY_SENDING_FAILURE,
    MAP_NOTIFY_MEMORY_FULL,
    MAP_NOTIFY_MEMORY_AVAILABLE,
    MAP_NOTIFY_MESSAGE_DELETED,
    MAP_NOTIFY_MESSAGE_SHIFT,
    MAP_NOTIFY_READ_STATUS_CHANGED, // v1.1
    MAP_NOTIFY_MSG_EXT_DATA_CHANGED, // v1.2
    MAP_NOTIFY_MESSAGE_REMOVED,
    MAP_NOTIFY_CONVO_CHANGED,
    MAP_NOTIFY_PARTI_PRESENCE_CHANGED,
    MAP_NOTIFY_PARTI_CHATSTAT_CHANGED,
    MAP_NOTIFY_MAX_TYPE_VALUE,
} map_notify_type_t;

struct map_type_value_t {
    const char* type;
    uint16_t type_len;
    map_notify_type_t type_value;
    uint8_t padding;
} __attribute__ ((packed));

struct map_handle_value_t {
    char* handle;
    uint16_t padding;
    uint16_t handle_len;
    uint64_t handle_value;
} __attribute__ ((packed));

struct map_size_value_t {
    char* size;
    uint16_t size_len;
    uint16_t padding;
    uint32_t size_value;
} __attribute__ ((packed));

typedef enum {
    MAP_FOLDER_USER_DEFINE = 0,
    MAP_FOLDER_INBOX,
    MAP_FOLDER_OUTBOX,
    MAP_FOLDER_SENT,
    MAP_FOLDER_DELETED,
    MAP_FOLDER_DRAFT,
} map_folder_t;

struct map_folder_value_t {
    char* folder;
    uint16_t folder_len;
    map_folder_t folder_value;
    uint8_t padding;
} __attribute__ ((packed));

struct map_datetime_value_t {
    char* datetime; // v1.1
    uint16_t datetime_len;
    uint16_t year;
    uint32_t month: 4;
    uint32_t day: 5;
    uint32_t hour: 5;
    uint32_t min: 6;
    uint32_t sec: 6;
    uint32_t offset_min: 6;
    uint8_t offset_hour;
    uint8_t has_utc_offset;
    uint8_t minus_offset;
    uint8_t padding;
} __attribute__ ((packed));

struct map_subject_value_t {
    char* subject;
    uint16_t subject_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_sendername_value_t {
    char* sender_name;
    uint16_t sender_name_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_priority_value_t {
    char* priority;
    uint16_t priority_len;
    bool is_priority_msg;
} __attribute__ ((packed));

struct map_convoname_value_t {
    char* convo_name; // v1.2
    uint16_t convo_name_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_convoid_value_t {
    char* convo_id;
    uint16_t convo_id_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_presenceavail_value_t {
    char* presence_avail;
    uint16_t presence_avail_len;
    uint8_t presence_avail_value;
    uint8_t padding;
} __attribute__ ((packed));

struct map_presencetext_value_t {
    char* presence_text;
    uint16_t presence_text_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_chatstate_value_t {
    char* chat_state;
    uint16_t chat_state_len;
    uint8_t chat_state_value;
    uint8_t padding;
} __attribute__ ((packed));

struct map_readstatus_value_t {
    char* read_status;
    uint16_t read_status_len;
    bool is_already_read;
    uint8_t padding;
} __attribute__ ((packed));

struct map_extendeddata_value_t {
    char* extended_data;
    uint16_t extended_data_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_partiuci_value_t {
    char* parti_uci;
    uint16_t parti_uci_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_contactuid_value_t {
    char* contact_uid;
    uint16_t contact_uid_len;
    uint16_t padding;
} __attribute__ ((packed));

struct map_new_message_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_datetime_value_t datetime; // v1.1
    struct map_subject_value_t subject;
    struct map_sendername_value_t sender_name;
    struct map_priority_value_t priority;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_readstatus_value_t read_status;
    struct map_partiuci_value_t parti_uci;
    struct map_contactuid_value_t contact_uid;
} __attribute__ ((packed));

struct map_delivery_success_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_sending_success_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_delivery_failure_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_sending_failure_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoid_value_t convo_id; // v1.2
} __attribute__ ((packed));

struct map_memory_full_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
} __attribute__ ((packed));

struct map_memory_available_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
} __attribute__ ((packed));

struct map_message_deleted_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_message_shift_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_folder_value_t old_folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_read_status_changed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_readstatus_value_t read_status;
} __attribute__ ((packed));

struct map_extended_data_changed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_datetime_value_t datetime; // v1.1
    struct map_sendername_value_t sender_name;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_extendeddata_value_t extended_data;
    struct map_partiuci_value_t parti_uci;
    struct map_contactuid_value_t contact_uid;
} __attribute__ ((packed));

struct map_message_removed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_handle_value_t handle;
    struct map_folder_value_t folder;
    struct map_msgtype_value_t msg_type;
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
} __attribute__ ((packed));

struct map_convo_changed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_presenceavail_value_t presence_avail;
    struct map_presencetext_value_t presence_text;
    struct map_datetime_value_t last_activity;
    struct map_chatstate_value_t chat_state;
    struct map_partiuci_value_t parti_uci;
    struct map_contactuid_value_t contact_uid;
} __attribute__ ((packed));

struct map_parti_presence_changed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_sendername_value_t sender_name; // v1.1
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_presenceavail_value_t presence_avail;
    struct map_presencetext_value_t presence_text;
    struct map_datetime_value_t last_activity;
    struct map_partiuci_value_t parti_uci;
    struct map_contactuid_value_t contact_uid;
} __attribute__ ((packed));

struct map_parti_chatstat_changed_t {
    uint32_t mas_instance_id;
    struct map_type_value_t type; // v1.0
    struct map_sendername_value_t sender_name; // v1.1
    struct map_convoname_value_t convo_name; // v1.2
    struct map_convoid_value_t convo_id;
    struct map_datetime_value_t last_activity;
    struct map_chatstate_value_t chat_state;
    struct map_partiuci_value_t parti_uci;
    struct map_contactuid_value_t contact_uid;
} __attribute__ ((packed));

typedef enum {
    MAP_STATE_CLOSED,
    MAP_STATE_STANDBY = 1, // ready
    MAP_STATE_QUERING,
    MAP_STATE_CONNECTING,
    MAP_STATE_OPEN,
    MAP_STATE_DISCONNECTING,
} map_state_t;

struct map_conn_opened_t {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t connhdl;
};

struct map_conn_closed_t {
    uint8_t error_code;
    uint8_t device_id;
    uint16_t connhdl;
};

struct map_conn_state_t {
    map_state_t state;
    uint8_t device_id;
    uint16_t connhdl;
};

struct map_access_channel_opened_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint16_t connhdl;
};

struct map_access_channel_closed_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint16_t connhdl;
};

struct map_notify_channel_opened_t {
    uint8_t error_code;
    uint16_t connhdl;
};

struct map_notify_channel_closed_t {
    uint8_t error_code;
    uint16_t connhdl;
};

struct map_notify_register_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

struct map_notify_filter_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

struct map_instance_info_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    struct map_str_len_value_t owner_uci;
    struct map_str_len_value_t instance_info;
};

struct map_get_object_test_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    bool is_final_get_rsp;
    struct map_str_len_value_t body;
};

struct map_set_msg_status_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

struct map_set_folder_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

#define MAP_PARAM_MAX_LISTING_SIZE 1024

struct map_get_folder_listing_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint16_t folder_listing_size;
    struct map_str_len_value_t body;
};

struct map_push_message_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint64_t message_handle;
};

struct map_update_inbox_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

struct map_get_message_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    struct map_str_len_value_t body;
};

struct map_get_msg_listing_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    bool have_unread_messages;
    bool is_final_packet;
    uint16_t listing_size;
    uint16_t curr_item_index;
    struct map_msg_listing_item_t item;
};

struct map_set_owner_status_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
};

struct map_get_owner_status_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint8_t parsent_avail;
    uint8_t chat_state;
    struct map_str_len_value_t parsent_text;
    struct map_datetime_value_t last_activity; 
};

struct map_get_convo_listing_rsp_t {
    uint8_t error_code;
    uint8_t mas_instance_id;
    uint16_t listing_size;
    struct map_str_len_value_t body;
};

typedef void (*map_event_callback_t)(uint16_t connhdl, map_event_t event, void *param);

void map_stack_init(map_event_callback_t cb);

uint64_t map_str_to_hex_value(const char *start, int len);

void map_set_obex_over_rfcomm(bool use_rfcomm);

bool map_client_connect(const bt_bdaddr_t *remote);

void map_client_disconnect(const bt_bdaddr_t *remote);

void map_send_obex_disconnect_req(uint16_t connhdl, uint8_t mas_instance_id);

void map_send_obex_connect_req(uint16_t connhdl, uint8_t mas_instance_id);

void map_set_dont_auto_send_obex_conn_req(uint16_t connhdl, uint8_t mas_instance_id);

void map_connect_mas(uint16_t connhdl, uint8_t mas_instance_id);

void map_open_mas_channel(uint16_t connhdl, uint8_t mas_instance_id);

void map_open_all_mas_channel(uint16_t connhdl);

void map_close_mas_channel(uint16_t connhdl, uint8_t mas_instance_id);

void map_send_mns_obex_disc_req(uint16_t connhdl);

void map_close_mns_channel(uint16_t connhdl);

bool map_set_notify_register(uint16_t connhdl, uint8_t mas_instance_id, bool notify_enable);

bool map_set_notify_filter(uint16_t connhdl, uint8_t mas_instance_id, uint32_t event_masks);

bool map_get_instance_info(uint16_t connhdl, uint8_t mas_instance_id);

bool map_get_object_test(uint16_t connhdl, uint8_t mas_instance_id, const char *object_type, const char* object_name);

#define MAP_EXTENDED_DATA_MAX_STR_LEN 32

typedef enum {
    MAP_SET_READ_STATUS = 0,
    MAP_SET_DELETE_STATUS,
    MAP_SET_EXTENDED_DATA,
    MAP_SET_STATUS_MAX_VALUE,
} map_set_status_t;

struct map_set_message_param_t {
    map_set_status_t set_status;
    bool set_msg_read;
    bool set_msg_deleted;
    char msg_ext_data[MAP_EXTENDED_DATA_MAX_STR_LEN+1];
    uint64_t message_handle;
};

bool map_set_message_status(uint16_t connhdl, uint8_t mas_instance_id, struct map_set_message_param_t *param);

bool map_get_folder_listing_size(uint16_t connhdl, uint8_t mas_instance_id);

bool map_get_folder_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_folder_listing_param_t *param);

typedef enum {
    MAP_SET_PATH_TO_ROOT = 1,
    MAP_SET_PATH_TO_PARENT,
    MAP_SET_PATH_TO_CHILD,
    MAP_SET_PATH_TO_SIBLING,
} map_set_path_op_t;

struct map_set_folder_param_t {
    map_set_path_op_t set_path_op;
    const char *folder_name;
};

bool map_set_folder(uint16_t connhdl, uint8_t mas_instance_id, struct map_set_folder_param_t *param);

bool map_update_inbox(uint16_t connhdl, uint8_t mas_instance_id);

typedef enum {
    MAP_MULTI_PUSH_INVALID = 0,
    MAP_MULTI_PUSH_START,
    MAP_MULTI_PUSH_CONTINUE,
    MAP_MULTI_PUSH_END,
} map_multi_push_flag_t;

struct map_push_message_param_t {
    bool transparent_sent;
    bool dont_retry_sent;
    bool bmsg_is_utf8_text;
    bool use_conversation_id; // 32 character ASCII hex string, use convo_id instead of folder_name
    char folder_name[MAP_FOLDER_NAME_MAX_LEN+1]; // store conversation id when it is used
    const uint8_t *bmsg_object;
    uint16_t bmsg_object_len;
    uint16_t body_total_len;
    uint16_t curr_body_len;
    map_multi_push_flag_t multi_push_flag;
    bool forward_attachment;    // for forward message
    bool prepend_to_origin_msg; // for forward message
    uint64_t origin_msg_handle; // for forward message
};

bool map_push_message(uint16_t connhdl, uint8_t mas_instance_id, struct map_push_message_param_t *param);

struct map_msg_prop_t {
    bool use_bmsg_v_1_1;
    bool msg_is_read;
    map_msgtype_t bmsg_type;
    const char* msg_folder;
    const char* ext_data; // only for version 1.1
};

typedef enum {
    MAP_BODY_EMAIL_MMS_8BIT = 0, // 8-bit clean encoding
    MAP_BODY_IM_8BIT,
    MAP_BODY_GSM_SMS_G7BIT, // GSM 7 bit default alphabet
    MAP_BODY_GSM_SMS_G7BIT_EXT, // GSM 7 bit alphabet with natianal language extension
    MAP_BODY_GSM_SMS_GUCS2,
    MAP_BODY_GSM_SMS_G8BIT,
    MAP_BODY_CDMA_SMS_C8BIT, // octet, unspecified
    MAP_BODY_CDMA_SMS_CEPM, // extended protocol message
    MAP_BODY_CDMA_SMS_C7ASCII, // 7-bit ascii
    MAP_BODY_CDMA_SMS_CIA5,
    MAP_BODY_CDMA_SMS_CUNICODE,
    MAP_BODY_CDMA_SMS_CSJIS, // shift-jis
    MAP_BODY_CDMA_SMS_CKOREAN,
    MAP_BODY_CDMA_SMS_CLATINHEB, // latin / hebrew
    MAP_BODY_CDMA_SMS_CLATIN,
    MAP_BODY_MAX_ENCODING,
} map_body_encoding_t;

typedef enum {
    MAP_BODY_LANG_UNKNOWN = 0,
    MAP_BODY_LANG_GSM_SMS_TURKISH,
    MAP_BODY_LANG_GSM_SMS_SPANISH,
    MAP_BODY_LANG_GSM_SMS_PORTUGUESE,
    MAP_BODY_LANG_CDMA_SMS_ENGLISH,
    MAP_BODY_LANG_CDMA_SMS_FRENCH,
    MAP_BODY_LANG_CDMA_SMS_SPANISH,
    MAP_BODY_LANG_CDMA_SMS_JAPANESE,
    MAP_BODY_LANG_CDMA_SMS_KOREAN,
    MAP_BODY_LANG_CDMA_SMS_CHINESE,
    MAP_BODY_LANG_CDMA_SMS_HEBREW,
    MAP_BODY_MAX_LANG,
} map_body_lang_t;

struct map_msg_head_t {
    struct map_msg_prop_t prop;
    struct map_msg_originator_t originator;
};

struct map_msg_body_t {
    bool is_part_body;
    uint16_t part_id;
    bool has_body_encoding;
    bool has_body_charset;
    bool use_charset_utf8;
    bool has_body_lang;
    map_body_encoding_t encoding;
    map_body_lang_t body_lang;
    uint32_t body_length;
    struct pp_buff *msg_objects;
};

struct map_bmessage_t {
    struct map_msg_head_t head;
    struct map_msg_body_t body;
    struct map_push_message_param_t push;
    struct pp_buff *ppb;
};

struct map_bmessage_t *map_alloc_bmessage(map_msgtype_t msg_type, uint16_t size);

void map_free_bmessage(struct map_bmessage_t *bmsg);

void map_begin_bmessage(struct map_bmessage_t *bmsg);

void map_end_bmessage(struct map_bmessage_t *bmsg);

void map_begin_envelope(struct map_bmessage_t *bmsg, struct map_msg_recipient_t *recipient);

void map_end_envelope(struct map_bmessage_t *bmsg);

void map_write_msg_object(struct map_bmessage_t *bmsg, const uint8_t *msg_data, uint16_t msg_len);

uint16_t map_get_curr_msg_objects_length(struct map_bmessage_t *bmsg);

void map_begin_body(struct map_bmessage_t *bmsg, uint16_t objects_length);

void map_continue_body(struct map_bmessage_t *bmsg);

void map_end_body(struct map_bmessage_t *bmsg);

struct map_get_message_param_t {
    uint64_t message_handle;
    bool get_attachments;
    bool get_native_sms_pdu;
    bool get_fractioned_email;
};

void map_set_srm_in_wait(uint16_t connhdl, uint8_t mas_instance_id, bool wait);

bool map_get_message(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_message_param_t *param);

bool map_get_msg_listing_size(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_msg_listing_param_t *param);

bool map_get_message_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_msg_listing_param_t *param);

bool map_set_owner_status(uint16_t connhdl, uint8_t mas_instance_id, struct map_set_owner_status_param_t *param);

bool map_get_owner_status(uint16_t connhdl, uint8_t mas_instance_id, const char *conversation_id);

bool map_get_convo_listing_size(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_convo_listing_param_t *param);

bool map_get_convo_listing(uint16_t connhdl, uint8_t mas_instance_id, struct map_get_convo_listing_param_t *param);

#ifdef IBRT
uint32_t map_save_ctx(uint16_t connhdl, uint8_t *buf, uint32_t buf_len);
uint32_t map_restore_ctx(struct btm_conn_item_t *btm_conn, uint8_t *buf, uint32_t buf_len);
#endif

#ifdef __cplusplus
}
#endif

#endif /* BT_MAP_SUPPORT */

#endif // __STACK_MAP_I_H__

