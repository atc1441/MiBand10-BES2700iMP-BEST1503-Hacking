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
//#include "mbed.h"
#include <stdio.h>
#include <assert.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "tgt_hardware.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_overlay.h"
#include "analog.h"
#include "app_overlay.h"
#include "app_audio.h"
#include "app_utils.h"
#include "btapp.h"
#include "app_a2dp.h"
#include "app_bt_sync.h"
#ifdef ANC_APP
#include "app_anc.h"
#endif
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "resample_coef.h"
#include "hal_codec.h"
#include "hal_i2s.h"
#include "hal_dma.h"
#include "hal_bootmode.h"
#include "app_karaoke.h"
#ifdef MEDIA_PLAYER_SUPPORT
#include "app_media_player.h"
#endif
#ifdef __FACTORY_MODE_SUPPORT__
#include "app_factory_audio.h"
#endif
#ifdef TX_RX_PCM_MASK
#include "hal_chipid.h"
#endif

#ifdef  __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef __AI_VOICE__
#include "app_ai_voice.h"
#include "app_ai_if.h"
#endif

#ifdef AI_AEC_CP_ACCEL
#include "app_ai_algorithm.h"
#endif

#include "app_ring_merge.h"
#include "bt_drv.h"
#include "bt_xtal_sync.h"
#include "bt_drv_reg_op.h"
#include "besbt.h"
#include "hal_chipid.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_hfp.h"
#include "app_bt.h"
#include "audio_policy.h"
#include "audio_process.h"
#include "voice_dev.h"
#include "app_a2dp.h"
#include "app_media_player.h"
#include "bt_sco_chain.h"

#if defined(BT_SOURCE)
#include "bt_source.h"
#if defined(APP_USB_A2DP_SOURCE)
#include "btusb_audio.h"
#endif
#endif
#include "audio_dump.h"
#include "a2dp_decoder.h"

#if defined(__AUDIO_SPECTRUM__)
#include "audio_spectrum.h"
#endif

#if defined(SPEECH_BONE_SENSOR)
#include "speech_bone_sensor.h"
#endif

#if defined(SPEECH_ALGO_DSP)
#include "mcpp_client.h"
#endif

#if defined(ANC_NOISE_TRACKER)
#include "noise_tracker.h"
#include "noise_tracker_callback.h"
#endif

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "earbud_ux_api.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_audio_analysis.h"
#include "app_tws_ibrt_audio_sync.h"
#include "app_ibrt_a2dp.h"
#undef MUSIC_DELAY_CONTROL
#endif

#if defined(IBRT_UI)
#include "app_tws_ibrt_conn_api.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#if defined(AUDIO_ANC_FB_ADJ_MC)
#include "adj_mc.h"
#include "fftfilt2.h"
#endif

#if defined(AUDIO_ADJ_EQ)
#include "adj_eq.h"
#include "fftfilt2.h"
#endif

#if defined(ANC_ASSIST_ENABLED)
#include "app_anc_assist.h"
#if defined(AUDIO_ADAPTIVE_EQ)
#include "app_voice_assist_adaptive_eq.h"
#endif
#if defined(ASSIST_LOW_RAM_MOD)
#include "custom_allocator.h"
#include "anc_assist_resample.h"
#include "integer_resampling.h"
IntegerResamplingState *anc_assist_resample_inst = NULL;
int32_t *anc_assist_resample_buf = NULL;
#endif
#if defined(VOICE_ASSIST_FF_FIR_LMS)
#include "app_voice_assist_fir_lms.h"
#endif
#endif

#if defined(AUDIO_PCM_PLAYER)
#include "app_bt_stream_pcm_player.h"
#endif

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
#include "sidetone_process.h"
extern const IIR_CFG_T * const  audio_sidetone_iir_cfg_list[];
int sidetone_opened=0;
#endif
#endif

#ifdef IS_MULTI_AI_ENABLED
#include "app_ai_if.h"
#endif

#if defined(APP_USB_A2DP_SOURCE)
#include "btusb_audio.h"
#endif

#include "audio_trigger_a2dp.h"
#include "audio_trigger_checker.h"
#if defined(IBRT)
#include "audio_trigger_ibrt.h"
#endif

#ifdef __BIXBY
#include "app_bixby_thirdparty_if.h"
#endif
#if defined(SPA_AUDIO_ENABLE)
#include "spa_app_main.h"
#ifdef SPA_AUDIO_SEC
#include "tz_audio_process.h"
#include "ree_audio_process.h"
#endif
#endif
#if defined(A2DP_LDAC_ON)
#include "bes_mem_api.h"
#endif
// #define A2DP_STREAM_AUDIO_DUMP      (16)

#if defined(A2DP_STREAM_AUDIO_DUMP)
static uint32_t g_a2dp_pcm_dump_frame_len = 0;
static uint32_t g_a2dp_pcm_dump_channel_num = 0;
static uint32_t g_a2dp_pcm_dump_sample_bytes = 0;
#if A2DP_STREAM_AUDIO_DUMP == 16
#define A2DP_PCM_DUMP_BUFFER_MAX_SIZE      2048
static int16_t g_a2dp_pcm_dump_buf[A2DP_PCM_DUMP_BUFFER_MAX_SIZE];
#endif
#endif

#if defined(__SW_IIR_EQ_PROCESS__)
static uint8_t audio_eq_sw_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[];
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_fir_index = 0;
extern const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[];
#if defined(__HW_FIR_EQ_PROCESS_2CH__)
extern const FIR_CFG_T * const audio_hw_hpfir_cfg_list[];
extern const FIR_CFG_T * const audio_hw_lpfir_cfg_list[];
#endif
#if defined(AUDIO_ADAPTIVE_FIR_EQ)
extern FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg;
#endif
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_dac_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_hw_dac_iir_cfg_list[];
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
extern const IIR_CFG_T * const audio_eq_hw_dac_iir_cfg_list2[];
#endif
#if defined(AUDIO_ADAPTIVE_IIR_EQ)
extern IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg;
#endif
#endif

#include "audio_prompt_sbc.h"

#if defined(__HW_IIR_EQ_PROCESS__)
static uint8_t audio_eq_hw_iir_index = 0;
extern const IIR_CFG_T * const audio_eq_hw_iir_cfg_list[];
#endif

#if defined(HW_DC_FILTER_WITH_IIR)
#include "hw_filter_codec_iir.h"
#include "hw_codec_iir_process.h"

static hw_filter_codec_iir_cfg POSSIBLY_UNUSED adc_iir_cfg = {
    .bypass = 0,
    .iir_device = HW_CODEC_IIR_ADC,
#if 1
    .iir_cfg = {
        .iir_filtes_l = {
            .iir_bypass_flag = 0,
            .iir_counter = 2,
            .iir_coef = {
                    {{0.994406, -1.988812, 0.994406}, {1.000000, -1.988781, 0.988843}}, // iir_designer('highpass', 0, 20, 0.7, 16000);
                    {{4.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},
            }
        },
        .iir_filtes_r = {
            .iir_bypass_flag = 0,
            .iir_counter = 2,
            .iir_coef = {
                    {{0.994406, -1.988812, 0.994406}, {1.000000, -1.988781, 0.988843}},
                    {{4.0, 0.0, 0.0}, {1.0, 0.0, 0.0}},
            }
        }
    }
#else
    .iir_cfg = {
        .gain0 = 0,
        .gain1 = 0,
        .num = 1,
        .param = {
            {IIR_TYPE_HIGH_PASS, 0,   20.0,   0.7},
        }
    }
#endif
};

hw_filter_codec_iir_state *hw_filter_codec_iir_st;
#endif

#if defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST3003)|| \
     defined(CHIP_BEST1400) || defined(CHIP_BEST1402)   || defined(CHIP_BEST1000) || \
     defined(CHIP_BEST2000) || defined(CHIP_BEST3001)   || defined(CHIP_BEST2001) \

#undef AUDIO_RESAMPLE_ANTI_DITHER

#else
#define  AUDIO_RESAMPLE_ANTI_DITHER
#endif

#include "audio_cfg.h"

// #define SCO_DUMP_BTPCM_DATA
//#define SCO_DMA_SNAPSHOT_DEBUG
#define SCO_TUNING_NEWMETHOD

extern uint8_t bt_audio_get_eq_index(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t anc_status);
extern uint32_t bt_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t index);
extern uint8_t bt_audio_updata_eq_for_anc(uint8_t anc_status);

#include "app_bt_media_manager.h"

#include "string.h"
#include "hal_location.h"

#include "bt_drv_interface.h"

#include "audio_resample_ex.h"

#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402) || defined(CHIP_BEST2001)
#define BT_INIT_XTAL_SYNC_FCAP_RANGE (0x1FF)
#else
#define BT_INIT_XTAL_SYNC_FCAP_RANGE (0xFF)
#endif
#define BT_INIT_XTAL_SYNC_MIN (20)
#define BT_INIT_XTAL_SYNC_MAX (BT_INIT_XTAL_SYNC_FCAP_RANGE - BT_INIT_XTAL_SYNC_MIN)

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#include"anc_process.h"

#ifdef ANC_FB_MC_96KHZ
#define DELAY_SAMPLE_MC (29*2)     //  2:ch
#define SAMPLERATE_RATIO_THRESHOLD (4) //384 = 96*4
#else
#define DELAY_SAMPLE_MC (31*2)     //  2:ch
#define SAMPLERATE_RATIO_THRESHOLD (8) //384 = 48*8
#endif

static int32_t delay_buf_bt[DELAY_SAMPLE_MC];
#endif

#ifdef ANC_APP
static uint8_t anc_status_record = 0xff;
#endif

#if defined(SCO_DMA_SNAPSHOT)

#if defined(BT_CLK_UNIT_IN_HSLOT)
#define BTCLK_UNIT (312.5f) //us
#define BTCNT_UNIT (0.5f) //us
#define MAX_BT_CLOCK   ((1L<<28) - 1)
#else
#define BTCLK_UNIT (625.0f) //us
#define BTCNT_UNIT (1.0f) //us
#define MAX_BT_CLOCK   ((1L<<27) - 1)
#endif

#define CODEC_TRIG_DELAY 2500.0f //us

#ifdef PCM_FAST_MODE
#define CODEC_TRIG_BTCLK_OFFSET ((uint32_t)(CODEC_TRIG_DELAY/BTCLK_UNIT))
#elif TX_RX_PCM_MASK
#define CODEC_TRIG_BTCLK_OFFSET (2*(uint32_t)(CODEC_TRIG_DELAY/BTCLK_UNIT))
#else
#define CODEC_TRIG_BTCLK_OFFSET ((uint32_t)(CODEC_TRIG_DELAY/BTCLK_UNIT))
#endif

#if defined(LOW_DELAY_SCO)
#define CODEC_BTPCM_BUF 7500.0f //us
#else
#define CODEC_BTPCM_BUF 15000.0f //us
#endif

#define BUF_BTCLK_NUM ((uint32_t)((CODEC_BTPCM_BUF/BTCLK_UNIT)))

#define WRAPED_CLK_OFFSET  (((MAX_BT_CLOCK/BUF_BTCLK_NUM+1)*BUF_BTCLK_NUM)&MAX_BT_CLOCK)

#define CODEC_TRIG_DELAY_OFFSET (CODEC_TRIG_BTCLK_OFFSET*BTCLK_UNIT)

#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300)
#define MUTE_PATTERN (0x00)
#else
#define MUTE_PATTERN (0x55)
#endif


//#define A2DP_DUMP_REF_FB

extern void  app_tws_ibrt_audio_mobile_clkcnt_get(uint8_t device_id, uint32_t btclk, uint16_t btcnt,
                                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt);

static uint8_t *playback_buf_codecpcm;
static uint32_t playback_size_codecpcm;
static uint8_t *capture_buf_codecpcm;
static uint32_t capture_size_codecpcm;

static uint8_t *playback_buf_btpcm;
static uint32_t playback_size_btpcm;
static uint8_t *capture_buf_btpcm;
static uint32_t capture_size_btpcm;

#ifdef TX_RX_PCM_MASK
static uint8_t *playback_buf_btpcm_copy=NULL;
static uint32_t playback_size_btpcm_copy=0;
static uint8_t *capture_buf_btpcm_copy=NULL;
static uint32_t capture_size_btpcm_copy=0;
#endif

volatile int sco_btpcm_mute_flag=0;
volatile int sco_disconnect_mute_flag=0;

static uint8_t *playback_buf_btpcm_cache=NULL;

static enum AUD_SAMPRATE_T playback_samplerate_codecpcm;
static int32_t mobile_master_clk_offset_init;
static uint32_t last_mobile_master_clk=0;
#endif

#if defined(TX_RX_PCM_MASK) || defined(PCM_PRIVATE_DATA_FLAG)
struct PCM_DATA_FLAG_T pcm_data_param[4];
#define PCM_PRIVATE_DATA_LENGTH 11
#define BTPCM_PRIVATE_DATA_LENGTH 32
#define BTPCM_PUBLIC_DATA_LENGTH 120
#define BTPCM_TOTAL_DATA_LENGTH (BTPCM_PRIVATE_DATA_LENGTH+BTPCM_PUBLIC_DATA_LENGTH)
#endif

enum PLAYER_OPER_T
{
    PLAYER_OPER_START,
    PLAYER_OPER_STOP,
    PLAYER_OPER_RESTART,
};

#if defined(AF_ADC_I2S_SYNC)
extern "C" void hal_codec_capture_enable(void);
extern "C" void hal_codec_capture_enable_delay(void);
#endif

int8_t stream_local_volume;
#ifdef AUDIO_LINEIN
int8_t stream_linein_volume;
#endif

struct btdevice_volume *btdevice_volume_p;
struct btdevice_volume default_stream_volume;
struct btdevice_volume current_btdevice_volume;

#ifdef AUDIO_RMS_MONITOR_ENABLE
uint32_t rms_debug_cnt = 0xFFFF;
int32_t rms_dac_volume = 0;
#endif

#ifdef __BT_ANC__
static uint32_t bt_sco_upsampling_ratio = 1;
static uint8_t *bt_anc_sco_dec_buf;
extern void us_fir_init(uint32_t upsampling_ratio);
extern uint32_t voicebtpcm_pcm_resample (short *src_samp_buf, uint32_t src_smpl_cnt, short *dst_samp_buf);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static enum AUD_BITS_T sample_size_play_bt;
static enum AUD_SAMPRATE_T sample_rate_play_bt;
static uint32_t data_size_play_bt;

static uint8_t *playback_buf_bt;
static uint32_t playback_size_bt;
static int32_t playback_samplerate_ratio_bt;

static uint8_t *playback_buf_mc;
static uint32_t playback_size_mc;
static enum AUD_CHANNEL_NUM_T  playback_ch_num_bt;
#ifdef AUDIO_ANC_FB_ADJ_MC
uint32_t adj_mc_capture_sample_rate;
#endif
#endif

#ifdef AUDIO_ADJ_EQ
uint32_t adj_eq_capture_sample_rate;
#endif

#if defined(LOW_DELAY_SCO)
static int32_t  btpcm_int_counter;
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
static enum AUD_BITS_T lowdelay_sample_size_play_bt;
static enum AUD_SAMPRATE_T lowdelay_sample_rate_play_bt;
static uint32_t lowdelay_data_size_play_bt;
static enum AUD_CHANNEL_NUM_T  lowdelay_playback_ch_num_bt;
#endif

#if defined(VOICE_DEV)
#if defined(APP_NOISE_ESTIMATION)
int app_noise_estimation_open(void);
int app_noise_estimation_close(void);
int app_noise_estimation_start(void);
int app_noise_estimation_stop(void);
#endif
#endif

static uint32_t a2dp_data_buf_size;
#ifdef AUDIO_OUTPUT_ROUTE_SELECT

#define NOTIFY_RECEIVE_DATA_SIGNAL        0x01
#define A2DP_CHACE_DATA_NUM               0X02

typedef uint32_t (*bt_stream_read_callback)(uint8_t *data_ptr, uint32_t data_len);

extern void a2dp_audio_reg_store_data_ntf_cb(void *cb);
FRAM_TEXT_LOC uint32_t bt_sbc_player_more_data(uint8_t *buf, uint32_t len);
static void app_bt_output_thread(const void *arg);

typedef struct
{
    // 0: player send,  1:device read
    uint8_t data_transfer_mode;
    void (*output_start)(uint32_t sample_rate, uint8_t bits_depth, uint8_t num_channels, uint32_t frame_samples, uint8_t vol);
    void (*output_stop)(void);
    void (*output_set_volume)(uint8_t vol);
    // data_transfer_mode = 0
    void (*output_data)(uint8_t *data_ptr, uint32_t data_len);
    // data_transfer_mode = 1
    void (*output_receive_ntf)(void);
    void (*output_reg_read_cb)(bt_stream_read_callback cb);
} app_bt_stream_output_device_info_t;

typedef struct
{
    uint8_t cache_pack_num;
    uint8_t codec_type;
    uint8_t *send_buf;
    osMutexId     mutex_id;
    osMutexAttr_t mutex_attr;
    osThreadId output_thread_id;
    uint32_t buffer_samples;
    app_bt_stream_output_device_info_t *output_route_info;
} app_bt_stream_route_env_t;

static osThreadDef(app_bt_output_thread, osPriorityHigh, 1, (1024*2), "app_bt_decode_thread");
static app_bt_stream_route_env_t stream_route_info = {0};

static bool app_bt_stream_store_data_not_enough(void)
{
    if (stream_route_info.codec_type == BT_A2DP_CODEC_TYPE_SBC)
    {
        if (a2dp_audio_list_length_TWP() >= (a2dp_data_buf_size/1024))
            return true;
        else
            return false;
    }
    else
    {
        if (a2dp_audio_list_length_TWP())
            return true;
        else
            return false;
    }
}

static void app_bt_output_thread(const void *arg)
{
    osEvent evt;
    while(1)
    {
        evt = osSignalWait(NOTIFY_RECEIVE_DATA_SIGNAL, osWaitForever);

        osMutexAcquire(stream_route_info.mutex_id, osWaitForever);
        if (!stream_route_info.buffer_samples)
        {
            osMutexRelease(stream_route_info.mutex_id);
            break;
        }

        if (evt.status == osEventSignal)
        {
            if(app_bt_stream_store_data_not_enough())
            {
                bt_sbc_player_more_data(stream_route_info.send_buf,stream_route_info.buffer_samples);

                if (stream_route_info.output_route_info)
                {
                    stream_route_info.output_route_info->output_data(stream_route_info.send_buf, stream_route_info.buffer_samples);
                }
            }
        }

        osMutexRelease(stream_route_info.mutex_id);
    }
}

static void app_bt_notify_receive_bt_meida_data(void)
{
    osMutexAcquire(stream_route_info.mutex_id, osWaitForever);
    if (stream_route_info.buffer_samples && app_bt_stream_store_data_not_enough())
    {
        if (stream_route_info.codec_type == BT_A2DP_CODEC_TYPE_SBC)
        {
            if (!(a2dp_audio_list_length_TWP()/(a2dp_data_buf_size/1024)))
            {
                osMutexRelease(stream_route_info.mutex_id);
                return;
            }
        }

        // stream_route_info.output_route_info->data_transfer_mode != 0
        if (stream_route_info.output_route_info
            && stream_route_info.output_route_info->data_transfer_mode)
        {
            stream_route_info.output_route_info->output_receive_ntf();
            osMutexRelease(stream_route_info.mutex_id);
            return;
        }

        // stream_route_info.output_route_info->data_transfer_mode == 0
        if(stream_route_info.cache_pack_num > A2DP_CHACE_DATA_NUM)
        {
            if (stream_route_info.output_thread_id)
            {
                osSignalSet(stream_route_info.output_thread_id, NOTIFY_RECEIVE_DATA_SIGNAL);
            }
        }
        else
        {
            stream_route_info.cache_pack_num++;
        }
    }
    osMutexRelease(stream_route_info.mutex_id);
}

static uint32 app_bt_stream_read_meida_data(uint8_t *data_ptr, uint32_t data_len)
{
    uint32_t read_data_len = 0;

    osMutexAcquire(stream_route_info.mutex_id, osWaitForever);
    if(stream_route_info.buffer_samples && app_bt_stream_store_data_not_enough())
    {
        bt_sbc_player_more_data(data_ptr, data_len);
        read_data_len = data_len;
    }
    osMutexRelease(stream_route_info.mutex_id);

    return read_data_len;
}

static void app_bt_output_route_init(uint32_t sample_rate, uint8_t bits_depth, uint8_t num_channels, uint32_t frame_samples, uint8_t vol)
{
    TRACE(1,"app_bt_output_route_init start");

    stream_route_info.mutex_id = osMutexNew(&stream_route_info.mutex_attr);

    if (bits_depth <= AUD_BITS_16){
        stream_route_info.buffer_samples = a2dp_data_buf_size/num_channels/2;
    }else{
        stream_route_info.buffer_samples = a2dp_data_buf_size/num_channels/4;
    }

    if (stream_route_info.output_route_info)
    {
        TRACE(1,"route_init,ttransfer_mode %d", stream_route_info.output_route_info->data_transfer_mode);
        if(stream_route_info.output_route_info->data_transfer_mode)
        {
            TRACE(1,"route_init,read_data_func = 0x%p", app_bt_stream_read_meida_data);
            stream_route_info.output_route_info->output_reg_read_cb(app_bt_stream_read_meida_data);
        }
        else
        {
            stream_route_info.cache_pack_num = 0;
            stream_route_info.output_thread_id = osThreadCreate(osThread(app_bt_output_thread), NULL);
            TRACE(1,"route_init,thread_id 0x%p", stream_route_info.output_thread_id);
        }

        stream_route_info.output_route_info->output_start(sample_rate, bits_depth, num_channels, frame_samples, vol);
    }

    #if (A2DP_DECODER_VER == 2)
        a2dp_audio_store_packet_ntf_callback_register(app_bt_notify_receive_bt_meida_data);
    #else
        a2dp_audio_reg_store_data_ntf_cb((void *)app_bt_notify_receive_bt_meida_data);
    #endif
    TRACE(1,"app_bt_output_route_init end");

}

static void app_bt_output_route_deinit(void)
{
    osStatus evt;

#if (A2DP_DECODER_VER == 2)
    a2dp_audio_store_packet_ntf_callback_register(NULL);
#else
    a2dp_audio_reg_store_data_ntf_cb(NULL);
#endif

    osMutexAcquire(stream_route_info.mutex_id, osWaitForever);
    stream_route_info.buffer_samples   = 0;
    if (stream_route_info.output_route_info)
    {
        stream_route_info.output_route_info->output_stop();
    }

    if(stream_route_info.output_thread_id)
    {
        evt = osThreadTerminate(stream_route_info.output_thread_id);
        if(evt == osOK) {
            stream_route_info.output_thread_id = NULL;
        }
    }
    osMutexRelease(stream_route_info.mutex_id);
    osMutexDelete(stream_route_info.mutex_id);

    TRACE(0,"app_bt_decode_to_pcm_thread end");
}
#endif



#define A2DP_SCO_TRIGGER_CHANNEL    0

static APP_STREAM_NOTIFY_TRIGGER_STATUS_CALLBACK app_stream_notify_trigger_status_callback = NULL;
static app_bt_sco_switch_trigger_t sco_switch_trigger_check = NULL;
extern void bt_media_clear_media_type(uint16_t media_type,int device_id);

extern "C" uint8_t is_a2dp_mode(void);
uint8_t bt_a2dp_mode;

extern "C" FRAM_TEXT_LOC uint8_t is_a2dp_mode(void)
{
    return bt_a2dp_mode;
}

extern "C" uint8_t is_sco_mode(void);
uint8_t bt_sco_mode;
extern "C" FRAM_TEXT_LOC uint8_t is_sco_mode(void)
{
    return bt_sco_mode;
}

extern "C" FRAM_TEXT_LOC uint8_t amgr_is_bluetooth_sco_on()
{
    return bt_sco_mode;
}

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
#define LHDC_EXT_FLAGS_JAS    0x01
#define LHDC_EXT_FLAGS_AR     0x02
#define LHDC_EXT_FLAGS_LLAC   0x04
#define LHDC_EXT_FLAGS_MQA    0x08
#define LHDC_EXT_FLAGS_MBR    0x10
#define LHDC_EXT_FLAGS_LARC   0x20
#define LHDC_EXT_FLAGS_V4     0x40
#define LHDC_SET_EXT_FLAGS(X) (lhdc_ext_flags |= X)
#define LHDC_CLR_EXT_FLAGS(X) (lhdc_ext_flags &= ~X)
#define LHDC_CLR_ALL_EXT_FLAGS() (lhdc_ext_flags = 0)
extern "C" bool a2dp_lhdc_get_ext_flags(uint32_t flags);
extern "C" uint8_t a2dp_lhdc_config_llc_get(void);
#endif

APP_STREAM_GET_A2DP_PARAM_CALLBACK app_stream_get_a2dp_non_type_callback = NULL;

#define SYNCHRONIZE_NEED_DISCARDS_DMA_CNT 4

uint32_t app_bt_stream_get_dma_buffer_samples(void);
#if defined(IBRT)
extern "C" bool is_need_discards_samples;
#endif

int app_bt_stream_trigger_success(uint32_t trigger_checker)
{
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
#if defined(IBRT)
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC) && is_need_discards_samples) {
        //limter to water line upper
        uint32_t list_samples = 0;
        a2dp_audio_convert_list_to_samples(&list_samples);
        TRACE_AUD_STREAM_W("[STRM_TRIG][CHK] synchronize:%d", list_samples);
    }
#endif
#endif
    app_bt_stream_trigger_checker_stop();
    if (app_stream_notify_trigger_status_callback)
    {
        app_stream_notify_trigger_status_callback(true);
    }
    return 0;
}

uint16_t gStreamplayer = APP_BT_STREAM_INVALID;

uint32_t a2dp_audio_more_data(uint8_t codec_type, uint8_t *buf, uint32_t len);

enum AUD_SAMPRATE_T a2dp_sample_rate = AUD_SAMPRATE_48000;
#ifdef RB_CODEC
extern int app_rbplay_audio_onoff(bool onoff, uint16_t aud_id);
#endif

enum AUD_SAMPRATE_T bt_parse_music_sample_rate(uint32_t sbc_samp_rate)
{
    enum AUD_SAMPRATE_T sample_rate;
    sbc_samp_rate = sbc_samp_rate & A2D_STREAM_SAMP_FREQ_MSK;

    switch (sbc_samp_rate)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
//            sample_rate = AUD_SAMPRATE_16000;
//            break;
        case A2D_SBC_IE_SAMP_FREQ_32:
//            sample_rate = AUD_SAMPRATE_32000;
//            break;
        case A2D_SBC_IE_SAMP_FREQ_48:
            sample_rate = AUD_SAMPRATE_48000;
            break;
        case A2D_SBC_IE_SAMP_FREQ_44:
            sample_rate = AUD_SAMPRATE_44100;
            break;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) || defined(A2DP_SCALABLE_ON) || defined(A2DP_LC3_ON) || defined(A2DP_LDAC_ON)
        case A2D_SBC_IE_SAMP_FREQ_96:
            sample_rate = AUD_SAMPRATE_96000;
            break;
#endif
#if defined(A2DP_LHDCV5_ON)
         case A2D_SBC_IE_SAMP_FREQ_192:
            sample_rate = AUD_SAMPRATE_192000;
            break;
#endif //A2DP_LHDCV5_ON
        default:
            ASSERT(0, "[%s] 0x%x is invalid", __func__, sbc_samp_rate);
            break;
    }
    return sample_rate;
}

void bt_store_music_sample_rate(enum AUD_SAMPRATE_T sample_rate)
{
    a2dp_sample_rate = sample_rate;
}

enum AUD_SAMPRATE_T bt_get_sbc_sample_rate(void)
{
    return a2dp_sample_rate;
}

enum AUD_SAMPRATE_T bt_parse_store_music_sample_rate(uint32_t sbc_samp_rate)
{
    enum AUD_SAMPRATE_T sample_rate;

    sample_rate = bt_parse_music_sample_rate(sbc_samp_rate);
    bt_store_music_sample_rate(sample_rate);

    return sample_rate;
}

int app_bt_stream_register_get_a2dp_non_type_callback(APP_STREAM_GET_A2DP_PARAM_CALLBACK callback)
{
    app_stream_get_a2dp_non_type_callback = callback;

    return 0;
}

int bt_sbc_player_setup(uint8_t freq)
{
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;
    static uint8_t sbc_samp_rate = 0xff;
    uint32_t ret;

    if (sbc_samp_rate == freq)
        return 0;

    switch (freq)
    {
        case A2D_SBC_IE_SAMP_FREQ_16:
        case A2D_SBC_IE_SAMP_FREQ_32:
        case A2D_SBC_IE_SAMP_FREQ_48:
            a2dp_sample_rate = AUD_SAMPRATE_48000;
            break;
#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) ||defined(A2DP_SCALABLE_ON) || defined(A2DP_LC3_ON) || defined(A2DP_LDAC_ON) || defined(A2DP_L2HC_ON)
        case A2D_SBC_IE_SAMP_FREQ_96:
            a2dp_sample_rate = AUD_SAMPRATE_96000;
            TRACE_AUD_STREAM_I("%s:Sample rate :%d", __func__, freq);
            break;
        case A2D_SBC_IE_SAMP_FREQ_88D2:
            a2dp_sample_rate = AUD_SAMPRATE_88200;
            TRACE_AUD_STREAM_I("%s:Sample rate :%d", __func__, freq);
            break;
#endif
#if defined(A2DP_LDAC_ON) || defined(A2DP_LHDCV5_ON)
        case A2D_SBC_IE_SAMP_FREQ_192:
            a2dp_sample_rate = AUD_SAMPRATE_192000;
            TRACE_AUD_STREAM_I("%s:Sample rate :%d", __func__, freq);
            break;
#endif //A2DP_LHDCV5_ON or A2DP_LHDCV5_ON
        case A2D_SBC_IE_SAMP_FREQ_44:
            a2dp_sample_rate = AUD_SAMPRATE_44100;
            break;
        default:
            break;
    }

    ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, true);
    if (ret == 0) {
        stream_cfg->sample_rate = a2dp_sample_rate;
        af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, stream_cfg);
    }

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    ret = af_stream_get_cfg(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg, true);
    if (ret == 0) {
        stream_cfg->sample_rate = a2dp_sample_rate;
        sample_rate_play_bt=stream_cfg->sample_rate;
        af_stream_setup(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, stream_cfg);
        anc_mc_run_setup(hal_codec_anc_convert_rate(sample_rate_play_bt));
    }
#endif

    sbc_samp_rate = freq;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_forcefully_stop();
    }
#endif

    return 0;
}

void merge_stereo_to_mono_16bits(int16_t *src_buf, int16_t *dst_buf,  uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2)
    {
        dst_buf[i] = (src_buf[i]>>1) + (src_buf[i+1]>>1);
        dst_buf[i+1] = dst_buf[i];
    }
}

void merge_stereo_to_mono_24bits(int32_t *src_buf, int32_t *dst_buf,  uint32_t src_len)
{
    uint32_t i = 0;
    for (i = 0; i < src_len; i+=2)
    {
        dst_buf[i] = (src_buf[i]>>1) + (src_buf[i+1]>>1);
        dst_buf[i+1] = dst_buf[i];
    }
}

const char *audio_op_to_str(enum APP_BT_SETTING_T op)
{
    switch (op)
    {
    case APP_BT_SETTING_OPEN: return "[OPEN]";
    case APP_BT_SETTING_CLOSE: return "[CLOSE]";
    case APP_BT_SETTING_SETUP: return "[SETUP]";
    case APP_BT_SETTING_RESTART: return "[RESTART]";
    case APP_BT_SETTING_CLOSEALL: return "[CLOSEALL]";
    case APP_BT_SETTING_CLOSEMEDIA: return "[CLOSEMEDIA]";
    default: return "[N/A]";
    }
}

static char _player_type_str[168];
static char *_catstr(char *dst, const char *src) {
     while(*dst) dst++;
     while((*dst++ = *src++));
     return --dst;
}
const char *player2str(uint16_t player_type) {
    const char *s = NULL;
    char _cat = 0, first = 1, *d = NULL;
    _player_type_str[0] = '\0';
    d = _player_type_str;
    d = _catstr(d, "[");
    if (player_type != 0) {
        for (int i = 15 ; i >= 0; i--) {
            _cat = 1;
            //TRACE_AUD_STREAM_I("i=%d,player_type=0x%d,player_type&(1<<i)=0x%x", i, player_type, player_type&(1<<i));
            switch(player_type&(1<<i)) {
                case 0: _cat = 0; break;
                case APP_BT_STREAM_HFP_PCM: s = "HFP_PCM"; break;
                case APP_BT_STREAM_HFP_CVSD: s = "HFP_CVSD"; break;
                case APP_BT_STREAM_HFP_VENDOR: s = "HFP_VENDOR"; break;
                case APP_BT_STREAM_A2DP_SBC: s = "A2DP_SBC"; break;
                case APP_BT_STREAM_A2DP_AAC: s = "A2DP_AAC"; break;
                case APP_BT_STREAM_A2DP_VENDOR: s = "A2DP_VENDOR"; break;
            #ifdef __FACTORY_MODE_SUPPORT__
                case APP_FACTORYMODE_AUDIO_LOOP: s = "AUDIO_LOOP"; break;
            #endif
                case APP_PLAY_BACK_AUDIO: s = "BACK_AUDIO"; break;
            #ifdef RB_CODEC
                case APP_BT_STREAM_RBCODEC: s = "RBCODEC"; break;
            #endif
            #ifdef AUDIO_LINEIN
                case APP_PLAY_LINEIN_AUDIO: s = "LINEIN_AUDIO"; break;
            #endif
            #ifdef AUDIO_PCM_PLAYER
                case APP_PLAY_PCM_PLAYER: s = "PCM_PLAYER"; break;
            #endif
            #if defined(BT_SOURCE)
                case APP_A2DP_SOURCE_LINEIN_AUDIO: s = "SRC_LINEIN_AUDIO"; break;
                case APP_A2DP_SOURCE_I2S_AUDIO: s = "I2S_AUDIO"; break;
                case APP_A2DP_SOURCE_USB_AUDIO: s = "SRC_USB_AUDIO"; break;
            #endif
            #ifdef __AI_VOICE__
                case APP_BT_STREAM_AI_VOICE: s = "AI_VOICE"; break;
            #endif
                default:  s = "UNKNOWN"; break;
            }
            if (_cat) {
                if (!first)
                    d = _catstr(d, "|");
                //TRACE_AUD_STREAM_I("d=%s,s=%s", d, s);
                d = _catstr(d, s);
                first = 0;
            }
        }
    }

    _catstr(d, "]");

    return _player_type_str;
}


#ifdef __HEAR_THRU_PEAK_DET__
#include "peak_detector.h"
// Depend on codec_dac_vol
static const float pkd_vol_multiple[18] = {
    0.281838, 0.000010, 0.005623, 0.007943, 0.011220, 0.015849,
    0.022387, 0.031623, 0.044668, 0.063096, 0.089125, 0.125893,
    0.177828, 0.251189, 0.354813, 0.501187, 0.707946, 1.000000
};

int app_bt_stream_local_volume_get(void);
#endif

#ifdef PLAYBACK_FORCE_48K
static uint32_t force48k_pcm_bytes = sizeof(int32_t);
static bool force48k_resample_needed = true;
static struct APP_RESAMPLE_T *force48k_resample;
static int app_force48k_resample_iter(uint8_t *buf, uint32_t len);
struct APP_RESAMPLE_T *app_force48k_resample_any_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step);
int app_playback_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len);
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))

#define BT_USPERCLK (625)
#define BT_MUTIUSPERSECOND (1000000/BT_USPERCLK)

#define CALIB_DEVIATION_MS (2)
#define CALIB_FACTOR_MAX_THRESHOLD (0.0001f)
#define CALIB_BT_CLOCK_FACTOR_STEP (0.0000005f)

#define CALIB_FACTOR_DELAY (0.001f)

//bt time
static int32_t  bt_old_clock_us=0;
static uint32_t bt_old_clock_mutius=0;
static int32_t  bt_old_offset_us=0;

static int32_t  bt_clock_us=0;
static uint32_t bt_clock_total_mutius=0;
static int32_t  bt_total_offset_us=0;

static int32_t bt_clock_ms=0;

