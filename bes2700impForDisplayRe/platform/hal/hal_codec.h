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
#ifndef __HAL_CODEC_H__
#define __HAL_CODEC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_aud.h"

#define DAC_DC_VALID_MARK                   (0xDAC0DC00)
#define ADC_DC_VALID_MARK                   (0xADC00000)

#define DB_TO_QDB(n)                        ((n) * 4)
#define QDB_TO_DB(n)                        ((n) / 4)

enum HAL_CODEC_ID_T {
    HAL_CODEC_ID_0 = 0,
    HAL_CODEC_ID_NUM,
};

enum HAL_CODEC_CONFIG_FLAG_T{
    HAL_CODEC_CONFIG_NULL = 0x00,

    HAL_CODEC_CONFIG_BITS = 0x01,
    HAL_CODEC_CONFIG_SAMPLE_RATE = 0x02,
    HAL_CODEC_CONFIG_CHANNEL_NUM = 0x04,
    HAL_CODEC_CONFIG_CHANNEL_MAP = 0x08,
    HAL_CODEC_CONFIG_VOL = 0x10,

    HAL_CODEC_CONFIG_ALL = 0xff,
};

enum HAL_CODEC_DAC_RESET_STAGE_T {
    HAL_CODEC_DAC_PRE_RESET,
    HAL_CODEC_DAC_POST_RESET,
};

enum HAL_CODEC_SYNC_TYPE_T {
    HAL_CODEC_SYNC_TYPE_NONE,
    HAL_CODEC_SYNC_TYPE_GPIO,
    HAL_CODEC_SYNC_TYPE_BT,
    HAL_CODEC_SYNC_TYPE_WIFI,
};

enum HAL_CODEC_PERF_TEST_POWER_T {
    HAL_CODEC_PERF_TEST_30MW,
    HAL_CODEC_PERF_TEST_10MW,
    HAL_CODEC_PERF_TEST_5MW,
    HAL_CODEC_PERF_TEST_M60DB,

    HAL_CODEC_PERF_TEST_QTY
};

enum HAL_CODEC_TIMER_TRIG_MODE_T {
    HAL_CODEC_TIMER_TRIG_MODE_DAC,
    HAL_CODEC_TIMER_TRIG_MODE_ADC,
    HAL_CODEC_TIMER_TRIG_MODE_ANY,

    HAL_CODEC_TIMER_TRIG_MODE_QTY,
};

enum HAL_CODEC_PSAP_MODE_T {
    HAL_CODEC_PSAP_MODE_ADC = 0,
    HAL_CODEC_PSAP_MODE_ADC_DAC,
    HAL_CODEC_PSAP_MODE_DAC,
    HAL_CODEC_PSAP_MODE_DRC_LIM,
};

#if defined(CODEC_ECHO_PATH_VER) && (CODEC_ECHO_PATH_VER >= 2)
enum HAL_CODEC_ECHO_PATH_T {
    HAL_CODEC_ECHO_PATH_PCM_TRI_SND = 0,
    HAL_CODEC_ECHO_PATH_PCM_TRI = 1,
    HAL_CODEC_ECHO_PATH_PCM_SND = 2,
    HAL_CODEC_ECHO_PATH_PCM = 3,
    HAL_CODEC_ECHO_PATH_EQ_TRI_SND = 4,
    HAL_CODEC_ECHO_PATH_EQ_TRI = 5,
    HAL_CODEC_ECHO_PATH_EQ_SND = 6,
    HAL_CODEC_ECHO_PATH_EQ = 7,

    HAL_CODEC_ECHO_PATH_DAC_2_6_UP_OUT = 8,          //Before ANC
    HAL_CODEC_ECHO_PATH_HBF4_DATA_IN = 9,            //After ANC
    HAL_CODEC_ECHO_PATH_HBF4_DATA_OUT_CH0 = 10,      //768k Before ANC
    HAL_CODEC_ECHO_PATH_HBF5_DATA_IN = 11,           //768k After ANC
    HAL_CODEC_ECHO_PATH_DAC_PSAP_MODE = 12,          //dac_psap_mode ? dac_2_6_up_out_psap : hbf1_data_out_ch0  After PSAP: 48k or 384k

