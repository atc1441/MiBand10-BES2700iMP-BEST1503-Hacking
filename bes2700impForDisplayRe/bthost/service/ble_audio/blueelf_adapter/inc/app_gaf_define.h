/**
 ****************************************************************************************
 *
 * @file app_gaf_define.h
 *
 * @brief BLE Audio Generic Audio Framework
 *
 * Copyright 2015-2021 BES.
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
 * @addtogroup APP_GAF_BAP
 * @{
 ****************************************************************************************
 */
/**
 * NOTE: This header file defines the common used module for upper layer
 */

#ifndef APP_GAF_DEFINE_H_
#define APP_GAF_DEFINE_H_

#if BLE_AUDIO_ENABLED
/*****************************header include********************************/
#include <stdbool.h>       // standard boolean definitions
#include <stddef.h>        // standard definitions
#include <stdint.h>        // standard integer definitions

#include "co_math.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************macro defination*****************************/
/// Retrieves module id from event id.
#define GAF_ID_GET(event_id) (((event_id) & 0xF000) >> 12)
/// Retrieves event from event id.
#define GAF_EVENT_GET(event_id) ((event_id) & 0x0FFF)
/// Builds the identifier from the id and the event
#define GAF_BUILD_ID(id, event) ( (uint16_t)(((id) << 12)|(event)) )

#define AOB_CODEC_ID_LC3          (&codec_id_lc3)
#define AOB_CODEC_ID_LC3PLUS      (&codec_id_lc3plus)

#define APP_GAF_CSIS_SIRK_LEN_VALUE         (16)
#define APP_GAF_CSIS_RSI_LEN                (6)
#define APP_GAF_BAP_BC_BROADCAST_ID_LEN     (3)
#define APP_GAF_BAP_PER_ADV_DATA_MAX_LEN    (247)

#define APP_GAF_INVALID_LID                 (0xFF)

#define APP_GAF_INVALID_PRF_LID             (0xFF)
#define APP_GAF_INVALID_CON_LID             (0xFF)
#define APP_GAF_INVALID_ANY_LID             (0xFF)
#define APP_GAF_INVALID_CON_HDL             (0xFFFF)

#define APP_CO_BIT(pos) (1UL<<(pos))

/// Length of GAP Broadcast ID
#define APP_GAP_BCAST_ID_LEN    (3)
/// Length of GAP broadcast Key
#define APP_GAP_KEY_LEN         (16)
/// Length of Device Address
#define APP_GAP_BD_ADDR_LEN     (6)
/// Default Preffered MTU
#define APP_GAF_DFT_PREF_MTU    (128)
/// Length of Codec ID value
#define APP_GAF_CODEC_ID_LEN    (5)

typedef struct app_gaf_codec_id app_gaf_codec_id_t;

/// structure of a list element header
/*@TRACE*/
struct app_co_list_hdr
{
    /// Pointer to next co_list_hdr
    struct app_co_list_hdr *next;
};

/// simplify type name of list element header
typedef struct app_co_list_hdr app_co_list_hdr_t;

/// structure of a list
struct app_co_list
{
    /// pointer to first element of the list
    struct app_co_list_hdr *first;
    /// pointer to the last element
    struct app_co_list_hdr *last;

    #if (KE_PROFILING)
    /// number of element in the list
    uint32_t cnt;
    /// max number of element in the list
    uint32_t maxcnt;
    /// min number of element in the list
    uint32_t mincnt;
    #endif //KE_PROFILING
};

/// simplify type name of list
typedef struct app_co_list app_co_list_t;

/**
 * date and time structure
 * size = 7 bytes
 */
/// Time profile information
typedef struct
{
    /// year time element
    uint16_t year;
    /// month time element
    uint8_t  month;
    /// day time element
    uint8_t  day;
    /// hour time element
    uint8_t  hour;
    /// minute time element
    uint8_t  min;
    /// second time element
    uint8_t  sec;
} app_gaf_prf_date_time_t;

/*****************************value declaration*****************************/
extern const app_gaf_codec_id_t codec_id_lc3;
extern const app_gaf_codec_id_t codec_id_lc3plus;
/******************************type defination******************************/
/// Error codes
enum app_gaf_err
{
    /// No Error
    APP_GAF_ERR_NO_ERROR = 0,
    /// Params Invalid
    APP_GAF_ERR_INVALID_PARAM,
    /// Mermory Malloced Failed
    APP_GAF_ERR_MALLOC_ERROR,
};

/// Codec Type values
typedef enum app_gaf_codec_type
{
    /// LC3 Codec
    APP_GAF_CODEC_TYPE_LC3    = 0x06,
    /// Maximum SIG Codec
    APP_GAF_CODEC_TYPE_SIG_MAX,
    /// Vendor Specific Codec
    APP_GAF_CODEC_TYPE_VENDOR = 0xFF,
} APP_GAF_CODEC_TYPE_T;

/// LTV structure format
enum app_gaf_bap_ltv_fmt
{
    /// Length
    APP_GAF_BAP_LTV_LENGTH_POS = 0,
    /// Type
    APP_GAF_BAP_LTV_TYPE_POS,
    /// Value
    APP_GAF_BAP_LTV_VALUE_POS,

    /// Minimal length of LTV structure
    APP_GAF_BAP_LTV_LENGTH_MIN = 1,
};

/// Codec Specific Capabilities Types values
enum app_gaf_bap_capa_type
{
    /// Supported Sampling Frequencies
    APP_GAF_BAP_CAPA_TYPE_SAMP_FREQ = 1,
    /// Supported Frame Durations
    APP_GAF_BAP_CAPA_TYPE_FRAME_DUR,
    /// Audio Channel Counts
    APP_GAF_BAP_CAPA_TYPE_CHNL_CNT,
    /// Supported Octets per Codec Frame
    APP_GAF_BAP_CAPA_TYPE_OCTETS_FRAME,
    /// Maximum Supported Codec Frames per SDU
    APP_GAF_BAP_CAPA_TYPE_FRAMES_SDU,
    /// Supported channel capabilities
    APP_GAF_BAP_CAPA_TYPE_SUPP_CHAN_CAPA = 0xE0,
    /// Supported Codec Frame Interval
    APP_GAF_BAP_CAPA_TYPE_SUPP_CF_INTV = 0xE2,
    /// Supported HT Frame Formats
    APP_GAF_BAP_CAPA_TYPE_SUPP_HTF_FMT = 0xE5,
    /// Supported HT Frame Interval
    APP_GAF_BAP_CAPA_TYPE_SUPP_HTF_INTV = 0xE6,
    /// Supported Rendering Info Frame Interval
    APP_GAF_BAP_CAPA_TYPE_SUPP_RIF_INTV,
};

/// Metadata Types values
enum app_gaf_bap_metadata_type
{
    /// Preferred Audio Contexts
    APP_GAF_BAP_METADATA_TYPE_PREF_CONTEXTS = 1,
    /// Streaming Audio Contexts
    APP_GAF_BAP_METADATA_TYPE_STREAM_CONTEXTS,

    /// Vendor Specific
    APP_GAF_BAP_METADATA_TYPE_VENDOR = 0xFF,
};

/// Minimal value of length field for Codec Specific Capabilities LTV structure
enum app_gaf_bap_capa_length
{
    /// Supported Sampling Frequencies
    APP_GAF_BAP_CAPA_LENGTH_SAMP_FREQ = 3,
    /// Supported Frame Durations
    APP_GAF_BAP_CAPA_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Counts
    APP_GAF_BAP_CAPA_LENGTH_CHNL_CNT = 2,
    /// Supported Octets per Codec Frame
    APP_GAF_BAP_CAPA_LENGTH_OCTETS_FRAME = 5,
    /// Maximum Supported Codec Frames per SDU
    APP_GAF_BAP_CAPA_LENGTH_FRAMES_SDU = 2,
};

/// Minimal value of length field for Metadata LTV structure
enum app_gaf_bap_metadata_length
{
    /// Preferred Audio Contexts
    APP_GAF_BAP_METADATA_LENGTH_PREF_CONTEXTS = 3,
    /// Streaming Audio Contexts
    APP_GAF_BAP_METADATA_LENGTH_STREAM_CONTEXTS = 3,
};

/// Codec Specific Configuration Types values
enum app_gaf_bap_cfg_type
{
    /// Sampling Frequencies
    APP_GAF_BAP_CFG_TYPE_SAMP_FREQ = 1,
    /// Frame Duration
    APP_GAF_BAP_CFG_TYPE_FRAME_DUR,
    /// Audio Channel Allocation
    APP_GAF_BAP_CFG_TYPE_CHNL_LOCATION,
    /// Octets per Codec Frame
    APP_GAF_BAP_CFG_TYPE_OCTETS_FRAME,
    /// Codec Frame Blocks Per SDU
    APP_GAF_BAP_CFG_TYPE_FRAMES_SDU,
    /// Channel Type Configurations
    APP_GAF_BAP_CFG_TYPE_CHAN_TYPE_CFG = 0xE1,
    // Codec Frame Interval
    APP_GAF_BAP_CFG_TYPE_CF_FRAME_INTV = 0xE3,
    // Codec Frame CID
    APP_GAF_BAP_CFG_TYPE_CF_FRAME_CID = 0xE4,
    // Headtracking Frame CID
    APP_GAF_BAP_CFG_TYPE_HT_FRAME_CID = 0xE7,
    // Headtracking Frame Format
    APP_GAF_BAP_CFG_TYPE_HT_FRAME_FMT = 0xE8,
    // Headtracking Frame Interval
    APP_GAF_BAP_CFG_TYPE_HT_FRAME_INTV = 0xE9,
    // Rendering Info Frame Format
    APP_GAF_BAP_CFG_TYPE_RI_FRAME_CID,
    // Rendering Info Frame Interval
    APP_GAF_BAP_CFG_TYPE_RI_FRAME_INTV,
};

/// ASE Direction
typedef enum app_gaf_direction
{
    /// Sink direction
    APP_GAF_DIRECTION_SINK = 0,
    /// Source direction
    APP_GAF_DIRECTION_SRC,

    APP_GAF_DIRECTION_MAX,
} app_gaf_direction_t;

/// Sampling_Frequency
typedef enum gaf_bap_sampling_freq
{
    APP_GAF_BAP_SAMPLE_FREQ_8000    = 0x01,
    APP_GAF_BAP_SAMPLE_FREQ_11025,
    APP_GAF_BAP_SAMPLE_FREQ_16000,
    APP_GAF_BAP_SAMPLE_FREQ_22050,
    APP_GAF_BAP_SAMPLE_FREQ_24000,
    APP_GAF_BAP_SAMPLE_FREQ_32000,
    APP_GAF_BAP_SAMPLE_FREQ_44100,
    APP_GAF_BAP_SAMPLE_FREQ_48000,
    APP_GAF_BAP_SAMPLE_FREQ_88200,
    APP_GAF_BAP_SAMPLE_FREQ_96000,
    APP_GAF_BAP_SAMPLE_FREQ_176400,
    APP_GAF_BAP_SAMPLE_FREQ_192000,
    APP_GAF_BAP_SAMPLE_FREQ_384000,

    APP_GAF_BAP_SAMPLE_FREQ_MAX,
} GAF_BAP_SAMLLING_REQ_T;

// Frame_Duration   #app_gaf_bap_frame_dur
enum gaf_bap_frame_duration
{
    APP_GAF_BAP_FRAME_DURATION_7_5MS    = 0x00,
    APP_GAF_BAP_FRAME_DURATION_10MS     = 0x01,
    APP_GAF_BAP_FRAME_DURATION_5MS      = 0x02,
    APP_GAF_BAP_FRAME_DURATION_2_5MS    = 0x03,
    APP_GAF_BAP_FRAME_DURATION_MAX,
};

/// Minimal value of length field for Codec Specific Configuration LTV structure
enum app_gaf_bap_cfg_length
{
    /// Sampling Frequencies
    APP_GAF_BAP_CFG_LENGTH_SAMP_FREQ = 2,
    /// Frame Duration
    APP_GAF_BAP_CFG_LENGTH_FRAME_DUR = 2,
    /// Audio Channel Allocation
    APP_GAF_BAP_CFG_LENGTH_CHNL_LOCATION = 5,
    /// Octets per Codec Frame
    APP_GAF_BAP_CFG_LENGTH_OCTETS_FRAME = 3,
    /// Codec Frame Blocks Per SDU
    APP_GAF_BAP_CFG_LENGTH_FRAMES_SDU = 2,
};

/// Sampling Frequency values
enum app_gaf_bap_sampling_freq
{
    APP_GAF_BAP_SAMPLING_FREQ_MIN = 1,
    /// 8000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_8000HZ = APP_GAF_BAP_SAMPLING_FREQ_MIN,
    /// 11025 Hz
    APP_GAF_BAP_SAMPLING_FREQ_11025HZ,
    /// 16000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_16000HZ,
    /// 22050 Hz
    APP_GAF_BAP_SAMPLING_FREQ_22050HZ,
    /// 24000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_24000HZ,
    /// 32000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_32000HZ,
    /// 44100 Hz
    APP_GAF_BAP_SAMPLING_FREQ_44100HZ,
    /// 48000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_48000HZ,
    /// 88200 Hz
    APP_GAF_BAP_SAMPLING_FREQ_88200HZ,
    /// 96000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_96000HZ,
    /// 176400 Hz
    APP_GAF_BAP_SAMPLING_FREQ_176400HZ,
    /// 192000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_192000HZ,
    /// 384000 Hz
    APP_GAF_BAP_SAMPLING_FREQ_384000HZ,

    /// Maximum value
    APP_GAF_BAP_SAMPLING_FREQ_MAX
};

/// Supported Sampling Frequencies bit field meaning
enum app_gaf_bap_sampling_freq_bf
{
    /// 8000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_8000HZ_POS = 0,
    /// 8000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_8000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_8000HZ_POS),

    /// 11025 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_11025HZ_POS = 1,
    /// 11025 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_11025HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_11025HZ_POS),

    /// 16000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_16000HZ_POS = 2,
    /// 16000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_16000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_16000HZ_POS),

    /// 22050 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_22050HZ_POS = 3,
    /// 22050 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_22050HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_22050HZ_POS),

    /// 24000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_24000HZ_POS = 4,
    /// 24000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_24000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_24000HZ_POS),

    /// 32000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_32000HZ_POS = 5,
    /// 32000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_32000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_32000HZ_POS),

    /// 44100 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_44100HZ_POS = 6,
    /// 44100 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_44100HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_44100HZ_POS),

    /// 48000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_48000HZ_POS = 7,
    /// 48000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_48000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_48000HZ_POS),

    /// 88200 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_88200HZ_POS = 8,
    /// 88200 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_88200HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_88200HZ_POS),

    /// 96000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_96000HZ_POS = 9,
    /// 96000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_96000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_96000HZ_POS),

    /// 176400 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_176400HZ_POS = 10,
    /// 176400 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_176400HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_176400HZ_POS),

    /// 192000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_192000HZ_POS = 11,
    /// 192000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_192000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_192000HZ_POS),

    /// 384000 Hz - Position
    APP_GAF_BAP_SAMPLING_FREQ_384000HZ_POS = 12,
    /// 384000 Hz - Bit
    APP_GAF_BAP_SAMPLING_FREQ_384000HZ_BIT = APP_CO_BIT(APP_GAF_BAP_SAMPLING_FREQ_384000HZ_POS),
};

/// Frame Duration values
enum app_gaf_bap_frame_dur
{
    APP_GAF_BAP_FRAME_DUR_MIN    = 0,
    /// Use 7.5ms Codec frames
    APP_GAF_BAP_FRAME_DUR_7_5MS  = 0,
    /// Use 10ms Codec frames
    APP_GAF_BAP_FRAME_DUR_10MS   = 1,
#ifdef LC3PLUS_SUPPORT
    /// Use 5ms Codec frames
    APP_GAF_BAP_FRAME_DUR_5MS    = 2,
    /// Use 2.5ms Codec frames
    APP_GAF_BAP_FRAME_DUR_2_5MS  = 3,
#endif
    /// Maximum value
    APP_GAF_BAP_FRAME_DUR_MAX
};

/// Supported Frame Duration Bitfield
/*
Examples:
0x01 = 0b00000001: The codec supports the 7.5 ms frame duration.
0x02 = 0b00000010: The codec supports the 10 ms frame duration.
0x03 = 0b00000011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration.
0x13 = 0b00010011: The codec supports the 7.5 ms frame duration and the 10 ms frame duration and prefers the 7.5 ms frame duration.
*/
/// Supported Frame Durations bit field meaning
enum app_gaf_bap_frame_dur_bf
{
    /// 7.5ms frame duration is supported - Position
    APP_GAF_BAP_FRAME_DUR_7_5MS_POS = 0,
    /// 7.5ms frame duration is supported - Bit
    APP_GAF_BAP_FRAME_DUR_7_5MS_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_7_5MS_POS),

    /// 10ms frame duration is supported - Position
    APP_GAF_BAP_FRAME_DUR_10MS_POS = 1,
    /// 10ms frame duration is supported - Bit
    APP_GAF_BAP_FRAME_DUR_10MS_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_10MS_POS),

#ifdef LC3PLUS_SUPPORT
    /// 5ms frame duration is supported - Position
    APP_GAF_BAP_FRAME_DUR_5MS_POS  = 2,
    /// 5ms frame duration is supported - Bit
    APP_GAF_BAP_FRAME_DUR_5MS_BIT  = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_5MS_POS),

    /// 10ms frame duration is supported - Position
    APP_GAF_BAP_FRAME_DUR_2_5MS_POS = 3,
    /// 10ms frame duration is supported - Bit
    APP_GAF_BAP_FRAME_DUR_2_5MS_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_2_5MS_POS),
#endif

    /// 7.5ms frame duration is preferred - Position
    APP_GAF_BAP_FRAME_DUR_7_5MS_PREF_POS = 4,
    /// 7.5ms frame duration is preferred - Bit
    APP_GAF_BAP_FRAME_DUR_7_5MS_PREF_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_7_5MS_PREF_POS),

    /// 10ms frame duration is preferred - Position
    APP_GAF_BAP_FRAME_DUR_10MS_PREF_POS = 5,
    /// 10ms frame duration is preferred - Bit
    APP_GAF_BAP_FRAME_DUR_10MS_PREF_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_10MS_PREF_POS),
#ifdef LC3PLUS_SUPPORT
    /// 10ms frame duration is preferred - Position
    APP_GAF_BAP_FRAME_DUR_5MS_PREF_POS = 6,
    /// 10ms frame duration is preferred - Bit
    APP_GAF_BAP_FRAME_DUR_5MS_PREF_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_5MS_PREF_POS),

    /// 10ms frame duration is preferred - Position
    APP_GAF_BAP_FRAME_DUR_2_5MS_PREF_POS = 7,
    /// 10ms frame duration is preferred - Bit
    APP_GAF_BAP_FRAME_DUR_2_5MS_PREF_BIT = APP_CO_BIT(APP_GAF_BAP_FRAME_DUR_2_5MS_PREF_POS),
#endif
};

