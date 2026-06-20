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
#ifndef __ACC_TBS__
#define __ACC_TBS__

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "tb_common.h"

#define TBS_CONNECTION_MAX          (GAF_CONNECTION_MAX)

#define TBS_GENERIC_BEARER_LID      (0)

/// Callback for tbs bond data
typedef void (*tbs_cb_bond_data_evt)(uint8_t bearer_lid, uint8_t con_lid, uint8_t char_type,
                                     uint16_t cli_cfg);
/// Callback function called when Signal Report intvl characteristic value has been updated
typedef void (*tbs_cb_report_intv)(uint8_t bearer_lid, uint8_t con_lid,
                                   uint8_t sign_strength_intv_s);
/// Callback function called to request from upper layer complete or piece of value for one of the following characteristics:
/*
 * - Bearer Provider Name characteristic
 * - Incoming Call Target Bearer URI characteristic
 * - Call Friendly Name characteristic
 */
typedef void (*tbs_cb_val_get_req)(uint8_t bearer_lid, uint8_t call_id, uint8_t con_lid,
                                   uint8_t char_type, uint16_t offset);
/// Callback function called to inform upper layer that a client device has requested creation of an outgoing call
typedef void (*tbs_cb_call_req)(uint8_t bearer_lid, uint8_t con_lid, uint8_t opcode,
                                uint8_t call_id, uint8_t len, const uint8_t *p_val);
/// Callback function called to inform upper layer that a action is complete with status
typedef void (*tbs_cb_call_action_cmp)(uint8_t bearer_lid, uint8_t action, uint16_t status, uint8_t call_id);

/*Structure*/
typedef struct tbs_evt_cb
{
    /// Callback function called when client configuration for (G)TBS has been updated
    tbs_cb_bond_data_evt cb_bond_data;
    /// Callback function called when Signal Report intvl characteristic value has been updated
    tbs_cb_report_intv cb_report_intv;
    /// Callback function called to inform upper layer that a client device has requested creation of an outgoing call
    tbs_cb_call_req cb_call_req;
    /// Callback function called to request from upper layer complete or piece of value
    tbs_cb_val_get_req cb_get_req;
    /// Callback function called to inform upper layer that a action is complete with status
    tbs_cb_call_action_cmp cb_action_cmp;
} tbs_evt_cb_t;

typedef struct tbs_inst_configurations
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Content Control ID
    uint8_t ccid;
    /// Optional Opcode Bitfiled
    uint8_t opt_opcodes_bf;
    /// Bearer UCI length
    uint8_t bearer_uci_len;
    /// Bearer UCI value
    const uint8_t *p_bearer_uci;
} tbs_inst_cfg_t;

typedef struct tbs_cfg
{
    /// Call pool size
    uint8_t call_pool_size;
    /// Call pool uri len
    uint8_t call_pool_uri_len;
    /// uri len max
    uint8_t uri_len_max;
    /// Number of TBS instant
    uint8_t nb_tbs_inst_supp;
    /// TBS configurations
    const tbs_inst_cfg_t *p_gtbs_inst_cfg;
} tbs_init_cfg_t;

/*FUCNTION DECLARATION*/
/**
 * @brief Telephone bearer server initilization
 *
 * @param  tbs_init_cfg
 *                     Initilization configuration
 * @param  tbs_evt_cb  Event callbacks
 *
 * @return int         status
 */
int tbs_init(const tbs_init_cfg_t *tbs_init_cfg, const tbs_evt_cb_t *tbs_evt_cb);

/**
 * @brief Telephone bearer server deinitilization
 *
 * @return int         status
 */
int tbs_deinit(void);

/**
 * @brief Telephone bearer server add tbs instant
 *
 * @param  p_tbs_inst_cfg
 *                     TBS instant configuration
 * @param  p_bearer_lid_ret
 *                     TBS instant local index return
 *
 * @return int         status
 */
int tbs_add_tbs_instant(const tbs_inst_cfg_t *p_tbs_inst_cfg, uint8_t *p_bearer_lid_ret);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief Telephone bearer server restore client configuration bitfield
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  cli_cfg_bf  client configuration bitfield
 *
 * @return int         status
 */
