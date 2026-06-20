/**
 * @file aob_volume_api.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-06-30
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

#ifndef __AOB_VOLUME_API_H__
#define __AOB_VOLUME_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "aob_mgr_gaf_evt.h"

/******************************macro defination*****************************/
#define MAX_AOB_VOL    (255)
#define MIN_AOB_VOL    (0)
/******************************type defination******************************/
enum app_vc_char_type
{
    /// Volume State characteristic
    APP_VC_CHAR_TYPE_STATE = 0,
    /// Volume Flags characteristic
    APP_VC_CHAR_TYPE_FLAGS,

    APP_VC_CHAR_TYPE_MAX,
};

// @arc_vc_changed_reason
enum app_vc_changed_reason
{
    /// Local change
    APP_VC_CHANGED_LOCAL = 0,
    /// Remote change
    APP_VC_CHANGED_REMOTE,

    APP_VC_CHANGED_MAX,
};

/****************************function declaration***************************/

/**
 * @brief Get volume info API on earbud
 * @param[in] none
 *
 */
app_gaf_arc_vcs_volume_ind_t *aob_volume_info_get(void);

/**
 * @brief Volume get API on earbud
 * @param[in] none
 *
 */
uint8_t aob_volume_get(void);

/**
 * @brief Volume mute API on earbud
 * @param[in] none
 *
 */
void aob_vol_mute(void);

/**
 * @brief Volume unmute API on earbud
 * @param[in] none
 *
 */
void aob_vol_unmute(void);

/**
 * @brief Volume up API on earbud, volume increase in a relative way
 *
 */
void aob_vol_up(void);

/**
 * @brief Volume down API on earbud, volume decrease in a relative way
 *
 */
void aob_vol_down(void);

/**
 * @brief Set absolute volume API on earbud side
 *
 * @param vol           param_description
 */
void aob_vol_set_abs(uint8_t vol);

/**
 * @brief Set absolute volume API on earbud side and just notify peer
 * 
 * @param  con_lid     Connection local index
 * @param  le_vol      LE Volume
 * 
 */
void aob_vol_set_and_notify_abs_volume(uint8_t con_lid, uint8_t le_vol);

/**
 * @brief Set codec gain directly
 *
 * @param local_vol    Mapping to level in codec
 */
void aob_vol_set_local_volume(uint8_t local_vol);

/**
 * @brief Set audio location API on earbuds side
 *
 * @param output_lid     Output local index
 * @param value          Value to set
 */
void aob_vol_set_volume_offset(uint8_t output_lid, uint32_t value);

/**
 * @brief Set vocs audio location API on earbuds side
 *
 * @param output_lid     Output local index
 * @param value          Value to set
 */
void aob_vol_set_audio_location(uint8_t output_lid, uint32_t value);

/**
 * @brief Set vocs output description API on earbuds side
 *
 * @param output_lid     Output local index
 * @param p_val          Value to set
 * @param val_len        Value length to set
 */
void aob_vol_set_output_description(uint8_t output_lid,
                                    uint8_t *p_val, uint8_t val_len);

/**
 * @brief Convert local vol to LE vol (Volume level in BLE stack)
 *
 * @param  bt_vol
 * @return le_vol
 */
uint8_t aob_convert_local_vol_to_le_vol(uint8_t bt_vol);

/**
 * @brief Covert the LE vol (Volume level in BLE stack) to local vol
 *
 */
uint8_t aob_convert_le_vol_to_local_vol(uint8_t le_vol);

/**
 * @brief Get real time volume level(level in BLE stack)
 *
 */
uint8_t aob_vol_get_real_time_volume(uint8_t con_lid);

/**
 * @brief Store volume level into NV
 * @param  pBdAddr      device address
 * @param  leVol        Volume level in BLE stack
 */
void aob_vol_store_volume_into_nv(uint8_t con_lid, uint8_t leVol);

