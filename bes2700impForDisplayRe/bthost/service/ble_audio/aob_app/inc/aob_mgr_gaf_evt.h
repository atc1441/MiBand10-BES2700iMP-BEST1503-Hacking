/**
 * @file aob_mgr_gaf_evt.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-07-08
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
 */

#ifndef __AOB_MGR_GAF_EVT_H__
#define __AOB_MGR_GAF_EVT_H__

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/
#include "app_gaf_define.h"
#include "app_bap_bc_src_msg.h"
#include "app_bap_uc_srv_msg.h"
#include "aob_call_info_define.h"
#include "ble_aob_common.h"
/******************************macro defination*****************************/

/******************************type defination******************************/

typedef enum
{
    /// Sink direction
    AOB_MGR_DIRECTION_SINK = 0,
    /// Source direction
    AOB_MGR_DIRECTION_SRC,

    AOB_MGR_DIRECTION_MAX,
} AOB_MGR_DIRECTION_E;

typedef enum
{
    AOB_VOL_CHANGED_REASON_LOCAL = 0,
    AOB_VOL_CHANGED_REASON_REMOTE,
    AOB_VOL_CHANGED_REASON_TWS,
    AOB_VOL_CHANGED_REASON_UNKNOW = 0xFF,
} AOB_VOL_CHANGED_REASON_E;

/// Media control characteristic type values
/// Values are ordered so that the notification-capable characteristic type values are equal
/// to the characteristic type values
typedef enum
{
    /// Media Player Name characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYER_NAME = 0,
    /// Track Changed characteristic
    AOB_MGR_MC_CHAR_TYPE_TRACK_CHANGED,
    /// Track Title characteristic
    AOB_MGR_MC_CHAR_TYPE_TRACK_TITLE,
    /// Track Duration characteristic
    AOB_MGR_MC_CHAR_TYPE_TRACK_DURATION,
    /// Track Position characteristic
    AOB_MGR_MC_CHAR_TYPE_TRACK_POSITION,
    /// Playback Speed characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYBACK_SPEED,
    /// Seeking Speed characteristic
    AOB_MGR_MC_CHAR_TYPE_SEEKING_SPEED,
    /// Current Track Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_CUR_TRACK_OBJ_ID,
    /// Next Track Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_NEXT_TRACK_OBJ_ID,
    /// Current Group Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_CUR_GROUP_OBJ_ID,
    /// Parent Group Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_PARENT_GROUP_OBJ_ID,
    /// Playing Order characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYING_ORDER,
    /// Media State characteristic
    AOB_MGR_MC_CHAR_TYPE_MEDIA_STATE,
    /// Media Control Point
    AOB_MGR_MC_CHAR_TYPE_MEDIA_CP,
    /// Media Control Point Opcodes Supported
    AOB_MGR_MC_CHAR_TYPE_MEDIA_CP_OPCODES_SUPP,
    /// Search Results Object ID
    AOB_MGR_MC_CHAR_TYPE_SEARCH_RESULTS_OBJ_ID,
    /// Search Control Point
    AOB_MGR_MC_CHAR_TYPE_SEARCH_CP,

    /// All characteristics above are notification-capable
    AOB_MGR_MC_CHAR_NTF_CHAR_TYPE_MAX,

    /// Current Track Segments Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_CUR_TRACK_SEG_OBJ_ID = AOB_MGR_MC_CHAR_NTF_CHAR_TYPE_MAX,
    /// Media Player Icon Object ID characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYER_ICON_OBJ_ID,
    /// Media Player Icon URL characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYER_ICON_URL,
    /// Playing Order Supported characteristic
    AOB_MGR_MC_CHAR_TYPE_PLAYING_ORDER_SUPP,
    /// Content Control ID
    AOB_MGR_MC_CHAR_TYPE_CCID,

    AOB_MGR_MC_CHAR_TYPE_MAX,
} AOB_MGR_MC_CHAR_TYPE_E;

