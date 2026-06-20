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
#ifndef __BAP_BC_SOURCE_INC__
#define __BAP_BC_SOURCE_INC__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"

#include "public_broadcast_audio.h"

/*DEFINE*/
#define BAP_BC_SRC_GRP_MAX_SUPP_NUM     (GAF_SUPP_MAX_BIG_NUM)

#define BAP_BC_SRC_GRP_INC_ADI_IN_PA    (false)

/*btif_me_enumerate_remote_devices*/
enum bap_bc_src_state
{
    /// Broadcast Group in Idle state
    BAP_BC_SRC_STATE_IDLE,
    /// Broadcast Group in Configured state
    /// - Periodic Advertising started
    BAP_BC_SRC_STATE_CONFIGURED,
    /// Broadcast Group in Streaming state
    /// - Periodic Advertising started
    /// - Broadcast Group started
    BAP_BC_SRC_STATE_STREAMING,
};

/*TYPEDEF*/
typedef void (*bap_bc_src_grp_state)(uint8_t grp_lid, uint8_t state);

typedef void (*bap_bc_src_big_create_state)(uint8_t grp_lid, const bap_big_opened_t *p_bis_timing);

typedef void (*bap_bc_src_big_term)(uint8_t grp_lid, uint8_t big_id, uint16_t err_code);

typedef void (*bap_bc_src_iso_dp_state)(bool is_create, uint8_t grp_lid, uint8_t big_id, uint8_t bis_idx, uint16_t iso_hdl, uint16_t err_code);

typedef struct bap_bc_src_evt_callback
{
    bap_bc_src_grp_state cb_src_grp_state;

    bap_bc_src_big_create_state cb_big_create_state;

    bap_bc_src_big_term cb_big_term;

    bap_bc_src_iso_dp_state cb_bis_iso_dp_state;
} bap_bc_src_evt_cb_t;

typedef struct bap_bc_src_init_cfg
{

} bap_bc_src_init_cfg_t;

typedef struct bap_bc_src_pa_param
{
    /// PA interval
    uint16_t pa_interval;
    /// Include TX power
    bool include_tx_pwr;
    // Additional Periodic adv data
    gen_aud_var_info_t add_pa_data;
} bap_bc_src_pa_param_t;

/*FUNCTIONS*/
/**
 * @brief BAP broadcast Source initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_evt_cb    Event callbacks
 *
 * @return int         status
 */
int bap_bc_src_init(const bap_bc_src_init_cfg_t *p_init_cfg, const bap_bc_src_evt_cb_t *p_evt_cb);

/**
 * @brief BAP broadcast Source deinitilization
 *
 * @return int         status
 */
int bap_bc_src_deinit(void);

/**
 * @brief BAP broadcast Source Create Source Group
 *
 * @param  p_broadcast_id
 *                     Broadcast ID value pointer
 * @param  nb_subgrp   Number of subgrps
 * @param  nb_bis_stream
 *                     Number if BIS streams
 * @param  p_broadcaster_name
 *                     Broadcaster Name value pointer set in EA
 * @param  p_pba_info  PBA info
 * @param  p_ea_param  Extend Advertising paramters
 * @param  p_add_ea_data
 *                     Additional Extend advertising data
 * @param  p_grp_lid   Group local index return
 *
 * @return int         status
 */
int bap_bc_src_create_group(const uint8_t *p_broadcast_id,
                            // Number of subgrp
                            uint8_t nb_subgrp,
                            // Number of bis stream
                            uint8_t nb_bis_stream,
                            // Broadcaster name
                            const pbp_broadcast_name_ptr_t *p_broadcaster_name,
                            // PBP PBA
                            const pbp_pba_info_t *p_pba_info,
                            // EA param
                            const gap_adv_param_t *p_ea_param,
                            // Additional extend adv data
                            const gen_aud_var_info_t *p_add_ea_data,
                            // Group local index return
                            uint8_t *p_grp_lid);

/**
 * @brief BAP broadcast Source Set subgrp info
 *
 * @param  grp_lid     Group local index
 * @param  subgrp_lid  Subgro local index
 * @param  p_codec_id  Codec ID value pointer
 * @param  p_cc        Codec Configuration
 * @param  p_md        Metadata
 *
 * @return int         status
 */
