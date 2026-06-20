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
#ifndef __OPP_I_H__
#define __OPP_I_H__

#ifdef BT_OPP_SUPPORT

#include "obex_i.h"
#include "btm_i.h"
#include "sdp_i.h"

#ifdef __cplusplus
extern "C" {
#endif

#define OPP_MAX_SUPPORT_FEATURES_SIZE       6
#define OPP_MAX_BODY_CONTENT_BUFFER_SIZE    2048
#define OPP_CONNECT_ID_HEADER_SIZE         5
#define OPP_OVER_L2CAP_NO_SRM_SIZE         2
#define OPP_FIXED_OBEX_HEDAER_EXTRA_SIZE   (3)

struct opp_callback_parm_t;
struct opp_pull_vcard_parameters;
struct btif_opp_push_param_t;

typedef enum {
    OPP_STATE_CLOSED,
    OPP_STATE_STANDBY = 1, // ready
    OPP_STATE_QUERING,
    OPP_STATE_CONNECTING,
    OPP_STATE_OPEN,
    OPP_STATE_DISCONNECTING,
} opp_state_t;

typedef enum {
    OPP_OP_NULL,
    OPP_OP_OBJECT_PUSH = 1,
    OPP_OP_BUSINESS_CARD_PULL,
    OPP_OP_BUSINESS_CARD_EXCHANGE,
    OPP_OP_OBJECT_PULL_RSP,
    OPP_OP_END,
} opp_opcode_enum_t;

struct opp_control_t {
    struct obex_conn_t *obex_conn;
    struct bdaddr_t remote;
    opp_state_t state;
    uint8 device_id;
    uint8 remote_supported_repos;
    bool abort_curr_operation;
    bool wait_flag;
    uint8 remote_rfcomm_channel;
    uint16 remote_l2cap_psm;
    uint16 remote_version;
    uint16 remote_supported_features;
    struct list_node opp_opcode_list;
    struct list_node opp_push_list;
    uint16 wait_push_object_num;
    uint32 total_body_len;
    bool once_push_mult;
    bool added_pull_rsp;
    opp_opcode_enum_t curr_opp_op;
    bool auth_connect_req_en;
    uint8 (*opp_callback)(uint8 device_id, struct opp_control_t *opp_ctl, const struct opp_callback_parm_t *parm);
    uint8 body_content_buffer[OPP_MAX_BODY_CONTENT_BUFFER_SIZE];
    uint16 content_length;
    void *wait_op_list;
};

typedef uint8 (*opp_callback_t)(uint8 device_id, struct opp_control_t *opp_ctl, const struct opp_callback_parm_t *parm);

void opp_stack_init(void);

void opp_control_init(struct opp_control_t *opp_ctl, opp_callback_t opp_callback);

void opp_register_profile(void);

int opp_client_open_req(struct opp_control_t *opp_ctl);

bool opp_channel_is_connected(struct opp_control_t *opp_ctl);

int opp_client_close_req(struct opp_control_t *opp_ctl);

opp_state_t opp_get_state(struct opp_control_t *opp_ctl);

int opp_send_obex_connect_req(struct opp_control_t *opp_ctl, uint8 *parameters, uint16 length);

int opp_send_obex_disconnect_req(struct opp_control_t *opp_ctl);

int opp_send_abort_req(struct opp_control_t *opp_ctl);

int opp_send_abort_operation(struct opp_control_t *opp_ctl);

bool opp_pull_business_card_object(struct opp_control_t *opp_ctl,
            const char *object_path_name, struct opp_pull_vcard_parameters *parameters);

bool opp_push_object_list_init(struct opp_control_t *opp_ctl);

bool opp_add_push_object_req(struct opp_control_t *opp_ctl, struct btif_opp_push_param_t *cfg);

bool opp_send_push_object_req(struct opp_control_t *opp_ctl, bool once_multiple);

bool opp_push_object_count(struct opp_control_t *opp_ctl, uint16 body_length);

bool opp_send_exchange_object_req(struct opp_control_t *opp_ctl,
                    const char *object_path_name, struct opp_pull_vcard_parameters *parameters);

void opp_set_srm_in_wait(struct opp_control_t *opp_ctl, bool wait_flag);

bool opp_send_pull_rsp_object(struct opp_control_t *opp_ctl, void *obejct_info);

#ifdef __cplusplus
}
#endif

#endif /* BT_OPP_SUPPORT */

#endif /* __OPP_I_H__ */

