/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#undef MOUDLE
#define MOUDLE APP_BT
#include "hal_aud.h"
#include "hal_chipid.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "bluetooth_ble_api.h"
#include "app_bt_media_manager.h"
#include "apps.h"
#include "app_bt.h"
#include "app_hfp.h"
#include "app_a2dp.h"
#include "app_thread.h"
#include "app_status_ind.h"
#include "bluetooth.h"
#include "app_bt_func.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "bt_if.h"
#include "besbt_cfg.h"
#include "app_bt_cmd.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "app_key.h"
#include "app_audio.h"
#include "app_audio_control.h"
#include "app_media_player.h"
#include "audio_player_adapter.h"
#include "system_utils.h"

#include "co_heap.h"
#include "ecc_p192.h"
#ifdef BLE_HOST_SUPPORT
#include "ecc_p256.h"
#endif

#if defined(BT_SOURCE)
#include "bt_source.h"
#endif

#if defined(IBRT)
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_sync_a2dp_status.h"
#include "app_ibrt_a2dp.h"
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_nvrecord.h"

#ifdef IBRT_UI
#include "app_tws_ibrt_conn_api.h"
#include "app_custom_api.h"
#include "app_audio_control.h"
#endif
#endif

#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef GFPS_ENABLED
#include "bluetooth_ble_api.h"
#ifdef SASS_ENABLED
#include "gfps_sass.h"
#endif
#endif

#ifdef __INTERCONNECTION__
#include "app_interconnection.h"
#include "app_interconnection_ble.h"
#include "spp_api.h"
#include "app_interconnection_ccmp.h"
#include "app_interconnection_spp.h"
#endif

#ifdef __INTERACTION__
#include "app_interaction.h"
#endif

#if BLE_AUDIO_ENABLED
#include "bluetooth_ble_api.h"
#endif

#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
#include "app_dbg_ble_audio_info.h"
#endif

/// FUNCTIONS DECLARATION
static uint32_t bt_export_bes_cfg_get(int cfg)
{
    switch (cfg)
    {
        case BT_BES_CFG_VENDOR_CODEC_EN:
            return besbt_cfg.vendor_codec_en;
        case BT_BES_CFG_DONT_PLAY_MUTE_WHEN_A2DP_STUCK:
            return besbt_cfg.dont_play_mute_when_a2dp_stuck;
        case BT_BES_CFG_MARK_CODE_FOR_FUZZ_TEST:
            return besbt_cfg.mark_some_code_for_fuzz_test;
        case BT_BES_CFG_KEEP_ONLY_ONE_STREAM_CLOSE_CONNECTED_A2DP:
#ifdef BLE_ONLY_ENABLED
            return false;
#else
            return app_bt_manager.config.keep_only_one_stream_close_connected_a2dp;
#endif
        case BT_BES_CFG_BT_SINK_ENABLED:
            return besbt_cfg.bt_sink_enable;
        case BT_BES_CFG_PTS_TEST_DONT_ROLE_SWITCH:
            return besbt_cfg.pts_test_dont_bt_role_switch;
        default:
            break;
    }
    return 0;
}

uint32_t bes_bt_me_bes_cfg_get(int cfg)
{
    return bt_export_bes_cfg_get(cfg);
}

bool bt_export_get_hci_trace_cfg(uint8_t cfg)
{
    switch (cfg)
    {
        case HCI_BUFFER_TRACE_ENABLE:
            return besbt_cfg.hci_buff_trace_enable;
        case HCI_BUFFER_TRACE_HIGH_ENABLE:
            return besbt_cfg.hci_buffer_trace_high_enable;
        case HCI_TRACE_ENABLE:
            return besbt_cfg.hci_trace_enable;
        case HCI_A2DP_STREAM_TRAC_ENABLE:
            return besbt_cfg.hci_a2dp_stream_trace_enable;
        case HCI_ACL_DATA_TRAC_ENABLE:
            return besbt_cfg.hci_acl_packet_trace_enable;
        case HCI_ISO_DATA_TRAC_ENABLE:
            return besbt_cfg.hci_iso_packet_trace_enable;
        default:
            break;
    }
    return false;
}

