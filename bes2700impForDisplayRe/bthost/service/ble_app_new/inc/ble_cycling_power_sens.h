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

#ifndef __BLE_CYCLING_POWER_SENS_H__
#define __BLE_CYCLING_POWER_SENS_H__

#include "ble_cycling_power_i.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Cycling power service server callback set
typedef struct ble_cps_cb
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
     * @brief Completion of vector transmission
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_vector_send_cmp)(uint16_t status);

    /**
     ****************************************************************************************
     * @brief Inform that bond data updated for the connection.
     *
     * @param[in] conidx        Connection index
     * @param[in] char_code     Characteristic Code (CP Measurement or Control Point)
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @note control point request must be answered using #cps_ctrl_pt_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see enum #cpp_ctrl_pt_code)
     * @param[in] p_value       Pointer to control point request value
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_req)(uint8_t conidx, uint8_t op_code, const union ble_cps_ctrl_pt_req_val *p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} ble_cps_cb_t;


struct ble_cps_db_cfg
{
    /**
     * CP Feature Value - Not supposed to be modified during the lifetime of the device
     * This bit field is set in order to decide which features are supported:
     *   Supported features (specification) ---------------- Bits 0 to 18
     */
    uint32_t cp_feature;
    /// Initial count for wheel revolutions
    uint32_t wheel_rev;
    /**
     * Profile characteristic configuration:
     *   Enable broadcaster mode in Measurement Characteristic --- Bit 0
     *   Enable Control Point Characteristic (*) ----------------- Bit 1
     *
     * (*) Note this characteristic is mandatory if server supports:
     *     - Wheel Revolution Data
     *     - Multiple Sensor Locations
     *     - Configurable Settings
     *     - Offset Compensation
     *     - Server allows to be requested for parameters (CPP_CTNL_PT_REQ... codes)
     */
    uint8_t prfl_config;
    /**
     * Indicate the sensor location.
     */
    uint8_t sensor_loc;
};

/**
 ****************************************************************************************
 * @brief Initilization of Cycling Power Sensor and register gatt service
 *
 * @param[in] p_init_cfg        @see struct ble_cps_db_cfg
 * @param[in] p_cb              @see ble_cps_cb_t
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_init(struct ble_cps_db_cfg *p_init_cfg, const ble_cps_cb_t *p_cb);

/**
 ****************************************************************************************
 * @brief Deinitilization of Cycling Power Sensor and unregister gatt service
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_deinit(void);

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx            Connection index
 * @param[in] ntf_ind_cfg       Profile characteristic configuration:
 *                                   - Bit 0: Measurement Characteristic notification config
 *                                   - Bit 1: Measurement Characteristic broadcast config
 *                                   - Bit 2: Vector Characteristic notification config
 *                                   - Bit 3: Control Point Characteristic indication config
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_enable(uint8_t conidx, uint8_t ntf_ind_cfg);

/**
 ****************************************************************************************
 * @brief Pack measurement information for advertising
 *
 * @param[in] p_buf   Pointer to output buffer - Buffer tail length must support a value up
 *                    to CPP_CP_MEAS_ADV_MAX_LEN
 * @param[in] buf_len p_buf value length up to CPP_CP_MEAS_ADV_MAX_LEN
 * @param[in] p_meas  Pointer of CPP measurement
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_adv_data_pack(uint8_t *p_buf, uint8_t buf_len, const ble_cps_cp_meas_t *p_meas);

/**
 ****************************************************************************************
 * @brief Send a cycling power measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports measurement reception
 * @param[in] cumul_wheel_rev  Cumulative Wheel Revolutions
 * @param[in] p_meas           Pointer to the cycling power measurement
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_meas_send(uint32_t conidx_bf, int16_t cumul_wheel_rev, const ble_cps_cp_meas_t *p_meas);

/**
 ****************************************************************************************
 * @brief Send a cycling power vector to registered peer devices
 *
 * Wait for #cb_vector_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf      Bit field of connection index on which measurement will be send
 *                           will be mask according to client that supports measurement reception
 * @param[in] p_vector       Pointer to the cycling power vector information
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_vector_send(uint32_t conidx_bf, const ble_cps_cp_vector_t *p_vector);

/**
 ****************************************************************************************
 * @brief Send control point response.
 *
 * Wait for #cb_ctrl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Operation Code (see enum #cpp_ctrl_pt_code)
 * @param[in] resp_val      Control point response value (see enum #cpp_ctrl_pt_resp_val)
 * @param[in] p_value       Pointer to control point response value
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cps_ctrl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val, const union ble_cps_ctrl_pt_rsp_val *p_value);

#ifdef __cplusplus
}
#endif

#endif // __BLE_CYCLING_POWER_SENS_H__
