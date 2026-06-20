/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

/*****************************header include********************************/
#include "cmsis.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "app_trace_rx.h"
#include "plat_types.h"
#include "ble_audio_dbg.h"
#include "heap_api.h"
#include "tgt_hardware.h"

#include "aob_gaf_api.h"
#include "aob_pacs_api.h"
#include "aob_csip_api.h"

#include "app_bap.h"
#include "app_ahp.h"
#include "app_gaf.h"

#include "gaf_media_stream.h"

#include "ble_audio_earphone_info.h"

#include "sdp_service.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/****************************PACS init CFG********************************/
#ifndef LC3PLUS_SUPPORT
#define CUSTOM_CAPA_NB_PAC_SINK                  (1)    // for lc3 sink
#define CUSTOM_CAPA_NB_PAC_SRC                   (1)    // for lc3 src
#else
#define CUSTOM_CAPA_NB_PAC_SINK                  (2)    // one for lc3 sink one for lc3plus
#define CUSTOM_CAPA_NB_PAC_SRC                   (2)    // for lc3 src one for lc3plus
#endif // LC3PLUS_SUPPORT
/****************************LC3 Codec CFG*********************************/
#define CUSTOM_CAPA_CODEC_ID                     "\x06\x00\x00\x00\x00"
#define CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF_SINK    (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_32000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_24000 | AOB_SUPPORTED_SAMPLE_FREQ_16000)
#define CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF_SRC    (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_32000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_16000)
#define CUSTOM_CAPA_ALL_CONTEXT_BF               (0x1FFF)

#define CUSTOM_CAPA_SINK_CONTEXT_BF_SUPP         CUSTOM_CAPA_ALL_CONTEXT_BF

#define CUSTOM_CAPA_SRC_CONTEXT_BF_SUPP          CUSTOM_CAPA_ALL_CONTEXT_BF

#define CUSTOM_CAPA_SINK_CONTEXT_BF_AVA          CUSTOM_CAPA_ALL_CONTEXT_BF

#define CUSTOM_CAPA_SRC_CONTEXT_BF_AVA           CUSTOM_CAPA_ALL_CONTEXT_BF

#define CUSTOM_CAPA_PREFERRED_CONTEXT_BF         (APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_GAF_BAP_CONTEXT_TYPE_MEDIA)

#define CUSTOM_CAPA_FRAME_DURATION_BF            (AOB_SUPPORTED_FRAME_DURATION_7_5MS | AOB_SUPPORTED_FRAME_DURATION_10MS)

#if defined(LC3PLUS_SUPPORT)
/**************************LC3Plus Codec CFG******************************/
#define CUSTOM_CAPA_LC3PLUS_CODEC_ID             "\xFF\x08\xA9\x00\x02"
#define CUSTOM_CAPA_LC3PLUS_SAMPLING_FREQ_BF     (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_96000)
#define CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MIN        (20)
#define CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MAX        (190)
#define CUSTOM_CAPA_LC3PLUS_PREFERRED_CONTEXT_BF (APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_GAF_BAP_CONTEXT_TYPE_MEDIA)
#define CUSTOM_CAPA_LC3PLUS_FRAME_DURATION_BF    (AOB_SUPPORTED_FRAME_DURATION_10MS | AOB_SUPPORTED_FRAME_DURATION_5MS\
                                                    |AOB_SUPPORTED_FRAME_DURATION_2_5MS)
/****************************LC3Plus Qos CFG******************************/
#define QOS_SETTING_LC3PLUS_MAX_TRANS_DELAY      (100)
#define QOS_SETTING_LC3PLUS_MAX_RTX_NUMER        (13)
#define QOS_SETTING_LC3PLUS_DFT_PRESDELAY        (40000)
#endif /// LC3PLUS_SUPPORT

#if defined(HID_ULL_ENABLE)
/**************************ULL Codec CFG******************************/
#define CUSTOM_CAPA_ULL_CODEC_ID                 "\x08\x00\x00\x00\x00"
#define CUSTOM_CAPA_ULL_SAMPLING_FREQ_BF         (AOB_SUPPORTED_SAMPLE_FREQ_48000 | AOB_SUPPORTED_SAMPLE_FREQ_24000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_32000 | AOB_SUPPORTED_SAMPLE_FREQ_16000 |\
                                                  AOB_SUPPORTED_SAMPLE_FREQ_8000)
#define CUSTOM_CAPA_ULL_FRAME_OCT_MIN            (20)
#define CUSTOM_CAPA_ULL_FRAME_OCT_MAX            (190)
#define CUSTOM_CAPA_ULL_PREFERRED_CONTEXT_BF         (APP_BAP_CONTEXT_TYPE_CONVERSATIONAL | APP_BAP_CONTEXT_TYPE_MEDIA)
#define CUSTOM_CAPA_ULL_FRAME_DURATION_BF        (AOB_SUPPORTED_FRAME_DURATION_10MS | AOB_SUPPORTED_FRAME_DURATION_5MS\
                                                    |AOB_SUPPORTED_FRAME_DURATION_2_5MS)
#define CUSTOM_CAPA_ULL_CHAN_CNT                 (0x01)
#define CUSTOM_CAPA_ULL_MAX_FRAME_PER_SDU        (1)
/****************************ULL Qos CFG******************************/
#define QOS_SETTING_ULL_MAX_TRANS_DELAY          (100)
#define QOS_SETTING_ULL_MAX_RTX_NUMER            (2)
#define QOS_SETTING_ULL_DFT_PRESDELAY            (5000)
#define APP_BAP_DFT_ASCS_FRAMING_TYPE            (APP_ISO_UNFRAMED_MODE)
#define APP_BAP_DFT_ASCS_PHY_BF                  (APP_PHY_2MBPS_VALUE)
#endif // HID_ULL_ENABLE

/*****************************USB DONGLE**********************************/
#if defined(BLE_AUDIO_CENTRAL_APP_ENABLED) || defined(AOB_LOW_LATENCY_MODE)
#define APP_BAP_DFT_ASCS_RTN                     (13)
#define APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US       (10000)
#define APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US       (10000)
#define APP_BAP_DFT_ASCS_MAX_TRANS_LATENCY_MS    (100)
#define APP_BAP_DFT_ASCS_FRAMING_TYPE            (APP_ISO_UNFRAMED_MODE)
#define APP_BAP_DFT_ASCS_PHY_BF                  (APP_PHY_2MBPS_VALUE)
#endif // BLE_AUDIO_CENTRAL_APP_ENABLED

/// GMAP defines 10000 to includes in [codec cfg-qos req ntf] presdelay range
#if defined (AOB_GMAP_ENABLED)
#define QOS_SETTING_DEFAULT_MIN_PRESDELAY       (10000)
#else
#define QOS_SETTING_DEFAULT_MIN_PRESDELAY       (40000)
#endif

