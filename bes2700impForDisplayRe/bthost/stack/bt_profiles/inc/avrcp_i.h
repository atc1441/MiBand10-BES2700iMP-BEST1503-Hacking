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
#ifndef __AVRCP_I_H__
#define __AVRCP_I_H__
#include "bluetooth.h"
#include "btm_i.h"
#include "sdp_i.h"
#include "l2cap_i.h"
#if defined(__cplusplus)
extern "C" {
#endif

#ifdef BT_AVRCP_SUPPORT

#define AVCTP_DEFAULT_MTU     127
#define AVCTP_DEFAULT_CREDITS 7

#define AVCTP_MAX_L2CAP_MTU 1013
#define AVCTP_MAX_CREDITS   40

/* flow control states */
#define AVCTP_CFC_DISABLED 0
#define AVCTP_CFC_ENABLED  AVCTP_DEFAULT_CREDITS

#define AVCTP_CFG_SESSIONS_MAX   2    /* means how many l2cap channel */

#define AVRCP_RECVDATA_BUF_SIZE 48

#define AVRCP_UINT_INFO_IND      0x30
#define AVRCP_SUBUNIT_INFO_IND   0x31
#define AVRCP_PASSTHROUGH_IND    0x7C
#define AVRCP_VENDOR_DEP_IND     0x00

#define AVRCP_RESP_NOT_IMPLEMENTED  0x08
#define AVRCP_RESP_ACCEPT           0x09
#define AVRCP_RESP_REJECT           0x0A
#define AVRCP_RESP_STABLE           0x0C
#define AVRCP_RESP_CHANGED          0x0D
#define AVRCP_RESP_INTERIM          0x0F

#define AVRCP_OP_GET_CAPABILITIES               0x10
#define AVRCP_OP_LIST_PLAYER_SETTING_ATTRIBS    0x11
#define AVRCP_OP_LIST_PLAYER_SETTING_VALUES     0x12
#define AVRCP_OP_GET_PLAYER_SETTING_VALUE       0x13
#define AVRCP_OP_SET_PLAYER_SETTING_VALUE       0x14
#define AVRCP_OP_GET_PLAYER_SETTING_ATTR_TEXT   0x15
#define AVRCP_OP_GET_PLAYER_SETTING_VALUE_TEXT  0x16
#define AVRCP_OP_INFORM_DISP_CHAR_SET           0x17
#define AVRCP_OP_INFORM_BATT_STATUS             0x18
#define AVRCP_OP_GET_MEDIA_INFO                 0x20
#define AVRCP_OP_GET_PLAY_STATUS                0x30
#define AVRCP_OP_REGISTER_NOTIFY                0x31
#define AVRCP_OP_REQUEST_CONT_RESP              0x40
#define AVRCP_OP_ABORT_CONT_RESP                0x41
#define AVRCP_OP_SET_ABSOLUTE_VOLUME            0x50
#define AVRCP_OP_SET_ADDRESSED_PLAYER           0x60
#define AVRCP_OP_SET_BROWSED_PLAYER             0x70
#define AVRCP_OP_GET_FOLDER_ITEMS               0x71
#define AVRCP_OP_CHANGE_PATH                    0x72
#define AVRCP_OP_GET_ITEM_ATTRIBUTES            0x73
#define AVRCP_OP_PLAY_ITEM                      0x74
#define AVRCP_OP_GET_ITEMS_NUM                  0x75
#define AVRCP_OP_SEARCH                         0x80
#define AVRCP_OP_ADD_TO_NOW_PLAYING             0x90
#define AVRCP_OP_GENERAL_REJECT                 0xA0
#define AVRCP_OP_CUSTOM_CMD                     0xF0

#define AVRCP_BT_COMPANY_ID "\x00\x19\x58"

 /* Define Opcode */
#define AVRCP_OPCODE_UNIT_INFO       0x30
#define AVRCP_OPCODE_SUBUNIT_INFO    0x31
#define AVRCP_OPCODE_PASS_THROUGH    0x7C
#define AVRCP_OPCODE_VENDOR_DEP      0x00

/* Define Command Type*/
#define AVRCP_CTYPE_CONTROL   0x00
#define AVRCP_CTYPE_STATUS    0x01
#define AVRCP_CTYPE_NOTIFY    0x03
#define AVCTP_CTYPE_GENERAL_INQUIRY       0x04

/* Response */
#define AVRCP_RESPONSE_STABLE    0x0C

/* Define PASSTHROUGH OP_ID */
#define AVRCP_OP_ID_PLAY    0x44
#define AVRCP_OP_ID_STOP    0x45
#define AVRCP_OP_ID_PAUSE   0x46
#define AVRCP_OP_ID_REC     0x47
#define AVRCP_OP_ID_FB      0x48
#define AVRCP_OP_ID_FF      0x49
#define AVRCP_OP_ID_FW      0x4B
#define AVRCP_OP_ID_BW      0x4C

#define AVRCP_OP_NEXT_GROUP  0x017E
#define AVRCP_OP_PREV_GROUP  0x027E

#define AVRCP_BTN_PUSHED	0x00
#define AVRCP_BTN_RELEASED	0x80

//Supports Cover Art GetImage feature
#define AVRCP_TG_COVER_ART_SUPPORT_MASK    0x0100
#define AVRCP_TG_BROWSING_SUPPORT_MASK     0x0040

enum avctp_event_enum {
    AVCTP_CHANNEL_OPEN,
    AVCTP_CHANNEL_NEW_OPENED,
    AVCTP_CHANNEL_TX_HANDLED,
    AVCTP_CHANNEL_CLOSED,
    AVCTP_BROWSING_CHANNEL_OPEN,
    AVCTP_BROWSING_CHANNEL_NEW_OPENED,
    AVCTP_BROWSING_CHANNEL_TX_HANDLED,
    AVCTP_BROWSING_CHANNEL_CLOSED
};

enum avctp_ctl_state_enum {
    AVCTP_CTL_FREE,
    AVCTP_CTL_INUSE
};

enum avctp_frame_type_enum {
    AVCTP_FRAME_ADV_CMD,
    AVCTP_FRAME_ADV_RSP,
    AVCTP_FRAME_PNL_CMD,
    AVCTP_FRAME_PNL_RSP,
    AVCTP_FRAME_UNITINFO_CMD,
    AVCTP_FRAME_UNITINFO_RSP,
    AVCTP_FRAME_SUBUNITINFO_RSP,
};

enum avctp_tx_status_enum {
    AVCTP_TX_IDLE,
    AVCTP_TX_BUSY,
};

enum avctp_role_enum {
    AVCTP_MASTER,
    AVCTP_SLAVE
};

#define AVCTP_CFG_SERVER_CHANNEL 0x08

#define AVCTP_PKT_HEADER_PROFILE_ID 0x110E

#define AVCTP_PKT_HEADER_VALID_PID 0
#define AVCTP_PKT_HEADER_INVALID_PID 1

#define AVCTP_PKT_HEADER_IS_CMD 0
#define AVCTP_PKT_HEADER_IS_RESPONSE 1

enum avctp_packet_type
{
    AVCTP_PACKET_TYPE_SINGLE = 0,
    AVCTP_PACKET_TYPE_START = 1,
    AVCTP_PACKET_TYPE_CONTINUE = 2,
    AVCTP_PACKET_TYPE_END = 3,
};

struct avctp_header {
    uint32
    ipid_ind    : 1,
    com_or_resp : 1,
    packet_type : 2,
    trans_label : 4;
    uint8 profile_ind[2];
}__attribute__ ((packed));

struct avctp_frame_t {
    struct list_node node;
    struct avctp_header header;
    int8 frame_type;
    bool need_free;
    uint8 send_count;
    uint8 *data;
    uint32 data_len;
    uint32 data_offset;
    bool is_browsing_frame;
};

struct avctp_resp {
    struct avctp_header header;
    void *avrcp_resp;
} __attribute__ ((packed));

struct avctp_control_t {
    struct list_node node;
    struct list_node tx_list;

    struct bdaddr_t remote;
    struct avctp_header header;
    int8 server_channel;
    uint32 l2cap_handle;
    uint32 browsing_l2cap_handle;

    int8 tx_status;
    bool ibrt_slave_force_disc_avrcp_dont_modify_tx_list;

    enum avctp_ctl_state_enum ctl_state;

    enum avctp_role_enum role;
    int (*notify_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata);
    void (*datarecv_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb);
};

struct avctp_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    uint32 l2cap_handle;
    struct avctp_control_t *avctp_ctl;
};

