/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#ifndef _BLE_HRPS_H_
#define _BLE_HRPS_H_

#include "ble_hrp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Operation codes
enum hrps_op_codes
{
    /// Database Creation Procedure
    HRPS_RESERVED_OP_CODE      = 0,
    /// Send Measurement value Operation Code
    HRPS_MEAS_SEND_CMD_OP_CODE = 1,
};

/// Parameters of the @ref HRPS_CREATE_DB_REQ message
struct hrps_init_cfg
{
    /// Database configuration
    uint8_t features;
    /// Body Sensor Location
    hrp_sen_loc_t body_sensor_loc;
};

/// Heart rate sensor server callback set
typedef struct hrps_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of measurement transmission
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_meas_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Inform APP that Energy Expanded must be reset value
     *
     * @param[in] conidx        Connection index
     ****************************************************************************************
     */
    void (*cb_energy_exp_reset)(uint8_t conidx);
} hrps_cb_t;

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx                Connection index
 * @param[in] hr_meas_ntf           Heart Rate Notification configuration
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_hrps_enable(uint8_t conidx, uint16_t hr_meas_ntf);

/**
 ****************************************************************************************
 * @brief Send a measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports measurement reception
 * @param[in] p_meas           Pointer to the Heart Rate measurement
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_hrps_meas_send(uint32_t conidx_bf, const hrs_hr_meas_t* p_meas);

/**
 ****************************************************************************************
 * @brief Initilization of Heart Rate Profile and register gatt service
 *
 * @param[in] hrps_cfg              @see struct hrps_init_cfg
 * @param[in] hrps_upper_cb         @see hrps_cb_t
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_hrps_init(const struct hrps_init_cfg* hrps_cfg, const hrps_cb_t* hrps_upper_cb);

/**
 ****************************************************************************************
 * @brief Deinitilization of Heart Rate Profile and unregister gatt service
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_hrps_deinit();

#ifdef __cplusplus
}
#endif

#endif /* _BLE_HRPS_H_ */
