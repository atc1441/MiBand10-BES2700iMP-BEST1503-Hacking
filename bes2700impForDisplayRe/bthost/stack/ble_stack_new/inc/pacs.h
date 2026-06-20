/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
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
 ****************************************************************************/
#ifndef __BAP_PACS__
#define __BAP_PACS__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "pac_common.h"

#define PACS_CONNECTION_MAX             (GAF_CONNECTION_MAX)

#define PACS_MIN_PAC_LID                (0)

#define PACS_MAX_SUPP_PAC_SET_NUM_TOTAL (10)

#define PACS_MAX_SUPP_CHAN_CAPA_NUM     (10)

#define PACS_MAX_SUPP_PREF_AUD_CFG_NUM  (20)

/// Callback for pacs bond data
typedef void (*pacs_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint16_t cli_cfg_bf);
/// Callback for pacs audio location changed
typedef void (*pacs_cb_loc_change)(uint8_t con_lid, uint8_t direction, uint32_t location_bf);
/// Callback for pacs characteristic cccd written
typedef void (*pacs_cb_cccd_written)(uint8_t con_lid, uint8_t char_type);

/*Structure*/
typedef struct pacs_evt_cb
{
    /// Callback function called when client configuration for PACS has been updated
    pacs_cb_bond_data cb_bond_data;
    /// Callback function called when either Sink Audio Locations or Source Audio Locations characteristic has been
    /// written by a client device
    pacs_cb_loc_change cb_location;
    /// Callback function called when Characteristic Configuration descriptor has been
    /// written by a client device
    pacs_cb_cccd_written cb_cccd_written;
} pacs_evt_cb_t;

typedef struct pacs_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Supported Audio Locations bit field for Sink direction
    uint32_t location_bf_sink;
    /// Supported Audio Locations bit field for Source direction
    uint32_t location_bf_src;
    /// Supported Audio Contexts bit field for Sink direction
    uint16_t supp_context_bf_sink;
    /// Supported Audio Contexts bit field for Source direction
    uint16_t supp_context_bf_src;
    /// Sink PAC number supported
    uint8_t num_sink_pac_supp;
    /// Source PAC number supported
    uint8_t num_src_pac_supp;
    /// Sink PAC V2 number supported
    uint8_t num_sink_pac_v2_supp;
    /// Source PAC V2 number supported
    uint8_t num_src_pac_v2_supp;
} pacs_init_cfg_t;

/*FUNCTIONS DECLARATION*/
/**
 * @brief Published Audio capability server initilization
 *
 * @param  pacs_init_cfg
 *                     Initilization configuration
 * @param  pacs_evt_cb Event callbacks
 *
 * @return int         status
 */
int pacs_init(const pacs_init_cfg_t *pacs_init_cfg, const pacs_evt_cb_t *pacs_evt_cb);

/**
 * @brief Published Audio capability server deinitilization
 *
 * @return int         status
 */
int pacs_deinit(void);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Published Audio capability server restore client configuration bitfiled
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_bf  client configuration bitfiled
 *
 * @return int         status
 */
int pacs_restore_cli_cfg_cache(uint8_t con_lid, uint16_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Published Audio capability server set available audio context
 *
 * @param  con_lid     Connection local index
 * @param  direction   Direction @see bap_direction
 * @param  ava_audio_context_bf
 *                     Available audio context bitfield
 *
 * @return int         status
 */
int pacs_set_ava_audio_context(uint8_t con_lid, uint8_t direction, uint16_t ava_audio_context_bf);

/**
 * @brief Published Audio capability server set supported audio context
 *
 * @param  direction   Direction @see bap_direction
 * @param  supp_audio_context_bf
 *                     Available audio context bitfield
 *
 * @return int         status
 */
int pacs_set_supp_audio_context(uint8_t direction, uint32_t supp_audio_context_bf);

/**
 * @brief Published Audio capability server set audio location
 *
 * @param  direction   Direction @see bap_direction
 * @param  audio_location_bf
 *                     Audio location bitfield
 *
 * @return int         status
 */
int pacs_set_audio_location(uint8_t direction, uint32_t audio_location_bf);

/**
 * @brief Published Audio capability server add pac record
 *
 * @param  pac_lid     Pac local index
 * @param  record_lid  Pac record local index
 * @param  p_codec_id  Codec ID value pointer
 * @param  p_capa_codec
 *                     Codec Capability vale pointer
 * @param  p_metadata  Metadata Capability value pointer
 *
 * @return int         status
 */
int pacs_add_pac_record(uint8_t pac_lid, uint8_t record_lid, const uint8_t *p_codec_id,
                        const gen_aud_capa_t *p_capa_codec, const gen_aud_metadata_t *p_metadata);

/**
 * @brief Published Audio capability server delete pac record
 *
 * @param  record_lid  Pac record local index
 *
 * @return int         status
 */
int pacs_del_pac_record(uint8_t record_lid);

/**
 * @brief Published Audio capability server add pac v2 record
 *
 * @param  pac_set_lid PAC set local index to distinguish cross all PAC v2 set
 * @param  record_lid  Pac record local index
 * @param  p_codec_id  Codec ID value pointer
 * @param  p_capa_codec
 *                     Codec Capability vale pointer
 * @param  p_metadata  Metadata Capability value pointer
 *
 * @return int         status
 */
int pacs_add_pac_v2_record(uint8_t pac_set_lid, uint8_t record_lid, const uint8_t *p_codec_id,
                           const gen_aud_var_info_t *p_capa_codec, const gen_aud_metadata_t *p_metadata);

/**
 * @brief Published Audio capability server delete pac v2 record
 *
 * @param  record_lid  Pac record local index
 *
 * @return int         status
 */
int pacs_del_pac_v2_record(uint8_t record_lid);

/**
 * @brief Published Audio capability server add channel capabilities record
 *
 * @param  record_lid  Channel capabilities record local index
 * @param  channel_type Channel Type @see (enum pacs_channel_type_id << 16 | Company ID)
 * @param  p_desc      Channel capabilities descriptor @see pacs_chan_capa_desc_u
 *
 * @return int         status
 */
int pacs_add_chan_capa_record(uint8_t record_lid, uint32_t channel_type, const pacs_chan_capa_desc_u *p_desc);

/**
 * @brief Published Audio capability server delete channel capabilities record
 *
 * @param  record_lid  Channel capabilities record local index
 *
 * @return int         status
 */
int pacs_del_chan_capa_record(uint8_t record_lid);

/**
 * @brief Published Audio capability server add preferred audio configurations record
 *
 * @param  record_lid  Preferred Audio configurations record local index
 * @param  pref_aud_cfg
 *                     Preferred Audio Configurations
 *
 * @return int         status
 */
int pacs_add_preferred_audio_cfg_record(uint8_t record_lid, const pacs_pref_aud_cfg_t *pref_aud_cfg);

/**
 * @brief Published Audio capability server delete preferred audio configurations record
 *
 * @param  record_lid  Preferred Audio configurations record local index
 *
 * @return int         status
 */
int pacs_del_pref_aud_cfg_record(uint8_t record_lid);

/**
 * @brief Published Audio capability server delete preferred audio configurations record by use case
 *
 * @param  use_case_id Preferred Audio configurations use case id
 *
 * @return uint8_t     Deleted preferred audio configurations count
 */
uint8_t pacs_del_pref_aud_cfg_record_by_use_case(uint16_t use_case_id);

#endif /// __BAP_PACS__