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
#include "audio_test_defs.h"
#if defined(AUDIO_TEST_HIFI)
#include "cmsis_os.h"
#include "app_utils.h"
#include "audioflinger.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "string.h"
#include "hal_sys2bth.h"

typedef enum {
    SCO_DSP_CMD_NULL = 0,
    SCO_DSP_CMD_OPEN,
    SCO_DSP_CMD_CLOSE,
    SCO_DSP_CMD_PROCESS,
    SCO_DSP_CMD_QTY
} sco_dsp_cmd_t;

typedef enum {
    SCO_DSP_STATUS_OPEN,
    SCO_DSP_STATUS_CLOSE,
    SCO_DSP_STATUS_QTY
} sco_dsp_status_t;

typedef struct {
    int sample_rate;
    int frame_len;
    int channel_num;
} sco_dsp_cfg_t;

typedef struct {
    void    *mic;
    void    *ref;
    int     *len;
} sco_dsp_pcm_t;

typedef struct {
    sco_dsp_cmd_t cmd;
    union {
        sco_dsp_cfg_t cfg;
        sco_dsp_pcm_t pcm;
    } data;
} sco_dsp_data_t;

static uint32_t g_debug_tx_cnt = 0;
static uint32_t g_debug_rx_cnt = 0;

static volatile uint32_t g_rx_done = false;

static unsigned int bth_dsp_rx_data_handler(const void *data, unsigned int len)
{
    g_rx_done = true;

    return len;
}

static void bth_dsp_tx_data_done_handler(const void* data, unsigned int len)
{
    TRACE(0, "[%s] len=%d", __func__, len);
}

static int bth_dsp_tx_data(const void *data, uint32_t len)
{
    return hal_sys2bth_send(HAL_SYS2BTH_ID_0, data, len);
}

static void bth_dsp_core_open(void)
{
    int ret;
    ret = hal_sys2bth_open(HAL_SYS2BTH_ID_0, bth_dsp_rx_data_handler, bth_dsp_tx_data_done_handler, false, false);
    ASSERT(ret == 0, "hal_sys2bth_opened failed: %d", ret);
    ret = hal_sys2bth_start_recv(HAL_SYS2BTH_ID_0);
    ASSERT(ret == 0, "hal_sys2bth_start_recv failed: %d", ret);
}

static void bth_dsp_core_close(void)
{
    hal_sys2bth_close(HAL_SYS2BTH_ID_0);
}

int speech_algo_dsp_init(sco_dsp_cfg_t *cfg, uint32_t len)
{
    TRACE(0, "[%s] len=%d", __func__, len);

    g_debug_tx_cnt = 0;
    g_debug_rx_cnt = 0;

    // bth_dsp_core_open();

    return 0;
}

int speech_algo_dsp_deinit(void)
{
    TRACE(0,"%s...", __func__);

    bth_dsp_core_close();

    return 0;
}

int32_t audio_test_hifi_tx_data(void)
{
    uint8_t data[4] = {0, 1, 2, 3};
    bth_dsp_tx_data(data, sizeof(data));

    return 0;
}

int32_t audio_test_hifi_init(void)
{
    bth_dsp_core_open();

    return 0;
}
#endif