/// Context type bit field meaning
enum app_gaf_bap_context_type_bf
{
    /// Unspecified - Position
    APP_GAF_BAP_CONTEXT_TYPE_UNSPECIFIED_POS = 0,
    /// Unspecified - Bit
    APP_GAF_BAP_CONTEXT_TYPE_UNSPECIFIED_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_UNSPECIFIED_POS),

    /// Conversational - Position
    APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL_POS = 1,
    /// Conversational - Bit\n
    /// Conversation between humans as, for example, in telephony or video calls
    APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL_POS),

    /// Media - Position
    APP_GAF_BAP_CONTEXT_TYPE_MEDIA_POS = 2,
    /// Media - Bit\n
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    APP_GAF_BAP_CONTEXT_TYPE_MEDIA_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_MEDIA_POS),

    /// Game - Position
    APP_GAF_BAP_CONTEXT_TYPE_GAME_POS = 3,
    /// Game - Bit\n
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    /// between participants; or a mix of all the above
    APP_GAF_BAP_CONTEXT_TYPE_GAME_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_GAME_POS),

    /// Instructional - Position
    APP_GAF_BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS = 4,
    /// Instructional - Bit\n
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    APP_GAF_BAP_CONTEXT_TYPE_INSTRUCTIONAL_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_INSTRUCTIONAL_POS),

    /// Man Machine - Position
    APP_GAF_BAP_CONTEXT_TYPE_MAN_MACHINE_POS = 5,
    /// Man Machine - Bit\n
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    APP_GAF_BAP_CONTEXT_TYPE_MAN_MACHINE_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_MAN_MACHINE_POS),

    /// Live - Position
    APP_GAF_BAP_CONTEXT_TYPE_LIVE_POS = 6,
    /// Live - Bit\n
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    APP_GAF_BAP_CONTEXT_TYPE_LIVE_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_LIVE_POS),

    /// Sound Effects - Position
    APP_GAF_BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS = 7,
    /// Sound Effects - Bit\n
    /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    APP_GAF_BAP_CONTEXT_TYPE_SOUND_EFFECTS_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_SOUND_EFFECTS_POS),

    /// Attention Seeking - Position
    APP_GAF_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_POS = 8,
    /// Attention Seeking - Bit\n
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    APP_GAF_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_ATTENTION_SEEKING_POS),

    /// Ringtone - Position
    APP_GAF_BAP_CONTEXT_TYPE_RINGTONE_POS = 9,
    /// Ringtone - Bit\n
    /// Ringtone as in a call alert
    APP_GAF_BAP_CONTEXT_TYPE_RINGTONE_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_RINGTONE_POS),

    /// Immediate Alert - Position
    APP_GAF_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_POS = 10,
    /// Immediate Alert - Bit\n
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    APP_GAF_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT_POS),

    /// Emergency Alert - Position
    APP_GAF_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_POS = 11,
    /// Emergency Alert - Bit\n
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    APP_GAF_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_EMERGENCY_ALERT_POS),

    /// TV - Position
    APP_GAF_BAP_CONTEXT_TYPE_TV_POS = 12,
    /// TV - Bit\n
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    APP_GAF_BAP_CONTEXT_TYPE_TV_BIT = APP_CO_BIT(APP_GAF_BAP_CONTEXT_TYPE_TV_POS),
};

/// Supported Audio Channel Counts Bitfield
/*
Example:
0x01 = 0b00000001: One channel supported.
0x02 = 0b00000010: Two channels supported.
0x27 = 0b00100111: One channel supported, two channels supported, three channels supported, and six channels supported.
*/
/// Supported Audio Channel Counts Bitfield
enum app_gaf_bap_chan_cnt_bf
{
    /// One channel supported
    APP_GAF_BAP_CHAN_CNT_1_POS    = 1,
    APP_GAF_BAP_CHAN_CNT_1_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_1_POS),

    /// two channels supported
    APP_GAF_BAP_CHAN_CNT_2_POS    = 2,
    APP_GAF_BAP_CHAN_CNT_2_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_2_POS),

    /// three channels supported
    APP_GAF_BAP_CHAN_CNT_3_POS    = 3,
    APP_GAF_BAP_CHAN_CNT_3_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_3_POS),

    /// four channels supported
    APP_GAF_BAP_CHAN_CNT_4_POS    = 4,
    APP_GAF_BAP_CHAN_CNT_4_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_4_POS),

    /// five channels supported
    APP_GAF_BAP_CHAN_CNT_5_POS    = 5,
    APP_GAF_BAP_CHAN_CNT_5_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_5_POS),

    /// six channels supported
    APP_GAF_BAP_CHAN_CNT_6_POS    = 6,
    APP_GAF_BAP_CHAN_CNT_6_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_6_POS),

    /// seven channels supported
    APP_GAF_BAP_CHAN_CNT_7_POS    = 7,
    APP_GAF_BAP_CHAN_CNT_7_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_7_POS),

    /// eight channels supported
    APP_GAF_BAP_CHAN_CNT_8_POS    = 8,
    APP_GAF_BAP_CHAN_CNT_8_BIT = APP_CO_BIT(APP_GAF_BAP_CHAN_CNT_8_POS),

};

/// Specify what PHY the Controller has changed for TX/RX. HCI:7.7.65.12
/*@TRACE*/
enum app_le_phy_value
{
    APP_PHY_UNDEF_VALUE    = 0,
    APP_PHY_1MBPS_VALUE    = 1,
    APP_PHY_2MBPS_VALUE    = 2,
    APP_PHY_CODED_VALUE    = 3,
#if (mHDT_LE_SUPPORT)
    APP_PHY_4MBPS_VALUE    = 128,
#endif
};

/// Specify what PHY Host prefers to use for RX or TX HCI:7.8.48 / HCI:7.8.49
enum app_le_phy_mask
{
    /// The Host prefers to use the LE 1M transmitter/receiver PHY (possibly among others)
    APP_PHY_1MBPS_BIT      = (1 << 0),
    APP_PHY_1MBPS_POS      = (0),
    /// The Host prefers to use the LE 2M transmitter/receiver PHY (possibly among others)
    APP_PHY_2MBPS_BIT      = (1 << 1),
    APP_PHY_2MBPS_POS      = (1),
    /// The Host prefers to use the LE Coded transmitter/receiver PHY (possibly among others)
    APP_PHY_CODED_BIT      = (1 << 2),
    APP_PHY_CODED_POS      = (2),
    /// The Host prefers to use the LE 4M transmitter/receiver PHY (possibly among others)
#if (mHDT_LE_SUPPORT)
    APP_PHY_4MBPS_BIT      = (1 << 7),
    APP_PHY_4MBPS_POS      = (7),
#endif
    /// The Host prefers to use the LE Coded transmitter/receiver PHY (possibly among others)
    APP_PHY_ALL        = (APP_PHY_1MBPS_BIT | APP_PHY_2MBPS_BIT | APP_PHY_CODED_BIT
#if (mHDT_LE_SUPPORT)
                          | APP_PHY_4MBPS_BIT
#endif
                         ),
};

/// Isochronous Group packing preference
enum app_iso_packing
{
    /// Sequential stream packing
    APP_ISO_PACKING_SEQUENTIAL = 0,
    /// Interleaved stream packing
    APP_ISO_PACKING_INTERLEAVED,

    APP_ISO_PACKING_MAX,
};

/// Isochronous PDU Framing mode
enum app_iso_frame
{
    /// Unframed mode
    APP_ISO_UNFRAMED_MODE = 0,
    /// Framed mode
    APP_ISO_FRAMED_MODE,

    APP_ISO_FRAME_MODE_MAX,
};

/// Context type values
enum app_gaf_bap_capa_context_type
{
    /// Supported Audio Contexts
    APP_GAF_BAP_CAPA_CONTEXT_TYPE_SUPP = 0,
    /// Available Audio Contexts
    APP_GAF_BAP_CAPA_CONTEXT_TYPE_AVA,

    APP_GAF_BAP_CAPA_CONTEXT_TYPE_MAX
};

enum app_gaf_bap_ht_f_data_format
{
    /// Quaternion is supported
    APP_GAF_BAP_HT_TYPE_DATA_QUATERNION = 0x00,
    /// Rotation Vector is supported
    APP_GAF_BAP_HT_TYPE_ROTATION_VECTOR = 0x01,
};

enum app_gaf_bap_ht_frame_interval
{
    /// HT Frame Interval 7.5MS
    APP_GAF_BAP_HT_FRAME_INTV_7_5MS = 0x00,
    /// HT Frame Interval 10MS
    APP_GAF_BAP_HT_FRAME_INTV_10MS = 0x01,
    /// HT Frame Interval 20MS
    APP_GAF_BAP_HT_FRAME_INTV_20MS = 0x02,
};

enum app_gaf_bap_ht_f_data_formats_bit
{
    /// Quaternion is supported
    APP_GAF_BAP_HT_DATA_QUATERNION_BIT = 0x01,
    /// Rotation Vector is supported
    APP_GAF_BAP_HT_ROTATION_VECTOR_BIT = 0x02,
};

enum app_gaf_bap_ht_f_related_flags_bit
{
    /// Timestamp Included. Indicates whether a timestamp is included with the IMU data.
    APP_GAF_BAP_HT_FLAG_TS_INC_BIT = 0x01,
};

enum app_gaf_bap_ht_frame_interval_bit
{
    /// HT Frame Interval 7.5MS
    APP_GAF_BAP_HT_FRAME_7_5MS_BIT = 0x01,
    /// HT Frame Interval 10MS
    APP_GAF_BAP_HT_FRAME_10MS_BIT = 0x02,
    /// HT Frame Interval 20MS
    APP_GAF_BAP_HT_FRAME_20MS_BIT = 0x04,
};

enum app_gaf_bap_aud_cfg_fields_present_bit
{
    /// Preferred Audio Configuration 
    APP_GAF_PREF_AC_DATA_AUDIO_CONFIGURATIONS = 0x01,
    /// Preferred Channel Capability index
    APP_GAF_PREF_AC_DATA_CHAN_CAPA_RECORD_IDX = 0x02,
    /// Preferred Sink PAC Record List
    APP_GAF_PREF_AC_DATA_SINK_PAC_RECORD_LIST = 0x04,
    /// Preferred QoS Setting Sink
    APP_GAF_PREF_AC_DATA_SINK_QOS_SETTING     = 0x08,
    /// Preferred Source PAC Record List
    APP_GAF_PREF_AC_DATA_SRC_PAC_RECORD_LIST  = 0x10,
    /// Preferred QoS Setting Source
    APP_GAF_PREF_AC_DATA_SRC_QOS_SETTING      = 0x20,
    /// Preferred Codec Configuration
    APP_GAF_PREF_AC_DATA_CODEC_CONFIGURATION  = 0x40,
    /// Preferred Presentation Delay Sink
    APP_GAF_PREF_AC_DATA_PRES_DELAY_US_SINK   = 0x80,
    /// Preferred Presentation Delay Source
    APP_GAF_PREF_AC_DATA_PRES_DELAY_US_SRC    = 0x100,

    APP_GAF_PREF_AC_DATA_PRES_MASK            = 0x1FF,
};

enum app_gaf_bap_use_case_id
{
    /// Conversational
    APP_GAF_USE_CASE_ID_CONVERSATIONAL      = 0x0002,
    /// Spatial Conversational
    APP_GAF_USE_CASE_ID_SPA_CONVERSATIONAL  = 0x4002,
    /// Media
    APP_GAF_USE_CASE_ID_MEDIA               = 0x0003,
    /// Media with AV Sync
    APP_GAF_USE_CASE_ID_MEDIA_AV_SYNC       = 0x2003,
    /// Spatial Media
    APP_GAF_USE_CASE_ID_SPA_MEDIA           = 0x4003,
    /// Spatial Media with AV Sync
    APP_GAF_USE_CASE_ID_SPA_MEDIA_AV_SYNC   = 0x6003,
    /// Gaming
    APP_GAF_USE_CASE_ID_GAMING              = 0x0008,
    /// Spatial Gaming
    APP_GAF_USE_CASE_ID_SPA_GAMING          = 0x4008,
    /// Gaming with voice back channel
    APP_GAF_USE_CASE_ID_GAMING_VOICE_BACK   = 0x000A,
    /// Spatial Gaming with Voice back channel
    APP_GAF_USE_CASE_ID_SPA_GAMING_VOICE    = 0x400A,
};

/// Data value in LTV format
typedef struct app_gaf_ltv
{
    /// Length of data value
    uint8_t len;
    /// Data value
    uint8_t data[__ARRAY_EMPTY];
} app_gaf_ltv_t;

/// Structure for #bap_uc_srv_get_quality_cmp_evt_t
typedef struct
{
    ///Cmd return status
    uint16_t status;
    /// ASE local index
    uint8_t ase_lid;
    /// Number of packets transmitted and unacked
    uint32_t tx_unacked_packets;
    /// Number of flushed transmitted packets
    uint32_t tx_flushed_packets;
    /// Number of packets transmitted during last subevent
    uint32_t tx_last_subevent_packets;
    /// Number of retransmitted packets
    uint32_t retx_packets;
    /// Number of packets received with a CRC error
    uint32_t crc_error_packets;
    /// Number of unreceived packets
    uint32_t rx_unrx_packets;
    /// Number of duplicate packets received
    uint32_t duplicate_packets;
} __attribute__((__packed__)) app_gaf_iso_quality_rpt_evt_t;

typedef struct app_gaf_iso_tx_sync_dbg_info
{
    /// 0x00 - The HCI_LE_Read_ISO_TX_Sync command succeeded ; 0x01-0xFF Failed reason
    uint8_t  status;
    /// Connection handle of the CIS or BIS (Range: 0x0000-0x0EFF)
    uint16_t con_hdl;
    ///packet sequence number
    uint16_t packet_seq_num;
    ///The CIG reference point or BIG anchor point of a transmitted SDU
    ///derived using the Controller's free running reference clock (in microseconds).
    uint32_t tx_time_stamp;
    ///The time offset, in microseconds, that is associated with a transmitted SDU.
    uint32_t time_offset;
} __attribute__((__packed__)) app_gaf_iso_tx_sync_dbg_info_t;

/// Broadcast Group Parameters structure
typedef struct app_gaf_bap_bc_grp_param
{
    /// SDU interval in microseconds
    /// From 256us (0x00000100) to 1.048575s (0x000FFFFF)
    uint32_t sdu_intv_us;
    /// Maximum size of an SDU
    /// From 1 to 4095 bytes
    uint16_t max_sdu;
    /// Sequential or Interleaved scheduling (see TODO [LT])
    uint8_t packing;
    /// Unframed or framed mode (see TODO [LT])
    uint8_t framing;
    /// Bitfield indicating PHYs that can be used by the controller for transmission of SDUs (see TODO [LT])
    uint8_t phy_bf;
    // creat test big hci cmd, 0:creat BIG cmd, 1:creat BIG test cmd
    uint8_t test;

    //// test = 0, set this param
    /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
    /// of the same SDU
    /// From 0ms to 4.095s (0x0FFF)
    uint16_t max_tlatency_ms;
    /// Number of times every PDU should be transmitted
    /// From 0 to 15
    uint8_t rtn;

    //// test = 1, set this param
    /// ISO interval in multiple of 1.25ms. From 0x4 (5ms) to 0xC80 (4s)
    uint16_t iso_intv_frame;
    /// Number of subevents in each interval of each stream in the group
    uint8_t  nse;
    /// Maximum size of a PDU
    uint8_t  max_pdu;
    /// Burst number (number of new payload in each interval). From 1 to 7
    uint8_t  bn;
    /// Number of times the scheduled payload is transmitted in a given event. From 0x1 to 0xF
    uint8_t  irc;
    /// Isochronous Interval spacing of payloads transmitted in the pre-transmission subevents.
    /// From 0x00 to 0x0F
    uint8_t  pto;

} app_gaf_bap_bc_grp_param_t;

/// Advertising Parameters structure
typedef struct app_gaf_bap_bc_adv_param
{
    /// Minimum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_min_slot;
    /// Maximum advertising interval in multiple of 0.625ms
    /// From 20ms (0x00000020) to 10485.759375s (0x00FFFFFF)
    uint32_t adv_intv_max_slot;
    /// Channel Map (@see TODO [LT])
    uint8_t chnl_map;
    /// PHY for primary advertising (see #gap_phy_val enumeration)
    /// Only LE 1M and LE Codec PHYs are allowed
    uint8_t phy_prim;
    /// PHY for secondary advertising (see #gap_phy_val enumeration)
    uint8_t phy_second;
    /// Advertising SID
    /// From 0x00 to 0x0F
    uint8_t adv_sid;
} app_gaf_bap_bc_adv_param_t;

/// Periodic Advertising Parameters structure
typedef struct app_gaf_bap_bc_per_adv_param
{
    /// Minimum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_min_frame;
    /// Maximum Periodic Advertising interval in multiple of 1.25ms
    /// Must be higher than 7.5ms (0x0006)
    uint16_t adv_intv_max_frame;
} app_gaf_bap_bc_per_adv_param_t;

/// Codec Capabilities parameters structure
typedef struct app_gaf_bap_capa_param
{
    /// Supported Sampling Frequencies bit field (see #bap_sampling_freq_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint16_t sampling_freq_bf;
    /// Supported Frame Durations bit field (see #bap_freq_dur_bf enumeration)\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// Mandatory for LC3
    uint8_t frame_dur_bf;
    /// Supported Audio Channel Counts\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 channel supported (forced to 0x01
    /// on reception side)
    uint8_t chan_cnt_bf;
    /// Supported Octets Per Codec Frame - Minimum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_max also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_min;
    /// Supported Octets Per Codec Frame - Maximum\n
    /// Not part of the Codec Specific Capabilities is equal to 0 and frame_octet_min also equal to 0\n
    /// Mandatory for LC3
    uint16_t frame_octet_max;
    /// Supported Maximum Codec Frames Per SDU\n
    /// 0 means that the field is not part of the Codec Specific Capabilities\n
    /// For LC3, absence in the Codec Specific Capabilities is equivalent to 1 Frame Per SDU (forced to 1 on
    /// reception side)
    uint8_t max_frames_sdu;
} app_gaf_bap_capa_param_t;

/// Codec Capabilities structure
typedef struct app_gaf_bap_capa
{
    /// Parameters structure
    app_gaf_bap_capa_param_t param;
    /// Additional Codec Capabilities (in LTV format)
    app_gaf_ltv_t add_capa;
} app_gaf_bap_capa_t;

/// Codec Capabilities Metadata parameters structure
typedef struct app_gaf_bap_capa_metadata_param
{
    /// Preferred Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t context_bf;
} app_gaf_bap_capa_metadata_param_t;

/// Codec Capabilities Metadata structure
typedef struct app_gaf_bap_capa_metadata
{
    /// Parameters structure
    app_gaf_bap_capa_metadata_param_t param;
    /// Additional Metadata (in LTV format)
    app_gaf_ltv_t add_metadata;
} app_gaf_bap_capa_metadata_t;

typedef struct app_gaf_bap_chan_capa_supp_index_list_packed
{
    uint8_t len;
    uint8_t type;
    uint8_t supp_chan_capa_idx[__ARRAY_EMPTY];
} __attribute__((packed)) app_gaf_bap_chan_capa_supp_ltv_t;

typedef struct app_gaf_bap_ht_frame_supp_formats_packed
{
    uint8_t len;
    uint8_t type;
    /// Octet 0: bitfield of supported data formats
    /// Octet 1: bitfield of related flags
    /// @see app_gaf_bap_ht_f_data_formats_bit
    uint8_t supp_data_formats;
    /// @see app_gaf_bap_ht_f_related_flags_bit
    uint8_t related_flags;
} __attribute__((packed)) app_gaf_bap_ht_supp_fmt_ltv_t;

typedef struct app_gaf_bap_ht_frame_supp_interval_packed
{
    uint8_t len;
    uint8_t type;
    /// IMU supported frame rates, dependent on IMU model
    /// @see app_gaf_bap_ht_frame_interval_bit
    uint8_t intv_bf;
} __attribute__((packed)) app_gaf_bap_ht_supp_intv_ltv_t;

typedef struct app_gaf_bap_vendor_specific_cfg
{
    uint8_t length;
    uint8_t type;
    uint16_t company_id;
    uint8_t s2m_encode_channel;
    uint8_t s2m_decode_channel;
} app_gaf_bap_vendor_specific_cfg_t;

/// Codec Configuration parameters structure
typedef struct app_gaf_bap_cfg_param
{
    /// Audio Locations of the Audio Channels being configured for the codec (i.e the number of codec frames per
    /// block) and their ordering within a single block of codec frames
    /// When transmitted, part of Codec Specific Configuration only if not equal to 0
    /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
    uint32_t location_bf;
    /// Length of a codec frame in octets
    uint16_t frame_octet;
    /// Sampling Frequency (see #bap_sampling_freq enumeration)
    uint8_t sampling_freq;
    /// Frame Duration (see #bap_frame_dur enumeration)
    uint8_t frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t frames_sdu;
} app_gaf_bap_cfg_param_t;

/// Codec Configuration structure
typedef struct app_gaf_bap_cfg
{
    /// Parameters structure
    app_gaf_bap_cfg_param_t param;
    /// Additional Codec Configuration (in LTV format)
    app_gaf_ltv_t add_cfg;
} app_gaf_bap_cfg_t;

/// Codec Configuration Metadata parameters structure
typedef struct app_gaf_bap_cfg_metadata_param
{
    /// Streaming Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t context_bf;
} app_gaf_bap_cfg_metadata_param_t;

