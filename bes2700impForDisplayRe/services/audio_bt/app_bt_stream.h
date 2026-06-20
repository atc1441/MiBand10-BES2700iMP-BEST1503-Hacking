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
#ifndef __APP_BT_STREAM_H__
#define __APP_BT_STREAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth_bt_api.h"
#include "hal_aud.h"
#include "stdint.h"
#define BT_AUDIO_BUFF_AAC_SIZE (1024*2*2*2)

#define BT_AUDIO_BUFF_SBC_44P1K_SIZE (128*5*2*2*2)
#define BT_AUDIO_BUFF_SBC_48K_SIZE (128*6*2*2*2)

#ifdef A2DP_LHDC_ON
#define LHDC_AUDIO_96K_BUFF_SIZE        (256*2*4*8) // 2.6ms * 8 = 20.8ms
#define LHDC_AUDIO_BUFF_SIZE            (256*2*4*4) //48khz: 5.2ms * 4 = 20.8ms
                                                    //44.1khz: 5.8ms * 4 = 23.2ms
#define LLAC_AUDIO_44P1K_BUFF_SIZE      (220*2*4*4) // 4.9ms * 4 = 19.9ms
#define LLAC_AUDIO_BUFF_SIZE            (240*2*4*4) // 5ms * 4 = 20ms
#define LHDC_LLC_AUDIO_BUFF_SIZE        (256*2*2*2)
#endif

#if defined(A2DP_LHDCV5_ON)
#define LHDCV5_AUDIO_192K_BUFF_SIZE                (960*2*4)//(960*2*2*4)
#define LHDCV5_AUDIO_96K_BUFF_SIZE                 (480*2*4)//(480*2*2*4)
#define LHDCV5_AUDIO_48K_BUFF_SIZE                 (240*2*8)//(240*2*2*8)
#define LHDCV5_AUDIO_192K_LLC_BUFF_SIZE            (960*2*2*2)
#define LHDCV5_AUDIO_96K_LLC_BUFF_SIZE             (480*2*2*2)
#define LHDCV5_AUDIO_48K_LLC_BUFF_SIZE             (240*2*2*2)
#endif

#if defined(A2DP_LDAC_ON)
#define BT_AUDIO_BUFF_SIZE_192K_LDAC (512*4*2*2*2)
#define BT_AUDIO_BUFF_SIZE_96K_LDAC (256*4*2*2*2)
#define BT_AUDIO_BUFF_SIZE_48K_LDAC (128*4*2*2*2)
#define LDAC_CC_MEM_POOL_SIZE (36*1024)
#endif

#ifdef A2DP_LC3_ON
#define LC3_AUDIO_96K_BUFF_SIZE        (96*10*1*2*2*2)
#define LC3_AUDIO_BUFF_SIZE            (48*10*1*2*2*2)
#endif

#define BT_AUDIO_CACHE_2_UNCACHE(addr) \
    ((unsigned char *)((unsigned int)addr & ~(0x04000000)))

// #if BT_AUDIO_BUFF_SIZE < BT_AUDIO_SCO_BUFF_SIZE * 4
// #error BT_AUDIO_BUFF_SIZE must be at least BT_AUDIO_SCO_BUFF_SIZE * 4
// #endif

// low 11 bits are output streams index while high 5 bits are input streams index
// one output stream and one input stream can exit at the same time,
// while two streams with the same direction are mutex
#define APP_BT_STREAM_BORDER_BIT_OFFSET     12
// the index < the border is the output stream
// the index >= the border is the input stream
#define APP_BT_STREAM_BORDER_INDEX          (1 << APP_BT_STREAM_BORDER_BIT_OFFSET)

// output streams
#define APP_BT_STREAM_HFP_PCM               (1 << 0)
#define APP_BT_STREAM_HFP_CVSD              (1 << 1)
#define APP_BT_STREAM_HFP_VENDOR            (1 << 2)
#define APP_BT_STREAM_A2DP_SBC              (1 << 3)
#define APP_BT_STREAM_A2DP_AAC              (1 << 4)
#define APP_BT_STREAM_A2DP_VENDOR           (1 << 5)
#ifdef __FACTORY_MODE_SUPPORT__
#define APP_FACTORYMODE_AUDIO_LOOP          (1 << 6)
#endif
#define APP_PLAY_BACK_AUDIO                 (1 << 7)
#ifdef RB_CODEC
#define APP_BT_STREAM_RBCODEC               (1 << 8)
#endif
#ifdef AUDIO_LINEIN
#define APP_PLAY_LINEIN_AUDIO               (1 << 9)
#endif
#ifdef BT_SOURCE
#define APP_A2DP_SOURCE_LINEIN_AUDIO        (1 << 10)
#define APP_A2DP_SOURCE_I2S_AUDIO           (1 << 11)
#define APP_A2DP_SOURCE_USB_AUDIO           (1 << 13)
#endif
#ifdef AUDIO_PCM_PLAYER
#define APP_PLAY_PCM_PLAYER                 (1 << 12)
#endif
// input streams
#ifdef __AI_VOICE__
#define APP_BT_STREAM_AI_VOICE             (1 << APP_BT_STREAM_BORDER_BIT_OFFSET)
#endif

