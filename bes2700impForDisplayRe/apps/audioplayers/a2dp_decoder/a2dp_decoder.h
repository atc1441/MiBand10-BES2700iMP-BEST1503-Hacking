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
#ifndef __A2DPPLAY_H__
#define __A2DPPLAY_H__

#include "bluetooth_bt_api.h"
#include "app_utils.h"
#include "a2dp_api.h"

#define A2DP_DECODER_HISTORY_SEQ_SAVE          (25)
//#define A2DP_DECODER_HISTORY_LOCTIME_SAVE      (1)
//#define A2DP_DECODER_HISTORY_CHECK_SUM_SAVE    (1)

typedef uint16_t A2DP_AUDIO_CODEC_TYPE;

#define A2DP_AUDIO_CODEC_TYPE_SBC           (1u<<0)
#define A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC   (1u<<1)
#define A2DP_AUDIO_CODEC_TYPE_OPUS          (1u<<2)
#define A2DP_AUDIO_CODEC_TYPE_SCALABL       (1u<<3)
#define A2DP_AUDIO_CODEC_TYPE_LHDC          (1u<<4)
#define A2DP_AUDIO_CODEC_TYPE_LDAC          (1u<<5)
#define A2DP_AUDIO_CODEC_TYPE_LC3           (1u<<6)
#define A2DP_AUDIO_CODEC_TYPE_LHDCV5        (1u<<7)
#define A2DP_AUDIO_CODEC_TYPE_L2HC          (1u<<8)

#define A2DP_AUDIO_SYNCFRAME_MASK_SEQ         (1u<<0)
#define A2DP_AUDIO_SYNCFRAME_MASK_TIMESTAMP   (1u<<1)
#define A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ  (1u<<2)
#define A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ (1u<<3)

#define A2DP_AUDIO_SYNCFRAME_MASK_ALL (A2DP_AUDIO_SYNCFRAME_MASK_SEQ        | \
                                       A2DP_AUDIO_SYNCFRAME_MASK_TIMESTAMP  | \
                                       A2DP_AUDIO_SYNCFRAME_MASK_CURRSUBSEQ | \
                                       A2DP_AUDIO_SYNCFRAME_MASK_TOTALSUBSEQ)

#define A2DP_AUDIO_SYNCFRAME_CHK(equ, mask_val, mask) (((equ)|!(mask_val&mask)))

typedef enum {
    A2DP_AUDIO_LATENCY_STATUS_LOW,
    A2DP_AUDIO_LATENCY_STATUS_HIGH,
} A2DP_AUDIO_LATENCY_STATUS_E;

typedef struct
{
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    uint8_t* ptrData;
    uint32_t dataLen;
} A2DP_COMMON_MEDIA_FRAME_HEADER_T;


typedef struct  {
    uint32_t sample_rate;
    uint8_t num_channels;
    uint8_t bits_depth;
    uint32_t frame_samples;
    uint32_t mtu_per_frame;
    float factor_reference;
} A2DP_AUDIO_OUTPUT_CONFIG_T;

typedef enum {
    A2DP_AUDIO_CHANNEL_SELECT_STEREO,
    A2DP_AUDIO_CHANNEL_SELECT_LRMERGE,
    A2DP_AUDIO_CHANNEL_SELECT_LCHNL,
    A2DP_AUDIO_CHANNEL_SELECT_RCHNL,
} A2DP_AUDIO_CHANNEL_SELECT_E;

typedef struct {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
    uint32_t frame_samples;
    uint32_t list_samples;
    uint32_t decoded_frames;
    uint32_t undecode_frames;
    uint32_t undecode_min_frames;
    uint32_t undecode_max_frames;
    uint32_t average_frames;
    uint32_t check_sum;
    A2DP_AUDIO_OUTPUT_CONFIG_T stream_info;
    A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel;
} A2DP_AUDIO_LASTFRAME_INFO_T;

typedef struct {
    uint16_t sequenceNumber;
    uint32_t timestamp;
    uint16_t curSubSequenceNumber;
    uint16_t totalSubSequenceNumber;
} A2DP_AUDIO_HEADFRAME_INFO_T;

typedef struct{
    float proportiongain;
    float integralgain;
    float derivativegain;
    float error[3];
    float result;
}A2DP_AUDIO_SYNC_PID_T;

#if defined(MUTE_FRAME_DETECT)
typedef struct
{
    uint16_t mute_count;
    uint64_t val_sum;
    uint16_t sample_count;
    uint16_t total_sample_count;
    bool mute_frame_flag[BT_DEVICE_NUM];
}A2DP_AUDIO_MUTE_DETECT_T;
#endif // MUTE_FRAME_DETECT

typedef A2DP_AUDIO_LASTFRAME_INFO_T A2DP_AUDIO_SYNCFRAME_INFO_T;

typedef int(*A2DP_AUDIO_DETECT_NEXT_PACKET_CALLBACK)(uint8_t device_id, btif_media_header_t *, unsigned char *, unsigned int len);

typedef void(*A2DP_AUDIO_STORE_DATA_NTF_CALLBACK)(void);

