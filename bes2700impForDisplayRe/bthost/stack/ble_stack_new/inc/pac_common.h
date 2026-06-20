/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __BAP_PAC_COMMON__
#define __BAP_PAC_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"

#define PACS_MAX_PRES_DELAY_US               (0x00FFFFFF)

/// Characteristic type values for Published Audio Capabilities Service
enum pacs_char_type
{
    /// Available Audio Contexts characteristic
    PACS_CHAR_TYPE_CONTEXT_AVA = 0,
    /// Supported Audio Contexts characteristic
    PACS_CHAR_TYPE_CONTEXT_SUPP,
    /// Sink Audio Locations characteristic
    PACS_CHAR_TYPE_LOC_SINK,
    /// Source Audio Locations characteristic
    PACS_CHAR_TYPE_LOC_SRC,
    /// Channel capabilities characteristic
    PACS_CHAR_TYPE_CHAN_CAPA,
    /// Prefer Audio Configuration characteristic
    PACS_CHAR_TYPE_PREF_AUD_CFG,
    /// SINK PAC characteristic
    PACS_CHAR_TYPE_SINK_PAC,
    /// SRC PAC characteristic
    PACS_CHAR_TYPE_SRC_PAC,
    /// SINK PAC V2 characteristic
    PACS_CHAR_TYPE_SINK_PAC_V2,
    /// SRC PAC V2 characteristic
    PACS_CHAR_TYPE_SRC_PAC_V2,
    /// MAY BE MORE THAN ONE PAC
    PACS_CHAR_TYPE_MAX,
};

enum pacs_channel_type_id
{
    /// Generic Channels
    PACS_CHAN_TYPE_ID_GENERIC   = 0x0000,
    /// Raw Channels
    PACS_CHAN_TYPE_ID_RAW       = 0x0001,
};

enum pacs_pref_aud_cfg_data_present_bit
{
    /// Preferred Audio Configuration
    PACS_PREF_AC_DATA_AUDIO_CONFIGURATIONS = 0x01,
    /// Preferred Channel Capability index
    PACS_PREF_AC_DATA_CHAN_CAPA_RECORD_IDX = 0x02,
    /// Preferred Sink PAC Record List
    PACS_PREF_AC_DATA_SINK_PAC_RECORD_LIST = 0x04,
    /// Preferred QoS Setting Sink
    PACS_PREF_AC_DATA_SINK_QOS_SETTING     = 0x08,
    /// Preferred Source PAC Record List
    PACS_PREF_AC_DATA_SRC_PAC_RECORD_LIST  = 0x10,
    /// Preferred QoS Setting Source
    PACS_PREF_AC_DATA_SRC_QOS_SETTING      = 0x20,
    /// Preferred Codec Configuration
    PACS_PREF_AC_DATA_CODEC_CONFIGURATION  = 0x40,
    /// Preferred Presentation Delay
    PACS_PREF_AC_DATA_PRES_DELAY_US_SINK   = 0x80,
    /// Preferred Presentation Delay
    PACS_PREF_AC_DATA_PRES_DELAY_US_SRC    = 0x100,

    PACS_PREF_AC_DATA_MASK                 = 0x1FF,
};

typedef struct pacs_pref_aud_cfg_data_aud_cfg_uc_bc
{
    uint8_t uc_aud_cfg[13];
    uint8_t bc_aud_cfg[4];
} pacs_pref_aud_cfg_data_aud_cfg_uc_bc_t;

typedef union pacs_pref_aud_cfg_data_aud_cfg
{
    uint8_t aud_cfg[17];
    pacs_pref_aud_cfg_data_aud_cfg_uc_bc_t data;
} pacs_pref_ac_data_aud_cfg_u;

typedef struct pacs_pref_aud_cfg_data_chan_capa
{
    uint8_t chan_capa_idx;
} pacs_pref_ac_data_chan_capa_idx_t;

typedef struct pacs_pref_aud_cfg_data_pac_record
{
    uint8_t pref_pac_num;
    struct pref_pac_info
    {
        uint8_t pac_set_id;
        uint8_t pac_idx;
    } pac_info[GAF_ARRAY_EMPTY];
} pacs_pref_ac_data_pac_rec_t;