void bt_export_reconfig_hci_trace(uint8_t cfg_type, bool enable)
{
    switch (cfg_type)
    {
        case HCI_BUFFER_TRACE_ENABLE:
            besbt_cfg.hci_buff_trace_enable = enable;
            break;
        case HCI_BUFFER_TRACE_HIGH_ENABLE:
            besbt_cfg.hci_buffer_trace_high_enable = enable;
            break;
        case HCI_TRACE_ENABLE:
            besbt_cfg.hci_trace_enable = enable;
            break;
        case HCI_A2DP_STREAM_TRAC_ENABLE:
            besbt_cfg.hci_a2dp_stream_trace_enable = enable;
            break;
        case HCI_ACL_DATA_TRAC_ENABLE:
            besbt_cfg.hci_acl_packet_trace_enable = enable;
            break;
        case HCI_ISO_DATA_TRAC_ENABLE:
            besbt_cfg.hci_iso_packet_trace_enable = enable;
            break;
        default:
            break;
    }
}

void bt_add_event_callback(bt_event_callback_t cb, uint32_t masks)
{
    btif_add_bt_event_callback(cb, masks);
}

#if defined(__HOST_GEN_ECDH_KEY__)
static uint32_t lm_priv_key_192[6];
static uint32_t lm_pub_key_192[12];
static uint32_t lm_priv_key_256[8];
static uint32_t lm_pub_key_256[16];
#endif

void bt_generate_ecdh_key_pair(void)
{
#if defined(__HOST_GEN_ECDH_KEY__)
    POSSIBLY_UNUSED uint32_t time_start = hal_sys_timer_get();

    btif_ecc_gen_new_secret_key_192((uint8_t *)lm_priv_key_192);
    btif_ecc_gen_new_public_key_192((uint8_t *)lm_priv_key_192,(uint8_t *)lm_pub_key_192);
    bt_drv_reg_op_write_private_public_key((uint8_t *)lm_priv_key_192,(uint8_t *)lm_pub_key_192);

    DEBUG_INFO(1,"use time: %d ms", TICKS_TO_MS((hal_sys_timer_get()-time_start)));
#endif
}

void bt_generate_full_ecdh_key_pair(void)
{
#if defined(__HOST_GEN_ECDH_KEY__)
    POSSIBLY_UNUSED uint32_t time_start = hal_sys_timer_get();

    btif_ecc_gen_new_secret_key_192((uint8_t *)lm_priv_key_192);
    btif_ecc_gen_new_public_key_192((uint8_t *)lm_priv_key_192,(uint8_t *)lm_pub_key_192);

#ifdef BLE_HOST_SUPPORT
    ecc_gen_new_secret_key_256((uint8_t *)lm_priv_key_256,false);
    ecc_gen_new_public_key_256((uint8_t *)lm_priv_key_256,(uint8_t *)lm_pub_key_256);
    DEBUG_INFO(1,"full_ecdh use time: %d ms", TICKS_TO_MS((hal_sys_timer_get()-time_start)));
#else
    DEBUG_INFO(1,"full_ecdh use time: %d ms ble not enable", TICKS_TO_MS((hal_sys_timer_get()-time_start)));
#endif
#endif
}

void bt_apply_ecdh_key_pair(void)
{
#if defined(__HOST_GEN_ECDH_KEY__)
    bt_drv_reg_op_write_private_public_key((uint8_t *)lm_priv_key_192,(uint8_t *)lm_pub_key_192);
#endif
}

void bt_apply_full_ecdh_key_pair(void)
{
#if defined(__HOST_GEN_ECDH_KEY__)
    bt_drv_reg_op_write_private_public_192_256_key((uint8_t *)lm_priv_key_192,(uint8_t *)lm_pub_key_192,true);
    bt_drv_reg_op_write_private_public_192_256_key((uint8_t *)lm_priv_key_256,(uint8_t *)lm_pub_key_256,false);
#ifdef BLE_HOST_SUPPORT
    bes_ble_gap_set_ecdh_key_pair((uint8_t *)lm_priv_key_256, (uint8_t *)lm_pub_key_256);
#endif
#endif
}

static char g_state_checker_log_buffer[100*BT_DEVICE_NUM];

char *app_bt_get_global_state_buffer(void)
{
    return g_state_checker_log_buffer;
}

void app_bt_print_buff_status(void)
{
    btif_hci_print_statistic();
}


//immediate send disconnect api only assert
#ifdef BLE_HOST_SUPPORT
static bool app_bt_fault_free_ble_conn(gap_conn_item_t *conn, void *priv)
{
    if (conn->conn_type == HCI_CONN_TYPE_LE_ACL)
    {
        btif_me_fault_free_link(conn->connhdl);
    }
    return true;
}
#endif

void app_bt_host_fault_free_links(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t i = 0;
    for (i = 0; i < BT_DEVICE_NUM; i += 1)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->acl_is_connected)
        {
            btif_me_fault_free_link(curr_device->acl_conn_hdl);
        }
    }
