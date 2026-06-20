/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#ifndef __BT_HFP_SERVICE_H__
#define __BT_HFP_SERVICE_H__
#include "adapter_service.h"
#include "hfp_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

/**
 * standard hfp (hf role) interface
 *
 */

typedef enum {
    BT_HF_FEAT_ECNR         = 0x0001,
    BT_HF_FEAT_3WAY         = 0x0002,
    BT_HF_FEAT_CLI          = 0x0004,
    BT_HF_FEAT_VR           = 0x0008,
    BT_HF_FEAT_RVC          = 0x0010,
    BT_HF_FEAT_ECS          = 0x0020,
    BT_HF_FEAT_ECC          = 0x0040,
    BT_HF_FEAT_CODEC        = 0x0080,
    BT_HF_FEAT_HF_IND       = 0x0100,
    BT_HF_FEAT_ESCO_S4_T2   = 0x0200,
} bt_hf_feature_t;

typedef enum {
    BT_AG_FEAT_3WAY        = 0x0001,
    BT_AG_FEAT_ECNR        = 0x0002,
    BT_AG_FEAT_VR          = 0x0004,
    BT_AG_FEAT_INBAND      = 0x0008,
    BT_AG_FEAT_VTAG        = 0x0010,
    BT_AG_FEAT_REJ_CALL    = 0x0020,
    BT_AG_FEAT_ECS         = 0x0040,
    BT_AG_FEAT_ECC         = 0x0080,
    BT_AG_FEAT_EXT_ERR     = 0x0100,
    BT_AG_FEAT_CODEC       = 0x0200,
    BT_AG_FEAT_HF_IND      = 0x0400,
    BT_AG_FEAT_ESCO_S4_T2  = 0x0800,
} bt_ag_feature_t;

typedef enum {
    BT_HF_VR_STATE_STOPPED = 0,
    BT_HF_VR_STATE_STARTED,
} bt_hf_vr_state_t;

typedef enum {
    BT_HF_VOLUME_TYPE_SPK = 0,
    BT_HF_VOLUME_TYPE_MIC
} bt_hf_volume_type_t;

typedef enum {
    BT_HF_NETWORK_STATE_NOT_AVAILABLE = 0,
    BT_HF_NETWORK_STATE_AVAILABLE,
} bt_hf_network_state_t;

typedef enum {
    BT_HF_SERVICE_TYPE_HOME = 0,
    BT_HF_SERVICE_TYPE_ROAMING,
} bt_hf_service_type_t;

typedef enum {
    BT_HF_CALL_NO_CALLS_IN_PROGRESS = 0,
    BT_HF_CALL_CALLS_IN_PROGRESS,
} bt_hf_call_ind_t;

typedef enum {
    BT_HF_CALLSETUP_NONE = 0,
    BT_HF_CALLSETUP_INCOMING,
    BT_HF_CALLSETUP_OUTGOING,
    BT_HF_CALLSETUP_ALERTING,
} bt_hf_callsetup_ind_t;

typedef enum {
    BT_HF_CALLHELD_NONE = 0,
    BT_HF_CALLHELD_HOLD_AND_ACTIVE,
    BT_HF_CALLHELD_HOLD,
} bt_hf_callheld_ind_t;

typedef enum {
    BT_HF_CALL_STATE_ACTIVE = 0,
    BT_HF_CALL_STATE_HELD,
    BT_HF_CALL_STATE_DIALING,
    BT_HF_CALL_STATE_ALERTING,
    BT_HF_CALL_STATE_INCOMING,
    BT_HF_CALL_STATE_WAITING,
    BT_HF_CALL_STATE_HELD_BY_RESP_HOLD,
    BT_HF_CALL_STATE_DISCONNECTED,
} bt_hf_call_state_t;

typedef enum {
    BT_HF_CALL_DIRECTION_OUTGOING = 0,
    BT_HF_CALL_DIRECTION_INCOMING,
} bt_hf_call_direction_t;