/// Codec Configuration Metadata structure
typedef struct app_gaf_bap_cfg_metadata
{
    /// Parameters structure
    app_gaf_bap_cfg_metadata_param_t param;
    /// Additional Metadata value (in LTV format)
    app_gaf_ltv_t add_metadata;
} app_gaf_bap_cfg_metadata_t;

typedef struct app_gaf_bap_ht_frame_cfg_format_packed
{
    uint8_t len;
    uint8_t type;
    /// Octet 0: bitfield of supported data formats
    /// Octet 1: bitfield of related flags
    /// @see app_gaf_bap_ht_f_data_format
    uint8_t data_format;
    /// @see app_gaf_bap_ht_f_related_flags_bit
    uint8_t related_flags;
} __attribute__((packed)) app_gaf_bap_ht_cfg_fmt_ltv_t;

typedef struct app_gaf_bap_ht_frame_cfg_interval_packed
{
    uint8_t len;
    uint8_t type;
    /// IMU supported frame rates, dependent on IMU model
    /// @see app_gaf_bap_ht_frame_interval
    uint8_t intv;
} __attribute__((packed)) app_gaf_bap_ht_cfg_intv_ltv_t;

typedef struct app_gaf__pref_aud_cfg_data_aud_cfg_uc_bc
{
    uint8_t uc_aud_cfg[13];
    uint8_t bc_aud_cfg[4];
} app_gaf_pref_aud_cfg_aud_cfg_uc_bc_t;

typedef union app_gaf_pref_aud_cfg_data_aud_cfg
{
    uint8_t aud_cfg[17];
    app_gaf_pref_aud_cfg_aud_cfg_uc_bc_t data;
} app_gaf_pref_ac_data_aud_cfg_u;

typedef struct app_gaf_pref_aud_cfg_data_chan_capa
{
    uint8_t chan_capa_idx;
} app_gaf_pref_ac_data_chan_capa_idx_t;

typedef struct app_gaf_pref_aud_cfg_data_pac_record
{
    uint8_t pref_pac_num;
    struct pac_info
    {
        uint8_t pac_set_id;
        uint8_t pac_idx;
    } pac_info[__ARRAY_EMPTY];
} app_gaf_pref_ac_data_pac_rec_t;

typedef struct app_gaf_pref_aud_cfg_data_qos_setting
{
    uint32_t sdu_interval_us;
    uint8_t framing_type;
    uint16_t max_sdu_size;
    uint8_t rtn;
    uint16_t trans_latency_ms;
} app_gaf_pref_ac_data_qos_setting_t;

typedef struct app_gaf_pref_aud_cfg_data_codec_cfg
{
    uint8_t codec_id[5];
    app_gaf_bap_cfg_t codec_cfg;
} app_gaf_pref_ac_data_specific_cc_t;

typedef struct app_gaf_pref_aud_cfg_data_pres_delay
{
    uint32_t pres_delay_us;
} app_gaf_pref_ac_data_pres_delay_t;

typedef struct app_gaf_pref_aud_cfg_field
{
    app_gaf_pref_ac_data_aud_cfg_u *pref_aud_cfg;
    app_gaf_pref_ac_data_chan_capa_idx_t *pref_chan_capa;
    app_gaf_pref_ac_data_pac_rec_t *pref_pac_sink;
    app_gaf_pref_ac_data_qos_setting_t *pref_qos_setting_sink;
    app_gaf_pref_ac_data_pac_rec_t *pref_pac_src;
    app_gaf_pref_ac_data_qos_setting_t *pref_qos_setting_src;
    app_gaf_pref_ac_data_specific_cc_t *pref_codec_cfg;
    app_gaf_pref_ac_data_pres_delay_t *pref_pres_delay_sink;
    app_gaf_pref_ac_data_pres_delay_t *pref_pres_delay_src;
} app_gaf_pref_aud_cfg_data_t;

/// Supported Audio Location Bitfield
enum gaf_bap_supported_locations_bf
{
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT               = 0x00000001,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT              = 0x00000002,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_CENTER             = 0x00000004,
    APP_GAF_BAP_AUDIO_LOCATION_LOW_FREQ_EFFECTS_1       = 0x00000008,
    APP_GAF_BAP_AUDIO_LOCATION_BACK_LEFT                = 0x00000010,
    APP_GAF_BAP_AUDIO_LOCATION_BACK_RIGHT               = 0x00000020,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT_OF_CENTER     = 0x00000040,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT_OF_CENTER    = 0x00000080,
    APP_GAF_BAP_AUDIO_LOCATION_BACK_CENTER              = 0x00000100,
    APP_GAF_BAP_AUDIO_LOCATION_LOW_FREQ_EFFECTS_2       = 0x00000200,
    APP_GAF_BAP_AUDIO_LOCATION_SIDE_LEFT                = 0x00000400,
    APP_GAF_BAP_AUDIO_LOCATION_SIDE_RIGHT               = 0x00000800,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_FRONT_LEFT           = 0x00001000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_FRONT_RIGHT          = 0x00002000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_FRONT_CENTER         = 0x00004000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_CENTER               = 0x00008000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_BACK_LEFT            = 0x00010000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_BACK_RIGHT           = 0x00020000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_SIDE_LEFT            = 0x00040000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_SIDE_RIGHT           = 0x00080000,
    APP_GAF_BAP_AUDIO_LOCATION_TOP_BACK_CENTER          = 0x00100000,
    APP_GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_CENTER      = 0x00200000,
    APP_GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_LEFT        = 0x00400000,
    APP_GAF_BAP_AUDIO_LOCATION_BOTTOM_FRONT_RIGHT       = 0x00800000,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_LEFT_WIDE          = 0x01000000,
    APP_GAF_BAP_AUDIO_LOCATION_FRONT_RIGHT_WIDE         = 0x02000000,
    APP_GAF_BAP_AUDIO_LOCATION_LEFT_SURROUND            = 0x04000000,
    APP_GAF_BAP_AUDIO_LOCATION_RIGHT_SURROUND           = 0x08000000,

    APP_GAF_BAP_AUDIO_LOCATION_RFU                      = 0xF0000000,
};

/// Context type bit field meaning
typedef enum gaf_bap_context_type_bf
{
    APP_GAF_BAP_CONTEXT_TYPE_UNSPECIFIED            = 0x0001,
    /// Conversation between humans as, for example, in telephony or video calls
    APP_GAF_BAP_CONTEXT_TYPE_CONVERSATIONAL         = 0x0002,
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    APP_GAF_BAP_CONTEXT_TYPE_MEDIA                  = 0x0004,
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    /// between participants; or a mix of all the above
    APP_GAF_BAP_CONTEXT_TYPE_GAME                   = 0x0008,
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    APP_GAF_BAP_CONTEXT_TYPE_INSTRUCTIONAL          = 0x0010,
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    APP_GAF_BAP_CONTEXT_TYPE_MAN_MACHINE            = 0x0020,
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    APP_GAF_BAP_CONTEXT_TYPE_LIVE                   = 0x0040,
    /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    APP_GAF_BAP_CONTEXT_TYPE_SOUND_EFFECT           = 0x0080,
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    APP_GAF_BAP_CONTEXT_TYPE_ATTENTION_SEEKING      = 0x0100,
    /// Ringtone as in a call alert
    APP_GAF_BAP_CONTEXT_TYPE_RINGTONE               = 0x0200,
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    APP_GAF_BAP_CONTEXT_TYPE_IMMEDIATE_ALERT        = 0x0400,
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    APP_GAF_BAP_CONTEXT_TYPE_EMERGENCY_ALERT        = 0x0800,
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    APP_GAF_BAP_CONTEXT_TYPE_TV                     = 0x1000,
} app_gaf_bap_context_type_bf_t;

/// Target Latency values
enum app_gaf_bap_uc_tgt_latency
{
    APP_GAF_BAP_UC_TGT_LATENCY_MIN = 1,

    /// Target lower latency
    APP_GAF_BAP_UC_TGT_LATENCY_LOWER = APP_GAF_BAP_UC_TGT_LATENCY_MIN,
    /// Target balanced latency and reliability
    APP_GAF_BAP_UC_TGT_LATENCY_BALENCED,
    /// Target higher reliability
    APP_GAF_BAP_UC_TGT_LATENCY_RELIABLE,

    APP_GAF_BAP_UC_TGT_LATENCY_MAX,
};

/// Audio Locations bit field meaning
enum app_gaf_loc_bf
{
    /// Front Left
    APP_GAF_LOC_FRONT_LEFT_POS = 0,
    APP_GAF_LOC_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_POS),
    /// Front Right
    APP_GAF_LOC_FRONT_RIGHT_POS = 1,
    APP_GAF_LOC_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_POS),
    /// Front Center
    APP_GAF_LOC_FRONT_CENTER_POS = 2,
    APP_GAF_LOC_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_CENTER_POS),
    /// Low Frequency Effect 1
    APP_GAF_LOC_LFE1_POS = 3,
    APP_GAF_LOC_LFE1_BIT = APP_CO_BIT(APP_GAF_LOC_LFE1_POS),
    /// Back Left
    APP_GAF_LOC_BACK_LEFT_POS = 4,
    APP_GAF_LOC_BACK_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_LEFT_POS),
    /// Back Right
    APP_GAF_LOC_BACK_RIGHT_POS = 5,
    APP_GAF_LOC_BACK_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_RIGHT_POS),
    /// Front Left Center
    APP_GAF_LOC_FRONT_LEFT_CENTER_POS = 6,
    APP_GAF_LOC_FRONT_LEFT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_CENTER_POS),
    /// Front Right Center
    APP_GAF_LOC_FRONT_RIGHT_CENTER_POS = 7,
    APP_GAF_LOC_FRONT_RIGHT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_CENTER_POS),
    /// Back Center
    APP_GAF_LOC_BACK_CENTER_POS = 8,
    APP_GAF_LOC_BACK_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_BACK_CENTER_POS),
    /// Low Frequency Effect 2
    APP_GAF_LOC_LFE2_POS = 9,
    APP_GAF_LOC_LFE2_BIT = APP_CO_BIT(APP_GAF_LOC_LFE2_POS),
    /// Side Left
    APP_GAF_LOC_SIDE_LEFT_POS = 10,
    APP_GAF_LOC_SIDE_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_SIDE_LEFT_POS),
    /// Side Right
    APP_GAF_LOC_SIDE_RIGHT_POS = 11,
    APP_GAF_LOC_SIDE_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_SIDE_RIGHT_POS),
    /// Top Front Left
    APP_GAF_LOC_TOP_FRONT_LEFT_POS = 12,
    APP_GAF_LOC_TOP_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_LEFT_POS),
    /// Top Front Right
    APP_GAF_LOC_TOP_FRONT_RIGHT_POS = 13,
    APP_GAF_LOC_TOP_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_RIGHT_POS),
    /// Top Front Center
    APP_GAF_LOC_TOP_FRONT_CENTER_POS = 14,
    APP_GAF_LOC_TOP_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_FRONT_CENTER_POS),
    /// Top Center
    APP_GAF_LOC_TOP_CENTER_POS = 15,
    APP_GAF_LOC_TOP_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_CENTER_POS),
    /// Top Back Left
    APP_GAF_LOC_TOP_BACK_LEFT_POS = 16,
    APP_GAF_LOC_TOP_BACK_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_LEFT_POS),
    /// Top Back Right
    APP_GAF_LOC_TOP_BACK_RIGHT_POS = 17,
    APP_GAF_LOC_TOP_BACK_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_RIGHT_POS),
    /// Top Side Left
    APP_GAF_LOC_TOP_SIDE_LEFT_POS = 18,
    APP_GAF_LOC_TOP_SIDE_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_SIDE_LEFT_POS),
    /// Top Side Right
    APP_GAF_LOC_TOP_SIDE_RIGHT_POS = 19,
    APP_GAF_LOC_TOP_SIDE_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_SIDE_RIGHT_POS),
    /// Top Back Center
    APP_GAF_LOC_TOP_BACK_CENTER_POS = 20,
    APP_GAF_LOC_TOP_BACK_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_TOP_BACK_CENTER_POS),
    /// Bottom Front Center
    APP_GAF_LOC_BOTTOM_FRONT_CENTER_POS = 21,
    APP_GAF_LOC_BOTTOM_FRONT_CENTER_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_CENTER_POS),
    /// Bottom Front Left
    APP_GAF_LOC_BOTTOM_FRONT_LEFT_POS = 22,
    APP_GAF_LOC_BOTTOM_FRONT_LEFT_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_LEFT_POS),
    /// Bottom Front Right
    APP_GAF_LOC_BOTTOM_FRONT_RIGHT_POS = 23,
    APP_GAF_LOC_BOTTOM_FRONT_RIGHT_BIT = APP_CO_BIT(APP_GAF_LOC_BOTTOM_FRONT_RIGHT_POS),
    /// Front Left Wide
    APP_GAF_LOC_FRONT_LEFT_WIDE_POS = 24,
    APP_GAF_LOC_FRONT_LEFT_WIDE_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_LEFT_WIDE_POS),
    /// Front Right Wide
    APP_GAF_LOC_FRONT_RIGHT_WIDE_POS = 25,
    APP_GAF_LOC_FRONT_RIGHT_WIDE_BIT = APP_CO_BIT(APP_GAF_LOC_FRONT_RIGHT_WIDE_POS),
    /// Left Surround
    APP_GAF_LOC_LEFT_SURROUND_POS = 26,
    APP_GAF_LOC_LEFT_SURROUND_BIT = APP_CO_BIT(APP_GAF_LOC_LEFT_SURROUND_POS),
    /// Right Surround
    APP_GAF_LOC_RIGHT_SURROUND_POS = 27,
    APP_GAF_LOC_RIGHT_SURROUND_BIT = APP_CO_BIT(APP_GAF_LOC_RIGHT_SURROUND_POS),
};

/// Volume Operation Code values
/// @see enum arc_vc_opcode
enum gaf_arc_vc_opcode
{
    /// Relative Volume Down
    GAF_ARC_VC_OPCODE_VOL_DOWN = 0,
    /// Relative Volume Up
    GAF_ARC_VC_OPCODE_VOL_UP,
    /// Unmute/Relative Volume Down
    GAF_ARC_VC_OPCODE_VOL_DOWN_UNMUTE,
    /// Unmute/Relative Volume Up
    GAF_ARC_VC_OPCODE_VOL_UP_UNMUTE,
    /// Set Absolute Volume
    GAF_ARC_VC_OPCODE_VOL_SET_ABS,
    /// Unmute
    GAF_ARC_VC_OPCODE_VOL_UNMUTE,
    /// Mute
    GAF_ARC_VC_OPCODE_VOL_MUTE,

    GAF_ARC_VC_OPCODE_MAX
};

/// Operation Code values for Audio Input Control Point characteristic
enum gaf_arc_aic_opcode
{
    /// Set Gain Setting
    GAF_ARC_AIC_OPCODE_SET_GAIN = 1,
    /// Unmute
    GAF_ARC_AIC_OPCODE_UNMUTE,
    /// Mute
    GAF_ARC_AIC_OPCODE_MUTE,
    /// Set Manual Gain Mode
    GAF_ARC_AIC_OPCODE_SET_MANUAL_MODE,
    /// Set Automatic Gain Mode
    GAF_ARC_AIC_OPCODE_SET_AUTO_MODE,
};

/// Set type values
enum gaf_arc_voc_set_type
{
    /// Volume offset
    GAF_ARC_VOC_SET_TYPE_OFFSET = 0,
    /// Audio location
    GAF_ARC_VOC_SET_TYPE_LOCATION,

    GAF_ARC_VOC_SET_TYPE_MAX
};

/// TMAP Role characteristic bit field meaning
enum gaf_tmap_role_bf
{
    /// Indicate if Server supports Call Gateway role (= 1) or not - Bit
    GAF_TMAP_ROLE_CG_BIT = CO_BIT(0),
    /// Indicate if Server supports Call Terminal role (= 1) or not - Bit
    GAF_TMAP_ROLE_CT_BIT = CO_BIT(1),
    /// Indicate if Server supports Unicast Media Sender role (= 1) or not - Bit
    GAF_TMAP_ROLE_UMS_BIT = CO_BIT(2),
    /// Indicate if Server supports Unicast Media Receiver role (= 1) or not - Bit
    GAF_TMAP_ROLE_UMR_BIT = CO_BIT(3),
    /// Indicate if Server supports Broadcast Media Sender role (= 1) or not - Bit
    GAF_TMAP_ROLE_BMS_BIT = CO_BIT(4),
    /// Indicate if Server supports Broadcast Media Receiver role (= 1) or not - Bit
    GAF_TMAP_ROLE_BMR_BIT = CO_BIT(5),
    /// Mask indicating that all roles are supported
    GAF_TMAP_ROLE_ALLSUPP_MASK = 0x003F,
    /// Mask indicating RFU bits
    GAF_TMAP_ROLE_RFU_MASK = 0xFFC0,
};

/// Characteristic type values for Gaming Audio Service
enum gaf_gmap_char_type
{
    /// GAF_GMAP Role characteristic
    GAF_GMAP_CHAR_TYPE_ROLE = 0,
    /// GAF_GMAP UGG feature characteristic
    GAF_GMAP_CHAR_TYPE_UGG_FEAT,
    /// GAF_GMAP UGT feature characteristic
    GAF_GMAP_CHAR_TYPE_UGT_FEAT,
    /// GAF_GMAP BGS feature characteristic
    GAF_GMAP_CHAR_TYPE_BGS_FEAT,
    /// GAF_GMAP BGR feature characteristic
    GAF_GMAP_CHAR_TYPE_BGR_FEAT,

    GAF_GMAP_CHAR_TYPE_MAX,
};

/// GAF_GMAP Role characteristic bit field meaning
enum gaf_gmap_role_bf
{
    /// Indicate if Server supports Unicast Game Gateway (UGG) role (= 1) or not - Bit
    GAF_GMAP_ROLE_UGG_BIT = CO_BIT(0),
    /// Indicate if Server supports Unicast Game Terminal (UGT) role (= 1) or not - Bit
    GAF_GMAP_ROLE_UGT_BIT = CO_BIT(1),
    /// Indicate if Server supports Broadcast Game Sender (BGS) role (= 1) or not - Bit
    GAF_GMAP_ROLE_BGS_BIT = CO_BIT(2),
    /// Indicate if Server supports Broadcast Game Receiver (BGR) role (= 1) or not - Bit
    GAF_GMAP_ROLE_BGR_BIT = CO_BIT(3),
    /// Mask indicating that all roles are supported
    GAF_GMAP_ROLE_ALLSUPP_MASK = 0x0F,
    /// Mask indicating RFU bits
    GAF_GMAP_ROLE_RFU_MASK = 0xF0,
};

/// GAF_GMAP UGG features bit field meaning
enum gaf_gmap_ugg_feat_bf
{
    /// Indicate if UGG Multiplex feature support (= 1) or not - Bit
    GAF_GMAP_UGG_FEAT_MULTIPLEX_BIT = CO_BIT(0),
    /// Indicate if UGG 96 kbps Source feature support (= 1) or not - Bit
    GAF_GMAP_UGG_FEAT_96KBPS_SRC_BIT = CO_BIT(1),
    /// Indicate if UGG Multisink feature support role (= 1) or not - Bit
    GAF_GMAP_UGG_FEAT_MULTISINK_BIT = CO_BIT(2),
    /// Mask indicating that all features are supported
    GAF_GMAP_UGG_FEAT_ALLSUPP_MASK = 0x07,
    /// Mask indicating RFU bits
    GAF_GMAP_UGG_FEAT_RFU_MASK = 0xF8,
};

/// GAF_GMAP UGT features bit field meaning
enum gaf_gmap_ugt_feat_bf
{
    /// Indicate if UGT Source feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_SOURCE_BIT = CO_BIT(0),
    /// Indicate if UGT 80 kbps Source feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_80KBPS_SRC_BIT = CO_BIT(1),
    /// Indicate if UGT Sink feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_SINK_BIT = CO_BIT(2),
    /// Indicate if UGT 64 kbps Sink feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_64KBPS_SINK_BIT = CO_BIT(3),
    /// Indicate if UGT Multiplex feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_MULTIPLEX_BIT = CO_BIT(4),
    /// Indicate if UGT Multisink feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_MULTISINK_BIT = CO_BIT(5),
    /// Indicate if UGT Multisource feature support (= 1) or not - Bit
    GAF_GMAP_UGT_FEAT_MULTISOURCE_BIT = CO_BIT(6),
    /// Mask indicating that all features are supported
    GAF_GMAP_UGT_FEAT_ALLSUPP_MASK = 0x7F,
    /// Mask indicating RFU bits
    GAF_GMAP_UGT_FEAT_RFU_MASK = 0x80,
};

