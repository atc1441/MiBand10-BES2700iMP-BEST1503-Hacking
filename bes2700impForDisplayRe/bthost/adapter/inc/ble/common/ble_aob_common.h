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
#ifndef __BLE_AOB_COMMON_H__
#define __BLE_AOB_COMMON_H__
#include "ble_common_define.h"
#include "ble_core_common.h"
#include "ble_gatt_common.h"
#include "ble_acc_common.h"
#ifdef BLE_HOST_SUPPORT

#define CHECK_SIZE_TYPE(dataType, nBytes)       typedef POSSIBLY_UNUSED char bes_##dataType[(sizeof(dataType) == (nBytes)) ? 1 : -1]

#define AOB_COMMON_MOBILE_CONNECTION_MAX        (BLE_AUDIO_CONNECTION_CNT)
#define AOB_COMMON_MAX_NUM_OF_MOBILE_CONNECT_TO (BLE_AUDIO_CONNECTION_CNT)

#define AOB_COMMON_ATC_CSIS_RSI_LEN             (6)

#define AOB_COMMON_CODEC_ID_LEN                 (5)
#define AOB_COMMON_BC_ID_LEN                    (3)
#define AOB_COMMON_IAP_NB_STREAMS               (4)
#define AOB_COMMON_GAP_KEY_LEN                  (16)

#define AOB_COMMON_BAP_ASCC_SINK_ASE            (2)
#define AOB_COMMON_BAP_ASCC_SRC_ASE             (2)
#define AOB_COMMON_BAP_ASCC_TOTAL_ASE_CHAR      (AOB_COMMON_BAP_ASCC_SINK_ASE + AOB_COMMON_BAP_ASCC_SRC_ASE)
#define AOB_COMMON_BAP_ASCC_TOTAL_ASE_CFG       (BLE_CONNECTION_MAX * (AOB_COMMON_BAP_ASCC_TOTAL_ASE_CHAR))

#define AOB_COMMON_BAP_DFT_UC_GRP_LID           (0)
#define AOB_COMMON_DUAL_BLEAUDIO_CONNECT        (2)
#define AOB_COMMON_INVALID_CON_ID               (0xFF)

