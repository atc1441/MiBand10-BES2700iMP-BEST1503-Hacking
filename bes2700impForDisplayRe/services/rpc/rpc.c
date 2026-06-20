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
#include "hal_timer.h"
#include "cmsis.h"
#include "cmsis_os.h"
#include <string.h>
#include "rpc.h"
#include "rpc_internal.h"
#include "hal_mcu2sens.h"
#include "hal_sys2bth.h"

struct rpc_core_context {
    bool            opened;
    volatile bool   connected;
    int             hw_chan_id;
#if defined(RTOS)
    osSemaphoreId_t sem_id;
    osMutexId_t     lock;
#endif
    uint8_t         *xfer_buf;
    uint32_t        xfer_buf_len;
    uint8_t         *rx_buf;
    uint32_t        rx_buf_len;
    int (*rpc_send)(int chan_id, const void *data,uint32_t len);
};

#define GENERATE_HANDLE(core, cmd_idx) (((uint32_t)core & 0xf) << 24 | cmd_idx)
#define GET_CORE(handle) (((uint32_t)handle >> 24) & 0xf)

#ifdef CHIP_BEST1600
#define HW_IPC_HAS_MCU2SEN  1
#define HW_IPC_HAS_SYS2BTH  1
#endif

static uint32_t rpc_xfer_buf[RPC_XFER_BUF_LEN / sizeof(uint32_t)];

static uint32_t rpc_rsp_buf[RPC_XFER_BUF_LEN / sizeof(uint32_t)];

static int rpc_bth2sens_send(int chan_id, const void *data, uint32_t len)
{
#ifdef HW_IPC_HAS_MCU2SEN
    return hal_mcu2sens_send(chan_id, data, len);
#else
    return 0;
#endif
}

static int rpc_bth2dsp_send(int chan_id, const void *data, uint32_t len)
{
#ifdef HW_IPC_HAS_SYS2BTH
    return hal_sys2bth_send(chan_id, data, len);
#else
    return 0;
#endif
}

#define RPC_SYS2BTH_ID HAL_SYS2BTH_ID_0

struct rpc_core_context rpc_bth2sens = {
    .opened     = false,
    .connected  = false,
    .hw_chan_id = HAL_MCU2SENS_ID_1,
    .rpc_send   = rpc_bth2sens_send,
};

struct rpc_core_context rpc_bth2dsp = {
    .opened     = false,
    .connected  = false,
    .hw_chan_id = RPC_SYS2BTH_ID,
    .rpc_send   = rpc_bth2dsp_send,
};

static struct rpc_core_context *rpc_core_tb[RPC_CORE_QTY] = {
    [RPC_CORE_BTH_SESHUB] = &rpc_bth2sens,
    [RPC_CORE_BTH_DSP] = &rpc_bth2dsp,
    /*...*/
};


static int rpc_handle_response(struct rpc_pkt *pkt);

#if defined(RTOS)
static osMessageQueueId_t rpc_mq;
#else
static volatile bool rpc_cmd_received = false;
static const void *rpc_buffer;
#endif

static POSSIBLY_UNUSED uint32_t rpc_msg_receive(const void *data, unsigned int len)
{
    ASSERT(data, "receive null message ");
    struct rpc_pkt * pkt = (struct rpc_pkt *)data;
    uint32_t cmd_type = pkt->cmd_rsp;

    /* TRACE(0, "%s data %p, len %d", __func__, data, len); */
    if ((cmd_type & RPC_RSP_TAG) == RPC_RSP_TAG) {
        rpc_handle_response(pkt);
    }else {
#if defined(RTOS)
        osStatus_t  status;
        struct rpc_pkt rpc_msg;

        memcpy(&rpc_msg, pkt, sizeof(struct rpc_pkt));
        status = osMessageQueuePut(rpc_mq, &rpc_msg, 0U, 0U);
        if (status != osOK) {
            TRACE(0, "rpc cannot put msg %d", status);
        }
#else
        rpc_cmd_received  = true;
        rpc_buffer = data;
#endif
    }

    return len;
}