#define QOS_SETTING_DEFAULT_MAX_PRESDELAY       (40000)

/***************************AUDIO CONFIGURATION****************************/
#define AOB_AUD_LOCATION_WITH_ROLE               (0xFFFF)
#define AOB_AUD_LOCATION_LEFT                    (AOB_SUPPORTED_LOCATION_FRONT_LEFT | AOB_SUPPORTED_LOCATION_SIDE_LEFT)
#define AOB_AUD_LOCATION_RIGHT                   (AOB_SUPPORTED_LOCATION_SIDE_RIGHT | AOB_SUPPORTED_LOCATION_FRONT_RIGHT)
#define AOB_AUD_LOCATION_ALL_SUPP                (AOB_AUD_LOCATION_LEFT | AOB_AUD_LOCATION_RIGHT)
#define AOB_AUD_CHAN_CNT_MONO                    (0x1)
#define AOB_AUD_CHAN_CNT_STEREO_ONLY             (0x2)
#define AOB_AUD_CHAN_CNT_ALL_SUPP                (AOB_AUD_CHAN_CNT_MONO | AOB_AUD_CHAN_CNT_STEREO_ONLY)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

/*
 * External Functions declaration
 ****************************************************************************************
 */
extern void aob_gaf_api_get_qos_req_info_cb_init(void *cb_func);

/*
 * Internal Functions declaration
 ****************************************************************************************
 */
static void aob_init_add_sdp_record(bool is_central_dev);
static void aob_init_remove_sdp_record(bool is_central_dev);
static void aob_init_add_pac_record(aob_audio_cfg_e aob_aud_cfg);
static bool aob_init_fill_qos_req_for_codec_cfg_cb_func(uint8_t direction, const app_gaf_codec_id_t *codec_id, uint8_t tgt_latency,
                                                        app_gaf_bap_cfg_t *codec_cfg_req, app_gaf_bap_qos_req_t *ntf_qos_req);

/*
 * Internal Values
 ****************************************************************************************
 */
#ifndef BLE_ONLY_ENABLED

/**
 * When ASCS is supported over BR/EDR, the attributes defined below shall be included
 * in the SDP service record.
 *
 */

static const uint8_t _ascs_service_browse_group[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    0x10, 0x02,
};

static const uint8_t _ascs_service_class_id_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SDP_SPLIT_16BITS_BE(GATT_SVC_AUDIO_STREAM_CTRL),
};

static const uint8_t _ascs_service_protocol_descriptor_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(13),
    SDP_DE_DESEQ_8BITSIZE_H2_D(6),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_L2CAP,
    SDP_DE_UINT_H1_D2,
    SDP_SPLIT_16BITS_BE(PSM_ATT),
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_ATT,
};

static const uint8_t _ascs_service_additional_protocol_descriptor_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(15),
    SDP_DE_DESEQ_8BITSIZE_H2_D(13),
    SDP_DE_DESEQ_8BITSIZE_H2_D(6),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_L2CAP,
    SDP_DE_UINT_H1_D2,
    SDP_SPLIT_16BITS_BE(PSM_EATT),
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_ATT,
};

static const bt_sdp_record_attr_t _ascs_service_sdp_attrs[] =   // list attr id in ascending order
{
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_SERVICE_CLASS_ID_LIST, _ascs_service_class_id_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_PROTOCOL_DESC_LIST, _ascs_service_protocol_descriptor_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_BROWSE_GROUP_LIST, _ascs_service_browse_group),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_ADDITIONAL_PROT_DESC_LISTS, _ascs_service_additional_protocol_descriptor_list),
};

/**
 * When PACS is supported over BR/EDR, the attributes defined below shall be included
 * in the SDP service record.
 *
 */

static const uint8_t _pacs_service_browse_group[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    0x10, 0x02,
};

static const uint8_t _pacs_service_class_id_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SDP_SPLIT_16BITS_BE(GATT_SVC_PUBLISHED_AUDIO_CAPA),
};

static const uint8_t _pacs_service_protocol_descriptor_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(13),
    SDP_DE_DESEQ_8BITSIZE_H2_D(6),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_L2CAP,
    SDP_DE_UINT_H1_D2,
    SDP_SPLIT_16BITS_BE(PSM_ATT),
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_ATT,
};

static const uint8_t _pacs_service_additional_protocol_descriptor_list[] =
{
    SDP_DE_DESEQ_8BITSIZE_H2_D(15),
    SDP_DE_DESEQ_8BITSIZE_H2_D(13),
    SDP_DE_DESEQ_8BITSIZE_H2_D(6),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_L2CAP,
    SDP_DE_UINT_H1_D2,
    SDP_SPLIT_16BITS_BE(PSM_EATT),
    SDP_DE_DESEQ_8BITSIZE_H2_D(3),
    SDP_DE_UUID_H1_D2,
    SERV_UUID_ATT,
};

static const bt_sdp_record_attr_t _pacs_service_sdp_attrs[] =   // list attr id in ascending order
{
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_SERVICE_CLASS_ID_LIST, _pacs_service_class_id_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_PROTOCOL_DESC_LIST, _pacs_service_protocol_descriptor_list),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_BROWSE_GROUP_LIST, _pacs_service_browse_group),
    SDP_DEF_ATTRIBUTE(SERV_ATTRID_ADDITIONAL_PROT_DESC_LISTS, _pacs_service_additional_protocol_descriptor_list),
};

#endif /* BLE_ONLY_ENABLED */

/*LC3 Audio Configuration V1.0*/
static const bap_audio_cfg_t aob_audio_conf_v_1[AOB_AUD_CFG_MAX] = \
{
    /*AOB_AUD_CFG_TWS_MONO*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_WITH_ROLE, AOB_AUD_LOCATION_WITH_ROLE, 2},
    /*AOB_AUD_CFG_TWS_STEREO_ONE_CIS*/
    {AOB_AUD_CHAN_CNT_ALL_SUPP, AOB_AUD_CHAN_CNT_ALL_SUPP, 2, 2, AOB_AUD_LOCATION_ALL_SUPP, AOB_AUD_LOCATION_ALL_SUPP, 2},
    /*AOB_AUD_CFG_TWS_STEREO_TWO_CIS*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_WITH_ROLE, AOB_AUD_LOCATION_WITH_ROLE, 2},
    /*AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS*/
    {AOB_AUD_CHAN_CNT_ALL_SUPP, AOB_AUD_CHAN_CNT_MONO, 2, 1, AOB_AUD_LOCATION_ALL_SUPP, AOB_AUD_LOCATION_WITH_ROLE, 1},
    /*AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS*/
    {AOB_AUD_CHAN_CNT_MONO, AOB_AUD_CHAN_CNT_MONO, 1, 1, AOB_AUD_LOCATION_ALL_SUPP, AOB_AUD_LOCATION_ALL_SUPP, 1},
};

