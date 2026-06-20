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
#include "tgt_hardware.h"
#include "aud_section.h"
#include "iir_process.h"
#include "fir_process.h"
#include "drc.h"
#include "limiter.h"
#include "spectrum_fix.h"
#include "reverb.h"
#include "dynamic_boost.h"
#include "tgt_hardware_capsensor.h"
#include "bass_enhancer.h"
#include "stereo_process.h"
#include "dynamic_eq.h"

#if !defined(__NuttX__)

const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_hw_pinmux_pwl[CFG_HW_PWL_NUM] = {
#if (CFG_HW_PWL_NUM > 0)
    {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    {HAL_IOMUX_PIN_P1_4, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
#endif
};

#ifdef __APP_USE_LED_INDICATE_IBRT_STATUS__
const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_ibrt_indication_pinmux_pwl[3] = {
    {HAL_IOMUX_PIN_P1_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
    {HAL_IOMUX_PIN_LED1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VBAT, HAL_IOMUX_PIN_PULLUP_ENABLE},
    {HAL_IOMUX_PIN_LED2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VBAT, HAL_IOMUX_PIN_PULLUP_ENABLE},
};
#endif

#ifdef __KNOWLES
const struct HAL_IOMUX_PIN_FUNCTION_MAP cfg_pinmux_uart[2] = {
    {HAL_IOMUX_PIN_P2_2, HAL_IOMUX_FUNC_UART2_RX, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    {HAL_IOMUX_PIN_P2_3, HAL_IOMUX_FUNC_UART2_TX, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};
#endif

// adckey define
const uint16_t CFG_HW_ADCKEY_MAP_TABLE[CFG_HW_ADCKEY_NUMBER] = {
#if (CFG_HW_ADCKEY_NUMBER > 0)
    HAL_KEY_CODE_FN9,
    HAL_KEY_CODE_FN8,
    HAL_KEY_CODE_FN7,
    HAL_KEY_CODE_FN6,
    HAL_KEY_CODE_FN5,
    HAL_KEY_CODE_FN4,
    HAL_KEY_CODE_FN3,
    HAL_KEY_CODE_FN2,
    HAL_KEY_CODE_FN1,
#endif
};

// gpiokey define
#define CFG_HW_GPIOKEY_DOWN_LEVEL (0)
#define CFG_HW_GPIOKEY_UP_LEVEL (1)
const struct HAL_KEY_GPIOKEY_CFG_T cfg_hw_gpio_key_cfg[CFG_HW_GPIOKEY_NUM] = {
#if (CFG_HW_GPIOKEY_NUM > 0)
#ifdef BES_AUDIO_DEV_Main_Board_9v0
    {HAL_KEY_CODE_FN1, {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
    {HAL_KEY_CODE_FN2, {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
    {HAL_KEY_CODE_FN3, {HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
    {HAL_KEY_CODE_FN4, {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
    {HAL_KEY_CODE_FN5, {HAL_IOMUX_PIN_P2_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
    {HAL_KEY_CODE_FN6, {HAL_IOMUX_PIN_P2_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
#else
    {HAL_KEY_CODE_FN1, {HAL_IOMUX_PIN_P0_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}}, // left
    {HAL_KEY_CODE_FN2, {HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}}, // right
    {HAL_KEY_CODE_FN3, {HAL_IOMUX_PIN_P0_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}}, // middle
#ifndef TPORTS_KEY_COEXIST
// {HAL_KEY_CODE_FN3,{HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
//{HAL_KEY_CODE_FN15,{HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
#endif
#endif
#ifdef IS_MULTI_AI_ENABLED
//{HAL_KEY_CODE_FN13,{HAL_IOMUX_PIN_P1_3, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
//{HAL_KEY_CODE_FN14,{HAL_IOMUX_PIN_P1_2, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE}},
#endif
#endif
};
#endif

// bt config
const char *BT_LOCAL_NAME = TO_STRING(BT_DEV_NAME) "\0";
const char *BLE_DEFAULT_NAME = "BES_BLE";
uint8_t ble_global_addr[6] = {
#ifdef BLE_DEV_ADDR
    BLE_DEV_ADDR
#else
    0xBE, 0x99, 0x34, 0x45, 0x56, 0x67
#endif
};
uint8_t bt_global_addr[6] = {
#ifdef BT_DEV_ADDR
    BT_DEV_ADDR
#else
    0x1e, 0x57, 0x34, 0x45, 0x56, 0x67
#endif
};

#ifdef __TENCENT_VOICE__
#define REVISION_INFO ("0.1.0\0")
const char *BT_FIRMWARE_VERSION = REVISION_INFO;
#endif

#define TX_PA_GAIN CODEC_TX_PA_GAIN_DEFAULT

const struct CODEC_DAC_VOL_T codec_dac_vol[TGT_VOLUME_LEVEL_QTY] = {
    {TX_PA_GAIN, 0x03, -99},
    {TX_PA_GAIN, 0x03, -45},
    {TX_PA_GAIN, 0x03, -42},
    {TX_PA_GAIN, 0x03, -39},
    {TX_PA_GAIN, 0x03, -36},
    {TX_PA_GAIN, 0x03, -33},
    {TX_PA_GAIN, 0x03, -30},
    {TX_PA_GAIN, 0x03, -27},
    {TX_PA_GAIN, 0x03, -24},
    {TX_PA_GAIN, 0x03, -21},
    {TX_PA_GAIN, 0x03, -18},
    {TX_PA_GAIN, 0x03, -15},
    {TX_PA_GAIN, 0x03, -12},
    {TX_PA_GAIN, 0x03, -9},
    {TX_PA_GAIN, 0x03, -6},
    {TX_PA_GAIN, 0x03, -3},
    {TX_PA_GAIN, 0x03, 0}, // 0dBm
};

const struct CODEC_DAC_VOL_T codec_dac_a2dp_vol[TGT_VOLUME_LEVEL_QTY] = {
    {TX_PA_GAIN, 0x03, -99},
    {TX_PA_GAIN, 0x03, -45},
    {TX_PA_GAIN, 0x03, -42},
    {TX_PA_GAIN, 0x03, -39},
    {TX_PA_GAIN, 0x03, -36},
    {TX_PA_GAIN, 0x03, -33},
    {TX_PA_GAIN, 0x03, -30},
    {TX_PA_GAIN, 0x03, -27},
    {TX_PA_GAIN, 0x03, -24},
    {TX_PA_GAIN, 0x03, -21},
    {TX_PA_GAIN, 0x03, -18},
    {TX_PA_GAIN, 0x03, -15},
    {TX_PA_GAIN, 0x03, -12},
    {TX_PA_GAIN, 0x03, -9},
    {TX_PA_GAIN, 0x03, -6},
    {TX_PA_GAIN, 0x03, -3},
    {TX_PA_GAIN, 0x03, 0}, // 0dBm
};

const struct CODEC_DAC_VOL_T codec_dac_hfp_vol[TGT_VOLUME_LEVEL_QTY] = {
    {TX_PA_GAIN, 0x03, -99},
    {TX_PA_GAIN, 0x03, -45},
    {TX_PA_GAIN, 0x03, -42},
    {TX_PA_GAIN, 0x03, -39},
    {TX_PA_GAIN, 0x03, -36},
    {TX_PA_GAIN, 0x03, -33},
    {TX_PA_GAIN, 0x03, -30},
    {TX_PA_GAIN, 0x03, -27},
    {TX_PA_GAIN, 0x03, -24},
    {TX_PA_GAIN, 0x03, -21},
    {TX_PA_GAIN, 0x03, -18},
    {TX_PA_GAIN, 0x03, -15},
    {TX_PA_GAIN, 0x03, -12},
    {TX_PA_GAIN, 0x03, -9},
    {TX_PA_GAIN, 0x03, -6},
    {TX_PA_GAIN, 0x03, -3},
    {TX_PA_GAIN, 0x03, 0}, // 0dBm
};

// Dev mother board VMIC1 <---> CHIP VMIC2
// Dev mother board VMIC2 <---> CHIP VMIC1
#ifndef VMIC_MAP_CFG
#define VMIC_MAP_CFG                        AUD_VMIC_MAP_VMIC1
#endif

#if SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 2
#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | VMIC_MAP_CFG)
#elif SPEECH_CODEC_CAPTURE_CHANNEL_NUM == 3
#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1 | AUD_CHANNEL_MAP_CH4 | VMIC_MAP_CFG)
#else
#define CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV   (AUD_CHANNEL_MAP_CH0 | VMIC_MAP_CFG)
#endif

#define CFG_HW_AUD_INPUT_PATH_LINEIN_DEV    (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_CH1)
#ifdef VOICE_DETECTOR_SENS_EN
#define CFG_HW_AUD_INPUT_PATH_VADMIC_DEV    (AUD_CHANNEL_MAP_CH4 | VMIC_MAP_CFG)
#else
#define CFG_HW_AUD_INPUT_PATH_ASRMIC_DEV    (AUD_CHANNEL_MAP_CH0 | VMIC_MAP_CFG)
//#define CFG_HW_AUD_INPUT_PATH_ASRMIC_DEV    (AUD_CHANNEL_MAP_CH0 | AUD_CHANNEL_MAP_ECMIC_CH0 | VMIC_MAP_CFG)
#endif

#define CFG_HW_AUD_INPUT_PATH_ANC_ASSIST_DEV (ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L | ANC_TALK_MIC_CH_L | ANC_REF_MIC_CH_L | VMIC_MAP_CFG)

#define CFG_HW_AUD_INPUT_PATH_HEARING_DEV   (AUD_CHANNEL_MAP_CH0 | VMIC_MAP_CFG)

#define CFG_HW_AUD_INPUT_PATH_DC_CALIB      (CFG_ADC_DC_CALIB_MIC_DEV | VMIC_MAP_CFG)

const struct AUD_IO_PATH_CFG_T cfg_audio_input_path_cfg[CFG_HW_AUD_INPUT_PATH_NUM] = {
#if defined(SPEECH_TX_AEC_CODEC_REF)
    // NOTE: If enable Ch5 and CH6, need to add channel_num when setup audioflinger stream
    { AUD_INPUT_PATH_MAINMIC, CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV | AUD_CHANNEL_MAP_ECMIC_CH0, },
#else
    { AUD_INPUT_PATH_MAINMIC, CFG_HW_AUD_INPUT_PATH_MAINMIC_DEV, },
#endif
    { AUD_INPUT_PATH_LINEIN,  CFG_HW_AUD_INPUT_PATH_LINEIN_DEV, },
#ifdef VOICE_DETECTOR_SENS_EN
    { AUD_INPUT_PATH_VADMIC,  CFG_HW_AUD_INPUT_PATH_VADMIC_DEV, },
#else
    { AUD_INPUT_PATH_ASRMIC,  CFG_HW_AUD_INPUT_PATH_ASRMIC_DEV, },
#endif
    { AUD_INPUT_PATH_ANC_ASSIST,  CFG_HW_AUD_INPUT_PATH_ANC_ASSIST_DEV, },
#if defined(SPEECH_TX_AEC_CODEC_REF)
    { AUD_INPUT_PATH_HEARING,   CFG_HW_AUD_INPUT_PATH_HEARING_DEV | AUD_CHANNEL_MAP_ECMIC_CH0, },
#else
    { AUD_INPUT_PATH_HEARING,   CFG_HW_AUD_INPUT_PATH_HEARING_DEV, },
#endif
    {
        AUD_INPUT_PATH_DC_CALIB,
        CFG_HW_AUD_INPUT_PATH_DC_CALIB,
    },
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_enable_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_detecter_cfg = {
#ifdef CHARGER_1802
    HAL_IOMUX_PIN_P7_1, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
#else
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
#endif
};

const struct HAL_IOMUX_PIN_FUNCTION_MAP app_battery_ext_charger_indicator_cfg = {
    HAL_IOMUX_PIN_NUM, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
};

#define IIR_COUNTER_FF_L (6)
#define IIR_COUNTER_FF_R (6)
#define IIR_COUNTER_FB_L (5)
#define IIR_COUNTER_FB_R (5)

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode0 = {
    #ifndef VOICE_ASSIST_ADA_IIR
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_ff_l = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133132833, -265460650, 132663529},//3k
        // .iir_coef[2].coef_a={134217728, -265460650, 131578634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_ff_r = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_fb_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133853574, -267512069, 133696049},//1k
        // .iir_coef[2].coef_a={134217728, -267512069, 133331895},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_fb_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_tt_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={130003803,    -257605108,    128180940}, //4k test code
        // .iir_coef[2].coef_a={134217728,    -257605108,    123967016},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_tt_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_mc_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={131723630,    -260565855,    130644731},  //7K
        // .iir_coef[2].coef_a={134217728,    -260565855,    128150634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_mc_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#else
.anc_cfg_ff_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,

        //beijing
        .iir_counter = 8,
        .iir_coef[0].coef_b = {0x05ffe37e,0xf400afe2,0x05ff6cad},
        .iir_coef[0].coef_a = {0x08000000,0xf0005697,0x07ffa96e},
        .iir_coef[1].coef_b = {0x07fee72e,0xf00ab3ed,0x07f66e11},
        .iir_coef[1].coef_a = {0x08000000,0xf00ab3ed,0x07f5553f},
        .iir_coef[2].coef_b = {0x07ffce5a,0xf0022f6f,0x07fe030b},
        .iir_coef[2].coef_a = {0x08000000,0xf0022f6f,0x07fdd165},
        .iir_coef[3].coef_b = {0x08041ff0,0xf0207d53,0x07dc03bf},
        .iir_coef[3].coef_a = {0x08000000,0xf0207d53,0x07e023af},
        .iir_coef[4].coef_b = {0x029f7210,0xfb313e00,0x023e7e4c},
        .iir_coef[4].coef_a = {0x08000000,0xf0bb96d6,0x07539785},
        .iir_coef[5].coef_b = {0x075fafb0,0xf1c5615b,0x06f421c1},
        .iir_coef[5].coef_a = {0x08000000,0xf1c5615b,0x0653d170},

        // adaptive iir
        .iir_coef[6].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		.iir_coef[6].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[7].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		.iir_coef[7].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[8].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[8].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[9].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[9].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[10].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[10].coef_a={0x08000000,0xf024cb39,0x07dbd999},

/*		.fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
		.dac_gain_offset=0,
		.adc_gain_offset= -24, // (0)*4
    },
    .anc_cfg_fb_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 8,
        // adaptive iir
        .iir_coef[0].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[0].coef_a = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[1].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[1].coef_a = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[2].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[2].coef_a = {0x08000000,0xf024cb39,0x07dbd999},

        // real fb filter
        .iir_coef[3].coef_b = {0x110c37ba,0xde61c857,0x10acc11e},
        .iir_coef[3].coef_a = {0x08000000,0xf0fbbe6f,0x071034fb},
        .iir_coef[4].coef_b = {0x04855a96,0xf722ea8e,0x04596e08},
        .iir_coef[4].coef_a = {0x08000000,0xf03c4608,0x07c56d24},
        .iir_coef[5].coef_b = {0x0802921c,0xf0069ff2,0x07f6d2b5},
        .iir_coef[5].coef_a = {0x08000000,0xf0069ff2,0x07f964d0},
        .iir_coef[6].coef_b = {0x0800985a,0xf002e38c,0x07fc850b},
        .iir_coef[6].coef_a = {0x08000000,0xf002e38c,0x07fd1d65},
        .iir_coef[7].coef_b = {0x08017c7a,0xf0073dbe,0x07f74ef7},
        .iir_coef[7].coef_a = {0x08000000,0xf0073dbe,0x07f8cb71},
        // .iir_coef[5].coef_b = {0x07a15426,0xf0eb2d9a,0x0778602f},
        // .iir_coef[5].coef_a = {0x08000000,0xf0eb2d9a,0x0719b455},
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_mc_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 5,
        .iir_coef[0].coef_b = {0x05bb982e,0xf48c36ff,0x05b830ef},
        .iir_coef[0].coef_a = {0x08000000,0xf00225e0,0x07fdda48},
        .iir_coef[1].coef_b = {0x0801f05e,0xf006a3ee,0x07f76e57},
        .iir_coef[1].coef_a = {0x08000000,0xf006a3ee,0x07f95eb5},
        .iir_coef[2].coef_b = {0x080317ef,0xf00c1b92,0x07f0dbfd},
        .iir_coef[2].coef_a = {0x08000000,0xf00c1b92,0x07f3f3ec},
        .iir_coef[3].coef_b = {0x0483a30b,0xf7187409,0x04644641},
        .iir_coef[3].coef_a = {0x08000000,0xf02a2fe5,0x07d62d70},
        .iir_coef[4].coef_b = {0x07fe8c8c,0xf0115ef1,0x07f03395},
        .iir_coef[4].coef_a = {0x08000000,0xf0115ef1,0x07eec020},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
#endif
};

#ifdef VOICE_ASSIST_ADA_IIR
static struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_50p7k_mode1 = {
.anc_cfg_ff_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,

        //beijing
        .iir_counter = 8,
        .iir_coef[0].coef_b = {0x05ffe37e,0xf400afe2,0x05ff6cad},
        .iir_coef[0].coef_a = {0x08000000,0xf0005697,0x07ffa96e},
        .iir_coef[1].coef_b = {0x07fee72e,0xf00ab3ed,0x07f66e11},
        .iir_coef[1].coef_a = {0x08000000,0xf00ab3ed,0x07f5553f},
        .iir_coef[2].coef_b = {0x07ffce5a,0xf0022f6f,0x07fe030b},
        .iir_coef[2].coef_a = {0x08000000,0xf0022f6f,0x07fdd165},
        .iir_coef[3].coef_b = {0x08041ff0,0xf0207d53,0x07dc03bf},
        .iir_coef[3].coef_a = {0x08000000,0xf0207d53,0x07e023af},
        .iir_coef[4].coef_b = {0x029f7210,0xfb313e00,0x023e7e4c},
        .iir_coef[4].coef_a = {0x08000000,0xf0bb96d6,0x07539785},
        .iir_coef[5].coef_b = {0x075fafb0,0xf1c5615b,0x06f421c1},
        .iir_coef[5].coef_a = {0x08000000,0xf1c5615b,0x0653d170},

        // ada_ff
        .iir_coef[6].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		.iir_coef[6].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[7].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		.iir_coef[7].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[8].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[8].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[9].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[9].coef_a={0x08000000,0xf024cb39,0x07dbd999},
        // .iir_coef[10].coef_b={0x08000000,0xf024cb39,0x07dbd999},
		// .iir_coef[10].coef_a={0x08000000,0xf024cb39,0x07dbd999},

/*		.fir_bypass_flag=1,
        .fir_len = AUD_COEF_LEN,
        .fir_coef =
        {
            32767,
        },
*/
		.dac_gain_offset=0,
		.adc_gain_offset= -24, // (0)*4
    },

    .anc_cfg_fb_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 8,
        // ada_ff
        .iir_coef[0].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[0].coef_a = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[1].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[1].coef_a = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[2].coef_b = {0x08000000,0xf024cb39,0x07dbd999},
        .iir_coef[2].coef_a = {0x08000000,0xf024cb39,0x07dbd999},

        // real fb filter
        .iir_coef[3].coef_b = {0x110c37ba,0xde61c857,0x10acc11e},
        .iir_coef[3].coef_a = {0x08000000,0xf0fbbe6f,0x071034fb},
        .iir_coef[4].coef_b = {0x04855a96,0xf722ea8e,0x04596e08},
        .iir_coef[4].coef_a = {0x08000000,0xf03c4608,0x07c56d24},
        .iir_coef[5].coef_b = {0x0802921c,0xf0069ff2,0x07f6d2b5},
        .iir_coef[5].coef_a = {0x08000000,0xf0069ff2,0x07f964d0},
        .iir_coef[6].coef_b = {0x0800985a,0xf002e38c,0x07fc850b},
        .iir_coef[6].coef_a = {0x08000000,0xf002e38c,0x07fd1d65},
        .iir_coef[7].coef_b = {0x08017c7a,0xf0073dbe,0x07f74ef7},
        .iir_coef[7].coef_a = {0x08000000,0xf0073dbe,0x07f8cb71},
        // .iir_coef[5].coef_b = {0x07a15426,0xf0eb2d9a,0x0778602f},
        // .iir_coef[5].coef_a = {0x08000000,0xf0eb2d9a,0x0719b455},
        .dac_gain_offset = 0,
        .adc_gain_offset = -24,
    },
    .anc_cfg_mc_l = {
        .total_gain = 512,
        .iir_bypass_flag = 0,
        .iir_counter = 5,
        .iir_coef[0].coef_b = {0x05bb982e,0xf48c36ff,0x05b830ef},
        .iir_coef[0].coef_a = {0x08000000,0xf00225e0,0x07fdda48},
        .iir_coef[1].coef_b = {0x0801f05e,0xf006a3ee,0x07f76e57},
        .iir_coef[1].coef_a = {0x08000000,0xf006a3ee,0x07f95eb5},
        .iir_coef[2].coef_b = {0x080317ef,0xf00c1b92,0x07f0dbfd},
        .iir_coef[2].coef_a = {0x08000000,0xf00c1b92,0x07f3f3ec},
        .iir_coef[3].coef_b = {0x0483a30b,0xf7187409,0x04644641},
        .iir_coef[3].coef_a = {0x08000000,0xf02a2fe5,0x07d62d70},
        .iir_coef[4].coef_b = {0x07fe8c8c,0xf0115ef1,0x07f03395},
        .iir_coef[4].coef_a = {0x08000000,0xf0115ef1,0x07eec020},
        .iir_coef[5].coef_b = {0,0,0},
        .iir_coef[5].coef_a = {0,0,0},
        .iir_coef[6].coef_b = {0,0,0},
        .iir_coef[6].coef_a = {0,0,0},
        .iir_coef[7].coef_b = {0,0,0},
        .iir_coef[7].coef_a = {0,0,0},
        .dac_gain_offset = 0,
        .adc_gain_offset = 0,
    },
};
#endif

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_48k_mode0 = {
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_ff_l = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133132833, -265460650, 132663529},//3k
        // .iir_coef[2].coef_a={134217728, -265460650, 131578634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_ff_r = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_fb_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133853574, -267512069, 133696049},//1k
        // .iir_coef[2].coef_a={134217728, -267512069, 133331895},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_fb_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_tt_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={130003803,    -257605108,    128180940}, //4k test code
        // .iir_coef[2].coef_a={134217728,    -257605108,    123967016},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_tt_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (AUD_SECTION_STRUCT_VERSION != 1)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_mc_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={131723630,    -260565855,    130644731},  //7K
        // .iir_coef[2].coef_a={134217728,    -260565855,    128150634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_mc_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#endif
};

static const struct_anc_cfg POSSIBLY_UNUSED AncFirCoef_44p1k_mode0 = {
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_ff_l = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133132833, -265460650, 132663529},//3k
        // .iir_coef[2].coef_a={134217728, -265460650, 131578634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_ff_r = {
        .total_gain = 512 * 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_fb_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={133853574, -267512069, 133696049},//1k
        // .iir_coef[2].coef_a={134217728, -267512069, 133331895},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_fb_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_tt_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={130003803,    -257605108,    128180940}, //4k test code
        // .iir_coef[2].coef_a={134217728,    -257605108,    123967016},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_tt_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FF_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#if (AUD_SECTION_STRUCT_VERSION != 1)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH0)
    .anc_cfg_mc_l = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_L,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},
        // .iir_coef[2].coef_b={131723630,    -260565855,    130644731},  //7K
        // .iir_coef[2].coef_a={134217728,    -260565855,    128150634},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#if ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
#if (CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV & AUD_CHANNEL_MAP_CH1)
    .anc_cfg_mc_r = {
        .total_gain = 512 / 2,

        .iir_bypass_flag = 0,
        .iir_counter = IIR_COUNTER_FB_R,

        .iir_coef[0].coef_b = {0x8000000, 0, 0},
        .iir_coef[0].coef_a = {0x8000000, 0, 0},

        .iir_coef[1].coef_b = {0x8000000, 0, 0},
        .iir_coef[1].coef_a = {0x8000000, 0, 0},

        .iir_coef[2].coef_b = {0x8000000, 0, 0},
        .iir_coef[2].coef_a = {0x8000000, 0, 0},

        .iir_coef[3].coef_b = {0x8000000, 0, 0},
        .iir_coef[3].coef_a = {0x8000000, 0, 0},

        .iir_coef[4].coef_b = {0x8000000, 0, 0},
        .iir_coef[4].coef_a = {0x8000000, 0, 0},

        .iir_coef[5].coef_b = {0x8000000, 0, 0},
        .iir_coef[5].coef_a = {0x8000000, 0, 0},

        .dac_gain_offset = 0,
        .adc_gain_offset = (0) * 4,
    },
#endif
#endif
#endif
};

const struct_anc_cfg *anc_coef_list_50p7k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_50p7k_mode0,
    #ifdef VOICE_ASSIST_ADA_IIR
    &AncFirCoef_50p7k_mode1,
    #endif
};

const struct_anc_cfg *anc_coef_list_48k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_48k_mode0,
};

const struct_anc_cfg *anc_coef_list_44p1k[ANC_COEF_LIST_NUM] = {
    &AncFirCoef_44p1k_mode0,
};

static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_44p1k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain = {0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b = {0x04fa1dfc, 0x0c6d6b7e, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a = {0x08000000, 0x0c6d6b7e, 0x04fa1dfc, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b = {0x04f9f115, 0x11279bfc, 0x1404d0c8, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a = {0x08000000, 0x1404d0c8, 0x11279bfc, 0x04f9f115},
        .psap_cpd_cfg[0] = {0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg = {524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay = 0,
        .psap_dehowling_cfg.dehowling_l.total_gain = 0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag = 0,
        .psap_dehowling_cfg.dehowling_l.iir_counter = 1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b = {0x08000000, 0xf096cb51, 0x07733cd7},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a = {0x08000000, 0xf096cb51, 0x07733cd7},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset = 0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset = 0,
        .psap_type = 3,
        .psap_dac_gain_offset = 0,
        .psap_adc_gain_offset = 0,
    }};
static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_48k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain = {0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b = {0x0364529c, 0x09dd9c0b, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a = {0x08000000, 0x09dd9c0b, 0x0364529c, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b = {0x0361ed64, 0x0cc9bec2, 0x109eeca2, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a = {0x08000000, 0x109eeca2, 0x0cc9bec2, 0x0361ed64},
        .psap_cpd_cfg[0] = {0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg = {524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay = 0,
        .psap_dehowling_cfg.dehowling_l.total_gain = 0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag = 0,
        .psap_dehowling_cfg.dehowling_l.iir_counter = 1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b = {0x08000000, 0xf08a323c, 0x077e4bc1},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a = {0x08000000, 0xf08a323c, 0x077e4bc1},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset = 0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset = 0,
        .psap_type = 3,
        .psap_dac_gain_offset = 0,
        .psap_adc_gain_offset = 0,
    }};
static const struct_psap_cfg POSSIBLY_UNUSED hwlimiter_para_50p7k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 811,
        .psap_band_num = 2,
        .psap_band_gain = {0x3f8bd79e, 0x08000000},
        .psap_iir_coef[0].iir0.coef_b = {0x03c46400, 0x0a8a3bd5, 0x08000000, 0x00000000},
        .psap_iir_coef[0].iir0.coef_a = {0x08000000, 0x0a8a3bd5, 0x03c46400, 0x00000000},
        .psap_iir_coef[0].iir1.coef_b = {0x03c305c1, 0x0de12c8a, 0x118f7298, 0x08000000},
        .psap_iir_coef[0].iir1.coef_a = {0x08000000, 0x118f7298, 0x0de12c8a, 0x03c305c1},
        .psap_cpd_cfg[0] = {0x5740, 0x0000, 0x4e02, 0x0000, 0x3b85, 0x0000, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 0x7333, 0x0ccd, 0},
        .psap_limiter_cfg = {524287, 0x7cf7, 0x0309, 0x7ff0, 0x0010, 127},
        .psap_dehowling_cfg.dehowling_delay = 0,
        .psap_dehowling_cfg.dehowling_l.total_gain = 0,
        .psap_dehowling_cfg.dehowling_l.iir_bypass_flag = 0,
        .psap_dehowling_cfg.dehowling_l.iir_counter = 1,
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_b = {0x08000000, 0xf08d9c03, 0x077b49d0},
        .psap_dehowling_cfg.dehowling_l.iir_coef[0].coef_a = {0x08000000, 0xf08d9c03, 0x077b49d0},
        .psap_dehowling_cfg.dehowling_l.dac_gain_offset = 0,
        .psap_dehowling_cfg.dehowling_l.adc_gain_offset = 0,
        .psap_type = 3,
        .psap_dac_gain_offset = 0,
        .psap_adc_gain_offset = 0,
    }};

const struct_psap_cfg *hwlimiter_para_list_50p7k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_50p7k_mode0,
};

const struct_psap_cfg *hwlimiter_para_list_48k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_48k_mode0,
};

const struct_psap_cfg *hwlimiter_para_list_44p1k[HWLIMITER_PARA_LIST_NUM] = {
    &hwlimiter_para_44p1k_mode0,
};

static const struct_psap_cfg POSSIBLY_UNUSED PsapFirCoef_50p7k_mode0 = {
    .psap_cfg_l = {
        .psap_total_gain = 723,
        .psap_band_num = 9,
        .psap_band_gain = {0, 476222470, 672682118, 534330399, 950188747, 1066129310, 1066129310, 424433723, 0},
        .psap_iir_coef[0].iir0.coef_b = {131557370, -265754754, 134217728, 0},
        .psap_iir_coef[0].iir0.coef_a = {134217728, -265754754, 131557370, 0},
        .psap_iir_coef[0].iir1.coef_b = {-131557370, 397299677, -399959783, 134217728},
        .psap_iir_coef[0].iir1.coef_a = {134217728, -399959783, 397299677, -131557370},
        .psap_iir_coef[1].iir0.coef_b = {129986488, -264152445, 134217728, 0},
        .psap_iir_coef[1].iir0.coef_a = {134217728, -264152445, 129986488, 0},
        .psap_iir_coef[1].iir1.coef_b = {-129986488, 394107454, -398337669, 134217728},
        .psap_iir_coef[1].iir1.coef_a = {134217728, -398337669, 394107454, -129986488},
        .psap_iir_coef[2].iir0.coef_b = {125134130, -259108702, 134217728, 0},
        .psap_iir_coef[2].iir0.coef_a = {134217728, -259108702, 125134130, 0},
        .psap_iir_coef[2].iir1.coef_b = {-125134129, 384097929, -393171000, 134217728},
        .psap_iir_coef[2].iir1.coef_a = {134217728, -393171000, 384097929, -125134129},
        .psap_iir_coef[3].iir0.coef_b = {120220377, -253849171, 134217728, 0},
        .psap_iir_coef[3].iir0.coef_a = {134217728, -253849171, 120220377, 0},
        .psap_iir_coef[3].iir1.coef_b = {-120220366, 373726258, -387683554, 134217728},
        .psap_iir_coef[3].iir1.coef_a = {134217728, -387683554, 373726258, -120220366},
        .psap_iir_coef[4].iir0.coef_b = {109848832, -242200690, 134217728, 0},
        .psap_iir_coef[4].iir0.coef_a = {134217728, -242200690, 109848832, 0},
        .psap_iir_coef[4].iir1.coef_b = {-109848632, 351018377, -375156857, 134217728},
        .psap_iir_coef[4].iir1.coef_a = {134217728, -375156857, 351018377, -109848632},
        .psap_iir_coef[5].iir0.coef_b = {81171724, -205166742, 134217728, 0},
        .psap_iir_coef[5].iir0.coef_a = {134217728, -205166742, 81171724, 0},
        .psap_iir_coef[5].iir1.coef_b = {-81156770, 281801460, -331721088, 134217728},
        .psap_iir_coef[5].iir1.coef_a = {134217728, -331721088, 281801460, -81156770},
        .psap_iir_coef[6].iir0.coef_b = {66181540, -181899702, 134217728, 0},
        .psap_iir_coef[6].iir0.coef_a = {134217728, -181899702, 66181540, 0},
        .psap_iir_coef[6].iir1.coef_b = {-66113394, 241306967, -301500735, 134217728},
        .psap_iir_coef[6].iir1.coef_a = {134217728, -301500735, 241306967, -66113394},
        .psap_iir_coef[7].iir0.coef_b = {62863906, -176244865, 134217728, 0},
        .psap_iir_coef[7].iir0.coef_a = {134217728, -176244865, 62863906, 0},
        .psap_iir_coef[7].iir1.coef_b = {-62771443, 231892680, -293809571, 134217728},
        .psap_iir_coef[7].iir1.coef_a = {134217728, -293809571, 231892680, -62771443},
        .psap_cpd_cfg[0] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[1] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[2] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[3] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[4] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[5] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[6] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_cpd_cfg[7] = {27069, 0, 27069, 0, 10505, -29491, 32689, 79, 32766, 2, 29491, 3277, 0},
        .psap_limiter_cfg = {524287, 31991, 777, 32752, 16, 0},
        .psap_dac_gain_offset = 0,
        .psap_adc_gain_offset = -12,
    }

};

const struct_psap_cfg *psap_coef_list_50p7k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};

const struct_psap_cfg *psap_coef_list_48k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};

const struct_psap_cfg *psap_coef_list_44p1k[PSAP_COEF_LIST_NUM] = {
    &PsapFirCoef_50p7k_mode0,
};

const IIR_CFG_T audio_eq_sw_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 5,
    .param = {
        {IIR_TYPE_PEAK, .0,   200,   2},
        {IIR_TYPE_PEAK, .0,   600,  2},
        {IIR_TYPE_PEAK, .0,   2000.0, 2},
        {IIR_TYPE_PEAK, .0,  6000.0, 2},
        {IIR_TYPE_PEAK, .0,  12000.0, 2}
    }
};

const IIR_CFG_T * const audio_eq_sw_iir_cfg_list[EQ_SW_IIR_LIST_NUM]={
    &audio_eq_sw_iir_cfg,
};

const FIR_CFG_T audio_eq_hw_fir_cfg_44p1k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};

const FIR_CFG_T audio_eq_hw_fir_cfg_48k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};


const FIR_CFG_T audio_eq_hw_fir_cfg_96k = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1<<23)-1,
    }
};

FIR_CFG_T audio_eq_hw_fir_adaptive_eq_cfg = {
    .gain = 0.0f,
    .len = 384,
    .coef =
    {
        (1 << 23) - 1,
    }
};

const FIR_CFG_T * const audio_eq_hw_fir_cfg_list[EQ_HW_FIR_LIST_NUM]={
    &audio_eq_hw_fir_cfg_44p1k,
    &audio_eq_hw_fir_cfg_48k,
    &audio_eq_hw_fir_cfg_96k,
};

//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_dac_iir_cfg = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
    }
};

const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
    }
};

#ifdef CUSTOMER_APP_BOAT
const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg_1 = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = -5,
    .gain1 = -5,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
    }
};