static volatile bool rpc_rsp_received = false;

static rpc_status_t rpc_wait_for_complte(struct rpc_core_context *core_ctx,
                                uint32_t timeout, struct rpc_args *out_args)
{
    struct rpc_pkt *recv_args;

#if defined(RTOS)
    osStatus_t ret = osSemaphoreAcquire(core_ctx->sem_id, timeout);
    if (ret != osOK) {
        if (ret == osErrorTimeout) {
            TRACE(0, "%s timeout %d", __func__, timeout);
            return RPC_TIMEOUT;
        } else {
            return RPC_NO_DEV;
        }
    }
#else
    while (!rpc_rsp_received) {
        TRACE(0, "wait for rsp");
    }
    rpc_rsp_received = false;
#endif

    //copy the out_args from recv buffer
    recv_args = (struct rpc_pkt *)core_ctx->rx_buf;
    __COMPILER_BARRIER();
    if ((recv_args->cmd_rsp & 0x00ffffff) != 0) {
        TRACE(0, "cmd_rsp error %x", recv_args->cmd_rsp);
        return RPC_CMD_ERR;
    }

    if (!out_args || (out_args->args_len == 0)) {
        return RPC_OK;
    }

    if (out_args->args_len != recv_args->args_len) {
        ASSERT(0, "args between cli %d &svr %d is not sync", out_args->args_len,
                                                           recv_args->args_len);
        return RPC_CMD_ARG_ERR;
    }

    /* TRACE(0, "receive %d", recv_args->args_len); */
    __COMPILER_BARRIER();
    if (recv_args->args_len) {
        ASSERT(out_args->args, "no memory for argument");
        memcpy(out_args->args, &recv_args->args, recv_args->args_len);
    }

    return RPC_OK;
}

static inline uint32_t rpc_lock(struct rpc_core_context *ctx)
{
#if defined(RTOS)
    if (ctx)
        osMutexAcquire(ctx->lock, RPC_WAIT_FOREVER);
    return 0;
#else
    return int_lock();
#endif
}

static inline void rpc_unlock(struct rpc_core_context *ctx, uint32_t flags)
{
#if defined(RTOS)
    if (ctx)
        osMutexRelease(ctx->lock);
#else
    int_unlock(flags);
#endif
}

static uint32_t rpc_setup_packet(uint8_t *buf, int32_t core, int rpc_cmd,
                                const char *cmd_name, struct rpc_args *args)
{
    uint32_t len = 0;
    struct rpc_core_context *core_ctx;
    uint32_t flags;

    core_ctx = rpc_core_tb[core];
    flags = rpc_lock(core_ctx);
    memcpy(buf, &core, sizeof(int32_t));
    buf += sizeof(int32_t);
    len += sizeof(int32_t);

    memcpy(buf, &rpc_cmd, sizeof(int32_t));
    buf += sizeof(int32_t);
    len += sizeof(int32_t);

    if (cmd_name) {
        memcpy(buf, cmd_name, RPC_CMD_NAME_LEN);
    } else {
        memset(buf, 0, RPC_CMD_NAME_LEN);
    }

    buf += RPC_CMD_NAME_LEN;
    len += RPC_CMD_NAME_LEN;

    if (args != NULL && args->args_len != 0 ) {

        ASSERT(args->args_len <= 32, "args so big");
        memcpy(buf, &args->args_len, sizeof(uint32_t));
        buf += sizeof(uint32_t);
        len += sizeof(uint32_t);

        memcpy(buf, args->args, args->args_len);

        len += args->args_len;
    }
    rpc_unlock(core_ctx, flags);
    return len;
}

