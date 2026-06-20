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
#ifndef __BES_AOB_API_H__
#define __BES_AOB_API_H__
#include "ble_aob_common.h"
#include "nvrecord_extension.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif


/// SDU Buffer structure
typedef struct
{
    /// Time_Stamp
    uint32_t        time_stamp;
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    /// length of the ISO SDU (in bytes)
    uint16_t        sdu_length;
    /// Reception status (@see enum hci_iso_pkt_stat_flag)
    uint8_t         status;
    /// SDU
    uint8_t         *sdu;
} bes_ble_dp_itf_iso_buffer_t;


#if BLE_AUDIO_ENABLED

typedef enum
{
    BES_BLE_AUDIO_TWS_MASTER,
    BES_BLE_AUDIO_TWS_SLAVE,
    BES_BLE_AUDIO_MOBILE,
    BES_BLE_AUDIO_ROLE_UNKNOW,
} BES_BLE_AUDIO_TWS_ROLE_E;

/// ASE Direction, sync @see app_gaf_direction
typedef enum bes_gaf_direction
{
    /// Sink direction
    BES_BLE_GAF_DIRECTION_SINK = 0,
    /// Source direction
    BES_BLE_GAF_DIRECTION_SRC,

    BES_BLE_GAF_DIRECTION_MAX,
} bes_gaf_direction_t;

// Frame_Duration   #app_gaf_bap_frame_dur
enum bes_ble_gaf_bap_frame_duration
{
    BES_BLE_GAF_BAP_FRAME_DURATION_7_5MS    = 0x00,
    BES_BLE_GAF_BAP_FRAME_DURATION_10MS     = 0x01,
    BES_BLE_GAF_BAP_FRAME_DURATION_5MS      = 0x02,
    BES_BLE_GAF_BAP_FRAME_DURATION_2_5MS    = 0x03,
    BES_BLE_GAF_BAP_FRAME_DURATION_MAX,
};

/// Audio Location Bitfield
typedef enum
{
    BES_BLE_LOC_FRONT_LEFT               = 0x00000001,
    BES_BLE_LOC_FRONT_RIGHT              = 0x00000002,
    BES_BLE_LOC_FRONT_CENTER             = 0x00000004,
    BES_BLE_LOC_LOW_FREQ_EFFECTS_1       = 0x00000008,
    BES_BLE_LOC_BACK_LEFT                = 0x00000010,
    BES_BLE_LOC_BACK_RIGHT               = 0x00000020,
    BES_BLE_LOC_FRONT_LEFT_OF_CENTER     = 0x00000040,
    BES_BLE_LOC_FRONT_RIGHT_OF_CENTER    = 0x00000080,
    BES_BLE_LOC_BACK_CENTER              = 0x00000100,
    BES_BLE_LOC_LOW_FREQ_EFFECTS_2       = 0x00000200,
    BES_BLE_LOC_SIDE_LEFT                = 0x00000400,
    BES_BLE_LOC_SIDE_RIGHT               = 0x00000800,
    BES_BLE_LOC_TOP_FRONT_LEFT           = 0x00001000,
    BES_BLE_LOC_TOP_FRONT_RIGHT          = 0x00002000,
    BES_BLE_LOC_TOP_FRONT_CENTER         = 0x00004000,
    BES_BLE_LOC_TOP_CENTER               = 0x00008000,
    BES_BLE_LOC_TOP_BACK_LEFT            = 0x00010000,
    BES_BLE_LOC_TOP_BACK_RIGHT           = 0x00020000,
    BES_BLE_LOC_TOP_SIDE_LEFT            = 0x00040000,
    BES_BLE_LOC_TOP_SIDE_RIGHT           = 0x00080000,
    BES_BLE_LOC_TOP_BACK_CENTER          = 0x00100000,
    BES_BLE_LOC_BOTTOM_FRONT_CENTER      = 0x00200000,
    BES_BLE_LOC_BOTTOM_FRONT_LEFT        = 0x00400000,
    BES_BLE_LOC_BOTTOM_FRONT_RIGHT       = 0x00800000,
    BES_BLE_LOC_FRONT_LEFT_WIDE          = 0x01000000,
    BES_BLE_LOC_FRONT_RIGHT_WIDE         = 0x02000000,
    BES_BLE_LOC_LEFT_SURROUND            = 0x04000000,
    BES_BLE_LOC_RIGHT_SURROUND           = 0x08000000,

    BES_BLE_LOC_RFU                      = 0xF0000000,
} BES_BLE_GAF_LOCATION_BF_BIT_E;

typedef enum
{
    BES_BLE_GAF_SAMPLE_FREQ_8000    = 0x01,
    BES_BLE_GAF_SAMPLE_FREQ_11025,
    BES_BLE_GAF_SAMPLE_FREQ_16000,
    BES_BLE_GAF_SAMPLE_FREQ_22050,
    BES_BLE_GAF_SAMPLE_FREQ_24000,
    BES_BLE_GAF_SAMPLE_FREQ_32000,
    BES_BLE_GAF_SAMPLE_FREQ_44100,
    BES_BLE_GAF_SAMPLE_FREQ_48000,
    BES_BLE_GAF_SAMPLE_FREQ_88200,
    BES_BLE_GAF_SAMPLE_FREQ_96000,
    BES_BLE_GAF_SAMPLE_FREQ_176400,
    BES_BLE_GAF_SAMPLE_FREQ_192000,
    BES_BLE_GAF_SAMPLE_FREQ_384000,
} BES_BLE_GAF_SAMPLE_FREQ_E;

/// Frame Duration values
typedef enum
{
    /// Use 7.5ms Codec frames
    BES_BLE_GAF_FRAME_DUR_7_5MS = 0,
    /// Use 10ms Codec frames
    BES_BLE_GAFFRAME_DUR_10MS,
    /// Use 5ms Codec frames
    BES_BLE_GAF_FRAME_DUR_5MS,
    /// Use 2.5ms Codec frames
    BES_BLE_GAF_FRAME_DUR_2_5MS,
    /// Maximum value
    BES_BLE_GAF_FRAME_DUR_MAX
} BES_BLE_GAF_FRAME_DUR_E;