//local time
static uint32_t local_total_samples=0;
static uint32_t local_total_frames=0;

//static uint32_t local_clock_us=0;
static int32_t local_clock_ms=0;

//bt and local time
static uint32_t bt_local_clock_s=0;

//calib time
static int32_t calib_total_delay=0;
static int32_t calib_flag=0;

//calib factor
static float   calib_factor_offset=0.0f;
static int32_t calib_factor_flag=0;
static volatile int calib_reset=1;
#endif

bool process_delay(int32_t delay_ms)
{
#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
    if (delay_ms == 0)return 0;

    TRACE_AUD_STREAM_I("delay_ms:%d", delay_ms);

    if(calib_flag==0)
    {
        calib_total_delay=calib_total_delay+delay_ms;
        calib_flag=1;
        return 1;
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
void a2dp_clock_calib_process(uint32_t len)
{
//    btif_remote_device_t   * p_a2dp_remDev=NULL;
    uint32_t smplcnt = 0;
    int32_t btoffset = 0;

    uint32_t btclk = 0;
    uint32_t btcnt = 0;
    uint32_t btofs = 0;
    btclk = *((volatile uint32_t*)0xd02201fc);
    btcnt = *((volatile uint32_t*)0xd02201f8);
    btcnt=0;

   // TRACE_AUD_STREAM_I("bt_sbc_player_more_data btclk:%08x,btcnt:%08x\n", btclk, btcnt);


 //  a2dp_get_curStream_remDev(&p_a2dp_remDev);
    if(a2dp_Get_curr_a2dp_conhdl() >=0x80 && a2dp_Get_curr_a2dp_conhdl()<=0x82)
    {
        btofs = btdrv_rf_bitoffset_get( a2dp_Get_curr_a2dp_conhdl() -0x80);

        if(calib_reset==1)
        {
            calib_reset=0;

            bt_clock_total_mutius=0;

            bt_old_clock_us=btcnt;
            bt_old_clock_mutius=btclk;

            bt_total_offset_us=0;


            local_total_samples=0;
            local_total_frames=0;
            local_clock_ms = 0;

            bt_local_clock_s=0;
            bt_clock_us = 0;
            bt_clock_ms = 0;

            bt_old_offset_us=btofs;

            calib_factor_offset=0.0f;
            calib_factor_flag=0;
            calib_total_delay=0;
            calib_flag=0;
        }
        else
        {
            btoffset=btofs-bt_old_offset_us;

            if(btoffset<-BT_USPERCLK/3)
            {
                btoffset=btoffset+BT_USPERCLK;
            }
            else if(btoffset>BT_USPERCLK/3)
            {
                btoffset=btoffset-BT_USPERCLK;
            }

            bt_total_offset_us=bt_total_offset_us+btoffset;
            bt_old_offset_us=btofs;

            local_total_frames++;
            if(lowdelay_sample_size_play_bt==AUD_BITS_16)
            {
                smplcnt=len/(2*lowdelay_playback_ch_num_bt);
            }
            else
            {
                smplcnt=len/(4*lowdelay_playback_ch_num_bt);
            }

            local_total_samples=local_total_samples+smplcnt;

            bt_clock_us=btcnt-bt_old_clock_us-bt_total_offset_us;

            btoffset=btclk-bt_old_clock_mutius;
            if(btoffset<0)
            {
                btoffset=0;
            }
            bt_clock_total_mutius=bt_clock_total_mutius+btoffset;

            bt_old_clock_us=btcnt;
            bt_old_clock_mutius=btclk;

            if((bt_clock_total_mutius>BT_MUTIUSPERSECOND)&&(local_total_samples>lowdelay_sample_rate_play_bt))
            {
                bt_local_clock_s++;
                bt_clock_total_mutius=bt_clock_total_mutius-BT_MUTIUSPERSECOND;
                local_total_samples=local_total_samples-lowdelay_sample_rate_play_bt;
            }

            bt_clock_ms=(bt_clock_total_mutius*BT_USPERCLK/1000)+bt_clock_us/625;
            local_clock_ms=(local_total_samples*1000)/lowdelay_sample_rate_play_bt;

            local_clock_ms=local_clock_ms+calib_total_delay;

            //TRACE_AUD_STREAM_I("A2DP bt_clock_ms:%8d,local_clock_ms:%8d,bt_total_offset_us:%8d\n",bt_clock_ms, local_clock_ms,bt_total_offset_us);

            if(bt_clock_ms>(local_clock_ms+CALIB_DEVIATION_MS))
            {
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                app_resample_tune(a2dp_resample, CALIB_FACTOR_DELAY);
#else
                af_codec_tune(AUD_STREAM_PLAYBACK, CALIB_FACTOR_DELAY);
#endif
                calib_factor_flag=1;
                //TRACE_AUD_STREAM_I("*************1***************");
            }
            else if(bt_clock_ms<(local_clock_ms-CALIB_DEVIATION_MS))
            {
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                app_resample_tune(a2dp_resample, -CALIB_FACTOR_DELAY);
#else
                af_codec_tune(AUD_STREAM_PLAYBACK, -CALIB_FACTOR_DELAY);
#endif
                calib_factor_flag=-1;
                //TRACE_AUD_STREAM_I("*************-1***************");
            }
            else
            {
                if((calib_factor_flag==1||calib_factor_flag==-1)&&(bt_clock_ms==local_clock_ms))
                {
                    if(calib_factor_offset<CALIB_FACTOR_MAX_THRESHOLD&&calib_flag==0)
                    {
                        if(calib_factor_flag==1)
                        {
                            calib_factor_offset=calib_factor_offset+CALIB_BT_CLOCK_FACTOR_STEP;
                        }
                        else
                        {
                            calib_factor_offset=calib_factor_offset-CALIB_BT_CLOCK_FACTOR_STEP;
                        }

                    }
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
                    app_resample_tune(a2dp_resample, calib_factor_offset);
#else
                    af_codec_tune(AUD_STREAM_PLAYBACK, calib_factor_offset);
#endif
                    calib_factor_flag=0;
                    calib_flag=0;
                    //TRACE_AUD_STREAM_I("*************0***************");
                }
            }
          //  TRACE_AUD_STREAM_I("factoroffset:%d\n",(int32_t)((factoroffset)*(float)10000000.0f));
        }
    }

    return;
}

#endif

bool app_if_need_fix_target_rxbit(void)
{
    return (!bt_drv_is_enhanced_ibrt_rom());
}

static uint8_t isBtPlaybackTriggered = false;

bool bt_is_playback_triggered(void)
{
    return isBtPlaybackTriggered;
}

static void bt_set_playback_triggered(bool isEnable)
{
    TRACE(0, "%s isEnable %d", __func__, isEnable);
    isBtPlaybackTriggered = isEnable;
}

static void inline a2dp_audio_convert_16bit_to_24bit(int32_t *out, int16_t *in, int pcm_len)
{
    for (int i = pcm_len - 1; i >= 0; i--) {
        out[i] = ((int32_t)in[i] << 8);
    }
}

#if defined(AUDIO_ADJ_EQ)
#define AUDIO_DELAY 48*2
#define AUDIO_MAX_LEN 1280*2
#define AUDIO_CACHE_LEN AUDIO_DELAY+AUDIO_MAX_LEN
static int32_t audio_tmp_buffer[AUDIO_MAX_LEN];
static int32_t audio_cache_buffer[AUDIO_CACHE_LEN];

extern int get_ibrt_ch(void);
extern int32_t g_adj_eq;
#endif

FRAM_TEXT_LOC uint32_t bt_sbc_player_more_data(uint8_t *buf, uint32_t len)
{
    POSSIBLY_UNUSED uint8_t nonType = bta_get_curr_a2dp_vender_codec_type();
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_A2DP_PLAYERBLACK);
#if defined(A2DP_STREAM_AUDIO_DUMP)
    audio_dump_clear_up();
#if A2DP_STREAM_AUDIO_DUMP == 16
    if (g_a2dp_pcm_dump_sample_bytes == sizeof(int32_t)) {
        int32_t *buf_int = (int32_t *)buf;
        for (uint16_t i=0; i<g_a2dp_pcm_dump_frame_len; i++) {
            g_a2dp_pcm_dump_buf[i] = buf_int[g_a2dp_pcm_dump_channel_num * i] >> 8;
        }
        audio_dump_add_channel_data(0, g_a2dp_pcm_dump_buf, g_a2dp_pcm_dump_frame_len);
    } else {
        audio_dump_add_channel_data_from_multi_channels(0, buf, g_a2dp_pcm_dump_frame_len, g_a2dp_pcm_dump_channel_num, 0);
    }
#else
    audio_dump_add_channel_data_from_multi_channels(0, buf, g_a2dp_pcm_dump_frame_len, g_a2dp_pcm_dump_channel_num, 0);
#endif

    audio_dump_run();
#endif

    if (!bt_is_playback_triggered())
    {
#if defined(IBRT)
        app_tws_ibrt_request_modify_tws_bandwidth(TWS_TIMING_CONTROL_USER_FAST_COMMUNICATION, false);
        app_tws_ibrt_request_modify_tws_bandwidth(TWS_TIMING_CONTROL_USER_A2DP, true);
#endif
        bt_set_playback_triggered(true);
    }

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
    a2dp_clock_calib_process(len);
#endif

#ifdef BT_XTAL_SYNC
#ifdef BT_XTAL_SYNC_NEW_METHOD
    uint16_t a2dp_channel = a2dp_Get_curr_a2dp_conhdl();
    if(a2dp_channel >= 0x80 && a2dp_channel <= 0x82)
    {
        uint32_t bitoffset = btdrv_rf_bitoffset_get(a2dp_channel - 0x80);
        bt_xtal_sync_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
    }

#else
    bt_xtal_sync(BT_XTAL_SYNC_MODE_MUSIC);
#endif
#endif

#ifndef FPGA
    uint8_t codec_type = bta_get_curr_a2dp_codec_type();
    uint32_t overlay_id = 0;
    if(codec_type ==  BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
    {
        overlay_id = APP_OVERLAY_A2DP_AAC;
    }
    else if(codec_type ==  BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
#if defined(A2DP_LHDC_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)
        {
            overlay_id = APP_OVERLAY_A2DP_LHDC;
        } else
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
        {
            overlay_id = APP_OVERLAY_A2DP_LHDC_V5;
        } else
#endif // A2DP_LHDCV5_ON
#if defined(A2DP_LDAC_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)
        {
            overlay_id = APP_OVERLAY_A2DP_LDAC;
        } else
#endif // A2DP_LDAC_ON
#if defined(A2DP_SCALABLE_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
        {
            overlay_id = APP_OVERLAY_A2DP_SCALABLE;
        } else
#endif // A2DP_SCALABLE_ON
#if defined(A2DP_LC3_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_LC3)
        {
            overlay_id = APP_OVERLAY_A2DP_LC3;
        } else
#endif // A2DP_LC3_ON
#if defined(A2DP_L2HC_ON)
        if (nonType == BT_A2DP_CODEC_NONE_TYPE_L2HC)
        {
            overlay_id = APP_OVERLAY_A2DP_L2HC;
        } else
#endif // A2DP_L2HC_ON
        {
            TRACE(0, "WARNING nonType Invalid");
        }
    }
    else
    {
        overlay_id = APP_OVERLAY_A2DP;
    }

    memset(buf, 0, len);

    if(app_get_current_overlay() != overlay_id)
    {
        return len;
    }
#endif

#if defined(PLAYBACK_FORCE_48K)
    len = len / (force48k_pcm_bytes / sizeof(int16_t));
    if (force48k_resample_needed) {
        app_playback_resample_run(force48k_resample, buf, len);
    } else
#endif
    {
#if (A2DP_DECODER_VER == 2)
        a2dp_audio_playback_handler(device_id, buf, len);
#else
#ifndef FPGA
        a2dp_audio_more_data(overlay_id, buf, len);
#endif
#endif
    }

#if defined(AUDIO_ADJ_EQ)
    TRACE(0,"[%s] g_adj_eq = %d", __func__, g_adj_eq);
    //g_adj_eq = 0;
    uint32_t current_ch = 2;
    if(g_adj_eq){
        adj_eq_run_mono(buf, current_ch-2, audio_tmp_buffer, len, AUD_BITS_24);
        int32_t *tmp_buf = (int32_t *)buf;
        for(uint32_t i= AUDIO_DELAY; i< len/4 + AUDIO_DELAY; i++){
            audio_cache_buffer[i] = tmp_buf[i-AUDIO_DELAY];
        }

        for(uint32_t i=0; i<len/4; i++){
            tmp_buf[i] = audio_cache_buffer[i];
        }

        for(uint32_t i=0; i<AUDIO_CACHE_LEN-len/4; i++){
            audio_cache_buffer[i] = audio_cache_buffer[i+len/4];
        }

        adj_eq_audio_mix(tmp_buf, current_ch-2, audio_tmp_buffer, len/4);
    }else{
        int32_t *tmp_buf = (int32_t *)buf;
        for(uint32_t i=0; i<len/8; i++){
            tmp_buf[2*i+current_ch-2] /= 4;
        }
    }
#endif

#if defined(PLAYBACK_FORCE_48K)
    if (force48k_pcm_bytes == sizeof(int32_t)) {
        a2dp_audio_convert_16bit_to_24bit((int32_t *)buf, (int16_t *)buf, len / sizeof(int16_t));
    }
    len = len * (force48k_pcm_bytes / sizeof(int16_t));
#endif

#ifdef __AUDIO_SPECTRUM__
    audio_spectrum_run(buf, len);
#endif


#ifdef __KWS_AUDIO_PROCESS__
    short* pdata = (short*)buf;
    short pdata_mono = 0;
    for(unsigned int i = 0;i<len/4;i++)
    {
        pdata_mono = pdata[2*i]/2+pdata[2*i+1]/2;
        pdata[2*i] = pdata_mono;
        pdata[2*i+1] = pdata_mono;
    }
#endif

#ifdef __AUDIO_OUTPUT_MONO_MODE__
#ifdef AUDIO_OUTPUT_ROUTE_SELECT
if (stream_route_info.output_route_info)
    {
        merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)buf, len/sizeof(int16_t));
    }
    else
#endif
    {
#ifdef A2DP_EQ_24BIT
        merge_stereo_to_mono_24bits((int32_t *)buf, (int32_t *)buf, len/sizeof(int32_t));
#else
        merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)buf, len/sizeof(int16_t));
#endif
    }
#endif

#ifdef __HEAR_THRU_PEAK_DET__
#ifdef ANC_APP
    if(app_anc_work_status())
#endif
    {
        int vol_level = 0;
        vol_level = app_bt_stream_local_volume_get();
        peak_detector_run(buf, len, pkd_vol_multiple[vol_level]);
    }
#endif

#ifndef AUDIO_EQ_TUNING
#ifdef ANC_APP
    bt_audio_updata_eq_for_anc(app_anc_work_status());
#endif
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
    if (!stream_route_info.output_route_info)
#endif
    {
        audio_process_run(buf, len);

#if defined(SPA_AUDIO_ENABLE)
        spatial_audio_app_run_if(buf,len,app_ibrt_if_get_ui_role());
#ifdef SPA_AUDIO_SEC
        tz_audio_process_stream_proc_handler(buf,len);
        ree_audio_test_demo(buf,len);
#endif
#endif
#if defined(IBRT) && !defined(FPGA)
        app_tws_ibrt_audio_analysis_audiohandler_tick(device_id);
#endif

#ifndef BT_BUILD_WITH_CUSTOMER_HOST
        osapi_notify_evm();
#endif
    }
    return len;
}

FRAM_TEXT_LOC void bt_sbc_player_playback_post_handler(uint8_t *buf, uint32_t len, void *cfg)
{
    POSSIBLY_UNUSED struct AF_STREAM_CONFIG_T *config = (struct AF_STREAM_CONFIG_T *)cfg;

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    uint8_t device_id = bes_aud_bt->aud_get_curr_a2dp_device();
#ifdef TWS_PROMPT_SYNC
    tws_playback_ticks_check_for_mix_prompt(device_id);
#endif
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_processing_handler(device_id, len, buf);
    }
#else
    app_ring_merge_more_data(buf, len);
#endif

#ifdef ANC_ASSIST_USE_INT_CODEC
    codec_play_merge_pilot_data(buf, len, cfg);
#endif
}

#ifdef __THIRDPARTY
bool start_by_music = false;
#endif



#ifdef A2DP_DUMP_REF_FB
#define A2DP_DUMP_STREAM_ID       AUD_STREAM_ID_1

#define A2DP_DUMP_SAMPLE_BITS         (16)
#define A2DP_DUMP_SAMPLE_BYTES      (A2DP_DUMP_SAMPLE_BITS / 8)
#define A2DP_DUMP_CHANNEL_NUM       (2)
#define A2DP_DUMP_FRAME_LEN            (256)
#define A2DP_DUMP_BUF_SIZE   (A2DP_DUMP_FRAME_LEN * A2DP_DUMP_CHANNEL_NUM * A2DP_DUMP_SAMPLE_BYTES * 2)//pingpong
static uint8_t A2DP_DUMP_buf[A2DP_DUMP_BUF_SIZE];
//static uint32_t a2dp_capture_sample_rate = 16000;

static uint32_t a2dp_dump_ref_fb_handler(uint8_t *buf, uint32_t len)
{
	int16_t *fb_ptr = (int16_t *)buf;
	int16_t *ref_ptr = (int16_t *)(buf+len);

    audio_dump_clear_up();

    audio_dump_add_channel_data(0, fb_ptr, 256);
    audio_dump_add_channel_data(1, ref_ptr, 256);
	audio_dump_run();
	return 0;
}
#endif

#if defined(AUDIO_ADJ_EQ)
#define ADJ_EQ_STREAM_ID       AUD_STREAM_ID_1

#define ADJ_EQ_SAMPLE_BITS         (24)
#define ADJ_EQ_SAMPLE_BYTES      (4)
#define ADJ_EQ_CHANNEL_NUM       (2)
#define ADJ_EQ_FRAME_LEN            (256)
#define ADJ_EQ_BUF_SIZE   (ADJ_EQ_FRAME_LEN * ADJ_EQ_CHANNEL_NUM * ADJ_EQ_SAMPLE_BYTES * 2)//pingpong
static uint8_t POSSIBLY_UNUSED adj_eq_buf[ADJ_EQ_BUF_SIZE];

#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static int32_t mid_p_8_old_l=0;
static int32_t mid_p_8_old_r=0;
#ifdef AUDIO_ANC_FB_ADJ_MC
#define ADJ_MC_STREAM_ID       AUD_STREAM_ID_1

#define ADJ_MC_SAMPLE_BITS         (16)
#define ADJ_MC_SAMPLE_BYTES      (ADJ_MC_SAMPLE_BITS / 8)
#define ADJ_MC_CHANNEL_NUM       (2)
#define ADJ_MC_FRAME_LEN            (256)
#define ADJ_MC_BUF_SIZE   (ADJ_MC_FRAME_LEN * ADJ_MC_CHANNEL_NUM * ADJ_MC_SAMPLE_BYTES * 2)//pingpong
static uint8_t POSSIBLY_UNUSED adj_mc_buf[ADJ_MC_BUF_SIZE];

static uint32_t audio_adj_mc_data_playback_a2dp(uint8_t *buf, uint32_t mc_len_bytes)
{
   uint32_t begin_time;
   //uint32_t end_time;
   begin_time = hal_sys_timer_get();
   TRACE_AUD_STREAM_I("[A2DP][MUSIC_CANCEL] begin_time: %d",begin_time);

   float left_gain;
   float right_gain;
   int playback_len_bytes,mc_len_bytes_run;
   int i,j,k;
   int delay_sample;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   //TRACE_AUD_STREAM_I("[A2DP][MUSIC_CANCEL]playback_samplerate_ratio:  %d",playback_samplerate_ratio);

  // TRACE_AUD_STREAM_I("[A2DP][MUSIC_CANCEL]left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE_AUD_STREAM_I("[A2DP][MUSIC_CANCEL]right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

   mc_len_bytes_run=mc_len_bytes/SAMPLERATE_RATIO_THRESHOLD;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
			mid_p[j++]=delay_buf_bt[i+1];
        }

        for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
			mid_p[j++]=sour_p[i+1];
        }

        for(j=0;i<playback_len_bytes/2;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
			delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
				mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
					mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_adj_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_run,AUD_BITS_16);
        for(i=0,j=0;i<(mc_len_bytes_run)/2;i=i+2)
        {
            int32_t l_value=mid_p_8[i];
			int32_t r_value=mid_p_8[i+1];

#ifdef ANC_FB_MC_96KHZ
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
#else
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
#endif

        }

    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == (playback_buf_mc))
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
			mid_p[j++]=delay_buf_bt[i+1];
        }

         for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
			mid_p[j++]=sour_p[i+1];
        }

         for(j=0;i<playback_len_bytes/4;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
			delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
				mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
					mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_adj_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_run,AUD_BITS_24);

        for(i=0,j=0;i<(mc_len_bytes_run)/4;i=i+2)
        {
            int32_t l_value=mid_p_8[i];
			int32_t r_value=mid_p_8[i+1];

#ifdef ANC_FB_MC_96KHZ
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
#else
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
			dest_p[j++]=l_value;
			dest_p[j++]=r_value;
#endif

        }
    }

  //  end_time = hal_sys_timer_get();

 //   TRACE_AUD_STREAM_I("[A2DP][MUSIC_CANCEL] run time: %d", end_time-begin_time);

    return 0;
}
#else
static uint32_t audio_mc_data_playback_a2dp(uint8_t *buf, uint32_t mc_len_bytes)
{
//    uint32_t begin_time;
//    uint32_t end_time;
//    begin_time = hal_sys_timer_get();
//    TRACE_AUD_STREAM_I("music cancel: %d",begin_time);

   float left_gain;
   float right_gain;
   int playback_len_bytes,mc_len_bytes_run;
   int i,j,k;
   int delay_sample;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   //TRACE_AUD_STREAM_I("playback_samplerate_ratio:  %d",playback_samplerate_ratio);

  // TRACE_AUD_STREAM_I("left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE_AUD_STREAM_I("right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

   mc_len_bytes_run=mc_len_bytes/SAMPLERATE_RATIO_THRESHOLD;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
            mid_p[j++]=delay_buf_bt[i+1];
        }

        for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

        for(j=0;i<playback_len_bytes/2;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
            delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_run,left_gain,right_gain,AUD_BITS_16);
        for(i=0,j=0;i<(mc_len_bytes_run)/2;i=i+2)
        {
            int32_t l_value=mid_p_8[i];
            int32_t r_value=mid_p_8[i+1];

#ifdef ANC_FB_MC_96KHZ
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
#else
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
#endif
        }
    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_run);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == (playback_buf_mc))
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        delay_sample=DELAY_SAMPLE_MC;

        for(i=0,j=0;i<delay_sample;i=i+2)
        {
            mid_p[j++]=delay_buf_bt[i];
            mid_p[j++]=delay_buf_bt[i+1];
        }

         for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
        {
            mid_p[j++]=sour_p[i];
            mid_p[j++]=sour_p[i+1];
        }

         for(j=0;i<playback_len_bytes/4;i=i+2)
        {
            delay_buf_bt[j++]=sour_p[i];
            delay_buf_bt[j++]=sour_p[i+1];
        }

        if(playback_samplerate_ratio_bt<=SAMPLERATE_RATIO_THRESHOLD)
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(SAMPLERATE_RATIO_THRESHOLD/playback_samplerate_ratio_bt))
            {
                mid_p_8[j++]=mid_p[i];
                mid_p_8[j++]=mid_p[i+1];
            }
        }
        else
        {
            for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
            {
                for(k=0;k<playback_samplerate_ratio_bt/SAMPLERATE_RATIO_THRESHOLD;k++)
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
        }

        anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_run,left_gain,right_gain,AUD_BITS_24);
        for(i=0,j=0;i<(mc_len_bytes_run)/4;i=i+2)
        {
            int32_t l_value=mid_p_8[i];
            int32_t r_value=mid_p_8[i+1];

#ifdef ANC_FB_MC_96KHZ
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
#else
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
            dest_p[j++]=l_value;
            dest_p[j++]=r_value;
#endif

        }
    }
  //  end_time = hal_sys_timer_get();

 //   TRACE_AUD_STREAM_I("%s:run time: %d", __FUNCTION__, end_time-begin_time);

    return 0;
}
#endif
#endif

static uint8_t g_current_eq_index = 0;
static bool isMeridianEQON = false;

bool app_is_meridian_on()
{
    return isMeridianEQON;
}

uint8_t app_audio_get_eq()
{
    return g_current_eq_index;
}

bool app_meridian_eq(bool onoff)
{
    isMeridianEQON = onoff;
    return onoff;
}

int app_audio_set_eq(uint8_t index)
{
#ifdef __SW_IIR_EQ_PROCESS__
    if (index >=EQ_SW_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_FIR_EQ_PROCESS__
    if (index >=EQ_HW_FIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
    if (index >=EQ_HW_DAC_IIR_LIST_NUM)
        return -1;
#endif
#ifdef __HW_IIR_EQ_PROCESS__
    if (index >=EQ_HW_IIR_LIST_NUM)
        return -1;
#endif
    g_current_eq_index = index;
    return index;
}

void bt_audio_updata_eq(uint8_t index)
{
    TRACE_AUD_STREAM_I("[EQ] update idx = %d", index);
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    AUDIO_EQ_TYPE_T audio_eq_type;
#ifdef __SW_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_SW_IIR;
#endif

#ifdef __HW_FIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_FIR;
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_DAC_IIR;
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    audio_eq_type = AUDIO_EQ_TYPE_HW_IIR;
#endif
    bt_audio_set_eq(audio_eq_type,index);
#endif
}

// FIXME: It's a workaround method
void app_le_set_dac_eq(void)
{
#if defined(__SW_IIR_EQ_PROCESS__)
    bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR, app_audio_get_eq());
#endif
#ifdef __HW_DAC_IIR_EQ_PROCESS__
    bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR, app_audio_get_eq());
#endif
}

#ifdef ANC_APP
static uint8_t app_user_eq_index = 0xFF;
uint8_t bt_audio_updata_eq_for_anc(uint8_t anc_status)
{
    anc_status = app_anc_work_status();
    if((app_user_eq_index == 0xFF || app_user_eq_index == 0 || app_user_eq_index == 1) && anc_status_record != anc_status)
    {
        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_104M);

        anc_status_record = anc_status;
        TRACE_AUD_STREAM_I("[EQ] update anc_status = %d",  anc_status);
#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,anc_status));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,anc_status));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,anc_status));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,anc_status));
#endif

        hal_sysfreq_req(HAL_SYSFREQ_USER_ANC, HAL_CMU_FREQ_32K);
    }

    return 0;
}
#endif

#ifdef AUDIO_ADAPTIVE_IIR_EQ
extern "C" uint32_t bt_audio_set_adaptive_dac_iir_eq(IIR_CFG_T *iir_cfg)
{
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        memcpy(&audio_eq_hw_dac_iir_adaptive_eq_cfg, iir_cfg, sizeof(IIR_CFG_T));
        audio_eq_set_cfg(NULL, audio_eq_hw_dac_iir_cfg_list[0], AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif

    return 0;
}
#endif

#ifdef AUDIO_ADAPTIVE_FIR_EQ
extern "C" uint32_t bt_audio_set_adaptive_fir_eq(FIR_CFG_T *fir_cfg)
{
#if defined(__HW_FIR_EQ_PROCESS__)
    audio_eq_set_cfg(fir_cfg, NULL, AUDIO_EQ_TYPE_HW_FIR);
#endif
    return 0;
}
#endif

uint8_t bt_audio_get_eq_index(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t anc_status)
{
    uint8_t index_eq=0;

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if (anc_status) {
                index_eq=audio_eq_sw_iir_index+1;
                if (index_eq >= EQ_SW_IIR_LIST_NUM) {
                    index_eq = 0;
                }
            } else {
                index_eq=audio_eq_sw_iir_index;
            }
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(a2dp_sample_rate == AUD_SAMPRATE_44100) {
                index_eq = 0;
            } else if(a2dp_sample_rate == AUD_SAMPRATE_48000) {
                index_eq = 1;
            } else if(a2dp_sample_rate == AUD_SAMPRATE_96000) {
                index_eq = 2;
            } else {
                ASSERT(0, "[%s] sample_rate_recv(%d) is not supported", __func__, a2dp_sample_rate);
            }
            audio_eq_hw_fir_index=index_eq;

            if(anc_status) {
                index_eq=index_eq+3;
                if (index_eq >= EQ_HW_FIR_LIST_NUM) {
                    index_eq = 0;
                }
            }
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if (anc_status) {
                index_eq=audio_eq_hw_dac_iir_index+1;
                if (index_eq >= EQ_HW_DAC_IIR_LIST_NUM) {
                    index_eq = 0;
                }
            } else {
                index_eq=audio_eq_hw_dac_iir_index;
            }
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(anc_status) {
                index_eq=audio_eq_hw_iir_index+1;
                if (index_eq >= EQ_HW_IIR_LIST_NUM) {
                    index_eq = 0;
                }
            } else {
                index_eq=audio_eq_hw_iir_index;
            }
        }
        break;
#endif
        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif
    return index_eq;
}


uint32_t bt_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type, uint8_t index)
{
    const FIR_CFG_T *fir_cfg=NULL;
    const IIR_CFG_T *iir_cfg=NULL;

    #if defined(__HW_FIR_EQ_PROCESS_2CH__)
        const FIR_CFG_T *fir_cfg_2=NULL;
    #endif

#ifdef ANC_APP
    app_user_eq_index = index;
#endif

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    const IIR_CFG_T *iir_cfg_2=NULL;
#endif

    TRACE_AUD_STREAM_I("[EQ] set type=%d,index=%d",  audio_eq_type,index);

#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(index >= EQ_SW_IIR_LIST_NUM)
            {
                TRACE_AUD_STREAM_W("[EQ] SET index %u > EQ_SW_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_sw_iir_cfg_list[index];
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(index >= EQ_HW_FIR_LIST_NUM)
            {
                TRACE_AUD_STREAM_W("[EQ] SET index %u > EQ_HW_FIR_LIST_NUM", index);
                return 1;
            }

        #if defined(__HW_FIR_EQ_PROCESS_2CH__)
            fir_cfg=audio_hw_hpfir_cfg_list[index];
            fir_cfg_2=audio_hw_lpfir_cfg_list[index];
        #else
            fir_cfg=audio_eq_hw_fir_cfg_list[index];
        #endif
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(index >= EQ_HW_DAC_IIR_LIST_NUM)
            {
                TRACE_AUD_STREAM_W("[EQ] SET index %u > EQ_HW_DAC_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_hw_dac_iir_cfg_list[index];
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
            iir_cfg_2=audio_eq_hw_dac_iir_cfg_list2[index];
#endif
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(index >= EQ_HW_IIR_LIST_NUM)
            {
                TRACE_AUD_STREAM_W("[EQ] SET index %u > EQ_HW_IIR_LIST_NUM", index);
                return 1;
            }

            iir_cfg=audio_eq_hw_iir_cfg_list[index];
        }
        break;
#endif
        default:
        {
            ASSERT(false,"[%s]Error eq type!",__func__);
        }
    }
#endif

#ifdef AUDIO_SECTION_ENABLE
    const IIR_CFG_T *iir_cfg_from_audio_section = (const IIR_CFG_T *)load_audio_cfg_from_audio_section(AUDIO_PROCESS_TYPE_IIR_EQ,index);
    if (iir_cfg_from_audio_section)
    {
        iir_cfg = iir_cfg_from_audio_section;
    }
#endif

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
    return audio_eq_set_cfg(fir_cfg,fir_cfg_2,iir_cfg,audio_eq_type);
#elif defined(__IIR_EQ_PROCESS_LR_2CH__)
    return audio_eq_set_cfg(fir_cfg,iir_cfg,iir_cfg_2,audio_eq_type);
#else
    return audio_eq_set_cfg(fir_cfg,iir_cfg,audio_eq_type);
#endif
}

void app_bt_stream_playback_irq_notification(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream);

uint32_t app_bt_stream_get_dma_buffer_delay_us(void)
{
    uint32_t dma_buffer_delay_us = 0;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;

    if (!af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false)){
        if (stream_cfg->bits <= AUD_BITS_16){
            dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/2*1000000LL/stream_cfg->sample_rate;
        }else{
            dma_buffer_delay_us = stream_cfg->data_size/stream_cfg->channel_num/4*1000000LL/stream_cfg->sample_rate;
        }
    }
    return dma_buffer_delay_us;
}

uint32_t app_bt_stream_get_dma_buffer_samples(void)
{
    uint32_t dma_buffer_delay_samples = 0;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
    if (stream_route_info.output_route_info)
    {
        return stream_route_info.buffer_samples;
    }
    else
#endif
    {
        if (!af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false)){
            if (stream_cfg->bits <= AUD_BITS_16){
                dma_buffer_delay_samples = stream_cfg->data_size/stream_cfg->channel_num/2;
            }else{
                dma_buffer_delay_samples = stream_cfg->data_size/stream_cfg->channel_num/4;
            }
        }
        return dma_buffer_delay_samples;
    }
}

void app_bt_stream_playback_irq_notification(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    if (id != AUD_STREAM_ID_0 || stream != AUD_STREAM_PLAYBACK) {
        return;
    }
    uint8_t a2dp_device = app_bt_audio_get_curr_a2dp_device();
    app_bt_stream_trigger_result(a2dp_device);
#if defined(IBRT)
    app_tws_ibrt_audio_analysis_interrupt_tick();
#endif
}

void app_bt_stream_sco_irq_notification(enum AUD_STREAM_ID_T id, enum AUD_STREAM_T stream)
{
    if ((id == AUD_STREAM_ID_1) && (stream == AUD_STREAM_CAPTURE)) {
        sco_btpcm_mute_flag = 0;
    }
}

extern void a2dp_audio_set_mtu_limit(uint8_t mut);
extern float a2dp_audio_latency_factor_get(void);

int app_bt_stream_detect_next_packet_cb(uint8_t device_id, btif_media_header_t * header, unsigned char *buf, unsigned int len)
{
    if(app_bt_stream_trigger_onprocess()){
        TRACE_AUD_STREAM_I("[AUTO_SYNC] start");
        app_bt_stream_trigger_start(device_id, 0);
    }
    return 0;
}

#ifdef CUSTOM_BITRATE
typedef struct{
    uint8_t a2dp_sbc_delay_mtu;
    uint8_t a2dp_aac_delay_mtu;
}a2dp_customer_codec_delay_mtu;

static a2dp_customer_codec_delay_mtu a2dp_player_playback_delay_mtu =
{
    A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU,
    A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU,
};

void app_audio_dynamic_update_dest_packet_mtu_set(uint8_t codec_index, uint8_t packet_mtu, uint8_t user_configure)
{
    if (codec_index != A2DP_AUDIO_CODEC_TYPE_SBC && codec_index != A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC){
        TRACE(0,"error codec type index %d",codec_index);
        return ;
    }

    TRACE(3,"%s index %d mtu %d",__func__,codec_index,packet_mtu);
    if (user_configure)
    {
        switch(codec_index)
        {
            case A2DP_AUDIO_CODEC_TYPE_SBC:
                a2dp_player_playback_delay_mtu.a2dp_sbc_delay_mtu = packet_mtu;
                break;
            case A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC:
                a2dp_player_playback_delay_mtu.a2dp_aac_delay_mtu = packet_mtu;
                break;
            default:
                break;
        }
    }
    else
    {
        a2dp_player_playback_delay_mtu.a2dp_sbc_delay_mtu = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
        a2dp_player_playback_delay_mtu.a2dp_aac_delay_mtu = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
    }
}

uint8_t app_audio_a2dp_player_playback_delay_mtu_get(uint16_t codec_type)
{
    uint8_t mtu = 0;
    if (codec_type == A2DP_AUDIO_CODEC_TYPE_SBC)
        mtu = a2dp_player_playback_delay_mtu.a2dp_sbc_delay_mtu;
    else if (codec_type == A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC)
        mtu = a2dp_player_playback_delay_mtu.a2dp_aac_delay_mtu;
    else{
        TRACE(0,"error codec tpye [%d]",codec_type);
    }
    return mtu;
}
#endif

static bool a2dp_is_run =  false;

