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
#ifndef __CO_PPBUFF_H__
#define __CO_PPBUFF_H__
#include "bt_common_define.h"
#if defined(__cplusplus)
extern "C" {
#endif

struct pp_buff_flag_t {
    uint16_t conn_type: 4;
    uint16_t is_alloc_inplace: 1;
    uint16_t is_buff_auto_grow: 1;
    uint16_t dont_free_rx_ppb : 1;
    uint16_t defer_free_rx_ppb: 1;
    uint16_t insert_to_tx_header: 1;
    uint16_t dont_report_tx_done: 1;
    uint16_t tx_cont_fragment: 1;
    uint16_t dont_auto_free_by_hci: 1;
    uint16_t wait_cmpl_status_event: 1;
    uint16_t is_hci_rx_desc_ppb: 1;
    uint16_t is_sent_by_ble: 1;
};

struct pp_buff {
    struct single_link_node_t node;
    struct pp_buff_flag_t ppb_flag;
    uint16_t buff_size;
    uint16_t len; // data len
    uint16_t data_start;
    void *context;
};

#define ppb_alloc(size) ppb_alloc_with_ca((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define ppb_auto_grow_alloc(size) ppb_auto_grow_alloc_with_ca((size), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)
#define ppb_free(ppb) ppb_free_with_ca((ppb), (uint32_t)(uintptr_t)__builtin_return_address(0), __LINE__)

struct pp_buff *ppb_alloc_with_ca(uint16_t size, uint32_t ca, uint32_t line);
struct pp_buff *ppb_auto_grow_alloc_with_ca(uint16_t size, uint32_t ca, uint32_t line);
bool ppb_auto_grow_realloc(struct pp_buff **pptr, uint16_t size);
bool ppb_auto_grow_put_data(struct pp_buff **pptr, const uint8_t *data, uint16_t len);
uint8_t *ppb_auto_grow_put(struct pp_buff **pptr, uint16_t len);
struct pp_buff *ppb_alloc_inplace(uint16_t size, uint8_t *inplace_buff, uint16_t inplace_buff_len);
void ppb_free_with_ca(struct pp_buff *ppb, uint32_t ca, uint32_t line);

uint8_t *ppb_buff_head(struct pp_buff *ppb);
uint8_t *ppb_buff_end(struct pp_buff *ppb);
uint8_t *ppb_get_data(struct pp_buff *ppb);
uint8_t *ppb_data_end(struct pp_buff *ppb);
uint8_t *ppb_put(struct pp_buff *ppb, uint16_t len);
bool ppb_put_data(struct pp_buff *ppb, const uint8_t *data, uint16_t len);
void ppb_set_len(struct pp_buff *ppb, uint16_t len);
void ppb_reserve(struct pp_buff *ppb, uint16_t len);
uint8_t *ppb_use_reserve(struct pp_buff *ppb, uint16_t len);
uint8_t *ppb_data_forward(struct pp_buff *ppb, uint16_t len);
uint8_t *ppb_data_backward(struct pp_buff *ppb, uint16_t len);
uint8_t *ppb_tail_forward(struct pp_buff *ppb, uint16_t len);
uint8_t *ppb_tail_backward(struct pp_buff *ppb, uint16_t len);
uint16_t ppb_get_len(struct pp_buff *ppb);
uint16_t ppb_unused_len(struct pp_buff *ppb);

#if defined(__cplusplus)
}
#endif
#endif /* __CO_PPBUFF_H__ */