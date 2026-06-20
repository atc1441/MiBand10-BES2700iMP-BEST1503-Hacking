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
#ifndef __ANALOG_H__
#define __ANALOG_H__

#ifdef __cplusplus
extern "C" {
#endif
#include "stdbool.h"
#include "hal_analogif.h"
#include "hal_cmu.h"
#include "hal_aud.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(analog)
#ifdef DONGLE_SUPPORT
#include CHIP_SPECIFIC_HDR(dongle)
#endif

#define ANALOG_DEBUG_TRACE(n, s, ...)       TR_DUMMY(n, s, ##__VA_ARGS__)
#define ANALOG_INFO_TRACE_IMM(n, s, ...)    TR_INFO((n) | TR_ATTR_IMM, s, ##__VA_ARGS__)
#define ANALOG_INFO_TRACE(n, s, ...)        TR_INFO(n, s, ##__VA_ARGS__)

#ifndef ISPI_ANA_REG
#define ISPI_ANA_REG(reg)                   (reg)
#endif
#define analog_read(reg, val)               hal_analogif_reg_read(ISPI_ANA_REG(reg), val)
#define analog_write(reg, val)              hal_analogif_reg_write(ISPI_ANA_REG(reg), val)

#define FLOAT_TO_PPB_INT(f)                 ((int)(f * 1000 * 1000 * 1000))

#ifndef ANA_AUD_PLL_USER_T
enum ANA_AUD_PLL_USER_T {
    ANA_AUD_PLL_USER_CODEC      = (1 << 0),
    ANA_AUD_PLL_USER_I2S        = (1 << 1),
    ANA_AUD_PLL_USER_SPDIF      = (1 << 2),
    ANA_AUD_PLL_USER_PCM        = (1 << 3),
    ANA_AUD_PLL_USER_ANC_IIR    = (1 << 4),

    ANA_AUD_PLL_USER_END        = (1 << 5),
};
#endif

enum ANA_DAC_DC_CALIB_MODE_T {
    ANA_DAC_DC_CALIB_MODE_ADC_ONLY,
    ANA_DAC_DC_CALIB_MODE_DAC_TO_ADC,
    ANA_DAC_DC_CALIB_MODE_NORMAL,
};

void analog_aud_freq_pll_config(uint32_t freq, uint32_t div);

void analog_aud_get_dc_calib_value(int16_t *dc_l, int16_t *dc_r);

void analog_open(void);

void analog_aud_xtal_tune(float ratio);

void analog_aud_pll_tune(float ratio);

void analog_aud_pll_open(enum ANA_AUD_PLL_USER_T user);

void analog_aud_pll_close(enum ANA_AUD_PLL_USER_T user);

void analog_aud_set_dac_gain(int32_t v);

uint32_t analog_codec_get_dac_gain(void);

uint32_t analog_codec_dac_gain_to_db(int32_t gain);

int32_t analog_codec_dac_max_attn_db(void);

void analog_aud_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int16_t offset_l, int16_t offset_r);

void analog_aud_restore_anc_dyn_adc_gain(enum ANC_TYPE_T type);

void analog_aud_apply_adc_gain_offset(enum AUD_CHANNEL_MAP_T ch_map, int16_t offset);

void analog_aud_apply_dyn_adc_gain(enum AUD_CHANNEL_MAP_T ch_map, int16_t gain);

void analog_aud_codec_open(void);

void analog_aud_codec_close(void);

void analog_aud_codec_mute(void);

void analog_aud_codec_nomute(void);

void analog_aud_codec_adc_enable(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map, bool en);

void analog_aud_codec_adc2_enable(enum AUD_IO_PATH_T input_path, enum AUD_CHANNEL_MAP_T ch_map, bool en);

void analog_aud_codec_dac_enable(bool en);

void analog_aud_codec_speaker_enable(bool en);

void analog_aud_codec_anc_enable(enum ANC_TYPE_T type, bool en);

void analog_aud_mickey_enable(bool en);

void analog_sleep(void);

void analog_wakeup(void);

int analog_debug_config_audio_output(bool diff);

int analog_debug_config_codec(uint16_t mv);

int analog_debug_config_low_power_adc(bool enable);

void analog_debug_config_anc_calib_mode(bool enable);

bool analog_debug_get_anc_calib_mode(void);

void analog_productiontest_settings_checker(void);

bool analog_aud_dc_calib_valid(void);

uint16_t analog_aud_dac_dc_diff_to_val(int32_t diff);

uint16_t analog_aud_dc_calib_val_to_efuse(uint16_t val);

int16_t analog_aud_dac_dc_get_step(void);

void analog_aud_save_dc_calib(uint16_t val);

void analog_aud_dc_calib_set_value(uint16_t dc_l, uint16_t dc_r);

void analog_aud_dc_calib_get_cur_value(uint16_t *dc_l, uint16_t *dc_r);

void analog_aud_dc_calib_get_fixed_comp_value(uint32_t dre_gain, int32_t *dc_l, int32_t *dc_r);

bool analog_aud_dc_calib_get_large_ana_dc_value(int16_t *ana_dc, int cur_dig_dc, int tgt_dig_dc,
    int chan, bool init);

void analog_aud_dc_calib_enable(bool en);

void analog_aud_dac_dc_auto_calib_enable(void);

void analog_aud_dac_dc_auto_calib_disable(void);

void analog_aud_dac_dc_auto_calib_set_mode(enum ANA_DAC_DC_CALIB_MODE_T mode);

void analog_capsensor_init(void);

void analog_capsensor_deinit(void);

int analog_rc_clock_calib(void);

void analog_reg_val_print(void);

#ifdef __cplusplus
}
#endif

#endif