#if defined(BT_SOURCE)
    struct BT_SOURCE_DEVICE_T *device = NULL;
    for (i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
    {
        device = app_bt_source_get_device(i);
        if (device->base_device->acl_is_connected)
        {
            btif_me_fault_free_link(device->base_device->acl_conn_hdl);
        }
    }
#endif

#ifdef BLE_HOST_SUPPORT
    gap_conn_foreach(app_bt_fault_free_ble_conn, NULL);
#endif

}

void app_bt_host_fault_dump_trace(void)
{
    app_bt_host_fault_free_links();

    if (bt_defer_curr_func_0(app_bt_host_fault_dump_trace))
    {
        return;
    }

    DEBUG_INFO(0, "%s start\n", __func__);

#ifdef BLE_HOST_SUPPORT
    gap_dump_ble_status();
    hal_trace_flush_buffer();
    hal_sys_timer_delay(MS_TO_TICKS(50));
#endif

    hci_print_statistic();
    hal_trace_flush_buffer();
    hal_sys_timer_delay(MS_TO_TICKS(50));

    cobuf_print_statistic();

    DEBUG_INFO(0, "%s end", __func__);
}

void *app_bt_profile_active_store_ptr_get(uint8_t *bdAddr)
{
    static btdevice_profile device_profile = {true, false, true,0};
    btdevice_profile *ptr;

#ifndef FPGA
    nvrec_btdevicerecord *record = NULL;
    if (!nv_record_btdevicerecord_find((bt_bdaddr_t *)bdAddr,&record))
    {
        uint32_t lock = nv_record_pre_write_operation();
        ptr = &(record->device_plf);
        DUMP8("0x%02x ", bdAddr, BT_ADDR_OUTPUT_PRINT_NUM);
        DEBUG_INFO(5,"%s hfp_act:%d a2dp_abs_vol:%d a2dp_act:0x%x codec_type=%x", __func__, ptr->hfp_act, ptr->a2dp_abs_vol, ptr->a2dp_act,ptr->a2dp_codectype);
        /* always need connect a2dp and hfp */
        ptr->hfp_act = true;
        ptr->a2dp_act = true;
        nv_record_post_write_operation(lock);
    }
    else
#endif
    {
        ptr = &device_profile;
        DEBUG_INFO(1,"%s default", __func__);
    }
    return (void *)ptr;
}

bt_status_t bt_adapter_write_sleep_enable(bool enable)
{
#ifdef __BT_NO_SLEEP__
    enable = false;
#endif
    bt_defer_call_func_1(btif_me_write_bt_sleep_enable, bt_fixed_param(enable));
    return BT_STS_SUCCESS;
}

void app_bt_get_local_device_address(void *bd_addr)
{
    bt_bdaddr_t *remote = (bt_bdaddr_t *)bd_addr;
#ifdef IBRT
    ibrt_ctrl_t *p_ibrt_ctrl = app_tws_ibrt_get_bt_ctrl_ctx();
    *remote = p_ibrt_ctrl->peer_addr;
#else
    *remote = *(bt_bdaddr_t *)bt_get_local_address();
#endif
}

void app_bt_sleep_init(void)
{
    bt_adapter_write_sleep_enable(true);
}

uint8_t bes_bt_me_count_mobile_link(void)
{
#ifdef BLE_ONLY_ENABLED
    return 0;
#else
    return app_bt_count_mobile_link();
#endif
}

void bes_bt_sleep_init(void)
{
    bt_adapter_write_sleep_enable(true);
}

void bes_bt_me_gen_addr_for_debug(void)
{
    gen_bt_addr_for_debug();
}

void bes_bt_me_set_stay_active_mode(bool keep_active_mode, BT_ACTIVE_MODE_KEEP_USER_T user, uint32_t link_id)
{
#ifndef BLE_ONLY_ENABLED
    app_bt_set_keep_active_mode(keep_active_mode, user, link_id);
#endif
}

bt_status_t bes_bt_me_write_access_mode(btif_accessible_mode_t mode,bool disable_before_update)
{
#ifdef BLE_ONLY_ENABLED
    return BT_STS_SUCCESS;
#else
    return bt_export_me_write_access_mode(mode, disable_before_update);
#endif
}

void bes_bt_me_send_bt_key(APP_KEY_STATUS *status)
{
    bt_key_send(status);
}

void app_bt_volumeup(void)
{
#ifndef BLE_ONLY_ENABLED
    audio_player_volume_up();
#endif
}

void app_bt_volumedown(void)
{
#ifndef BLE_ONLY_ENABLED
    audio_player_volume_down();
#endif
}

