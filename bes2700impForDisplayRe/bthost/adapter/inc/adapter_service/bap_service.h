/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BAP_SERVICE_H__
#define __BAP_SERVICE_H__
#include "gatt_service.h"
#ifdef __cplusplus
extern "C" {
#endif

void bap_global_init(void);
void bap_set_security_requirements(uint16_t connhdl);
bt_status_t bap_start_connection(bt_addr_type_t own_addr_type);
bt_status_t bap_start_direct_connection(bt_addr_type_t own_addr_type, bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * BAP CIS and BIS
 *
 */

#define BAP_TOTAL_CIG_COUNT 8
#define BAP_MAX_CIS_PER_CIG 8
#define BAP_TOTAL_BIG_COUNT 8
#define BAP_MAX_BIS_PER_BIG 8

#define BAP_INVALID_CIG_ID 0xFF
#define BAP_INVALID_CIS_ID 0xFF
#define BAP_INVALID_BIG_ID 0xFF
#define BAP_INVALID_BIS_ID 0xFF

#define BAP_ISO_CODEC_CFG_MAX_LEN 30

/// Minimum Transport Latency (in milliseconds)
#define BAP_ISO_MIN_TRANS_LATENCY                (0x0005)
/// Maximum Transport Latency (in milliseconds)
#define BAP_ISO_MAX_TRANS_LATENCY                (0x0FA0)

/// Minimum ISO Interval value (in units of 1.25 ms)
#define BAP_ISO_MIN_INTERVAL                     (0x0004)
/// Maximum ISO Interval value (in units of 1.25 ms)
#define BAP_ISO_MAX_INTERVAL                     (0x0C80)

/// Maximum number of retransmission
#define BAP_CIS_MAX_RTN                          (0x0F)

/// Isochronous PDU Framing mode
enum bap_iso_frame
{
    /// Unframed mode
    BAP_ISO_UNFRAMED_MODE = 0,
    /// Framed mode
    BAP_ISO_FRAMED_MODE,

    BAP_ISO_FRAME_MODE_MAX,
};

enum bap_direction
{
    BAP_DIRECTION_MIN = 0,
    /// Direction sink
    BAP_DIRECTION_SINK = BAP_DIRECTION_MIN,
    /// Direction src
    BAP_DIRECTION_SRC,

    BAP_DIRECTION_MAX,
};

#define ASCS_GENERAL_ANNOUNCEMENT 0x00
#define ASCS_TARGETED_ANNOUNCEMENT 0x01

#define BAP_CONTEXT_TYPE_PROHIBITED         0x0000
#define BAP_CONTEXT_TYPE_UNSPECIFIED        0x0001
#define BAP_CONTEXT_TYPE_CONVERSATIONAL     0x0002 // telephony, video call, VoIP, Push-to-Talk
#define BAP_CONTEXT_TYPE_MEDIA              0x0004 // music playback, radio, podcast, movie soundtrack, tv audio
#define BAP_CONTEXT_TYPE_GAME               0x0008 // gaming media, gaming effects, music and in-game voice, a mix of above
#define BAP_CONTEXT_TYPE_INSTRUCTIONAL      0x0010 // navigation, announcements, user guidance
#define BAP_CONTEXT_TYPE_VOICE_ASSIST       0x0020 // voice recognition, virtual assistant
#define BAP_CONTEXT_TYPE_LIVE               0x0040 // live audio
#define BAP_CONTEXT_TYPE_SOUND_EFFECTS      0x0080 // keyboard and touch feedback, menu and user interface sounds, system sounds
#define BAP_CONTEXT_TYPE_NOTIFICATIONS      0x0100 // notification and reminder sounds, attention-seeking audio
#define BAP_CONTEXT_TYPE_RINGTONE           0x0200 // incoming tele or vidio call, including cellular, VoIP and Push-to-Talk
#define BAP_CONTEXT_TYPE_ALERTS             0x0400 // alarms and timers, immediate alerts, toaster, cooker, kettle, micowave, etc.
#define BAP_CONTEXT_TYPE_EMERGENCY_ALARM    0x0800 // fire alarms, other urgent alerts

#define BAP_AVIALABLE_SINK_CONTEXTS \
    (BAP_CONTEXT_TYPE_UNSPECIFIED | \
     BAP_CONTEXT_TYPE_CONVERSATIONAL | \
     BAP_CONTEXT_TYPE_MEDIA | \
     BAP_CONTEXT_TYPE_GAME | \
     BAP_CONTEXT_TYPE_LIVE)

#define BAP_AVIALABLE_SOURCE_CONTEXTS \
    (BAP_CONTEXT_TYPE_CONVERSATIONAL | \
     BAP_CONTEXT_TYPE_MEDIA | \
     BAP_CONTEXT_TYPE_GAME | \
     BAP_CONTEXT_TYPE_VOICE_ASSIST | \
     BAP_CONTEXT_TYPE_LIVE)

/**
 * ascs_service_data_t *service_data = NULL;
 *  uint8_t ascs_raw_data[6] = {0};
 *  service_data = (ascs_service_data_t *)ascs_raw_data;
 *  service_data->announcement_type = directed ? ASCS_TARGETED_ANNOUNCEMENT : ASCS_GENERAL_ANNOUNCEMENT;
 *  service_data->available_sink_contexts = co_host_to_uint16_le(BAP_AVIALABLE_SINK_CONTEXTS);
 *  service_data->available_source_contexts = co_host_to_uint16_le(BAP_AVIALABLE_SOURCE_CONTEXTS);
 *  service_data->metadata_length = 0;
 *  gap_dt_add_service_data(adv_data_buf, GATT_UUID_ASC_SERVICE, (uint8_t *)service_data, sizeof(ascs_raw_data));
 *
 */
typedef struct {
    uint8_t announcement_type; // 0x00 general, 0x01 target
    uint16_t available_sink_contexts;
    uint16_t available_source_contexts;
    uint8_t metadata_length;
    gap_dt_head_t metadata[1];
} __attribute__ ((packed)) ascs_service_data_t;

typedef struct bap_iso_stream_t bap_iso_stream_t;

typedef struct {
    uint32_t cig_sync_delay_us; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of all CISes in a CIG event
    uint32_t cis_sync_delay_us; // 0xEA to 0x7F_FFFF, max us for transmission of PDUs of the specified CIS in a CIG event
    uint32_t transport_latency_c2p_us; // us, 0xEA to 0x7F_FFFF, the actual transport latency us from C btc to P btc
    uint32_t transport_latency_p2c_us; // us, 0xEA to 0x7F_FFFF, the actual transport latency us from P btc to C btc
    gap_le_phy_t tx_phy_c2p; // the tx PHY of packets from C to P
    gap_le_phy_t tx_phy_p2c; // the tx PHY of packets from P to C
    uint8_t nse; // 0x01 to 0x1F, max number of subevents in each ISO event (CIS event)
    uint8_t bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
    uint8_t ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t ft_p2c; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from P to C
    uint16_t max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t max_pdu_p2c; // 0x00 to 0xFB, max octets of the pdu payload from P LL to C LL
    uint16_t iso_interval_1_25ms; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
    /// Below are Core Spec 6.0 present parameters, other version may be invalid data
    uint32_t sub_interval_us; // 0:NSE=1. Time between the start of consecutive subevents in a CIS event Range: 0x000190 to ISO_Interval×1250 – 1
    uint16_t max_sdu_c2p; // Maximum size, in octets, of the payload from the Central’s Host. Range: 0 to 0x0FFF
    uint16_t max_sdu_p2c; // Maximum size, in octets, of the payload from the Peripheral’s Host. Range: 0 to 0x0FFF
    uint32_t sdu_interval_us_c2p; // Time between the start of consecutive SDUs sent by the Central. Range: 0x0000FF to 0x0FFFFF
    uint32_t sdu_interval_us_p2c; // Time between the start of consecutive SDUs sent by the Peripheral. Range: 0x0000FF to 0x0FFFFF
    uint8_t framing; // 0x00:Unframed PDUs, 0x01:Framed PDUs (Segmentable mode), Core Spec 6.0: 0x02 framed (Unsegmented mode)
} bap_cis_timing_t;

typedef struct {
    uint32_t big_sync_delay_us; // 0xEA to 0x7F_FFFF, max time in us for tx PDUs of all BISes in a BIG event, not in sync device
    uint32_t transport_latency_big_us; // 0xEA to 0x7F_FFFF, the actual transport latency in us
    uint8_t nse; // 0x01 to 0x1F, num of subevents in each BIS event in the BIG
    uint8_t bn; // 0x01 to 0x07, the number of new payloads in each BIS event
    uint8_t pto; // 0x00 to 0x0F, offset used for pre-transmissions
    uint8_t irc; // 0x01 to 0x0F, num of times a payload is transmitted in a BIS event
    uint16_t max_pdu_size; // 0x01 to 0xFB, max octets of the PDU payload
    uint16_t iso_interval_1_25ms; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, BIG anchor points interval
} bap_bis_timing_t;

typedef struct {
    uint8_t cig_id;
    uint8_t cis_count;
    uint8_t cis_id[BAP_MAX_CIS_PER_CIG];
    uint16_t cis_handle[BAP_MAX_CIS_PER_CIG];
} bap_cig_opened_t;

typedef struct {
    uint8_t cig_id;
} bap_cig_closed_t;

typedef struct {
    uint8_t error_code;
    const bap_iso_stream_t *stream;
    const bap_cis_timing_t *timing;
} bap_cis_opened_t;

typedef struct {
    uint8_t error_code;
    const bap_iso_stream_t *stream;
} bap_cis_closed_t;

typedef struct {
    uint8_t error_code;
    const bap_iso_stream_t *stream;
} bap_cis_open_req_t;

typedef bap_cis_closed_t bap_cis_rejected_t;

typedef struct {
    uint8_t big_id;
    uint8_t bis_count;
    bool is_broadcaster;
    uint8_t pa_train_adv_handle;
    uint16_t err_code;
    uint16_t sync_handle;
    const bap_iso_stream_t *stream[BAP_MAX_BIS_PER_BIG];
    const bap_bis_timing_t *timing;
} bap_big_opened_t;

typedef struct {
    uint8_t big_id;
    uint16_t err_code;
} bap_big_closed_t;

typedef struct {
    uint8_t error_code;
    bool tx_path_setup;
    bool rx_path_setup;
    const bap_iso_stream_t *stream;
} bap_iso_path_setup_t;

typedef union {
    void *param_ptr;
    bap_cig_opened_t *cig_opened;
    bap_cig_opened_t *cig_update;
    bap_cig_closed_t *cig_closed;
    bap_cis_opened_t *cis_opened;
    bap_cis_closed_t *cis_closed;
    bap_cis_open_req_t *cis_open_req;
    bap_cis_rejected_t *cis_rejected;
    bap_big_opened_t *big_opened;
    bap_big_closed_t *big_closed;
    bap_iso_path_setup_t *iso_path_setup;
    bap_iso_path_setup_t *iso_path_removed;
} bap_event_param_t;

typedef enum {
    BAP_EVENT_CIG_OPENED = BT_EVENT_BAP_EVENT_START,
    BAP_EVENT_CIG_UPDATE,
    BAP_EVENT_CIG_CLOSED,
    BAP_EVENT_CIS_OPENED,
    BAP_EVENT_CIS_CLOSED,
    BAP_EVENT_CIS_OPEN_REQ,
    BAP_EVENT_CIS_REJECTED,
    BAP_EVENT_BIG_OPENED,
    BAP_EVENT_BIG_CLOSED,
    BAP_EVENT_ISO_PATH_SETUP,
    BAP_EVENT_ISO_PATH_REMOVED,
} bap_event_t;

typedef int (*bap_event_callback_t)(uintptr_t group_id, bap_event_t event, bap_event_param_t param);

typedef struct {
    uint8_t is_opened: 1;
    uint8_t is_central: 1;
    uint8_t is_broadcaster: 1;
    uint8_t cis_stream: 1;
    uint8_t tx_path_setup: 1;
    uint8_t rx_path_setup: 1;
} bap_iso_flag_t;

typedef struct bap_iso_stream_t {
    bap_iso_flag_t iso_flag;
    uint8_t group_id; // cig or big id
    uint8_t stream_id; // cis or bis id
    uint8_t iso_conidx; // cis or bis index in group
    uint16_t iso_handle; // cis_handle or bis_handle
    uint16_t connhdl; // acl_handle or adv_handle or sync_handle
    bap_event_callback_t bap_callback;
} bap_iso_stream_t;

typedef struct {
    bap_iso_stream_t stream;
    bap_cis_timing_t timing;
    uint8_t cis_accept_timer;
} bap_cis_t;

typedef struct {
    bap_iso_stream_t stream;
} bap_bis_t;

typedef struct {
    uint32_t sdu_interval_c2p_us; // 0xFF to 0x0F_FFFF, SDU interval from the C Host for all CISes in CIG
    uint32_t sdu_interval_p2c_us; // 0xFF to 0x0F_FFFF, SDU interval from the P host for all CISes in CIG
    uint8_t worst_case_sca; // worst case sleep clock accuracy of all the Peripherals that will participate in the CIG
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, this is a recommendation the controller may ignore
    uint8_t framing; // 0x00 unframed, 0x01 framed (Segmentable mode), Core Spec 6.0: 0x02 framed (Unsegmented mode)
    uint16_t max_transport_latency_c2p_ms; // 0x05 to 0x0FA0, max transport latency from C btc to P btc, ignored for test cmd
    uint16_t max_transport_latency_p2c_ms; // 0x05 to 0x0FA0, max transport latency from C btc to P btc, ignored for test cmd
    uint8_t test_ft_c2p; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from C to P
    uint8_t test_ft_p2c; // 0x01 to 0xFF, flush timeout in multiples of ISO_Interval for each payload sent from P to C
    uint16_t test_iso_interval_1_25ms; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, CIS anchor points interval
} bap_cig_param_t;

typedef struct {
    uint16_t max_sdu_c2p; // 0x0000 to 0x0FFF, max octets of the sdu payload from C host
    uint16_t max_sdu_p2c; // 0x0000 to 0x0FFF, max octets of the sdu payload from P Host
    uint8_t phy_bits_c2p; // bit 0 - C TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t phy_bits_p2c; // bit 0 - P TX PHY is LE 1M, bit 1 - LE 2M, bit 2 - LE Coded, host shall set at least one bit
    uint8_t rtn_c2p; // retx number of every CIS Data PDU from C to P before ack or flushed, just recommendation, ignore for test
    uint8_t rtn_p2c; // retx number of every CIS Data PDU from P to C before ack or flushed, just recommendation, ignore for test
    uint16_t test_max_pdu_c2p; // 0x00 to 0xFB, max octets of the pdu payload from C LL to P LL
    uint16_t test_max_pdu_p2c; // 0x00 to 0xFB, max octets of the pdu payload from P LL to C LL
    uint8_t test_nse; // 0x01 to 0x1F, max number of subevents in each CIS event
    uint8_t test_bn_c2p; // 0x00 no ISO data from C to P, 0x01 to 0x0F BN for C to P transmission
    uint8_t test_bn_p2c; // 0x00 no ISO data from P to C, 0x01 to 0x0F BN for P to C transmission
    uint8_t cis_id; // only valid when update cis configure
} bap_cis_param_t;

uint8_t bap_set_cig_parameters(bap_event_callback_t cb, const bap_cig_param_t *cig, const bap_cis_param_t *cis, uint8_t cis_count, bool use_test_cmd);
bt_status_t bap_add_cis_configure(uint8_t cig_id, const bap_cis_param_t *cis, uint8_t cis_count);
bt_status_t bap_update_cis_configure(uint8_t cig_id, const bap_cis_param_t *cis, uint8_t cis_count);
bt_status_t bap_remove_cig(uint8_t cig_id);
bt_status_t bap_create_cis(uint16_t cis_handle, uint16_t connhdl);
bt_status_t bap_create_multi_cis(uint8_t cis_count, const uint16_t *cis_handle, const uint16_t *connhdl);
bt_status_t bap_register_cis_callback(uint16_t connhdl, bap_event_callback_t cb);
bt_status_t bap_accept_cis_request(uint16_t cis_handle, bool accept);
bt_status_t bap_disconnect_cis(uint16_t cis_handle);
bt_status_t bap_disconnect_associated_cis(uint16_t connhdl);
bap_cis_t *bap_get_cis_item(uint16_t cis_handle);

typedef struct {
    uint8_t bis_count; // 0x01 to 0x1F
    uint8_t phy_bits; // bit 0: transmitter phy is LE 1M, bit 1: LE 2M, bit 2: LE Coded, Host shall set at least one bit
    uint16_t max_sdu_size; // 0x01 to 0x0FFFF, max octets of an SDU
    uint32_t sdu_interval_us; // 0xFF to 0x0F_FFFF, the interval in us of periodic SDUs
    uint8_t packing; // 0x00 sequential, 0x01 interleaved, just recommendation
    uint8_t framing; // 0x00 unframed, 0x01 framed (Segmentable mode), Core Spec 6.0: 0x02 framed (Unsegmented mode)
    uint8_t broadcast_code[16]; // used to derive the session key that is used to encrypt and decrpt BIS payloads
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t rtn; // 0x00 to 0x1E, retrans num of every BIS data PDU, just recommend, ignored for test cmd
    uint16_t max_transport_latency_us; // 0x05 to 0x0FA0, max transport latency in us, includes pre-transmissions, ignored for test cmd
    uint16_t test_iso_interval_1_25ms; // 0x04 to 0x0C80, per 1.25ms, 5ms to 4s, interval between BIG anchor points
    uint16_t test_max_pdu_size; // 0x01 to 0xFB, max octets of PDU payload
    uint8_t test_nse;// 0x01 to 0x1F, total number of subevents in each interval of each BIS in the BIG
    uint8_t test_bn; // 0x01 to 0x07, number of new payloads in each interval for each BIS
    uint8_t test_irc; // 0x01 to 0x0F, number of times the scheduled payloads are transmitted in a given event
    uint8_t test_pto; // 0x00 to 0x0F, offset used for pre-transmissions
} bap_big_param_t;

typedef struct {
    uint8_t bis_count; // 0x01 to 0x1F, total number of BISes to synchronize
    uint8_t bis_index[BAP_MAX_BIS_PER_BIG]; // 0x01 to 0x1F, index of a BIS in the BIG
    uint16_t big_sync_timeout; // 0x0A to 0x4000, per 10ms, 100ms to 163.84s, sync timeout for the BIG
    uint8_t broadcast_code[16]; // used for deriving the session key for decrypting payloads of BISes in the BIG
    uint8_t encryption; // 0x00 unencrypted, 0x01 encrypted
    uint8_t mse; // 0x00 controler can schedule reception of any num of se up to NSE, 0x01 to 0x1F max num of se should be used
} bap_big_sync_param_t;

bt_status_t bap_create_big(bap_event_callback_t cb, uint8_t pa_train_adv_handle, const bap_big_param_t *big, bool use_test_cmd);
bt_status_t bap_terminate_big(uint8_t big_id);
bt_status_t bap_create_big_sync(bap_event_callback_t cb, uint16_t sync_handle, const bap_big_sync_param_t *big);
bt_status_t bap_terminate_big_sync(uint8_t big_id);
bt_status_t bap_terminate_big_sync_by_sync_hdl(uint16_t sync_handle);
bt_status_t bap_set_big_sec_level(uint8_t big_id, gap_big_sec_level_t sec_level);
bap_bis_t *bap_get_bis_item(uint16_t bis_handle);

typedef struct bap_big_info
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
    /// Framing mode (0x00: Unframed, 0x01: Framed (Segmentable mode),
    /// Core Spec 6.0: 0x02 framed (Unsegmented mode), All other values: RFU)
    uint8_t   framing;
    /// True if broadcast isochronous group is encrypted, False otherwise
    bool      encrypted;
} bap_big_info_t;

typedef struct {
    uint32_t controller_delay_us;
    uint8_t codec_id[5];
    uint8_t codec_cfg_len;
    uint8_t codec_cfg[BAP_ISO_CODEC_CFG_MAX_LEN];
} bap_iso_param_t;

bt_status_t bap_setup_iso_tx_data_path(uint16_t iso_handle, const bap_iso_param_t *param);
bt_status_t bap_setup_iso_rx_data_path(uint16_t iso_handle, const bap_iso_param_t *param);
bt_status_t bap_remove_iso_data_path(uint16_t iso_handle);
bt_status_t bap_remove_iso_rx_data_path(uint16_t iso_handle);
bt_status_t bap_remove_iso_tx_data_path(uint16_t iso_handle);

typedef struct bap_cis_node_t
{
    struct bap_cis_node_t *next;
    bap_cis_t cis;
} bap_cis_node_t;

typedef struct
{
    bap_cis_node_t *next;
} bap_cis_list_t;

typedef struct
{
    uint8_t cig_id; // 0x00 to 0xEF
    uint8_t cig_op_id;
    uint8_t cig_op_cis_count;
    uint16_t connhdl; // acl_handle only valid when peripheral, otherwise 0xFFFF
    bap_event_callback_t bap_callback;
    bap_cis_list_t cis_list;
    bap_cig_param_t cig_param;
    uint8_t cig_op_id_list[BAP_MAX_CIS_PER_CIG];
} bap_cig_t;

typedef struct
{
    bap_cig_t *cig;
} bap_cig_item_t;

typedef struct bap_big_node_t bap_big_node_t;

typedef struct
{
    bap_big_node_t *next;
} bap_big_list_t;

typedef struct
{
    bap_cig_item_t cig[BAP_TOTAL_CIG_COUNT];
    bap_big_list_t big_list;
    uint8_t big_count;
    uint8_t adv_handle;
} bap_global_t;

#ifdef __cplusplus
}
#endif
#endif /* __BAP_SERVICE_H__ */
