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
#ifndef __AOB_CONN_API_H__
#define __AOB_CONN_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "gatt_service.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

typedef enum gatt_cache_restore_state
{
    /// 0 is reserved
    GATT_C_PACS_MIN_POS = 0,
    /// SRV
    GATT_C_PACS_POS,
    GATT_C_ASCS_POS,
    GATT_C_BATT_POS,
    GATT_C_VCS_POS,
    GATT_C_MICS_POS,
    GATT_C_CSIS_POS,
    GATT_C_BASS_POS,
    /// CLI
    GATT_C_MCS_POS  = 8,
    GATT_C_TBS_POS,

    /// Others
    GATT_C_SVC_SET_CFG_DONE,

    GATT_C_PACS_MAX_POS,
} gatt_c_restore_svc_pos_e;

/****************************function declearation**************************/

/**
 ****************************************************************************************
 * @brief Create TWS connection
 *
 * @param[in] timeout_ms: Timeout for TWS connection
 ****************************************************************************************
 */
void aob_conn_create_tws_link(uint32_t timeout_ms);

/**
 ****************************************************************************************
 * @brief Disconnect TWS link
 *
 ****************************************************************************************
 */
int aob_conn_disconnect_tws_link(void);

/**
 ****************************************************************************************
 * @brief Cancel TWS connection
 *
 ****************************************************************************************
 */
void aob_conn_cancel_connecting_tws(void);

/**
 ****************************************************************************************
 * @brief Check if the TWS is connected
 * @return True: TWS Connected
 *         False: TWS not connected
 *
 ****************************************************************************************
 */
bool aob_conn_is_tws_connected(void);

/**
 ****************************************************************************************
 * @brief get TWS connection local id
 * @return connection id
 *
 ****************************************************************************************
 */
uint8_t aob_conn_get_tws_con_lid(void);

#ifdef CUSTOMER_DEFINE_ADV_DATA
/**
 ***************************************************************************************
 * @brief refresh adv
 * *************************************************************************************
 */
void aob_conn_adv_data_refresh(void);
#endif

/**
 ****************************************************************************************
 * @brief start ADV
 *
 ****************************************************************************************
 */
bool aob_conn_start_adv(bool br_edr_support, bool discoverable, bool init_reconnect);

/**
 ****************************************************************************************
 * @brief stop ADV
 *
 ****************************************************************************************
 */
bool aob_conn_stop_adv(void);

/**
 ****************************************************************************************
 * @brief Create Mobile connection
 *
 * @param[in] addr: The address need to connect
 ****************************************************************************************
 */
void aob_conn_create_mobile_link(ble_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Disconnect Mobile link
 *
 * @param[in] addr: The address need to disconnect
 ****************************************************************************************
 */
void aob_conn_disconnect_mobile_link(ble_bdaddr_t *addr);

/**
 ****************************************************************************************
 * @brief Get mobile link id
 *
 * @param[out] con_lid
 *
 * @return connected device count
 ****************************************************************************************
 */
uint8_t aob_conn_get_connected_mobile_lid(uint8_t con_lid[]);

/**
 ****************************************************************************************
 * @brief Get remote device address
 *
 * @param[in] con_lid: index of remote device
 *
 * @return see pointer of remote device see@ble_bdaddr_t
 ****************************************************************************************
 */
ble_bdaddr_t *aob_conn_get_remote_address(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief Get mobile link id
 *
 * @param[in] conidx: index of remote device
 *            object: get remote or local tx power see@ble_tx_object_e
 *            phy   : PHY select see@ble_phy_pwr_value_e
 ****************************************************************************************
 */
void aob_conn_get_tx_power(uint8_t conidx, ble_tx_object_e object, ble_phy_pwr_value_e phy);

/**
 ****************************************************************************************
 * @brief Get mobile link id
 *
 * @param[in] conidx:        index of remote device
 *            local_enable:  local tx power report , 0:off , 1:on
 *            remote_enable: remote tx power report , 0:off , 1:on
 ****************************************************************************************
 */
void aob_conn_tx_power_report_enable(uint8_t conidx, bool local_enable, bool remote_enable);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] conidx: index of remote device
 *            enable: 0, disable path loss report and the other parameter(except conidx) are useless
 *                    1, set path loss parameter and enable path loss report
 *            high_threshold  :high_threshold for the path loss
 *            high_hysteresis :hysteresis value for the high_threshold
 *            low_threshold   :low_threshold for the path loss
 *            low_hysteresis  :hysteresis value for the low_threshold
 *            min_time        :min time in number of connection event to be observed
 ****************************************************************************************
 */
void aob_conn_set_path_loss_param(uint8_t conidx, uint8_t enable, uint8_t high_threshold,
                                  uint8_t high_hysteresis, uint8_t low_threshold,
                                  uint8_t low_hysteresis, uint8_t min_time);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] subrate_min      : Minimum subrate factor allowed in requests by a Peripheral
 *            subrate_max      : Maximum subrate factor allowed in requests by a Peripheral
 *            latency_max      : Maximum Peripheral latency allowed in requests by a Peripheral,
 *                               inunits of subrated connection intervals
 *            continuation_num : Minimum number of underlying connection events to remain active
 *                               after a packet containing a Link Layer PDU with a non-zero Length
 *                               field is sent or received in requests by a Peripheral
 *            timeout          : Maximum supervision timeout allowed in requests by a Periphera
 ****************************************************************************************
 */