/// Descriptor type values
enum
{
    /// Client Characteristic Configuration descriptor for Media Player Name characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_PLAYER_NAME = 0,
    /// Client Characteristic Configuration descriptor for Track Changed characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_TRACK_CHANGED,
    /// Client Characteristic Configuration descriptor for Track Title characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_TRACK_TITLE,
    /// Client Characteristic Configuration descriptor for Track Duration characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_TRACK_DURATION,
    /// Client Characteristic Configuration descriptor for Track Position characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_TRACK_POSITION,
    /// Client Characteristic Configuration descriptor for Playback Speed characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_PLAYBACK_SPEED,
    /// Client Characteristic Configuration descriptor for Seeking Speed characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_SEEKING_SPEED,
    /// Client Characteristic Configuration descriptor for Current Track Object ID characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_CUR_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Next Track Object ID characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_NEXT_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Current Group Object ID characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_CUR_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Parent Group Object ID characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_PARENT_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Playing Order characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_PLAYING_ORDER,
    /// Client Characteristic Configuration descriptor for Media State characteristic
    AOB_MGR_MC_DESC_TYPE_CCC_MEDIA_STATE,
    /// Client Characteristic Configuration descriptor for Media Control Point
    AOB_MGR_MC_DESC_TYPE_CCC_MEDIA_CP,
    /// Client Characteristic Configuration descriptor for Media Control Point Opcodes Supported
    AOB_MGR_MC_DESC_TYPE_CCC_MEDIA_CP_OPCODES_SUPP,
    /// Client Characteristic Configuration descriptor for Search Results Object ID
    AOB_MGR_MC_DESC_TYPE_CCC_SEARCH_RESULTS_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Search Control Point
    AOB_MGR_MC_DESC_TYPE_CCC_SEARCH_CP,

    AOB_MGR_MC_DESC_TYPE_MAX,
};

typedef enum
{
    /// Bearer Provider Name characteristic
    AOB_MGR_TB_CHAR_TYPE_PROV_NAME = 0,
    /// Bearer Technology characteristic
    AOB_MGR_TB_CHAR_TYPE_TECHNO,
    /// Bearer Signal Strength characteristic
    AOB_MGR_TB_CHAR_TYPE_SIGN_STRENGTH,
    /// Bearer List Current Calls characteristic
    AOB_MGR_TB_CHAR_TYPE_CURR_CALLS_LIST,
    /// Status Flags characteristic
    AOB_MGR_TB_CHAR_TYPE_STATUS_FLAGS,
    /// Incoming Call Target Bearer URI characteristic
    AOB_MGR_TB_CHAR_TYPE_IN_TGT_CALLER_ID,
    /// Call State characteristic
    AOB_MGR_TB_CHAR_TYPE_CALL_STATE,
    /// Call Control Point characteristic
    AOB_MGR_TB_CHAR_TYPE_CALL_CTL_PT,
    /// Termination Reason characteristic
    AOB_MGR_TB_CHAR_TYPE_TERM_REASON,
    /// Incoming Call characteristic
    AOB_MGR_TB_CHAR_TYPE_INCOMING_CALL,
    /// Call Friendly Name characteristic
    AOB_MGR_TB_CHAR_TYPE_CALL_FRIENDLY_NAME,
    /// Bearer URI Schemes Supported List characteristic
    AOB_MGR_TB_CHAR_TYPE_URI_SCHEMES_LIST,

    /// All characteristics above are notification-capable
    AOB_MGR_TB_NTF_CHAR_TYPE_MAX,

    /// Bearer UCI characteristic
    AOB_MGR_TB_CHAR_TYPE_UCI = AOB_MGR_TB_NTF_CHAR_TYPE_MAX,
    /// Bearer Signal Strength Reporting Interval characteristic
    AOB_MGR_TB_CHAR_TYPE_SIGN_STRENGTH_INTV,
    /// Content Control ID characteristic
    AOB_MGR_TB_CHAR_TYPE_CCID,
    /// Call Control Point Optional Opcodes characteristic
    AOB_MGR_TB_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES,

    AOB_MGR_TB_CHAR_TYPE_MAX,
} AOB_MGR_TB_CHAR_TYPE_E;

