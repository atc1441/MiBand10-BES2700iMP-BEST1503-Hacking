/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __OBEX_I_H__
#define __OBEX_I_H__

#ifdef BT_OBEX_SUPPORT

#include "bt_common_define.h"
#include "sdp_i.h"
#include "rfcomm_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OBEX_DEFAULT_MAX_PACKET_LENGTH (255) // min packet size that OBEX shall support

#define OBEX_TARGET_PROFILE_UUID_SIZE (16)

#define OBEX_AUTH_NONCE_LEN (16)

#define OBEX_HOST16_TO_BE(value,ptr) \
    *(((uint8*)(ptr))+0) = ((value)>>8)&0xFF; \
    *(((uint8*)(ptr))+1) = ((value))&0xFF

#define OBEX_HOST16_TO_LE(value,ptr) \
    *(((uint8*)(ptr))+0) = ((value))&0xFF; \
    *(((uint8*)(ptr))+1) = ((value)>>8)&0xFF

#define OBEX_BE_TO_HOST16(ptr) \
    ((*(((uint8*)(ptr))+0)<<8) | (*(((uint8*)ptr)+1)))

#define OBEX_HOST32_TO_BE(value,ptr) \
    *(((uint8*)(ptr))+0) = ((value)>>24)&0xFF; \
    *(((uint8*)(ptr))+1) = ((value)>>16)&0xFF; \
    *(((uint8*)(ptr))+2) = ((value)>>8)&0xFF; \
    *(((uint8*)(ptr))+3) = ((value))&0xFF \

#define OBEX_HOST32_TO_LE(value,ptr) \
    *(((uint8*)(ptr))+0) = ((value))&0xFF; \
    *(((uint8*)(ptr))+1) = ((value)>>8)&0xFF; \
    *(((uint8*)(ptr))+2) = ((value)>>16)&0xFF; \
    *(((uint8*)(ptr))+3) = ((value)>>24)&0xFF \

#define OBEX_BE_TO_HOST32(ptr) \
    ((*(((uint8*)(ptr))+0)<<24) | (*(((uint8*)(ptr))+1)<<16) | \
     (*(((uint8*)(ptr))+2)<<8) | (*(((uint8*)(ptr))+3)))

typedef enum {
    OBEX_OPCODE_NULL = 0x00,
    OBEX_OPCODE_CONNECT = 0x80,
    OBEX_OPCODE_DISCONNECT = 0x81,
    OBEX_OPCODE_PUT = 0x02,
    OBEX_OPCODE_PUT_FINAL = 0x82,
    OBEX_OPCODE_GET = 0x03,
    OBEX_OPCODE_GET_FINAL = 0x83,
    OBEX_OPCODE_SETPATH = 0x85,
    OBEX_OPCODE_ACTION = 0x86,
    OBEX_OPCODE_SESSION = 0x87,
    OBEX_OPCODE_ABORT = 0xff,
    // 0x04 0x84 0x08~0x0f are reserved
    // 0x10 to 0x1f are user definable
    // opcode bits 5 and 6 are reserved and should be set to zero
    // bit 7 of the opcode means Final packet or not
} obex_opcode_t;

typedef enum {
    OBEX_STATE_CLOSED,
    OBEX_STATE_STANDBY = 1, // ready
    OBEX_STATE_CHANNEL_CONNECTING,
    OBEX_STATE_CHANNEL_OPEN,
    OBEX_STATE_CONNECTED,
    OBEX_STATE_DISCONNECTING,
} obex_state_t;

typedef enum {
    OBEX_EVENT_CHANNEL_OPEN = 1,
    OBEX_EVENT_CONNECTED,
    OBEX_EVENT_DISCONNECTED,
    OBEX_EVENT_TX_DONE,
    OBEX_EVENT_GET_RSP,
    OBEX_EVENT_PUT_RSP,
    OBEX_EVENT_SETPATH_RSP,
    OBEX_EVENT_ABORT_RSP,
    OBEX_EVENT_CONN_REQ,
    OBEX_EVENT_GET_REQ,
    OBEX_EVENT_PUT_REQ,
} obex_event_enum_t;

