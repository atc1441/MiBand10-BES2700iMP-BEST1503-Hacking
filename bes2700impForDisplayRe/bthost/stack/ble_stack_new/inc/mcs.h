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
#ifndef __ACC_MCS__
#define __ACC_MCS__

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "mc_common.h"

#define MCS_CONNECTION_MAX          (GAF_CONNECTION_MAX)

#define MCS_GENERIC_MEDIA_LID       (0)

#define MCS_INVALID_MEDIA_LID       (GAF_INVALID_ANY_LID)

/// Callback for mcs bond data
typedef void (*mcs_cb_bond_data_evt)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                     uint32_t cli_cfg);
/// Callback function called when either Current Track Object ID or Next Track Object
typedef void (*mcs_cb_set_object_id_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type,
                                         const mcs_object_id_t *p_obj_id);
/// Callback function called when a Client device request a control on a Media
typedef void (*mcs_cb_ctrl_req)(uint8_t media_lid, uint8_t con_lid, uint8_t opcode, int32_t val);
/// Callback function called when a Search request has been received from Client device
typedef void (*mcs_cb_search_req)(uint8_t media_lid, uint8_t con_lid, uint8_t param_len,
                                  const uint8_t *p_param);
/// Callback function called when a Client device request to set value for either
typedef void (*mcs_cb_val_set_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type, uint32_t val);
/// Callback function called when a Client device request to set value for either
typedef void (*mcs_cb_val_get_req)(uint8_t media_lid, uint8_t con_lid, uint8_t char_type, uint16_t offset);
/// Callback function called to inform upper layer that a action is complete with status
typedef void (*mcs_cb_action_cmp)(uint8_t media_lid, uint8_t action, uint16_t status);

/*Structure*/
typedef struct mcs_evt_cb
{
    /// Callback function called when client configuration for (G)MCS has been updated
    mcs_cb_bond_data_evt cb_bond_data;
    /// Callback function called when either Current Track Object ID or Next Track Object
    /// ID or CUrrent Group Object ID characteristic value has been written by a Client device
    mcs_cb_set_object_id_req cb_set_object_id_req;
    /// Callback function called when a Search request has been received from Client device
    mcs_cb_search_req cb_search_req;
    /// Callback function called when a Client device request a control on a Media
    mcs_cb_ctrl_req cb_control_req;
    /// Callback function called when a Client device request to set value for either
    /// Track Position or Playback Speed or Playing Order characteristic
    mcs_cb_val_set_req cb_set_req;
    /// Callback function called when a Client device request to set value for either
    /// Media Player Name or Media Player Icon Object URL or Track Title characteristic
    mcs_cb_val_get_req cb_get_req;
    /// Callback function called to inform upper layer that a action is complete with status
    mcs_cb_action_cmp cb_action_cmp;
} mcs_evt_cb_t;

typedef struct mcs_inst_configurations
{
    /// Preferred mtu size
    uint16_t pref_mtu;
    /// Content Control ID
    uint8_t ccid;
    /// Opcode supp bf
    uint32_t opcodes_supp_bf;
    /// Playing order supp
    uint32_t playing_order_supp_bf;
    /// OTS transfer lid
    uint8_t transfer_lid;
} mcs_inst_cfg_t;

typedef struct mcs_cfg
{
    /// GMCS init cfg
    const mcs_inst_cfg_t *p_gmcs_inst_cfg;
    // Number of MCS inst supp
    uint8_t nb_mcs_inst_supp;
} mcs_init_cfg_t;

typedef struct mcs_search_cp_operation
{
    /// Search CP item length
    uint8_t item_len;
    /// Search CP item type
    uint8_t type;
    /// Search CP params
    uint8_t param[GAF_ARRAY_EMPTY];
} mcs_mcp_search_op_t;

/*FUCNTION DECLARATION*/
/**
 * @brief Media control server initilization
 *
 * @param  mcs_init_cfg
 *                     Initilization confiuration
 * @param  mcs_evt_cb  Event callbacks
 *
 * @return int         status
 */
int mcs_init(const mcs_init_cfg_t *mcs_init_cfg, const mcs_evt_cb_t *mcs_evt_cb);