bool bt_a2dp_is_run(void)
{
    return a2dp_is_run;
}

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
static int32_t stereo_surround_ldac = 0;
extern "C" int32_t stereo_surround_status;
extern "C" int32_t audio_process_stereo_surround_onoff(int32_t onoff);
int32_t stereo_surround_is_ldac(void){
    return stereo_surround_ldac;
}
#endif

static enum APP_SYSFREQ_FREQ_T sbc_player_reselect_freq(
                enum APP_SYSFREQ_FREQ_T old_freq, uint8_t codec_type)
{
    enum APP_SYSFREQ_FREQ_T freq = old_freq;
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
    stereo_surround_ldac = 0;
#endif
#if defined(IBRT)
#if defined(IBRT_UI)
    if ((app_ibrt_conn_support_max_mobile_dev() == 2) &&
        (btif_me_get_mobile_link_num() > 1))
    {
        if (freq < APP_SYSFREQ_104M)
        {
            freq = APP_SYSFREQ_104M;
        }
    }
#endif
#endif

#if defined(__SW_IIR_EQ_PROCESS__)&&defined(__HW_FIR_EQ_PROCESS__)&&defined(CHIP_BEST1000)
    if (audio_eq_hw_fir_cfg_list[bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0)]->len>128)
    {
        if (freq < APP_SYSFREQ_104M)
        {
            freq = APP_SYSFREQ_104M;
        }
    }
#endif
#if defined(APP_MUSIC_26M) && !defined(__SW_IIR_EQ_PROCESS__)&& !defined(__HW_IIR_EQ_PROCESS__)&& !defined(__HW_FIR_EQ_PROCESS__)
    if (freq < APP_SYSFREQ_26M) {
        freq = APP_SYSFREQ_26M;
    }
#else
    if (freq < APP_SYSFREQ_52M) {
        freq = APP_SYSFREQ_52M;
    }
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    if (freq < APP_SYSFREQ_104M) {
        freq = APP_SYSFREQ_104M;
    }
#endif

#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) ||defined(A2DP_AAC_ON) || defined(A2DP_SCALABLE_ON) || defined(A2DP_LDAC_ON) || defined(A2DP_L2HC_ON)
    TRACE_AUD_STREAM_I("[A2DP_PLAYER] codec_type:%d", codec_type);
#endif

    POSSIBLY_UNUSED bool isFreqConfiguredFromCodecType = false;

#if defined(A2DP_AAC_ON)
    if (!isFreqConfiguredFromCodecType &&
        codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC) {
        if(freq < APP_SYSFREQ_52M) {
            freq = APP_SYSFREQ_52M;
        }
        isFreqConfiguredFromCodecType = true;
    }
#endif
#if defined(A2DP_SCALABLE_ON)
    if (!isFreqConfiguredFromCodecType &&
        codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
    {
        if(a2dp_sample_rate==44100)
        {
            if(freq < APP_SYSFREQ_78M) {
                freq = APP_SYSFREQ_78M;
            }
        }
        else if(a2dp_sample_rate==96000)
        {
            if (freq < APP_SYSFREQ_208M) {
                freq = APP_SYSFREQ_208M;
            }
        }

        isFreqConfiguredFromCodecType = true;
    }
    TRACE_AUD_STREAM_I("[A2DP_PLAYER] a2dp_sample_rate=%d",a2dp_sample_rate);
#endif


#if defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON) || defined(A2DP_LDAC_ON)
    if (!isFreqConfiguredFromCodecType &&
        codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP){
        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }

        isFreqConfiguredFromCodecType = true;
    }
#endif
#if defined(A2DP_LC3_ON)
    if (!isFreqConfiguredFromCodecType &&
        codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP){
        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }

        isFreqConfiguredFromCodecType = true;
    }
#endif
#if defined(A2DP_L2HC_ON)
    if (!isFreqConfiguredFromCodecType &&
        codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP){
        if (freq < APP_SYSFREQ_208M) {
            freq = APP_SYSFREQ_208M;
        }
        isFreqConfiguredFromCodecType = true;
    }
#endif
#if defined(__AUDIO_DRC__) || defined(__AUDIO_LIMITER__)
    freq = (freq < APP_SYSFREQ_208M) ? APP_SYSFREQ_208M : freq;
#endif

#ifdef A2DP_CP_ACCEL
    // Default freq for SBC
    freq = APP_SYSFREQ_26M;
    POSSIBLY_UNUSED uint8_t nonType = bta_get_curr_a2dp_vender_codec_type();
#if defined(A2DP_LHDC_ON)
    if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        && (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)){
        freq = APP_SYSFREQ_52M;
        if (a2dp_sample_rate==AUD_SAMPRATE_96000)
        {
            freq = APP_SYSFREQ_104M;
        }
    }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
    if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        && (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)){
        freq = APP_SYSFREQ_208M;
        if (a2dp_sample_rate==AUD_SAMPRATE_96000)
        {
            freq = APP_SYSFREQ_208M;
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
            stereo_surround_ldac = 2;
#endif
        }
    }
#endif // A2DP_LHDCV5_ON
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    if (freq < APP_SYSFREQ_52M) {
        freq = APP_SYSFREQ_52M;
    }
#endif
#if defined(A2DP_LDAC_ON)
    if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        && (nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)){
        if (freq < APP_SYSFREQ_104M) {
            freq = APP_SYSFREQ_104M;
        }
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
        stereo_surround_ldac = 1;
#endif
    }
#endif
#endif

#if defined(PLAYBACK_FORCE_48K)
    if (freq < APP_SYSFREQ_104M) {
        freq = APP_SYSFREQ_104M;
    }
#endif

#if defined(AUDIO_EQ_TUNING)
    if (freq < APP_SYSFREQ_104M) {
        freq = APP_SYSFREQ_104M;
    }
#endif

#if defined(IBRT)
#if defined(IBRT_UI)
    if(app_ibrt_conn_support_max_mobile_dev() == 2)
    {
        if((codec_type == BT_A2DP_CODEC_TYPE_SBC) && (a2dp_sample_rate == AUD_SAMPRATE_48000))
        {
            freq = APP_SYSFREQ_52M;
            TRACE(1,"sbc 48k freq %d",freq);
        }
    }
#endif
#endif

#ifdef APP_BT_SPEAKER
    freq = (freq < APP_SYSFREQ_208M) ? APP_SYSFREQ_208M : freq;
#endif
#ifdef SPA_AUDIO_ENABLE
        freq = (freq < APP_SYSFREQ_104M) ? APP_SYSFREQ_104M : freq;
#endif

    if (freq < APP_SYSFREQ_52M)
    {
        if (app_bt_audio_count_straming_mobile_links() > 1)
        {
            freq = APP_SYSFREQ_52M;
        }
    }

#ifdef DIRAC_AUDIO_ENABLE 
    freq = APP_SYSFREQ_208M;
#endif

    return freq;
}

#ifdef CODEC_VCM_CHECK
uint8_t vcm_sta = 0;
#endif
static int bt_sbc_player(enum PLAYER_OPER_T on, enum APP_SYSFREQ_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
#ifdef AUDIO_ANC_FB_ADJ_MC
    struct AF_STREAM_CONFIG_T stream_cfg_adj_mc;
#endif

#ifdef AUDIO_ADJ_EQ
        struct AF_STREAM_CONFIG_T stream_cfg_adj_eq;
#endif

    enum AUD_SAMPRATE_T sample_rate;
    POSSIBLY_UNUSED const char *g_log_player_oper_str[] =
    {
        "PLAYER_OPER_START",
        "PLAYER_OPER_STOP",
        "PLAYER_OPER_RESTART",
    };

    uint8_t* bt_audio_buff = NULL;

    uint8_t POSSIBLY_UNUSED *bt_eq_buff = NULL;
    uint32_t POSSIBLY_UNUSED eq_buff_size = 0;
    uint8_t POSSIBLY_UNUSED play_samp_size;
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t POSSIBLY_UNUSED dma_base;
    POSSIBLY_UNUSED uint8_t *ldac_cc_mem_pool = NULL;
    TRACE_AUD_STREAM_I("[A2DP_PLAYER] work:%d op:%s freq:%d :sample:%d \n", a2dp_is_run, g_log_player_oper_str[on], freq,a2dp_sample_rate);

    if ((a2dp_is_run && on == PLAYER_OPER_START) || (!a2dp_is_run && on == PLAYER_OPER_STOP))
    {
        TRACE_AUD_STREAM_W("[A2DP_PLAYER],fail,isRun=%x,on=%x",a2dp_is_run,on);
        return 0;
    }

    if (on == PLAYER_OPER_START || on == PLAYER_OPER_RESTART)
    {
        af_set_priority(AF_USER_SBC, osPriorityHigh);
    }
    POSSIBLY_UNUSED uint8_t codec_type = bta_get_curr_a2dp_codec_type();
    POSSIBLY_UNUSED uint8_t nonType = bta_get_curr_a2dp_vender_codec_type();

    if (on == PLAYER_OPER_STOP || on == PLAYER_OPER_RESTART)
    {
#ifdef __THIRDPARTY
        start_by_music = false;
#endif

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_enable();
#endif

#if defined(VOICE_DEV)
#if defined(APP_NOISE_ESTIMATION)
        app_noise_estimation_stop();
        app_noise_estimation_close();
#endif

#if defined(VOICE_DEV_TEST)
        test_voice_dev_stop();
        test_voice_dev_close();
#endif

        // Add more ...
        uint32_t algos = VOICE_DEV_ALGO_NONE;
        voice_dev_ctl(VOICE_DEV_USER0, VOICE_DEV_SET_SUPPORT_ALGOS, &algos);
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        bool isToClearActiveMedia = audio_prompt_clear_pending_stream(PENDING_TO_STOP_A2DP_STREAMING);
        if (isToClearActiveMedia)
        {
            // clear active media mark
            bt_media_clear_media_type(BT_STREAM_MUSIC, BT_DEVICE_ID_1);
            bt_media_current_music_set(BT_DEVICE_INVALID_ID);
        }
#endif
#if (A2DP_DECODER_VER == 2)
        a2dp_audio_preparse_stop();
#endif

        if (bt_is_playback_triggered())
        {
#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
#if defined(ANC_ASSIST_USE_INT_CODEC)
            af_stream_playback_fadeout(AUD_STREAM_ID_0, 30);
#else
            af_stream_playback_fadeout(AUD_STREAM_ID_0, 10);
#endif
#endif
        }
#if defined(IBRT)
        app_bt_stream_ibrt_auto_synchronize_stop();
        app_tws_ibrt_audio_analysis_stop();
        app_tws_ibrt_audio_sync_stop();
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
    if (stream_route_info.output_route_info)
    {
        app_bt_output_route_deinit();
    }
#endif

#if (A2DP_DECODER_VER == 2)
        a2dp_audio_stop();
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            af_codec_set_playback_post_handler(NULL);
            af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        }

#ifdef A2DP_DUMP_REF_FB
        af_stream_stop(A2DP_DUMP_STREAM_ID, AUD_STREAM_CAPTURE);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#ifdef AUDIO_ANC_FB_ADJ_MC
        af_stream_stop(ADJ_MC_STREAM_ID, AUD_STREAM_CAPTURE);
#endif
        af_stream_stop(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(AUDIO_ADJ_EQ)
        af_stream_stop(ADJ_EQ_STREAM_ID, AUD_STREAM_CAPTURE);
#endif

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
        calib_reset = 1;
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#endif
#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
            }
        }

#if defined(__AUDIO_SPECTRUM__)
        audio_spectrum_close();
#endif

#if defined(A2DP_KARAOKE)
        app_karaoke_set_buf(NULL, 0);
#endif

        audio_process_close();

        TRACE_AUD_STREAM_I("[A2DP_PLAYER] syspool free size: %d/%d", syspool_free_size(), syspool_total_size());

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        }

#ifdef A2DP_DUMP_REF_FB
        af_stream_close(A2DP_DUMP_STREAM_ID, AUD_STREAM_CAPTURE);
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_32K);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
#ifdef AUDIO_ANC_FB_ADJ_MC
        af_stream_close(ADJ_MC_STREAM_ID, AUD_STREAM_CAPTURE);
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_32K);
#endif

        af_stream_close(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#ifdef AUDIO_ADJ_EQ
        af_stream_close(ADJ_EQ_STREAM_ID, AUD_STREAM_CAPTURE);
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_32K);
#endif

        bt_a2dp_mode = 0;

#ifdef AI_AEC_CP_ACCEL
        cp_aec_deinit();
#endif

#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#endif

#if defined(CODEC_DAC_MULTI_VOLUME_TABLE)
        hal_codec_set_dac_volume_table(NULL, 0);
#endif

        af_set_irq_notification(NULL);
        if (on == PLAYER_OPER_STOP)
        {
            app_bt_stream_trigger_checker_stop();
#ifdef __A2DP_PLAYER_USE_BT_TRIGGER__
            app_bt_stream_trigger_deinit();
#endif
#ifndef FPGA
#ifdef BT_XTAL_SYNC
            bt_term_xtal_sync(false);
            bt_term_xtal_sync_default();
#endif
#endif
            a2dp_audio_deinit();
#if defined(SPA_AUDIO_ENABLE)
        spatial_audio_app_deinit_if();
#ifdef SPA_AUDIO_SEC
        tz_audio_process_info_deinit();
#endif
#endif

#ifdef A2DP_DUMP_REF_FB
            audio_dump_deinit();
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)&&defined(AUDIO_ANC_FB_ADJ_MC)
            adj_mc_deinit();
#endif

#if defined(AUDIO_ADJ_EQ)
            adj_eq_deinit();
#endif
            app_overlay_unloadall();
#ifdef __THIRDPARTY
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_STOP2MIC, 0);
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START, 0);
#endif
            app_sysfreq_req(APP_SYSFREQ_USER_BT_A2DP, APP_SYSFREQ_32K);
            af_set_priority(AF_USER_SBC, osPriorityAboveNormal);

#if defined(__IAG_BLE_INCLUDE__)
            bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_A2DP,
                                           USER_ALL,
                                           BLE_ADV_INVALID_INTERVAL);
#endif

#ifdef AUDIO_RMS_MONITOR_ENABLE
            rms_debug_cnt = 0xFFFF;
#endif
        }
        bt_set_playback_triggered(false);
    }

    if (on == PLAYER_OPER_START || on == PLAYER_OPER_RESTART)
    {
#ifdef CODEC_VCM_CHECK
        vcm_sta = 0;
#endif
        bt_set_playback_triggered(false);
#if defined(__IAG_BLE_INCLUDE__)
        bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_A2DP,
                                       USER_ALL,
                                       BLE_ADVERTISING_INTERVAL);
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        audio_prompt_stop_playing();
#endif

#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_STOP, 0);
#endif

        bt_media_volume_ptr_update_by_mediatype(BT_STREAM_MUSIC);
        stream_local_volume = btdevice_volume_p->a2dp_vol;

        freq = sbc_player_reselect_freq(freq, codec_type);
        app_audio_set_a2dp_freq(freq);

        TRACE_AUD_STREAM_I("[A2DP_PLAYER] sysfreq %d", freq);

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
        TRACE_AUD_STREAM_I("[A2DP_PLAYER] sysfreq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif

        if (on == PLAYER_OPER_START)
        {
            af_set_irq_notification(app_bt_stream_playback_irq_notification);
            ASSERT(!app_ring_merge_isrun(), "Ring playback will be abnormal, please check.");
            if (codec_type == BT_A2DP_CODEC_TYPE_SBC)
            {
                app_overlay_select(APP_OVERLAY_A2DP);
            }
#if defined(A2DP_AAC_ON)
            else if (codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
            {
                app_overlay_select(APP_OVERLAY_A2DP_AAC);
            }
#endif
            else if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            {
                TRACE(0, "[APP_BT_STREAM] %d, nonType %d", __LINE__, nonType);
                if (nonType == BT_A2DP_CODEC_NONE_TYPE_INVALID)
                {
                    TRACE(0, "WARNING nonType Invalid");
                }
#if defined(A2DP_LHDC_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)
                {
                    app_overlay_select(APP_OVERLAY_A2DP_LHDC);
                }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                {
                    app_overlay_select(APP_OVERLAY_A2DP_LHDC_V5);
                }
#endif // A2DP_LHDCV5_ON
#if defined(A2DP_SCALABLE_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
                {
                    app_overlay_select(APP_OVERLAY_A2DP_SCALABLE);
                }
#endif // A2DP_SCALABLE_ON
#if defined(A2DP_LDAC_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)
                {
                    app_overlay_select(APP_OVERLAY_A2DP_LDAC);
                }
#endif // A2DP_LDAC_ON
#if defined(A2DP_LC3_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LC3)
                {
                    TRACE(0,"bt_sbc_player lc3 overlay select \n");
                    app_overlay_select(APP_OVERLAY_A2DP_LC3);
                }
#endif // A2DP_LC3_ON
#if defined(A2DP_L2HC_ON)
                else if (nonType == BT_A2DP_CODEC_NONE_TYPE_L2HC)
                {
                    TRACE(0,"bt_sbc_player l2hc overlay select \n");
                    app_overlay_select(APP_OVERLAY_A2DP_L2HC);
                }
#endif // A2DP_L2HC_ON
                else
                {
                    TRACE(0, "WARNING nonType Invalid");
                }
            }
            else
            {
                app_overlay_select(APP_OVERLAY_A2DP);
            }

#ifdef __THIRDPARTY
            app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START2MIC, 0);
#endif

            bt_a2dp_mode = 1;
        }

        app_audio_mempool_init_with_specific_size(app_audio_mempool_size());

#ifdef __THIRDPARTY
        start_by_music = true;
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START, 0);
#endif
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
        sample_rate = AUD_SAMPRATE_50781;
#else
        sample_rate = a2dp_sample_rate;
#endif

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_disable();
#endif

#if defined(CODEC_DAC_MULTI_VOLUME_TABLE)
        hal_codec_set_dac_volume_table(codec_dac_a2dp_vol, ARRAY_SIZE(codec_dac_a2dp_vol));
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.channel_num = AUD_CHANNEL_NUM_2;
#if defined(A2DP_LHDCV5_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            &&(nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)){
                stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        }
#endif
#ifdef PLAYBACK_FORCE_48K
        stream_cfg.sample_rate = AUD_SAMPRATE_48000;
#else
        stream_cfg.sample_rate = sample_rate;
#endif

#ifdef FPGA
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#else
#ifdef PLAYBACK_USE_I2S
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
#else
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#endif
#endif
#ifdef PLAYBACK_USE_I2S
        stream_cfg.io_path = AUD_IO_PATH_NULL;
#else
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
#endif

        stream_cfg.vol = stream_local_volume;
        stream_cfg.handler = bt_sbc_player_more_data;

#if defined(A2DP_SCALABLE_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            && (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE))
        {
            stream_cfg.data_size = SCALABLE_FRAME_SIZE*8;
        }else
#endif
#if defined(A2DP_LHDC_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            && (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)){
            if (a2dp_lhdc_config_llc_get()){
                if (a2dp_lhdc_get_ext_flags(LHDC_EXT_FLAGS_LLAC)) // LLAC
                {
                    if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_44100) {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LLAC USE 44.1K Buffer");
                        stream_cfg.data_size = LLAC_AUDIO_44P1K_BUFF_SIZE>>1;
                    } else {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LLAC USE 48K Buffer");
                        stream_cfg.data_size = LLAC_AUDIO_BUFF_SIZE>>1;
                    }
                }else{ // LHDC
                    if (bt_get_sbc_sample_rate() != AUD_SAMPRATE_96000) {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LHDC USE 48K Buffer");
                        stream_cfg.data_size = LHDC_AUDIO_BUFF_SIZE>>1;
                    } else {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LHDC USE 96K Buffer");
                        stream_cfg.data_size = LHDC_AUDIO_96K_BUFF_SIZE>>1;
                    }
                }              
            }
            else
            {
                if (a2dp_lhdc_get_ext_flags(LHDC_EXT_FLAGS_LLAC)) // LLAC
                {
                    if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_44100) {
                        stream_cfg.data_size = LLAC_AUDIO_44P1K_BUFF_SIZE;
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LLAC USE 44.1K LL Buffer");
                    } else {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LLAC USE 48K LL Buffer");
                        stream_cfg.data_size = LLAC_AUDIO_BUFF_SIZE;
                    }
                }else{ // LHDC
                    if (bt_get_sbc_sample_rate() != AUD_SAMPRATE_96000) {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LHDC USE 48K LL Buffer");
                        stream_cfg.data_size = (LHDC_AUDIO_BUFF_SIZE);
                    } else {
                        TRACE_AUD_STREAM_I("[A2DP_PLAYER] LHDC USE 96K LL Buffer");
                        stream_cfg.data_size = LHDC_AUDIO_96K_BUFF_SIZE;
                    }
                }
            }
        }else
#endif //#if defined(A2DP_LHDC_ON)
#if defined(A2DP_LHDCV5_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            &&(nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)){
            if (a2dp_lhdc_config_llc_get()){
                // LL
                if ((bt_get_sbc_sample_rate() == AUD_SAMPRATE_44100) ||
                    (bt_get_sbc_sample_rate() == AUD_SAMPRATE_48000)) {
                    // 44.1KHz or 48KHz
                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5-LL 44.1KHz or 48KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_48K_LLC_BUFF_SIZE;
                } else if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_96000){
                    // 96KHz

                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5-LL 96KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_96K_LLC_BUFF_SIZE;
                } else {
                    // 192KHz

                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5-LL 192KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_192K_LLC_BUFF_SIZE;
                }
            } else {
                // Normal
                if ((bt_get_sbc_sample_rate() == AUD_SAMPRATE_44100) ||
                    (bt_get_sbc_sample_rate() == AUD_SAMPRATE_48000)) {
                    // 44.1KHz or 48KHz
                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5 44.1KHz or 48KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_48K_BUFF_SIZE;
                } else if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_96000){
                    // 96KHz
                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5 96KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_96K_BUFF_SIZE;
                } else {
                    // 192KHz
                    TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE LHDCV5 192KHz Buf");

                    stream_cfg.data_size = LHDCV5_AUDIO_192K_BUFF_SIZE;
                }
            }
        }else
#endif //#if defined(A2DP_LHDCV5_ON)
#if defined(A2DP_LDAC_ON)
        if((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            &&(nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)){
            if (sample_rate > AUD_SAMPRATE_96000) {
                stream_cfg.data_size = BT_AUDIO_BUFF_SIZE_192K_LDAC;
            } else if (sample_rate > AUD_SAMPRATE_48000) {
                stream_cfg.data_size = BT_AUDIO_BUFF_SIZE_96K_LDAC;
            } else {
                stream_cfg.data_size = BT_AUDIO_BUFF_SIZE_48K_LDAC;
            }
            syspool_get_buff(&ldac_cc_mem_pool, LDAC_CC_MEM_POOL_SIZE);
            bes_cc_heap_init(ldac_cc_mem_pool, LDAC_CC_MEM_POOL_SIZE);
        }else
#endif
#if defined(A2DP_AAC_ON)
        if (codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC)
            stream_cfg.data_size = BT_AUDIO_BUFF_AAC_SIZE;
        else
#endif
#if defined(A2DP_LC3_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            && (nonType == BT_A2DP_CODEC_NONE_TYPE_LC3))
        {
            if (bt_get_sbc_sample_rate() == AUD_SAMPRATE_96000) {
                stream_cfg.data_size = LC3_AUDIO_96K_BUFF_SIZE;
            } else {
                stream_cfg.data_size = LC3_AUDIO_BUFF_SIZE;
            }
        }else
#endif
#if defined(A2DP_L2HC_ON)
        if ((codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
            && (nonType == BT_A2DP_CODEC_NONE_TYPE_L2HC))
        {
            uint8_t duration = bes_bt_a2dp_get_curr_duration();
            TRACE(0, "L2HC duration %d", duration/10);
            if ((bt_get_sbc_sample_rate() == AUD_SAMPRATE_44100) ||
                (bt_get_sbc_sample_rate() == AUD_SAMPRATE_48000)) {
                // 44.1KHz or 48KHz
                TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE L2HC 44.1KHz or 48KHz Buf");
                if(duration == 50)
                    stream_cfg.data_size = L2HC_AUDIO_BUFF_SIZE_1; //duration 5
                else
                    stream_cfg.data_size = L2HC_AUDIO_BUFF_SIZE_2; //duration 10
            } else if ((bt_get_sbc_sample_rate() == AUD_SAMPRATE_88200) ||
                       (bt_get_sbc_sample_rate() == AUD_SAMPRATE_96000)) {
                // 88.2KHz or 96KHz
                TRACE_AUD_STREAM_I("[A2DP_PLAYER] USE L2HC 88.2KHz or 96KHz Buf");
                if(duration == 50)
                    stream_cfg.data_size = L2HC_AUDIO_BUFF_SIZE_2; //duration 5
                else
                    stream_cfg.data_size = L2HC_AUDIO_BUFF_SIZE_3; //duration 10
            }
        } else
#endif
        {
            if (stream_cfg.sample_rate == AUD_SAMPRATE_44100){
                stream_cfg.data_size = BT_AUDIO_BUFF_SBC_44P1K_SIZE;
            }else{
                stream_cfg.data_size = BT_AUDIO_BUFF_SBC_48K_SIZE;
            }
        }

        stream_cfg.bits = AUD_BITS_16;

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
#ifdef A2DP_EQ_24BIT
            stream_cfg.data_size *= 2;
            stream_cfg.bits = AUD_BITS_24;
#elif defined(A2DP_SCALABLE_ON) || defined(A2DP_LHDC_ON) || defined(A2DP_LHDCV5_ON)
            if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP) {
                stream_cfg.data_size *= 2;
                stream_cfg.bits = AUD_BITS_24;
            }
#endif
        }

#if 0//defined(A2DP_LHDC_ON)
        if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP)
        {
            if(bt_sbc_player_get_sample_bit() == AUD_BITS_16)
            {
                stream_cfg.bits = AUD_BITS_16;
            }
        }
#endif

        a2dp_data_buf_size = stream_cfg.data_size;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
        lowdelay_sample_size_play_bt=stream_cfg.bits;
        lowdelay_sample_rate_play_bt=stream_cfg.sample_rate;
        lowdelay_data_size_play_bt=stream_cfg.data_size;
        lowdelay_playback_ch_num_bt=stream_cfg.channel_num;
#endif


#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        uint8_t* promptTmpSourcePcmDataBuf;
        uint8_t* promptTmpTargetPcmDataBuf;
        uint8_t* promptPcmDataBuf;
        uint8_t* promptResamplerBuf;

        app_audio_mempool_get_buff(&promptTmpSourcePcmDataBuf, AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptTmpTargetPcmDataBuf, AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptResamplerBuf, AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);

        audio_prompt_buffer_config(MIX_WITH_A2DP_STREAMING,
                                   stream_cfg.channel_num,
                                   stream_cfg.bits,
                                   promptTmpSourcePcmDataBuf,
                                   promptTmpTargetPcmDataBuf,
                                   promptPcmDataBuf,
                                   AUDIO_PROMPT_PCM_BUFFER_SIZE,
                                   promptResamplerBuf,
                                   AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt = stream_cfg.bits;
        sample_rate_play_bt = stream_cfg.sample_rate;
        data_size_play_bt = stream_cfg.data_size;
        playback_buf_bt = stream_cfg.data_ptr;
        playback_size_bt = stream_cfg.data_size;
        if (sample_rate_play_bt == AUD_SAMPRATE_96000)
        {
            playback_samplerate_ratio_bt = 4;
        }
        else
        {
            playback_samplerate_ratio_bt = 8;
        }
        playback_ch_num_bt=stream_cfg.channel_num;
        mid_p_8_old_l=0;
        mid_p_8_old_r=0;
#endif

#if defined(AUDIO_ADJ_EQ)
        static enum AUD_SAMPRATE_T sample_rate_play_bt;
        sample_rate_play_bt = stream_cfg.sample_rate;
#endif

#if defined(PLAYBACK_FORCE_48K)
        if (stream_cfg.bits == AUD_BITS_16) {
            force48k_pcm_bytes = sizeof(int16_t);
        } else {
            force48k_pcm_bytes = sizeof(int32_t);
        }

        if (sample_rate == AUD_SAMPRATE_48000) {
            force48k_resample_needed = false;
        } else {
            force48k_resample_needed = true;
            force48k_resample= app_force48k_resample_any_open(bt_get_sbc_sample_rate(), stream_cfg.channel_num,
                                app_force48k_resample_iter, stream_cfg.data_size / stream_cfg.channel_num / (force48k_pcm_bytes / sizeof(int16_t)),
                                (float)sample_rate / AUD_SAMPRATE_48000);
            TRACE(3, "[%s] Resample: %d --> %d", __func__, sample_rate, AUD_SAMPRATE_48000);
        }
#endif

#if defined(ANC_ASSIST_ENABLED)
        app_anc_assist_set_playback_info(stream_cfg.sample_rate);
#if 1
        app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC);
#else
        if (codec_type == BT_A2DP_CODEC_TYPE_MPEG2_4_AAC) {
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_AAC);
        } else if (codec_type == BT_A2DP_CODEC_TYPE_SBC) {
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_SBC);
        } else if (codec_type == BT_A2DP_CODEC_TYPE_NON_A2DP) {
            // TODO: Be careful audioflinger callback frame length.
            app_anc_assist_set_mode(ANC_ASSIST_MODE_MUSIC_AAC);
        } else {
            ASSERT(0, "[%s] codec_type(%d) is invalid!!!", __func__, codec_type);
        }
#endif
#if defined(AUDIO_ADAPTIVE_EQ)
        app_voice_assist_adaptive_eq_open();
#endif
#endif

        TRACE(4, "A2DP Playback: sample rate: %d, bits = %d, channel number = %d, data size:%d",
                                            stream_cfg.sample_rate,
                                            stream_cfg.bits,
                                            stream_cfg.channel_num,
                                            stream_cfg.data_size);

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (stream_route_info.output_route_info)
        {
            stream_route_info.codec_type = codec_type;
            app_bt_output_route_init(stream_cfg.sample_rate, stream_cfg.bits,
                                     stream_cfg.channel_num, stream_cfg.data_size/2, stream_cfg.vol);
            if (!stream_route_info.output_route_info->data_transfer_mode)
            {
                 app_audio_mempool_get_buff(&stream_route_info.send_buf, stream_route_info.buffer_samples);
            }
        }
        else
#endif
        {
            af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);
        }

#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_PLAYBACK, A2DP_SCO_TRIGGER_CHANNEL);
#endif

#if defined(A2DP_STREAM_AUDIO_DUMP)
        if (stream_cfg.bits == 16) {
        } else {
            g_a2dp_pcm_dump_sample_bytes = sizeof(int32_t);
        }
        g_a2dp_pcm_dump_channel_num = stream_cfg.channel_num;
        g_a2dp_pcm_dump_frame_len = stream_cfg.data_size / 2 / g_a2dp_pcm_dump_sample_bytes / g_a2dp_pcm_dump_channel_num;
#if A2DP_STREAM_AUDIO_DUMP == 16
        ASSERT(g_a2dp_pcm_dump_frame_len < A2DP_PCM_DUMP_BUFFER_MAX_SIZE,
                "DUMP ERROR: frame_len(%d) > MAX LEN(%d) ", g_a2dp_pcm_dump_frame_len, A2DP_PCM_DUMP_BUFFER_MAX_SIZE);
        audio_dump_init(g_a2dp_pcm_dump_frame_len, sizeof(int16_t), 1);
#else
        audio_dump_init(g_a2dp_pcm_dump_frame_len, g_a2dp_pcm_dump_sample_bytes, 1);
#endif
#endif

#ifdef A2DP_DUMP_REF_FB
		struct AF_STREAM_CONFIG_T stream_a2dp_dump;
		audio_dump_init(A2DP_DUMP_FRAME_LEN, sizeof(short), 2);
		app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_104M);
        TRACE(0, "[A2DP_DUMP] sysfreq %d", freq);
        // TRACE(0, "[A2DP_DUMP] sysfreq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
		memset(&stream_a2dp_dump, 0, sizeof(stream_a2dp_dump));
        stream_a2dp_dump.channel_num = (enum AUD_CHANNEL_NUM_T)A2DP_DUMP_CHANNEL_NUM;
        stream_a2dp_dump.channel_map = (enum AUD_CHANNEL_MAP_T)(ANC_FB_MIC_CH_L | AUD_CHANNEL_MAP_ECMIC_CH0);
        stream_a2dp_dump.data_size = A2DP_DUMP_BUF_SIZE;
        stream_a2dp_dump.sample_rate = AUD_SAMPRATE_16000;
        stream_a2dp_dump.bits = AUD_BITS_16;
        stream_a2dp_dump.vol = 12;
        stream_a2dp_dump.chan_sep_buf = true;
        stream_a2dp_dump.device = AUD_STREAM_USE_INT_CODEC;
        stream_a2dp_dump.io_path = AUD_INPUT_PATH_ANC_ASSIST;
        stream_a2dp_dump.handler = a2dp_dump_ref_fb_handler;
        stream_a2dp_dump.data_ptr = A2DP_DUMP_buf;

        af_stream_open(A2DP_DUMP_STREAM_ID, AUD_STREAM_CAPTURE, &stream_a2dp_dump);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_a2dp_dump.device, AUD_STREAM_CAPTURE, A2DP_SCO_TRIGGER_CHANNEL);
#endif
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
    #ifdef AUDIO_ANC_FB_ADJ_MC
        stream_cfg.handler = audio_adj_mc_data_playback_a2dp;
    #else
        stream_cfg.handler = audio_mc_data_playback_a2dp;
	#endif
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_PLAYBACK, A2DP_SCO_TRIGGER_CHANNEL);
#endif
        // ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
#ifdef AUDIO_ANC_FB_ADJ_MC
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_104M);
        TRACE(0, "[ADJ_MC] sysfreq %d", freq);
        // TRACE(0, "[ADJ_MC] sysfreq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

        adj_mc_capture_sample_rate = sample_rate_play_bt / 3;
        adj_mc_init(ADJ_MC_FRAME_LEN);
        memset(&stream_cfg_adj_mc, 0, sizeof(stream_cfg_adj_mc));
        stream_cfg_adj_mc.channel_num = (enum AUD_CHANNEL_NUM_T)ADJ_MC_CHANNEL_NUM;
        stream_cfg_adj_mc.channel_map = (enum AUD_CHANNEL_MAP_T)(ANC_FB_MIC_CH_L | AUD_CHANNEL_MAP_ECMIC_CH0);
        stream_cfg_adj_mc.data_size = ADJ_MC_BUF_SIZE;
        stream_cfg_adj_mc.sample_rate = (enum AUD_SAMPRATE_T)adj_mc_capture_sample_rate;
        stream_cfg_adj_mc.bits = AUD_BITS_16;
        stream_cfg_adj_mc.vol = 12;
        stream_cfg_adj_mc.chan_sep_buf = true;
        stream_cfg_adj_mc.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg_adj_mc.io_path = AUD_INPUT_PATH_ANC_ASSIST;
        stream_cfg_adj_mc.handler = adj_mc_filter_estimate;
        stream_cfg_adj_mc.data_ptr = adj_mc_buf;
        TRACE(2, "[A2DP_PLAYER] capture sample_rate:%d, data_size:%d", stream_cfg_adj_mc.sample_rate, stream_cfg_adj_mc.data_size);
        af_stream_open(ADJ_MC_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg_adj_mc);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg_adj_mc.device, AUD_STREAM_CAPTURE, A2DP_SCO_TRIGGER_CHANNEL);
#endif
#endif

#endif