    HAL_CODEC_ECHO_PATH_QTY
};
#else
enum HAL_CODEC_ECHO_PATH_T {
    HAL_CODEC_ECHO_PATH_DAC1_DAC2_PSAP = 0,
    HAL_CODEC_ECHO_PATH_DAC1_ALONE = 1,
    HAL_CODEC_ECHO_PATH_ALL = 2,
    HAL_CODEC_ECHO_PATH_DAC2_ALONE = 3,
    HAL_CODEC_ECHO_PATH_QTY
};
#endif

enum HAL_CODEC_ECHO_CHAN_T {
    HAL_CODEC_ECHO_CHAN_DAC_DATA_OUT_L = 0,
    HAL_CODEC_ECHO_CHAN_DAC_DATA_OUT_R = 1,
    HAL_CODEC_ECHO_CHAN_DAC_DATA_DBG_OUT_L = 2,
    HAL_CODEC_ECHO_CHAN_DAC_DATA_DBG_OUT_R = 3,

    HAL_CODEC_ECHO_CHAN_DAC_SND_DATA = 4,
    HAL_CODEC_ECHO_CHAN_DAC_TRI_DATA = 5,

    HAL_CODEC_ECHO_CHAN_QTY
};

enum HAL_CODEC_ECHO_RATE_T {
    HAL_CODEC_ECHO_RATE_48K  = 0,
    HAL_CODEC_ECHO_RATE_384K = 1,
    HAL_CODEC_ECHO_RATE_768K = 2, // 1x

    HAL_CODEC_ECHO_RATE_QTY
};

typedef void (*HAL_CODEC_DAC_RESET_CALLBACK)(enum HAL_CODEC_DAC_RESET_STAGE_T stage);
typedef void (*HAL_CODEC_SW_OUTPUT_COEF_CALLBACK)(float coef);
typedef void (*HAL_CODEC_BT_TRIGGER_CALLBACK)(void);
typedef void (*HAL_CODEC_EVENT_TRIGGER_CALLBACK)(void);
typedef void (*HAL_CODEC_TIMER_TRIGGER_CALLBACK)(void);
typedef void (*HAL_CODEC_IRQ_CALLBACK)(uint32_t status);
typedef void (*HAL_CODEC_IRQ_CALLBACK2)(uint32_t status1, uint32_t status2);

struct HAL_CODEC_CONFIG_T {
    enum AUD_BITS_T bits;
    enum AUD_SAMPRATE_T sample_rate;
    enum AUD_CHANNEL_NUM_T channel_num;
    enum AUD_CHANNEL_MAP_T channel_map;

    uint32_t use_dma:1;
    uint32_t vol:7;

    enum AUD_IO_PATH_T io_path;

    uint32_t set_flag;
};

struct dac_classg_cfg {
    uint8_t thd2;
    uint8_t thd1;
    uint8_t thd0;
    uint8_t lr;
    uint8_t step_4_3n;
    uint8_t quick_down;
    uint16_t wind_width;
};

struct HAL_CODEC_PSAP_CFG_T {
    enum HAL_CODEC_PSAP_MODE_T mode;
    uint8_t din0_samp_delay;
    float din0_gain;
    uint8_t din1_sign;
};

struct HAL_CODEC_DAC_DRE_CALIB_CFG_T {
    uint32_t valid;
    uint32_t dc_l;
    uint32_t dc_r;
    float    gain_l;
    float    gain_r;
    uint16_t ana_dc_l;
    uint16_t ana_dc_r;
    uint8_t ana_gain;
    uint8_t ini_ana_gain;
    uint8_t gain_offset;
    uint8_t step_mode;
    uint8_t top_gain;
    uint8_t rsv[3];
};

struct HAL_CODEC_ADC_DC_CALIB_CFG_T {
    uint32_t valid;
    uint32_t adc_channel_map;
    uint32_t adc_dc_calib_offset;
    uint32_t adc_dc_calib_step;
    uint32_t adc_ana_gain;
    int32_t  adc_dc_original;
    int32_t  adc_dc_calib_result;
    uint32_t rsvd0;
};