#ifdef __INTERACTION__
extern uint8_t g_findme_fadein_vol;

static uint8_t bt_export_get_findme_fadein_vol(void)
{
    return g_findme_fadein_vol;
}

static void bt_export_set_findme_fadein_vol(uint8_t vol)
{
    g_findme_fadein_vol = vol;
}
#endif

uint8_t a2dp_convert_local_vol_to_bt_vol(uint8_t localVol)
{
#ifdef BT_A2DP_SUPPORT
    return unsigned_range_value_map(localVol, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX, 0, MAX_A2DP_VOL);
#else
    return 0xFF;
#endif
}

uint8_t bes_bt_a2dp_local_vol_to_bt_vol(uint8_t localVol)
{
#ifdef BT_A2DP_SUPPORT
    return a2dp_convert_local_vol_to_bt_vol(localVol);
#else
    return 0xFF;
#endif
}

void bes_bt_a2dp_report_speak_gain(void)
{
#ifdef BT_A2DP_SUPPORT
    btapp_a2dp_report_speak_gain();
#endif
}

static const bta_key_click_event_cb_t *key_event_cb = NULL;

void bt_adapter_register_key_evt_handle_cb(const bta_key_click_event_cb_t *cbs)
{
    key_event_cb = cbs;
}

void bt_key_handle_bt_func_click(void)
{
    if (key_event_cb->key_tripleclick_func)
    {
        key_event_cb->key_tripleclick_func();
    }
}

void bt_key_handle_func_doubleclick(void)
{
    if (key_event_cb->key_doubleclick_func)
    {
        key_event_cb->key_doubleclick_func();
    }
}

void bt_key_handle_func_tripleclick(void)
{
    if (key_event_cb->key_tripleclick_func)
    {
        key_event_cb->key_tripleclick_func();
    }
}

void bt_key_handle_func_longpress(void)
{
    if (key_event_cb->key_longpress_func)
    {
        key_event_cb->key_longpress_func();
    }
}

void bes_bt_hfp_report_speak_gain(void)
{
#ifndef BLE_ONLY_ENABLED
    btapp_hfp_report_speak_gain();
#endif
}

void bt_key_handle_music_playback(void)
{
    PLAYBACK_STATE_E music_state = app_bt_get_music_playback_status();
    if(music_state == PLAYING)
    {
        app_audio_control_media_pause();
    }
    else
    {
        app_audio_control_media_play();
    }
}

void bt_key_handle_call(CALL_STATE_E call_state)
{
    switch (call_state)
    {
        case CALL_STATE_INCOMING:
           app_audio_control_call_answer();
        break;
        case CALL_STATE_ALERTING:
            //fall through
        case CALL_STATE_OUTGOING:
            app_audio_control_call_terminate();
        break;
        case CALL_STATE_ACTIVE:
            app_audio_control_call_terminate();
        break;
        case CALL_STATE_THREE_WAY_INCOMING:
            app_audio_control_handle_three_way_incoming_call();
            break;
        case CALL_STATE_TRREE_WAY_HOLD_CALLING:
            app_audio_control_release_active_call();
            break;
        //case HFCALL_MACHINE_CURRENT_3WAY_INCOMMING:
        //    bes_bt_hfp_key_handler(HFP_KEY_THREEWAY_HANGUP_AND_ANSWER);
        //break;
        //case HFCALL_MACHINE_CURRENT_3WAY_HOLD_CALLING:
        //   bes_bt_hfp_key_handler(HFP_KEY_THREEWAY_HOLD_AND_ANSWER);
        //break;
        default:
        break;
    }
}

void bt_key_handle_func_click(void)
{
    DEBUG_INFO(1,"%s enter",__func__);

#ifdef BT_TEST_CURRENT_KEY
#ifndef BLE_ONLY_ENABLED
    bt_drv_accessmode_switch_test();
#endif
    return;
#endif

#if BLE_AUDIO_ENABLED
    CALL_STATE_E call_state = app_bt_get_call_state();
    if (call_state == CALL_STATE_IDLE)
    {
       bt_key_handle_music_playback();
    }
    else
    {
        bt_key_handle_call(call_state);
    }
#else
#ifndef BLE_ONLY_ENABLED
    bt_key_handle_bt_func_click();
#endif
#endif
#if HF_CUSTOM_FEATURE_SUPPORT & HF_CUSTOM_FEATURE_SIRI_REPORT
    extern int open_siri_flag;
    open_siri_flag = 0;
#endif
    return;
}

void bt_key_handle_customer_doubleclick(void)
{
    bt_key_handle_music_playback();
}