/*LC3 QOS SETTING BAP V1.0*/
static const bap_qos_setting_t bap_qos_setting_v_1[BAP_QOS_SETTING_NUM_MAX] = \
{
     /// Low Latency
     /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "8_1_1"      */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  26, },/*
    "8_2_1"      */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 30, },/*
    "16_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  30, },/*
    "16_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 40, },/*
    "24_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  45, },/*
    "24_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 60, },/*
    "32_1_1"     */{APP_ISO_UNFRAMED_MODE, 2,  8,  40, 75,  60, },/*
    "32_2_1"     */{APP_ISO_UNFRAMED_MODE, 2,  10, 40, 100, 80, },/*
    "441_1_1"    */{APP_ISO_FRAMED_MODE,   5,  24, 40, 75,  97, },/*
    "441_2_1"    */{APP_ISO_FRAMED_MODE,   5,  31, 40, 100, 20, },/*
    "48_1_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  75, },/*
    "48_2_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 100,},/*
    "48_3_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  90, },/*
    "48_4_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 120,},/*
    "48_5_1"     */{APP_ISO_UNFRAMED_MODE, 5,  15, 40, 75,  117,},/*
    "48_6_1"     */{APP_ISO_UNFRAMED_MODE, 5,  20, 40, 100, 155,},/*
     /// High Reliable
    "8_1_2"      */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  26, },/*
    "8_2_2"      */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 30, },/*
    "16_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  30, },/*
    "16_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 40, },/*
    "24_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  45, },/*
    "24_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 60, },/*
    "32_1_2"     */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  60, },/*
    "32_2_2"     */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 80, },/*
    "441_1_2"    */{APP_ISO_FRAMED_MODE,   13, 80, 40, 75,  97, },/*
    "441_2_2"    */{APP_ISO_FRAMED_MODE,   13, 85, 40, 100, 130,},/*
    "48_1_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  75, },/*
    "48_2_2",    */{APP_ISO_UNFRAMED_MODE, 13, 95, 40, 100, 100,},/*
    "48_3_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  90, },/*
    "48_4_2",    */{APP_ISO_UNFRAMED_MODE, 13, 100, 40, 100, 120,},/*
    "48_5_2",    */{APP_ISO_UNFRAMED_MODE, 13, 75, 40, 75,  117,},/*
    "48_6_2",    */{APP_ISO_UNFRAMED_MODE, 13, 100, 40, 100, 155,},
#if defined (AOB_GMAP_ENABLED)
    /*
    "48_1_GC"    */{APP_ISO_UNFRAMED_MODE,  2,  8, 40, 75,  75, }, /*
    "48_2_GC"    */{APP_ISO_UNFRAMED_MODE,  2, 10, 40, 100, 100,}, /*
    "48_1_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 13, 10, 75,  75, }, /*
    "48_2_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 16, 10, 100, 100,}, /*
    "48_3_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 13, 10, 75,  90,}, /*
    "48_4_GR"    */{APP_ISO_UNFRAMED_MODE,  2, 17, 10, 100, 120,},
#endif
};

#if (BLE_AHP_SERVER_SUPPORT)
/*LC3 QOS SETTING AHP V1.0*/
static const bap_qos_setting_t ahp_qos_setting_v_1[AHP_QOS_SETTING_MAX] = \
{
    /// AMT based spatialization
    /// Low Latency
    /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "32_1_al"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 20, 75,  (120 / 2),}, /*
    "32_2_al"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 20, 100, (160 / 2),}, /*
    "48_1_al"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 20, 75,  (150 / 2),}, /*
    "48_2_al"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 20, 100, (200 / 2),}, /*
    "48_3_al"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 20, 75,  (180 / 2),}, /*
    "48_4_al"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 20, 100, (240 / 2),}, /*
    "32_1_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 60, 20, 75,  (120 / 2),}, /*
    "32_2_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 80, 20, 100, (160 / 2),}, /*
    "48_1_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 60, 40, 75,  (150 / 2),}, /*
    "48_2_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 80, 40, 100, (200 / 2),}, /*
    "48_3_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 60, 40, 75,  (180 / 2),}, /*
    "48_4_ah"    */{APP_ISO_UNFRAMED_MODE,  15, 80, 40, 100, (240 / 2),}, /**/

    /// AMG based spatialization sink
    /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "48_1_ar"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 10, 75,  75, }, /*
    "48_2_ar"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 10, 100, 100,}, /*
    "48_3_ar"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 10, 75,  90, }, /*
    "48_4_ar"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 10, 100, 120,}, /**/

    /// AMG based spatialization src
    /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "24_1_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  45, }, /*
    "24_2_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 60, }, /*
    "32_1_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  60, }, /*
    "32_2_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 80, }, /*
    "48_1_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  75, }, /*
    "48_2_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 100,}, /*
    "48_3_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  90, }, /*
    "48_4_ac"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 120,}, /**/

    /// AMG based spatialization HT
    /*            {frame_type, rtn, max_trans, max_pres, sdu_intv, max_oxts,}*//*
    "HT_1_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  8, }, /*
    "HT_2_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 8, }, /*
    "HT_3_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 200, 8, }, /*
    "HT_4_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  13,}, /*
    "HT_5_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 13,}, /*
    "HT_6_8V"    */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 200, 13,}, /*
    "HT_1_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  8, }, /*
    "HT_2_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 8, }, /*
    "HT_3_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 200, 8, }, /*
    "HT_4_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  15, 40, 75,  13,}, /*
    "HT_5_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 100, 13,}, /*
    "HT_6_8VI"   */{APP_ISO_UNFRAMED_MODE,  3,  20, 40, 200, 13,}, /**/
};
#endif /* BLE_AHP_SERVER_SUPPORT */

/*
 * Public functions
 ****************************************************************************************
 */

/**
 ************************************************************************************************
 * @brief @brief main thread system call init for tws earbuds
 *
 ************************************************************************************************
 **/