/// GAF_GMAP BGS features bit field meaning
enum gaf_gmap_bgs_feat_bf
{
    /// Indicate if BGS 96 kbps feature support (= 1) or not - Bit
    GAF_GMAP_BGS_FEAT_96KBPS_BIT = CO_BIT(0),
    /// Mask indicating that all features are supported
    GAF_GMAP_BGS_FEAT_ALLSUPP_MASK = 0x01,
    /// Mask indicating RFU bits
    GAF_GMAP_BGS_FEAT_RFU_MASK = 0xFE,
};

/// GAF_GMAP BGR features bit field meaning
enum gaf_gmap_bgr_feat_bf
{
    /// Indicate if BGR Multisink feature support (= 1) or not - Bit
    GAF_GMAP_BGR_FEAT_MULTISINK_BIT = CO_BIT(0),
    /// Indicate if BGR Multiplex feature support (= 1) or not - Bit
    GAF_GMAP_BGR_FEAT_MULTIPLEX_BIT = CO_BIT(1),
    /// Mask indicating that all features are supported
    GAF_GMAP_BGR_FEAT_ALLSUPP_MASK = 0x03,
    /// Mask indicating RFU bits
    GAF_GMAP_BGR_FEAT_RFU_MASK = 0xFC,
};

/// Service information structure
typedef struct
{
    /// start handle
    uint16_t shdl;
    /// end handle
    uint16_t ehdl;
} app_gaf_prf_svc_t;

/// Characteristic information structure
typedef struct app_gaf_prf_char
{
    /// Value handle
    uint16_t val_hdl;
    /// Characteristic properties
    uint8_t prop;
} app_gaf_prf_char_t;

/// Descriptor information structure
typedef struct app_gaf_prf_desc
{
    /// Descriptor handle
    uint16_t desc_hdl;
} app_gaf_prf_desc_t;

/// Unicast group configuration structure (provided by controller after stream establisment)
typedef struct
{
    /// Group synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from master to slave
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_m2s_us;
    /// The maximum time, in microseconds, for transmission of SDUs of all CISes from slave to master
    /// (range 0x0000EA to 0x7FFFFF)
    uint32_t tlatency_s2m_us;
    /// ISO interval (1.25ms unit, range: 5ms to 4s)
    uint16_t iso_intv_frames;
} app_gaf_iap_ug_config_t;

/// Unicast stream configuration structure (provided by controller after stream establishment)
typedef struct
{
    /// Stream synchronization delay time in microseconds
    uint32_t sync_delay_us;
    /// Maximum size, in octets, of the payload from master to slave (Range: 0x00-0xFB)
    uint16_t max_pdu_m2s;
    /// Maximum size, in octets, of the payload from slave to master (Range: 0x00-0xFB)
    uint16_t max_pdu_s2m;
    /// Master to slave PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_m2s;
    /// Slave to master PHY, bit 0: 1Mbps, bit 1: 2Mbps, bit 2: LE-Coded
    uint8_t phy_s2m;
    /// The burst number for master to slave transmission (0x00: no isochronous data from the master to the slave, range 0x01-0x0F)
    uint8_t bn_m2s;
    /// The burst number for slave to master transmission (0x00: no isochronous data from the slave to the master, range 0x01-0x0F)
    uint8_t bn_s2m;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the master to the slave (Range: 0x01-0x1F)
    uint8_t ft_m2s;
    /// The flush timeout, in multiples of the ISO_Interval, for each payload sent from the slave to the master (Range: 0x01-0x1F)
    uint8_t ft_s2m;
    /// Maximum number of subevents in each isochronous interval. From 0x1 to 0x1F
    uint8_t nse;
} app_gaf_iap_us_config_t;

/// Structure for BAP_UC_SRV_CIS_STATE indication message
typedef struct
{
    /// Indication code (@see enum bap_uc_srv_ind_code)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated by the controller
    /// GAP_INVALID_CONHDL means that the CIS is not established
    uint16_t conhdl;
    /// Group configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_ug_config_t cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_us_config_t cis_config;
    /// status
    uint8_t status;
    /// reason
    uint8_t reason;
} app_gaf_uc_srv_cis_state_ind_t;

/// List of Audio Stream Control Service characteristics
enum app_gaf_bap_uc_char_type
{
    /// ASE Control Point characteristic
    APP_GAF_BAP_UC_CHAR_TYPE_CP = 0,
    /// Sink/Source ASE characteristic
    APP_GAF_BAP_UC_CHAR_TYPE_ASE,

    APP_GAF_BAP_UC_CHAR_TYPE_MAX,
};

/// ASE State values
enum app_gaf_bap_uc_ase_state
{
    /// Idle
    APP_GAF_BAP_UC_ASE_STATE_IDLE = 0,
    /// Codec configured
    APP_GAF_BAP_UC_ASE_STATE_CODEC_CONFIGURED,
    /// QoS configured
    APP_GAF_BAP_UC_ASE_STATE_QOS_CONFIGURED,
    /// Enabling
    APP_GAF_BAP_UC_ASE_STATE_ENABLING,
    /// Streaming
    APP_GAF_BAP_UC_ASE_STATE_STREAMING,
    /// Disabling
    APP_GAF_BAP_UC_ASE_STATE_DISABLING,
    /// Releasing
    APP_GAF_BAP_UC_ASE_STATE_RELEASING,

    APP_GAF_BAP_UC_ASE_STATE_MAX,
};

/// Reason values for ASE Control Point characteristic
enum
{
    APP_GAF_BAP_UC_CP_REASON_MIN = 1,
    /// Codec ID
    APP_GAF_BAP_UC_CP_REASON_CODEC_ID = APP_GAF_BAP_UC_CP_REASON_MIN,
    /// Codec Specific Configuration
    APP_GAF_BAP_UC_CP_REASON_CODEC_SPEC_CFG,
    /// SDU Interval
    APP_GAF_BAP_UC_CP_REASON_SDU_INTERVAL,
    /// Framing
    APP_GAF_BAP_UC_CP_REASON_FRAMING,
    /// PHY
    APP_GAF_BAP_UC_CP_REASON_PHY,
    /// Maximum SDU Size
    APP_GAF_BAP_UC_CP_REASON_MAX_SDU_SIZE,
    /// Retransmission Number
    APP_GAF_BAP_UC_CP_REASON_RETX_NB,
    /// Maximum Transport Latency
    APP_GAF_BAP_UC_CP_REASON_MAX_TRANS_LATENCY,
    /// Presentation Delay
    APP_GAF_BAP_UC_CP_REASON_PRES_DELAY,
    /// Invalid ASE CIS Mapping
    APP_GAF_BAP_UC_CP_REASON_INVALID_ASE_CIS_MAPPING,
    /// APP layer rejected
    APP_GAF_BAP_UC_CP_REASON_APP_REJECTED,
    /// OPCODE not match
    APP_GAF_BAP_UC_CP_REASON_OPCODE_NOT_MATCH,
    /// Parameter check failed
    APP_GAF_BAP_UC_CP_REASON_PARAM_ERR,
    /// ASE lid not match
    APP_GAF_BAP_UC_CP_REASON_ASE_LID_NOT_MATCH,
    /// ASE lid invalid
    APP_GAF_BAP_UC_CP_REASON_ASE_LID_INTVALID,
    /// Additional info error
    APP_GAF_BAP_UC_CP_REASON_ADD_INFO_ERR,
    /// ASE ENV invalid
    APP_GAF_BAP_UC_CP_REASON_ASE_ENV_INVALID,

    APP_GAF_BAP_UC_CP_REASON_MAX,
};

enum app_gaf_bap_uc_opcode
{
    /// Configure Codec
    APP_GAF_BAP_UC_OPCODE_CFG_CODEC = 1,
    APP_GAF_BAP_UC_OPCODE_MIN = APP_GAF_BAP_UC_OPCODE_CFG_CODEC,
    /// Configure QoS
    APP_GAF_BAP_UC_OPCODE_CFG_QOS,
    /// Enable
    APP_GAF_BAP_UC_OPCODE_ENABLE,
    /// Receiver Start Ready
    APP_GAF_BAP_UC_OPCODE_RX_START_READY,
    /// Disable
    APP_GAF_BAP_UC_OPCODE_DISABLE,
    /// Receiver Stop Ready
    APP_GAF_BAP_UC_OPCODE_RX_STOP_READY,
    /// Update Metadata
    APP_GAF_BAP_UC_OPCODE_UPDATE_METADATA,
    /// Release
    APP_GAF_BAP_UC_OPCODE_RELEASE,

    APP_GAF_BAP_UC_OPCODE_MAX,
};

/// Structure for cis stream started indication message
typedef struct
{
    uint8_t ase_lid;
    uint16_t con_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascs_cis_stream_started_t;

/// Structure for cis stream stopped indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascs_cis_stream_stopped_t;

/// Codec Identifier
struct app_gaf_codec_id
{
    /// Codec ID value
    uint8_t codec_id[APP_GAF_CODEC_ID_LEN];
};

/// Data path configuration structure
typedef struct app_gaf_bap_uc_dp_cfg
{
    /// Datapath ID
    uint8_t dp_id;
    /// Controller Delay in microseconds
    uint32_t ctl_delay_us;
} app_gaf_bap_dp_cfg_t;

/// QoS Requirement structure
typedef struct app_gaf_bap_qos_req
{
    /// Presentation Delay minimum microseconds
    uint32_t pres_delay_min_us;
    /// Presentation Delay maximum in microseconds
    uint32_t pres_delay_max_us;
    /// Minimum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be >= pres_delay_min_us
    uint32_t pref_pres_delay_min_us;
    /// Maximum preferred presentation delay in microseconds
    /// 0 means no preference
    /// If not equal to 0, shall be <= pres_delay_max_us
    uint32_t pref_pres_delay_max_us;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// PDU framing arrangement
    uint8_t framing;
    /// Preferred PHY bit field
    uint8_t phy_bf;
    /// Preferred maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
} app_gaf_bap_qos_req_t;

/// QoS Configuration structure
typedef struct app_gaf_bap_qos_cfg
{
    /// PDU framing arrangement
    uint8_t framing;
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Maximum Transport latency in milliseconds
    /// From 5ms (0x5) to 4000ms (0xFA0)
    uint16_t trans_latency_max_ms;
    /// Presentation Delay in microseconds
    uint32_t pres_delay_us;
    /// SDU interval in microseconds
    /// From 255us (0xFF) to 16777215us (0xFFFFFF)
    uint32_t sdu_intv_us;
} app_gaf_bap_qos_cfg_t;

/// Structure for BAP_UC_SRV_CONFIGURE_CODEC request indication message
typedef struct
{
    /// Request indication code (set to #BAP_UC_SRV_CONFIGURE_CODEC_RI)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// Target Latency (see #bap_uc_tgt_latency enumeration)
    uint8_t tgt_latency;
    /// Target PHY (see #bap_uc_tgt_phy enumeration)
    uint8_t tgt_phy;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    app_gaf_bap_cfg_t cfg;
} app_gaf_uc_srv_configure_codec_req_ind_t;

/// Structure for BAP_UC_SRV_ENABLE request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_ENABLE
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    app_gaf_bap_cfg_metadata_t metadata;
} app_gaf_uc_srv_enable_req_ind_t;

/// Structure for BAP_UC_SRV_UPDATE_METADATA request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_UPDATE_METADATA_RI
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
    /// Metadata structure
    app_gaf_bap_cfg_metadata_t metadata;
} app_gaf_uc_srv_update_metadata_req_ind_t;

/// Structure for BAP_UC_SRV_RELEASE request indication message
typedef struct
{
    /// Request indication code (@see enum bap_uc_srv_req_ind_code)
    ///  - BAP_UC_SRV_RELEASE_RI
    uint16_t req_ind_code;
    /// ASE local index
    uint8_t ase_lid;
} app_gaf_uc_srv_release_req_ind_t;

/// Structure for BAP_CAPA_SRV_LOCATION indication message
typedef struct
{
    /// Indication code (@see enum bap_capa_srv_ind_code)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Direction (@see enum gaf_direction)
    uint8_t direction;
    /// Location bit field
    uint32_t location_bf;
} app_gaf_capa_srv_location_ind_t;

typedef struct
{
    /// Indication code (shall be set to #BAP_CAPA_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field\n
    /// Each bit corresponds to a characteristic in the range [0, BAP_CAPA_CHAR_TYPE_PAC[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for Sink/Source PAC characteristic\n
    /// Each bit corresponds to an instance of the Sink/Source PAC characteristic (Sink placed first)
    uint16_t pac_cli_cfg_bf;
} app_gaf_capa_srv_bond_data_ind_t;

typedef struct
{
    /// Indication code (shall be set to #BAP_CAPA_SRV_LOCATION)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_capa_srv_cccd_written_ind_t;

/// Broadcast ID
typedef struct app_gaf_bap_bcast_id
{
    /// ID
    uint8_t id[APP_GAF_BAP_BC_BROADCAST_ID_LEN];
} app_gaf_bap_bcast_id_t;

/// Broadcast code used for stream encryption
typedef struct app_gaf_bc_code_t
{
    /// Broadcast Code value
    uint8_t bcast_code[APP_GAP_KEY_LEN];
} app_gaf_bc_code_t;

///  scan group state
enum app_gaf_bap_bc_scan_state
{
    ///  group in idle state
    APP_GAF_BAP_BC_SCAN_STATE_IDLE = 0,
    ///  group in scanning state
    APP_GAF_BAP_BC_SCAN_STATE_SCANNING,
    ///  group in periodic synchronizing state
    APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZING,
    ///  group in periodic synchronized state
    APP_GAF_BAP_BC_SCAN_STATE_SYNCHRONIZED,
    ///  group in stream state
    APP_GAF_BAP_BC_SCAN_STATE_STREAMING,
};

/// scan method
enum app_gaf_bap_bc_scan_method
{
    ///  sink scan broadcast src directly
    APP_GAF_BAP_BC_SINK_SCAN = 0,
    ///  assist scan broadcast src on behalf of deleg
    APP_GAF_BAP_BC_ASSIST_SCAN,
};

/// scan trigger method
enum app_gaf_bap_bc_scan_trigger_method
{
    ///  sink scan broadcast src directly
    APP_GAF_BAP_BC_SINK_TRIGGER = 0,
    ///  assist scan broadcast src on behalf of deleg
    APP_GAF_BAP_BC_ASSIST_TRIGGER,
    ///  deleg scan pa after solicition
    APP_GAF_BAP_BC_DELEG_TRIGGER,
};

/// Bit Field of periodic advertising information that is supposed to be report by scan module
/// Filtering level to report information about basic audio announcement present in periodic advertising report
enum app_gaf_bap_bc_scan_report_filter_bf
{
    /// Report Periodic advertising data (in raw format)
    APP_GAF_BAP_BC_SCAN_REPORT_PER_ADV_DATA_BIT     = 0x01,
    /// Report BIG Info present in ACAD data of a periodic advertising report
    APP_GAF_BAP_BC_SCAN_REPORT_BIG_INFO_BIT         = 0x02,
    /// Report Broadcast Group information of basic audio announcement present in periodic advertising report
    APP_GAF_BAP_BC_SCAN_REPORT_ANNOUNCE_LVL_1_BIT   = 0x04,
    /// Report Broadcast SubGroups information of basic audio announcement present in periodic advertising report
    APP_GAF_BAP_BC_SCAN_REPORT_ANNOUNCE_LVL_2_BIT   = 0x08,
    /// Report Broadcast Stream information of basic audio announcement present in periodic advertising report
    APP_GAF_BAP_BC_SCAN_REPORT_ANNOUNCE_LVL_3_BIT   = 0x10,
};

/// Periodic advertising address information
typedef struct
{
    /// BD Address of device
    uint8_t addr[APP_GAP_BD_ADDR_LEN];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
    /// Advertising SID
    uint8_t adv_sid;
} app_gaf_bap_adv_id_t;

/// Extended advertising information
typedef struct
{
    // Device Address
    app_gaf_bap_adv_id_t        adv_id;
    // Length of complete extend advertising data
    uint8_t                     length;
    // Complete extend advertising data containing the complete Broadcast Audio Announcement
    uint8_t                     data[251];
} app_gaf_extend_adv_report_t;

/// Structure for scan report indication message
typedef struct
{
    uint8_t             scan_trigger_method;
    /// Broadcast ID
    app_gaf_bap_bcast_id_t      bcast_id;

    int8_t                      rssi;

    app_gaf_extend_adv_report_t adv_report;
} app_gaf_bc_scan_adv_report_t;

/// Periodic Advertising Sync Params
typedef struct app_gaf_bap_bc_scan_sync_param
{
    /// Periodic Advertising identification
    app_gaf_bap_adv_id_t adv_id;
    /// Number of Periodic Advertising that can be skipped after a successful reception\n
    /// Maximum authorized value is 499
    uint16_t skip;
    /// Report filtering bit field (see #bap_bc_scan_report_filter_bf enumeration)
    uint8_t report_filter_bf;
    /// Synchronization timeout for the Periodic Advertising (in unit of 10ms between 100ms and 163.84s)
    uint16_t sync_to_10ms;
    /// Scan Timeout in seconds
    uint16_t timeout_s;
} app_gaf_bap_bc_scan_sync_param_t;

/// Scan Parameters structure
typedef struct app_gaf_bap_bc_scan_param
{
    /// Scan interval for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
    uint16_t intv_1m_slot;
    /// Scan interval for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
    uint16_t intv_coded_slot;
    /// Scan window for LE 1M PHY in multiple of 0.625ms - Must be higher than 2.5ms
    uint16_t wd_1m_slot;
    /// Scan window for LE Codec PHY in multiple of 0.625ms - Must be higher than 2.5ms
    uint16_t wd_coded_slot;
} app_gaf_bap_bc_scan_param_t;

/// Structure for #BAP_BC_SCAN_PA_ESTABLISHED indication message
typedef struct
{
    /// Indication code (set to #BAP_BC_SCAN_PA_ESTABLISHED)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Periodic Advertising identification
    app_gaf_bap_adv_id_t adv_id;
    /// PHY on which synchronization has been established (see #gap_phy_val enumeration)
    uint8_t phy;
    /// Periodic advertising interval (in unit of 1.25ms, min is 7.5ms)
    uint16_t interval_frames;
} app_gaf_bap_bc_scan_pa_established_ind_t;

/// Structure for #BAP_BC_SCAN_PA_TERMINATED indication message
typedef struct
{
    /// Indication code (set to #BAP_BC_SCAN_PA_TERMINATED)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Stop reason (see #bap_bc_scan_pa_term_reason enumeration)
    uint8_t reason;
} app_gaf_bap_bc_scan_pa_terminated_ind_t;

/// Stream information
typedef struct app_gaf_bap_bc_scan_stream
{
    /// Stream position in group
    uint8_t              stream_pos;
    /// Codec configuration length
    uint8_t              cfg_len;
    /// Codec configuration data in LTV format
    uint8_t              *cfg;
} app_gaf_bap_bc_scan_stream_t;

/// Stream information
typedef struct app_gaf_bap_bc_scan_stream_info
{
    /// Subgroup identifier
    uint8_t sgrp_id;
    /// Stream position in group
    uint8_t stream_pos;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    /// Parameters structure
    app_gaf_bap_cfg_param_t param;
    /// Length of data value
    uint8_t ltv_len;
    /// Data value
    uint8_t *ltv_data;
} app_gaf_bap_bc_scan_stream_info_t;

/// Subgroup information
typedef struct app_gaf_bap_bc_scan_subgrp_info
{
    /// Subgroup identifier
    uint8_t                 sgrp_id;
    /// Codec ID value
    /// (Octet  = 0xFE if equals to group codec, surcharged otherwise)
    app_gaf_codec_id_t       codec_id;
    /// Stream position index bit field indicating for which streams are part of the subgroup
    uint32_t                 stream_pos_bf;
    /// Codec Configuration parameters structure
    app_gaf_bap_cfg_param_t          param;
    /// Codec Configuration Metadata parameters structure
    app_gaf_bap_cfg_metadata_param_t param_metadata;
    /// Length of Codec Configuration value
    uint8_t                  cfg_len;
    /// Length of Metadata value
    uint8_t                  metadata_len;
    /// Codec Configuration value followed by Metadata value
    /// Both values are in LTV format
    uint8_t                  *val;
} app_gaf_bap_bc_scan_subgrp_info_t;

/// Group information
typedef struct app_gaf_bap_bc_scan_group_info
{
    /// Number of subgroups in the Broadcast Group
    uint8_t              nb_subgroups;
    /// Total number of streams in the Broadcast Group
    uint8_t              nb_streams;
    /// Audio output presentation delay in microseconds
    uint32_t             pres_delay_us;
} app_gaf_bap_bc_scan_group_info_t;

/// BIG Info Report
typedef struct app_gaf_bap_bc_scan_big_info
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
} app_gaf_bap_bc_scan_big_info_t;

