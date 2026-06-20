/**
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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

#include "cmsis_os.h"
#include "hal_trace.h"
#include "factory_section.h"

#include "walkie_talkie_dbg.h"
#include "app_walkie_talkie_key_handler.h"
#include "app_walkie_talkie.h"
#include "walkie_talkie_ble_gap.h"
#include "walkie_talkie.h"
#include "app_walkie_talkie_full_duplex.h"
#include "app_walkie_talkie_half_duplex.h"
#include "walkie_talkie_network_addr.h"
#include "walkie_talkie_build_network.h"
#include "app_media_player.h"

#ifdef WT_GAP_UNIT_TEST
#include "app_walkie_talkie_test.h"
#include "walkie_talkie_unit_test.h"
#endif

/*
 *  MVP framework,App_walkie_talkie as the Presenter
 *                Watch UI as the View layer
 *                walkie talkie service as the Mode
*/
static void app_walkie_talkie_ready_to_send_data_cb()
{
    LOG_D("W-T-APP:ready_to_send_data_cb");
}

static void app_walkie_talkie_find_device_cb(uint8_t device_id, uint8_t* device_addrss)
{
    LOG_D("W-T-APP:find_device(%d),address:",device_id);
    if(device_addrss != NULL)
    {
        DUMP8("%02x ",device_addrss, 6);
        //media_PlayAudio(AUD_ID_BT_CONNECTED, 0);
    }
}

static void app_walkie_talkie_device_loss_cb(uint8_t device_id,uint8_t* device_addrss)
{
    LOG_D("W-T-APP:device(%d),address loss",device_id);
    if(device_addrss != NULL)
    {
        DUMP8("%02x ",device_addrss, 6);
        //media_PlayAudio(AUD_ID_BT_DIS_CONNECT, 0);
    }
}

static void app_walkie_talkie_device_is_talking(uint8_t device_id,uint8_t* device_addrss)
{
    LOG_D("W-T-APP:device(%d) is talking",device_id);
    if(device_addrss != NULL)
    {
        DUMP8("%02x ",device_addrss, 6);
    }
}

static void app_wakie_talkie_device_end_talking(uint8_t device_id,uint8_t* device_addrss)
{
    LOG_D("W-T-APP:device(%d) talking end",device_id);
    if(device_addrss != NULL)
    {
        DUMP8("%02x ",device_addrss, 6);
    }
}

static void app_wt_allow_speaking()
{
    LOG_D("W-T-APP:%s",__func__);
}

static void app_wt_not_allow_speaking()
{
    LOG_D("W-T-APP:%s",__func__);
}

static const walkie_full_dup_event_cb walkie_talkie_full_dup_event_cbs = {
    .wt_ready_to_send_data = app_walkie_talkie_ready_to_send_data_cb,
    .wt_find_device = app_walkie_talkie_find_device_cb,
    .wt_device_loss = app_walkie_talkie_device_loss_cb,
    .wt_device_is_stalking = app_walkie_talkie_device_is_talking,
    .wt_device_end_stalking = app_wakie_talkie_device_end_talking,
    .wt_allow_speaking      = app_wt_allow_speaking,
    .wt_not_allow_speaking  = app_wt_not_allow_speaking,
};

void app_walkie_talkie_init()
{
    uint8_t* local_ble_addr = factory_section_get_ble_address();
    LOG_D("W-T:Local Address:");
    ASSERT(local_ble_addr,"Local Address Err!");
    DUMP8("%02x ",local_ble_addr, 6);

#ifdef BLE_WALKIE_TALKIE_HALF_DUPLEX
    app_walkie_talkie_half_dup_init(wt_auto_build_net_addr,
            ARRAY_SIZE(wt_auto_build_net_addr),WALKIE_TALKIE_HALF_DUPLEX_MODE);
#else
    app_walkie_talkie_full_dup_init(NULL,0 ,WALKIE_TALKIE_FULL_DUPLEX_MODE);

    app_wt_full_dup_reg_state_changed_callback(&walkie_talkie_full_dup_event_cbs);
#endif

#ifdef WT_AUTO_BUILD_NET
    wt_network_rebuild(WT_NW_TYPE_MULTIPLE, wt_auto_build_net_addr,
            sizeof(wt_auto_build_net_addr), wt_auto_build_net_addr, WALKIE_GAP_PER_ADV);
#endif

#ifdef WT_GAP_UNIT_TEST
    // app_wt_uart_cmd_register();
#endif
}

void app_walkie_talkie_handle_event(APP_W_T_KEY_EVENT_E event)
{
#ifdef BLE_WALKIE_TALKIE_HALF_DUPLEX
    app_walkie_talkie_half_dup_handle_key_event(event);
#else
    app_walkie_talkie_full_dup_handle_key_event(event);
#endif
}

void app_walkie_talkie_exit()
{
#ifdef BLE_WALKIE_TALKIE_FULL_DUPLEX
    app_wt_full_dup_start_exit();
#endif
}


