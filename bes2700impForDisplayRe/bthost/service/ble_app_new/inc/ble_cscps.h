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

#ifndef _BLE_CSCPS_H_
#define _BLE_CSCPS_H_

#include "ble_cscp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Parameters of the initialization function
struct cscps_db_cfg
{
    /**
     * CSC Feature Value - Not supposed to be modified during the lifetime of the device
     * This value is used to decide if the SC Control Point Characteristic is part of the
     * Cycling Speed and Cadence service.
     */
    uint16_t    csc_feature;
    /**
     * Indicate if the Sensor Location characteristic is supported. Note that if the Multiple
     * Sensor Location feature is set has supported in the csc_feature parameter, the
     * characteristic will be added (mandatory).
     */
    uint8_t     sensor_loc_supp;
    /// Sensor location
    uint8_t     sensor_loc;
    /// Wheel revolution
    uint32_t    wheel_rev;
};

/// Operation Code used in the profile state machine
enum cscps_op_code
{
    /// Reserved Operation Code
    CSCPS_RESERVED_OP_CODE          = 0x00,
    /// Send CSC Measurement Operation Code
    CSCPS_SEND_CSC_MEAS_OP_CODE     = 0x01,
    /// Send SC Control point response
    CSCPS_CTNL_PT_RSP_OP_CODE       = 0x02,
};

/// Cycling speed and cadence server callback set
typedef struct cscps_cb
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
     * @param[in] char_code     Characteristic Code (CSC Measurement or SC Control Point)
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint8_t char_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Inform that peer device requests an action using control point
     *
     * @note control point request must be answered using #cscps_ctnl_pt_rsp_send function
     *
     * @param[in] conidx        Connection index
     * @param[in] op_code       Operation Code (see #cscp_sc_ctnl_pt_op_code)
     * @param[in] p_value       Pointer to control point request value
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req)(uint8_t conidx, uint8_t op_code, const union cscp_sc_ctnl_pt_req_val* p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} cscps_cb_t;

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * @param[in] conidx              Connection index
 * @param[in] csc_meas_ntf_cfg    Stored CSC Measurement Char. Client Characteristic Configuration
 * @param[in] sc_ctnl_pt_ntf_cfg  Stored SC Control Point Char. Client Characteristic Configuration
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscps_enable(uint8_t conidx, uint16_t csc_meas_ntf_cfg, uint16_t sc_ctnl_pt_ntf_cfg);

/**
 ****************************************************************************************
 * @brief Send a measurement to registered peer devices
 *
 * Wait for #cb_meas_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx_bf        Bit field of connection index on which measurement will be send
 *                             will be mask according to client that supports measurement reception
 * @param[in] wheel_rev        Wheel Revolution since the last wheel event time
 * @param[in] p_meas           Pointer to the CSC measurement
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscps_meas_send(uint32_t conidx_bf, int16_t wheel_rev, const cscp_csc_meas_t* p_meas);

/**
 ****************************************************************************************
 * @brief Send control point response.
 *
 * Wait for #cb_ctnl_pt_rsp_send_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] op_code       Operation Code (see enum #cscp_sc_ctnl_pt_op_code)
 * @param[in] resp_val      Control point response value (see enum #cscp_ctnl_pt_resp_val)
 * @param[in] p_value       Pointer to control point response value
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscps_ctnl_pt_rsp_send(uint8_t conidx, uint8_t op_code, uint8_t resp_val,
                                    const union cscp_sc_ctnl_pt_rsp_val* p_value);

/**
 ****************************************************************************************
 * @brief Initilization of Cycling Speed and Cadence Profile and register gatt service
 *
 * @param[in] cscps_cfg                 @see struct cscps_db_cfg
 * @param[in] cscps_upper_cb            @see cscps_cb_t
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscps_init(struct cscps_db_cfg* cscps_cfg, const cscps_cb_t* cscps_upper_cb);

/**
 ****************************************************************************************
 * @brief Deinitilization of Cycling Speed and Cadence Profile and unregister gatt service
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscps_deinit();

#ifdef __cplusplus
}
#endif

#endif /* _BLE_CSCPS_H_ */