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
#ifndef __PRF_CSISC__
#define __PRF_CSISC__
#include "bluetooth.h"

#include "gaf_prf.h"
#include "gaf_log.h"

#include "csi_common.h"

/// TODO: move it to prf layer
#define PRF_CSISC_ENABLE            (1)

#define CSISC_CONNECTION_MAX        (GAF_CONNECTION_MAX)

typedef struct csisc_prf_svc_info csisc_prf_svc_t;

/// Callback for csisc bond data
typedef void (*csisc_cb_bond_data_evt)(uint8_t con_lid, uint8_t set_lid,
                                       const csisc_prf_svc_t *param);
/// Callback for csisc discovery done
typedef void (*csisc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for csisc op set lock complete
typedef void (*csisc_cb_set_lock_cmp_evt)(uint8_t con_lid, uint8_t set_lid, uint16_t err_code);
/// Callback for csisc cccd write complete
typedef void (*csisc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t set_lid, uint8_t char_type,
                                         uint16_t err_code);
/// Callback for csisc cccd cfg value received or read
typedef void (*csisc_cb_cfg_evt)(uint8_t con_lid, uint8_t set_lid, uint8_t char_type, bool enabled,
                                 uint16_t err_code);
/// Callback for csisc value ntf or read
typedef void (*csisc_cb_value_evt)(uint8_t con_lid, uint8_t set_lid, uint8_t char_type,
                                   const uint8_t *data, uint16_t len, uint16_t err_code);
/// Callback for csisc sirk decrypt get ltk
typedef void (*csisc_cb_get_ltk)(uint8_t con_lid, uint16_t *status, uint8_t *ltk);
/// Callback for csisc rsi resolved
typedef void (*csisc_cb_rsi_resolved)(uint8_t con_lid, uint8_t set_lid, uint16_t err_code,
                                      const uint8_t *p_rsi);
/// Callback function called when prf status event generated
typedef gatt_prf_status_event_callback csisc_cb_prf_status_evt;

/*Structure*/
typedef struct csisc_evt_cb
{
    /// Callback function called when client configuration for CSIS has been updated
    csisc_cb_bond_data_evt cb_bond_data;
    /// Callback function called when CSIS is discovered or failed
    csisc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when a set lock val cmd is complete with status
    csisc_cb_set_lock_cmp_evt cb_set_lock_val_cmp;
    /// Callback for csisc gatt cmd complete
    csisc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when a cfg of cccd is received
    csisc_cb_cfg_evt cb_cfg_value;
    /// Callback function called when a value is received with status
    csisc_cb_value_evt cb_char_value;
    /// Callback function called when a sirk need to be decrpted using ltk
    csisc_cb_get_ltk cb_ltk;
    /// Callback function called when a rsi is resolved
    csisc_cb_rsi_resolved cb_rsi_resolved;
    /// Callback function called when prf status event generated
    csisc_cb_prf_status_evt cb_prf_status_event;
} csisc_evt_cb_t;

typedef struct csisc_cfg
{
    /// Number of SIRK values that can be stored
    uint8_t nb_sirk;
} csisc_cfg_t;

struct csisc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
};

/*FUCNTIONS DECLARATION*/
/**
 * @brief CSIS client initiliazation
 *
 * @param  p_init_cfg  Initilization configuration
 * @param  p_cb        Event callbacks
 *
 * @return int         status
 */
int csisc_init(const csisc_cfg_t *p_init_cfg, const csisc_evt_cb_t *p_cb);

/**
 * @brief CSIS client deinitilization
 *
 * @return int         status
 */
int csisc_deinit(void);

/**
 * @brief CSIS client csis service discovery
 *
 * @param  con_lid     Connecton local index
 *
 * @return int         status
 */
int csisc_service_discovery(uint8_t con_lid);

/**
 * @brief CSIS client character value read
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  char_type   Character type
 *
 * @return int         status
 */
int csisc_character_value_read(uint8_t con_lid, uint8_t set_lid, uint8_t char_type);

/**
 * @brief CSIS client character cccd write
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  char_type   Character type
 * @param  enable_ntf  Enable or disable notify
 *
 * @return int         status
 */
int csisc_character_cccd_write(uint8_t con_lid, uint8_t set_lid, uint8_t char_type,
                               bool enable_ntf);

/**
 * @brief CSIS client set lock value
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  lock        lock value to write
 *
 * @return int         status
 */
int csisc_set_lock_value(uint8_t con_lid, uint8_t set_lid, bool lock);

/**
 * @brief CSIS client sirk decryption
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  p_enc_sirk  Encrypted SIRK to be decrypted
 *
 * @return int         status
 */
int csisc_sirk_decryption_function(uint8_t con_lid, uint8_t set_lid, const uint8_t *p_enc_sirk);

/**
 * @brief CSIS client RSI resolve use sirk provided
 *
 * @param  dummy       Dummy contain any value you want and return with callback
 * @param  p_sirk      SIRK in plain text to resolve RSI
 * @param  p_rsi       RSI value
 *
 * @return int         status
 */
int csisc_rsi_resolve_function(uint8_t dummy, const uint8_t *p_sirk, const uint8_t *p_rsi);

/**
 * @brief CSIS client RSI resolve use set lid specific SIRK
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  p_rsi       RSI value
 *
 * @return int         status
 */
int csisc_rsi_resolve_use_set_sirk(uint8_t con_lid, uint8_t set_lid, const uint8_t *p_rsi);

/**
 * @brief CSIS client get set member size
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  size        Size to return
 *
 * @return int         status
 */
int csisc_get_set_member_size(uint8_t con_lid, uint8_t set_lid, uint8_t *size);

/**
 * @brief CSIS client get set member lock
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  lock        lock to return
 *
 * @return int         status
 */
int csisc_get_set_member_lock(uint8_t con_lid, uint8_t set_lid, bool *lock);

/**
 * @brief CSIS client get set member rank
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  rank        rank to return
 *
 * @return int         status
 */
int csisc_get_set_member_rank(uint8_t con_lid, uint8_t set_lid, uint8_t *rank);

/**
 * @brief CSIS client get set member SIRK
 *
 * @param  con_lid     Connecton local index
 * @param  set_lid     Set local index
 * @param  sirk        SIRK to return
 *
 * @return int         status
 */
int csisc_get_set_member_sirk(uint8_t con_lid, uint8_t set_lid, uint8_t *sirk);
#endif /// __PRF_CSISC__