#ifdef __cplusplus
extern "C" {
#endif

#ifdef AOB_MOBILE_ENABLED
/**
 * @brief define BLE connection mobile event for upper layer
 *
 */
typedef enum
{
    /// MOB CONN EVENT
    MOB_BLE_AUD_CONNECTING,
    MOB_BLE_AUD_CANCEL_CONNECTION,
    MOB_BLE_AUD_CONNECTION_CANCELED,
    MOB_BLE_AUD_CONNECTION_FAILED,
    MOB_BLE_AUD_CONNECTION_TIMEOUT,
    MOB_BLE_AUD_BOND,
    MOB_BLE_AUD_BOND_FAIL,
    MOB_BLE_AUD_ENCRYPT,
    MOB_BLE_AUD_CONNECTED,
    MOB_BLE_AUD_DISCONNECTING,
    MOB_BLE_AUD_DISCONNECTED,

    // ADV STATE

    /// CALL

    /// GAF

} BLE_AUD_MOB_CORE_EVT_E;

/**
 * @brief ble audio core event callback structure define for mobile
*/
typedef struct
{
    void (*mob_ble_acl_state_changed)(uint32_t evt_type, ble_event_handled_t *p);
    void (*scan_data_report)(ble_event_t *event);

} BLE_AUD_MOB_CORE_EVT_CB_T;
#endif  ///AOB_MOBILE_ENABLED

typedef enum AOB_AUDIO_CONFIGURATION
{
    AOB_AUD_CFG_MIN = 0,
    /// TWS mode
    AOB_AUD_CFG_TWS_MONO = AOB_AUD_CFG_MIN,
    /// TWS Spatial audio
    AOB_AUD_CFG_TWS_STEREO_ONE_CIS,
    AOB_AUD_CFG_TWS_STEREO_TWO_CIS,//(may not be used)
    /// FREEMAN mode (Not single ear usage)
    AOB_AUD_CFG_FREEMAN_STEREO_ONE_CIS,
    AOB_AUD_CFG_FREEMAN_STEREO_TWO_CIS,
    /// Audio configuration max
    AOB_AUD_CFG_MAX,
} aob_audio_cfg_e;

/// Activity Type
typedef enum gaf_bap_activity_type
{
    GAF_BAP_ACT_TYPE_CIS_AUDIO = 0,
    GAF_BAP_ACT_TYPE_BIS_AUDIO,
    GAF_BAP_ACT_TYPE_BIS_SHARE,
    GAF_BAP_ACT_TYPE_MAX,
} gaf_bap_activity_type_e;

typedef enum {
    AOB_MGR_STREAM_STATE_IDLE             = 0,
    AOB_MGR_STREAM_STATE_CODEC_CONFIGURED = 1,
    AOB_MGR_STREAM_STATE_QOS_CONFIGURED   = 2,
    AOB_MGR_STREAM_STATE_ENABLING         = 3,
    AOB_MGR_STREAM_STATE_STREAMING        = 4,
    AOB_MGR_STREAM_STATE_DISABLING        = 5,
    AOB_MGR_STREAM_STATE_RELEASING        = 6,

    AOB_MGR_STREAM_STATE_MAX,
} AOB_MGR_STREAM_STATE_E;

typedef enum {
    AOB_ADV_START         = 0,
    AOB_ADV_FAILED        = 1,
    AOB_ADV_STOP          = 2,
} AOB_ADV_STATE_T;

typedef enum {
    AOB_ACL_DISCONNECTED       = 0,
    AOB_ACL_CONNECTING         = 1,
    AOB_ACL_FAILED             = 3,
    AOB_ACL_CONNECTED          = 4,
    AOB_ACL_BOND_SUCCESS       = 5,
    AOB_ACL_BOND_FAILURE       = 6,
    AOB_ACL_ENCRYPT            = 7,
    AOB_ACL_ATTR_BOND          = 8,
    AOB_ACL_DISCONNECTING      = 9,
} AOB_ACL_STATE_T;

typedef struct {
    AOB_ACL_STATE_T                     acl_state;
    uint8_t                             err_code;
} AOB_CONNECTION_STATE_T;

//see @acc_mc_media_state
typedef enum {
    /// Inactive
    AOB_MGR_PLAYBACK_STATE_INACTIVE = 0,
    /// Playing
    AOB_MGR_PLAYBACK_STATE_PLAYING,
    /// Paused
    AOB_MGR_PLAYBACK_STATE_PAUSED,
    /// Seeking
    AOB_MGR_PLAYBACK_STATE_SEEKING,

    AOB_MGR_PLAYBACK_STATE_MAX,
} AOB_MGR_PLAYBACK_STATE_E;

typedef struct
{
    uint8_t outgoing_call_flag:        1;
    uint8_t withheld_server_flag:      1;
    uint8_t withheld_network_flag:     1;
    uint8_t reserved:                  5;
} AOB_CALL_CALL_FLAGS_T;

/// Call state type
typedef enum
{
    AOB_CALL_STATE_INCOMING                     = 0,
    AOB_CALL_STATE_DIALING                      = 1,
    AOB_CALL_STATE_ALERTING                     = 2,
    AOB_CALL_STATE_ACTIVE                       = 3,
    AOB_CALL_STATE_LOCAL_HELD                   = 4,
    AOB_CALL_STATE_REMOTE_HELD                  = 5,
    AOB_CALL_STATE_LOCAL_AND_REMOTE_HELD        = 6,
    AOB_CALL_STATE_IDLE                         = 7,
} AOB_CALL_STATE_E;

typedef struct
{
    /// Bearer local index, reserve value is 0xFF
    uint8_t                         bearer_lid;
    uint8_t                         signal_strength;
    /// Call flags
    AOB_CALL_CALL_FLAGS_T           call_flags;
    /// Call index, reserve value is 0x00
    uint8_t                         call_id;
    /// Call state
    AOB_CALL_STATE_E                state;
    /// Length of Incoming or Outgoing Call URI value
    uint8_t                         uri_len;
    /// Remote Call URI value
    uint8_t                         *uri;
} AOB_SINGLE_CALL_INFO_T;

typedef struct
{
    uint16_t inband_ring_enable: 1;
    uint16_t silent_mode_enable: 1;
    uint16_t reserved:           14;
} AOB_CALL_STATUS_FLAGS_T;

typedef struct
{
    uint8_t local_hold_op_supported:        1;
    uint8_t join_op_supported:              1;
    uint8_t reserved:                       6;
} AOB_CALL_OPT_OPCODE_FEATURE_T;

typedef struct
{
    bool                is_busy;
    struct list_node    action_list;
} AOB_CALL_ACTION_CONTEXT_T;

typedef struct
{
    /// Number of calls (only join opcode)
    uint8_t                         nb_calls;
    /// Status flag feature
    AOB_CALL_STATUS_FLAGS_T         status_flags;
    /// Optional opcode feature
    AOB_CALL_OPT_OPCODE_FEATURE_T   opt_opcode_flags;
    /// List of call indexes (only join opcode)
    AOB_SINGLE_CALL_INFO_T          single_call_info[AOB_COMMON_CALL_MAX_NB_IDS];
    /// Call control action queue
    AOB_CALL_ACTION_CONTEXT_T       action_ctx;
} AOB_CALL_ENV_INFO_T;

typedef struct
{
    uint8_t mic_mute;
    //Media state Value
    uint8_t media_state;
} AOB_MEDIA_INFO_T;

typedef struct {
    /// Cmd return status
    uint16_t status;
    /// Ase_lid
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
} __attribute__ ((__packed__)) AOB_ISO_LINK_QUALITY_INFO_T;

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
} AOB_UG_CONFIG_T;

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
} AOB_US_CONFIG_T;


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
    AOB_UG_CONFIG_T cig_config;
    /// Stream configuration\n
    /// Meaningful only if conhdl is not GAP_INVALID_CONHDL
    AOB_US_CONFIG_T cis_config;
} AOB_UC_SRV_CIS_INFO_T;

