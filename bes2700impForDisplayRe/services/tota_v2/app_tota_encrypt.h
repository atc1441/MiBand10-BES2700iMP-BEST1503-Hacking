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

#ifndef __APP_TOTA_ENCRYPT_H__
#define __APP_TOTA_ENCRYPT_H__


/* enable or disable TOTA_ENCODE */
#define TOTA_ENCODE             1


/* unit: byte */
#define HASH_KEY_SIZE           32
#define ENCRYPT_KEY_SIZE        16
#define RANDOM_KEY_SIZE         16
#define MAX_RANDOM_KEY_SIZE     128


// void test_tota_encode_decode();
void tota_set_encrypt_key_from_hash_key(uint8_t * hash_key);
uint16_t tota_encrypt(uint8_t * out, uint8_t * in, uint16_t inLen);
uint16_t tota_decrypt(uint8_t * out, uint8_t * in, uint16_t inLen);

#endif