enum avrcp_role_enum {
    AVRCP_MASTER,
    AVRCP_SLAVE
};

enum avrcp_state_enum {
    AVRCP_STOP,
    AVRCP_STANDBY = 1,  //ready
    AVRCP_QUERING,
    AVRCP_CONNECTING,
    AVRCP_CONNECTED
 //   AVRCP_BUSY
};

enum music_state_enum {
    MUSIC_NO_ACTION,
    MUSIC_PLAYING,
    MUSIC_PAUSE,
    MUSIC_RESUME,
    MUSIC_STOP
};

typedef enum {
    AVRCP_OBEX_STATE_CLOSED,
    AVRCP_OBEX_STATE_STANDBY = 1, // ready
    AVRCP_OBEX_STATE_QUERING,
    AVRCP_OBEX_STATE_CONNECTING,
    AVRCP_OBEX_STATE_OPEN,
    AVRCP_OBEX_STATE_DISCONNECTING,
} avrcp_obex_state_t;

typedef enum {
    AVRCP_OBEX_OP_NULL,
    AVRCP_OBEX_OP_GET_IMAGE = 1,
    AVRCP_OBEX_OP_GET_THM,
    AVRCP_OBEX_OP_GET_IMAGE_PROPERTIES,
    AVRCP_OBEX_OP_ABORT,
} avrcp_obex_op_enum_t;

