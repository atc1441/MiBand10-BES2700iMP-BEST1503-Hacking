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
#ifndef __BT_APP_OPP_API_H__
#define __BT_APP_OPP_API_H__

#ifdef BT_OPP_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

#define OPP_RSP_SUCCESS                   0xa0
#define OPP_RSP_CONTINUE                  0x90

#define OPP_PULL_VCARD_TYPE               "text/x-vcard"
#define OPP_MAIN_VCARD_OBJECT_PATH_NAME   "telecom/pb.vcf"

typedef enum {
    OPP_CHANNEL_CLOSED = 0,
    OPP_OBEX_CONNECTED,
    OPP_CHANNEL_OPENED,
    OPP_RECEIVED_GET_REQ,
    OPP_RECEIVED_GET_RSP,

    OPP_RECEIVED_PUT_REQ,
    OPP_RECEIVED_PUT_RSP,
    OPP_PULL_BUSINESS_VCARD_DONE,
    OPP_PUSH_EXCHANGE_DONE,
    OPP_PUSH_OBJECT_DONE,

    /*
    OPP_PHONEBOOK_SIZE_RSP,
    OPP_NEW_MISSED_CALLS_RSP,
    OPP_SET_PHONEBOOK_DONE,
    OPP_ABORT_CURR_OP_DONE,
    OPP_PULL_PHONEBOOK_DONE,
    OPP_PULL_VCARD_LISTING_DONE,
    OPP_PULL_VCARD_ENTRY_DONE,
    */
} opp_event_t;

typedef enum {
    OPP_OBJECT_ACCEPT = 0,
    OPP_OBJECT_REJECT,
} opp_info_rsp_t;

typedef enum {
    OPP_PUSH_IDLE = 0,
    OPP_WAIT_PUSH,
    OPP_END_PUSH,
} opp_push_status_t;

typedef enum {
    OPP_VCARD_SELECTOR_OPERATOR_OR = 0x00, // default operator
    OPP_VCARD_SELECTOR_OPERATOR_AND = 0x01,
} opp_vcard_selector_operator_enum_t;

struct opp_get_cmd_info_t {
    uint16_t unicode_name_length;
    uint16_t app_parameters_length;
    uint16_t body_data_length;
    uint8_t *unicode_name;
    uint8_t *app_parameters;
    uint8_t *body_data;
};

struct opp_pull_object_context_t {
    bool is_final_packet;
    uint16_t unicode_name_length;
    uint16_t app_parameters_length;
    uint16_t body_data_length;
    uint8_t *unicode_name;
    uint8_t *app_parameters;
    uint8_t *body_data;
};

struct opp_pull_object_req_t {
    bool is_final_put_req;
    uint16_t unicode_name_length;
    uint16_t app_parameters_length;
    uint8_t *unicode_name;
    uint8_t *app_parameters;
    uint8_t *object_type;
    uint16_t type_length;
};

struct opp_push_object_req_t {
    bool is_final_get_req;
    uint16_t unicode_name_length;
    uint16_t app_parameters_length;
    uint16_t body_data_length;
    uint8_t* unicode_name;
    uint8_t* app_parameters;
    uint8_t *object_type;
    uint8_t *body_data;
    uint16_t type_length;
};

struct opp_callback_parm_t {
    opp_event_t event;
    uint8_t error_code;
    const struct opp_get_cmd_info_t *get_info;
    uint16_t phonebook_size;
    uint8_t new_missed_calls;
    const struct opp_pull_object_context_t *pull_context;
    const struct opp_pull_object_req_t *pull_req;
    const struct opp_push_object_req_t *put_req;
};

