
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
#ifndef __BAP_ASCS__
#define __BAP_ASCS__

#include "bluetooth.h"

#include "gaf_prf.h"
#include "gaf_cfg.h"
#include "generic_audio.h"
#include "gaf_log.h"

#include "asc_common.h"

#define ASCS_CONNECTION_MAX                     (GAF_CONNECTION_MAX)

#define ASCS_MAX_SUPP_CIS_NUM                   (GAF_SUPP_MAX_CIS_NUM)

#define ASCS_MAX_SUPP_ASE_NUM_PER_DIRECTION     (7)

/// ASCS callback for read ase char
typedef void (*ascs_cb_get_ase_value_buf)(uint8_t con_lid, uint8_t ase_id,
                                          const uint8_t **p_ase_value_buf, const uint8_t *p_buf_len);
/// ASCS callback for write ase cp char
typedef int (*ascs_cb_set_ase_cp_value)(gatt_server_write_ctx_t *p_wr_ctx, uint8_t con_lid, const uint8_t *char_value_buf, uint8_t buf_len);
/// ASCS callback for bond data
typedef void (*ascs_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint16_t cli_cfg_bf);
/// ASCS callback for character value notify sent, ase_id < MIN means ase cp
typedef void (*ascs_cb_val_ntf_sent)(uint8_t con_lid, uint8_t ase_id);
/// ASCS callback for service disconnect
typedef void (*ascs_cb_disconnect)(uint8_t con_lid);

typedef struct ascs_upper_callback
{
    /// value read
    ascs_cb_get_ase_value_buf cb_ase_value_buf_get;
    /// value write
    ascs_cb_set_ase_cp_value cb_ase_cp_write;
    /// cccd read
    ascs_cb_bond_data cb_bond_data;
    /// val ntf sent
    ascs_cb_val_ntf_sent cb_val_ntf_sent;
    /// ascs disconnect
    ascs_cb_disconnect cb_disconnect;
} ascs_upper_cb_t;

typedef struct ascs_init_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Sink ASE number supported |
    /// Source ASE number supported
    uint8_t num_ase_supp_bf;
} ascs_init_cfg_t;

/*FUNCTIONS*/
#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief ASCS restore CCCD value
 *
 * @param  con_lid     Connection local index
 * @param  cli_cfg_bf  Client Configuration Bitfield
 *
 * @return int         status
 */
int ascs_restore_cli_cfg_cache(uint8_t con_lid, uint16_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

#endif /// __BAP_ASCS__