/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

#ifndef __APP_TWS_IBRT_ANALYSIS_SYSTEM__
#define __APP_TWS_IBRT_ANALYSIS_SYSTEM__

#include "stdint.h"

#define BT_NETWORK_TOPOLOGY_MAX_ACTIVE_CONN_NUM 7
#define BT_LINK_LOSS_AFH_MAP_LEN (3 * 4)

#ifndef BT_LINK_LOSS_RX_CLOCK_INFO_SIZE
#define BT_LINK_LOSS_RX_CLOCK_INFO_SIZE 25
#endif

typedef struct link_conn_role_info {
    uint8_t num_conns : 3;
    uint8_t link_role : 1;
    uint8_t rfu :       4;
} __attribute__ ((__packed__)) link_conn_role_info_t;

typedef struct link_loss_lsto_tx_info {
    uint8_t lsto_tx_type :            5;
    uint8_t lsto_tx_br_edr :          1;
    uint8_t lsto_tx_BB_flow_stop :    1;
    uint8_t lsto_tx_l2cap_flow_stop : 1;
} __attribute__ ((__packed__)) link_loss_lsto_tx_info_t;

typedef struct link_loss_lsto_rx_info {
    uint8_t lsto_rx_type :            5;
    uint8_t lsto_rx_br_edr :          1;
    uint8_t lsto_rx_BB_flow_stop :    1;
    uint8_t lsto_rx_l2cap_flow_stop : 1;
} __attribute__ ((__packed__)) link_loss_lsto_rx_info_t;

typedef struct network_topology_info {
    uint16_t                  nt_conn_hdl[BT_NETWORK_TOPOLOGY_MAX_ACTIVE_CONN_NUM];
    uint8_t                   nt_role[BT_NETWORK_TOPOLOGY_MAX_ACTIVE_CONN_NUM];
} __attribute__ ((__packed__)) network_topology_info_t;

//link loss clock info
typedef struct tws_ibrt_link_loss_clock_info {
    uint32_t                  nmu_no_sync_anchor;
    uint32_t                  Rx_clk[BT_LINK_LOSS_RX_CLOCK_INFO_SIZE];
    uint8_t                   rf_channel_num[BT_LINK_LOSS_RX_CLOCK_INFO_SIZE];
} __attribute__ ((__packed__)) tws_ibrt_link_loss_clock_info_t;

//link loss universal info
typedef struct tws_ibrt_link_loss_universal_info {
    uint16_t                  conn_hdl;
    link_conn_role_info_t     num_conn_role;
    int8_t                    LST_RSSI;
    int8_t                    LSR_RSSI;
    uint32_t                  LS_clk;
    uint8_t                   AFH_state;
    uint8_t                   AFH_MAP[BT_LINK_LOSS_AFH_MAP_LEN];
    uint32_t                  super_tmot;
    uint8_t                   sniff_mode;
    uint16_t                  sniff_offset;
    uint16_t                  sniff_interval;
    uint16_t                  sniff_atmt;
    uint16_t                  sniff_tmot;
    uint16_t                  sub_latency;
    uint16_t                  sub_min_Rmt_to;
    uint16_t                  sub_min_Lcl_to;
    link_loss_lsto_tx_info_t  lsto_tx_info;
    uint8_t                   lsto_tx_pwr;
    uint16_t                  lsto_tx_lmp;
    uint16_t                  lsto_tx_retr_tx;
    link_loss_lsto_rx_info_t  lsto_rx_info;
    uint16_t                  lsto_rx_lmp;
    uint16_t                  lmp_op_tmot;
    network_topology_info_t   net_topo_info;
} __attribute__ ((__packed__)) tws_ibrt_link_loss_universal_info_t;

typedef struct tws_ibrt_link_loss_info {
    tws_ibrt_link_loss_universal_info_t  link_loss_universal_info;
    tws_ibrt_link_loss_clock_info_t      link_loss_clock_info;
} __attribute__ ((__packed__)) tws_ibrt_link_loss_info_t;

//A2DP sink informations
typedef struct tws_ibrt_a2dp_sink_info {
    uint16_t                  conn_hdl;
    uint8_t                   retran_cnt;
    uint16_t                  ant_gt;
    uint16_t                  hec_cnt;
    uint16_t                  crc_cnt;
    uint16_t                  sync_tmot_cnt; //Count of sync timeout
    uint8_t                   lmp_tx_cnt;
    uint8_t                   pull_tx_cnt;
    uint8_t                   null_tx_cnt;
    uint8_t                   lmp_rx_cnt;
    uint8_t                   pull_rx_cnt;
    uint8_t                   null_rx_cnt;
    //latest good packet information
    uint8_t                   last_txpwr;
    int8_t                    last_rssi;
    uint8_t                   last_channel;
    uint32_t                  last_pic_clk;
    uint32_t                  last_jitter_buf_cnt;
    //previous good packet information
    uint8_t                   pre_txpwr;
    int8_t                    pre_rssi;
    uint8_t                   pre_channel;
    uint32_t                  pre_pic_clk;
    uint32_t                  pre_jitter_buf_cnt;
} __attribute__ ((__packed__)) tws_ibrt_a2dp_sink_info_t;

typedef enum
{
    LINK_LOSS_UNISERVAL_INFO_TYPE = 0,
    LINK_LOSS_CLOCK_INFO_TYPE,
    A2DP_SINK_INFO_TYPE,

    ANALYSIS_INFO_UNKNOWN_TYPE,
} ANALYSISING_INFO_TYPE;

typedef void (*app_tws_ibrt_analysing_info_received_cb)(ANALYSISING_INFO_TYPE info_type, uint16_t conn_hdl, uint8_t* info, uint8_t info_len);

void app_tws_ibrt_register_link_loss_universal_info_received_cb(app_tws_ibrt_analysing_info_received_cb function);

void app_tws_ibrt_register_link_loss_clock_info_received_cb(app_tws_ibrt_analysing_info_received_cb function);

void app_tws_ibrt_register_a2dp_sink_info_received_cb(app_tws_ibrt_analysing_info_received_cb function);

tws_ibrt_link_loss_info_t* app_tws_ibrt_get_wireless_signal_link_loss_total_info(void);

tws_ibrt_a2dp_sink_info_t* app_tws_ibrt_get_wireless_signal_a2dp_sink_total_info(void);

bool app_tws_ibrt_update_wireless_signal_analysing_link_loss_universal_info(uint8_t* info, uint32_t info_len);

bool app_tws_ibrt_update_wireless_signal_analysing_link_loss_clock_info(uint8_t* info, uint32_t info_len);

bool app_tws_ibrt_update_wireless_signal_analysing_a2dp_sink_info(uint8_t* info, uint32_t info_len);

#endif