#ifdef __cplusplus
extern "C" {
#endif

uint32_t a2dp_audio_playback_handler(uint8_t device_id, uint8_t *buffer, uint32_t buffer_bytes);
float a2dp_audio_sync_pid_calc(A2DP_AUDIO_SYNC_PID_T *pid, float diff);
int a2dp_audio_sync_init(double ratio);
int a2dp_audio_sync_reset_data(void);
int a2dp_audio_sync_tune_sample_rate(double ratio);
int a2dp_audio_sync_direct_tune_sample_rate(double ratio);
int a2dp_audio_sync_tune_cancel(void);
bool a2dp_audio_sync_tune_onprocess(void);
int a2dp_audio_sysfreq_boost_start(uint32_t boost_cnt);
int a2dp_audio_sysfreq_boost_running(void);
#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
void a2dp_audio_max_waterline_init(uint16_t max_waterline_packet_mtu);
void a2dp_audio_min_waterline_init(uint16_t min_waterline_packet_mtu);
#endif
#if (A2DP_DECODER_VER < 2)
int a2dp_audio_init(uint32_t sysfreq);
#else
int a2dp_audio_init(uint32_t sysfreq, A2DP_AUDIO_CODEC_TYPE codec_type, A2DP_AUDIO_OUTPUT_CONFIG_T *config,
                           A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel, uint16_t dest_packet_mut, uint16_t chopping_dest_mut);
#endif
int a2dp_audio_deinit(void);
int a2dp_audio_start(void);
int a2dp_audio_preparse_stop(void);
int a2dp_audio_stop(void);
int a2dp_audio_detect_next_packet_callback_register(A2DP_AUDIO_DETECT_NEXT_PACKET_CALLBACK callback);
int a2dp_audio_detect_store_packet_callback_register(A2DP_AUDIO_DETECT_NEXT_PACKET_CALLBACK callback);
int a2dp_audio_store_packet_ntf_callback_register(A2DP_AUDIO_STORE_DATA_NTF_CALLBACK callback);
int a2dp_audio_detect_first_packet(void);
int a2dp_audio_detect_first_packet_clear(void);
int a2dp_audio_store_packet(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len);
int a2dp_audio_discards_packet(uint32_t packets);
int a2dp_audio_synchronize_dest_packet_mut(uint32_t mtu);
int a2dp_audio_discards_samples(uint32_t samples);
int a2dp_audio_convert_list_to_samples(uint32_t *samples);
int a2dp_audio_synchronize_packet(A2DP_AUDIO_SYNCFRAME_INFO_T *sync_info, uint32_t mask);
int a2dp_audio_decoder_headframe_info_get(A2DP_AUDIO_HEADFRAME_INFO_T *headframe_info);
int a2dp_audio_get_packet_samples(void);
int a2dp_audio_lastframe_info_get(A2DP_AUDIO_LASTFRAME_INFO_T *lastframe_info);
int a2dp_audio_lastframe_info_reset_undecodeframe(void);
void a2dp_audio_clear_input_raw_packet_list(void);
int a2dp_audio_refill_packet(void);
bool a2dp_audio_auto_synchronize_support(void);
A2DP_AUDIO_OUTPUT_CONFIG_T *a2dp_audio_get_output_config(void);
int a2dp_audio_latency_factor_setlow(void);
int a2dp_audio_latency_factor_sethigh(void);
float a2dp_audio_latency_factor_get(void);
int a2dp_audio_latency_factor_set(float factor);
int a2dp_audio_frame_delay_get(void);
int a2dp_audio_dest_packet_mut_get(void);
int a2dp_audio_latency_factor_status_get(A2DP_AUDIO_LATENCY_STATUS_E *latency_status, float *more_latency_factor);
float a2dp_audio_get_sample_reference(void);
int8_t a2dp_audio_get_current_buf_size(void);
void a2dp_audio_status_updated_callback(bool isA2dpAudioOn);

#if A2DP_DECODER_HISTORY_SEQ_SAVE
int a2dp_audio_show_history_seq(void);
#endif
int a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_E chnl_sel);

void a2dp_audio_update_latency_chunk_count(uint16_t chunkCount);
void app_ibrt_request_peer_custom_play_speed_tuning(void);
void app_ibrt_request_peer_sync_target_buf_cnt(void);

void app_audio_set_a2dp_freq(uint32_t freq);
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__) || defined(__AUDIO_BASS_ENHANCER__)
int stereo_surround_freq_controller(uint32_t freq);
#endif
typedef enum
{
    SBM_INITIAL_BUF_CNT_APPROACHED_AFTER_STREAM_STARTED = 0,
    SBM_MANUAL_TARGET_BUF_CNT_APPROACHED = 1,
    SBM_AUTO_TARGET_BUF_RESTORD = 2,
} SBM_JITTER_BUF_LEVEL_UPDATE_COMPLETED_EVENT_E;

typedef void(*SBM_JITTER_BUF_LEVEL_UPDATE_COMPLETED_FUNC)(SBM_JITTER_BUF_LEVEL_UPDATE_COMPLETED_EVENT_E type,
    int32_t target_ms, int32_t current_average_ms);

