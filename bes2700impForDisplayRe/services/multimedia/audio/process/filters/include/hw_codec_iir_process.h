/***************************************************************************
 * @file      hw_codec_iir_process.h
 * @brief     Public header file for hardware codec iir filter
 * @version   0.1
 * @date      2021-02-09
 ***************************************************************************/
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
#ifndef __HW_CODEC_IIR_PROCESS_H__
#define __HW_CODEC_IIR_PROCESS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "hal_aud.h"
#include "iir_process.h"
#include "hw_dac_iir_limiter.h"

/**
 * @defgroup hw_codec_iir Hardware Codec Infinite Impulse Response (IIR) Filter
 *
 * Control codec iir filter.
 *
 * Basic usage can be found in audio_process.c. Below is a simple example.
 *
 * @code{.c}
 * // init hw codec iir
 * int ret = hw_codec_iir_open(stream_cfg.sample_rate, HW_CODEC_IIR_DAC, CODEC_OUTPUT_DEV);
 * ASSERT(ret == 0, "[%s] hw codec iir open error %d", __FUNCTION__, ret);

 * // calculate the coeffs for hw codec iir, or use your own calculation algorithm
 * HW_CODEC_IIR_CFG_T *hw_codec_iir_cfg = hw_codec_iir_get_cfg(stream_cfg.sample_rate, &iir_cfg);

 * // set coeffs
 * ret = hw_codec_iir_set_cfg(hw_codec_iir_cfg, stream_cfg.sample_rate, HW_CODEC_IIR_DAC);
 * ASSERT(ret == 0, "[%s] hw codec iir set cfg error %d", __FUNCTION__, ret);
 * @endcode
 */

/**
 * @addtogroup hw_codec_iir
 * @{
 */

/**
 * @brief Max supported IIR band num
 */
#if defined(CHIP_BEST1402) || defined(CHIP_BEST2001) || defined(CHIP_BEST2300A) || defined(CHIP_BEST1305) || defined(CHIP_BEST1306)
#define AUD_DAC_IIR_NUM_EQ                        (20)
#elif defined(CHIP_BEST1501) || defined(CHIP_BEST1501P) || defined(CHIP_BEST1600) || defined(CHIP_BEST1502X) \
|| defined(CHIP_BEST1502P) || defined(CHIP_BEST1503) || defined(CHIP_BEST1306P)
#define AUD_DAC_IIR_NUM_EQ                        (40)
#else
#define AUD_DAC_IIR_NUM_EQ                        (8)
#endif

/**
 * @brief Error status
 */
typedef enum {
    HW_CODEC_IIR_NO_ERR=0,         /**< No error */
    HW_CODEC_IIR_TYPE_ERR,         /**< Hardware IIR type is incorrect */
    HW_CODEC_IIR_SAMPLERATE_ERR,   /**< Samplerate is incorrect */
    HW_CODEC_IIR_COUNTER_ERR,      /**< IIR band num is incorrect */
    HW_CODEC_IIR_OTHER_ERR,        /**< Some other error */
    HW_CODEC_IIR_ERR_TOTAL,        /**< Total error num */
}HW_CODEC_IIR_ERROR;

/**
 * @brief Hardware IIR type
 */
typedef enum  {
    HW_CODEC_IIR_NOTYPE = 0,     /**< Invalid type */
    HW_CODEC_IIR_DAC,            /**< Hardware Codec IIR in DAC */
    HW_CODEC_IIR_ADC,            /**< Hardware Codec IIR in ADC */
}HW_CODEC_IIR_TYPE_T;

/**
 * @brief Codec iir band coefficients in q27
 */
typedef struct _hw_codec_iir_coefs
{
    int32_t coef_b[3];  /**< b0 b1 b2 */
    int32_t coef_a[3];  /**< a0 a1 a2 */
}hw_codec_iir_coefs;

/**
 * @brief Codec iir band coefficients in f32
 */
typedef struct _hw_codec_iir_coefs_f
{
    float coef_b[3];  /**< b0 b1 b2 */
    float coef_a[3];  /**< a0 a1 a2 */
}hw_codec_iir_coefs_f;

/**
 * @brief Codec iir filter coefficients in q27
 */
typedef struct _hw_codec_iir_filters
{
    uint16_t iir_bypass_flag;                         /**< 0 - enable, 1 - disable */
    uint16_t iir_counter;                             /**< Band num */
    hw_codec_iir_coefs iir_coef[AUD_DAC_IIR_NUM_EQ];  /**< Coefficient array */
} HW_CODEC_IIR_FILTERS_T;

/**
 * @brief Codec iir filter coefficients in f32
 */
typedef struct _hw_codec_iir_filters_f
{
    uint16_t iir_bypass_flag;                           /**< 0 - enable, 1 - disable */
    uint16_t iir_counter;                               /**< Band num */
    hw_codec_iir_coefs_f iir_coef[AUD_DAC_IIR_NUM_EQ];  /**< Coefficient array */
} HW_CODEC_IIR_FILTERS_F;

