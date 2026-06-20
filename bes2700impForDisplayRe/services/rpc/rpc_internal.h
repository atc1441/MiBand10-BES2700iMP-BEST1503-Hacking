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
#ifndef __RPC_INTERNAL_H__
#define __RPC_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#define RPC_VERSION "1.0"

#define RPC_CMD_TAG     (0x1UL << 28)
#define RPC_RSP_TAG     (0x2UL << 28)


#define RPC_CMD_NAME_LEN  16

struct rpc_pkt {
    int32_t core;
    union {
        int32_t    cmd;
        int32_t    cmd_rsp;
    };
    char cmd_name[RPC_CMD_NAME_LEN];
    uint32_t args_len;
    uint8_t    args[32];
};

enum RPC_COMMAND {
    RPC_HANDSHAKE = 0x1001,
    RPC_REGISTER_CMD,
    RPC_EXEC_CMD,
    RPC_SIGNAL_CMD,
};

#define RPC_MAX_CMD  20

#define INVALID_RPC_HANDLE      (-1U)

struct rpc_cmd_param {
    uint32_t  handle;
    char      cmd_name[RPC_CMD_NAME_LEN];
};


#define RPC_XFER_BUF_LEN 256  /*size of bytes*/

#define RPC_HS_MSG_LEN 16

struct rpc_hs_msg {
    char shake_msg[RPC_HS_MSG_LEN];
    uint8_t *xfer_buf;
    uint32_t xfer_buf_len;

    uint8_t *rx_buf;
    uint32_t rx_buf_len;
};

struct rpc_hs_rsp {
    char rpc_version[RPC_HS_MSG_LEN];
};

#define VERIFY_RPC_CORE(core) \
    do { \
        ASSERT(core < RPC_CORE_QTY, "rpc core error"); \
    } while(0)


#ifdef __cplusplus
}
#endif

#endif /* __RPC_INTERNAL_H__ */