void ble_audio_tws_init(void)
{
#if BLE_AUDIO_ENABLED
    // Add SDP record for LE Audio service
    aob_init_add_sdp_record(false);
    // Below are some configuration about BLE Audio Profiles, sunch as PACS and only PACS
    aob_gaf_capa_info_t gaf_pacs_info;
    uint32_t role_bf = 0;
    // Get aob audio cfg select value
    aob_audio_cfg_e aob_aud_cfg = ble_audio_earphone_info_get_audido_cfg_select();
    /// Check for select audio configuration location sink
    if (aob_audio_conf_v_1[aob_aud_cfg].sink_aud_location_bf == AOB_AUD_LOCATION_WITH_ROLE)
    {
        if (BLE_AUDIO_TWS_MASTER == ble_audio_get_tws_nv_role())
        {
            gaf_pacs_info.sink_location_bf = AOB_AUD_LOCATION_RIGHT;
        }
        else
        {
            gaf_pacs_info.sink_location_bf = AOB_AUD_LOCATION_LEFT;
        }
    }
    else
    {
        gaf_pacs_info.sink_location_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_aud_location_bf;
    }
    /// Check for select audio configuration location src
    if (aob_audio_conf_v_1[aob_aud_cfg].src_aud_location_bf == AOB_AUD_LOCATION_WITH_ROLE)
    {
        if (BLE_AUDIO_TWS_MASTER == ble_audio_get_tws_nv_role())
        {
            gaf_pacs_info.src_location_bf = AOB_AUD_LOCATION_RIGHT;
        }
        else
        {
            gaf_pacs_info.src_location_bf = AOB_AUD_LOCATION_LEFT;
        }
    }
    else
    {
        gaf_pacs_info.src_location_bf = aob_audio_conf_v_1[aob_aud_cfg].src_aud_location_bf;
    }

    LOG_I("aud cfg select:[%d] audio location bf sink:[0x%x] src:[0x%x]", aob_aud_cfg,
          gaf_pacs_info.sink_location_bf, gaf_pacs_info.src_location_bf);
    /// Init supp and ava context
    gaf_pacs_info.sink_context_bf_supp = CUSTOM_CAPA_SINK_CONTEXT_BF_SUPP;
    gaf_pacs_info.sink_ava_bf = CUSTOM_CAPA_SINK_CONTEXT_BF_AVA;
    gaf_pacs_info.src_context_bf_supp = CUSTOM_CAPA_SRC_CONTEXT_BF_SUPP;
    gaf_pacs_info.src_ava_bf = CUSTOM_CAPA_SRC_CONTEXT_BF_SUPP;

    /// Init pac record num for adding pac records
    gaf_pacs_info.sink_nb_pacs = CUSTOM_CAPA_NB_PAC_SINK;
    gaf_pacs_info.src_nb_pacs = CUSTOM_CAPA_NB_PAC_SRC;

    // The role bf is use to inidcate which BLE Audio Profile should be enbaled
    role_bf |= APP_BAP_ROLE_SUPP_CAPA_SRV_BIT;
    role_bf |= APP_BAP_ROLE_SUPP_UC_SRV_BIT;
    role_bf |= APP_BAP_ROLE_SUPP_BC_SINK_BIT;
    role_bf |= APP_BAP_ROLE_SUPP_BC_DELEG_BIT;
    role_bf |= APP_BAP_ROLE_SUPP_BC_SCAN_BIT;

    aob_gaf_bis_init();

    /// Init BLE Audio Profiles and Services sunch as BAP
    // Init csip set info fill callback, must set it before earbuds init
    aob_csip_if_user_parameters_init(aob_audio_conf_v_1[aob_aud_cfg].csip_size);//aob_csip_if_register_sets_config_cb(cb);
    // Only PACS need a configuration when init
    aob_gaf_earbuds_init(&gaf_pacs_info, role_bf);
    // After PACS is init done, add pac records for earbuds support codec capabilties
    aob_init_add_pac_record(aob_aud_cfg);
    /*
    Init for codec cfg req_ind - cfm qos req callback function register
    custom may use @see aob_media_ascs_register_codec_req_handler_cb
    */
    aob_gaf_api_get_qos_req_info_cb_init((void *)aob_init_fill_qos_req_for_codec_cfg_cb_func);
#endif
}

void ble_audio_tws_deinit(void)
{
#if BLE_AUDIO_ENABLED
    // Remove SDP record for LE Audio service
    aob_init_remove_sdp_record(false);
    // Deinit le audio service for earbuds
    aob_gaf_earbuds_deinit();
#endif
}

/**
 ************************************************************************************************
 * @brief Add SDP record in this fucntion
 *
 ************************************************************************************************
 **/
static void aob_init_add_sdp_record(bool is_central_dev)
{
#if 0
    LOG_I("Add lea sdp record, is central dev: %d", is_central_dev);

    if (is_central_dev == false)
    {
        if (bt_l2cap_get_config().gatt_over_br_edr && bt_l2cap_get_config().eatt_over_br_edr)
        {
            bt_sdp_create_record(_pacs_service_sdp_attrs, ARRAY_SIZE(_pacs_service_sdp_attrs));
            bt_sdp_create_record(_ascs_service_sdp_attrs, ARRAY_SIZE(_ascs_service_sdp_attrs));
        }
    }
    else
    {

    }
#endif
}

/**
 ************************************************************************************************
 * @brief Remove SDP record in this fucntion
 *
 ************************************************************************************************
 **/
static void aob_init_remove_sdp_record(bool is_central_dev)
{
#if 0
    LOG_I("Remove lea sdp record, is central dev: %d", is_central_dev);

    if (is_central_dev == false)
    {
        bt_sdp_remove_record(_pacs_service_sdp_attrs);
        bt_sdp_remove_record(_ascs_service_sdp_attrs);
    }
    else
    {

    }
#endif
}

/**
 ************************************************************************************************
 * @brief Add pac record in this fucntion, sunch as LC3/LC3Plus
 *
 * ATTENTION: call it after PACS first init is called  @see aob_gaf_earbuds_init
 ************************************************************************************************
 **/