void hal_codec_adc_dig_dc_offset_enable(uint32_t adc_chan, int32_t dc);
uint32_t hal_codec_get_input_path_cfg(enum AUD_IO_PATH_T io_path);
uint32_t hal_codec_get_all_input_path_cfg(void);
uint32_t hal_codec_get_input_map_chan_num(uint32_t ch_map);
uint32_t hal_codec_get_input_path_chan_num(enum AUD_IO_PATH_T io_path);
void hal_codec_set_dac_volume_table(const struct CODEC_DAC_VOL_T *table_ptr, uint32_t table_num);
uint32_t hal_codec_get_default_dac_volume_index(void);
const struct CODEC_DAC_VOL_T *hal_codec_get_dac_volume(uint32_t index);
const CODEC_ADC_VOL_T *hal_codec_get_adc_volume(uint32_t index);
uint32_t hal_codec_get_mic_chan_volume_level(uint32_t map);
uint8_t hal_codec_get_digmic_hw_index(uint8_t chan);
uint32_t hal_codec_get_real_sample_rate(enum AUD_SAMPRATE_T rate, bool resamp);

int hal_codec_open(enum HAL_CODEC_ID_T id);
int hal_codec_close(enum HAL_CODEC_ID_T id);
void hal_codec_crash_mute(void);
void hal_codec_stop_playback_stream(enum HAL_CODEC_ID_T id);
void hal_codec_start_playback_stream(enum HAL_CODEC_ID_T id);
int hal_codec_start_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream);
int hal_codec_stop_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream);
int hal_codec_start_interface(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream, int dma);
int hal_codec_stop_interface(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream);
int hal_codec_setup_stream(enum HAL_CODEC_ID_T id, enum AUD_STREAM_T stream, const struct HAL_CODEC_CONFIG_T *cfg);
int hal_codec_anc_adc_enable(enum ANC_TYPE_T type);
int hal_codec_anc_adc_disable(enum ANC_TYPE_T type);
enum AUD_SAMPRATE_T hal_codec_anc_convert_rate(enum AUD_SAMPRATE_T rate);
int hal_codec_anc_dma_enable(enum HAL_CODEC_ID_T id);
int hal_codec_anc_dma_disable(enum HAL_CODEC_ID_T id);
void hal_codec_set_anc_boost_gain_attn(float attn);
void hal_codec_apply_anc_adc_gain_offset(enum ANC_TYPE_T type, int8_t offset_l, int8_t offset_r);
int hal_codec_aux_mic_dma_enable(enum HAL_CODEC_ID_T id);
int hal_codec_aux_mic_dma_disable(enum HAL_CODEC_ID_T id);
uint32_t hal_codec_get_alg_dac_shift(void);
void hal_codec_set_dac_reset_callback(HAL_CODEC_DAC_RESET_CALLBACK callback);
void hal_codec_set_sw_output_coef_callback(HAL_CODEC_SW_OUTPUT_COEF_CALLBACK callback);
void hal_codec_set_dac2_sw_output_coef_callback(HAL_CODEC_SW_OUTPUT_COEF_CALLBACK callback);
void hal_codec_set_dac3_sw_output_coef_callback(HAL_CODEC_SW_OUTPUT_COEF_CALLBACK callback);
void hal_codec_dac_gain_m60db_check(enum HAL_CODEC_PERF_TEST_POWER_T type);
void hal_codec_set_noise_reduction(bool enable);
void hal_codec_classg_config(const struct dac_classg_cfg *cfg);
void hal_codec_set_dac_dc_gain_attn(float attn);
void hal_codec_set_dac_dc_offset(int16_t dc_l, int16_t dc_r);
void hal_codec_sidetone_enable(void);
void hal_codec_sidetone_disable(void);
int hal_codec_sidetone_gain_ramp_up(float step);
int hal_codec_sidetone_gain_ramp_down(float step);
void hal_codec_select_adc_iir_mic(uint32_t index, enum AUD_CHANNEL_MAP_T mic_map);
void hal_codec_dac_mute(bool mute);
void hal_codec_adc_mute(bool mute);
int hal_codec_set_chan_vol(enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);
void hal_codec_sync_dac_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac_disable(void);
void hal_codec_sync_adc_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_adc_disable(void);
void hal_codec_sync_dac_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac_resample_rate_disable(void);
void hal_codec_sync_adc_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_adc_resample_rate_disable(void);
void hal_codec_sync_dac_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac_gain_disable(void);
void hal_codec_sync_adc_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_adc_gain_disable(void);
int hal_codec_dac_reset_set(void);
int hal_codec_dac_reset_clear(void);
int hal_codec_dac_sdm_reset_set(void);
int hal_codec_dac_sdm_reset_clear(void);
void hal_codec_dac_sdm_reset_pulse(void);
void hal_codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio);
void hal_codec_tune_both_resample_rate(float ratio);
void hal_codec_get_dac_gain(float *left_gain,float *right_gain);
int hal_codec_select_clock_out(uint32_t cfg);
int hal_codec_config_digmic_phase(uint8_t phase);
void hal_codec_mc_enable(void);
void hal_codec_mc_disable(void);
void hal_codec_setup_mc(enum AUD_CHANNEL_NUM_T channel_num, enum AUD_BITS_T bits);
void hal_codec_dsd_enable(void);
void hal_codec_dsd_disable(void);
void hal_codec_swap_output(bool swap);

