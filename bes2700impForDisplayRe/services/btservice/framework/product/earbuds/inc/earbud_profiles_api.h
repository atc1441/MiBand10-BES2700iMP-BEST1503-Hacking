/**
 ****************************************************************************************
 *
 * @file app_ibrt_common_api.h
 *
 * @brief APIs For Customer
 *
 * Copyright 2015-2023 BES.
 *
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
 ****************************************************************************************
 */

#ifndef __EARBUD_PROFILES_API__
#define __EARBUD_PROFILES_API__
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_hfp.h"

typedef enum {
    IBRT_PROFILE_STATUS_SUCCESS                  = 0,
    IBRT_PROFILE_STATUS_PENDING                  = 1,
    IBRT_PROFILE_STATUS_ERROR_INVALID_PARAMETERS = 2,
    IBRT_PROFILE_STATUS_ERROR_NO_CONNECTION      = 3,
    IBRT_PROFILE_STATUS_ERROR_CONNECTION_EXISTS  = 4,
    IBRT_PROFILE_STATUS_IN_PROGRESS              = 5,
    IBRT_PROFILE_STATUS_ERROR_DUPLICATE_REQUEST  = 6,
    IBRT_PROFILE_STATUS_ERROR_INVALID_STATE      = 7,
    IBRT_PROFILE_STATUS_ERROR_TIMEOUT            = 8,
    IBRT_PROFILE_STATUS_ERROR_ROLE_SWITCH_FAILED = 9,
    IBRT_PROFILE_STATUS_ERROR_UNEXPECTED_VALUE  = 10,
    IBRT_PROFILE_STATUS_ERROR_OP_NOT_ALLOWED    = 11,
} AppIbrtStatus;

typedef enum {
    IBRT_PROFILE_A2DP_IDLE             = 0,
    IBRT_PROFILE_A2DP_CODEC_CONFIGURED = 1,
    IBRT_PROFILE_A2DP_OPEN             = 2,
    IBRT_PROFILE_A2DP_STREAMING        = 3,
    IBRT_PROFILE_A2DP_CLOSED           = 4,
}AppIbrtA2dpState;

typedef enum {
    IBRT_PROFILE_AVRCP_DISCONNECTED   = 0,
    IBRT_PROFILE_AVRCP_CONNECTED      = 1,
    IBRT_PROFILE_AVRCP_PLAYING        = 2,
    IBRT_PROFILE_AVRCP_PAUSED         = 3,
    IBRT_PROFILE_AVRCP_VOLUME_UPDATED = 4,
}AppIbrtAvrcpState;

typedef enum {
    IBRT_PROFILE_HFP_SLC_DISCONNECTED = 0,
    IBRT_PROFILE_HFP_CLOSED           = 1,
    IBRT_PROFILE_HFP_SCO_CLOSED       = 2,
    IBRT_PROFILE_HFP_PENDING          = 3,
    IBRT_PROFILE_HFP_SLC_OPEN         = 4,
    IBRT_PROFILE_HFP_NEGOTIATE        = 5,
    IBRT_PROFILE_HFP_CODEC_CONFIGURED = 6,
    IBRT_PROFILE_HFP_SCO_OPEN         = 7,
    IBRT_PROFILE_HFP_INCOMING_CALL    = 8,
    IBRT_PROFILE_HFP_OUTGOING_CALL    = 9,
    IBRT_PROFILE_HFP_RING_INDICATION  = 10,
} AppIbrtHfpState;

typedef enum {
    IBRT_PROFILE_NO_CALL             = 0,
    IBRT_PROFILE_CALL_ACTIVE         = 1,
    IBRT_PROFILE_HOLD                = 2,
    IBRT_PROFILE_SETUP_INCOMMING     = 3,
    IBRT_PROFILE_SETUP_OUTGOING      = 4,
    IBRT_PROFILE_SETUP_ALERT         = 5,
} AppIbrtCallStatus;

typedef enum {
    IBRT_PROFILES_HFP_PROFILE_ID = 1,
    IBRT_PROFILES_A2DP_PROFILE_ID = 2,
    IBRT_PROFILES_AVRCP_PROFILE_ID = 3,
    IBRT_PROFILES_SDP_PROFILE_ID = 4,
    IBRT_PROFILES_HID_PROFILE_ID = 5,
    IBRT_PROFILES_MAX_PROFILE_ID,
} EarbudProfilesIdEnum;

typedef void (*app_ibrt_spp_write_result_callback_t)(char* data, uint32_t length, bool isSuccessful);
typedef void (*BSIR_event_callback_t)(uint8_t is_in_band_ring);
typedef bool (*earbud_profiles_allow_resume_request)(bt_bdaddr_t *bdaddr);

