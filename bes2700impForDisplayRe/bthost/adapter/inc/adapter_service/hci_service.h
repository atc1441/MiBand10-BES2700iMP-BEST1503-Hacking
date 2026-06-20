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
#ifndef __BT_HCI_SERVICE_H__
#define __BT_HCI_SERVICE_H__
#include "bt_common_define.h"
#include "hci_transport.h"
#include "co_ppbuff.h"
#ifdef __cplusplus
extern "C" {
#endif

/// HCI ISO_Data_Load - Packet Status Flag
typedef enum
{
    /// Valid data. The complete ISO_SDU was received correctly
    GAF_ISO_PKT_STATUS_VALID   = (0),
    /// Possibly invalid data. The contents of the ISO_SDU may contain errors or part of the ISO_SDU may
    /// be missing. This is reported as "data with possible errors".
    GAF_ISO_PKT_STATUS_INVALID = (1),
    /// Part(s) of the ISO_SDU were not received correctly. This is reported as "lost data".
    GAF_ISO_PKT_STATUS_LOST    = (2),
} GAF_ISO_PKT_STATUS_E;

typedef struct
{
    /// Time_Stamp
    uint32_t        time_stamp;
    /// Packet Sequence Number
    uint16_t        pkt_seq_nb;
    uint16_t        data_len;
    uint8_t         *origin_buffer;
    uint8_t         *sdu_data;
    /// the status of data packet
    GAF_ISO_PKT_STATUS_E pkt_status;
    uint8_t         cisChannel;
    uint16_t        conhdl;
#ifdef GAF_CODEC_CROSS_CORE
    bool            isFeedoffCore;
#endif
    bool            isPLC;
} gaf_media_data_t;

struct hci_evt_packet_t {
    uint8 evt_code;
    uint8 length;
    uint8 data[1];
} __attribute__ ((packed));

struct hci_cmd_evt_param_t {
    const uint8_t *origin_cmd;
    const uint8_t *cmpl_event;
    uint8_t cmpl_event_len;
    uint8_t event_status;
    uint8_t event_code;
    uint8_t subevent;
    void *priv;
    void *cont;
    void *cmpl;
};

typedef enum {
    HCI_STREAM_TYPE_LE_CIS = 0,
    HCI_STREAM_TYPE_LE_BIS = 1,
} hci_stream_type_t;

struct hci_conn_flag_t {
    uint16_t is_in_use: 1;
    uint16_t conn_type: 3;
    uint16_t stream_type: 1;
    uint16_t addr_type: 3;
    uint16_t is_central: 1;
    uint16_t is_opened: 1;
    uint16_t is_tx_streaming: 1;
    uint16_t out_disconnecting: 1;
    uint16_t cannot_continue_send: 1;
    uint16_t acl_rx_flow_is_stopped: 1;
    uint16_t is_near_link_type: 1;
} __attribute__ ((packed));

typedef struct hci_conn_item_t {
    struct hci_conn_flag_t flag;
    bt_bdaddr_t peer_addr;
    uint16_t connhdl;
    uint16_t associate_acl_handle;
    uint8_t host_completed_rx_packets;
    uint8_t controller_pend_tx_packets;
    uint8_t group_id;
    uint8_t stream_id;
    void *priv_param;
} __attribute__ ((packed)) hci_conn_item_t;

#ifdef BLE_HOST_SUPPORT
struct hci_le_callback_t {
    void (*le_acl_tx_done)(uint16_t connhdl);
    void (*rx_le_acl_data)(uint16_t connhdl_flags, const uint8_t *l2cap_hdr_ptr, uint16_t data_len);
    void (*rx_le_evt_data)(struct hci_evt_packet_t *evt_packet);
#ifdef ISO_BEARER_SUPPORT
    void *(*iso_rx_alloc)(uint32_t);
    void (*iso_rx_free)(void *);
    bool (*rx_iso_ready)(void);
    void (*rx_iso_data)(uint16_t connhdl);
#endif
};
#endif

#if defined(BLE_HOST_SUPPORT) || defined(NEARLINK_HOST_SUPPORT)
#define HCI_MAX_NUM_CONN_HANDLE (25)
#else
#define HCI_MAX_NUM_CONN_HANDLE (5)
#endif

typedef void (*hci_cmd_evt_func_t)(uint16_t cmd_opcode, struct hci_cmd_evt_param_t *param); // cmd cmpl status cb func

struct hci_cmd_tx_desc_t {
    uint8_t event_wait;
    uint8_t subevt_wait;
    uint16_t connhdl;
    hci_cmd_evt_func_t priv_cmpl_status_cb;
    hci_cmd_evt_func_t cmpl_status_cb;
    void *priv;
    void *cont;
    void *cmpl;
} __attribute__ ((packed));

typedef enum {
    HCI_RX_FLOW_CONTROL_OFF = 0x00, // default
    HCI_RX_FLOW_CONTROL_ACL_ON_SCO_OFF = 0x01,
    HCI_RX_FLOW_CONTROL_ACL_OFF_SCO_ON = 0x02,
    HCI_RX_FLOW_CONTROL_ACL_ON_SCO_ON = 0x03,
} hci_rx_flow_control_enable_t;

typedef enum {
    HCI_PACKET_BASED_FLOW_CONTROL_MODE = 0x00, // default mode for br/edr traffic, only mode for le traffic
    HCI_DATA_BLOCK_BASED_FLOW_CONTROL_MODE = 0x01,
} hci_tx_flow_control_mode_t; // acl tx flow control is always on

typedef enum {
    HCI_SCO_TX_FLOW_CONTROL_OFF = 0x00,
    HCI_SCO_TX_FLOW_CONTROL_ON = 0x01,
} hci_sco_tx_flow_control_enable_t;

struct hci_config_t {
    bool hci_cmd_evt_debug_enable;
    bool hci_tx_flow_debug_enable;
    bool hci_tx_0c35_without_alloc;
    void (*hci_controller_state_check)(void);
    void (*hci_pend_too_many_acl_rx_packets)(void);
#ifdef IBRT
    bool hci_start_ibrt_reserve_buff;
    void (*hci_tx_buff_tss_process)(void);
#endif
};

#include "adapter_service.h"

typedef struct {
    uint8_t error_code;
    hci_conn_type_t conn_type;
    hci_stream_type_t stream_type;
    struct hci_conn_item_t *conn;
} bt_hci_conn_opened_t;

typedef struct {
    uint8_t error_code;
    hci_conn_type_t conn_type;
    hci_stream_type_t stream_type;
    struct hci_conn_item_t *conn;
} bt_hci_conn_closed_t;

typedef union {
    void *param_ptr;
    bt_hci_conn_opened_t *opened;
    bt_hci_conn_closed_t *closed;
} bt_hci_callback_param_t;

typedef enum {
    BT_HCI_EVENT_CONN_OPENED,
    BT_HCI_EVENT_CONN_CLOSED,
} bt_hci_event_t;

typedef int (*bt_hci_callback_t)(uintptr_t connhdl, bt_hci_event_t event, bt_hci_callback_param_t param);
typedef void (*hci_acl_tx_done_callback_t)(uint16_t connhdl, struct pp_buff *ppb);
typedef void (*hci_rx_acl_data_callback_t)(uint16_t connhdl, struct pp_buff *ppb);
typedef void (*hci_rx_evt_data_callback_t)(struct hci_evt_packet_t *evt_packet);

struct hci_bt_callback_t {
    bt_hci_callback_t hci_cb_event_handle;
    void (*bt_acl_tx_done)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_bt_acl_data)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_bt_evt_data)(struct hci_evt_packet_t *evt_packet);
	void (*rx_bt_debug_data)(struct hci_evt_packet_t *evt_packet);