const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg_2 = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = -10,
    .gain1 = -10,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
    }
};

const IIR_CFG_T audio_eq_anc_hw_dac_iir_cfg_3 = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = -15,
    .gain1 = -15,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
        {IIR_TYPE_PEAK, 0, 1000.0, 0.7},
    }
};
#endif

IIR_CFG_T audio_eq_hw_dac_iir_adaptive_eq_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 1,
    .param = {
        {IIR_TYPE_PEAK, 0, 1000, 0.7},
    }
};

const IIR_CFG_T *const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list[EQ_HW_DAC_IIR_LIST_NUM] = {
    &audio_eq_hw_dac_iir_cfg,
    &audio_eq_anc_hw_dac_iir_cfg,
#ifdef CUSTOMER_APP_BOAT
    &audio_eq_anc_hw_dac_iir_cfg_1,
    &audio_eq_anc_hw_dac_iir_cfg_2,
    &audio_eq_anc_hw_dac_iir_cfg_3,
#endif
};

#if defined(__IIR_EQ_PROCESS_LR_2CH__)
const IIR_CFG_T audio_eq_hw_dac_iir_cfg_2 = {
#if defined(AUDIO_HEARING_COMPSATN)
    .gain0 = -22,
    .gain1 = -22,
#else
    .gain0 = 0,
    .gain1 = 0,
#endif
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
        {IIR_TYPE_PEAK, 0,   1000.0,   0.7},
    }
};