static int rpc_send_cmd(int core, int rpc_cmd, const char *cmd_name,
                        struct rpc_args *in_args, struct rpc_args *out_args,
                        uint32_t timeout, rpc_frontend_complete complete)
{

    int ret;
    struct rpc_core_context *core_ctx;
    uint8_t *send_buf;
    uint32_t len;

    core_ctx = rpc_core_tb[core];
    send_buf = core_ctx->xfer_buf;

    rpc_cmd = (rpc_cmd & 0x0fffffff) | RPC_CMD_TAG;
    len = rpc_setup_packet(send_buf, core, rpc_cmd, cmd_name, in_args);
    /* TRACE(0, "sending cmd %x", rpc_cmd); */

    //send cmd to core
    if (core_ctx->rpc_send)
        core_ctx->rpc_send(core_ctx->hw_chan_id, core_ctx->xfer_buf, len);
    //
    //wait_for_complete
    ret = rpc_wait_for_complte(core_ctx, timeout, out_args);

    return ret;
}

static int rpc_send_cmd_sync(int core, int rpc_cmd, const char *cmd_name,
                    struct rpc_args *in_args, struct rpc_args *out_args,
                    uint32_t timeout)
{
    return rpc_send_cmd(core, rpc_cmd, cmd_name, in_args, out_args, timeout, NULL);
}

static int rpc_send_cmd_async(int core, int rpc_cmd, const char *cmd_name,
                        struct rpc_args *in_args, rpc_frontend_complete complete)
{
    return rpc_send_cmd(core, rpc_cmd, cmd_name, in_args, NULL, 0, complete);
}

static int rpc_open_core(enum RPC_CORE_T core)
{
    int ret = -1;
    TR_INFO(0, "%s core %d", __func__, core);

    switch(core) {
    case RPC_CORE_BTH_SESHUB:
#ifdef HW_IPC_HAS_MCU2SEN
        TR_INFO(0, "mcu2sens opened");
        ret = hal_mcu2sens_open(HAL_MCU2SENS_ID_1, rpc_msg_receive, NULL, false);
        ASSERT(ret == 0, "hal_mcu2sens_open failed: %d", ret);

        ret = hal_mcu2sens_start_recv(HAL_MCU2SENS_ID_1);
        ASSERT(ret == 0, "hal_mcu2sens_start_recv failed: %d", ret);
#endif
        break;
    case RPC_CORE_BTH_DSP:
    case RPC_CORE_BTH_M55:
#ifdef HW_IPC_HAS_SYS2BTH
        ret = hal_sys2bth_open(RPC_SYS2BTH_ID, rpc_msg_receive, NULL, false);
        ASSERT(ret == 0, "hal_sys2bth_opened failed: %d", ret);
        ret = hal_sys2bth_start_recv(RPC_SYS2BTH_ID);
        ASSERT(ret == 0, "hal_sys2bth_start_recv failed: %d", ret);
#endif
        break;
    default:
        break;
    }

    return ret;
}

static int rpc_send_rsp(int32_t core, uint32_t handle, int cmd_rsp,
                                                struct rpc_args *out_args)
{
    uint32_t len = 0;
    struct rpc_core_context *core_ctx;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];

    uint8_t *send_buf = core_ctx->xfer_buf;
    /* send_buf = rpc_rsp_buf; */
    /* TRACE(0, "%s send_buf %p", __func__, send_buf); */

    cmd_rsp = (cmd_rsp & 0x0fffffff) | RPC_RSP_TAG;
    len = rpc_setup_packet(send_buf, core, cmd_rsp, NULL, out_args);

    core_ctx->rpc_send(core_ctx->hw_chan_id, core_ctx->xfer_buf, len);

    return 0;
}

struct rpc_cmd_map {
    struct rpc_cmd_param cmd;
    rpc_backend_handler  handler;
};

static struct rpc_cmd_map cmd_srv_tbl[RPC_MAX_CMD];
static int cmd_idx = 0;

