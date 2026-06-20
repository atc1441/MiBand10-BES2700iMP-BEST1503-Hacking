/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BTH_PATCH_L2CAP_H__
#define __BTH_PATCH_L2CAP_H__

#if defined(BUILD_BTH_ROM)
#include "besaud.h"
#include "l2cap_service.h"
#include "l2cap_i.h"
#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t g_patch_tbl_l2cap[];

typedef enum
{
    FUNC_ID_l2cap_l2cap_psm_to_scid_prefix,
    FUNC_ID_l2cap_l2cap_get_psm_target_profile_from_scid,
    FUNC_ID_l2cap_l2cap_scid_prefix_to_converted_psm,
    FUNC_ID_l2cap_l2cap_channel_search_scid,
    FUNC_ID_l2cap_l2cap_start_wait_disconnect_acl_timer,
    FUNC_ID_l2cap_l2cap_directly_disconnect_channel,
    FUNC_ID_l2cap_l2cap_channel_close,
    FUNC_ID_l2cap_l2cap_report_open,
    FUNC_ID_l2cap_l2cap_send_data_auto_fragment,
    FUNC_ID_l2cap_l2cap_handle_signal,
    FUNC_ID_l2cap_l2cap_handle_data,
    FUNC_ID_l2cap_btif_besaud_callback,
    FUNC_ID_l2cap_btif_besaud_send_cmd_no_wait,
} bt_patch_l2cap_func_enum_t;

extern void btif_besaud_callback(BesaudChannel *Chan, BesaudCallbackParms *Info);
extern bt_status_t btif_besaud_send_cmd_no_wait(uint8_t* cmd, uint16_t len);
extern uint16 l2cap_psm_to_scid_prefix(uint16 psm, l2cap_psm_target_profile_t target);
extern l2cap_psm_target_profile_t l2cap_get_psm_target_profile_from_scid(uint16 scid);
extern uint16 l2cap_scid_prefix_to_converted_psm(uint16 scid);
extern l2cap_channel_t *l2cap_channel_search_scid(l2cap_conn_t *conn, uint16 scid, enum l2cap_search_scid_ctx_t ctx);
extern void l2cap_start_wait_disconnect_acl_timer(l2cap_channel_t *channel, uint8 reason);
extern void l2cap_directly_disconnect_channel(uint8 device_id, l2cap_channel_t *channel);
extern int8 l2cap_channel_close(uint8 device_id, l2cap_channel_t *channel, uint8 reason);
extern void l2cap_report_open(uint8 device_id, l2cap_channel_t *channel);
extern int8 l2cap_send_data_auto_fragment(uint32 l2cap_handle, const uint8* data, uint32 len, void *context);
extern void l2cap_handle_signal(l2cap_conn_t *conn, struct pp_buff *ppb);
extern void l2cap_handle_data(uint8 device_id, l2cap_channel_t *channel, struct pp_buff *ppb);

typedef void (*btif_besaud_callback_func_t)(BesaudChannel *Chan, BesaudCallbackParms *Info);
typedef bt_status_t (*btif_besaud_send_cmd_no_wait_func_t)(uint8_t* cmd, uint16_t len);
typedef uint16 (*l2cap_psm_to_scid_prefix_func_t)(uint16 psm, l2cap_psm_target_profile_t target);
typedef l2cap_psm_target_profile_t (*l2cap_get_psm_target_profile_from_scid_func_t)(uint16 scid);
typedef uint16 (*l2cap_scid_prefix_to_converted_psm_func_t)(uint16 scid);
typedef l2cap_channel_t* (*l2cap_channel_search_scid_func_t)(l2cap_conn_t *conn, uint16 scid, enum l2cap_search_scid_ctx_t ctx);
typedef void (*l2cap_start_wait_disconnect_acl_timer_func_t)(l2cap_channel_t *channel, uint8 reason);
typedef void (*l2cap_directly_disconnect_channel_func_t)(uint8 device_id, l2cap_channel_t *channel);
typedef int8 (*l2cap_channel_close_func_t)(uint8 device_id, l2cap_channel_t *channel, uint8 reason);
typedef void (*l2cap_report_open_func_t)(uint8 device_id, l2cap_channel_t *channel);
typedef int8 (*l2cap_send_data_auto_fragment_func_t)(uint32 l2cap_handle, const uint8* data, uint32 len, void *context);
typedef void (*l2cap_handle_signal_func_t)(l2cap_conn_t *conn, struct pp_buff *ppb);
typedef void (*l2cap_handle_data_func_t)(uint8 device_id, l2cap_channel_t *channel, struct pp_buff *ppb);

#ifdef __cplusplus
}
#endif
#endif /* BUILD_BTH_ROM */
#endif /* __BTH_PATCH_L2CAP_H__ */