#ifdef CFG_SCO_OVER_HCI
    void (*rx_bt_sco_data)(uint16_t connhdl, hci_sco_ps_flag_t sco_ps_flag, const uint8_t *data, uint8_t len);
#endif
#if defined(BLE_HOST_SUPPORT)
    void (*le_acl_tx_done)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_le_acl_data)(uint16_t connhdl, struct pp_buff *ppb);
    void (*rx_le_evt_data)(struct hci_evt_packet_t *evt_packet);
#endif
    hci_acl_tx_done_callback_t near_acl_tx_done;
    hci_rx_acl_data_callback_t rx_near_acl_data;
    hci_rx_evt_data_callback_t rx_near_evt_data;
};

struct hci_event_wait_item_t {
    struct single_link_node_t node;
    hci_cmd_evt_func_t cmpl_status_cb;
    uint16_t cmd_opcode;
    uint16_t event_wait;
    void *priv;
    void *cont;
    void *cmpl;
};

struct hci_cmd_cmpl_event_pair_t {
    uint16_t cmd_opcode;
    uint8_t event_wait;
    uint8_t subevt_wait;
};

struct hci_event_wait_t {
    struct single_link_head_t wait_q;
    uint8_t head_event_wait_timer;
};

typedef struct hci_global_t {
    bool initialized;
    bt_bdaddr_t bt_addr;
    bt_bdaddr_t le_addr;
    bt_bdaddr_t random_addr;
    osMutexId hci_buffer_mutex_id;
    struct hci_config_t hci_config;
    struct hci_bt_callback_t bt_callback;
    struct hci_conn_item_t conn_item[HCI_MAX_NUM_CONN_HANDLE];
    struct hci_event_wait_t cmpl_evt_wait;
    /**
     * fc config
     */
    hci_rx_flow_control_enable_t rx_fc_enable;
    hci_tx_flow_control_mode_t tx_fc_mode; // acl tx fc is always on
#ifdef CFG_SCO_OVER_HCI
    hci_sco_tx_flow_control_enable_t sco_tx_fc_enable;
#endif
    /**
     * iso rx
     */
#ifdef ISO_BEARER_SUPPORT
    struct single_link_head_t iso_rx_list;
    uint16_t rx_iso_host_alloced_max;
#endif
    /**
     * sco rx
     */
#ifdef CFG_SCO_OVER_HCI
    struct single_link_head_t sco_rx_list;
    uint16_t rx_sco_host_packet_left;
    uint16_t rx_sco_host_alloced_max;
#endif
    /**
     * evt & acl (bt and le ) rx
     */
    struct single_link_head_t evt_acl_rx_list; // evt and acl shall be handled in the order they received
    uint16_t rx_acl_host_packet_left;
    uint16_t rx_acl_host_alloced;
    uint16_t rx_acl_host_alloced_max;
    uint16_t rx_evt_host_alloced;
    uint16_t rx_evt_host_alloced_max;
    uint16_t rx_trace_host_alloced;
    uint16_t rx_trace_host_alloced_max;
    /**
     * cmd tx fc, cmd tx steps:
     * 1. cmd queued to cmd_pend_list and sent one by one or directly move to cmd_tx_list if no need wait
     * 2. cmd become outstanding_cmd and moved to cmd_tx_list
     * 3. hci_tx_buff_process send commands in cmd_tx_list
     * 4. wait outstanding_cmd's command complete or command status event
     * 5. do cmpl_status_cb() and finish curr outstanding_cmd
     * 6. move next cmd from cmd_pend_list to cmd_tx_list
     */
    struct single_link_head_t cmd_pend_list;
    struct single_link_head_t cmd_tx_list;
    struct pp_buff *outstanding_cmd;
    uint8_t cmd_wait_complete_timer;
    uint16_t tx_cmd_controller_left;
    uint16_t tx_cmd_host_alloced;
    uint16_t tx_cmd_host_alloced_max;
    /**
     * sco tx fc
     */
#ifdef CFG_SCO_OVER_HCI
    struct single_link_head_t sco_tx_list;
    uint16_t tx_sco_host_alloced_max;
    uint16_t tx_sco_packet_length;
    uint16_t tx_sco_controller_left;
    uint16_t tx_sco_controller_total;
#endif
    /**
     * bt acl tx fc
     */
    struct single_link_head_t bt_acl_tx_list;
    uint16_t bt_tx_acl_host_alloced_max;
    uint16_t bt_tx_acl_packet_length;
    uint16_t bt_tx_acl_controller_left;
    uint16_t bt_tx_acl_controller_total;
    bool acl_tx_wait_controller_cmpl;
    /**
     * le acl tx fc
     */
#if defined(BLE_HOST_SUPPORT)
    struct hci_le_callback_t le_callback;
#endif
#if defined(BLE_HOST_SUPPORT) || defined(NEARLINK_HOST_SUPPORT)
    struct single_link_head_t le_acl_tx_list;
    uint16_t le_tx_acl_host_alloced_max;
    uint16_t le_tx_acl_packet_length;
    uint16_t le_tx_acl_controller_left;
    uint16_t le_tx_acl_controller_total;
    bool le_share_bt_acl_tx_buffer;
#endif /* BLE_HOST_SUPPORT */
    /**
     * iso tx fc
     */
#ifdef ISO_BEARER_SUPPORT
    bool iso_tx_wait_controller_cmpl;
    struct single_link_head_t iso_tx_list;
    uint16_t tx_iso_host_alloced_max;
    uint16_t tx_iso_packet_length;
    uint16_t tx_iso_controller_left;
    uint16_t tx_iso_controller_total;
#endif
} hci_global_t;

