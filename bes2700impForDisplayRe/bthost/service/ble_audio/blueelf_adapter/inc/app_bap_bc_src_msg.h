/**
 ****************************************************************************************
 *
 * @file app_bap_bc_src_msg.h
 *
 * @brief BLE Audio Broadcast Source
 *
 * Copyright 2015-2019 BES.
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
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_BAP_BC_SRC_MSG_H_
#define APP_BAP_BC_SRC_MSG_H_
#if BLE_AUDIO_ENABLED
#include "app_bap.h"

//Broadcast source default value
#define APP_BAP_DFT_BC_SRC_MAX_BIG_NUM            1
#define APP_BAP_DFT_BC_SRC_MAX_SUBGRP_NUM         2
#define APP_BAP_DFT_BC_SRC_MAX_STREAM_NUM         4
#define APP_BAP_DFT_BC_SRC_NB_STREAMS             1
#define APP_BAP_DFT_BC_SRC_NB_SUBGRPS             1
#define APP_BAP_DFT_BC_SRC_SDU_INTV_US            10000
#define APP_BAP_DFT_BC_SRC_MAX_SDU_SIZE           240
#define APP_BAP_DFT_BC_SRC_MAX_TRANS_LATENCY_MS   40
#define APP_BAP_DFT_BC_SRC_PACKING_TYPE           APP_ISO_PACKING_SEQUENTIAL
#define APP_BAP_DFT_BC_SRC_FRAMING_TYPE           APP_ISO_UNFRAMED_MODE
#define APP_BAP_DFT_BC_SRC_PHY_BF                 APP_PHY_2MBPS_BIT
#define APP_BAP_DFT_BC_SRC_RTN                    2
#define APP_BAP_DFT_BC_SRC_ADV_INTERVAL           256 //100ms  *0.625ms
#define APP_BAP_DFT_BC_SRC_PERIODIC_INTERVAL      80  //100ms  *1.25ms
#define APP_BAP_DFT_BC_SRC_TEST                   0

#define APP_BAP_DFT_BC_SRC_PRES_DELAY_US          10000
#define APP_BAP_DFT_BC_SRC_CONTEXT_BF             APP_GAF_BAP_CONTEXT_TYPE_MEDIA_BIT
#define APP_BAP_DFT_BC_SRC_METADATA_LEN           5
#define APP_BAP_DFT_BC_SRC_CODEC_ID               APP_GAF_CODEC_TYPE_LC3
#define APP_BAP_DFT_BC_SRC_ADD_CODEC_LEN          0
#define APP_BAP_DFT_BC_SRC_LOCATION_BF            (APP_GAF_LOC_SIDE_LEFT_BIT | APP_GAF_LOC_SIDE_RIGHT_BIT)
#define APP_BAP_DFT_BC_SRC_FRAME_OCTET            120
#define APP_BAP_DFT_BC_SRC_SAMPLING_FREQ          APP_GAF_BAP_SAMPLING_FREQ_48000HZ
#define APP_BAP_DFT_BC_SRC_FRAME_DURATION         APP_GAF_BAP_FRAME_DUR_10MS
#define APP_BAP_DFT_BC_SRC_NB_LC3_STREAM          0
#define APP_BAP_DFT_BC_SRC_NB_FRAME_BLOCKS_SDU    1
#define APP_BAP_DFT_BC_SRC_IS_ENCRYPTED           1
#define APP_BAP_DFT_BC_SRC_DP_ID                  0
#define APP_BAP_DFT_BC_SRC_CTL_DELAY_US           0x0102

#define APP_BAP_DFT_BC_SRC_ADV_DATA               "bis_adv_test"
#define APP_BAP_DFT_BC_SRC_PA_ADV_DATA            "bis_pa_adv_test"
#define APP_BAP_DFT_BC_SRC_MAX_ADV_DATA_LEN       252

/// Structure for #BAP_BC_SRC_INFO indication message
typedef struct app_bap_bc_src_cmp_info
{
    /// Number of BISes
    uint8_t nb_bis;
    /// Broadcast Group configuration
      /// Transmission delay time in microseconds of all BISs in the BIG (in us range 0x0000EA-0x7FFFFF)
    uint32_t sync_delay_us;
    /// The maximum delay time, in microseconds, for transmission of SDUs of all BISes
    /// (in us range 0x0000EA-0x7FFFFF)
    uint32_t tlatency_us;
    /// ISO interval in frames\n
    /// From 5ms to 4s
    uint16_t iso_interval_frames;
    /// The number of subevents in each BIS event in the BIG, range 0x01-0x1E
    uint8_t nse;
    /// The number of new payloads in each BIS event, range 0x01-0x07
    uint8_t bn;
    /// Offset used for pre-transmissions, range 0x00-0x0F
    uint8_t pto;
    /// The number of times a payload is transmitted in a BIS event, range 0x01-0x0F
    uint8_t irc;
    /// Maximum size of the payload in octets, range 0x00-0xFB
    uint8_t max_pdu;
    /// PHY
    uint8_t phy;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    uint16_t conhdl[3];
} app_bap_bc_src_cmp_info_t;

/// Stream information
typedef struct app_bap_bc_src_stream_info
{
    /// Stream local identifier
    uint8_t                  stream_lid;
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// IAP configuration
    app_gaf_bap_dp_cfg_t     dp_cfg;
    /// Pointer to level 3 Codec configuration value
    uint8_t                  cfg_len;
    /// Pointer to Codec Configuration structure
    app_gaf_bap_cfg_t        *p_cfg;
    /// BIS Handle
    uint16_t                 bis_hdl;
} app_bap_bc_src_stream_info_t;

/// Subgroup information structure
typedef struct app_bap_bc_src_subgrp_info
{
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// Codec ID
    app_gaf_codec_id_t       codec_id;
    /// Length of complete Codec Configuration
    uint8_t                  cfg_len;
    /// Pointer to Codec Configuration structure
    app_gaf_bap_cfg_t           *p_cfg;
    /// Length of complete Metadata
    uint8_t                  metadata_len;
    /// Pointer to Metadata structure
    app_gaf_bap_cfg_metadata_t  *p_metadata;
} app_bap_bc_src_subgrp_info_t;

/// Broadcast Source BIG Information Structure
typedef struct app_bap_bc_src_grp_param
{
    //// Add group apram
    /// Broadcast ID
    app_gaf_bap_bcast_id_t     bcast_id;
    /// Number of Streams in the Broadcast Group. Cannot be 0
    uint8_t                    nb_streams;
    /// Number of Subgroups in the Broadcast Group. Cannot be 0
    uint8_t                    nb_subgroups;
    /// Broadcast Group parameters
    app_gaf_bap_bc_grp_param_t     grp_param;
    /// Advertising parameters
    app_gaf_bap_bc_adv_param_t     adv_param;
    /// Periodic Advertising parameters
    app_gaf_bap_bc_per_adv_param_t per_adv_param;
    // Audio output presentation delay in microseconds
    uint32_t                   pres_delay_us;

    //// Enable bis param
    // Indicate if streams are encrypted (!= 0) or not
    uint8_t                    encrypted;
    // Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    app_gaf_bc_code_t          bcast_code;

    //// Enable pa param
    /// Length of additional advertising data
    uint8_t                    adv_data_len;
    /// Additional advertising data
    uint8_t                    *adv_data;
    /// Length of additional periodic advertising data
    uint8_t                    per_adv_data_len;
    /// Additional periodic advertising data
    uint8_t                    *per_adv_data;
} app_bap_bc_src_grp_param_t;

typedef struct app_bap_bc_src_grp
{
    /// Group local index
    uint8_t                        grp_lid;
    /// BIG State
    uint8_t                        big_state;
    /// creat big cmp info
    app_bap_bc_src_cmp_info_t      grp_cmp;
    /// set big group param
    app_bap_bc_src_grp_param_t     big_param;
    /// Pointer to array of Subgroup information structure
    app_bap_bc_src_subgrp_info_t  *subgrp_info;
    /// Array of Stream information structures
    app_bap_bc_src_stream_info_t  *stream_info;
} app_bap_bc_src_grp_info_t;

/// Content of Broadcast source environment
typedef struct app_bap_bc_src_env
{
    app_bap_bc_src_grp_info_t p_grp[APP_BAP_DFT_BC_SRC_MAX_BIG_NUM];
} app_bap_bc_src_env_t;

typedef struct app_bap_bc_src_cfg_info
{
    /// Broadcast ID
    app_gaf_bap_bcast_id_t bcast_id;
    /// Number of Streams in the Broadcast Group. Cannot be 0
    uint8_t                nb_streams;
    /// Number of Subgroups in the Broadcast Group. Cannot be 0
    uint8_t                nb_subgroups;
    /// SDU interval in microseconds
    uint32_t sdu_intv_us;
    /// Maximum size of an SDU
    uint16_t max_sdu;
    /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
    /// of the same SDU
    uint16_t max_tlatency;
    /// Minimum advertising interval in multiple of 0.625ms. Must be higher than 20ms.
    uint32_t adv_intv_min_slot;
    /// Maximum advertising interval in multiple of 0.625ms. Must be higher than 20ms.
    uint32_t adv_intv_max_slot;
    /// Minimum periodic advertising interval in multiple of 1.25ms. Must be higher than 7.5ms
    uint32_t adv_intv_min_frame;
    /// Maximum periodic advertising interval in multiple of 1.25ms. Must be higher than 7.5ms
    uint32_t adv_intv_max_frame;
    // Indicate if streams are encrypted (!= 0) or not
    uint8_t encrypted;
    // Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    app_gaf_bc_code_t bcast_code;
} app_bap_bc_src_cfg_info_t;

/// Content of Broadcast source environment
typedef struct
{
    uint8_t  stream_lid;
    uint16_t bis_hdl;
} app_bap_bc_src_stream_msg_t;

typedef bool (*app_bap_src_cfg_info_cb_func)(app_bap_bc_src_cfg_info_t *info);

#ifdef __cplusplus
extern "C" {
#endif

int app_bap_bc_src_enable_pa(app_bap_bc_src_grp_info_t *p_grp);
int app_bap_bc_src_enable(app_bap_bc_src_grp_info_t *p_grp);
int app_bap_bc_src_disable_pa(uint8_t grp_lid);
int app_bap_bc_src_disable(uint8_t grp_lid);

int app_bap_bc_src_find_big_idx(uint8_t grp_lid);
int app_bap_bc_src_find_stream_lid(uint8_t bis_hdl);
app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info_by_big_idx(uint8_t big_idx);
app_bap_bc_src_grp_info_t *app_bap_bc_src_get_big_info_by_bis_hdl(uint16_t bis_hdl);
app_bap_bc_src_stream_info_t *app_bap_bc_src_get_bis_stream_info(uint8_t big_idx, uint8_t bis_index);
int app_bap_bc_src_update_grp_info(uint8_t big_idx, uint8_t nb_subgrp,
                                   uint8_t nb_stream, uint32_t sdu_itvl, uint16_t max_sdu);

int app_bap_bc_src_iso_send_data_to_all_channel(uint8_t **payload, uint16_t payload_len, uint32_t ref_time);

int app_bap_bc_src_register_cfg_info_cb(app_bap_src_cfg_info_cb_func cb);
#ifdef __cplusplus
}
#endif

#endif
#endif // APP_BAP_BC_SRC_MSG_H_

/// @} APP_BAP
