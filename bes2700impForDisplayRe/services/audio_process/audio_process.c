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
/*******************************************************************************
** namer    : Audio Process
** description  : Manage auido process algorithms, include :hw iir eq, sw iir eq,
**                  hw fir eq, drc...
** version  : V1.0
** author   : Yunjie Huo
** modify   : 2018.12.4
** todo     : NULL
** MIPS     :   NO PROCESS: 34M(TWS, one ear, Mono, AAC, 48k)
**              DRC1: 36M(3 bands)
**              DRC2: 12M
*******************************************************************************/
/*
Audio Flow:
    DECODE --> SW IIR EQ --> DRC --> LIMTER --> VOLUME --> HW IIR EQ --> SPK

                                                       +-----------------------------+
                                                       |             DAC             |
                                                       |                             |
+--------+     +-----------+    +-----+    +--------+  | +--------+    +-----------+ |  +-----+
|        | PCM |           |    |     |    |        |  | |        |    |           | |  |     |
| DECODE +---->+ SW IIR EQ +--->+ DRC +--->+ LIMTER +--->+ VOLUME +--->+ HW IIR EQ +--->+ SPK |
|        |     |           |    |     |    |        |  | |        |    |           | |  |     |
+--------+     +-----------+    +-----+    +--------+  | +--------+    +-----------+ |  +-----+
                                                       +-----------------------------+

| ------------ | ------------------------- | -------- | ----------- |
| Algorithm    | description               | MIPS(M)  | RAM(kB)     |
| ------------ | ------------------------- | -------- | ----------- |
| DRC          | Dynamic Range Compression | 12M/band | 13          |
| Limiter/DRC2 | Limiter                   | 12M      | 5           |
| EQ           | Equalizer                 | 1M/band  | Almost zero |
*/

#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "string.h"
#include "audio_process.h"
#include "stdbool.h"
#include "hal_location.h"
#include "app_bt_stream.h"
#include "hw_codec_iir_process.h"
#include "hw_iir_process.h"
#include "tgt_hardware.h"
#include "drc.h"
#include "limiter.h"
#include "reverb.h"
#include "bass_enhancer.h"
#include "stereo_process.h"
#include "audio_cfg.h"
#include "hal_codec.h"
#include "hw_limiter_cfg.h"
#include "audioflinger.h"
#include "heap_api.h"
#include "dynamic_boost.h"
#include "a2dp_decoder.h"
#include "dynamic_eq.h"
#include "app_bt_stream.h"

// Enable this to test AUDIO_CUSTOM_EQ with audiotools,
// eq config from audiotools will be merge with default eq config.
// #define TEST_AUDIO_CUSTOM_EQ

#if defined(AUDIO_CUSTOM_EQ)
#include "ae_math.h"
#endif

#if defined(__AUDIO_HW_LIMITER__)
#include "psap_process.h"
#endif

#if defined(ANC_APP)
#include "app_anc.h"
#endif

//#define AUDIO_PROCESS_DUMP
#ifdef AUDIO_PROCESS_DUMP
#include "audio_dump.h"
#endif

#ifdef DIRAC_AUDIO_ENABLE
#include "heap_api.h"
#include "app_tws_ibrt.h"
#include <dirac.h>

void *dirac_memory = NULL;

void dirac_log_cb(const char *msg) {
    TRACE(0, msg);
}

dirac_channel_config_t get_dirac_channel_config() {
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    switch((A2DP_AUDIO_CHANNEL_SELECT_E)p_ibrt_ctrl->audio_chnl_sel) {
        case A2DP_AUDIO_CHANNEL_SELECT_LCHNL:
            return DIRAC_CHANNEL_CONFIG_TWS_LEFT;
        case A2DP_AUDIO_CHANNEL_SELECT_RCHNL:
            return DIRAC_CHANNEL_CONFIG_TWS_RIGHT;
        case A2DP_AUDIO_CHANNEL_SELECT_STEREO:
        case A2DP_AUDIO_CHANNEL_SELECT_LRMERGE:
            return DIRAC_CHANNEL_CONFIG_STEREO;
        default:
            return DIRAC_CHANNEL_CONFIG_RESERVED;
    }
}
#endif //DIRAC_AUDIO_ENABLE

#if defined(USB_EQ_TUNING)
#if !defined(__HW_DAC_IIR_EQ_PROCESS__) && !defined(__SW_IIR_EQ_PROCESS__)
#error "Either HW_DAC_IIR_EQ_PROCESS or SW_IIR_EQ_PROCESS should be defined when enabling USB_EQ_TUNING"
#endif
#endif

#ifdef AUDIO_DSP_ACCEL
#include "app_mcpp.h"
#endif

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
#if !defined(__HW_DAC_IIR_EQ_PROCESS__)
#error "Either HW_DAC_IIR_EQ_PROCESS should be defined when enabling IIR_EQ_PROCESS_LR_2CH"
#endif
#endif

#if defined(AUDIO_EQ_TUNING)
#include "hal_cmd.h"

#if defined(__SW_IIR_EQ_PROCESS__)
#define AUDIO_EQ_SW_IIR_UPDATE_CFG
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
#define AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
#define IIR_CHANNEL_SELECT_LCHNL    1
#define IIR_CHANNEL_SELECT_RCHNL    2
#define IIR_CHANNEL_SELECT_STEREO   3
int iir_channel_status = IIR_CHANNEL_SELECT_STEREO;
#endif // defined(__IIR_EQ_PROCESS_LR_2CH__)
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
#define AUDIO_EQ_HW_IIR_UPDATE_CFG
#endif

#ifdef __HW_FIR_EQ_PROCESS__
#define AUDIO_EQ_HW_FIR_UPDATE_CFG
#endif

#ifdef __AUDIO_DRC__
#define AUDIO_DRC_UPDATE_CFG
#endif

#ifdef __AUDIO_LIMITER__
#define AUDIO_LIMITER_UPDATE_CFG
#endif

#ifdef __AUDIO_HW_LIMITER__
#define AUDIO_HW_LIMITER_UPDATE_CFG
#endif

#ifdef __AUDIO_BASS_ENHANCER__
#define AUDIO_BASS_ENHANCER_UPDATE_CFG
#endif

#ifdef __AUDIO_REVERB__
#define AUDIO_REVERB_UPDATE_CFG
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#define AUDIO_DYNAMIC_BOOST_UPDATE_CFG
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
#define AUDIO_DYNAMIC_EQ_UPDATE_CFG
#endif

#if defined (__VIRTUAL_SURROUND__) || defined (__VIRTUAL_SURROUND_STEREO__)
#define VIRTUAL_SURROUND_UPDATE_CFG
#endif

#endif

#if defined(__SW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[EQ_SW_IIR_LIST_NUM];
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM];
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list2[EQ_HW_DAC_IIR_LIST_NUM];
#endif
#ifdef AUDIO_ADAPTIVE_IIR_EQ
extern IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg;
#endif
#endif

#if defined(__AUDIO_HW_LIMITER__)
extern const struct_psap_cfg * const hwlimiter_para_list_50p7k[HWLIMITER_PARA_LIST_NUM];
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
extern const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_iir_cfg_list[EQ_HW_IIR_LIST_NUM];
#endif

#ifdef __HW_FIR_EQ_PROCESS__
extern const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[EQ_HW_FIR_LIST_NUM];
#ifdef AUDIO_ADAPTIVE_FIR_EQ
extern FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg;
#endif
#endif

#ifdef __AUDIO_DRC__
extern const DrcConfig audio_drc_cfg;
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
extern const DynamicBoostConfig audio_dynamic_boost_cfg;
// Select dynamic boost using EQ type
#define DYNAMIC_BOOST_USE_HW_EQ
// #define DYNAMIC_BOOST_USE_SW_EQ
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
extern const DynamicEqConfig audio_dynamic_eq_cfg;
#endif

#ifdef __AUDIO_LIMITER__
extern const LimiterConfig audio_limiter_cfg;
#endif

#ifdef __AUDIO_REVERB__
extern const ReverbConfig audio_reverb_cfg;
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_STEREO__) || defined(__VIRTUAL_SURROUND_HWFIR__)
extern const VirtualSurroundConfig audio_virtual_surround_cfg;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
extern const BassEnhancerConfig audio_bass_enhancer_cfg;
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) || defined(AUDIO_EQ_HW_FIR_UPDATE_CFG)|| defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)|| defined(AUDIO_EQ_HW_IIR_UPDATE_CFG) || defined(AUDIO_DRC_UPDATE_CFG) || defined(AUDIO_LIMITER_UPDATE_CFG) || defined(AUDIO_DYNAMIC_BOOST_UPDATE_CFG) || defined(AUDIO_HW_LIMITER_UPDATE_CFG) || defined(AUDIO_BASS_ENHANCER_UPDATE_CFG) || defined(VIRTUAL_SURROUND_UPDATE_CFG) || defined(AUDIO_DYNAMIC_EQ_UPDATE_CFG)
#define AUDIO_UPDATE_CFG
#endif

#ifdef __AUDIO_DRC__
#define AUDIO_DRC_NEEDED_SIZE (1024*13)
#else
#define AUDIO_DRC_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#define AUDIO_DYNAMIC_BOOST_NEEDED_SIZE (1024*18)
#else
#define AUDIO_DYNAMIC_BOOST_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
#define AUDIO_DYNAMIC_EQ_NEEDED_SIZE (1024*11)
#else
#define AUDIO_DYNAMIC_EQ_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_LIMITER__
#define AUDIO_LIMITER_NEEDED_SIZE (1024*9)
#else
#define AUDIO_LIMITER_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_REVERB__
#define AUDIO_REVERB_NEEDED_SIZE (1024*9)
#else
#define AUDIO_REVERB_NEEDED_SIZE (0)
#endif

#ifdef __AUDIO_BASS_ENHANCER__
#define AUDIO_BASS_ENHANCER_NEEDED_SIZE (1024*25)
#else
#define AUDIO_BASS_ENHANCER_NEEDED_SIZE (0)
#endif

#if defined(__VIRTUAL_SURROUND__)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE (1024*40)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2 (1024*0)
#elif defined(__VIRTUAL_SURROUND_CP__)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE (1024*35)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2 (1024*0)
#elif defined(__VIRTUAL_SURROUND_HWFIR__)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE (1024*60)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2 (1024*0)
#elif defined(__VIRTUAL_SURROUND_STEREO__)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE (1024*90)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2 (1024*0)
#else
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE (0)
#define AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2 (1024*0)
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
StereoSurroundState * stereo_surround_st = NULL;
#endif

#define AUDIO_MEMORY_SIZE (AUDIO_DRC_NEEDED_SIZE + AUDIO_LIMITER_NEEDED_SIZE + AUDIO_REVERB_NEEDED_SIZE + AUDIO_STEREO_SORROUNDING_NEEDED_SIZE + AUDIO_BASS_ENHANCER_NEEDED_SIZE + + AUDIO_DYNAMIC_BOOST_NEEDED_SIZE + AUDIO_DYNAMIC_EQ_NEEDED_SIZE)
#ifdef DIRAC_AUDIO_ENABLE
#define DIRAC_SIZE (1024*48)
#undef AUDIO_MEMORY_SIZE
#define AUDIO_MEMORY_SIZE (AUDIO_DRC_NEEDED_SIZE + AUDIO_LIMITER_NEEDED_SIZE + AUDIO_REVERB_NEEDED_SIZE + AUDIO_STEREO_SORROUNDING_NEEDED_SIZE + AUDIO_BASS_ENHANCER_NEEDED_SIZE + DIRAC_SIZE + + AUDIO_DYNAMIC_BOOST_NEEDED_SIZE + AUDIO_DYNAMIC_EQ_NEEDED_SIZE)
#endif

#define SPEECH_MEMORY_SIZE (AUDIO_STEREO_SORROUNDING_NEEDED_SIZE2)


#if AUDIO_MEMORY_SIZE > 0
#include "audio_memory.h"
#endif

#if SPEECH_MEMORY_SIZE > 0
#include "speech_memory.h"
#endif

#ifdef __AUDIO_EQ_DYNAMICS_MEM__
HW_CODEC_IIR_FILTERS_T* l_old = NULL;
HW_CODEC_IIR_FILTERS_T* r_old = NULL;
HW_CODEC_IIR_FILTERS_T* cfg_l = NULL;
HW_CODEC_IIR_FILTERS_T* cfg_r = NULL;
HW_CODEC_IIR_FILTERS_T* new_l = NULL;
HW_CODEC_IIR_FILTERS_T* new_r = NULL;
#endif

#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif

typedef signed int          pcm_24bits_t;
typedef signed short int    pcm_16bits_t;

#ifdef AUDIO_HEARING_COMPSATN
extern float get_hear_compen_val_fir(int *test_buf, float *gain_buf, int num);
extern float get_hear_compen_val_multi_level(int *test_buf, float *gain_buf, int num);
extern short get_hear_level(void);
#include "hear_fir2.h"
#include "speech_memory.h"
#define HEAR_COMP_BUF_SIZE    1024*17
#ifdef HEARING_USE_STATIC_RAM
extern char HEAR_DET_STREAM_BUF[HEAR_COMP_BUF_SIZE];
#else
uint8_t *hear_comp_buf = NULL;
#endif

//int band_std[6] = {14,10,10,8,9,18};
#if HWFIR_MOD==HEARING_MOD_VAL
float fir_filter[384] = {0.0F};
#else
float hear_iir_coef[30] = {0.0F};
#endif
#define HWFIR_MOD   0
#define SWIIR_MOD   1
#define HWIIR_MOD   2
#endif