/// MEDIA control operation code values
typedef enum
{
    /// Start playing the current track
    AOB_MGR_MC_OP_PLAY = 1,
    /// Pause playing the current track
    AOB_MGR_MC_OP_PAUSE,
    /// Fast rewind the current track
    AOB_MGR_MC_OP_FAST_RW,
    /// Fast forward the current track
    AOB_MGR_MC_OP_FAST_FW,
    /// Stop current activity and return to inactive state
    AOB_MGR_MC_OP_STOP,
    /// Set the current position relative to the current position
    AOB_MGR_MC_OP_MOVE_RELATIVE = 16,
    /// Set the current position to the previous segment of the current track
    AOB_MGR_MC_OP_PREV_SEG = 32,
    /// Set the current position to the next segment of the current track
    AOB_MGR_MC_OP_NEXT_SEG,
    /// Set the current position to the first segment of the current track
    AOB_MGR_MC_OP_FIRST_SEG,
    /// Set the current position to the last segment of the current track
    AOB_MGR_MC_OP_LAST_SEG,
    /// Set the current position to the nth segment of the current track
    AOB_MGR_MC_OP_GOTO_SEG,
    /// Set the current track to the previous track in the current group playing order
    AOB_MGR_MC_OP_PREV_TRACK = 48,
    /// Set the current track to the next track in the current group playing order
    AOB_MGR_MC_OP_NEXT_TRACK,
    /// Set the current track to the first track in the current group playing order
    AOB_MGR_MC_OP_FIRST_TRACK,
    /// Set the current track to the last track in the current group playing order
    AOB_MGR_MC_OP_LAST_TRACK,
    /// Set the current track to the nth track in the current group playing order
    AOB_MGR_MC_OP_GOTO_TRACK,
    /// Set the current group to the previous group in the sequence of groups
    AOB_MGR_MC_OP_PREV_GROUP = 64,
    /// Set the current group to the next group in the sequence of groups
    AOB_MGR_MC_OP_NEXT_GROUP,
    /// Set the current group to the first group in the sequence of groups
    AOB_MGR_MC_OP_FIRST_GROUP,
    /// Set the current group to the last group in the sequence of groups
    AOB_MGR_MC_OP_LAST_GROUP,
    /// Set the current group to the nth group in the sequence of groups
    AOB_MGR_MC_OP_GOTO_GROUP,

    AOB_MGR_MC_MEDIA_CP_OPCODES_SUPP_MASK = 0x001FFFFF,
} AOB_MGR_MC_OPCODE_E;

/// Action type values
typedef enum aob_mgr_mcs_action
{
    /// No action
    AOB_MGR_MCS_ACTION_NO_ACTION = 0,
    /// Play
    AOB_MGR_MCS_ACTION_PLAY,
    /// Pause
    AOB_MGR_MCS_ACTION_PAUSE,
    /// Stop
    AOB_MGR_MCS_ACTION_STOP,
    /// Fast Forward or Fast Rewind
    AOB_MGR_MCS_ACTION_SEEK,
    /// Current Track becomes invalid
    AOB_MGR_MCS_ACTION_INACTIVE,
    /// Change track
    AOB_MGR_MCS_ACTION_CHANGE_TRACK,

    AOB_MGR_MCS_ACTION_MAX,
} AOB_MGR_MCS_ACTION_E;