/**
 * @brief send notification to remote device
 * @param  con_lid      Local connection ID
 * @param  char_type    Characteristic type
 */
void aob_vol_send_notification(uint8_t con_lid, enum app_vc_char_type char_type);

void aob_vol_sync_volume_info_cb(uint8_t *buf, uint8_t len);

void aob_vol_api_init(void);

#ifdef AOB_MOBILE_ENABLED
/**
 * @brief Volume mute API on mobile, volume mute in a relative way
 *
 * @param con_lid        Connection index to set volume
 */
void aob_mobile_vol_mute(uint8_t con_lid);

/**
 * @brief Volume unmute API on mobile, volume unmute in a relative way
 *
 * @param con_lid        Connection index to set volume
 */
void aob_mobile_vol_unmute(uint8_t con_lid);

/**
 * @brief Volume up API on mobile, volume increase in a relative way
 *
 * @param con_lid        Connection index to set volume
 */
void aob_mobile_vol_up(uint8_t con_lid);

/**
 * @brief Volume down API on mobile, volume decrease in a relative way
 *
 * @param con_lid        Connection index to set volume
 */
void aob_mobile_vol_down(uint8_t con_lid);

/**
 * @brief Set absolute volume API on mobile side
 *
 * @param con_lid        Connection index to set volume
 * @param local_vol      Local Volume to set (0-16)
 */
void aob_mobile_vol_set_abs(uint8_t con_lid, uint8_t local_vol);

/**
 * @brief Set absolute volume API on mobile side
 *
 * @param con_lid        Connection index to set volume
 * @param le_vol         LE Volume to set (0-255)
 */
void aob_mobile_vol_set_abs_le_vol(uint8_t con_lid, uint8_t le_vol);

/**
 * @brief Set volume offset on mobile site
 *
 * @param con_lid        Connection index to set volume
 * @param output_lid     Output local index
 * @param value          Value to set
 */
void aob_mobile_vol_set_volume_offset(uint8_t con_lid, uint8_t output_lid, int16_t value);

/**
 * @brief Set audio location API on mobile site
 *
 * @param con_lid        Connection index to set volume
 * @param output_lid     Output local index
 * @param value          Value to set
 */
void aob_mobile_vol_set_audio_location(uint8_t con_lid, uint8_t output_lid, int16_t value);

/**
 * @brief Set MIC input gain
 *
 * @param con_lid        Connection index to set gain
 * @param input_lid      Input local index
 * @param gain           Gain to set
 */
void aob_mobile_aic_set_input_gain(uint8_t con_lid, uint8_t input_lid, int16_t gain);

/**
 * @brief Set MIC input mute or unmute
 *
 * @param con_lid        Connection index to set mute
 * @param input_lid      Input local index
 * @param mute           Mute or Unmute
 */
void aob_mobile_aic_set_input_mute(uint8_t con_lid, uint8_t input_lid, bool mute);

/**
 * @brief Set MIC gain mode
 *
 * @param con_lid        Connection index to set gain mode
 * @param input_lid      Input local index
 * @param auto_mode      Auto gain mode or manual gain mode
 */
void aob_mobile_aic_set_input_gain_mode(uint8_t con_lid, uint8_t input_lid, bool auto_mode);

/**
 * @brief Read aics character
 *
 * @param con_lid        Connection index to set gain mode
 * @param input_lid      Input local index
 * @param char_type      Character type
 */
void aob_mobile_aic_read_input_char(uint8_t con_lid, uint8_t input_lid, uint8_t char_type);

void aob_mobile_mic_read_char(uint8_t con_lid, uint8_t input_lid, uint8_t char_type);

void aob_mobile_vocc_read(uint8_t con_lid, uint8_t output_lid, uint8_t char_type);

void aob_vol_mobile_api_init(void);

void aob_mobile_vol_up_unmute(uint8_t con_lid);

void aob_mobile_vol_down_unmute(uint8_t con_lid);

#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_VOLUME_API_H__ */

/// @} AOB_APP