typedef enum {
    AVRCP_OBEX_CHANNEL_CLOSED = 0,
    AVRCP_OBEX_CONNECTED,
    AVRCP_OBEX_CHANNEL_OPENED,
    AVRCP_OBEX_RECEIVED_GET_REQ,
    AVRCP_OBEX_RECEIVED_GET_RSP,

    AVRCP_OBEX_ABORT_DONE,
    AVRCP_OBEX_GET_IMAGE_DONE,
    AVRCP_OBEX_GET_THM_DONE,
    AVRCP_OBEX_GET_IMAGE_PROPERTIES_DONE,
} avrcp_obex_event_t;

struct avrcp_pull_object_context_t {
    bool is_final_packet;
    uint16_t body_data_length;
    uint8_t *body_data;
};

struct avrcp_obex_callback_parm_t {
    avrcp_obex_event_t event;
    uint8_t error_code;
    uint8_t opcode;
    struct avrcp_pull_object_context_t *pull_context;
};

#ifdef BT_BROWSING_SUPPORT
#define AVRCP_MAX_BODY_CONTENT_BUFFER_SIZE   2048
#else
#define AVRCP_MAX_BODY_CONTENT_BUFFER_SIZE   2
#endif

struct avrcp_control_t {
    struct list_node node;
    struct avctp_control_t avctp_ctl;
    enum avrcp_role_enum role;
    struct bdaddr_t remote;
    enum avrcp_state_enum state;
    uint32 handle; //l2cap handle
    uint8 op_code;
    uint8 op_id;
    struct avctp_frame_t pnl_cmd;
    struct avctp_frame_t adv_cmd;
    struct avctp_frame_t adv_rsp;
    struct avctp_frame_t unitinfo_cmd;
    struct avctp_frame_t subunitinfo_cmd;
    uint8 pnl_cmd_buff[64];
    uint8 adv_cmd_buff[128];
    uint8 adv_rsp_buff[128];
    uint8 unitinfo_cmd_buff[64];
    uint8 is_src_playing;

    void (*indicate) (uint8_t device_id, struct avrcp_control_t *avrcp_ctl, uint8 event, void *pdata);
    void (*data_cb) (uint8_t device_id, struct avrcp_control_t *avrcp_ctl, struct pp_buff *ppb);