#ifdef AUDIO_ADJ_EQ
        app_sysfreq_req(APP_SYSFREQ_USER_VOICE_ASSIST, APP_SYSFREQ_104M);
        TRACE(0, "[ADJ_EQ] sysfreq %d", freq);
        // TRACE(0, "[ADJ_MC] sysfreq calc : %d\n", hal_sys_timer_calc_cpu_freq(5, 0));

        adj_eq_capture_sample_rate = sample_rate_play_bt / 3;
        adj_eq_init(ADJ_EQ_FRAME_LEN);
        memset(&stream_cfg_adj_eq, 0, sizeof(stream_cfg_adj_eq));
        stream_cfg_adj_eq.channel_num = (enum AUD_CHANNEL_NUM_T)ADJ_EQ_CHANNEL_NUM;
        stream_cfg_adj_eq.channel_map = (enum AUD_CHANNEL_MAP_T)(ANC_FB_MIC_CH_L | AUD_CHANNEL_MAP_ECMIC_CH0);
        stream_cfg_adj_eq.data_size = ADJ_EQ_BUF_SIZE;
        stream_cfg_adj_eq.sample_rate = (enum AUD_SAMPRATE_T)adj_eq_capture_sample_rate;
        stream_cfg_adj_eq.bits = AUD_BITS_24;
        stream_cfg_adj_eq.vol = 12;
        stream_cfg_adj_eq.chan_sep_buf = true;
        stream_cfg_adj_eq.device = AUD_STREAM_USE_INT_CODEC;
        stream_cfg_adj_eq.io_path = AUD_INPUT_PATH_ANC_ASSIST;
        stream_cfg_adj_eq.handler = adj_eq_filter_estimate;
        stream_cfg_adj_eq.data_ptr = adj_eq_buf;
        TRACE(2, "[ADJ_EQ] capture sample_rate:%d, data_size:%d", stream_cfg_adj_eq.sample_rate, stream_cfg_adj_eq.data_size);
        af_stream_open(ADJ_EQ_STREAM_ID, AUD_STREAM_CAPTURE, &stream_cfg_adj_eq);
#endif


#ifdef __HEAR_THRU_PEAK_DET__
        PEAK_DETECTOR_CFG_T peak_detector_cfg;
        peak_detector_cfg.fs = stream_cfg.sample_rate;
        peak_detector_cfg.bits = stream_cfg.bits;
        peak_detector_cfg.factor_up = 0.6;
        peak_detector_cfg.factor_down = 2.0;
        peak_detector_cfg.reduce_dB = -30;
        peak_detector_init();
        peak_detector_setup(&peak_detector_cfg);
#endif

#if defined(__AUDIO_SPECTRUM__)
        audio_spectrum_open(stream_cfg.sample_rate, stream_cfg.bits);
#endif

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = a2dp_data_buf_size*2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)

        play_samp_size = (stream_cfg.bits <= AUD_BITS_16) ? 2 : 4;
#if defined(CHIP_BEST2000)
        eq_buff_size = a2dp_data_buf_size * sizeof(int32_t) / play_samp_size;
#elif defined(CHIP_BEST1000)
        eq_buff_size = a2dp_data_buf_size * sizeof(int16_t) / play_samp_size;
#elif defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A) || defined(CHIP_BEST1501) || defined(CHIP_BEST2003)
        eq_buff_size = a2dp_data_buf_size;
#endif
#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = a2dp_data_buf_size;
#else
        eq_buff_size = 0;
        bt_eq_buff = NULL;
#endif

        if(eq_buff_size > 0)
        {
            app_audio_mempool_get_buff(&bt_eq_buff, eq_buff_size);
        }

#if defined(IBRT)&& !defined(FREEMAN_ENABLED_STERO)
#ifdef SPA_AUDIO_ENABLE
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_2;
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_1;
#endif
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = stream_cfg.channel_num;
#endif

        audio_process_open(stream_cfg.sample_rate, stream_cfg.bits, sw_ch_num, stream_cfg.channel_num, stream_cfg.data_size/stream_cfg.channel_num/(stream_cfg.bits <= AUD_BITS_16 ? 2 : 4)/2, bt_eq_buff, eq_buff_size);
#if defined(SPA_AUDIO_ENABLE)
        spatial_audio_app_init_if(stream_cfg.sample_rate,stream_cfg.bits,stream_cfg.channel_num,stream_cfg.data_size/sw_ch_num/(stream_cfg.bits <= AUD_BITS_16 ? 2 : 4)/2,(void*)app_audio_mempool_get_buff, app_ibrt_if_get_ui_role(), ((ibrt_ctrl_t *)app_tws_ibrt_get_bt_ctrl_ctx())->audio_chnl_sel);
#ifdef SPA_AUDIO_SEC
        {
            tz_audio_process_info_t info;
            info.fs = stream_cfg.sample_rate;
            info.bits = stream_cfg.bits;
            info.channels = stream_cfg.channel_num;
            if (stream_cfg.bits <= AUD_BITS_16){
                info.frame_len = stream_cfg.data_size/stream_cfg.channel_num/2;
            }else{
                info.frame_len = stream_cfg.data_size/stream_cfg.channel_num/4;
            }
            tz_audio_process_info_init((void *)&info);
        }
#endif
#endif

#if defined(A2DP_KARAOKE)
        uint32_t karaoke_buf_size = app_karaoke_need_buf_size();
        if (karaoke_buf_size != 0) {
            app_karaoke_set_buf((uint8_t *)app_audio_mempool_calloc(karaoke_buf_size, sizeof(int8_t)), karaoke_buf_size);
        }
#endif
// disable audio eq config on a2dp start for audio tuning tools
#ifndef AUDIO_EQ_TUNING
#ifdef ANC_APP
        anc_status_record = 0xff;
        bt_audio_updata_eq_for_anc(app_anc_work_status());
#else   // #ifdef ANC_APP
#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,0));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,0));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,0));
#endif
#endif  // #ifdef ANC_APP
#endif  // #ifndef AUDIO_EQ_TUNING

#if defined(IBRT)
        APP_TWS_IBRT_AUDIO_SYNC_CFG_T sync_config;
        sync_config.factor_reference  = TWS_IBRT_AUDIO_SYNC_FACTOR_REFERENCE;
        sync_config.factor_fast_limit = TWS_IBRT_AUDIO_SYNC_FACTOR_FAST_LIMIT;
        sync_config.factor_slow_limit = TWS_IBRT_AUDIO_SYNC_FACTOR_SLOW_LIMIT;;
        sync_config.dead_zone_us      = TWS_IBRT_AUDIO_SYNC_DEAD_ZONE_US;
        app_tws_ibrt_audio_sync_reconfig(&sync_config);
#else
#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_PLAYBACK, 0);
#endif
#endif
        if (on == PLAYER_OPER_START)
        {
            // This might use all of the rest buffer in the mempool,
            // so it must be the last configuration before starting stream.
#if (A2DP_DECODER_VER == 2)
            A2DP_AUDIO_OUTPUT_CONFIG_T output_config;
            A2DP_AUDIO_CODEC_TYPE a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
            A2DP_AUDIO_CHANNEL_SELECT_E a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
            float chopping_dest_mut = 0;
            output_config.sample_rate = sample_rate;
            output_config.num_channels = 2;
            output_config.bits_depth = stream_cfg.bits;
            output_config.frame_samples = app_bt_stream_get_dma_buffer_samples()/2;
            output_config.factor_reference = 1.0f;
#if defined(IBRT)
            uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
            POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
            ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
            float dest_packet_mut = 0;
            int need_autotrigger = a2dp_bt_stream_need_autotrigger_getandclean_flag();
            uint32_t offset_mut = 0;
            uint8_t codec_type = BT_A2DP_CODEC_TYPE_SBC;

            if (!a2dp_audio_chopping_is_in_post())
            {
                a2dp_audio_latency_factor_setlow();
            }

            codec_type = bta_get_a2dp_codec_type_by_id(device_id);
            switch (codec_type)
            {
                case BT_A2DP_CODEC_TYPE_SBC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
                    offset_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_FRAME_MTU * A2DP_PLAYER_PLAYBACK_WATER_LINE;
#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
                    a2dp_audio_max_waterline_init(A2DP_PLAYER_PLAYBACK_MAX_WATERLINE_SBC_MTU);
                    a2dp_audio_min_waterline_init(A2DP_PLAYER_PLAYBACK_MIN_WATERLINE_SBC_MTU);
#endif
#endif
#ifdef CUSTOM_BITRATE
                    dest_packet_mut = app_audio_a2dp_player_playback_delay_mtu_get(A2DP_AUDIO_CODEC_TYPE_SBC);
                    offset_mut = 0;
#endif
                    break;
                case BT_A2DP_CODEC_TYPE_MPEG2_4_AAC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
#ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
                    offset_mut =  A2DP_PLAYER_PLAYBACK_WATER_LINE;
#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
                    a2dp_audio_max_waterline_init(A2DP_PLAYER_PLAYBACK_MAX_WATERLINE_AAC_MTU);
                    a2dp_audio_min_waterline_init(A2DP_PLAYER_PLAYBACK_MIN_WATERLINE_AAC_MTU);
#endif
#endif
#ifdef CUSTOM_BITRATE
                    dest_packet_mut = app_audio_a2dp_player_playback_delay_mtu_get(A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC);
                    offset_mut = 0;
#endif
                    break;
                case BT_A2DP_CODEC_TYPE_NON_A2DP:
                    if (nonType == BT_A2DP_CODEC_NONE_TYPE_INVALID)
                    {
                        TRACE(0, "[APP_BT_STREAM] WARNNING %d non_type invalid %d", __LINE__, nonType);
                    }
#if defined(A2DP_LHDC_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDC;
                        if (bes_aud_bt->a2dp_lhdc_config_llc_get()){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                        }else if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                        }
                    }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDCV5;
                        if (bes_aud_bt->a2dp_lhdc_config_llc_get()){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                        }else if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                        }

                    }
#endif // A2DP_LHDCV5_ON
#if defined(A2DP_LDAC_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)
                    {
                        output_config.mtu_per_frame = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_FRAME_MTU;
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LDAC;
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU;
                    }
#endif // A2DP_LDAC_ON
#if defined(A2DP_L2HC_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_L2HC)
                    {
                        output_config.mtu_per_frame = A2DP_PLAYER_PLAYBACK_DELAY_L2HC_FRAME_MTU;
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_L2HC;
                        if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_L2HC_HIRES_MTU;//todo
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_L2HC_BASERES_MTU;//todo
                        }

                    }
#endif // A2DP_L2HC_ON
#if defined(A2DP_SCALABLE_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SCALABL;
                        if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU;
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU;
                        }
                        #ifdef A2DP_PLAYER_PLAYBACK_WATER_LINE
                                offset_mut =  A2DP_PLAYER_PLAYBACK_WATER_LINE;
                        #endif
#if defined(BT_DONT_PLAY_MUTE_WHEN_A2DP_STUCK_PATCH)
                        a2dp_audio_max_waterline_init(A2DP_PLAYER_PLAYBACK_MAX_WATERLINE_SCALABLE_MTU);
                        a2dp_audio_min_waterline_init(A2DP_PLAYER_PLAYBACK_MIN_WATERLINE_SCALABLE_MTU);
#endif
                    }
#endif // A2DP_SCALABLE_ON
#if defined(A2DP_LC3_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LC3)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LC3;
                        if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_MTU;//todo
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LC3_BASERES_MTU;//todo
                        }

                    }
#endif // A2DP_LC3_ON
                    else
                    {
                        TRACE(0, "[APP_BT_STREAM] WARNNING %d non_type invalid %d", __LINE__, nonType);
                    }
                    break;
                default:
                    break;
            }
            output_config.factor_reference = TWS_IBRT_AUDIO_SYNC_FACTOR_REFERENCE;

            if (a2dp_audio_chopping_is_in_post())
            {
                offset_mut = (uint32_t)(offset_mut/a2dp_audio_latency_factor_get());
                chopping_dest_mut = dest_packet_mut*a2dp_audio_latency_factor_get();
                chopping_dest_mut += offset_mut;
            }
            dest_packet_mut += offset_mut;

            switch ((AUDIO_CHANNEL_SELECT_E)p_ibrt_ctrl->audio_chnl_sel)
            {
                case AUDIO_CHANNEL_SELECT_STEREO:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
                    break;
                case AUDIO_CHANNEL_SELECT_LRMERGE:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_LRMERGE;
                    break;
                case AUDIO_CHANNEL_SELECT_LCHNL:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_LCHNL;
                    break;
                case AUDIO_CHANNEL_SELECT_RCHNL:
                    a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_RCHNL;
                    break;
                default:
                    break;
            }
            if(app_ibrt_conn_is_freeman_mode())
                a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_LRMERGE;

#if defined(__AUDIO_REVERB__) || defined(SPA_AUDIO_ENABLE)|| defined(__AUDIO_DYNAMIC_BOOST__)
            // force stereo
            a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
#endif
#if defined(FREEMAN_ENABLED_STERO)
            a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
            if(stereo_surround_status == 1){
                audio_process_stereo_surround_onoff(1);
                a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
                if(a2dp_sample_rate > AUD_SAMPRATE_48000){
                    freq = APP_SYSFREQ_208M;
                } else {
#if defined(__VIRTUAL_SURROUND_HWFIR__)
                    freq = APP_SYSFREQ_52M;
#else
                    freq = APP_SYSFREQ_104M;
#endif
                }
            }
#endif
#if defined(__VIRTUAL_SURROUND_CP__)
            if(stereo_surround_status == 1){
                audio_process_stereo_surround_onoff(1);
                a2dp_audio_channel_sel = A2DP_AUDIO_CHANNEL_SELECT_STEREO;
                if(a2dp_sample_rate > AUD_SAMPRATE_48000){
                    freq = APP_SYSFREQ_104M;
                } else {
                    freq = APP_SYSFREQ_52M;
                }  
            }
#endif
            a2dp_audio_init(freq, a2dp_audio_codec_type, &output_config, a2dp_audio_channel_sel, (uint16_t)dest_packet_mut, (uint16_t)chopping_dest_mut);
            app_bt_stream_ibrt_reset_synchronize_status();

#if defined(A2DP_SCALABLE_ON)
            if (BT_A2DP_CODEC_TYPE_NON_A2DP == codec_type)
            {
                app_tws_ibrt_audio_analysis_interval_set(20);
            }
            else
            {
                app_tws_ibrt_audio_analysis_interval_set(sample_rate > AUD_SAMPRATE_48000 ? AUDIO_ANALYSIS_INTERVAL*2 : AUDIO_ANALYSIS_INTERVAL);
            }
#else
            app_tws_ibrt_audio_analysis_interval_set(sample_rate > AUD_SAMPRATE_48000 ? AUDIO_ANALYSIS_INTERVAL*2 : AUDIO_ANALYSIS_INTERVAL);
#endif
            if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
            {
#ifdef AUDIO_OUTPUT_ROUTE_SELECT
                if (!stream_route_info.output_route_info)
#endif
                {
                    app_tws_ibrt_audio_analysis_start(0, AUDIO_ANALYSIS_CHECKER_INTERVEL_INVALID);
                    app_tws_ibrt_audio_sync_start();
                    app_bt_stream_ibrt_audio_master_detect_next_packet_start();
                }
            }
            else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
            {
                app_bt_stream_ibrt_audio_slave_detect_next_packet_start(need_autotrigger);
            }
            else
            {
                TRACE_AUD_STREAM_E("[A2DP_PLAYER] mobile_link:%d %04x ibrt_link:%d %04x", \
                    APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), \
                    APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
                    APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote),
                    APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
            }
#else
            uint8_t codec_type = bta_get_curr_a2dp_codec_type();
            uint16_t dest_packet_mut = 0;

            switch (codec_type)
            {
                case BT_A2DP_CODEC_TYPE_SBC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SBC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SBC_MTU;
                    break;
                case BT_A2DP_CODEC_TYPE_MPEG2_4_AAC:
                    a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_MPEG2_4_AAC;
                    dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_AAC_MTU;
                    break;
                case BT_A2DP_CODEC_TYPE_NON_A2DP:
                    if (nonType == BT_A2DP_CODEC_NONE_TYPE_INVALID)
                    {
                        TRACE(0, "[APP_BT_STREAM] WARNNING %d non_type invalid %d", __LINE__, nonType);
                    }
#if defined(A2DP_LHDC_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDC)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDC;
                        if (bes_aud_bt->a2dp_lhdc_config_llc_get())
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                        }
                        else if (sample_rate > AUD_SAMPRATE_48000)
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                        }
                        else
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                        }
                    }
#endif // A2DP_LHDC_ON
#if defined(A2DP_LHDCV5_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LHDCV5)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LHDCV5;
                        if (bes_aud_bt->a2dp_lhdc_config_llc_get())
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_LLC_MTU;
                        }
                        else if (sample_rate > AUD_SAMPRATE_48000)
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_HIRES_MTU;
                        }
                        else
                        {
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LHDC_BASERES_MTU;
                        }
                    }
#endif // A2DP_LHDCV5_ON
#if defined(A2DP_LDAC_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LDAC)
                    {
                        output_config.mtu_per_frame = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_FRAME_MTU;
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LDAC;
                        dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LDAC_MTU;
                    }
#endif // A2DP_LDAC_ON
#if defined(A2DP_SCALABLE_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_SCALABLE)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_SCALABL;
                        if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_HIRES_MTU;
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_SCALABLE_BASERES_MTU;
                        }
                    }
#endif // A2DP_SCALABLE_ON
#if  defined(A2DP_LC3_ON)
                    else if (nonType == BT_A2DP_CODEC_NONE_TYPE_LC3)
                    {
                        a2dp_audio_codec_type = A2DP_AUDIO_CODEC_TYPE_LC3;
                        if (sample_rate > AUD_SAMPRATE_48000){
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LC3_HIRES_MTU;//todo
                        }else{
                            dest_packet_mut = A2DP_PLAYER_PLAYBACK_DELAY_LC3_BASERES_MTU;//todo
                        }
                    }
#endif // A2DP_LC3_ON
                    else
                    {
                        TRACE(0, "[APP_BT_STREAM] WARNNING %d non_type invalid %d", __LINE__, nonType);
                    }
                    break;
                default:
                    break;
            }

            a2dp_audio_init(freq, a2dp_audio_codec_type, &output_config, a2dp_audio_channel_sel, (uint16_t)dest_packet_mut, (uint16_t)chopping_dest_mut);
            a2dp_audio_detect_next_packet_callback_register(app_bt_stream_detect_next_packet_cb);
#endif
            a2dp_audio_start();
#else
            a2dp_audio_init(freq);
#endif
        }

#if defined(MUSIC_DELAY_CONTROL) && (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)|| defined(CHIP_BEST1400)|| defined(CHIP_BEST1402))
        calib_reset = 1;
#endif

        osThreadId localThread = osThreadGetId();
        osPriority currentPriority = osThreadGetPriority(localThread);
        osThreadSetPriority(localThread, osPriorityRealtime);

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
            }
        }

#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
        af_codec_dac1_sw_gain_enable(false);
#endif

        af_codec_set_playback_post_handler(bt_sbc_player_playback_post_handler);

#ifdef __A2DP_PLAYER_USE_BT_TRIGGER__
#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            app_bt_stream_trigger_init();
        }
#endif

#if defined(VOICE_DEV)
        uint32_t algos = VOICE_DEV_ALGO_AEC;
        // TODO: Based on A2DP. SBC: 16ms; AAC: 24ms
        uint32_t algos_frame_len = 256;

#if defined(APP_NOISE_ESTIMATION)
        algos |= VOICE_DEV_ALGO_NOISE_EST;
#endif

        voice_dev_ctl(VOICE_DEV_USER0, VOICE_DEV_SET_ALGO_FRAME_LEN, &algos_frame_len);
        voice_dev_ctl(VOICE_DEV_USER0, VOICE_DEV_SET_SUPPORT_ALGOS, &algos);

#if defined(VOICE_DEV_TEST)
        test_voice_dev_open();
        test_voice_dev_start();
#endif

#if defined(APP_NOISE_ESTIMATION)
        app_noise_estimation_open();
        app_noise_estimation_start();
#endif
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            osThreadSetPriority(localThread, currentPriority);
        }

#ifdef AI_AEC_CP_ACCEL
        cp_aec_init();
#endif

#ifdef A2DP_DUMP_REF_FB
    	af_stream_start(A2DP_DUMP_STREAM_ID, AUD_STREAM_CAPTURE);
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#ifdef AUDIO_ANC_FB_ADJ_MC
        af_stream_start(ADJ_MC_STREAM_ID, AUD_STREAM_CAPTURE);
#endif
#endif

#ifdef AUDIO_ADJ_EQ
        af_stream_start(ADJ_EQ_STREAM_ID, AUD_STREAM_CAPTURE);
#endif

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
        if (!stream_route_info.output_route_info)
#endif
        {
            app_bt_stream_trigger_checker_start();
        }
#ifdef AUDIO_RMS_MONITOR_ENABLE
        rms_debug_cnt = 256;
#endif
    }

    a2dp_is_run = (on != PLAYER_OPER_STOP);
    a2dp_audio_status_updated_callback(a2dp_is_run);
    return 0;
}

#if defined(SCO_DMA_SNAPSHOT)
static uint32_t sco_trigger_wait_codecpcm = 0;
static uint32_t sco_trigger_wait_btpcm = 0;
void app_bt_stream_sco_trigger_set_codecpcm_triggle(uint8_t triggle_en)
{
    sco_trigger_wait_codecpcm = triggle_en;
}

uint32_t app_bt_stream_sco_trigger_wait_codecpcm_triggle(void)
{
    return sco_trigger_wait_codecpcm;
}

void app_bt_stream_sco_trigger_set_btpcm_triggle(uint32_t triggle_en)
{
    sco_trigger_wait_btpcm = triggle_en;
}

uint32_t app_bt_stream_sco_trigger_wait_btpcm_triggle(void)
{
    return sco_trigger_wait_btpcm;
}

static int app_bt_stream_sco_trigger_codecpcm_tick(void)
{
    if(app_bt_stream_sco_trigger_wait_codecpcm_triggle() == 0)
        return 0;

    app_bt_stream_sco_trigger_set_codecpcm_triggle(0);
#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
    {
        TRACE_AUD_STREAM_I("[SCO_PLAYER] codecpcm_tick:%x/%x", btdrv_syn_get_curr_ticks(),
                bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote)));
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        TRACE_AUD_STREAM_I("[SCO_PLAYER] codecpcm_tick:%x/%x", btdrv_syn_get_curr_ticks(),
                bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)));
    }
    else
    {
        TRACE_AUD_STREAM_E("[SCO_PLAYER] codecpcm_tick mobile_link:%d %04x ibrt_link:%d %04x",
                APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), \
                APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
                APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote), \
                APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
#else
    uint16_t conhdl = 0xFFFF;
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID)
    {
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
        conhdl = app_bt_get_curr_sco_hci_handle(curr_sco);
#else
        conhdl = btif_hf_get_remote_hci_handle(app_audio_manager_get_active_sco_chnl());
#endif
    }
    if (conhdl != 0xFFFF){
        TRACE_AUD_STREAM_I("[SCO_PLAYER] codecpcm_tick:%x", bt_syn_get_curr_ticks(conhdl));
    }
#endif

    return 1;
}

void bt_sco_clear_trigger(void)
{
    btdrv_syn_clr_trigger(0);
    bt_syn_cancel_tg_ticks(0);
}

int app_bt_stream_sco_trigger_btpcm_tick(void)
{
    if(app_bt_stream_sco_trigger_wait_btpcm_triggle() == 0)
        return 0;

    app_bt_stream_sco_trigger_set_btpcm_triggle(0);
#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());
    bool is_link_connected = APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote);
    if (is_link_connected)
    {
        TRACE_AUD_STREAM_I("[SCO_PLAYER] btpcm_tick:%x/%x", btdrv_syn_get_curr_ticks(),
                            bt_syn_get_curr_ticks(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote)));
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        TRACE_AUD_STREAM_I("[SCO_PLAYER] btpcm_tick:%x/%x", btdrv_syn_get_curr_ticks(),
                            bt_syn_get_curr_ticks(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote)));
    }
    else
    {
        TRACE_AUD_STREAM_I("[SCO_PLAYER] btpcm_tick: mobile_link:%d %04x ibrt_link:%d %04x", \
                is_link_connected, \
                APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
                APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote), \
                APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
#else
    uint16_t conhdl = 0xFFFF;
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID)
    {
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
        conhdl = app_bt_get_curr_sco_hci_handle(curr_sco);
#else
        conhdl = btif_hf_get_remote_hci_handle(app_audio_manager_get_active_sco_chnl());
#endif
    }
    TRACE_AUD_STREAM_I("[SCO_PLAYER] btpcm_tick:%x", bt_syn_get_curr_ticks(conhdl));
#endif
    btdrv_syn_clr_trigger(0);
    bt_syn_cancel_tg_ticks(0);
    return 1;
}

void app_bt_stream_sco_trigger_btpcm_start(void )
{
    uint32_t curr_ticks = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    uint16_t conhdl = 0xFFFF;

    uint32_t lock;

#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote)){
        conhdl = APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        conhdl = APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote);
    }
    else
    {
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start mobile_link:%d %04x ibrt_link:%d %04x", \
            APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), \
            APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote),APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
#else
    int curr_sco;
    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID)
    {
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
        conhdl = app_bt_get_curr_sco_hci_handle(curr_sco);
#else
        conhdl = btif_hf_get_remote_hci_handle(app_audio_manager_get_active_sco_chnl());
#endif
    }
#endif

    lock = int_lock();
    curr_ticks = bt_syn_get_curr_ticks(conhdl);

    tg_acl_trigger_offset_time = (curr_ticks+0x180) - ((curr_ticks+0x180)%192);

    btdrv_syn_clr_trigger(0);
    btdrv_enable_playback_triggler(SCO_TRIGGLE_MODE);

#if defined(IBRT)
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
    {
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start set ticks:%d,",tg_acl_trigger_offset_time);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote)){
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start set ticks:%d,",tg_acl_trigger_offset_time);
    }
    else
    {
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start mobile_link:%d %04x ibrt_link:%d %04x", \
            APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote), APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote),
            APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote),APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
    TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start get ticks:%d,",curr_ticks);

#else
    bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, conhdl, BT_TRIG_SLAVE_ROLE,0,false);
#endif

    app_bt_stream_sco_trigger_set_btpcm_triggle(1);
    TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] btpcm_start curr clk=%x, triggle_clk=%x, bt_clk=%x",
                            btdrv_syn_get_curr_ticks(),
                            tg_acl_trigger_offset_time,
                            bt_syn_get_curr_ticks(conhdl));
    int_unlock(lock);
}

void app_bt_stream_sco_trigger_btpcm_stop(void)
{
    return;
}


#define TIRG_DELAY_THRESHOLD_325US (15) //total:TIRG_DELAY_THRESHOLD_325US*325us
#define TIRG_DELAY_MAX  (20) //total:20*TIRG_DELAY_325US*325us

#define TIRG_DELAY_325US (96) //total:TIRG_DELAY_325US*325us It' up to the codec and bt pcm pingpang buffer.

void app_bt_stream_sco_trigger_codecpcm_start(uint32_t btclk, uint16_t btcnt )
{
    uint32_t curr_ticks = 0;
    uint32_t tg_acl_trigger_offset_time = 0;
    uint32_t lock;
    uint16_t conhdl = 0xFFFF;
    //must lock the interrupts when set trig ticks.

#if defined(IBRT)
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());
    bool is_link_connected = APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote);
    if (is_link_connected)
    {
        conhdl = APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        conhdl = APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote);
    }
    else
    {
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] codecpcm_start mobile_link:%d %04x ibrt_link:%d %04x",
            is_link_connected, APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote),
            APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote),   APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
#else
    int curr_sco;
    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID)
    {
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
        conhdl = app_bt_get_curr_sco_hci_handle(curr_sco);
#else
        conhdl = btif_hf_get_remote_hci_handle(app_audio_manager_get_active_sco_chnl());
#endif
    }
#endif

    lock = int_lock();
    curr_ticks = bt_syn_get_curr_ticks(conhdl);
    TRACE_AUD_STREAM_I("[[STRM_TRIG][SCO] codecpcm_start 1 curr:%d clk:%dcnt:%d", curr_ticks,btclk,btcnt);

#ifdef LOW_DELAY_SCO
    tg_acl_trigger_offset_time=btclk+BUF_BTCLK_NUM+CODEC_TRIG_BTCLK_OFFSET;
#else
    tg_acl_trigger_offset_time=btclk+BUF_BTCLK_NUM+CODEC_TRIG_BTCLK_OFFSET;
#endif

#if defined(BT_CLK_UNIT_IN_HSLOT)

#else
    tg_acl_trigger_offset_time = tg_acl_trigger_offset_time * 2;
#endif

    if(tg_acl_trigger_offset_time<curr_ticks+TIRG_DELAY_THRESHOLD_325US)
    {
        int tirg_delay=0;
        tirg_delay=((curr_ticks+TIRG_DELAY_THRESHOLD_325US)-tg_acl_trigger_offset_time)/TIRG_DELAY_325US;
        tirg_delay=tirg_delay+1;
        if(tirg_delay>TIRG_DELAY_MAX)
        {
            tirg_delay=TIRG_DELAY_MAX;
            TRACE_AUD_STREAM_W("[STRM_TRIG][SCO] codecpcm_start bt clk convolution!");
        }
        tg_acl_trigger_offset_time=tg_acl_trigger_offset_time+tirg_delay*TIRG_DELAY_325US;
        TRACE_AUD_STREAM_W("[STRM_TRIG][SCO] codecpcm_start need more tirg_delay:%d offset:%d curr:%d,",
                            tirg_delay,tg_acl_trigger_offset_time,curr_ticks);
    }

#if defined(CHIP_BEST1501SIMU) || defined(CHIP_BEST1600SIMU)
    {
        tg_acl_trigger_offset_time = curr_ticks + 500; ////temporarily added
    }
#endif
    tg_acl_trigger_offset_time &= 0x0fffffff;

//    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
//    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);
    btdrv_syn_clr_trigger(0);


#if defined(IBRT)
    if (is_link_connected)
    {
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] codecpcm_start set 2 tg_acl_trigger_offset_time:%d",tg_acl_trigger_offset_time);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote), BT_TRIG_SLAVE_ROLE,0,false);
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] codecpcm_start set 2 tg_acl_trigger_offset_time:%d",tg_acl_trigger_offset_time);
    }
    else
    {
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] codecpcm_start mobile_link:%d %04x ibrt_link:%d %04x",
            is_link_connected, APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote),
            APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote),   APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote));
    }
#else
    bt_syn_set_tg_ticks(tg_acl_trigger_offset_time, conhdl, BT_TRIG_SLAVE_ROLE,0,false);
#endif
    app_bt_stream_sco_trigger_set_codecpcm_triggle(1);

    btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);

    int_unlock(lock);

    TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] codecpcm_start enable curr:%x trig:%x curr:%x",
                        btdrv_syn_get_curr_ticks(),
                        tg_acl_trigger_offset_time,
                        curr_ticks);

//    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
//    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);

}

void app_bt_stream_sco_trigger_codecpcm_stop(void)
{
#ifdef PLAYBACK_USE_I2S
    af_i2s_sync_config(AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, false);
#else
    af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, false);
#endif
    af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, false);
}
#endif

void speech_tx_aec_set_frame_len(int len);
int voicebtpcm_pcm_echo_buf_queue_init(uint32_t size);
void voicebtpcm_pcm_echo_buf_queue_reset(void);
void voicebtpcm_pcm_echo_buf_queue_deinit(void);
#if defined(SCO_DMA_SNAPSHOT)
int voicebtpcm_pcm_audio_init(int sco_sample_rate, int tx_vqe_sample_rate, int rx_vqe_sample_rate, int tx_codec_sample_rate, int rx_codec_sample_rate, int capture_channel_num);
#else
int voicebtpcm_pcm_audio_init(int sco_sample_rate, int codec_sample_rate);
#endif
int voicebtpcm_pcm_audio_deinit(void);
uint32_t voicebtpcm_pcm_audio_data_come(uint8_t *buf, uint32_t len);
uint32_t voicebtpcm_pcm_audio_more_data(uint8_t *buf, uint32_t len);
int store_voicebtpcm_m2p_buffer(unsigned char *buf, unsigned int len);
int get_voicebtpcm_p2m_frame(unsigned char *buf, unsigned int len);
extern "C" int32_t plc_monitor_dump_data(uint8_t *buf, uint32_t len);
static uint32_t mic_force_mute = 0;
static uint32_t spk_force_mute = 0;

static enum AUD_CHANNEL_NUM_T sco_play_chan_num;
static enum AUD_CHANNEL_NUM_T sco_cap_chan_num;

static hfp_sco_codec_t g_bt_sco_codec_type = BT_HFP_SCO_CODEC_NONE;

static void bt_sco_codec_store(void)
{
    g_bt_sco_codec_type = app_audio_manager_get_scocodecid();

    if (g_bt_sco_codec_type == BT_HFP_SCO_CODEC_NONE) {
        TRACE(2, "[%s] WARNING:%d is invalid sco codec type, use default codec type!", __func__, g_bt_sco_codec_type);
        g_bt_sco_codec_type = BT_HFP_SCO_CODEC_CVSD;
    }
    TRACE(0, "[%s] Codec Type: %d", __func__, g_bt_sco_codec_type);
}

hfp_sco_codec_t bt_sco_codec_get_type(void)
{
    return g_bt_sco_codec_type;
}

extern "C" bool bt_sco_codec_is_msbc(void)
{
    if (g_bt_sco_codec_type == BT_HFP_SCO_CODEC_MSBC) {
        return true;
    } else {
        return false;
    }
}

extern "C" bool bt_sco_codec_is_cvsd(void)
{
    if (g_bt_sco_codec_type == BT_HFP_SCO_CODEC_CVSD) {
        return true;
    } else {
        return false;
    }
}

extern "C" bool bt_sco_codec_is_lc3(void)
{
    if (g_bt_sco_codec_type == BT_HFP_SCO_CODEC_LC3_SWB) {
        return true;
    } else {
        return false;
    }
}

void bt_sco_mobile_clkcnt_get(uint32_t btclk, uint16_t btcnt,
                                     uint32_t *mobile_master_clk, uint16_t *mobile_master_cnt)
{
#if defined(IBRT)
    uint8_t curr_sco_device = app_bt_audio_get_curr_sco_device();
    app_tws_ibrt_audio_mobile_clkcnt_get(curr_sco_device, btclk, btcnt, mobile_master_clk, mobile_master_cnt);
#else
    uint16_t conhdl = 0xFFFF;
    int32_t clock_offset;
    uint16_t bit_offset;
    int curr_sco;

    curr_sco = app_audio_manager_get_active_sco_num();
    if (curr_sco != BT_DEVICE_INVALID_ID){
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
        conhdl = app_bt_get_curr_sco_hci_handle(curr_sco);
#else
        conhdl = btif_hf_get_remote_hci_handle(app_audio_manager_get_active_sco_chnl());
#endif
    }

    if (conhdl != BT_INVALID_CONN_HANDLE){
        bt_drv_reg_op_piconet_clk_offset_get(conhdl, &clock_offset, &bit_offset);
        //TRACE_AUD_STREAM_I("mobile piconet clk:%d bit:%d loc clk:%d cnt:%d", clock_offset, bit_offset, btclk, btcnt);
        btdrv_slave2master_clkcnt_convert(btclk, btcnt,
                                          clock_offset, bit_offset,
                                          mobile_master_clk, mobile_master_cnt);
    }else{
        TRACE_AUD_STREAM_W("[STRM_TRIG][SCO] mobile_clkcnt_get warning conhdl NULL conhdl:%x", conhdl);
        *mobile_master_clk = 0;
        *mobile_master_cnt = 0;
    }
#endif
}


#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)

#ifdef CHIP_BEST1000
#error "Unsupport SW_SCO_RESAMPLE on best1000 by now"
#endif
#ifdef NO_SCO_RESAMPLE
#error "Conflicted config: NO_SCO_RESAMPLE and SW_SCO_RESAMPLE"
#endif

// The decoded playback data in the first irq is output to DAC after the second irq (PING-PONG buffer)
#define SCO_PLAY_RESAMPLE_ALIGN_CNT     2

static uint8_t sco_play_irq_cnt;
static bool sco_dma_buf_err;
static struct APP_RESAMPLE_T *sco_capture_resample;
static struct APP_RESAMPLE_T *sco_playback_resample;

static int bt_sco_capture_resample_iter(uint8_t *buf, uint32_t len)
{
    voicebtpcm_pcm_audio_data_come(buf, len);
    return 0;
}

static int bt_sco_playback_resample_iter(uint8_t *buf, uint32_t len)
{
    voicebtpcm_pcm_audio_more_data(buf, len);
    return 0;
}

#endif
#if defined(SCO_DMA_SNAPSHOT)

extern int process_downlink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len,
                                        uint8_t *out_buf, uint32_t out_len);
extern int process_uplink_bt_voice_frames(uint8_t *in_buf, uint32_t in_len,
                uint8_t *ref_buf, uint32_t ref_len, uint8_t *out_buf, uint32_t out_len);