static void aob_init_add_pac_record(aob_audio_cfg_e aob_aud_cfg)
{
    aob_codec_capa_t p_codec_capa;
    aob_codec_id_t codec_id;
#ifdef BLE_AUDIO_USE_ONE_CIS_FOR_DONGLE
    app_gaf_bap_vendor_specific_cfg_t vendor_specific_cfg;
    uint32_t capa_data_len = sizeof(app_gaf_bap_vendor_specific_cfg_t);
#else
    uint32_t capa_data_len = 0;
#endif
    uint32_t metadata_data_len = 0;

    // Prepare for codec capability add use
    p_codec_capa.capa = (app_gaf_bap_capa_t *)bes_bt_buf_malloc(sizeof(app_gaf_bap_capa_t) + capa_data_len);
    p_codec_capa.metadata = (app_gaf_bap_capa_metadata_t *)bes_bt_buf_malloc(sizeof(app_gaf_bap_capa_metadata_t) + metadata_data_len);

    if ((!p_codec_capa.capa) || (!p_codec_capa.metadata))
    {
        ASSERT(0, "aob_codec_capa_t codec cfg init malloc failed");
    }

    memset(p_codec_capa.capa, 0, sizeof(app_gaf_bap_capa_t) + capa_data_len);
    memset(p_codec_capa.metadata, 0, sizeof(app_gaf_bap_capa_metadata_t) + metadata_data_len);

    p_codec_capa.metadata->add_metadata.len = metadata_data_len;//only merge aob without app_bap from le_audio_dev,avoid build error

#ifdef BLE_AUDIO_USE_ONE_CIS_FOR_DONGLE
    /*******************************Vendor Specific use***************************/
    /// Vendor specified metadata
    vendor_specific_cfg.length = sizeof(app_gaf_bap_vendor_specific_cfg_t) -1;
    //vendor specific type is 0xff
    vendor_specific_cfg.type = 0xff;
    //Add finished product suppliers company id
    vendor_specific_cfg.company_id = 0xffff;
    vendor_specific_cfg.s2m_decode_channel = GAF_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM;
    vendor_specific_cfg.s2m_encode_channel = GAF_AUDIO_STREAM_CAPTURE_CHANNEL_NUM;
    memcpy(&p_codec_capa.capa->add_capa.data[0] + p_codec_capa.capa->add_capa.len,
           &vendor_specific_cfg, sizeof(app_gaf_bap_vendor_specific_cfg_t));
    p_codec_capa.capa->add_capa.len += sizeof(app_gaf_bap_vendor_specific_cfg_t);
#endif

    /*********************************LC3 CODEC CFG*******************************/
    /// LC3 CAPA
    memcpy(&codec_id.codec_id[0], CUSTOM_CAPA_CODEC_ID, AOB_CODEC_ID_LEN);
    p_codec_capa.capa->param.frame_dur_bf = CUSTOM_CAPA_FRAME_DURATION_BF;
    p_codec_capa.metadata->param.context_bf = CUSTOM_CAPA_PREFERRED_CONTEXT_BF;

#if !defined (AOB_SPLIT_PAC_RECORD_INTO_FOUR_RECORDS)
    /// 8_1_1 -> 48_6_2
    p_codec_capa.capa->param.sampling_freq_bf = CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF_SINK;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_8_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_48_6_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.sampling_freq_bf = CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF_SRC;
    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
    /*******************************************************************************/
#else
    // Do not need vendor capa
    p_codec_capa.capa->add_capa.len = 0;
    /// 8_1_1 -> 8_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_8000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_8_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_8_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 16_1_1 -> 16_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_16000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_16_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_16_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 32_1_1 -> 32_2_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_32000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_32_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_32_2_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);

    /// 48_1_1 -> 48_6_2
    p_codec_capa.capa->param.sampling_freq_bf = AOB_SUPPORTED_SAMPLE_FREQ_48000;
    p_codec_capa.capa->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_48_1_1].Oct_max;
    p_codec_capa.capa->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_48_6_2].Oct_max;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

#if defined (AOB_GMAP_ENABLED)
    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
#endif
    /*******************************************************************************/
#endif

#ifdef LC3PLUS_SUPPORT
    /*******************************LC3Plus CODEC CFG*******************************/
    /// LC3Plus capa
    memcpy(&codec_id.codec_id[0], &CUSTOM_CAPA_LC3PLUS_CODEC_ID, AOB_CODEC_ID_LEN);

    p_codec_capa.capa->param.frame_dur_bf           = CUSTOM_CAPA_LC3PLUS_FRAME_DURATION_BF;
    p_codec_capa.metadata->param.context_bf         = CUSTOM_CAPA_LC3PLUS_PREFERRED_CONTEXT_BF;
    p_codec_capa.capa->param.sampling_freq_bf       = CUSTOM_CAPA_LC3PLUS_SAMPLING_FREQ_BF;
    p_codec_capa.capa->param.frame_octet_min        = CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MIN;
    p_codec_capa.capa->param.frame_octet_max        = CUSTOM_CAPA_LC3PLUS_FRAME_OCT_MAX;

    p_codec_capa.capa->param.chan_cnt_bf            = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu         = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf            = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu         = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
    /******************************************************************************/
#endif // LC3PLUS_SUPPORT
#ifdef HID_ULL_ENABLE
    /*******************************ULL CODEC CFG*******************************/
    /// HID_ULL_ENABLE capa
    memcpy(&codec_id.codec_id[0], &CUSTOM_CAPA_ULL_CODEC_ID, AOB_CODEC_ID_LEN);
    p_codec_capa.capa->param.frame_dur_bf           = CUSTOM_CAPA_ULL_FRAME_DURATION_BF;
    p_codec_capa.metadata->param.context_bf         = CUSTOM_CAPA_ULL_PREFERRED_CONTEXT_BF;

    p_codec_capa.capa->param.sampling_freq_bf       = CUSTOM_CAPA_ULL_SAMPLING_FREQ_BF;
    p_codec_capa.capa->param.frame_octet_min        = CUSTOM_CAPA_ULL_FRAME_OCT_MIN;
    p_codec_capa.capa->param.frame_octet_max        = CUSTOM_CAPA_ULL_FRAME_OCT_MAX;

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    aob_pacs_add_sink_pac_record(&codec_id, &p_codec_capa);

    p_codec_capa.capa->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].src_supp_aud_chn_cnt_bf;
    p_codec_capa.capa->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].src_max_cfs_per_sdu;
    aob_pacs_add_src_pac_record(&codec_id, &p_codec_capa);
    /******************************************************************************/
#endif // HID_ULL_ENABLE

    // Free allocated buffer
    bes_bt_buf_free(p_codec_capa.capa);
    bes_bt_buf_free(p_codec_capa.metadata);

