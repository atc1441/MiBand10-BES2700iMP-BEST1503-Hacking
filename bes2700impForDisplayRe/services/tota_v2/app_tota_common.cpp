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
#include <stdio.h>
#include <string.h>
#include "cmsis.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "app_tota_encrypt.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_common.h"
#include "app_tota.h"
#include "app_spp_tota.h"
#include "crc32_c.h"
#include "heap_api.h"

#if (TOTA_FACTORY_USED)
#include "app_anc_assist.h"
#include "app_audio.h"
#endif

TOTA_COMMON_ENV_T totaEnv;
static APP_TOTA_CMD_PAYLOAD_T *payload = NULL;

#if !(TOTA_FACTORY_USED)
APP_TOTA_CMD_PAYLOAD_T payloadBuf;
uint8_t codeBuf[MAX_SPP_PACKET_SIZE];
uint8_t RxDataCacheBuffer[TOTA_DATA_CACHE_BUFFER_SIZE];
uint8_t rxDataBuf[TOTA_RX_BUF_SIZE];
#endif


#if (TOTA_FACTORY_USED)
#define TOTA_BUF_FOR_ALIGNMENT  12
void tota_mempool_init(void)
{
    if(NULL == payload){
        // TRACE(0,"[%s] %d %d %d %d ",__func__, sizeof(APP_TOTA_CMD_PAYLOAD_T), (MAX_SPP_PACKET_SIZE), (TOTA_DATA_CACHE_BUFFER_SIZE), (TOTA_RX_BUF_SIZE));
        uint32_t tota_buf_size = sizeof(APP_TOTA_CMD_PAYLOAD_T) +
                                MAX_SPP_PACKET_SIZE + 
                                TOTA_DATA_CACHE_BUFFER_SIZE +
                                TOTA_RX_BUF_SIZE + 
                                TOTA_BUF_FOR_ALIGNMENT;

        app_tota_mempool_init(tota_buf_size);
        app_tota_mempool_get_buff((uint8_t**)&payload, sizeof(APP_TOTA_CMD_PAYLOAD_T));
        ASSERT((payload!=NULL),"tota app payload malloc fail.");
        app_tota_mempool_get_buff(&(totaEnv.pcodeBuf), (MAX_SPP_PACKET_SIZE));
        ASSERT((totaEnv.pcodeBuf!=NULL),"totaEnv pcodeBuf malloc fail.");
        app_tota_mempool_get_buff(&(totaEnv.pRxDataCacheBuffer), (TOTA_DATA_CACHE_BUFFER_SIZE));
        ASSERT((totaEnv.pRxDataCacheBuffer!=NULL),"totaEnv pRxDataCacheBuffer malloc fail.");
        app_tota_mempool_get_buff(&(totaEnv.prxDataBuf), (TOTA_RX_BUF_SIZE));
        ASSERT((totaEnv.prxDataBuf!=NULL),"totaEnv prxDataBuf malloc fail.");
    }

}
#endif

void tota_common_init(void)
{
#if (TOTA_FACTORY_USED)
    tota_mempool_init();
#else
    payload = &payloadBuf;
    totaEnv.pcodeBuf = codeBuf;
    totaEnv.pRxDataCacheBuffer = RxDataCacheBuffer;
    totaEnv.prxDataBuf = rxDataBuf;
#endif
    app_tota_payload_set(payload);
    totaEnv.connectStatus = TOTA_DISCONNECTED;

    totaEnv.isBusy = false;
    totaEnv.connectPath = APP_TOTA_PATH_IDLE;

    totaEnv.transMTU = MAX_SPP_PACKET_SIZE;

    totaEnv.tota_data_ctrl.totalDataSize = 0;
    totaEnv.tota_data_ctrl.reverse = 0;

    totaEnv.RxDataCacheBufferOffset = 0;

    tota_rx_queue_init();
}

void tota_common_deinit(void)
{
#if (TOTA_FACTORY_USED)
    payload = NULL;
#endif
}
void tota_set_connect_status(TOTA_CONNECT_STATUS_E status)
{
    totaEnv.connectStatus = status;
}

TOTA_CONNECT_STATUS_E tota_get_connect_status(void)
{
    return totaEnv.connectStatus;
}

void tota_set_connect_path(APP_TOTA_TRANSMISSION_PATH_E path)
{
    totaEnv.connectPath = path;
}

APP_TOTA_TRANSMISSION_PATH_E tota_get_connect_path(void)
{
    return totaEnv.connectPath;
}

void tota_set_trans_MTU(uint16_t MTU)
{
    if(MTU <= totaEnv.transMTU)
    {
        totaEnv.transMTU = MTU;
    }
}

uint16_t tota_get_trans_MTU(void)
{
    return totaEnv.transMTU;
}

/*******************************************************************
* rx queue
*********************************************************************/
void tota_rx_queue_init(void)
{
    InitCQueue(&(totaEnv.rxDataQueue), TOTA_RX_BUF_SIZE, (unsigned char *)totaEnv.prxDataBuf);
    memset(totaEnv.prxDataBuf, 0x00, TOTA_RX_BUF_SIZE);
}

