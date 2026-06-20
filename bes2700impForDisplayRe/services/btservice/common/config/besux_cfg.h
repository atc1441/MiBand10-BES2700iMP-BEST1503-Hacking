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
/***
 * besbt_cfg.h
 */

#ifndef __BESUX_CFG_H__
#define __BESUX_CFG_H__
#include <stdbool.h>

#ifdef BT_HFP_SUPPORT
#include "hfp_api.h"
#include "app_hfp.h"
#endif

#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "dip_api.h"
#include "app_dip.h"
#include "dip_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

struct btservice_cfg_t{
    bool ux_hfp_support;
    bool ux_map_support;
    bool ux_hid_support;
    bool ux_dip_support;
    bool ux_set_master_on_tws_disconnected;
    bool ux_always_in_discoverable_mode;
    bool ux_tws_rs_by_btc_support;
    bool ux_tws_rs_without_mobile;
    bool ux_role_switch_monitor;
    bool ux_set_right_is_master;
    bool ux_use_safe_disconnect;
    bool ux_report_evt_to_cudtomux_support;
    bool ux_codec_error_handling;
    bool ux_bes_ota_support;
    bool ux_gam_voice_support;
    bool ux_interaction_support;
    bool ux_prompt_self_mgr_support;
    bool ux_media_player_support;
    bool ux_bt_sync_support;
    bool ux_ble_ctkd_support;
    bool ux_glasses_project_support;
    bool ux_bisto_support;
    bool ux_ai_voice_support;
    bool ux_me_mediator_support;
    bool ux_a2dp_support;
    bool ux_muti_dma_tc_support;
    bool ux_bth_subsys_only_support;
    bool ux_hf_siri_support;
};

#ifndef BT_HFP_SUPPORT
#define BTIF_HF_CALL_SETUP_NONE   0
typedef uint8_t bt_hfp_chan_state_t;
typedef uint8_t hfp_sco_codec_t;
typedef uint8_t ibrt_hfp_status_t;
typedef uint8_t btif_hf_channel_t;
#endif

#ifndef BISTO_ENABLED
#define  GSOUND_CHANNEL_CONTROL   0
#define  GSOUND_CHANNEL_AUDIO     1
#endif

typedef int(*btservice_audio_detect_next_packet_callback)(uint8_t device_id, btif_media_header_t *, unsigned char *, unsigned int len);
typedef void (*btservice_dip_info_queried_callback)(uint8_t *remote_addr, bt_dip_pnp_info_t *pnp_info);

extern const struct btservice_cfg_t btservice_cfg;

/**
 * bt hfp to ux
 */

int btservice_hfp_siri_voice(bool en);

bool btservice_hfp_is_sco_active(void);

bt_hfp_chan_state_t btservice_get_hf_chan_state(btif_hf_channel_t* chan_h);

void btservice_register_sco_link(uint8_t device_id, bt_bdaddr_t *remote);

uint32_t btservice_restore_hfp_app_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_save_hfp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

bool btservice_is_sco_connected(uint8_t device_id);

void btservice_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action);

uint8_t btservice_get_call_setup(void);

bt_status_t btservice_force_disconnect_hfp_profile(uint8_t device_id,uint8_t reason);

uint32_t btservice_hfp_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);

bt_status_t btservice_hf_sync_conn_audio_connected(hfp_sco_codec_t codec,uint16_t conhdl);

bt_status_t btservice_hf_sync_conn_audio_disconnected(uint16_t conhdl);

bool btservice_hfp_is_profile_initiator(const bt_bdaddr_t* remote);

uint8_t btservice_hf_get_negotiated_codec(btif_hf_channel_t* chan_h);

void btservice_receive_peer_sco_codec_info(const void* remote, uint8_t codec);

int btservice_hfp_ibrt_service_connected_mock(uint8_t device_id);

void btservice_hfp_update_local_volume(int id, uint8_t localVol);
#ifdef IBRT
int btservice_sync_get_hfp_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);

int btservice_sync_set_hfp_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);

int btservice_sync_set_hfp_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status);
#endif
uint32_t btservice_hfp_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

/**
 * bt map to ux
 */

uint32_t btservice_map_profile_save_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_save_map_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_map_profile_restore_ctx(const bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_restore_map_app_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);


/**
 * bt hid to ux
 */