static void bt_key_common_handle_func_doubleclick(void)
{
#ifdef BLE_ONLY_ENABLED
#else
    bt_key_handle_func_doubleclick();
#endif
}

static void bt_key_common_handle_func_tripleclick(void)
{
#ifdef BLE_ONLY_ENABLED
#else
    bt_key_handle_func_tripleclick();
#endif
}

static void bt_key_common_handle_func_longpress(void)
{
#ifdef BLE_ONLY_ENABLED
#else
    bt_key_handle_func_longpress();
#endif
}

void bt_key_handle_func_key(enum APP_KEY_EVENT_T event)
{
    switch (event) {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            bt_key_handle_func_click();
            break;
        case  APP_KEY_EVENT_DOUBLECLICK:
            bt_key_common_handle_func_doubleclick();
            break;
        case APP_KEY_EVENT_TRIPLECLICK:
            bt_key_common_handle_func_tripleclick();
            break;
        case  APP_KEY_EVENT_LONGPRESS:
            bt_key_common_handle_func_longpress();
            break;
        default:
            DEBUG_INFO(0,"unregister func key event=%x", event);
            break;
    }
}

#if defined(__APP_KEY_FN_STYLE_A__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumeup();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_LONGPRESS:
            bes_bt_a2dp_key_handler(AVRCP_KEY_FORWARD);
            break;
#endif
#if defined(BT_SOURCE)
        case  APP_KEY_EVENT_DOUBLECLICK:
            if (app_bt_source_nv_snk_or_src_enabled())  {
                //debug switch src mode
                struct nvrecord_env_t *nvrecord_env=NULL;
                nv_record_env_get(&nvrecord_env);
                if(app_bt_source_is_enabled())
                {
                    nvrecord_env->src_snk_flag.src_snk_mode = 1;
                }
                else
                {
                    nvrecord_env->src_snk_flag.src_snk_mode = 0;
                }
                nv_record_env_set(nvrecord_env);
                app_reset();
            }
            break;
#endif
        default:
            DEBUG_INFO(1,"unregister up key event=%x",event);
            break;
    }
}

void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            app_bt_volumedown();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_LONGPRESS:
            bes_bt_a2dp_key_handler(AVRCP_KEY_BACKWARD);
            break;
#endif
        default:
            DEBUG_INFO(1,"unregister down key event=%x",event);
            break;
    }
}
#else //#elif defined(__APP_KEY_FN_STYLE_B__)
void bt_key_handle_up_key(enum APP_KEY_EVENT_T event)
{
    DEBUG_INFO(1,"%s",__func__);
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumeup();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            bes_bt_a2dp_key_handler(AVRCP_KEY_FORWARD);
            break;
#endif
        default:
            DEBUG_INFO(1,"unregister up key event=%x",event);
            break;
    }
}

void bt_key_handle_down_key(enum APP_KEY_EVENT_T event)
{
    switch(event)
    {
        case  APP_KEY_EVENT_REPEAT:
            app_bt_volumedown();
            break;
#ifdef BT_AVRCP_SUPPORT
        case  APP_KEY_EVENT_UP:
        case  APP_KEY_EVENT_CLICK:
            bes_bt_a2dp_key_handler(AVRCP_KEY_BACKWARD);
            break;
#endif
        default:
            DEBUG_INFO(1,"unregister down key event=%x",event);
            break;
    }
}
#endif

APP_KEY_STATUS bt_key;
static void bt_update_key_event(uint32_t code, uint8_t event)
{
    DEBUG_INFO(3,"%s code:%d evt:%d",__func__, code, event);

    bt_key.code = code;
    bt_key.event = event;
    btif_osapi_notify_evm();
}

void bt_key_send(APP_KEY_STATUS *status)
{
    uint32_t lock = int_lock();
    bool isKeyBusy = false;
    if (0xff != bt_key.code)
    {
        isKeyBusy = true;
    }
    int_unlock(lock);

    if (!isKeyBusy)
    {
        app_bt_start_custom_function_in_bt_thread(
            (uint32_t)status->code,
            (uint32_t)status->event,
            (uint32_t)bt_update_key_event);
    }
}