typedef struct app_gaf_bap_bc_scan_pa_report_info
{
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Length of periodic advertising data
    uint8_t length;
    /// Complete periodic advertising report
    uint8_t pa_adv_data[APP_GAF_BAP_PER_ADV_DATA_MAX_LEN];
    /// BIG Info
    app_gaf_bap_bc_scan_big_info_t big_info;
    /// Group Info
    app_gaf_bap_bc_scan_group_info_t group_info;
    /// Subgroup Info
    app_gaf_bap_bc_scan_subgrp_info_t *subgroup_info;
    /// Stream Info
    app_gaf_bap_bc_scan_stream_info_t *stream_info;
    /// PA sync hdl
    uint16_t pa_sync_hdl;
    /// Addr info
    app_gaf_bap_adv_id_t pa_addr;
} app_gaf_bap_bc_scan_pa_report_info_t;

/// Structure for scan report indication message
typedef struct
{
    uint8_t   scan_trigger_method;
    // True if broadcast isochronous group is encrypted, False otherwise
    bool      encrypted;
} app_gaf_bc_scan_state_stream_t;

///  Basic structure for GAF_CMP_EVT message
typedef struct
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
} app_gaf_cmp_evt_t;

/// Structure for BAP_BC_SCAN_PA_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_REPORT
    uint16_t    ind_code;
    /// Periodic Advertising local index
    uint8_t     pa_lid;
    /// Length of advertising data
    uint8_t     length;
    /// Complete periodic advertising report
    uint8_t     data[__ARRAY_EMPTY];
} app_gaf_bc_scan_pa_report_ind_t;

/// Structure for BAP_BC_SCAN_PA_ESTABLISHED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_ESTABLISHED
    uint16_t            ind_code;
    /// Periodic Advertising local index
    uint8_t             pa_lid;
    /// Periodic advertising address information
    app_gaf_bap_adv_id_t   adv_addr;
    /// Only valid for a Periodic Advertising Sync Transfer, else ignore
    uint16_t            serv_data;
} app_gaf_bc_scan_pa_established_ind_t;

/// Structure for BAP_BC_SCAN_PA_TERMINATED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_PA_TERMINATED
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Stop reason (@see enum gaf_err)
    uint16_t             reason;
} app_gaf_bc_scan_pa_terminated_ind_t;

/// Structure for BAP_BC_SCAN_GROUP_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_GROUP_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Number of subgroups in the Broadcast Group
    uint8_t              nb_subgroups;
    /// Total number of streams in the Broadcast Group
    uint8_t              nb_streams;
    /// Codec ID value
    app_gaf_codec_id_t       codec_id;
    /// Audio output presentation delay in microseconds
    uint32_t             pres_delay_us;
    /// Length of Codec Configuration value
    uint8_t              cfg_len;
    /// Length of Metadata value
    uint8_t              metadata_len;
    /// Codec Configuration value followed by Metadata value
    /// Both values are in LTV format
    uint8_t              val[__ARRAY_EMPTY];
} app_gaf_bc_scan_group_report_ind_t;

/// Structure for BAP_BC_SCAN_SUBGROUP_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_SUBGROUP_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Subgroup identifier
    uint8_t              sgrp_id;
    /// Codec ID value
    /// (Octet  = 0xFE if equals to group codec, surcharged otherwise)
    app_gaf_codec_id_t       codec_id;
    /// Stream position index bit field indicating for which streams are part of the subgroup
    uint32_t             stream_pos_bf;
    /// Codec Configuration parameters structure
    struct
    {
        /// Audio Locations of the Audio Channels being configured for the codec (i.e the number of codec frames per
        /// block) and their ordering within a single block of codec frames
        /// When transmitted, part of Codec Specific Configuration only if not equal to 0
        /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
        uint32_t location_bf;
        /// Length of a codec frame in octets
        uint16_t frame_octet;
        /// Sampling Frequency (see #bap_sampling_freq enumeration)
        uint8_t sampling_freq;
        /// Frame Duration (see #bap_frame_dur enumeration)
        uint8_t frame_dur;
        /// Number of blocks of codec frames that shall be sent or received in a single SDU
        uint8_t frames_sdu;
    } cfg_param;
    /// Codec Configuration Metadata parameters structure
    struct
    {
        /// Streaming Audio Contexts bit field (see #bap_context_type_bf enumeration)
        uint16_t context_bf;
    } param_metadata;
    /// Length of Codec Configuration value
    uint8_t              cfg_len;
    /// Length of Metadata value
    uint8_t              metadata_len;
    /// Codec Configuration value followed by Metadata value
    /// Both values are in LTV format
    const uint8_t        *cfg_val;
    const uint8_t        *metadata_val;
} app_gaf_bc_scan_subgroup_report_ind_t;

/// Structure for BAP_BC_SCAN_STREAM_REPORT indication message
typedef struct
{
    /// Indication code (set to #BAP_BC_SCAN_STREAM_REPORT)
    uint16_t ind_code;
    /// Periodic Advertising local index
    uint8_t pa_lid;
    /// Subgroup identifier
    uint8_t sgrp_id;
    /// Stream position in group
    uint8_t stream_pos;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Codec Configuration structure
    app_gaf_bap_cfg_t cfg;
} app_gaf_bc_scan_stream_report_ind_t;

/// BIG Info Report
typedef struct
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
} app_gaf_big_info_t;

/// Structure for BAP_BC_SCAN_BIG_INFO_REPORT indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_ind_codes)
    /// - BAP_BC_SCAN_BIG_INFO_REPORT
    uint16_t             ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// BIG Info Report
    app_gaf_big_info_t   report;
} app_gaf_bc_scan_big_info_report_ind_t;

/// Structure for BAP_BC_SCAN_PA_SYNCHRONIZE_RI request indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_scan_req_ind_codes)
    /// - BAP_BC_SCAN_PA_SYNCHRONIZE_RI
    uint16_t             req_ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Source local index
    uint8_t              src_lid;
    /// Connection Local Identifier of requester
    uint8_t              con_lid;
    /// Targeted periodic advertiser
    app_gaf_bap_adv_id_t addr;
} app_gaf_bc_scan_pa_synchronize_req_ind_t;

/// Advertising Parameters structure
typedef struct app_gaf_bap_bc_adv_data
{
    /// Advertising data length
    uint8_t adv_data_len;
    /// Advertising data adv_type
    uint8_t adv_type;
    /// Advertising data
    uint8_t adv_data[0];
} app_gaf_bap_bc_adv_data_t;

typedef struct
{
    /// Indication code (@see enum bap_bc_scan_req_ind_codes)
    /// - BAP_BC_SCAN_PA_TERMINATE_RI
    uint16_t             req_ind_code;
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Source local index
    uint8_t              src_lid;
    /// Connection Local Identifier of requester
    uint8_t              con_lid;
} app_gaf_bc_scan_pa_terminate_req_ind_t;

/// Broadcast group configuration structure (provided by controller after stream establisment)
typedef struct
{
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
} app_gaf_iap_bg_config_t;

/// Broadcast source group state
enum app_gaf_bap_bc_src_state
{
    /// Broadcast group in idle state
    APP_GAF_BAP_BC_SRC_STATE_IDLE,
    /// Broadcast group in configured state
    /// - Periodic ADV started
    APP_GAF_BAP_BC_SRC_STATE_CONFIGURED,
    /// Broadcast group in stream state
    /// - broadcaster started
    APP_GAF_BAP_BC_SRC_STATE_STREAMING,
};

/// Structure for APP_GAF_BAP_BC_SINK_STATUS indication message
typedef struct
{
    /// Indication code (see #bap_bc_sink_msg_ind_code enumeration)
    /// - APP_GAF_BAP_BC_SINK_STATUS
    uint16_t ind_code;
    /// Group local index
    uint8_t grp_lid;
    /// Broadcast Sink state (see #bap_bc_sink_state enumeration)
    uint8_t state;
    /// Stream position bit field indicating Stream with which synchronization is established
    /// Meaningful only if synchronization has been established
    uint32_t stream_pos_bf;
    /// When sync established, provides information about broadcast group else meaningless
    app_gaf_iap_bg_config_t bg_cfg;
    /// Number of BISes synchronization has been established with
    /// Meaningful only if synchronization has been established
    uint8_t nb_bis;
    /// List of Connection Handle values provided by the Controller (nb_bis elements)
    uint16_t conhdl[__ARRAY_EMPTY];
} app_gaf_bc_sink_status_ind_t;

/// Broadcast source group state
enum app_gaf_bap_bc_sink_app_state
{
    /// Broadcast group in idle state
    APP_GAF_BAP_BC_SINK_STATE_IDLE,
    /// Broadcast group in ENABLE state
    APP_GAF_BAP_BC_SINK_STATE_ENABLED,
    /// Broadcast group in stream state
    APP_GAF_BAP_BC_SINK_STATE_STREAMING,
};

/// Structure for BAP_BC_SINK_ENABLE
typedef struct app_gaf_bap_bc_sink_enable
{
    /// Periodic Advertising local index
    uint8_t              pa_lid;
    /// Broadcast ID
    app_gaf_bap_bcast_id_t bcast_id;
    /// Maximum number of subevents the controller should use to receive data payloads in each interval
    uint8_t              mse;
    /// Stream position bit field indicating streams to synchronize with.
    uint32_t             stream_pos_bf;
    /// Timeout duration (10ms unit) before considering synchronization lost (Range 100 ms to 163.84 s).
    uint16_t             timeout_10ms;
    /// Indicate if streams are encrypted (!= 0) or not
    uint8_t              encrypted;
    /// Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    app_gaf_bc_code_t bcast_code;
} app_gaf_bap_bc_sink_enable_t;

/// Start Sink Streaming
typedef struct app_gaf_bap_bc_sink_audio_streaming
{
    /// Group local index
    uint8_t             grp_lid;
    /// Position of the stream in the group (range 1 to 32)
    uint8_t             stream_pos;
    /// Codec ID value
    app_gaf_codec_id_t  codec_id;
    /// Select which audio channel to play when a bis contains multiple audio channels
    uint8_t             audio_chan;
    /// Length of Codec Configuration value - in bytes
    uint8_t             cfg_len;
    /// Codec Configuration - to be casted as bap_lc3_cfg_t for LC3 codec.
    /// Array of bytes - 32-bit aligned to be casted as a SW structure
    uint32_t            cfg[0];
} app_gaf_bap_bc_sink_audio_streaming_t;

/// Broadcast group sink state
enum app_gaf_bap_bc_sink_state
{
    /// Synchronization has been established
    APP_GAF_BAP_BC_SINK_ESTABLISHED = 0x00,
    /// Synchronization has failed
    APP_GAF_BAP_BC_SINK_FAILED = 0x01,
    /// Synchronization establishment has been cancelled
    APP_GAF_BAP_BC_SINK_CANCELLED = 0x02,
    /// Synchronization has been lost
    APP_GAF_BAP_BC_SINK_LOST = 0x03,
    /// Synchronization stopped due to peer termination
    APP_GAF_BAP_BC_SINK_PEER_TERMINATE = 0x04,
    /// Synchronization stopped due to upper layer termination
    APP_GAF_BAP_BC_SINK_UPPER_TERMINATE = 0x05,
    /// Synchronization stopped due to an encryption error
    APP_GAF_BAP_BC_SINK_MIC_FAILURE = 0x06,
};

/// Structure for BAP_BC_DELEG_SOLICITE_STOPPED indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_deleg_ind_code)
    ///  - BAP_BC_DELEG_SOLICITE_STOPPED
    uint16_t ind_code;
    /// Reason why sending of solicitation request has been stopped (@see enum bap_bc_deleg_reason)
    uint16_t reason;
} app_gaf_bc_deleg_solicite_stopped_ind_t;

/// List of GAF_CMD command code values for BAP Broadcast Assistant module
enum
{
    /// Start discovery of Solicitation Requests sent by Delegator devices (see #bap_bc_assist_start_scan_cmd_t)
    APP_GAF_BAP_BC_ASSIST_START_SCAN = 0,
    /// Stop discovery of Solicitation Requests (see #bap_bc_assist_stop_scan_cmd_t)
    APP_GAF_BAP_BC_ASSIST_STOP_SCAN,
    /// Discover Broadcast Audio Scan Service in peer device database (see #bap_bc_assist_discover_cmd_t)
    APP_GAF_BAP_BC_ASSIST_DISCOVER,
    /// Get value of an instance of Broadcast Receive State characteristic (see #bap_bc_assist_get_state_cmd_t)
    APP_GAF_BAP_BC_ASSIST_GET_STATE,
    /// Get current notification state for an instance of the Broadcast Receive State characteristic
    /// (see #bap_bc_assist_get_cfg_cmd_t)
    APP_GAF_BAP_BC_ASSIST_GET_CFG,
    /// Enable or disable sending of notifications for an instance of the Broadcast Receive State characteristic
    /// (see #bap_bc_assist_set_cfg_cmd_t)
    APP_GAF_BAP_BC_ASSIST_SET_CFG,
    /// Inform peer Delegator device that Assistant has either started or stopped scan on its behalf
    /// (see #bap_bc_assist_update_scan_cmd_t)
    APP_GAF_BAP_BC_ASSIST_UPDATE_SCAN,
    /// Add a Broadcast Source (see #bap_bc_assist_add_source_cmd_t)
    APP_GAF_BAP_BC_ASSIST_ADD_SOURCE,
    /// Add a Broadcast Source locally created (Source and Assistant colocated)
    /// (see #bap_bc_assist_add_source_local_cmd_t)
    APP_GAF_BAP_BC_ASSIST_ADD_SOURCE_LOCAL,
    /// Remove a Broadcast Source (see #bap_bc_assist_remove_source_cmd_t)
    APP_GAF_BAP_BC_ASSIST_REMOVE_SOURCE,
    /// Update a Broadcast Source (see #bap_bc_assist_modify_source_cmd_t)
    APP_GAF_BAP_BC_ASSIST_MODIFY_SOURCE,
    /// Update a Broadcast Source locally created (Source and Assistant colocated)
    /// (see #bap_bc_assist_modify_source_local_cmd_t)
    APP_GAF_BAP_BC_ASSIST_MODIFY_SOURCE_LOCAL,
};

/// Values for PA Sync field in Broadcast Audio Scan Control Point characteristic value
enum app_gaf_bap_bc_pa_sync_state
{
    /// Do not synchronize to PA
    APP_GAF_BAP_BC_PA_SYNC_NO_SYNC = 0,
    /// Synchronize to PA, no PAST on client
    APP_GAF_BAP_BC_PA_SYNC_SYNC_NO_PAST,
    /// Synchronize to PA, PAST on client
    APP_GAF_BAP_BC_PA_SYNC_SYNC_PAST,

    APP_GAF_BAP_BC_PA_SYNC_MAX
};

/// Structure For assistant source add
typedef struct app_gaf_bap_bc_assist_add_src
{
    /// Broadcast Source adv information
    app_gaf_extend_adv_report_t adv_report;
    /// Broadcast ID
    app_gaf_bap_bcast_id_t bcast_id;
    /// Required PA synchronization state
    uint8_t pa_sync;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Number of Subgroups\n
    /// From 0 to 32
    uint8_t nb_subgroups;
    /// Required BIS synchronization bit field for the first Subgroup\n
    /// Meaningful only if nb_subgroups != 0\n
    /// #BAP_BC_ASSIST_SET_SGRP_INFO request messages must follow for each additional Subgroup
    uint32_t bis_sync_bf;
} app_gaf_bap_bc_assist_add_src_t;

/// Structure for #BAP_BC_ASSIST_SOLICITATION indication message
typedef struct app_gaf_bap_bc_assist_solicitation_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SOLICITATION)
    uint16_t ind_code;
    /// Address type
    uint8_t addr_type;
    /// Address
    uint8_t addr[6];
    /// Available Audio Contexts bit field
    uint8_t context_bf;
    /// Length of advertising data
    uint16_t length;
    /// Advertising data
    uint8_t adv_data[__ARRAY_EMPTY];
} app_gaf_bap_bc_assist_solicitation_ind_t;

/// Structure for #BAP_BC_ASSIST_SVC_CHANGED indication message
typedef struct app_gaf_bap_bc_assist_svc_changed_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_SVC_CHANGED)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_bap_bc_assist_svc_changed_ind_t;

/// Structure for #BAP_BC_ASSIST_BCAST_CODE request indication message
typedef struct app_gaf_bap_bc_assist_bcast_code_req_ind
{
    /// Request indication code (set to #BAP_BC_ASSIST_BCAST_CODE)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;;
    /// Source local index
    uint8_t src_lid;
} app_gaf_bap_bc_assist_bcast_code_req_ind_t;

/// Broadcast Receive State characteristic description structure
typedef struct app_gaf_bap_bc_assist_rx_state_char
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client characteristic configuration descriptor handle
    uint16_t desc_hdl;
    /// Source ID
    uint8_t src_id;
} app_gaf_bap_bc_assist_rx_state_char_t;

/// Broadcast Audio Scan Service content description structure
typedef struct app_gaf_bap_bc_assist_bass
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// Characteristic value handle for Broadcast Audio Scan Control Point characteristic
    uint16_t cp_val_hdl;
    /// Number of discovered Broadcast Receive State characteristics
    uint8_t nb_rx_state;
    /// Pointer to information structures for Broadcast Receive State characteristic
    app_gaf_bap_bc_assist_rx_state_char_t rx_state_char_info[__ARRAY_EMPTY];
} app_gaf_bap_bc_assist_bass_t;

/// Structure for #BAP_BC_ASSIST_BOND_DATA indication message
typedef struct app_gaf_bap_bc_assist_bond_data_ind
{
    /// Indication code (set to #BAP_BC_ASSIST_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Service description
    app_gaf_bap_bc_assist_bass_t bass_info;
} app_gaf_bap_bc_assist_bond_data_ind_t;

/// Structure for command complete event message
typedef struct app_gaf_bap_bc_assist_cmp_evt
{
    /// Command code (see #bap_bc_assist_msg_cmd_code enumeration)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Source local index
    uint8_t src_lid;
} app_gaf_bap_bc_assist_cmp_evt_t;

/// Structure for delegator periodic advertising sync
typedef struct app_gaf_bap_bc_deleg_pa_sync
{
    /// Bit field of filtered report types - The reception of BIGInfo is forced to enable
    /// (@see enum bap_bc_scan_report_filter_bf)
    uint8_t  report_filter_bf;
    /// Number of periodic advertising that can be skipped after a successful receive.
    /// Maximum authorized value is 499
    uint16_t skip;
    /// Synchronization timeout for the periodic advertising
    /// (in unit of 10ms between 100ms and 163.84s)
    uint16_t sync_to_10ms;
    /// Sync establishment timeout in seconds
    uint16_t timeout_s;
} app_gaf_bap_bc_deleg_pa_sync_t;

/// Structure for big sync
typedef struct app_gaf_bap_bc_deleg_sink_enable
{
    /// Source local index
    uint8_t          src_lid;
    /// True if Broadcast code is meaningful, False otherwise
    /// if False it means that broadcast code has already been received by delegator or Broadcast group isn't encrypted
    bool             bcast_code_present;
    /// Broadcast code
    app_gaf_bc_code_t bcast_code;
    /// Stream position bit field indicating streams to synchronize with.
    uint32_t         stream_pos_bf;
    /// Timeout duration (10ms unit) before considering synchronization lost - (Range 100 ms to 163.84 s).
    uint16_t         timeout_10ms;
    /// Maximum number of subevents the controller should use to received data payloads in each interval
    uint8_t          mse;
} app_gaf_bap_bc_deleg_sink_enable_t;

