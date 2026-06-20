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
#ifndef __RPC_COMMON_H__
#define __RPC_COMMON_H__

#include "rpc_bth_dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef RPC_CMD_DEFAULT_WAIT_RSP_TIMEOUT_MS
#define RPC_CMD_DEFAULT_WAIT_RSP_TIMEOUT_MS (500)
#endif

#ifndef RPC_MAX_DATA_PACKET_SIZE
#define RPC_MAX_DATA_PACKET_SIZE    (512)
#endif

typedef struct
{
    uint16_t  cmdcode;
    uint16_t cmdseq;
    uint8_t   content[RPC_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) rpc_core_data_t;

typedef struct
{
    uint16_t  cmdcode;
    uint8_t   content[RPC_MAX_DATA_PACKET_SIZE];
} __attribute__((packed)) rpc_cmd_data_t;

typedef void (*app_rpc_transmit_handler_t)(uint8_t*, uint16_t);
typedef void (*app_rpc_cmd_receivd_handler_t)(uint8_t*, uint16_t);
typedef void (*app_rpc_wait_rsp_timeout_handle_t)(uint8_t*, uint16_t);
typedef void (*app_rpc_rsp_handle_t)(uint8_t*, uint16_t);
typedef void (*app_rpc_cmd_transmit_done_handler_t) (uint16_t, uint8_t*, uint16_t);

typedef struct
{
    uint16_t                            cmdcode;
    const char                          *cmd_code_str;
    app_rpc_transmit_handler_t          cmd_transmit_handler;
    app_rpc_cmd_receivd_handler_t       cmd_received_handler;
    uint32_t                            cmd_wait_rsp_timeout_ms;
    app_rpc_wait_rsp_timeout_handle_t   cmd_wait_rsp_timeout_handle;
    app_rpc_rsp_handle_t                cmd_rsp_received_handle;
    app_rpc_cmd_transmit_done_handler_t cmd_transmit_done_handler;
} __attribute__((aligned(4))) rpc_cmd_inst_t;

#ifdef __cplusplus
}
#endif
#endif