/// Codec Identifier
typedef struct
{
    /// Codec ID value
    uint8_t codec_id[5];
} AOB_CODEC_ID_T;

/// Codec Configuration parameters structure
typedef struct
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
} AOB_BAP_CFG_PARAM_T;

/// Data value in LTV format
typedef struct
{
    /// Length of data value
    uint8_t len;
    /// Data value
    uint8_t data[0];
} AOB_CFG_LTV_T;

/// Codec Capabilities parameters structure
typedef struct
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
} AOB_BAP_CAPA_PARAM_T;

/// Codec Capabilities structure
typedef struct
{
    /// Parameters structure
    AOB_BAP_CAPA_PARAM_T param;
    /// Additional Codec Capabilities (in LTV format)
    AOB_CFG_LTV_T add_capa;
} AOB_BAP_CAPA_T;

/// Codec Configuration structure
typedef struct
{
    /// Parameters structure
    AOB_BAP_CFG_PARAM_T param;
    /// Additional Codec Configuration (in LTV format)
    AOB_CFG_LTV_T add_cfg;
} AOB_BAP_CFG_T;

/// QoS Requirement structure
typedef struct
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
} AOB_BAP_QOS_REQ_T;

/// QoS Configuration structure
typedef struct
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
} AOB_BAP_QOS_CFG_T;

/// Codec Configuration Metadata parameters structure
typedef struct
{
    /// Streaming Audio Contexts bit field (see #enum bap_context_type_bf enumeration)
    uint16_t context_bf;
} AOB_BAP_CFG_MD_PARAM_t;

/// Codec Configuration Metadata structure
typedef struct
{
    /// Parameters structure
    AOB_BAP_CFG_MD_PARAM_t param;
    /// Additional Metadata value (in LTV format)
    AOB_CFG_LTV_T add_metadata;
} AOB_BAP_CFG_METADATA_T;

typedef struct
{
    // Central to Peripheral Burst Number
    int c2p_bn;
    // Central to Peripheral Number of Subevent
    int c2p_nse;
    // Central to Peripheral Flush Timeout
    int c2p_ft;
    // Peripheral to Central Burst Number
    int p2c_bn;
    // Peripheral to Central Number of Subevent
    int p2c_nse;
    // Peripheral to Central Flush Timeout
    int p2c_ft;
    // ISO interval 1.25ms * N
    int iso_interval_1_25ms;
} AOB_BAP_CIG_PARAM_T;

