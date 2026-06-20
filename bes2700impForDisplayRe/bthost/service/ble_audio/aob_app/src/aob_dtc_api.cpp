/**
 * @file aob_dtc_api.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2021-11-20
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/
#ifdef AOB_MOBILE_ENABLED
#include "bluetooth.h"
#include "app_gaf_define.h"
#include "app_gaf_dbg.h"
#include "aob_mgr_gaf_evt.h"

#include "aob_dtc_api.h"
#include "app_acc_dtc_msg.h"
#include "app_gaf_custom_api.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
void aob_dtc_connect(uint8_t con_lid, uint16_t local_max_sdu, uint16_t spsm)
{
    app_acc_dtc_coc_connect(con_lid, local_max_sdu, spsm);
}

void aob_dtc_send_data(uint8_t con_lid, uint16_t spsm, uint16_t length, uint8_t *sdu)
{
    app_acc_dtc_coc_send(con_lid, spsm, length, sdu);
}

void aob_dtc_disconnect(uint8_t con_lid, uint16_t spsm)
{
    app_acc_dtc_coc_disconnect(con_lid, spsm);
}

static void aob_dtc_coc_connected_cb(uint8_t con_lid, uint16_t tx_mtu, uint16_t tx_mps, uint16_t spsm)
{
    LOG_I("%s con_lid %d, spsm = %04x, tx_mtu = %d, tx_mps = %d", __func__,
          con_lid, spsm, tx_mtu, tx_mps);
}

static void aob_dtc_coc_disconnected_cb(uint8_t con_lid, uint16_t reason, uint16_t spsm)
{
    LOG_I("%s con_lid %d, reason %d, spsm = %04x", __func__,
          con_lid, reason, spsm);
}

static void aob_dtc_coc_data_cb(uint8_t con_lid, uint16_t length, uint8_t *sdu, uint16_t spsm)
{
    LOG_I("%s con_lid %d, length %d, spsm = %04x", __func__,
          con_lid, length, spsm);
    DUMP8("%02x ", sdu, length);
}

static dtc_coc_event_handler_t dtc_coc_event_cb =
{
    .dtc_coc_connected_cb = aob_dtc_coc_connected_cb,
    .dtc_coc_disconnected_cb = aob_dtc_coc_disconnected_cb,
    .dtc_coc_data_cb = aob_dtc_coc_data_cb,
};

void aob_dtc_api_init(void)
{
    aob_mgr_dtc_coc_evt_handler_register(&dtc_coc_event_cb);
}
#endif