static int rpc_exec_cmd(struct rpc_pkt *pkt, int need_rsp)
{
    int i;
    uint32_t handle = -1;
    int32_t core;
    struct rpc_args *in_args;
    const char *cmd_name;

    core = pkt->core;
    cmd_name = pkt->cmd_name;
    in_args = (struct rpc_args *)&pkt->args;

    //TODO: check cmd_idx valid
    for(i = 0; i < ARRAY_SIZE(cmd_srv_tbl); i++) {
        if (strncmp(cmd_srv_tbl[i].cmd.cmd_name, cmd_name, RPC_CMD_NAME_LEN) == 0)
            break;
    }

    if (i == ARRAY_SIZE(cmd_srv_tbl)) {
        TRACE(0, "cannot find server cmd for %s", cmd_name);
        if (need_rsp)
            rpc_send_rsp(core, handle, -1, NULL);
    } else {
        /* TRACE(0, "exec cmd %s", cmd_srv_tbl[i].cmd.cmd_name); */
        if (need_rsp)
            cmd_srv_tbl[i].handler(handle, in_args, rpc_send_rsp);
        else
            cmd_srv_tbl[i].handler(handle, in_args, NULL);
    }

    return 0;
}

static int rpc_handle_response(struct rpc_pkt *pkt)
{
#if defined(RTOS)
    struct rpc_core_context *core_ctx;
    uint32_t core;

    core = pkt->core;
    VERIFY_RPC_CORE(core);

    core_ctx = rpc_core_tb[core];
    /* TRACE(0, "%s args %p, len %d", __func__, pkt->args, pkt->args_len); */

    //get core from data packet
    osSemaphoreRelease(core_ctx->sem_id);
#else
    rpc_rsp_received = true;
#endif

    return 0;
}

static int rpc_handshake_rmt(int32_t core, struct rpc_hs_msg *msg)
{
    struct rpc_args out_args;
    struct rpc_hs_rsp rsp;
    struct rpc_core_context *core_ctx;


    VERIFY_RPC_CORE(core);


    core_ctx = rpc_core_tb[core];
    core_ctx->xfer_buf = msg->rx_buf;
    core_ctx->xfer_buf_len = msg->rx_buf_len;

    core_ctx->rx_buf = msg->xfer_buf;
    core_ctx->rx_buf_len = msg->xfer_buf_len;

    TRACE(0, "rpc_xfer_buf %p, len %d", core_ctx->xfer_buf, core_ctx->xfer_buf_len);
    TRACE(0, "rpc_rx_buf %p, len %d", core_ctx->rx_buf, core_ctx->rx_buf_len);

    memset(rsp.rpc_version, 0x0, sizeof(rsp.rpc_version));
    strncpy(rsp.rpc_version, "VER_1.0", sizeof(rsp.rpc_version) - 1);

    out_args.args = &rsp;
    out_args.args_len = sizeof(rsp);

    core_ctx->connected = true;
    rpc_send_rsp(core, -1, 0, &out_args);
    return 0;
}

static NOINLINE int rpc_handle_recv_msg(struct rpc_pkt *recv_args)
{
    int cmd = recv_args->cmd & 0x0fffffff;
    int32_t core = recv_args->core;

    if (cmd == RPC_HANDSHAKE)
        rpc_handshake_rmt(core, (struct rpc_hs_msg *)&recv_args->args);
    else if (cmd == RPC_EXEC_CMD) {
        rpc_exec_cmd(recv_args, 1);
    } else if (cmd == RPC_SIGNAL_CMD) {
        rpc_exec_cmd(recv_args, 0);
    }

    return 0;
}

#if defined(RTOS)
static void rpc_locol_thread(void const *arg)
{
    osStatus_t ret;
    struct rpc_pkt rpc_msg;

    for(;;) {
        ret = osMessageQueueGet(rpc_mq, &rpc_msg, NULL, osWaitForever);
        if (ret != osOK) {
            continue;
        }

        if ((rpc_msg.cmd & RPC_CMD_TAG) == RPC_CMD_TAG) {
            /* TRACE(0, "exec cmd"); */
            rpc_handle_recv_msg(&rpc_msg);
            continue;
        }
    }
}

osThreadDef(rpc_locol_thread, (osPriorityNormal), 1, (0x1000), "rpc_local");
#endif