typedef enum {
    BT_HF_CALL_MPTY_TYPE_SINGLE = 0,
    BT_HF_CALL_MPTY_TYPE_MULTI,
} bt_hf_call_mpty_type_t;

typedef enum {
    BT_HF_CALL_ACTION_CHLD_0 = 0,
    BT_HF_CALL_ACTION_CHLD_1,
    BT_HF_CALL_ACTION_CHLD_2,
    BT_HF_CALL_ACTION_CHLD_3,
    BT_HF_CALL_ACTION_CHLD_4,
    BT_HF_CALL_ACTION_CHLD_1x,
    BT_HF_CALL_ACTION_CHLD_2x,
    BT_HF_CALL_ACTION_ATA,
    BT_HF_CALL_ACTION_CHUP,
    BT_HF_CALL_ACTION_REDIAL,
    BT_HF_CALL_ACTION_BTRH_0,
    BT_HF_CALL_ACTION_BTRH_1,
    BT_HF_CALL_ACTION_BTRH_2,
} bt_hf_call_action_t;

typedef enum {
    BT_HF_RESP_AND_HOLD_HELD = 0,
    BT_HF_RESP_AND_HOLD_ACCEPT,
    BT_HF_RESP_AND_HOLD_REJECT,
} bt_hf_resp_and_hold_t;

typedef enum {
    BT_HF_CALL_MODE_VOICE = 0,
    BT_HF_CALL_MODE_DATA,
    BT_HF_CALL_MODE_FAX,
} bt_hf_call_mode_t;

typedef enum {
    BT_HF_CALL_SERVICE_UNKNOWN = 0,
    BT_HF_CALL_SERVICE_VOICE = 4,
    BT_HF_CALL_SERVICE_FAX = 5,
} bt_hf_cnum_service_t;

typedef enum {
    BT_HF_IN_BAND_RINGTONE_NOT_PROVIDED = 0,
    BT_HF_IN_BAND_RINGTONE_PROVIDED,
} bt_hf_in_band_ring_state_t;

typedef enum {
    BT_HF_AUDIO_CODEC_NONE = 0,
    BT_HF_AUDIO_CODEC_CVSD = 1,
    BT_HF_AUDIO_CODEC_MSBC = 2,
    BT_HF_AUDIO_CODEC_XXXX = 16,
} bt_hf_audio_codec_t;

typedef enum {
    BT_HF_CMD_COMPLETE_OK = 0,
    BT_HF_CMD_COMPLETE_ERROR,
    BT_HF_CMD_COMPLETE_ERROR_NO_CARRIER,
    BT_HF_CMD_COMPLETE_ERROR_BUSY,
    BT_HF_CMD_COMPLETE_ERROR_NO_ANSWER,
    BT_HF_CMD_COMPLETE_ERROR_DELAYED,
    BT_HF_CMD_COMPLETE_ERROR_BLACKLISTED,
    BT_HF_CMD_COMPLETE_ERROR_CME,
} bt_hf_cmd_complete_t;

typedef enum {
    BT_HF_AT_RESPONSE_OK = 0,
    BT_HF_AT_RESPONSE_ERROR,
    BT_HF_AT_RESPONSE_ERROR_NO_CARRIER,
    BT_HF_AT_RESPONSE_ERROR_BUSY,
    BT_HF_AT_RESPONSE_ERROR_NO_ANSWER,
    BT_HF_AT_RESPONSE_ERROR_DELAYED,
    BT_HF_AT_RESPONSE_ERROR_BLACKLISTED,
    BT_HF_AT_RESPONSE_ERROR_CME,
} bt_hf_at_response_t;

typedef enum {
    BT_HF_CALL_NUMBER_TYPE_UNKNOWN = 0x81,
    BT_HF_CALL_NUMBER_TYPE_INTERNATIONAL = 0x91,
} bt_hf_call_number_type_t;