void hal_codec_set_echo_rate(enum HAL_CODEC_ECHO_RATE_T rate);
void hal_codec_set_echo_ch1_rate(enum HAL_CODEC_ECHO_RATE_T rate);
void hal_codec_set_echo_ch(enum HAL_CODEC_ECHO_CHAN_T chan);
void hal_codec_set_echo_ch1(enum HAL_CODEC_ECHO_CHAN_T chan);
uint32_t hal_codec_get_echo_path(void);
uint32_t hal_codec_get_echo1_path(void);
void hal_codec_set_echo_path(enum HAL_CODEC_ECHO_PATH_T path);
void hal_codec_set_echo1_path(enum HAL_CODEC_ECHO_PATH_T path);

void hal_codec_gpio_trigger_debounce_enable(void);
void hal_codec_gpio_trigger_debounce_disable(void);

uint32_t hal_codec_timer_get(void);
uint32_t hal_codec_timer_ticks_to_us(uint32_t ticks);
void hal_codec_timer_trigger_read(void);

void hal_codec_anc_fb_check_set_irq_handler(HAL_CODEC_IRQ_CALLBACK cb);

int hal_codec_vad_open(const struct AUD_VAD_CONFIG_T *cfg);
int hal_codec_vad_close(void);
int hal_codec_vad_start(void);
int hal_codec_vad_stop(void);
uint32_t hal_codec_vad_recv_data(uint8_t *dst, uint32_t dst_size);
void hal_codec_get_vad_data_info(struct CODEC_VAD_BUF_INFO_T* vad_buf_info);

void hal_codec_set_bt_trigger_callback(HAL_CODEC_BT_TRIGGER_CALLBACK callback);
int hal_codec_bt_trigger_start(void);
int hal_codec_bt_trigger_stop(void);
void hal_codec_set_bt_trigger_ex_callback(uint32_t index, HAL_CODEC_BT_TRIGGER_CALLBACK callback);
int hal_codec_bt_trigger_ex_start(uint32_t index);
int hal_codec_bt_trigger_ex_stop(uint32_t index);

void hal_codec_min_phase_mode_enable(enum AUD_STREAM_T stream);
void hal_codec_min_phase_mode_disable(enum AUD_STREAM_T stream);

int hal_codec_timer_trig_i2s_enable(enum HAL_CODEC_TIMER_TRIG_MODE_T mode, uint32_t ticks, bool periodic);
int hal_codec_timer_trig_i2s_disable(void);

void hal_codec_iir_enable(uint32_t speed);
void hal_codec_iir_disable(void);

void hal_codec_iir_eq_enable(uint32_t speed);
void hal_codec_iir_eq_disable(void);

void hal_codec_fir_enable(uint32_t speed);
void hal_codec_fir_disable(void);
void hal_codec_fir_select_sys_clock(void);
void hal_codec_fir_select_own_clock(void);

void hal_codec_psap_enable(uint32_t speed);
void hal_codec_psap_disable(void);
void hal_codec_psap_setup(enum AUD_CHANNEL_MAP_T ch_map, const struct HAL_CODEC_PSAP_CFG_T *cfg);

int  hal_codec_adc2_start_interface(enum HAL_CODEC_ID_T id, int dma);
int  hal_codec_adc2_stop_interface(enum HAL_CODEC_ID_T id);
int  hal_codec_adc2_setup_stream(enum HAL_CODEC_ID_T id, const struct HAL_CODEC_CONFIG_T *cfg);
void hal_codec_adc2_mute(bool mute);
int  hal_codec_adc2_set_chan_vol(enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);

int  hal_codec_dac2_start_interface(enum HAL_CODEC_ID_T id, int dma);
int  hal_codec_dac2_stop_interface(enum HAL_CODEC_ID_T id);
int  hal_codec_dac2_setup_stream(enum HAL_CODEC_ID_T id, const struct HAL_CODEC_CONFIG_T *cfg);
void hal_codec_dac2_mute(bool mute);
int  hal_codec_dac2_set_chan_vol(enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);

