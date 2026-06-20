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

#ifndef _BLE_CSCPC_H_
#define _BLE_CSCPC_H_

#include "ble_cscp_common.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Cycling Speed and Cadence Service Characteristic Descriptors
enum cscpc_cscs_descs
{
    /// CSC Measurement Char. - Client Characteristic Configuration
    CSCPC_DESC_CSC_MEAS_CL_CFG,
    /// SC Control Point Char. - Client Characteristic Configuration
    CSCPC_DESC_SC_CTNL_PT_CL_CFG,

    CSCPC_DESC_MAX,

    CSCPC_DESC_MASK = 0x10,
};

/// Codes for reading/writing a CSCS characteristic with one single request
enum cscpc_codes
{
    /// Notified CSC Measurement
    CSCPC_NTF_CSC_MEAS          = CSCP_CSCS_CSC_MEAS_CHAR,
    /// Read CSC Feature
    CSCPC_RD_CSC_FEAT           = CSCP_CSCS_CSC_FEAT_CHAR,
    /// Read Sensor Location
    CSCPC_RD_SENSOR_LOC         = CSCP_CSCS_SENSOR_LOC_CHAR,
    /// Indicated SC Control Point
    CSCPC_IND_SC_CTNL_PT        = CSCP_CSCS_SC_CTNL_PT_CHAR,

    /// Read/Write CSC Measurement Client Char. Configuration Descriptor
    CSCPC_RD_WR_CSC_MEAS_CFG    = (CSCPC_DESC_CSC_MEAS_CL_CFG   | CSCPC_DESC_MASK),
    /// Read SC Control Point Client Char. Configuration Descriptor
    CSCPC_RD_WR_SC_CTNL_PT_CFG  = (CSCPC_DESC_SC_CTNL_PT_CL_CFG | CSCPC_DESC_MASK),
};


/// Cycling speed and cadence client callback set
typedef struct cscpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of enable procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     ****************************************************************************************
     */
    void (*cb_enable_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] sensor_feat   CSC sensor feature
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_feat_cmp)(uint8_t conidx, uint16_t status, uint16_t sensor_feat);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor location procedure.
     *
     * Wait for #cb_read_loc_cmp execution before starting a new procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] sensor_loc    Sensor Location
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_loc_cmp)(uint8_t conidx, uint16_t status, uint8_t sensor_loc);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #cscpc_codes)
     *                              - CSCPC_RD_WR_CSC_MEAS_CFG: CSC Measurement Client Char. Configuration
     *                              - CSCPC_RD_WR_SC_CTNL_PT_CFG: SC Control Point Client Char. Configuration
     * @param[in] cfg_val       Configuration value
     *
     ****************************************************************************************
     */
    void (*cb_read_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code, uint16_t cfg_val);

    /**
     ****************************************************************************************
     * @brief Completion of write Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #hl_err)
     * @param[in] desc_code     Descriptor Code (see enum #cscpc_codes)
     *                              - CSCPC_RD_WR_CSC_MEAS_CFG: CSC Measurement Client Char. Configuration
     *                              - CSCPC_RD_WR_SC_CTNL_PT_CFG: SC Control Point Client Char. Configuration
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Function called when CSC measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_meas         Pointer to CSC measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const cscp_csc_meas_t* p_meas);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #hl_err)
     * @param[in] req_op_code   Requested Operation Code see enum #cscp_sc_ctnl_pt_op_code
     * @param[in] resp_value    Response Value see enum #cscp_ctnl_pt_resp_val
     * @param[in] p_value       Pointer to response data content
     ****************************************************************************************
     */
    void (*cb_ctnl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               const union cscp_sc_ctnl_pt_rsp_val* p_value);
} cscpc_cb_t;

/**
 ****************************************************************************************
 * @brief Restore bond data of a known peer device (at connection establishment)
 *
 * Wait for #cb_enable_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_enable(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor feature procedure.
 *
 * Wait for #cb_read_sensor_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_read_sensor_feat(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor location procedure.
 *
 * Wait for #cb_read_loc_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_read_sensor_loc(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #cscpc_codes)
 *                              - CSCPC_RD_WR_CSC_MEAS_CFG: CSC Measurement Client Char. Configuration
 *                              - CSCPC_RD_WR_SC_CTNL_PT_CFG: SC Control Point Client Char. Configuration
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_read_cfg(uint8_t conidx, uint8_t desc_code);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #cscpc_codes)
 *                              - CSCPC_RD_WR_CSC_MEAS_CFG: CSC Measurement Client Char. Configuration
 *                              - CSCPC_RD_WR_SC_CTNL_PT_CFG: SC Control Point Client Char. Configuration
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctnl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] req_op_code    Requested Operation Code see enum #cscp_sc_ctnl_pt_op_code
 * @param[in] p_value        Pointer to request data content
 *
 * @return Status of the function execution (see enum #hl_err)
 ****************************************************************************************
 */
uint16_t ble_cscpc_ctnl_pt_req(uint8_t conidx, uint8_t req_op_code, const union cscp_sc_ctnl_pt_req_val* p_value);

/**
 ****************************************************************************************
 * @brief Initilization of Cycling Speed and Cadence Profile and register gatt service
 *
 * @param[in] p_cb          @see cscpc_cb_t
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscpc_init(const cscpc_cb_t* p_cb);

/**
 ****************************************************************************************
 * @brief Deinitilization of Cycling Speed and Cadence Profile and unregister gatt service
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cscpc_deinit();

#ifdef __cplusplus
}
#endif

#endif /* _BLE_CSCPC_H_ */
