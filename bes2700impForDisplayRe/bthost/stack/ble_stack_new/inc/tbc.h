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
#ifndef __ACC_TBC__
#define __ACC_TBC__

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "gaf_log.h"

#include "tb_common.h"

#define TBC_CONNECTION_MAX                  (GAF_CONNECTION_MAX)

#define TBC_CP_NTF_RX_TIMEOUT_DUR_MS        (gaf_prf_get_rx_ntf_ind_timeout_in_ms())

/*ENUMERATIONS*/

/*TYPEDEFS*/
typedef struct tbc_prf_svc_info tbc_prf_svc_t;

/// Callback for tbc bond data
typedef void (*tbc_cb_bond_data_evt)(uint8_t con_lid, uint8_t bearer_lid,
                                     const tbc_prf_svc_t *param);
/// Callback for tbc discovery done
typedef void (*tbc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for tbc call control opcode cmp evt
typedef void (*tbc_cb_ccp_control_cmp_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t op_code,
                                           uint16_t err_code, uint8_t ctrl_result);
/// Callback for tbc gatt cmd complete, @see enum tbc_cmd_code
typedef void (*tbc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type,
                                       uint16_t err_code);
/// Callback for tbc op set val complete
typedef void (*tbc_cb_set_val_cmp_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type,
                                       uint16_t err_code);
/// Callback for tbc cccd cfg value received or read
typedef void (*tbc_cb_cfg_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type, bool enabled,
                               uint16_t err_code);
/// Callback for tbc value ntf or read
typedef void (*tbc_cb_call_state_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id,
                                      uint8_t flags, uint8_t state);
/// Callback for tbc value ntf or read
typedef void (*tbc_cb_call_state_long_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id,
                                           uint8_t flags, uint8_t state, uint8_t uri_len, const uint8_t *p_uri);
/// Callback for tbc value ntf or read
typedef void (*tbc_cb_value_evt)(uint8_t con_lid, uint8_t bearer_lid, uint8_t call_id,
                                 uint8_t char_type, const uint8_t *data, uint16_t len, uint16_t err_code);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback tbc_cb_prf_status_evt;

/*Structure*/
typedef struct tbc_evt_cb
{
    /// Callback function called when client configuration for (G)TBS has been updated
    tbc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when (G)TBS is discovered or failed
    tbc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when call control opcode cmp
    tbc_cb_ccp_control_cmp_evt cb_ccp_ctrl_cmp;
    /// Callback for tbc gatt cmd complete
    tbc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback for tbc op set val complete
    tbc_cb_set_val_cmp_evt cb_set_val_cmp;
    /// Callback function called when a cfg of cccd is received
    tbc_cb_cfg_evt cb_cfg_value;
    /// Callback function called when Call state has been received through Call State
    /// characteristic
    tbc_cb_call_state_evt cb_call_state;
    /// Callback function called when Call state has been received through Bearer List
    /// Current Calls characteristic
    tbc_cb_call_state_long_evt cb_call_state_long;
    /// Callback function called when a value is received with status
    tbc_cb_value_evt cb_char_value;
    /// Callback function called when prf status event generated
    tbc_cb_prf_status_evt cb_prf_status_event;
} tbc_evt_cb_t;

struct tbc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

typedef struct tbc_init_cfg
{
    /// Max support tbs instant found
    uint8_t max_supp_tbs_inst;
    /// CP Write reliable
    bool cp_write_reliable;
    /// TBS discovery with character value read and cccd write
    uint8_t discovery_only_find_char;
    /// Preferred MTU size
    uint16_t pref_mtu;
} tbc_init_cfg_t;

/*FUCNTIONS DECLARATION*/
/**
 * @brief Telephone bearer client initilization
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int tbc_init(const tbc_init_cfg_t *p_init_cfg, const tbc_evt_cb_t *p_cb);

/**
 * @brief Telephone bearer client deinitilization
 *
 * @return int         status
 */
int tbc_deinit(void);

/**
 * @brief Telephone bearer client tbs service discovery with full characters report
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int tbc_service_discovery_full(uint8_t con_lid);

/**
 * @brief Telephone bearer client tbs service discovery with specific characters report
 *
 * @param  con_lid     Connection local index
 * @param  p_char_uuid_list
 *                     Character UUID list
 * @param  char_uuid_list_len
 *                     Character UUID list length
 *
 * @return int         status
 */
int tbc_service_discovery_with_uuid_list(uint8_t con_lid, const uint16_t *p_char_uuid_list,
                                         uint8_t char_uuid_list_len);

/**
 * @brief Telephone bearer client read character value
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int tbc_character_value_read(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type);

/**
 * @brief Telephone bearer client write character cccd
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int tbc_character_cccd_write(uint8_t con_lid, uint8_t bearer_lid, uint8_t char_type,
                             bool enable_ntf);

/**
 * @brief Telephone bearer client set signal strength report interval value
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  signal_strength_intv_s
 *                     Signal strength report interval in second
 *
 * @return int         status
 */
int tbc_ccp_set_signal_report_intvl(uint8_t con_lid, uint8_t bearer_lid,
                                    uint8_t signal_strength_intv_s);

/**
 * @brief Telephone bearer client ccp call outgoing
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  uri_val     URI value pointer
 * @param  uri_len     URI value length
 *
 * @return int         status
 */
int tbc_ccp_call_outgoing(uint8_t con_lid, uint8_t bearer_lid,
                          const uint8_t *uri_val, uint16_t uri_len);

/**
 * @brief Telephone bearer client ccp call join
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  call_id     Number of calls
 * @param  p_call_id_list
 *                     Caller ID list
 *
 * @return int         status
 */
int tbc_ccp_call_join(uint8_t con_lid, uint8_t bearer_lid, uint8_t nb_calls,
                      const uint8_t *p_call_id_list);

/**
 * @brief Telephone bearer client ccp call action (termiante/hold...)
 *
 * @param  con_lid     Connection local index
 * @param  bearer_lid  Bearer local index
 * @param  op_code     Operation code @see enum tbs_opcode
 * @param  call_id     Caller ID
 *
 * @return int         status
 */
int tbc_ccp_call_action(uint8_t con_lid, uint8_t bearer_lid, uint8_t op_code, uint8_t call_id);
#endif /// __ACC_TBC__