typedef enum {
    AOB_EVENT_TW_CONNECTION_STATE                 = 0,
    AOB_EVENT_MOB_CONNECTION_STATE                = 1,
    AOB_EVENT_ADV_STATE                           = 2,
    AOB_EVENT_VOL_CHANGED                         = 3,
    AOB_EVENT_VOCS_OFFSET_CHANGED                 = 4,
    AOB_EVENT_VOCS_BOND_DATA_CHANGED              = 5,
    AOB_EVENT_MEDIA_TRACK_CHANGED                 = 6,
    AOB_EVENT_STREAM_STATUS_CHANGED               = 7,
    AOB_EVENT_PLAYBACK_STATUS_CHANGED             = 8,
    AOB_EVENT_MIC_STATE                           = 9,
    AOB_EVENT_CALL_STATE_CHANGE                   = 10,
    AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND     = 11,
    AOB_EVENT_CALL_STATUS_FLAGS_IND               = 12,
    AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND   = 13,
    AOB_EVENT_CALL_TERMINATE_REASON_IND           = 14,
    AOB_EVENT_CALL_INCOMING_NUM_INF_IND           = 15,
    AOB_EVENT_CALL_SVC_CHANGED_IND                = 16,
    AOB_EVENT_CALL_ACTION_RESULT_IND              = 17,
    AOB_EVENT_ISO_LINK_QUALITY_IND                = 18,
    AOB_EVENT_PACS_CCCD_WRITTEN_IND               = 19,
    AOB_EVENT_SIRK_REFRESHED                      = 20,
    AOB_EVENT_CIS_ESTABLISHED_IND                 = 21,
    AOB_EVENT_CIS_REJECTED_IND                    = 22,
    AOB_EVENT_CIG_TERMINATED_IND                  = 23,
    AOB_EVENT_ASE_NTF_VALUE_IND                   = 24,
    AOB_EVENT_ASE_CODEC_CFG_VALUE_IND             = 25,
    AOB_EVENT_ASE_METADATA_UPDATA_IND             = 26,
    AOB_EVENT_BIS_SINK_STATUS_IND                 = 27,
    AOB_EVENT_BIS_SINK_ENABLE_IND                 = 28,
    AOB_EVENT_BIS_SINK_DISABLE_IND                = 29,
    AOB_EVENT_BIS_SINK_STREAM_START_IND           = 30,
    AOB_EVENT_BIS_SINK_STREAM_STOP_IND            = 31,
    AOB_EVENT_BIS_DELEGE_SOURCE_ADD_IND           = 32,
    AOB_EVENT_BIS_DELEGE_SOURCE_RM_IND            = 33,
    AOB_EVENT_BIS_DELEGE_SOURCE_UPD_IND           = 34,

    AOB_EVENT_LAST                                = 0xFF,

} AOB_EVENT_E;

typedef struct {
    AOB_EVENT_E                         type;
    uint8_t                             length;
} AOB_EVENT_HEADER_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    AOB_ADV_STATE_T                     adv_state;
    uint8_t                             err_code;
} AOB_EVENT_ADV_STATE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    AOB_CONNECTION_STATE_T              state;
    uint32_t                            evt_type;
    uint8_t                             conidx;
    ble_bdaddr_t                        peer_bdaddr;
} AOB_EVENT_TWS_STATE_T;