int tota_rx_queue_length(void)
{
    int len;
    len = LengthOfCQueue(&(totaEnv.rxDataQueue));
    return len;
}

int tota_rx_queue_push(uint8_t *buff, uint16_t len)
{
    int status;
    status = EnCQueue(&(totaEnv.rxDataQueue), buff, len);
    return status;
}

int tota_rx_queue_pop(uint8_t *buff, uint16_t len)
{
    int status;

    status = DeCQueue(&(totaEnv.rxDataQueue), buff, len);

    return status;
}

int tota_rx_queue_peek(uint8_t *buff, uint16_t len)
{
    CQueue *ptrQueue = &(totaEnv.rxDataQueue);
    uint8_t *e1 = NULL, *e2 = NULL;
    unsigned int len1 = 0, len2 = 0;

    PeekCQueue(ptrQueue, len, &e1, &len1, &e2, &len2);
    if (len == (len1 + len2))
    {
        memcpy(buff, e1, len1);
        memcpy(buff + len1, e2, len2);
    }
    else
    {
        memset(buff, 0x00, len);
    }
    return len;
}
void tota_rx_queue_deinit(void)
{
    ResetCQueue(&(totaEnv.rxDataQueue));
    memset(totaEnv.prxDataBuf, 0x00, TOTA_RX_BUF_SIZE);
}

/*******************************************************************
* tota data ctrl
*********************************************************************/
void app_tota_ctrl_reset(void)
{
    totaEnv.tota_data_ctrl.totalDataSize = 0;
    totaEnv.tota_data_ctrl.reverse = 0;
}

void app_tota_ctrl_set(uint8_t* data_ctrl)
{
    ASSERT((data_ctrl!=NULL), "%s:data_ctrl NULL!", __func__);
    memcpy((uint8_t*)&(totaEnv.tota_data_ctrl),data_ctrl,sizeof(TOTA_Data_Ctrl_t));

    uint16_t actLen = totaEnv.tota_data_ctrl.totalDataSize;
    totaEnv.tota_data_ctrl.totalDataSize = (actLen % ENCRYPT_KEY_SIZE)? (actLen + ENCRYPT_KEY_SIZE - (actLen % ENCRYPT_KEY_SIZE)) : actLen;

    TOTA_LOG_DBG(2, "actLen:0x%x, totalDataSize:0x%x", actLen, totaEnv.tota_data_ctrl.totalDataSize);
}
/*******************************************************************
* tota data cache
*********************************************************************/
void app_tota_store_reset(void)
{
    memset(totaEnv.pRxDataCacheBuffer,0x0,TOTA_DATA_CACHE_BUFFER_SIZE);
    totaEnv.RxDataCacheBufferOffset = 0;
}

TOTA_STORE_STATUS_E app_tota_store(uint8_t * pdata, uint16_t dataLen)
{
    uint8_t * pbuf = totaEnv.pRxDataCacheBuffer + totaEnv.RxDataCacheBufferOffset;
    uint16_t store_pos = totaEnv.RxDataCacheBufferOffset + dataLen;
    TOTA_STORE_STATUS_E ret = TOTA_STORE_OK;

    if(TOTA_DATA_CACHE_BUFFER_SIZE >= store_pos)
    {
        memcpy(pbuf, pdata, dataLen);
        totaEnv.RxDataCacheBufferOffset = store_pos;
        APP_TOTA_CMD_PAYLOAD_T *payLoad = (APP_TOTA_CMD_PAYLOAD_T *)totaEnv.pRxDataCacheBuffer;
        if(store_pos < (payLoad->paramLen+TOTA_DATA_HEAD_SIZE))
        {
            ret = TOTA_STORE_ON_GOING;
        }
    }
    else
    {
        ret = TOTA_STORE_FAIL;
        app_tota_store_reset();
        app_tota_ctrl_reset();
    }

    TOTA_LOG_DBG(2,"app_tota_store status=%d,DataCacheBufferOffset=0x%x", ret,totaEnv.RxDataCacheBufferOffset);
    return ret;
}