void hci_global_init(hci_send_data_callback_t tx, hci_bt_rx_isr_callback_t rx);
void hci_register_bt_callback(const struct hci_bt_callback_t *bt_cb);
hci_global_t *hci_get_global(void);
struct hci_rx_desc_t *hci_get_rx_packet(void);
bt_status_t hci_send_bt_acl_packet(hci_conn_type_t type, uint16_t connhdl_flags, struct pp_buff *ppb);
bt_status_t hci_send_cmd_packet(struct pp_buff *ppb);
bt_status_t hci_send_command(uint16_t cmd_opcode, const uint8_t *cmd_data, uint8_t data_len);
bt_status_t hci_set_cmd_wait_event(struct pp_buff *cmd_ppb, uint8_t wait_event);
void hci_cmd_is_completed(uint8_t evt_code, uint16_t cmd_opcode, uint16_t num_cmd_packets, uint8_t status, const uint8_t *cmpl_parameters, uint8_t cmpl_param_len);
void hci_handle_fc_on_disconnect(hci_conn_type_t conn_type, struct hci_conn_item_t *item, uint16_t connhdl);
void hci_handle_controller_num_of_complete(struct hci_evt_packet_t *pkt);
void hci_free_conn_by_handle(hci_conn_type_t type, uint16_t connhdl);
void hci_foreach_near_conn_item(hci_conn_type_t conn_type, int (*cb)(void *priv_param, void *param), void *param);
int hci_count_conn_type_items(hci_conn_type_t conn_type, bool is_near_link_type);
void hci_defer_free_rx_buffer(struct pp_buff *ppb, uint32_t ca, uint32_t line);
struct hci_conn_item_t *hci_find_conn_by_priv_param(hci_conn_type_t type, void *priv_param);
struct hci_conn_item_t *hci_alloc_conn_item(hci_conn_type_t conn_type, uint8_t addr_type, const bt_bdaddr_t *peer_addr, uint16_t connhdl);
struct hci_conn_item_t *hci_find_conn_of_all_type(uint16_t connhdl);