typedef enum
{
    // 8K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_26     = 26,
    // 8K__10MS
    // 16K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_30     = 30,
    // 16K__10MS
    BES_BLE_GAF_CODEC_FRAME_40     = 40,
    // 24K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_45     = 45,
    // 24K__10MS
    // 32K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_60     = 60,
    // 32K__10MS
    BES_BLE_GAF_CODEC_FRAME_80     = 80,
    // 44.1K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_97     = 97,
    // 44.1K__10MS
    BES_BLE_GAF_CODEC_FRAME_130    = 130,
    // 48K__7_5MS
    BES_BLE_GAF_CODEC_FRAME_75     = 75,
    BES_BLE_GAF_CODEC_FRAME_90     = 90,
    BES_BLE_GAF_CODEC_FRAME_117    = 117,
    // 48K__10MS
    BES_BLE_GAF_CODEC_FRAME_100    = 100,
    BES_BLE_GAF_CODEC_FRAME_120    = 120,
    BES_BLE_GAF_CODEC_FRAME_155    = 155,
} BES_BLE_GAF_CODEC_FRAME_E;

/// Context type bit field meaning
typedef enum
{
    /// Unspecified - Position
    BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED = 0,
    BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_UNSPECIFIED),
    /// Conversation between humans as, for example, in telephony or video calls
    BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL = 1,
    BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_CONVERSATIONAL),
    /// Media as, for example, in music, public radio, podcast or video soundtrack.
    BES_BLE_GAF_CONTEXT_TYPE_MEDIA = 2,
    BES_BLE_GAF_CONTEXT_TYPE_MEDIA_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_MEDIA),
    /// Audio associated with video gaming, for example gaming media, gaming effects, music and in-game voice chat
    BES_BLE_GAF_CONTEXT_TYPE_GAME = 3,
    BES_BLE_GAF_CONTEXT_TYPE_GAME_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_GAME),
    /// Instructional audio as, for example, in navigation, traffic announcements or user guidance
    BES_BLE_GAF_CONTEXT_TYPE_INSTRUCTIONAL = 4,
    BES_BLE_GAF_CONTEXT_TYPE_INSTRUCTIONAL_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_INSTRUCTIONAL),
    /// Man machine communication as, for example, with voice recognition or virtual assistant
    BES_BLE_GAF_CONTEXT_TYPE_MAN_MACHINE = 5,
    BES_BLE_GAF_CONTEXT_TYPE_MAN_MACHINE_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_MAN_MACHINE),
    /// Live audio as from a microphone where audio is perceived both through a direct acoustic path and through
    /// an LE Audio Stream
    BES_BLE_GAF_CONTEXT_TYPE_LIVE = 6,
    BES_BLE_GAF_CONTEXT_TYPE_LIVE_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_LIVE),
    /// Sound effects including keyboard and touch feedback;
    /// menu and user interface sounds; and other system sounds
    BES_BLE_GAF_CONTEXT_TYPE_SOUND_EFFECTS = 7,
    BES_BLE_GAF_CONTEXT_TYPE_SOUND_EFFECTS_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_SOUND_EFFECTS),
    /// Attention seeking audio as, for example, in beeps signalling arrival of a message or keyboard clicks
    BES_BLE_GAF_CONTEXT_TYPE_ATTENTION_SEEKING = 8,
    BES_BLE_GAF_CONTEXT_TYPE_ATTENTION_SEEKING_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_ATTENTION_SEEKING),
    /// Ringtone as in a call alert
    BES_BLE_GAF_CONTEXT_TYPE_RINGTONE = 9,
    BES_BLE_GAF_CONTEXT_TYPE_RINGTONE_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_RINGTONE),
    /// Immediate alerts as, for example, in a low battery alarm, timer expiry or alarm clock.
    BES_BLE_GAF_CONTEXT_TYPE_IMMEDIATE_ALERT = 10,
    BES_BLE_GAF_CONTEXT_TYPE_IMMEDIATE_ALERT_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_IMMEDIATE_ALERT),
    /// Emergency alerts as, for example, with fire alarms or other urgent alerts
    BES_BLE_GAF_CONTEXT_TYPE_EMERGENCY_ALERT = 11,
    BES_BLE_GAF_CONTEXT_TYPE_EMERGENCY_ALERT_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_EMERGENCY_ALERT),
    /// TV - Position
    /// Audio associated with a television program and/or with metadata conforming to the Bluetooth Broadcast TV
    /// profile
    BES_BLE_GAF_CONTEXT_TYPE_TV = 12,
    BES_BLE_GAF_CONTEXT_TYPE_TV_BIT = CO_BIT_MASK(BES_BLE_GAF_CONTEXT_TYPE_TV),
} BES_BLE_GAF_CONTEXT_E;

/// ASE State values
enum bes_ble_gaf_bap_uc_ase_state
{
    /// Idle
    BES_BLE_GAF_ASCS_ASE_STATE_IDLE = 0,
    /// Codec configured
    BES_BLE_GAF_ASCS_ASE_STATE_CODEC_CONFIGURED,
    /// QoS configured
    BES_BLE_GAF_ASCS_ASE_STATE_QOS_CONFIGURED,
    /// Enabling
    BES_BLE_GAF_ASCS_ASE_STATE_ENABLING,
    /// Streaming
    BES_BLE_GAF_ASCS_ASE_STATE_STREAMING,
    /// Disabling
    BES_BLE_GAF_ASCS_ASE_STATE_DISABLING,
    /// Releasing
    BES_BLE_GAF_ASCS_ASE_STATE_RELEASING,

    BES_BLE_GAF_ASCS_ASE_STATE_MAX,
};

/// Codec Type values
typedef enum bes_ble_gaf_codec_type
{
    /// LC3 Codec
    BES_BLE_GAF_CODEC_TYPE_LC3    = 0x06,
    /// ULL
    BES_BLE_GAF_CODEC_TYPE_ULL    = 0x08,
    /// Maximum SIG Codec
    BES_BLE_GAF_CODEC_TYPE_SIG_MAX,
    /// Vendor Specific Codec
    BES_BLE_GAF_CODEC_TYPE_VENDOR = 0xFF,
} BES_BLE_GAF_CODEC_TYPE_T;

/// ASCS ASE Structure @ see bes_ble_bap_ascs_ase_t
typedef struct bes_ble_bap_ascs_ase
{
    /// ASE local index
    uint8_t ase_lid;
    /// Connection local index
    uint8_t con_lid;
    /// ASE Instance local index
    uint8_t ase_id;
    /// ASE Direction
    bes_gaf_direction_t direction;
    /// ASE State
    uint8_t ase_state;
    /// Codec ID
    AOB_CODEC_ID_T codec_id;
    /// Pointer to Codec Configuration structure
    AOB_BAP_CFG_T *p_cfg;
    /// QoS Requirements
    AOB_BAP_QOS_REQ_T qos_req;
    /// CIG ID
    uint8_t cig_id;
    /// CIS ID
    uint8_t cis_id;
    /// CIS Connection Handle
    uint16_t cis_hdl;
    /// QoS Configuration structure
    AOB_BAP_QOS_CFG_T qos_cfg;
    /// Pointer to Metadata structure
    AOB_BAP_CFG_METADATA_T *p_metadata;
    /// CIG sync delay in us
    uint32_t cig_sync_delay;
    /// CIS sync delay in us
    uint32_t cis_sync_delay;
    /// iso interval in us
    uint32_t iso_interval_us;
    /// bn count from master to slave
    uint32_t bn_m2s;
    /// bn count from slave to master
    uint32_t bn_s2m;
    /// context type bring by enable req
    uint16_t init_context_bf;
} bes_ble_bap_ascs_ase_t;