#define MSBC_FRAME_LEN (60)
#define PCM_LEN_PER_FRAME (240)

#define CAL_FRAME_NUM (22)

static void bt_sco_codec_sync_tuning(void)
{
    uint32_t btclk = 0;
    uint16_t btcnt = 0;

    uint32_t mobile_master_clk;
    uint16_t mobile_master_cnt;

    uint32_t mobile_master_clk_offset;
    int32_t mobile_master_cnt_offset;

    static float fre_offset=0.0f;

#if defined(SCO_TUNING_NEWMETHOD)
    static float fre_offset_long_time=0.0f;
    static int32_t fre_offset_flag=0;
#endif

    static int32_t mobile_master_cnt_offset_init;
    static int32_t mobile_master_cnt_offset_old;
    static uint32_t first_proc_flag=0;
#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
    static uint32_t frame_counter=0;
    static int32_t mobile_master_cnt_offset_max=0;
    static int32_t mobile_master_cnt_offset_min=0;
    static int32_t mobile_master_cnt_offset_resample=0;

     int32_t offset_max=0;
     int32_t offset_min=0;
#endif

#if defined(MULTI_DMA_TC)
    uint8_t adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    uint32_t dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
    if(adma_ch != HAL_DMA_CHAN_NONE)
    {
        bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
    }
#else
    bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
#endif
    bt_sco_mobile_clkcnt_get(btclk, btcnt,
                                             &mobile_master_clk, &mobile_master_cnt);

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
    TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] tune btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif

    if((mobile_master_clk < last_mobile_master_clk)&&(mobile_master_clk < BUF_BTCLK_NUM*2))
    {
        //clock wrapped, 0x555556*24=0x8000010, so after clock wrap, offset init need add 16
        mobile_master_clk_offset_init = (mobile_master_clk_offset_init+WRAPED_CLK_OFFSET)%BUF_BTCLK_NUM;
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO]:MAX_CLOCK reach btclk:%d,last btclk:%d",mobile_master_clk,last_mobile_master_clk);
    }
    last_mobile_master_clk = mobile_master_clk;

    mobile_master_clk_offset=(mobile_master_clk-mobile_master_clk_offset_init+BUF_BTCLK_NUM)%BUF_BTCLK_NUM;
    mobile_master_cnt_offset=(int32_t)(mobile_master_clk_offset*BTCLK_UNIT+(625-mobile_master_cnt)*BTCNT_UNIT);
    mobile_master_cnt_offset=(int32_t)(mobile_master_cnt_offset-(CODEC_TRIG_DELAY_OFFSET+mobile_master_cnt_offset_init));

    if(app_bt_stream_sco_trigger_codecpcm_tick())
    {
        fre_offset=0.0f;

#if defined(SCO_TUNING_NEWMETHOD)
        fre_offset_long_time=0.0f;
        fre_offset_flag=0;
#endif

        mobile_master_cnt_offset_old=0;
        first_proc_flag=0;

       if(playback_samplerate_codecpcm==AUD_SAMPRATE_16000)
       {
#ifdef  ANC_APP

#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=171;
#else
             mobile_master_cnt_offset_init=107;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=90;
        }

#else

#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=171;
#else
             mobile_master_cnt_offset_init=146;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=113;
        }
#endif
       }
       else if(playback_samplerate_codecpcm==AUD_SAMPRATE_8000)
       {
#ifdef  ANC_APP
#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=-270;
#else
             mobile_master_cnt_offset_init=-468;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=-445;
        }
#else
#if defined( __AUDIO_RESAMPLE__)
        if (hal_cmu_get_audio_resample_status())
        {
#if defined(AUDIO_RESAMPLE_ANTI_DITHER)
             mobile_master_cnt_offset_init=-270;
#else
             mobile_master_cnt_offset_init=-327;
#endif
        }
        else
#endif
        {
            mobile_master_cnt_offset_init=-386;
        }
#endif
       }
        mobile_master_cnt_offset=(int32_t)(mobile_master_clk_offset*BTCLK_UNIT+(625-mobile_master_cnt)*BTCNT_UNIT);
        mobile_master_cnt_offset=(int32_t)(mobile_master_cnt_offset-(CODEC_TRIG_DELAY_OFFSET+mobile_master_cnt_offset_init));
        TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] tune mobile_master_cnt_offset:%d,",mobile_master_cnt_offset);

#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
#ifdef LOW_DELAY_SCO
        fre_offset=(float)mobile_master_cnt_offset/(CAL_FRAME_NUM*7.5f*1000.0f);
#else
        fre_offset=(float)mobile_master_cnt_offset/(CAL_FRAME_NUM*15.0f*1000.0f);
#endif
#endif

#if defined( __AUDIO_RESAMPLE__) && !defined(AUDIO_RESAMPLE_ANTI_DITHER)
      if (hal_cmu_get_audio_resample_status())
      {
            frame_counter=0;
            mobile_master_cnt_offset_max=0;
            mobile_master_cnt_offset_min=0;
            mobile_master_cnt_offset_resample=0;
      }
#endif
    }

#if defined(  __AUDIO_RESAMPLE__) &&!defined(SW_PLAYBACK_RESAMPLE)&& !defined(AUDIO_RESAMPLE_ANTI_DITHER)
      if (hal_cmu_get_audio_resample_status())
      {
           if(playback_samplerate_codecpcm==AUD_SAMPRATE_16000)
           {
            offset_max=28;
            offset_min=-33;
           }
           else if(playback_samplerate_codecpcm==AUD_SAMPRATE_8000)
           {
            offset_max=12;
            offset_min=-112;
           }

           if(mobile_master_cnt_offset>mobile_master_cnt_offset_max)
           {
            mobile_master_cnt_offset_max=mobile_master_cnt_offset;
           }

           if(mobile_master_cnt_offset<mobile_master_cnt_offset_min)
           {
            mobile_master_cnt_offset_min=mobile_master_cnt_offset;
           }

            frame_counter++;

            if(frame_counter>=CAL_FRAME_NUM)
            {
               if(mobile_master_cnt_offset_min<offset_min)
               {
                    mobile_master_cnt_offset_resample=mobile_master_cnt_offset_min-offset_min;
               }
               else if(mobile_master_cnt_offset_max>offset_max)
               {
                   mobile_master_cnt_offset_resample=mobile_master_cnt_offset_max-offset_max;
               }
               else
               {
                   mobile_master_cnt_offset_resample=0;
               }
              TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] tune mobile_master_cnt_offset:%d/%d",mobile_master_cnt_offset_min,mobile_master_cnt_offset_max);
               mobile_master_cnt_offset=mobile_master_cnt_offset_resample;

            if(first_proc_flag==0)
            {
                  fre_offset=((int32_t)(mobile_master_cnt_offset*0.5f))*0.0000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.0000001f;
                  first_proc_flag=1;
            }
            else
            {
                  fre_offset=fre_offset+((int32_t)(mobile_master_cnt_offset*0.5f))*0.0000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.0000001f;
                  first_proc_flag=1;
            }

            mobile_master_cnt_offset_old=mobile_master_cnt_offset;
#if defined(SCO_DMA_SNAPSHOT_DEBUG)
               TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] tune mobile_master_cnt_offset:%d", mobile_master_cnt_offset);
#endif
               mobile_master_cnt_offset_max=0;
               mobile_master_cnt_offset_min=0;
               frame_counter=0;
            }
        }
        else
#endif
        {
#if defined(SCO_TUNING_NEWMETHOD)
            if(mobile_master_cnt_offset>0)
            {
#if defined(LOW_DELAY_SCO)
                fre_offset=0.00005f+fre_offset_long_time;
#else
                fre_offset=0.0001f+fre_offset_long_time;
#endif
                first_proc_flag=0;
                fre_offset_flag=1;
                if(mobile_master_cnt_offset>=mobile_master_cnt_offset_old)
                {
                    fre_offset_long_time=fre_offset_long_time+0.0000001f;
                    fre_offset_flag=0;
                }
            }
            else if(mobile_master_cnt_offset<0)
            {
#if defined(LOW_DELAY_SCO)
                fre_offset=-0.00005f+fre_offset_long_time;
#else
                fre_offset=-0.0001f+fre_offset_long_time;
#endif
                first_proc_flag=0;
                fre_offset_flag=-1;
                if(mobile_master_cnt_offset<=mobile_master_cnt_offset_old)
                {
                    fre_offset_long_time=fre_offset_long_time-0.0000001f;
                    fre_offset_flag=0;
                }
            }
            else
            {
                fre_offset=fre_offset_long_time+fre_offset_flag*0.0000001f;
                fre_offset_long_time=fre_offset;
                first_proc_flag=1;
                fre_offset_flag=0;
            }

            if(fre_offset_long_time>0.0001f)fre_offset_long_time=0.0001f;
            if(fre_offset_long_time<-0.0001f)fre_offset_long_time=-0.0001f;

            mobile_master_cnt_offset_old=mobile_master_cnt_offset;
#if defined(SCO_DMA_SNAPSHOT_DEBUG)
            {
                int freq_ppm_int=(int)(fre_offset_long_time*1000000.0f);
                int freq_ppm_Fra=(int)(fre_offset_long_time*10000000.0f)-((int)(fre_offset_long_time*1000000.0f))*10;
                if(freq_ppm_Fra<0)freq_ppm_Fra=-freq_ppm_Fra;

                TRACE(1,"freq_offset_long_time(ppm):%d.%d",freq_ppm_int,freq_ppm_Fra);
            }
#endif

#else
            if(mobile_master_cnt_offset>5&&first_proc_flag==0)
            {
                fre_offset=0.0001f;
            }
            else if(mobile_master_cnt_offset<-5&&first_proc_flag==0)
            {
                fre_offset=-0.0001f;
            }
            else
            {
                if(first_proc_flag==0)
                {
                    fre_offset=0;
                }
                fre_offset=fre_offset+((int32_t)(mobile_master_cnt_offset*0.5f))*0.00000001f
                    +(mobile_master_cnt_offset-mobile_master_cnt_offset_old)*0.00000001f;

                mobile_master_cnt_offset_old=mobile_master_cnt_offset;
                first_proc_flag=1;
            }
#endif
        }

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
        {
            POSSIBLY_UNUSED int freq_ppm_int=(int)(fre_offset*1000000.0f);
            int freq_ppm_Fra=(int)(fre_offset*10000000.0f)-((int)(fre_offset*1000000.0f))*10;
            if(freq_ppm_Fra<0)freq_ppm_Fra=-freq_ppm_Fra;
            TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] time_offset(us):%d",mobile_master_cnt_offset);
            TRACE_AUD_STREAM_I("[STRM_TRIG][SCO] freq_offset(ppm):%d.%d",freq_ppm_int,freq_ppm_Fra);
        }
#endif
        if(first_proc_flag==1)
       {
            if(fre_offset>0.0001f)fre_offset=0.0001f;
            if(fre_offset<-0.0001f)fre_offset=-0.0001f;
       }
#if defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE)
    app_resample_tune(playback_samplerate_codecpcm, fre_offset);
#else
    af_codec_tune(AUD_STREAM_NUM, fre_offset);
#endif

    return;
}
#endif
extern CQueue* get_tx_esco_queue_ptr();

#if defined(ANC_NOISE_TRACKER)
static int16_t *anc_buf = NULL;
#endif

#if defined(SCO_DMA_SNAPSHOT)
// This is reletive with chip.
#ifdef TX_RX_PCM_MASK
#define BTPCM_TX_OFFSET_BYTES   (0)
#else
#if defined (PCM_FAST_MODE)
#define BTPCM_TX_OFFSET_BYTES   (1)
#else
#define BTPCM_TX_OFFSET_BYTES   (2)
#endif
#endif

#define SCO_MSBC_PACKET_SIZE    (60)

#ifndef PCM_PRIVATE_DATA_FLAG
static uint8_t g_btpcm_tx_buf[BTPCM_TX_OFFSET_BYTES];
#endif
static void adjust_btpcm_msbc_tx(uint8_t *buf, uint32_t len)
{
    uint16_t *buf_u16 = (uint16_t *)buf;
#ifndef PCM_PRIVATE_DATA_FLAG
    uint8_t *buf_ptr = (uint8_t *)buf;
    uint32_t loop_cnt = len / SCO_MSBC_PACKET_SIZE;

    ASSERT(len % SCO_MSBC_PACKET_SIZE == 0, "[%s] len(%d) is invalid!", __func__, len);

    // Shift
    for (uint32_t cnt=0; cnt <loop_cnt; cnt++) {
        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            g_btpcm_tx_buf[i] = buf_ptr[i];
        }

        for (uint32_t i=0; i<SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i] = buf_ptr[i + BTPCM_TX_OFFSET_BYTES];
        }

        for (uint32_t i=0; i<BTPCM_TX_OFFSET_BYTES; i++) {
            buf_ptr[i + SCO_MSBC_PACKET_SIZE - BTPCM_TX_OFFSET_BYTES] = g_btpcm_tx_buf[i];
        }

        buf_ptr += SCO_MSBC_PACKET_SIZE;
    }
#endif
    // BTCPM trans data with 16bits format and valid data is in high 8 bits.
    for (int32_t i = len-1; i >= 0; i--) {
        buf_u16[i] = ((int16_t)buf[i]) << 8;
    }

}
#endif

//#define BT_SCO_HANDLER_PROFILE

//( codec:mic-->btpcm:tx
// codec:mic
static uint32_t bt_sco_codec_capture_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE);

#if defined(CHIP_BEST1501SIMU) && defined(PLAYBACK_USE_I2S)
    int16_t *pcm_buf = (int16_t *)buf;
    uint32_t frame_len = len / sizeof(int16_t) / AUD_CHANNEL_NUM_2;

    for(uint32_t i = 0; i < frame_len; i++)
        pcm_buf[i] = pcm_buf[AUD_CHANNEL_NUM_2*i];

    len /= AUD_CHANNEL_NUM_2;
#endif

#if defined(ANC_NOISE_TRACKER)
    int16_t *pcm_buf = (int16_t *)buf;
    uint32_t pcm_len = len / sizeof(short);
    uint32_t ch_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM + ANC_NOISE_TRACKER_CHANNEL_NUM;
    uint32_t remain_ch_num = SPEECH_CODEC_CAPTURE_CHANNEL_NUM;


#if defined(SPEECH_TX_AEC_CODEC_REF)
    ch_num += 1;
    remain_ch_num += 1;
#endif

    ASSERT(pcm_len % ch_num == 0, "[%s] input data length error", __FUNCTION__);

    // assume anc mic in ch0
    for (uint32_t i = 0, j = 0; i < pcm_len; i += ch_num, j += ANC_NOISE_TRACKER_CHANNEL_NUM)
    {
        for (uint32_t ch = 0; ch < ANC_NOISE_TRACKER_CHANNEL_NUM; ch++)
            anc_buf[j + ch] = pcm_buf[i + ch];
    }

    noise_tracker_process(anc_buf, pcm_len / ch_num * ANC_NOISE_TRACKER_CHANNEL_NUM);

    for (uint32_t i = 0, j = 0; i < pcm_len; i += ch_num, j += remain_ch_num)
    {
        for (uint32_t chi = ANC_NOISE_TRACKER_CHANNEL_NUM, cho = 0; chi < ch_num; chi++, cho++)
            pcm_buf[j + cho] = pcm_buf[i + chi];
    }

    len = len / ch_num * remain_ch_num;
#endif

#if defined(ANC_ASSIST_ENABLED)
        if(app_anc_assist_is_runing()) {
#if defined(ASSIST_LOW_RAM_MOD)
            //resample  16k->8k
            integer_resampling_process_q23(anc_assist_resample_inst, (int32_t *)buf, len / sizeof(_PCM_T), (int32_t *)anc_assist_resample_buf);
            // TODO: Use capture buf
            app_anc_assist_process(anc_assist_resample_buf, len / 2);
#else
            app_anc_assist_process(buf, len);
#endif
        }
        app_anc_assist_parser_app_mic_buf(buf, &len); 
#endif

#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
    if (mic_force_mute)
#else
    if (mic_force_mute || btapp_hfp_mic_need_skip_frame() || btapp_hfp_need_mute())
#endif
    {
        memset(buf, 0, len);
    }

#if defined(SCO_DMA_SNAPSHOT)

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    int pingpang;

    //processing  ping pang flag
    if(buf==capture_buf_codecpcm)
    {
     pingpang=0;
    }
    else
    {
     pingpang=1;
    }

#ifndef PCM_PRIVATE_DATA_FLAG
    uint16_t *playback_dst=(uint16_t *)(playback_buf_btpcm+(pingpang)*playback_size_btpcm/2);
    uint16_t *playback_src=(uint16_t *)playback_buf_btpcm_cache;

    for(uint32_t  i =0; i<playback_size_btpcm/4; i++)
    {
        playback_dst[i]=playback_src[i];
    }
#endif

#ifdef TX_RX_PCM_MASK
    //processing btpcm.(It must be from CPU's copy )
   if(btdrv_is_pcm_mask_enable()==1&&bt_sco_codec_is_msbc())
   {
	uint32_t lock;
	uint32_t i;
	//must lock the interrupts when exchanging data.
	lock = int_lock();
	uint16_t *playback_src=(uint16_t *)(playback_buf_btpcm+(pingpang)*playback_size_btpcm/2);
	for( i =0; i<playback_size_btpcm_copy; i++)
	{
		playback_buf_btpcm_copy[i]=(uint8_t)(playback_src[i]>>8);
	}
	int_unlock(lock);
   }
#endif

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
    if(sidetone_opened==0)
    {
        sidetone_set_gain_f32(1.0f,1.0f);
        sidetone_opened=1;
    }
#endif
#endif

    //TRACE_AUD_STREAM_I("pcm length:%d",len);

    //processing clock
    bt_sco_codec_sync_tuning();


    //processing mic
    uint8_t *capture_pcm_frame_p = capture_buf_codecpcm + pingpang * capture_size_codecpcm / 2;
    uint8_t *ref_pcm_frame_p = playback_buf_codecpcm + (pingpang^1) * playback_size_codecpcm / 2;

    uint8_t *dst = playback_buf_btpcm_cache;
    uint32_t packet_len = playback_size_btpcm/2;

#if defined(PCM_PRIVATE_DATA_FLAG)
	packet_len=packet_len-(packet_len/BTPCM_TOTAL_DATA_LENGTH)*BTPCM_PRIVATE_DATA_LENGTH;
#endif

    if (bt_sco_codec_is_cvsd()) {
        process_uplink_bt_voice_frames(capture_pcm_frame_p, len, ref_pcm_frame_p, playback_size_codecpcm / 2, dst, packet_len);
    } else {
        process_uplink_bt_voice_frames(capture_pcm_frame_p, len, ref_pcm_frame_p, playback_size_codecpcm / 2, dst, packet_len/2);
        adjust_btpcm_msbc_tx(dst, packet_len/2);
    }

#if defined(PCM_PRIVATE_DATA_FLAG)
    {
        uint8_t *playback_dst=playback_buf_btpcm+(pingpang)*playback_size_btpcm/2;
        uint8_t *playback_src=playback_buf_btpcm_cache;

        for (uint32_t i = 0; i < (playback_size_btpcm/2)/BTPCM_TOTAL_DATA_LENGTH; i++)
        {
		uint8_t *dst=playback_dst+i*BTPCM_TOTAL_DATA_LENGTH+BTPCM_PRIVATE_DATA_LENGTH-(BTPCM_TX_OFFSET_BYTES*2);
		uint8_t *src=playback_src+i*BTPCM_PUBLIC_DATA_LENGTH;
        	memcpy(dst,src,BTPCM_PUBLIC_DATA_LENGTH);
        }
    }
#endif


#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][MIC] takes %d us",
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;

#else

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
    if(hal_cmu_get_audio_resample_status())
    {
        if (af_stream_buffer_error(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE)) {
            sco_dma_buf_err = true;
        }
        // The decoded playback data in the first irq is output to DAC after the second irq (PING-PONG buffer),
        // so it is aligned with the capture data after 2 playback irqs.
        if (sco_play_irq_cnt < SCO_PLAY_RESAMPLE_ALIGN_CNT) {
            // Skip processing
            return len;
        }
        app_capture_resample_run(sco_capture_resample, buf, len);
    }
    else
#endif
    {
        voicebtpcm_pcm_audio_data_come(buf, len);
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][MIC] takes %d us",
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}

#ifdef _SCO_BTPCM_CHANNEL_
// btpcm:tx
static uint32_t bt_sco_btpcm_playback_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK);

#if defined(SCO_DMA_SNAPSHOT)
    return len;
#else

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    get_voicebtpcm_p2m_frame(buf, len);

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][SPK] takes %d us",
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;

#endif
}
//)

#if defined(AF_ADC_I2S_SYNC)
void codec_capture_i2s_enable(void)
{
    uint32_t lock;

    TRACE_AUD_STREAM_I("[SCO][IIS] Start...", __func__);

    lock = int_lock();
    hal_codec_capture_enable();
    hal_i2s_enable(HAL_I2S_ID_0);
    int_unlock(lock);

}
#endif

extern CQueue* get_rx_esco_queue_ptr();

static volatile bool is_codec_stream_started = false;
#ifdef PCM_PRIVATE_DATA_FLAG

uint8_t bt_sco_pri_data_get_head_pos(uint8_t *buf, uint32_t len, uint8_t offset, uint8_t frame_num)
{
    uint8_t head_pos = 127;
    uint8_t max_j = 0;
    if(offset==0 && frame_num > 1)
    {
        // 保证至少找到Buf里的一个offset
        max_j = 120;
    }
    else
    {
        max_j = 101;
    }

    for(uint8_t j=0; j<max_j; j++)
    {
        // 查找自定义的私有数据
        if(buf[j+offset]==0xff && buf[j+14+offset]==0xff && buf[j+16+offset]==0xff && buf[j+18+offset]==0xff)
        {
            head_pos=j;
        }
    }
    return head_pos;
}

void bt_sco_btpcm_get_pcm_priv_data(struct PCM_DATA_FLAG_T *pcm_data, uint8_t *buf, uint32_t len)
{
    uint8_t frame_num = len/120;
    for(uint8_t i=0; i<frame_num; i++)
    {
        uint8_t head_pos = 120*i;
        pcm_data[i].offset = 127;
        pcm_data[i].offset = bt_sco_pri_data_get_head_pos(buf, len, head_pos, frame_num);
        if(pcm_data[i].offset == 127)
        {
            TRACE(0,"sco_pri_data: found head offset failed!");
        }
        else
        {
            uint16_t head_offset = head_pos + pcm_data[i].offset;
            pcm_data[i].undef = buf[head_offset];
            pcm_data[i].bitcnt = (buf[head_offset+2]|(buf[head_offset+4]<<8))&0x3ff;
            pcm_data[i].softbit_flag = (buf[head_offset+4]>>5)&3;
            pcm_data[i].btclk = buf[head_offset+6]|(buf[head_offset+8]<<8)|(buf[head_offset+10]<<16)|(buf[head_offset+12]<<24);
            pcm_data[i].reserved = buf[head_offset+14]|(buf[head_offset+16]<<8)|(buf[head_offset+18]<<16)|(buf[head_offset+20]<<24);
            //clear private msg in buffer
            for(uint8_t j=0; j<PCM_PRIVATE_DATA_LENGTH; j++)
                buf[head_offset+2*j] = 0;
        }
    }
}
#endif
//( btpcm:rx-->codec:spk
// btpcm:rx

static uint32_t bt_sco_btpcm_capture_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_BTPCM_CAPTURE);

#if defined(SCO_DMA_SNAPSHOT)
    uint32_t btclk = 0;
    uint16_t btcnt = 0;

    uint32_t mobile_master_clk;
    uint16_t mobile_master_cnt;

    bool  codec_stream_trig = false;
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t dma_base;
#if defined(LOW_DELAY_SCO)
    if(btpcm_int_counter>0)
    {
        btpcm_int_counter--;
        return len;
    }
#endif

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    if((is_codec_stream_started == false)&&(buf==capture_buf_btpcm))
    {
        if(!af_stream_buffer_error(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE))
        {
#if defined(MULTI_DMA_TC)
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
            }
#else
            bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
#endif
            bt_sco_mobile_clkcnt_get(btclk, btcnt,&mobile_master_clk, &mobile_master_cnt);
            hal_sys_timer_delay_us(1);
            if(!af_stream_buffer_error(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE))
            {
                codec_stream_trig = true;
            }
        }
    }

    if (codec_stream_trig) {
        if (app_bt_stream_sco_trigger_btpcm_tick()) {
            af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
            }
            af_stream_dma_tc_irq_disable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
            }

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
            af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
            if(adma_ch != HAL_DMA_CHAN_NONE)
            {
                bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
            }
#endif
            TRACE_AUD_STREAM_I("[SCO][BTPCMRX] buf:%p,capture_buf_btpcm:%p",buf,capture_buf_btpcm);

            // uint16_t *source=(uint16_t *)buf;
            // DUMP16("%02x,", source, MSBC_FRAME_LEN);

            mobile_master_clk_offset_init=mobile_master_clk%BUF_BTCLK_NUM;
            last_mobile_master_clk = mobile_master_clk;
            app_bt_stream_sco_trigger_codecpcm_start(mobile_master_clk,mobile_master_cnt);
            is_codec_stream_started = true;

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
            TRACE_AUD_STREAM_I("[SCO][BTPCMRX] btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif
        }
    } else {
#if defined(SCO_DMA_SNAPSHOT_DEBUG)
#if defined(MULTI_DMA_TC)
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(&btclk, &btcnt,adma_ch&0xFF, dma_base);
        }
#else
        bt_drv_reg_op_dma_tc_clkcnt_get(&btclk, &btcnt);
#endif
        bt_sco_mobile_clkcnt_get(btclk, btcnt,&mobile_master_clk, &mobile_master_cnt);
        TRACE_AUD_STREAM_I("[SCO][BTPCMRX]:btclk:%d,btcnt:%d,",mobile_master_clk,mobile_master_cnt);
#endif
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][BTPCMRX] takes %d us",
            FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;

#else
#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    if(!is_sco_mode()){
        TRACE_AUD_STREAM_E("[SCO][BTPCMRX] player exit!");
        memset(buf,0x0,len);
        return len;
    }

#if defined(TX_RX_PCM_MASK)
    TRACE_AUD_STREAM_I("[SCO][BTPCMRX] TX_RX_PCM_MASK");
    CQueue* Rx_esco_queue_temp = NULL;
    Rx_esco_queue_temp = get_rx_esco_queue_ptr();
    if(bt_sco_codec_is_msbc() && btdrv_is_pcm_mask_enable() ==1 )
    {
        memset(buf,0,len);
        int status = 0;
        len /= 2;
        uint8_t rx_data[len];
        status = DeCQueue(Rx_esco_queue_temp,rx_data,len);
        for(uint32_t i = 0; i<len; i++)
        {
            buf[2*i+1] = rx_data[i];
        }
        len*=2;
        if(status)
        {
            TRACE_AUD_STREAM_E("[SCO][BTPCMRX] Rx Dec Fail");
        }
    }
#endif

    if (is_codec_stream_started == false) {
        if (bt_sco_codec_is_cvsd())
            hal_sys_timer_delay_us(3000);

        TRACE_AUD_STREAM_I("[SCO][BTPCMRX] start codec %d", FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(SPEECH_BONE_SENSOR)
#if defined(AF_ADC_I2S_SYNC)
        hal_i2s_enable_delay(HAL_I2S_ID_0);
        hal_codec_capture_enable_delay();
#endif
        speech_bone_sensor_start();
#endif

#if defined(AF_ADC_I2S_SYNC)
        codec_capture_i2s_enable();
#endif
        is_codec_stream_started = true;

        return len;
    }
    store_voicebtpcm_m2p_buffer(buf, len);

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][BTPCMRX] takes %d us",
            FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}
#endif

#if defined(__BT_ANC__)&&(!defined(SCO_DMA_SNAPSHOT))
static void bt_anc_sco_down_sample_16bits(int16_t *dst, int16_t *src, uint32_t dst_cnt)
{
    for (uint32_t i = 0; i < dst_cnt; i++) {
        dst[i] = src[i * bt_sco_upsampling_ratio * sco_play_chan_num];
    }
}
#endif

static void bt_sco_codec_playback_data_post_handler(uint8_t *buf, uint32_t len, void *cfg)
{
    POSSIBLY_UNUSED struct AF_STREAM_CONFIG_T *config = (struct AF_STREAM_CONFIG_T *)cfg;
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    uint8_t device_id = app_bt_audio_get_curr_sco_device();
#ifdef TWS_PROMPT_SYNC
    tws_playback_ticks_check_for_mix_prompt(device_id);
#endif
    if (audio_prompt_is_playing_ongoing())
    {
        audio_prompt_processing_handler(device_id, len, buf);
    }
#else
    app_ring_merge_more_data(buf, len);
#endif

#ifdef ANC_ASSIST_USE_INT_CODEC
    codec_play_merge_pilot_data(buf, len, cfg);
#endif
}

#if defined(HIGH_EFFICIENCY_TX_PWR_CTRL)
#define APP_SCO_LOWER_RSSI_THRESHOLD  (-60)
#define APP_SCO_UPPER_RSSI_THRESHOLD  (-45)

static bool isScoStreamingSwitchedToLowPowerTxGain = true;

void bt_sco_reset_tx_gain_mode(bool sco_on)
{
    isScoStreamingSwitchedToLowPowerTxGain = true;

    app_bt_reset_rssi_collector();

    if(sco_on)
    {
        //sco on set to low power mode
        bt_drv_rf_high_efficency_tx_pwr_ctrl(true, true);
    }
    else
    {
        //sco off set to default value
        bt_drv_rf_high_efficency_tx_pwr_ctrl(false, true);
    }
}

void bt_sco_tx_gain_mode_ajdustment_handler(void)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());

    if (!curr_device)
        return;

    rx_agc_t mobile_agc;
    bool ret = bt_drv_reg_op_read_rssi_in_dbm(curr_device->acl_conn_hdl, &mobile_agc);
    if (ret)
    {
        int32_t average_rssi = app_bt_tx_rssi_analyzer(mobile_agc.rssi);
        //TRACE(0, "S:    %d  db:  %d  db", average_rssi, mobile_agc.rssi);
        if ((average_rssi < APP_SCO_LOWER_RSSI_THRESHOLD) && isScoStreamingSwitchedToLowPowerTxGain)
        {
            TRACE(0, "switch to normal mode tx gain as rssi is %d", average_rssi);
            isScoStreamingSwitchedToLowPowerTxGain = false;

            bt_drv_rf_high_efficency_tx_pwr_ctrl(false, false);
        }
        else if ((average_rssi >= APP_SCO_UPPER_RSSI_THRESHOLD) && !isScoStreamingSwitchedToLowPowerTxGain)
        {
            TRACE(0, "switch to low power mode tx gain as rssi is %d", average_rssi);
            isScoStreamingSwitchedToLowPowerTxGain = true;

            bt_drv_rf_high_efficency_tx_pwr_ctrl(true, true);
        }
    }
}
#endif

static uint32_t bt_sco_codec_playback_data(uint8_t *buf, uint32_t len)
{
    app_bt_stream_trigger_checker_handler(TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK);
    if(sco_switch_trigger_check && sco_switch_trigger_check() == true)
    {
        TRACE(1,"%s switch ok",__func__);
    }

    if (!bt_is_playback_triggered())
    {
        bt_set_playback_triggered(true);
    }

#ifdef BT_XTAL_SYNC
#ifdef BT_XTAL_SYNC_NEW_METHOD
#ifdef IBRT
    bool valid = false;
    uint32_t bitoffset = 0;
    POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = app_bt_get_device(app_bt_audio_get_curr_sco_device());
    if (APP_IBRT_MOBILE_LINK_CONNECTED(&curr_device->remote))
    {
        valid = true;
        bitoffset = btdrv_rf_bitoffset_get(APP_IBRT_UI_GET_MOBILE_CONNHANDLE(&curr_device->remote) -0x80);
    }
    else if (APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(&curr_device->remote))
    {
        valid = true;
        bitoffset = btdrv_rf_bitoffset_get(APP_IBRT_UI_GET_IBRT_HANDLE(&curr_device->remote) -0x80);
    }

    if(valid)
    {
        bt_xtal_sync_new(bitoffset,app_if_need_fix_target_rxbit(),BT_XTAL_SYNC_MODE_WITH_MOBILE);
    }
#endif
#else
    bt_xtal_sync(BT_XTAL_SYNC_MODE_VOICE);
#endif
#endif

#if defined(SCO_DMA_SNAPSHOT)
    //processing  ping pang flag
    int pingpang;

    if(buf==playback_buf_codecpcm)
    {
        pingpang=0;
    }
    else
    {
        pingpang=1;
    }
#ifdef TX_RX_PCM_MASK
    //processing btpcm.(It must be from CPU's copy )
    if(btdrv_is_pcm_mask_enable()==1&&bt_sco_codec_is_msbc())
    {
        uint32_t lock;
        uint32_t i;
        //must lock the interrupts when exchanging data.
        lock = int_lock();
        uint16_t *capture_dst=(uint16_t *)(capture_buf_btpcm + pingpang * capture_size_btpcm / 2);

        for( i =0; i<capture_size_btpcm/4; i++)
        {
            capture_dst[i]=(uint16_t)capture_buf_btpcm_copy[i]<<8;
        }
        int_unlock(lock);
    }
#endif

#if defined(HIGH_EFFICIENCY_TX_PWR_CTRL)
    bt_sco_tx_gain_mode_ajdustment_handler();
#endif

    //processing spk
    uint8_t *playbakce_pcm_frame_p = playback_buf_codecpcm + pingpang * playback_size_codecpcm / 2;
    uint8_t *source = capture_buf_btpcm + pingpang * capture_size_btpcm / 2;
    uint32_t source_len=playback_size_btpcm / 2;

    plc_monitor_dump_data(source, source_len);

#ifdef SCO_DUMP_BTPCM_DATA
    // TRACE(2, "[%s] len:%d", __func__, source_len);
    for (uint32_t i=0; i<source_len/120; i++) {
        DUMP16("%04x,", source + 120 * i, 60);
        hal_sys_timer_delay_us(100);
    }
#endif

#if defined(PCM_PRIVATE_DATA_FLAG)
    {
    	uint32_t i;
        for (i = 0; i < source_len/BTPCM_TOTAL_DATA_LENGTH; i++)
        {
		uint8_t *dst=source+i*BTPCM_PUBLIC_DATA_LENGTH;
		uint8_t *src=source+i*BTPCM_TOTAL_DATA_LENGTH+BTPCM_PRIVATE_DATA_LENGTH;
        	memmove(dst,src,BTPCM_PUBLIC_DATA_LENGTH);
        }
	source_len=source_len-BTPCM_PRIVATE_DATA_LENGTH*i;
    }
#endif

    if(sco_btpcm_mute_flag == 1 || sco_disconnect_mute_flag == 1) {
        for(uint32_t i =0; i < source_len; i++) {
            source[i]=MUTE_PATTERN;
        }
        TRACE_AUD_STREAM_I("[SCO][SPK]mute....................");
    } else {
        sco_btpcm_mute_flag=1;
    }

    if (bt_sco_codec_is_cvsd() == false) {
        uint16_t *source_u16 = (uint16_t *)source;
        for (uint32_t i = 0; i < source_len/2; i++) {
            source[i] = (source_u16[i] >> 8);
        }
        source_len >>= 1;
    }
    process_downlink_bt_voice_frames(source, source_len, playbakce_pcm_frame_p,
                                    playback_size_codecpcm / sco_play_chan_num / 2);

    if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
        // Convert mono data to stereo data
#if defined(SPEECH_RX_24BIT)
        app_bt_stream_copy_track_one_to_two_24bits((int32_t *)playbakce_pcm_frame_p,
                                (int32_t *)playbakce_pcm_frame_p,
                                playback_size_codecpcm / 2 / sco_play_chan_num / sizeof(int32_t));
#else
        app_bt_stream_copy_track_one_to_two_16bits((int16_t *)playbakce_pcm_frame_p,
                                (int16_t *)playbakce_pcm_frame_p,
                                playback_size_codecpcm / 2 / sco_play_chan_num / sizeof(int16_t));
#endif
    }
    return len;
#else
#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t start_ticks = hal_fast_sys_timer_get();
#endif

    uint8_t *dec_buf;
    uint32_t mono_len;