static rpc_status_t rpc_init(int core)
{
    struct rpc_core_context *core_ctx;
    int ret;

    core_ctx = rpc_core_tb[core];
    if (!core_ctx) {
        ASSERT(core_ctx, "core %d is not init", core);
    }

    if (core_ctx->opened)
        return RPC_OK;
#if defined(RTOS)
    //init semphore, every core should has one sem
    core_ctx->sem_id = osSemaphoreNew(3, 0, NULL);
    if (!core_ctx->sem_id) {
        TRACE(0, "cannot create semphore for rpc");
        return RPC_NO_MEM;
    }
    core_ctx->lock = osMutexNew(NULL);
    if (!core_ctx->lock) {
        TRACE(0, "cannot create mutex for rpc");
        return RPC_NO_MEM;
    }

    rpc_mq = osMessageQueueNew (5, sizeof(struct rpc_pkt), NULL);
    if (rpc_mq == NULL) {
        TRACE(0, "cannot create message queue for rpc");
        return RPC_NO_MEM;
    }

    osThreadId tid;
    //create a thread;
    tid = osThreadCreate(osThread(rpc_locol_thread), NULL);
    if (!tid) {
        ASSERT(0, "can't create task for rpc");
    }
#endif

    ret = rpc_open_core(core);
    if (ret != 0)
        return RPC_NO_DEV;

    //TODO: should get buffer from a pool
    core_ctx->xfer_buf = (uint8_t *)rpc_xfer_buf;
    core_ctx->xfer_buf_len = sizeof(rpc_xfer_buf);

    core_ctx->rx_buf = (uint8_t *)rpc_rsp_buf;
    core_ctx->rx_buf_len = sizeof(rpc_rsp_buf);
    core_ctx->opened = true;
    return RPC_OK;
}


static int rpc_handshake(enum RPC_CORE_T core)
{
    struct rpc_args in_args = {0};
    struct rpc_args out_args = {0};

    struct rpc_hs_msg hs_msg;
    struct rpc_hs_rsp hs_rsp;

    struct rpc_core_context *core_ctx;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];

    memset(hs_msg.shake_msg, 0, sizeof(hs_msg.shake_msg));
    strncpy(hs_msg.shake_msg, "rpc", sizeof(hs_msg.shake_msg) - 1);

    hs_msg.xfer_buf = core_ctx->xfer_buf;
    hs_msg.xfer_buf_len = core_ctx->xfer_buf_len;

    hs_msg.rx_buf = core_ctx->rx_buf;
    hs_msg.rx_buf_len = core_ctx->rx_buf_len;

    in_args.args = &hs_msg;
    in_args.args_len = sizeof(hs_msg);

    out_args.args = &hs_rsp;
    out_args.args_len = sizeof(hs_rsp);

    rpc_status_t ret = rpc_send_cmd_sync(core, RPC_HANDSHAKE, 0, &in_args,
                                                        &out_args, 1000);
    if (ret == RPC_OK) {
        core_ctx->connected = true;
        TRACE(0, "handshake %s ", (char *)hs_rsp.rpc_version);
    }
    return ret;
}

int32_t rpc_open_backend_cmd(enum RPC_CORE_T core, const char *cmd)
{
#if 0
    struct rpc_args in_args;
    struct rpc_cmd_param cmd_param;
    /* const struct rpc_cli_cmd *cmd = cmds; */
    int ret = RPC_OK;

    if(!cmd)
        return RPC_CMD_ERR;

    //aquire the mutex
    //
    for ( int i = 0; i < cmds_num; i++) {
        if ((strlen(cmd->cmd_name) > RPC_CMD_NAME_LEN)) {
            TRACE(0, "rpc command wrong");
            ret = RPC_CMD_ERR;
            goto out;
        }

        memset(cmd_param.cmd_name, 0, RPC_CMD_NAME_LEN);
        strncpy(cmd_param.cmd_name, cmd->cmd_name, RPC_CMD_NAME_LEN - 1);
        cmd_param.handle = cmd->handle;

        in_args.args = &cmd_param;
        in_args.args_len = sizeof(cmd_param);

        ret = rpc_send_cmd_sync(core, RPC_REGISTER_CMD, 0, &in_args,
                NULL, osWaitForever);
        if (ret != RPC_OK) {
            TRACE(0, "rpc command cannot register %s", cmd_param.cmd_name);
        }

        cmd++;
    }

out:
    //release the mutex
    //
    TRACE(0, " %s %d ", __func__, ret);
    return ret;
#endif
    return RPC_OK;
}