/// Structure for BAP_BC_DELEG_BOND_REMOTE_SCAN indication message
typedef struct
{
    /// Indication code (@see enum bap_bc_deleg_ind_code)
    ///  - BAP_BC_DELEG_BOND_REMOTE_SCAN
    uint16_t ind_code;
    /// Connection local index
    uint8_t  con_lid;
    /// Broadcast Assistant device scan state (@see enum bap_bc_deleg_scan_state)
    uint8_t  state;
} app_gaf_bc_deleg_bond_remote_scan_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_ADD_RI request indication message
typedef struct
{
    /// Request Indication code (set to #BAP_BC_DELEG_ADD_SOURCE_RI)
    uint16_t req_ind_code;
    /// Allocated Source local index
    uint8_t src_lid;
    /// Connection local index for device that has added the source
    uint8_t con_lid;
    /// Periodic Advertising identification
    app_gaf_bap_adv_id_t adv_id;
    /// Broadcast ID
    app_gaf_bap_bcast_id_t bcast_id;
    /// Periodic Advertising interval in frames\n
    /// From 0x0006 to 0xFFFE\n
    /// BAP_BC_UNKNOWN_PA_INTV indicates that interval is unknown
    uint16_t pa_intv_frames;
    /// Requested synchronization state for Periodic Advertising
    uint8_t pa_sync_req;
    /// Number of Subgroups
    uint8_t nb_subgroups;
    /// Subgroup information\n
    /// Must be casted as an array of #bap_bc_sgrp_info_t structures. Each structure has a variable size
    uint32_t subgroup_info[__ARRAY_EMPTY];
} app_gaf_bc_deleg_source_add_req_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_REMOVE_RI request indication message
typedef struct
{
    /// Request Indication code (@see enum bap_bc_deleg_req_ind_code)
    ///  - BAP_BC_DELEG_SOURCE_REMOVE_RI
    uint16_t             req_ind_code;
    /// Source local index
    uint8_t              src_lid;
    /// Connection local index
    uint8_t              con_lid;
} app_gaf_bc_deleg_source_remove_req_ind_t;

/// Structure for BAP_BC_DELEG_SOURCE_UPDATE_RI request indication message
typedef struct
{
    /// Request Indication code (@see enum bap_bc_deleg_req_ind_code)
    ///  - BAP_BC_DELEG_SOURCE_UPDATE_RI
    uint16_t             req_ind_code;
    /// Source local index
    uint8_t              src_lid;
    /// Connection local index
    uint8_t              con_lid;
    /// PA sync request
    uint8_t              pa_sync_req;
    /// Metadata in LTV format
    app_gaf_ltv_t        metadata;
} app_gaf_bc_deleg_source_update_req_ind_t;

typedef struct
{
    /// Indication code (@see enum bap_bc_deleg_ind_code)
    ///  - BAP_BC_DELEG_BOND_DATA
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field (1 bit per Broadcast Receive State characteristic instance)
    uint16_t cli_cfg_bf;
} app_gaf_bc_deleg_bond_data_ind_t;

/// Structure for command complete event message
typedef struct
{
    /// Command code (@see enum acc_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    union
    {
        /// Additional parameter
        uint8_t param;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Result
    uint8_t result;
} app_gaf_acc_mcc_cmp_evt_t;

/// Structure for ACC_MCC_TRACK_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
} app_gaf_acc_mcc_track_changed_ind_t;

/// Structure for ACC_MCC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint32_t val;
        /// Playback speed
        int8_t playback_speed;
        /// Seeking speed
        int8_t seeking_speed;
        /// Media state
        uint8_t state;
        /// Supported media control operation codes bit field
        uint32_t opcodes_supp_bf;
        /// Playing Order
        uint8_t playing_order;
        /// Supported Playing Order bit field
        uint32_t playing_order_supp_bf;
        /// Track duration of the current track in 0.01 second resolution
        int32_t track_dur;
        /// Track position of the current track in 0.01 second resolution
        int32_t track_pos;
        /// Content Control ID
        uint8_t ccid;
    } val;
} app_gaf_acc_mcc_value_ind_t;

/// Structure for ACC_MCC_VALUE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} app_gaf_acc_mcc_value_long_ind_t;

/// Structure for ACC_MCC_SVC_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_acc_mcc_svc_changed_ind_t;

/// Characteristic type values
/// Values are ordered so that the notification-capable characteristic type values are equal
/// to the characteristic type values
enum app_gaf_acc_mc_char_type
{
    /// Media Player Name characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYER_NAME = 0,
    /// Track Changed characteristic
    APP_GAF_MC_CHAR_TYPE_TRACK_CHANGED,
    /// Track Title characteristic
    APP_GAF_MC_CHAR_TYPE_TRACK_TITLE,
    /// Track Duration characteristic
    APP_GAF_MC_CHAR_TYPE_TRACK_DURATION,
    /// Track Position characteristic
    APP_GAF_MC_CHAR_TYPE_TRACK_POSITION,
    /// Playback Speed characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYBACK_SPEED,
    /// Seeking Speed characteristic
    APP_GAF_MC_CHAR_TYPE_SEEKING_SPEED,
    /// Current Track Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_CUR_TRACK_OBJ_ID,
    /// Next Track Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_NEXT_TRACK_OBJ_ID,
    /// Current Group Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_CUR_GROUP_OBJ_ID,
    /// Parent Group Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_PARENT_GROUP_OBJ_ID,
    /// Playing Order characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYING_ORDER,
    /// Media State characteristic
    APP_GAF_MC_CHAR_TYPE_MEDIA_STATE,
    /// Media Control Point
    APP_GAF_MC_CHAR_TYPE_MEDIA_CP,
    /// Media Control Point Opcodes Supported
    APP_GAF_MC_CHAR_TYPE_MEDIA_CP_OPCODES_SUPP,
    /// Search Results Object ID
    APP_GAF_MC_CHAR_TYPE_SEARCH_RESULTS_OBJ_ID,
    /// Search Control Point
    APP_GAF_MC_CHAR_TYPE_SEARCH_CP,

    /// All characteristics above are notification-capable
    APP_GAF_MC_NTF_CHAR_TYPE_MAX,

    /// Current Track Segments Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_CUR_TRACK_SEG_OBJ_ID = APP_GAF_MC_NTF_CHAR_TYPE_MAX,
    /// Media Player Icon Object ID characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYER_ICON_OBJ_ID,
    /// Media Player Icon URL characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYER_ICON_URL,
    /// Playing Order Supported characteristic
    APP_GAF_MC_CHAR_TYPE_PLAYING_ORDER_SUPP,
    /// Content Control ID
    APP_GAF_MC_CHAR_TYPE_CCID,

    APP_GAF_MC_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum app_gaf_acc_mc_desc_type
{
    /// Client Characteristic Configuration descriptor for Media Player Name characteristic
    APP_GAF_MC_DESC_TYPE_CCC_PLAYER_NAME = 0,
    /// Client Characteristic Configuration descriptor for Track Changed characteristic
    APP_GAF_MC_DESC_TYPE_CCC_TRACK_CHANGED,
    /// Client Characteristic Configuration descriptor for Track Title characteristic
    APP_GAF_MC_DESC_TYPE_CCC_TRACK_TITLE,
    /// Client Characteristic Configuration descriptor for Track Duration characteristic
    APP_GAF_MC_DESC_TYPE_CCC_TRACK_DURATION,
    /// Client Characteristic Configuration descriptor for Track Position characteristic
    APP_GAF_MC_DESC_TYPE_CCC_TRACK_POSITION,
    /// Client Characteristic Configuration descriptor for Playback Speed characteristic
    APP_GAF_MC_DESC_TYPE_CCC_PLAYBACK_SPEED,
    /// Client Characteristic Configuration descriptor for Seeking Speed characteristic
    APP_GAF_MC_DESC_TYPE_CCC_SEEKING_SPEED,
    /// Client Characteristic Configuration descriptor for Current Track Object ID characteristic
    APP_GAF_MC_DESC_TYPE_CCC_CUR_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Next Track Object ID characteristic
    APP_GAF_MC_DESC_TYPE_CCC_NEXT_TRACK_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Current Group Object ID characteristic
    APP_GAF_MC_DESC_TYPE_CCC_CUR_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Parent Group Object ID characteristic
    APP_GAF_MC_DESC_TYPE_CCC_PARENT_GROUP_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Playing Order characteristic
    APP_GAF_MC_DESC_TYPE_CCC_PLAYING_ORDER,
    /// Client Characteristic Configuration descriptor for Media State characteristic
    APP_GAF_MC_DESC_TYPE_CCC_MEDIA_STATE,
    /// Client Characteristic Configuration descriptor for Media Control Point
    APP_GAF_MC_DESC_TYPE_CCC_MEDIA_CP,
    /// Client Characteristic Configuration descriptor for Media Control Point Opcodes Supported
    APP_GAF_MC_DESC_TYPE_CCC_MEDIA_CP_OPCODES_SUPP,
    /// Client Characteristic Configuration descriptor for Search Results Object ID
    APP_GAF_MC_DESC_TYPE_CCC_SEARCH_RESULTS_OBJ_ID,
    /// Client Characteristic Configuration descriptor for Search Control Point
    APP_GAF_MC_DESC_TYPE_CCC_SEARCH_CP,

    APP_GAF_MC_DESC_TYPE_MAX,
};

/// Content description structure for Media Control Service
typedef struct app_gaf_acc_mcc_mcs_info
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// UUID
    uint16_t uuid;
    /// Characteristics description
    app_gaf_prf_char_t char_info[APP_GAF_MC_CHAR_TYPE_MAX];
    /// Descriptors description
    app_gaf_prf_desc_t desc_info[APP_GAF_MC_DESC_TYPE_MAX];
} app_gaf_acc_mcc_mcs_info_t;

/// Structure for ACC_MCC_BOND_DATA indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Media local index
    uint8_t media_lid;

    app_gaf_acc_mcc_mcs_info_t mcs_info;
} app_gaf_acc_mcc_bond_data_ind_t;

/// Structure for command complete event message
typedef struct
{
    /// Command code (@see enum acc_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    union
    {
        /// Value
        uint8_t val;
        /// Characteristic type
        uint8_t char_type;
        /// Operation code
        uint8_t opcode;
    } u;
    /// Call index
    uint8_t call_id;
    /// Result
    uint8_t result;
} app_gaf_acc_tbc_cmp_evt_t;

/// Structure for APP_GAF_ACC_TBC_CALL_STATE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state
    uint8_t state;
} app_gaf_acc_tbc_call_state_ind_t;

/// Structure for APP_GAF_ACC_TBC_CALL_STATE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t id;
    /// Call flags
    uint8_t flags;
    /// Call state, @see enum acc_tb_call_state
    uint8_t state;
    /// Length of Incoming or Outgoing Call URI value
    uint8_t uri_len;
    /// Incoming or Outgoing Call URI value
    uint8_t uri[__ARRAY_EMPTY];
} app_gaf_acc_tbc_call_state_long_ind_t;

/// Structure for APP_GAF_ACC_TBC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    /// Meaningful only for Termination Reason characteristic
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint16_t val;
        /// Bearer Technology
        uint8_t techno;
        /// Signal Strength
        uint8_t sign_strength;
        /// Signal Strength Reporting Interval in seconds
        uint8_t sign_strength_intv_s;
        /// Content Control ID
        uint8_t ccid;
        /// Status Flags bit field
        uint16_t status_flags_bf;
        /// Call Control Point Optional Opcodes bit field
        uint16_t opt_opcodes_bf;
        /// Termination Reason
        uint8_t term_reason;
    } val;
} app_gaf_acc_tbc_value_ind_t;

/// Structure for APP_GAF_ACC_TBC_VALUE_LONG indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t call_id;
    /// Characteristic type
    uint8_t char_type;
    /// Length of value
    uint16_t val_len;
    /// Value
    uint8_t val[__ARRAY_EMPTY];
} app_gaf_acc_tbc_value_long_ind_t;

/// Structure for APP_GAF_ACC_TBC_SVC_CHANGED indication message
typedef struct
{
    /// Indication code (@see enum acc_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_acc_tbc_svc_changed_ind_t;


/// Call state values
enum
{
    /// Incoming
    APP_GAF_TB_CALL_STATE_INCOMING = 0,
    /// Dialing
    APP_GAF_TB_CALL_STATE_DIALING,
    /// Alerting
    APP_GAF_TB_CALL_STATE_ALERTING,
    /// Active
    APP_GAF_TB_CALL_STATE_ACTIVE,
    /// Locally Held
    APP_GAF_TB_CALL_STATE_LOC_HELD,
    /// Remotely Held
    APP_GAF_TB_CALL_STATE_REMOTE_HELD,
    /// Locally and Remotely Held
    APP_GAF_TB_CALL_STATE_LOC_REMOTE_HELD,

    APP_GAF_TB_CALL_STATE_MAX,
};

/// Characteristic type values
enum
{
    /// Bearer Provider Name characteristic
    APP_GAF_TB_CHAR_TYPE_PROV_NAME = 0,
    /// Bearer Technology characteristic
    APP_GAF_TB_CHAR_TYPE_TECHNO,
    /// Bearer Signal Strength characteristic
    APP_GAF_TB_CHAR_TYPE_SIGN_STRENGTH,
    /// Bearer List Current Calls characteristic
    APP_GAF_TB_CHAR_TYPE_CURR_CALLS_LIST,
    /// Status Flags characteristic
    APP_GAF_TB_CHAR_TYPE_STATUS_FLAGS,
    /// Incoming Call Target Bearer URI characteristic
    APP_GAF_TB_CHAR_TYPE_IN_TGT_CALLER_ID,
    /// Call State characteristic
    APP_GAF_TB_CHAR_TYPE_CALL_STATE,
    /// Call Control Point characteristic
    APP_GAF_TB_CHAR_TYPE_CALL_CTL_PT,
    /// Termination Reason characteristic
    APP_GAF_TB_CHAR_TYPE_TERM_REASON,
    /// Incoming Call characteristic
    APP_GAF_TB_CHAR_TYPE_INCOMING_CALL,
    /// Call Friendly Name characteristic
    APP_GAF_TB_CHAR_TYPE_CALL_FRIENDLY_NAME,
    /// Bearer URI Schemes Supported List characteristic
    APP_GAF_TB_CHAR_TYPE_URI_SCHEMES_LIST,

    /// All characteristics above are notification-capable
    APP_GAF_TB_NTF_CHAR_TYPE_MAX,

    /// Bearer UCI characteristic
    APP_GAF_TB_CHAR_TYPE_UCI = APP_GAF_TB_NTF_CHAR_TYPE_MAX,
    /// Bearer Signal Strength Reporting Interval characteristic
    APP_GAF_TB_CHAR_TYPE_SIGN_STRENGTH_INTV,
    /// Content Control ID characteristic
    APP_GAF_TB_CHAR_TYPE_CCID,
    /// Call Control Point Optional Opcodes characteristic
    APP_GAF_TB_CHAR_TYPE_CALL_CTL_PT_OPT_OPCODES,

    APP_GAF_TB_CHAR_TYPE_MAX,
};

/// Descriptor type values
enum
{
    /// Client Characteristic Configuration descriptor for Bearer Provider Name characteristic
    APP_GAF_TB_DESC_TYPE_CCC_PROV_NAME = 0,
    /// Client Characteristic Configuration descriptor for Bearer Technology characteristic
    APP_GAF_TB_DESC_TYPE_CCC_TECHNO,
    /// Client Characteristic Configuration descriptor for Bearer Signal Strength characteristic
    APP_GAF_TB_DESC_TYPE_CCC_SIGN_STRENGTH,
    /// Client Characteristic Configuration descriptor for Bearer List Current Calls characteristic
    APP_GAF_TB_DESC_TYPE_CCC_CURR_CALLS_LIST,
    /// Client Characteristic Configuration descriptor for Status Flags characteristic
    APP_GAF_TB_DESC_TYPE_CCC_STATUS_FLAGS,
    /// Client Characteristic Configuration descriptor for Incoming Call Target Bearer
    /// URI characteristic
    APP_GAF_TB_DESC_TYPE_CCC_IN_TGT_CALLER_ID,
    /// Client Characteristic Configuration descriptor for Call State characteristic
    APP_GAF_TB_DESC_TYPE_CCC_CALL_STATE,
    /// Client Characteristic Configuration descriptor for Call Control Point characteristic
    APP_GAF_TB_DESC_TYPE_CCC_CALL_CTL_PT,
    /// Client Characteristic Configuration descriptor for Termination Reason characteristic
    APP_GAF_TB_DESC_TYPE_CCC_TERM_REASON,
    /// Client Characteristic Configuration descriptor for Incoming Call characteristic
    APP_GAF_TB_DESC_TYPE_CCC_INCOMING_CALL,
    /// Client Characteristic Configuration descriptor for Call Friendly Name characteristic
    APP_GAF_TB_DESC_TYPE_CCC_CALL_FRIENDLY_NAME,
    /// Client Characteristic Configuration descriptor for Bearer URI Schemes Supported
    /// List characteristic
    APP_GAF_TB_DESC_TYPE_CCC_URI_SCHEMES_LIST,

    APP_GAF_TB_DESC_TYPE_MAX,
};

/// Content description structure for Telephone Bearer Service
typedef struct app_gaf_acc_tbc_tbs_info
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// Service uuid
    uint16_t uuid;
    /// Characteristics description
    app_gaf_prf_char_t char_info[APP_GAF_TB_CHAR_TYPE_MAX];
    /// Descriptors description
    app_gaf_prf_desc_t desc_info[APP_GAF_TB_DESC_TYPE_MAX];
} app_gaf_acc_tbc_tbs_info_t;

/// Structure for APP_GAF_TBC_SVC_BOND_DATA indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;

    app_gaf_acc_tbc_tbs_info_t tbs_info;
} app_gaf_acc_tbc_bond_data_ind_t;

/// Structure for response message
typedef struct app_gaf_acc_tbs_rsp
{
    /// Request code (see #acc_tbs_msg_req_codes enumeration)
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Union
    union
    {
        /// Value
        uint8_t val;
        /// Characteristic type for #APP_GAF_ACC_TBS_SET request
        uint8_t char_type;
        /// Status type for #APP_GAF_ACC_TBS_SET_STATUS request
        uint8_t status_type;
        /// Connection local index for #APP_GAF_ACC_TBS_RESTORE_BOND_DATA request
        uint8_t con_lid;
        /// Allocated Call index
        uint8_t call_id;
    } u;
    /// Action for #APP_GAF_ACC_APP_GAF_TB_CALL_ACTION request
    uint8_t action;
    /// Operation defined in app_gaf_layer
    uint8_t app_operation;
} app_gaf_acc_tbs_rsp_t;

/// Call action values
enum app_gaf_acc_tbs_call_action
{
    /// Accept call
    APP_GAF_ACC_TBS_ACTION_ACCEPT = 0,
    /// Terminate call
    APP_GAF_ACC_TBS_ACTION_TERMINATE,
    /// Hold call (local)
    APP_GAF_ACC_TBS_ACTION_HOLD_LOCAL,
    /// Retrieve call (local)
    APP_GAF_ACC_TBS_ACTION_RETRIEVE_LOCAL,
    /// Hold call (remote)
    APP_GAF_ACC_TBS_ACTION_HOLD_REMOTE,
    /// Retrieve call (remote)
    APP_GAF_ACC_TBS_ACTION_RETRIEVE_REMOTE,
    /// Remote alert started for a call
    APP_GAF_ACC_TBS_ACTION_ALERT_START,
    /// Remote answer for a call
    APP_GAF_ACC_TBS_ACTION_ANSWER,

    APP_GAF_ACC_TBS_ACTION_MAX
};

/// Direction values
enum
{
    /// Incoming
    APP_GAF_ACC_TB_DIRECTION_INCOMING = 0,
    /// Outgoing
    APP_GAF_ACC_TB_DIRECTION_OUTGOING,
};