// R channel iir
const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_dac_iir_cfg_list2[EQ_HW_DAC_IIR_LIST_NUM]={
    &audio_eq_hw_dac_iir_cfg_2,
};
#endif

//hardware dac iir eq
const IIR_CFG_T audio_eq_hw_adc_iir_adc_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 1,
    .param = {
        {IIR_TYPE_PEAK, 0.0,   1000.0,   0.7},
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_adc_iir_cfg_list[EQ_HW_ADC_IIR_LIST_NUM]={
    &audio_eq_hw_adc_iir_adc_cfg,
};

//hardware iir eq
const IIR_CFG_T audio_eq_hw_iir_cfg = {
    .gain0 = 0,
    .gain1 = 0,
    .num = 8,
    .param = {
        {IIR_TYPE_PEAK, -10.1,   100.0,   7},
        {IIR_TYPE_PEAK, -10.1,   400.0,   7},
        {IIR_TYPE_PEAK, -10.1,   700.0,   7},
        {IIR_TYPE_PEAK, -10.1,   1000.0,  7},
        {IIR_TYPE_PEAK, -10.1,   3000.0,  7},
        {IIR_TYPE_PEAK, -10.1,   5000.0,  7},
        {IIR_TYPE_PEAK, -10.1,   7000.0,  7},
        {IIR_TYPE_PEAK, -10.1,   9000.0,  7},
    }
};

const IIR_CFG_T * const POSSIBLY_UNUSED audio_eq_hw_iir_cfg_list[EQ_HW_IIR_LIST_NUM]={
    &audio_eq_hw_iir_cfg,
};

const DrcConfig audio_drc_cfg = {
     .knee = 3,
     .filter_type = {14, -1},
     .band_num = 2,
     .look_ahead_time = 10,
     .band_settings = {
         {-20, 0, 2, 3, 3000, 1},
         {-20, 0, 2, 3, 3000, 1},
     }
 };

const LimiterConfig audio_limiter_cfg = {
    .knee = 2,
    .look_ahead_time = 10,
    .threshold = -20,
    .makeup_gain = 19,
    .ratio = 1000,
    .attack_time = 3,
    .release_time = 3000,
};

const SpectrumFixConfig audio_spectrum_cfg = {
    .freq_num = 9,
    .freq_list = {200, 400, 600, 800, 1000, 1200, 1400, 1600, 1800},
};

const ReverbConfig audio_reverb_cfg = {
    .bypass = 0,
    .high_pass_f0 = 100,
    .gain = 0,
};

const BassEnhancerConfig audio_bass_cfg =
{
    .low_cut_freq = 100,
    .high_cut_freq = 200,
    .gain0 = 0,
    .gain1 = -40,
};

const DynamicBoostConfig audio_dynamic_boost_cfg = {
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
            .type = IIR_TYPE_PEAK,
            .gain = 10,
            .freq = 33,
            .Q = 0.5,
        },
        {
            .type = IIR_TYPE_PEAK,
            .gain = -1,
            .freq = 240,
            .Q = 1.1,
        },
        {
            .type = IIR_TYPE_PEAK,
            .gain = 10,
            .freq = 1000, // -1 for unused eq
            .Q = 0.7,
        },
        {
            .gain = 10,
            .freq = 2000, // -1 for unused eq
            .Q = 0.7,
        }
    }
};

const VirtualSurroundConfig audio_virtual_surround_cfg =
{
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
    .output_ratio = 100,

    .hrtf_input_ratio = 100,
    .hrtf_output_ratio = 100,
    .hrtf_select_index = 0,
};

const DynamicEqConfig audio_dynamic_eq_cfg = {
    .switch_on = 1,
    .debug = 0,
    .eq_num = 6,
    .dyeq_param_cfg = {
        /*
            {
                .dyeq_eq_cfg = {type,   gain,   fc,   q}
                .dyeq_drc_cfg = {CT,   CR,   WT,   WR,   ET,   ER,   attack,   release}
            }
        */
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
};