/// Supported Audio Location Bitfield
typedef enum
{
    AOB_MGR_LOCATION_FRONT_LEFT               = 0x00000001,
    AOB_MGR_LOCATION_FRONT_RIGHT              = 0x00000002,
    AOB_MGR_LOCATION_FRONT_CENTER             = 0x00000004,
    AOB_MGR_LOCATION_LOW_FREQ_EFFECTS_1       = 0x00000008,
    AOB_MGR_LOCATION_BACK_LEFT                = 0x00000010,
    AOB_MGR_LOCATION_BACK_RIGHT               = 0x00000020,
    AOB_MGR_LOCATION_FRONT_LEFT_OF_CENTER     = 0x00000040,
    AOB_MGR_LOCATION_FRONT_RIGHT_OF_CENTER    = 0x00000080,
    AOB_MGR_LOCATION_BACK_CENTER              = 0x00000100,
    AOB_MGR_LOCATION_LOW_FREQ_EFFECTS_2       = 0x00000200,
    AOB_MGR_LOCATION_SIDE_LEFT                = 0x00000400,
    AOB_MGR_LOCATION_SIDE_RIGHT               = 0x00000800,
    AOB_MGR_LOCATION_TOP_FRONT_LEFT           = 0x00001000,
    AOB_MGR_LOCATION_TOP_FRONT_RIGHT          = 0x00002000,
    AOB_MGR_LOCATION_TOP_FRONT_CENTER         = 0x00004000,
    AOB_MGR_LOCATION_TOP_CENTER               = 0x00008000,
    AOB_MGR_LOCATION_TOP_BACK_LEFT            = 0x00010000,
    AOB_MGR_LOCATION_TOP_BACK_RIGHT           = 0x00020000,
    AOB_MGR_LOCATION_TOP_SIDE_LEFT            = 0x00040000,
    AOB_MGR_LOCATION_TOP_SIDE_RIGHT           = 0x00080000,
    AOB_MGR_LOCATION_TOP_BACK_CENTER          = 0x00100000,
    AOB_MGR_LOCATION_BOTTOM_FRONT_CENTER      = 0x00200000,
    AOB_MGR_LOCATION_BOTTOM_FRONT_LEFT        = 0x00400000,
    AOB_MGR_LOCATION_BOTTOM_FRONT_RIGHT       = 0x00800000,
    AOB_MGR_LOCATION_FRONT_LEFT_WIDE          = 0x01000000,
    AOB_MGR_LOCATION_FRONT_RIGHT_WIDE         = 0x02000000,
    AOB_MGR_LOCATION_LEFT_SURROUND            = 0x04000000,
    AOB_MGR_LOCATION_RIGHT_SURROUND           = 0x08000000,

    AOB_MGR_LOCATION_RFU                      = 0xF0000000,
} AOB_MGR_LOCATION_BF_E;

/// Context type bit field meaning
typedef enum
{
    AOB_AUDIO_CONTEXT_TYPE_UNSPECIFIED            = 0x0001,
    /// Conversation between humans as, for example, in telephony or video calls
    AOB_AUDIO_CONTEXT_TYPE_CONVERSATIONAL         = 0x0002,
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    AOB_AUDIO_CONTEXT_TYPE_MEDIA                  = 0x0004,
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    /// between participants; or a mix of all the above
    AOB_AUDIO_CONTEXT_TYPE_GAME                   = 0x0008,
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    AOB_AUDIO_CONTEXT_TYPE_INSTRUCTIONAL          = 0x0010,
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    AOB_AUDIO_CONTEXT_TYPE_MAN_MACHINE            = 0x0020,
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    AOB_AUDIO_CONTEXT_TYPE_LIVE                   = 0x0040,
    /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    AOB_AUDIO_CONTEXT_TYPE_SOUND_EFFECT           = 0x0080,
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    AOB_AUDIO_CONTEXT_TYPE_ATTENTION_SEEKING      = 0x0100,
    /// Ringtone as in a call alert
    AOB_AUDIO_CONTEXT_TYPE_RINGTONE               = 0x0200,
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    AOB_AUDIO_CONTEXT_TYPE_IMMEDIATE_ALERT        = 0x0400,
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    AOB_AUDIO_CONTEXT_TYPE_EMERGENCY_ALERT        = 0x0800,
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    AOB_AUDIO_CONTEXT_TYPE_TV                     = 0x1000,
} AOB_MGR_CONTEXT_TYPE_BF_E;

typedef enum
{
    /// Set Identity Resolving Key characteristic
    AOB_CSIS_CHAR_TYPE_SIRK = 0,
    /// Coordinated Set Size characteristic
    AOB_CSIS_CHAR_TYPE_SIZE,
    /// Set Member Lock characteristic
    AOB_CSIS_CHAR_TYPE_LOCK,
    /// Set Member Rank characteristic
    AOB_CSIS_CHAR_TYPE_RANK,
} AOB_CSIS_RSP_CHAR_TYPE_E;

/// Context type values
typedef enum
{
    /// Supported Audio Contexts
    AOB_MGR_CONTEXT_TYPE_SUPP = 0,
    /// Available Audio Contexts
    AOB_MGR_CONTEXT_TYPE_AVA,

    AOB_MGR_CONTEXT_TYPE_MAX
} AOB_MGR_CONTEXT_TYPE_E;
typedef struct
{
    void (*call_state_change_cb)(uint8_t con_lid, uint8_t call_id, void *param);
    void (*call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t value);
    void (*call_status_flags_ind_cb)(uint8_t con_lid, uint8_t call_id, bool inband_ring, bool silent_mode);
    void (*call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t url_len, uint8_t *url);
    void (*call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*call_action_result_ind_cb)(uint8_t con_lid, void *param);
} call_event_handler_t;