struct opp_pull_vcard_parameters {
    bool has_property_selector_paramter; // which vcard fields should be returned, default all fields are returned
    uint8_t property_selector_parameter_byte7;
    uint8_t property_selector_parameter_byte6;
    uint8_t property_selector_parameter_byte5;
    uint8_t property_selector_parameter_byte4;
    uint8_t property_selector_parameter_byte3;
    uint8_t property_selector_parameter_byte2;
    uint8_t property_selector_parameter_byte1;
    uint8_t property_selector_parameter_byte0;
    bool dont_use_default_max_list_count; // default is 65535
    uint16_t max_list_count; // max number of entries are returned
    uint16_t list_start_offset; // specifies which one is the start entry returned in the response, default is 0
    bool reset_new_missed_calls;
    bool has_vcard_selector_parameter; // only return vcards that the specified field's value is not empty
    uint8_t vcard_selector_parameter_byte7;
    uint8_t vcard_selector_parameter_byte6;
    uint8_t vcard_selector_parameter_byte5;
    uint8_t vcard_selector_parameter_byte4;
    uint8_t vcard_selector_parameter_byte3;
    uint8_t vcard_selector_parameter_byte2;
    uint8_t vcard_selector_parameter_byte1;
    uint8_t vcard_selector_parameter_byte0;
    opp_vcard_selector_operator_enum_t vcard_selector_operator;
};

struct btif_opp_push_param_t {
    bool is_final_put_req;
    bool name_is_unicode;
    uint16 name_length;
    const char* object_name;
    const char* object_type;
    const uint8 *app_parameters;
    uint16 app_parameters_length;
    const uint8 *body_content;
    uint16 body_length;
/*********************************************************/
    opp_push_status_t push_status;
    bool stack_fragment_flag;
    const uint8 *fragment_body_content;
    uint16 fragment_body_length;

    //bool is_apper_fragment;
    //bool add_init_complete;
    //bool is_first_package;

    //uint32_t total_body_length;
    //uint32_t sended_body_length;
};

struct btif_opp_channel_t;

typedef uint8 (*btif_opp_callback_t)(uint8 device_id, struct btif_opp_channel_t *opp_ctl, const struct opp_callback_parm_t *parm);

void btif_opp_init(btif_opp_callback_t btif_opp_callback);

void btif_opp_server_init(btif_opp_callback_t btif_opp_callback);

struct btif_opp_channel_t *btif_alloc_opp_channel(void);

struct btif_opp_channel_t *btif_alloc_opp_server_channel(void);

struct opp_control_t  *btif_get_opp_server_channel_by_device_id(uint8_t device_id);

bt_status_t btif_opp_connect(bt_bdaddr_t *addr);

bt_status_t btif_opp_disconnect(struct btif_opp_channel_t *opp_chan);

bool btif_opp_is_connected(struct btif_opp_channel_t *opp_chan);

bt_status_t btif_opp_exchang_object(struct btif_opp_channel_t *opp_channel,
        const char* object_path_name,
        struct opp_pull_vcard_parameters *parameters);

bool btif_opp_pull_business_card_object(struct btif_opp_channel_t *opp_chan,
        const char* object_path_name,
        struct opp_pull_vcard_parameters *parameters);

bool btif_opp_push_object_list_init(struct btif_opp_channel_t *opp_channel);

bool btif_opp_push_object_count(struct btif_opp_channel_t *opp_channel, uint16 body_length);

bool btif_opp_add_to_push_object_list(struct btif_opp_channel_t *opp_channel, struct btif_opp_push_param_t *cfg);

bt_status_t btif_opp_push_object(struct btif_opp_channel_t *opp_channel, bool once_multiple);

bt_status_t btif_opp_send_abort_req(struct btif_opp_channel_t *opp_chan);

bt_status_t btif_opp_send_obex_disconnect_req(struct btif_opp_channel_t *opp_chan);

bt_status_t btif_opp_set_srm_in_wait(struct btif_opp_channel_t *opp_chan, bool wait_flag);

bt_status_t btif_opp_pull_rsp_object(struct btif_opp_channel_t *opp_channel, void *obejct_info);

#ifdef __cplusplus
}
#endif

#endif /* BT_OPP_SUPPORT */

#endif /* __BT_APP_OPP_API_H__ */
 
