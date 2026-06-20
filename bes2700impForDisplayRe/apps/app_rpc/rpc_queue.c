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
#include "cmsis.h"
#include "cmsis_os.h"
#include "plat_types.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"

#include "rpc_queue.h"

#define APP_RPC_QUEUE_DATA_LEN_BYTES    (2)

void rpc_queue_init(CQueue* ptrQueue, uint8_t* ptrBuf, uint32_t bufLen)
{
    uint32_t lock = int_lock();
    memset(ptrBuf, 0, bufLen);
    InitCQueue(ptrQueue, bufLen, (CQItemType *)ptrBuf);
    int_unlock(lock);
}

int32_t rpc_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length)
{
    int32_t nRet = -1;
    uint32_t lock = int_lock();
    if (length > 0)
    {
        uint16_t dataLen = (uint16_t)length;
        int queueAvailableLen = AvailableOfCQueue(ptrQueue);
        if ((dataLen+APP_RPC_QUEUE_DATA_LEN_BYTES) <= queueAvailableLen)
        {
            EnCQueue(ptrQueue, (CQItemType *)&dataLen, APP_RPC_QUEUE_DATA_LEN_BYTES);
            EnCQueue(ptrQueue, (CQItemType *)ptrData, length);
            nRet = 0;
        }
    }
    int_unlock(lock);
    // osSignalSet(app_core_bridge_rx_thread_id, APP_CORE_BRIDGE_RX_THREAD_SIGNAL_DATA_RECEIVED);

    return nRet;
}

uint16_t rpc_queue_get_next_entry_length(CQueue* ptrQueue)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;
    uint16_t length = 0;
    uint8_t* ptr;

    uint32_t lock = int_lock();

    ptr = (uint8_t *)&length;
    // get the length of the fake message
    PeekCQueue(ptrQueue, APP_RPC_QUEUE_DATA_LEN_BYTES, &e1, &len1, &e2, &len2);

    memcpy(ptr,e1,len1);
    memcpy(ptr+len1,e2,len2);

    int_unlock(lock);

    return length;
}

void rpc_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len)
{
    uint8_t *e1 = NULL, *e2 = NULL;
    uint32_t len1 = 0, len2 = 0;

    uint32_t lock = int_lock();
    // overcome the two bytes of msg length
    DeCQueue(ptrQueue, 0, APP_RPC_QUEUE_DATA_LEN_BYTES);

    PeekCQueue(ptrQueue, len, &e1, &len1, &e2, &len2);
    if (len==(len1+len2)){
        memcpy(buff,e1,len1);
        memcpy(buff+len1,e2,len2);
        DeCQueue(ptrQueue, 0, len);

        // reset the poped data to ZERO
        memset(e1, 0, len1);
        memset(e2, 0, len2);
    }else{
        memset(buff, 0x00, len);
    }
    int_unlock(lock);
}

int32_t rpc_get_queue_length(CQueue *ptrQueue)
{
    int32_t nRet = 0;

    uint32_t lock = int_lock();
    nRet = LengthOfCQueue(ptrQueue);
    int_unlock(lock);

    return nRet;
}