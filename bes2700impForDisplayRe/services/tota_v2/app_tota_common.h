/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __APP_TOTA_COMMON_H__
#define __APP_TOTA_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "cqueue.h"
#include "app_tota_cmd_code.h"
#define TOTA_SPP_ID                  0

#define APP_TOTA_DATA_CMD_TIME_OUT_IN_MS    5000

#define TOTA_HEAD "HEAD"    //packet head verify code
#define TOTA_HEAD_SIZE 4

#define TOTA_LEN_SIZE 4
#define TOTA_DATA_HEAD_SIZE 4       //cmdcode and paramLen sizie

#define TOTA_CRC_SIZE 4

#define TOTA_TAIL "TAIL"    //packet tail verify code
#define TOTA_TAIL_SIZE 4


#define TOTA_PACKET_BASE 0
#define TOTA_PACKET_HEAD_OFFSET TOTA_PACKET_BASE
#define TOTA_PACKET_LEN_OFFSET (TOTA_PACKET_BASE+TOTA_HEAD_SIZE)
#define TOTA_PACKET_BUF_OFFSET (TOTA_PACKET_BASE+TOTA_HEAD_SIZE+TOTA_LEN_SIZE)
#define TOTA_PACKET_CRC_OFFSET(len) ((len) + TOTA_PACKET_BUF_OFFSET)
#define TOTA_PACKET_TAIL_OFFSET(len) ((len) + TOTA_PACKET_BUF_OFFSET+TOTA_CRC_SIZE)

//packet verify code len==sizeof(TOTA_PACKET_VERIFY_T)
#define TOTA_PACKET_VERIFY_SIZE (TOTA_HEAD_SIZE+TOTA_LEN_SIZE+TOTA_CRC_SIZE+TOTA_TAIL_SIZE)
#define TOTA_PACKET_TOTAL_SIZE(len) ((len) + TOTA_PACKET_VERIFY_SIZE)


#define TOTA_CMD_HEAD_SIZE 4//cmdcode+len
#define TOTA_CMD_HL_HEAD_MAX_SIZE 32 //high layer head max size
/*cache buffer size for OTA data.*/
#define TOTA_DATA_CACHE_BUFFER_SIZE (TOTA_CMD_HEAD_SIZE+TOTA_CMD_HL_HEAD_MAX_SIZE+4096)

/*used for packet sanity check*/
typedef struct
{
    uint8_t head[TOTA_HEAD_SIZE];

    uint32_t bufLen;

    uint32_t crc;

    uint8_t tail[TOTA_TAIL_SIZE];

} TOTA_PACKET_VERIFY_T;

/**
 * @brief Type of the tota connection status.
 *
 */
typedef enum
{
    TOTA_CONNECTED = 0,
    TOTA_SHAKE_HANDED = 1,
    TOTA_DISCONNECTED = 0xff,
} TOTA_CONNECT_STATUS_E;

/**
 * @brief Type of the tota store status.
 *
 */
typedef enum
{
    TOTA_STORE_OK = 0,
    TOTA_STORE_ON_GOING = 1,
    TOTA_STORE_FAIL = 0xff,
} TOTA_STORE_STATUS_E;

typedef struct {
    /* the length of the data (padded 16 bytes) will be transfered */
    uint32_t totalDataSize;

    uint32_t reverse;

} TOTA_Data_Ctrl_t;

typedef struct
{
    bool isBusy;

    TOTA_CONNECT_STATUS_E connectStatus;
    APP_TOTA_TRANSMISSION_PATH_E connectPath;

    uint16_t transMTU;

    //encode and decode use the same buffer.
    //uint8_t decodeBuf[MAX_SPP_PACKET_SIZE];
    //uint8_t encodeBuf[MAX_SPP_PACKET_SIZE];
    uint8_t *pcodeBuf;

    TOTA_Data_Ctrl_t tota_data_ctrl;

    /// used to cache TOTA decoded data
    uint8_t *pRxDataCacheBuffer;
    /// offset of data cached in cache-buffer
    uint32_t RxDataCacheBufferOffset;

    /// used to cache TOTA decoded data
    //uint8_t TxDataCacheBuffer[TOTA_DATA_CACHE_BUFFER_SIZE];
    /// offset of data cached in cache-buffer
    //uint32_t TxDataCacheBufferOffset;

    /// spp relay data RX buffer
    uint8_t *prxDataBuf;
    /// data RX queue
    CQueue rxDataQueue;

} TOTA_COMMON_ENV_T;

extern TOTA_COMMON_ENV_T totaEnv;


#if (TOTA_FACTORY_USED)
void tota_mempool_init(void);
#endif

void tota_common_init(void);
void tota_common_deinit(void);

void tota_set_connect_status(TOTA_CONNECT_STATUS_E status);

TOTA_CONNECT_STATUS_E tota_get_connect_status(void);

void tota_set_connect_path(APP_TOTA_TRANSMISSION_PATH_E path);

APP_TOTA_TRANSMISSION_PATH_E tota_get_connect_path(void);

void tota_set_trans_MTU(uint16_t MTU);

uint16_t tota_get_trans_MTU(void);

/*******************************************************************
* rx queue
*********************************************************************/
void tota_rx_queue_init(void);

int tota_rx_queue_length(void);

int tota_rx_queue_push(uint8_t *buff, uint16_t len);

int tota_rx_queue_pop(uint8_t *buff, uint16_t len);

int tota_rx_queue_peek(uint8_t *buff, uint16_t len);
void tota_rx_queue_deinit(void);

/*******************************************************************
* tota data ctrl
*********************************************************************/
void app_tota_ctrl_reset(void);

void app_tota_ctrl_set(uint8_t* data_ctrl);
/*******************************************************************
* tota data cache
*********************************************************************/
void app_tota_store_reset(void);

TOTA_STORE_STATUS_E app_tota_store(uint8_t * pdata, uint16_t dataLen);

int app_tota_rx_unpack(uint8_t * pdata, uint16_t dataLen);
uint16_t app_tota_tx_pack(uint8_t * pdata, uint16_t dataLen);

#ifdef __cplusplus
}
#endif

#endif