#ifdef AOB_MOBILE_ENABLED
typedef struct
{
    void (*call_state_event_cb)(uint8_t con_lid, uint8_t call_event, void *param, uint8_t status_err_code);
} call_mobile_event_handler_t;
#endif

typedef struct
{
    void (*media_track_change_cb)(uint8_t con_lid);
    void (*media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*media_mic_state_cb)(uint8_t mute);
    void (*media_iso_link_quality_cb)(void *event);
    void (*media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*ase_codec_cfg_req_handler_cb)(uint8_t con_lid, app_bap_ascs_ase_t *p_ase_info, uint8_t tgt_latency,
                                         const app_gaf_codec_id_t *codec_id, app_gaf_bap_cfg_t *codec_cfg);
    void (*ase_enable_req_handler_cb)(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context);
    void (*ase_release_req_handler_cb)(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context);
    void (*ase_update_metadata_req_handler_cb)(uint8_t con_lid, uint8_t ase_lid, app_gaf_bap_cfg_metadata_t *context, uint8_t ase_state);
} media_event_handler_t;

#ifdef AOB_MOBILE_ENABLED
typedef struct
{
    void (*media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*media_codec_capa_change_cb)(uint8_t con_lid, uint8_t type);
    void (*media_location_cb)(uint8_t con_lid, AOB_MGR_DIRECTION_E direction, AOB_MGR_LOCATION_BF_E loc);
    void (*media_control_cb)(uint8_t con_lid, uint8_t media_lid, AOB_MGR_MC_OPCODE_E opCode, int32_t val);
    void (*media_val_get_cb)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint16_t offset);
    void (*media_val_set_cb)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, uint32_t value);
    void (*media_pac_found_cb)(uint8_t con_lid);
    void (*media_sup_context_changed_cb)(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_sink, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_src);
    void (*media_ava_context_changed_cb)(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_sink, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_src);
    void (*media_ase_found_cb)(uint8_t con_lid, uint8_t nb_ases);
    void (*media_cis_group_state_change_cb)(bool isCreate, uint8_t ase_lid, uint16_t status, uint8_t grp_lid);
    void (*media_mic_state_cb)(uint8_t mute);
} media_mobile_event_handler_t;
#endif

typedef struct
{
    void (*vol_changed_cb)(uint8_t con_lid, uint8_t volume, uint8_t mute, uint8_t change_cnt, uint8_t reason);
    void (*vcs_bond_data_changed_cb)(uint8_t con_lid, uint8_t char_type, uint8_t cfg_bf);
    void (*vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*vocc_offset_changed_cb)(uint8_t con_lid, int16_t value, uint8_t output_lid);
    void (*vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*vocc_bond_data_changed_cb)(uint8_t con_lid, uint8_t output_lid);
} vol_event_handler_t;

typedef struct
{
    void (*bis_sink_state_cb)(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf);
    void (*bis_sink_enabled_cb)(uint8_t grp_lid);
    void (*bis_sink_disabled_cb)(uint8_t grp_lid);
    void (*bis_sink_stream_started_cb)(uint8_t grp_lid);
    void (*bis_sink_stream_stoped_cb)(uint8_t grp_lid);
} sink_event_handler_t;

