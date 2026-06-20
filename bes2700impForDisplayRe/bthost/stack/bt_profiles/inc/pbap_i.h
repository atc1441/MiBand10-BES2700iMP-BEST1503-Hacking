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
#ifndef __PBAP_I_H__
#define __PBAP_I_H__

#ifdef BT_PBAP_SUPPORT

#include "obex_i.h"
#include "btm_i.h"
#include "sdp_i.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PBAP_STATE_CLOSED,
    PBAP_STATE_STANDBY = 1, // ready
    PBAP_STATE_QUERING,
    PBAP_STATE_CONNECTING,
    PBAP_STATE_OPEN,
    PBAP_STATE_DISCONNECTING,
} pbap_state_t;

typedef enum {
    PBAP_OP_NULL,
    PBAP_OP_PULL_PHONEBOOK = 1,
    PBAP_OP_SET_PHONEBOOK,
    PBAP_OP_PULL_VCARD_LISTING,
    PBAP_OP_PULL_VCARD_ENTRY,
} pbap_op_enum_t;

struct pbap_callback_parm_t;
struct pbap_pull_phonebook_parameters;
struct pbap_pull_vcard_listing_parameters;
struct pbap_pull_vcard_entry_parameters;

#define PBAP_MAX_BODY_CONTENT_BUFFER_SIZE 2048

struct pbap_control_t {
    struct obex_conn_t *obex_conn;
    struct bdaddr_t remote;
    pbap_state_t state;
    uint8 device_id;
    uint8 remote_supported_repos;
    bool has_remote_features;
    bool abort_curr_operation;
    uint8 remote_rfcomm_channel;
    uint16 remote_l2cap_psm;
    uint16 remote_version;
    uint32 remote_supported_features;
    struct list_node pbap_op_list;
    pbap_op_enum_t curr_pbap_op;
    bool auth_connect_req_en;
    void (*pbap_callback)(uint8_t device_id, struct pbap_control_t *pbap_ctl, const struct pbap_callback_parm_t *parm);
    uint8 body_content_buffer[PBAP_MAX_BODY_CONTENT_BUFFER_SIZE];
    uint16 content_length;
};

typedef void (*pbap_callback_t)(uint8_t device_id, struct pbap_control_t *pbap_ctl, const struct pbap_callback_parm_t *parm);

pbap_state_t pbap_get_state(struct pbap_control_t *pbap_ctl);

void pbap_stack_init(void);

void pbap_control_init(struct pbap_control_t *pbap_ctl, pbap_callback_t pbap_callback);

bool pbap_channel_is_connected(struct pbap_control_t *pbap_ctl);

int pbap_client_open_req(struct pbap_control_t *pbap_ctl);

int pbap_client_close_req(struct pbap_control_t *pbap_ctl);

int pbap_send_obex_disconnect_req(struct pbap_control_t *pbap_ctl);

int pbap_send_abort_req(struct pbap_control_t *pbap_ctl);

bool pbap_pull_phonebook(struct pbap_control_t *pbap_ctl,
            const char *object_path_name, struct pbap_pull_phonebook_parameters *parameters);

bool pbap_set_phonebook_current_folder_to_root(struct pbap_control_t *pbap_ctl);

bool pbap_set_phonebook_current_folder_to_parent(struct pbap_control_t *pbap_ctl);

bool pbap_set_phonebook_current_folder_to_child(struct pbap_control_t *pbap_ctl, const char *name);

bool pbap_pull_vcard_listing(struct pbap_control_t *pbap_ctl,
            const char *relative_folder_name, struct pbap_pull_vcard_listing_parameters *parameters);

bool pbap_pull_vcard_entry(struct pbap_control_t *pbap_ctl,
            const char *vcard_entry_name, struct pbap_pull_vcard_entry_parameters *parameters);

#ifdef __cplusplus
}
#endif

#endif /* BT_PBAP_SUPPORT */

#endif /* __PBAP_I_H__ */