/************
*prase rx queue data
*pdata:reserse
*dataLen:queue data Length
************/
int app_tota_rx_unpack(uint8_t * pdata, uint16_t dataLen)
{
    uint8_t temp_buf[MAX_SPP_PACKET_SIZE];
    TOTA_STORE_STATUS_E status = TOTA_STORE_FAIL;
    TOTA_CONNECT_STATUS_E connect_status;
    uint32_t crc_value = 0;
    uint32_t store_len = 0;
    TOTA_PACKET_VERIFY_T totaPacketVerify;
    TOTA_LOG_DBG(1,"%s",__func__);
    dataLen = (dataLen < MAX_SPP_PACKET_SIZE) ? dataLen:MAX_SPP_PACKET_SIZE;

    tota_rx_queue_peek((uint8_t *)&totaPacketVerify, TOTA_HEAD_SIZE+TOTA_LEN_SIZE);
    TOTA_LOG_DBG(1, "bufLen:0x%x", totaPacketVerify.bufLen);
    //DUMP8("%02x",totaPacketVerify.head,TOTA_HEAD_SIZE);
    //TOTA_LOG_DBG(0,"\n");

    if(memcmp(totaPacketVerify.head,TOTA_HEAD,TOTA_HEAD_SIZE))
    {
        TOTA_LOG_ERR(0,"rx data head err!!!!!!!");
        tota_rx_queue_deinit();
        return -1;
    }
    if(TOTA_PACKET_TOTAL_SIZE(totaPacketVerify.bufLen) > MAX_SPP_PACKET_SIZE)
    {
        TOTA_LOG_ERR(0,"bufLen err!!!!!!!");
        tota_rx_queue_deinit();
        return -2;
    }

    if(TOTA_PACKET_TOTAL_SIZE(totaPacketVerify.bufLen) > dataLen)
    {
        TOTA_LOG_DBG(0,"bufLen underflow,wait!!!");
        return -3;
    }

    if(tota_rx_queue_pop(temp_buf,TOTA_PACKET_TOTAL_SIZE(totaPacketVerify.bufLen)))
    {
        TOTA_LOG_ERR(0,"rx queue pop err!!!!!!!");
        tota_rx_queue_deinit();
        return -4;
    }

    totaPacketVerify.crc = *(uint32_t *)(temp_buf+TOTA_PACKET_CRC_OFFSET(totaPacketVerify.bufLen));
    memcpy(totaPacketVerify.tail,temp_buf+TOTA_PACKET_TAIL_OFFSET(totaPacketVerify.bufLen),TOTA_TAIL_SIZE);
    //DUMP8("%02x",totaPacketVerify.tail,TOTA_TAIL_SIZE);

    if(memcmp(totaPacketVerify.tail,TOTA_TAIL,TOTA_TAIL_SIZE))
    {
        TOTA_LOG_ERR(0,"rx data tail err!!!!!!!");
        return -6;
    }

    uint8_t *pbuf = temp_buf+TOTA_PACKET_BUF_OFFSET;

    crc_value = crc32_c(crc_value,(const unsigned char *)pbuf,(unsigned int)totaPacketVerify.bufLen);
    if(crc_value != totaPacketVerify.crc)
    {
        TOTA_LOG_ERR(0,"rx data crc err!!!!!!!");
        return -7;
    }

    store_len = totaPacketVerify.bufLen;
    connect_status = tota_get_connect_status();

    switch (connect_status)
    {
        case TOTA_CONNECTED:
            TOTA_LOG_DBG(0,"TOTA_CONNECTED");
            status = app_tota_store(pbuf, store_len);
            break;

        case TOTA_SHAKE_HANDED:
            TOTA_LOG_DBG(0,"TOTA_SHAKE_HANDED");
#if TOTA_ENCODE
            store_len = tota_decrypt(totaEnv.pcodeBuf, pbuf, store_len);
#endif
            status = app_tota_store(totaEnv.pcodeBuf, store_len);
            break;

        case TOTA_DISCONNECTED:
        default:
            TOTA_LOG_DBG(0,"TOTA_DISCONNECTED");
            break;
    }


    if(TOTA_STORE_OK == status)
    {
        app_tota_cmd_received(totaEnv.pRxDataCacheBuffer,totaEnv.RxDataCacheBufferOffset);
        app_tota_store_reset();
        app_tota_ctrl_reset();
    }

    return 0;
}

uint16_t app_tota_tx_pack(uint8_t * pdata, uint16_t dataLen)
{
    uint8_t temp_buf[MAX_SPP_PACKET_SIZE];
    uint32_t crc_value = 0;

    if(TOTA_PACKET_TOTAL_SIZE(dataLen) > MAX_SPP_PACKET_SIZE)
    {
        TOTA_LOG_DBG(0,"data overflow!!!");
        return 0;
    }

    memcpy(temp_buf+TOTA_PACKET_HEAD_OFFSET,TOTA_HEAD,TOTA_HEAD_SIZE);
    *(uint32_t *)(temp_buf+TOTA_PACKET_LEN_OFFSET) = dataLen;
    memcpy(temp_buf+TOTA_PACKET_BUF_OFFSET,pdata,dataLen);
    crc_value = crc32_c(crc_value,(const unsigned char *)pdata,(unsigned int)dataLen);
    *(uint32_t *)(temp_buf+TOTA_PACKET_CRC_OFFSET(dataLen)) = crc_value;
    memcpy(temp_buf+TOTA_PACKET_TAIL_OFFSET(dataLen),TOTA_TAIL,TOTA_TAIL_SIZE);
    memcpy(pdata,temp_buf,TOTA_PACKET_TOTAL_SIZE(dataLen));

    return (TOTA_PACKET_TOTAL_SIZE(dataLen));

}


