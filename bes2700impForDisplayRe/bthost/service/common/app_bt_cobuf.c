/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include "co_heap.h"

/// BT APP MEMERY
#define BT_APP_COHEAP_BIG_BUFFER_LOW_LIMIT      (128)
#define BT_APP_COHEAP_BIG_BUFFER_HIGH_LIMIT     (1024*1)

#if defined(BLE_ONLY_ENABLED) && !(BLE_AUDIO_ENABLED)  //BLE only without LEA
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_SML   (0)
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_BIG   (1024*5)
#elif defined(BLE_TOTA_ENABLED)                        //Support OTA over LE
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_SML   (1024*2)
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_BIG   (1024*10)
#else                                                  //Default app cobuffer size
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_SML   (1024*2)
#define BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_BIG   (1024*3)
#endif

static uint8_t bt_app_coheap_b[BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_BIG] __attribute__((aligned(32)));
static uint8_t bt_app_coheap_s[BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_SML] __attribute__((aligned(32)));

struct coheap_global_t bt_app_coheap =
{
    .coheap_max_sml_size = BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_SML,
    .coheap_max_big_size = BT_APP_COHEAP_MAX_BUFFER_SIZE_FOR_BIG,
    .coheap_big_low_limit = BT_APP_COHEAP_BIG_BUFFER_LOW_LIMIT,
    .coheap_big_high_limit = BT_APP_COHEAP_BIG_BUFFER_HIGH_LIMIT,
    .coheap_b_pool = bt_app_coheap_b,
    .coheap_s_pool = bt_app_coheap_s,
};

unsigned char *app_bt_buf_malloc(uint16 size, uint32 ca, uint32 line)
{
    return (unsigned char *)coheap_bt_malloc_with_ca(&bt_app_coheap, size, ca, line, false);
}

void app_bt_buf_free(unsigned char *buf, uint32 ca, uint32 line)
{
    coheap_free_with_ca(&bt_app_coheap, (struct coheap_buff_t *)buf, ca, line);
}

bool app_bt_check_buf_available(uint16_t size)
{
    return coheap_check_malloc_available(&bt_app_coheap, size);
}

void app_bt_buf_init(void)
{
    coheap_init(&bt_app_coheap);
}

unsigned char *bes_bt_me_malloc(uint16 size, uint32 ca, uint32 line)
    __attribute__((alias("app_bt_buf_malloc")));

void bes_bt_me_free(unsigned char *buf, uint32 ca, uint32 line)
    __attribute__((alias("app_bt_buf_free")));

bool bes_bt_check_buffer_available(uint16_t size)
    __attribute__((alias("app_bt_check_buf_available")));