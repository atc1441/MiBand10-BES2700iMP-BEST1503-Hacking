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
#ifndef __ANC_PROCESS_H__
#define __ANC_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "aud_section.h"
#include "hal_aud.h"

typedef enum {
    ANC_NO_ERR=0,
    ANC_TYPE_ERR,
    ANC_GAIN_ERR,
    ANC_OTHER_ERR,
    ANC_ERR_TOTAL,
}ANC_ERROR;

typedef enum {
    ANC_GAIN_NO_DELAY=0,
    ANC_GAIN_DELAY,
    ANC_GAIN_TOTAL,
}ANC_GAIN_TIME;

typedef enum {
    ANC_SWITCHING_DELAY_50ms=50,
    ANC_SWITCHING_DELAY_100ms=100,
    ANC_SWITCHING_DELAY_200ms=200,
    ANC_SWITCHING_DELAY_450ms=450,
    ANC_SWITCHING_DELAY_650ms=650,
    ANC_SWITCHING_DELAY_850ms=850,
    ANC_SWITCHING_DELAY_1300ms=1300,
    ANC_SWITCHING_DELAY_1600ms=1600,
    ANC_SWITCHING_DELAY_1900ms=1900,
    ANC_SWITCHING_DELAY_2200ms=2200,
    ANC_SWITCHING_DELAY_2600ms=2600,
    ANC_SWITCHING_DELAY_TOTAL,
}ANC_SWITCHING_DELAY;

typedef enum {
    ANC_HOWLING_WINDOW_32=32,
    ANC_HOWLING_WINDOW_64=64,
    ANC_HOWLING_WINDOW_128=128,
    ANC_HOWLING_WINDOW_256=256,
    ANC_HOWLING_WINDOW_512=512,
    ANC_HOWLING_WINDOW_TOTAL,
}ANC_HOWLING_WINDOW;

typedef enum {
    ANC_HOWLING_THRESHOLD_m12dB=2949972,
    ANC_HOWLING_THRESHOLD_m10dB=3713795,
    ANC_HOWLING_THRESHOLD_m8dB=4675391,
    ANC_HOWLING_THRESHOLD_m6dB=5872025,
    ANC_HOWLING_THRESHOLD_m4dB=7409995,
    ANC_HOWLING_THRESHOLD_m2dB=9328631,
    ANC_HOWLING_THRESHOLD_0dB=11744051,
    ANC_HOWLING_THRESHOLD_2dB=14784884,
    ANC_HOWLING_THRESHOLD_4dB=18613066,
    ANC_HOWLING_THRESHOLD_6dB=23488102,
    ANC_HOWLING_THRESHOLD_8dB=29499722,
    ANC_HOWLING_THRESHOLD_10dB=37137950,
    ANC_HOWLING_THRESHOLD_12dB=46753909,
    ANC_HOWLING_THRESHOLD_TOTAL,
}ANC_HOWLING_THRESHOLD;


typedef enum {
    ANC_ADC_ONLY_ADC=0,
    ANC_ADC_ONLY_MC,
    ANC_ADC_ADC_ADD_MC,
    ANC_ADC_TOTAL,
}ANC_ADC_DATA;

typedef struct _aud_fir_item
{
    int32_t    fir_bypass_flag;
    int32_t    fir_len;
    int32_t    fir_coef[AUD_COEF_LEN];
    int32_t    fir_step;
} aud_fir_item;

typedef struct _struct_anc_fir_cfg
{
    aud_fir_item anc_fir_cfg_ff_l;
    aud_fir_item anc_fir_cfg_ff_r;
    aud_fir_item anc_fir_cfg_fb_l;
    aud_fir_item anc_fir_cfg_fb_r;
    aud_fir_item anc_fir_cfg_tt_l;
    aud_fir_item anc_fir_cfg_tt_r;
    aud_fir_item anc_fir_cfg_mc_l;
    aud_fir_item anc_fir_cfg_mc_r;
} struct_anc_fir_cfg;

typedef struct _anc_howling_check_cfg {
    int32_t howling_recover_time;
    int32_t howling_interval_time;

    int32_t howling_set_gain_level;
} anc_howling_check_cfg;

/*
ANC beamforming
Support CHIPS: 1603
Can set delay on tt channel, then mix it to ff channel.
*/
typedef struct _anc_bf_cfg {
    int32_t enable;               // 0:disable beamforming by default    1:enable.
    int32_t tt_delay_flag;        // 0:don't need set delay              1:set delay
    int32_t tt_delay_num;         // range: 1~15, delay point on ANC sample rate.
    int32_t tt_pdu_mix_flag;      // 1:mix to ff before iir              0:not mix.
    int32_t tt_pdu_off_flag;      // 1:disconnect tt channel after iir   0:connect.
    int32_t anc_ff_calib_gain;    // the gain is before iir.
    int32_t anc_tt_calib_gain;    // the gain is before iir.
} anc_bf_cfg_t;

typedef struct _anc_process_cfg {
    bool using_tt_as_ff_en;
    bool using_ff_as_dehowling_en;

    anc_howling_check_cfg howling_check_cfg;
    anc_bf_cfg_t bf_cfg;
} anc_process_cfg;

