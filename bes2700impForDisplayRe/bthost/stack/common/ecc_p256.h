/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __ECC_P256_H__
#define __ECC_P256_H__
#include <stdint.h>
#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ECC_PUBLICKEY_GENERATION 0x01
#define ECC_DHKEY_GENERATION     0x02

struct ecc_result_ind
{
    uint8_t key_res_x[32];
    uint8_t key_res_y[32];
};

extern const uint8_t DebugE256SecretKey[32];

/**
 ****************************************************************************************
 * @brief Initialize Elliptic Curve algorithm
 *
 * @param[in] init_type  Type of initialization (@see enum rwip_init_type)
 ****************************************************************************************
 */
void ecc_init(uint8_t init_type);

/**
 ****************************************************************************************
 * @brief Generate a Secret Key compliant with ECC P256 algorithm
 *
 * If key is forced, just check its validity
 *
 * @param[out] secret_key Private key - MSB First
 * @param[in]  forced True if provided key is forced, else generate it.
 ****************************************************************************************
 */
void ecc_gen_new_secret_key_256(uint8_t* secret_key, bool forced);

/**
 ****************************************************************************************
 * @brief Calculate DHkey
 *
 * @param[in]  secret_key  Private key             - MSB First
 * @param[in]  pub_key_x   Public key x coordinate - LSB First
 * @param[in]  pub_key_y   Public key y coordinate - LSB First
 * @param[out] out_dh_key  DHkey calculated - LSB First
 ****************************************************************************************
 */
uint8_t ecc_p256_gen_dh_key(const uint8_t* secret_key,
                            const uint8_t* public_key_x, const uint8_t* public_key_y,
                            uint8_t* out_dh_key);

/**
 ****************************************************************************************
 * @brief Retrieve debug private and public keys
 *
 * @param[out] secret_key Private key             - MSB First
 * @param[out] pub_key_x  Public key x coordinate - LSB First
 * @param[out] pub_key_y  Public key y coordinate - LSB First
 ****************************************************************************************
 */
void ecc_get_debug_Keys(uint8_t*secret_key, uint8_t* pub_key_x, uint8_t* pub_key_y);
void ecc_gen_new_public_key_256(uint8_t* secret_key,uint8_t* out_public_key);

#ifdef __cplusplus
}
#endif
#endif /* __ECC_P256_H__ */