typedef void (*hci_cmd_evt_func_t)(uint16_t cmd_opcode, struct hci_cmd_evt_param_t *param); // cmd cmpl status cb func
struct pp_buff *hci_create_cmd_packet(uint16_t cmd_opcode, uint8_t cmd_data_len, hci_cmd_evt_func_t cmpl_status_cb);
struct pp_buff *hci_create_cmd_packet_with_priv(uint16_t cmd_opcode, uint8_t cmd_data_len, hci_cmd_evt_func_t cmpl_status_cb, void *priv, void *cont, void *cmpl);
bt_status_t hci_send_command_with_callback(uint16_t cmd_opcode, const uint8_t *cmd_data, uint8_t data_len, hci_cmd_evt_func_t cmpl_status_cb);
bt_status_t hci_send_raw_comand(const uint8_t *cmd_packet, uint8_t packet_len, hci_cmd_evt_func_t cmpl_status_cb);

#ifdef BLE_HOST_SUPPORT
void hci_register_le_callback(const struct hci_bt_callback_t *bt_cb);
void hci_register_iso_callback(const struct hci_le_callback_t *bt_cb);
void hci_free_le_acl_rx_buffer(uint8_t *l2cap_hdr_ptr);
int hci_count_free_le_tx_buff(void);
#endif