typedef struct {
    AOB_EVENT_HEADER_T                   header;
    AOB_CONNECTION_STATE_T               state;
    uint32_t                             evt_type;
    uint8_t                              conidx;
    ble_bdaddr_t                         peer_bdaddr;
} AOB_EVENT_MOB_STATE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             volume;
    uint8_t                             mute;
    uint8_t                             change_counter;
    uint8_t                             reason;
} AOB_EVENT_VOL_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    int16_t                             offset;
    uint8_t                             output_lid;
} AOB_EVENT_VOCS_OFFSET_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             output_lid;
    uint8_t                             cli_cfg_bf;
} AOB_EVENT_VOCS_BOND_DATA_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
} AOB_EVENT_MEDIA_TRACK_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             ase_lid;
    AOB_MGR_STREAM_STATE_E              state;
} AOB_EVENT_STREAM_STATUS_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    AOB_MGR_PLAYBACK_STATE_E            state;
} AOB_EVENT_PLAYBACK_STATUS_CHANGED_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             mute;
} AOB_EVENT_MIC_STATE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    AOB_ISO_LINK_QUALITY_INFO_T         param;
} AOB_EVENT_ISO_LINK_QUALITY_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
} AOB_EVENT_PACS_CCCD_WRITTEN_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    void                                *param;
} AOB_EVENT_CALL_STATE_CHANGE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             value;
} AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    bool                                inband_ring;
    bool                                silent_mode;
} AOB_EVENT_CALL_STATUS_FLAGS_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    bool                                local_hold_op_supported;
    bool                                join_op_supported;
} AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             call_id;
    uint8_t                             reason;
} AOB_EVENT_CALL_TERMINATE_REASON_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             url_len;
    uint8_t                             *url;
} AOB_EVENT_CALL_INCOMING_NUM_INF_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
} AOB_EVENT_CALL_SVC_CHANGED_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    void                                *param;
} AOB_EVENT_CALL_ACTION_RESULT_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    AOB_UC_SRV_CIS_INFO_T      *ascs_cis_established;
} AOB_EVENT_CIS_ESTABLISHED_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint16_t                            con_hdl;
    uint8_t                             error;
} AOB_EVENT_CIS_REJECTED_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             cig_id;
    uint8_t                             group_lid;
    uint8_t                             stream_lid;
} AOB_EVENT_CIG_TERMINATED_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             opcode;
    uint8_t                             nb_ases;
    uint8_t                             ase_lid;
    uint8_t                             rsp_code;
    uint8_t                             reason;
} AOB_EVENT_ASE_NTF_VALUE_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             ase_lid;
    AOB_CODEC_ID_T                      codec_id;
    uint8_t                             tgt_latency;
    AOB_BAP_CFG_T                       codec_cfg_req;
    AOB_BAP_QOS_REQ_T                   ntf_qos_req;
} AOB_EVENT_ASE_CODEC_CFG_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             con_lid;
    uint8_t                             ase_lid;
    void                                *param;
    uint8_t                             state;
} AOB_EVENT_ASE_METADATA_UPDATA_IND_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             grp_lid;
    uint8_t                             state;
    uint32_t                            stream_pos_bf;
} AOB_EVENT_BIS_SINK_STATUS_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             grp_lid;
} AOB_EVENT_BIS_SINK_ENABLE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             grp_lid;
} AOB_EVENT_BIS_SINK_DISABLE_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             grp_lid;
} AOB_EVENT_BIS_SINK_STREAM_START_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             grp_lid;
} AOB_EVENT_BIS_SINK_STREAM_STOP_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             src_lid;
    uint8_t                             con_lid;
    uint8_t                             pa_syn_req;
} AOB_EVENT_BIS_DELEG_SOURCE_ADD_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             src_lid;
    uint8_t                             con_lid;
} AOB_EVENT_BIS_DELEG_SOURCE_RM_T;

typedef struct {
    AOB_EVENT_HEADER_T                  header;
    uint8_t                             src_lid;
    uint8_t                             con_lid;
    uint8_t                             pa_syn_req;
} AOB_EVENT_BIS_DELEG_SOURCE_UPD_T;