int  hal_codec_dac3_start_interface(enum HAL_CODEC_ID_T id, int dma);
int  hal_codec_dac3_stop_interface(enum HAL_CODEC_ID_T id);
int  hal_codec_dac3_setup_stream(enum HAL_CODEC_ID_T id, const struct HAL_CODEC_CONFIG_T *cfg);
void hal_codec_dac3_mute(bool mute);
int  hal_codec_dac3_set_chan_vol(enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);

void hal_codec_sync_adc2_disable(void);
void hal_codec_sync_adc2_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_adc2_resample_rate_disable(void);
void hal_codec_sync_adc2_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_adc2_gain_disable(void);
void hal_codec_sync_adc2_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type);

void hal_codec_sync_dac2_disable(void);
void hal_codec_sync_dac2_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac2_resample_rate_disable(void);
void hal_codec_sync_dac2_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac2_gain_disable(void);
void hal_codec_sync_dac2_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type);

void hal_codec_sync_dac3_disable(void);
void hal_codec_sync_dac3_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac3_resample_rate_disable(void);
void hal_codec_sync_dac3_resample_rate_enable(enum HAL_CODEC_SYNC_TYPE_T type);
void hal_codec_sync_dac3_gain_disable(void);
void hal_codec_sync_dac3_gain_enable(enum HAL_CODEC_SYNC_TYPE_T type);

void hal_codec_sw_sync_cap_open(void);
void hal_codec_sw_sync_cap_close(void);
void hal_codec_sw_sync_cap_enable(void);
void hal_codec_sw_sync_cap2_open(void);
void hal_codec_sw_sync_cap2_close(void);
void hal_codec_sw_sync_cap2_enable(void);

void hal_codec_sw_sync_play_open(void);
void hal_codec_sw_sync_play_close(void);
void hal_codec_sw_sync_play_enable(void);
void hal_codec_sw_sync_play2_open(void);
void hal_codec_sw_sync_play2_close(void);
void hal_codec_sw_sync_play2_enable(void);
void hal_codec_sw_sync_play3_open(void);
void hal_codec_sw_sync_play3_close(void);
void hal_codec_sw_sync_play3_enable(void);

void hal_codec_tune_dac2_resample_rate(enum AUD_STREAM_T stream, float ratio);
void hal_codec_tune_dac3_resample_rate(enum AUD_STREAM_T stream, float ratio);

void hal_codec_set_dac_bt_sync_source(uint32_t src);
void hal_codec_set_adc_bt_sync_source(uint32_t src);
void hal_codec_set_dac2_bt_sync_source(uint32_t src);
void hal_codec_pll_bt_sync_source(uint32_t src);

struct HAL_CODEC_ADC_DC_CALIB_CFG_T *hal_codec_adc_get_calib_cfg(uint32_t *nr);

void hal_codec_dac_dre_init_calib_cfg(void);
struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *hal_codec_dac_dre_get_calib_cfg(uint32_t *nr);
int  hal_codec_dac_dre_check_calib_cfg(struct HAL_CODEC_DAC_DRE_CALIB_CFG_T *cfg);
void hal_codec_set_adc_calib_status(bool status);
void hal_codec_set_dac_calib_status(bool status);
void hal_codec_set_dac_ana_gain(uint8_t ini_ana_gain, uint8_t gain_offset);
bool hal_codec_get_dig_dc_calib_value_high_dre_gain(int32_t *dc_l, int32_t *dc_r);
int  hal_codec_dac_dc_auto_calib_enable(void);
int  hal_codec_dac_dc_auto_calib_disable(void);
void hal_codec_dac_dc_offset_enable(int32_t dc_l, int32_t dc_r);
void hal_codec_set_dig_dac_gain_dr(enum AUD_CHANNEL_MAP_T map, int32_t gain);

bool hal_codec_adc_dc_auto_calib_check(void);
int hal_codec_adc_dc_auto_calib_enable(enum AUD_CHANNEL_MAP_T ch_map);
int hal_codec_adc_dc_auto_calib_disable(enum AUD_CHANNEL_MAP_T ch_map);
void hal_codec_adc_dc_offset_update(enum AUD_CHANNEL_MAP_T ch_map, uint16_t calib_step, uint16_t calib_value);
#ifdef __cplusplus
}
#endif

#endif