/// QoS Configuration structure (short)
typedef struct bes_ble_bap_ascc_qos_cfg
{
    /// PHY
    uint8_t phy;
    /// Maximum number of retransmissions for each CIS Data PDU
    /// From 0 to 15
    uint8_t retx_nb;
    /// Maximum SDU size
    /// From 0 to 4095 bytes (0xFFF)
    uint16_t max_sdu_size;
    /// Presentation delay in microseconds
    uint32_t pres_delay_us;
} bes_ble_bap_ascc_qos_cfg_t;

/// ASCC ASE Information structure
typedef struct bes_ble_bap_ascc_ase
{
    /// ASE local index
    uint8_t ase_lid;
    /// ASE State
    uint8_t ase_state;
    /// Connection local index
    uint8_t con_lid;
    /// ASE instance index
    uint8_t ase_instance_idx;
    /// ASE Direction
    bes_gaf_direction_t direction;
    /// Codec ID
    AOB_CODEC_ID_T codec_id;
    /// Pointer to Codec Configuration
    AOB_BAP_CFG_T *p_cfg;
    /// QoS configuration
    bes_ble_bap_ascc_qos_cfg_t qos_cfg;
    /// Pointer to Metadata structure
    AOB_BAP_CFG_METADATA_T *p_metadata;
    /// CIS index
    uint8_t cis_id;
    /// CIS Connection Handle
    uint16_t cis_hdl;
    /// current audio stream type, @see gaf_bap_context_type_bf
    //uint16_t context_bf;
    /// CIG sync delay in us
    uint32_t cig_sync_delay;
    /// CIS sync delay in us
    uint32_t cis_sync_delay;
    /// iso interval in us
    uint32_t iso_interval_us;
    /// bn count from master to slave
    uint32_t bn_m2s;
    /// bn count from slave to master
    uint32_t bn_s2m;
} bes_ble_bap_ascc_ase_t;

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
} bes_ble_bap_bc_big_info_t;

/// sync see@aob_bis_src_big_param_t
typedef struct
{
    uint8_t                    bcast_id[3];
    // Indicate if streams are encrypted (!= 0) or not
    uint8_t                    encrypted;
    // Broadcast code. Meaningful only if encrypted parameter indicates that streams are encrypted
    uint8_t                    bcast_code[16];
    /// Number of Streams in the Broadcast Group. Cannot be 0
    uint8_t                    nb_streams;
    /// Number of Subgroups in the Broadcast Group. Cannot be 0
    uint8_t                    nb_subgroups;
    /// SDU interval in microseconds
    /// From 256us (0x00000100) to 1.048575s (0x000FFFFF)
    uint32_t                   sdu_intv_us;
    /// Maximum size of an SDU
    /// From 1 to 4095 bytes
    uint16_t                   max_sdu;
    // Audio output presentation delay in microseconds
    uint32_t                   pres_delay_us;
    // creat test big hci cmd, 0:creat BIG cmd, 1:creat BIG test cmd
    uint8_t                    test;

    //// test = 0, set this param
    struct {
        /// Maximum time (in milliseconds) between the first transmission of an SDU to the end of the last transmission
        /// of the same SDU
        /// From 0ms to 4.095s (0x0FFF)
        uint16_t                   max_tlatency_ms;
        /// Number of times every PDU should be transmitted
        /// From 0 to 15
        uint8_t                    rtn;
    } big_param;

    //// test = 1, set this param
    struct {
        /// ISO interval in multiple of 1.25ms. From 0x4 (5ms) to 0xC80 (4s)
        uint16_t                   iso_intv_frame;
        /// Number of subevents in each interval of each stream in the group
        uint8_t                    nse;
        /// Maximum size of a PDU
        uint8_t                    max_pdu;
        /// Burst number (number of new payload in each interval). From 1 to 7
        uint8_t                    bn;
        /// Number of times the scheduled payload is transmitted in a given event. From 0x1 to 0xF
        uint8_t                    irc;
        /// Isochronous Interval spacing of payloads transmitted in the pre-transmission subevents.
        /// From 0x00 to 0x0F
        uint8_t                    pto;
    } test_big_param;

    uint8_t adv_data_len;
    uint8_t *adv_data;
}bes_ble_bis_src_big_param_t;

/// sync see @aob_bis_src_subgrp_param_t
typedef struct
{
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// Codec ID
    uint8_t                  codec_id[AOB_COMMON_CODEC_ID_LEN];

    ///LTV cfg info
    // audio location bf, bit see@BES_BLE_GAF_LOCATION_BF_BIT_E
    uint32_t                 location_bf;
    /// Length of a codec frame in octets
    uint16_t                 frame_octet;
    /// Sampling Frequency (see #BES_BLE_GAF_SAMPLE_FREQ_E enumeration)
    uint8_t                  sampling_freq;
    /// Frame Duration (see #BES_BLE_GAF_FRAME_DUR_E enumeration)
    uint8_t                  frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t                  frames_sdu;

    ///LTV media data
    /// Streaming Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t                 context_bf;

    uint8_t                  add_metadata_len;
    uint8_t*                 add_metadata_data;
} bes_ble_bis_src_subgrp_param_t;

/// sync see@aob_bis_src_stream_param_t
typedef struct
{
    /// Stream local identifier
    uint8_t                  stream_lid;
    /// Subgroup local identifier
    uint8_t                  sgrp_lid;
    /// LTV cfg param
    /// When received, 0 shall be interpreted as a single channel with no specified Audio Location
    /// (see @BES_BLE_GAF_LOCATION_BF_BIT_E)
    uint32_t                 location_bf;
    /// Length of a codec frame in octets
    uint16_t                 frame_octet;
    /// Sampling Frequency (see #BES_BLE_GAF_SAMPLE_FREQ_E enumeration)
    uint8_t                  sampling_freq;
    /// Frame Duration (see #BES_BLE_GAF_FRAME_DUR_E enumeration)
    uint8_t                  frame_dur;
    /// Number of blocks of codec frames that shall be sent or received in a single SDU
    uint8_t                  frames_sdu;
} bes_ble_bis_src_stream_param_t;