void bt_key_handle(void)
{
    if(bt_key.code != 0xff)
    {
        DEBUG_INFO(3,"%s code:%d evt:%d",__func__, bt_key.code, bt_key.event);
        switch(bt_key.code)
        {
            case BTAPP_FUNC_KEY:
#if defined(BT_SOURCE)
                if(app_bt_source_is_enabled())
                {
                    bt_key_handle_source_func_key((enum APP_KEY_EVENT_T)bt_key.event);
                }
                else
#endif
                {
                    bt_key_handle_func_key((enum APP_KEY_EVENT_T)bt_key.event);
                }
                break;
            case BTAPP_VOLUME_UP_KEY:
                bt_key_handle_up_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
            case BTAPP_VOLUME_DOWN_KEY:
                bt_key_handle_down_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#if defined(SUPPORT_SIRI) && defined(HF_CUSTOM_FEATURE_SUPPORT)
            case BTAPP_RELEASE_KEY:
                bt_key_handle_siri_key((enum APP_KEY_EVENT_T)bt_key.event);
                break;
#endif
            default:
                DEBUG_INFO(0,"bt_key_handle  undefined key");
                break;
        }
        bt_key.code = 0xff;
    }
}

void bt_key_init(void)
{
#ifdef APP_KEY_ENABLE
    Besbt_hook_handler_set(BESBT_HOOK_USER_2, bt_key_handle);
    bt_key.code = 0xff;
    bt_key.event = 0xff;
#endif
}

bool app_bt_update_tx_power_idx(uint16_t handle, int8_t tx_power_idx)
{
#ifdef IBRT
    btif_remote_device_t* dev = app_bt_get_remote_dev_by_handle(handle);
    uint8_t device_id = btif_me_get_device_id_from_rdev(dev);

    if (device_id == BT_DEVICE_INVALID_ID) {
        return false;
    }

    TRACE(1, "bt_core:update tx power idx=%d handle:%02x", tx_power_idx, handle);
    if (device_id == BT_DEVICE_TWS_ID) {
        app_bt_manager.tws_conn.tx_power_idx = tx_power_idx;
        return true;
    } else if (device_id < BT_DEVICE_NUM) {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
        curr_device->tx_power_idx = tx_power_idx;
        return true;
    }
#endif
    return false;
}

bool app_bt_get_tx_power_idx(uint16_t handle, int8_t *tx_power_idx)
{
#ifdef IBRT
    btif_remote_device_t* dev = app_bt_get_remote_dev_by_handle(handle);
    uint8_t device_id = btif_me_get_device_id_from_rdev(dev);

    if (device_id == BT_DEVICE_INVALID_ID) {
        return false;
    }

    if (device_id == BT_DEVICE_TWS_ID) {
        *tx_power_idx = app_bt_manager.tws_conn.tx_power_idx;
        return true;
    } else if (device_id < BT_DEVICE_NUM) {
        struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
        *tx_power_idx = curr_device->tx_power_idx;
        return true;
    }
#endif
    return false;
}

bt_status_t bes_bt_me_set_ble_adv_data(uint8_t len, uint8_t * data)
{
    return btif_me_ble_set_adv_data(len, data);
}

bt_status_t bes_bt_me_set_ble_scan_rsp_data(U8 len, U8 * data)
{
    return btif_me_ble_set_scan_rsp_data(len, data);
}

bt_status_t bes_bt_me_set_ble_adv_parameters(btif_adv_para_struct_t * para)
{
    return btif_me_ble_set_adv_parameters(para);
}

bt_status_t bes_bt_me_set_ble_adv_en(uint8_t en)
{
    return btif_me_ble_set_adv_en(en);
}

bt_status_t bes_bt_me_set_ble_scan_parameter(btif_scan_para_struct_t * para)
{
    return btif_me_ble_set_scan_parameter(para);
}

bt_status_t bes_bt_me_set_ble_scan_en(uint8_t scan_en, uint8_t filter_duplicate)
{
    return btif_me_ble_set_scan_en(scan_en, filter_duplicate);
}

bt_status_t bes_bt_me_receive_ble_adv_report(void (*cb)(const btif_ble_adv_report* report))
{
    return btif_me_ble_receive_adv_report(cb);
}

bt_status_t bes_bt_me_read_ble_chnle_map(uint16_t conn_handle)
{
    return btif_me_ble_read_chnle_map(conn_handle);
}

bt_status_t bes_bt_me_set_ble_host_chnl_classification(uint8_t *chnl_map)
{
    return btif_me_ble_set_host_chnl_classification(chnl_map);
}

bt_status_t bes_bt_me_dbg_set_iso_quality_rep_thr(uint16_t conn_handle, uint16_t qlty_rep_evt_cnt_thr,
    uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr,
    uint16_t retrans_pkts_thr, uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr,
    uint16_t duplicate_pkts_thr)
{
    return btif_me_bt_dbg_set_iso_quality_rep_thr(conn_handle, qlty_rep_evt_cnt_thr, tx_unack_pkts_thr,
        tx_flush_pkts_thr, tx_last_subevent_pkts_thr, retrans_pkts_thr, crc_err_pkts_thr, rx_unreceived_pkts_thr,
        duplicate_pkts_thr);
}

