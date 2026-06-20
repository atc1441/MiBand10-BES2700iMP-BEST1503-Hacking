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
#ifndef __RPC_QUEUE_H__
#define __RPC_QUEUE_H__

#include "plat_types.h"
#include "cqueue.h"

#ifdef __cplusplus
extern "C" {
#endif

void rpc_queue_init(CQueue* ptrQueue, uint8_t* ptrBuf, uint32_t bufLen);
int32_t rpc_queue_push(CQueue* ptrQueue, const void* ptrData, uint32_t length);
uint16_t rpc_queue_get_next_entry_length(CQueue* ptrQueue);
void rpc_queue_pop(CQueue* ptrQueue, uint8_t *buff, uint32_t len);
int32_t rpc_get_queue_length(CQueue *ptrQueue);

#ifdef __cplusplus
}
#endif
#endif
