/**
 ****************************************************************************************
 *
 * @file app_bap_capa_srv_msg.h
 *
 * @brief BLE Audio Published Audio Capabilities Server
 *
 * Copyright 2015-2019 BES.
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

#ifndef APP_BAP_CAPA_SRV_MSG_H_
#define APP_BAP_CAPA_SRV_MSG_H_
#if BLE_AUDIO_ENABLED
#include "app_bap.h"

/// Available Context information structure
typedef struct app_bap_capa_ava_context
{
    /// Connection Local Index
    uint8_t con_lid;
    /// Available Context Type Bitfield
    uint16_t ava_bf;
} app_bap_capa_ava_context_t;

/// Direction information structure
typedef struct app_bap_capa_srv_dir
{
    /// Supported Audio Locations for the direction
    uint32_t location_bf;
    /// Available Audio Contexts for the direction (one for each connection)
    uint16_t context_bf_ava[BLE_CONNECTION_MAX];
    /// Supported Audio Contexts for the direction
    uint16_t context_bf_supp;
    /// Number of PACs for the direction
    uint8_t nb_pacs;
} app_bap_capa_srv_dir_t;

/// PAC Record information structure
typedef struct app_bap_capa_srv_record
{
    /// List header
    struct list_node hdr;
    /// Record identifier
    uint8_t record_id;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Pointer to Codec Capabilities structure (allocated by Upper Layer)
    app_gaf_bap_capa_t *p_capa;
    /// Pointer to Codec Capabilities Metadata structure (allocated by Upper Layer)
    app_gaf_bap_capa_metadata_t *p_metadata;
} app_bap_capa_srv_record_t;

/// PAC information structure
typedef struct app_bap_capa_srv_pac
{
    /// List of records (@see app_bap_capa_srv_record_t for elements)
    struct list_node list_record;
    /// Number of records
    uint8_t nb_records;
} app_bap_capa_srv_pac_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field for Audio Stream Control Service\n
    /// Each bit correspond to a characteristic in the range [0, BAP_UC_CHAR_TYPE_ASE[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic
    uint16_t ase_cli_cfg_bf;
} app_bap_uc_srv_bond_data_ind_t;

/// Published Audio Capabilities Server environment structure
typedef struct app_bap_capa_srv_env
{
    /// Preferred Mtu
    uint8_t preferred_mtu;
    /// Total number of PAC
    uint8_t nb_pacs;
    /// Total number of PAC Records
    uint8_t nb_records;
    /// Total number of PAC V2 Records
    uint8_t nb_v2_records;
    /// Total number of Channel capability Records
    uint8_t nb_chan_capa_records;
    /// Total number of Preferred Audio Configurations Records
    uint8_t nb_pref_aud_cfg_records;
    /// Pointer to Direction information structure
    app_bap_capa_srv_dir_t dir_info[APP_GAF_DIRECTION_MAX];
    /// PAC informations (nb_pacs elements)
    app_bap_capa_srv_pac_t *p_pac_info;
} app_bap_capa_srv_env_t;

#ifdef __cplusplus
extern "C" {
#endif
int app_bap_capa_srv_restore_bond_data_req(uint8_t con_lid, uint8_t cli_cfg_bf, uint8_t pac_cli_cfg_bf);

int app_bap_capa_srv_add_pac_v2_record(uint8_t direction, app_gaf_codec_id_t *codec_id,
                                       app_gaf_bap_capa_t *p_capa, app_gaf_bap_capa_metadata_t *p_metadata);

int app_bap_capa_srv_delete_pac_v2_record(uint8_t record_lid);

int app_bap_capa_srv_add_chan_capa_record(uint32_t channel_type, const uint8_t *p_desc_val, uint8_t val_len);

int app_bap_capa_srv_delete_chan_capa_record(uint8_t record_lid);

int app_bap_capa_srv_add_pref_aud_cfg_record(uint16_t use_case_id, uint16_t data_present_bits,
                                             const app_gaf_pref_aud_cfg_data_t *p_pref_aud_cfg_data);

int app_bap_capa_srv_delete_pref_aud_cfg_record(uint8_t record_lid);

int app_bap_capa_srv_delete_pref_aud_cfg_record_by_use_case(uint16_t use_case_id);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_CAPA_SRV_MSG_H_

/// @} APP_BAP
