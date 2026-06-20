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
#ifndef __HAL_CMD_H__
#define __HAL_CMD_H__

#include "stdint.h"

typedef enum {
    HAL_CMD_RX_START,
    HAL_CMD_RX_STOP,
    HAL_CMD_RX_DONE
} hal_cmd_rx_status_t;

typedef int (*hal_cmd_callback_t)(uint8_t *buf, uint32_t  len);
typedef int (*CMD_CALLBACK_HANDLER_T)(hal_cmd_rx_status_t status);

#ifdef __cplusplus
extern "C" {
#endif
int hal_cmd_init(void);
int hal_cmd_open(void);
int hal_cmd_close(void);

int hal_cmd_run(hal_cmd_rx_status_t status);
void hal_cmd_set_callback(CMD_CALLBACK_HANDLER_T handler);
int hal_cmd_register(const char *name, hal_cmd_callback_t callback);

int hal_cmd_list_process(const uint8_t *buf);
void hal_cmd_tx_process(uint8_t **ppbuf, uint16_t *plen);

bool hal_cmd_debug_check(uint8_t *cmd, uint16_t len);
uint8_t *hal_cmd_debug_process(uint8_t *cmd, uint16_t len, uint16_t *out_len);

#if defined(TOTA_EQ_TUNING) && defined(AUDIO_SECTION_ENABLE)
int app_tota_audio_EQ(uint8_t *buf, uint16_t len);
#endif

#ifdef __cplusplus
}
#endif
// hal_cmd_t *hal_cmd_get_ptr(void);

#endif