typedef struct{
    enum AUD_BITS_T         sample_bits;
    enum AUD_SAMPRATE_T     sample_rate;
    enum AUD_CHANNEL_NUM_T  sw_ch_num;
    enum AUD_CHANNEL_NUM_T  hw_ch_num;

#if defined(__SW_IIR_EQ_PROCESS__)
    bool sw_iir_enable;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    bool hw_dac_iir_enable;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
    bool hw_iir_enable;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
    bool hw_fir_enable;
#endif

#if AUDIO_MEMORY_SIZE > 0
    uint8_t *audio_heap;
#endif

#if SPEECH_MEMORY_SIZE > 0
    uint8_t *speech_heap;
#endif

#ifdef __AUDIO_DRC__
    DrcState *drc_st;
#endif

#ifdef __AUDIO_LIMITER__
    LimiterState *limiter_st;
#endif

#ifdef __AUDIO_REVERB__
    ReverbState *reverb_st;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    BassEnhancerState *bass_st;
#endif


#ifdef AUDIO_UPDATE_CFG
    bool update_cfg;
#endif

#ifdef USB_EQ_TUNING
	bool eq_updated_cfg;
#endif

#ifdef AUDIO_HEARING_COMPSATN
    //bool hear_updated_cfg;
#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
        IIR_CFG_T hear_iir_cfg;
        IIR_CFG_T hear_iir_cfg_update;
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        IIR_CFG_T hear_iir_cfg_2;
        IIR_CFG_T hear_iir_cfg_update_2;
#endif
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
        FIR_CFG_T hear_hw_fir_cfg;
#endif
#endif
#endif

#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    IIR_CFG_T sw_iir_cfg;
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    IIR_CFG_T hw_dac_iir_cfg;
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    IIR_CFG_T hw_dac_iir_cfg2;
#endif
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    IIR_CFG_T hw_iir_cfg;
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    FIR_CFG_T hw_fir_cfg;
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
    DynamicBoostState *dynamic_boost_st;
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
    DynamicEqState *dynamic_eq_st;
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    bool drc_update;
    DrcConfig drc_cfg;
#endif

#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    bool dynamic_boost_update;
    DynamicBoostConfig dynamic_boost_cfg;
#endif

#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    bool dynamic_eq_update;
    DynamicEqConfig dynamic_eq_cfg;
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    bool limiter_update;
    LimiterConfig limiter_cfg;
#endif

#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    bool hw_limiter_update;
    struct_psap_cfg hw_limiter_cfg;
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    bool reverb_update;
    ReverbConfig reverb_cfg;
#endif

#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    bool bass_enhancer_update;
    BassEnhancerConfig bass_enhancer_cfg;
#endif

#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    bool virtual_surround_update;
    VirtualSurroundConfig virtual_surround_cfg;
#endif

#ifdef AUDIO_CUSTOM_EQ
    uint32_t nfft;
    float *work_buffer;
#endif
} AUDIO_PROCESS_T;

static AUDIO_PROCESS_T audio_process = {
    .sample_bits = AUD_BITS_24,
    .sample_rate = AUD_SAMPRATE_44100,
    .sw_ch_num = AUD_CHANNEL_NUM_2,
    .hw_ch_num = AUD_CHANNEL_NUM_2,

#if defined(__SW_IIR_EQ_PROCESS__)
    .sw_iir_enable =  false,
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    .hw_dac_iir_enable =  false,
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
    .hw_iir_enable =  false,
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
    .hw_fir_enable =  false,
#endif

#ifdef AUDIO_HEARING_COMPSATN
#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
    .hear_iir_cfg = {.num = 0},
    .hear_iir_cfg_update = {.num = 0},
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    .hear_iir_cfg_2 = {.num = 0},
    .hear_iir_cfg_update_2 = {.num = 0},
#endif
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    .hear_hw_fir_cfg = {.len = 0},
#endif
#endif
#endif


#ifdef AUDIO_UPDATE_CFG
    .update_cfg = false,
#endif

#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    .sw_iir_cfg = {.num = 0},
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    .hw_dac_iir_cfg = {.num = 0},
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    .hw_dac_iir_cfg2 = {.num = 0},
#endif
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    .hw_iir_cfg = {.num = 0},
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    .hw_fir_cfg = {.len = 0},
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    .drc_update = false,
    .drc_cfg = {
        .knee = 0,
        .filter_type = {-1, -1},
        .band_num = 1,
        .look_ahead_time = 0,
        .band_settings = {
            {0, 0, 1, 1, 1, 1},
            {0, 0, 1, 1, 1, 1},
        }
    },
#endif

#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    .dynamic_boost_update = false,
    .dynamic_boost_cfg = {
        .debug = 0,
        .xover_freq = {200},
        .order = 4,
        .CT = -40,
        .CS = 0.18,
        .WT = -40,
        .WS = 0.3,
        .ET = -60,
        .ES = 0,
        .attack_time        = 0.0001f,
        .release_time       = 0.0001f,
        .makeup_gain        = -6,
        .delay              = 128,
        .tav                = 1.0f,
        .eq_num = 2,
        .boost_eq = {
            {
                .gain = 10,
                .freq = 33,
                .Q = 0.5,
            },
            {
                .gain = -1,
                .freq = 240,
                .Q = 1.1,
            },
            {
                .gain = 10,
                .freq = 1000, // -1 for unused eq
                .Q = 0.7,
            },
            {
                .gain = 10,
                .freq = 2000, // -1 for unused eq
                .Q = 0.7,
            }
        },
    },
#endif

#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    .dynamic_eq_update = false,
    .dynamic_eq_cfg = {
        .switch_on = 1,
        .debug = 0,
        .eq_num = 6,
        .dyeq_param_cfg = {
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 6.0,   100.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 0.0,   500.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 0.0,   2000.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 0.0,   4000.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 0.0,   6000.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
            {
                .dyeq_eq_cfg = {IIR_TYPE_PEAK, 0.0,   8000.0,   1.0},
                .dyeq_drc_cfg = {-50.0,   2.0,   -70.0,   1.0,   -70.0,   1.0,   20.0,   200.0},
            },
        }
    },
#endif

#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    .virtual_surround_update = false,
    .virtual_surround_cfg = {
        .switch_on = false,
        .pre_delay_1 = 115,
        .pre_delay_2 = 215,
        .pre_delay_3 = 215,
        .allpass_len1 = 400,
        .allpass_len2 = 353,
        .allpass_len3 = 215,
        .allpass_len4 = 400,
        .allpass_len5 = 130,
        .allpass_len6 = 400,

        // percent
        .reverb_input_ratio_1 = 100,
        .reverb_input_ratio_2 = 100,

        .k1 = 50,
        .k2 = 40,
        .k3 = 50,
        .k4 = 40,
        .k5 = 20,
        .k6 = 100,

        .center_channel_ratio = 30,
        .surround_channel_ratio = 100,
        .output_ratio = 120,

        .hrtf_input_ratio = 100,
        .hrtf_output_ratio = 100,
        .hrtf_select_index = 1,
    },
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    .limiter_update = false,
    .limiter_cfg = {
        .knee = 0,
        .look_ahead_time = 0,
        .threshold = 0,
        .makeup_gain = 0,
        .ratio = 1000,
        .attack_time = 1,
        .release_time = 1,
    },
#endif

#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    .hw_limiter_update = false,
    .hw_limiter_cfg = {
        .psap_cfg_l = {
            .psap_total_gain = 512,
            .psap_band_num = 2,
            .psap_band_gain={0x08000000, 0x08000000},
            .psap_iir_coef[0].iir0.coef_b={0x07f4eae6, 0xf00b1afc, 0x08000000, 0x00000000},
            .psap_iir_coef[0].iir0.coef_a={0x08000000, 0xf00b1afc, 0x07f4eae6, 0x00000000},
            .psap_iir_coef[0].iir1.coef_b={0xf80b151a, 0x17e9cc4a, 0xe80b1ea1, 0x08000000},
            .psap_iir_coef[0].iir1.coef_a={0x08000000, 0xe80b1ea1, 0x17e9cc4a, 0xf80b151a},
            .psap_cpd_cfg[0]={0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
            .psap_limiter_cfg={524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0},
            .psap_dehowling_cfg.dehowling_delay=0.0,
            .psap_dehowling_cfg.dehowling_l.total_gain=0,
            .psap_dehowling_cfg.dehowling_l.iir_bypass_flag=0,
            .psap_dehowling_cfg.dehowling_l.iir_counter=1,
            .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b={0x08000000, 0xf08d9c03, 0x077b49d0},
            .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a={0x08000000, 0xf08d9c03, 0x077b49d0},
            .psap_dehowling_cfg.dehowling_l.dac_gain_offset=0,
            .psap_dehowling_cfg.dehowling_l.adc_gain_offset=0,
            .psap_type = 3,
            .psap_dac_gain_offset=0,
            .psap_adc_gain_offset=-24,
        }
    }
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    .reverb_update = false,
    .reverb_cfg = {
        .bypass = 1,
        .high_pass_f0 = 100,
        .gain = 0,
    },
#endif

#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    .bass_enhancer_update = false,
    .bass_enhancer_cfg = {
        .switch_on = false,
        .low_cut_freq = 100,
        .high_cut_freq = 700,
        .gain0 = 0,
        .gain1 = 7.95,
    },
#endif

};

uint32_t audio_process_need_audio_buf_size(void)
{
    return AUDIO_MEMORY_SIZE;
}

#ifdef __AUDIO_DYNAMIC_BOOST__
static IIR_CFG_T g_audio_dynamcic_boost_iir_eq_running_cfg;
static int32_t curr_dynamic_boost_status = 0;
static int32_t next_dynamic_boost_status = 0;

int audio_dynamic_boost_set_dynamic_level(int val) 
{
    if(audio_process.dynamic_boost_st == NULL){
        return 0;
    }
    TRACE(1,"[%s] %d", __func__, val);

    dynamic_boost_set_dynamic_level(audio_process.dynamic_boost_st, val);
    return 0;
}

int audio_dynamic_boost_switch(int val)
{
    TRACE(1,"[%s] %d", __func__, val);
    next_dynamic_boost_status = val;

    return 0;
}

static IIR_CFG_T customer_iir_cfg_for_dynamic_boost;
static AUDIO_EQ_TYPE_T customer_iir_type_for_dynamic_boost;

int audio_dynamic_boost_set_new_customer_iir_eq(const IIR_CFG_T *customer_iir_cfg, const AUDIO_EQ_TYPE_T customer_iir_type)
{
    memcpy(&customer_iir_cfg_for_dynamic_boost, customer_iir_cfg, sizeof(IIR_CFG_T));
    customer_iir_type_for_dynamic_boost = customer_iir_type;
    dynamic_boost_set_new_eq_compensation(audio_process.dynamic_boost_st, &customer_iir_cfg_for_dynamic_boost);
    return 0;
}

#endif

#ifdef __AUDIO_DYNAMIC_EQ__
static IIR_CFG_T g_audio_dynamcic_eq_iir_eq_running_cfg;
#endif

void audio_process_iir_eq_mix(IIR_CFG_T *iir_cfg_mix)
{
#ifdef __AUDIO_DYNAMIC_BOOST__
    iir_cfg_mix->gain0 += g_audio_dynamcic_boost_iir_eq_running_cfg.gain0;
    iir_cfg_mix->gain1 += g_audio_dynamcic_boost_iir_eq_running_cfg.gain1;
    ASSERT(iir_cfg_mix->num + g_audio_dynamcic_boost_iir_eq_running_cfg.num <= IIR_PARAM_NUM, 
    "[%s] %d exceeds maximum EQ band number!", __func__, iir_cfg_mix->num + g_audio_dynamcic_boost_iir_eq_running_cfg.num);
    for(int i = 0; i < g_audio_dynamcic_boost_iir_eq_running_cfg.num; i++) {
        iir_cfg_mix->param[i + iir_cfg_mix->num] = g_audio_dynamcic_boost_iir_eq_running_cfg.param[i];
    }

    iir_cfg_mix->num += g_audio_dynamcic_boost_iir_eq_running_cfg.num;
#endif

#if defined(AUDIO_ADAPTIVE_IIR_EQ)
    iir_cfg_mix.gain0 += audio_eq_hw_dac_iir_adaptive_eq_cfg.gain0;
    iir_cfg_mix.gain1 += audio_eq_hw_dac_iir_adaptive_eq_cfg.gain1;
    ASSERT(iir_cfg_mix.num + audio_eq_hw_dac_iir_adaptive_eq_cfg.num <= IIR_PARAM_NUM, "[%s] %d exceeds maximum EQ band number!",
                            __func__, iir_cfg_mix.num + audio_eq_hw_dac_iir_adaptive_eq_cfg.num);
    for(int i=0; i<audio_eq_hw_dac_iir_adaptive_eq_cfg.num; i++){
        iir_cfg_mix.param[i+iir_cfg_mix.num] = audio_eq_hw_dac_iir_adaptive_eq_cfg.param[i];
    }

    iir_cfg_mix.num += audio_eq_hw_dac_iir_adaptive_eq_cfg.num;
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
    iir_cfg_mix->gain0 += g_audio_dynamcic_eq_iir_eq_running_cfg.gain0;
    iir_cfg_mix->gain1 += g_audio_dynamcic_eq_iir_eq_running_cfg.gain1;
    ASSERT(iir_cfg_mix->num + g_audio_dynamcic_eq_iir_eq_running_cfg.num <= IIR_PARAM_NUM, 
    "[%s] %d exceeds maximum EQ band number!", __func__, iir_cfg_mix->num + g_audio_dynamcic_eq_iir_eq_running_cfg.num);
    for(int i = 0; i < g_audio_dynamcic_eq_iir_eq_running_cfg.num; i++) {
        iir_cfg_mix->param[i + iir_cfg_mix->num] = g_audio_dynamcic_eq_iir_eq_running_cfg.param[i];
    }

    iir_cfg_mix->num += g_audio_dynamcic_eq_iir_eq_running_cfg.num;
#endif
    // Add more IIR EQ MIX case
}

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
HW_CODEC_IIR_CFG_T hw_iir_cfg_dac_total;
#endif

#if defined(__HW_FIR_EQ_PROCESS_2CH__)
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const FIR_CFG_T *fir_cfg_2,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type)
#elif defined(__IIR_EQ_PROCESS_LR_2CH__)
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const IIR_CFG_T *iir_cfg,const IIR_CFG_T *iir_cfg_2,AUDIO_EQ_TYPE_T audio_eq_type)
#else
int audio_eq_set_cfg(const FIR_CFG_T *fir_cfg,const IIR_CFG_T *iir_cfg,AUDIO_EQ_TYPE_T audio_eq_type)
#endif
{
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_FIR_EQ_PROCESS__)|| defined(__HW_DAC_IIR_EQ_PROCESS__)|| defined(__HW_IIR_EQ_PROCESS__)
    switch (audio_eq_type)
    {
#if defined(__SW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_SW_IIR:
        {
            if(iir_cfg)
            {
                audio_process.sw_iir_enable = false;
#ifdef USB_EQ_TUNING
                if (audio_process.eq_updated_cfg) {
                    iir_set_cfg(&audio_process.sw_iir_cfg);
                } else
#endif
                {
#if defined(AUDIO_HEARING_COMPSATN) && SWIIR_MOD==HEARING_MOD_VAL
                    iir_set_cfg(&audio_process.hear_iir_cfg_update);
#elif defined(__AUDIO_DYNAMIC_BOOST__) && defined(DYNAMIC_BOOST_USE_SW_EQ)
                    IIR_CFG_T iir_cfg_mix;
                    memcpy(&iir_cfg_mix, iir_cfg, sizeof(IIR_CFG_T));
                    audio_process_iir_eq_mix(&iir_cfg_mix);
                    iir_set_cfg(&iir_cfg_mix);
#else
                    iir_set_cfg(iir_cfg);
#endif
                }
                audio_process.sw_iir_enable = true;
            }
            else
            {
                audio_process.sw_iir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_FIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_FIR:
        {
            if(fir_cfg)
            {
                audio_process.hw_fir_enable = false;
#if defined(AUDIO_HEARING_COMPSATN) && HWFIR_MOD==HEARING_MOD_VAL
                fir_set_cfg(&audio_process.hear_hw_fir_cfg);
#elif __HW_FIR_EQ_PROCESS_2CH__
                fir_set_cfg_ch(fir_cfg,AUD_CHANNEL_NUM_1);
                fir_set_cfg_ch(fir_cfg_2,AUD_CHANNEL_NUM_2);
#else

                fir_set_cfg(fir_cfg);
#endif
                audio_process.hw_fir_enable = true;
            }
            else
            {
                audio_process.hw_fir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_DAC_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_DAC_IIR:
        {
            if(iir_cfg)
            {
                HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac=NULL;
                enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
                sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_process.sample_rate,1);
                TRACE(3,"audio_process.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_process.sample_rate, sample_rate_hw_dac_iir);
#else
                sample_rate_hw_dac_iir=audio_process.sample_rate;
#endif
                audio_process.hw_dac_iir_enable = false;
#ifdef USB_EQ_TUNING
                if (audio_process.eq_updated_cfg) {
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &audio_process.hw_dac_iir_cfg);
                } else
#endif
                {
#if defined(AUDIO_HEARING_COMPSATN) && HWIIR_MOD==HEARING_MOD_VAL
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_process.hear_iir_cfg_update);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_l),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));

                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_process.hear_iir_cfg_update_2);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_r),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));