typedef enum {
    BT_HF_IND_ENHANCED_DRIVER_SAFETY = 1,
    BT_HF_IND_BATTERY_LEVEL_STATUS = 2,
} bt_hf_ind_type_t;

typedef enum
{
    HFCALL_MACHINE_CURRENT_IDLE = 0,                                //0
    HFCALL_MACHINE_CURRENT_INCOMMING,                               //1
    HFCALL_MACHINE_CURRENT_OUTGOING,                                //2
    HFCALL_MACHINE_CURRENT_CALLING,                                 //3
    HFCALL_MACHINE_CURRENT_3WAY_INCOMMING,                          //4
    HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING,                       //5
    HFCALL_MACHINE_CURRENT_IDLE_ANOTHER_IDLE,                       //6
    HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_IDLE,                  //7
    HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_IDLE,                   //8
    HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_IDLE,                    //9
    HFCALL_MACHINE_CURRENT_3WAY_INCOMMING_ANOTHER_IDLE,             //10
    HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING_ANOTHER_IDLE,          //11
    HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_CALLING,               //13
    HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_INCOMMING,             //14
    HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_INCOMMING,               //15
    HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_CHANGETOPHONE,           //16
    HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_INCOMMING,              //17
    HFCALL_MACHINE_CURRENT_OUTGOING_ANOTHER_CALLING,
    HFCALL_MACHINE_CURRENT_INCOMMING_ANOTHER_OUTGOING,              //18
    HFCALL_MACHINE_CURRENT_CALLING_ANOTHER_HOLD,                    //19
    HFCALL_MECHINE_CURRENT_3WAY_HOLD_ANOTHER_IDLE,                  //20
    HFCALL_MACHINE_NUM
} HFCALL_MACHINE_ENUM;

typedef struct {
    uint8_t error_code;
    uint8_t call_status;
    uint8_t callsetup_status;
    uint8_t callhold_status;
    uint32_t peer_feat;
} bt_hf_opened_param_t;

typedef struct {
    uint8_t error_code;
} bt_hf_closed_param_t;

typedef struct {
    bt_hf_network_state_t state;
} bt_hf_network_state_param_t;

typedef struct {
    bt_hf_service_type_t type;
} bt_hf_network_roaming_param_t;

typedef struct {
    uint8_t signal_strength;
} bt_hf_network_signal_param_t;

typedef struct {
    uint8_t battery_level;
} bt_hf_battery_level_param_t;

typedef struct {
    bt_hf_call_ind_t call;
} bt_hf_call_ind_param_t;

typedef struct {
    bt_hf_callsetup_ind_t callsetup;
} bt_hf_callsetup_ind_param_t;

typedef struct {
    bt_hf_callheld_ind_t callheld;
} bt_hf_callheld_ind_param_t;

typedef struct {
    const char *number;
    int number_len;
} bt_hf_clip_ind_param_t;

typedef struct {
    const char *number;
    int number_len;
} bt_hf_call_waiting_ind_param_t;

typedef struct {
    bt_hf_resp_and_hold_t resp_and_hold;
} bt_hf_resp_and_hold_param_t;

typedef struct {
    int index;
    bt_hf_call_direction_t dir;
    bt_hf_call_state_t state;
    bt_hf_call_mode_t mode;
    bt_hf_call_mpty_type_t mpty;
    const char *number;
} bt_hf_current_calls_param_t;

typedef struct {
    bt_hf_volume_type_t type;
    int volume;
} bt_hf_volume_change_param_t;

typedef struct {
    bt_hf_volume_type_t type;
    int volume;
} bt_hf_volume_control_param_t;

typedef struct {
    bt_hf_cmd_complete_t type;
    int cme_error;
} bt_hf_at_cmd_complete_param_t;

typedef struct {
    const char *name;
} bt_hf_current_operator_param_t;

typedef struct {
    const char *number;
    bt_hf_cnum_service_t service;
} bt_hf_subscriber_info_param_t;