#if (BLE_AHP_SERVER_SUPPORT)
    uint32_t sink_aud_loc_bf = aob_pacs_get_cur_audio_location(APP_GAF_DIRECTION_SINK);
    // Channel capabilities, id = 0
    aob_pacs_add_chan_capa_record(0x0000003F, (uint8_t *)&sink_aud_loc_bf, sizeof(sink_aud_loc_bf));
    // PACS v2 source, malloc the max length
    app_gaf_bap_capa_t *p_capa_ht_info = (app_gaf_bap_capa_t *)bes_bt_buf_malloc(sizeof(app_gaf_bap_capa_t) +
                                                                        sizeof(app_gaf_bap_ht_supp_fmt_ltv_t) +
                                                                        sizeof(app_gaf_bap_ht_supp_intv_ltv_t));
    memset(p_capa_ht_info, 0, sizeof(app_gaf_bap_capa_t));
    // Contains capa and metadata
    aob_codec_capa_t ahp_pac_info =
    {
        .capa = p_capa_ht_info,
        .metadata = NULL,
    };
    // Only HT format and interval in this pac record
    p_capa_ht_info->add_capa.len = sizeof(app_gaf_bap_ht_supp_fmt_ltv_t)  + sizeof(app_gaf_bap_ht_supp_intv_ltv_t);
    app_gaf_bap_ht_supp_fmt_ltv_t *ht_fmt_data = (app_gaf_bap_ht_supp_fmt_ltv_t *)p_capa_ht_info->add_capa.data;
    app_gaf_bap_ht_supp_intv_ltv_t *ht_intv_data = (app_gaf_bap_ht_supp_intv_ltv_t *)(ht_fmt_data + 1);
    // Supported_HT_Frame_Formats length
    ht_fmt_data->len = sizeof(app_gaf_bap_ht_supp_fmt_ltv_t) - 1;
    // Supported_HT_Frame_Formats Type
    ht_fmt_data->type = APP_GAF_BAP_CAPA_TYPE_SUPP_HTF_FMT;
    // Rotation Vector and qu are supported
    ht_fmt_data->supp_data_formats = APP_GAF_BAP_HT_DATA_QUATERNION_BIT | APP_GAF_BAP_HT_ROTATION_VECTOR_BIT;
    ht_fmt_data->related_flags = 0;
    // Supported_HT_Frame_Intervals length
    ht_intv_data->len = sizeof(app_gaf_bap_ht_supp_intv_ltv_t) - 1;
    // Supported_HT_Frame_Intervals type
    ht_intv_data->type = APP_GAF_BAP_CAPA_TYPE_SUPP_HTF_INTV;
    // 20ms | 10ms | 7.5ms
    ht_intv_data->intv_bf = APP_GAF_BAP_HT_FRAME_7_5MS_BIT | APP_GAF_BAP_HT_FRAME_10MS_BIT | APP_GAF_BAP_HT_FRAME_20MS_BIT;
    // Add source pac v2 record
    aob_pacs_add_src_pac_v2_record((aob_codec_id_t *)"\xFF\x02\x00\x00\x00", &ahp_pac_info);

    /// Codec Capabilities
    /// 8_1_1 -> 48_6_2
    p_capa_ht_info->param.frame_octet_min = bap_qos_setting_v_1[BAP_QOS_SETTING_LL_8_1_1].Oct_max;
    p_capa_ht_info->param.frame_octet_max = bap_qos_setting_v_1[BAP_QOS_SETTING_HR_48_6_2].Oct_max;
    // Channel capabilities supported index list
    p_capa_ht_info->add_capa.len = sizeof(app_gaf_bap_chan_capa_supp_ltv_t) + 1;
    app_gaf_bap_chan_capa_supp_ltv_t *supp_chan_capa_list =
                                    (app_gaf_bap_chan_capa_supp_ltv_t *)p_capa_ht_info->add_capa.data;
    supp_chan_capa_list->len = sizeof(app_gaf_bap_chan_capa_supp_ltv_t) - 1 + 1;
    supp_chan_capa_list->type = APP_GAF_BAP_CAPA_TYPE_SUPP_CHAN_CAPA;
    supp_chan_capa_list->supp_chan_capa_idx[0] = 0;

    p_capa_ht_info->param.sampling_freq_bf = CUSTOM_CAPA_ALL_SAMPLING_FREQ_BF_SINK;
    p_capa_ht_info->param.chan_cnt_bf = aob_audio_conf_v_1[aob_aud_cfg].sink_supp_aud_chn_cnt_bf;
    p_capa_ht_info->param.max_frames_sdu = aob_audio_conf_v_1[aob_aud_cfg].sink_max_cfs_per_sdu;
    p_capa_ht_info->param.frame_dur_bf = CUSTOM_CAPA_FRAME_DURATION_BF;
    aob_pacs_add_sink_pac_v2_record((aob_codec_id_t *)"\xFF\x02\x00\x00\x00", &ahp_pac_info);

    bes_bt_buf_free(p_capa_ht_info);
#endif /* BLE_AHP_SERVER_SUPPORT */
}

