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
#ifndef __BLE_FTMC_H__
#define __BLE_FTMC_H__
#if defined(BLE_FTMC_ENABLED)
#include "ble_ftm.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ble_ftmc_cb
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
     * @brief Completion of read char procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     * @param[in] value         value
     * @param[in] len           len
     *
     ****************************************************************************************
     */
    void (*cb_read_char_cmp)(uint8_t conidx, uint16_t status, const uint8_t *value, uint8_t len);

    /**
     ****************************************************************************************
     * @brief Completion of read Characteristic Configuration procedure.
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the procedure execution (see enum #bt_status_t)
     * @param[in] desc_code     Descriptor Code (see enum #)
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
     * 
     ****************************************************************************************
     */
    void (*cb_write_cfg_cmp)(uint8_t conidx, uint16_t status, uint8_t desc_code);

    /**
     ****************************************************************************************
     * @brief Completion of control point request procedure
     *
     * @param[in] conidx        Connection index
     * @param[in] status        Status of the Request Send (see enum #bt_status_t)
     * @param[in] req_op_code   Requested Operation Code see enum #ftmc_opcode_id
     * @param[in] resp_value    Response Value see enum #resp_val
     * @param[in] p_value       Pointer to response data content
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_req_cmp)(uint8_t conidx, uint16_t status, uint8_t req_op_code, uint8_t resp_value,
                               ble_ftm_ctrl_pt_rsp_val *p_value);
} ble_ftmc_cb_t;

bt_status_t ble_ftmc_start_discover(uint16_t connhdl);

uint16_t ble_ftmc_init(const ble_ftmc_cb_t *p_cb);

uint16_t ble_ftmc_deinit(void);


#ifdef __cplusplus
    }
#endif
#endif /* BLE_FTMC_ENABLED */
#endif /* __BLE_FTMC_H__ */
