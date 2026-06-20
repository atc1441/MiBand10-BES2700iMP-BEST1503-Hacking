/**
 * @file aob_dts_api.cpp
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
#if BLE_AUDIO_ENABLED

#include "app_gaf_custom_api.h"
#include "app_gaf_define.h"
#include "app_gaf_dbg.h"

#include "aob_dts_api.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

/****************************function defination****************************/
void aob_dts_send_data(uint8_t con_lid, uint16_t spsm, uint16_t length, const uint8_t *sdu)
{
    app_acc_dts_coc_send(con_lid, spsm, length, sdu);
}

void aob_dts_disconnect(uint8_t con_lid, uint16_t spsm)
{
    app_acc_dts_coc_disconnect(con_lid, spsm);
}

void aob_dts_register_spsm(uint16_t spsm, uint16_t initial_credits)
{
    app_acc_dts_coc_register_spsm(spsm, initial_credits);
}

void aob_dts_api_init(dts_coc_event_handler_t *dts_coc_event_cb)
{
    aob_mgr_dts_coc_evt_handler_register(dts_coc_event_cb);
}
#endif
