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
#ifndef __BT_DRV_TEST_MODE_H_
#define __BT_DRV_TEST_MODE_H_


#ifdef __cplusplus
extern "C" {
#endif

enum APP_DBG_NONSIG_PKT_TYPE {
    APP_DBG_NONSIG_TYPE_ID_NUL = 0,
    APP_DBG_NONSIG_TYPE_POLL   = 1,
    APP_DBG_NONSIG_TYPE_FHS    = 2,
    APP_DBG_NONSIG_TYPE_DM1    = 3,
    APP_DBG_NONSIG_TYPE_DH1    = 4,
    APP_DBG_NONSIG_TYPE_DH1_2  = 5,
    APP_DBG_NONSIG_TYPE_DH1_3  = 6,
    APP_DBG_NONSIG_TYPE_HV1    = 7,
    APP_DBG_NONSIG_TYPE_HV2    = 8,
    APP_DBG_NONSIG_TYPE_EV3_2  = 9,
    APP_DBG_NONSIG_TYPE_HV3    = 10,
    APP_DBG_NONSIG_TYPE_EV3    = 11,
    APP_DBG_NONSIG_TYPE_EV3_3  = 12,
    APP_DBG_NONSIG_TYPE_DV     = 13,
    APP_DBG_NONSIG_TYPE_AUX1   = 14,
    APP_DBG_NONSIG_TYPE_DM3    = 15,
    APP_DBG_NONSIG_TYPE_DH3    = 16,
    APP_DBG_NONSIG_TYPE_DH3_2  = 17,
    APP_DBG_NONSIG_TYPE_DH3_3  = 18,
    APP_DBG_NONSIG_TYPE_EV4    = 19,
    APP_DBG_NONSIG_TYPE_EV5_2  = 20,
    APP_DBG_NONSIG_TYPE_EV5    = 21,
    APP_DBG_NONSIG_TYPE_EV5_3  = 22,
    APP_DBG_NONSIG_TYPE_DM5    = 23,
    APP_DBG_NONSIG_TYPE_DH5    = 24,
    APP_DBG_NONSIG_TYPE_DH5_2  = 25,
    APP_DBG_NONSIG_TYPE_DH5_3  = 26,
    APP_DBG_NONSIG_TYPE_END    = 27,
};

///Transmitter test Packet Payload Type
enum
{
    ///Pseudo-random 9 TX test payload type
    PAYL_PSEUDO_RAND_9            = 0x00,
    ///11110000 TX test payload type
    PAYL_11110000,
    ///10101010 TX test payload type
    PAYL_10101010,
    ///Pseudo-random 15 TX test payload type
    PAYL_PSEUDO_RAND_15,
    ///All 1s TX test payload type
    PAYL_ALL_1,
    ///All 0s TX test payload type
    PAYL_ALL_0,
    ///00001111 TX test payload type
    PAYL_00001111,
    ///01010101 TX test payload type
    PAYL_01010101,
};

#define MAX_SWITCHING_PATTERN_LEN  (0x4B)

#define BLE_TX_V3_CMD_MAX_LEN  (11 + MAX_SWITCHING_PATTERN_LEN)
#define BLE_RX_V3_CMD_MAX_LEN  (11 + MAX_SWITCHING_PATTERN_LEN)

#define BLE_TX_V4_CMD_MAX_LEN  (12 + MAX_SWITCHING_PATTERN_LEN)

typedef struct
{
    enum APP_DBG_NONSIG_PKT_TYPE enum_pkt_type;
    uint8_t packet_type;
    uint8_t edr_enabled;
    uint16_t payload_length;
}APP_DBG_NONSIG_TESTER_PKT_TYPE_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t power_level;
    uint8_t addr[6];
    enum APP_DBG_NONSIG_PKT_TYPE btdrv_pkt_type;
    uint8_t payload_pattern;
}APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T;

typedef struct
{
    uint8_t rx_freq;
    uint8_t addr[6];
    enum APP_DBG_NONSIG_PKT_TYPE btdrv_pkt_type;
}APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T;

typedef struct
{
    uint16_t pkt_counters;
    uint16_t head_errors;
    uint16_t payload_errors;
}APP_DBG_NONSIG_TESTER_RESULT_BT_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t test_data_len;
    uint8_t pkt_payload;
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V1_T;

typedef struct
{
    uint8_t rx_freq;
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V1_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V2_T;

typedef struct
{
    uint8_t rx_freq;
    uint8_t phy;
    uint8_t modulation_idx;
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V2_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
    uint8_t cte_len;
    uint8_t cte_type;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V3_T;

typedef struct
{
    uint8_t rx_freq;
    uint8_t phy;
    uint8_t modulation_idx;
    uint8_t exp_cte_len;
    uint8_t exp_cte_type;
    uint8_t slot_durations;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V3_T;

typedef struct
{
    uint8_t tx_freq;
    uint8_t test_data_len;
    uint8_t pkt_payload;
    uint8_t phy;
    uint8_t cte_len;
    uint8_t cte_type;
    uint8_t switching_pattern_len;
    uint8_t antenna_id[MAX_SWITCHING_PATTERN_LEN];
    int8_t tx_pwr_lvl;
}APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V4_T;

typedef struct
{
    uint16_t pkt_counters;
}APP_DBG_NONSIG_TESTER_RESULT_BLE_T;


int app_testmode_enter(uint8_t mode);
int app_testmode_exit(void);
int app_testmode_bt_nonsig_tx(APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T *tx_cfg);
int app_testmode_bt_nonsig_rx(APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T *rx_cfg);
int app_testmode_bt_nonsig_endtest(void);
int app_testmode_bt_nonsig_result_get(APP_DBG_NONSIG_TESTER_RESULT_BT_T *bt_result);
int app_testmode_ble_endtest(void);
int app_testmode_ble_result_get(APP_DBG_NONSIG_TESTER_RESULT_BLE_T *ble_result);
int app_testmode_enable_dut(void);
void app_enter_normal_mode(void);
void app_enter_signal_testmode(void);
void app_exit_signal_testmode(void);
void app_enter_nosignal_tx_testmode(void);
void app_enter_nosignal_rx_testmode(void);
void app_exit_nosignal_trx_testmode(void);
int app_testmode_nonsig_test_result_save(const unsigned char *data, unsigned int len);
void app_enter_ble_tx_v1_testmode(void);
void app_enter_ble_rx_v1_testmode(void);
void app_enter_ble_tx_v2_testmode(void);
void app_enter_ble_rx_v2_testmode(void);
void app_enter_ble_tx_v3_testmode(void);
void app_enter_ble_rx_v3_testmode(void);
void app_enter_ble_tx_v4_testmode(void);
void app_exit_ble_trx_testmode(void);


#ifdef __cplusplus
}
#endif

#endif

