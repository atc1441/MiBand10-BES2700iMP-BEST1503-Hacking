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
#ifndef AUDIO_FLINGER_H
#define AUDIO_FLINGER_H

#include "plat_types.h"
#include "stdbool.h"
#include "hal_aud.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t (*AF_STREAM_HANDLER_T)(uint8_t *buf, uint32_t len);
typedef void (*AF_IRQ_NOTIFICATION_T)(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
typedef void (*AF_CODEC_PLAYBACK_POST_HANDLER_T)(uint8_t *buf, uint32_t len, void *cfg);

//pingpong machine
enum AF_PP_T{
    PP_PING = 0,
    PP_PANG = 1
};

typedef enum  {
    AF_USER_AI,
    AF_USER_THIRDPART,
    AF_USER_AUDIO,
    AF_USER_SBC,
    AF_USER_SCO,
    AF_USER_TEST,
    AF_USER_HEARING_AID,
    AF_USER_AOB_CAPTURE,
    AF_USER_AOB_PLAYBACK,
#ifdef BLE_WALKIE_TALKIE
    AF_USER_WT_PLAYBACK,
    AF_USER_WT_CAPATRUE,
#endif
    AF_USER_BIS_TRAN_PLAYBACK,
    AF_USER_NUM
} AF_USER_E;

struct AF_STREAM_CONFIG_T {
    uint32_t ext_mclk_freq;
    enum AUD_SAMPRATE_T sample_rate;
    enum AUD_CHANNEL_MAP_T channel_map;
    enum AUD_CHANNEL_NUM_T channel_num;
    enum AUD_BITS_T bits;
    enum AUD_STREAM_USE_DEVICE_T device;
    enum AUD_IO_PATH_T io_path;
    enum AUD_DATA_ALIGN_T align;
    enum AUD_FS_FIRST_EDGE_T fs_edge;
    uint16_t fs_cycles;
    uint8_t slot_cycles;
    bool chan_sep_buf;
    bool sync_start;
    //should define type
    uint8_t vol;

    AF_STREAM_HANDLER_T handler;

    uint8_t *data_ptr;
    uint32_t data_size;
};

//Should define return status
uint32_t af_open(void);
void *af_thread_tid_get(void);
uint32_t af_stream_open(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, const struct AF_STREAM_CONFIG_T *cfg);
uint32_t af_stream_setup(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, const struct AF_STREAM_CONFIG_T *cfg);
uint32_t af_stream_mute(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, bool mute);
uint32_t af_stream_set_chan_vol(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, enum AUD_CHANNEL_MAP_T ch_map, uint8_t vol);
uint32_t af_stream_restore_chan_vol(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_get_cfg(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, struct AF_STREAM_CONFIG_T **cfg, bool needlock);
uint32_t af_stream_start(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_stop(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_close(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_close(void);
uint32_t af_stream_get_cur_dma_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
int af_stream_get_cur_dma_pos(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_get_cur_dma_virtual_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
int af_stream_get_cur_dma_virtual_pos(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
int af_stream_buffer_error(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_dma_tc_irq_enable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_dma_tc_irq_disable(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint32_t af_stream_get_dma_base_addr(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
uint8_t af_stream_get_dma_chan(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);
void af_stream_tune(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, float ratio);
void af_set_irq_notification(AF_IRQ_NOTIFICATION_T notif);

void af_lock_thread(void);
void af_unlock_thread(void);
void af_set_priority(AF_USER_E user, int priority);
int af_get_priority(void);
int af_get_default_priority(void);
void af_reset_priority(void);

void af_codec_tune_resample_rate(enum AUD_STREAM_T stream, float ratio);
void af_codec_direct_tune_resample_rate(enum AUD_STREAM_T stream, float ratio);
void af_codec_tune_both_resample_rate(float ratio);
void af_codec_tune_pll(float ratio);
void af_codec_tune_xtal(float ratio);
void af_codec_tune(enum AUD_STREAM_T stream, float ratio);
void af_codec_direct_tune(enum AUD_STREAM_T stream, float ratio);

void af_codec_tune_dac2_resample_rate(enum AUD_STREAM_T stream, float ratio);
void af_codec_direct_tune_dac2_resample_rate(enum AUD_STREAM_T stream, float ratio);
void af_codec_tune_dac2(enum AUD_STREAM_T stream, float ratio);
void af_codec_direct_tune_dac2(enum AUD_STREAM_T stream, float ratio);

void af_codec_set_perf_test_power(int type);
void af_codec_set_noise_reduction(bool enable);
void af_codec_swap_output(bool swap);
void af_codec_set_playback_post_handler(AF_CODEC_PLAYBACK_POST_HANDLER_T hdlr);
void af_codec_set_playback2_post_handler(AF_CODEC_PLAYBACK_POST_HANDLER_T hdlr);
void af_codec_min_phase_mode_enable(enum AUD_STREAM_T stream);
void af_codec_min_phase_mode_disable(enum AUD_STREAM_T stream);

enum AF_CODEC_SYNC_TYPE_T {
    AF_CODEC_SYNC_TYPE_NONE,
    AF_CODEC_SYNC_TYPE_GPIO,
    AF_CODEC_SYNC_TYPE_BT,
    AF_CODEC_SYNC_TYPE_WIFI,
};

enum AF_I2S_SYNC_TYPE_T {
    AF_I2S_SYNC_TYPE_NONE,
    AF_I2S_SYNC_TYPE_BT,
    AF_I2S_SYNC_TYPE_GPIO,
};


void af_codec_set_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src);
void af_codec_sync_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);
void af_codec_sync_resample_rate_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);
void af_codec_sync_gain_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);

void af_codec_set_dac2_bt_sync_source(enum AUD_STREAM_T stream, uint32_t src);
void af_codec_sync_dac2_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);
void af_codec_sync_dac2_resample_rate_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);
void af_codec_sync_dac2_gain_config(enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);

void af_codec_set_device_bt_sync_source(enum AUD_STREAM_USE_DEVICE_T device, enum AUD_STREAM_T stream, uint32_t src);
void af_codec_sync_device_config(enum AUD_STREAM_USE_DEVICE_T device, enum AUD_STREAM_T stream, enum AF_CODEC_SYNC_TYPE_T type, bool enable);

typedef void (*AF_ANC_HANDLER)(enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T rate, enum AUD_SAMPRATE_T *new_play, enum AUD_SAMPRATE_T *new_cap);

int af_anc_open(enum ANC_TYPE_T type, enum AUD_SAMPRATE_T play_rate, enum AUD_SAMPRATE_T capture_rate, AF_ANC_HANDLER handler);
int af_anc_close(enum ANC_TYPE_T type);

int af_vad_open_simplified(bool init, struct AUD_VAD_SIMP_CFG_T *cfg);
int af_vad_open(const struct AUD_VAD_CONFIG_T *cfg);
int af_vad_close(void);
int af_vad_start(void);
int af_vad_stop(void);
uint32_t af_vad_get_data(uint8_t *buf, uint32_t len);
void af_vad_get_data_info(struct CODEC_VAD_BUF_INFO_T * vad_buf_info);

void af_dsd_enable(void);
void af_dsd_disable(void);

void af_i2s_sync_config(enum AUD_STREAM_T stream, enum AF_I2S_SYNC_TYPE_T type, bool enable);
int af_i2s_resample_enable(enum AUD_STREAM_USE_DEVICE_T dev);
int af_i2s_resample_disable(enum AUD_STREAM_USE_DEVICE_T dev);
int af_tdm_resample_enable(enum AUD_STREAM_USE_DEVICE_T dev);
int af_tdm_resample_disable(enum AUD_STREAM_USE_DEVICE_T dev);

typedef void (*AF_CODEC_BT_TRIGGER_CALLBACK)(void);
void af_codec_bt_trigger_config(bool en, AF_CODEC_BT_TRIGGER_CALLBACK callback);
uint32_t af_stream_playback_fadeout(enum AUD_STREAM_ID_T id, uint32_t ms);

/* DAC DC and Gain calibration */
enum AF_CODEC_CALIB_CMD_T {
    CODEC_CALIB_CMD_CLOSE,
    CODEC_CALIB_CMD_OPEN,
    CODEC_CALIB_CMD_SET_LARGE_ANA_DC,
    CODEC_CALIB_CMD_DIG_DC,
    CODEC_CALIB_CMD_DIG_GAIN,
    CODEC_CALIB_CMD_GET_CUR_DC,
};

enum AF_CODEC_CALIB_PARAM_T {
    DAC_PARAM_ANA_GAIN,
    DAC_PARAM_ANA_DC,
    DAC_PARAM_DIG_GAIN,
    DAC_PARAM_DIG_DC,
    DAC_PARAM_OUT_DC,
};

enum AF_CODEC_CALIB_STATE_T {
    CODEC_CALIB_STATE_NONE,
    CODEC_CALIB_STATE_ERR_ADC_DC_L,
    CODEC_CALIB_STATE_ERR_ADC_DC_R,
    CODEC_CALIB_STATE_ERR_DAC_DC_L,
    CODEC_CALIB_STATE_ERR_DAC_DC_R,
};

struct AF_CODEC_CALIB_CFG_T {
    uint8_t *buf;
    uint32_t len;
    uint32_t dig_dc_l;
    uint32_t dig_dc_r;
    uint16_t ana_dc_l;
    uint16_t ana_dc_r;
    uint32_t out_dc_l;
    uint32_t out_dc_r;
    int32_t  state;
};

#ifdef CODEC_ERROR_HANDLING
#define AF_NO_DMA_IRQ_COMING_TIMEOUT_MS  2000
#define AF_SUPERVISOR_PERIOD_IN_MS    500
#define AF_SUPERVISOR_TIMOUT_PERIOD_CNT_SINCE_LAST_DMA_IRQ \
    ((AF_NO_DMA_IRQ_COMING_TIMEOUT_MS)/(AF_SUPERVISOR_PERIOD_IN_MS))

typedef void (*af_enter_abnormal_state_callback_func_t)(uint32_t streamId, uint32_t streamType);
void af_register_enter_abnormal_state_callback(af_enter_abnormal_state_callback_func_t func);
#endif

int af_codec_calib_dac_dc(enum AF_CODEC_CALIB_CMD_T cmd, struct AF_CODEC_CALIB_CFG_T *cfg);
void af_codec_calib_dac_chan_enable(bool en_l, bool en_r);
void af_codec_calib_param_setup(enum AF_CODEC_CALIB_PARAM_T type, uint32_t p0, uint32_t p1, uint32_t p2);

void af_codec_dac1_sw_gain_process(uint8_t *buf, uint32_t len, enum AUD_BITS_T bits, enum AUD_CHANNEL_NUM_T chans);
void af_codec_dac1_sw_gain_enable(bool enable);

#ifdef ADVANCE_FILL_ENABLED
void af_pre_fill_handler(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream, uint8_t pre_fill_seq);
#endif

#ifdef AUDIO_OUTPUT_SW_GAIN
void af_codec_dac1_set_algo_gain(float coef);
void af_codec_dac1_set_custom_eq_peak(float peak);
#endif

#ifdef __cplusplus
}
#endif

#endif /* AUDIO_FLINGER_H */