/// Call control point notification result code values
enum
{
    /// Opcode write was successful
    APP_GAF_ACC_TB_CP_NTF_RESULT_SUCCESS = 0,
    /// An invalid opcode was used for the Call Control Point write
    APP_GAF_ACC_TB_CP_NTF_RESULT_OPCODE_NOT_SUPPORTED,
    /// Requested operation cannot be completed
    APP_GAF_ACC_TB_CP_NTF_RESULT_OP_NOT_POSSIBLE,
    /// The Call Index used for the Call Control Point write is invalid
    APP_GAF_ACC_TB_CP_NTF_RESULT_INVALID_CALL_INDEX,
    /// The opcode written to the Call Control Point was received when the current Call State
    /// for the Call Index is not in the expected state
    APP_GAF_ACC_TB_CP_NTF_RESULT_STATE_MISMATCH,
    /// Lack of internal resources to complete the requested action
    APP_GAF_ACC_TB_CP_NTF_RESULT_LACK_OF_RESSOURCES,
    /// The Outgoing URI is incorrect or invalid when an Originate opcode is sent
    APP_GAF_ACC_TB_CP_NTF_RESULT_INVALID_URI,

    APP_GAF_ACC_TB_CP_NTF_RESULT_MAX,
};

/// Structure for #APP_GAF_ACC_TBS_BOND_DATA indication message
typedef struct app_gaf_acc_tbs_bond_data_ind
{
    /// Indication code (see #acc_tbs_msg_ind_codes enumeration)
    uint16_t ind_code;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint16_t cli_cfg_bf;
} app_gaf_acc_tbs_bond_data_ind_t;

/// Structure for #APP_GAF_ACC_TBS_REPORT_INTV indication message
typedef struct app_gaf_acc_tbs_report_intv_ind
{
    /// Indication code (see #acc_tbs_msg_ind_codes enumeration)
    uint16_t ind_code;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Signal Strength Reporting Interval in seconds
    uint8_t sign_strength_intv_s;
} app_gaf_acc_tbs_report_intv_ind_t;

/// Structure for #APP_GAF_ACC_TBS_GET request indication message
typedef struct app_gaf_acc_tbs_get_req_ind
{
    /// Request Indication code (see #acc_tbs_msg_req_ind_codes enumeration)
    uint16_t req_ind_code;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Call index
    uint8_t call_id;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Offset
    uint16_t offset;
    /// Maximum length
    uint16_t length;
} app_gaf_acc_tbs_get_req_ind_t;

/// Structure for #APP_GAF_ACC_APP_GAF_TB_CALL_OUTGOING request indication message
typedef struct app_gaf_acc_tbs_call_out_req_ind
{
    /// Request Indication code (see #acc_tbs_msg_req_ind_codes enumeration)
    uint16_t req_ind_code;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Call index
    uint8_t call_id;
    /// Length of Outgoing URI value
    uint8_t uri_len;
    /// Outgoing URI value
    uint8_t uri[__ARRAY_EMPTY];
} app_gaf_acc_tbs_call_out_req_ind_t;

/// Structure for #APP_GAF_ACC_APP_GAF_TB_CALL_ACTION request indication message
typedef struct app_gaf_acc_tbs_call_action_req_ind
{
    /// Request Indication code (see #acc_tbs_msg_req_ind_codes enumeration)
    uint16_t req_ind_code;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Operation code
    uint8_t opcode;
    /// Call index
    uint8_t call_id;
} app_gaf_acc_tbs_call_action_req_ind_t;

/// Operation code values
enum app_gaf_acc_tb_opcode
{
    /// Accept
    APP_GAF_ACC_TB_OPCODE_ACCEPT = 0,
    /// Terminate
    APP_GAF_ACC_TB_OPCODE_TERMINATE,
    /// Local Hold
    APP_GAF_ACC_TB_OPCODE_HOLD,
    /// Local Retrieve
    APP_GAF_ACC_TB_OPCODE_RETRIEVE,
    /// Originate
    APP_GAF_ACC_TB_OPCODE_ORIGINATE,
    /// Join
    APP_GAF_ACC_TB_OPCODE_JOIN,

    APP_GAF_ACC_TB_OPCODE_MAX
};

/// Termination reason values
enum app_gaf_acc_tb_term_reason
{
    /// URI value improperly formed
    APP_GAF_ACC_TB_TERM_REASON_URI = 0,
    /// Call fail
    APP_GAF_ACC_TB_TERM_REASON_CALL_FAIL,
    /// Remote party ended Call
    APP_GAF_ACC_TB_TERM_REASON_REMOTE_END,
    /// Call ended from the Server
    APP_GAF_ACC_TB_TERM_REASON_SRV_END,
    /// Line Busy
    APP_GAF_ACC_TB_TERM_REASON_BUSY,
    /// Network Congestion
    APP_GAF_ACC_TB_TERM_REASON_CONGESTION,
    /// Call ended from the Client
    APP_GAF_ACC_TB_TERM_REASON_CLI_END,
    /// No service
    APP_GAF_ACC_TB_TERM_REASON_NO_SVC,
    /// No answer
    APP_GAF_ACC_TB_TERM_REASON_NO_ANSWER,
    /// Unspecified
    APP_GAF_ACC_TB_TERM_REASON_UNSPECIFIED,

    APP_GAF_ACC_TB_TERM_REASON_MAX,
};

/// Structure for #APP_GAF_ACC_APP_GAF_TB_CALL_JOIN request indication message
typedef struct app_gaf_acc_tbs_call_join_req_ind
{
    /// Request Indication code (see #acc_tbs_msg_req_ind_codes enumeration)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Bearer local index
    uint8_t bearer_lid;
    /// Number of calls
    uint8_t nb_calls;
    /// List of call indexes
    uint8_t call_ids[__ARRAY_EMPTY];
} app_gaf_acc_tbs_call_join_req_ind_t;

/// Structure for ARC_AICS_STATE indication message
typedef struct
{
    /// Indication code (@see enum arc_aics_ind_code)
    ///  - ARC_AICS_STATE
    uint16_t ind_code;
    /// Input local index
    uint8_t input_lid;
    /// Gain
    int8_t gain;
    /// Gain Mode
    uint8_t gain_mode;
    /// Mute
    uint8_t mute;
} app_gaf_arc_aics_state_ind_t;

/// Structure for ARC_MICS_MUTE indication message
typedef struct
{
    /// Indication code (@see enum arc_mics_ind_code)
    ///  - ARC_MICS_MUTE
    uint16_t ind_code;
    /// Mute value
    uint8_t mute;
} app_gaf_arc_mics_mute_ind_t;

/// Structure for ARC_MICC_MUTE indication message
typedef struct
{
    /// Indication code (@see enum arc_micc_ind_code)
    ///  - ARC_MICC_MUTE
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Mute value
    uint8_t mute;
} app_gaf_arc_micc_mute_ind_t;

/// Structure for ARC_VCS_VOLUME indication message
typedef struct
{
    ///  Connection local index
    uint16_t con_lid;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
    /// Change counter
    uint8_t change_cnt;
    /// Reason
    uint8_t reason;
} app_gaf_arc_vcs_volume_ind_t;

/// Structure for ARC_VCS_FLAGS indication message
typedef struct
{
    /// Indication code (@see enum arc_vcs_ind_code)
    ///  - ARC_VCS_FLAGS
    uint16_t con_lid;
    /// Volume Flags
    uint8_t flags;
} app_gaf_arc_vcs_flags_ind_t;

// @arc_vcs_bond_ind
typedef struct
{
    /// Indication code (shall be set to #ARC_VCS_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
    // Characteristic type
    uint8_t char_type;
} app_gaf_arc_vcs_bond_data_ind_t;

/// Structure for ARC_VOCS_SET_LOCATION request indication message
typedef struct
{
    /// Request Indication code (@see enum arc_vocs_req_ind_code)
    ///  - ARC_VOCS_SET_LOCATION
    uint16_t req_ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Audio location
    uint8_t location;
} app_gaf_arc_vocs_set_location_req_ind_t;

/// Structure for ARC_VOCS_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum arc_vocs_ind_code)
    ///  - ARC_VOCS_BOND_DATA
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} app_gaf_arc_vocs_cfg_ind_t;

/// Structure for ARC_VOCS_OFFSET indication message
typedef struct
{
    /// Indication code (@see enum arc_vocs_ind_code)
    ///  - ARC_VOCS_OFFSET
    uint16_t ind_code;
    /// Output local index
    uint8_t output_lid;
    /// Offset
    int16_t offset;
} app_gaf_arc_vocs_offset_ind_t;

/// SIRK
typedef struct app_gaf_csis_sirk
{
    /// SIRK
    uint8_t sirk[APP_GAF_CSIS_SIRK_LEN_VALUE];
} app_gaf_csis_sirk_t;

/// RSI
typedef struct app_gaf_csis_rsi
{
    /// RSI
    uint8_t rsi[APP_GAF_CSIS_RSI_LEN];
} app_gaf_csis_rsi_t;

/// LTK
typedef struct app_gaf_csis_ltk
{
    /// Indicate if LTK is obtained
    bool obtained;
    /// LTK
    uint8_t ltk[16];
} app_gaf_csis_ltk_t;

/// Structure for CSISM_LOCK indication message
typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// New lock state
    uint8_t lock;
    /// Connection local index of connection for which Coordinated Set has been locked
    /// or was locked
    uint8_t con_lid;
    /// Reason why Coordinated Set is not locked anymore
    uint8_t reason;
} app_gaf_atc_csism_lock_ind_t;

/// Structure for CSISM_AUTHORIZATION request indication message
typedef struct
{
    /// Request indication code
    uint16_t req_ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
} app_gaf_atc_csism_ltk_req_ind_t;

typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Client configuration bit field
    uint8_t cli_cfg_bf;
} app_gaf_atc_csism_bond_data_ind_t;

/// Structure CSISM_ADD/CSISM_SET_SIRK/CSISM_UPDATE_PSRI command complete event
typedef struct
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Coordinated Set local index
    uint8_t set_lid;
} app_gaf_atc_csism_cmp_evt_t;

/// Structure for CSISM_RSI indication message
typedef struct
{
    /// Indication code
    uint16_t ind_code;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// RSI value
    app_gaf_csis_rsi_t rsi;
} app_gaf_atc_csism_rsi_ind_t;

typedef struct
{
    uint16_t ind_code;
    uint8_t con_lid;
    uint8_t char_type;
} app_gaf_atc_csism_ntf_sent_t;

typedef struct
{
    uint16_t ind_code;
    uint8_t con_lid;
    uint8_t char_type;
    uint8_t data_len;
    uint8_t data[APP_GAF_CSIS_SIRK_LEN_VALUE];
} app_gaf_atc_csism_read_rsp_sent_t;

/// Structure for response message
typedef struct app_gaf_csisc_rsp
{
    /// Request code
    uint16_t req_code;
    /// Status
    uint16_t status;
    /// Local index
    union
    {
        /// Local index
        uint8_t lid;
        /// Key local index
        uint8_t key_lid;
        /// Connection local index
        uint8_t con_lid;
    } lid;
} app_gaf_csisc_rsp_t;

/// Structure for CSISC_INFO indication message
typedef struct app_gaf_csisc_info_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Value
    union
    {
        /// Value
        uint8_t val;
        /// Number of devices in the Coordinated Set the Service device belongs to
        uint8_t size;
        /// Current lock state for the device
        uint8_t lock;
        /// Rank
        uint8_t rank;
    } val;
} app_gaf_csisc_info_ind_t;

/// Characteristic type values for Coordinated Set Identification Service
enum app_gaf_csis_char_type
{
    /// Set Identity Resolving Key characteristic
    APP_GAF_CSIS_CHAR_TYPE_SIRK = 0,
    /// Coordinated Set Size characteristic
    APP_GAF_CSIS_CHAR_TYPE_SIZE,
    /// Set Member Lock characteristic
    APP_GAF_CSIS_CHAR_TYPE_LOCK,

    APP_GAF_CSIS_CHAR_TYPE_NTF_MAX,

    /// Set Member Rank characteristic
    APP_GAF_CSIS_CHAR_TYPE_RANK = APP_GAF_CSIS_CHAR_TYPE_NTF_MAX,

    APP_GAF_CSIS_CHAR_TYPE_MAX,
};

/// Descriptor type values for Coordinated Set Identification Service
enum app_gaf_csis_desc_type
{
    /// Client Characteristic Configuration descriptor for Set Identity Resolving Key characteristic
    APP_GAF_CSIS_DESC_TYPE_CCC_SIRK = 0,
    /// Client Characteristic Configuration descriptor for Coordinated Set Size characteristic
    APP_GAF_CSIS_DESC_TYPE_CCC_SIZE,
    /// Client Characteristic Configuration descriptor for Set Member Lock characteristic
    APP_GAF_CSIS_DESC_TYPE_CCC_LOCK,

    APP_GAF_CSIS_DESC_TYPE_MAX,
};

/// Content description structure for Coordinated Set Identification Service
typedef struct app_gaf_csisc_csis_info
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// Characteristics description
    app_gaf_prf_char_t char_info[APP_GAF_CSIS_CHAR_TYPE_MAX];
    /// Descriptors description
    app_gaf_prf_desc_t desc_info[APP_GAF_CSIS_DESC_TYPE_MAX];
} app_gaf_csisc_csis_info_t;

/// Structure for CSISC_BOND_DATA indication message
typedef struct app_gaf_csisc_bond_data_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Set local index
    uint8_t set_lid;
    /// Content description of Coordinated Set Identification Service instance
    app_gaf_csisc_csis_info_t csis_info;
} app_gaf_csisc_bond_data_ind_t;

/// Structure command complete event
typedef struct app_gaf_csisc_cmp_evt
{
    /// Command code
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Local index
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Key local index
        uint8_t key_lid;
    } lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Type
    union
    {
        /// Type
        uint8_t type;
        /// Characteristic type
        uint8_t char_type;
    } type;
} app_gaf_csisc_cmp_evt_t;

/// Structure for CSISC_SIRK indication message
typedef struct app_gaf_csisc_sirk_ind
{
    /// Indication code
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Coordinated Set local index
    uint8_t set_lid;
    /// Key local index
    uint8_t key_lid;
    /// SIRK value
    app_gaf_csis_sirk_t sirk;
} app_gaf_csisc_sirk_ind_t;

/// Structure command complete event
typedef struct
{
    /// Request Indication code (see enum #dts_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Status
    uint16_t status;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_registerd_ind_t;

/// Structure for DTC_COC_CONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t tx_mtu;
    /// Maximum packet size that the peer on the link can receive
    uint16_t tx_mps;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// initial credits
    uint16_t initial_credits;
} app_gaf_dts_coc_connected_ind_t;

/// Structure for DTC_COC_DISCONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_coc_disconnected_ind_t;

/// Structure for DTC_COC_DATA indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} app_gaf_dts_coc_data_ind_t;

typedef struct
{
    /// Command code (see enum #ots_cmd_codes)
    uint16_t cmd_code;
    /// Status
    uint16_t status;
    /// Connection local index
    uint8_t con_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_cmp_evt_t;

typedef struct
{
    /// Request Indication code (see enum #ots_msg_req_ind_codes)
    uint16_t req_ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Token value to return in the confirmation
    uint16_t token;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t peer_max_sdu;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dts_coc_connect_req_ind_t;

/// CIS stream state
typedef enum
{
    APP_GAF_CIS_STREAM_IDLE             = 0,
    APP_GAF_CIS_STREAM_CODEC_CONFIGURED = 1,
    APP_GAF_CIS_STREAM_QOS_CONFIGURED   = 2,
    APP_GAF_CIS_STREAM_ENABLING         = 3,
    APP_GAF_CIS_STREAM_DISABLING        = 4,
    APP_GAF_CIS_STREAM_STREAMING        = 5,
    APP_GAF_CIS_STREAM_RELEASING        = 6,

} APP_GAF_CIS_STREAM_STATE_E;

/// Structure for APP_GAF_CIS_CLI_STREAM_STATE_UPDATED and
/// APP_GAF_CIS_SRV_STREAM_STATE_UPDATED event indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE local index
    uint8_t ase_lid;
    /// Former state
    APP_GAF_CIS_STREAM_STATE_E formerState;
    /// Current state
    APP_GAF_CIS_STREAM_STATE_E currentState;
} app_gaf_cis_stream_state_updated_ind_t;

// @bap_uc_srv_bond_data_ind_t
typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_BOND_DATA)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    uint8_t char_type;
    /// Client configuration bit field for Audio Stream Control Service\n
    /// Each bit correspond to a characteristic in the range [0, BAP_UC_CHAR_TYPE_ASE[
    uint8_t cli_cfg_bf;
    /// Client configuration bit field for instances of the ASE characteristics\n
    /// Each bit correspond to an instance of the ASE characteristic
    uint16_t ase_cli_cfg_bf;
} app_gaf_bap_uc_srv_bond_data_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_CIS_REJECTED)
    uint16_t ind_code;
    /// Connection handle of Connected Isochronous Stream
    uint16_t con_hdl;
    /// Reject reason
    uint8_t error;
} app_gaf_bap_uc_srv_cis_rejected_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_CIG_TERMINATED)
    uint16_t ind_code;
    /// CIG ID
    uint8_t cig_id;
    /// Group local index
    uint8_t group_lid;
    /// Stream local index
    uint8_t stream_lid;
    /// Cig terminated reason
    uint8_t reason;
} app_gaf_bap_uc_srv_cig_terminated_ind_t;

typedef struct
{
    /// Indication code (set to #BAP_UC_SRV_ASE_NTF_VALUE)
    uint16_t ind_code;
    /// Opcode of the client-initiated ASE Control operation causing this response
    uint8_t opcode;
    /// Total number of ASEs the server is providing a response for
    uint8_t nb_ases;
    /// ASE Local ID for this ASE
    uint8_t ase_lid;
    /// Response code
    uint8_t rsp_code;
    /// Reason
    uint8_t reason;
} app_gaf_bap_uc_srv_ase_ntf_value_ind_t;

/// Structure for response message
typedef struct
{
    /// Request code (@see enum bap_uc_cli_req_code)
    uint16_t req_code;
    /// Status
    uint16_t status;
    union
    {
        /// Local index
        uint8_t lid;
        /// Connection local index
        uint8_t con_lid;
        /// Group local index
        uint8_t grp_lid;
    } lid;
    /// ASE local index
    uint8_t ase_lid;
} app_gaf_uc_cli_rsp_t;

/// see @bap_uc_cli_cis_state_ind_t
typedef struct
{
    /// Indication code (set to #BAP_UC_CLI_CIS_STATE)
    uint16_t ind_code;
    /// Stream local index
    uint8_t stream_lid;
    /// Connection local index of LE connection the CIS is bound with
    uint8_t con_lid;
    /// ASE local index for Sink direction
    uint8_t ase_lid_sink;
    /// ASE local index for Source direction
    uint8_t ase_lid_src;
    /// Group local index
    uint8_t grp_lid;
    /// CIS ID
    uint8_t cis_id;
    /// Connection handle allocated for the CIS by Controller
    /// GAP_INVALID_CONHDL indicates that CIS has been lost
    uint16_t conhdl;
    /// Event that has triggered update of CIS state (see #bap_uc_cli_cis_event enumeration)
    uint8_t event;
    /// Group configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_ug_config_t cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    app_gaf_iap_us_config_t cis_config;
} app_gaf_uc_cli_cis_state_ind_t;

/// Structure for cis stream started indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascc_cis_stream_started_t;

/// Structure for cis stream stopped indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t ase_lid;
    uint16_t cis_hdl;
    uint8_t direction;
} app_gaf_ascc_cis_stream_stopped_t;

/// Structure for BAP_CAPA_CLI_RECORD indication message
typedef struct
{
    /// Indication code (shall be set to #BAP_CAPA_CLI_RECORD)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// PAC local index
    uint8_t pac_lid;
    /// Record local index
    uint8_t record_lid;
    /// Number of records
    uint8_t nb_records;
    /// Codec ID
    app_gaf_codec_id_t codec_id;
    /// Codec Capabilities parameters structure
    app_gaf_bap_capa_param_t param;
    /// Codec Capabilities Metadata parameters structure
    app_gaf_bap_capa_metadata_param_t param_metadata;
    /// Length of additional Codec Capabilities
    uint8_t add_capa_len;
    /// Length of additional Metadata
    uint8_t add_metadata_len;
    /// Additional Codec Capabilities (in LTV format) followed by additional Metadata (in LTV format)\n
    /// Length of array is add_capa_len + add_metadata_len
    uint8_t val[__ARRAY_EMPTY];
} app_gaf_capa_cli_record_ind_t;

typedef struct
{
    uint8_t con_lid;
    uint16_t cmd_code;
} app_gaf_capa_operation_cmd_ind_t;

/// Structure for ACC_MCS_CONTROL request indication message
typedef struct
{
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Operation code
    uint8_t opcode;
} app_gaf_mcs_control_req_ind;

