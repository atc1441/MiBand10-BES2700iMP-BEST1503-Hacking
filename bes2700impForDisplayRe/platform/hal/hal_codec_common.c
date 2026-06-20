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
#include "cmsis.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "tgt_hardware.h"

extern const CODEC_ADC_VOL_T codec_adc_vol[TGT_ADC_VOL_LEVEL_QTY];

#ifndef CODEC_MIC_CH0_SADC_VOL
#define CODEC_MIC_CH0_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH1_SADC_VOL
#define CODEC_MIC_CH1_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH2_SADC_VOL
#define CODEC_MIC_CH2_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH3_SADC_VOL
#define CODEC_MIC_CH3_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH4_SADC_VOL
#define CODEC_MIC_CH4_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH5_SADC_VOL
#define CODEC_MIC_CH5_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH6_SADC_VOL
#define CODEC_MIC_CH6_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_MIC_CH7_SADC_VOL
#define CODEC_MIC_CH7_SADC_VOL              CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH0_SADC_VOL
#define CODEC_DIGMIC_CH0_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH1_SADC_VOL
#define CODEC_DIGMIC_CH1_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH2_SADC_VOL
#define CODEC_DIGMIC_CH2_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH3_SADC_VOL
#define CODEC_DIGMIC_CH3_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH4_SADC_VOL
#define CODEC_DIGMIC_CH4_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH5_SADC_VOL
#define CODEC_DIGMIC_CH5_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH6_SADC_VOL
#define CODEC_DIGMIC_CH6_SADC_VOL           CODEC_SADC_VOL
#endif
#ifndef CODEC_DIGMIC_CH7_SADC_VOL
#define CODEC_DIGMIC_CH7_SADC_VOL           CODEC_SADC_VOL
#endif

static const uint8_t codec_mic_chan_vol[] = {
    CODEC_MIC_CH0_SADC_VOL,    CODEC_MIC_CH1_SADC_VOL,    CODEC_MIC_CH2_SADC_VOL,    CODEC_MIC_CH3_SADC_VOL,
    CODEC_MIC_CH4_SADC_VOL,    CODEC_MIC_CH5_SADC_VOL,    CODEC_MIC_CH6_SADC_VOL,    CODEC_MIC_CH7_SADC_VOL,
    CODEC_DIGMIC_CH0_SADC_VOL, CODEC_DIGMIC_CH1_SADC_VOL, CODEC_DIGMIC_CH2_SADC_VOL, CODEC_DIGMIC_CH3_SADC_VOL,
    CODEC_DIGMIC_CH4_SADC_VOL, CODEC_DIGMIC_CH5_SADC_VOL, CODEC_DIGMIC_CH6_SADC_VOL, CODEC_DIGMIC_CH7_SADC_VOL,
};

static const struct CODEC_DAC_VOL_T *codec_dac_vol_table = codec_dac_vol;
static uint32_t codec_dac_vol_num = ARRAY_SIZE(codec_dac_vol);

uint32_t hal_codec_get_input_path_cfg(enum AUD_IO_PATH_T io_path)
{
    int i;

    for (i = 0; i < CFG_HW_AUD_INPUT_PATH_NUM; i++) {
        if (io_path == cfg_audio_input_path_cfg[i].io_path) {
            return cfg_audio_input_path_cfg[i].cfg;
        }
    }
    ASSERT(false, "%s: Bad input path: %d", __func__, io_path);
    return 0;
}

uint32_t hal_codec_get_all_input_path_cfg(void)
{
    int i;
    uint32_t cfg = 0;

    for (i = 0; i < CFG_HW_AUD_INPUT_PATH_NUM; i++) {
        cfg |= cfg_audio_input_path_cfg[i].cfg;
    }
    return cfg;
}

uint32_t hal_codec_get_input_map_chan_num(uint32_t ch_map)
{
    int i;

    ch_map &= AUD_CHANNEL_MAP_ALL;

    i = 0;
    while (ch_map) {
        if (ch_map & 0x1) {
            i++;
        }
        ch_map >>= 1;
    }
    return i;
}

