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
#ifndef __PRF_CSIS__
#define __PRF_CSIS__
#include "bluetooth.h"

#include "gaf_log.h"

#include "csi_common.h"

/// TODO:move this to ble prf
#define PRF_CSIS_ENABLE                             (1)

#define CSIS_CONNECTION_MAX                         (GAF_CONNECTION_MAX)

#define CSIS_SET_LID_TO_SERVICE_INST_ID(set_lid)    (set_lid)

/// Position/length of fields in SIRK Encryption/Decryption buffer
enum csis_sirk_pos_len
{
    /// Position of Type field
    CSIS_SIRK_POS_TYPE = 0,
    /// Position of Value field
    CSIS_SIRK_POS_VALUE = 1,

    /// Length of Type field
    CSIS_SIRK_LEN_TYPE = 1,
    /// Length of VALUE field
    CSIS_SIRK_LEN_VALUE = 16,

    /// Length of SIRK characteristic value
    CSIS_SIRK_LEN = CSIS_SIRK_LEN_TYPE + CSIS_SIRK_LEN_VALUE,

    /// Position of LTK
    CSIS_SIRK_POS_LTK = CSIS_SIRK_LEN,
};

/// Length of RSI value
#define CSIS_RSI_LEN                    (6)
/// Length of Coordinated Set Size characteristic value
#define CSIS_SIZE_LEN                   (1)
/// Length of Set Member Lock characteristic value
#define CSIS_LOCK_LEN                   (1)
/// Length of Set Member Rank characteristic value
#define CSIS_RANK_LEN                   (1)
/// Length of Prand value
#define CSIS_PRAND_LEN                  (3)
/// Length of Hash value
#define CSIS_HASH_LEN                   (3)
/// Position of Hash value in RSI value
#define CSIS_HASH_POS                   (0)
/// Position of Prand value
#define CSIS_PRAND_POS                  (3)
/// Invalid local index
#define CSIS_INVALID_LID                (0xFF)
/// Minimum Size value
#define CSIS_SIZE_MIN                   (2)
/// S1 M value length ("SIRKenc")
#define CSIS_S1_M_LEN                   (7)
/// K1 P value length ("csis")
#define CSIS_K1_P_LEN                   (4)
/// K1 Salt length
#define CSIS_K1_SALT_LEN                (16)
/// K1 length
#define CSIS_K1_LEN                     (16)

/// CSISM callback for read char
typedef int (*csis_cb_get_char_value)(uint8_t con_lid, uint8_t char_type,
                                      uint8_t *char_value_buf, uint8_t *buf_len);
/// CSISM callback for write char
typedef int (*csis_cb_set_char_value)(uint8_t con_lid, uint8_t char_type,
                                      const uint8_t *char_value_buf, uint8_t buf_len);
/// CSISM callback for val notify sent out
typedef void (*csis_cb_val_ntf_sent)(uint8_t con_lid, uint8_t char_type);
/// CSISM callback for write cccd
typedef void (*csis_cb_bond_data)(uint8_t con_lid, uint8_t char_type, uint8_t cli_cfg_bf);
/// CSISM callback for rsi generated
typedef int (*csis_cb_rsi_value)(uint16_t err_code, uint8_t *rsi);
/// CSISM callback for encrypted sirk generated
typedef int (*csis_cb_enc_sirk_value)(uint16_t dummy, uint8_t err_code, uint8_t *encrypted_sirk);
/// CSISM callback for encrypted key generated
typedef void (*csis_cb_enc_key_value)(uint8_t con_lid, const uint8_t *p_encrypted_key);
/// CSISM callback for service disconnect
typedef int (*csis_cb_disconnect)(uint8_t con_lid);

typedef struct
{
    /// value read
    csis_cb_get_char_value cb_value_read;
    /// value write
    csis_cb_set_char_value cb_value_write;
    /// value notify sent
    csis_cb_val_ntf_sent cb_val_ntf_sent;
    /// cccd write
    csis_cb_bond_data cb_bond_data;
    /// RSI Generated
    csis_cb_rsi_value cb_rsi_generated;
    /// Encrypted sirk generated
    csis_cb_enc_sirk_value cb_enc_sirk_generated;
    /// Encrypted key generated
    csis_cb_enc_key_value cb_enc_key_val;
    /// csism disconnect
    csis_cb_disconnect cb_disconnect;
} csism_inst_callback;

/**
 * @brief CSIS server add set member
 *
 * @param  p_cb        Event callbacks
 * @param  pref_mtu    Preferred MTU size
 * @param  set_lid_ret Set local index return
 *
 * @return int         status
 */
int csis_add_set_member(const csism_inst_callback *p_cb, uint16_t pref_mtu, uint8_t *set_lid_ret);

/**
 * @brief CSIS server delete set member
 *
 * @param  set_lid     Set local index
 *
 * @return int         status
 */
int csis_del_set_member(uint8_t set_lid);

#if (GAF_USE_CACHE_GATT_CCCD)
/**
 * @brief CSIS server restore client configuration bitfield
 *
 * @param  con_lid     Connection local index
 * @param  set_lid     Set local index
 * @param  cli_cfg_bf  Client configuration bitfield
 *
 * @return int         status
 */
int csis_restore_cli_cfg_cache(uint8_t con_lid, uint8_t set_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

/**
 * @brief CSIS server generate encrypted SIRK
 *
 * @param  set_lid     Set local index
 * @param  dummy       Dummy return to upper layer with callback
 * @param  p_sirk_plain
 *                     SIRK value in plain text pointer
 * @param  p_enc_key   Encryption Key value pointer
 *
 * @return int         status
 */
int csis_generate_encrypted_sirk(uint8_t set_lid, uint16_t dummy, const uint8_t *p_sirk_plain,
                                 const uint8_t *p_enc_key);

/**
 * @brief CSIS server generate RSI using SIRK
 *
 * @param  set_lid     Set local index
 * @param  p_sirk_plain
 *                     SIRK value in ecnrypted pointer
 *
 * @return int         status
 */
int csis_generate_rsi(uint8_t set_lid, uint8_t *p_sirk_plain);

/**
 * @brief CSIS server send character value notify to all connected devices
 *
 * @param  set_lid     Set local index
 * @param  char_type   Character type
 * @param  value       Character value to set pointer
 * @param  value_len   Character value length to set
 *
 * @return int         status
 */
int csis_send_char_ntf_to_all(uint8_t set_lid, uint8_t char_type, const uint8_t *value, uint8_t value_len);

/**
 * @brief CSIS server send character value notify to specific connected devices
 *
 * @param  set_lid     Set local index
 * @param  con_lid     Connection local index
 * @param  char_type   Character type
 * @param  value       Character value to set pointer
 * @param  value_len   Character value length to set
 *
 * @return int         status
 */
int csis_send_char_ntf_to_specific(uint8_t set_lid, uint8_t con_lid, uint8_t char_type, const uint8_t *value, uint8_t value_len);
#endif /// __PRF_CSIS__