typedef struct pacs_pref_aud_cfg_data_qos_setting
{
    uint32_t sdu_interval_us;
    uint8_t framing_type;
    uint16_t max_sdu_size;
    uint8_t rtn;
    uint16_t trans_latency_ms;
} pacs_pref_ac_data_qos_setting_t;

typedef struct pacs_pref_aud_cfg_data_codec_cfg
{
    uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
    gen_aud_cc_t codec_cfg;
} pacs_pref_ac_data_specific_cc_t;

typedef struct pacs_pref_aud_cfg_data_codec_cfg_ptr
{
    uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
    gen_aud_cc_ptr_t codec_cfg;
} pacs_pref_ac_data_specific_cc_ptr_t;

typedef struct pacs_pref_aud_cfg_data_pres_delay
{
    uint32_t pres_delay_us;
} pacs_pref_ac_data_pres_delay_t;

typedef struct pacs_pref_aud_cfg_data
{
    const pacs_pref_ac_data_aud_cfg_u *pref_aud_cfg;
    const pacs_pref_ac_data_chan_capa_idx_t *pref_chan_capa;
    const pacs_pref_ac_data_pac_rec_t *pref_pac_sink;
    const pacs_pref_ac_data_qos_setting_t *pref_qos_setting_sink;
    const pacs_pref_ac_data_pac_rec_t *pref_pac_src;
    const pacs_pref_ac_data_qos_setting_t *pref_qos_setting_src;
    const pacs_pref_ac_data_specific_cc_t *pref_codec_cfg;
    const pacs_pref_ac_data_pres_delay_t *pref_pres_delay_sink;
    const pacs_pref_ac_data_pres_delay_t *pref_pres_delay_src;
} pacs_pref_aud_cfg_data_t;

typedef struct pacs_pref_aud_cfg_data_ptr
{
    const pacs_pref_ac_data_aud_cfg_u *pref_aud_cfg;
    const pacs_pref_ac_data_chan_capa_idx_t *pref_chan_capa;
    const pacs_pref_ac_data_pac_rec_t *pref_pac_sink;
    const pacs_pref_ac_data_qos_setting_t *pref_qos_setting_sink;
    const pacs_pref_ac_data_pac_rec_t *pref_pac_src;
    const pacs_pref_ac_data_qos_setting_t *pref_qos_setting_src;
    const pacs_pref_ac_data_specific_cc_ptr_t *pref_codec_cfg_ptr;
    const pacs_pref_ac_data_pres_delay_t *pref_pres_delay_sink;
    const pacs_pref_ac_data_pres_delay_t *pref_pres_delay_src;
} pacs_pref_aud_cfg_data_ptr_t;

typedef struct pacs_preferred_audio_cfg
{
    /// @see pacs_pref_aud_cfg_data_present_bit
    uint16_t data_present_bits;
    /// @see generic_audio_context_type_bf
    uint16_t use_case_id;
    /// @see pacs_pref_aud_cfg_data
    pacs_pref_aud_cfg_data_t data_list;
} pacs_pref_aud_cfg_t;

typedef struct pacs_preferred_audio_cfg_ptr
{
    /// @see pacs_pref_aud_cfg_data_present_bit
    uint16_t data_present_bits;
    /// @see pacs_pref_aud_cfg_data
    pacs_pref_aud_cfg_data_ptr_t data_ptr_list;
} pacs_pref_aud_cfg_ptr_t;

typedef struct pacs_generic_chan_descriptor
{
    /// Bitdata of supported Audio Channels that
    /// are used together as one channel set.
    /// @see gen_aud_supp_loc_bf_e
    uint32_t gen_aud_chan_mask;
} pacs_gen_chan_desc_t;

typedef struct pacs_raw_chan_descriptor
{
    /// Number of specific channel sets supported by the device
    uint8_t aud_chan_sets_num;
    /// Bitdata of supported Audio Channels
    /// @see gen_aud_raw_aud_chan_e
    uint8_t audio_chan_set[GAF_ARRAY_EMPTY];
} pacs_raw_chan_desc_t;

typedef union pacs_chan_capa_descriptor
{
    /// Generic Audio channel type
    pacs_gen_chan_desc_t gen_chan_desc;
    /// Raw Audio channel type
    pacs_raw_chan_desc_t raw_chan_desc;
} pacs_chan_capa_desc_u;

#endif /// __BAP_PAC_COMMON__