typedef union {
    AOB_EVENT_TWS_STATE_T                         aob_tws_connection_state;
    AOB_EVENT_MOB_STATE_T                         aob_mob_connection_state;
    AOB_EVENT_VOL_CHANGED_T                       aob_vol_changed;
    AOB_EVENT_VOCS_OFFSET_CHANGED_T               aob_vocs_offset_changed;
    AOB_EVENT_VOCS_BOND_DATA_CHANGED_T            aob_vocs_bond_data_changed;
    AOB_EVENT_MEDIA_TRACK_CHANGED_T               aob_media_track_changed;
    AOB_EVENT_STREAM_STATUS_CHANGED_T             aob_stream_state;
    AOB_EVENT_PLAYBACK_STATUS_CHANGED_T           aob_playback_status_changed;
    AOB_EVENT_MIC_STATE_T                         aob_mic_state;
    AOB_EVENT_ISO_LINK_QUALITY_IND_T              aob_iso_link_quality_ind;
    AOB_EVENT_PACS_CCCD_WRITTEN_IND_T             aob_pacs_cccd_written_ind;
    AOB_EVENT_CALL_STATE_CHANGE_T                 aob_call_state_change;
    AOB_EVENT_CALL_SRV_SIG_STRENGTH_VALUE_IND_T   aob_call_srv_sig_strength_value_ind;
    AOB_EVENT_CALL_STATUS_FLAGS_IND_T             aob_call_status_flags_ind;
    AOB_EVENT_CALL_CCP_OPT_SUPPORTED_OPCODE_IND_T aob_call_ccp_opt_support_opcode_ind;
    AOB_EVENT_CALL_TERMINATE_REASON_IND_T         aob_call_terminate_reason_ind;
    AOB_EVENT_CALL_INCOMING_NUM_INF_IND_T         aob_call_incoming_num_inf_ind;
    AOB_EVENT_CALL_SVC_CHANGED_IND_T              aob_call_svc_changed_ind;
    AOB_EVENT_CALL_ACTION_RESULT_IND_T            aob_call_action_result_ind;
    AOB_EVENT_CIS_ESTABLISHED_IND_T               aob_cis_established_ind;
    AOB_EVENT_CIS_REJECTED_IND_T                  aob_cis_rejected_ind;
    AOB_EVENT_CIG_TERMINATED_IND_T                aob_cig_terminated_ind;
    AOB_EVENT_ASE_NTF_VALUE_IND_T                 aob_ase_ntf_value_ind;
    AOB_EVENT_ASE_CODEC_CFG_T                     aob_ase_codec_cfg_value_ind;
    AOB_EVENT_ASE_METADATA_UPDATA_IND_T           aob_ase_metadata_update_ind;
    AOB_EVENT_BIS_SINK_STATUS_T                   aob_bis_sink_state_ind;
    AOB_EVENT_BIS_SINK_ENABLE_T                   aob_bis_sink_enable_ind;
    AOB_EVENT_BIS_SINK_DISABLE_T                  aob_bis_sink_disable_ind;
    AOB_EVENT_BIS_SINK_STREAM_START_T             aob_bis_sink_stream_start_ind;
    AOB_EVENT_BIS_SINK_STREAM_STOP_T              aob_bis_sink_stream_stop_ind;
    AOB_EVENT_BIS_DELEG_SOURCE_ADD_T              aob_bis_deleg_source_add_ind;
    AOB_EVENT_BIS_DELEG_SOURCE_RM_T               aob_bis_deleg_source_removed_ind;
    AOB_EVENT_BIS_DELEG_SOURCE_UPD_T              aob_bis_deleg_source_upd_ind;
} AOB_EVENT_PACKET;

/**
 * @brief define BLE connection event for upper layer
 *
 */
typedef enum
{
    /// TWS CONN EVENT
    BLE_TWS_CONNECTING,
    BLE_TWS_CANCEL_CONNECTION,
    BLE_TWS_CONNECTION_CANCELED,
    BLE_TWS_CONNECTION_FAILED,
    BLE_TWS_CONNECTION_TIMEOUT,
    BLE_TWS_CONNECTED,
    BLE_TWS_DISCONNECTING,
    BLE_TWS_DISCONNECTED,  // 7

    /// MOB CONN EVENT
    BLE_MOB_CONNECTING,
    BLE_MOB_CANCEL_CONNECTION,
    BLE_MOB_CONNECTION_CANCELED,
    BLE_MOB_CONNECTION_FAILED,
    BLE_MOB_CONNECTION_TIMEOUT,
    BLE_MOB_BOND_SUCCESS,
    BLE_MOB_BOND_FAIL,
    BLE_MOB_ENCRYPT,
    BLE_MOB_CONNECTED,
    BLE_MOB_AUD_ATTR_BOND,
    BLE_MOB_DISCONNECTING,   // 16
    BLE_MOB_DISCONNECTED,

    // ADV STATE
    BLE_AUD_ADV_STARTED,
    BLE_AUD_ADV_DELETING,

    /// CALL

    /// GAF

} BLE_AUD_CORE_EVT_E;