bt_status_t bes_bt_me_dbg_set_txpwr_link_thd(uint8_t index, uint8_t enable,uint8_t link_id, uint16_t rssi_avg_nb_pkt,
    int8_t rssi_high_thd, int8_t rssi_low_thd, int8_t rssi_below_low_thd, int8_t rssi_interf_thd)
{
    return btif_me_bt_dbg_set_txpwr_link_thd(index, enable, link_id, rssi_avg_nb_pkt, rssi_high_thd,
        rssi_low_thd, rssi_below_low_thd, rssi_interf_thd);
}

int bes_bt_me_set_bt_local_name(const unsigned char *device_name, uint8_t len)
{
    return bt_set_local_dev_name(device_name, len);
}

void bes_bt_me_reset_bt_controller(void)
{
    btif_me_reset_bt_controller();
}

bt_status_t bes_bt_me_set_bt_address(const uint8_t *local_addr)
{
    return btif_me_set_bt_address(local_addr);
}

bt_status_t bes_bt_me_set_ble_address(const uint8_t *local_addr)
{
    return btif_me_set_ble_bd_address(local_addr);
}

bt_status_t bes_bt_me_set_bt_name(char *localName, uint8_t length)
{
    if ((!localName) || (!length))
    {
        return BT_STS_FAILED;
    }

    bt_set_local_dev_name((const unsigned char*)localName, strlen((const char*)localName) + 1);
    bt_stack_config((const unsigned char*)localName, strlen((const char*)localName) + 1);

    return BT_STS_SUCCESS;
}

bt_status_t bes_bt_me_set_ble_name(char *localName, unsigned char length)
{
    if ((!localName) || (!length))
    {
        return BT_STS_FAILED;
    }

    return BT_STS_SUCCESS;
}

bt_status_t bes_bt_me_acl_set_link_monitor(uint16_t conn_handle, uint8_t control_flag, uint8_t report_format,
    uint32_t data_format, uint8_t report_unit)
{
    return btif_me_set_link_lowlayer_monitor_by_handle(conn_handle, control_flag, report_format, data_format, report_unit);
}

bt_status_t bes_bt_me_set_accessible_mode(btif_accessible_mode_t mode,
                                        const btif_access_mode_info_t *info)
{
    return app_bt_set_access_mode(mode);
}

bt_status_t bes_bt_me_cancel_create_connection(const bt_bdaddr_t *remote)
{
    return btif_hci_cancel_create_connection(remote);
}

bt_status_t bes_bt_get_remote_dev_addr_by_device_id(uint8_t deviceId, uint8_t* btAddr)
{
    return app_bt_get_device_bdaddr(deviceId, btAddr);
}

bt_status_t bes_bt_get_remote_device_id_by_addr(uint8_t* btAddr, uint8_t *deviceId)
{
    *deviceId = app_bt_get_device_id_byaddr((bt_bdaddr_t *)btAddr);
    if (*deviceId == BT_DEVICE_INVALID_ID)
    {
        return BT_STS_NOT_FOUND;
    }

    return BT_STS_SUCCESS;
}

uint8_t bes_bt_get_remote_device_id_by_handle(uint16_t conn_hdl)
{
    return app_bt_get_device_id_byhandle(conn_hdl);
}

bt_status_t bes_bt_get_remote_addr_by_conn_handle( uint16_t conn_hdl, uint8_t* btAddr)
{
    uint8_t* btAddr_p = NULL;

    btAddr_p = app_bt_get_addr_byhandle(conn_hdl);
    if (!btAddr_p)
    {
        return BT_STS_NOT_FOUND;
    }
    memcpy(btAddr, btAddr_p, 6);

    return BT_STS_SUCCESS;
}

void *bes_bt_me_profile_active_store_ptr_get(uint8_t *bdAddr)
{
    return app_bt_profile_active_store_ptr_get(bdAddr);
}

uint16_t bes_bt_get_remote_conn_handle_by_addr(uint8_t* btAddr)
{
    return app_bt_get_handle_byaddr((bt_bdaddr_t *)btAddr);
}

void bes_bt_me_confirmation_register_callback(btif_confirmation_req_callback_t callback)
{
    btif_confirmation_register_callback(callback);
}

void bes_bt_me_confirmation_resp(struct bdaddr_t *bdaddr, bool accept)
{
    btif_confirmation_resp(bdaddr, accept);
}