#else
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,iir_cfg);
#endif
#elif defined(__AUDIO_DYNAMIC_EQ__) || (defined(__AUDIO_DYNAMIC_BOOST__) && defined(DYNAMIC_BOOST_USE_HW_EQ))
                    IIR_CFG_T iir_cfg_mix;
                    memcpy(&iir_cfg_mix, iir_cfg, sizeof(IIR_CFG_T));
                    audio_process_iir_eq_mix(&iir_cfg_mix);
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &iir_cfg_mix);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_l),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));

                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, iir_cfg_2);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_r),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));
#else
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &iir_cfg_mix);
#endif
#else
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,iir_cfg);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_l),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));

                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,iir_cfg_2);
                    memcpy(&(hw_iir_cfg_dac_total.iir_filtes_r),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));
#else
                    hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, iir_cfg);
#endif
#endif
                }
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
                // ASSERT(hw_iir_cfg_dac_total != NULL, "[%s] codec IIR parameter error!", __func__);
                hw_codec_iir_set_cfg(&hw_iir_cfg_dac_total, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
#else
                ASSERT(hw_iir_cfg_dac != NULL, "[%s] codec IIR parameter error!", __func__);
                hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
#endif
                audio_process.hw_dac_iir_enable = true;
            }
            else
            {
                audio_process.hw_dac_iir_enable = false;
            }
        }
        break;
#endif

#if defined(__HW_IIR_EQ_PROCESS__)
        case AUDIO_EQ_TYPE_HW_IIR:
        {
            if(iir_cfg)
            {
                HW_IIR_CFG_T *hw_iir_cfg=NULL;
                audio_process.hw_iir_enable = false;
                hw_iir_cfg = hw_iir_get_cfg(audio_process.sample_rate,iir_cfg);
                ASSERT(hw_iir_cfg != NULL,"[%s] 0x%x codec IIR parameter error!", __func__, (unsigned int)hw_iir_cfg);
                hw_iir_set_cfg(hw_iir_cfg);
                audio_process.hw_iir_enable = true;
            }
            else
            {
                audio_process.hw_iir_enable = false;
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

    return 0;
}

#ifdef __AUDIO_BASS_ENHANCER__
static int32_t curr_virtual_bass_status = 0xff;
static int32_t next_virtual_bass_status = 0xff;
void audio_virtual_bass_switch(bool onoff)
{
    TRACE(1,"[%s] audio_virtual_bass_switch %d", __func__, onoff);
    next_virtual_bass_status = onoff;
}

void audio_virtual_bass_set_freq(int32_t val){
    TRACE(0,"%s val = %d",__func__,val);
    if(val == 1){
        stereo_surround_freq_controller(APP_SYSFREQ_104M);
    } else { // back to origin freq
        stereo_surround_freq_controller(APP_SYSFREQ_52M);
    }
}

#endif

#ifdef __AUDIO_DYNAMIC_EQ__
static int32_t curr_dynamic_eq_status = 0xff;
static int32_t next_dynamic_eq_status = 0xff;

int audio_dynamic_eq_set_dynamic_level(float *val) 
{
    if(audio_process.dynamic_eq_st == NULL){
        return 0;
    }

    TRACE(4,"[%s] %d, %d, %d", __func__, (int)val[0], (int)val[1], (int)val[2]);
    dynamic_eq_set_dynamic_level(audio_process.dynamic_eq_st, val);
    return 0;
}

int audio_dynamic_eq_switch(int val)
{
    TRACE(1,"[%s] %d", __func__, val);
    next_dynamic_eq_status = val;
    return 0;
}

static IIR_CFG_T customer_iir_cfg_for_dynamic_eq;

int audio_dynamic_eq_set_new_customer_eq(const IIR_CFG_T *customer_iir_cfg)
{
    memcpy(&customer_iir_cfg_for_dynamic_eq, customer_iir_cfg, sizeof(IIR_CFG_T));
    return 0;
}

#endif


#ifdef AUDIO_PROCESS_DUMP
short dump_buf[1024] = {0};
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
#include "a2dp_decoder.h"
static int nv_role = 0;
// static bool virtual_surround_open_flag = false;
static int32_t curr_surround_status = 0xff;
static int32_t next_surround_status = 0xff;
#endif
int SRAM_TEXT_LOC audio_process_run(uint8_t *buf, uint32_t len)
{
    int POSSIBLY_UNUSED pcm_len = 0;

    if(audio_process.sample_bits == AUD_BITS_16)
    {
        pcm_len = len / sizeof(pcm_16bits_t);
    }
    else if(audio_process.sample_bits == AUD_BITS_24)
    {
        pcm_len = len / sizeof(pcm_24bits_t);
    }
    else
    {
        ASSERT(0, "[%s] bits(%d) is invalid", __func__, audio_process.sample_bits);
    }



#if defined(AUDIO_OUTPUT_SW_GAIN) && defined(AUDIO_OUTPUT_SW_GAIN_BEFORE_DRC)
    af_codec_dac1_sw_gain_process(buf, len, audio_process.sample_bits, audio_process.hw_ch_num);
#endif

#ifdef __AUDIO_BASS_ENHANCER__
#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    if(audio_process.bass_enhancer_update)
    {
        TRACE(1,"[%s] bass_enhancer_update: %d", __func__,audio_process.bass_enhancer_update);
        bass_enhancer_set_cfg(audio_process.bass_st, &audio_process.bass_enhancer_cfg);
        audio_process.bass_enhancer_update =false;
    }
#endif

    if (next_virtual_bass_status != curr_virtual_bass_status) {
        if(audio_process.bass_st != NULL){
            bass_enhancer_switch(audio_process.bass_st, next_virtual_bass_status);
            curr_virtual_bass_status = next_virtual_bass_status;
        }else{
            TRACE(0,"%s warning, BASS_ENHANCER is not created",__func__);
        }
    }
    bass_enhancer_process(audio_process.bass_st, buf, pcm_len);
#endif

#ifdef __AUDIO_REVERB__
    reverb_preprocess(audio_process.reverb_st, buf, len);
#endif


#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    if(audio_process.virtual_surround_update)
    {
        TRACE(1, "[%s] virtual_surround_update: %d", __func__, audio_process.virtual_surround_update);
        stereo_surround_set_config(stereo_surround_st, &audio_process.virtual_surround_cfg);
        audio_process.virtual_surround_update = false;
    }
#endif

    if (next_surround_status != curr_surround_status) {
        if(stereo_surround_st != NULL){
            if(next_surround_status == 1){
                stereo_surround_fir_onoff(stereo_surround_st,next_surround_status);
                a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_STEREO); 
            }
            stereo_surround_onoff(stereo_surround_st, next_surround_status);
            curr_surround_status = next_surround_status;
        } else {
            TRACE(0,"%s warning, module is not created",__func__);
        }
    }
    stereo_surround_preprocess(stereo_surround_st,buf,pcm_len);
#elif defined(__VIRTUAL_SURROUND_CP__)
    stereo_surround_preprocess_ap(stereo_surround_st,buf,pcm_len);
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    if(audio_process.dynamic_boost_update)
    {
        TRACE(1,"[%s] dynamic_boost_update: %d", __func__, audio_process.dynamic_boost_update);
        dynamic_boost_set_config(audio_process.dynamic_boost_st, &audio_process.dynamic_boost_cfg);
        audio_process.dynamic_boost_update =false;
    }
#endif

    if (next_dynamic_boost_status != curr_dynamic_boost_status) {
        if(audio_process.dynamic_boost_st != NULL){
            dynamic_boost_switch(audio_process.dynamic_boost_st, next_dynamic_boost_status);
            curr_dynamic_boost_status = next_dynamic_boost_status;
        }else{
            TRACE(0,"%s warning, DYNAMIC_BOOST is not created",__func__);
        }
    }

    int vol_level = app_bt_stream_local_volume_get();
    dynamic_boost_process(audio_process.dynamic_boost_st, buf, pcm_len, vol_level);

    // update eq cfg
    if(dynamic_boost_get_eq_update_flag(audio_process.dynamic_boost_st) == 1) {
        IIR_CFG_T * dynamic_boost_iir_eq_running_cfg = dynamic_boost_get_iir_eq_running_cfg(audio_process.dynamic_boost_st);
        memcpy(&g_audio_dynamcic_boost_iir_eq_running_cfg, dynamic_boost_iir_eq_running_cfg, sizeof(IIR_CFG_T));
        dynamic_boost_set_eq_update_flag(audio_process.dynamic_boost_st, 0);

        // merge customer eq cfg
        audio_eq_set_cfg(NULL, &customer_iir_cfg_for_dynamic_boost, customer_iir_type_for_dynamic_boost);
    }
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    if(audio_process.dynamic_eq_update)
    {
        TRACE(1,"[%s] dynamic_eq_update: %d", __func__, audio_process.dynamic_eq_update);
        dynamic_eq_set_config(audio_process.dynamic_eq_st, &audio_process.dynamic_eq_cfg);
        audio_process.dynamic_eq_update =false;
    }
#endif

    if (next_dynamic_eq_status != curr_dynamic_eq_status) {
        if(audio_process.dynamic_eq_st != NULL){
            dynamic_eq_switch(audio_process.dynamic_eq_st, next_dynamic_eq_status);
            curr_dynamic_eq_status = next_dynamic_eq_status;
        }else{
            TRACE(0,"%s warning, DYNAMIC EQ is not created",__func__);
        }
    }

    int vol_level = app_bt_stream_local_volume_get();
    dynamic_eq_process(audio_process.dynamic_eq_st, buf, pcm_len, vol_level);

    // update eq cfg
    if(dynamic_eq_get_eq_update_flag(audio_process.dynamic_eq_st) == 1) {
        IIR_CFG_T * dynamic_eq_iir_eq_running_cfg = dynamic_eq_get_iir_eq_running_cfg(audio_process.dynamic_eq_st);
        memcpy(&g_audio_dynamcic_eq_iir_eq_running_cfg, dynamic_eq_iir_eq_running_cfg, sizeof(IIR_CFG_T));
        dynamic_eq_set_eq_update_flag(audio_process.dynamic_eq_st, 0);

        // merge customer eq cfg
        audio_eq_set_cfg(NULL, &customer_iir_cfg_for_dynamic_eq, AUDIO_EQ_TYPE_HW_DAC_IIR);
    }
#endif

    if (audio_process.sw_ch_num == audio_process.hw_ch_num) {
        // do nothing
    } else if (audio_process.sw_ch_num == AUD_CHANNEL_NUM_1 &&
               audio_process.hw_ch_num == AUD_CHANNEL_NUM_2) {
        if (audio_process.sample_bits == AUD_BITS_16) {
            int16_t *pcm_buf = (int16_t *)buf;
            for (uint32_t i = 0, j = 0; i < pcm_len; i += 2, j++) {
                pcm_buf[j] = pcm_buf[i];
            }
        } else {
            int32_t *pcm_buf = (int32_t *)buf;
            for (uint32_t i = 0, j = 0; i < pcm_len; i += 2, j++) {
                pcm_buf[j] = pcm_buf[i];
            }
        }

        pcm_len /= 2;
        len /= 2;
    } else {
        ASSERT(0, "[%s] sw_ch_num(%d) or hw_ch_num(%d) is invalid", __FUNCTION__, audio_process.sw_ch_num, audio_process.hw_ch_num);
    }

#ifdef AUDIO_DSP_ACCEL
    APP_MCPP_PLAY_PCM_T pcm_cfg;
    memset(&pcm_cfg, 0, sizeof(pcm_cfg));
    pcm_cfg.in = buf;
    pcm_cfg.out = buf;
    pcm_cfg.frame_len = pcm_len;
    app_mcpp_playback_process(APP_MCPP_USER_AUDIO, &pcm_cfg);
#endif

#ifdef AUDIO_PROCESS_DUMP
    int *buf32 = (int *)buf;
    for(int i=0;i<1024;i++)
        dump_buf[i] = buf32[2 * i]>>8;
    audio_dump_clear_up();
    audio_dump_add_channel_data(0, dump_buf, 1024);

#endif

    //int32_t s_time,e_time;
    //s_time = hal_fast_sys_timer_get();

#ifdef __SW_IIR_EQ_PROCESS__
    if(audio_process.sw_iir_enable)
    {
        iir_run(buf, pcm_len);
    }
#endif

#ifdef __HW_FIR_EQ_PROCESS__
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
    // surround will do it inside 
#else
    if(audio_process.hw_fir_enable)
    {
        fir_run(buf, pcm_len);
    }
#endif
#endif

#ifdef AUDIO_PROCESS_DUMP
    for(int i=0;i<1024;i++)
        dump_buf[i] = buf32[2 * i+0]>>8;
    audio_dump_add_channel_data(1, dump_buf, 1024);
#endif


#ifdef __AUDIO_DRC__
#ifdef AUDIO_DRC_UPDATE_CFG
	if(audio_process.drc_update)
	{
        drc_set_config(audio_process.drc_st, &audio_process.drc_cfg);
        audio_process.drc_update =false;
	}
#endif

	drc_process(audio_process.drc_st, buf, pcm_len);
#endif

    // int32_t m_time = hal_fast_sys_timer_get();
#ifdef __AUDIO_LIMITER__
#ifdef AUDIO_LIMITER_UPDATE_CFG
    if(audio_process.limiter_update)
    {
        limiter_set_config(audio_process.limiter_st, &audio_process.limiter_cfg);
        audio_process.limiter_update =false;
    }
#endif

    limiter_process(audio_process.limiter_st, buf, pcm_len);
#endif

#ifdef __AUDIO_HW_LIMITER__
#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    if(audio_process.hw_limiter_update)
    {
        if(psap_opened())
        {
            psap_set_cfg_coef(&audio_process.hw_limiter_cfg);
        }
        audio_process.hw_limiter_update = false;
    }
#endif
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    if(audio_process.hw_iir_enable)
    {
        hw_iir_run(buf, pcm_len);
    }
#endif

#ifdef __AUDIO_REVERB__
#ifdef AUDIO_REVERB_UPDATE_CFG
    if(audio_process.reverb_update)
    {
        reverb_set_config(audio_process.reverb_st, &audio_process.reverb_cfg);
        audio_process.reverb_update = false;
    }
#endif

    reverb_process(audio_process.reverb_st, buf, len);
#endif

    if (audio_process.sw_ch_num == audio_process.hw_ch_num) {
        // do nothing
    } else if (audio_process.sw_ch_num == AUD_CHANNEL_NUM_1 &&
        audio_process.hw_ch_num == AUD_CHANNEL_NUM_2) {
        if (audio_process.sample_bits == AUD_BITS_16) {
            int16_t *pcm_buf = (int16_t *)buf;
            for (int32_t i = pcm_len - 1, j = 2 * pcm_len - 2; i >= 0; i--, j -= 2) {
                pcm_buf[j + 1] = pcm_buf[i];
                pcm_buf[j + 0] = pcm_buf[i];
            }
        } else {
            int32_t *pcm_buf = (int32_t *)buf;
            for (int32_t i = pcm_len - 1, j = 2 * pcm_len -  2; i >= 0; i--, j -= 2) {
                pcm_buf[j + 1] = pcm_buf[i];
                pcm_buf[j + 0] = pcm_buf[i];
            }
        }

        pcm_len *= 2;
        len *= 2;
    } else {
        ASSERT(0, "[%s] sw_ch_num(%d) or hw_ch_num(%d) is invalid", __FUNCTION__, audio_process.sw_ch_num, audio_process.hw_ch_num);
    }

#ifdef AUDIO_PROCESS_DUMP
        //for(int i=0;i<1024;i++)
        //    dump_buf[i] = buf32[2 * i+0]>>8;
        //audio_dump_add_channel_data(1, dump_buf, 1024);
        audio_dump_run();
#endif

    //e_time = hal_fast_sys_timer_get();
    //TRACE(4,"[%s] Sample len = %d, drc1 %d us, limiter %d us",
    //    __func__, pcm_len, FAST_TICKS_TO_US(m_time - s_time), FAST_TICKS_TO_US(e_time - m_time));

#ifdef DIRAC_AUDIO_ENABLE
    int32_t s_time1,e_time1;
    s_time1 = hal_fast_sys_timer_get();
    dirac_process(buf, pcm_len);
    e_time1 = hal_fast_sys_timer_get();
    TRACE(4,"[%s] dirac time = %d us", __func__, FAST_TICKS_TO_US(e_time1 - s_time1));
#endif

    return 0;
}

