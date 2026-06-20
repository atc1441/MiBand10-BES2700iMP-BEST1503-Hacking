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
#ifndef __APP_RPC_API_H__
#define __APP_RPC_API_H__

#include "rpc_common.h"

#ifdef __cplusplus
extern "C" {
#endif

enum APP_RPC_CORE_T {
    APP_RPC_CORE_BTH_M55,
    APP_RPC_CORE_BTH_DSP,
    APP_RPC_CORE_DSP_M55,
    APP_RPC_CORE_MCU_SENSOR,

    APP_RPC_CORE_QTY,
};

enum APP_RPC_RESULT_T{
    RPC_RES_SUCCESS = 0,
    RPC_RES_FAILD = -1
};

int32_t app_rpc_send_data_no_rsp(enum APP_RPC_CORE_T core, uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
int32_t app_rpc_send_data_waiting_rsp(enum APP_RPC_CORE_T core, uint16_t cmdcode, uint8_t *p_buff, uint16_t length);
int32_t app_rpc_send_cmd(enum APP_RPC_CORE_T core, uint16_t cmd_code, uint8_t *p_buff, uint16_t length);
int32_t app_rpc_send_cmd_rsp(enum APP_RPC_CORE_T core, uint16_t rsp_code, uint8_t *p_buff, uint16_t length);

void app_rpc_ctx_init(enum APP_RPC_CORE_T core);
void app_rpc_ctx_deinit(enum APP_RPC_CORE_T core);

#ifdef __cplusplus
}
#endif
#endif