#if defined(SPEECH_RX_24BIT)
    len /= 2;
#endif

#ifdef __BT_ANC__
    mono_len = len / sco_play_chan_num / bt_sco_upsampling_ratio;
    dec_buf = bt_anc_sco_dec_buf;
#else
    mono_len = len / sco_play_chan_num;
    dec_buf = buf;
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
    if(hal_cmu_get_audio_resample_status())
    {
        if (sco_play_irq_cnt < SCO_PLAY_RESAMPLE_ALIGN_CNT) {
            sco_play_irq_cnt++;
        }
        if (sco_dma_buf_err || af_stream_buffer_error(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK)) {
            sco_dma_buf_err = false;
            sco_play_irq_cnt = 0;
            app_resample_reset(sco_playback_resample);
            app_resample_reset(sco_capture_resample);
            voicebtpcm_pcm_echo_buf_queue_reset();
            TRACE_AUD_STREAM_I("[SCO][SPK]: DMA buffer error: reset resample");
        }
        app_playback_resample_run(sco_playback_resample, dec_buf, mono_len);
    }
    else
#endif
    {
#ifdef __BT_ANC__
        bt_anc_sco_down_sample_16bits((int16_t *)dec_buf, (int16_t *)buf, mono_len / 2);
#else
        if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
            // Convert stereo data to mono data (to save into echo_buf)
            app_bt_stream_copy_track_two_to_one_16bits((int16_t *)dec_buf, (int16_t *)buf, mono_len / 2);
        }
#endif
        voicebtpcm_pcm_audio_more_data(dec_buf, mono_len);
    }

#ifdef __BT_ANC__
    voicebtpcm_pcm_resample((int16_t *)dec_buf, mono_len / 2, (int16_t *)buf);
#endif

#if defined(SPEECH_RX_24BIT)
    len <<= 1;
#endif

    if (sco_play_chan_num == AUD_CHANNEL_NUM_2) {
        // Convert mono data to stereo data
#if defined(SPEECH_RX_24BIT)
        app_bt_stream_copy_track_one_to_two_24bits((int32_t *)buf, (int32_t *)buf, len / 2 / sizeof(int32_t));
#else
        app_bt_stream_copy_track_one_to_two_16bits((int16_t *)buf, (int16_t *)buf, len / 2 / sizeof(int16_t));
#endif
    }

    if (spk_force_mute)
    {
        memset(buf, 0, len);
    }

#if defined(BT_SCO_HANDLER_PROFILE)
    uint32_t end_ticks = hal_fast_sys_timer_get();
    TRACE_AUD_STREAM_I("[SCO][SPK] takes %d us",
        FAST_TICKS_TO_US(end_ticks - start_ticks));
#endif

    return len;
#endif
}

int bt_sco_player_forcemute(bool mic_mute, bool spk_mute)
{
    mic_force_mute = mic_mute;
    spk_force_mute = spk_mute;
    return 0;
}

int bt_sco_player_get_codetype(void)
{
    if (gStreamplayer & APP_BT_STREAM_HFP_PCM)
    {
        // NOTE: Be careful of this function during sco multi-ponit
        return app_audio_manager_get_scocodecid();
    }
    else
    {
        return 0;
    }
}

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
static uint32_t audio_mc_data_playback_sco(uint8_t *buf, uint32_t mc_len_bytes)
{
  // uint32_t begin_time;
   //uint32_t end_time;
  // begin_time = hal_sys_timer_get();
  // TRACE_AUD_STREAM_I("phone cancel: %d",begin_time);

   float left_gain;
   float right_gain;
   int32_t playback_len_bytes,mc_len_bytes_8;
   int32_t i,j,k;
   int delay_sample;

   mc_len_bytes_8=mc_len_bytes/8;

   hal_codec_get_dac_gain(&left_gain,&right_gain);

   TRACE_AUD_STREAM_I("[SCO][SPK][MC] playback_samplerate_ratio:  %d,ch:%d,sample_size:%d.",playback_samplerate_ratio_bt,playback_ch_num_bt,sample_size_play_bt);
   TRACE_AUD_STREAM_I("[SCO][SPK][MC] len:  %d",mc_len_bytes);

  // TRACE_AUD_STREAM_I("left_gain:  %d",(int)(left_gain*(1<<12)));
  // TRACE_AUD_STREAM_I("right_gain: %d",(int)(right_gain*(1<<12)));

   playback_len_bytes=mc_len_bytes/playback_samplerate_ratio_bt;

    if (sample_size_play_bt == AUD_BITS_16)
    {
        int16_t *sour_p=(int16_t *)(playback_buf_bt+playback_size_bt/2);
        int16_t *mid_p=(int16_t *)(buf);
        int16_t *mid_p_8=(int16_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int16_t *dest_p=(int16_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int16_t *)playback_buf_bt;
        }

        if(playback_ch_num_bt==AUD_CHANNEL_NUM_2)
        {
            delay_sample=DELAY_SAMPLE_MC;

            for(i=0,j=0;i<delay_sample;i=i+2)
            {
                mid_p[j++]=delay_buf_bt[i];
                mid_p[j++]=delay_buf_bt[i+1];
            }

            for(i=0;i<playback_len_bytes/2-delay_sample;i=i+2)
            {
                mid_p[j++]=sour_p[i];
                mid_p[j++]=sour_p[i+1];
            }

            for(j=0;i<playback_len_bytes/2;i=i+2)
            {
                delay_buf_bt[j++]=sour_p[i];
                delay_buf_bt[j++]=sour_p[i+1];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+2*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+2)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                        mid_p_8[j++]=mid_p[i+1];
                    }
                }
            }

            anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_16);

            for(i=0,j=0;i<(mc_len_bytes_8)/2;i=i+2)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                    dest_p[j++]=mid_p_8[i+1];
                }
            }

        }
        else if(playback_ch_num_bt==AUD_CHANNEL_NUM_1)
        {
            delay_sample=DELAY_SAMPLE_MC/2;

            for(i=0,j=0;i<delay_sample;i=i+1)
            {
                mid_p[j++]=delay_buf_bt[i];
            }

            for(i=0;i<playback_len_bytes/2-delay_sample;i=i+1)
            {
                mid_p[j++]=sour_p[i];
            }

            for(j=0;i<playback_len_bytes/2;i=i+1)
            {
                delay_buf_bt[j++]=sour_p[i];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+1*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/2;i=i+1)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                    }
                }
            }

            anc_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,AUD_BITS_16);

            for(i=0,j=0;i<(mc_len_bytes_8)/2;i=i+1)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                }
            }
        }

    }
    else if (sample_size_play_bt == AUD_BITS_24)
    {
        int32_t *sour_p=(int32_t *)(playback_buf_bt+playback_size_bt/2);
        int32_t *mid_p=(int32_t *)(buf);
        int32_t *mid_p_8=(int32_t *)(buf+mc_len_bytes-mc_len_bytes_8);
        int32_t *dest_p=(int32_t *)buf;

        if(buf == playback_buf_mc)
        {
            sour_p=(int32_t *)playback_buf_bt;
        }

        if(playback_ch_num_bt==AUD_CHANNEL_NUM_2)
        {
            delay_sample=DELAY_SAMPLE_MC;

            for(i=0,j=0;i<delay_sample;i=i+2)
            {
                mid_p[j++]=delay_buf_bt[i];
                mid_p[j++]=delay_buf_bt[i+1];
            }

            for(i=0;i<playback_len_bytes/4-delay_sample;i=i+2)
            {
                mid_p[j++]=sour_p[i];
                mid_p[j++]=sour_p[i+1];
            }

            for(j=0;i<playback_len_bytes/4;i=i+2)
            {
                delay_buf_bt[j++]=sour_p[i];
                delay_buf_bt[j++]=sour_p[i+1];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+2*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                    mid_p_8[j++]=mid_p[i+1];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+2)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                        mid_p_8[j++]=mid_p[i+1];
                    }
                }
            }

            anc_mc_run_stereo((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,right_gain,AUD_BITS_24);

            for(i=0,j=0;i<(mc_len_bytes_8)/4;i=i+2)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                    dest_p[j++]=mid_p_8[i+1];
                }
            }

        }
        else if(playback_ch_num_bt==AUD_CHANNEL_NUM_1)
        {
            delay_sample=DELAY_SAMPLE_MC/2;

            for(i=0,j=0;i<delay_sample;i=i+1)
            {
                mid_p[j++]=delay_buf_bt[i];
            }

            for(i=0;i<playback_len_bytes/4-delay_sample;i=i+1)
            {
                mid_p[j++]=sour_p[i];
            }

            for(j=0;i<playback_len_bytes/4;i=i+1)
            {
                delay_buf_bt[j++]=sour_p[i];
            }

            if(playback_samplerate_ratio_bt<=8)
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+1*(8/playback_samplerate_ratio_bt))
                {
                    mid_p_8[j++]=mid_p[i];
                }
            }
            else
            {
                for(i=0,j=0;i<playback_len_bytes/4;i=i+1)
                {
                    for(k=0;k<playback_samplerate_ratio_bt/8;k++)
                    {
                        mid_p_8[j++]=mid_p[i];
                    }
                }
            }

            anc_mc_run_mono((uint8_t *)mid_p_8,mc_len_bytes_8,left_gain,AUD_BITS_24);

            for(i=0,j=0;i<(mc_len_bytes_8)/4;i=i+1)
            {
                for(k=0;k<8;k++)
                {
                    dest_p[j++]=mid_p_8[i];
                }
            }
        }

    }

  //  end_time = hal_sys_timer_get();

 //   TRACE_AUD_STREAM_I("[SCO][SPK][MC]:run time: %d", end_time-begin_time);

    return 0;
}
#endif

#if defined(LOW_DELAY_SCO)
int speech_get_frame_size(int fs, int ch, int ms)
{
    return (fs / 1000 * ch * ms)/2;
}
#else
int speech_get_frame_size(int fs, int ch, int ms)
{
    return (fs / 1000 * ch * ms);
}
#endif


int speech_get_codecpcm_buf_len(int fs, int ch, int ms)
{
    return speech_get_frame_size(fs, ch, ms) * 2 * 2;
}

static int speech_get_btpcm_buf_len(hfp_sco_codec_t sco_codec, int fs,int ms)
{
    //RAW size per ms(Bytes).
    int buf_len=(fs/1000)*ms*2;

    //after encoding.
    switch(sco_codec) {
    case BT_HFP_SCO_CODEC_CVSD:
        buf_len=buf_len/2;
        break;
    case BT_HFP_SCO_CODEC_MSBC:
        buf_len=buf_len/4;
        break;
    default:
        ASSERT(0, "sco codec is not correct!");
        break;
    }

    //after BES platform.
    buf_len=buf_len*2;

    //add private data.
#if defined(PCM_PRIVATE_DATA_FLAG)
    buf_len=buf_len+BTPCM_PRIVATE_DATA_LENGTH*((int)(ms/7.5f));
#endif

    //after pingpang
    buf_len=buf_len*2;

    //if low delay. we will use half buffer(should be 7.5ms);
#if defined(LOW_DELAY_SCO)
    buf_len=buf_len/2;
#endif

    TRACE_AUD_STREAM_I("speech_get_btpcm_buf_len:%d", buf_len);

    return buf_len;
}



static enum AUD_SAMPRATE_T speech_sco_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

    auto codec_get_type = bt_sco_codec_get_type();
    if (codec_get_type == BT_HFP_SCO_CODEC_CVSD) {
        sample_rate = AUD_SAMPRATE_8000;
    } else if(codec_get_type == BT_HFP_SCO_CODEC_LC3_SWB){
        sample_rate = AUD_SAMPRATE_32000;
    } else{
        sample_rate = AUD_SAMPRATE_16000;
    }

    return sample_rate;
}

static POSSIBLY_UNUSED enum AUD_SAMPRATE_T speech_vqe_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

#if defined(SPEECH_VQE_FIXED_SAMPLE_RATE)
    sample_rate = (enum AUD_SAMPRATE_T)SPEECH_VQE_FIXED_SAMPLE_RATE;
#else
    auto codec_get_type = bt_sco_codec_get_type();
    if (codec_get_type == BT_HFP_SCO_CODEC_CVSD) {
        sample_rate = AUD_SAMPRATE_8000;
    } else if(codec_get_type == BT_HFP_SCO_CODEC_LC3_SWB) {
        sample_rate = AUD_SAMPRATE_32000;
    } else {
        sample_rate = AUD_SAMPRATE_16000;
    }
#endif

    return sample_rate;
}

enum AUD_SAMPRATE_T speech_codec_get_sample_rate(void)
{
    enum AUD_SAMPRATE_T sample_rate;

#if defined(SPEECH_CODEC_FIXED_SAMPLE_RATE)
    sample_rate = (enum AUD_SAMPRATE_T)SPEECH_CODEC_FIXED_SAMPLE_RATE;
#else
    if (bt_sco_codec_is_cvsd()) {
        sample_rate = AUD_SAMPRATE_8000;
    } else if(bt_sco_codec_is_lc3()) {
        sample_rate = AUD_SAMPRATE_32000;
    } else { 
        sample_rate = AUD_SAMPRATE_16000;
    }  
#endif

    return sample_rate;
}

int app_bt_stream_volumeset(int8_t vol);

enum AUD_SAMPRATE_T sco_sample_rate;

#if defined(AF_ADC_I2S_SYNC)
void bt_sco_bt_trigger_callback(void)
{
    TRACE_AUD_STREAM_I("[SCO][IIS] Start...", __func__);

    hal_i2s_enable(HAL_I2S_ID_0);
}
#endif

extern void bt_drv_reg_op_pcm_set(uint8_t en);
extern uint8_t bt_drv_reg_op_pcm_get();

static enum APP_SYSFREQ_FREQ_T sco_player_reselect_freq(enum APP_SYSFREQ_FREQ_T old_freq)
{
    enum APP_SYSFREQ_FREQ_T freq = old_freq;

    if (freq < APP_SYSFREQ_104M) {
        freq = APP_SYSFREQ_104M;
    }

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    if (freq < APP_SYSFREQ_208M) {
        freq = APP_SYSFREQ_208M;
    }
#endif

#if defined(SCO_CP_ACCEL)
    freq = APP_SYSFREQ_52M;
#endif

#if defined(SPEECH_TX_THIRDPARTY)
    freq = APP_SYSFREQ_104M;
#endif

#if defined(IBRT)
    if (app_bt_audio_count_straming_mobile_links() > 1)
    {
        if (freq < APP_SYSFREQ_78M)
        {
            freq = APP_SYSFREQ_78M;
        }
    }
#endif
    return freq;
}

static void app_multipoint_timer_handler(void const *param);
osTimerDef (APP_MULTIPOINT_TIMER, app_multipoint_timer_handler);
osTimerId app_multipoint_timer_id = NULL;
bool isInMultipointTimer = false;
static void app_multipoint_timer_handler(void const *param)
{
    TRACE(0, "%s", __func__);
    isInMultipointTimer = false;
}

void app_start_multipoint_timer_handler(void)
{
    if (NULL == app_multipoint_timer_id)
    {
        app_multipoint_timer_id =
            osTimerCreate(osTimer(APP_MULTIPOINT_TIMER), osTimerOnce, NULL);
    }
    TRACE(0, "%s", __func__);
    isInMultipointTimer = true;
    osTimerStart(app_multipoint_timer_id, 1500);
}

bool app_is_in_multipoint_timer(void)
{
    return isInMultipointTimer;
}

static int bt_sco_player(bool on, enum APP_SYSFREQ_FREQ_T freq)
{
    struct AF_STREAM_CONFIG_T stream_cfg;
    static bool isRun =  false;
    uint8_t * bt_audio_buff = NULL;
    enum AUD_SAMPRATE_T sample_rate;
    uint8_t POSSIBLY_UNUSED adma_ch = HAL_DMA_CHAN_NONE;
    uint32_t POSSIBLY_UNUSED dma_base;

    TRACE_AUD_STREAM_I("[SCO_PLAYER] work:%d op:%d freq:%d", isRun, on, freq);

#ifdef CHIP_BEST2000
    btdrv_enable_one_packet_more_head(0);
#endif

    if (isRun==on)
        return 0;

    if (on)
    {  
#ifdef CODEC_VCM_CHECK
        vcm_sta = 0;
#endif
        bt_set_playback_triggered(false);
        af_set_irq_notification(app_bt_stream_sco_irq_notification);
#ifdef  IS_MULTI_AI_ENABLED
        app_ai_open_mic_user_set(AI_OPEN_MIC_USER_SCO);
#endif
        bt_sco_codec_store();
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        audio_prompt_stop_playing();
#endif
#if defined(__IAG_BLE_INCLUDE__)
        bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_SCO,
                                       USER_ALL,
                                       BLE_ADVERTISING_INTERVAL);
#endif
#ifdef TX_RX_PCM_MASK
        if (btdrv_is_pcm_mask_enable() ==1 && bt_sco_codec_is_msbc())
        {
            bt_drv_reg_op_pcm_set(1);
            TRACE_AUD_STREAM_I("[SCO_PLAYER] PCM MASK");
        }
#endif

#if defined(PCM_FAST_MODE)
        btdrv_open_pcm_fast_mode_enable();
#ifdef PCM_PRIVATE_DATA_FLAG
        bt_drv_reg_op_set_pcm_flag();
#endif
#endif

#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_STOP, 0);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_MIC_OPEN, 0);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO3,THIRDPARTY_STOP, 0);
#ifdef __AMA_VOICE__
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS,THIRDPARTY_CALL_START, AI_SPEC_AMA);
#else
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS,THIRDPARTY_CALL_START, 0);
#endif
#ifdef __BIXBY
        //TODO: use the thirdparty generic interface.
        app_bixby_on_suspend_handle();
#endif
#endif
        //bt_syncerr set to max(0x0a)
//        BTDIGITAL_REG_SET_FIELD(REG_BTCORE_BASE_ADDR, 0x0f, 0, 0x0f);
//        af_set_priority(AF_USER_SCO, osPriorityRealtime);
        af_set_priority(AF_USER_SCO, osPriorityHigh);
        bt_media_volume_ptr_update_by_mediatype(BT_STREAM_VOICE);
        stream_local_volume = btdevice_volume_p->hfp_vol;
        app_audio_manager_sco_status_checker();

        freq = sco_player_reselect_freq(freq);
        freq = APP_SYSFREQ_USER_BT_SCO_MASTER;
        app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, freq);

#if defined(ENABLE_CALCU_CPU_FREQ_LOG)
        TRACE_AUD_STREAM_I("[SCO_PLAYER] sysfreq calc[%d]: %d\n", freq, hal_sys_timer_calc_cpu_freq(5, 0));
#endif

#if defined(VPU_CFG_ON_SENSOR_HUB)
#if defined(LOW_DELAY_SCO)
        speech_bone_sensor_open(16000, 120);
#else
        speech_bone_sensor_open(16000, 240);
#endif
#endif

#if !defined(FPGA) || defined(FPGA_BUILD_IN_FLASH)
        app_overlay_select(APP_OVERLAY_HFP);
#endif

#if defined(SPEECH_ALGO_DSP)
        speech_enable_mcpp(true);
        MCPP_CLI_INIT_CFG_T init_cfg = {0};
        init_cfg.capture_enable = true;
        init_cfg.playback_enable = true;
#if defined(SPEECH_ALGO_DSP_M55)
        init_cfg.capture_server  = APP_MCPP_CORE_M55;
        init_cfg.playback_server = APP_MCPP_CORE_M55;
#elif defined(SPEECH_ALGO_DSP_HIFI)
        init_cfg.capture_server  = APP_MCPP_CORE_HIFI;
        init_cfg.playback_server = APP_MCPP_CORE_HIFI;
#elif defined(SPEECH_ALGO_DSP_SENS)
        init_cfg.capture_server  = APP_MCPP_CORE_SENS;
        init_cfg.playback_server = APP_MCPP_CORE_SENS;
#elif defined(SPEECH_ALGO_DSP_BTH)
        init_cfg.capture_server  = APP_MCPP_CORE_BTH;
        init_cfg.playback_server = APP_MCPP_CORE_BTH;
#endif
        mcpp_cli_init(APP_MCPP_USER_CALL, &init_cfg);
#endif

        sample_rate = speech_codec_get_sample_rate();

#if !defined(SCO_DMA_SNAPSHOT)
        int aec_frame_len = speech_get_frame_size(sample_rate, 1, SPEECH_SCO_FRAME_MS);
        speech_tx_aec_set_frame_len(aec_frame_len);
#endif

        bt_sco_player_forcemute(false, false);

        bt_sco_mode = 1;

#if defined(ANC_ASSIST_ENABLED)
#if defined(VOICE_ASSIST_FF_FIR_LMS)
        app_voice_assist_fir_lms_enable_fir(false);
#endif
        app_anc_assist_set_playback_info(sample_rate);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_PHONE_CALL);
#endif
        app_audio_mempool_init_with_specific_size(app_audio_mempool_original_size());

#ifndef _SCO_BTPCM_CHANNEL_
        memset(&hf_sendbuff_ctrl, 0, sizeof(hf_sendbuff_ctrl));
#endif

        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)SPEECH_CODEC_CAPTURE_CHANNEL_NUM;

#if defined(FPGA)
        sco_cap_chan_num = AUD_CHANNEL_NUM_2;
#endif

#if defined(SPEECH_TX_AEC_CODEC_REF)
        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)(sco_cap_chan_num + 1);
#endif

        uint32_t sco_algo_chan_num = sco_cap_chan_num;

#if defined(ANC_NOISE_TRACKER)
        sco_cap_chan_num = (enum AUD_CHANNEL_NUM_T)(sco_cap_chan_num + ANC_NOISE_TRACKER_CHANNEL_NUM);
#endif

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_disable();
#endif

#if defined(CODEC_DAC_MULTI_VOLUME_TABLE)
        hal_codec_set_dac_volume_table(codec_dac_hfp_vol, ARRAY_SIZE(codec_dac_hfp_vol));
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        // codec:mic
#if defined(ANC_ASSIST_ENABLED)
        stream_cfg.channel_map  = (enum AUD_CHANNEL_MAP_T)app_anc_assist_get_mic_ch_map(AUD_INPUT_PATH_MAINMIC);
        sco_cap_chan_num        = (enum AUD_CHANNEL_NUM_T)app_anc_assist_get_mic_ch_num(AUD_INPUT_PATH_MAINMIC);
#if defined(ASSIST_LOW_RAM_MOD) && !defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
        anc_assist_resample_inst = integer_resampling_create_with_custom_allocator(app_anc_assist_get_frame_len(), sco_cap_chan_num, INTEGER_RESAMPLING_ITEM_32K_TO_16K, pool_allocator());
        app_audio_mempool_get_buff((uint8_t **)&anc_assist_resample_buf, speech_get_codecpcm_buf_len(sample_rate / 2, sco_cap_chan_num, SPEECH_SCO_FRAME_MS));
        memset(anc_assist_resample_buf, 0, speech_get_codecpcm_buf_len(sample_rate / 2, sco_cap_chan_num, SPEECH_SCO_FRAME_MS));
#endif
#endif
        stream_cfg.io_path = AUD_INPUT_PATH_MAINMIC;
        stream_cfg.channel_num = sco_cap_chan_num;
        stream_cfg.data_size = speech_get_codecpcm_buf_len(sample_rate, stream_cfg.channel_num, SPEECH_SCO_FRAME_MS);

#if defined(__AUDIO_RESAMPLE__) && defined(NO_SCO_RESAMPLE)
        // When __AUDIO_RESAMPLE__ is defined,
        // resample is off by default on best1000, and on by default on other platforms
#ifndef CHIP_BEST1000
        hal_cmu_audio_resample_disable();
#endif
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        if (sample_rate == AUD_SAMPRATE_8000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_8463;
        }
        else if (sample_rate == AUD_SAMPRATE_16000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_16927;
        }
#ifdef RESAMPLE_ANY_SAMPLE_RATE
        sco_capture_resample = app_capture_resample_any_open( stream_cfg.channel_num,
                            bt_sco_capture_resample_iter, stream_cfg.data_size / 2,
                            (float)CODEC_FREQ_26M / CODEC_FREQ_24P576M);
#else
        sco_capture_resample = app_capture_resample_open(sample_rate, stream_cfg.channel_num,
                            bt_sco_capture_resample_iter, stream_cfg.data_size / 2);
#endif
        uint32_t mono_cap_samp_cnt = stream_cfg.data_size / 2 / 2 / stream_cfg.channel_num;
        uint32_t cap_irq_cnt_per_frm = ((mono_cap_samp_cnt * stream_cfg.sample_rate + (sample_rate - 1)) / sample_rate +
            (aec_frame_len - 1)) / aec_frame_len;
        if (cap_irq_cnt_per_frm == 0) {
            cap_irq_cnt_per_frm = 1;
        }
#else
        stream_cfg.sample_rate = sample_rate;
#endif

#if defined(SPEECH_TX_24BIT)
        stream_cfg.bits = AUD_BITS_24;
        stream_cfg.data_size *= 2;
#else
        stream_cfg.bits = AUD_BITS_16;
#endif
        stream_cfg.vol = stream_local_volume;

#ifdef FPGA
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#endif
        stream_cfg.handler = bt_sco_codec_capture_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

#if defined(SCO_DMA_SNAPSHOT)
        capture_buf_codecpcm=stream_cfg.data_ptr;
        capture_size_codecpcm=stream_cfg.data_size;
#endif

        TRACE_AUD_STREAM_I("[SCO_PLAYER] Capture: sample_rate: %d, data_size: %d, chan_num: %d",
                        stream_cfg.sample_rate, stream_cfg.data_size, stream_cfg.channel_num);

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);

#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_CAPTURE, A2DP_SCO_TRIGGER_CHANNEL);
#endif

#if defined(HW_DC_FILTER_WITH_IIR)
        hw_filter_codec_iir_st = hw_filter_codec_iir_create(stream_cfg.sample_rate,
                                stream_cfg.channel_num, stream_cfg.bits, &adc_iir_cfg);
#endif

        // codec:spk
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)0;
#if defined(CHIP_BEST1000)
        sco_play_chan_num = AUD_CHANNEL_NUM_2;
#else
#ifdef PLAYBACK_USE_I2S
        sco_play_chan_num = AUD_CHANNEL_NUM_2;
#else
        sco_play_chan_num = AUD_CHANNEL_NUM_1;
#endif
#endif

        stream_cfg.channel_num = sco_play_chan_num;
        // stream_cfg.data_size = BT_AUDIO_SCO_BUFF_SIZE * stream_cfg.channel_num;
        stream_cfg.data_size = speech_get_codecpcm_buf_len(sample_rate, sco_play_chan_num, SPEECH_SCO_FRAME_MS);
#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        if (sample_rate == AUD_SAMPRATE_8000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_8463;
        }
        else if (sample_rate == AUD_SAMPRATE_16000)
        {
            stream_cfg.sample_rate = AUD_SAMPRATE_16927;
        }
#ifdef RESAMPLE_ANY_SAMPLE_RATE
        sco_playback_resample = app_playback_resample_any_open( AUD_CHANNEL_NUM_1,
                            bt_sco_playback_resample_iter, stream_cfg.data_size / stream_cfg.channel_num / 2,
                            (float)CODEC_FREQ_24P576M / CODEC_FREQ_26M);
#else
        sco_playback_resample = app_playback_resample_open(sample_rate, AUD_CHANNEL_NUM_1,
                            bt_sco_playback_resample_iter, stream_cfg.data_size / stream_cfg.channel_num / 2);
#endif
        sco_play_irq_cnt = 0;
        sco_dma_buf_err = false;

        uint32_t mono_play_samp_cnt = stream_cfg.data_size / 2 / 2 / stream_cfg.channel_num;
        uint32_t play_irq_cnt_per_frm = ((mono_play_samp_cnt * stream_cfg.sample_rate + (sample_rate - 1)) / sample_rate +
            (aec_frame_len - 1)) / aec_frame_len;
        if (play_irq_cnt_per_frm == 0) {
            play_irq_cnt_per_frm = 1;
        }
        uint32_t play_samp_cnt_per_frm = mono_play_samp_cnt * play_irq_cnt_per_frm;
        uint32_t cap_samp_cnt_per_frm = mono_cap_samp_cnt * cap_irq_cnt_per_frm;
        uint32_t max_samp_cnt_per_frm = (play_samp_cnt_per_frm >= cap_samp_cnt_per_frm) ? play_samp_cnt_per_frm : cap_samp_cnt_per_frm;
        uint32_t echo_q_samp_cnt = (((max_samp_cnt_per_frm + mono_play_samp_cnt * SCO_PLAY_RESAMPLE_ALIGN_CNT) *
            // convert to 8K/16K sample cnt
             sample_rate +(stream_cfg.sample_rate - 1)) / stream_cfg.sample_rate +
            // aligned with aec_frame_len
            (aec_frame_len - 1)) / aec_frame_len * aec_frame_len;
        if (echo_q_samp_cnt == 0) {
            echo_q_samp_cnt = aec_frame_len;
        }
        voicebtpcm_pcm_echo_buf_queue_init(echo_q_samp_cnt * 2);
#else
        stream_cfg.sample_rate = speech_sco_get_sample_rate();
        stream_cfg.data_size = speech_get_codecpcm_buf_len(stream_cfg.sample_rate, sco_play_chan_num, SPEECH_SCO_FRAME_MS);
#endif

#ifdef __BT_ANC__
        // Mono channel decoder buffer (8K or 16K sample rate)
        app_audio_mempool_get_buff(&bt_anc_sco_dec_buf, stream_cfg.data_size / 2 / sco_play_chan_num);
        // The playback size for the actual sample rate
        bt_sco_upsampling_ratio = 6/(sample_rate/AUD_SAMPRATE_8000);
        stream_cfg.data_size *= bt_sco_upsampling_ratio;
#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = AUD_SAMPRATE_48000;
#endif
        //damic_init();
        //init_amic_dc_bt();
        //ds_fir_init();
        us_fir_init(bt_sco_upsampling_ratio);
#endif
        stream_cfg.bits = AUD_BITS_16;
#ifdef PLAYBACK_USE_I2S
        stream_cfg.device = AUD_STREAM_USE_I2S0_MASTER;
        stream_cfg.io_path = AUD_IO_PATH_NULL;
#else
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
#endif
        stream_cfg.handler = bt_sco_codec_playback_data;

#if defined(SPEECH_RX_24BIT)
        stream_cfg.bits = AUD_BITS_24;
        stream_cfg.data_size *= 2;
#endif

#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        uint8_t* promptTmpSourcePcmDataBuf;
        uint8_t* promptTmpTargetPcmDataBuf;
        uint8_t* promptPcmDataBuf;
        uint8_t* promptResamplerBuf;

        sco_sample_rate = stream_cfg.sample_rate;
        app_audio_mempool_get_buff(&promptTmpSourcePcmDataBuf, AUDIO_PROMPT_SOURCE_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptTmpTargetPcmDataBuf, AUDIO_PROMPT_TARGET_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptPcmDataBuf, AUDIO_PROMPT_PCM_BUFFER_SIZE);
        app_audio_mempool_get_buff(&promptResamplerBuf, AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);

        audio_prompt_buffer_config(MIX_WITH_SCO_STREAMING,
                                   stream_cfg.channel_num,
                                   stream_cfg.bits,
                                   promptTmpSourcePcmDataBuf,
                                   promptTmpTargetPcmDataBuf,
                                   promptPcmDataBuf,
                                   AUDIO_PROMPT_PCM_BUFFER_SIZE,
                                   promptResamplerBuf,
                                   AUDIO_PROMPT_BUF_SIZE_FOR_RESAMPLER);
#endif

        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

        TRACE_AUD_STREAM_I("[SCO_PLAYER] playback sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;

#ifdef __BT_ANC__
        playback_samplerate_ratio_bt=8;
#else
        if (sample_rate_play_bt == AUD_SAMPRATE_8000)
        {
            playback_samplerate_ratio_bt=8*3*2;
        }
        else if (sample_rate_play_bt == AUD_SAMPRATE_16000)
        {
            playback_samplerate_ratio_bt=8*3;
        }
#endif

        playback_ch_num_bt=stream_cfg.channel_num;
#endif

#if defined(SCO_DMA_SNAPSHOT)
        playback_buf_codecpcm=stream_cfg.data_ptr;
        playback_size_codecpcm=stream_cfg.data_size;
        playback_samplerate_codecpcm=stream_cfg.sample_rate;
#endif

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_PLAYBACK, A2DP_SCO_TRIGGER_CHANNEL);
#endif

        af_stream_dma_tc_irq_enable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
        }

        af_codec_set_playback_post_handler(bt_sco_codec_playback_data_post_handler);

#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_sco;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_PLAYBACK, A2DP_SCO_TRIGGER_CHANNEL);
#endif
#endif
#if defined(SCO_DMA_SNAPSHOT)
        btdrv_disable_playback_triggler();
#ifdef PLAYBACK_USE_I2S
        af_i2s_sync_config(AUD_STREAM_PLAYBACK, AF_I2S_SYNC_TYPE_BT, true);
#else
        af_codec_sync_config(AUD_STREAM_PLAYBACK, AF_CODEC_SYNC_TYPE_BT, true);
#endif
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_PLAYBACK, 0);
        af_codec_sync_config(AUD_STREAM_CAPTURE, AF_CODEC_SYNC_TYPE_BT, true);
        af_codec_set_device_bt_sync_source(AUD_STREAM_USE_INT_CODEC, AUD_STREAM_CAPTURE, 0);
#endif

#if defined(SPEECH_BONE_SENSOR)
        // NOTE: Use VMIC to power on VPU, So must call lis25ba_init after af_stream_start(AUD_STREAM_CAPTURE);
#if !defined(VPU_CFG_ON_SENSOR_HUB)
#if defined(LOW_DELAY_SCO)
        speech_bone_sensor_open(16000, 120);
#else
        speech_bone_sensor_open(16000, 240);
#endif
#endif

#if defined(AF_ADC_I2S_SYNC)
        hal_i2s_enable_delay(HAL_I2S_ID_0);
#elif defined(HW_I2S_TDM_TRIGGER)
        af_i2s_sync_config(AUD_STREAM_CAPTURE, AF_I2S_SYNC_TYPE_BT, true);
#else
        ASSERT(0, "Need to define AF_ADC_I2S_SYNC or HW_I2S_TDM_TRIGGER");
#endif
        speech_bone_sensor_start();

#if defined(AF_ADC_I2S_SYNC)
        af_codec_bt_trigger_config(true, bt_sco_bt_trigger_callback);
#endif
#endif

#if defined(ANC_NOISE_TRACKER)
        app_audio_mempool_get_buff(
            (uint8_t **)&anc_buf,
            speech_get_frame_size(
                sample_rate,
                ANC_NOISE_TRACKER_CHANNEL_NUM,
                SPEECH_SCO_FRAME_MS) * sizeof(int16_t));
        noise_tracker_init(nt_demo_words_cb, ANC_NOISE_TRACKER_CHANNEL_NUM, -20);
#endif

        // Must call this function before af_stream_start
        // Get all free app audio buffer except SCO_BTPCM used(2k)
        enum AUD_SAMPRATE_T cur_sco_sample_rate;
        cur_sco_sample_rate = speech_sco_get_sample_rate();
#if defined(SCO_DMA_SNAPSHOT)
        voicebtpcm_pcm_audio_init(cur_sco_sample_rate, speech_vqe_get_sample_rate(),
                    cur_sco_sample_rate, sample_rate, cur_sco_sample_rate, sco_algo_chan_num);
#else
        voicebtpcm_pcm_audio_init(cur_sco_sample_rate, sample_rate);
#endif

        /*
        TRACE_AUD_STREAM_I("[SCO_PLAYER] start codec %d", FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        */

#ifdef SPEECH_SIDETONE
#ifdef HW_SIDETONE_IIR_PROCESS
        {
            HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac=NULL;
            uint32_t sample_rate_hw_sidetone_iir;

            sidetone_open();
            hal_codec_sidetone_enable();
#ifdef __AUDIO_RESAMPLE__
            sample_rate_hw_sidetone_iir = hal_codec_get_real_sample_rate(cur_sco_sample_rate,1);
            TRACE(3,"sco sample rate:%d, sample_rate_hw_sidetone_iir: %d.",
                            cur_sco_sample_rate, sample_rate_hw_sidetone_iir);
#else
            sample_rate_hw_sidetone_iir=cur_sco_sample_rate;
#endif
            hw_iir_cfg_dac = hw_codec_iir_get_cfg((enum AUD_SAMPRATE_T)sample_rate_hw_sidetone_iir,
                                                    audio_sidetone_iir_cfg_list[0]);
            sidetone_set_cfg(hw_iir_cfg_dac);
        }
