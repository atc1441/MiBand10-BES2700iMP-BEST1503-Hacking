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
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_cmu.h"
#include "rpc.h"
#include "sbc.h"
#include "codec_sbc.h"
#include "cmsis_os.h"
#include "rpc_ping.h"
#include "rpc_sbc_backend.h"

static sbc_decoder_t sbc_decoder;

static int rpc_sbc_init(uint32_t handle, struct rpc_args *in_args,
                                    rpc_backend_complete complete)
{
    TRACE(0, "%s", __func__);
    sbc_decoder_init(&sbc_decoder);

    /* sbc_decoder.streamInfo.sampleFreq = 0x55; */
    TRACE(0, "sampleFreq %x", sbc_decoder.streamInfo.sampleFreq);

    struct rpc_args out_args = {0};
    out_args.args = &sbc_decoder.streamInfo.sampleFreq;
    out_args.args_len = sizeof(sbc_decoder.streamInfo.sampleFreq);
    /* out_args->args = &sbc_decoder; */
    /* out_args->args_len = sizeof(sbc_decoder_t); */
    TRACE(0, "decoder len %d", out_args.args_len);
    if (complete)
        complete(RPC_CORE_BTH_SESHUB, handle, 0, &out_args);

    return 0;
}

#define CFG_HW_AUD_EQ_NUM_BANDS (8)
static float media_sbc_eq_band_gain[CFG_HW_AUD_EQ_NUM_BANDS];


static int rpc_sbc_decode_frame(uint32_t handle, struct rpc_args *in_args,
                                                rpc_backend_complete complete)
{
    struct sbc_decord_params *params;

    params = (struct sbc_decord_params *)in_args;
    uint8_t *Buff = params->Buff;
    uint16_t Len = params->Len;
    sbc_pcm_data_t *PcmData = (sbc_pcm_data_t *)params->PcmData;
    uint16_t         MaxPcmData = params->MaxPcmData;
    /* float*       gains = params->gains; */
    uint16_t BytesParsed = 0;
    bt_status_t ret;

    /* TRACE_IMM(0, "buff %p, len %d, PcmData %p MaxPcmData %d", Buff, Len, PcmData, MaxPcmData); */
    ret = sbc_frames_decode(&sbc_decoder, Buff, Len, &BytesParsed, PcmData,
                                    MaxPcmData, &media_sbc_eq_band_gain[0]);

    struct rpc_args out_args = {0};
    struct sbc_decode_result sbc_res;

    sbc_res.result = ret;
    sbc_res.BytesParsed = BytesParsed;
    /* TRACE_IMM(0, "BytesParsed %d", BytesParsed); */
    out_args.args = &sbc_res;
    out_args.args_len = sizeof(sbc_res);

    if (complete)
        complete(RPC_CORE_BTH_SESHUB, handle, 0, &out_args);

    return 0;
}

static int rpc_hello_world(uint32_t handle, struct rpc_args *in_args,
                                                rpc_backend_complete complete)
{
    struct rpc_args out_args = {0};
    char hello_world[16] = "hello world";

    TRACE(0, "%s", __func__);
    out_args.args = &hello_world[0];
    /* out_args.args_len = strlen(hello_world); */
    out_args.args_len = 16;

    if (complete)
        complete(RPC_CORE_BTH_SESHUB, handle, 0, &out_args);

    return 0;

}
static int rpc_seshub_ping_back(uint32_t handle, struct rpc_args *in_args,
                                            rpc_backend_complete complete)

{
    return rpc_ping_back(RPC_CORE_BTH_SESHUB, handle, in_args, complete);
}

static const struct rpc_backend_cmd sbc_commands[] = {
    {"ping",          rpc_seshub_ping_back},
    {"hello_world",   rpc_hello_world},
    {INIT_CMD_NAME, rpc_sbc_init},
    {DECODE_FRAME_CMD_NAME, rpc_sbc_decode_frame},
};

void test_who()
{
    struct rpc_args out_args;
    char who[16];

    out_args.args = &who[0];
    out_args.args_len = sizeof(who);

    rpc_status_t ret = rpc_exec_cmd_sync(RPC_CORE_BTH_SESHUB, "who", NULL,
                                            &out_args, RPC_WAIT_FOREVER);
    if (ret != RPC_OK) {
        TRACE(0, "test who err %d", ret);
    }
    TRACE(0, "%s, %s", __func__, who);
}


static void test_remote_commands(void)
{
    rpc_ping_remote(RPC_CORE_BTH_SESHUB, 5);

    test_who();
}

void rpc_start_loop(uint32_t unused);

int sbc_decoder_open(void)
{
    rpc_status_t ret;

    TRACE(0, "%s", __func__);
    for (int i = 0; i < CFG_HW_AUD_EQ_NUM_BANDS; i++){
        media_sbc_eq_band_gain[i] = 1.0;
    }

#ifdef BTH_M55
    ret = rpc_open_backend(RPC_CORE_BTH_M55);
#else
    ret = rpc_open_backend(RPC_CORE_BTH_SESHUB);
#endif
    if (ret != RPC_OK) {
        TRACE(0, "cannot open server");
        return -1;
    }

#ifdef BTH_M55
    ret = rpc_register_backend_cmd(RPC_CORE_BTH_M55, &sbc_commands[0],
                                                ARRAY_SIZE(sbc_commands));
#else
    ret = rpc_register_backend_cmd(RPC_CORE_BTH_SESHUB, &sbc_commands[0],
                                                ARRAY_SIZE(sbc_commands));
#endif

    if (ret != RPC_OK) {
        TRACE(0, "cannot register commands");
    }


#if defined(RTOS)
    test_remote_commands();
#endif

#if !defined(RTOS)
    rpc_start_loop(RPC_CORE_BTH_SESHUB);
#endif

    return 0;
}

