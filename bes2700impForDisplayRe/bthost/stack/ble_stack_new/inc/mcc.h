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
#ifndef __ACC_MCC__
#define __ACC_MCC__

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "mc_common.h"

#define MCC_CONNECTION_MAX                  (GAF_CONNECTION_MAX)

#define MCC_CP_NTF_RX_TIMEOUT_DUR_MS        (gaf_prf_get_rx_ntf_ind_timeout_in_ms())

/*ENUMERATIONS*/

/*TYPEDEFS*/
typedef struct mcc_prf_svc_info mcc_prf_svc_t;

/// Callback for mcc bond data
typedef void (*mcc_cb_bond_data_evt)(uint8_t con_lid, uint8_t media_lid,
                                     const mcc_prf_svc_t *param);
/// Callback for mcc discovery done
typedef void (*mcc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for mcc include service found
typedef void (*mcc_cb_include_service_found_evt)(uint8_t con_lid, uint8_t media_lid, uint16_t uuid, uint8_t transfer_lid);
/// Callback for mcc op control complete
typedef void (*mcc_cb_mcp_control_cmp_evt)(uint8_t con_lid, uint8_t media_lid, uint8_t op_code,
                                           uint16_t err_code, uint8_t ctrl_result);
/// Callback for mcc op search complete
typedef void (*mcc_cb_mcp_search_cmp_evt)(uint8_t con_lid, uint8_t media_lid, uint16_t err_code,
                                          uint8_t search_result);
/// Callback for mcc gatt cmd complete, @see enum mcc_cmd_code
typedef void (*mcc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                       uint16_t err_code);
/// Callback for mcc op set val complete
typedef void (*mcc_cb_set_val_cmp_evt)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                       uint16_t err_code);
/// Callback for mcc cccd cfg value received or read
typedef void (*mcc_cb_cfg_evt)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, bool enabled,
                               uint16_t err_code);
/// Callback for mcc value ntf or read
typedef void (*mcc_cb_value_evt)(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                                 const uint8_t *data, uint16_t len, uint16_t err_code);
/// Callback for mcc prf event
typedef gatt_prf_status_event_callback mcc_cb_prf_status_evt;

/*Structure*/
typedef struct mcc_evt_cb
{
    /// Callback function called when client configuration for (G)MCS has been updated
    mcc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when include service for (G)MCS has been found
    mcc_cb_include_service_found_evt cb_inc_svc_found;
    /// Callback function called when (G)MCS is discovered or failed
    mcc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when a control cmd is complete with status
    mcc_cb_mcp_control_cmp_evt cb_mcp_ctrl_cmp;
    /// Callback function called when a set val cmd is complete with status
    mcc_cb_set_val_cmp_evt cb_set_val_cmp;
    /// Callback function called when a search cmd is complete with status
    mcc_cb_mcp_search_cmp_evt cb_mcp_search_cmp;
    /// Callback for mcc gatt cmd complete
    mcc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when a cfg of cccd is received
    mcc_cb_cfg_evt cb_cfg_value;
    /// Callback function called when a value is received with status
    mcc_cb_value_evt cb_char_value;
    /// Callback function called when prf status event generated
    mcc_cb_prf_status_evt cb_prf_status_event;

} mcc_evt_cb_t;

typedef struct mcc_init_cfg
{
    /// Max support mcs instant found
    uint8_t max_supp_mcs_inst;
    /// CP Write reliable
    bool cp_write_reliable;
    /// Discovery with on character value read and cccd set
    bool discovery_only_find_char;
    /// Preferred MTU size
    uint16_t pref_mtu;
} mcc_init_cfg_t;

struct mcc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

typedef struct mcc_search_cp_operation
{
    /// Search CP item length
    uint8_t item_len;
    /// Search CP item type
    uint8_t type;
    /// Search CP params
    uint8_t param[GAF_ARRAY_EMPTY];
} mcc_mcp_search_op_t;

/*FUCNTION DECLARATION*/
/**
 * @brief Media control client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int mcc_init(const mcc_init_cfg_t *p_init_cfg, const mcc_evt_cb_t *p_cb);

/**
 * @brief Media control client deinitilization
 *
 * @return int         status
 */
int mcc_deinit(void);

/**
 * @brief Media control client MCS service discovery with all character report
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int mcc_service_discovery_full(uint8_t con_lid);

/**
 * @brief Media control client MCS service discovery with specific Characters report
 *
 * @param  con_lid     Connection local index
 * @param  p_char_uuid_list
 *                     UUID list
 * @param  char_uuid_list_len
 *                     UUID list length
 *
 * @return int         status
 */
int mcc_service_discovery_with_uuid_list(uint8_t con_lid, const uint16_t *p_char_uuid_list,
                                         uint8_t char_uuid_list_len);

/**
 * @brief Media control client read Character value
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int mcc_character_value_read(uint8_t con_lid, uint8_t media_lid, uint8_t char_type);

/**
 * @brief Media control client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int mcc_character_cccd_write(uint8_t con_lid, uint8_t media_lid, uint8_t char_type, bool enable_ntf);

/**
 * @brief Media control client write character value
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  char_type   Character type
 * @param  p_val       Value pointer
 * @param  val_len     Value length
 *
 * @return int         status
 */
int mcc_character_val_write(uint8_t con_lid, uint8_t media_lid, uint8_t char_type,
                            const uint8_t *p_val, uint8_t val_len);

/**
 * @brief Media control client MCP control
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  op_code     Operation code
 * @param  val         Value pointer
 *
 * @return int         status
 */
int mcc_mcp_control(uint8_t con_lid, uint8_t media_lid, uint8_t op_code, uint32_t val);

/**
 * @brief Media control client MCP search items
 *
 * @param  con_lid     Connection local index
 * @param  media_lid   Media local index
 * @param  p_search_op Search operation paramters
 * @param  serach_op_len
 *                     Search operation paramters length
 *
 * @return int         status
 */
int mcc_mcp_search_items(uint8_t con_lid, uint8_t media_lid,
                         mcc_mcp_search_op_t *p_search_op, uint8_t serach_op_len);

#endif /// __ACC_MCC__