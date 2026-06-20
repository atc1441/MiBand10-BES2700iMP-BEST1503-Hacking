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
#ifndef __ATC_CSISC__
#define __ATC_CSISC__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_log.h"

#include "csi_common.h"
#include "csisc.h"

#define CSISC_CONNECTION_MAX            (GAF_CONNECTION_MAX)

/*
typedef int (*csisc_atc_cb_bond_data)(uint8_t con_lid, uint8_t set_lid, const csisc_prf_svc_t *param);

typedef int (*csisc_atc_cb_ltk_get)(uint8_t con_lid, uint8_t *ltk);

typedef int (*csisc_atc_cb_rsi_resolved)(uint8_t con_lid, uint16_t err_code, const uint8_t *p_rsi);

typedef int (*csisc_atc_cb_sirk_decrypted)(uint8_t con_lid, uint16_t err_code, const uint8_t *p_sirk);

typedef int (*csisc_atc_cb_lock_state)(uint8_t con_lid, uint8_t err_code, bool lock);

typedef int (*csisc_atc_cb_set_size)(uint8_t con_lid, uint8_t err_code, uint8_t size);

typedef int (*csisc_atc_cb_set_rank)(uint8_t con_lid, uint8_t err_code, uint8_t rank);

typedef struct
{
    csisc_atc_cb_bond_data cb_bond_data;

    csisc_atc_cb_ltk_get cb_ltk_get;

    csisc_atc_cb_rsi_resolved cb_rsi;

    csisc_atc_cb_sirk_decrypted cb_sirk;

    csisc_atc_cb_lock_state cb_lock;

    csisc_atc_cb_set_size cb_size;

    csisc_atc_cb_set_rank cb_rank;

} csisc_atc_evt_callback_t;
*/

/**
 * @brief ATC Coordinator Initilization
 *
 * @return int         status
 */
int csisc_atc_init(void);

/**
 * @brief ATC Coordinator deinitilization
 *
 * @return int         status
 */
int csisc_atc_deinit(void);

/**
 * @brief ATC Coordinator enable set member by set local index
 *
 * @param  con_lid     Connection local index
 * @param  set_lid     Set local index
 *
 * @return int         status
 */
int csisc_atc_enable_set_member_by_set_lid(uint8_t con_lid, uint8_t set_lid);

/**
 * @brief ATC Coordinator disable set memnber
 *
 * @param  con_lid     Connection local index
 *
 * @return int         status
 */
int csisc_atc_disable_set_member(uint8_t con_lid);

/**
 * @brief ATC Coordinator get set member local index
 *
 * @param  con_lid     Connection local index
 * @param  set_lid     Set local index return
 *
 * @return int         status
 */
int csisc_atc_get_set_member_lid(uint8_t con_lid, uint8_t *set_lid);

/**
 * @brief ATC Coordinator get set member size
 *
 * @param  con_lid     Connection local index
 * @param  size        Size value pinter return
 *
 * @return int         status
 */
int csisc_atc_get_set_member_size(uint8_t con_lid, uint8_t *size);

/**
 * @brief ATC Coordinator get set member lock
 *
 * @param  con_lid     Connection local index
 * @param  lock        Lock value pointer return
 *
 * @return int         status
 */
int csisc_atc_get_set_member_lock(uint8_t con_lid, bool *lock);

/**
 * @brief ATC Coordinator get set member rank
 *
 * @param  con_lid     Connection local index
 * @param  rank        Rank value poniter return
 *
 * @return int         status
 */
int csisc_atc_get_set_member_rank(uint8_t con_lid, uint8_t *rank);
#endif /// __ATC_CSISC__