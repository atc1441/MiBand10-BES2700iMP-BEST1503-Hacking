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
#ifndef __BLE_FTMS_H__
#define __BLE_FTMS_H__
#if defined(BLE_FTMS_ENABLED)
#include "ble_ftm.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Fitness Machine service server callback set
typedef struct ble_ftms_cb
{
    /**
     ****************************************************************************************
     * @brief Completion of ntf send
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_ntf_send_cmp)(uint16_t status);

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
    void (*cb_ctrl_pt_req)(uint8_t conidx, uint8_t op_code, const ftm_cpt_opcode *p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

} ble_ftms_cb_t;

uint16_t ble_ftms_init(struct ble_ftms_db_cfg *p_init_cfg, const ble_ftms_cb_t *p_cb);

uint16_t ble_ftms_deinit(void);

#ifdef __cplusplus
    }
#endif
#endif /* BLE_FTMS_ENABLED */
#endif /* __BLE_FTMS_H__ */
