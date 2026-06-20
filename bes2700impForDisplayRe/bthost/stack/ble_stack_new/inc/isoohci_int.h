/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifndef __ISOOHCI_INT_H__
#define __ISOOHCI_INT_H__
#include "bap_service.h"

#include "arch.h"
#include "co_bt.h"

#include "rwble_config.h"
#ifdef __cplusplus
extern "C" {
#endif

#define ISOOHCI_INVALID_BUF_IDX    (0xFF)

/// HCI ISO_Data_Load - Length of Time_Stamp field
#define HCI_ISO_DATA_LOAD_TIME_STAMP_LEN    (4)

/// HCI ISO_Data_Load - Length of Packet Sequence Number field
#define HCI_ISO_DATA_LOAD_PKT_SEQ_NB_LEN    (2)

/// HCI ISO_Data_Load - Length of ISO SDU Length and packet status flags field
#define HCI_ISO_DATA_LOAD_ISO_SDU_LEN_LEN   (2)

/// HCI ISO_Data_Load - maximum header length
#define HCI_ISO_DATA_LOAD_HDR_LEN_MAX       (HCI_ISO_DATA_LOAD_TIME_STAMP_LEN + HCI_ISO_DATA_LOAD_PKT_SEQ_NB_LEN + HCI_ISO_DATA_LOAD_ISO_SDU_LEN_LEN)

///HCI Transport Header length - change if different transport
#define HCI_TRANSPORT_HDR_LEN               (0x01)

#define HCI_ISO_HDR_HDL_FLAGS_LEN           (2)

#define HCI_ISO_HDR_ISO_DATA_LOAD_LEN_LEN   (2)

///HCI ACL data packet header length
#define HCI_ISO_HDR_LEN                     (HCI_ISO_HDR_HDL_FLAGS_LEN + HCI_ISO_HDR_ISO_DATA_LOAD_LEN_LEN)

/// Isochronous Channel data path selection
enum iso_dp_type
{
    /// ISO over HCI Data Path
    ISO_DP_ISOOHCI                  = 0x00,
    /// Data Path direction is disabled
    ISO_DP_DISABLE                  = 0xFF,

    // vendor specifics
    // @see enum dp_type
    // @see enum plf_dp_type
};

/// Isochronous Data Path Direction
enum iso_dp_direction
{
    /// Input (Host to Controller)
    ISO_DP_INPUT                     = 0x00,
    /// Output (Controller to Host)
    ISO_DP_OUTPUT                    = 0x01,
};

typedef struct isoohci_buffer
{
    /// Time_Stamp
    uint32_t        time_stamp;
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    /// length of the ISO SDU (in bytes)
    uint16_t        sdu_length;
    /// Connection handle
    uint16_t        conhdl;
    /// Buffer index
    uint8_t         buf_idx;
    /// Reception status (@see enum hci_iso_pkt_stat_flag)
    uint8_t         status;
    /// Number of HCI ISO data packets in the SDU (used only for input direction - from Host to Controller)
    uint8_t         hci_iso_data_pkt_nb;
    /// Area reserved for HCI header (used only for output direction - from Controller to Host)
    uint8_t         hci_iso_hdr[HCI_TRANSPORT_HDR_LEN + HCI_ISO_HDR_LEN];
    /**
     * ISO_Data_Load header
     *     - Output: always present, timestamp always included
     *     - Input: only in first fragment of an SDU, timestamp may or may not be present
     */
    uint8_t         hci_iso_data_load_hdr[HCI_ISO_DATA_LOAD_HDR_LEN_MAX];
    /// SDU
    uint8_t         sdu[__ARRAY_EMPTY];
} isoohci_buffer_t;

struct isoohci_in_sdu_info_tag
{
    /// Connection handle
    uint16_t conhdl;

    /// Number of HCI ISO data packets in the SDU
    uint8_t hci_iso_data_pkt_nb;
};

struct isoohci_in_last_sdu_tx_sync
{
    /// The time stamp, in microseconds, of the reference anchor point of a transmitted SDU (Range 0x00000000-0xFFFFFFFF)
    uint32_t time_stamp;
    /// The time offset, in microseconds, that is associated with a transmitted SDU (Range 0x000000-0xFFFFFF)
    uint32_t time_offset;
    /// The packet sequence number of an SDU (Range 0x0000-0xFFFF)
    uint16_t pkt_seq_nb;
};

struct isoohci_in_info
{
    /// Timing information about last transmitted SDU
    struct isoohci_in_last_sdu_tx_sync last_sdu;
    /// Buffer of the current SDU in reception
    struct isoohci_buffer* current_buf;
    /// Buffer in use by ISOAL
    struct isoohci_buffer* buf_in_use;
    /// Max SDU Size (in bytes)
    uint16_t max_sdu_size;
    /// Length of SDU data received in current SDU (in bytes)
    uint16_t sdu_rx_len;
};

struct isoohci_out_info
{
    /// SDU packet counter
    uint32_t    sdu_cnt;
    /// Maximum SDU length (in bytes)
    uint16_t max_sdu;
    /// Total number of buffers
    uint8_t     nb_buf;
    /// Number of buffer in use (received from transport, in transmission to Host)
    uint8_t     nb_buf_in_use;
    /// Buffer index
    uint8_t     buf_idx;
    /// Data path to be removed
    bool remove;
};

typedef uint8_t (*data_path_start_cb) (uint16_t conhdl, uint32_t sdu_interval, uint32_t trans_latency, uint16_t max_sdu);
typedef void (*data_path_stop_cb) (uint16_t conhdl, uint8_t reason);
typedef uint8_t* (*data_path_sdu_get_cb) (uint16_t conhdl,uint16_t seq_num, uint32_t* p_ref_time, uint16_t* p_sdu_len);
typedef void (*data_path_sdu_done_cb) (uint16_t conhdl, uint16_t sdu_len, uint32_t ref_time, uint8_t* p_buf, uint8_t status);
typedef void (*data_path_local_sync_cb) (uint32_t t_gpio);
typedef void (*data_path_peer_sync_cb) (uint16_t evt_cnt, uint32_t ref_anchor_pt);

struct data_path_itf
{
    data_path_start_cb cb_start;
    data_path_stop_cb cb_stop;
    data_path_sdu_get_cb cb_sdu_get;
    data_path_sdu_done_cb cb_sdu_done;
    data_path_local_sync_cb cb_local_sync;
    data_path_peer_sync_cb cb_peer_sync;
};

void isoohci_in_defer_handler(void);
void isoohci_data_come_callback_register(void *callback);
void isoohci_data_comed_callback_deregister(void);
bool isoohci_data_comed_callback_already_flag(void);
void isoohci_data_rx_callback(uint16_t connhdl, int ps_flag);

void isoohci_out_defer_handler(void);
void isoohci_out_buf_release(uint8_t* iso_sdu_ptr);
void isoohci_init(uint8_t init_type);
const struct data_path_itf* isoohci_itf_get(uint8_t  direction);

void data_path_init(uint8_t init_type);
const struct data_path_itf* data_path_itf_get(uint8_t type, uint8_t direction);
bool data_path_is_disabled(const struct data_path_itf* dp);

#ifdef __cplusplus
}
#endif
#endif /* __ISOOHCI_INT_H__ */