    uint16 remote_tg_avctp_version;
    uint16 remote_tg_avrcp_version;
    uint16 remote_tg_support_features;
    bool conn_avctp_after_sdp;

    uint16 remote_ct_avctp_version;
    uint16 remote_ct_avrcp_version;
    uint16 remote_ct_support_features;
    bool get_remote_ct_info_done;

    uint32 browsing_handle; //l2cap handle
    bool browsing_conn_flag;

    bool wait_flag;
    bool abort_curr_operation;
    struct obex_conn_t *obex_conn;
    uint16 remote_l2cap_psm;
    avrcp_obex_state_t obex_state;
    char image_handle[7];
    struct list_node avrcp_obex_op_list;
    avrcp_obex_op_enum_t curr_obex_op;
    void (*obex_callback)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, const struct avrcp_obex_callback_parm_t *parm);
    uint8 body_content_buffer[AVRCP_MAX_BODY_CONTENT_BUFFER_SIZE];
    uint16 content_length;
};

struct avrcp_advanced_cmd_pdu {
    struct list_node node;
    uint8 op;
    U8  trans_id;
    U16 parm_len;
    U8 *parms;

    U16  cur_len;
    U16  bytes_to_send;
    BOOL more;
    U8   cont_op;
    BOOL abort;
    BOOL  internal;

    U8  response;
    U8 error;
    BOOL is_cmd;
    U8  ctype;          /* 4 bits */
    BOOL is_browsing_cmd;
};

struct avrcp_adv_cmd_rsp_cb_parms {
    uint8 rsp;
    uint8 subunit_type;
    uint8 subunit_id;
    uint8 op_code;
    uint8 *origin_data;
    uint8 *op_data;
    uint16 op_data_len;
    BOOL is_browsing;
};

struct avrcp_adv_cmd_cb_parms {
    uint8 ctype;
    uint8 subunit_type;
    uint8 subunit_id;
    uint8 op_code;
    uint8 *origin_data;
    uint8 *op_data;
    uint16 op_data_len;
    uint8 cmd_trans_id;
    BOOL is_browsing;
};

struct avrcp_ctx_input {
    struct ctx_content ctx;
    struct bdaddr_t *remote;
    struct avrcp_control_t *avrcp_ctl;
};

struct avrcp_ctx_output {
};

struct avctp_ctx_output {
};

int8 avctp_init(struct avctp_control_t *avctp_ctl,
        struct avctp_control_t* (*accept)(uint8_t device_id, void* remote),
        struct avctp_control_t* (*search)(uint8_t device_id, uint32 l2cap_handle));

int8 avctp_init_service(void);

int8 avctp_register_server (struct avctp_control_t *avctp_ctl, int8 server_channel,
                            int (*avctp_notify_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata),
                            void (*avctp_datarecv_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb)
                            );
                            
int8 avctp_unregister_server (struct avctp_control_t *avctp_ctl, int8 server_channel);

void avctp_init_packet_header(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame, 
    int8 is_cmd, int8 ipid_ind, uint16 profile_ind);
int8 avctp_send_message(struct avctp_control_t *avctp_ctl, struct avctp_frame_t *frame);
int8 avctp_close(struct avctp_control_t *avctp_ctl);
int8 avctp_open_i(struct avctp_control_t *avctp_ctl, struct bdaddr_t *remote,
                  int (*avctp_notify_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata),
                  void (*avctp_datarecv_callback)(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb),
                  uint32 *avctp_handle);