uint32_t btservice_hid_profile_save_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_save_hid_app_ctx(btif_remote_device_t *rem_dev, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_hid_profile_restore_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_restore_hid_app_ctx(bt_bdaddr_t *bdaddr_p, uint8_t *buf, uint32_t buf_len);

bt_status_t btservice_hid_profile_connect(const bt_bdaddr_t *remote, int capture);

bt_status_t btservice_hid_profile_disconnect(const bt_bdaddr_t *remote);

/**
 * bt dip to ux
 */

bt_dip_pnp_info_t* btservice_dip_get_device_info(bt_bdaddr_t *remote);

void btservice_dip_sync_init(void);

void btservice_register_dip_info_queried_callback(btservice_dip_info_queried_callback func);


/**
 * ota to ux
 */

void btservice_set_ota_role_switch_initiator(bool is_initiate);

void btservice_ota_send_role_switch_req(void);

uint8_t btservice_get_bes_ota_state(void);

bool btservice_ota_is_in_progress(void);

void btservice_ota_send_role_switch_complete(void);


/**
 * other app to ux
 */

bool btservice_custom_stop_ibrt_ongoing(void);

void btservice_custom_all_safe_disconnect(void);

void btservice_gma_secret_key_send(void);

bool btservice_bixby_hotword_detect_value_get(void);

void btservice_media_PlayAudio(AUD_ID_ENUM id,uint8_t device_id);

void btservice_trigger_media_play(AUD_ID_ENUM id, uint8_t device_id, uint16_t aud_pram);

void btservice_media_PlayAudio_standalone(AUD_ID_ENUM id, uint8_t device_id);


/**
 * apps ai to ux
 */

uint32_t btservice_ai_tws_role_switch_prepare(uint32_t *wait_ms);

void btservice_ai_tws_role_switch(void);

void btservice_ai_tws_master_role_switch_prepare(void);

void btservice_ai_tws_role_switch_prepare_done(void);

void btservice_ai_if_mobile_connect_handle(void *addr);

void btservice_ai_tws_gsound_sync_init(void);

void btservice_ai_tws_role_switch_complete(void);

void btservice_ai_tws_sync_init(void);

uint8_t btservice_ai_tws_get_local_role(void);

void btservice_tws_update_roleswitch_initiator(uint8_t role);

bool btservice_tws_request_roleswitch(void);

void btservice_gsound_set_ble_connect_state(uint8_t chnl, bool state);

void btservice_gsound_on_bt_link_disconnected(uint8_t *addr);

void btservice_gsound_on_system_role_switch_done(uint8_t newRole);

void btservice_gsound_tws_role_update(uint8_t newRole);

/**
 * btsync to ux
 */

void btservice_bt_sync_tws_cmd_handler(uint8_t *p_buff, uint16_t length);

bool btservice_bt_sync_enable(uint32_t opCode, uint8_t length, uint8_t *p_buff, uint8_t prama);

void btservice_bt_sync_send_tws_cmd_done(uint8_t *ptrParam, uint16_t paramLen);

/**
 * bt to ux
 */

uint32_t btservice_bt_save_bd_addr_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_save_avrcp_app_ctx(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32_t btservice_avrcp_profile_save_ctxs(const bt_bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

/**
 * ux to Audioplayers
 */

int btservice_to_audio_a2dp_sync_init(float ratio);

int btservice_to_audio_a2dp_reset_data(void);

int btservice_to_audio_a2dp_sysfreq_boost_running(void);

int btservice_to_audio_a2dp_sysfreq_boost_start(uint32_t boost_cnt,bool is_init_boost);

int btservice_to_audio_a2dp_latency_factor_set(float factor);

int btservice_to_audio_a2dp_sync_tune_cancel(void);

int btservice_to_audio_a2dp_sync_tune_sample_rate(float ratio);

void btservice_to_audio_local_scalable_sbm_feature_handler(uint8_t device_id, bool isEnable);

void btservice_to_audio_local_scalable_sbm_feature_updated_callback(uint8_t device_id, bool isEnable, bool isSuccessful);

bool btservice_to_audio_a2dp_retrigger_is_on_process(void);

void btservice_to_audio_a2dp_retrigger_set_on_process(bool flag);

int btservice_to_audio_a2dp_stream_trigger_checker_stop(void);

int btservice_to_audio_a2dp_detect_next_packet_callback_register(btservice_audio_detect_next_packet_callback callback);

int btservice_to_audio_a2dp_detect_store_packet_callback_register(btservice_audio_detect_next_packet_callback callback);

int btservice_to_audio_a2dp_sync_direct_tune_sample_rate(float ratio);

void btservice_to_audio_play_speed_tuning_req_process(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

void btservice_to_audio_ibrt_sync_target_buf_ms_req_process(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);

void btservice_to_audio_ibrt_mobile_link_playback_info_receive(uint8_t device_id,
        APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);

bool btservice_to_audio_stream_trigger_onprocess(void);

void btservice_to_audio_stream_ibrt_set_trigger_time(uint8_t device_id, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *sync_trigger);

int btservice_to_audio_sco_chain_set_master_role(bool is_master);

void btservice_to_audio_sync_mix_prompt_req_handler(uint8_t* ptrParam, uint16_t paramLen);

void btservice_to_audio_tws_cmd_prompt_play_req_handler(uint8_t *ptrParam, uint32_t paramLen);

int btservice_to_audio_manager_sendrequest_need_callback(uint8_t massage_id, uint16_t stream_type, uint8_t device_id,
                                                    uint32_t aud_id, uint32_t cb, uint32_t cb_param);

int32_t btservice_to_audio_process_tws_sync_change(uint8_t *buf, uint32_t len);

uint8_t btservice_to_audio_af_stream_get_dma_chan(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

uint32_t btservice_to_audio_af_stream_get_dma_base_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

#ifdef __cplusplus
}
#endif
#endif /* __BESUX_CFG_H__ */
