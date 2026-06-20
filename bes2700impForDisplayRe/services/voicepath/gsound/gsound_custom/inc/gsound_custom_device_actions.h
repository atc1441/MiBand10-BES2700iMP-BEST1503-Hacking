/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/


#ifndef __GSOUND_CUSTOM_DEVICE_ACTIONS_H__
#define __GSOUND_CUSTOM_DEVICE_ACTIONS_H__

/*****************************header include********************************/
#include "gsound_target_device_actions.h"
#include "current_device_state.pb.h"

#ifdef __cplusplus
extern "C"{
#endif

/******************************macro defination*****************************/
// The google-bisto-dev-v1 device model id can be used for general development
// purposes, as it registers support for every capability supported on action.
// devices.types.HEADPHONES devices. However, you must switch to a production
// device model id prior to certification and launch.
//#define DEVICE_MODEL        "google-bisto-dev-v1"

// you must be consistent with the update strategy -
// if you provide complete state reporting in some cases,
// you can never implement partial state reporting.

// The device is providing a complete update to the status, including values
// for battery levels of all devices(in the case of a multi-part device).
// If you report an update for all, do not report partial updates for
// battery_details, current_noise_cancellation_level, etc.
//#define UPDATE_KEY_ALL      "all"

// All battery levels are being updated. If a device part (e.g., the
// right ear bud) is disconnected, a BatteryDetail message is
// provided indicating that is_connected is false.
// Note: when reporting battery_details for a multi-part device like
// wireless earbuds, the state update MUST include a
// BatteryDetail message for every device part (e.g., left, right,
// case), which indicates whether the device is connected or not.
//#define UPDATE_KEY_BATTERY  "battery_details"

// The current level of noise cancellation, from
// 0...SliderProperties.max_value (inclusive)
//#define UPDATE_KEY_ANC      "current_noise_cancellation_level"

// The current level of ambient mode, from
// 0...SliderProperties.max_value (inclusive)
//#define UPDATE_KEY_AM       "current_audio_ambient_mode_level"

// device battery index
#define SINGLE_BATTERY_DEVICE_IDX   0
#define UNSPECIFIED_DEVICE_TYPE_IDX 0
#define LEFT_EAR_BUD_IDX            0
#define RIGHT_EAR_BUD_IDX           1
#define EAR_BUD_CASE_IDX            2

/******************************type defination******************************/
typedef enum
{
    DEVICE_PART_SINGLE_PIECE = 1,
    DEVICE_PART_EARBUDS_WITHOUT_CASE = 2,
    DEVICE_PART_EARBUDS_WITH_CASE = 3,
} DEVICE_PART_E;

/****************************function declearation**************************/
/**
 * @brief Callback function of state update consumed.
 * 
 * @param key           APP consumed key.
 */
void gsound_custom_device_actions_state_update_consumed(const uint8_t * const key);

/**
 * @brief Handling request from glib to request the device state.
 * 
 * @return GSoundStatus handling result.
 */
GSoundStatus gsound_custom_device_actions_request_state(void);

/**
 * @brief Excute the device action requested by gsound APP
 * 
 * @param action        string of actions
 * @param actionLen     length of string of actions
 * @return GSoundStatus result of the excution
 */
GSoundStatus gsound_custom_device_actions_excute(const uint8_t *const action,
                                                 uint32_t actionLen);

/**
 * @brief store the device actions related handlers pointer comes from gsound library
 * 
 * @param hanlders pointer of device actions interface
 */
void gsound_custom_device_actions_store_lib_interface(const GSoundDeviceActionsInterface *hanlders);
void gsound_custom_deivce_acionts_update_init(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_DEVICE_ACTIONS_H__ */