typedef struct
{
    void (*scan_state_idle_cb)(void);
    void (*scan_state_scanning_cb)(void);
    void (*scan_state_synchronizing_cb)(uint16_t status);
    void (*scan_state_synchronized_cb)(void);
    void (*scan_state_streaming_cb)(app_gaf_bc_scan_state_stream_t *p_scan_state);
    void (*scan_pa_sync_req_cb)(uint8_t pa_lid);
    void (*scan_pa_terminate_req_cb)(uint8_t pa_lid);
    void (*scan_pa_established_cb)(uint8_t pa_lid, uint8_t *addr, uint8_t addr_type, uint8_t adv_sid, uint16_t serv_data);
    void (*scan_pa_terminated_cb)(uint8_t pa_lid, uint16_t reason);
    void (*scan_subgrp_report_cb)(app_gaf_bc_scan_subgroup_report_ind_t* subgrp_info);
    void (*scan_stream_report_cb)(app_gaf_bc_scan_stream_report_ind_t *stream_info);
    void (*scan_big_info_report_cb)(app_gaf_bc_scan_big_info_report_ind_t *big_info);
    void (*scan_report_cb)(app_gaf_bap_adv_id_t *p_addr_info, uint8_t *bcast_id,
                                            uint8_t *adv_data, uint8_t adv_data_len, int8_t rssi, bool *choose_to_sync);
} scan_event_handler_t;

typedef struct
{
    void (*bis_src_enabled_ind)(app_bap_bc_src_grp_info_t *p_grp);
    void (*bis_src_disabled_ind)(uint8_t grp_lid);
    void (*bis_src_pa_enabled_ind)(app_bap_bc_src_grp_info_t *p_grp);
    void (*bis_src_pa_disabled_ind)(uint8_t grp_lid);
    void (*bis_src_stream_started_cb)(uint8_t stream_lid, uint16_t bis_hdl);
    void (*bis_src_stream_stoped_cb)(uint8_t stream_lid, uint16_t bis_hdl);
} src_event_handler_t;

typedef struct
{
    void (*assist_solicitation_cb)(uint8_t addr_type, uint8_t *addr, uint16_t length, uint8_t *adv_data);
    void (*assist_source_state_cb)(uint16_t cmd_code, uint16_t status, uint8_t con_lid, uint8_t src_lid);
    void (*assist_bond_data_cb)(uint8_t con_lid, uint8_t nb_rx_state, uint16_t shdl, uint16_t ehdl);
    void (*assist_bcast_code_ri_cb)(uint8_t con_lid, uint8_t src_lid);
} assist_event_handler_t;

typedef struct
{
    void (*deleg_solicite_started_cb)(void);
    void (*deleg_solicite_stoped_cb)(void);
    void (*deleg_remote_scan_started)(uint8_t con_lid, uint8_t state);
    void (*deleg_remote_scan_stoped)(uint8_t con_lid, uint8_t state);
    void (*deleg_source_add_ri_cb)(uint8_t src_lid, uint8_t *bcast_id, uint8_t con_lid, uint8_t pa_sync_req);
    void (*deleg_source_remove_ri_cb)(uint8_t src_lid, uint8_t con_lid);
    void (*deleg_source_update_ri_cb)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req);
    void (*deleg_pref_bis_sync_ri_cb)(uint8_t src_lid, uint8_t con_lid);
} deleg_event_handler_t;

typedef struct
{
    void (*csip_rsi_value_updated_cb)(uint8_t *rsi, uint8_t rsi_len);
    void (*csip_ntf_sent_cb)(uint8_t con_lid, uint8_t char_type);
    void (*csip_read_rsp_sent_cb)(uint8_t con_lid, uint8_t char_type, uint8_t *p_data, uint8_t data_len);
} csip_event_handler_t;

#ifdef AOB_MOBILE_ENABLED
typedef struct
{
    void (*csip_sirk_value_report_cb)(uint8_t con_lid, uint8_t *sirk);
    void (*csip_resolve_result_report_cb)(uint8_t con_lid, uint8_t result);
    void (*csip_discover_server_cmp_ind)(uint8_t con_lid, uint8_t result);
    void (*csip_sirk_add_result_report_cb)(uint8_t key_lid, uint8_t result);
    void (*csip_sirk_remove_result_report_cb)(uint8_t key_lid, uint8_t result);
} csip_mobile_event_handler_t;
typedef struct
{
    void (*dtc_coc_connected_cb)(uint8_t con_lid, uint16_t tx_mtu, uint16_t tx_mps, uint16_t spsm);
    void (*dtc_coc_disconnected_cb)(uint8_t con_lid, uint16_t reason, uint16_t spsm);
    void (*dtc_coc_data_cb)(uint8_t con_lid, uint16_t length, uint8_t *sdu, uint16_t spsm);
} dtc_coc_event_handler_t;
#endif