#else
        hal_codec_sidetone_enable();
#endif
#endif

#ifdef _SCO_BTPCM_CHANNEL_
        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.sample_rate = cur_sco_sample_rate;
        stream_cfg.channel_num = AUD_CHANNEL_NUM_1;
        // stream_cfg.data_size = BT_AUDIO_SCO_BUFF_SIZE * stream_cfg.channel_num;

        if (bt_sco_codec_is_cvsd()) {
            stream_cfg.data_size = speech_get_btpcm_buf_len(BT_HFP_SCO_CODEC_CVSD,
                                            stream_cfg.sample_rate, SPEECH_SCO_FRAME_MS);
        } else {
            stream_cfg.data_size = speech_get_btpcm_buf_len(BT_HFP_SCO_CODEC_MSBC,
                                            stream_cfg.sample_rate, SPEECH_SCO_FRAME_MS);
        }

        // btpcm:rx
        stream_cfg.device = AUD_STREAM_USE_BT_PCM;
        stream_cfg.handler = bt_sco_btpcm_capture_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

        TRACE_AUD_STREAM_I("[SCO_PLAYER] sco btpcm sample_rate:%d, data_size:%d",stream_cfg.sample_rate,stream_cfg.data_size);

#if defined(SCO_DMA_SNAPSHOT)
        sco_btpcm_mute_flag=0;
        sco_disconnect_mute_flag=0;

        capture_buf_btpcm=stream_cfg.data_ptr;
        capture_size_btpcm=stream_cfg.data_size;
#ifdef TX_RX_PCM_MASK
        capture_size_btpcm_copy=stream_cfg.data_size/4;//only need ping or pang;
        app_audio_mempool_get_buff(&capture_buf_btpcm_copy, capture_size_btpcm_copy);
#endif
#endif
        af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE, &stream_cfg);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_CAPTURE, A2DP_SCO_TRIGGER_CHANNEL);
#endif
        // btpcm:tx
        stream_cfg.device = AUD_STREAM_USE_BT_PCM;
        stream_cfg.handler = bt_sco_btpcm_playback_data;
        app_audio_mempool_get_buff(&bt_audio_buff, stream_cfg.data_size);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);

#if defined(SCO_DMA_SNAPSHOT)
        playback_buf_btpcm=stream_cfg.data_ptr;
        playback_size_btpcm=stream_cfg.data_size;
#ifdef TX_RX_PCM_MASK
        playback_size_btpcm_copy=stream_cfg.data_size/4; //only need ping or pang;
        app_audio_mempool_get_buff(&playback_buf_btpcm_copy, playback_size_btpcm_copy);
#endif
         //only need ping or pang;
        app_audio_mempool_get_buff(&playback_buf_btpcm_cache, stream_cfg.data_size/2);
#endif

        af_stream_open(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK, &stream_cfg);
#if BLE_AUDIO_ENABLED
        af_codec_set_device_bt_sync_source(stream_cfg.device, AUD_STREAM_PLAYBACK, A2DP_SCO_TRIGGER_CHANNEL);
#endif
#if defined(SCO_DMA_SNAPSHOT)
        app_bt_stream_sco_trigger_btpcm_start();
        af_stream_dma_tc_irq_enable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_enable_dma_tc(adma_ch&0xFF, dma_base);
        }
#endif

#if defined(LOW_DELAY_SCO)
    btpcm_int_counter=1;
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_NUM, 0);
#endif

        TRACE_AUD_STREAM_I("[SCO_PLAYER] start btpcm %d", FAST_TICKS_TO_US(hal_fast_sys_timer_get()));
#if defined(CHIP_BEST1501) || defined(CHIP_BEST2003) || defined(CHIP_BEST1600)
        btdrv_pcm_disable();
#endif

        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);

        af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
        af_stream_start(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);

        btdrv_set_btpcm_mute_pattern(MSBC_MUTE_PATTERN | (MSBC_MUTE_PATTERN << 8));

#if defined(CVSD_BYPASS)
        btdrv_cvsd_bypass_enable();
#endif

#ifdef IBRT
        app_bt_enable_bt_pcm(true);
#endif
        is_codec_stream_started = false;

#if defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402) || defined(CHIP_BEST2001)
#if !defined(SCO_DMA_SNAPSHOT)
        btdrv_pcm_enable();
#endif
#endif

#endif

#ifdef FPGA
        app_bt_stream_volumeset(stream_local_volume);
#endif

#if defined(CALL_BYPASS_SLAVE_TX_PROCESS)
        extern int bt_sco_chain_set_master_role(bool is_master);
        bool is_master = 1;
#ifdef IBRT
        if (app_ibrt_if_get_ui_role() == TWS_UI_SLAVE)
        {
            is_master = 0;
        }
#endif
        bt_sco_chain_set_master_role(is_master);
#endif

        app_bt_stream_trigger_checker_start();

#ifdef AUDIO_RMS_MONITOR_ENABLE
        rms_debug_cnt = 256;
#endif
        TRACE_AUD_STREAM_I("[SCO_PLAYER] on");
    }
    else
    {
#ifdef  IS_MULTI_AI_ENABLED
        app_ai_open_mic_user_set(AI_OPEN_MIC_USER_NONE);
#endif
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
        bool isToClearActiveMedia= audio_prompt_clear_pending_stream(PENDING_TO_STOP_SCO_STREAMING);
        if (isToClearActiveMedia)
        {
            uint8_t device_id = app_bt_audio_get_curr_sco_device();
            bt_media_clear_media_type(BT_STREAM_VOICE, (enum BT_DEVICE_ID_T)device_id);
        }
#endif
        app_bt_stream_trigger_checker_stop();
#if defined(SCO_DMA_SNAPSHOT)
#ifdef TX_RX_PCM_MASK
        playback_buf_btpcm_copy=NULL;
        capture_buf_btpcm_copy=NULL;
        playback_size_btpcm_copy=0;
        capture_size_btpcm_copy=0;
#endif
#endif

#ifdef __THIRDPARTY
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO2,THIRDPARTY_MIC_CLOSE, 0);
#endif
#if defined(SCO_DMA_SNAPSHOT)
        app_bt_stream_sco_trigger_codecpcm_stop();
#endif

        if (bt_is_playback_triggered())
        {
#ifdef AF_STREAM_ID_0_PLAYBACK_FADEOUT
        // fadeout should be called before af_codec_set_playback_post_handler
        // as pilot tone is merged in playback post handler
#if defined(ANC_ASSIST_USE_INT_CODEC)
            af_stream_playback_fadeout(AUD_STREAM_ID_0, 50);
#else
            af_stream_playback_fadeout(AUD_STREAM_ID_0, 10);
#endif
#endif
        }

        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }
        af_codec_set_playback_post_handler(NULL);

#if defined(SCO_DMA_SNAPSHOT_DEBUG)
        af_stream_dma_tc_irq_disable(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        adma_ch = af_stream_get_dma_chan(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        dma_base = af_stream_get_dma_base_addr(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        if(adma_ch != HAL_DMA_CHAN_NONE)
        {
            bt_drv_reg_op_disable_dma_tc(adma_ch&0xFF, dma_base);
        }
#endif

#ifdef SPEECH_SIDETONE
        hal_codec_sidetone_disable();
#ifdef HW_SIDETONE_IIR_PROCESS
        sidetone_close();
        sidetone_opened=0;
#endif
#endif

        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_stop(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif
        is_codec_stream_started = false;

#if defined(SPEECH_BONE_SENSOR)
        speech_bone_sensor_stop();
#endif

#if !(defined(__AUDIO_RESAMPLE__) && defined(SW_PLAYBACK_RESAMPLE))
        af_codec_tune(AUD_STREAM_NUM, 0);
#endif

#ifdef _SCO_BTPCM_CHANNEL_
        af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);

        af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_1, AUD_STREAM_PLAYBACK);
#ifdef IBRT
        app_bt_enable_bt_pcm(false);
#endif
#endif
#ifdef TX_RX_PCM_MASK
        if (btdrv_is_pcm_mask_enable()==1 && bt_drv_reg_op_pcm_get())
        {
            bt_drv_reg_op_pcm_set(0);
            TRACE_AUD_STREAM_I("[SCO_PLAYER] PCM UNMASK");
        }
#endif
#if defined(PCM_FAST_MODE)
        btdrv_open_pcm_fast_mode_disable();
#endif
        bt_drv_reg_op_sco_txfifo_reset(bt_sco_codec_get_type());
#if defined(HW_DC_FILTER_WITH_IIR)
        hw_filter_codec_iir_destroy(hw_filter_codec_iir_st);
#endif

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);
#if defined(AUDIO_ANC_FB_MC_SCO) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_close(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

#if defined(SPEECH_BONE_SENSOR)
        speech_bone_sensor_close();
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(SW_SCO_RESAMPLE)
        app_capture_resample_close(sco_capture_resample);
        sco_capture_resample = NULL;
        app_capture_resample_close(sco_playback_resample);
        sco_playback_resample = NULL;
#endif

#if defined(__AUDIO_RESAMPLE__) && defined(NO_SCO_RESAMPLE)
#ifndef CHIP_BEST1000
        // When __AUDIO_RESAMPLE__ is defined,
        // resample is off by default on best1000, and on by default on other platforms
        hal_cmu_audio_resample_enable();
#endif
#endif

#if defined(CODEC_DAC_MULTI_VOLUME_TABLE)
        hal_codec_set_dac_volume_table(NULL, 0);
#endif

#ifdef PLAYBACK_USE_I2S
        hal_cmu_audio_resample_enable();
#endif

#ifdef __BT_ANC__
        bt_anc_sco_dec_buf = NULL;
        //damic_deinit();
        //app_cap_thread_stop();
#endif
        voicebtpcm_pcm_audio_deinit();

#ifndef FPGA
#ifdef BT_XTAL_SYNC
        bt_term_xtal_sync(false);
        bt_term_xtal_sync_default();
#endif
#endif

#if defined(__IAG_BLE_INCLUDE__)
        bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_SCO,
                                       USER_ALL,
                                       BLE_ADV_INVALID_INTERVAL);
#endif

        TRACE_AUD_STREAM_I("[SCO_PLAYER] off");
        bt_sco_mode = 0;

#if defined(IBRT)
#if defined(IBRT_UI)
    if ((app_ibrt_conn_support_max_mobile_dev() == 2) &&
        (btif_me_get_mobile_link_num() > 1))
    {
        app_start_multipoint_timer_handler();
    }
#endif
#endif

        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_BT_SCO, APP_SYSFREQ_32K);
        af_set_priority(AF_USER_SCO, osPriorityAboveNormal);

#if defined(ANC_ASSIST_ENABLED)
#if defined(AUDIO_ADAPTIVE_EQ)
        app_voice_assist_adaptive_eq_close();
#endif
        app_anc_assist_set_playback_info(AUD_SAMPRATE_NULL);
        app_anc_assist_set_mode(ANC_ASSIST_MODE_STANDALONE);
#if defined(VOICE_ASSIST_FF_FIR_LMS)
        app_voice_assist_fir_lms_enable_fir(true);
#endif
#endif

        //bt_syncerr set to default(0x07)
//       BTDIGITAL_REG_SET_FIELD(REG_BTCORE_BASE_ADDR, 0x0f, 0, 0x07);
#ifdef __THIRDPARTY
        //app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO1,THIRDPARTY_START);
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_NO3,THIRDPARTY_START, 0);
#ifdef __AMA_VOICE__
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS,THIRDPARTY_CALL_STOP, AI_SPEC_AMA);
#else
        app_thirdparty_specific_lib_event_handle(THIRDPARTY_FUNC_KWS,THIRDPARTY_CALL_STOP, 0);
#endif
#ifdef __BIXBY
        app_bixby_on_resume_handle();
#endif
#endif

#ifdef AUDIO_RMS_MONITOR_ENABLE
        rms_debug_cnt = 0xFFFF;
#endif
        af_set_irq_notification(NULL);
        bt_set_playback_triggered(false);
    }

#if defined(HIGH_EFFICIENCY_TX_PWR_CTRL)
    bt_sco_reset_tx_gain_mode(on);
#endif

    isRun=on;
    return 0;
}

#ifdef AUDIO_LINEIN
#include "app_status_ind.h"
//player channel should <= capture channel number
//player must be 2 channel
#define LINEIN_PLAYER_CHANNEL (2)
#ifdef __AUDIO_OUTPUT_MONO_MODE__
#define LINEIN_CAPTURE_CHANNEL (1)
#else
#define LINEIN_CAPTURE_CHANNEL (2)
#endif

#if (LINEIN_CAPTURE_CHANNEL == 1)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE/2)
#elif (LINEIN_CAPTURE_CHANNEL == 2)
#define LINEIN_PLAYER_BUFFER_SIZE (1024*LINEIN_PLAYER_CHANNEL)
#define LINEIN_CAPTURE_BUFFER_SIZE (LINEIN_PLAYER_BUFFER_SIZE)
#endif

int8_t app_linein_buffer_is_empty(void)
{
    if (app_audio_pcmbuff_length()){
        return 0;
    }else{
        return 1;
    }
}

uint32_t app_linein_pcm_come(uint8_t * pcm_buf, uint32_t len)
{
    app_audio_pcmbuff_put(pcm_buf, len);

    return len;
}

uint32_t app_linein_need_pcm_data(uint8_t* pcm_buf, uint32_t len)
{
#if (LINEIN_CAPTURE_CHANNEL == 1)
    app_audio_pcmbuff_get((uint8_t *)app_linein_play_cache, len/2);
    app_play_audio_lineinmode_more_data((uint8_t *)app_linein_play_cache,len/2);
    app_bt_stream_copy_track_one_to_two_16bits((int16_t *)pcm_buf, app_linein_play_cache, len/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
    app_audio_pcmbuff_get((uint8_t *)pcm_buf, len);
    app_play_audio_lineinmode_more_data((uint8_t *)pcm_buf, len);
#endif

#if defined(__AUDIO_OUTPUT_MONO_MODE__)
    merge_stereo_to_mono_16bits((int16_t *)buf, (int16_t *)pcm_buf, len/2);
#endif

#ifdef ANC_APP
    bt_audio_updata_eq_for_anc(app_anc_work_status());
#else
    bt_audio_updata_eq(app_audio_get_eq());
#endif

    audio_process_run(pcm_buf, len);

    return len;
}

int app_play_linein_onoff(bool onoff)
{
    static bool isRun =  false;
    uint8_t *linein_audio_cap_buff = 0;
    uint8_t *linein_audio_play_buff = 0;
    uint8_t *linein_audio_loop_buf = NULL;
    struct AF_STREAM_CONFIG_T stream_cfg;

    uint8_t POSSIBLY_UNUSED *bt_eq_buff = NULL;
    uint32_t POSSIBLY_UNUSED eq_buff_size;
    uint8_t POSSIBLY_UNUSED play_samp_size;

    TRACE_AUD_STREAM_I("[LINEIN_PLAYER] work:%d op:%d", isRun, onoff);

    if (isRun == onoff)
        return 0;

    if (onoff) {
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_104M);
        app_overlay_select(APP_OVERLAY_A2DP);
        app_audio_mempool_init_with_specific_size(app_audio_mempool_size());
        app_audio_mempool_get_buff(&linein_audio_cap_buff, LINEIN_CAPTURE_BUFFER_SIZE);
        app_audio_mempool_get_buff(&linein_audio_play_buff, LINEIN_PLAYER_BUFFER_SIZE);
        app_audio_mempool_get_buff(&linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);
        app_audio_pcmbuff_init(linein_audio_loop_buf, LINEIN_PLAYER_BUFFER_SIZE<<2);

#if (LINEIN_CAPTURE_CHANNEL == 1)
        app_audio_mempool_get_buff((uint8_t **)&app_linein_play_cache, LINEIN_PLAYER_BUFFER_SIZE/2/2);
        app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2/2);
#elif (LINEIN_CAPTURE_CHANNEL == 2)
        app_play_audio_lineinmode_init(LINEIN_CAPTURE_CHANNEL, LINEIN_PLAYER_BUFFER_SIZE/2);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_PLAYER_CHANNEL;
#if defined(__AUDIO_RESAMPLE__)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = AUD_SAMPRATE_44100;
#endif
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.vol = stream_linein_volume;
        TRACE_AUD_STREAM_I("[LINEIN_PLAYER] vol = %d",stream_linein_volume);
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;
        stream_cfg.handler = app_linein_need_pcm_data;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_play_buff);
        stream_cfg.data_size = LINEIN_PLAYER_BUFFER_SIZE;

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        sample_size_play_bt=stream_cfg.bits;
        sample_rate_play_bt=stream_cfg.sample_rate;
        data_size_play_bt=stream_cfg.data_size;
        playback_buf_bt=stream_cfg.data_ptr;
        playback_size_bt=stream_cfg.data_size;
        if(sample_rate_play_bt==AUD_SAMPRATE_96000)
        {
            playback_samplerate_ratio_bt=4;
        }
        else
        {
            playback_samplerate_ratio_bt=8;
        }
        playback_ch_num_bt=stream_cfg.channel_num;
        mid_p_8_old_l=0;
        mid_p_8_old_r=0;
#endif

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg);

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size*2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)

        play_samp_size = (stream_cfg.bits <= AUD_BITS_16) ? 2 : 4;
#if defined(CHIP_BEST2000)
        eq_buff_size = stream_cfg.data_size * sizeof(int32_t) / play_samp_size;
#elif  defined(CHIP_BEST1000)
        eq_buff_size = stream_cfg.data_size * sizeof(int16_t) / play_samp_size;
#elif defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST2300A)
        eq_buff_size = stream_cfg.data_size;
#endif

#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
        eq_buff_size = stream_cfg.data_size;
#else
        eq_buff_size = 0;
        bt_eq_buff = NULL;
#endif

        if(eq_buff_size>0)
        {
            app_audio_mempool_get_buff(&bt_eq_buff, eq_buff_size);
        }

#if defined(IBRT)&& !defined(FREEMAN_ENABLED_STERO)
        enum AUD_CHANNEL_NUM_T sw_ch_num = AUD_CHANNEL_NUM_1;
#else
        enum AUD_CHANNEL_NUM_T sw_ch_num = stream_cfg.channel_num;
#endif

        audio_process_open(stream_cfg.sample_rate, stream_cfg.bits, sw_ch_num, stream_cfg.channel_num, stream_cfg.data_size/stream_cfg.channel_num/(stream_cfg.bits <= AUD_BITS_16 ? 2 : 4)/2, bt_eq_buff, eq_buff_size);

#ifdef __SW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_SW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_SW_IIR,0));
#endif

#ifdef __HW_FIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_FIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_FIR,0));
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_DAC_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_DAC_IIR,0));
#endif

#ifdef __HW_IIR_EQ_PROCESS__
        bt_audio_set_eq(AUDIO_EQ_TYPE_HW_IIR,bt_audio_get_eq_index(AUDIO_EQ_TYPE_HW_IIR,0));
#endif

#ifdef ANC_APP
        anc_status_record = 0xff;
#endif

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        uint8_t* bt_audio_buff = NULL;
        stream_cfg.bits = sample_size_play_bt;
        stream_cfg.channel_num = playback_ch_num_bt;
        stream_cfg.sample_rate = sample_rate_play_bt;
        stream_cfg.device = AUD_STREAM_USE_MC;
        stream_cfg.vol = 0;
        stream_cfg.handler = audio_mc_data_playback_a2dp;
        stream_cfg.io_path = AUD_OUTPUT_PATH_SPEAKER;

        app_audio_mempool_get_buff(&bt_audio_buff, data_size_play_bt*playback_samplerate_ratio_bt);
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(bt_audio_buff);
        stream_cfg.data_size = data_size_play_bt*playback_samplerate_ratio_bt;

        playback_buf_mc=stream_cfg.data_ptr;
        playback_size_mc=stream_cfg.data_size;

        anc_mc_run_init(hal_codec_anc_convert_rate(sample_rate_play_bt));

        memset(delay_buf_bt,0,sizeof(delay_buf_bt));

        af_stream_open(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg);
        //ASSERT(ret == 0, "af_stream_open playback failed: %d", ret);
#endif


        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

#if defined(AUDIO_ANC_FB_MC) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
        af_stream_start(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK);
#endif

        memset(&stream_cfg, 0, sizeof(stream_cfg));

        stream_cfg.bits = AUD_BITS_16;
#if defined(__AUDIO_RESAMPLE__)
        stream_cfg.sample_rate = AUD_SAMPRATE_50781;
#else
        stream_cfg.sample_rate = AUD_SAMPRATE_44100;
#endif
#if FPGA==0
        stream_cfg.device = AUD_STREAM_USE_INT_CODEC;
#else
        stream_cfg.device = AUD_STREAM_USE_EXT_CODEC;
#endif
        stream_cfg.io_path = AUD_INPUT_PATH_LINEIN;
        stream_cfg.channel_num = (enum AUD_CHANNEL_NUM_T)LINEIN_CAPTURE_CHANNEL;
        stream_cfg.channel_map = (enum AUD_CHANNEL_MAP_T)hal_codec_get_input_path_cfg(stream_cfg.io_path);
        stream_cfg.handler = app_linein_pcm_come;
        stream_cfg.data_ptr = BT_AUDIO_CACHE_2_UNCACHE(linein_audio_cap_buff);
        stream_cfg.data_size = LINEIN_CAPTURE_BUFFER_SIZE;

        af_stream_open(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE, &stream_cfg);
        af_stream_start(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
     }else     {
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_stop(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        audio_process_close();

        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_CAPTURE);
        af_stream_close(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK);

        app_overlay_unloadall();
        app_sysfreq_req(APP_SYSFREQ_USER_APP_0, APP_SYSFREQ_32K);
     }

    isRun = onoff;
    TRACE_AUD_STREAM_I("[LINEIN_PLAYER] end!\n");
    return 0;
}
#endif

#if defined(APP_AUDIO_DELAY_AFTER_OPEN_FOR_TRIGGER)
static int app_bt_stream_trigger_delay(uint16_t player)
{
    uint32_t delay = 100;

    switch (player) {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            delay = 100;
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            delay = 500;
            break;
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        case APP_PLAY_BACK_AUDIO:
            delay = 100;
            break;
#endif
        default:
            delay = 100;
    }

    osDelay(delay);

    return 0;
}
#endif

int app_bt_stream_open(APP_AUDIO_STATUS* status)
{
    int nRet = -1;
    uint16_t player = status->id;
    APP_AUDIO_STATUS next_status = {0};
    enum APP_SYSFREQ_FREQ_T freq = (enum APP_SYSFREQ_FREQ_T)status->freq;

    TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] prev:0x%x%s freq:%d", gStreamplayer, player2str(gStreamplayer), freq);
    TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] cur:0x%x%s freq:%d", player, player2str(player), freq);

    APP_AUDIO_STATUS streamToClose = {0};

    if (gStreamplayer != APP_BT_STREAM_INVALID)
    {
        if (gStreamplayer & player)
        {
            TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] 0x%x%s has opened", player, player2str(player));
            return -1;
        }

        if (player >= APP_BT_STREAM_BORDER_INDEX)
        {
            if (APP_BT_INPUT_STREAM_INDEX(gStreamplayer) > 0)
            {
                TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] close 0x%x%s prev opening", gStreamplayer, player2str(gStreamplayer));
            }
        }
#if  defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
        else if ((player == APP_PLAY_BACK_AUDIO) &&
            (((PROMPT_ID_FROM_ID_VALUE(status->aud_id) != AUDIO_ID_BT_MUTE) ||
                app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))))
        {
            // Do nothing for following case
            // 1. the prompt is not mute prompt 2. call is on-going
        }
#endif
        else
        {
            if (APP_BT_OUTPUT_STREAM_INDEX(gStreamplayer) > 0)
            {
                TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] close 0x%x%s prev opening", gStreamplayer, player2str(gStreamplayer));
                uint16_t player2close = APP_BT_OUTPUT_STREAM_INDEX(gStreamplayer);
                nRet = app_bt_stream_close(player2close);
                if (nRet)
                {
                    return -1;
                }
                else
                {
                #if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
                    if (APP_PLAY_BACK_AUDIO&player2close)
                    {
                        app_prompt_inform_completed_event();
                    }
                #endif
                    if (((APP_BT_STREAM_A2DP_SBC == player2close) || (APP_BT_STREAM_A2DP_AAC == player2close)) &&
                        (APP_PLAY_BACK_AUDIO == player) &&
                        (PROMPT_ID_FROM_ID_VALUE(status->aud_id) == AUDIO_ID_BT_MUTE))
                    {
                    }
                    else
                    {
                        streamToClose.id = player2close;
                        app_audio_list_rmv_callback(&streamToClose, &next_status, APP_BT_SETTING_Q_POS_TAIL);
                    }
                }
            }
        }
    }

    switch (player)
    {
    case APP_BT_STREAM_HFP_PCM:
    case APP_BT_STREAM_HFP_CVSD:
    case APP_BT_STREAM_HFP_VENDOR:
        nRet = bt_sco_player(true, freq);
        break;
    case APP_BT_STREAM_A2DP_SBC:
    case APP_BT_STREAM_A2DP_AAC:
    case APP_BT_STREAM_A2DP_VENDOR:
        nRet = bt_sbc_player(PLAYER_OPER_START, freq);
        break;
#ifdef __FACTORY_MODE_SUPPORT__
    case APP_FACTORYMODE_AUDIO_LOOP:
        nRet = app_factorymode_audioloop(true, freq);
        break;
#endif
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
    case APP_PLAY_BACK_AUDIO:
        nRet = app_play_audio_onoff(true, status);
        break;
#endif

#ifdef RB_CODEC
    case APP_BT_STREAM_RBCODEC:
        nRet = app_rbplay_audio_onoff(true, 0);
        break;
#endif

#ifdef AUDIO_LINEIN
    case APP_PLAY_LINEIN_AUDIO:
        nRet = app_play_linein_onoff(true);
        break;
#endif

#ifdef AUDIO_PCM_PLAYER
    case APP_PLAY_PCM_PLAYER:
        nRet = audio_pcm_player_onoff(true);
        break;
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)
    case APP_A2DP_SOURCE_LINEIN_AUDIO:
        nRet = app_a2dp_source_linein_on(true);
        break;
#endif

#if defined(APP_I2S_A2DP_SOURCE)
    case APP_A2DP_SOURCE_I2S_AUDIO:
        nRet = app_a2dp_source_I2S_onoff(true);
        break;
#endif

#if defined(APP_USB_A2DP_SOURCE)
    case APP_A2DP_SOURCE_USB_AUDIO:
#ifdef BT_USB_AUDIO_DUAL_MODE
        btusb_switch(BTUSB_MODE_USB);
#endif
        break;
#endif

#ifdef __AI_VOICE__
    case APP_BT_STREAM_AI_VOICE:
        nRet = app_ai_voice_start_mic_stream();
        break;
#endif
    default:
        nRet = -1;
        break;
    }

    if (!nRet)
    {
        gStreamplayer |= player;
        TRACE_AUD_STREAM_I("[STRM_PLAYER][OPEN] updated to  0x%x%s", gStreamplayer, player2str(gStreamplayer));
#if defined(APP_AUDIO_DELAY_AFTER_OPEN_FOR_TRIGGER)
        // Add extra delay to resolve trigger conflict
        app_bt_stream_trigger_delay(player);
#endif
    }
    return nRet;
}

int app_bt_stream_close(uint16_t player)
{
    int nRet = -1;
    TRACE_AUD_STREAM_I("[STRM_PLAYER][CLOSE] gStreamplayer: 0x%x%s", gStreamplayer, player2str(gStreamplayer));
    TRACE_AUD_STREAM_I("[STRM_PLAYER][CLOSE] player:0x%x%s", player, player2str(player));

    if ((gStreamplayer & player) != player)
    {
        return -1;
    }

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            nRet = bt_sco_player(false, APP_SYSFREQ_32K);
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            nRet = bt_sbc_player(PLAYER_OPER_STOP, APP_SYSFREQ_32K);
            break;
#ifdef __FACTORY_MODE_SUPPORT__
        case APP_FACTORYMODE_AUDIO_LOOP:
            nRet = app_factorymode_audioloop(false, APP_SYSFREQ_32K);
            break;
#endif
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        case APP_PLAY_BACK_AUDIO:
            nRet = app_play_audio_onoff(false, NULL);
            break;
#endif
#ifdef RB_CODEC
        case APP_BT_STREAM_RBCODEC:
            nRet = app_rbplay_audio_onoff(false, 0);
            break;
#endif

#ifdef AUDIO_LINEIN
        case APP_PLAY_LINEIN_AUDIO:
            nRet = app_play_linein_onoff(false);
            break;
#endif

#ifdef AUDIO_PCM_PLAYER
        case APP_PLAY_PCM_PLAYER:
            nRet = audio_pcm_player_onoff(false);
            break;
#endif

#if defined(APP_LINEIN_A2DP_SOURCE)
        case APP_A2DP_SOURCE_LINEIN_AUDIO:
            nRet = app_a2dp_source_linein_on(false);
            break;
#endif

#if defined(APP_I2S_A2DP_SOURCE)
        case APP_A2DP_SOURCE_I2S_AUDIO:
            nRet = app_a2dp_source_I2S_onoff(false);
            break;
#endif

#if defined(APP_USB_A2DP_SOURCE)
        case APP_A2DP_SOURCE_USB_AUDIO:
            btusb_switch(BTUSB_MODE_BT);
            break;
#endif

#ifdef __AI_VOICE__
        case APP_BT_STREAM_AI_VOICE:
            nRet = app_ai_voice_stop_mic_stream();
            break;
#endif
        default:
            nRet = -1;
            break;
    }
    if (!nRet)
    {
        gStreamplayer &= (~player);
        TRACE_AUD_STREAM_I("[STRM_PLAYER][CLOSE] updated to 0x%x%s", gStreamplayer, player2str(gStreamplayer));
    }
    return nRet;
}

int app_bt_stream_setup(uint16_t player, uint8_t status)
{
    int nRet = -1;

    TRACE_AUD_STREAM_I("[STRM_PLAYER][SETUP] prev:%d%s sample:%d", gStreamplayer, player2str(gStreamplayer), status);
    TRACE_AUD_STREAM_I("[STRM_PLAYER][SETUP] cur:%d%s sample:%d", player, player2str(player), status);

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            bt_sbc_player_setup(status);
            break;
        default:
            nRet = -1;
            break;
    }

    return nRet;
}

int app_bt_stream_restart(APP_AUDIO_STATUS* status)
{
    int nRet = -1;
    uint16_t player = status->id;
    enum APP_SYSFREQ_FREQ_T freq = (enum APP_SYSFREQ_FREQ_T)status->freq;

    TRACE_AUD_STREAM_I("[STRM_PLAYER][RESTART] prev:%d%s freq:%d", gStreamplayer, player2str(gStreamplayer), freq);
    TRACE_AUD_STREAM_I("[STRM_PLAYER][RESTART] cur:%d%s freq:%d", player, player2str(player), freq);

    if ((gStreamplayer & player) != player)
    {
        return -1;
    }

    switch (player)
    {
        case APP_BT_STREAM_HFP_PCM:
        case APP_BT_STREAM_HFP_CVSD:
        case APP_BT_STREAM_HFP_VENDOR:
            nRet = bt_sco_player(false, freq);
            nRet = bt_sco_player(true, freq);
            break;
        case APP_BT_STREAM_A2DP_SBC:
        case APP_BT_STREAM_A2DP_AAC:
        case APP_BT_STREAM_A2DP_VENDOR:
            {
#if defined(IBRT)
                uint8_t device_id = 0;
                ibrt_a2dp_status_t a2dp_status;
                POSSIBLY_UNUSED struct BT_DEVICE_T *curr_device = NULL;
                device_id = app_bt_audio_get_curr_a2dp_device();
                curr_device = app_bt_get_device(device_id);
                a2dp_ibrt_sync_get_status(device_id, &a2dp_status);
                TRACE_AUD_STREAM_I("[STRM_PLAYER][RESTART] state:%d", a2dp_status.state);
                if (a2dp_status.state == BT_A2DP_STREAM_STATE_STREAMING){
                    if (app_audio_manager_a2dp_is_active(device_id)){
                        TRACE_AUD_STREAM_I("[STRM_PLAYER][RESTART] resume");
                        nRet = bt_sbc_player(PLAYER_OPER_STOP, freq);
                        nRet = bt_sbc_player(PLAYER_OPER_START, freq);
                    }else{
                        if (APP_IBRT_IS_PROFILE_EXCHANGED(&curr_device->remote)){
                            TRACE_AUD_STREAM_I("[STRM_PLAYER][RESTART] force_audio_retrigger");
                            app_audio_manager_sendrequest(APP_BT_STREAM_MANAGER_START,BT_STREAM_MUSIC, device_id,MAX_RECORD_NUM);
                            app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_STREAM_RESTART);
                        }
                    }
                }
#elif BT_DEVICE_NUM > 1
                if (bes_bt_me_count_mobile_link() > 1)
                {
                    a2dp_audio_sysfreq_reset();
                    bt_media_volume_ptr_update_by_mediatype(BT_STREAM_MUSIC);
                    app_bt_stream_volumeset(btdevice_volume_p->a2dp_vol);
                }
#endif
            }
            break;
#ifdef __FACTORY_MODE_SUPPORT__
        case APP_FACTORYMODE_AUDIO_LOOP:
            break;
#endif
#if defined(MEDIA_PLAYER_SUPPORT)&&(!defined(PROMPT_SELF_MANAGEMENT))
        case APP_PLAY_BACK_AUDIO:
            break;
#endif
        default:
            nRet = -1;
            break;
    }

    return nRet;
}

extern uint8_t bt_media_current_music_get(void);
extern uint8_t bt_media_current_sco_get(void);

static uint8_t app_bt_stream_volumeup_generic(bool isToUpdateLocalVolumeLevel)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
    return BT_DEVICE_INVALID_ID;
#else
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t volume_changed_device_id = BT_DEVICE_INVALID_ID;

#if defined AUDIO_LINEIN
    if(app_bt_stream_isrun(APP_PLAY_LINEIN_AUDIO))
    {
        stream_linein_volume ++;
        if (stream_linein_volume > TGT_VOLUME_LEVEL_MAX)
        stream_linein_volume = TGT_VOLUME_LEVEL_MAX;
        app_bt_stream_volumeset(stream_linein_volume);
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][UP] set linein volume %d\n", stream_linein_volume);
    }else
