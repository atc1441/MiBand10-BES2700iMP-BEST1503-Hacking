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
#ifndef __ATC_CSISM__
#define __ATC_CSISM__
#include "gaf_cfg.h"
#include "gaf_log.h"
#include "gatt_service.h"
#include "csis.h"

typedef void (*csism_atc_cb_bond_data)(uint8_t con_lid, uint8_t char_type_written,
                                       uint16_t cli_cfg_bf);

typedef void (*csism_atc_cb_char_val_sent)(uint8_t con_lid, bool is_read_rsp, uint8_t char_type, const uint8_t *p_val_rsp);

typedef void (*csism_atc_cb_ltk_get)(uint8_t con_lid, uint16_t *status, uint8_t *ltk);

typedef void (*csism_atc_cb_rsi_generated)(uint16_t err_code, uint8_t *rsi);

typedef void (*csism_atc_cb_lock_state)(uint8_t con_lid, bool lock);

typedef struct csism_atc_evt_callback
{
    csism_atc_cb_bond_data cb_bond_data;

    csism_atc_cb_char_val_sent cb_val_sent;

    csism_atc_cb_ltk_get cb_ltk_get;

    csism_atc_cb_rsi_generated cb_rsi;

    csism_atc_cb_lock_state cb_lock;

} csism_atc_evt_cb_t;

typedef struct
{
    uint8_t rank;
    uint8_t size;
    uint8_t lock_bf;
    uint8_t sirk[CSIS_SIRK_VAL_LEN];
    bool sirk_encrypt_needed;
    uint16_t pref_mtu;
} csism_atc_init_cfg_t;

/*FUNCTIONS DECLARATION*/
/**
 * @brief ATC set member initilization
 *
 * @param  p_cfg       Initilization configuration
 * @param  p_cb        Event callbacks
 * @param  set_lid_ret Set local index return
 *
 * @return int         status
 */
int csism_atc_init(csism_atc_init_cfg_t *p_cfg, const csism_atc_evt_cb_t *p_cb,
                   uint8_t *set_lid_ret);

/**
 * @brief ATC set member deinitilization
 *
 * @return int         status
 */
int csism_atc_deinit(void);

/**
 * @brief ATC set member restore client configuration
 *
 * @param  con_lid     Connection local index
 * @param  lock        Lock value
 * @param  cli_cfg_bf  Client Configuration Bitfield
 *
 * @return int         status
 */
int csism_atc_restore_cli_cfg_cache(uint8_t con_lid, bool lock, uint8_t cli_cfg_bf);

/**
 * @brief ATC set member set SIRK value
 *
 * @param  sirk        Value
 *
 * @return int         status
 */
int csism_atc_set_sirk(uint8_t *sirk);

/**
 * @brief ATC set member set size value
 *
 * @param  size        Size value
 *
 * @return int         status
 */
int csism_atc_set_size(uint8_t size);

/**
 * @brief ATC set member set rank value
 *
 * @param  rank        Rank value
 *
 * @return int         status
 */
int csism_atc_set_rank(uint8_t rank);

/**
 * @brief ATC set member set lock value
 *
 * @param  con_lid     Connection local index
 * @param  lock        Lock value
 *
 * @return int         status
 */
int csism_atc_set_lock(uint8_t con_lid, bool lock);

/**
 * @brief ATC set member generate RSI value
 *
 * @return int         status
 */
int csism_atc_generate_rsi(void);

#endif /// __ATC_CSISM__
