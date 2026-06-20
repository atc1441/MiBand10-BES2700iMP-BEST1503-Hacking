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
#ifndef __PUBLIC_BROADCAST_AUDIO__
#define __PUBLIC_BROADCAST_AUDIO__
#include "bluetooth.h"

#include "gaf_cfg.h"
#include "generic_audio.h"

/*ENUMERATION*/
enum public_broadcast_announcemnt_features
{
    /// Encryption of the BIG
    PBA_FEAT_BC_STREAMS_ENCRYPTED_BIT           = CO_BIT(0),
    /// Standard Quality Public Broadcast Audio
    PBA_FEAT_STAND_QUAL_AUD_CFG_PRESENT_BIT     = CO_BIT(1),
    /// High Quality Public Broadcast Audio
    PBA_FEAT_HIGH_QUAL_AUD_CFG_PRESENT_BIT      = CO_BIT(2),
    /// RFU
    PBA_FEAT_RFU_MASK                           = (~(0b111)),
};

/*TYPEDEF*/
typedef struct public_broadcast_announcemnt
{
    /// Public Broadcast Announcement features
    uint8_t pba_features;
    /// metadata
    gen_aud_metadata_t metadata;
} pbp_pba_info_t;

typedef struct pbp_broadcast_name_pointer
{
    /// Broadcast Name length
    uint8_t name_length;
    /// Broadcast Name Pointer
    const uint8_t *p_name;
} pbp_broadcast_name_ptr_t;

typedef struct public_broadcast_announcemnt_packed
{
    /// Public Broadcast Announcement features
    uint8_t pba_features;
    /// metadata packed in ltv
    gen_aud_var_info_t metadata_ltv;
} __attribute__((packed)) pbp_pba_p_t;

/*FUNCTIONS*/
/**
 * @brief Public broadcast audio profile data unpack
 *
 * @param  length      Length of @see p_pba_data
 * @param  p_pba_data  PBA data value pointer
 * @param  p_pba_feat  PBA feature value pointer
 * @param  p_metadata_ptr
 *                     PBA metadata pointer value pointer
 *
 * @return uint16_t    status
 */
uint16_t pbp_pba_data_unpack(uint8_t length, const uint8_t *p_pba_data, uint8_t *p_pba_feat, gen_aud_metadata_ptr_t *p_metadata_ptr);

/**
 * @brief Public broadcast audio profile data pack
 *
 * @param  pba_features
 *                     PBA feature to be packed
 * @param  p_metadata  PBA metadata to be packed
 * @param  p_pba_data_packed_out
 *                     PBA data value pointer return
 *
 * @return uint16_t    status
 */
uint16_t pbp_pba_data_pack(uint8_t pba_features, const gen_aud_metadata_t *p_metadata, pbp_pba_p_t *p_pba_data_packed_out);

#endif /// __PUBLIC_BROADCAST_AUDIO__