uint32_t hal_codec_get_input_path_chan_num(enum AUD_IO_PATH_T io_path)
{
    uint32_t ch_map;

    ch_map = hal_codec_get_input_path_cfg(io_path);

    return hal_codec_get_input_map_chan_num(ch_map);
}

void hal_codec_set_dac_volume_table(const struct CODEC_DAC_VOL_T *table_ptr, uint32_t table_num)
{
    if (table_ptr == NULL) {
        TRACE(0, "[%s] Set default volume table", __func__);
        codec_dac_vol_table = codec_dac_vol;
        codec_dac_vol_num = ARRAY_SIZE(codec_dac_vol);
    } else {
        TRACE(0, "[%s] Set new volume table", __func__);
        codec_dac_vol_table = table_ptr;
        codec_dac_vol_num = table_num;
    }
}

uint32_t hal_codec_get_default_dac_volume_index(void)
{
#ifdef AUDIO_OUTPUT_VOLUME_DEFAULT
    if (TGT_VOLUME_LEVEL_MUTE <= AUDIO_OUTPUT_VOLUME_DEFAULT &&
            AUDIO_OUTPUT_VOLUME_DEFAULT <= TGT_VOLUME_LEVEL_MAX) {
        return AUDIO_OUTPUT_VOLUME_DEFAULT;
    }
#endif
    return TGT_VOLUME_LEVEL_MAX;
}

const struct CODEC_DAC_VOL_T *hal_codec_get_dac_volume(uint32_t index)
{
    ASSERT(codec_dac_vol_table != NULL, "[%s] codec_dac_vol_table = NULL", __func__);

    if (index < codec_dac_vol_num) {
        return &codec_dac_vol_table[index];
    } else {
        return &codec_dac_vol_table[codec_dac_vol_num-1];
    }
}

const CODEC_ADC_VOL_T *hal_codec_get_adc_volume(uint32_t index)
{
    uint32_t _array_sizeof_vol = ARRAY_SIZE(codec_adc_vol);
    if (index < _array_sizeof_vol) {
        return &codec_adc_vol[index];
    } else {
        return &codec_adc_vol[_array_sizeof_vol-1];
    }
}

uint32_t hal_codec_get_mic_chan_volume_level(uint32_t map)
{
    uint32_t mic_ch;

    mic_ch = get_lsb_pos(map);

    if (mic_ch < ARRAY_SIZE(codec_mic_chan_vol)) {
        return codec_mic_chan_vol[mic_ch];
    } else {
        return ARRAY_SIZE(codec_adc_vol);
    }
}

uint8_t hal_codec_get_digmic_hw_index(uint8_t chan)
{
    return chan - get_lsb_pos(AUD_CHANNEL_MAP_DIGMIC_CH0);
}

uint32_t hal_codec_get_real_sample_rate(enum AUD_SAMPRATE_T rate, bool resamp)
{
    uint32_t div, remain;

    div = CODEC_FREQ_24P576M / rate;
    remain = CODEC_FREQ_24P576M % rate;
    if (remain == 0) {
        if (resamp) {
            return CODEC_FREQ_CRYSTAL / div;
        } else {
            return rate;
        }
    }

    div = CODEC_FREQ_22P5792M / rate;
    remain = CODEC_FREQ_22P5792M % rate;
    if (CODEC_FREQ_22P5792M % rate == 0) {
        if (resamp) {
            return CODEC_FREQ_CRYSTAL / div;
        } else {
            return rate;
        }
    }

    // Resample
#if 0
    if (CODEC_FREQ_CRYSTAL == CODEC_FREQ_26M) {
        return rate;
    }

    div = CODEC_FREQ_26M / rate;
    return CODEC_FREQ_CRYSTAL / div;
#else
    // div must be a multiple of 32
    div = CODEC_FREQ_26M / rate;
    if (div % 32) {
        div = CODEC_FREQ_24M / rate;
    }

    return CODEC_FREQ_CRYSTAL / div;
#endif
}