/// Codec Configuration Metadata structure
typedef struct
{
    /// Parameters structure
    uint16_t context_bf;
    /// Additional Metadata value (in LTV format)
    /// Length of data value
    uint8_t len;
    /// Data value
    uint8_t data[0];
} bes_ble_bis_src_metadata_update_t;

typedef struct
{
    uint32_t big_trans_latency;
} bes_ble_bis_src_started_param_t;

/// sync see@aob_bis_src_stream_param_t
typedef struct
{
    void (*bis_stream_get_data)(uint8_t stream_id, uint8_t **data, uint16_t *data_len, uint8_t cache_num);
    void (*bis_stream_get_buf_free)(uint8_t stream_id, uint8_t *data);
    void (*bis_stream_start_ind)(uint8_t stream_id, bes_ble_bis_src_started_param_t *param);
    void (*bis_stream_stop_ind)(uint8_t stream_id);
} bes_ble_bis_src_start_param_t;

/// sync see@aob_bis_sink_start_param_t
typedef struct
{
    uint32_t ch_bf;
    uint8_t *bc_id;
    uint8_t *bc_code;
    struct
    {
        void (*bis_sink_scan_state_cb)(bool scan_or_pa_sync, bool started, uint32_t param);
        bool (*bis_sink_select_source)(ble_bdaddr_t *addr, uint8_t adv_sid, uint8_t *bcast_id,
                                            uint8_t *adv_data, uint8_t adv_data_len, int8_t rssi);
        void (*bis_sink_started_callback)(uint8_t grp_lid);
        void (*bis_sink_stoped_callback)(uint8_t grp_lid, uint16_t err_code);
        void (*bis_sink_metadata_cb)(uint8_t subgrp_lid, uint8_t *buf, uint8_t buf_len);
    } event_callback;
} bes_ble_bis_sink_start_param_t;

/// sync see@app_gaf_codec_id_t
typedef struct
{
    /// Codec ID value
    uint8_t codec_id[5];
} bes_gaf_codec_id_t;

/// sync see@AOB_MEDIA_ASE_CFG_INFO_T
typedef struct
{
    uint16_t sample_rate;
    uint16_t frame_octet;
    bes_gaf_direction_t direction;
    const bes_gaf_codec_id_t *codec_id;
    BES_BLE_GAF_CONTEXT_E context_type;
} bes_lea_ase_cfg_param_t;

/**
 * @brief Le audio init
 * 
 */
void bes_ble_audio_common_init(void);

/**
 * @brief Le audio deinit
 * 
 */
void bes_ble_audio_common_deinit(void);

/**
 * @brief Get bis handle by big idx
 *
 * @param[in] big_idx Big idx
 * 
 * @return uint16_t   Bis_hdl
 */
uint16_t bes_ble_bis_src_get_bis_hdl_by_big_idx(uint8_t big_idx);

/**
 * @brief Get bis codec id by big idx
 * 
 * @param[in] big_idx     Big idx
 * @param[in] subgrp_idx  Subgroup idx
 * 
 * @return const AOB_CODEC_ID_T* Codec type
 */
const AOB_CODEC_ID_T *bes_ble_bis_src_get_codec_id_by_big_idx(uint8_t big_idx, uint8_t subgrp_idx);

/**
 * @brief Get bis codec cfg by big idx
 * 
 * @param[in] big_idx   Big idx
 * 
 * @return const AOB_BAP_CFG_T*  BAP config param
 */
const AOB_BAP_CFG_T *bes_ble_bis_src_get_codec_cfg_by_big_idx(uint8_t big_idx);

/**
 * @brief Get bis iso interval by big idx
 * 
 * @param[in] big_idx   Big idx
 * 
 * @return uint32_t     ISO interval in frames From 5ms to 4s
 */
uint32_t bes_ble_bis_src_get_iso_interval_ms_by_big_idx(uint8_t big_idx);

/**
 * @brief Bis send iso data to all channel
 * 
 * @param[in] payload      Data need to send
 * @param[in] payload_len  Data len will to send
 * @param[in] ref_time     Set SDU timestamp
 * 
 * @return uint8_t No need to pay attention still return 0
 */
uint8_t bes_ble_bis_src_send_iso_data_to_all_channel(uint8_t **payload, uint16_t payload_len, uint32_t ref_time);

/**
 * @brief Set big src param
 * 
 * @param[in] big_idx      Big idx
 * @param[in] p_big_param  Big param config@bes_ble_bis_src_big_param_t
 */
void bes_ble_bis_src_set_big_param(uint8_t big_idx, bes_ble_bis_src_big_param_t *p_big_param);


/**
 * @brief Set bis src subgroup param
 * 
 * @param[in] big_idx          Big idx
 * @param[in] p_subgrp_param   Subgroup parm config@bes_ble_bis_src_subgrp_param_t
 */
void bes_ble_bis_src_set_subgrp_param(uint8_t big_idx, bes_ble_bis_src_subgrp_param_t *p_subgrp_param);

/**
 * @brief Set bis src steam param
 * 
 * @param[in] big_idx          Big idx
 * @param[in] p_stream_param   Stream param config@bes_ble_bis_src_stream_param_t
 */
void bes_ble_bis_src_set_stream_param(uint8_t big_idx, bes_ble_bis_src_stream_param_t *p_stream_param);

/**
 * @brief Update bis src metadata
 * 
 * @param[in] grp_lid    group lid
 * @param[in] sgrp_lid   subgroup lid
 * @param[in] metadata   metadata@bes_ble_bis_src_metadata_update_t
 */
void bes_ble_bis_src_update_metadata(uint8_t grp_lid, uint8_t sgrp_lid, bes_ble_bis_src_metadata_update_t *metadata);

/**
 * @brief Get bis stream anchor time
 * 
 * @param[in] big_idx      Big idx
 * @param[in] stream_lid   Stream lid
 * @return uint32_t        bis anchor timestamp
 */
uint32_t bes_ble_bis_src_get_stream_anchor_time(uint8_t big_idx, uint8_t stream_lid);

/**
 * @brief Start bis src
 * 
 * @param[in] big_idx        Big idx
 * @param[in] start_bis_info Bis info param@bes_ble_bis_src_start_param_t
 */
void bes_ble_bis_src_start(uint8_t big_idx, bes_ble_bis_src_start_param_t *start_bis_info);

/**
 * @brief Stop bis src
 * 
 * @param[in] big_idx  Big idx
 */
void bes_ble_bis_src_stop(uint8_t big_idx);