#ifdef ISO_BEARER_SUPPORT
int hci_get_iso_free_packet_num();
bt_status_t hci_send_iso_packet(struct hci_tx_iso_desc_t *tx_iso_desc);
gaf_media_data_t *hci_get_iso_rx_packet(uint16_t connhdl);
void hci_clean_iso_rx_packet(uint16_t connhdl);
void hci_clean_iso_tx_packet(uint16_t connhdl);
#endif

void hci_enable_cmd_evt_debug(bool enable);
void hci_enable_tx_flow_debug(bool enable);
void hci_enable_tx_0c35_without_alloc(bool enable);
void hci_register_controller_state_check(void (*cb)(void));
void hci_register_pending_too_many_rx_acl_packets(void (*cb)(void));
#ifdef IBRT
void hci_set_start_ibrt_reserve_buff(bool reserve);
void hci_register_acl_tx_buff_tss_process(void (*cb)(void));
#endif

uint8_t *hci_get_curr_pending_cmd(uint16_t cmd_opcode);
uint16_t hci_get_curr_cmd_opcode(void);
void hci_print_statistic(void);
void hci_rxtx_buff_process(void);
int hci_count_free_bt_tx_buff(void);
const bt_bdaddr_t *hci_get_bt_address(void);
const bt_bdaddr_t *hci_get_le_address(void);
const bt_bdaddr_t *hci_get_random_address(void);

void *gap_create_hci_cmd_packet_extra(struct pp_buff **ppb_out, uint16_t cmd_opcode, uint8_t cmd_len, hci_cmd_evt_func_t cmd_cb, void *priv, void *cont, void *cmpl);
void *gap_create_hci_cmd_packet(struct pp_buff **ppb_out, uint16_t cmd_opcode, uint8_t cmd_len, hci_cmd_evt_func_t cmd_cb, void *priv);
bt_status_t gap_send_raw_hci_cmd_extra(uint16_t cmd_opcode, uint16_t cmd_len, const uint8_t *data_little_endian, hci_cmd_evt_func_t cmd_cb, void *priv, void *cont, void *cmpl);
bt_status_t gap_send_raw_hci_cmd(uint16_t cmd_opcode, uint16_t cmd_len, const uint8_t *data_little_endian, hci_cmd_evt_func_t cmd_cb, void *priv);
bt_status_t gap_send_raw_hci_command(uint16_t cmd_opcode, uint16_t cmd_len, const uint8_t *data_little_endian);
bt_status_t gap_send_short_hci_cmd(uint16_t cmd_opcode, uint16_t cmd_len, uint32_t cmd_data, hci_cmd_evt_func_t cmd_cb, void *priv);
bt_status_t gap_send_short_hci_command(uint16_t cmd_opcode, uint16_t cmd_len, uint32_t cmd_data);
bt_status_t gap_send_hci_cmd_packet(struct pp_buff *ppb);
void gap_acl_rx_slowdown_stop(hci_conn_type_t conn_type, bt_addr_type_t addr_type, const struct bdaddr_t *remote);
void gap_acl_rx_slowdown_check(hci_conn_type_t conn_type, bt_addr_type_t addr_type, const struct bdaddr_t *remote);

#ifdef __cplusplus
}
#endif
#endif /* __BT_HCI_SERVICE_H__ */
