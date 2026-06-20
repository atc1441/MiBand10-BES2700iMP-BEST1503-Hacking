/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __APP_CUSTOM_THREAD_H__
#define __APP_CUSTOM_THREAD_H__

/*****************************header include********************************/
#include "bluetooth_bt_api.h"
#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif
/******************************macro defination*****************************/
#define APP_CUSTOM_EVENT_MAX        (30)
#define APP_CUSTOM_THREAD_SIZE      (2048)

#ifdef __cplusplus
extern "C" {
#endif

/******************************type defination******************************/
typedef struct
{
    void (*global_state_changed)(ibrt_global_state_change_event *state);
    void (*a2dp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_a2dp_state_change *state);
    void (*hfp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_hfp_state_change *state);
    void (*avrcp_state_changed)(const bt_bdaddr_t *addr, ibrt_conn_avrcp_state_change *state);
    void (*tws_acl_state_changed)(ibrt_conn_tws_conn_state_event *state, uint8_t reason_code);
    void (*mobile_acl_state_changed)(const bt_bdaddr_t *addr, ibrt_mobile_conn_state_event *state, uint8_t reason_code);
    void (*tws_role_switch_status_ind)(const bt_bdaddr_t *addr, ibrt_conn_role_change_state state, ibrt_role_e role);
    void (*access_mode_changed)(btif_accessible_mode_t newAccessMode);

    bool (*incoming_connect_request_response_cb)(void);
    void (*sco_disconnect_cb)(const bt_bdaddr_t *addr, uint8_t reason_code);
    void (*dip_info_queried_cb)(uint8_t *remote_addr, bt_dip_pnp_info_t *pnp_info);
    void (*bsir_event_cb)(uint8_t is_in_band_ring);
} ibrt_core_status_changed_cb_t;

#if BLE_AUDIO_ENABLED
typedef AOB_CONNECTION_STATE_T CUSTOM_UX_AOB_CONNECTION_STATE_T;

typedef struct
{
    void (*aob_tws_acl_state_changed)(uint32_t evt_type, uint8_t conidx, CUSTOM_UX_AOB_CONNECTION_STATE_T state, ble_bdaddr_t addr);
    void (*aob_mob_acl_state_changed)(uint32_t evt_type, uint8_t conidx, CUSTOM_UX_AOB_CONNECTION_STATE_T state, ble_bdaddr_t addr);
    void (*aob_vol_changed_cb)(uint8_t con_lid, uint8_t volume, uint8_t mute);
    void (*aob_vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*aob_vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*aob_media_track_change_cb)(uint8_t con_lid);
    void (*aob_media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*aob_media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*aob_media_mic_state_cb)(uint8_t mute);
    void (*aob_media_iso_link_quality_cb)(AOB_ISO_LINK_QUALITY_INFO_T param);
    void (*aob_media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*aob_call_state_change_cb)(uint8_t con_lid, void *param);
    void (*aob_call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t value);
    void (*aob_call_status_flags_ind_cb)(uint8_t con_lid, bool inband_ring, bool silent_mode);
    void (*aob_call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*aob_call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*aob_call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t url_len, uint8_t *url);
    void (*aob_aob_call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*aob_call_action_result_ind_cb)(uint8_t con_lid, void *param);
    void (*aob_cis_established_ind_cb)(AOB_UC_SRV_CIS_INFO_T  *ascs_cis_established);
    void (*aob_cis_rejected_ind_cb)(uint16_t con_hdl, uint8_t error);
    void (*aob_cig_terminated_ind_cb)(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid);
    void (*aob_ase_ntf_value_ind_cb)(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason);
} CUSTOM_UX_AOB_EVENT_CB_HANDLER_T;
#endif //BLE_AUDIO_ENABLED

typedef struct {
    bool aob_event;
    union {
        ibrt_conn_event_packet        bt_con_event_packet;
#if BLE_AUDIO_ENABLED
        AOB_EVENT_PACKET              aob_event_packet;
#endif
    } u;
} CUSTOM_UX_EVENT_PACKET;

/****************************function declaration***************************/
void app_custom_ux_thread_init(void);

/**
 ****************************************************************************************
 * @brief Register event callback for custom UI
 *
 * @param[in] cb    callback list see@ibrt_core_status_changed_cb_t
 *
 ****************************************************************************************
 */
void app_custom_ux_register_callback(ibrt_core_status_changed_cb_t *cb);

/**
 ****************************************************************************************
 * @brief Report BT event
 *
 * @param[in] ev    event header
 *
 ****************************************************************************************
 */
void app_custom_ux_notify_bt_event(ibrt_conn_evt_header* ev);

#if BLE_AUDIO_ENABLED
/**
 ****************************************************************************************
 * @brief Report AOB event
 *
 * @param[in] ev    event header
 *
 ****************************************************************************************
 */
void app_custom_ux_aob_event_notify(AOB_EVENT_HEADER_T *ev);

/**
 ****************************************************************************************
 * @brief Register aob callback
 *
 * @param[in] cb    event callback header
 *
 ****************************************************************************************
 */
void app_custom_ux_register_aob_callback(CUSTOM_UX_AOB_EVENT_CB_HANDLER_T *cb);
#endif

#ifdef __cplusplus
}
#endif

#endif
