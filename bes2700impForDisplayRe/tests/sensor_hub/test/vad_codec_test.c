/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#if defined(VAD_CODEC_TEST) || defined(VD_TEST)
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "analog.h"
#include "audioflinger.h"
#include "sens_comp_test.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "sensor_hub.h"
#include "sensor_hub_core.h"
#include "sens_msg.h"
#include "string.h"

#define ALIGNED4                        ALIGNED(4)

#define NON_EXP_ALIGN(val, exp)         (((val) + ((exp) - 1)) / (exp) * (exp))

#define AF_DMA_LIST_CNT                 4

#define CHAN_NUM_CAPTURE                1

#define RX_BURST_NUM                    4
#define RX_ALL_CH_DMA_BURST_SIZE        (RX_BURST_NUM * 2 * CHAN_NUM_CAPTURE)
#define RX_BUFF_ALIGN                   (RX_ALL_CH_DMA_BURST_SIZE * AF_DMA_LIST_CNT)

#define CAPTURE_SAMP_RATE               16000 //48000

#define CAPTURE_FRAME_SIZE              (CAPTURE_SAMP_RATE / 1000 * 2 * CHAN_NUM_CAPTURE)
#define CAPTURE_SIZE                    NON_EXP_ALIGN(CAPTURE_FRAME_SIZE * 8, RX_BUFF_ALIGN)

static uint8_t ALIGNED4 capture_buf[CAPTURE_SIZE];

static enum ANALOG_RPC_ID_T cur_ana_rpc;
static volatile bool wait_rpc_reply;

static struct ANA_RPC_REQ_MSG_T ana_rpc_req;

static unsigned int vad_codec_test_rx_handler(const void *data, unsigned int len)
{
    struct SENS_MSG_HDR_T *hdr;
    struct ANA_RPC_REPLY_MSG_T *ana_rpc_reply;

    //TR_INFO(0, "%s: data=%p len=%u", __func__, data, len);

    ASSERT(len >= 2, "%s: Bad len=%u", __func__, len);

    hdr = (struct SENS_MSG_HDR_T *)data;
    if (hdr->reply) {
        if (hdr->id == SENS_MSG_ID_ANA_RPC) {
            ASSERT(wait_rpc_reply, "%s: Rx ana rpc reply msg while not waiting it: id=%d", __func__, hdr->id);
            ana_rpc_reply = (struct ANA_RPC_REPLY_MSG_T *)data;
            if (ana_rpc_reply->rpc_id == cur_ana_rpc) {
                wait_rpc_reply = false;
            } else {
                ASSERT(false, "%s: Waiting ana rpc reply msg for %d but rx %d", __func__, cur_ana_rpc, hdr->id);
            }
        } else {
            ASSERT(false, "%s: Bad reply msg: id=%d", __func__, hdr->id);
        }
    } else {
        ASSERT(false, "%s: Bad msg: id=%d", __func__, hdr->id);
    }

    return len;
}

static void vad_codec_test_tx_handler(const void *data, unsigned int len)
{
    //TR_INFO(0, "%s: data=%p len=%u", __func__, data, len);
}

static int analog_rpc(enum ANALOG_RPC_ID_T id, uint32_t param0, uint32_t param1, uint32_t param2)
{
    int ret;

    TR_INFO(0, "%s: id=%d param=0x%X/0x%X/0x%X", __func__, id, param0, param1, param2);

    cur_ana_rpc = id;
    wait_rpc_reply = true;

    memset(&ana_rpc_req, 0, sizeof(ana_rpc_req));
    ana_rpc_req.hdr.id = SENS_MSG_ID_ANA_RPC;
    ana_rpc_req.rpc_id = id;
    ana_rpc_req.param[0] = param0;
    ana_rpc_req.param[1] = param1;
    ana_rpc_req.param[2] = param2;

    ret = sensor_hub_send(&ana_rpc_req, sizeof(ana_rpc_req));
    ASSERT(ret == 0, "%s: sensor_hub_send failed: %d", __func__, ret);

    while (wait_rpc_reply);

    return 0;
}

static int af_rpc(enum AUD_STREAM_ID_T id, uint8_t *buf, uint32_t len)
{
    struct AF_RPC_REQ_MSG_T af_req;
    int ret;
    unsigned int seq;

    //TR_INFO(0, "%s: id=%d buf=%p len=%u", __func__, id, buf, len);

    memset(&af_req, 0, sizeof(af_req));
    af_req.hdr.id = SENS_MSG_ID_AF_BUF;
    af_req.stream_id = id;
    af_req.buf = buf;
    af_req.len = len;

    ret = sensor_hub_send_seq(&af_req, sizeof(af_req), &seq);
    ASSERT(ret == 0, "%s: sensor_hub_send_seq failed: %d", __func__, ret);

    while (sensor_hub_tx_active(seq));

    return 0;
}

static uint32_t af_capture(uint8_t *buf, uint32_t len)
{
    af_rpc(AUD_STREAM_ID_0, buf, len);
    return 0;
}

static void vad_codec_test_cmd_loop(void)
{
}

void vad_test_env_init(void)
{
    sensor_hub_core_register_rx_irq_handler(vad_codec_test_rx_handler);
    sensor_hub_core_register_tx_done_irq_handler(vad_codec_test_tx_handler);

    analog_aud_register_rpc_callback(analog_rpc);
}

void vad_codec_test(void)
{
    int ret = 0;
    struct AF_STREAM_CONFIG_T stream_cfg;

    vad_test_env_init();

    af_open();

    memset(&stream_cfg, 0, sizeof(stream_cfg));

    stream_cfg.bits = AUD_BITS_16;
    stream_cfg.channel_num = CHAN_NUM_CAPTURE;
    stream_cfg.channel_map = 0;
    stream_cfg.chan_sep_buf = false;
    stream_cfg.sample_rate = CAPTURE_SAMP_RATE;
    stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
    stream_cfg.vol = TGT_ADC_VOL_LEVEL_7;
    stream_cfg.handler = af_capture;
    stream_cfg.io_path = AUD_INPUT_PATH_VADMIC;
    stream_cfg.data_ptr = capture_buf;
    stream_cfg.data_size = CAPTURE_SIZE;

    ret = af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
    ASSERT(ret == 0, "af_stream_open capture failed: %d", ret);

    ret = af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
    ASSERT(ret == 0, "af_stream_start capture failed: %d", ret);

    while (1) {
        vad_codec_test_cmd_loop();

#ifdef RTOS
        osDelay(2);
#else
        extern void af_thread(void const *argument);
        af_thread(NULL);

        hal_sleep_enter_sleep();
#endif
    }

    return;
}

#endif