int8 avctp_disconnectReq (struct avctp_control_t *avctp_ctl);
int8 avctp_disconnectReq_v2(struct avctp_control_t *avctp_ctl,uint8 reason);
uint32 avctp_get_l2cap_handle(struct avctp_control_t *avctp_ctl);
uint32 avctp_save_ctx(struct avctp_control_t *avctp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avctp_restore_ctx(struct avctp_ctx_input *input, struct avctp_ctx_output *output);

int avrcp_notify_callback(uint8 device_id, struct avctp_control_t *avctp_ctl, uint8 event, uint32 handle, void *pdata);
void avrcp_datarecv_callback(uint8 device_id, struct avctp_control_t *avctp_ctl, uint32 handle, struct pp_buff *ppb);

void app_callback(uint8 event, void *pdata);
int8 avrcp_init_inst(struct avrcp_control_t *avrcp_ctl,
                void (*indicate)(uint8 device_id, struct avrcp_control_t *avrcp_ctl, uint8 event, void *pdata),
                void (*datarecv_callback)(uint8 device_id, struct avrcp_control_t *avrcp_ctl, struct pp_buff *ppb));

enum avrcp_state_enum avrcp_getState(struct avrcp_control_t *avrcp_ctl);
void avrcp_setState(struct avrcp_control_t *avrcp_ctl, enum avrcp_state_enum state);
int8 avrcp_turnOn(struct avrcp_control_t *avrcp_ctl,
        struct avctp_control_t* (*accept)(uint8_t device_id, void* remote),
        struct avctp_control_t* (*search)(uint8_t device_id, uint32 l2cap_handle));
int8 avrcp_turnOff(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_connectReq(struct avrcp_control_t *avrcp_ctl, struct bdaddr_t *peer);
int8 avrcp_disconnectReq(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_disconnectReq_v2(struct avrcp_control_t *avrcp_ctl,uint8 reason);
int8 avrcp_send_cmd(struct avrcp_control_t *avrcp_ctl, int op);
void avrcp_query_remote_control_sdp(struct avrcp_control_t *avrcp_ctl);

/* Used by Controller to Send Command */
bool avrcp_is_advanced_command_can_send(struct avrcp_control_t * avrcp_ctl, bool is_cmd);
int8 avrcp_send_advanced_command(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_advanced_browsing_command(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_advanced_response(struct avrcp_control_t *avrcp_ctl, struct avrcp_advanced_cmd_pdu *pdu);
int8 avrcp_send_panel_key(struct avrcp_control_t *avrcp_ctl, uint16 op, uint8 press);
bool avrcp_is_control_channel_connected(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_is_panel_cmd_can_send(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_send_panel_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id, uint16 op, uint8 press, uint8 response);
int8 avrcp_send_unit_info_cmd(struct avrcp_control_t *avrcp_ctl);
int8 avrcp_send_unit_info_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
int8 avrcp_send_subunit_info_response(struct avrcp_control_t *avrcp_ctl, uint8 trans_id);
void avrcp_register_panel_command_tx_handled_callback(void (*cb)(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, void *pdata));

struct avctp_control_t *avrcp_get_avctp_control(struct avrcp_control_t *avrcp_ctl);
uint32 avrcp_save_ctx(struct avrcp_control_t *avrcp_ctl, uint8_t *buf, uint32_t buf_len);
uint32 avrcp_restore_ctx(struct avrcp_ctx_input *input, struct avrcp_ctx_output *output);

void avctp_send_get_folder_items_test(struct avctp_control_t *avctp_ctl);

bool avrcp_send_obex_get_image_properties(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, const char *image_handle);

bool avrcp_send_obex_get_image(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, const char *image_handle, const char *descriptor, uint16_t descriptor_len);

bool avrcp_send_obex_get_link_thumbnail_by_art_handle(uint8_t device_id, struct avrcp_control_t *avrcp_ctl, const char *image_handle);

bool avrcp_send_obex_get_link_thumbnail(uint8_t device_id, struct avrcp_control_t *avrcp_ctl);

void avrcp_start_avctp_browsing_connect(struct avrcp_control_t *avrcp_ctl);
int avrcp_create_obex_channel(uint8_t device_id, struct avrcp_control_t *avrcp_ctl);
int8 avctp_browsing_init_service(struct avctp_control_t* (*accept)(uint8_t device_id, void* remote),
                                      struct avctp_control_t* (*search)(uint8_t device_id, uint32 l2cap_handle));
void avrcp_obex_init(struct avrcp_control_t *avrcp_ctl, struct avrcp_control_t* (*search)(uint8_t device_id));

void avrcp_register_obex_callback(void);

#endif /* BT_AVRCP_SUPPORT */
#if defined(__cplusplus)
}
#endif
#endif /* __AVRCP_I_H__ */
