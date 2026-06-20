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
#ifndef __BAP_PACC__
#define __BAP_PACC__
#include "bluetooth.h"

#include "generic_audio.h"
#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "pac_common.h"

#define PACC_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

#define PACC_MAX_SUPP_PAC_SET_NUM_TOTAL         (10)

#define PACC_UUID_TO_DIRECTION(pac_uuid)        ((pac_uuid == GATT_CHAR_UUID_SINK_PAC || \
                                                    pac_uuid == GATT_CHAR_UUID_SINK_PAC_V2) ? BAP_DIRECTION_SINK : BAP_DIRECTION_SRC)

typedef struct pacc_prf_svc_info pacc_prf_svc_t;
typedef struct pacc_pac_set_record pacc_pac_set_record_t;

/// Callback for pacc bond data
typedef void (*pacc_cb_bond_data_evt)(uint8_t con_lid, const pacc_prf_svc_t *param);
/// Callback for pacc discovery done
typedef void (*pacc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for pacc gatt set cfg cmp evt
typedef void (*pacc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid,
                                        uint16_t err_code);
/// Callback for pacc cccd cfg value received or read
typedef void (*pacc_cb_cfg_evt)(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid, bool enabled,
                                uint16_t err_code);
/// Callback for pacc ava context value received or read
typedef void (*pacc_cb_ava_context_value_evt)(uint8_t con_lid, uint16_t sink_ava_context,
                                              uint16_t src_ava_context, uint16_t err_code);
/// Callback for pacc supp context value received or read
typedef void (*pacc_cb_supp_context_value_evt)(uint8_t con_lid, uint16_t sink_supp_context,
                                               uint16_t src_supp_context, uint16_t err_code);
/// Callback for pacc location bf value received or read
typedef void (*pacc_cb_location_value_evt)(uint8_t con_lid, uint8_t direction, uint32_t location_bf,
                                           uint16_t err_code);
/// Callback for pacc chan capa value received or read
typedef void (*pacc_cb_chan_capa_record_value_evt)(uint8_t con_lid, uint8_t num_record, uint8_t record_lid,
                                                   uint32_t channel_type, const pacs_chan_capa_desc_u *p_desc, uint16_t err_code);
/// Callback for pacc pref aud cfg value received or read
typedef void (*pacc_cb_pref_aud_cfg_record_value_evt)(uint8_t con_lid, uint8_t num_record, uint8_t record_lid,
                                                      uint16_t use_case_id, const pacs_pref_aud_cfg_ptr_t *p_pref_aud_cfg, uint16_t err_code);
/// Callback for pacc pac set record value received or read
typedef void (*pacc_cb_pac_record_value_evt)(uint8_t con_lid, uint8_t direction, uint8_t pac_lid, uint8_t num_record,
                                             uint8_t rec_lid, const pacc_pac_set_record_t *pac_record, uint16_t err_code);
/// Callback for pacc pac set V2 record value received or read
typedef void (*pacc_cb_pac_v2_record_value_evt)(uint8_t con_lid, uint8_t direction, uint8_t pac_lid, uint8_t num_record,
                                                uint8_t rec_lid, uint8_t pac_set_id, const pacc_pac_set_record_t *pac_record, uint16_t err_code);
/// Callback for pacc set sink/src audio location cmp evt
typedef void (*pacc_cb_set_location_cmp_evt)(uint8_t con_lid, uint8_t direction, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback pacc_cb_prf_status_evt;

/*Structure*/
typedef struct pacc_evt_cb
{
    /// Callback function called when client configuration for pacs has been updated
    pacc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when pacs is discovered or failed
    pacc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback for pacc gatt cmd complete
    pacc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when a cfg of cccd is received
    pacc_cb_cfg_evt cb_cfg_value;
    /// Callback function called when a value of available audio context is received
    pacc_cb_ava_context_value_evt cb_ava_context_value;
    /// Callback function called when a value of support audio context is received
    pacc_cb_supp_context_value_evt cb_supp_context_value;
    /// Callback function called when a value of audio location is received
    pacc_cb_location_value_evt cb_location_bf_value;
    /// Callback function called when a value of chan capa record is received
    pacc_cb_chan_capa_record_value_evt cb_chan_capa_record_value;
    /// Callback function called when a value of pref aud cfg record is received
    pacc_cb_pref_aud_cfg_record_value_evt cb_pref_aud_cfg_record_value;
    /// Callback function called when a value of pac record is received
    pacc_cb_pac_record_value_evt cb_pac_record_value;
    /// Callback function called when a value of pac v2 record is received
    pacc_cb_pac_v2_record_value_evt cb_pac_v2_record_value;
    /// Callback function called when set location cmd is cmp
    pacc_cb_set_location_cmp_evt cb_set_location_cmp;
    /// Callback function called when prf status event generated
    pacc_cb_prf_status_evt cb_prf_status_event;
} pacc_evt_cb_t;

struct pacc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

struct pacc_pac_set_record
{
    const uint8_t *p_codec_id;
    const gen_aud_capa_ptr_t *p_capa_codec;
    const gen_aud_metadata_ptr_t *p_metadata;
};

typedef struct pacc_init_cfg
{
    /// Max support sink pac found
    uint8_t max_supp_sink_pac;
    /// Max support src pac found
    uint8_t max_supp_src_pac;
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Max support sink pac v2 found
    uint8_t max_supp_sink_pac_v2;
    /// Max support src pac v2 found
    uint8_t max_supp_src_pac_v2;
} pacc_init_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief Published Audio capability client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int pacc_init(const pacc_init_cfg_t *p_init_cfg, const pacc_evt_cb_t *p_cb);

/**
 * @brief Published Audio capability client deinitilization
 *
 * @return int         status
 */
int pacc_deinit(void);

/**
 * @brief Published Audio capability client pacs service discovery
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int pacc_service_discovery(uint8_t con_lid);

/**
 * @brief Published Audio capability client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 * @param  pac_lid     PAC local index
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int pacc_character_cccd_write(uint8_t con_lid, uint8_t char_type, uint8_t pac_lid, bool enable_ntf);

/**
 * @brief Published Audio capability client read available context
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int pacc_ava_context_read(uint8_t con_lid);

/**
 * @brief Published Audio capability client read supported context
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int pacc_supp_context_read(uint8_t con_lid);

/**
 * @brief Published Audio capability client read audio location
 *
 * @param  con_lid     Connection local index
 * @param  direction   Direction @see bap_direction
 *
 * @return int         status
 */
int pacc_audio_location_value_read(uint8_t con_lid, uint8_t direction);

/**
 * @brief Published Audio capability client read Channel Capabilities value
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int pacc_channel_capa_read(uint8_t con_lid);

/**
 * @brief Published Audio capability client read Preferred Audio Configurations value
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int pacc_pref_aud_cfg_read(uint8_t con_lid);

/**
 * @brief Published Audio capability client read PAC value
 *
 * @param  con_lid     Connection local index
 * @param  direction   Direction @see bap_direction
 * @param  pac_lid     PAC local index
 *
 * @return int         status
 */
int pacc_pac_value_read(uint8_t con_lid, uint8_t direction, uint8_t pac_lid);

/**
 * @brief Published Audio capability client read PAC v2 value
 *
 * @param  con_lid     Connection local index
 * @param  direction   Direction @see bap_direction
 * @param  pac_lid     PAC local index for v2
 *
 * @return int         status
 */
int pacc_pac_v2_value_read(uint8_t con_lid, uint8_t direction, uint8_t pac_lid);

/**
 * @brief Published Audio capability client write audio location
 *
 * @param  con_lid     Connection local index
 * @param  direction   Direction @see bap_direction
 * @param  location_bf Audio location bitfield @see gen_aud_supp_loc_bf_e
 *
 * @return int         status
 */
int pacc_audio_location_write(uint8_t con_lid, uint8_t direction, uint32_t location_bf);

#endif /// __BAP_PACC__