typedef enum {
    OBEX_RSP_CONTINUE = 0x90,                      // 100 http status code
    OBEX_RSP_SUCCESS = 0xa0,                       // 200
    OBEX_RSP_CREATED = 0xa1,                       // 201
    OBEX_RSP_ACCEPTED = 0xa2,                      // 202
    OBEX_RSP_NOT_AUTHORITATIVE = 0xa3,             // 203
    OBEX_RSP_NO_CONTENT = 0xa4,                    // 204
    OBEX_RSP_RESET_CONTENT = 0xa5,                 // 205
    OBEX_RSP_PARTIAL_CONTENT = 0xa6,               // 206
    OBEX_RSP_MULT_CHOICE = 0xb0,                   // 300
    OBEX_RSP_MOVED_PERMANENTLY = 0xb1,             // 301
    OBEX_RSP_MOVED_TEMPORARILY = 0xb2,             // 302
    OBEX_RSP_SEE_OTHER = 0xb3,                     // 303
    OBEX_RSP_NOT_MODIFIED = 0xb4,                  // 304
    OBEX_RSP_USE_PROXY = 0xb5,                     // 305
    OBEX_RSP_BAD_REQUEST = 0xc0,                   // 400
    OBEX_RSP_UNAUTHORIZED = 0xc1,                  // 401
    OBEX_RSP_PAYMENT_REQUIRED = 0xc2,              // 402
    OBEX_RSP_FORBIDDEN = 0xc3,                     // 403
    OBEX_RSP_NOT_FOUND = 0xc4,                     // 404
    OBEX_RSP_BAD_METHOD = 0xc5,                    // 405
    OBEX_RSP_NOT_ACCEPTABLE = 0xc6,                // 406
    OBEX_RSP_PROXY_AUTH_REQUIRED = 0xc7,           // 407
    OBEX_RSP_REQUEST_TIMEOUT = 0xc8,               // 408
    OBEX_RSP_CONFLICT = 0xc9,                      // 409
    OBEX_RSP_GONE = 0xca,                          // 410
    OBEX_RSP_LENGTH_REQUIRED = 0xcb,               // 411
    OBEX_RSP_PRECONDITION_FAILED = 0xcc,           // 412
    OBEX_RSP_REQUESTED_ENTIRY_TOO_LARGE = 0xcd,    // 413
    OBEX_RSP_REQUESTED_URL_TOO_LARGE = 0xce,       // 414
    OBEX_RSP_UNSUPPORTED_MEDIA_TYPE = 0xcf,        // 415
    OBEX_RSP_INTERNAL_ERROR = 0xd0,                // 500
    OBEX_RSP_NOT_IMPLEMENTED = 0xd1,               // 501
    OBEX_RSP_BAD_GATEWAY = 0xd2,                   // 502
    OBEX_RSP_SERVICE_UNAVAILABLE = 0xd3,           // 503
    OBEX_RSP_GATEWAY_TIMEOUT = 0xd4,               // 504
    OBEX_RSP_HTTP_VERSION_NOT_SUPPORT = 0xd5,      // 505
    OBEX_RSP_DATABASE_FULL = 0xe0,                 // ---
    OBEX_RSP_DATABASE_LOCKED = 0xe1,               // ---
} obex_rsp_code_t;

struct obex_target_profile_uuid
{
    uint8 uuid[OBEX_TARGET_PROFILE_UUID_SIZE];
};

struct obex_callback_parms_t
{
    obex_event_enum_t event;
    uint8_t error_code;
    bool is_final_packet;
    uint16_t unicode_name_length;
    uint16_t app_parameters_length;
    uint16_t body_data_length;
    uint8_t *unicode_name;
    uint8_t *app_parameters;
    uint8_t *body_data;
    uint8_t *object_type;
    uint16_t type_length;
};

struct obex_tx_done_parms_t {
    obex_event_enum_t event;
    uint8_t error_code;
    void *priv;
};

struct obex_get_req_parms_t
{
    obex_event_enum_t event;
    uint8_t error_code;
    bool is_final_get_req;
    uint8_t* unicode_name;
    uint8_t* object_type;
    uint8_t* app_parameters;
    uint16_t unicode_name_length;
    uint16_t type_length;
    uint16_t app_parameters_length;
};

struct obex_put_req_parms_t
{
    obex_event_enum_t event;
    uint8_t error_code;
    bool is_final_put_req;
    uint8_t *unicode_name;
    uint8_t* object_type;
    uint8_t* app_parameters;
    uint8_t *body_content;
    uint16_t unicode_name_length;
    uint16_t type_length;
    uint16_t app_parameters_length;
    uint16_t body_length;
};

struct obex_get_rsp_param_t {
    bool is_final_get_rsp;
    bool name_is_unicode;
    uint16 name_length;
    const char* object_name;
    const char* object_type;
    const uint8_t *body_content;
    uint16_t body_length;
    obex_rsp_code_t rsp_code;
};

struct obex_get_param_t {
    bool name_is_unicode;
    uint16_t name_length;
    const char* object_name;
    const char* object_type;
    const uint8_t *app_parameters;
    uint16_t app_parameters_length;
    const char *img_handle;
    uint8_t img_handle_length;
    const uint8_t *img_descriptor;
    uint16_t img_descriptor_length;
};

struct obex_put_param_t {
    bool is_final_put_req;
    bool name_is_unicode;
    uint16_t name_length;
    const char* object_name;
    const char* object_type;
    const uint8_t *app_parameters;
    uint16_t app_parameters_length;
    const uint8_t *body_content;
    uint16_t body_length;
    uint32_t total_body_length;
};

