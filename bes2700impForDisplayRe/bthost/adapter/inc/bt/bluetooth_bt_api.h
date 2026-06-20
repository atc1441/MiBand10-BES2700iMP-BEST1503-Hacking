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
 * @brief Bluetooth Application Programming Interface.
 *
 ****************************************************************************/
#ifndef __BLUETOOTH_BT_API_H__
#define __BLUETOOTH_BT_API_H__

/****************************** header include ********************************/
#include "tgt_hardware.h"
#include "bt_common_define.h"
#include "source_common_define.h"
#include "adapter_service.h"
#include "bes_source_api.h"
#include "bes_me_api.h"
#include "bes_a2dp_api.h"
#include "bes_avrcp_api.h"
#include "bes_hfp_api.h"
#include "bes_ibrt_api.h"
#include "bes_dip_api.h"
#include "bes_hid_api.h"
#include "hci_api.h"
#include "spp_api.h"
#include "app_bt_hid.h"
#include "app_map.h"
#include "besbt.h"
#include "gap_service.h"
#include "gatt_service.h"

#if defined(__GATT_OVER_BR_EDR__)
#include "app_btgatt.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/***************************** external declaration *****************************/

/***************************** macro defination *******************************/

/*****************************  type defination ********************************/

/*****************************  function declaration ****************************/
/**
 ***************************************************************************
 * @brief Bluetooth host initialization
 * @param[in]  : init_done_cb, initialization completed callback function
 * @param[out] : none
 * @return: error code, see@bt_status_t
 ***************************************************************************
 */
bt_status_t bt_host_init(void (*init_done_cb)());

/**
 ***************************************************************************
 * @brief Bluetooth host deinitialization
 * @param[in]  : none
 * @param[out] : none
 * @return: error code, see@bt_status_t
 ***************************************************************************
 */
bt_status_t bt_host_deinit();

/**
 ***************************************************************************
 * @brief Print bluetooth device address in a simplified format: bc:...:5a:b4
 * @param[in]  : bdaddr_array
 * @param[out] : none
 * @return: none
 ***************************************************************************
 */
void bt_print_bdaddr(const uint8_t *bdaddr_array);

/**
 ***************************************************************************
 * @brief Print out the full six bytes of the Bluetooth address
 * @param[in]  : bdaddr_array
 * @param[out] : none
 * @return: none
 ***************************************************************************
 */
void bt_print_bdaddr_full(const uint8_t *bdaddr_array);

/**
 ***************************************************************************
 * @brief Whether it is an all-zero address
 * @param[in]  : bdaddr_array
 * @param[out] : none
 * @return: bool
 ***************************************************************************
 */
bool bt_is_allzero_bdaddr(const uint8_t *bdaddr_array);

#ifdef __cplusplus
}
#endif
#endif /* __BLUETOOTH_BT_API_H__ */