enum ANC_HW_NOTCH_IIR_ID {
    ANC_HW_NOTCH_IIR0 = 0,
    ANC_HW_NOTCH_IIR1,
    ANC_HW_NOTCH_IIR2,
    ANC_HW_NOTCH_IIR3,
    ANC_HW_NOTCH_IIR4,
    ANC_HW_NOTCH_IIR6,

    ANC_HW_NOTCH_IIR_NUM,
};

int anc_set_fir_cfg( struct_anc_fir_cfg * cfg, enum ANC_TYPE_T anc_type);

void anc_fir_step_sample_mode(uint16_t step);

void anc_fir_open(struct_anc_fir_cfg *cfg, enum ANC_TYPE_T anc_type);

/**
 * Used to calib anc gain in production line
 * Call this function when init platform
 */
int32_t anc_set_calib_gain(enum ANC_TYPE_T type, int32_t gain_l, int32_t gain_r);

int anc_opened(enum ANC_TYPE_T anc_type);
int anc_open(enum ANC_TYPE_T anc_type);
void anc_close(enum ANC_TYPE_T anc_type);

int anc_disable(void);
int anc_enable(void);

int anc_set_cfg(const struct_anc_cfg * cfg,enum ANC_TYPE_T anc_type,ANC_GAIN_TIME anc_gain_delay);
int anc_set_gain(int32_t gain_ch_l, int32_t gain_ch_r,enum ANC_TYPE_T anc_type);
int anc_get_gain(int32_t *gain_ch_l, int32_t *gain_ch_r,enum ANC_TYPE_T anc_type);
int anc_get_cfg_gain(int32_t *gain_ch_l, int32_t *gain_ch_r,enum ANC_TYPE_T anc_type);
int anc_set_gain_f32(float gain_l, float gain_r, enum ANC_TYPE_T type);

//void AncPduHardwarCh0(const short Coef[], short CoefLen);
//void AncPduHardwarCh1(const short Coef[], short CoefLen);
void AncPduHardwarCh0(const short Coef[], short CoefLen,short gain);
void AncPduHardwarCh1(const short Coef[], short CoefLen,short gain);

int anc_select_coef(enum AUD_SAMPRATE_T rate,enum ANC_INDEX index,enum ANC_TYPE_T anc_type,ANC_GAIN_TIME anc_gain_delay);
int spkcalib_select_coef(enum AUD_SAMPRATE_T rate,int index);
int anc_set_psap_eq_cfg(const struct_psap_eq_cfg *cfg);

int anc_load_cfg(void);
int anc_cmd_receve_process(uint8_t *buf,uint32_t len);
int anc_cmd_send_process(uint8_t **pbuf,uint16_t *len);

enum ANC_INDEX anc_get_current_coef_index(void);
enum AUD_SAMPRATE_T anc_get_current_coef_samplerate(void);
int anc_mc_run_stereo(uint8_t *buf, int len,float left_gain,float right_gain,enum AUD_BITS_T sample_bit);
int anc_mc_run_mono(uint8_t *buf, int len,float left_gain,enum AUD_BITS_T sample_bit);
void anc_mc_run_init(enum AUD_SAMPRATE_T rate);
void anc_mc_run_setup(enum AUD_SAMPRATE_T rate);

void anc_disable_gain_updated_when_pass0(uint8_t on);
void  anc_set_ch_map( int32_t ch_map );
void  anc_howling_check_enable(int32_t flag );
void anc_howling_gain_reset(void);
int anc_set_switching_delay(ANC_SWITCHING_DELAY  anc_switching_delay,enum ANC_TYPE_T anc_type);
int anc_howling_set(ANC_HOWLING_WINDOW window, ANC_HOWLING_THRESHOLD threshold);
int anc_set_dehowling_cfg(const dehowling_config *cfg);
int anc_set_spkcalib_cfg(const struct_spkcalib_cfg *cfg);
int anc_adc_data_select(ANC_ADC_DATA data_select);
int anc_limiter_enable(enum ANC_TYPE_T anc_type);
int anc_limiter_disable(enum ANC_TYPE_T anc_type);
int anc_limiter_threhold_set(enum ANC_TYPE_T anc_type,int32_t threhold_db);

int anc_set_gain2_float(float gain);
float anc_get_gain2_float(void);

int32_t anc_process_set_cfg(const anc_process_cfg * anc_cfg);

/* API: just support partial chip version*/
int anc_limiter_rls_time_set(enum ANC_TYPE_T anc_type, float release_ms);

void set_anc_lmt_th_val(int ff_th_val, int fb_th_val, int tt_th_val);
int get_anc_ff_lmt_th(void);
int get_anc_fb_lmt_th(void);
int get_anc_tt_lmt_th(void);

int32_t anc_fir_set_status(bool flag);

void anc_dehowl_keep_ch0_enable(bool en, uint16_t adc_ch);

void anc_dehowl_keep_ch1_enable(bool en, uint16_t adc_ch);

void anc_dehowl_enable(bool en);

void anc_tt_pass_mc_enable(bool en);

void anc_fir_tt_mode_enable(enum ANC_TYPE_T anc_type, bool en);

#ifdef __cplusplus
}
#endif

#endif
