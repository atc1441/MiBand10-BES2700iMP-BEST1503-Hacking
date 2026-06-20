/**
 * @brief Bluetooth Service Low Energy Audio Unicast Server
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BT_SVC_LEA_UC_SERVER_H__
#define __BT_SVC_LEA_UC_SERVER_H__

/*****************************header include********************************/
#include "bluetooth_ble_api.h"
#include "ble_aob_common.h"

/******************************macro defination*****************************/
/// 8 bit access types
#define _8_Bit                              8
/// 16 bit access types
#define _16_Bit                             16
/// 32 bit access types
#define _32_Bit                             32

#define _STM_ENUM_DEF(z, t)                 z ## t

/// Invalid local index
#define AOB_INVALID_LID     (0xFF)

#ifndef LC3PLUS_SUPPORT
/// lc3 sink pac_lid
#define AOB_LC3_SINK_PAC_LID      (0x00)
/// lc3 source pac_lid
#define AOB_LC3_SRC_PAC_LID       (0x01)
#else
/// lc3 sink pac_lid
#define AOB_LC3_SINK_PAC_LID      (0x00)
/// lc3plus sink pac_lid
#define AOB_LC3PLUS_SINK_PAC_LID  (0x01)
/// lc3 source pac_lid
#define AOB_LC3_SRC_PAC_LID       (0x02)
/// lc3plus source pac_lid
#define AOB_LC3PLUS_SRC_PAC_LID   (0x03)
#endif
/// Length of Codec ID value
#define AOB_CODEC_ID_LEN    (5)

/******************************type defination******************************/
/// audio stream type
enum
{
    AOB_CIG_CIS_MUSIC_INDEX =0,
    AOB_CIG_CIS_CALL_INDEX = 1,
    AOB_CIG_CIS_VOICE_INDEX = 2,
    AOB_CIG_CIS_MAX_NUM
};

/// Codec Type values
typedef enum
{
    /// LC3 Codec
    AOB_CODEC_TYPE_LC3    = 0x06,

    /// Maximum SIG Codec
    AOB_CODEC_TYPE_SIG_MAX,

    /// Same codec as broadcast group codec use for broadcast subgroup
    AOB_CODEC_TYPE_SAME   = 0xFE,
    /// Vendor Specific Codec
    AOB_CODEC_TYPE_VENDOR = 0xFF,
} AOB_CODEC_TYPE_E;

/// Supported Sampling_Frequency Bitfield
typedef enum
{
    AOB_SUPPORTED_SAMPLE_FREQ_8000    = 0x0001,
    AOB_SUPPORTED_SAMPLE_FREQ_11025   = 0x0002,
    AOB_SUPPORTED_SAMPLE_FREQ_16000   = 0x0004,
    AOB_SUPPORTED_SAMPLE_FREQ_22050   = 0x0008,
    AOB_SUPPORTED_SAMPLE_FREQ_24000   = 0x0010,
    AOB_SUPPORTED_SAMPLE_FREQ_32000   = 0x0020,
    AOB_SUPPORTED_SAMPLE_FREQ_44100   = 0x0040,
    AOB_SUPPORTED_SAMPLE_FREQ_48000   = 0x0080,
    AOB_SUPPORTED_SAMPLE_FREQ_88200   = 0x0100,
    AOB_SUPPORTED_SAMPLE_FREQ_96000   = 0x0200,
    AOB_SUPPORTED_SAMPLE_FREQ_176400  = 0x0400,
    AOB_SUPPORTED_SAMPLE_FREQ_192000  = 0x0800,
    AOB_SUPPORTED_SAMPLE_FREQ_384000  = 0x1000,
} AOB_SUPPORTED_SAMPLING_FREQS_BF_E;

/// Supported Frame Duration Bitfield
/*
Examples:
0x01 = 0b00000001: The codec supports the 7.5 ms frame duration.
0x02 = 0b00000010: The codec supports the 10 ms frame duration.
0x03 = 0b00000011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration.
0x15 = 0b00010011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration and prefers the 7.5 ms frame duration.
*/
typedef enum
{
    AOB_SUPPORTED_FRAME_DURATION_7_5MS  = 0x01,
    AOB_SUPPORTED_FRAME_DURATION_10MS   = 0x02,
#ifdef LC3PLUS_SUPPORT
    AOB_SUPPORTED_FRAME_DURATION_5MS    = 0x04,
    AOB_SUPPORTED_FRAME_DURATION_2_5MS  = 0x08,
#endif
    ///  7.5 ms preferred.valid only when 7.5 ms is supported and 10 ms is supported. Shall not be set to 0b1 if bit 5 is set to 0b1.
    AOB_PREFERRED_FRAME_DURATION_7_5MS  = 0x10,
    /// 10 ms preferred. Valid only when 7.5 ms is supported and 10 ms is supported. Shall not be set to 0b1 if bit 4 is set to 0b1.
    AOB_PREFERRED_FRAME_DURATION_10MS   = 0x20,
#ifdef LC3PLUS_SUPPORT
    AOB_PREFERRED_FRAME_DURATION_5MS    = 0x40,
    AOB_PREEFERED_FRAME_DURATION_2_5MS  = 0x80,
#endif
} AOB_SUPPORTED_FRAME_DURATIONS_BF_E;

