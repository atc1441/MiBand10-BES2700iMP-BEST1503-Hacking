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
#include "plat_types.h"
#include "hal_trace.h"
#include <string.h>
#include "rpc.h"
#include "rpc_internal.h"
#include "cmsis_os.h"

int rpc_ping_back(enum RPC_CORE_T core, uint32_t handle,
                    struct rpc_args *in_args, rpc_backend_complete complete)
{
    struct rpc_args out_args = {0};
    char rsp[4] = "ok";

    TRACE(0, "%s", __func__);
    out_args.args = &rsp[0];
    out_args.args_len = 4;

    if (complete)
        complete(core, handle, 0, &out_args);

    return 0;

}

rpc_status_t rpc_ping_remote(enum RPC_CORE_T core, int try_times)
{
    struct rpc_args out_args;
    char ping_rsp[4];
    rpc_status_t ret;

    out_args.args = &ping_rsp[0];
    out_args.args_len = sizeof(ping_rsp);

    while (try_times-- > 0) {
        ret = rpc_exec_cmd_sync(core, "ping", NULL,
                                            &out_args, 1000);
        if (ret == RPC_OK) {
            break;
        } else {
            TRACE(0, "try ping remote...");
            osDelay(1000);
        }
    }
    if (try_times <= 0) {
        TRACE(0, "ping fail %d", ret);
        return RPC_NO_CONNECT;
    }

    TRACE(0, "%s, %s", __func__, ping_rsp);
    return RPC_OK;
}