/**
 * @brief Media control server deinitilization
 *
 * @return int         status
 */
int mcs_deinit(void);

/**
 * @brief Media control server add mcs instant
 *
 * @param  p_mcs_inst_cfg
 *                     Initilization confiuration
 * @param  p_media_lid_ret
 *                     Media local index return
 *
 * @return int         status
 */
int mcs_add_mcs_instant(const mcs_inst_cfg_t *p_mcs_inst_cfg, uint8_t *p_media_lid_ret);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Media control server restore client configuration bitfield
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  cli_cfg_bf  Client configuration bitfield
 *
 * @return int         status
 */
int mcs_restore_cli_cfg_cache(uint8_t con_lid, uint8_t media_lid, uint32_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Media control server confirm control request
 *
 * @param  status      Confirm status
 * @param  media_lid   Media local index
 * @param  result      Result
 * @param  action      Media action @see enum mcs_action
 * @param  track_pos   Track position
 * @param  seeking_speed
 *                     Seeking speed
 *
 * @return int         status
 */
int mcs_control_req_cfm(uint16_t status, uint8_t media_lid, uint8_t result, uint8_t action,
                        int32_t track_pos, int8_t seeking_speed);

/**
 * @brief Media control server confirm set value request
 *
 * @param  status      Confirm status
 * @param  media_lid   Media local index
 * @param  val         Value to set
 *
 * @return int         status
 */
int mcs_val_set_req_cfm(uint16_t status, uint8_t media_lid, uint32_t val);

/**
 * @brief Media control server confirm get value request
 *
 * @param  status      Confirm status
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  p_buf_prepare
 *                     Buffer prepared to send response pointer
 * @param  buf_len     Buffer prepared to send response length
 *
 * @return int         status
 */
int mcs_val_get_req_cfm(uint16_t status, uint8_t con_lid, uint8_t media_lid,
                        const uint8_t *p_buf_prepare, uint16_t buf_len);

/**
 * @brief Media control server set character value
 *
 * @param  media_lid   Media local index
 * @param  char_type   Character type
 * @param  p_val       Character type
 * @param  val_len     Value length
 *
 * @return int         status
 */
int mcs_set_character_val(uint8_t media_lid, uint8_t char_type, const uint8_t *p_val, uint8_t val_len);

/**
 * @brief Media control server track changed update
 *
 * @param  media_lid   Media local index
 * @param  track_dur   Track duration
 * @param  p_segments_obj_id
 *                     Track segment object ID value pointer
 * @param  p_current_obj_id
 *                     Current Track object ID value pointer
 * @param  p_next_obj_id
 *                     Next Track object ID value pointer
 * @param  p_group_obj_id
 *                     Current Group object ID value pointer
 * @param  p_parent_obj_id
 *                     Parent Group object ID value pointer
 * @param  title_len   Track Title length
 * @param  p_title     Track Title value length
 *
 * @return int         status
 */
int mcs_track_change_update(uint8_t media_lid, int32_t track_dur,
                            const mcs_object_id_t *p_segments_obj_id, const mcs_object_id_t *p_current_obj_id,
                            const mcs_object_id_t *p_next_obj_id, const mcs_object_id_t *p_group_obj_id,
                            const mcs_object_id_t *p_parent_obj_id,
                            uint8_t title_len, const uint8_t *p_title);

/**
 * @brief Media control server media action (play/pause/stop...)
 *
 * @param  media_lid   Media local index
 * @param  action      Media action @see enum mcs_action
 * @param  track_pos   Track position
 * @param  seeking_speed
 *                     Seeking speed
 *
 * @return int         status
 */
int mcs_media_action(uint8_t media_lid, uint8_t action, int32_t track_pos, int8_t seeking_speed);

/**
 * @brief Media control server confirm control request
 *
 * @param  status      Confirm status
 * @param  media_lid   Media local index
 * @param  p_obj_id    Search object ID value found pointer
 *
 * @return int         status
 */
int mcs_search_req_cfm(uint16_t status, uint8_t media_lid, const mcs_object_id_t *p_obj_id);
#endif /// __ACC_MCS__