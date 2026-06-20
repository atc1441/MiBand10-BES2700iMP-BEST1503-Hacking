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
#ifndef __BAP_BC_COMMON__
#define __BAP_BC_COMMON__

#include "bluetooth.h"

#include "gaf_cfg.h"

#include "generic_audio.h"

/// Unknown Periodic Advertising interval
#define BAP_BC_UNKNOWN_PA_INTV                    (0xFFFF)
/// Maximum Presentation Delay for Broadcast (in microseconds)
#define BAP_BC_MAX_PRES_DELAY_US                  (0x0000FFFF)
/// Length of Broadcast Id
#define BAP_BC_BROADCAST_ID_LEN                   (3)
/// Length of Broadcast Code
#define BAP_BC_BROADCAST_CODE_LEN                 (16)
/// Minimum index in big
#define BAP_BC_BIS_INDEX_MIN                      (1)
/// Maximum index in big
#define BAP_BC_BIS_INDEX_MAX                      (31)
/// Maximum number of Subgroups in a Group
#define BAP_BC_NUM_SUBGROUPS_MAX                  (BAP_BC_BIS_INDEX_MAX)
/// Length of header for Basic Audio Announcement (without length field)
#define BAP_BC_BASIC_AUDIO_ANNOUNCE_HEAD_LENGTH   (3)
/// Minimal length of Broadcast Audio Announcement
#define BAP_BC_BCAST_AUDIO_ANNOUNCEMENT_LEN       (6)

#define BAP_BC_CONTROLLER_DELAY_US                (GAF_CONTROLLER_DELAY_US)

#define BAP_BC_INVALID_GRP_LID                    (0xFF)

#define BAP_BC_INVALID_PA_SYNC_HDL                (0xFFFF)

#define BAP_BC_PA_SYNC_PARAM_CTE_TYPE             (0)

/*
 * ENUMERATIONS
 ****************************************************************************************
 */

typedef enum bap_bc_common_user
{
    BAP_BC_COMMON_USER_MIN = 0,
    /// Scan Delegator
    BAP_BC_COMMON_USER_INTERNAL_DELEGATOR = BAP_BC_COMMON_USER_MIN,
    /// Broadcast Assistant
    BAP_BC_COMMON_USER_INTERNAL_ASSISTANT,
    /// Internal Use USER max
    BAP_BC_COMMON_USER_INTERNAL_MAX,
    /// App layer implement, like LEA player
    BAP_BC_COMMON_USER_APPLICATION_IMPL = BAP_BC_COMMON_USER_INTERNAL_MAX,

    BAP_BC_COMMON_USER_MAX,
} bap_bc_common_user_e;

/*
 * TYPEDEF
 ****************************************************************************************
*/
typedef struct bap_bc_addr_info
{
    /// Source addr info
    ble_bdaddr_t adv_addr;
    /// Source adv sid
    uint8_t adv_sid;
} bap_bc_pa_addr_t;

typedef struct bap_bc_base_lvl_3
{
    /// BIS index
    uint8_t bis_idx;
    /// Codec info for subgrp (May be NULL == zero length)
    const gen_aud_cc_t *p_codec_cfg;
} bap_bc_base_bis_info_t;

typedef struct bap_bc_base_lvl_2
{
    /// Codec info for subgrp (May be NULL == zero length)
    const gen_aud_cc_t *p_codec_cfg;
    /// Metadata for sub_grp (May be NULL == zero length)
    const gen_aud_metadata_t *p_metadata;
    /// Codec ID
    uint8_t codec_id[GEN_AUD_CODEC_ID_LEN];
    /// Subgrp BIS count
    uint8_t num_bis;
    /// Level 3 in BASE
    bap_bc_base_bis_info_t specific_bis_info[GAF_ARRAY_EMPTY];
} bap_bc_base_sub_grp_t;

