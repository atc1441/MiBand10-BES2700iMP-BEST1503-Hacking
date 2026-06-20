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
#ifndef __aud_section_h__
#define __aud_section_h__

#include "plat_types.h"
#include "section_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"

#define aud_section_debug
#ifdef aud_section_debug
#define aud_trace     TRACE
#else
#define aud_trace(...)
#endif
#include "hal_aud.h"
#define audsec_tag  "audsec_tag"

#define aud_section_magic               0xdad1
#define aud_section_struct_version      1

#define AUD_COEF_LEN        (500)
#if (AUD_SECTION_STRUCT_VERSION == 1)
#define AUD_IIR_NUM         (6)
#elif (AUD_SECTION_STRUCT_VERSION == 2)
#define AUD_IIR_NUM         (8)
#elif (AUD_SECTION_STRUCT_VERSION == 3)
#define AUD_IIR_NUM         (8)
#elif (AUD_SECTION_STRUCT_VERSION == 4)
#define AUD_IIR_NUM         (13)
#else
#error "Can not support this version!!!"
#endif


#define AUD_PSAP_IIR_NUM         (16)
#define AUD_PSAP_BAND_NUM         (AUD_PSAP_IIR_NUM+1)

typedef struct _anc_rir_coefs {
    int32_t coef_b[3];
    int32_t coef_a[3];
} anc_iir_coefs;

typedef struct _aud_item {
    int32_t     total_gain;

    uint16_t   iir_bypass_flag;
    uint16_t   iir_counter;
    anc_iir_coefs   iir_coef[AUD_IIR_NUM];

#if (AUD_SECTION_STRUCT_VERSION == 1)
    uint16_t    fir_bypass_flag;
    uint16_t    fir_len;
    int16_t     fir_coef[AUD_COEF_LEN];
    int8_t      pos_tab[16];
#elif (AUD_SECTION_STRUCT_VERSION == 2)
    uint16_t   limiter_flag;
    uint16_t   limiter_reserved;
    float    limiter_th;
    float    limiter_att_time;
    float    limiter_rls_time;
    int32_t    reserved_for_drc[32-4];
#elif (AUD_SECTION_STRUCT_VERSION == 3)

#elif (AUD_SECTION_STRUCT_VERSION == 4)
    uint16_t   limiter_flag;
    uint16_t   limiter_reserved;
    float    limiter_th;
    float    limiter_att_time;
    float    limiter_rls_time;
#endif
    int16_t     reserved1;
    int8_t      dac_gain_offset; // in qdb (quater of dB)
    int8_t      adc_gain_offset; // in qdb (quater of dB)
} aud_item;

typedef struct {
#if (AUD_SECTION_STRUCT_VERSION == 1)
    aud_item anc_cfg_ff_l;
    aud_item anc_cfg_ff_r;
    aud_item anc_cfg_fb_l;
    aud_item anc_cfg_fb_r;
#elif ((AUD_SECTION_STRUCT_VERSION & 0x1) == 0)
    aud_item anc_cfg_ff_l;
    aud_item anc_cfg_ff_r;
    aud_item anc_cfg_fb_l;
    aud_item anc_cfg_fb_r;

    aud_item anc_cfg_tt_l;
    aud_item anc_cfg_tt_r;
    aud_item anc_cfg_mc_l;
    aud_item anc_cfg_mc_r;
#elif ((AUD_SECTION_STRUCT_VERSION & 0x1) != 0)
    aud_item anc_cfg_ff_l;
    aud_item anc_cfg_fb_l;
    aud_item anc_cfg_tt_l;
    aud_item anc_cfg_mc_l;
#endif

} struct_anc_cfg;

typedef struct _psap_iir_coefs {
    int32_t coef_b[4];
    int32_t coef_a[4];
} psap_iir_coefs;

typedef struct _psap_cross_over_coefs
{
    psap_iir_coefs iir0;
    psap_iir_coefs iir1;
}psap_cross_over_coefs;