/*
 * frame_size stands for samples per channel
 */
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
#include "app_tws_ibrt.h"
#include "app_utils.h"

int32_t audio_process_stereo_set_yaw(float yaw){
    if(stereo_surround_st == NULL){
        return 0;
    }
    stereo_surround_set_yaw_angle(stereo_surround_st, 57.3*yaw);
    // stereo_surround_set_yaw_angle(stereo_surround_st, yaw);
    return 0;
}

int32_t audio_process_stereo_set_pitch(float pitch){
    if(stereo_surround_st == NULL){
        return 0;
    }
    stereo_surround_set_pitch_angle(stereo_surround_st, 57.3*pitch);
    return 0;
}

extern void app_audio_set_a2dp_freq(uint32_t freq);
int32_t audio_process_stereo_surround_onoff(int32_t onoff){
    TRACE(0,"%s onoff = %d",__func__,onoff);
    next_surround_status = onoff;
    return 0;
}
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
int32_t stereo_surround_set_freq(int onoff){
    TRACE(0,"%s onoff = %d",__func__,onoff);
    if(onoff == 1){
        if(stereo_surround_is_ldac()){ // 96k to 208M
            stereo_surround_freq_controller(APP_SYSFREQ_208M);
            // app_audio_set_a2dp_freq(APP_SYSFREQ_208M);
        } else { // 44.1/48 to 104M
#if defined(__VIRTUAL_SURROUWND_HWFIR__)
            stereo_surround_freq_controller(APP_SYSFREQ_52M);
#else
            stereo_surround_freq_controller(APP_SYSFREQ_104M);
#endif
            // app_audio_set_a2dp_freq(APP_SYSFREQ_104M);
        }
        
    } else { // back to origin freq
#if !defined(__VIRTUAL_SURROUND_STEREO__)
        if (app_tws_ibrt_get_bt_ctrl_ctx()->nv_role == IBRT_SLAVE){
            a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_LCHNL);  // left
        } else{
            a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_RCHNL);  // right
        }
#endif
        if(stereo_surround_is_ldac()){ // 96k to 104M
            stereo_surround_freq_controller(APP_SYSFREQ_104M);
            // app_audio_set_a2dp_freq(APP_SYSFREQ_104M);
        } else { // 44.1/48 to 26M
            stereo_surround_freq_controller(APP_SYSFREQ_26M);
            // app_audio_set_a2dp_freq(APP_SYSFREQ_26M);
        }
    }
    return 0;
}
#elif defined(__VIRTUAL_SURROUND_CP__)
int32_t stereo_surround_set_freq(int onoff){
    TRACE(0,"%s onoff = %d",__func__,onoff);
    if(onoff == 1){
        if(stereo_surround_is_ldac() == 0){ // aac sbc 44.1
            stereo_surround_freq_controller(APP_SYSFREQ_52M);
        } else if(stereo_surround_is_ldac() == 1){ // ldac 48k
            stereo_surround_freq_controller(APP_SYSFREQ_104M);
        } else { // ldac 96k
            stereo_surround_freq_controller(APP_SYSFREQ_208M);
        }
    } else { // back to origin freq
        if (app_tws_ibrt_get_bt_ctrl_ctx()->nv_role == IBRT_SLAVE){
            a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_LCHNL);  // left
        } else{
            a2dp_audio_set_channel_select(A2DP_AUDIO_CHANNEL_SELECT_RCHNL);  // right
        }
        if(stereo_surround_is_ldac() == 0){ // aac sbc 44.1
            stereo_surround_freq_controller(APP_SYSFREQ_26M);
        } else if(stereo_surround_is_ldac() == 1){ // ldac 48k
            stereo_surround_freq_controller(APP_SYSFREQ_52M);
        } else { // ldac 96k
            stereo_surround_freq_controller(APP_SYSFREQ_104M);
        }
    }
    return 0;
}
#endif

