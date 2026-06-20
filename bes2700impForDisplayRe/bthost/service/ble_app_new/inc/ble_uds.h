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
#ifndef __BLE_UDS_H__
#define __BLE_UDS_H__
#if defined(BLE_UDP_ENABLE)
#include "gatt_service.h"
#include "app_ble.h"
#include "prf_types.h"
#include "co_math.h"
#ifdef __cplusplus
extern "C" {
#endif

/// UDS CT Point timeout (in milliseconds)
#define UDS_CP_TIMEOUT                    (30000)

///UDS cp Notification Value Max Length
#define UDS_CP_NTF_MAX_LEN               (4)
///UDS cp Value Min Length
#define UDS_CP_NTF_MIN_LEN           (3)

/// Control Point Value Max Length
#define UDS_CTRL_PT_RSP_MAX_LEN        (4)
/// Control Point Value Min Length
#define UDS_CTRL_PT_RSP_MIN_LEN        (3)

#define UDS_2_GAP_CONIDX_BF(conidx)  (gap_conn_bf(gap_zero_based_conidx_to_ble_conidx(conidx)))

/// UDS Control Point: Opcode
enum uds_opcode_id {
    UDS_CTRL_PT_REGIS_NEW_USER     = 0x01,
    UDS_CTRL_PT_CONSENT,
    UDS_CTRL_PT_DELET_USER_DATA,
    UDS_CTRL_PT_LIST_ALL_USERS,
    UDS_CTRL_PT_DELET_USER,
    UDS_CTRL_PT_RESERVED,
    UDS_CTRL_PT_RSP_CODE = 0x20,
};

/// UDS Control Point rsp result code
enum uds_result_code_id {
    UDS_RESULT_SUCCESS = 1,
    UDS_RESULT_OPCODE_NOT_SUPPORTED,
    UDS_RESULT_INVALID_PARAM,
    UDS_RESULT_OPERATION_FAILED,
    UDS_RESULT_USER_NOT_AUTHORIZED,
};

typedef struct {
    uint8_t user_index;
    uint16_t consent_code;
}consent_data;

typedef struct {
    uint8_t opcode;
    union
    {
        uint16_t consent_code;
        uint8_t user_index;
        consent_data consent;
    } data;
}uds_cpt_opcode;

typedef struct {
    uint8_t rspcode;
    uint8_t reqcode;
    uint8_t resultcode;
    uint8_t rsp_param;
} uds_cpt_rsp;

/// User Data Service server callback set
typedef struct ble_uds_cb
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
     * @param[in] cfg_val       Stop/notify/indicate value to configure into the peer characteristic
     ****************************************************************************************
     */
    void (*cb_bond_data_upd)(uint8_t conidx, uint16_t cfg_val);

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
    void (*cb_ctrl_pt_req)(uint8_t conidx, uint8_t op_code, const uds_cpt_opcode *p_value);

    /**
     ****************************************************************************************
     * @brief Completion of control point response send procedure
     *
     * @param[in] status Status of the procedure execution (see enum #bt_status_t)
     ****************************************************************************************
     */
    void (*cb_ctrl_pt_rsp_send_cmp)(uint8_t conidx, uint16_t status);

}ble_uds_cb_t;

uint16_t ble_uds_init(const ble_uds_cb_t *p_cb);

uint16_t ble_uds_deinit(void);



#ifdef __cplusplus
    }
#endif
#endif /* BLE_UDP_ENABLE */
#endif /* __BLE_UDS_H__ */