#define APP_BT_STREAM_INVALID               0

#if defined (SPEECH_TX_3MIC_NS) || !defined (SCO_CP_ACCEL)
#define APP_SYSFREQ_USER_BT_SCO_MASTER APP_SYSFREQ_208M
#else
#define APP_SYSFREQ_USER_BT_SCO_MASTER APP_SYSFREQ_104M
#endif

#define APP_SYSFREQ_USER_BT_SCO_SLAVE APP_SYSFREQ_52M

#if defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
#define APP_BT_OUTPUT_STREAM_INDEX(player)  ((player) & ((1 << APP_BT_STREAM_BORDER_BIT_OFFSET) - 1) & (~APP_PLAY_BACK_AUDIO))
#else
#define APP_BT_OUTPUT_STREAM_INDEX(player)  ((player) & ((1 << APP_BT_STREAM_BORDER_BIT_OFFSET) - 1))
#endif
#define APP_BT_INPUT_STREAM_INDEX(player)   ((player) & (~((1 << APP_BT_STREAM_BORDER_BIT_OFFSET) - 1)))

#include <stdint.h>

enum APP_BT_SETTING_T {
    APP_BT_SETTING_OPEN = 0,
    APP_BT_SETTING_CLOSE,
    APP_BT_SETTING_SETUP,
    APP_BT_SETTING_RESTART,
    APP_BT_SETTING_CLOSEALL,
    APP_BT_SETTING_CLOSEMEDIA,
    APP_BT_SETTING_START_PROMPT,
    APP_BT_SETTING_STOP_PROMPT,
    APP_BT_SETTING_START_BLEAUDIO,
    APP_BT_SETTING_STOP_BLEAUDIO,
    APP_BT_SETTING_STOP_SINGLE_STREAM,
    APP_BT_SETTING_OUTPUT_ROUTE,
    APP_BT_SETTING_NUM,
};

#if defined(TX_RX_PCM_MASK)
struct PCM_DATA_FLAG_T {
    //
    uint32_t curr_time;
    //sample2+3 [9:0]
    uint8_t toggle;
    //0: sco_frm_isr 1: sco_sket_isr
    uint8_t flag;
    //frame counter 0~0xffff
    uint16_t counter;
};
#elif defined(PCM_FAST_MODE) && defined(PCM_PRIVATE_DATA_FLAG)
struct PCM_DATA_FLAG_T {
    //sample1 d0220ca0[7:0]
    uint8_t undef;
    //sample2+3 [9:0]
    uint16_t bitcnt;
    //sample2+3 [15:13]
    uint8_t softbit_flag;
    //sample4+5+6+7
    uint32_t btclk;
    //sample8+9+10+11 d0220c9c[31:0]
    uint32_t reserved;
    //head_offset
    uint8_t offset;
};
#endif

const char *audio_op_to_str(enum APP_BT_SETTING_T op);

const char *player2str(uint16_t player_type);

bool bt_a2dp_is_run(void);

int app_bt_stream_init(void);

int app_bt_stream_open(APP_AUDIO_STATUS* status);

int app_bt_stream_close(uint16_t player);

int app_bt_stream_setup(uint16_t player, uint8_t status);

int app_bt_stream_restart(APP_AUDIO_STATUS* status);

int app_bt_stream_closeall();

bool app_bt_stream_isrun(uint16_t player);

void app_bt_set_volume(uint16_t type,uint8_t level);

void  app_bt_stream_bt_volumeup(void);

void  app_bt_stream_bt_volumedown(void);

uint8_t  app_bt_stream_local_volumeup(void);

uint8_t  app_bt_stream_local_volumedown(void);

void app_bt_stream_update_local_hfp_vol(uint8_t updatedLocalVol);

void app_bt_stream_volume_ptr_update(uint8_t *bdAddr);

struct btdevice_volume * app_bt_stream_volume_get_ptr(void);

int app_bt_stream_volumeset(int8_t vol);

void app_bt_stream_set_tune_samplerate_ratio(enum AUD_STREAM_T stream, float ratio);

void app_bt_stream_set_output_route(void *route_info);

uint8_t app_bt_stream_a2dpvolume_get(void);

uint8_t app_bt_stream_hfpvolume_get(void);

void app_bt_stream_a2dpvolume_reset(void);

void app_bt_stream_hfpvolume_reset(void);

void app_bt_stream_copy_track_one_to_two_24bits(int32_t *dst_buf, int32_t *src_buf, uint32_t src_len);

void app_bt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len);