typedef struct _psap_cpd_config
{
    uint16_t psap_cpd_ct;
    uint16_t psap_cpd_cs;
    uint16_t psap_cpd_wt;
    uint16_t psap_cpd_ws;
    uint16_t psap_cpd_et;
    uint16_t psap_cpd_es;
    uint16_t psap_cpd_coefa_at;
    uint16_t psap_cpd_coefb_at;
    uint16_t psap_cpd_coefa_rt;
    uint16_t psap_cpd_coefb_rt;
    uint16_t psap_cpd_tava;
    uint16_t psap_cpd_tavb;
    uint16_t psap_cpd_delay;
    uint16_t psap_cpd_gl;
}psap_cpd_config;

typedef struct _psap_limiter_config
{
    uint32_t psap_limiter_th;
    uint16_t psap_limiter_coefa_at;
    uint16_t psap_limiter_coefb_at;
    uint16_t psap_limiter_coefa_rt;
    uint16_t psap_limiter_coefb_rt;
    uint16_t psap_limiter_delay;
    uint16_t reserved1;
}psap_limiter_config;

typedef struct _dehowling_config
{
    uint32_t dehowling_delay;
    aud_item dehowling_l;
}dehowling_config;

typedef struct _struct_spkcalib_cfg
{
    aud_item spkcalib_cfg_l;
}struct_spkcalib_cfg;

typedef struct _struct_psap_eq_cfg
{
    aud_item psap_eq_cfg_l;
}struct_psap_eq_cfg;

typedef struct _aud_item_psap{
    int32_t     psap_total_gain;
    int32_t     psap_band_num;
    int32_t     psap_band_gain[AUD_PSAP_BAND_NUM];
    psap_cpd_config psap_cpd_cfg[AUD_PSAP_BAND_NUM];
    psap_cross_over_coefs   psap_iir_coef[AUD_PSAP_IIR_NUM];
    psap_limiter_config psap_limiter_cfg;
    dehowling_config psap_dehowling_cfg;
    int32_t     reserved_for_drc[32];
    int16_t     psap_type;
    int8_t     psap_dac_gain_offset; // in qdb (quater of dB)
    int8_t     psap_adc_gain_offset; // in qdb (quater of dB)
} aud_item_psap;

typedef struct {
    aud_item_psap psap_cfg_l;
    aud_item_psap psap_cfg_r;
} struct_psap_cfg;


enum ANC_INDEX {
    ANC_INDEX_0 = 0,
    ANC_INDEX_1,
    ANC_INDEX_2,
    ANC_INDEX_3,
    ANC_INDEX_TOTAL,
};

typedef struct {
    unsigned char anc_ver[16];
    unsigned char batch_info[16];
    unsigned char serial[16];
} anc_ident;


enum auditem_sample_enum_t {
    PCTOOL_SAMPLERATE_44_1X8K,
#if (AUD_SECTION_STRUCT_VERSION == 4)
    PCTOOL_SAMPLERATE_48X8K = PCTOOL_SAMPLERATE_44_1X8K,
    PCTOOL_SAMPLERATE_50_7X8K = PCTOOL_SAMPLERATE_44_1X8K,
#else
    PCTOOL_SAMPLERATE_48X8K,
    PCTOOL_SAMPLERATE_50_7X8K = PCTOOL_SAMPLERATE_48X8K,
#endif
    PCTOOL_AUDITEM_SAMPLERATE_COUNT
};

typedef struct {
    struct_anc_cfg    anc_cfg[PCTOOL_AUDITEM_SAMPLERATE_COUNT];
} pctool_struct_anc_cfg;

typedef struct {
    struct_spkcalib_cfg    spkcalib_cfg[PCTOOL_AUDITEM_SAMPLERATE_COUNT];
} pctool_struct_spkcalib_cfg;

typedef struct {
    struct_psap_cfg    psap_cfg[PCTOOL_AUDITEM_SAMPLERATE_COUNT];
} pctool_struct_psap_cfg;

#if ((AUD_SECTION_STRUCT_VERSION == 3) || (AUD_SECTION_STRUCT_VERSION == 4))
#define PCTOOL_ANC_APPMODE_COUNT      10
#define ANC_SECTIONE_SIZE      (64*1024)
#else
#define PCTOOL_ANC_APPMODE_COUNT      4
#define ANC_SECTIONE_SIZE      (64*1024)
#endif

#define PCTOOL_PSAP_APPMODE_COUNT      4

#define PCTOOL_SPKCALIB_APPMODE_COUNT      2

