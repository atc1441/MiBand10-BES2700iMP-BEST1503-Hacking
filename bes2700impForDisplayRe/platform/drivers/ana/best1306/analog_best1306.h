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
#ifndef __ANALOG_BEST1306_H__
#define __ANALOG_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#define ISPI_ANA_REG(r)                     (((r) & 0xFFF) | 0x1000)

#define MAX_ANA_MIC_CH_NUM                  2

void analog_aud_pll_set_dig_div(uint32_t div);

uint32_t analog_aud_get_max_dre_gain(void);

void analog_aud_codec_anc_boost(bool en);

int analog_debug_config_vad_mic(bool enable);

void analog_aud_classd_pa_enable(bool en);

void analog_audio_codec_lower_power_mode(void);

void analog_aud_adc_dc_auto_calib_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on);

void analog_aud_adc_dc_calib_set_step(enum AUD_CHANNEL_MAP_T ch_map, uint16_t step);

void analog_aud_adc_dc_calib_offset_update_enable(enum AUD_CHANNEL_MAP_T ch_map, bool on);

void analog_aud_adc_dc_calib_offset_update(enum AUD_CHANNEL_MAP_T ch_map, uint16_t value);

#ifdef __cplusplus
}
#endif

#endif