void app_bt_stream_copy_track_two_to_one_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t dst_len);

enum AUD_SAMPRATE_T bt_get_sbc_sample_rate(void);

void bt_store_music_sample_rate(enum AUD_SAMPRATE_T sample_rate);

enum AUD_SAMPRATE_T bt_parse_store_music_sample_rate(uint32_t sbc_samp_rate);

bool app_bt_stream_is_mic_mix_during_music_enabled(void);
void app_bt_sream_set_mix_mic_flag(bool isEnable);
void bt_sbc_mix_mic_init(void);
void bt_sbc_mix_mic_deinit(void);
uint32_t bt_sbc_codec_capture_data(uint8_t *buf, uint32_t len);
uint16_t app_bt_get_curr_sco_hci_handle(uint8_t curr_sco_device_id);
uint8_t app_bt_get_curr_a2dp_sample_rate(uint8_t curr_a2dp_device_id);
uint16_t app_bt_get_conhandle_by_device_id(uint8_t device_id);
bool app_bt_is_curr_a2dp_streaming(uint8_t curr_a2dp_device_id);
uint16_t a2dp_Get_curr_a2dp_conhdl(void);

void app_audio_buffer_check(void);
uint8_t app_audio_get_eq();
int app_audio_set_eq(uint8_t index);

bool bt_sco_codec_is_cvsd(void);
bool bt_sco_codec_is_msbc(void);

uint8_t amgr_is_bluetooth_sco_on();
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
int32_t stereo_surround_is_ldac(void);
#endif

bool app_is_in_multipoint_timer(void);
// =======================================================
// APP RESAMPLE
// =======================================================

#include "hal_aud.h"

typedef int (*APP_RESAMPLE_BUF_ALLOC_CALLBACK)(uint8_t **buff, uint32_t size);

typedef int (*APP_RESAMPLE_ITER_CALLBACK)(uint8_t *buf, uint32_t len);

typedef uint8_t (*APP_STREAM_GET_A2DP_PARAM_CALLBACK)(uint8_t);
typedef int (*RMT_A2DP_SINK_DEV_DATA_IND_CALLBACK)(uint8_t *buffer, uint32_t size);

struct APP_RESAMPLE_T {
    enum AUD_STREAM_T stream;
    void *id;
    APP_RESAMPLE_ITER_CALLBACK cb;
    uint8_t *iter_buf;
    uint32_t iter_len;
    uint32_t offset;
    float ratio_step;
};

struct APP_RESAMPLE_T *app_playback_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
                                                  APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len);
struct APP_RESAMPLE_T *app_playback_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
                                                      APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
                                                      float ratio_step);
struct APP_RESAMPLE_T *app_playback_resample_any_open_with_pre_allocated_buffer(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step, uint8_t* ptrBuf, uint32_t bufSize);                                                      
int app_playback_resample_close(struct APP_RESAMPLE_T *resamp);
int app_playback_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len);

struct APP_RESAMPLE_T *app_capture_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
                                                 APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len);
struct APP_RESAMPLE_T *app_capture_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
                                                     APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
                                                     float ratio_step);
int app_capture_resample_close(struct APP_RESAMPLE_T *resamp);
int app_capture_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len);

void app_resample_reset(struct APP_RESAMPLE_T *resamp);
void app_resample_tune(struct APP_RESAMPLE_T *resamp, float ratio);
APP_RESAMPLE_BUF_ALLOC_CALLBACK app_resample_set_buf_alloc_callback(APP_RESAMPLE_BUF_ALLOC_CALLBACK cb);
uint16_t app_bt_stream_ibrt_trigger_seq_diff_calc(int32_t dma_samples, int32_t frame_samples, int32_t sub_seq, int32_t interval);
void app_bt_stream_volumeset_handler(int8_t vol);

#ifdef TX_RX_PCM_MASK
void store_encode_frame2buff();
#endif

int app_bt_stream_local_volume_get(void);

bool bt_is_playback_triggered(void);

int app_bt_stream_register_a2dp_param_callback(APP_STREAM_GET_A2DP_PARAM_CALLBACK callback);

int app_bt_stream_register_get_a2dp_non_type_callback(APP_STREAM_GET_A2DP_PARAM_CALLBACK callback);

void app_bt_stream_volume_edge_check(void);

typedef void (*APP_STREAM_NOTIFY_TRIGGER_STATUS_CALLBACK)(bool status);

void app_bt_stream_register_notify_trigger_status_callback(APP_STREAM_NOTIFY_TRIGGER_STATUS_CALLBACK callback);

typedef bool (*app_bt_sco_switch_trigger_t)(void);

void app_bt_sco_switch_trigger_init(app_bt_sco_switch_trigger_t sco_switch_trigger);

void app_bt_sco_switch_trigger_deinit(void);
#ifdef __cplusplus
}
#endif

#endif
