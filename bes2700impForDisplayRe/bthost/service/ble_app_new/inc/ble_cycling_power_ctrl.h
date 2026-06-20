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

#ifndef __BLE_CYCLING_POWER_CTRL_H__
#define __BLE_CYCLING_POWER_CTRL_H__

#include "ble_cycling_power_i.h"

#ifdef __cplusplus
extern "C" {
#endif


/// Cycling Power Service Characteristic Descriptors
enum cppc_cps_descs
{
    /// CP Measurement Char. - Client Characteristic Configuration
    BLE_CPC_DESC_CP_MEAS_CL_CFG,
    /// CP Measurement Char. - Server Characteristic Configuration
    BLE_CPC_DESC_CP_MEAS_SV_CFG,
    /// CP Vector Char. - Client Characteristic Configuration
    BLE_CPC_DESC_VECTOR_CL_CFG,
    /// Control Point Char. - Client Characteristic Configuration
    BLE_CPC_DESC_CTRL_PT_CL_CFG,

    BLE_CPC_DESC_MAX,

    BLE_CPC_DESC_MASK = 0x10,
};

/// Codes for reading/writing a CPS characteristic with one single request
enum cppc_code
{
    /// Notified CP Measurement
    BLE_CPC_NTF_CP_MEAS          = CPS_MEAS_CHAR,
    /// Read CP Feature
    BLE_CPC_RD_CP_FEAT           = CPS_FEAT_CHAR,
    /// Read Sensor Location
    BLE_CPC_RD_SENSOR_LOC        = CPS_SENSOR_LOC_CHAR,
    /// Notified Vector
    BLE_CPC_NTF_CP_VECTOR        = CPS_VECTOR_CHAR,
    /// Indicated SC Control Point
    BLE_CPC_IND_CTRL_PT          = CPS_CTRL_PT_CHAR,

    /// Read/Write CP Measurement Client Char. Configuration Descriptor
    BLE_CPC_RD_WR_CP_MEAS_CL_CFG    = (BLE_CPC_DESC_CP_MEAS_CL_CFG   | BLE_CPC_DESC_MASK),
    /// Read/Write CP Measurement Server Char. Configuration Descriptor
    BLE_CPC_RD_WR_CP_MEAS_SV_CFG    = (BLE_CPC_DESC_CP_MEAS_SV_CFG   | BLE_CPC_DESC_MASK),

    /// Read/Write Vector Client Char. Configuration Descriptor
    BLE_CPC_RD_WR_VECTOR_CFG    = (BLE_CPC_DESC_VECTOR_CL_CFG   | BLE_CPC_DESC_MASK),
    /// Read/Write CP Control Point Client Char. Configuration Descriptor
    BLE_CPC_RD_WR_CTRL_PT_CFG  = (BLE_CPC_DESC_CTRL_PT_CL_CFG | BLE_CPC_DESC_MASK),
};

/// Cycling Power service client callback set
typedef struct ble_cpc_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of service discovery procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     *
     ****************************************************************************************
     */
    void (*cb_svc_discover_cmp)(uint8_t conidx, uint16_t status);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor feature procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     * @param[in] sensor_feat   CP sensor feature
     *
     ****************************************************************************************
     */
    void (*cb_read_sensor_feat_cmp)(uint8_t conidx, uint16_t status, uint32_t sensor_feat);

    /**
     ****************************************************************************************
     * @brief Completion of read sensor location procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
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
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     * @param[in] desc_code     Descriptor Code (see enum #ble_cpc_code)
     *                              - BLE_CPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
     *                              - BLE_CPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
     *                              - BLE_CPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
     *                              - BLE_CPC_RD_WR_CTRL_PT_CFG:    CP Control Point Client Char. Configuration
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
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     * @param[in] desc_code     Descriptor Code (see enum #ble_cpc_code)
     *                              - BLE_CPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
     *                              - BLE_CPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
     *                              - BLE_CPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
     *                              - BLE_CPC_RD_WR_CTRL_PT_CFG:    CP Control Point Client Char. Configuration
     *
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Function called when CP measurement information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_meas         Pointer to CP measurement information
     ****************************************************************************************
     */
    void (*cb_meas)(uint8_t conidx, const ble_cps_cp_meas_t *p_meas);

    /**
     ****************************************************************************************
     * @brief Function called when CP vector information is received
     *
     * @param[in] conidx         Connection index
     * @param[in] p_vector       Pointer to CP vector information
     ****************************************************************************************
     */
    void (*cb_vector)(uint8_t conidx, const ble_cps_cp_vector_t *p_vector);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #bt_status_t)
     * @param[in] req_op_code   Requested Operation Code see enum #ble_cps_ctrl_pt_code
     * @param[in] resp_value    Response Value see enum #ble_cps_ctrl_pt_resp_val
     * @param[in] p_value       Pointer to response data content
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               const union ble_cps_ctrl_pt_rsp_val *p_value);
} ble_cpc_cb_t;