/**
 * @brief Update tws nv role
 * 
 * @param[in] role  nv role@BLE_AUDIO_TWS_ROLE_E
 */
void bes_ble_update_tws_nv_role(uint8_t role);

/**
 * @brief Set activity type
 * 
 * @param[in] type  @gaf_bap_activity_type_e
 */
void bes_ble_bap_set_activity_type(gaf_bap_activity_type_e type);

/**
 * @brief If dev is connected
 * 
 * @param[in] p_addr  addr and addr type@ble_bdaddr_t
 * 
 * @return true/false
 */
bool bes_ble_gap_is_remote_dev_connected(const ble_bdaddr_t *p_addr);

/**
 * @brief Start connectable adv
 * 
 * @param[in] br_edr_support   Local device if support BR/EDR
 * @param[in] discoverable     Adv if discoverable
 * @param[in] init_reconnect   Adv if start will be reconnect
 * 
 * @return true/false
 */
bool bes_ble_aob_conn_start_adv(bool br_edr_support, bool discoverable, bool init_reconnect);

/**
 * @brief Stop connectable adv
 * 
 * @return true/false
 */
bool bes_ble_aob_conn_stop_adv(void);

/**
 * @brief If use custom's sirk
 * 
 * @return true/false
 */
bool bes_ble_aob_csip_is_use_custom_sirk(void);

/**
 * @brief Get set rsi generated using pre set sirk
 * 
 * @param[in] p_rsi_get
 *                     RSI value in 6 octets
 * 
 * @return bool        RSI value exist or not
 */
bool bes_ble_aob_csip_get_rsi_value(uint8_t *p_rsi_get);

/**
 * @brief Gattc reset cache
 * 
 * @param[in] record  Cache record in nv@GATTC_NV_SRV_ATTR_t
 */
void bes_ble_aob_gattc_rebuild_cache(GATTC_NV_SRV_ATTR_t *record);

/**
 * @brief Handler service data received
 * 
 * @param[in] p_buff  Service uuid and data
 * @param[in] len     Recv data len
 */
void bes_ble_aob_service_recv_handler(uint8_t *p_buff, uint16_t len);

/**
 * @brief Generate temporary sirk and update
 * 
 */
void bes_ble_aob_csip_if_use_temporary_sirk();

/**
 * @brief Refresh sirk
 * 
 * @param[in] sirk  Update sirk len is APP_GAF_CSIS_SIRK_LEN_VALUE
 */
void bes_ble_aob_csip_if_refresh_sirk(uint8_t *sirk);

/**
 * @brief If sirk has been refreshed
 * 
 * @return true/false
 */
bool bes_ble_aob_csip_sirk_already_refreshed();

/**
 * @brief Update sirk
 * 
 * @param[in] sirk     Ptr of sirk
 * @param[in] sirk_len Len of sirk APP_GAF_CSIS_SIRK_LEN_VALUE
 */
void bes_ble_aob_csip_if_update_sirk(uint8_t *sirk, uint8_t sirk_len);

/**
 * @brief Get local nv record sirk
 * 
 * @param[out] sirk   Local sirk
 * @return true/false
 */
bool bes_ble_aob_csip_if_get_sirk(uint8_t *sirk);

/**
 * @brief Set local device numbers in csip
 * 
 * @param[in] dev_num  The num flag of local device in CSIP is unique
 * @return true/false
 */
bool bes_ble_aob_csip_if_set_device_numbers(uint8_t dev_num);

/**
 * @brief Get local device numbers in csip
 * 
 * @return uint8_t     Local device num flag in csip
 */
uint8_t bes_ble_aob_csip_if_get_device_numbers(void);

/**
 * @brief Dump local device connected and stream state
 * 
 */
void bes_ble_aob_conn_dump_state_info(void);

/**
 * @brief TWS sync state req
 * 
 */
void bes_ble_aob_bis_tws_sync_state_req(void);

/**
 * @brief TWS sync state handler
 * 
 * @param[in] buf 
 */
void bes_ble_aob_bis_tws_sync_state_req_handler(uint8_t *buf);

/**
 * @brief TBC call outgoing
 * 
 * @param[in] conidx  Connection idx
 * @param[in] uri     Call num eg."tel:13066778839"
 * @param[in] uriLen  Len of call num
 */
void bes_ble_aob_call_if_outgoing_dial(uint8_t conidx, uint8_t *uri, uint8_t uriLen);

/**
 * @brief Get connected remote addr
 * 
 * @param[in] con_lid    Connection lid
 * @return ble_bdaddr_t* 
 */
ble_bdaddr_t *bes_ble_aob_conn_get_remote_address(uint8_t con_lid);

/**
 * @brief MCC control paly prev music
 * 
 * @param[in] con_lid    Connection lid
 */
void bes_ble_aob_media_prev(uint8_t con_lid);

/**
 * @brief MCC control paly next music
 * 
 * @param[in] con_lid    Connection lid
 */
void bes_ble_aob_media_next(uint8_t con_lid);

/**
 * @brief MCC fast forward the current track
 * 
 * @param[in] con_lid    Connection lid
 */
void bes_ble_aob_media_fast_fw(uint8_t con_lid);

/**
 * @brief MCC fast rewind the current track
 * 
 * @param[in] con_lid    Connection lid
 */
void bes_ble_aob_media_fast_rw(uint8_t con_lid);

/**
 * @brief TBC retrieve local call
 *
 * @param[in] conidx     Connection lid
 * @param[in] call_id    Call lid
 */
void bes_ble_aob_call_if_retrieve_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief TBC hold local call
 *
 * @param[in] conidx     Connection lid
 * @param[in] call_id    Call lid
 */
void bes_ble_aob_call_if_hold_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief TBC terminate the call
 * 
 * @param[in] conidx     Connection lid
 * @param[in] call_id    Call lid
 */
void bes_ble_aob_call_if_terminate_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief TBC accept the call
 * 
 * @param[in] conidx     Connection lid
 * @param[in] call_id    Call lid
 */
void bes_ble_aob_call_if_accept_call(uint8_t conidx, uint8_t call_id);

/**
 * @brief TBC free pending actions
 *
 * @param[in] action_list list of pending actions
 */
void bes_ble_aob_call_if_free_pending_actions(struct list_node *action_list);

/**
 * @brief Get if any call state not idle at the connection
 * 
 * @param[in] conidx     Connection lid
 * @param[out] p_call_id Call lid
 * 
 * @return uint8_t Call state@AOB_CALL_STATE_E  
 */
uint8_t bes_ble_aob_call_if_get_any_call_by_conidx(uint8_t conidx, uint8_t *p_call_id);

