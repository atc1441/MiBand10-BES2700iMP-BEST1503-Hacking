/**
 * @brief Bluetooth Service Low Energy Audio Unicast Client
 *
 * @copyright Copyright (c) 2015-20223 BES Technic.
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
 */
#ifndef __BT_SVC_LEA_UC_CLIENT_H__
#define __BT_SVC_LEA_UC_CLIENT_H__

/*****************************header include********************************/
#include "bt_svc_lea_uc_client_conn_stm.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/

#ifdef AOB_MOBILE_ENABLED
void bt_svc_lea_uc_client_init();

bt_svc_lea_uc_client_conn_stm_t* bt_svc_lea_uc_client_find_sm_by_address(ble_bdaddr_t *addr);

bt_svc_lea_uc_client_conn_stm_t* bt_svc_lea_uc_client_make_sm_by_address(ble_bdaddr_t *addr);
#endif

#endif