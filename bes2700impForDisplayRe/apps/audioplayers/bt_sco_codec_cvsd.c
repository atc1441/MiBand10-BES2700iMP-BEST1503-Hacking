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
#include <string.h>
#include "plat_types.h"
#include "hal_trace.h"
#include "audio_dump.h"
#include "plc_utils.h"
#include "speech_cfg.h"
#include "bt_drv.h"
#include "bt_sco_codec.h"

#define ENABLE_LPC_PLC

// #define SPEECH_RX_PLC_DUMP_DATA

// #define PLC_DEBUG_PRINT_DATA

#if defined(CVSD_BYPASS)
#include "Pcm8k_Cvsd.h"
#endif

#define CVSD_PACKET_SIZE (120)

#define CVSD_PACKET_NUM (2)

#define CVSD_MUTE_PATTERN (0x55)

#define CVSD_PCM_SIZE (120)

#if defined(ENABLE_LPC_PLC)
#include "lpc_plc_api.h"
static LpcPlcState *plc_st = NULL;
#else
#include "plc_8000.h"
static  void *plc_st;
#endif

#ifdef SPEECH_RX_PLC_DUMP_DATA
static int16_t *audio_dump_temp_buf = NULL;
#endif

#if defined(ENABLE_LPC_PLC)
static uint8_t *cvsd_buf_all = NULL;

plc_type_t check_cvsd_mute_pattern(uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
        if (buf[i] != CVSD_MUTE_PATTERN)
            return PLC_TYPE_PASS;

    return PLC_TYPE_CONTROLLER_MUTE;
}
#endif

int32_t bt_sco_codec_cvsd_init(uint32_t sample_rate)
{
    TRACE(0, "[%s] SCO: CVSD...", __func__);

#if defined(ENABLE_LPC_PLC)
    // 120 + 120 = 240 bytes
    cvsd_buf_all = (uint8_t *)speech_calloc(CVSD_PACKET_SIZE + CVSD_OFFSET_BYTES, sizeof(uint8_t));
    memset(cvsd_buf_all, CVSD_MUTE_PATTERN & 0xFF, (CVSD_PACKET_SIZE + CVSD_OFFSET_BYTES) * sizeof(uint8_t));
#endif

#if defined(CVSD_BYPASS)
    Pcm8k_CvsdInit();
#endif

#if defined(ENABLE_LPC_PLC)
    plc_st = lpc_plc_create(sample_rate);
#else
    plc_st = (PlcSt_8000 *)speech_plc_8000_init(voicebtpcm_get_ext_buff);
#endif

#ifdef SPEECH_RX_PLC_DUMP_DATA
    audio_dump_temp_buf = (int16_t *)speech_calloc(CVSD_PACKET_SIZE / sizeof(int16_t), sizeof(int16_t));     // 240 Bytes
    audio_dump_init(CVSD_PACKET_SIZE / sizeof(int16_t), sizeof(int16_t), 3);
#endif

    return 0;
}

int32_t bt_sco_codec_cvsd_deinit(void)
{
    TRACE(0, "[%s] ...", __func__);

#ifdef SPEECH_RX_PLC_DUMP_DATA
    speech_free(audio_dump_temp_buf);
#endif

#if defined(ENABLE_LPC_PLC)
    lpc_plc_destroy(plc_st);
#endif

#if defined(ENABLE_LPC_PLC)
    speech_free(cvsd_buf_all);
#endif

    return 0;
}

// cvsd_size = 240, pcm_size = 240
int32_t bt_sco_codec_cvsd_encoder(uint8_t *cvsd_buf, uint32_t cvsd_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // TRACE(0, "[%s] cvsd_size: %d, pcm_size: %d", __func__, cvsd_size, pcm_size);

    ASSERT(cvsd_size / CVSD_PACKET_SIZE == pcm_size / CVSD_PCM_SIZE,
        "[%s] cvsd_size = %d, pcm_size = %d", __func__, cvsd_size, pcm_size);

#if defined(CVSD_BYPASS)
    Pcm8kToCvsd((short *)pcm_buf, cvsd_buf, pcm_size/2);
#else
    memcpy(cvsd_buf, pcm_buf, pcm_size);
#endif

    return 0;
}