int audio_process_open(enum AUD_SAMPRATE_T sample_rate, enum AUD_BITS_T sample_bits, enum AUD_CHANNEL_NUM_T sw_ch_num, enum AUD_CHANNEL_NUM_T hw_ch_num, int32_t frame_size, void *eq_buf, uint32_t len)
{
    TRACE(5,"[%s] sample_rate = %d, sample_bits = %d, sw_ch_num = %d, hw_ch_num = %d", __func__, sample_rate, sample_bits, sw_ch_num, hw_ch_num);
#ifdef AUDIO_PROCESS_DUMP
    audio_dump_init(1024, sizeof(short), 2);
#endif
    audio_process.sample_rate = sample_rate;
    audio_process.sample_bits = sample_bits;
    audio_process.sw_ch_num = sw_ch_num;
    audio_process.hw_ch_num = hw_ch_num;

#if defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
    void *fir_eq_buf = eq_buf;
    uint32_t fir_len = len/2;
    void *iir_eq_buf = (uint8_t *)eq_buf+fir_len;
    uint32_t iir_len = len/2;
#elif defined(__HW_FIR_EQ_PROCESS__) && !defined(__HW_IIR_EQ_PROCESS__)
    void *fir_eq_buf = eq_buf;
    uint32_t fir_len = len;
#elif !defined(__HW_FIR_EQ_PROCESS__) && defined(__HW_IIR_EQ_PROCESS__)
    void *iir_eq_buf = eq_buf;
    uint32_t iir_len = len;
#endif

#ifdef __SW_IIR_EQ_PROCESS__
    iir_open(sample_rate, sample_bits,sw_ch_num);
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    audio_eq_set_cfg(NULL, &audio_process.sw_iir_cfg, AUDIO_EQ_TYPE_SW_IIR);
#endif
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
    sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_process.sample_rate,1);
   TRACE(3,"audio_process.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_process.sample_rate, sample_rate_hw_dac_iir);
#else
    sample_rate_hw_dac_iir = audio_process.sample_rate;
#endif

#ifdef __AUDIO_EQ_DYNAMICS_MEM__
    syspool_get_buff((uint8_t **)&l_old, sizeof(HW_CODEC_IIR_FILTERS_T));
    syspool_get_buff((uint8_t **)&cfg_l, sizeof(HW_CODEC_IIR_FILTERS_T));
    syspool_get_buff((uint8_t **)&new_l, sizeof(HW_CODEC_IIR_FILTERS_T));
    if(hw_ch_num == 2){
        syspool_get_buff((uint8_t **)&r_old, sizeof(HW_CODEC_IIR_FILTERS_T));
        syspool_get_buff((uint8_t **)&cfg_r, sizeof(HW_CODEC_IIR_FILTERS_T));
        syspool_get_buff((uint8_t **)&new_r, sizeof(HW_CODEC_IIR_FILTERS_T));
    }
    hal_codec_iir_heap_register(l_old,r_old,cfg_l,cfg_r,new_l,new_r);
#endif
    hw_codec_iir_open(sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC, CODEC_OUTPUT_DEV);
#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    audio_eq_set_cfg(NULL, &audio_process.hw_dac_iir_cfg, &audio_process.hw_dac_iir_cfg2, AUDIO_EQ_TYPE_HW_DAC_IIR);
#else
    audio_eq_set_cfg(NULL, &audio_process.hw_dac_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    hw_iir_open(sample_rate, sample_bits, sw_ch_num, iir_eq_buf, iir_len);
#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    audio_eq_set_cfg(NULL, &audio_process.hw_iir_cfg, AUDIO_EQ_TYPE_HW_IIR);
#endif
#endif

#ifdef __HW_FIR_EQ_PROCESS__
#if defined(CHIP_BEST1000) && defined(FIR_HIGHSPEED)
    hal_cmu_fir_high_speed_enable(HAL_CMU_FIR_USER_EQ);
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
    if(0) fir_open(sample_rate, sample_bits, sw_ch_num, fir_eq_buf, fir_len);
#else
    fir_open(sample_rate, sample_bits, sw_ch_num, fir_eq_buf, fir_len);
#endif
#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    audio_eq_set_cfg(&audio_process.hw_fir_cfg, NULL, AUDIO_EQ_TYPE_HW_FIR);
#endif
#endif

#if AUDIO_MEMORY_SIZE > 0
    syspool_get_buff(&audio_process.audio_heap, AUDIO_MEMORY_SIZE);
    audio_heap_init(audio_process.audio_heap, AUDIO_MEMORY_SIZE);
#endif

#if SPEECH_MEMORY_SIZE > 0
    syspool_get_buff(&audio_process.speech_heap, SPEECH_MEMORY_SIZE);
    speech_heap_init(audio_process.speech_heap, SPEECH_MEMORY_SIZE);
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
    if(curr_surround_status == 1 && next_surround_status == 1) {
        curr_surround_status = 0xff;
    }
    if (app_tws_ibrt_get_bt_ctrl_ctx()->nv_role == IBRT_SLAVE){
        nv_role = 0;
    } else{
        nv_role = 1;
    }
    stereo_surround_st = stereo_surround_init(sample_rate,frame_size,sample_bits,nv_role,
#if defined(__VIRTUAL_SURROUND_CP__)
    1
#else 
    0
#endif
,sw_ch_num,
#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    &audio_process.virtual_surround_cfg
#else
    &audio_virtual_surround_cfg
#endif
    );
#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    audio_process.virtual_surround_update = false;
#endif
#endif

#ifdef __AUDIO_DRC__
    audio_process.drc_st = drc_create(sample_rate, sample_bits, sw_ch_num,
#ifdef AUDIO_DRC_UPDATE_CFG
        &audio_process.drc_cfg
#else
        &audio_drc_cfg
#endif
        );
#ifdef AUDIO_DRC_UPDATE_CFG
    audio_process.drc_update = false;
#endif
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
    curr_dynamic_boost_status = 0;
    audio_process.dynamic_boost_st = dynamic_boost_create(sample_rate, frame_size, sample_bits, sw_ch_num,
#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    	&audio_process.dynamic_boost_cfg
#else
        &audio_dynamic_boost_cfg
#endif
    );
#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    audio_process.dynamic_boost_update = false;
#endif

    dynamic_boost_set_customer_volume_table_type(audio_process.dynamic_boost_st, TGT_VOLUME_LEVEL_MAX);

#ifdef DYNAMIC_BOOST_USE_HW_EQ
#ifndef __HW_DAC_IIR_EQ_PROCESS__
        ASSERT(0, "[%s] HW_DAC_IIR_EQ_PROCESS must be enabled!", __func__);
#else
#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
        IIR_CFG_T *customer_iir_eq_cfg = &audio_process.hw_dac_iir_cfg;
#else
        const IIR_CFG_T *customer_iir_eq_cfg = audio_eq_hw_dac_iir_cfg_list[0];
#endif
        AUDIO_EQ_TYPE_T customer_iir_eq_type = AUDIO_EQ_TYPE_HW_DAC_IIR;
#endif
#endif

#ifdef DYNAMIC_BOOST_USE_SW_EQ
#ifndef __SW_IIR_EQ_PROCESS__
        ASSERT(0, "[%s] SW_IIR_EQ_PROCESS must be enabled!", __func__);
#else
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
        IIR_CFG_T *customer_iir_eq_cfg = &audio_process.sw_iir_cfg;
#else
        const IIR_CFG_T *customer_iir_eq_cfg = audio_eq_sw_iir_cfg_list[0];
#endif
        AUDIO_EQ_TYPE_T customer_iir_eq_type = AUDIO_EQ_TYPE_SW_IIR;
#endif
#endif

    audio_dynamic_boost_set_new_customer_iir_eq(customer_iir_eq_cfg, customer_iir_eq_type);
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
    curr_dynamic_eq_status = 0;
    audio_process.dynamic_eq_st = dynamic_eq_create(sample_rate, frame_size, sample_bits, sw_ch_num,
#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    	&audio_process.dynamic_eq_cfg
#else
        &audio_dynamic_eq_cfg
#endif
    );
#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    audio_process.dynamic_eq_update = false;
#endif

    dynamic_eq_set_customer_volume_table_type(audio_process.dynamic_eq_st, TGT_VOLUME_LEVEL_MAX);

#ifndef __HW_DAC_IIR_EQ_PROCESS__
        ASSERT(0, "[%s] HW_DAC_IIR_EQ_PROCESS must be enabled!", __func__);
#else
#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
        IIR_CFG_T *customer_iir_eq_cfg = &audio_process.hw_dac_iir_cfg;
#else
        const IIR_CFG_T *customer_iir_eq_cfg = audio_eq_hw_dac_iir_cfg_list[0];
#endif
#endif

    dynamic_eq_set_new_eq_compensation(audio_process.dynamic_eq_st,customer_iir_eq_cfg);
    audio_dynamic_eq_set_new_customer_eq(customer_iir_eq_cfg);
#endif

#ifdef DYNAMIC_BOOST_DUMP
    audio_dump_init(DUMP_LEN, sizeof(short), 2);
#endif

#ifdef __AUDIO_BASS_ENHANCER__
    curr_virtual_bass_status = 0;
    audio_process.bass_st = bass_enhancer_init(sample_rate, frame_size, sample_bits, sw_ch_num,
#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
        &audio_process.bass_enhancer_cfg
#else
        &audio_bass_enhancer_cfg
#endif
    );
#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    audio_process.bass_enhancer_update = false;
#endif
#endif

#ifdef __AUDIO_LIMITER__
    audio_process.limiter_st = limiter_create(sample_rate, sample_bits, sw_ch_num,
#ifdef AUDIO_LIMITER_UPDATE_CFG
        &audio_process.limiter_cfg
#else
        &audio_limiter_cfg
#endif
        );
#ifdef AUDIO_LIMITER_UPDATE_CFG
    audio_process.limiter_update = false;
#endif
#endif

#ifdef __AUDIO_REVERB__
    audio_process.reverb_st = reverb_create(sample_rate, frame_size, sample_bits, hw_ch_num,
#ifdef AUDIO_REVERB_UPDATE_CFG
        &audio_process.reverb_cfg
#else
        &audio_reverb_cfg
#endif
        );
#ifdef AUDIO_REVERB_UPDATE_CFG
    audio_process.reverb_update = false;
#endif
#endif

#ifdef __AUDIO_HW_LIMITER__
    hwlimiter_open(CODEC_OUTPUT_DEV);
#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    if(psap_opened())
    {
        psap_set_cfg_coef(&audio_process.hw_limiter_cfg);
    }
#else
#ifdef __AUDIO_RESAMPLE__
    hwlimiter_set_cfg(AUD_SAMPRATE_50781,0);
#else
    hwlimiter_set_cfg(sample_rate,0);
#endif
#endif
    hwlimiter_enable(CODEC_OUTPUT_DEV);
#endif

#ifdef AUDIO_CUSTOM_EQ
    audio_process.nfft = 1024;
    syspool_get_buff((uint8_t **)&audio_process.work_buffer, iir_cfg_find_peak_needed_size(IIR_PARAM_NUM, audio_process.nfft));
#endif

#ifdef TEST_AUDIO_CUSTOM_EQ
    af_codec_dac1_set_custom_eq_peak(2.0);
#endif

#if defined(__PC_CMD_UART__) && defined(USB_AUDIO_APP)
    hal_cmd_open();
#endif

#ifdef AUDIO_DSP_ACCEL
    APP_MCPP_CFG_T dsp_cfg;
    memset(&dsp_cfg, 0, sizeof(APP_MCPP_CFG_T));
    dsp_cfg.capture.stream_enable = false;
    dsp_cfg.playback.stream_enable = true;
    dsp_cfg.playback.sample_rate = sample_rate;
    if (sample_bits == AUD_BITS_24){
        dsp_cfg.playback.sample_bytes = sizeof(int32_t);
    }
    else if (sample_bits == AUD_BITS_16){
        dsp_cfg.playback.sample_bytes = sizeof(int16_t);
    }
    dsp_cfg.playback.algo_frame_len = frame_size;
    dsp_cfg.playback.channel_num = sw_ch_num;
#if defined(AUDIO_DSP_ACCEL_HIFI)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_HIFI;
#elif defined(AUDIO_DSP_ACCEL_M55)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_M55;
#elif defined(AUDIO_DSP_ACCEL_SENS)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_SENS;
#elif defined(AUDIO_DSP_ACCEL_CP_SUBSYS)
    dsp_cfg.playback.core_server = APP_MCPP_CORE_CP_SUBSYS;
#endif

    app_mcpp_open(APP_MCPP_USER_AUDIO, &dsp_cfg);
#endif

#ifdef DIRAC_AUDIO_ENABLE
    // set dirac_log_cb (may be omitted)
    dirac_set_log_cb(dirac_log_cb);

    // set dirac configuration
    const dirac_config_t dirac_config = {
        .channel_config = get_dirac_channel_config(),
        .samplerate     = sample_rate,
        .sample_format  = audio_process.sample_bits == AUD_BITS_16 ? DIRAC_SAMPLE_FORMAT_I16 : DIRAC_SAMPLE_FORMAT_R24,
    };

    // get dirac size and allocate memory
    size_t dirac_size = dirac_get_size(&dirac_config);
    // med_heap_doesn't actually give you the full requested size, so request some extra
    TRACE(2,"[%s] dirac_get_size = %d", __func__, dirac_size);
#if 0
    const size_t extra = 100;
    static uint8_t *g_medMemPool = NULL;
/*     if(dirac_size < 0)
    {
        TRACE(2,"[%s] dirac_size ##############", __func__);
        dirac_size = 50*1024;
    } */
    syspool_get_buff(&g_medMemPool, dirac_size + extra);
    med_heap_init(g_medMemPool, dirac_size + extra);
    dirac_memory = med_malloc(dirac_size);
#else
    dirac_memory = (void *)audio_calloc(sizeof(void) , dirac_size);
#endif
    dirac_create(&dirac_config, dirac_memory);
    TRACE(0,"dirac init");
#endif

	return 0;
}

int audio_process_close(void)
{
#ifdef __SW_IIR_EQ_PROCESS__
    audio_process.sw_iir_enable = false;
    iir_close();
#endif

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    audio_process.hw_dac_iir_enable = false;
    hw_codec_iir_close(HW_CODEC_IIR_DAC);
#endif

#ifdef __HW_IIR_EQ_PROCESS__
    audio_process.hw_iir_enable = false;
    hw_iir_close();
#endif

#ifdef __HW_FIR_EQ_PROCESS__
    audio_process.hw_fir_enable = false;
    fir_close();
#if defined(CHIP_BEST1000) && defined(FIR_HIGHSPEED)
    hal_cmu_fir_high_speed_disable(HAL_CMU_FIR_USER_EQ);
#endif
#endif

#ifdef __AUDIO_DRC__
#ifdef AUDIO_DRC_UPDATE_CFG
    audio_process.drc_update = false;
#endif
    drc_destroy(audio_process.drc_st);
    audio_process.drc_st = NULL;
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    audio_process.dynamic_boost_update = false;
#endif
    dynamic_boost_destroy(audio_process.dynamic_boost_st);
    audio_process.dynamic_boost_st = NULL;
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    audio_process.dynamic_eq_update = false;
#endif
    dynamic_eq_destroy(audio_process.dynamic_eq_st);
    audio_process.dynamic_eq_st = NULL;
#endif

#ifdef __AUDIO_LIMITER__
#ifdef AUDIO_LIMITER_UPDATE_CFG
    audio_process.limiter_update = false;
#endif
    limiter_destroy(audio_process.limiter_st);
    audio_process.limiter_st = NULL;
#endif

#ifdef __AUDIO_REVERB__
#ifdef AUDIO_REVERB_UPDATE_CFG
    audio_process.reverb_update = false;
#endif
    reverb_destroy(audio_process.reverb_st);
    audio_process.reverb_st = NULL;
#endif

#ifdef __AUDIO_BASS_ENHANCER__
#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    audio_process.bass_enhancer_update = false;
#endif
    bass_enhancer_destroy(audio_process.bass_st);
    audio_process.bass_st = NULL;
#endif

#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_CP__) || defined(__VIRTUAL_SURROUND_HWFIR__) || defined(__VIRTUAL_SURROUND_STEREO__)
#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    audio_process.virtual_surround_update = false;
#endif
    stereo_surround_fir_onoff(stereo_surround_st,0);
    stereo_surround_destroy(stereo_surround_st);
    stereo_surround_st = NULL;
#endif

#ifdef AUDIO_DSP_ACCEL
    app_mcpp_close(APP_MCPP_USER_AUDIO);
#endif

#ifdef DIRAC_AUDIO_ENABLE
    dirac_destroy();
    audio_free(dirac_memory);
#endif

#if AUDIO_MEMORY_SIZE > 0
    size_t total = 0, used = 0, max_used = 0;
    audio_memory_info(&total, &used, &max_used);
    TRACE(3,"AUDIO MALLOC MEM: total - %d, used - %d, max_used - %d.", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);
#endif

#if SPEECH_MEMORY_SIZE > 0
    size_t total_speech = 0, used_speech = 0, max_used_speech = 0;
    med_memory_info(&total_speech, &used_speech, &max_used_speech);
    TRACE(3,"SPEECH MALLOC MEM: total - %d, used - %d, max_used - %d.", total_speech, used_speech, max_used_speech);
    ASSERT(used_speech == 0, "[%s] used != 0", __func__);
#endif

#ifdef __AUDIO_HW_LIMITER__
#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    audio_process.hw_limiter_update = false;
#endif
    hwlimiter_disable(CODEC_OUTPUT_DEV);
    hwlimiter_close();
#endif

#if defined(__PC_CMD_UART__) && defined(USB_AUDIO_APP)
    hal_cmd_close();
#endif

#if defined(AUDIO_CUSTOM_EQ)
    af_codec_dac1_set_custom_eq_peak(1.0);
#endif

    return 0;
}

#if defined(AUDIO_CUSTOM_EQ)
/*
 * Current merge strategy:
 * Normally master gain should be the negative of max peak of frequency response, but it is not always true.
 * Sometime we want to reduce the overall output power, sometimes we can accept some distortion.
 * We define **HEADROOM** to be the extra gain applied to master gain, that is
 *
 * headroom = -master_gain - design_peak_dB
 *
 * When we merge design eq with custom eq, we keep the same headroom as design eq.
 */
void audio_eq_merge_custom_eq(IIR_CFG_T *cfg, IIR_CFG_T *custom_cfg, const IIR_CFG_T *design_cfg)
{
    if (custom_cfg->num + design_cfg->num > IIR_PARAM_NUM) {
        TRACE(0, "[%s] custom eq num(%d) exceed max avaliable num(%d)",
            __FUNCTION__, custom_cfg->num, IIR_PARAM_NUM - design_cfg->num);
        return;
    }

    memcpy(cfg, design_cfg, sizeof(IIR_CFG_T));

    if (custom_cfg->num == 0) {
        return;
    }

    for (uint8_t i = 0, j = design_cfg->num; i < custom_cfg->num; i++, j++) {
        memcpy(&cfg->param[j], &custom_cfg->param[i], sizeof(IIR_PARAM_T));
    }

    cfg->num += custom_cfg->num;

    POSSIBLY_UNUSED enum AUD_SAMPRATE_T fs = audio_process.sample_rate;
    POSSIBLY_UNUSED int32_t nfft = audio_process.nfft;

#ifndef  __AUDIO_HW_LIMITER__
#if 0
    float design_peak_idx = 0;
    float design_peak = iir_cfg_find_peak(audio_process.sample_rate, design_cfg, nfft, &design_peak_idx, audio_process.work_buffer);

    float design_headroom = -design_cfg->gain0 - LIN2DB(design_peak);

    TRACE(3, "design idx = %d(e-3), design peak = %d(e-3), f0 = %d(e-3)\n", (int)(design_peak_idx*1000), (int)(design_peak*1000), (int)(design_peak_idx / nfft * fs * 1000));

    float peak_idx = 0;
    float peak = iir_cfg_find_peak(audio_process.sample_rate, cfg, nfft, &peak_idx, audio_process.work_buffer);

    TRACE(3, "idx = %d(e-3), peak = %d(e-3), f0 = %d(e-3)\n", (int)(peak_idx*1000), (int)(peak*1000), (int)(peak_idx / nfft * fs * 1000));

    // update master gain
    float peak_dB = LIN2DB(peak);
    cfg->gain0 = -peak_dB - design_headroom;
    cfg->gain1 = -peak_dB - design_headroom;
#else
    float custom_peak_idx = 0;
    float custom_peak = iir_cfg_find_peak(audio_process.sample_rate, custom_cfg, nfft, &custom_peak_idx, audio_process.work_buffer);

    TRACE(3, "design idx = %d(e-3), design peak = %d(e-3), f0 = %d(e-3)\n", (int)(custom_peak_idx*1000), (int)(custom_peak*1000), (int)(custom_peak_idx / nfft * fs * 1000));

    // update master gain
    cfg->gain0 = design_cfg->gain0;
    cfg->gain1 = design_cfg->gain1;

    af_codec_dac1_set_custom_eq_peak(custom_peak);
#endif
#else    
    cfg->gain0 = design_cfg->gain0 - 6; //6 is max gain from jbl app
    cfg->gain1 = design_cfg->gain1 - 6; //6 is max gain from jbl app
#endif    
}
#endif

#if defined(AUDIO_EQ_TUNING)
int audio_ping_callback(uint8_t *buf, uint32_t len)
{
    //TRACE(0,"");
    return 0;
}

int audio_anc_switch_callback(uint8_t *buf, uint32_t len)
{
    TRACE(0, "[%s] len = %d, sizeof(uint32_t) = %d", __func__, len, sizeof(uint32_t));

    if (len != sizeof(uint32_t)) {
        return 1;
    }

#if defined(ANC_APP)
    uint32_t mode = *((uint32_t *)buf);

    if (mode < APP_ANC_MODE_QTY) {
        TRACE(0, "[%s] mode: %d", __func__, mode);
        app_anc_switch(mode);
    } else {
        TRACE(0, "[%s] WARNING: mode(%d) >= APP_ANC_MODE_QTY", __func__, mode);
    }
#else
    TRACE(0, "[%s] WARNING: ANC_APP is disabled", __func__);
#endif

    return 0;
}

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) && !defined(USB_EQ_TUNING)
#ifndef USB_AUDIO_APP
int audio_eq_sw_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

    memcpy(&audio_process.sw_iir_cfg, buf, sizeof(IIR_CFG_T));
    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_process.sw_iir_cfg.num,
                                                (int32_t)(audio_process.sw_iir_cfg.gain0*10),
                                                (int32_t)(audio_process.sw_iir_cfg.gain1*10));
    for (uint8_t i = 0; i<audio_process.sw_iir_cfg.num; i++){
        TRACE(5,"band num:%d type %d gain:%d fc:%d q:%d", i,
			                           (int32_t)(audio_process.sw_iir_cfg.param[i].type),
                                                (int32_t)(audio_process.sw_iir_cfg.param[i].gain*10),
                                                (int32_t)(audio_process.sw_iir_cfg.param[i].fc*10),
                                                (int32_t)(audio_process.sw_iir_cfg.param[i].Q*10));
    }