/**
 * @brief         send a commond to remote cpu for executing synchronously
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @param[in]     in_args: the input arguments
 * @param[in]     timeout:  the timeout(ms) value for the remote cpu response,
 *                          osWaitForever for wait for ever
 * @param[out]    out_args: the output arguments received from remote cpu
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_exec_cmd_sync(enum RPC_CORE_T core, const char *cmd_name,
                struct rpc_args *in_args, struct rpc_args *out_args, uint32_t timeout)
{
    rpc_status_t ret;
    struct rpc_core_context *core_ctx;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];
    if (core_ctx->connected == false) {
        TRACE(0, "rpc not connected....");
        return RPC_NO_DEV;
    }
    ret = rpc_send_cmd_sync(core, RPC_EXEC_CMD, cmd_name, in_args, out_args, timeout);
    return ret;
}

/**
 * @brief         send a commond to remote cpu for executing synchronously
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @param[in]     in_args: the input arguments
 * @param[in]     timeout:  the timeout(ms) value for the remote cpu response,
 *                          osWaitForever for wait for ever
 * @param[out]    out_args: the output arguments received from remote cpu
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_notify_cmd(enum RPC_CORE_T core, const char *cmd_name,
                                                struct rpc_args *in_args)
{
    rpc_status_t ret;
    struct rpc_core_context *core_ctx;
    uint8_t *send_buf;
    uint32_t len;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];
    if (core_ctx->connected == false) {
        TRACE(0, "rpc not connected....");
        return RPC_NO_DEV;
    }

    core_ctx = rpc_core_tb[core];
    send_buf = core_ctx->xfer_buf;

    int rpc_cmd = (RPC_SIGNAL_CMD & 0x0fffffff) | RPC_CMD_TAG;
    len = rpc_setup_packet(send_buf, core, rpc_cmd, cmd_name, in_args);
    TRACE(0, "signal cmd %x", rpc_cmd);

    //send cmd to core
    if (core_ctx->rpc_send)
        core_ctx->rpc_send(core_ctx->hw_chan_id, core_ctx->xfer_buf, len);

    return ret;
}

/**
 * @brief         send a commond to remote cpu asynchronously
 * @param[in]     core, rpc core
 * @param[in]     handle, the command's handle
 * @param[in]     in_args: the input arguments
 * @param[in]     complete: after the command issue, call back this function when
 *                          there is response
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_exec_cmd_async(enum RPC_CORE_T core, const char *cmd_name,
                        struct rpc_args *in_args, rpc_frontend_complete complete)
{
    rpc_status_t ret;

    ret = rpc_send_cmd_async(core, RPC_EXEC_CMD, cmd_name, in_args, complete);

    return ret;
}

#define RPC_HS_TRY_TIMES 5
/**
 * @brief open rpc
 * @param[in]     core, rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_open_frontend(enum RPC_CORE_T core)
{
    rpc_status_t ret = RPC_OK;

    ret = rpc_init(core);
    if (ret != RPC_OK)
        goto err;

    int count = RPC_HS_TRY_TIMES;
    while (count > 0) {
        ret = rpc_handshake(core);
        if (ret == RPC_OK)
            break;
        count--;
    }


    if (count <= 0) {
       ret = RPC_NO_DEV;
       goto err;
    }

    TRACE(0, "%s open", __func__);
    return RPC_OK;

err:
    TRACE(0, "%s ret error %d", __func__, ret);
    return ret;
}

/**
 * @brief close rpc
 * @param[in]     core, rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_close(enum RPC_CORE_T core)
{
    return RPC_OK;
}

/**
 * @brief register a command for rpc server
 * @param[in]     core, the rpc core
 * @param[in]     cmds, the array of the server side commands
 * @param[in]     cmds_num, the number of commands
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_register_backend_cmd(enum RPC_CORE_T core,
                    const struct rpc_backend_cmd *cmds, uint32_t cmds_num)
{
    int i;
    const struct rpc_backend_cmd *cmd = cmds;
    rpc_status_t ret = RPC_OK;

    if(!cmd || cmds_num == 0)
        return RPC_CMD_ERR;

    struct rpc_core_context *core_ctx;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];

    //TODO:lock
    /* struct rpc_core_context *core_ctx = &rpc_core_ctx; */

    uint32_t flags = rpc_lock(core_ctx);
    if ((cmds_num + cmd_idx) >= ARRAY_SIZE(cmd_srv_tbl)) {
        TRACE(0, "too much cmds");
        ret = RPC_CMD_ERR;
        goto out;
    }

    for (i = 0; i < cmds_num; i++) {
        if(cmd->cmd_name != NULL) {
            if (strlen(cmd->cmd_name) > (RPC_CMD_NAME_LEN - 1)) {
                TRACE(0, "cmd %s too long", cmd->cmd_name);
                ret = RPC_CMD_ERR;
                goto out;
            }

            TRACE(0, "register backend cmd %s ", cmd->cmd_name);
            strncpy(cmd_srv_tbl[cmd_idx].cmd.cmd_name, cmd->cmd_name,
                                        RPC_CMD_NAME_LEN - 1);

            int handle = GENERATE_HANDLE(core, cmd_idx);
            cmd_srv_tbl[cmd_idx].cmd.handle = handle;

            cmd_srv_tbl[cmd_idx].handler = cmd->handler;
            cmd_idx++;
        }
        cmd++;
    }