#if (BLE_AHP_SERVER_SUPPORT)
static bool aob_init_fill_ahp_qos_req_for_codec_cfg(uint8_t direction, const app_gaf_codec_id_t *codec_id,
                                                    uint8_t tgt_latency, uint8_t ht_frame_intv, uint8_t ht_frame_fmt,
                                                    app_gaf_bap_cfg_t *codec_cfg_req, app_gaf_bap_qos_req_t *ntf_qos_req)
{
    uint8_t qos_setting_idx = 0;
    uint8_t qos_setting_range = 0;
    uint8_t ht_f_intv_100us = 0;
    uint8_t amt_based_spa = 0;
    // Get aob audio cfg select value
    aob_audio_cfg_e aob_aud_cfg = ble_audio_earphone_info_get_audido_cfg_select();

    do
    {
        ht_f_intv_100us = app_ahp_ht_intv_enum_to_ht_intv_us(ht_frame_intv) / 100;

        if (ht_f_intv_100us != 0)
        {
            // Case 8VI
            if (aob_aud_cfg == AOB_AUD_CFG_TWS_STEREO_ONE_CIS)
            {
                qos_setting_idx = AHP_QOS_SETTING_HT_1_8VI;
                qos_setting_idx += (ht_frame_fmt == APP_GAF_BAP_HT_TYPE_DATA_QUATERNION) ? 0 : 3;
                qos_setting_range = qos_setting_idx + ((AHP_QOS_SETTING_HT_6_8VI - AHP_QOS_SETTING_HT_1_8VI + 1) / 2);
            }
            // Case 8V
            else
            {
                qos_setting_idx = AHP_QOS_SETTING_HT_1_8V;
                qos_setting_idx += (ht_frame_fmt == APP_GAF_BAP_HT_TYPE_DATA_QUATERNION) ? 0 : 3;
                qos_setting_range = qos_setting_idx + ((AHP_QOS_SETTING_HT_6_8V - AHP_QOS_SETTING_HT_1_8V + 1) / 2);
            }

            for (; qos_setting_idx <= qos_setting_range; qos_setting_idx++)
            {
                if (ahp_qos_setting_v_1[qos_setting_idx].Sdu_intval == ht_f_intv_100us)
                {
                    break;
                }
            }

            if (qos_setting_idx == qos_setting_range + 1)
            {
                LOG_E("No matched Qos setting latency for AHP HT Codec cfg!!!");
                /// use dft qos setting
                qos_setting_idx = qos_setting_range;
            }

            break;
        }

        // Check it is AMT or AMG based spatialization
        amt_based_spa = (app_bap_get_audio_location_l_r_cnt(codec_cfg_req->param.location_bf) > 1);

        if (amt_based_spa == true)
        {
            // QOS SETTING LOW LATENCY
            if (tgt_latency == APP_GAF_BAP_UC_TGT_LATENCY_LOWER)
            {
                qos_setting_idx = codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_48000HZ ?
                                                            AHP_QOS_SETTING_48_1_AL : AHP_QOS_SETTING_32_1_AL;
                qos_setting_range = codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_48000HZ ?
                                                            AHP_QOS_SETTING_48_4_AL : AHP_QOS_SETTING_32_2_AL;
            }
            else
            {
                qos_setting_idx = codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_48000HZ ?
                                                            AHP_QOS_SETTING_48_1_AH : AHP_QOS_SETTING_32_1_AH;
                qos_setting_range = codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_48000HZ ?
                                                            AHP_QOS_SETTING_48_4_AH : AHP_QOS_SETTING_32_2_AH;
            }

            for (; qos_setting_idx <= qos_setting_range; qos_setting_idx++)
            {
                if (ahp_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
            
            if (qos_setting_idx == qos_setting_range + 1)
            {
                LOG_E("No matched Qos setting latency for AHP AMT Codec cfg!!!");
                /// use dft qos setting
                qos_setting_idx = qos_setting_range;
                return false;
            }

            break;
        }

        // SINK
        if (direction == APP_GAF_DIRECTION_SINK)
        {
            qos_setting_idx = AHP_QOS_SETTING_48_1_AR;
            qos_setting_range = AHP_QOS_SETTING_48_4_AR;
        }
        // SRC
        else
        {
            if (codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_48000HZ)
            {
                qos_setting_idx = AHP_QOS_SETTING_48_1_AC;
                qos_setting_range = AHP_QOS_SETTING_48_4_AC;
            }
            else if (codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_32000HZ)
            {
                qos_setting_idx = AHP_QOS_SETTING_32_1_AC;
                qos_setting_range = AHP_QOS_SETTING_32_2_AC;
            }
            else if (codec_cfg_req->param.sampling_freq == APP_GAF_BAP_SAMPLING_FREQ_24000HZ)
            {
                qos_setting_idx = AHP_QOS_SETTING_24_1_AC;
                qos_setting_range = AHP_QOS_SETTING_24_2_AC;
            }
        }

        for (; qos_setting_idx <= qos_setting_range; qos_setting_idx++)
        {
            if (ahp_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
            {
                break;
            }
        }

        if (qos_setting_idx == qos_setting_range + 1)
        {
            LOG_E("No matched Qos setting latency for %d AHP AMG Codec cfg!!!", direction);
            /// use dft qos setting
            qos_setting_idx = qos_setting_range;
            return false;
        }
    } while (0);

    // Max transmission Latency
    ntf_qos_req->trans_latency_max_ms = ahp_qos_setting_v_1[qos_setting_idx].Max_trans_latency;
    ntf_qos_req->retx_nb = ahp_qos_setting_v_1[qos_setting_idx].Rtn_num;
    // Prefer PresDelay
    ntf_qos_req->pref_pres_delay_min_us = ahp_qos_setting_v_1[qos_setting_idx].Pres_Delay * 1000;
    ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // PresDelay
    ntf_qos_req->pres_delay_min_us = ahp_qos_setting_v_1[qos_setting_idx].Pres_Delay * 1000;
    ntf_qos_req->pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // Framing type
    ntf_qos_req->framing = ahp_qos_setting_v_1[qos_setting_idx].Faming_type;
    // PHY
    ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;

    LOG_I("%s use ahp qos setting label %d", __func__, qos_setting_idx);

    return true;
}
#endif /* BLE_AHP_SERVER_SUPPORT */

/**
 ************************************************************************************************
 * @brief A callback function example for Codec cfg request to fill qos req value
 *        @see app_gaf_bap_qos_req_t or you can modify codec_cfg_req @see app_gaf_bap_cfg_t
 *        in this function
 *
 * ATTENTION: Do not delete this callback function before using a new callback.
 *            If no callback function is set, codec cfg req - cfm will use dft.
 *
 * view more information @see get_qos_req_cfg_info_cb
 *
 *************************************************************************************************
 **/
static bool aob_init_fill_qos_req_for_codec_cfg_cb_func(uint8_t direction, const app_gaf_codec_id_t *codec_id,
                                                        uint8_t tgt_latency, app_gaf_bap_cfg_t *codec_cfg_req, app_gaf_bap_qos_req_t *ntf_qos_req)
{
    if (!codec_cfg_req || !ntf_qos_req)
    {
        LOG_E("%s Err params!!!", __func__);
        return false;
    }

    LOG_I("%s codec id is %d", __func__, codec_id->codec_id[0]);
    uint8_t qos_setting_idx = BAP_QOS_SETTING_MIN;

#if (BLE_AHP_SERVER_SUPPORT)
    // Fetch intv and fmt ltv
    app_gaf_bap_ht_cfg_intv_ltv_t *p_ht_intv =
            (app_gaf_bap_ht_cfg_intv_ltv_t *)app_bap_get_ltv_value_by_type(&codec_cfg_req->add_cfg,
                                                                           APP_GAF_BAP_CFG_TYPE_HT_FRAME_INTV);
    app_gaf_bap_ht_cfg_fmt_ltv_t *p_ht_fmt =
            (app_gaf_bap_ht_cfg_fmt_ltv_t *)app_bap_get_ltv_value_by_type(&codec_cfg_req->add_cfg,
                                                                          APP_GAF_BAP_CFG_TYPE_HT_FRAME_FMT);
    if (p_ht_intv != NULL && p_ht_fmt != NULL)
    {
        LOG_I("HT Frame interval: %d, format: %d", p_ht_intv->intv, p_ht_fmt->data_format);

        if (aob_init_fill_ahp_qos_req_for_codec_cfg(direction, codec_id, tgt_latency, p_ht_intv->intv,
                                                    p_ht_fmt->data_format, codec_cfg_req, ntf_qos_req) == true)
        {
            return true;
        }
    }
#endif /* BLE_AHP_SERVER_SUPPORT */

    /*************************Using app_gaf_bap_cfg_t to choose qos setting****************************/
#if defined(LC3PLUS_SUPPORT)
    if (app_bap_codec_is_lc3plus(codec_id))
    {
        /// @see LC3plus High Resolution Spec
        // Max transmission Latency
        ntf_qos_req->trans_latency_max_ms = QOS_SETTING_LC3PLUS_MAX_TRANS_DELAY;
        // Retransmission Number
        ntf_qos_req->retx_nb = QOS_SETTING_LC3PLUS_MAX_RTX_NUMER;
        // Prefer PresDelay
        ntf_qos_req->pref_pres_delay_min_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        // PresDelay
        ntf_qos_req->pres_delay_min_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        ntf_qos_req->pres_delay_max_us = QOS_SETTING_LC3PLUS_DFT_PRESDELAY;
        // Framing type
        ntf_qos_req->framing = APP_ISO_UNFRAMED_MODE;
        // PHY
        ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;
    }
    else
#endif /// LC3PLUS_SUPPORT
#if defined(HID_ULL_ENABLE)
    if (app_bap_codec_is_ull(codec_id))
    {
        LOG_I("app_bap_codec_is_ull");
        // Max transmission Latency
        ntf_qos_req->trans_latency_max_ms = QOS_SETTING_ULL_MAX_TRANS_DELAY;
        // Retransmission Number
        ntf_qos_req->retx_nb = QOS_SETTING_ULL_MAX_RTX_NUMER;
        // Prefer PresDelay
        ntf_qos_req->pref_pres_delay_min_us = QOS_SETTING_ULL_DFT_PRESDELAY;
        ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_ULL_DFT_PRESDELAY;
        // PresDelay
        ntf_qos_req->pres_delay_min_us = QOS_SETTING_ULL_DFT_PRESDELAY;
        ntf_qos_req->pres_delay_max_us = QOS_SETTING_ULL_DFT_PRESDELAY;
        // Framing type
        ntf_qos_req->framing = APP_ISO_UNFRAMED_MODE;
        // PHY
        ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;
    }
    else
#endif /// HID_ULL_ENABLE
    {
    // QOS SETTING LOW LATENCY
    if (tgt_latency == APP_GAF_BAP_UC_TGT_LATENCY_LOWER)
    {
        switch (codec_cfg_req->param.sampling_freq)
        {
        case APP_GAF_BAP_SAMPLING_FREQ_48000HZ:
        {
#if defined (AOB_GMAP_ENABLED)
            if (direction == APP_GAF_DIRECTION_SRC)
            {
                for (qos_setting_idx = BAP_QOS_SETTING_GMING_48_1_GC; qos_setting_idx <= BAP_QOS_SETTING_GMING_48_2_GC; qos_setting_idx++)
                {
                    if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                    {
                        LOG_I("GMAP qos setting %d found!!!", qos_setting_idx);
                        break;
                    }
                }
            }
            else //if (direction == APP_GAF_DIRECTION_SINK)
            {
                for (qos_setting_idx = BAP_QOS_SETTING_GMING_48_1_GR; qos_setting_idx <= BAP_QOS_SETTING_GMING_48_4_GR; qos_setting_idx++)
                {
                    if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                    {
                        LOG_I("GMAP qos setting %d found!!!", qos_setting_idx);
                        break;
                    }
                }
            }
            /// Could not find target GMAP qos setting
            if (qos_setting_idx == (BAP_QOS_SETTING_GMING_48_2_GC + 1) ||
                qos_setting_idx == (BAP_QOS_SETTING_GMING_48_4_GR + 1))
#endif
            for (qos_setting_idx = BAP_QOS_SETTING_LL_48_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_48_6_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_32000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_32_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_32_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_24000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_24_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_24_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_16000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_16_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_16_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_8000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_LL_8_1_1; qos_setting_idx <= BAP_QOS_SETTING_LL_8_2_1; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        default:
            break;
        }

        if (qos_setting_idx == BAP_QOS_SETTING_LL_MAX)
        {
            qos_setting_idx = BAP_QOS_SETTING_LL_48_6_1;
        }
    }
    // QOS SETTING HIGH RELIABLE
    else if ((tgt_latency == APP_GAF_BAP_UC_TGT_LATENCY_BALENCED || tgt_latency == APP_GAF_BAP_UC_TGT_LATENCY_RELIABLE))
    {
        LOG_I("codec_cfg_req->param.sampling_freq==%d",codec_cfg_req->param.sampling_freq);
        switch (codec_cfg_req->param.sampling_freq)
        {
        case APP_GAF_BAP_SAMPLING_FREQ_48000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_48_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_48_6_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_32000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_32_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_32_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_24000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_24_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_24_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_16000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_16_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_16_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        case APP_GAF_BAP_SAMPLING_FREQ_8000HZ:
        {
            for (qos_setting_idx = BAP_QOS_SETTING_HR_8_1_2; qos_setting_idx <= BAP_QOS_SETTING_HR_8_2_2; qos_setting_idx++)
            {
                if (bap_qos_setting_v_1[qos_setting_idx].Oct_max == codec_cfg_req->param.frame_octet)
                {
                    break;
                }
            }
        }
        break;
        default:
            break;
        }

        if (qos_setting_idx == BAP_QOS_SETTING_HR_MAX)
        {
            qos_setting_idx = BAP_QOS_SETTING_HR_48_6_2;
        }
    }
    else
    {
        LOG_E("No matched Qos setting latency for Codec cfg!!!");
        /// use dft qos setting
        qos_setting_idx = BAP_QOS_SETTING_HR_48_6_2;
    }

    LOG_I("qos_setting_idx == %d",qos_setting_idx);    

#if defined(BLE_AUDIO_CENTRAL_APP_ENABLED) || defined(AOB_LOW_LATENCY_MODE)
    // Max transmission Latency
    ntf_qos_req->trans_latency_max_ms = APP_BAP_DFT_ASCS_MAX_TRANS_LATENCY_MS;
    // Retransmission Number
    ntf_qos_req->retx_nb = APP_BAP_DFT_ASCS_RTN;
    // Prefer PresDelay
    ntf_qos_req->pref_pres_delay_min_us = APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US;
    ntf_qos_req->pref_pres_delay_max_us = APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US;
    // PresDelay
    ntf_qos_req->pres_delay_min_us = APP_BAP_DFT_ASCS_MIN_PRES_DELAY_US;
    ntf_qos_req->pres_delay_max_us = APP_BAP_DFT_ASCS_MAX_PRES_DELAY_US;
    // Framing type
    ntf_qos_req->framing = APP_BAP_DFT_ASCS_FRAMING_TYPE;
    // PHY
    // PHY
#if (mHDT_LE_SUPPORT)
    ntf_qos_req->phy_bf = APP_PHY_4MBPS_VALUE;
#else
    ntf_qos_req->phy_bf = APP_BAP_DFT_ASCS_PHY_BF;
#endif
#else
    // Max transmission Latency
    ntf_qos_req->trans_latency_max_ms = bap_qos_setting_v_1[qos_setting_idx].Max_trans_latency;

    LOG_I("ntf_qos_req->retx_nb == %d",ntf_qos_req->retx_nb); 
    ntf_qos_req->retx_nb = bap_qos_setting_v_1[qos_setting_idx].Rtn_num;
    // Prefer PresDelay
    ntf_qos_req->pref_pres_delay_min_us = QOS_SETTING_DEFAULT_MIN_PRESDELAY;
    ntf_qos_req->pref_pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // PresDelay
    ntf_qos_req->pres_delay_min_us = QOS_SETTING_DEFAULT_MIN_PRESDELAY;
    ntf_qos_req->pres_delay_max_us = QOS_SETTING_DEFAULT_MAX_PRESDELAY;
    // Framing type
    ntf_qos_req->framing = bap_qos_setting_v_1[qos_setting_idx].Faming_type;
    // PHY
#if (mHDT_LE_SUPPORT)
    ntf_qos_req->phy_bf = APP_PHY_4MBPS_VALUE;
#else
    ntf_qos_req->phy_bf = APP_PHY_2MBPS_VALUE;
#endif
#endif

    LOG_I("%s use qos setting label %d", __func__, qos_setting_idx);
    }

     /*******************************Modify codec_cfg_req here*******************************/
    // codec_cfg_req->param.frame_octet = some value custom difine

    // TRUE Means accept this codec req, will cfm after this callback
    return true;
}