typedef struct bap_bc_base_lvl_1
{
    /// Presentation Delay
    uint32_t pres_delay;
    /// Number subgroups
    uint8_t num_sub_grp;
    /// Sub gropu info
    bap_bc_base_sub_grp_t sub_grp[GAF_ARRAY_EMPTY];
} bap_bc_base_grp_info_t;

typedef struct bap_bc_big_info
{
    /// Value of the SDU interval in microseconds (Range 0x0000FF-0x0FFFFF)
    uint32_t  sdu_interval;
    /// Value of the ISO Interval (1.25 ms unit)
    uint16_t  iso_interval;
    /// Value of the maximum PDU size (Range 0x0000-0x00FB)
    uint16_t  max_pdu;
    /// VValue of the maximum SDU size (Range 0x0000-0x0FFF)
    uint16_t  max_sdu;
    /// Number of BIS present in the group (Range 0x01-0x1F)
    uint8_t   num_bis;
    /// Number of sub-events (Range 0x01-0x1F)
    uint8_t   nse;
    /// Burst number (Range 0x01-0x07)
    uint8_t   bn;
    /// Pre-transmit offset (Range 0x00-0x0F)
    uint8_t   pto;
    /// Initial retransmission count (Range 0x01-0x0F)
    uint8_t   irc;
    /// PHY used for transmission (0x01: 1M, 0x02: 2M, 0x03: Coded, All other values: RFU)
    uint8_t   phy;
    /// Framing mode (0x00: Unframed, 0x01: Framed, All other values: RFU)
    uint8_t   framing;
    /// True if broadcast isochronous group is encrypted, False otherwise
    bool      encrypted;
} bap_bc_big_info_t;

typedef struct bap_bc_sub_grp
{
    /// Subgrp bis sync request
    uint32_t bis_sync_req_bf;
    /// Metadata for sub_grp (May be NULL == zero length)
    gen_aud_metadata_t *p_metadata;
} bap_bc_sub_grp_t;

typedef struct bap_bc_bass_src_info_grp_pointer
{
    /// Source addr info
    const bap_bc_pa_addr_t *p_src_addr;
    /// Broadcast ID
    const uint8_t *p_broadcast_id;
    /// PA interval
    uint16_t pa_interval;
    /// PA sync req
    uint8_t pa_sync_req;
    /// Number of subgrp
    uint8_t sub_grp_num;
    /// Sub group info list
    const bap_bc_sub_grp_t *p_subgrp_list;
} bap_bc_bass_src_info_req_ptr_t;

typedef struct bap_bc_assist_src_sub_grp_pointer
{
    /// Source addr info
    const bap_bc_pa_addr_t *p_src_addr;
    /// Broadcast ID
    const uint8_t *p_broadcast_id;
    /// PA sync state
    uint8_t pa_sync_state;
    /// BIG encryption
    uint8_t big_encryption;
    /// Bad code, present if big encryption is bad code
    const uint8_t *p_bad_code;
    /// Number of subgrp
    uint8_t sub_grp_num;
    /// Sub group info list
    const bap_bc_sub_grp_t *p_subgrp_list;
} bap_bc_bass_src_info_ind_ptr_t;

/*FUNCTIONS DECLARATIONS*/
/**
 * @brief BAP broadcast common BASE data check
 *
 * @param  length      Total base data length
 * @param  p_base_data Base data value pointer
 * @param  p_nb_subgroups
 *                     Number of subgrps
 * @param  p_nb_streams
 *                     Number of streamsss
 *
 * @return true        Check success
 * @return false       Check failed
 */
bool bap_bc_common_base_data_check(uint8_t length, const uint8_t *p_base_data, uint8_t *p_nb_subgroups, uint8_t *p_nb_streams);

/**
 * @brief BAP broadcast common Source info PA address check
 *
 * @param  p_pa_addr   Source info PA address
 *
 * @return true        Check success
 * @return false       Check failed
 */
bool bap_bc_commom_pa_addr_check(const bap_bc_pa_addr_t *p_pa_addr);

#endif /// __BAP_BC_COMMON__