void fast_pair_enter_pairing_mode_handler(void)
{
#if defined(IBRT)
#if defined(IBRT_UI)
    app_ui_update_scan_type_policy(SCAN_EV_ENTER_PAIRING);
#endif
#else
#ifndef BLE_ONLY_ENABLED
    app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
#endif
#endif

#ifdef __INTERCONNECTION__
    clear_discoverable_adv_timeout_flag();
    app_interceonnection_start_discoverable_adv(INTERCONNECTION_BLE_FAST_ADVERTISING_INTERVAL,
            APP_INTERCONNECTION_FAST_ADV_TIMEOUT_IN_MS);
#endif
}

void bes_bt_me_fast_pair_enter_pairing_mode_handler(void)
{
    fast_pair_enter_pairing_mode_handler();
}

#ifdef __IAG_BLE_INCLUDE__
#define APP_FAST_BLE_ADV_TIMEOUT_IN_MS 30000
osTimerId app_fast_ble_adv_timeout_timer = NULL;
static int app_fast_ble_adv_timeout_timehandler(void const *param);
osTimerDef(APP_FAST_BLE_ADV_TIMEOUT_TIMER, ( void (*)(void const *) )app_fast_ble_adv_timeout_timehandler);

void app_start_fast_connectable_ble_adv(uint16_t advInterval)
{
    if (NULL == app_fast_ble_adv_timeout_timer)
    {
        app_fast_ble_adv_timeout_timer =
            osTimerCreate(osTimer(APP_FAST_BLE_ADV_TIMEOUT_TIMER),
                          osTimerOnce,
                          NULL);
    }

    osTimerStart(app_fast_ble_adv_timeout_timer, APP_FAST_BLE_ADV_TIMEOUT_IN_MS);
    bes_ble_gap_start_connectable_adv(advInterval);
}

static int app_fast_ble_adv_timeout_timehandler(void const *param)
{
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
    return 0;
}

void app_stop_fast_connectable_ble_adv_timer(void)
{
    if (NULL != app_fast_ble_adv_timeout_timer)
    {
        osTimerStop(app_fast_ble_adv_timeout_timer);
    }
}
#endif

#ifdef __IAG_BLE_INCLUDE__
void bes_bt_me_stop_fast_connectable_ble_adv(void)
{
    app_stop_fast_connectable_ble_adv_timer();
}
#endif

#ifdef __INTERACTION__
uint8_t bes_bt_me_get_findme_fadein_vol(void)
{
    return bt_export_get_findme_fadein_vol();
}

void bes_bt_me_set_findme_fadein_vol(uint8_t vol)
{
    bt_export_set_findme_fadein_vol(vol);
}
#endif

extern void app_ble_custom_deinit(void);

bt_status_t bt_host_init(void (*init_done_cb)())
{
    bes_bt_app_init();
    hci_set_rx_thread_priority(osPriorityHigh);
#if (defined(BT_SOURCE) && defined(A2DP_SOURCE_LHDC_ON) && defined(A2DP_SOURCE_LHDCV5_ON))
    bt_host_thread_init(osPriorityHigh1, init_done_cb);
#else
    bt_host_thread_init(osPriorityHigh, init_done_cb);
#endif

    return BT_STS_SUCCESS;
}

bt_status_t bt_host_deinit()
{
    app_ble_custom_deinit();
    bt_host_thread_deinit();
    return BT_STS_SUCCESS;
}

bool bes_bt_me_is_bt_thread(void)
{
    return app_bt_is_besbt_thread();
}

extern void app_bt_buf_init(void);
void bes_bt_app_init(void)
{
    app_bt_buf_init();
    app_bt_mail_init();
#ifndef BLE_ONLY_ENABLED
    app_bt_init();
#endif
}

void bt_print_bdaddr(const uint8_t *bdaddr_array)
{
    TRACE(0, "%02x:...:%02x:%02x", bdaddr_array[5], bdaddr_array[1], bdaddr_array[0]);
}

void bt_print_bdaddr_full(const uint8_t *bdaddr_array)
{
    TRACE(0, "%02x:%02x:%02x:%02x:%02x:%02x", bdaddr_array[5], bdaddr_array[4], bdaddr_array[3], bdaddr_array[2], bdaddr_array[1], bdaddr_array[0]);
}

bool bt_is_allzero_bdaddr(const uint8_t *bdaddr_array)
{
    for (int index = 0; index < BTIF_BD_ADDR_SIZE; ++index)
    {
        if (bdaddr_array[index])
        {
            return false;
        }
    }
    return true;
}