#ifdef __SW_IIR_EQ_PROCESS__
    {
        iir_set_cfg(&audio_process.sw_iir_cfg);
        audio_process.sw_iir_enable = true;
    }
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef DYNAMIC_BOOST_USE_SW_EQ
    audio_dynamic_boost_set_new_customer_iir_eq(&audio_process.sw_iir_cfg, AUDIO_EQ_TYPE_SW_IIR);
#endif
#endif

    return 0;
}
#else
int audio_eq_sw_iir_callback(uint8_t *buf, uint32_t  len)
{
    // IIR_CFG_T *rx_iir_cfg = NULL;

    // rx_iir_cfg = (IIR_CFG_T *)buf;

    // TRACE(3,"[%s] left gain = %f, right gain = %f", __func__, rx_iir_cfg->gain0, rx_iir_cfg->gain1);

    // for(int i=0; i<rx_iir_cfg->num; i++)
    // {
    //     TRACE(5,"[%s] iir[%d] gain = %f, f = %f, Q = %f", __func__, i, rx_iir_cfg->param[i].gain, rx_iir_cfg->param[i].fc, rx_iir_cfg->param[i].Q);
    // }

    // audio_eq_set_cfg(NULL,(const IIR_CFG_T *)rx_iir_cfg,AUDIO_EQ_TYPE_SW_IIR);

    iir_update_cfg_tbl(buf, len);

    return 0;
}
#endif
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
int audio_eq_hw_fir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(FIR_CFG_T));

    if (len != sizeof(FIR_CFG_T))
    {
        return 1;
    }

    FIR_CFG_T *rx_fir_cfg = NULL;

    rx_fir_cfg = (FIR_CFG_T *)buf;

    TRACE(3,"[%s] left gain = %d, right gain = %d", __func__, rx_fir_cfg->gain0, rx_fir_cfg->gain1);

    TRACE(6,"[%s] len = %d, coef: %d, %d......%d, %d", __func__, rx_fir_cfg->len, rx_fir_cfg->coef[0], rx_fir_cfg->coef[1], rx_fir_cfg->coef[rx_fir_cfg->len-2], rx_fir_cfg->coef[rx_fir_cfg->len-1]);

    rx_fir_cfg->gain0 = 6;
    rx_fir_cfg->gain1 = 6;

    if(rx_fir_cfg)
    {
        memcpy(&audio_process.fir_cfg, rx_fir_cfg, sizeof(audio_process.fir_cfg));
        audio_process.fir_enable = true;
        fir_set_cfg(&audio_process.fir_cfg);
    }
    else
    {
        audio_process.fir_enable = false;
    }

    return 0;
}
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
int audio_drc_callback(uint8_t *buf, uint32_t  len)
{
	TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(DrcConfig));

    if (len != sizeof(DrcConfig))
    {
        TRACE(1,"[%s] WARNING: Length is different", __func__);

        return 1;
    }

    if (audio_process.drc_st == NULL)
    {
        TRACE(1,"[%s] WARNING: audio_process.limiter_st = NULL", __func__);
        TRACE(1,"[%s] WARNING: Please Play music, then tuning Limiter", __func__);

        return 2;
    }

	memcpy(&audio_process.drc_cfg, buf, sizeof(DrcConfig));
    audio_process.drc_update = true;

    return 0;
}
#endif

#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
int audio_dynamic_boost_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(DynamicBoostConfig));

    if (len != sizeof(DynamicBoostConfig))
    {
        return 1;
    }

    memcpy(&audio_process.dynamic_boost_cfg, buf, sizeof(DynamicBoostConfig));
    audio_process.dynamic_boost_update = true;

    TRACE(10,"debug: %d ,freq: %d, order: %d, CT: %d ,CS*10: %d, WT: %d, WS*10: %d, ET: %d, ES*10: %d,makeup_gain: %d",
                (int32_t)(audio_process.dynamic_boost_cfg.debug),
                (int32_t)(audio_process.dynamic_boost_cfg.xover_freq[0] ),
                (int32_t)(audio_process.dynamic_boost_cfg.order),
                (int32_t)(audio_process.dynamic_boost_cfg.CT),
                (int32_t)(audio_process.dynamic_boost_cfg.CS*10),
                (int32_t)(audio_process.dynamic_boost_cfg.WT),
                (int32_t)(audio_process.dynamic_boost_cfg.WS*10),
                (int32_t)(audio_process.dynamic_boost_cfg.ET),
                (int32_t)(audio_process.dynamic_boost_cfg.ES*10),
                (int32_t)(audio_process.dynamic_boost_cfg.makeup_gain));

    for (uint8_t i = 0; i<audio_process.dynamic_boost_cfg.eq_num; i++){
        TRACE(3,"freq: %d , Q*100: %d , gain: %d",
                (int32_t)(audio_process.dynamic_boost_cfg.boost_eq[i].freq),
                (int32_t)(audio_process.dynamic_boost_cfg.boost_eq[i].Q*100),
                (int32_t)(audio_process.dynamic_boost_cfg.boost_eq[i].gain));
    }
    TRACE(1,"[%s] !!! dynamic_boost_update: %d", __func__,audio_process.dynamic_boost_update);
    return 0;
}
#endif

#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
int audio_dynamic_eq_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(DynamicEqConfig));

    if (len != sizeof(DynamicEqConfig))
    {
        return 1;
    }

    memcpy(&audio_process.dynamic_eq_cfg, buf, sizeof(DynamicEqConfig));
    audio_process.dynamic_eq_update = true;
    audio_process.dynamic_eq_cfg.debug = audio_dynamic_eq_cfg.debug;

    TRACE(3,"switch_on: %d, debug: %d, eq_num: %d",audio_process.dynamic_eq_cfg.switch_on, 
            audio_process.dynamic_eq_cfg.debug, audio_process.dynamic_eq_cfg.eq_num);

    for (uint8_t i = 0; i < audio_process.dynamic_eq_cfg.eq_num; i++) {
        TRACE(3,"gain: %d , fc: %d , Q*100: %d",
                (int32_t)(audio_process.dynamic_eq_cfg.dyeq_param_cfg[i].dyeq_eq_cfg.gain),
                (int32_t)(audio_process.dynamic_eq_cfg.dyeq_param_cfg[i].dyeq_eq_cfg.fc),
                (int32_t)(audio_process.dynamic_eq_cfg.dyeq_param_cfg[i].dyeq_eq_cfg.Q*100));
    }

    int onoff = audio_process.dynamic_eq_cfg.switch_on;
    audio_dynamic_eq_switch(onoff);

    TRACE(1,"[%s] !!! dynamic_eq_update: %d", __func__,audio_process.dynamic_eq_update);
    return 0;
}
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
int audio_limiter_callback(uint8_t *buf, uint32_t  len)
{
	TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(LimiterConfig));

    if (len != sizeof(LimiterConfig))
    {
        TRACE(1,"[%s] WARNING: Length is different", __func__);

        return 1;
    }

    if (audio_process.limiter_st == NULL)
    {
        TRACE(1,"[%s] WARNING: audio_process.limiter_st = NULL", __func__);
        TRACE(1,"[%s] WARNING: Please Play music, then tuning Limiter", __func__);

        return 2;
    }

	memcpy(&audio_process.limiter_cfg, buf, sizeof(LimiterConfig));
    audio_process.limiter_update = true;

    return 0;
}
#endif

#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
int audio_hw_limiter_callback(uint8_t *buf, uint32_t  len)
{
	TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(struct_psap_cfg));

    if (len != sizeof(struct_psap_cfg))
    {
        TRACE(1,"[%s] WARNING: Length is different", __func__);

        return 1;
    }

	memcpy(&audio_process.hw_limiter_cfg, buf, sizeof(struct_psap_cfg));
    audio_process.hw_limiter_update = true;

    return 0;
}
#endif

#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
int audio_bass_enhancer_callback(uint8_t *buf, uint32_t len)
{
    TRACE(3, "[%s] len = %d, sizeof(struct) = %d", __FUNCTION__, len, sizeof(BassEnhancerConfig));

    if (len != sizeof(BassEnhancerConfig))
    {
        TRACE(1, "[%s] WARNING: length is different", __FUNCTION__);
        DUMP8("0x%x, ", buf, len);
        return 1;
    }

    if (audio_process.bass_st == NULL)
    {
        TRACE(1, "[%s] WARNING: audio_process.bass_st = NULL", __FUNCTION__);
        TRACE(1, "[%s] WARNING: Please Play music, then tuning bass_enhancer", __FUNCTION__);

        return 2;
    }

	memcpy(&audio_process.bass_enhancer_cfg, buf, sizeof(BassEnhancerConfig));
    audio_process.bass_enhancer_update = true;

    int onoff = audio_process.bass_enhancer_cfg.switch_on;
    audio_virtual_bass_switch(onoff);
    return 0;
}
#endif

#ifdef VIRTUAL_SURROUND_UPDATE_CFG
int audio_virtual_surround_callback(uint8_t *buf, uint32_t len)
{
    TRACE(3, "[%s] len = %d, sizeof(struct) = %d", __FUNCTION__, len, sizeof(VirtualSurroundConfig));

    if (len != sizeof(VirtualSurroundConfig))
    {
        TRACE(1, "[%s] WARNING: length is different", __FUNCTION__);
        DUMP8("0x%x, ", buf, len);
        return 1;
    }

    if (stereo_surround_st == NULL)
    {
        TRACE(1, "[%s] WARNING: stereo_surround_st = NULL", __FUNCTION__);
        TRACE(1, "[%s] WARNING: Please Play music, then tuning virtual surround", __FUNCTION__);

        return 2;
    }

    memcpy(&audio_process.virtual_surround_cfg, buf, sizeof(VirtualSurroundConfig));
    audio_process.virtual_surround_update = true;

    int onoff = audio_process.virtual_surround_cfg.switch_on;
    audio_process_stereo_surround_onoff(onoff);
    return 0;
}
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
int audio_reverb_callback(uint8_t *buf, uint32_t len)
{
    TRACE(3, "[%s] len = %d, sizeof(struct) = %d", __FUNCTION__, len, sizeof(ReverbConfig));

    if (len != sizeof(ReverbConfig))
    {
        TRACE(1, "[%s] WARNING: length is different", __FUNCTION__);
        DUMP8("0x%x, ", buf, len);
        return 1;
    }

    if (audio_process.reverb_st == NULL)
    {
        TRACE(1, "[%s] WARNING: audio_process.reverb_st = NULL", __FUNCTION__);
        TRACE(1, "[%s] WARNING: Please Play music, then tuning Reverb", __FUNCTION__);

        return 2;
    }

	memcpy(&audio_process.reverb_cfg, buf, sizeof(ReverbConfig));
    audio_process.reverb_update = true;

    return 0;
}
#endif

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
int audio_eq_iir_channel_select_callback(uint8_t *buf, uint32_t len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(int));
    if (len != sizeof(int))
    {
        return 1;
    }

    iir_channel_status = *(int *)buf;
    TRACE(3,"[%s] iir_channel_status = %d !", __func__, iir_channel_status);

    return 0;
}
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG) && !defined(USB_EQ_TUNING)
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        HW_CODEC_IIR_CFG_T hw_iir_cfg_dac_lr;
#endif
int audio_eq_hw_dac_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    TRACE(3,"[%s] iir_channel_status = %d !", __func__, iir_channel_status);    // L -> 1 ; R -> 2 ; stereo -> 3
    if (iir_channel_status == IIR_CHANNEL_SELECT_LCHNL)
#endif
    {
#if defined(TEST_AUDIO_CUSTOM_EQ)
        audio_eq_merge_custom_eq(&audio_process.hw_dac_iir_cfg, (IIR_CFG_T *)buf, audio_eq_hw_dac_iir_cfg_list[0]);
#else
        memcpy(&audio_process.hw_dac_iir_cfg, buf, sizeof(IIR_CFG_T));
#endif

        TRACE(0, "LCHNL num: %d, gain0: %d, gain1: %d",
                                                    (int32_t)audio_process.hw_dac_iir_cfg.num,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.gain0*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.gain1*10));
        for (uint8_t i = 0; i<audio_process.hw_dac_iir_cfg.num; i++){
            TRACE(0, "[%d] type: %d, gain: %d, fc: %d, q: %d", i,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].type),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].gain*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].fc*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].Q*10));
        }
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    } else if (iir_channel_status == IIR_CHANNEL_SELECT_RCHNL) {
#if defined(TEST_AUDIO_CUSTOM_EQ)
        audio_eq_merge_custom_eq(&audio_process.hw_dac_iir_cfg2, (IIR_CFG_T *)buf, audio_eq_hw_dac_iir_cfg_list2[0]);
#else
        memcpy(&audio_process.hw_dac_iir_cfg2, buf, sizeof(IIR_CFG_T));
#endif
        TRACE(0, "RCHNL num: %d, gain0: %d, gain1: %d",
                                                    (int32_t)audio_process.hw_dac_iir_cfg2.num,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.gain0*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.gain1*10));
        for (uint8_t i = 0; i<audio_process.hw_dac_iir_cfg2.num; i++){
            TRACE(0, "[%d] type: %d, gain: %d, fc: %d, q: %d", i,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].type),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].gain*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].fc*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].Q*10));
        }
    } else if (iir_channel_status == IIR_CHANNEL_SELECT_STEREO) {
#if defined(TEST_AUDIO_CUSTOM_EQ)
        audio_eq_merge_custom_eq(&audio_process.hw_dac_iir_cfg, (IIR_CFG_T *)buf, audio_eq_hw_dac_iir_cfg_list[0]);
        audio_eq_merge_custom_eq(&audio_process.hw_dac_iir_cfg2, (IIR_CFG_T *)buf, audio_eq_hw_dac_iir_cfg_list2[0]);
#else
        memcpy(&audio_process.hw_dac_iir_cfg, buf, sizeof(IIR_CFG_T));
        memcpy(&audio_process.hw_dac_iir_cfg2, buf, sizeof(IIR_CFG_T));
#endif

        TRACE(0, "LCHNL num: %d, gain0: %d, gain1: %d",
                                                    (int32_t)audio_process.hw_dac_iir_cfg.num,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.gain0*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.gain1*10));
        for (uint8_t i = 0; i<audio_process.hw_dac_iir_cfg.num; i++){
            TRACE(0, "[%d] type: %d, gain: %d, fc: %d, q: %d", i,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].type),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].gain*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].fc*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg.param[i].Q*10));
        }

        TRACE(0, "RCHNL num: %d, gain0: %d, gain1: %d",
                                                    (int32_t)audio_process.hw_dac_iir_cfg2.num,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.gain0*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.gain1*10));
        for (uint8_t i = 0; i<audio_process.hw_dac_iir_cfg2.num; i++){
            TRACE(0, "[%d] type: %d, gain: %d, fc: %d, q: %d", i,
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].type),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].gain*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].fc*10),
                                                    (int32_t)(audio_process.hw_dac_iir_cfg2.param[i].Q*10));
        }
    } else {
        TRACE(3,"[ERR] Invalid iir_channel_status = %d !", iir_channel_status);
#endif
    }

