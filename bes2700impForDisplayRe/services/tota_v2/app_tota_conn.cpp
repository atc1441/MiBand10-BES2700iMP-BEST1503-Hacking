/********************************************************************************
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
 *******************************************************************************/
#include "app_tws_ibrt_cmd_handler.h"
#include "app_tws_ctrl_thread.h"
#include "app_tota_conn.h"
#include "app_tota_cmd_code.h"
#include "app_tota.h"
#include "app_tota_common.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_encrypt.h"
#include "stdlib.h"
#include "string.h"
#include "sha256.h"
#include "hal_norflash.h"

static bool _is_tota_initiate = false;

typedef struct {
    uint8_t     length;
    uint8_t     random_key[MAX_RANDOM_KEY_SIZE];
}TOTA_CONN_STRUCT_T;

typedef TOTA_CONN_STRUCT_T random_key_t;

typedef struct{
    /* local & peer random list */
    random_key_t    random_a;
    random_key_t    random_b;
    /* hash key generate from (random_a, random_b) */
    uint8_t         hash_key[HASH_KEY_SIZE];
    /* encrypt key generate from hash key */
    uint8_t         encrypt_key[ENCRYPT_KEY_SIZE];

    /* may be a timer to */
}TOTA_CONN_PARAMETER_T;

static TOTA_CONN_PARAMETER_T keyManager;

/*
**  @
*/
static void _tota_conn_and_update_key_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);

static uint8_t _generate_random_algorithm(uint8_t id, uint8_t a, uint8_t b);

/* generate key */
static void _generate_random_key();

/* calculate key */
static void _calculate_encrypt_key();

void app_tota_sync_encode_status_handle(uint8_t *key, uint16_t length)
{
    TOTA_LOG_DBG(0, "app_tota_sync_encode_status_handle");
    tota_set_encrypt_key_from_hash_key(key);
    tota_set_connect_status(TOTA_SHAKE_HANDED);
    TOTA_LOG_DBG(0, "get remote conn para");
}


static uint8_t _generate_random_algorithm(uint8_t id, uint8_t a, uint8_t b)
{
    uint8_t op = id % 6;
    switch (op)
    {
        case 0:
            return a + b;
        case 1:
            return a * b;
        case 2:
            return a & b;
        case 3:
            return a | b;
        case 4:
            return a ^ b;
        case 5:
            return a*a + b*b;
        default:
            return 0;
    }
}

static void _generate_random_key()
{
    uint8_t random_length = rand() % (MAX_RANDOM_KEY_SIZE + 1);
    if ( random_length < 16 )
    {
        random_length += 16;
    }
    keyManager.random_a.length = random_length;
    uint32_t * prandom = (uint32_t *)keyManager.random_a.random_key;
    for ( uint8_t i = 0; i < (random_length+3)/4; i ++  )
    {
        prandom[i] = rand();
    }
    TOTA_LOG_DBG(1,"generate random list, len=0x%x", random_length);
    TOTA_LOG_DUMP("%02x ", keyManager.random_a.random_key, random_length);
}

static void _calculate_encrypt_key()
{
    uint8_t random_key[MAX_RANDOM_KEY_SIZE];
    random_key_t * prandom_a = &keyManager.random_a;
    random_key_t * prandom_b = &keyManager.random_b;

    /* align random list */
    if ( prandom_a->length > prandom_b->length )
    {
        memset(prandom_b->random_key + prandom_b->length, 0, prandom_a->length - prandom_b->length);
        prandom_b->length = prandom_a->length;
    }
    else if ( prandom_a->length < prandom_b->length )
    {
        memset(prandom_a->random_key + prandom_a->length, 0, prandom_b->length - prandom_a->length);
        prandom_a->length = prandom_b->length;
    }
    else
    {
        TOTA_LOG_DBG(0, "random list is the same size");
    }
    TOTA_LOG_DBG(1,"random list a, len=0x%x", prandom_a->length);
    TOTA_LOG_DUMP("%02x ", keyManager.random_a.random_key, prandom_a->length);
    TOTA_LOG_DBG(1,"random list b, len=0x%x", prandom_b->length);
    TOTA_LOG_DUMP("%02x ", keyManager.random_b.random_key, prandom_b->length);

    /* calculate random_key */
    for ( uint8_t i = 0; i < prandom_a->length; i ++ )
    {
        random_key[i] = _generate_random_algorithm(i,prandom_a->random_key[i], prandom_b->random_key[i]);
    }

    TOTA_LOG_DBG(0,"generate random key:");
    TOTA_LOG_DUMP("%02x ", random_key, prandom_a->length);

    SHA256_hash(random_key, prandom_a->length, keyManager.hash_key);

    TOTA_LOG_DBG(0,"generate hash key:");
    TOTA_LOG_DUMP("%02x", keyManager.hash_key, HASH_KEY_SIZE);
}

static void _tota_conn_and_update_key_handle(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(3,"[%s]: opCode:0x%x, paramLen:%d", __func__, funcCode, paramLen);
    TOTA_CONN_STRUCT_T * pConn = (TOTA_CONN_STRUCT_T *)ptrParam;

    switch ( funcCode )
    {
        case OP_TOTA_CONN_INITIATE:
            _is_tota_initiate = true;
            keyManager.random_b = *pConn;
            _generate_random_key();
            app_tota_send((uint8_t*)&keyManager.random_a, keyManager.random_a.length+1, OP_TOTA_CONN_RESPONSE);
            _calculate_encrypt_key();
            break;
        case  OP_TOTA_CONN_CONFIRM:
            if ( _is_tota_initiate )
            {
                _is_tota_initiate = false;
                if ( memcmp(keyManager.hash_key, pConn->random_key, HASH_KEY_SIZE) == 0 )
                {
                    tota_set_encrypt_key_from_hash_key(keyManager.hash_key);
                    tota_set_connect_status(TOTA_SHAKE_HANDED);
#ifdef IBRT
                    tws_ctrl_send_cmd(APP_TWS_CMD_SYNC_TOTA_ENCODE_STATUS, keyManager.hash_key, HASH_KEY_SIZE);
#endif
                    TOTA_LOG_DBG(0, "tota connect success");
                    unsigned char str[] = "connect success.";
                    app_tota_send(str, strlen((const char*)str), OP_TOTA_STRING);
                }
                else
                {
                    tota_set_connect_status(TOTA_CONNECTED);
                    TOTA_LOG_DBG(0, "tota connect failed");
                    unsigned char str[] = "shake hand failed.";
                    app_tota_send(str, strlen((const char*)str), OP_TOTA_STRING);
                }
            }
            break;
        default:
            TOTA_LOG_DBG(1, "Unhandled OpCode:0x%x", funcCode);
    }
}

TOTA_COMMAND_TO_ADD(OP_TOTA_CONN_INITIATE, _tota_conn_and_update_key_handle, false, 0, NULL);
TOTA_COMMAND_TO_ADD(OP_TOTA_CONN_CONFIRM , _tota_conn_and_update_key_handle, false, 0, NULL);
TOTA_COMMAND_TO_ADD(OP_TOTA_CONN_RESPONSE , _tota_conn_and_update_key_handle, false, 0, NULL);