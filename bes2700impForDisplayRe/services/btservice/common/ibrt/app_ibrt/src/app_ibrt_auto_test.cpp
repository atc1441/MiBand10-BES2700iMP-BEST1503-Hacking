/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include <string.h>
#include "bluetooth_bt_api.h"
#include "app_tws_ibrt_trace.h"
#include "factory_section.h"
#include "hal_timer.h"
#include "apps.h"
#include "app_battery.h"
#include "app_anc.h"
#include "app_a2dp.h"
#include "app_key.h"
#include "app_ibrt_auto_test.h"
#include "app_tws_ibrt_conn_api.h"
#include "app_tws_ibrt_ui_test.h"
#include "bluetooth_bt_api.h"
#include "app_bt_cmd.h"
#include "a2dp_decoder.h"
#include "app_ibrt_keyboard.h"
#include "nvrecord_env.h"
#include "nvrecord_ble.h"
#include "besbt.h"
#include "app_bt.h"
#include "app_ai_if.h"
#include "app_ai_manager_api.h"
#include "app_bt_media_manager.h"
#include "app_audio.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#if defined(IBRT)
#include "btapp.h"
#include "app_hfp.h"

int bt_sco_player_get_codetype(void);
extern "C" uint8_t is_sco_mode (void);

AUTO_TEST_STATE_T auto_test_state_t = {0};


uint16_t app_ibrt_auto_test_get_tws_page_timeout_value(void)
{
    //return app_ibrt_ui_get_tws_page_timeout_value();
    return 0;
}

#ifdef BES_AUTOMATE_TEST
extern uint16_t bt_media_get_media_active(int device_id);
void app_ibrt_auto_test_print_earphone_state(void const *n)
{
#if 1
    //app_ibrt_ui_t *p_ibrt_ui = app_ibrt_ui_get_ctx();
    //ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();

    auto_test_state_t.head[0] = 0xE1;
    auto_test_state_t.head[1] = 0x38;
    auto_test_state_t.head[2] = 0x7C;
    auto_test_state_t.length = sizeof(auto_test_state_t);

    //a2dp & hfp state
    auto_test_state_t.bt_stream_state.local_volume = app_bt_stream_local_volume_get();
    auto_test_state_t.bt_stream_state.a2dp_volume = a2dp_volume_get(BT_DEVICE_ID_1);
    auto_test_state_t.bt_stream_state.hfp_volume = hfp_volume_get(BT_DEVICE_ID_1);
    auto_test_state_t.bt_stream_state.a2dp_streamming = app_bt_get_device(BT_DEVICE_ID_1)->a2dp_streamming;
    auto_test_state_t.bt_stream_state.a2dp_codec_type = app_bt_get_device(BT_DEVICE_ID_1)->codec_type;
    auto_test_state_t.bt_stream_state.a2dp_sample_rate = app_bt_get_device(BT_DEVICE_ID_1)->sample_rate;
    auto_test_state_t.bt_stream_state.a2dp_sample_bit = app_bt_get_device(BT_DEVICE_ID_1)->sample_bit;

    auto_test_state_t.bt_stream_state.sco_streaming = is_sco_mode();
    auto_test_state_t.bt_stream_state.sco_codec_type = (uint8_t)bt_sco_player_get_codetype();
    //app_ibrt_if_get_hfp_call_status(BT_DEVICE_ID_1, (AppIbrtCallStatus *)&auto_test_state_t.bt_stream_state.call_status);

    //media state
    auto_test_state_t.bt_media_state.media_active = bt_media_get_media_active(BT_DEVICE_ID_1);
    auto_test_state_t.bt_media_state.curr_active_media = bt_media_get_current_media();
    auto_test_state_t.bt_media_state.promt_exist = app_audio_list_playback_exist();

    // ui state
    auto_test_state_t.ui_state.current_ms = TICKS_TO_MS(hal_sys_timer_get());
    auto_test_state_t.ui_state.cpu_freq = hal_sysfreq_get();
    auto_test_state_t.ui_state.super_state = 0; //p_ibrt_ui->super_state;
    auto_test_state_t.ui_state.active_event = 0; //p_ibrt_ui->active_event.event;
    auto_test_state_t.ui_state.ibrt_sm_running = 0; //p_ibrt_ui->ibrt_sm_running;
    auto_test_state_t.ui_state.ui_state = 0; //ibrt_mgr_get_local_box_state(0);

    //if (IBRT_SLAVE != p_ibrt_ctrl->current_role)
    {
        //auto_test_state_t.ui_state.mobile_link_bt_role = app_tws_ibrt_get_local_mobile_role();
        auto_test_state_t.ui_state.mobile_link_bt_mode = 0; //p_ibrt_ctrl->mobile_mode;
        auto_test_state_t.ui_state.mobile_constate = 0; //p_ibrt_ctrl->mobile_constate;
        //auto_test_state_t.ui_state.mobile_connect = app_tws_ibrt_mobile_link_connected();
    }
    auto_test_state_t.ui_state.tws_role = 0; //p_ibrt_ctrl->current_role;
    auto_test_state_t.ui_state.tws_link_bt_role = app_tws_ibrt_get_local_tws_role();
    auto_test_state_t.ui_state.tws_link_bt_mode = 0; //p_ibrt_ctrl->tws_mode;
    auto_test_state_t.ui_state.tws_constate = 0; //p_ibrt_ctrl->tws_constate;
    auto_test_state_t.ui_state.role_switch_state = 0; //p_ibrt_ctrl->master_tws_switch_pending;
#ifdef __IAG_BLE_INCLUDE__
    auto_test_state_t.ui_state.ble_connection_state = 0;
    for (uint8_t i = 0; i < BLE_CONNECTION_MAX; i++)
    {
        if (app_ble_is_connection_on(i))
        {
            auto_test_state_t.ui_state.ble_connection_state |= 
                (1 << i);
        }
    }
#endif

    //LOG_I("earphone len %d state :", auto_test_state_t.length);
    DUMP8("%02x", (uint8_t *)&auto_test_state_t, auto_test_state_t.length);
#endif
}

#define APP_AUTO_TEST_PRINT_STATE_TIME_IN_MS 1000
osTimerDef (APP_AUTO_TEST_PRINT_STATE_TIMEOUT, app_ibrt_auto_test_print_earphone_state);
osTimerId   app_auto_test_print_state_timeout_timer_id = NULL;
#endif

void app_ibrt_auto_test_init(void)
{
#ifdef BES_AUTOMATE_TEST
    memset((uint8_t *)&auto_test_state_t, 0, sizeof(auto_test_state_t));
    if (app_auto_test_print_state_timeout_timer_id == NULL)
    {
        app_auto_test_print_state_timeout_timer_id = osTimerCreate(osTimer(APP_AUTO_TEST_PRINT_STATE_TIMEOUT), osTimerPeriodic, NULL);
        osTimerStart(app_auto_test_print_state_timeout_timer_id, APP_AUTO_TEST_PRINT_STATE_TIME_IN_MS);
    }
#endif
}

void app_ibrt_auto_test_inform_cmd_received(uint8_t group_code,
        uint8_t operation_code)
{
    //AUTO_TEST_LOG_I("AUTO_TEST_CMD received:%d:%d:", group_code, operation_code);
}

#endif