/**
 * @brief MCC play music
 * 
 * @param[in] con_lid Conn lid
 */
void bes_ble_aob_media_play(uint8_t con_lid);

/**
 * @brief MCC pause music
 * 
 * @param[in] con_lid Conn lid
 */
void bes_ble_aob_media_pause(uint8_t con_lid);

/**
 * @brief VCS control vol down
 * 
 */
void bes_ble_aob_vol_down(void);

/**
 * @brief VCS control vol down
 * 
 */
void bes_ble_aob_vol_up(void);

/**
 * @brief Convert bt vol to le vol
 * 
 * @param[in] bt_vol  Bt vol
 *
 * @return uint8_t    Ble vol
 */
uint8_t bes_ble_aob_convert_local_vol_to_le_vol(uint8_t bt_vol);

/**
 * @brief Get policy config
 * 
 * @return BLE_AUDIO_POLICY_CONFIG_T* 
 */
BLE_AUDIO_POLICY_CONFIG_T* bes_ble_audio_get_policy_config();

/**
 * @brief Set audio focous resume callback
 * 
 * @param[in] resume_cb  resume callback
 * 
 * @return int @bt_status_t
 */
int bes_ble_audio_bis_stream_set_resume_callback(void (*resume_cb)(uint8_t device_id, uint32_t param));

/**
 * @brief LEA sink streaming event handle
 * 
 * @param[in] con_lid   con lid
 * @param[in] data      stream lid
 * @param[in] direction stream direction@bes_gaf_direction_t
 * @param[in] event     event@app_ble_audio_event_t
 */
void bes_ble_audio_sink_streaming_handle_event(uint8_t con_lid, uint8_t data,
                                                               bes_gaf_direction_t direction, app_ble_audio_event_t event);


/**
 * @brief Dump BLE Aduio stream state
 * 
 */
void bes_ble_audio_dump_conn_state(void);

/**
 * @brief Get connected mobile addr
 * 
 * @param[in] deviceId  Device id
 * @param[out] addr      Mobile addr
 *
 * @return uint8_t 0/-1
 */
uint8_t bes_ble_audio_get_mobile_addr(uint8_t deviceId, uint8_t *addr);

/**
 * @brief Get call id
 * 
 * @param[in] device_id   Local active device id
 * @param[in] call_state  Call state@AOB_CALL_STATE_E
 *
 * @return uint8_t        Call lid
 */
uint8_t bes_ble_aob_get_call_id_by_conidx_and_type(uint8_t device_id, uint8_t call_state);

/**
 * @brief Get call id
 * 
 * @param[in] device_id Local active device id
 * 
 * @return uint8_t      Call lid
 */
uint8_t bes_ble_aob_get_call_id_by_conidx(uint8_t device_id);

/**
 * @brief Get ACC service bond status
 * 
 * @param[in] conidx  Conn lid
 * @param[in] type    Service type@BLE_AUDIO_ACC_TYPE_E
 * 
 * @return true/false 
 */
bool bes_ble_aob_get_acc_bond_status(uint8_t conidx, uint8_t type);

/**
 * @brief Get capa ava context bf
 * 
 * @param[in] con_lid               Conn lid
 * @param[in] context_bf_ava_sink   Ava sink context bf
 * @param[in] context_bf_ava_src    Ava src context bf
 */
void bes_ble_bap_capa_srv_get_ava_context_bf(uint8_t con_lid, uint16_t *context_bf_ava_sink, uint16_t *context_bf_ava_src);

/**
 * @brief Set capa ava context bf
 * 
 * @param[in] con_lid                Conn lid
 * @param[in] context_bf_ava_sink    Ava sink context bf
 * @param[in] context_bf_ava_src     Ava src context bf
 */
void bes_ble_bap_capa_srv_set_ava_context_bf(uint8_t con_lid, uint16_t context_bf_ava_sink, uint16_t context_bf_ava_src);

// sink api
/**
 * @brief BIS sink start scan
 * 
 */
void bes_ble_bis_start_scan(void);

/**
 * @brief BIS sink stop scan
 * 
 */
void bes_ble_bis_stop_scan(void);

/**
 * @brief BIS sink start with param
 * 
 * @param[in] param @bes_ble_bis_sink_start_param_t
 */
void bes_ble_bis_sink_start(bes_ble_bis_sink_start_param_t *param);

/**
 * @brief BIS sink stop
 * 
 */
void bes_ble_bis_sink_stop();

/**
 * @brief Set bis audio location bf preferred
 * 
 * @param  aud_loc_bf  Audio location bf
 * 
 */
void bes_ble_bis_sink_set_aud_location_bf(uint32_t aud_loc_bf);

/**
 * @brief BIS sink set src id and code
 * 
 * @param[in] bcast_id   BIS src id len APP_GAP_BCAST_ID_LEN
 * @param[in] bcast_code BIS src key if encrypted len APP_GAP_KEY_LEN
 */
void bes_ble_bis_sink_set_src_id_key(uint8_t *bcast_id, uint8_t *bcast_code);

/**
 * @brief Get pa sync hadle
 * 
 * @param[in] pa_lid  Pa lid
 * 
 * @return uint16_t   Pa synced handle
 */
uint16_t bes_ble_bis_sink_get_pa_sync_hdl_by_pa_lid(uint8_t pa_lid);

/**
 * @brief Sync to pa with param
 * 
 * @param[in] p_addr    Peer pa addr
 * @param[in] adv_sid   Pa adv sid
 * @param[in] sync_to_s Sync timeout *100ms
 */
void bes_ble_bis_scan_pa_sync_with_to(ble_bdaddr_t *p_addr, uint8_t adv_sid, uint16_t sync_to_s);

/**
 * @brief Cancel PA sync
 * 
 * 
 */
void bes_ble_bis_scan_pa_sync_cancel(void);

/**
 * @brief Stop scan pa
 * 
 */
void bes_ble_bis_scan_pa_sync_stop(void);

/**
 * @brief Control pa adv report
 * 
 * @param[in] pa_lid  Pa lid
 * @param[in] enable  Enable to report
 */
void bes_ble_bis_scan_pa_report_ctrl(uint8_t pa_lid, bool enable);

/**
 * @brief Start disconvery BAP service
 * 
 * @param[in] con_lid  Conn lid
 */
void bes_ble_bap_start_discovery(uint8_t con_lid);

/**
 * @brief Get local activity type
 * 
 * @return @gaf_bap_activity_type_e 
 */
gaf_bap_activity_type_e bes_ble_bap_get_actv_type(void);

/**
 * @brief Get connected device num
 * 
 * @return uint8_t  Device nums
 */