#ifdef __HW_DAC_IIR_EQ_PROCESS__
    {
        HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;
        enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
        sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_process.sample_rate,1);
        TRACE(3,"audio_process.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_process.sample_rate, sample_rate_hw_dac_iir);
#else
        sample_rate_hw_dac_iir = audio_process.sample_rate;
#endif
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_process.hw_dac_iir_cfg);
        memcpy(&(hw_iir_cfg_dac_lr.iir_filtes_l),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));

        hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_process.hw_dac_iir_cfg2);
        memcpy(&(hw_iir_cfg_dac_lr.iir_filtes_r),&(hw_iir_cfg_dac->iir_filtes_l),sizeof(HW_CODEC_IIR_FILTERS_T));

        hw_codec_iir_set_cfg(&hw_iir_cfg_dac_lr, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
#else
        hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir,&audio_process.hw_dac_iir_cfg);
        ASSERT(hw_iir_cfg_dac != NULL, "[%s] %d codec IIR parameter error!", __func__, (uint32_t)hw_iir_cfg_dac);

        // hal_codec_iir_dump(hw_iir_cfg_dac);

        hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
#endif
        audio_process.hw_dac_iir_enable = true;

    }
#endif

#ifdef __AUDIO_DYNAMIC_BOOST__
#ifdef DYNAMIC_BOOST_USE_HW_EQ
        audio_dynamic_boost_set_new_customer_iir_eq(&audio_process.hw_dac_iir_cfg, AUDIO_EQ_TYPE_HW_DAC_IIR);
#endif
#endif

#ifdef __AUDIO_DYNAMIC_EQ__
    dynamic_eq_set_new_eq_compensation(audio_process.dynamic_eq_st,&audio_process.hw_dac_iir_cfg);
#endif

    return 0;
}
#endif


#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
int audio_eq_hw_iir_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof(IIR_CFG_T));

    if (len != sizeof(IIR_CFG_T))
    {
        return 1;
    }

    memcpy(&audio_process.hw_iir_cfg, buf, sizeof(IIR_CFG_T));
    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_process.hw_iir_cfg.num,
                                                (int32_t)(audio_process.hw_iir_cfg.gain0*10),
                                                (int32_t)(audio_process.hw_iir_cfg.gain1*10));

    for (uint8_t i = 0; i<audio_process.hw_iir_cfg.num; i++)
    {
        TRACE(5,"band num:%d type %d gain:%d fc:%d q:%d", i,
			                                    (int32_t)(audio_process.hw_iir_cfg.param[i].type),
                                                (int32_t)(audio_process.hw_iir_cfg.param[i].gain*10),
                                                (int32_t)(audio_process.hw_iir_cfg.param[i].fc*10),
                                                (int32_t)(audio_process.hw_iir_cfg.param[i].Q*10));
    }

#ifdef __HW_IIR_EQ_PROCESS__
    {
        HW_IIR_CFG_T *hw_iir_cfg=NULL;
#ifdef __AUDIO_RESAMPLE__
        enum AUD_SAMPRATE_T sample_rate_hw_iir=AUD_SAMPRATE_50781;
#else
        enum AUD_SAMPRATE_T sample_rate_hw_iir=audio_process.sample_rate;
#endif
        hw_iir_cfg = hw_iir_get_cfg(sample_rate_hw_iir,&audio_process.hw_iir_cfg);
        ASSERT(hw_iir_cfg != NULL, "[%s] %p codec IIR parameter error!", __func__, hw_iir_cfg);
        hw_iir_set_cfg(hw_iir_cfg);
        audio_process.hw_iir_enable = true;
    }
#endif

    return 0;
}
#endif
#endif  // #if defined(AUDIO_EQ_TUNING)

#ifdef USB_EQ_TUNING

int audio_eq_usb_iir_callback(uint8_t *buf, uint32_t  len)
{
	IIR_CFG_T* cur_cfg;

	TRACE(2,"usb_iir_cb: len=[%d - %d]", len, sizeof(IIR_CFG_T));

	if (len != sizeof(IIR_CFG_T)) {
        return 1;
    }

	cur_cfg = (IIR_CFG_T*)buf;
	TRACE(2,"-> sample_rate[%d], num[%d]", /*cur_cfg->samplerate,*/ audio_process.sample_rate, cur_cfg->num);

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_process.sw_iir_cfg.gain0 = cur_cfg->gain0;
	audio_process.sw_iir_cfg.gain1 = cur_cfg->gain1;
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_process.hw_dac_iir_cfg.gain0 = 0;
	audio_process.hw_dac_iir_cfg.gain1 = 0;
#else
    audio_process.hw_dac_iir_cfg.gain0 = cur_cfg->gain0;
	audio_process.hw_dac_iir_cfg.gain1 = cur_cfg->gain1;
#endif
#endif

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
	if (cur_cfg->num > AUD_DAC_IIR_NUM_EQ) {
		audio_process.hw_dac_iir_cfg.num = AUD_DAC_IIR_NUM_EQ;
	} else {
		audio_process.hw_dac_iir_cfg.num = cur_cfg->num;
	}
    TRACE(1,"-> hw_dac_iir_num[%d]", audio_process.hw_dac_iir_cfg.num);
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	audio_process.sw_iir_cfg.num = cur_cfg->num - audio_process.hw_dac_iir_cfg.num;
    TRACE(1,"-> sw_iir_num[%d]", audio_process.sw_iir_cfg.num);
#endif
	//TRACE(2,"-> iir_num[%d - %d]", audio_process.hw_dac_iir_cfg.num, audio_process.sw_iir_cfg.num);

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
	if (audio_process.hw_dac_iir_cfg.num) {
		memcpy((void*)(&audio_process.hw_dac_iir_cfg.param[0]),
					(void*)(&cur_cfg->param[0]),
					audio_process.hw_dac_iir_cfg.num*sizeof(IIR_PARAM_T));
	}
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
	if (audio_process.sw_iir_cfg.num) {

		memcpy((void*)(&audio_process.sw_iir_cfg.param[0]),
					(void*)(&cur_cfg->param[audio_process.hw_dac_iir_cfg.num]),
					audio_process.sw_iir_cfg.num * sizeof(IIR_PARAM_T));

	} else {

		// set a default filter
		audio_process.sw_iir_cfg.num = 1;

		audio_process.sw_iir_cfg.param[0].fc = 1000.0;
		audio_process.sw_iir_cfg.param[0].gain = 0.0;
		audio_process.sw_iir_cfg.param[0].type = IIR_TYPE_PEAK;
		audio_process.sw_iir_cfg.param[0].Q = 1.0;

	}
#endif

	if (audio_process.sample_rate) {
		audio_process.update_cfg = true;
	}

	audio_process.eq_updated_cfg = true;

	return 0;
}

void audio_eq_usb_eq_update (void)
{
	if (audio_process.update_cfg) {

#if defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
		HW_CODEC_IIR_CFG_T *hw_iir_cfg_dac = NULL;

		if (audio_process.hw_dac_iir_cfg.num) {
                    enum AUD_SAMPRATE_T sample_rate_hw_dac_iir;
#ifdef __AUDIO_RESAMPLE__
                    sample_rate_hw_dac_iir=hal_codec_get_real_sample_rate(audio_process.sample_rate,1);
                    TRACE(3,"audio_process.sample_rate:%d, sample_rate_hw_dac_iir: %d.", audio_process.sample_rate, sample_rate_hw_dac_iir);
#else
                    sample_rate_hw_dac_iir = audio_process.sample_rate;
#endif
			hw_iir_cfg_dac = hw_codec_iir_get_cfg(sample_rate_hw_dac_iir, &audio_process.hw_dac_iir_cfg);

			hw_codec_iir_set_cfg(hw_iir_cfg_dac, sample_rate_hw_dac_iir, HW_CODEC_IIR_DAC);
			audio_process.hw_dac_iir_enable = true;

		} else {

			audio_process.hw_dac_iir_enable = false;
		}
#endif

#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG)
		iir_set_cfg(&audio_process.sw_iir_cfg);
		audio_process.sw_iir_enable = true;
#endif

		audio_process.update_cfg = false;

/*
		TRACE(4,"USB EQ Update: en[%d-%d], num[%d-%d]",
				audio_process.hw_dac_iir_enable, audio_process.sw_iir_enable,
				audio_process.hw_dac_iir_cfg.num, audio_process.sw_iir_cfg.num);
*/
	}

}

#endif	// USB_EQ_TUNING


typedef struct {
    uint8_t type;
    // EQ
    uint8_t max_eq_band_num;
    uint16_t sample_rate_num;
    uint8_t sample_rate[20];
    // ANC
    uint32_t anc_mode;

    // Add new items in order for compatibility
} query_eq_info_t;


extern int getMaxEqBand(void);
extern int getSampleArray(uint8_t* buf, uint16_t *num);

extern void hal_cmd_set_res_playload(uint8_t* data, int len);
#define CMD_TYPE_QUERY_DUT_EQ_INFO  0x00
int audio_cmd_callback(uint8_t *buf, uint32_t len)
{
    uint8_t type;
    query_eq_info_t  info;

    memset(&info, 0, sizeof(info));

    type = buf[0];

    TRACE(2,"%s type: %d", __func__, type);
    switch (type) {
        case CMD_TYPE_QUERY_DUT_EQ_INFO:
            info.type = CMD_TYPE_QUERY_DUT_EQ_INFO;

            // EQ
            info.max_eq_band_num = getMaxEqBand();
            getSampleArray(info.sample_rate, &info.sample_rate_num);

            // ANC
#if defined(ANC_APP)
            info.anc_mode = app_anc_get_curr_mode();
#else
            info.anc_mode = 0;
#endif

            hal_cmd_set_res_playload((uint8_t*)&info, sizeof(info));
            break;
        default:
            break;
    }

    return 0;
}

#ifdef AUDIO_SECTION_ENABLE
int audio_cfg_burn_callback(uint8_t *buf, uint32_t  len)
{
    TRACE(3,"[%s] len = %d, sizeof(struct) = %d", __func__, len, sizeof_audio_cfg());

    if (len != sizeof_audio_cfg())
    {
        return 1;
    }

    int res = 0;
    res = store_audio_cfg_into_audio_section((AUDIO_CFG_T *)buf);

    if(res)
    {
        TRACE(2,"[%s] ERROR: res = %d", __func__, res);
        res += 100;
    }
    else
    {
        TRACE(1,"[%s] Store audio cfg into audio section!!!", __func__);
    }

    return res;
}
#endif

#ifdef AUDIO_HEARING_COMPSATN

uint8_t hear_cfg_update_flg = 1;
void audio_get_eq_para(int *para_buf, uint8_t cfg_flag)
{
    if(1 == cfg_flag)
        hear_cfg_update_flg = 1;
    else
        hear_cfg_update_flg = 0;

    int *band_compen = (int *)para_buf;
    int iir_num = 6;
    float gain_buf[6] = {0.0F};
#if HWFIR_MOD==HEARING_MOD_VAL
    get_hear_compen_val_fir(band_compen,gain_buf,iir_num);
#else
    get_hear_compen_val_multi_level(band_compen,gain_buf,iir_num);
    iir_num -= 1;
#endif
    for(int i=0;i<iir_num;i++)
        TRACE(1,"gain_buf[%d]=%de-2",i,(int)(gain_buf[i]*100));

#if SWIIR_MOD==HEARING_MOD_VAL || HWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
    float fc_buf[6] = {600.0, 1200.0, 2400.0, 4800.0, 8000.0, 8000.0};
#if defined(__SW_IIR_EQ_PROCESS__)
    const IIR_CFG_T *iir_default_cfg= audio_eq_sw_iir_cfg_list[0];
#else
    const IIR_CFG_T *iir_default_cfg= audio_eq_hw_dac_iir_cfg_list[0];
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    const IIR_CFG_T *iir_default_cfg_2= audio_eq_hw_dac_iir_cfg_list2[0];
#endif
#endif
    int default_iir_num = iir_default_cfg->num;
    float default_iir_gain0 = iir_default_cfg->gain0;
    float default_iir_gain1 = iir_default_cfg->gain1;
    float gain_tmp0 = default_iir_gain0;
    float gain_tmp1 = default_iir_gain1;
    TRACE(3,"default IIR band num:%d gain0:%d, gain1:%d",
                                    default_iir_num,
                                    (int32_t)default_iir_gain0,
                                    (int32_t)default_iir_gain1);
    audio_process.hear_iir_cfg.num = default_iir_num + iir_num;
    audio_process.hear_iir_cfg.gain0 = gain_tmp0;
    audio_process.hear_iir_cfg.gain1 = gain_tmp1;

    int band_idx = 0 ;
    int iir_coef_idx = 0;
    for(int i=default_iir_num;i<audio_process.hear_iir_cfg.num;i++)
    {
        if(default_iir_num==i)
            audio_process.hear_iir_cfg.param[i].type = IIR_TYPE_LOW_SHELF;
        else if((default_iir_num+iir_num-1)==i)
            audio_process.hear_iir_cfg.param[i].type = IIR_TYPE_HIGH_SHELF;
        else
            audio_process.hear_iir_cfg.param[i].type = IIR_TYPE_PEAK;

        if(default_iir_num==i)
            audio_process.hear_iir_cfg.param[i].Q = 0.707;
        else if((default_iir_num+iir_num-1)==i)
            audio_process.hear_iir_cfg.param[i].Q = 0.707;
        else
            audio_process.hear_iir_cfg.param[i].Q = 1.2;

        audio_process.hear_iir_cfg.param[i].gain = gain_buf[band_idx];
        audio_process.hear_iir_cfg.param[i].fc = fc_buf[band_idx];

        get_iir_coef(&audio_process.hear_iir_cfg, i, 44100, &hear_iir_coef[iir_coef_idx]);
        iir_coef_idx += 6;
        band_idx++;
    }

    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                            (int32_t)audio_process.hear_iir_cfg.num,
                                            (int32_t)(audio_process.hear_iir_cfg.gain0),
                                            (int32_t)(audio_process.hear_iir_cfg.gain1));
    for (uint8_t i = default_iir_num; i<audio_process.hear_iir_cfg.num; i++)
    {
        TRACE(5,"band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                            (int32_t)(audio_process.hear_iir_cfg.param[i].type),
                                            (int32_t)(audio_process.hear_iir_cfg.param[i].gain*1000),
                                            (int32_t)(audio_process.hear_iir_cfg.param[i].fc),
                                            (int32_t)(audio_process.hear_iir_cfg.param[i].Q*1000));
    }

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    int default_iir_num_2 = iir_default_cfg_2->num;
    default_iir_gain0 = iir_default_cfg_2->gain0;
    default_iir_gain1 = iir_default_cfg_2->gain1;
    gain_tmp0 = default_iir_gain0;
    gain_tmp1 = default_iir_gain1;
    TRACE(3,"default IIR band num:%d gain0:%d, gain1:%d",
                                    default_iir_num_2,
                                    (int32_t)default_iir_gain0,
                                    (int32_t)default_iir_gain1);
    audio_process.hear_iir_cfg_2.num = default_iir_num_2 + iir_num;
    audio_process.hear_iir_cfg_2.gain0 = gain_tmp0;
    audio_process.hear_iir_cfg_2.gain1 = gain_tmp1;

    band_idx = 0 ;
    iir_coef_idx = 0;
    for(int i=default_iir_num_2;i<audio_process.hear_iir_cfg_2.num;i++)
    {
        if(default_iir_num_2==i)
            audio_process.hear_iir_cfg_2.param[i].type = IIR_TYPE_LOW_SHELF;
        else if((default_iir_num_2+iir_num-1)==i)
            audio_process.hear_iir_cfg_2.param[i].type = IIR_TYPE_HIGH_SHELF;
        else
            audio_process.hear_iir_cfg_2.param[i].type = IIR_TYPE_PEAK;

        if(default_iir_num_2==i)
            audio_process.hear_iir_cfg_2.param[i].Q = 0.707;
        else if((default_iir_num_2+iir_num-1)==i)
            audio_process.hear_iir_cfg_2.param[i].Q = 0.707;
        else
            audio_process.hear_iir_cfg_2.param[i].Q = 1.2;

        audio_process.hear_iir_cfg_2.param[i].gain = gain_buf[band_idx];
        audio_process.hear_iir_cfg_2.param[i].fc = fc_buf[band_idx];

        get_iir_coef(&audio_process.hear_iir_cfg_2, i, 44100, &hear_iir_coef[iir_coef_idx]);
        iir_coef_idx += 6;
        band_idx++;
    }

    TRACE(3,"band num:%d gain0:%d, gain1:%d",
                                            (int32_t)audio_process.hear_iir_cfg_2.num,
                                            (int32_t)(audio_process.hear_iir_cfg_2.gain0),
                                            (int32_t)(audio_process.hear_iir_cfg_2.gain1));
    for (uint8_t i = default_iir_num_2; i<audio_process.hear_iir_cfg_2.num; i++)
    {
        TRACE(5,"band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                            (int32_t)(audio_process.hear_iir_cfg_2.param[i].type),
                                            (int32_t)(audio_process.hear_iir_cfg_2.param[i].gain*1000),
                                            (int32_t)(audio_process.hear_iir_cfg_2.param[i].fc),
                                            (int32_t)(audio_process.hear_iir_cfg_2.param[i].Q*1000));
    }