#ifdef __cplusplus
extern "C" {
#endif

/**************************************APIs For Customer**********************************************/

/**
 ****************************************************************************************
 * @brief Connect hfp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_connect_hfp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Connect a2dp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_connect_a2dp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Connect avrcp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_connect_avrcp_profile(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Disconnect hfp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_disconnect_hfp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Disconnect a2dp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_disconnect_a2dp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Disconnect avrcp profile.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_disconnect_avrcp_profile(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send media play command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_play(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send media pause command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_pause(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Send media forward command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_forward(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send media backward command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_backward(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send media volume up command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_volume_up(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send media volume down command.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_volume_down(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Send the command to set media absolute volume.
 *
 * @param[in] device_id            device index
 * @param[in] volume               absolute volume value
 ****************************************************************************************
 */
void app_ibrt_if_a2dp_send_set_abs_volume(uint8_t device_id, uint8_t volume);

/**
 ****************************************************************************************
 * @brief Create an audio sco link.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_create_audio_link(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Disconnect an audio sco link.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_disc_audio_link(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief redial a call.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_call_redial(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief answer a call.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_call_answer(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief hangup a call.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_call_hangup(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief hold a call.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_call_hold(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Set local volume up.
 ****************************************************************************************
 */
void app_ibrt_if_set_local_volume_up(void);

/**
 ****************************************************************************************
 * @brief Set local volume down.
 ****************************************************************************************
 */
void app_ibrt_if_set_local_volume_down(void);

/**
 ****************************************************************************************
 * @brief Get A2DP stream state of specific mobile device address.
 *
 * @param[in] addr                Specify the mobile device address to get A2DP stream state
 * @param[out] a2dp_state         Pointer at which pointer to indicate A2DP stream state
 *
 * @return An error status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_get_a2dp_state(bt_bdaddr_t *addr, AppIbrtA2dpState *a2dp_state);

/**
 ****************************************************************************************
 * @brief Get AVRCP playback state of specific mobile device address.
 *
 * @param[in] addr                Specify the mobile device address to get AVRCP palyback state
 * @param[out] avrcp_state        Pointer at which pointer to indicate AVRCP palyback state
 *
 * @return An error status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_get_avrcp_state(bt_bdaddr_t *addr,AppIbrtAvrcpState *avrcp_state);

/**
 ****************************************************************************************
 * @brief Get HFP state iof specific mobile device address.
 *
 * @param[in] addr               Specify the mobile device address to get HFP state
 * @param[out] hfp_state         Pointer at which pointer to indicate HFP state
 *
 * @return An error status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_get_hfp_state(bt_bdaddr_t *addr, AppIbrtHfpState *hfp_state);

/**
 ****************************************************************************************
 * @brief Get HFP call status of specific mobile device address.
 *
 * @param[in] addr               Specify the mobile device address to get HFP Call status
 * @param[out] call_status       Pointer at which pointer to indicate HFP Call status
 *
 * @return An error status
 ****************************************************************************************
 */
AppIbrtStatus app_ibrt_if_get_hfp_call_status(bt_bdaddr_t *addr, AppIbrtCallStatus *call_status);

void app_ibrt_if_disonnect_rfcomm(bt_spp_channel_t *spp_chan, uint8_t reason);

void app_ibrt_if_enable_hfp_voice_assistant(bool isEnable);

void app_ibrt_if_spp_write(bt_spp_channel_t *dev, char *buff, uint16_t length,
                            app_ibrt_spp_write_result_callback_t func);

void app_ibrt_if_set_a2dp_current_abs_volume(int device_id, uint8_t volume);

void app_ibrt_if_a2dp_set_delay(a2dp_stream_t *Stream, uint16_t delayMs);
void app_ibrt_if_a2dp_foreward_streaming_set_delay(uint16_t delayMs);

/**
 ****************************************************************************************
 * @brief switch audio sco link.
 ****************************************************************************************
 */
void app_ibrt_if_switch_streaming_sco(void);

/**
 ****************************************************************************************
 * @brief switch streaming a2dp link.
 ****************************************************************************************
 */
void app_ibrt_if_switch_streaming_a2dp(void);

/**
 ****************************************************************************************
 * @brief hangup a incoming call when another call is call active in same device.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_3way_hungup_incoming(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief convert a customer battery level to SDK battery level .
 *
 * @param[in] level, max ,min
 ****************************************************************************************
 */
void app_ibrt_if_hf_battery_report_ext(uint8_t level, uint8_t min, uint8_t max);

/**
 ****************************************************************************************
 * @brief report battery level .
 *
 * @param[in] level            battery level
 ****************************************************************************************
 */
void app_ibrt_if_hf_battery_report(uint8_t level);

/**
 ****************************************************************************************
 * @brief hangup active call accept incoming call.
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_hf_3way_hungup_active_accept_incomming(uint8_t device_id);

void app_ibrt_if_hold_background_switch(void);

void app_ibrt_if_register_BSIR_command_event_callback(BSIR_event_callback_t callback);

void app_ibrt_if_BSIR_command_event(uint8_t is_in_band_ring);

void app_ibrt_if_ui_allow_resume_a2dp_callback(earbud_profiles_allow_resume_request allow_resume_request_cb);

#ifdef __cplusplus
}
#endif

#endif