typedef struct {
    pctool_struct_anc_cfg anc_config_arr[PCTOOL_ANC_APPMODE_COUNT];
} pctool_anc_config_t;

typedef struct {
    pctool_struct_spkcalib_cfg spkcalib_config_arr[PCTOOL_SPKCALIB_APPMODE_COUNT];
} pctool_spkcalib_config_t;

typedef struct {
    pctool_struct_psap_cfg psap_config_arr[PCTOOL_PSAP_APPMODE_COUNT];
} pctool_psap_config_t;


#define PCTOOL_AUDSEC_PSAP_LEN (sizeof(pctool_psap_config_t))
#define PCTOOL_AUDSEC_SPKCALIB_LEN (sizeof(pctool_spkcalib_config_t))

#define PCTOOL_AUDSEC_RESERVED_LEN (ANC_SECTIONE_SIZE -sizeof(section_head_t) - sizeof(anc_ident) - sizeof(pctool_anc_config_t)-PCTOOL_AUDSEC_SPKCALIB_LEN-PCTOOL_AUDSEC_PSAP_LEN)


typedef struct {
    anc_ident               ancIdent;
    pctool_anc_config_t   anc_config;
#ifndef CHIP_BEST1306
    pctool_spkcalib_config_t spkcalib_config;
    pctool_psap_config_t   psap_config;
#endif
    unsigned char           reserved[PCTOOL_AUDSEC_RESERVED_LEN];
} audsec_body;

typedef struct {
    section_head_t  sec_head;
    audsec_body     sec_body;
} pctool_aud_section;

typedef struct {
    uint8_t     io_pin;
    uint8_t     set_flag;
} pctool_iocfg;
/*
typedef struct{
    uint8_t digmic_ck_iomux_pin;
    uint8_t digmic_d0_iomux_pin;
    uint8_t digmic_d1_iomux_pin;
    uint8_t digmic_d2_iomux_pin;
    uint8_t digmic_phase;
}digital_mic_cfg;
*/

// Add audio and speech support
#define AUDIO_SECTION_DEBUG

// Device
#define AUDIO_SECTION_DEVICE_ANC            (0)
#define AUDIO_SECTION_DEVICE_AUDIO          (1)
#define AUDIO_SECTION_DEVICE_SPEECH         (2)
#define AUDIO_SECTION_DEVICE_NUM            (3)

// If add device, need add length to section_device_length
#define AUDIO_SECTION_LENGTH_ANC            (0)         // sperate anc from audio section
#define AUDIO_SECTION_LENGTH_AUDIO          (1024*8)
#define AUDIO_SECTION_LENGTH_SPEECH         (1024*8)

#define AUDIO_SECTION_CFG_RESERVED_LEN      (sizeof(audio_section_t))
#define AUDIO_SECTION_HEAD_LEN              (sizeof(section_head_t))

typedef struct {
    section_head_t      head;
    uint32_t            device;
    uint32_t            cfg_len;
} audio_section_t;

// README: Important!!!!!!
// App cfg struecture must define reserved like follow:
// typedef struct {
//     uint8_t         reserved[AUDIO_SECTION_CFG_RESERVED_LEN];
//     custom_cfg_t    custom_cfg;
// } audio_section_custom_cfg_t;

int audio_section_store_cfg(uint32_t device, uint8_t *cfg, uint32_t len);
int audio_section_load_cfg(uint32_t device, uint8_t *cfg, uint32_t len);
int psapcfg_loadfrom_audsec(const struct_psap_cfg *list[], const struct_psap_cfg *list_44p1k[], uint32_t count);
int spkcalibcfg_loadfrom_audsec(const struct_spkcalib_cfg *list[], const struct_spkcalib_cfg *list_44p1k[], uint32_t count);

void audio_section_nv_register(void);
void anc_section_nv_register(void);
int anc_section_nv_erase(uint32_t start_addr, uint32_t len, bool is_async);
int anc_section_nv_write(uint32_t start_addr, uint8_t* ptr, uint32_t len, bool is_async);
int audio_section_nv_erase(uint32_t start_addr, uint32_t len, bool is_async);
int audio_section_nv_write(uint32_t start_addr, uint8_t* ptr, uint32_t len, bool is_async);
uint32_t audio_section_get_device_size(uint8_t device);
#ifdef __cplusplus
}
#endif

#endif
