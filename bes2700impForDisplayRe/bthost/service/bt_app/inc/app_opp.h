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
#ifndef __APP_BT_OPP_H__
#define __APP_BT_OPP_H__

#ifdef BT_OPP_SUPPORT

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    OPP_PTS_IDLE = 0,
    OPP_PTS_PUSH_TWO_VCARD_OBJECT_START,
    OPP_PTS_PUSH_TWO_VCARD_OBJECT_END,
    OPP_PTS_PUSH_TWO_VCAL_OBJECT_START,
    OPP_PTS_PUSH_TWO_VCAL_OBJECT_END,
    OPP_PTS_PUSH_TWO_VMSG_OBJECT_START,
    OPP_PTS_PUSH_TWO_VMSG_OBJECT_END,
    OPP_PTS_PUSH_TWO_VNT_OBJECT_START,
    OPP_PTS_PUSH_TWO_VNT_OBJECT_END,
} opp_pts_event_t;

struct app_opp_object_info_t {
    const char *object_name;
    uint16 name_length;
    const char* object_type;
    const uint8 *app_parameters;
    uint16 app_parameters_length;
    const uint8 *body_content;
    uint16 body_length;
};

struct btif_opp_channel_t;

void app_bt_opp_init(void);

void app_bt_connect_opp_profile(bt_bdaddr_t *bdaddr);

void app_bt_disconnect_opp_profile(struct btif_opp_channel_t *opp_channel);

void app_bt_opp_exchang_object(struct btif_opp_channel_t *opp_channel, const char *param);

void app_bt_opp_pull_vcard_object(struct btif_opp_channel_t *opp_channel, const char *param);

void app_bt_pts_opp_srm_wait_pull_vcard_object(void);

bool app_bt_opp_add_to_push_object_list(struct btif_opp_channel_t *opp_channel, struct app_opp_object_info_t *object_info, bool is_final_put_req);

void app_bt_opp_push_object(struct btif_opp_channel_t *opp_channel, bool once_multiple);

void app_bt_opp_send_obex_disc_req(struct btif_opp_channel_t *opp_channel);

void app_bt_opp_send_obex_abort_req(struct btif_opp_channel_t *opp_channel);

#ifdef __cplusplus
}
#endif

#endif /* BT_OPP_SUPPORT */

#endif /* __APP_BT_OPP_H__ */