out:
    //unlock
    rpc_unlock(core_ctx, flags);
    return ret;
}

/**
 * @brief unregister a command for rpc server
 * @param[in]     rpc core
 * @param[in]     cmd_name, string for command
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_unregister_backend_cmd(enum RPC_CORE_T core, const char cmd_name[])
{
    return RPC_OK;
}

/**
 * @brief open server side rpc
 * @param[in]     rpc core
 * @return        RPC_OK, success,
 *                others, fail
 */
rpc_status_t rpc_open_backend(enum RPC_CORE_T core)
{

    VERIFY_RPC_CORE(core);
    rpc_init(core);

#if defined(RTOS)
    //this make sure backend can not send command before handshare
    //finished
    struct rpc_core_context *core_ctx;
    core_ctx = rpc_core_tb[core];
    int cnt = 60;
    while ((core_ctx->connected == false) && (cnt-- > 0)) {
        TRACE(0, "%s polling ", __func__);
        osDelay(500);
    }

    if (cnt <= 0) {
        TRACE(0, "%s timeout", __func__);
        return RPC_NO_DEV;
    }
#endif
    TRACE(0, "%s ok", __func__);
    return RPC_OK;
}


#if !defined(RTOS)
void rpc_start_loop(enum RPC_CORE_T core)
{
    struct rpc_pkt *recv_args;
    struct rpc_core_context *core_ctx;

    VERIFY_RPC_CORE(core);
    core_ctx = rpc_core_tb[core];

    for(;;) {
        //TODO:
        /* if (!rpc_cmd_received) { */
        if (rpc_cmd_received != true && rpc_rsp_received != true) {
            /* TRACE(0, "wait for cmd"); */
            __WFI();
            continue;
        }
        rpc_cmd_received = false;

        //get signal
        recv_args = (struct rpc_pkt *)rpc_buffer;
        /* TRACE(0, "get cmd %s cmd %x", recv_args->cmd_name, recv_args->cmd); */
        __COMPILER_BARRIER();
        rpc_handle_recv_msg(recv_args);

        static int cnt = 3;
        if (core_ctx->connected == true && cnt-- > 0) {
            hal_sys_timer_delay(MS_TO_TICKS(10));
            /* test_who(); */
        }
    }
}
#endif