uint8_t bes_ble_bap_get_device_num_to_be_connected(void);

/**
 * @brief Get ase info
 * 
 * @param[in] ase_lid  ASE lid
 * @return @const bes_ble_bap_ascs_ase_t* 
 */
const bes_ble_bap_ascs_ase_t *bes_ble_get_ascs_ase_info(uint8_t ase_lid);

/**
 * @brief Get local capa localtion bf
 * 
 * @param[in] direction  Audio direction@bes_gaf_direction_t
 * @return uint32_t      Capa location bf
 */
uint32_t bes_ble_bap_capa_get_location_bf(bes_gaf_direction_t direction);

/**
 * @brief Get ase lid list when ASE is streaming state
 * 
 * @param[in] con_lid       Conn lid
 * @param[out] ase_lid_list ASE lid list
 * @return uint8_t ASE count
 */
uint8_t bes_ble_bap_ascs_get_streaming_ase_lid_list(uint8_t con_lid, uint8_t *ase_lid_list);

/**
 * @brief CFM ASE enable req
 * 
 * @param[in] ase_lid  ASE lid
 * @param[in] accept   If accept enable req
 */
void bes_ble_bap_ascs_send_ase_enable_rsp(uint8_t ase_lid, bool accept);

/**
 * @brief Disable ase req
 * 
 * @param[in] ase_lid   ASE lid
 */
void bes_ble_bap_ascs_disable_ase_req(uint8_t ase_lid);

/**
 * @brief Release ASE req
 * 
 * @param[in] ase_lid   ASE lid
 */
void bes_ble_bap_ascs_release_ase_req(uint8_t ase_lid);

/**
 * @brief Register datapath callback
 * 
 * @param[in] callback 
 */
void bes_ble_bap_dp_itf_data_come_callback_register(void *callback);

/**
 * @brief Deregister datapath callbakc
 * 
 */
void bes_ble_bap_dp_itf_data_come_callback_deregister(void);

/**
 * @brief Send iso data
 * 
 * @param[in] conhdl      Conn handle can be convert from conn lid
 * @param[in] seq_num     Seq num when send data
 * @param[in] payload     Send data payload
 * @param[in] payload_len Payload len
 * @param[in] ref_time    ISO data timestamp
 */
void bes_ble_bap_iso_dp_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time);

/**
 * @brief Get local nv role
 * 
 * @return uint8_t nv role@BLE_AUDIO_TWS_ROLE_E
 */
uint8_t bes_ble_audio_get_tws_nv_role(void);

/**
 * @brief Get local support location num
 * 
 * @param[in] audio_location_bf  Audio location bf@gaf_bap_supported_locations_bf
 * @return uint8_t               Suppport left and right location cnt
 */
uint8_t bes_ble_audio_get_location_fs_l_r_cnt(uint32_t audio_location_bf);

/**
 * @brief Get mic state
 * 
 * @param[in] con_lid   Conn lid
 * @return uint8_t      Mic state
 */
uint8_t bes_ble_arc_get_mic_state(uint8_t con_lid);

/**
 * @brief Get ltv that specified by ltv type
 * 
 * @param  p_ltv_data  LTV data start
 * @param  ltv_type    LTV type
 * 
 * @return uint8_t*    LTV data specified by type
 */
uint8_t *bes_ble_audio_get_ltv_value_by_type(AOB_CFG_LTV_T *p_ltv_data, uint8_t ltv_type);

/**
 * @brief Convert le real vol to local vol
 * 
 * @param[in] le_vol  LE vol [0,255]
 * @return uint8_t    Local vol[0, 16]
 */
uint8_t bes_ble_arc_convert_le_vol_to_local_vol(uint8_t le_vol);

/**
 * @brief Get local real vol
 * 
 * @param[in] con_lid  Conn lid
 * @return uint8_t     Local real vol record in nv[0, 255]
 */
uint8_t bes_ble_arc_vol_get_real_time_volume(uint8_t con_lid);

/**
 * @brief Set local abs vol
 * 
 * @param[in] con_lid   Conn lid
 * @param[in] local_vol Local bt vol[0, 15]
 */
void bes_ble_arc_mobile_set_abs_vol(uint8_t con_lid, uint8_t local_vol);

/**
 * @brief Get rx data from iso datapath callback
 * 
 * @param[in] iso_hdl       ISO handle
 * @param[out] p_iso_buffer ISO data buf
 * @return void*  SDU buf @gaf_media_data_t
 */
void *bes_ble_bap_dp_itf_get_rx_data(uint16_t iso_hdl, bes_ble_dp_itf_iso_buffer_t *p_iso_buffer);

/**
 * @brief Get controller free iso tx num
 * 
 * @return int  free iso tx num
 */
int bes_ble_bap_get_free_iso_packet_num(void);

/**
 * @brief Stop tx iso datapath
 * 
 * @param[in] iso_hdl  ISO handle
 */
void bes_ble_bap_dp_tx_iso_stop(uint16_t iso_hdl);

/**
 * @brief Stop rx iso datapath
 * 
 * @param[in] iso_hdl  ISO handle
 */
void bes_ble_bap_dp_rx_iso_stop(uint16_t iso_hdl);

/**
 * @brief IF local device in call active state
 * 
 * @param[in] con_lid  Conn lid
 * @return true/false
 */
bool bes_ble_ccp_call_is_device_call_active(uint8_t con_lid);

/**
 * @brief Start service discovery
 * 
 * @param[in] con_lid   Conn lid
 */
void bes_ble_start_gaf_discovery(uint8_t con_lid);

void bes_ble_gaf_media_status_handler_cb_register(void (*cb)(uint8_t con_lid, bool paused));

#ifdef AOB_MOBILE_ENABLED
/**
 * @brief Start lea stream as mobile
 * 
 * @param[in] con_lid       Conn lid
 * @param[in] cfg           ASE cfg param@bes_lea_ase_cfg_param_t
 * @param[in] bidirectional If only sink/src or both
 */
void bes_lea_mobile_stream_start(uint8_t con_lid, bes_lea_ase_cfg_param_t *cfg, bool bidirectional);

/**
 * @brief Get ase info
 * 
 * @param[in] ase_lid   ASE lid
 * 
 * @return const bes_ble_bap_ascc_ase_t* 
 */
const bes_ble_bap_ascc_ase_t *bes_ble_bap_ascc_get_ase_info(uint8_t ase_lid);

/**
 * @brief Get ase lid list in specific state
 * 
 * @param[in] con_lid      Conn lid
 * @param[in] direction    Stream direction
 * @param[in] ase_state    ASE state
 * @param[out] ase_lid_list ASE lid list
 * 
 * @return uint8_t         ASE cnt
 */
