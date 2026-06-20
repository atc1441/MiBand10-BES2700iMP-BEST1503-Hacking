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
/***
 * besbt_cfg.h
 */

#ifndef BESBT_CFG_H
#define BESBT_CFG_H
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct besbt_cfg_t{
    uint16_t dip_vendor_id;
    uint16_t dip_product_id;
    uint16_t dip_product_version;
    uint16_t dip_vendor_id_source;
    bool apple_hf_at_support;
    bool hf_dont_support_cli_feature;
    bool hf_dont_support_enhanced_call;
    bool hf_dont_support_3way_call;
    bool hf_support_hf_ind_feature;
    bool sniff;
    bool dont_auto_report_delay_report;
    bool vendor_codec_en;
    bool hfp_support_lc3_swb_en;
    bool force_use_cvsd;
    bool support_enre_mode;
    bool bt_hid_cod_enable;
    bool bt_watch_enable;
    bool avdtp_cp_enable;
    bool bt_source_enable;
    bool bt_source_48k;
    bool bt_sink_enable;
    bool a2dp_sink_enable;
    bool lhdc_v3;
    bool hfp_ag_pts_enable;
    bool hfp_ag_pts_ecs_01;
    bool hfp_ag_pts_ecs_02;
    bool hfp_ag_pts_ecc;
    bool hfp_ag_clear_memory_number;
    bool avrcp_tg_pts_test;
    bool avrcp_tg_set_no_track;
    bool avrcp_tg_pts_large_rsp;
    bool source_get_all_cap_flag;
    bool source_suspend_err_flag;
    bool source_unknown_cmd_flag;
    bool acl_tx_flow_debug;
    bool hci_tx_cmd_debug;
    bool dont_play_mute_when_a2dp_stuck;
    bool send_l2cap_echo_req;
    bool a2dp_play_stop_media_first;
    bool disc_acl_after_auth_key_missing;
    bool use_page_scan_repetition_mode_r1;
    bool normal_test_mode_switch;
    bool mark_some_code_for_fuzz_test;
    bool hfp_hf_pts_acs_bv_09_i;
    bool retry_once_after_auth_failure;
    bool bt_search_auto_connect;
    bool ag_support_hf_ind;
    bool ag_support_inband_ring;
    bool ag_nrec_not_support;
    uint32_t ag_force_features;
    uint32_t hf_force_features;
    bool eaudio_debug_enable;
    bool watch_is_sending_spp;
    bool force_normal_search;
    bool rfcomm_tx_data_debug;
    bool rfcomm_rx_data_debug;
    bool only_accept_ata_atd_sco;
    bool disc_outgoing_acl_if_local_has_link_key_but_remote_dont_have;
    bool dont_allow_incoming_acl_if_local_no_link_key_but_remote_have;
    bool bt_pan_connect_enable;
    bool bt_delay_save_device_record;
    bool a2dp_source_disable_avrcp_sdp_ct;
    bool a2dp_sink_disable_avrcp_sdp_tg;
    bool a2dp_source_pts_test;
    bool pts_test_dont_bt_role_switch;
    bool a2dp_sink_avdtp_err_code_pts_test;
    bool le_audio_enabled;
    bool hsp_enable;
    // hci trace config
    bool hci_buff_trace_enable;
    bool hci_buffer_trace_high_enable;
    bool hci_trace_enable;
    bool hci_a2dp_stream_trace_enable;
    bool hci_acl_packet_trace_enable;
    bool hci_iso_packet_trace_enable;
};
extern struct besbt_cfg_t besbt_cfg;

#ifdef __cplusplus
}
#endif
#endif /* BESBT_H */
