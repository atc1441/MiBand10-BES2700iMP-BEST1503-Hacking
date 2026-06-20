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
#ifndef __BT_SERVICE_HCI_H__
#define __BT_SERVICE_HCI_H__

/*****************************header include********************************/
#include <stdint.h>

/*********************external declaration*************************/

/**********************private function declaration*************************/

/************************private macro defination***************************/

/************************private variable defination************************/
typedef struct
{
    // cmd completion event callback
    void (*cmd_cmp)(uint16_t opcode, uint8_t *param, uint8_t param_len);
    // cmd status event callback
    void (*cmd_status)(uint16_t opcode, uint8_t state, uint8_t *param, uint16_t param_len);
    // vender event callback
    void (*vender)(uint8_t *param, uint8_t param_len);
    // disconnection event callback
    void (*disconn)(uint16_t conn_hdl, uint8_t reason);
} bt_svc_base_event_callback_t;

/*****************************function declearation**************************/
/**
 ****************************************************************************************
 * @brief bt service hci initialization.
 *
 * @param[return]                see@UX_MODULE_ERROR_CODE
 ****************************************************************************************
 */
uint8_t bt_svc_base_hci_init(void);

/**
 ****************************************************************************************
 * @brief bt service hci deinitialization.
 *
 * @param[return]                see@UX_MODULE_ERROR_CODE
 ****************************************************************************************
 */
uint8_t bt_svc_base_hci_deinit(void);

/**
 ****************************************************************************************
 * @brief bt service that needs to be registered.
 *
 * @param[in]  callback          user callback
 *
 * @param[return]                see@UX_MODULE_ERROR_CODE
 ****************************************************************************************
 */
uint8_t bt_svc_base_hci_reg_callback(bt_svc_base_event_callback_t *callback);

/**
 ****************************************************************************************
 * @brief bt service that needs to be unregistered.
 *
 * @param[in]  callback          user callback
 *
 * @param[return]                see@UX_MODULE_ERROR_CODE
 ****************************************************************************************
 */
uint8_t bt_svc_base_hci_unreg_callback(bt_svc_base_event_callback_t *callback);

#endif