typedef struct {
    bt_hf_vr_state_t state;
} bt_hf_vr_state_change_param_t;

typedef struct {
    bt_hf_in_band_ring_state_t state;
} bt_hf_in_band_ring_state_param_t;

typedef struct {
    const char *number;
} bt_hf_last_voice_tag_number_param_t;

typedef struct {
    const char *remote_unknown_event;
} bt_hf_receive_unknown_param_t;

typedef union {
    bt_hf_opened_param_t *opened;
    bt_hf_closed_param_t *closed;
    bt_hf_network_state_param_t *network_state;
    bt_hf_network_roaming_param_t *network_roaming;
    bt_hf_network_signal_param_t *network_signal;
    bt_hf_battery_level_param_t *battery_level;
    bt_hf_call_ind_param_t *call_ind;
    bt_hf_callsetup_ind_param_t *callsetup_ind;
    bt_hf_callheld_ind_param_t *callheld_ind;
    bt_hf_clip_ind_param_t *clip_ind;
    bt_hf_call_waiting_ind_param_t *call_waiting;
    bt_hf_resp_and_hold_param_t *rsp_and_hold;
    bt_hf_current_calls_param_t *current_calls;
    bt_hf_volume_change_param_t *volume_change;
    bt_hf_at_cmd_complete_param_t *cmd_complete;
    bt_hf_current_operator_param_t *current_operator;
    bt_hf_subscriber_info_param_t *subscriber_info;
    bt_hf_vr_state_change_param_t *vr_state;
    bt_hf_in_band_ring_state_param_t *in_band_ring_state;
    bt_hf_last_voice_tag_number_param_t *last_voice_tag_number;
    bt_hf_receive_unknown_param_t *receive_unknown;
} bt_hf_callback_param_t;

typedef enum {
    BT_HF_EVENT_OPENED = BT_EVENT_HF_OPENED,
    BT_HF_EVENT_CLOSED,
    BT_HF_EVENT_AUDIO_OPENED,
    BT_HF_EVENT_AUDIO_CLOSED,
    BT_HF_EVENT_NETWORK_STATE,
    BT_HF_EVENT_NETWORK_ROAMING,
    BT_HF_EVENT_NETWORK_SIGNAL,
    BT_HF_EVENT_BATTERY_LEVEL,
    BT_HF_EVENT_CALL_IND,
    BT_HF_EVENT_CALLSETUP_IND,
    BT_HF_EVENT_CALLHELD_IND,
    BT_HF_EVENT_RING_IND,
    BT_HF_EVENT_CLIP_IND,
    BT_HF_EVENT_CALL_WAITING_IND,
    BT_HF_EVENT_RESP_AND_HOLD,
    BT_HF_EVENT_CURRENT_CALLS,
    BT_HF_EVENT_VOLUME_CHANGE,
    BT_HF_EVENT_AT_CMD_COMPLETE,
    BT_HF_EVENT_CURRENT_OPERATOR,
    BT_HF_EVENT_SUBSCRIBER_INFO,
    BT_HF_EVENT_VOICE_RECOGNITION_STATE,
    BT_HF_EVENT_IN_BAND_RING_STATE,
    BT_HF_EVENT_LAST_VOICE_TAG_NUMBER,
    BT_HF_EVENT_RECEIVE_UNKNOWN,
    BT_HF_EVENT_END,
} bt_hf_event_t;

#if BT_HF_EVENT_END != BT_EVENT_HF_END
#error "bt_hf_event_t error define"
#endif

typedef int (*bt_hf_callback_t)(const bt_bdaddr_t *bd_addr, bt_hf_event_t event, bt_hf_callback_param_t param);