typedef struct
{
    void (*dts_coc_registered_cb)(uint8_t status, uint16_t spsm);
    void (*dts_coc_connected_cb)(uint8_t con_lid, uint16_t tx_mtu, uint16_t tx_mps, uint16_t spsm, uint16_t initial_credits);
    void (*dts_coc_disconnected_cb)(uint8_t con_lid, uint16_t reason, uint16_t spsm);
    void (*dts_coc_data_cb)(uint8_t con_lid, uint16_t spsm, uint16_t length, uint8_t *sdu);
    void (*dts_coc_send_cb)(uint8_t con_lid, uint16_t spsm);
} dts_coc_event_handler_t;

typedef struct
{
    void (*cis_established_cb)(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_established);
    void (*cis_rejected_cb)(uint16_t con_hdl, uint8_t error);
    void (*cig_terminated_cb)(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid, uint8_t reason);
    void (*ase_ntf_value_cb)(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason);
    void (*cis_disconnected_cb)(app_gaf_uc_srv_cis_state_ind_t *ascs_cis_disconnected);
} cis_conn_evt_handler_t;

typedef struct
{
    void (*pacs_cccd_written_cb)(uint8_t con_lid);
} pacs_event_handler_t;

#ifdef AOB_MOBILE_ENABLED
typedef struct
{
    void (*mobile_cis_estab_cb)(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_established);
    void (*mobile_cis_discon_cb)(app_gaf_uc_cli_cis_state_ind_t *ascc_cis_disconnected);
} cis_mobile_conn_evt_handler_t;
#endif

typedef void (*aob_app_gaf_evt_cb_func)(void *event);

typedef struct
{
    uint16_t event;
    aob_app_gaf_evt_cb_func cb;
} aob_app_gaf_evt_cb_t;

/****************************function declaration***************************/
void aob_mgr_gaf_evt_handle(uint16_t evt_type, void *event_id);

void aob_mgr_gaf_evt_init(void);

void aob_mgr_call_evt_handler_register(call_event_handler_t *handlerBundle);

void aob_mgr_media_evt_handler_register(media_event_handler_t *handlerBundle);

void aob_mgr_gaf_vol_evt_handler_register(vol_event_handler_t *handlerBundle);

void aob_mgr_gaf_mobile_src_evt_handler_register(src_event_handler_t *handlerBundle);

void aob_mgr_gaf_mobile_assist_evt_handler_register(assist_event_handler_t *handlerBundle);

void aob_mgr_gaf_sink_evt_handler_register(sink_event_handler_t *handlerBundle);

void aob_mgr_gaf_scan_evt_handler_register(scan_event_handler_t *handlerBundle);

void aob_mgr_gaf_deleg_evt_handler_register(deleg_event_handler_t *handlerBundle);

void aob_mgr_csip_evt_handler_register(csip_event_handler_t *handlerBundle);

void aob_mgr_dts_coc_evt_handler_register(dts_coc_event_handler_t *handlerBundle);

void aob_mgr_cis_conn_evt_handler_t_register(cis_conn_evt_handler_t *handlerBundle);

void aob_mgr_pacs_evt_handler_t_register(pacs_event_handler_t *handlerBundle);

/****************************for client(mobile)****************************/
#ifdef AOB_MOBILE_ENABLED
void aob_mgr_gaf_mobile_evt_init(void);
void aob_mgr_gaf_mobile_evt_handle(uint16_t id, void *event_id);

void aob_mgr_gaf_mobile_media_evt_handler_register(media_mobile_event_handler_t *handlerBundle);
void aob_mgr_gaf_mobile_vol_evt_handler_register(vol_event_handler_t *handlerBundle);
void aob_mgr_gaf_mobile_call_evt_handler_register(call_mobile_event_handler_t *handlerBundle);
void aob_mgr_gaf_mobile_csip_evt_handler_register(csip_mobile_event_handler_t *handlerBundle);
void aob_mgr_dtc_coc_evt_handler_register(dtc_coc_event_handler_t *handlerBundle);
void aob_mgr_mobile_cis_conn_evt_handler_register(cis_mobile_conn_evt_handler_t *handlerBundle);

#endif //AOB_MOBILE_ENABLED

#ifdef __cplusplus
}
#endif

#endif
