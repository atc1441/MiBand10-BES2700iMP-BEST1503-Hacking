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
#ifndef __CSIS_CRYPTO__
#define __CSIS_CRYPTO__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_log.h"

typedef void (*csis_crypto_encrypted_sirk_cb)(uint8_t set_lid, uint16_t dummy, uint16_t err_code,
                                              const uint8_t *sirk_encrypted);

typedef csis_crypto_encrypted_sirk_cb csis_crypto_decrypted_sirk_cb;

typedef void (*csis_crypto_generate_rsi_cb)(uint8_t set_lid, uint16_t err_code,
                                            const uint8_t *rsi_generated);

typedef csis_crypto_generate_rsi_cb csis_crypto_resolve_rsi_cb;

/**
 * @brief CSIS crypto encrypt SIRK
 *
 * @param  set_lid     Set local index
 * @param  dummy       Dummy from upper layer and return with callback
 * @param  p_sirk      SIRK value pointer
 * @param  p_enc_key   Encryption Key value pointer
 * @param  callback    Complete Callback
 *
 * @return int         status
 */
int csis_crypto_encrypt_sirk(uint8_t set_lid, uint16_t dummy, const uint8_t *p_sirk,
                             const uint8_t *p_enc_key, csis_crypto_encrypted_sirk_cb callback);

/**
 * @brief
 *
 * @param  set_lid     Set local index
 * @param  dummy       Dummy from upper layer and return with callback
 * @param  p_enc_sirk  Encrypted SIRK value pointer
 * @param  p_enc_key   Encryption Key value pointer
 * @param  callback    Complete Callback
 *
 * @return int         status
 */
int csis_crypto_decrypt_encrypted_sirk(uint8_t set_lid, uint16_t dummy, const uint8_t *p_enc_sirk,
                                       const uint8_t *p_enc_key, csis_crypto_decrypted_sirk_cb callback);

/**
 * @brief
 *
 * @param  set_lid     Set local index
 * @param  p_sirk      SIRK value pointer
 * @param  callback    Complete Callback
 *
 * @return int         status
 */
int csis_crypto_generate_rsi(uint8_t set_lid, const uint8_t *p_sirk,
                             csis_crypto_generate_rsi_cb callback);

/**
 * @brief
 *
 * @param  set_lid     Set local index
 * @param  p_sirk      SIRK value pointer
 * @param  p_rsi       RSI value pointer
 * @param  callback    Complete Callback
 *
 * @return int         status
 */
int csis_crypto_resolve_rsi(uint8_t set_lid, const uint8_t *p_sirk, const uint8_t *p_rsi,
                            csis_crypto_resolve_rsi_cb callback);
#endif /// __CSIS_CRYPTO__