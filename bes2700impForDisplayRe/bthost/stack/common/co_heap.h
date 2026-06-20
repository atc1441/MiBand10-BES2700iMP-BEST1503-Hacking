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
#ifndef __CO_HEAP_H__
#define __CO_HEAP_H__

#include "bt_common_define.h"

#if defined(__cplusplus)
extern "C" {
#endif

struct coheap_global_t
{
    void *coheap_b_handle;
    void *coheap_s_handle;
    uint8_t *coheap_b_pool;
    uint8_t *coheap_s_pool;
    uint16_t coheap_max_sml_size;
    uint16_t coheap_max_big_size;
    uint16_t coheap_big_low_limit;
    uint16_t coheap_big_high_limit;
    bool coheap_enable_debug;
    uint16_t peak_size_b_pool;
    uint16_t peak_size_s_pool;
};

struct coheap_buff_header_t
{
    uint16 buf_seqn;
    uint16 alloc_ln;
    uint32 alloc_ca;
};

struct coheap_block_t
{
    intptr_t header;
    union
    {
        struct coheap_block_t *next_free;
        uint8_t data[1];
    };
};

struct coheap_header_t
{
    void *lock;
    size_t total_bytes;
    size_t free_bytes;
    size_t minimum_free_bytes;
    struct coheap_block_t *last_block;
    struct coheap_block_t first_block; /* initial 'free block', never allocated */
};

struct coheap_buff_t;

/// COHEAP API
void coheap_init(struct coheap_global_t *coheap);
bool coheap_check_malloc_available(struct coheap_global_t *coheap, uint16_t size);
void *coheap_get_heap_from_buffer(struct coheap_global_t *coheap, struct coheap_buff_t *buffer);
uint32_t coheap_lock(struct coheap_header_t *heap);
void coheap_unlock(struct coheap_header_t *heap, uint32_t flags);
struct coheap_buff_t *coheap_bt_malloc_with_ca(struct coheap_global_t *coheap, uint16 size, uint32 ca, uint32 line, bool allow_fail_alloc);
void coheap_free_with_ca(struct coheap_global_t *coheap, struct coheap_buff_t *p, uint32 ca, uint32 line);
void coheap_dump_statistics(struct coheap_global_t *coheap);
void coheap_enable_debug(struct coheap_global_t *coheap, bool enable);

#define coheap_malloc(coheap, size) coheap_bt_malloc_with_ca((struct coheap_global_t *)coheap, (size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__, false)
#define coheap_malloc_with_ca(coheap, size, ca, line) coheap_bt_malloc_with_ca((struct coheap_global_t *)coheap, (size), (ca), (line), false)
#define coheap_allow_fail_malloc(coheap, size) coheap_allow_fail_malloc((struct coheap_global_t *)coheap, (size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__, true)
#define coheap_free(coheap, buffer) coheap_free_with_ca((struct coheap_global_t *)coheap, (buffer), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)


#if defined(__cplusplus)
}
#endif
#endif /* __CO_HEAP_H__ */