extern uint32_t btdrv_syn_get_curr_ticks(void);
int32_t bt_sco_codec_cvsd_decoder(uint8_t *cvsd_buf, uint32_t cvsd_size, uint8_t *pcm_buf, uint32_t pcm_size)
{
    // TRACE(0, "[%s] cvsd_size: %d, pcm_size: %d", __func__, cvsd_size, pcm_size);

    ASSERT(cvsd_size / CVSD_PACKET_SIZE == pcm_size / CVSD_PCM_SIZE,
        "[%s] cvsd_size = %d, pcm_size = %d", __func__, cvsd_size, pcm_size);

#if defined(ENABLE_LPC_PLC)
    ASSERT(cvsd_size % CVSD_PACKET_SIZE == 0, "[%s] cvsd input length(%d) error", __FUNCTION__, cvsd_size);
    for (uint32_t i = 0; i < cvsd_size; i += CVSD_PACKET_SIZE) {
        memcpy(&cvsd_buf_all[CVSD_OFFSET_BYTES], cvsd_buf, CVSD_PACKET_SIZE);
        memcpy(cvsd_buf, cvsd_buf_all, CVSD_PACKET_SIZE);
        memcpy(cvsd_buf_all, &cvsd_buf_all[CVSD_PACKET_SIZE], CVSD_OFFSET_BYTES);

        //DUMP16("0x%x, ", cvsd_buf, CVSD_PACKET_SIZE / 2);

#ifdef SPEECH_RX_PLC_DUMP_DATA
        audio_dump_add_channel_data(2, (short *)cvsd_buf, CVSD_PACKET_SIZE / 2);
#endif

        plc_type_t plc_type = check_cvsd_mute_pattern(cvsd_buf, CVSD_PACKET_SIZE);

        if (plc_type != PLC_TYPE_PASS) {
            memset(pcm_buf, 0, CVSD_PCM_SIZE);
            goto do_plc;
        }

#if defined(CVSD_BYPASS)
        CvsdToPcm8k(cvsd_buf, (short *)(pcm_buf), CVSD_PACKET_SIZE/2, 0);
#else
        memcpy(pcm_buf, cvsd_buf, CVSD_PACKET_SIZE);
#endif

do_plc:
        if (plc_type == PLC_TYPE_PASS) {
            lpc_plc_save(plc_st, (int16_t *)pcm_buf);
#ifdef SPEECH_RX_PLC_DUMP_DATA
            audio_dump_add_channel_data(0, (short *)pcm_buf, CVSD_PACKET_SIZE / 2);
#endif
        } else {
            //TRACE(1,"PLC bad frame, plc type: %d, curr clk=%x, bt_clk=%x", plc_type, btdrv_syn_get_curr_ticks(), bt_syn_get_curr_ticks(conhdl));
            TRACE(1,"PLC bad frame, plc type: %d, curr clk=%x", plc_type, btdrv_syn_get_curr_ticks());
#if defined(PLC_DEBUG_PRINT_DATA)
            DUMP16("0x%x, ", cvsd_buf, CVSD_PACKET_SIZE / 2);
#endif
            lpc_plc_generate(plc_st, (int16_t *)pcm_buf, NULL);
#ifdef SPEECH_RX_PLC_DUMP_DATA
            for (uint32_t i = 0; i < CVSD_PACKET_SIZE / 2; i++) {
                audio_dump_temp_buf[i] = (plc_type - 1) * 5000;
            }
            audio_dump_add_channel_data(0, audio_dump_temp_buf, CVSD_PACKET_SIZE / 2);
#endif
        }

#ifdef SPEECH_RX_PLC_DUMP_DATA
        audio_dump_add_channel_data(1, (short *)pcm_buf, CVSD_PACKET_SIZE / 2);
        audio_dump_run();
#endif

        cvsd_buf += CVSD_PACKET_SIZE;
        pcm_buf += CVSD_PCM_SIZE;
    }
#else
#if defined(CVSD_BYPASS)
    CvsdToPcm8k(cvsd_buf, (short *)(pcm_buf), cvsd_size/2, 0);
#else
    memcpy(pcm_buf,cvsd_buf,cvsd_size);
#endif
#ifndef ENABLE_LPC_PLC
    speech_plc_8000((PlcSt_8000 *)plc_st, (short *)pcm_buf, pcm_size);
#endif
#endif

    return 0;
}

bt_sco_codec_t bt_sco_codec_cvsd = {
    .init       = bt_sco_codec_cvsd_init,
    .deinit     = bt_sco_codec_cvsd_deinit,
    .encoder    = bt_sco_codec_cvsd_encoder,
    .decoder    = bt_sco_codec_cvsd_decoder
};