/// Structure for #ACC_MCS_GET request indication message
typedef struct
{
    /// Request Indication code (shall be set to #ACC_MCS_GET)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Token
    uint16_t token;
    /// Offset
    uint16_t offset;
    /// Maximum length
    uint16_t length;
} app_gaf_mcs_get_req_ind_t;

/// Structure for #ACC_MCS_GET request indication message
typedef struct
{
    /// Request Indication code (shall be set to #ACC_MCS_SET)
    uint16_t req_ind_code;
    /// Media local index
    uint8_t media_lid;
    /// Connection local index
    uint8_t con_lid;
    /// Characteristic type
    uint8_t char_type;
    /// Union
    union
    {
        /// Additional parameter
        uint32_t param;
        /// Track position offset
        int32_t track_pos_offset;
        /// Playback speed
        int8_t playback_speed;
        /// Playing order
        uint8_t playing_order;
    } param;
} app_gaf_mcs_set_req_ind_t;

/// Structure for ARC_VOCC_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum arc_vocc_ind_code)
    ///  - ARC_VOCC_BOND_DATA
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
} app_gaf_arc_vocc_bond_data_ind_t;

/// Structure for ARC_VOCC_VALUE indication message
typedef struct
{
    /// Indication code (@see enum arc_vocc_ind_code)
    ///  - ARC_VOCC_VALUE
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Output local index
    uint8_t output_lid;
    /// Characteristic type
    uint8_t char_type;
    union
    {
        /// Value
        uint32_t val;
        /// Volume offset
        int16_t offset;
        /// Audio location
        uint32_t location;
    } u;
} app_gaf_arc_vocc_value_ind_t;

/// Structure for BAP_CAPA_CLI_CONTEXT indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Context type (@see enum bap_capa_context_type)
    uint8_t context_type;
    /// Context type bit field for Sink direction
    uint16_t context_bf_sink;
    /// Context type bit field for Source direction
    uint16_t context_bf_src;
} app_gaf_capa_cli_context_ind_t;

/// Structure for BAP_CAPA_CLI_LOCATION indication message
typedef struct
{
    /// Connection local index
    uint8_t con_lid;
    /// Direction (@see enum gaf_direction)
    uint8_t direction;
    /// Location bit field
    uint32_t location_bf;
} app_gaf_capa_cli_location_ind_t;

typedef struct
{
    /// Characteristic value handle
    uint16_t val_hdl;
    /// Client Characteristic Configuration descriptor handle
    uint16_t desc_hdl;
    /// ASE ID
    uint8_t ase_id;
} app_gaf_uc_cli_ascs_char_t;

/// Audio Stream Control Service content description structure
typedef struct
{
    /// Service description
    app_gaf_prf_svc_t svc_info;
    /// Number of discovered Sink ASE characteristics
    uint8_t nb_ases_sink;
    /// Number of discovered Source ASE characteristics
    uint8_t nb_ases_src;
    /// Characteristic information structures
    app_gaf_uc_cli_ascs_char_t char_info[__ARRAY_EMPTY];
} app_gaf_uc_cli_ascs_t;

/// Structure for BAP_UC_CLI_BOND_DATA indication message
typedef struct
{
    /// Indication code (@see enum bap_uc_cli_ind_code)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Audio Stream Control Service content description
    app_gaf_uc_cli_ascs_t ascs_info;
} app_gaf_uc_cli_bond_data_ind_t;

/// Structure for ARC_VCC_VOLUME indication message
typedef struct
{
    /// Indication code (@see enum arc_vcc_ind_code)
    ///  - ARC_VCC_VOLUME
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Volume
    uint8_t volume;
    /// Mute
    uint8_t mute;
    /// Change counter
    uint8_t change_cnt;
} app_gaf_vcc_volume_ind_t;

/// Structure for CSISC_DISCOVER_SERVER_COMPLETE indication message
typedef struct
{
    uint8_t con_lid;
    uint8_t result;
} app_gaf_csisc_discover_cmp_ind_t;

/// Structure for DTC_COC_CONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Maximum SDU size that the peer on the link can receive
    uint16_t tx_mtu;
    /// Maximum packet size that the peer on the link can receive
    uint16_t tx_mps;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dtc_coc_connected_ind_t;

/// Structure for DTC_COC_DISCONNECTED indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// Disconnection reason
    uint16_t reason;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
} app_gaf_dtc_coc_disconnected_ind_t;

/// Structure for DTC_COC_DATA indication message
typedef struct
{
    /// Indication code (see enum #dtc_msg_ind_codes)
    uint16_t ind_code;
    /// Connection local index
    uint8_t con_lid;
    /// SDU data length
    uint16_t length;
    /// Connected L2CAP channel local index
    uint8_t chan_lid;
    /// Simplified Protocol/Service Multiplexer
    uint16_t spsm;
    /// SDU data
    uint8_t sdu[__ARRAY_EMPTY];
} app_gaf_dtc_coc_data_ind_t;

typedef enum
{
    APP_GAF_ASCS_MODULE             = 0,
    APP_GAF_PACS_MODULE             = 1,
    APP_GAF_BIS_SCAN_MODULE         = 2,
    APP_GAF_BIS_SOURCE_MODULE       = 3,
    APP_GAF_BIS_ASSIST_MODULE       = 4,
    APP_GAF_BIS_SINK_MODULE         = 5,
    APP_GAF_BIS_DELEG_MODULE        = 6,
    APP_GAF_MCC_MODULE              = 7,
    APP_GAF_TBC_MODULE              = 8,
    APP_GAF_AICS_MODULE             = 9,
    APP_GAF_MICS_MODULE             = 10,
    APP_GAF_VCS_MODULE              = 11,
    APP_GAF_VOCS_MODULE             = 12,
    APP_GAF_CSISM_MODULE            = 13,
    APP_GAF_BC_SCAN_STATE_MODULE    = 14,
    APP_GAF_DTC_MODULE              = 15,
    APP_GAF_DTS_MODULE              = 16,

    APP_GAF_MAX_MODULE,
} app_gaf_evt_module_e;

/// earbuds event on GAF layer
// event only can use low 12 bits
typedef enum
{
    APP_GAF_EVENT_FIRST = 0x000,

    // ASCS Events
    APP_GAF_ASCS_CIS_ESTABLISHED_IND        = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x0),
    APP_GAF_ASCS_CIS_DISCONNETED_IND        = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x1),
    APP_GAF_ASCS_CIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x2),
    APP_GAF_ASCS_CIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x3),
    APP_GAF_ASCS_CONFIGURE_CODEC_RI         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x4),
    APP_GAF_ASCS_ENABLE_RI                  = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x5),
    APP_GAF_ASCS_UPDATE_METADATA_RI         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x6),
    APP_GAF_ASCS_RELEASE_RI                 = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x7),
    APP_GAF_ASCS_CLI_STREAM_STATE_UPDATED   = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x8),
    APP_GAF_ASCS_ISO_LINK_QUALITY_EVT       = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0x9),
    APP_GAF_ASCS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xA),
    APP_GAF_ASCS_CIS_REJECTED_IND           = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xB),
    APP_GAF_ASCS_CIG_TERMINATED_IND         = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xC),
    APP_GAF_ASCS_ASE_NTF_VALUE_IND          = GAF_BUILD_ID(APP_GAF_ASCS_MODULE, 0xD),

    // PACS Event
    APP_GAF_PACS_LOCATION_SET_IND           = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x0),
    APP_GAF_PACS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x1),
    APP_GAF_PACS_CCCD_WRITTEN_IND           = GAF_BUILD_ID(APP_GAF_PACS_MODULE, 0x2),

    // BIS SCAN Events
    APP_GAF_SCAN_TIMEOUT_IND                = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x0),
    APP_GAF_SCAN_REPORT_IND                 = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x1),
    APP_GAF_SCAN_PA_REPORT_IND              = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x2),
    APP_GAF_SCAN_PA_ESTABLISHED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x3),
    APP_GAF_SCAN_PA_TERMINATED_IND          = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x4),
    APP_GAF_SCAN_GROUP_REPORT_IND           = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x5),
    APP_GAF_SCAN_SUBGROUP_REPORT_IND        = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x6),
    APP_GAF_SCAN_STREAM_REPORT_IND          = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x7),
    APP_GAF_SCAN_BIGINFO_REPORT_IND         = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x8),
    APP_GAF_SCAN_PA_SYNC_REQ_IND            = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0x9),
    APP_GAF_SCAN_PA_TERMINATED_REQ_IND      = GAF_BUILD_ID(APP_GAF_BIS_SCAN_MODULE, 0xa),

    // BIS Source Events
    APP_GAF_SRC_BIS_PA_ENABLED_IND          = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X0),
    APP_GAF_SRC_BIS_PA_DISABLED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X1),
    APP_GAF_SRC_BIS_SRC_ENABLED_IND         = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X2),
    APP_GAF_SRC_BIS_SRC_DISABLED_IND        = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0X3),
    APP_GAF_SRC_BIS_STREAM_STARTED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0x4),
    APP_GAF_SRC_BIS_STREAM_STOPPED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SOURCE_MODULE, 0x5),

    //// BIS Assist Events
    APP_GAF_ASSIST_SCAN_TIMEOUT_IND         = GAF_BUILD_ID(APP_GAF_BIS_ASSIST_MODULE, 0x0),
    APP_GAF_ASSIST_SOLICITATION_IND         = GAF_BUILD_ID(APP_GAF_BIS_ASSIST_MODULE, 0x1),
    APP_GAF_ASSIST_SOURCE_STATE_IND         = GAF_BUILD_ID(APP_GAF_BIS_ASSIST_MODULE, 0x2),
    APP_GAF_ASSIST_BCAST_CODE_RI            = GAF_BUILD_ID(APP_GAF_BIS_ASSIST_MODULE, 0x3),
    APP_GAF_ASSIST_BOND_DATA_IND            = GAF_BUILD_ID(APP_GAF_BIS_ASSIST_MODULE, 0X4),

    // BIS Sink Events
    APP_GAF_SINK_BIS_STATUS_IND             = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x0),
    APP_GAF_SINK_BIS_SINK_ENABLED_IND       = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x1),
    APP_GAF_SINK_BIS_SINK_DISABLED_IND      = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x2),
    APP_GAF_SINK_BIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x3),
    APP_GAF_SINK_BIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_BIS_SINK_MODULE, 0x4),

    // BIS Delegator Events
    APP_GAF_DELEG_SOLICITE_STARTED_IND      = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x0),
    APP_GAF_DELEG_SOLICITE_STOPPED_IND      = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x1),
    APP_GAF_DELEG_REMOTE_SCAN_STARTED_IND   = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x2),
    APP_GAF_DELEG_REMOTE_SCAN_STOPPED_IND   = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x3),
    APP_GAF_DELEG_SOURCE_ADD_RI             = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x4),
    APP_GAF_DELEG_SOURCE_REMOVE_RI          = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x5),
    APP_GAF_DELEG_SOURCE_UPDATE_RI          = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x6),
    APP_GAF_DELEG_BOND_DATA_IND             = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x7),
    APP_GAF_DELEG_PREF_BIS_SYNC_RI          = GAF_BUILD_ID(APP_GAF_BIS_DELEG_MODULE, 0x8),

    // MCC Events
    APP_GAF_MCC_SVC_DISCOVERYED_IND         = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x0),
    APP_GAF_MCC_TRACK_CHANGED_IND           = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x1),
    APP_GAF_MCC_MEDIA_VALUE_IND             = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x2),
    APP_GAF_MCC_MEDIA_VALUE_LONG_IND        = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x3),
    APP_GAF_MCC_SVC_CHANGED_IND             = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x4),
    APP_GAF_MCC_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x5),
    APP_GAF_MCC_SET_CFG_CMP_IND             = GAF_BUILD_ID(APP_GAF_MCC_MODULE, 0x6),

    // TBC Events
    APP_GAF_TBC_SVC_DISCOVERYED_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x0),
    APP_GAF_TBC_CALL_STATE_IND              = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x1),
    APP_GAF_TBC_CALL_STATE_LONG_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x2),
    APP_GAF_TBC_CALL_VALUE_IND              = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x3),
    APP_GAF_TBC_CALL_VALUE_LONG_IND         = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x4),
    APP_GAF_TBC_SVC_CHANGED_IND             = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x5),
    APP_GAF_TBC_CALL_ACTION_RESULT_IND      = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x6),
    APP_GAF_TBC_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_TBC_MODULE, 0x7),

    // AICS Event
    APP_GAF_AICS_STATE_IND                  = GAF_BUILD_ID(APP_GAF_AICS_MODULE, 0x0),
    APP_GAF_AICS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_AICS_MODULE, 0x1),

    // MICS Event
    APP_GAF_MICS_MUTE_IND                   = GAF_BUILD_ID(APP_GAF_MICS_MODULE, 0x0),
    APP_GAF_MICS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_MICS_MODULE, 0x1),

    // VCS Events
    APP_GAF_VCS_VOLUME_IND                  = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x0),
    APP_GAF_VCS_FLAGS_IND                   = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x1),
    APP_GAF_VCS_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_VCS_MODULE, 0x2),

    // VOCS Events
    APP_GAF_VOCS_LOCATION_SET_RI            = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x0),
    APP_GAF_VOCS_OFFSET_IND                 = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x1),
    APP_GAF_VOCS_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_VOCS_MODULE, 0x2),

    // CSISM Events
    APP_GAF_CSISM_LOCK_IND                  = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x0),
    APP_GAF_CSISM_LTK_RI                    = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x1),
    APP_GAF_CSISM_NEW_RSI_GENERATED_IND     = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x2),
    APP_GAF_CSISM_BOND_DATA_IND             = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x3),
    APP_GAF_CSISM_NTF_SENT_IND              = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x4),
    APP_GAF_CSISM_READ_RSP_SENT_IND         = GAF_BUILD_ID(APP_GAF_CSISM_MODULE, 0x5),

    // BIS scan state update
    APP_BAP_BC_SCAN_STATE_IDLE_IND          = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x0),
    APP_BAP_BC_SCAN_STATE_SCANNING_IND      = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x1),
    APP_BAP_BC_SCAN_STATE_SYNCHRONIZING_IND = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x2),
    APP_BAP_BC_SCAN_STATE_SYNCHRONIZED_IND  = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x3),
    APP_BAP_BC_SCAN_STATE_STREAMING_IND     = GAF_BUILD_ID(APP_GAF_BC_SCAN_STATE_MODULE, 0x4),

    // DTS event
    APP_DTS_COC_REGISTERED_IND                  = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x0),
    APP_DTS_COC_CONNECTED_IND                   = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x1),
    APP_DTS_COC_DISCONNECTED_IND                = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x2),
    APP_DTS_COC_DATA_IND                        = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x3),
    APP_DTS_COC_SEND_IND                        = GAF_BUILD_ID(APP_GAF_DTS_MODULE, 0x4),

    APP_GAF_EVENT_LAST                      = 0xFFFF
} app_gaf_evt_e;

/// mobile event on GAF layer
#ifdef AOB_MOBILE_ENABLED

typedef enum
{
    APP_GAF_ASCC_MODULE         = 0,
    APP_GAF_PACC_MODULE         = 1,
    APP_GAF_MCS_MODULE          = 2,
    APP_GAF_TBS_MODULE          = 3,
    APP_GAF_AICC_MODULE         = 4,
    APP_GAF_MICC_MODULE         = 5,
    APP_GAF_VCC_MODULE          = 6,
    APP_GAF_VOCC_MODULE         = 7,
    APP_GAF_CSISC_MODULE        = 8,
    APP_GAF_MAX_MOBILE_EVENT_MODULE,
} app_gaf_mobile_evt_module_e;

// event only can use low 12 bits
typedef enum
{
    APP_GAF_EVENT_MOBILE_FIRST              = 0x0000,

    // ASCC Events
    APP_GAF_ASCC_CIS_ESTABLISHED_IND        = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x0),
    APP_GAF_ASCC_CIS_DISCONNETED_IND        = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x1),
    APP_GAF_ASCC_CIS_STREAM_STARTED_IND     = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x2),
    APP_GAF_ASCC_CIS_STREAM_STOPPED_IND     = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x3),
    APP_GAF_ASCC_CIS_STREAM_STATE_UPDATED   = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x4),
    APP_GAF_ASCC_ASE_FOUND_IND              = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x5),
    APP_GAF_ASCC_CIS_GRP_STATE_IND          = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x6),
    APP_GAF_ASCC_CMD_CMP_IND                = GAF_BUILD_ID(APP_GAF_ASCC_MODULE, 0x7),

    APP_GAF_PACC_PAC_RECORD_IND             = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x0),
    APP_GAF_PACC_LOCATION_IND               = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x1),
    APP_GAF_PACC_DISCOVERY_CMP_IND          = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x2),
    APP_GAF_PACC_CONTEXT_IND                = GAF_BUILD_ID(APP_GAF_PACC_MODULE, 0x3),

    APP_GAF_MCS_CONTROL_REQ_IND             = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x0),
    APP_GAF_MCS_SET_OBJ_ID_RI               = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x1),
    APP_GAF_MCS_CONTROL_RI                  = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x2),
    APP_GAF_MCS_SEARCH_RI                   = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x3),
    APP_GAF_MCS_GET_RI                      = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x4),
    APP_GAF_MCS_GET_POSITION_RI             = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x5),
    APP_GAF_MCS_SET_RI                      = GAF_BUILD_ID(APP_GAF_MCS_MODULE, 0x6),

    APP_GAF_TBS_REPORT_INTV_IND             = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x0),
    APP_GAF_TBS_GET_RI                      = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x1),
    APP_GAF_TBS_CALL_OUTGOING_RI            = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x2),
    APP_GAF_TBS_CALL_ACTION_RI              = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x3),
    APP_GAF_TBS_CALL_JOIN_RI                = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x4),
    APP_GAF_TBS_BOND_DATA_IND               = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x5),
    APP_GAF_TBS_CALL_ACTION_REQ_RSP         = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x6),
    APP_GAF_TBS_CALL_OUTGOING_REQ_RSP       = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x7),
    APP_GAF_TBS_CALL_INCOMING_REQ_RSP       = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x8),
    APP_GAF_TBS_CALL_JOIN_REQ_RSP           = GAF_BUILD_ID(APP_GAF_TBS_MODULE, 0x9),

    APP_GAF_AICC_GAIN_IND                   = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x0),
    APP_GAF_AICC_GAIN_PROP_IND              = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x1),
    APP_GAF_AICC_VALUE_IND                  = GAF_BUILD_ID(APP_GAF_AICC_MODULE, 0x2),

    APP_GAF_MICC_MUTE_IND                   = GAF_BUILD_ID(APP_GAF_MICC_MODULE, 0x0),

    // VCC Events
    APP_GAF_VCC_VOLUME_IND                  = GAF_BUILD_ID(APP_GAF_VCC_MODULE, 0x0),

    // VOCC Events
    APP_GAF_VOCC_BOND_DATA_IND              = GAF_BUILD_ID(APP_GAF_VOCC_MODULE, 0x0),
    APP_GAF_VOCC_VALUE_IND                  = GAF_BUILD_ID(APP_GAF_VOCC_MODULE, 0x1),

    // CSISC Events
    APP_GAF_CSISC_BOND_IND                  = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x0),
    APP_GAF_CSISC_SIRK_VALUE_IND            = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x1),
    APP_GAF_CSISC_CHAR_VALUE_RSULT_IND      = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x2),
    APP_GAF_CSISC_PSRI_RESOLVE_RESULT_IND   = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x3),
    APP_GAF_CSISC_SERVER_INIT_DONE_CMP_IND  = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x4),
    APP_GAF_CSISC_SIRK_ADD_RESULT_IND       = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x5),
    APP_GAF_CSISC_SIRK_REMOVE_RESULT_IND    = GAF_BUILD_ID(APP_GAF_CSISC_MODULE, 0x6),

    // DTC event
    APP_DTC_COC_CONNECTED_IND                   = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x0),
    APP_DTC_COC_DISCONNECTED_IND                = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x1),
    APP_DTC_COC_DATA_IND                        = GAF_BUILD_ID(APP_GAF_DTC_MODULE, 0x2),

    APP_GAF_EVENT_MOBILE_LAST               = 0xFFFF
} app_gaf_mobile_evt_e;

#endif

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_GAF_DEFINE_H_

/// @} APP_GAF
