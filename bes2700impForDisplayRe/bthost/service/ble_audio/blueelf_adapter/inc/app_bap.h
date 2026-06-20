/**
 ****************************************************************************************
 *
 * @file app_bap.h
 *
 * @brief BLE Audio Basic Audio Profile
 *
 * Copyright 2015-2021 BES.
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

/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_H_
#define APP_BAP_H_

#if BLE_AUDIO_ENABLED
#include "bluetooth.h"
#include "ble_aob_common.h"
#include "app_gaf_define.h"

#define APP_EXT_ADV_DATA_MAX_LEN                  229 // HCI:7.7.65.13
#define APP_PER_ADV_DATA_MAX_LEN                  247 // 248 // HCI:7.7.65.16

/// Supported Roles bit field meaning
enum app_bap_role_bf
{
    /// Capabilities Server supported
    APP_BAP_ROLE_SUPP_CAPA_SRV_POS = 0,
    APP_BAP_ROLE_SUPP_CAPA_SRV_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_CAPA_SRV_POS),
    /// Capabilities Client supported
    APP_BAP_ROLE_SUPP_CAPA_CLI_POS = 1,
    APP_BAP_ROLE_SUPP_CAPA_CLI_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_CAPA_CLI_POS),
    /// Unicast Server supported
    APP_BAP_ROLE_SUPP_UC_SRV_POS = 2,
    APP_BAP_ROLE_SUPP_UC_SRV_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_UC_SRV_POS),
    /// Unicast Client supported
    APP_BAP_ROLE_SUPP_UC_CLI_POS = 3,
    APP_BAP_ROLE_SUPP_UC_CLI_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_UC_CLI_POS),
    /// Broadcast Source supported
    APP_BAP_ROLE_SUPP_BC_SRC_POS = 4,
    APP_BAP_ROLE_SUPP_BC_SRC_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_BC_SRC_POS),
    /// Broadcast Sink supported
    APP_BAP_ROLE_SUPP_BC_SINK_POS = 5,
    APP_BAP_ROLE_SUPP_BC_SINK_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_BC_SINK_POS),
    /// Broadcast Scan supported
    APP_BAP_ROLE_SUPP_BC_SCAN_POS = 6,
    APP_BAP_ROLE_SUPP_BC_SCAN_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_BC_SCAN_POS),
    /// Broadcast Scan Assistant supported
    APP_BAP_ROLE_SUPP_BC_ASSIST_POS = 7,
    APP_BAP_ROLE_SUPP_BC_ASSIST_BIT = APP_CO_BIT(APP_BAP_ROLE_SUPP_BC_ASSIST_POS),
    /// Broadcast Scan Delegator supported
    APP_BAP_ROLE_SUPP_BC_DELEG_POS  = 8,
    APP_BAP_ROLE_SUPP_BC_DELEG_BIT  = APP_CO_BIT(APP_BAP_ROLE_SUPP_BC_DELEG_POS),
};

typedef enum
{
    BAP_QOS_SETTING_MIN = 0,

    BAP_QOS_SETTING_LL_8_1_1 = BAP_QOS_SETTING_MIN,
    BAP_QOS_SETTING_LL_8_2_1,
    BAP_QOS_SETTING_LL_16_1_1,
    BAP_QOS_SETTING_LL_16_2_1,
    BAP_QOS_SETTING_LL_24_1_1,
    BAP_QOS_SETTING_LL_24_2_1,
    BAP_QOS_SETTING_LL_32_1_1,
    BAP_QOS_SETTING_LL_32_2_1,
    BAP_QOS_SETTING_LL_441_1_1,
    BAP_QOS_SETTING_LL_441_2_1,
    BAP_QOS_SETTING_LL_48_1_1,
    BAP_QOS_SETTING_LL_48_2_1,
    BAP_QOS_SETTING_LL_48_3_1,
    BAP_QOS_SETTING_LL_48_4_1,
    BAP_QOS_SETTING_LL_48_5_1,
    BAP_QOS_SETTING_LL_48_6_1,
    BAP_QOS_SETTING_LL_MAX,

    BAP_QOS_SETTING_HR_8_1_2 = BAP_QOS_SETTING_LL_MAX,
    BAP_QOS_SETTING_HR_8_2_2,
    BAP_QOS_SETTING_HR_16_1_2,
    BAP_QOS_SETTING_HR_16_2_2,
    BAP_QOS_SETTING_HR_24_1_2,
    BAP_QOS_SETTING_HR_24_2_2,
    BAP_QOS_SETTING_HR_32_1_2,
    BAP_QOS_SETTING_HR_32_2_2,
    BAP_QOS_SETTING_HR_441_1_2,
    BAP_QOS_SETTING_HR_441_2_2,
    BAP_QOS_SETTING_HR_48_1_2,
    BAP_QOS_SETTING_HR_48_2_2,
    BAP_QOS_SETTING_HR_48_3_2,
    BAP_QOS_SETTING_HR_48_4_2,
    BAP_QOS_SETTING_HR_48_5_2,
    BAP_QOS_SETTING_HR_48_6_2,

    BAP_QOS_SETTING_HR_MAX,

#if defined (AOB_GMAP_ENABLED)
    BAP_QOS_SETTING_GMING_48_1_GC = BAP_QOS_SETTING_HR_MAX,
    BAP_QOS_SETTING_GMING_48_2_GC,
    BAP_QOS_SETTING_GMING_48_1_GR,
    BAP_QOS_SETTING_GMING_48_2_GR,
    BAP_QOS_SETTING_GMING_48_3_GR,
    BAP_QOS_SETTING_GMING_48_4_GR,
    BAP_QOS_SETTING_NUM_MAX,
#else
    BAP_QOS_SETTING_NUM_MAX = BAP_QOS_SETTING_HR_MAX,
#endif
} app_bap_qos_setting_e;

typedef struct BAP_QOS_CONFIG_SETTING
{
    uint8_t     Faming_type;
    uint8_t     Rtn_num;
    uint8_t     Max_trans_latency;
    uint8_t     Pres_Delay;
    uint8_t     Sdu_intval;
    uint16_t    Oct_max;
} bap_qos_setting_t;

typedef struct BAP_AUDIO_CONFIGURATION
{
    uint8_t     sink_supp_aud_chn_cnt_bf;
    uint8_t     src_supp_aud_chn_cnt_bf;
    uint8_t     sink_max_cfs_per_sdu;
    uint8_t     src_max_cfs_per_sdu;
    uint32_t    sink_aud_location_bf;
    uint32_t    src_aud_location_bf;
    uint8_t     csip_size;
} bap_audio_cfg_t;

typedef struct
{
    int m2s_bn;
    int m2s_nse;
    int m2s_ft;
    int s2m_bn;
    int s2m_nse;
    int s2m_ft;

    int frame_cnt_per_sdu;

    int iso_interval; // unit: 1.25 ms

} CIS_TIMING_CONFIGURATION_T;

uint8_t app_bap_frame_dur_us_to_frame_dur_enum(uint32_t frame_dur_us);
bool app_bap_get_specifc_ltv_data(app_gaf_ltv_t *add_data, uint8_t specific_ltv_type, void *p_cfg_out);
void app_bap_capa_param_print(app_gaf_bap_capa_param_t *capa_param);
void app_bap_capa_print(app_gaf_bap_capa_t *p_capa);
void app_bap_capa_metadata_print(app_gaf_bap_capa_metadata_t *p_capa_metadata);
void app_bap_cfg_print(app_gaf_bap_cfg_t *p_cfg);
void app_bap_cfg_metadata_print(app_gaf_bap_cfg_metadata_t *p_cfg_metadata);

#ifdef __cplusplus
extern "C" {
#endif
bool app_bap_codec_is_lc3(const void *p_codec_id);
#ifdef LC3PLUS_SUPPORT
bool app_bap_codec_is_lc3plus(const void *p_codec_id);
#endif
void app_bap_add_data_set(uint8_t *data, uint8_t data_len);
void app_bap_start(uint8_t con_lid);

#ifdef AOB_MOBILE_ENABLED
void app_bap_update_sdu_intv(uint32_t sdu_intv_m2s_us, uint32_t sdu_intv_s2m_us);
void app_bap_set_device_num_to_be_connected(uint8_t dev_num);
uint8_t app_bap_get_device_num_to_be_connected(void);
#endif

void app_bap_update_cis_timing(CIS_TIMING_CONFIGURATION_T *pTiming);
CIS_TIMING_CONFIGURATION_T *app_bap_get_cis_timing_config(void);
gaf_bap_activity_type_e app_bap_get_activity_type(void);
void app_bap_set_activity_type(gaf_bap_activity_type_e type);
uint32_t app_bap_get_role_bit_filed(void);

/**
 * @brief Frame_dur enumerate to frame_dur (us)
 *
 * @param frame_duration_e
 * @return uint32_t
 */
uint32_t app_bap_frame_dur_enum_to_frame_dur_us(uint8_t frame_duration_e);

/**
 * @brief Input audio location bf, return L+R cnt
 *
 * @param audio_location_bf
 * @return uint8_t
 */
uint8_t app_bap_get_audio_location_l_r_cnt(uint32_t audio_location_bf);

/**
 * @brief Input audio channel support chn bf, return max supp cnt
 *
 * @param audio_channel_cnt_bf
 * @return uint8_t
 */
uint8_t app_bap_get_max_audio_channel_supp_cnt(uint32_t audio_channel_cnt_bf);

/**
 * @brief Get specific type LTV value pointer from LTV data
 *
 * @param  p_ltv_data  LTV data include total length
 * @param  ltv_type    LTV value to found used type
 *
 * @return uint8_t*    LTV value pointer if found
 */
uint8_t *app_bap_get_ltv_value_by_type(app_gaf_ltv_t *p_ltv_data, uint8_t ltv_type);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_H_

/// @} APP_BAP