void sbm_reset_target_jitter_adjustment_env(void);
void sbm_target_jitter_buffer_updating_completed(void);
void sbm_set_auto_adjustment(void);
void sbm_target_jitter_buffer_initiative_approached(void);
bool sbm_update_local_target_jitter_buf_cnt(uint8_t targetCount);

void sbm_update_target_jitter_buf_length(uint16_t targetMs);
int32_t sbm_get_current_target_jitter_buf_length(void);
int32_t sbm_get_current_average_jitter_buf_length(void);
void sbm_register_jittr_buf_level_update_completed_event_cb(SBM_JITTER_BUF_LEVEL_UPDATE_COMPLETED_FUNC func);
bool sbm_is_manual_adjustment_on_going(void);
void sbm_clear_manual_adjustment_on_going_flag(void);
bool sbm_is_algorithm_adjustment_enabled(void);

typedef void (*sbm_feature_updated_callback_func_t)(uint8_t device_id, bool isEnable, bool isSuccessful);

typedef struct
{
    uint8_t device_id;
    uint8_t isEnable;
} SBM_CONTROL_REQ_T;

void a2dp_audio_scalable_update_local_scalable_sbm_feature(bool isEnable);
void sbm_control_local_scalable_sbm_feature(uint8_t device_id, bool isEnable);
bool sbm_is_local_scalable_sbm_enabled(uint8_t device_id);
void sbm_control_local_scalable_sbm_feature_handler(uint8_t device_id, bool isEnable);
void sbm_local_scalable_sbm_feature_updated_callback(uint8_t device_id, bool isEnable, bool isSuccessful);
void sbm_register_local_scalable_sbm_feature_updated_callback(sbm_feature_updated_callback_func_t callback_func);
void sbm_bump_cpu_freq_during_speed_tuning(void);
void sbm_release_cpu_freq_during_speed_tuning(void);

void app_audio_reset_rssi_based_env_ajustment(bool music_on);
void app_audio_rssi_based_env_ajdustment_handler(void);
void a2dp_audio_sysfreq_update_normalfreq(void);
bool a2dp_audio_retrigger_is_on_process(void);
void a2dp_audio_retrigger_set_on_process(bool flag);
bool a2dp_audio_is_underflow_in_processing(void);
void app_a2dp_audio_use_lower_frequency_flag_set(bool lower_freq_flag);
int a2dp_audio_custom_delay_ms_get(void);
bool a2dp_audio_chopping_is_in_post(void);

typedef enum
{
    A2DP_AUDIO_STARTED = 0,
    A2DP_AUDIO_STOPPED = 1,
} A2DP_AUDIO_STATUS_E;

typedef void (*a2dp_audio_status_updated_cb)(A2DP_AUDIO_STATUS_E status);
int32_t app_a2dp_audio_status_updated_callback_register(a2dp_audio_status_updated_cb cb);

typedef enum
{
    A2DP_AUDIO_MINIMUM_NORMAL_FREQ_WITH_ONE_MOBILE_LINK = 0,
    A2DP_AUDIO_MINIMUM_NORMAL_FREQ_WITH_TWO_MOBILE_LINK = 1,
    A2DP_AUDIO_BOOST_MODE_FREQ = 2,
    A2DP_AUDIO_FREQ_USER_CASE_CNT = 3,
    A2DP_AUDIO_FREQ_USER_CASE_INVALID = A2DP_AUDIO_FREQ_USER_CASE_CNT,

} A2DP_AUDIO_FREQ_USER_CASE_INDEX_E;

typedef enum
{
    A2DP_AUDIO_DYNAMIC_FREQ_HIGH_RSSI_CASE = 0,
    A2DP_AUDIO_DYNAMIC_FREQ_LOW_RSSI_CASE = 1,
    A2DP_AUDIO_DYNAMIC_FREQ_RSSI_CASE_CNT = 2,
    A2DP_AUDIO_DYNAMIC_FREQ_RSSI_CASE_INVALID = A2DP_AUDIO_DYNAMIC_FREQ_RSSI_CASE_CNT,
} A2DP_AUDIO_DYNAMIC_FREQ_RSSI_CASE_INDEX_E;

void a2dp_audio_set_freq_user_case(A2DP_AUDIO_FREQ_USER_CASE_INDEX_E userCase);
void a2dp_audio_set_freq_rssi_case(A2DP_AUDIO_DYNAMIC_FREQ_RSSI_CASE_INDEX_E rssiCase);
uint32_t a2dp_audio_list_length_TWP(void);
void a2dp_audio_sysfreq_reset(void);
#if defined(MUTE_FRAME_DETECT)
bool a2dp_audio_is_mute_frame(uint8_t device_id);
#endif
#if defined(IBRT)
void app_ibrt_sync_target_buf_ms_req_process(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
void app_ibrt_custom_play_speed_tuning_req_process(uint16_t rsp_seq, uint8_t *ptrParam, uint16_t paramLen);
#endif
#ifdef __cplusplus
}
#endif

#endif//__A2DPPLAY_H__
