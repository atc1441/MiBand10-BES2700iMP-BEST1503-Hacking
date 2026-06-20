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

#ifndef __OTA_SEC_MEM_H__
#define __OTA_SEC_MEM_H__

#ifdef __cplusplus
extern "C" {
#endif

void sec_mem_total_init(void);

unsigned int sec_mem_total_avalable(void);

unsigned int sec_mem_total_size_get(void);

unsigned int sec_mem_buf_alloc(unsigned char **buff, unsigned int size);

void sec_mem_heap_init(unsigned int size);

void * sec_mem_heap_malloc(unsigned int size) ;

void *sec_mem_heap_calloc(unsigned int  size);

void * sec_mem_heap_realloc(void *rmem, unsigned int  newsize);

void sec_mem_heap_free(void *rmem);

#ifdef __cplusplus
}
#endif

#endif