/// Supported Audio Location Bitfield
typedef enum
{
    AOB_SUPPORTED_LOCATION_FRONT_LEFT               = 0x00000001,
    AOB_SUPPORTED_LOCATION_FRONT_RIGHT              = 0x00000002,
    AOB_SUPPORTED_LOCATION_FRONT_CENTER             = 0x00000004,
    AOB_SUPPORTED_LOCATION_LOW_FREQ_EFFECTS_1       = 0x00000008,
    AOB_SUPPORTED_LOCATION_BACK_LEFT                = 0x00000010,
    AOB_SUPPORTED_LOCATION_BACK_RIGHT               = 0x00000020,
    AOB_SUPPORTED_LOCATION_FRONT_LEFT_OF_CENTER     = 0x00000040,
    AOB_SUPPORTED_LOCATION_FRONT_RIGHT_OF_CENTER    = 0x00000080,
    AOB_SUPPORTED_LOCATION_BACK_CENTER              = 0x00000100,
    AOB_SUPPORTED_LOCATION_LOW_FREQ_EFFECTS_2       = 0x00000200,
    AOB_SUPPORTED_LOCATION_SIDE_LEFT                = 0x00000400,
    AOB_SUPPORTED_LOCATION_SIDE_RIGHT               = 0x00000800,
    AOB_SUPPORTED_LOCATION_TOP_FRONT_LEFT           = 0x00001000,
    AOB_SUPPORTED_LOCATION_TOP_FRONT_RIGHT          = 0x00002000,
    AOB_SUPPORTED_LOCATION_TOP_FRONT_CENTER         = 0x00004000,
    AOB_SUPPORTED_LOCATION_TOP_CENTER               = 0x00008000,
    AOB_SUPPORTED_LOCATION_TOP_BACK_LEFT            = 0x00010000,
    AOB_SUPPORTED_LOCATION_TOP_BACK_RIGHT           = 0x00020000,
    AOB_SUPPORTED_LOCATION_TOP_SIDE_LEFT            = 0x00040000,
    AOB_SUPPORTED_LOCATION_TOP_SIDE_RIGHT           = 0x00080000,
    AOB_SUPPORTED_LOCATION_TOP_BACK_CENTER          = 0x00100000,
    AOB_SUPPORTED_LOCATION_BOTTOM_FRONT_CENTER      = 0x00200000,
    AOB_SUPPORTED_LOCATION_BOTTOM_FRONT_LEFT        = 0x00400000,
    AOB_SUPPORTED_LOCATION_BOTTOM_FRONT_RIGHT       = 0x00800000,
    AOB_SUPPORTED_LOCATION_FRONT_LEFT_WIDE          = 0x01000000,
    AOB_SUPPORTED_LOCATION_FRONT_RIGHT_WIDE         = 0x02000000,
    AOB_SUPPORTED_LOCATION_LEFT_SURROUND            = 0x04000000,
    AOB_SUPPORTED_LOCATION_RIGHT_SURROUND           = 0x08000000,

    AOB_SUPPORTED_LOCATION_RFU                      = 0xF0000000,
} AOB_SUPPORTED_LOCATION_BF_E;

/// Codec Identifier
typedef struct
{
    /// Codec ID value
    uint8_t codec_id[AOB_CODEC_ID_LEN];
} aob_codec_id_t;

/// LC3 Capabilities structure
typedef struct
{
    /// Pointer to Codec Capabilities structure (allocated by Upper Layer)
    AOB_BAP_CAPA_T *capa;
    /// Pointer to Codec Capabilities Metadata structure (allocated by Upper Layer)
    AOB_BAP_CFG_METADATA_T *metadata;
} aob_lc3_capa_t;

/****************************function declearation**************************/
uint8_t bt_svc_lea_uc_server_query_ble_mobile_by_address(ble_bdaddr_t *addr);

#endif