/**
 * @brief Codec iir filter config in q27
 */
typedef struct _hw_codec_iir_cfg_t
{
    HW_CODEC_IIR_FILTERS_T iir_filtes_l;                /**< Filter config for left channel */
    HW_CODEC_IIR_FILTERS_T iir_filtes_r;                /**< Filter config for right channel */
} HW_CODEC_IIR_CFG_T;

/**
 * @brief Codec iir filter config in f32
 */
typedef struct _hw_codec_iir_cfg_f
{
    HW_CODEC_IIR_FILTERS_F iir_filtes_l;               /**< Filter config for left channel */
    HW_CODEC_IIR_FILTERS_F iir_filtes_r;               /**< Filter config for right channel */
} HW_CODEC_IIR_CFG_F;

/**
 * @brief                       Open codec iir
 *
 * @param  sample_rate          Processing sample rate
 * @param  hw_iir_type          Codec iir type
 *                                - \ref HW_CODEC_IIR_DAC
 *                                - \ref HW_CODEC_IIR_ADC
 * @param  ch_map               Channel map
 *                                - When hw_iir_type is \ref HW_CODEC_IIR_DAC, it should be \ref CODEC_OUTPUT_DEV
 *                                - When hw_iir_type is \ref HW_CODEC_IIR_ADC, it should be mic cfg
 * @return \ref HW_CODEC_IIR_ERROR
 */
int hw_codec_iir_open(enum AUD_SAMPRATE_T sample_rate,  HW_CODEC_IIR_TYPE_T hw_iir_type, int32_t ch_map);

/**
 * @brief                       Set q27 coeffs for codec iir filter
 *
 * @param  cfg                  Config for codec iir filter, q27 coeffs
 * @param  sample_rate          Processing sample rate
 * @param  hw_iir_type          Codec iir type
 *                                - \ref HW_CODEC_IIR_DAC
 *                                - \ref HW_CODEC_IIR_ADC
 * @return \ref HW_CODEC_IIR_ERROR
 */
int hw_codec_iir_set_cfg( HW_CODEC_IIR_CFG_T *cfg, enum AUD_SAMPRATE_T sample_rate, HW_CODEC_IIR_TYPE_T hw_iir_type);

HW_CODEC_IIR_CFG_T *hw_codec_iir_convert_cfg(HW_CODEC_IIR_CFG_F *cfg);

/**
 * @brief                       Close codec iir filter
 *
 * @param  hw_iir_type          Codec iir type
 *                                - \ref HW_CODEC_IIR_DAC
 *                                - \ref HW_CODEC_IIR_ADC
 * @return \ref HW_CODEC_IIR_ERROR
 */
int hw_codec_iir_close( HW_CODEC_IIR_TYPE_T hw_iir_type);

/**
 * @brief                       Calculate HW_CODEC_IIR_CFG_T for IIR_CFG_T
 *
 * @param sample_rate           Processing sample rate
 * @param ext_cfg               IIR band config specify f0, q and gain, \ref IIR_CFG_T
 * @return \ref HW_CODEC_IIR_CFG_T
 */
HW_CODEC_IIR_CFG_T *hw_codec_iir_get_cfg(enum AUD_SAMPRATE_T sample_rate,const IIR_CFG_T* ext_cfg);

/**
 * @brief                       Set f32 coeffs for codec iir filter
 *
 * @param  cfg                  Config for codec iir filter, f32 coeffs
 * @param  hw_iir_type          Codec iir type
 *                                - \ref HW_CODEC_IIR_DAC
 *                                - \ref HW_CODEC_IIR_ADC
 * @return \ref HW_CODEC_IIR_ERROR
 */
int hw_codec_iir_set_coefs(HW_CODEC_IIR_CFG_F *cfg, HW_CODEC_IIR_TYPE_T hw_iir_type);

/**
 * @brief                       Print config coefficients
 *
 * @param  cfg                  Config for codec iir filter
 * @return \ref HW_CODEC_IIR_ERROR
 */
int hal_codec_iir_dump(HW_CODEC_IIR_CFG_T *cfg);

void hal_codec_iir_heap_register(HW_CODEC_IIR_FILTERS_T* l_old,HW_CODEC_IIR_FILTERS_T* r_old,HW_CODEC_IIR_FILTERS_T* cfg_l,HW_CODEC_IIR_FILTERS_T* cfg_r,HW_CODEC_IIR_FILTERS_T* new_l,HW_CODEC_IIR_FILTERS_T* new_r );
/**
 * @} end of hw_codec_iir group
 */
int hal_codec_iir_set_limiter_cfg(const limiter_cfg_t* cfg);

#ifdef __cplusplus
}
#endif

#endif