struct obex_conn_t {
    struct bdaddr_t remote;
    bool initiator;
    bool inuse;
    bool is_over_l2cap;
    bool remote_srm_is_enabled;
    bool srm_is_in_wait_state;
    bool srm_header_is_sent;
    bool local_sent_srm_wait;
    uint8_t local_srm_wait_count;
    uint16 remote_max_obex_rx_packet_length;
    uint32 channel_handle;
    obex_opcode_t curr_opcode;
    obex_state_t obex_state;
    uint8 target_rfcomm_channel;
    uint16 target_profile_psm;
    l2cap_psm_target_profile_t target_profile;
    bool has_target_uuid;
    bool has_connection_id;
    struct obex_target_profile_uuid target_profile_uuid;
    uint32 connection_id;
    int (*obex_callback)(uint8_t device_id, struct obex_conn_t *obex_conn, struct obex_callback_parms_t *info);
    void *priv;
    uint16 curr_header_field_len;
    bool auth_connect_req_en;
    bool auth_sent_user_id;
    uint8 auth_remote_challenge_nonce[OBEX_AUTH_NONCE_LEN];
    uint8 auth_local_challenge_nonce[OBEX_AUTH_NONCE_LEN];
};

struct obex_app_param_t {
    uint8_t *param_start;
    uint16_t param_len;
};

int obex_stack_init(void);

obex_state_t obex_get_state(struct obex_conn_t *obex_conn);

bool obex_channel_is_connected(struct obex_conn_t *obex_conn);

void obex_set_srm_in_wait(struct obex_conn_t *obex_conn, bool wait);

bool obex_is_srm_in_effect(struct obex_conn_t *obex_conn);

bool obex_is_in_srm_wait_state(struct obex_conn_t *obex_conn);

bool obex_register_only_client_port(l2cap_psm_target_profile_t target_profile);

bool obex_register_profile(uint16 local_profile_psm, int8 local_rfcomm_channel, l2cap_psm_target_profile_t target_profile,
        int (*target_profile_callback)(uint8_t, struct obex_conn_t *, struct obex_callback_parms_t *));

struct obex_conn_t *obex_create_channel(uint8_t device_id, struct bdaddr_t* remote,
        uint16 remote_profile_psm, uint8 remote_rfcomm_channel, l2cap_psm_target_profile_t target_profile,
        int (*target_profile_callback)(uint8_t, struct obex_conn_t *, struct obex_callback_parms_t *), void *priv);

void obex_close_channel(uint8_t device_id, struct obex_conn_t *obex_conn);

void obex_force_close_channel(uint8_t device_id, struct obex_conn_t *obex_conn);

bool obex_send_connect_req(uint8_t device_id, struct obex_conn_t *obex_conn,
                uint8 *application_parameters, uint16 application_parameters_length);

bool obex_send_disconnect_req(uint8_t device_id, struct obex_conn_t *obex_conn);

bool obex_send_get_req(uint8_t device_id, struct obex_conn_t *obex_conn, struct obex_get_param_t *param);

bool obex_send_get_rsp(uint8_t device_id, struct obex_conn_t *obex_conn, struct obex_get_rsp_param_t *rsp_param);

bool obex_send_put_req(uint8_t device_id, struct obex_conn_t *obex_conn, struct obex_put_param_t *param, void *priv);

bool obex_setpath_to_root_folder_req(uint8_t device_id, struct obex_conn_t *obex_conn);

bool obex_setpath_to_parent_folder_req(uint8_t device_id, struct obex_conn_t *obex_conn);

bool obex_setpath_to_sibling_folder_req(uint8_t device_id, struct obex_conn_t *obex_conn, const char *name);

bool obex_setpath_to_child_folder_req(uint8_t device_id, struct obex_conn_t *obex_conn, const char* name);

bool obex_send_abort_req(uint8_t device_id, struct obex_conn_t *obex_conn);

struct obex_app_param_t obex_find_an_application_parameter(uint8_t* application_parameters, int length, uint8_t parameter_tag_id);

char *obex_convert_unicode_to_ascii_string_inplace(char *unicode_string, uint16_t string_len);

const char* obex_find_char(const char* data, int len, char ch);

const char* obex_find_blank(const char* data, int len);

const char* obex_match_blank(const char* data);

const char* obex_skip_blanks(const char* data, int len);

char obex_to_lower_case(char ch);

const char* obex_match_string(const char* data, int len, const char* substr);

const char* obex_search_substr(const char* data, int len, const char* substr);

uint16 obex_allow_send_put_length(struct obex_conn_t *obex_conn, struct obex_put_param_t *param);

uint16 obex_allow_send_pull_rsp_length(struct obex_conn_t *obex_conn, struct obex_get_rsp_param_t *param);

#ifdef IBRT

struct obex_restore_input_t {
    struct ctx_content ctx;
    uint16_t connhdl;
    bt_bdaddr_t remote;
    int (*obex_callback)(uint8_t device_id, struct obex_conn_t *obex_conn, struct obex_callback_parms_t *info);
    void *priv;
};

struct obex_restore_output_t {
    struct obex_conn_t *obex_conn;
};

uint32_t obex_save_ctx(uint8_t device_id, uint32_t channel_handle, uint8_t *buf, uint32_t buf_len);

uint32_t obex_restore_ctx(uint8_t device_id, struct obex_restore_input_t *input, struct obex_restore_output_t *output);

#endif /* IBRT */

#ifdef __cplusplus
}
#endif

#endif /* BT_OBEX_SUPPORT */

#endif /* __OBEX_I_H__ */