/**
 * @brief ble audio core event callback structure define
*/
typedef struct
{
    void (*ble_tws_sirk_refreshed)();
    void (*ble_audio_adv_state_changed)(AOB_ADV_STATE_T state, uint8_t err_code);
    void (*ble_tws_acl_state_changed)(uint32_t evt_type, ble_event_handled_t *p);
    void (*ble_mob_acl_state_changed)(uint32_t evt_type, ble_bdaddr_t *peer_addr, uint8_t con_idx,uint8_t err_code);
    void (*ble_vol_changed)(uint8_t con_lid, uint8_t volume, uint8_t mute, uint8_t change_counter, uint8_t reason);
    void (*ble_vcs_cccd_changed_cb)(uint8_t con_lid, uint8_t currentVol, bool notifyEnabled);
    void (*ble_vocs_offset_changed_cb)(int16_t offset, uint8_t output_lid);
    void (*ble_vocs_bond_data_changed_cb)(uint8_t output_lid, uint8_t cli_cfg_bf);
    void (*ble_media_track_change_cb)(uint8_t con_lid);
    void (*ble_media_stream_status_change_cb)(uint8_t con_lid, uint8_t ase_lid, AOB_MGR_STREAM_STATE_E state);
    void (*ble_media_playback_status_change_cb)(uint8_t con_lid, AOB_MGR_PLAYBACK_STATE_E state);
    void (*ble_media_mic_state_cb)(uint8_t mute);
    void (*ble_media_iso_link_quality_cb)(void *event);
    void (*ble_media_pacs_cccd_written_cb)(uint8_t con_lid);
    void (*ble_con_pacs_cccd_gatt_load_cb)(uint8_t con_lid, uint8_t gatt_load_status);
    void (*ble_call_state_change_cb)(uint8_t con_lid, void *param);
    void (*ble_call_srv_signal_strength_value_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t value);
    void (*ble_call_status_flags_ind_cb)(uint8_t con_lid, uint8_t call_id, bool inband_ring, bool silent_mode);
    void (*ble_call_ccp_opt_supported_opcode_ind_cb)(uint8_t con_lid, bool local_hold_op_supported, bool join_op_supported);
    void (*ble_call_terminate_reason_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t reason);
    void (*ble_call_incoming_number_inf_ind_cb)(uint8_t con_lid, uint8_t call_id, uint8_t url_len, uint8_t *url);
    void (*ble_call_svc_changed_ind_cb)(uint8_t con_lid);
    void (*ble_call_action_result_ind_cb)(uint8_t con_lid, void *param);
    void (*ble_csip_ntf_sent_cb)(uint8_t con_lid, uint8_t char_type);
    void (*ble_csip_read_rsp_sent_cb)(uint8_t con_lid, uint8_t char_type, uint8_t *p_data, uint8_t data_len);
    void (*ble_csip_rsi_updated_cb)(uint8_t *rsi);
    void (*ble_cis_established)(AOB_UC_SRV_CIS_INFO_T *ascs_cis_established);
    void (*ble_cis_disconnected)(uint8_t cig_id, uint8_t cis_id, uint8_t status, uint8_t reason);
    void (*ble_cis_rejected)(uint16_t con_hdl, uint8_t error);
    void (*ble_cig_terminated)(uint8_t cig_id, uint8_t group_lid, uint8_t stream_lid, uint8_t reason);
    void (*ble_ase_ntf_value_cb)(uint8_t opcode, uint8_t nb_ases, uint8_t ase_lid, uint8_t rsp_code, uint8_t reason);
    void (*ble_ase_cp_cccd_written_cb)(uint8_t con_lid);
    void (*ble_ase_codec_cfg_req_cb)(uint8_t ase_lid, const AOB_CODEC_ID_T *codec_id, uint8_t tgt_latency,
                                    AOB_BAP_CFG_T *codec_cfg_req, AOB_BAP_QOS_REQ_T *ntf_qos_req);
    void (*ble_ase_update_metadata_req_cb)(uint8_t con_lid, uint8_t ase_lid, void *context, uint8_t ase_state);
    void (*ble_bis_sink_status_cb)(uint8_t grp_lid, uint8_t state, uint32_t stream_pos_bf);
    void (*ble_bis_sink_enabled_cb)(uint8_t grp_lid);
    void (*ble_bis_sink_disabled_cb)(uint8_t grp_lid);
    void (*ble_bis_sink_stream_start_cb)(uint8_t grp_lid);
    void (*ble_bis_sink_stream_stoped_cb)(uint8_t grp_lid);
    void (*ble_bis_deleg_scolite_start_cb)(void);
    void (*ble_bis_deleg_scolite_stoped_cb)(void);
    void (*ble_bis_deleg_source_add_ri_cb)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req);
    void (*ble_bis_deleg_source_rm_ri_cb)(uint8_t src_lid, uint8_t con_lid);
    void (*ble_bis_deleg_source_upd_ri_cb)(uint8_t src_lid, uint8_t con_lid, uint8_t pa_sync_req);
    void (*ble_audio_connected_cb)(uint8_t con_lid, uint8_t *peer_bdaddr);
} BLE_AUD_CORE_EVT_CB_T;