bt_status_t bt_hf_init(bt_hf_callback_t callback);
bt_status_t bt_hf_cleanup(void);
bt_status_t bt_hf_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_connect_audio(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_disconnect_audio(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_start_voice_recognition(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_stop_voice_recognition(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume);
bt_status_t bt_hf_dial(const bt_bdaddr_t *bd_addr, const char *number); // if number is NULL, redial last number
bt_status_t bt_hf_dial_memory(const bt_bdaddr_t *bd_addr, int location);
bt_status_t bt_hf_handle_call_action(const bt_bdaddr_t *bd_addr, bt_hf_call_action_t action, int idx);
bt_status_t bt_hf_query_current_calls(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_query_current_operator_name(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_retrieve_subscriber_info(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_send_dtmf(const bt_bdaddr_t *bd_addr, char code);
bt_status_t bt_hf_request_last_voice_tag_number(const bt_bdaddr_t *bd_addr);
bt_status_t bt_hf_send_at_cmd(const bt_bdaddr_t *bd_addr, const char *at_cmd_str);
bt_hfp_state_t bt_hf_get_state(int device_id);
/**
 * standard hfp (ag role) interface
 *
 */

#ifdef BT_HFP_AG_ROLE

typedef struct {
    bt_hf_network_state_t service;
    bt_hf_service_type_t roam;
    int signal;
    int battery_level;
} bt_ag_device_status_t;

typedef struct {
    bt_hf_network_state_t service;
    bt_hf_service_type_t roam;
    int signal;
    int battery_level;
    int num_active;
    int num_held;
    bt_hf_call_state_t call_state;
} bt_ag_cind_status_t;

typedef struct {
    int index;
    bt_hf_call_direction_t dir;
    bt_hf_call_state_t state;
    bt_hf_call_mode_t mode;
    bt_hf_call_mpty_type_t mpty;
    bt_hf_call_number_type_t number_type;
    const char *number;
} bt_ag_clcc_status_t;

typedef struct {
    int num_active;
    int num_held;
    bt_hf_call_state_t call_state;
    bt_hf_call_number_type_t number_type;
    const char *number;
    const char *caller_name;
} bt_ag_phone_status_t;

typedef struct {
    uint8_t service;
    uint8_t roam;
    uint8_t signal;
    uint8_t battchg;
    uint8_t call;
    uint8_t callsetup;
    uint8_t callheld;
} bt_ag_ind_status_t;

typedef struct {
    const char *number;
} bt_hf_dial_call_param_t;

typedef struct {
    int location;
} bt_hf_dial_memory_param_t;

typedef struct {
    bt_hf_call_action_t action;
    int idx;
} bt_hf_call_action_req_param_t;

typedef struct {
    bool wbs_enable;
    bt_hf_audio_codec_t codec;
} bt_hf_wbs_state_param_t;

typedef struct {
    bool service;
    bool roam;
    bool signal;
    bool battery;
} bt_hf_bia_ind_param_t;

typedef struct {
    bt_hf_ind_type_t ind_type;
    int ind_value;
} bt_hf_biev_ind_param_t;

typedef struct {
    char dtmf;
} bt_hf_dtmf_req_param_t;

typedef struct {
    bool start_voice_recognition;
} bt_hf_at_bvra_req_param_t;

typedef struct {
    bool enable_nrec; // noise reduction/echo cancellation
} bt_hf_at_nrec_req_param_t;

typedef struct { // bluetooth hf indicators
    const char *at_string;
} bt_hf_at_bind_req_param_t;

typedef struct {
    const char *unknown_at_cmd;
} bt_hf_unknown_at_req_param_t;

typedef union {
    bt_hf_opened_param_t *opened;
    bt_hf_closed_param_t *closed;
    bt_hf_volume_control_param_t *volume_control;
    bt_hf_wbs_state_param_t *wbs_state;
    bt_hf_bia_ind_param_t *bia_ind;
    bt_hf_biev_ind_param_t *biev_ind;
    bt_hf_dial_call_param_t *dial_call_req;
    bt_hf_dial_memory_param_t *dial_memory_req;
    bt_hf_call_action_req_param_t *call_action_req;
    bt_hf_dtmf_req_param_t *dtmf_req;
    bt_hf_at_bvra_req_param_t *bvra_req;
    bt_hf_at_nrec_req_param_t *nrec_req;
    bt_hf_at_bind_req_param_t *bind_req;
    bt_hf_unknown_at_req_param_t *unknown_at_req;
} bt_ag_callback_param_t;

typedef enum {
    BT_AG_EVENT_OPENED = BT_EVENT_AG_OPENED,
    BT_AG_EVENT_CLOSED,
    BT_AG_EVENT_AUDIO_OPENED,
    BT_AG_EVENT_AUDIO_CLOSED,
    BT_AG_EVENT_VOLUME_CONTROL,
    BT_AG_EVENT_WBS_STATE,
    BT_AG_EVENT_BIA_IND,
    BT_AG_EVENT_BIEV_IND,
    BT_AG_EVENT_DIAL_CALL_REQ,
    BT_AG_EVENT_DIAL_MEMORY_REQ,
    BT_AG_EVENT_CALL_ACTION_REQ,
    BT_AG_EVENT_AT_DTMF_REQ,
    BT_AG_EVENT_AT_BVRA_REQ,
    BT_AG_EVENT_AT_NREC_REQ,
    BT_AG_EVENT_AT_CNUM_REQ,
    BT_AG_EVENT_AT_CIND_REQ,
    BT_AG_EVENT_AT_COPS_REQ,
    BT_AG_EVENT_AT_CLCC_REQ,
    BT_AG_EVENT_AT_CKPD_REQ,
    BT_AG_EVENT_AT_BIND_REQ,
    BT_AG_EVENT_UNKNOWN_AT_REQ,
    BT_AG_EVENT_END,
} bt_ag_event_t;

#if BT_AG_EVENT_END != BT_EVENT_AG_END
#error "bt_ag_event_t error define"
#endif

typedef int (*bt_ag_callback_t)(const bt_bdaddr_t *bd_addr, bt_ag_event_t event, bt_ag_callback_param_t param);

bt_status_t bt_ag_init(bt_ag_callback_t callback);
bt_status_t bt_ag_cleanup(void);
bt_status_t bt_ag_connect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_disconnect(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_connect_audio(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_disconnect_audio(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_start_voice_recoginition(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_stop_voice_recoginition(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_volume_control(const bt_bdaddr_t *bd_addr, bt_hf_volume_type_t type, int volume);
bt_status_t bt_ag_set_curr_at_upper_handle(const bt_bdaddr_t *bd_addr);
bt_status_t bt_ag_cops_response(const bt_bdaddr_t *bd_addr, const char *operator_name);
bt_status_t bt_ag_clcc_response(const bt_bdaddr_t *bd_addr, const bt_ag_clcc_status_t *status);
bt_status_t bt_ag_cind_response(const bt_bdaddr_t *bd_addr, const bt_ag_cind_status_t *status);
bt_status_t bt_ag_send_response_code(const bt_bdaddr_t *bd_addr, bt_hf_at_response_t response_code, int cme_error);
bt_status_t bt_ag_device_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_device_status_t *status);
bt_status_t bt_ag_phone_status_change(const bt_bdaddr_t *bd_addr, const bt_ag_phone_status_t *status);
bt_status_t bt_ag_send_at_result(const bt_bdaddr_t *bd_addr, const char *at_result);
bt_status_t bt_ag_set_sco_allowed(const bt_bdaddr_t *bd_addr, bool sco_enable);
bt_status_t bt_ag_send_bsir(const bt_bdaddr_t *bd_addr, bool in_band_ring_enable);
int bt_ag_is_noise_reduction_supported(const bt_bdaddr_t *bd_addr);
int bt_ag_is_voice_recognition_supported(const bt_bdaddr_t *bd_addr);

#endif /* BT_HFP_AG_ROLE */
#ifdef __cplusplus
}
#endif
#endif /* __BT_HFP_SERVICE_H__ */