/**
 ****************************************************************************************
 * @brief Initilization of Cylcing Power Controller and register service.
 *
 * @param[in] p_cb        @see ble_cpc_cb_t
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_init(const ble_cpc_cb_t *p_cb);

/**
 ****************************************************************************************
 * @brief Denitilization of Cylcing Power Controller and unregister service.
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_deinit(void);

/**
 ****************************************************************************************
 * @brief Perform a service disocvery procedure.
 *
 * Wait for #cb_svc_discover_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_service_discover(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor feature procedure.
 *
 * Wait for #cb_read_sensor_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_read_sensor_feat(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read sensor location procedure.
 *
 * Wait for #cb_read_loc_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_read_sensor_loc(uint8_t conidx);

/**
 ****************************************************************************************
 * @brief Perform a read Characteristic Configuration procedure.
 *
 * Wait for #cb_read_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #ble_cpc_code)
 *                              - BLE_CPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
 *                              - BLE_CPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
 *                              - BLE_CPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
 *                              - BLE_CPC_RD_WR_CTRL_PT_CFG:    CP Control Point Client Char. Configuration
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_read_cfg(uint8_t conidx, uint8_t desc_code);

/**
 ****************************************************************************************
 * @brief Perform a write Characteristic Configuration procedure.
 *
 * Wait for #cb_write_cfg_cmp execution before starting a new procedure
 *
 * @param[in] conidx        Connection index
 * @param[in] desc_code     Descriptor Code (see enum #ble_cpc_code)
 *                              - BLE_CPC_RD_WR_CP_MEAS_CL_CFG: CP Measurement Client Char. Configuration
 *                              - BLE_CPC_RD_WR_CP_MEAS_SV_CFG: CP Measurement Server Char. Configuration
 *                              - BLE_CPC_RD_WR_VECTOR_CFG:     Vector Client Char. Configuration
 *                              - BLE_CPC_RD_WR_CTRL_PT_CFG:    CP Control Point Client Char. Configuration
 * @param[in] cfg_val       Configuration value
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_write_cfg(uint8_t conidx, uint8_t desc_code, uint16_t cfg_val);

/**
 ****************************************************************************************
 * @brief Function called to send a control point request
 *
 * Wait for #cb_ctrl_pt_req_cmp execution before starting a new procedure
 *
 * @param[in] conidx         Connection index
 * @param[in] req_op_code    Requested Operation Code see enum #ble_cps_ctrl_pt_code
 * @param[in] p_value        Pointer to request data content
 *
 * @return Status of the function execution (see enum #bt_status_t)
 ****************************************************************************************
 */
uint16_t ble_cpc_ctrl_pt_req(uint8_t conidx, uint8_t req_op_code, const union ble_cps_ctrl_pt_req_val *p_value);

#ifdef __cplusplus
}
#endif

#endif /* __BLE_CYCLING_POWER_CTRL_H__ */