int tbs_restore_cli_cfg_cache(uint8_t con_lid, uint8_t bearer_lid, uint16_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief Telephone bearer server confirm call request
 *
 * @param  result      Control result
 * @param  bearer_lid  Bearer local index
 * @param  friendly_name_len
 *                     Friendly name length
 * @param  p_friendly_name
 *                     Friendly name value pointer
 *
 * @return int         status
 */
int tbs_call_req_cfm(enum tbs_cp_ntf_result result, uint8_t bearer_lid, uint8_t friendly_name_len,
                     const uint8_t *p_friendly_name);

/**
 * @brief Telephone bearer server confirm value get request
 *
 * @param  status      Confirm status
 * @param  bearer_lid  Bearer local index
 * @param  call_id     Caller ID
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 * @param  offset      Value get offset
 * @param  length      Value get length
 * @param  p_val       Value get pointer
 *
 * @return int         status
 */
int tbs_val_get_req_cfm(uint16_t status, uint8_t bearer_lid, uint8_t call_id, uint8_t con_lid,
                        uint8_t char_type, uint16_t offset, uint16_t length, const uint8_t *p_val);

/**
 * @brief Telephone bearer server set call status flag
 *
 * @param  bearer_lid  Bearer local index
 * @param  status_type status type to set
 * @param  enable      Enable or disbale
 *
 * @return int         status
 */
int tbs_set_call_status(uint8_t bearer_lid, uint8_t status_type, bool enable);

/**
 * @brief Telephone bearer server set Character value
 *
 * @param  bearer_lid  Bearer local index
 * @param  char_type   Character type
 * @param  len         Value length
 * @param  p_val       Value pointer
 *
 * @return int         status
 */
int tbs_set_character_val(uint8_t bearer_lid, uint8_t char_type, uint8_t len, const uint8_t *p_val);

/**
 * @brief Telephone bearer server call incoming
 *
 * @param  bearer_lid  Bearer local index
 * @param  uri_len     URI length
 * @param  tgt_uri_len Target URI length
 * @param  friendly_name_len
 *                     Friendly name length
 * @param  p_uri       URI value pointer
 * @param  p_tgt_uri   Target URI value pointer
 * @param  p_friendly_name
 *                     Fiendly name value pointer
 *
 * @return int         status
 */
int tbs_call_incoming(uint8_t bearer_lid, uint8_t uri_len, uint8_t tgt_uri_len,
                      uint8_t friendly_name_len, const uint8_t *p_uri, const uint8_t *p_tgt_uri,
                      const uint8_t *p_friendly_name);

/**
 * @brief Telephone bearer server call outgoing
 *
 * @param  bearer_lid  Bearer local index
 * @param  uri_len     URI length
 * @param  friendly_name_len
 *                     Friendly name length
 * @param  p_uri       URI value pointer
 * @param  p_friendly_name
 *                     Friendly name value pointer
 *
 * @return int         status
 */
int tbs_call_outgoing(uint8_t bearer_lid, uint8_t uri_len, uint8_t friendly_name_len,
                      const uint8_t *p_uri, const uint8_t *p_friendly_name);

/**
 * @brief Telephone bearer server call join
 *
 * @param  bearer_lid  Bearer local index
 * @param  nb_calls    Number if calls
 * @param  p_call_ids  Caller ID list
 *
 * @return int         status
 */
int tbs_call_join(uint8_t bearer_lid, uint8_t nb_calls, const uint8_t *p_call_ids);

/**
 * @brief Telephone bearer server call action
 *
 * @param  bearer_lid  Bearer local index
 * @param  call_id     Caller ID
 * @param  action      Call action @see enum tbs_call_action
 * @param  reason      Reason if needed @see enum tbs_term_reason_pos
 *
 * @return int         status
 */
int tbs_call_action(uint8_t bearer_lid, uint8_t call_id, uint8_t action, uint8_t reason);
#endif /// __ACC_TBS__