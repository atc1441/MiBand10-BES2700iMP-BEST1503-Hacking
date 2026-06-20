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

#include "cmsis_os.h"
#include "app_tota_cmd_code.h"
#include "app_tota_encrypt.h"
#include "encrypt/aes.h" // utils/encrypt


static uint8_t key[ENCRYPT_KEY_SIZE] = {0x2b, 0x7e, 0x15, 0x16, 0x28, 0xae, 0xd2, 0xa6, 0xab, 0xf7, 0x15, 0x88, 0x09, 0xcf, 0x4f, 0x3c};
static uint8_t iv[ENCRYPT_KEY_SIZE]  = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f};

void tota_set_encrypt_key_from_hash_key(uint8_t * hash_key)
{
    for (uint8_t i = 0; i < ENCRYPT_KEY_SIZE; i ++)
    {
        key[i] = hash_key[2*i];
    }
    TOTA_LOG_DBG(0,"aes key:");
    DUMP8("%02x ", key, ENCRYPT_KEY_SIZE);
}

uint16_t tota_encrypt(uint8_t * out, uint8_t * in, uint16_t inLen)
{
    //TOTA_LOG_DBG(1,"[%s]:raw data:", __func__);
    //DUMP8("0x%02x, ", in, inLen);
    uint16_t outLen = inLen % ENCRYPT_KEY_SIZE? inLen + ENCRYPT_KEY_SIZE - (inLen % ENCRYPT_KEY_SIZE) : inLen;
    for (int i = inLen; i < outLen; i ++)
    {
        in[i] = 0;
    }
    //TOTA_LOG_DBG(1,"outLen: %d", outLen);
    // DUMP8("0x%02x, ", in, outLen);
    AES128_CBC_encrypt_buffer(out, in, outLen, key, iv);
    //TOTA_LOG_DBG(2,"encrypt data: %u -> %u", inLen, outLen);
    //DUMP8("0x%02x, ", out, outLen);
    return outLen;
}

uint16_t tota_decrypt(uint8_t * out, uint8_t * in, uint16_t inLen)
{
    //TOTA_LOG_DBG(1,"[%s]:raw data:", __func__);
    // DUMP8("0x%02x, ", in, inLen);
    uint16_t outLen = inLen % ENCRYPT_KEY_SIZE? inLen + ENCRYPT_KEY_SIZE - (inLen % ENCRYPT_KEY_SIZE) : inLen;
    for (int i = inLen; i < outLen; i ++)
    {
        in[i] = 0;
    }
    // TOTA_LOG_DBG(0, "make 16s:");
    // DUMP8("0x%02x, ", in, outLen);
    // TOTA_LOG_DBG(1,"outLen: %d", outLen);
    AES128_CBC_decrypt_buffer(out, in, outLen, key, iv);
    //TOTA_LOG_DBG(2,"decrypt data: %u -> %u", inLen, outLen);
    //DUMP8("0x%02x, ", out, outLen);
    //TOTA_LOG_DBG(0,"\n");
    return outLen;
}