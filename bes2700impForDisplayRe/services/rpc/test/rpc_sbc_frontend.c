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
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_cmu.h"
#include "sbc.h"
#include "codec_sbc.h"
#include "rpc_sbc_frontend.h"
#include "rpc.h"
#include "rpc_ping.h"

void btif_sbc_init_decoder_dsp(btif_sbc_decoder_t *Decoder)
{
    struct rpc_args out_args;

    TRACE(0, "%s", __func__);
    out_args.args = &Decoder->streamInfo.sampleFreq;
    out_args.args_len = sizeof(Decoder->streamInfo.sampleFreq);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_SESHUB, "init_decoder", NULL,
                                            &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK) {
        TRACE(0, "init decoder err %d", ret);
    }
}

bt_status_t btif_sbc_decode_frames_dsp(btif_sbc_decoder_t *Decoder,
                          uint8_t         *Buff,
                          uint16_t         Len,
                          uint16_t        *BytesParsed,
                          btif_sbc_pcm_data_t *PcmData,
                          uint16_t         MaxPcmData,
                          float*       gains)
{
    bt_status_t sbc_ret;
    struct sbc_decord_params params;
    struct rpc_args in_args = {0};

    params.Buff = Buff;
    params.Len = Len;
    params.PcmData = PcmData;
    params.MaxPcmData = MaxPcmData;
    /* params.gains = gains; */
    params.gains = NULL;

    in_args.args = &params;
    in_args.args_len = sizeof(params);

    struct rpc_args out_args;

    struct sbc_decode_result sbc_res;
    out_args.args = &sbc_res;
    out_args.args_len = sizeof(sbc_res);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_SESHUB, "start_decoder",
                                        &in_args, &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK)
        return BT_STS_FAILED;

    *BytesParsed = sbc_res.BytesParsed;
     sbc_ret = sbc_res.result;

    return sbc_ret;
}

void btif_sbc_hello_world_dsp()
{
    struct rpc_args out_args;
    char hello_world[16];

    out_args.args = &hello_world[0];
    out_args.args_len = sizeof(hello_world);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_SESHUB, "hello_world", NULL,
                                            &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK) {
        TRACE(0, "init decoder err %d", ret);
    }
    TRACE(0, "%s, %s", __func__, hello_world);
}

static int test_who_callback(uint32_t handle, struct rpc_args *in_args,
                                                rpc_backend_complete complete)
{
    struct rpc_args out_args = {0};
    char who[16] = "sensorhub";

    TRACE(0, "%s", __func__);
    out_args.args = &who[0];
    /* out_args.args_len = strlen(hello_world); */
    out_args.args_len = 16;

    /* if (complete) */
    /*     complete(RPC_CORE_BTH_SESHUB, handle, 0, &out_args); */

    return 0;

}

static int rpc_seshub_ping_back(uint32_t handle, struct rpc_args *in_args,
                                            rpc_backend_complete complete)

{
    return rpc_ping_back(RPC_CORE_BTH_SESHUB, handle, in_args, complete);
}

static const struct rpc_backend_cmd test_cmds_callback[] = {
    {"ping",  rpc_seshub_ping_back},
    {"who",   test_who_callback},
};

void register_local_callback()
{
    int ret;

    TRACE(0, "%s", __func__);
    ret = rpc_register_backend_cmd(RPC_CORE_BTH_SESHUB, &test_cmds_callback[0],
                                                ARRAY_SIZE(test_cmds_callback));

    if (ret != RPC_OK) {
        TRACE(0, "cannot register commands");
    }

}

#ifdef BTH_M55
void btif_sbc_hello_world_m55(btif_sbc_decoder_t *Decoder)
{
    struct rpc_args out_args;

    out_args.args = &hello_world[0];
    out_args.args_len = sizeof(hello_world);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_M55, 0, NULL,
                                            &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK) {
        TRACE(0, "init decoder err %d", ret);
    }
    TRACE(0, "%s, %s", __func__, hello_world);
}
#endif

int sbc_decoder_init_rpc(void)
{
    register_local_callback();

    rpc_ping_remote(RPC_CORE_BTH_SESHUB, 10);

    btif_sbc_hello_world_dsp();

#ifdef BTH_M55
    sbc_m55_decoder_init_rpc();
    btif_sbc_hello_world_m55();
#endif
    return 0;
}

