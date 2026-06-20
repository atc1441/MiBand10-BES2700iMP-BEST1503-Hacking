/**
 * @file aob_pacs_api.h
 * @author BES AI team
 * @version 0.1
 * @date 2022-04-18
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

#ifndef __AOB_PACS_API_H__
#define __AOB_PACS_API_H__

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/
#include "ble_audio_define.h"
#include "aob_mgr_gaf_evt.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
/// Codec Capabilities structure
typedef struct
{
    /// Pointer to Codec Capabilities structure (allocated by Upper Layer)
    app_gaf_bap_capa_t *capa;
    /// Pointer to Codec Capabilities Metadata structure (allocated by Upper Layer)
    app_gaf_bap_capa_metadata_t *metadata;
} aob_codec_capa_t;

/***************************function declaration****************************/
void aob_pacs_api_init(void);

/**
 ****************************************************************************************
 * @brief Add sink capability record.
 *
 * @param[in] codec_id      Codec Identifier
 * @param[in] p_codec_capa  codec capablity
 *
 ****************************************************************************************
 */

void aob_pacs_add_sink_pac_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa);

/**
 ****************************************************************************************
 * @brief clear all sink capablity.
 *
 *
 ****************************************************************************************
 */
void aob_pacs_clear_all_sink_capa(void);

/**
 ****************************************************************************************
 * @brief Add src capability record.
 *
 * @param[in] codec_id      Codec Identifier
 * @param[in] p_codec_capa  codec capablity
 *
 ****************************************************************************************
 */

void aob_pacs_add_src_pac_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa);

/**
 ****************************************************************************************
 * @brief Add pacs channel capability record.
 *
 * @param[in] channel_type  Channel Type
 * @param[in] p_desc_val    Descriptor value in ltv
 * @param[in] val_len       Length of descriptor value in ltv
 *
 ****************************************************************************************
 */
void aob_pacs_add_chan_capa_record(uint32_t channel_type, const uint8_t *p_desc_val, uint8_t val_len);

/**
 ****************************************************************************************
 * @brief Add pacs Preferred audio configurations record.
 *
 * @param[in] use_case_id   Use case id
 * @param[in] data_present_bits
 *                          Filed present bits @see app_gaf_bap_aud_cfg_data_present_bit
 * @param[in] p_pref_aud_cfg_data
 *                          Filed value pointer list corresponding to @see data_present_bits
 *
 ****************************************************************************************
 */
void aob_pacs_add_pref_aud_cfg_record(uint16_t use_case_id, uint16_t data_present_bits,
                                             const app_gaf_pref_aud_cfg_data_t *p_pref_aud_cfg_data);

/**
 ****************************************************************************************
 * @brief Delete pacs Preferred audio configurations record.
 *
 * @param[in] use_case_id   Use case id
 *
 ****************************************************************************************
 */
void aob_pacs_del_pref_aud_cfg_record(uint16_t use_case_id);

/**
 ****************************************************************************************
 * @brief Add sink capability v2 record.
 *
 * @param[in] codec_id      Codec Identifier
 * @param[in] p_codec_capa  codec capablity with no basic parameters
 *
 ****************************************************************************************
 */
void aob_pacs_add_sink_pac_v2_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa);

/**
 ****************************************************************************************
 * @brief Add src capability v2 record.
 *
 * @param[in] codec_id      Codec Identifier
 * @param[in] p_codec_capa  codec capablity with no basic parameters
 *
 ****************************************************************************************
 */
void aob_pacs_add_src_pac_v2_record(aob_codec_id_t *codec_id, aob_codec_capa_t *p_codec_capa);

/**
 ****************************************************************************************
 * @brief clear all src capablity.
 *
 *
 ****************************************************************************************
 */
void aob_pacs_clear_all_src_capa(void);

/**
 ****************************************************************************************
 * @brief start set earbud pacs audio location
 *
 * @param[in] location     Location see@AOB_MGR_LOCATION_BF_E
 *
 ****************************************************************************************
*/
void aob_pacs_set_audio_location(AOB_MGR_LOCATION_BF_E location);

/**
 ****************************************************************************************
 * @brief Get current media location.
 *
 * @param[in] direction       media location (@see AOB_MGR_LOCATION_BF_E for elements)
 ****************************************************************************************
 */
uint32_t aob_pacs_get_cur_audio_location(uint8_t direction);

/**
 ****************************************************************************************
 * @brief Set available audio context.
 *
 * @param[in] con_lid           Connection Local Index
 * @param[in] context_bf_ava_sink    available context type (@see AOB_MGR_CONTEXT_TYPE_BF_E for elements)
 * @param[in] context_bf_ava_src     available context type (@see AOB_MGR_CONTEXT_TYPE_BF_E for elements)
 ****************************************************************************************
 */
void aob_pacs_set_ava_audio_context(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src);

/**
 ****************************************************************************************
 * @brief Get available audio context.
 *
 * @param[in] con_lid           Connection Local Index
 * @param[in] context_bf_ava    available context type (@see AOB_MGR_CONTEXT_TYPE_BF_E for elements)
 ****************************************************************************************
 */
void aob_pacs_set_supp_audio_context(uint8_t con_lid, AOB_MGR_CONTEXT_TYPE_BF_E context_bf_supp);


#ifdef AOB_MOBILE_ENABLED
/**
 ****************************************************************************************
 * @brief start set peer device's location
 *
 * @param[in] con_lid      Connection local index
 * @param[in] location     Location see@AOB_MGR_LOCATION_BF_E
 *
 ****************************************************************************************
*/
void aob_pacs_mobile_set_location(uint8_t con_lid, uint32_t location);


/**
 ****************************************************************************************
 * @brief get peer ava ctx
 *
 * @param con_lid
 * @param direction
 * @return uint16_t
 ****************************************************************************************
 */
uint16_t aob_pacs_mobile_get_peer_ava_context(uint8_t con_lid, uint8_t direction);
#endif


#ifdef __cplusplus
}
#endif

#endif