int bap_bc_src_set_subgrp_parameter(uint8_t grp_lid, uint8_t subgrp_lid,
                                    const uint8_t *p_codec_id,
                                    const gen_aud_cc_t *p_cc,
                                    const gen_aud_metadata_t *p_md);

/**
 * @brief BAP broadcast Source Set stream info
 *
 * @param  grp_lid     Group local index
 * @param  stream_lid  Stream local index
 * @param  subgrp_lid_belong_to
 *                     Subgrp local index belong to
 * @param  p_codec_cfg Codec Configuration
 *
 * @return int         status
 */
int bap_bc_src_set_stream_parameter(uint8_t grp_lid, uint8_t stream_lid,
                                    uint8_t subgrp_lid_belong_to,
                                    const gen_aud_cc_t *p_codec_cfg);

/**
 * @brief BAP broadcast Source Configure Group to create PA and BIG
 *
 * @param  grp_lid     Group local index
 * @param  pres_delay  Presentation Delay set in BASE
 * @param  p_pa_param  Periodic Advertising paramters
 * @param  p_big_param Create BIG paramters
 *
 * @return int         status
 */
int bap_bc_src_configure_group(uint8_t grp_lid,
                               // Presentation Delay
                               uint32_t pres_delay,
                               // PA param
                               const bap_bc_src_pa_param_t *p_pa_param,
                               // BIG param
                               const bap_big_param_t *p_big_param);

/**
 * @brief BAP broadcast Source Disable group related EA
 *
 * @param  grp_lid     Group local index
 *
 * @return int         status
 */
int bap_bc_src_disable_group_related_ea(uint8_t grp_lid);

/**
 * @brief BAP broadcast Source disable group related PA
 *
 * @param  grp_lid     Group local index
 *
 * @return int         status
 */
int bap_bc_src_disable_group_related_pa(uint8_t grp_lid);

/**
 * @brief BAP broadcast Source Enable Group stream with iso dapatah setup
 *
 * @param  grp_lid     Group local index
 * @param  bis_index_bf
 *                     BIS index bitfield
 *
 * @return int         status
 */
int bap_bc_src_enable_group_stream(uint8_t grp_lid, uint32_t bis_index_bf);

/**
 * @brief BAP broadcast Source update group metadata
 *
 * @param  grp_lid     Group local index
 * @param  subgrp_lid_update
 *                     Subgrp local index to update metadata
 * @param  p_md_subgrp New metadata to place old metadata
 *
 * @return int         status
 */
int bap_bc_src_update_group_metadata(uint8_t grp_lid,
                                     /// Number of Subgrp
                                     uint8_t subgrp_lid_update,
                                     // Metadata Info for subgrp
                                     const gen_aud_metadata_t *p_md_subgrp);

/**
 * @brief BAP broadcast Source update group additional ea data
 *
 * @param  grp_lid     Group local index
 * @param  p_add_pa_data
 *                     Additional EA data
 *
 * @return int         status
 */
int bap_bc_src_update_group_additional_ea_data(uint8_t grp_lid,
                                               const gen_aud_var_info_t *p_add_ea_data);

/**
 * @brief BAP broadcast Source update group additional pa data
 *
 * @param  grp_lid     Group local index
 * @param  p_add_pa_data
 *                     Additional PA data
 *
 * @return int         status
 */
int bap_bc_src_update_group_additional_pa_data(uint8_t grp_lid,
                                               const gen_aud_var_info_t *p_add_pa_data);

/**
 * @brief BAP broadcast Source disable group stream with iso datapath remove
 *
 * @param  grp_lid     Group local index
 * @param  bis_index_bf
 *                     BIS index bitfield
 *
 * @return int         status
 */
int bap_bc_src_disable_group_stream(uint8_t grp_lid, uint32_t bis_index_bf);

/**
 * @brief BAP broadcast Source release source group with BIG/PA/EA terminate
 *
 * @param  grp_lid     Group local index
 *
 * @return int         status
 */
int bap_bc_src_release_group(uint8_t grp_lid);

/**
 * @brief BAP broadcast Source delete Source Group
 *
 * @param  grp_lid     Group local index
 *
 * @return int         status
 */
int bap_bc_src_delete_group(uint8_t grp_lid);

#endif /// __BAP_BC_SOURCE_INC__