typedef struct
{
      bool mute_new_music_stream;
      bool pause_new_music_stream;
} BLE_AUDIO_POLICY_CONFIG_T;

typedef enum app_ble_audio_event {
    BLE_AUDIO_MUSIC_QOS_CONFIG_IND          = 0x00,
    BLE_AUDIO_MUSIC_ENABLE_REQ,
    BLE_AUDIO_MUSIC_RELEASE_REQ,
    BLE_AUDIO_MUSIC_STREAM_START_IND,
    BLE_AUDIO_MUSIC_STREAM_STOP_IND,

    BLE_AUDIO_CALL_QOS_CONFIG_IND,
    BLE_AUDIO_CALL_ENABLE_REQ,
    BLE_AUDIO_CALL_RELEASE_REQ,
    BLE_AUDIO_CALL_STREAM_START_IND,
    BLE_AUDIO_CALL_SINGLE_STREAM_STOP_IND,
    BLE_AUDIO_CALL_CAPTURE_STREAM_STOP_IND,
    BLE_AUDIO_CALL_PLAYBACK_STREAM_STOP_IND,
    BLE_AUDIO_CALL_ALL_STREAMS_STOP_IND,
    BLE_AUDIO_CALL_RINGING_IND,
    BLE_AUDIO_CALL_ACTIVE_IND,
    BLE_AUDIO_CALL_TERMINATE_IND,
    BLE_AUDIO_CALL_ALERTING_IND,        //16-0x10

    BLE_AUDIO_FLEXIBLE_ENABLE_REQ,
    BLE_AUDIO_FLEXIBLE_RELEASE_REQ,
    BLE_AUDIO_FLEXIBLE_STREAM_START_IND,
    BLE_AUDIO_FLEXIBLE_CAPTURE_STREAM_STOP_IND,
    BLE_AUDIO_FLEXIBLE_PLAYBACK_STREAM_STOP_IND,
    BLE_AUDIO_FLEXIBLE_ALL_STREAMS_STOP_IND,  //22-0x16

    BLE_AUDIO_BIS_STREAM_START_IND,
    BLE_AUDIO_BIS_STREAM_STOP_IND,

    BLE_AUDIO_CIS_CONNECTED_IND,
    BLE_AUDIO_PROMPT_SOUND_ENABLE_REQ,
    BLE_AUDIO_PROMPT_SOUND_START,      //30
    BLE_AUDIO_PROMPT_SOUND_STOP,
    BLE_AUDIO_LE_LINK_CONNECTED_IND,
    BLE_AUDIO_LE_LINK_DISCONCETED_IND,     //25-0x19

    BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_CALL,
    BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_MUSIC,
    BLE_AUDIO_UPDATE_CONTEXT_TYPE_TO_FLEXIBLE,
    BLE_AUDIO_EVENT_ROUTE_CALL_TO_BT,
    BLE_AUDIO_TOGGLE_A2DP_CIS_REQ,
    BLE_AUDIO_MEDIA_PLAYSTATUS_CHANGED,
    BLE_AUDIO_CALL_STATUS_CHANGED, //32-0x1F
    BLE_AUDIO_MAX_IND,
} app_ble_audio_event_t;

void bes_ble_iso_quality(uint16_t cisHdl, uint8_t *param);

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_AOB_COMMON_H__ */