#endif //__IIR_EQ_PROCESS_LR_2CH__

    for(int i=0;i<6;i++)
        TRACE(1,"hear_iir_coef[%d]=%de-3",i,(int)(hear_iir_coef[i]*1000));

    TRACE(1,"hear_level=%d",get_hear_level());

    if(1 == hear_cfg_update_flg)
    {
        memcpy(&audio_process.hear_iir_cfg_update, &audio_process.hear_iir_cfg, sizeof(IIR_CFG_T));
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        memcpy(&audio_process.hear_iir_cfg_update_2, &audio_process.hear_iir_cfg_2, sizeof(IIR_CFG_T));
#endif

        TRACE(3,"update band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_process.hear_iir_cfg_update.num,
                                                (int32_t)(audio_process.hear_iir_cfg_update.gain0),
                                                (int32_t)(audio_process.hear_iir_cfg_update.gain1));
        for (uint8_t i = default_iir_num; i<audio_process.hear_iir_cfg_update.num; i++)
        {
            TRACE(5,"update band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                                (int32_t)(audio_process.hear_iir_cfg_update.param[i].type),
                                                (int32_t)(audio_process.hear_iir_cfg_update.param[i].gain*1000),
                                                (int32_t)(audio_process.hear_iir_cfg_update.param[i].fc),
                                                (int32_t)(audio_process.hear_iir_cfg_update.param[i].Q*1000));
        }

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        TRACE(3,"update band num:%d gain0:%d, gain1:%d",
                                                (int32_t)audio_process.hear_iir_cfg_update_2.num,
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.gain0),
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.gain1));
        for (uint8_t i = default_iir_num_2; i<audio_process.hear_iir_cfg_update_2.num; i++)
        {
            TRACE(5,"update band num:%d type %d gain:%de-3 fc:%d q:%de-3", i,
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.param[i].type),
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.param[i].gain*1000),
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.param[i].fc),
                                                (int32_t)(audio_process.hear_iir_cfg_update_2.param[i].Q*1000));
        }
#endif
    }

#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    float fir_scale_db = 0.0;
    //For large gain in low freq
    float gain_tmp = -22;
    TRACE(2,"[%s],gain_tmp0=%d",__func__,(int)(gain_tmp));
    for(int i=0;i<iir_num;i++)
        TRACE(2,"gain_buf[%d]=%de-2",i,(int)(gain_buf[i]*100));

    //Get buffer from system pool
#ifdef HEARING_USE_STATIC_RAM
    speech_heap_init(HEAR_DET_STREAM_BUF, HEAR_COMP_BUF_SIZE);
#else
    if(NULL==hear_comp_buf)
        syspool_get_buff(&hear_comp_buf, HEAR_COMP_BUF_SIZE);
    speech_heap_init(hear_comp_buf, HEAR_COMP_BUF_SIZE);
#endif

    HearFir2State* hear_st = HearFir2_create(44100);
    fir_scale_db = HearFir2_process(hear_st, fir_filter,gain_buf);
    TRACE(1,"fir_scale = %de-2",(int)(fir_scale_db*100));
    HearFir2_destroy(hear_st);
    //Free buffer
    size_t total = 0, used = 0, max_used = 0;
    speech_memory_info(&total, &used, &max_used);
    TRACE(3,"HEAP INFO: total=%d, used=%d, max_used=%d", total, used, max_used);
    ASSERT(used == 0, "[%s] used != 0", __func__);

    if(1 == hear_cfg_update_flg)
    {
        float fir_gain_tmp = gain_tmp + fir_scale_db;
        if(0 < fir_gain_tmp)
            fir_gain_tmp = 0;
        audio_process.hear_hw_fir_cfg.gain  = fir_gain_tmp;
        audio_process.hear_hw_fir_cfg.len   = 384;

        for(int i=0;i<384;i++)
        {
            audio_process.hear_hw_fir_cfg.coef[i] = (int)(fir_filter[i]*(1<<23));
        }
    }
    float fir_scale_val = hear_val2db(fir_scale_db);
    TRACE(1,"fir_scale_val = %de-2",(int)(fir_scale_val*100));
    for(int i=0;i<384;i++)
    {
        fir_filter[i] = fir_filter[i] * fir_scale_val;
    }

#endif
#endif
}

#endif

int audio_process_init(void)
{
#if defined(AUDIO_EQ_TUNING)
#if defined(USB_EQ_TUNING) || defined(__PC_CMD_UART__)
    hal_cmd_init();
#endif

#if defined(USB_EQ_TUNING)
#if defined(AUDIO_EQ_SW_IIR_UPDATE_CFG) || defined(AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG)
    hal_cmd_register("iir_eq", audio_eq_usb_iir_callback);
#endif

#else   // #if defined(USB_EQ_TUNING)
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    hal_cmd_register("sw_iir_eq", audio_eq_sw_iir_callback);
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    hal_cmd_register("dac_iir_eq", audio_eq_hw_dac_iir_callback);
#endif

#ifdef __IIR_EQ_PROCESS_LR_2CH__
    hal_cmd_register("iir_channel", audio_eq_iir_channel_select_callback);
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    hal_cmd_register("hw_iir_eq", audio_eq_hw_iir_callback);
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    hal_cmd_register("fir_eq", audio_eq_hw_fir_callback);
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    hal_cmd_register("drc", audio_drc_callback);
#endif

#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    hal_cmd_register("wdrc", audio_dynamic_boost_callback);
#endif

#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    hal_cmd_register("dyeq", audio_dynamic_eq_callback);
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    hal_cmd_register("limiter", audio_limiter_callback);
#endif

#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    hal_cmd_register("hw_drc", audio_hw_limiter_callback);
#endif

#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    hal_cmd_register("virtual_3D", audio_virtual_surround_callback);
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    hal_cmd_register("reverb", audio_reverb_callback);
#endif

#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    hal_cmd_register("virtualbass", audio_bass_enhancer_callback);
#endif


#ifdef AUDIO_SECTION_ENABLE
    hal_cmd_register("burn", audio_cfg_burn_callback);
    hal_cmd_register("audio_burn", audio_cfg_burn_callback);
#endif
#endif  // #if defined(USB_EQ_TUNING)

    hal_cmd_register("anc_switch", audio_anc_switch_callback);

    hal_cmd_register("cmd", audio_cmd_callback);
    hal_cmd_register("ping", audio_ping_callback);
#endif  // #if defined(AUDIO_EQ_TUNING)

#ifdef AUDIO_HEARING_COMPSATN
#if SWIIR_MOD==HEARING_MOD_VAL
#if defined(__SW_IIR_EQ_PROCESS__)
    memcpy(&audio_process.hear_iir_cfg, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
    memcpy(&audio_process.hear_iir_cfg_update, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif
#elif HWFIR_MOD==HEARING_MOD_VAL
#if defined(__HW_FIR_EQ_PROCESS__)
    memcpy(&audio_process.hear_hw_fir_cfg, audio_eq_hw_fir_cfg_list[0], sizeof(FIR_CFG_T));
#endif
#elif HWIIR_MOD==HEARING_MOD_VAL
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
    memcpy(&audio_process.hear_iir_cfg, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
    memcpy(&audio_process.hear_iir_cfg_update, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    memcpy(&audio_process.hear_iir_cfg_2, audio_eq_hw_dac_iir_cfg_list2[0], sizeof(IIR_CFG_T));
    memcpy(&audio_process.hear_iir_cfg_update_2, audio_eq_hw_dac_iir_cfg_list2[0], sizeof(IIR_CFG_T));
#endif
#endif
#endif
#endif

    // load default cfg
#ifdef AUDIO_EQ_SW_IIR_UPDATE_CFG
    memcpy(&audio_process.sw_iir_cfg, audio_eq_sw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif

#ifdef AUDIO_EQ_HW_DAC_IIR_UPDATE_CFG
    memcpy(&audio_process.hw_dac_iir_cfg, audio_eq_hw_dac_iir_cfg_list[0], sizeof(IIR_CFG_T));
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
    memcpy(&audio_process.hw_dac_iir_cfg2, audio_eq_hw_dac_iir_cfg_list2[0], sizeof(IIR_CFG_T));
#endif
#endif

#ifdef AUDIO_EQ_HW_IIR_UPDATE_CFG
    memcpy(&audio_process.hw_iir_cfg, audio_eq_hw_iir_cfg_list[0], sizeof(IIR_CFG_T));
#endif

#ifdef AUDIO_EQ_HW_FIR_UPDATE_CFG
    memcpy(&audio_process.hw_fir_cfg, audio_eq_hw_fir_cfg_list[0], sizeof(FIR_CFG_T));
#endif

#ifdef AUDIO_DRC_UPDATE_CFG
    memcpy(&audio_process.drc_cfg, &audio_drc_cfg, sizeof(DrcConfig));
#endif

#ifdef AUDIO_DYNAMIC_BOOST_UPDATE_CFG
    memcpy(&audio_process.dynamic_boost_cfg, &audio_dynamic_boost_cfg, sizeof(DynamicBoostConfig));
#endif

#ifdef AUDIO_DYNAMIC_EQ_UPDATE_CFG
    memcpy(&audio_process.dynamic_eq_cfg, &audio_dynamic_eq_cfg, sizeof(DynamicEqConfig));
#endif

#ifdef AUDIO_LIMITER_UPDATE_CFG
    memcpy(&audio_process.limiter_cfg, &audio_limiter_cfg, sizeof(LimiterConfig));
#endif

#ifdef AUDIO_HW_LIMITER_UPDATE_CFG
    memcpy(&audio_process.hw_limiter_cfg, hwlimiter_para_list_50p7k[0], sizeof(struct_psap_cfg));
#endif

#ifdef AUDIO_REVERB_UPDATE_CFG
    memcpy(&audio_process.reverb_cfg, &audio_reverb_cfg, sizeof(ReverbConfig));
#endif

#ifdef AUDIO_BASS_ENHANCER_UPDATE_CFG
    memcpy(&audio_process.bass_enhancer_cfg, &audio_bass_enhancer_cfg, sizeof(BassEnhancerConfig));
#endif

#ifdef VIRTUAL_SURROUND_UPDATE_CFG
    memcpy(&audio_process.virtual_surround_cfg, &audio_virtual_surround_cfg, sizeof(VirtualSurroundConfig));
#endif

    return 0;
}

POSSIBLY_UNUSED int32_t audio_eq_set_onoff(int32_t onoff, AUDIO_EQ_TYPE_T audio_eq_type)
{
    TRACE(3,"[audio_eq_set_onoff] onoff:%d audio_eq_type:%d",onoff,audio_eq_type);
    if (onoff) {
        //open
        const IIR_CFG_T* audio_eq_iir_cfg_default = NULL;
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        const IIR_CFG_T* audio_eq_iir_cfg_default_2 = NULL;
#endif
        switch (audio_eq_type)
        {
#ifdef __SW_IIR_EQ_PROCESS__
            case AUDIO_EQ_TYPE_SW_IIR:
            {
                audio_eq_iir_cfg_default = audio_eq_sw_iir_cfg_list[0];
            }
            break;
#endif
#if defined(__HW_DAC_IIR_EQ_PROCESS__)
            case AUDIO_EQ_TYPE_HW_DAC_IIR:
            {
                audio_eq_iir_cfg_default = audio_eq_hw_dac_iir_cfg_list[0];
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
                audio_eq_iir_cfg_default_2 = audio_eq_hw_dac_iir_cfg_list[0];
#endif
            }
            break;
#endif
            default:
            {
                ASSERT(false,"[%s]Error eq type!",__func__);
            }
        }
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        audio_eq_set_cfg(NULL, audio_eq_iir_cfg_default , audio_eq_iir_cfg_default_2, audio_eq_type);
#else
        audio_eq_set_cfg(NULL, audio_eq_iir_cfg_default, audio_eq_type);
#endif

    } else {
        //close
#if defined(__SW_IIR_EQ_PROCESS__) || defined(__HW_DAC_IIR_EQ_PROCESS__)
        const IIR_CFG_T audio_eq_iir_cfg_bypass = {
            .gain0 = 0,
            .gain1 = 0,
            .num = 0,
        };
#if defined(__IIR_EQ_PROCESS_LR_2CH__)
        audio_eq_set_cfg(NULL, &audio_eq_iir_cfg_bypass, &audio_eq_iir_cfg_bypass, audio_eq_type);
#else
        audio_eq_set_cfg(NULL, &audio_eq_iir_cfg_bypass, audio_eq_type);
#endif
#endif
    }
    return 0;
}