uint8_t bes_ble_bap_ascc_get_specific_state_ase_lid_list(uint8_t con_lid, uint8_t direction, uint8_t ase_state, uint8_t *ase_lid_list);

/**
 * @brief If peer support stereo channel
 * 
 * @param[in] con_lid   Conn lid
 * @param[in] direction Stream direction
 * 
 * @return true/false
 */
bool bes_ble_bap_pacc_is_peer_support_stereo_channel(uint8_t con_lid, uint8_t direction);

/**
 * @brief Config codec param
 * 
 * @param[in] ase_lid     ASE lid
 * @param[in] cis_id      CIS lid
 * @param[in] codec_id    Codec lid
 * @param[in] sampleRate  SampleRate@bap_sampling_freq
 * @param[in] frame_octet Frame oectet
 */
void bes_ble_bap_ascc_configure_codec_by_ase_lid(uint8_t ase_lid, uint8_t cis_id, const AOB_CODEC_ID_T *codec_id,
                                                        uint16_t sampleRate, uint16_t frame_octet);

/**
 * @brief QOS param cfg
 * 
 * @param[in] ase_lid  ASE lid
 * @param[in] grp_lid  CIG group lid
 */
void bes_ble_bap_ascc_ase_qos_cfg_by_ase_lid(uint8_t ase_lid, uint8_t grp_lid);

/**
 * @brief Create cig group
 * 
 * @param[in] cig_lid  CIG lid
 */
void bes_ble_bap_ascc_link_create_group_req(uint8_t cig_lid);

/**
 * @brief Remove cig group
 * 
 * @param[in] grp_lid  CIG group id
 */
void bes_ble_bap_ascc_link_remove_group_req(uint8_t grp_lid);

/**
 * @brief Enable ASE
 * 
 * @param[in] ase_lid    ASE lid
 * @param[in] context_bf @bap_context_type_bf
 */
void bes_ble_bap_ascc_ase_enable_by_ase_lid(uint8_t ase_lid, uint16_t context_bf);

/**
 * @brief Release ASE
 * 
 * @param[in] ase_lid    ASE lid
 */
void bes_ble_bap_ascc_ase_release_by_ase_lid(uint8_t ase_lid);

/**
 * @brief Disable ASE
 * 
 * @param[in] ase_lid    ASE lid
 */
void bes_ble_bap_ascc_ase_disable_by_ase_lid(uint8_t ase_lid);

/**
 * @brief Set sdu interval
 * 
 * @param[in] sdu_intv_c2p_us   SDU intervel central to perh
 * @param[in] sdu_intv_p2c_us   SDU intervel perh to central
 */
void bes_ble_bap_ascc_set_sdu_interval(uint32_t sdu_intv_c2p_us, uint32_t sdu_intv_p2c_us);

/**
 * @brief Set cis count in CIG
 * 
 * @param[in] cis_count  CIS count
 */
void bes_ble_bap_ascc_set_cis_count_in_cig(uint8_t cis_count);

/**
 * @brief Prepare CIG param
 * 
 * @param[in] cig_param  @AOB_BAP_CIG_PARAM_T
 */
void bes_ble_bap_ascc_prepare_cig_param(const AOB_BAP_CIG_PARAM_T *cig_param);

/**
 * @brief MCS control remote player
 * 
 * @param[in] media_lid  Media lid
 * @param[in] action     @MCS_ACTION_MAX
 */
void bes_ble_mcp_mcs_action_control(uint8_t media_lid, uint8_t action);

/**
 * @brief MCS changed track
 *
 * @param[in] media_lid     Media lid
 * @param[in] duration_10ms Duration tracks
 * @param[in] title         Track Title characteristic value 
 * @param[in] title_len     Len of write char value
 */
void bes_ble_mcp_mcs_track_changed(uint8_t media_lid, uint32_t duration_10ms, uint8_t *title, uint8_t title_len);

#endif /// AOB_MOBILE_ENABLED

#endif /* BLE_AUDIO_ENABLED */

#ifdef BLE_ISO_ENABLED
/**
 * @brief malloc iso data
 * 
 * @param[in] size      size of iso data
 */
void *bes_ble_iso_malloc_buff(uint32_t size);
/**
 * @brief free iso data
 * 
 * @param[in] mem_ptr      address for iso data
 */
void bes_ble_iso_free_buff(void *mem_ptr);

/**
 * @brief free iso data rx datapath
 * 
 * @param[in] mem_ptr  address for iso data rx by molloc by stack
 */
void bes_ble_iso_rx_free_buff(void *mem_ptr);
/**
 * @brief Send iso data
 * 
 * @param[in] conhdl      Conn handle can be convert from conn lid
 * @param[in] seq_num     Seq num when send data
 * @param[in] payload     Send data payload
 * @param[in] payload_len Payload len
 * @param[in] ref_time    ISO data timestamp
 */
void bes_ble_iso_dp_send_data(uint16_t conhdl, uint16_t seq_num, uint8_t *payload, uint16_t payload_len, uint32_t ref_time);

/**
 * @brief Get rx data from iso datapath callback
 * 
 * @param[in] iso_hdl       ISO handle
 * @param[out] p_iso_buffer ISO data buf
 * @return void*  SDU buf @gaf_media_data_t
 */
void *bes_ble_iso_dp_itf_get_rx_data(uint16_t iso_hdl, bes_ble_dp_itf_iso_buffer_t *p_iso_buffer);

/**
 * @brief Get controller free iso tx num
 * 
 * @return int  free iso tx num
 */
int bes_ble_iso_get_free_iso_packet_num(void);

/**
 * @brief Stop tx iso datapath
 * 
 * @param[in] iso_hdl  ISO handle
 */
void bes_ble_iso_dp_tx_iso_stop(uint16_t iso_hdl);

/**
 * @brief Stop rx iso datapath
 * 
 * @param[in] iso_hdl  ISO handle
 */
void bes_ble_iso_dp_rx_iso_stop(uint16_t iso_hdl);

void bes_ble_iso_dp_set_rx_dp_itf(void);

/**
 * @brief Register datapath callback
 * 
 * @param[in] callback 
 */
void bes_ble_iso_dp_itf_data_come_callback_register(void *callback);

/**
 * @brief Deregister datapath callbakc
 * 
 */
void bes_ble_iso_dp_itf_data_come_callback_deregister(void);

#endif

#ifdef __cplusplus
}
#endif
#endif /* BLE_HOST_SUPPORT */
#endif /* __BES_AOB_API_H__ */