void aob_conn_set_default_subrate(uint16_t subrate_min, uint16_t subrate_max, uint16_t latency_max,
                                  uint16_t continuation_num, uint16_t timeout);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] conidx: index of remote device
 *            subrate_min      : Minimum subrate factor allowed in requests by a Peripheral
 *            subrate_max      : Maximum subrate factor allowed in requests by a Peripheral
 *            latency_max      : Maximum Peripheral latency allowed in requests by a Peripheral,
 *                               inunits of subrated connection intervals
 *            continuation_num : Minimum number of underlying connection events to remain active
 *                               after a packet containing a Link Layer PDU with a non-zero Length
 *                               field is sent or received in requests by a Peripheral
 *            timeout          : Maximum supervision timeout allowed in requests by a Periphera
 ****************************************************************************************
 */
void aob_conn_subrate_request(uint8_t conidx, uint16_t subrate_min, uint16_t subrate_max,
                              uint16_t latency_max, uint16_t continuation_num, uint16_t timeout);

/**
 ****************************************************************************************
 * @brief Get mobile link id
 *
 * @param[in] conidx: index of remote device
 *            object: get remote or local tx power see@ble_tx_object_e
 *            phy   : PHY select see@ble_phy_pwr_value_e
 ****************************************************************************************
 */
void aob_conn_get_tx_power(uint8_t conidx, ble_tx_object_e object, ble_phy_pwr_value_e phy);

/**
 ****************************************************************************************
 * @brief Get mobile link id
 *
 * @param[in] conidx:        index of remote device
 *            local_enable:  local tx power report , 0:off , 1:on
 *            remote_enable: remote tx power report , 0:off , 1:on
 ****************************************************************************************
 */
void aob_conn_tx_power_report_enable(uint8_t conidx, bool local_enable, bool remote_enable);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] conidx: index of remote device
 *            enable: 0, disable path loss report and the other parameter(except conidx) are useless
 *                    1, set path loss parameter and enable path loss report
 *            high_threshold  :high_threshold for the path loss
 *            high_hysteresis :hysteresis value for the high_threshold
 *            low_threshold   :low_threshold for the path loss
 *            low_hysteresis  :hysteresis value for the low_threshold
 *            min_time        :min time in number of connection event to be observed
 ****************************************************************************************
 */
void aob_conn_set_path_loss_param(uint8_t conidx, uint8_t enable, uint8_t high_threshold,
                                  uint8_t high_hysteresis, uint8_t low_threshold,
                                  uint8_t low_hysteresis, uint8_t min_time);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] subrate_min      : Minimum subrate factor allowed in requests by a Peripheral
 *            subrate_max      : Maximum subrate factor allowed in requests by a Peripheral
 *            latency_max      : Maximum Peripheral latency allowed in requests by a Peripheral,
 *                               inunits of subrated connection intervals
 *            continuation_num : Minimum number of underlying connection events to remain active
 *                               after a packet containing a Link Layer PDU with a non-zero Length
 *                               field is sent or received in requests by a Peripheral
 *            timeout          : Maximum supervision timeout allowed in requests by a Periphera
 ****************************************************************************************
 */
void aob_conn_set_default_subrate(uint16_t subrate_min, uint16_t subrate_max, uint16_t latency_max,
                                  uint16_t continuation_num, uint16_t timeout);

/**
 ****************************************************************************************
 * @brief enable path loss monitoring
 *
 * @param[in] conidx: index of remote device
 *            subrate_min      : Minimum subrate factor allowed in requests by a Peripheral
 *            subrate_max      : Maximum subrate factor allowed in requests by a Peripheral
 *            latency_max      : Maximum Peripheral latency allowed in requests by a Peripheral,
 *                               inunits of subrated connection intervals
 *            continuation_num : Minimum number of underlying connection events to remain active
 *                               after a packet containing a Link Layer PDU with a non-zero Length
 *                               field is sent or received in requests by a Peripheral
 *            timeout          : Maximum supervision timeout allowed in requests by a Periphera
 ****************************************************************************************
 */
void aob_conn_subrate_request(uint8_t conidx, uint16_t subrate_min, uint16_t subrate_max,
                              uint16_t latency_max, uint16_t continuation_num, uint16_t timeout);

/**
 ****************************************************************************************
 * @brief restore gatt cccd data
 *
 * @param con_lid
 ****************************************************************************************
 */
void aob_conn_restore_gatt_cccd_cache_data(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief check service restore state and start discovery or set cfg (CCCD)
 *
 * @param con_lid
 ****************************************************************************************
 */
void aob_conn_check_svc_restore_and_cfg_cccd(uint8_t con_lid);

/**
 ****************************************************************************************
 * @brief clear gatt_c_restore_svc_pos_e about svc restore state
 *
 * @param con_lid
 * @param svc_pos see @gatt_c_restore_svc_pos_e
 ****************************************************************************************
 */
void aob_conn_clr_gatt_cache_restore_state(uint8_t con_lid, gatt_c_restore_svc_pos_e svc_pos);

void aob_conn_api_init(void);

void aob_conn_dump_state_info(void);
/**
 ****************************************************************************************
 * @brief get conhdl
 *
 * @param[in] conidx: index of remote device
*  return :conhdl,if value is 0xFFFF mean can't find conhdl for the conidx
****************************************************************************************
*/
uint16_t aob_ble_get_conhdl(uint8_t conidx);

/*
 ******************************************************************************************
 *  Set connection is encrypted ot not
 ******************************************************************************************
*/
void aob_conn_set_encrypt_state(uint8_t con_lid, bool is_encrypt);

/*
 ******************************************************************************************
 *  Check connection is encrypted ot not
 ******************************************************************************************
*/
bool aob_conn_get_encrypt_state(uint8_t con_lid);

#ifdef __cplusplus
}
#endif

#endif