#endif
    if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))
    {
        AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
        uint8_t hfp_local_vol = 0;

        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][UP] hfp volume");

        curr_device = app_bt_get_device(bt_media_current_sco_get());

        if (!curr_device)
        {
            TRACE(2, "%s invalid sco id %x", __func__, bt_media_current_sco_get());
            return BT_DEVICE_INVALID_ID;
        }

        hfp_local_vol = hfp_volume_local_get(curr_device->device_id);

        if(isToUpdateLocalVolumeLevel)
        {
            // get current local volume
            hfp_local_vol++;
            if (hfp_local_vol > TGT_VOLUME_LEVEL_MAX)
            {
                hfp_local_vol = TGT_VOLUME_LEVEL_MAX;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = hfp_convert_local_vol_to_bt_vol(hfp_local_vol);

            // increase bt volume
            if (currentBtVol >= MAX_HFP_VOL)
            {
                currentBtVol = MAX_HFP_VOL;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol++;
                //prompt_id = AUD_ID_VOLUME_UP;
            }

            hfp_local_vol = hfp_convert_bt_vol_to_local_vol(currentBtVol);
        }

        hfp_volume_local_set(curr_device->device_id, hfp_local_vol);

        current_btdevice_volume.hfp_vol = hfp_local_vol;

        app_bt_stream_volumeset(hfp_local_vol);

        volume_changed_device_id = curr_device->device_id;
    #if defined(IBRT)
        if (!app_ibrt_if_is_ui_slave())
    #endif
        {
            if (prompt_id != AUD_ID_INVALID) {
                TRACE(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(prompt_id, 0);
#endif
            }
        }
    }
    else if ((app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)) ||
        (app_bt_stream_isrun(APP_BT_STREAM_INVALID)))
    {
        AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
        uint8_t a2dp_local_vol = 0;

        curr_device = app_bt_get_device(bt_media_current_music_get());

        if (!curr_device)
        {
            TRACE(2, "%s invalid sbc id %x", __func__, bt_media_current_music_get());
            return BT_DEVICE_INVALID_ID;
        }

        TRACE(1, "%s set a2dp volume", __func__);

        a2dp_local_vol = a2dp_volume_local_get(curr_device->device_id);

        if(isToUpdateLocalVolumeLevel)
        {
            // get current local volume
            a2dp_local_vol++;
            if (a2dp_local_vol > TGT_VOLUME_LEVEL_MAX)
            {
                a2dp_local_vol = TGT_VOLUME_LEVEL_MAX;
                prompt_id = AUD_ID_BT_WARNING;
            }
            a2dp_volume_set_local_vol(curr_device->device_id, a2dp_local_vol);
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = a2dp_abs_volume_get(curr_device->device_id);

            // increase bt volume
            if (currentBtVol >= MAX_A2DP_VOL)
            {
                currentBtVol = MAX_A2DP_VOL;
                prompt_id = AUD_ID_BT_WARNING;

            }
            else
            {
                currentBtVol++;
                //prompt_id = AUD_ID_VOLUME_UP;
            }


            a2dp_volume_set(curr_device->device_id, currentBtVol);

            a2dp_local_vol = a2dp_convert_bt_vol_to_local_vol(currentBtVol);
        }

        current_btdevice_volume.a2dp_vol = a2dp_local_vol;

        app_bt_stream_volumeset(a2dp_local_vol);

        volume_changed_device_id = curr_device->device_id;
    #if defined(IBRT)
        if (!app_ibrt_if_is_ui_slave())
    #endif
        {
            if (prompt_id != AUD_ID_INVALID) {
                TRACE(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(prompt_id, 0);
#endif
            }
        }
    }

    TRACE(2,"%s a2dp: %d", __func__, current_btdevice_volume.a2dp_vol);
    TRACE(2,"%s hfp: %d", __func__, current_btdevice_volume.hfp_vol);

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif

    return volume_changed_device_id;
#endif
}

void app_bt_stream_bt_volumeup(void)
{
    app_bt_stream_volumeup_generic(false);
}

uint8_t app_bt_stream_local_volumeup(void)
{
    return app_bt_stream_volumeup_generic(true);
}

void app_bt_stream_update_local_hfp_vol(uint8_t updatedLocalVol)
{
    if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))
    {
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][SET] hfp volume");
        struct BT_DEVICE_T *curr_device = app_bt_get_device(bt_media_current_sco_get());

        if (!curr_device)
        {
            TRACE(2, "%s invalid sco id %x", __func__, bt_media_current_sco_get());
            return;
        }

        if (updatedLocalVol > TGT_VOLUME_LEVEL_MAX)
        {
            updatedLocalVol = TGT_VOLUME_LEVEL_MAX;
        }

        if (updatedLocalVol < TGT_VOLUME_LEVEL_MUTE)
        {
            updatedLocalVol = TGT_VOLUME_LEVEL_MUTE;
        }

        hfp_volume_local_set(curr_device->device_id, updatedLocalVol);

        app_bt_stream_volumeset(btdevice_volume_p->hfp_vol);
    }

    TRACE(2,"%s hfp: %d", __func__, btdevice_volume_p->hfp_vol);

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif
}

void app_bt_set_volume(uint16_t type,uint8_t level)
{
    if ((type&APP_BT_STREAM_HFP_PCM) && app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)) {
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL] set hfp volume");
        if (level >= TGT_VOLUME_LEVEL_MUTE && level <= TGT_VOLUME_LEVEL_15)
        {
            uint32_t lock = nv_record_pre_write_operation();
            btdevice_volume_p->hfp_vol = level;
            nv_record_post_write_operation(lock);
            app_bt_stream_volumeset(btdevice_volume_p->hfp_vol);
        }
        if (btdevice_volume_p->hfp_vol == TGT_VOLUME_LEVEL_MUTE)
        {
#ifdef MEDIA_PLAYER_SUPPORT
        #if defined(IBRT)
            if (!app_ibrt_if_is_ui_slave())
        #endif
            {
                media_PlayAudio(AUD_ID_BT_WARNING,0);
            }
#endif
        }
    }
    if ((type&APP_BT_STREAM_A2DP_SBC) && ((app_bt_stream_isrun(APP_BT_STREAM_INVALID)) ||
        (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)))) {
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL] set a2dp volume");
        if (level >= TGT_VOLUME_LEVEL_MUTE && level <= TGT_VOLUME_LEVEL_15)
        {
            uint32_t lock = nv_record_pre_write_operation();
            btdevice_volume_p->a2dp_vol = level;
            nv_record_post_write_operation(lock);
            app_bt_stream_volumeset(btdevice_volume_p->a2dp_vol);
        }
        if (btdevice_volume_p->a2dp_vol == TGT_VOLUME_LEVEL_MUTE)
        {
#ifdef MEDIA_PLAYER_SUPPORT
        #if defined(IBRT)
            if (!app_ibrt_if_is_ui_slave())
        #endif
            {
                media_PlayAudio(AUD_ID_BT_WARNING,0);
            }
#endif
        }
    }

    TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL] a2dp: %d", btdevice_volume_p->a2dp_vol);
    TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL] hfp: %d", btdevice_volume_p->hfp_vol);
#ifndef FPGA
    nv_record_touch_cause_flush();
#endif
}

static uint8_t app_bt_stream_volumedown_generic(bool isToUpdateLocalVolumeLevel)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
    return BT_DEVICE_INVALID_ID;
#else
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t volume_changed_device_id = BT_DEVICE_INVALID_ID;

#if defined AUDIO_LINEIN
    if(app_bt_stream_isrun(APP_PLAY_LINEIN_AUDIO))
    {
        stream_linein_volume --;
        if (stream_linein_volume < TGT_VOLUME_LEVEL_MUTE)
            stream_linein_volume = TGT_VOLUME_LEVEL_MUTE;
        app_bt_stream_volumeset(stream_linein_volume);
        TRACE(1,"set linein volume %d\n", stream_linein_volume);
    }else
#endif
    if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM))
    {
        AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
        uint8_t hfp_local_vol = 0;

        curr_device = app_bt_get_device(bt_media_current_sco_get());

        if (!curr_device)
        {
            TRACE(2, "%s invalid sco id %x", __func__, bt_media_current_sco_get());
            return BT_DEVICE_INVALID_ID;
        }

        TRACE(1, "%s set hfp volume", __func__);

        hfp_local_vol = hfp_volume_local_get(curr_device->device_id);

        if(isToUpdateLocalVolumeLevel)
        {
            // get current local volume
            if (hfp_local_vol)
            {
                hfp_local_vol--;
            }
            if (hfp_local_vol <= TGT_VOLUME_LEVEL_MUTE)
            {
                hfp_local_vol = TGT_VOLUME_LEVEL_MUTE;
                prompt_id = AUD_ID_BT_WARNING;
            }
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = hfp_convert_local_vol_to_bt_vol(hfp_local_vol);

            if (currentBtVol <= 0)
            {
                currentBtVol = 0;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol--;
                //prompt_id = AUD_ID_VOLUME_DOWN;
            }

            hfp_local_vol = hfp_convert_bt_vol_to_local_vol(currentBtVol);
        }

        hfp_volume_local_set(curr_device->device_id, hfp_local_vol);

        current_btdevice_volume.hfp_vol = hfp_local_vol;

        app_bt_stream_volumeset(hfp_local_vol);

        volume_changed_device_id = curr_device->device_id;
    #if defined(IBRT)
        if (!app_ibrt_if_is_ui_slave())
    #endif
        {
            if (prompt_id != AUD_ID_INVALID) {
                TRACE(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(prompt_id, 0);
#endif
            }
        }
    }
    else if ((app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)) ||
        (app_bt_stream_isrun(APP_BT_STREAM_INVALID)))
    {
        AUD_ID_ENUM prompt_id = AUD_ID_INVALID;
        uint8_t a2dp_local_vol = 0;

        curr_device = app_bt_get_device(bt_media_current_music_get());

        if (!curr_device)
        {
            TRACE(2, "%s invalid sbc id %x", __func__, bt_media_current_music_get());
            return BT_DEVICE_INVALID_ID;
        }

        TRACE(1, "%s set a2dp volume", __func__);

        a2dp_local_vol = a2dp_volume_local_get(curr_device->device_id);

        if(isToUpdateLocalVolumeLevel)
        {
            // get current local volume
            if (a2dp_local_vol)
            {
                a2dp_local_vol--;
            }
            if (a2dp_local_vol <= TGT_VOLUME_LEVEL_MUTE)
            {
                a2dp_local_vol = TGT_VOLUME_LEVEL_MUTE;
                prompt_id = AUD_ID_BT_WARNING;
            }

            a2dp_volume_set_local_vol(curr_device->device_id, a2dp_local_vol);
        }
        else
        {
            // get current bt volume
            uint8_t currentBtVol = a2dp_abs_volume_get(curr_device->device_id);

            if (currentBtVol <= 0)
            {
                currentBtVol = 0;
                prompt_id = AUD_ID_BT_WARNING;
            }
            else
            {
                currentBtVol--;
                //prompt_id = AUD_ID_VOLUME_DOWN;
            }

            a2dp_volume_set(curr_device->device_id, currentBtVol);

            a2dp_local_vol = a2dp_convert_bt_vol_to_local_vol(currentBtVol);
        }

        current_btdevice_volume.a2dp_vol = a2dp_local_vol;

        app_bt_stream_volumeset(a2dp_local_vol);

        volume_changed_device_id = curr_device->device_id;
    #if defined(IBRT)
        if (!app_ibrt_if_is_ui_slave())
    #endif
        {
            if (prompt_id != AUD_ID_INVALID) {
                TRACE(1, "AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
                media_PlayAudio(prompt_id, 0);
#endif
            }
        }
    }

    TRACE(2,"%s a2dp: %d", __func__, current_btdevice_volume.a2dp_vol);
    TRACE(2,"%s hfp: %d", __func__, current_btdevice_volume.hfp_vol);

#ifndef FPGA
    nv_record_touch_cause_flush();
#endif

    return volume_changed_device_id;
#endif
}

void app_bt_stream_bt_volumedown(void)
{
    app_bt_stream_volumedown_generic(false);
}

uint8_t app_bt_stream_local_volumedown(void)
{
    return app_bt_stream_volumedown_generic(true);
}

void app_bt_stream_volumeset_handler(int8_t vol)
{
    uint32_t ret;
    struct AF_STREAM_CONFIG_T *stream_cfg = NULL;

#ifdef AUDIO_OUTPUT_ROUTE_SELECT
    if (stream_route_info.output_route_info && stream_route_info.output_route_info->output_set_volume)
    {
        stream_route_info.output_route_info->output_set_volume(vol);
    }
    else
#endif
    {
        ret = af_stream_get_cfg(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, &stream_cfg, false);
        if (ret == 0 && stream_cfg) {
            stream_cfg->vol = vol;
            af_stream_setup(AUD_STREAM_ID_0, AUD_STREAM_PLAYBACK, stream_cfg);
        }
    }

#if (defined(AUDIO_ANC_FB_MC)||defined(AUDIO_ANC_FB_MC_SCO)) && defined(ANC_APP) && !defined(__AUDIO_RESAMPLE__)
    ret = af_stream_get_cfg(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, &stream_cfg, false);
    if (ret == 0) {
        stream_cfg->vol = vol;
        af_stream_setup(AUD_STREAM_ID_2, AUD_STREAM_PLAYBACK, stream_cfg);
    }
#endif
}

void app_bt_stream_volume_edge_check(void)
{
#if defined(IBRT)
    if (app_ibrt_if_is_ui_slave())
    {
        return;
    }
#endif
#ifdef MEDIA_PLAYER_SUPPORT
    bool isHfpStream = (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM) ||
        app_bt_stream_isrun(APP_BT_STREAM_HFP_CVSD) ||
        app_bt_stream_isrun(APP_BT_STREAM_HFP_VENDOR));
    bool isA2dpStream = (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC) ||
        app_bt_stream_isrun(APP_BT_STREAM_A2DP_AAC) ||
        app_bt_stream_isrun(APP_BT_STREAM_A2DP_VENDOR));
    if (isHfpStream || isA2dpStream)
    {
        if (TGT_VOLUME_LEVEL_MUTE == stream_local_volume)
        {
            media_PlayAudio(AUD_ID_BT_WARNING, 0);
        }
        else if (TGT_VOLUME_LEVEL_MAX == stream_local_volume)
        {
            media_PlayAudio(AUD_ID_BT_WARNING, 0);
        }
    }
#endif
}

int app_bt_stream_volumeset(int8_t vol)
{
    if (vol > TGT_VOLUME_LEVEL_MAX)
        vol = TGT_VOLUME_LEVEL_MAX;
    if (vol < TGT_VOLUME_LEVEL_MUTE)
        vol = TGT_VOLUME_LEVEL_MUTE;

    TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][SET] vol=%d", vol);

    stream_local_volume = vol;
#ifdef MIX_AUDIO_PROMPT_WITH_A2DP_MEDIA_ENABLED
    if ((!app_bt_stream_isrun(APP_PLAY_BACK_AUDIO)) &&
        (audio_prompt_is_allow_update_volume()))
#else
    if (!app_bt_stream_isrun(APP_PLAY_BACK_AUDIO))
#endif
    {
        app_bt_stream_volumeset_handler(vol);
    }
    return 0;
}

void app_bt_stream_set_tune_samplerate_ratio(enum AUD_STREAM_T stream, float ratio)
{
#ifdef AUDIO_OUTPUT_ROUTE_SELECT
    if (stream_route_info.output_route_info)
    {
        return;
    }
    else
#endif
    {
        TRACE(1,"codec_tune_resample_rate:%d", (int32_t)(ratio * 10000000));
        af_codec_tune(stream, ratio);
    }
}

void app_bt_stream_set_output_route(void *route_info)
{
    #ifdef AUDIO_OUTPUT_ROUTE_SELECT
        TRACE(0, "[%s][%d]: %p", __FUNCTION__, __LINE__, route_info);
        stream_route_info.output_route_info = (app_bt_stream_output_device_info_t *)route_info;
    #endif
}

int app_bt_stream_local_volume_get(void)
{
    return stream_local_volume;
}

uint8_t app_bt_stream_a2dpvolume_get(void)
{
   // return btdevice_volume_p->a2dp_vol;
   return current_btdevice_volume.a2dp_vol;
}

uint8_t app_bt_stream_hfpvolume_get(void)
{
    //return btdevice_volume_p->hfp_vol;
    return current_btdevice_volume.hfp_vol;

}

void app_bt_stream_a2dpvolume_reset(void)
{
    btdevice_volume_p->a2dp_vol = hal_codec_get_default_dac_volume_index();
    current_btdevice_volume.a2dp_vol = hal_codec_get_default_dac_volume_index();
}

void app_bt_stream_hfpvolume_reset(void)
{
    btdevice_volume_p->hfp_vol = hal_codec_get_default_dac_volume_index();
    current_btdevice_volume.hfp_vol = hal_codec_get_default_dac_volume_index();
}

void app_bt_stream_volume_ptr_update(uint8_t *bdAddr)
{
#ifndef FPGA
    nvrec_btdevicerecord *record = NULL;

    memset(&current_btdevice_volume, 0, sizeof(btdevice_volume));

    if (bdAddr && !nv_record_btdevicerecord_find((bt_bdaddr_t*)bdAddr,&record))
    {
        btdevice_volume_p = &(record->device_vol);
        DUMP8("0x%02x ", bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][UPDATE] a2dp_vol:%d hfp_vol:%d ptr:%p",  btdevice_volume_p->a2dp_vol, btdevice_volume_p->hfp_vol,btdevice_volume_p);
    }
    else
#endif
    {
        btdevice_volume_p = &default_stream_volume;
        TRACE_AUD_STREAM_I("[STRM_PLAYER][VOL][UPDATE] default");
        if (bdAddr){
            DUMP8("0x%02x ", bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);
        }
    }
    current_btdevice_volume.a2dp_vol=btdevice_volume_p->a2dp_vol;
    current_btdevice_volume.hfp_vol=btdevice_volume_p->hfp_vol;

}

struct btdevice_volume * app_bt_stream_volume_get_ptr(void)
{
    return btdevice_volume_p;
}

bool app_bt_stream_isrun(uint16_t player)
{
    if ((gStreamplayer & player) == player)
    {
        return true;
    }
    else
    {
        return false;
    }
}

int app_bt_stream_closeall()
{
    TRACE_AUD_STREAM_I("[STRM_PLAYER][CLOSEALL]");

    bt_sco_player(false, APP_SYSFREQ_32K);
    bt_sbc_player(PLAYER_OPER_STOP, APP_SYSFREQ_32K);

#if defined(MEDIA_PLAYER_SUPPORT) && !defined(AUDIO_PROMPT_USE_DAC2_ENABLED)
    app_play_audio_onoff(false, 0);
#endif

#ifdef RB_CODEC
    app_rbplay_audio_onoff(false, 0);
#endif

#ifdef AUDIO_LINEIN
    app_play_linein_onoff(false);
#endif

    gStreamplayer = APP_BT_STREAM_INVALID;

    return 0;
}

void app_bt_stream_copy_track_one_to_two_24bits(int32_t *dst_buf, int32_t *src_buf, uint32_t src_len)
{
    // Copy from tail so that it works even if dst_buf == src_buf
    for (int i = (int)(src_len - 1); i >= 0; i--)
    {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
    }
}

void app_bt_stream_copy_track_one_to_two_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t src_len)
{
    // Copy from tail so that it works even if dst_buf == src_buf
    for (int i = (int)(src_len - 1); i >= 0; i--)
    {
        dst_buf[i*2 + 0] = dst_buf[i*2 + 1] = src_buf[i];
    }
}

void app_bt_stream_copy_track_two_to_one_16bits(int16_t *dst_buf, int16_t *src_buf, uint32_t dst_len)
{
    for (uint32_t i = 0; i < dst_len; i++)
    {
        dst_buf[i] = src_buf[i*2];
    }
}

#ifdef PLAYBACK_FORCE_48K
static int app_force48k_resample_iter(uint8_t *buf, uint32_t len)
{
#if (A2DP_DECODER_VER == 2)
    uint8_t device_id = app_bt_audio_get_curr_a2dp_device();
    a2dp_audio_playback_handler(device_id, buf, len);
#else
    uint8_t codec_type = bta_get_curr_a2dp_codec_type();
    uint32_t overlay_id = 0;
    if(0){
#if defined(A2DP_LHDC_ON)
    }else if(codec_type ==  BT_A2DP_CODEC_TYPE_NON_A2DP){
        overlay_id = APP_OVERLAY_A2DP_LHDC;
#endif
#if defined(A2DP_AAC_ON)
    }else if(codec_type ==  BT_A2DP_CODEC_TYPE_MPEG2_4_AAC){
        overlay_id = APP_OVERLAY_A2DP_AAC;
#endif
    }else if(codec_type == BT_A2DP_CODEC_TYPE_SBC){
        overlay_id = APP_OVERLAY_A2DP;
    }
#ifndef FPGA
    a2dp_audio_more_data(overlay_id, buf, len);
#endif

#endif // #if (A2DP_DECODER_VER == 2)

    return 0;
}

static struct APP_RESAMPLE_T *app_resample_open(enum AUD_STREAM_T stream, const struct RESAMPLE_COEF_T *coef, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step);

struct APP_RESAMPLE_T *app_force48k_resample_any_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    const struct RESAMPLE_COEF_T *coef = NULL;

    if (sample_rate == AUD_SAMPRATE_44100)
    {
        coef = &resample_coef_44p1k_to_48k;
    }
    else
    {
        ASSERT(false, "%s: Bad sample rate: %u", __func__, sample_rate);
    }

    return app_resample_open(AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, 0);
}
#endif

// =======================================================
// APP RESAMPLE
// =======================================================

#ifndef MIX_MIC_DURING_MUSIC
#include "resample_coef.h"
#endif

static APP_RESAMPLE_BUF_ALLOC_CALLBACK resamp_buf_alloc = app_audio_mempool_get_buff;

static void memzero_int16(void *dst, uint32_t len)
{
    if (dst)
    {
        int16_t *dst16 = (int16_t *)dst;
        int16_t *dst16_end = dst16 + len / 2;

        while (dst16 < dst16_end)
        {
            *dst16++ = 0;
        }
    }
    else
    {
        TRACE_AUD_STREAM_I("WRN: receive null pointer");
    }
}

struct APP_RESAMPLE_T *app_resample_open_with_preallocated_buf(enum AUD_STREAM_T stream, const struct RESAMPLE_COEF_T *coef, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step, uint8_t* buf, uint32_t bufSize)
{
    TRACE_AUD_STREAM_I("[STRM_PLAYER][PROMPT_MIXER][OPEN]");
    struct APP_RESAMPLE_T *resamp;
    struct RESAMPLE_CFG_T cfg;
    enum RESAMPLE_STATUS_T status;
    uint32_t size, resamp_size;

    resamp_size = audio_resample_ex_get_buffer_size(chans, AUD_BITS_16, coef->phase_coef_num);

    size = sizeof(struct APP_RESAMPLE_T);
    size += ALIGN(iter_len, 4);
    size += resamp_size;

    ASSERT(size < bufSize, "Pre-allocated buffer size %d is smaller than the needed size %d",
        bufSize, size);

    resamp = (struct APP_RESAMPLE_T *)buf;
    buf += sizeof(*resamp);
    resamp->stream = stream;
    resamp->cb = cb;
    resamp->iter_buf = buf;
    buf += ALIGN(iter_len, 4);
    resamp->iter_len = iter_len;
    resamp->offset = iter_len;
    resamp->ratio_step = ratio_step;

    memset(&cfg, 0, sizeof(cfg));
    cfg.chans = chans;
    cfg.bits = AUD_BITS_16;
    cfg.ratio_step = ratio_step;
    cfg.coef = coef;
    cfg.buf = buf;
    cfg.size = resamp_size;

    status = audio_resample_ex_open(&cfg, (RESAMPLE_ID *)&resamp->id);
    ASSERT(status == RESAMPLE_STATUS_OK, "%s: Failed to open resample: %d", __func__, status);

#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_enable();
#endif

    return resamp;
}


static struct APP_RESAMPLE_T *app_resample_open(enum AUD_STREAM_T stream, const struct RESAMPLE_COEF_T *coef, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    TRACE_AUD_STREAM_I("[STRM_PLAYER][RESAMPLE][OPEN] ratio: %d/1000", uint32_t(ratio_step * 1000));
    struct APP_RESAMPLE_T *resamp;
    struct RESAMPLE_CFG_T cfg;
    enum RESAMPLE_STATUS_T status;
    uint32_t size, resamp_size;
    uint8_t *buf;

    resamp_size = audio_resample_ex_get_buffer_size(chans, AUD_BITS_16, coef->phase_coef_num);

    size = sizeof(struct APP_RESAMPLE_T);
    size += ALIGN(iter_len, 4);
    size += resamp_size;

    resamp_buf_alloc(&buf, size);

    resamp = (struct APP_RESAMPLE_T *)buf;
    buf += sizeof(*resamp);
    resamp->stream = stream;
    resamp->cb = cb;
    resamp->iter_buf = buf;
    buf += ALIGN(iter_len, 4);
    resamp->iter_len = iter_len;
    resamp->offset = iter_len;
    resamp->ratio_step = ratio_step;

    memset(&cfg, 0, sizeof(cfg));
    cfg.chans = chans;
    cfg.bits = AUD_BITS_16;
    cfg.ratio_step = ratio_step;
    cfg.coef = coef;
    cfg.buf = buf;
    cfg.size = resamp_size;

    status = audio_resample_ex_open(&cfg, (RESAMPLE_ID *)&resamp->id);
    ASSERT(status == RESAMPLE_STATUS_OK, "%s: Failed to open resample: %d", __func__, status);

#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_enable();
#endif

    return resamp;
}

static int app_resample_close(struct APP_RESAMPLE_T *resamp)
{
#ifdef CHIP_BEST1000
    hal_cmu_audio_resample_disable();
#endif

    if (resamp)
    {
        audio_resample_ex_close((RESAMPLE_ID *)resamp->id);
    }

    return 0;
}

struct APP_RESAMPLE_T *app_playback_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len)
{
    const struct RESAMPLE_COEF_T *coef = NULL;

    if (sample_rate == AUD_SAMPRATE_8000)
    {
        coef = &resample_coef_8k_to_8p4k;
    }
    else if (sample_rate == AUD_SAMPRATE_16000)
    {
        coef = &resample_coef_8k_to_8p4k;
    }
    else if (sample_rate == AUD_SAMPRATE_32000)
    {
        coef = &resample_coef_32k_to_50p7k;
    }
    else if (sample_rate == AUD_SAMPRATE_44100)
    {
        coef = &resample_coef_44p1k_to_50p7k;
    }
    else if (sample_rate == AUD_SAMPRATE_48000)
    {
        coef = &resample_coef_48k_to_50p7k;
    }
    else
    {
        ASSERT(false, "%s: Bad sample rate: %u", __func__, sample_rate);
    }

    return app_resample_open(AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, 0);
}

#ifdef RESAMPLE_ANY_SAMPLE_RATE
struct APP_RESAMPLE_T *app_playback_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;

    return app_resample_open(AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, ratio_step);
}

struct APP_RESAMPLE_T *app_playback_resample_any_open_with_pre_allocated_buffer(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step, uint8_t* ptrBuf, uint32_t bufSize)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;

    return app_resample_open_with_preallocated_buf(
        AUD_STREAM_PLAYBACK, coef, chans, cb, iter_len, ratio_step, ptrBuf, bufSize);
}
#endif

int app_playback_resample_close(struct APP_RESAMPLE_T *resamp)
{
    return app_resample_close(resamp);
}

int app_playback_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len)
{
    uint32_t in_size, out_size;
    struct RESAMPLE_IO_BUF_T io;
    enum RESAMPLE_STATUS_T status;
    int ret;
    //uint32_t lock;

    if (resamp == NULL)
    {
        goto _err_exit;
    }

    io.out_cyclic_start = NULL;
    io.out_cyclic_end = NULL;

    if (resamp->offset < resamp->iter_len)
    {
        io.in = resamp->iter_buf + resamp->offset;
        io.in_size = resamp->iter_len - resamp->offset;
        io.out = buf;
        io.out_size = len;

        //lock = int_lock();
        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        //int_unlock(lock);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        buf += out_size;
        len -= out_size;
        resamp->offset += in_size;

        ASSERT(len == 0 || resamp->offset == resamp->iter_len,
            "%s: Bad resample offset: len=%d offset=%u iter_len=%u",
            __func__, len, resamp->offset, resamp->iter_len);
    }

    while (len)
    {
        ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
        if (ret)
        {
            goto _err_exit;
        }

        io.in = resamp->iter_buf;
        io.in_size = resamp->iter_len;
        io.out = buf;
        io.out_size = len;

        //lock = int_lock();
        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        //int_unlock(lock);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        ASSERT(out_size <= len, "%s: Bad resample out_size: out_size=%u len=%d", __func__, out_size, len);
        ASSERT(in_size <= resamp->iter_len, "%s: Bad resample in_size: in_size=%u iter_len=%u", __func__, in_size, resamp->iter_len);

        buf += out_size;
        len -= out_size;
        if (in_size != resamp->iter_len)
        {
            resamp->offset = in_size;

            ASSERT(len == 0, "%s: Bad resample len: len=%d out_size=%u", __func__, len, out_size);
        }
    }

    return 0;

_err_exit:
    if (resamp)
    {
        app_resample_reset(resamp);
    }

    memzero_int16(buf, len);

    return 1;
}

struct APP_RESAMPLE_T *app_capture_resample_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len)
{
    const struct RESAMPLE_COEF_T *coef = NULL;

    if (sample_rate == AUD_SAMPRATE_8000)
    {
        coef = &resample_coef_8p4k_to_8k;
    }
    else if (sample_rate == AUD_SAMPRATE_16000)
    {
        // Same coef as 8K sample rate
        coef = &resample_coef_8p4k_to_8k;
    }
    else
    {
        ASSERT(false, "%s: Bad sample rate: %u", __func__, sample_rate);
    }

    return app_resample_open(AUD_STREAM_CAPTURE, coef, chans, cb, iter_len, 0);

}

#ifdef RESAMPLE_ANY_SAMPLE_RATE
struct APP_RESAMPLE_T *app_capture_resample_any_open(enum AUD_CHANNEL_NUM_T chans,
        APP_RESAMPLE_ITER_CALLBACK cb, uint32_t iter_len,
        float ratio_step)
{
    const struct RESAMPLE_COEF_T *coef = &resample_coef_any_up256;
    return app_resample_open(AUD_STREAM_CAPTURE, coef, chans, cb, iter_len, ratio_step);
}
#endif

int app_capture_resample_close(struct APP_RESAMPLE_T *resamp)
{
    return app_resample_close(resamp);
}

int app_capture_resample_run(struct APP_RESAMPLE_T *resamp, uint8_t *buf, uint32_t len)
{
    uint32_t in_size, out_size;
    struct RESAMPLE_IO_BUF_T io;
    enum RESAMPLE_STATUS_T status;
    int ret;

    if (resamp == NULL)
    {
        goto _err_exit;
    }

    io.out_cyclic_start = NULL;
    io.out_cyclic_end = NULL;

    if (resamp->offset < resamp->iter_len)
    {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf + resamp->offset;
        io.out_size = resamp->iter_len - resamp->offset;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        buf += in_size;
        len -= in_size;
        resamp->offset += out_size;

        ASSERT(len == 0 || resamp->offset == resamp->iter_len,
            "%s: Bad resample offset: len=%d offset=%u iter_len=%u",
            __func__, len, resamp->offset, resamp->iter_len);

        if (resamp->offset == resamp->iter_len)
        {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret)
            {
                goto _err_exit;
            }
        }
    }

    while (len)
    {
        io.in = buf;
        io.in_size = len;
        io.out = resamp->iter_buf;
        io.out_size = resamp->iter_len;

        status = audio_resample_ex_run((RESAMPLE_ID *)resamp->id, &io, &in_size, &out_size);
        if (status != RESAMPLE_STATUS_OUT_FULL && status != RESAMPLE_STATUS_IN_EMPTY &&
            status != RESAMPLE_STATUS_DONE)
        {
            goto _err_exit;
        }

        ASSERT(in_size <= len, "%s: Bad resample in_size: in_size=%u len=%u", __func__, in_size, len);
        ASSERT(out_size <= resamp->iter_len, "%s: Bad resample out_size: out_size=%u iter_len=%u", __func__, out_size, resamp->iter_len);

        buf += in_size;
        len -= in_size;
        if (out_size == resamp->iter_len)
        {
            ret = resamp->cb(resamp->iter_buf, resamp->iter_len);
            if (ret)
            {
                goto _err_exit;
            }
        }
        else
        {
            resamp->offset = out_size;

            ASSERT(len == 0, "%s: Bad resample len: len=%u in_size=%u", __func__, len, in_size);
        }
    }

    return 0;

_err_exit:
    if (resamp)
    {
        app_resample_reset(resamp);
    }

    memzero_int16(buf, len);

    return 1;
}

void app_resample_reset(struct APP_RESAMPLE_T *resamp)
{
    audio_resample_ex_flush((RESAMPLE_ID *)resamp->id);
    resamp->offset = resamp->iter_len;
}

void app_resample_tune(struct APP_RESAMPLE_T *resamp, float ratio)
{
    float new_step;

    if (resamp == NULL)
    {
        return;
    }

    TRACE_AUD_STREAM_I("%s: stream=%d ratio=%d", __FUNCTION__, resamp->stream, FLOAT_TO_PPB_INT(ratio));

    if (resamp->stream == AUD_STREAM_PLAYBACK) {
        new_step = resamp->ratio_step + resamp->ratio_step * ratio;
    } else {
        new_step = resamp->ratio_step - resamp->ratio_step * ratio;
    }
    audio_resample_ex_set_ratio_step(resamp->id, new_step);
}

APP_RESAMPLE_BUF_ALLOC_CALLBACK app_resample_set_buf_alloc_callback(APP_RESAMPLE_BUF_ALLOC_CALLBACK cb)
{
    APP_RESAMPLE_BUF_ALLOC_CALLBACK old_cb;

    old_cb = resamp_buf_alloc;
    resamp_buf_alloc = cb;

    return old_cb;
}

#ifdef TX_RX_PCM_MASK

#ifdef SCO_DMA_SNAPSHOT

#define MSBC_LEN  60

void store_encode_frame2buff()
{
    if(bt_sco_codec_is_msbc())
    {
    uint32_t len;
    //processing uplink msbc data.
        if(playback_buf_btpcm_copy!=NULL)
        {
            len=playback_size_btpcm_copy-MSBC_LEN;
        memcpy((uint8_t *)(*(volatile uint32_t *)(MIC_BUFF_ADRR_REG)),playback_buf_btpcm_copy,MSBC_LEN);
        memcpy(playback_buf_btpcm_copy,playback_buf_btpcm_copy+MSBC_LEN,len);
        }
    //processing downlink msbc data.
        if(capture_buf_btpcm_copy!=NULL)
        {
            len=capture_size_btpcm_copy-MSBC_LEN;
        memcpy(capture_buf_btpcm_copy,capture_buf_btpcm_copy+MSBC_LEN,len);
        memcpy(capture_buf_btpcm_copy+len,(uint8_t *)(*(volatile uint32_t *)(RX_BUFF_ADRR)),MSBC_LEN);
        }
#if defined(CHIP_BEST2300A)
        uint8_t sco_toggle = *(volatile uint8_t *)(RX_BUFF_ADRR+8);
        pcm_data_param[sco_toggle].curr_time = *(volatile uint32_t *)(RX_BUFF_ADRR+4);
        pcm_data_param[sco_toggle].toggle = sco_toggle;
        pcm_data_param[sco_toggle].flag = *(volatile uint8_t *)(RX_BUFF_ADRR+9);
        pcm_data_param[sco_toggle].counter = *(volatile uint16_t *)(RX_BUFF_ADRR+10);
#endif
    }
    return;
}
#else
extern CQueue* get_tx_esco_queue_ptr();
extern CQueue* get_rx_esco_queue_ptr();
void store_encode_frame2buff()
{
    CQueue* Tx_esco_queue_temp = NULL;
    CQueue* Rx_esco_queue_temp = NULL;
    Tx_esco_queue_temp = get_tx_esco_queue_ptr();
    Rx_esco_queue_temp = get_rx_esco_queue_ptr();
    unsigned int len;
    len= 60;
    int status = 0;
    if(bt_sco_codec_is_msbc())
    {
        status = DeCQueue(Tx_esco_queue_temp,(uint8_t *)(*(volatile uint32_t *)(MIC_BUFF_ADRR_REG)),len);
        if(status){
            //TRACE_AUD_STREAM_I("TX DeC Fail");
        }
        status =EnCQueue(Rx_esco_queue_temp, (uint8_t *)(*(volatile uint32_t *)(RX_BUFF_ADRR)), len);
        if(status){
            //TRACE_AUD_STREAM_I("RX EnC Fail");
        }
    }

}
#endif
#endif

int app_bt_stream_init(void)
{
    default_stream_volume.a2dp_vol = hal_codec_get_default_dac_volume_index();
    default_stream_volume.hfp_vol = hal_codec_get_default_dac_volume_index();
    stream_local_volume = hal_codec_get_default_dac_volume_index();
#ifdef AUDIO_LINEIN
    stream_linein_volume = hal_codec_get_default_dac_volume_index();
#endif

    app_bt_stream_trigger_checker_init();
    app_bt_stream_trigger_checker_set_cb(app_bt_stream_trigger_success);
    return 0;
}

void app_bt_stream_register_notify_trigger_status_callback(APP_STREAM_NOTIFY_TRIGGER_STATUS_CALLBACK callback)
{
    app_stream_notify_trigger_status_callback = callback;
}

void app_bt_sco_switch_trigger_init(app_bt_sco_switch_trigger_t sco_switch_trigger)
{
    sco_switch_trigger_check = sco_switch_trigger;
}

void app_bt_sco_switch_trigger_deinit()
{
    sco_switch_trigger_check = NULL;
}

