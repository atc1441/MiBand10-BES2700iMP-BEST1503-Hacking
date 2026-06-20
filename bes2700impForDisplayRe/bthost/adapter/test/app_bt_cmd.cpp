/***************************************************************************
*
* Copyright 2015-2020 BES.
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
#include "app_trace_rx.h"
#include "app_bt_cmd.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth.h"
#include "crc16_c.h"
#include "heap_api.h"
#include "hci_api.h"
#include "me_api.h"
#include "spp_api.h"
#include "l2cap_api.h"
#include "conmgr_api.h"
#include "bt_if.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "app_bt.h"
#include "btapp.h"
#include "app_factory_bt.h"
#include "apps.h"
#include "app_a2dp.h"
#include "app_hfp.h"
#include "nvrecord_extension.h"
#include "nvrecord_bt.h"
#include "app_bt_func.h"
#include "app_testmode.h"
#include "besbt_cfg.h"
#include "besaud_api.h"
#include "ddbif.h"
#include "audio_codec_api.h"
#include "app_ble_test.h"
#include "hid_api.h"

#ifdef APP_CHIP_BRIDGE_MODULE
#include "app_chip_bridge.h"
#endif

#if defined(BT_SOURCE)
#include "bt_source.h"
#include "app_a2dp_source.h"
#if defined(BT_WATCH_MASTER) || defined(BT_WATCH_SLAVE)
#include "bts_a2dp_source.h"
#endif
#endif

#ifdef AUDIO_LINEIN
#include "app_bt_media_manager.h"
#endif

#ifdef LOCAL_AUDIO_SUPPORT
extern "C" void pmu_reboot(void);
#include "hal_bootmode.h"
#ifdef LOCAL_AUDIO_TEST_ENABLE
#include "app_local_audio_stream_handler.h"
#endif // LOCAL_AUDIO_TEST_ENABLE
#endif // LOCAL_AUDIO_SUPPORT

#include "audio_player_adapter.h"

static bt_bdaddr_t g_app_bt_pts_addr = {{
#if 1
    0xed, 0xb6, 0xf4, 0xdc, 0x1b, 0x00
#elif 0
    0x81, 0x33, 0x33, 0x22, 0x11, 0x11
#elif 0
    0x13, 0x71, 0xda, 0x7d, 0x1a, 0x00
#else
    0x14, 0x71, 0xda, 0x7d, 0x1a, 0x00
#endif
}};

bt_bdaddr_t *app_bt_get_pts_address(void)
{
    return &g_app_bt_pts_addr;
}

#ifdef APP_TRACE_RX_ENABLE
WEAK void app_otaMode_enter(APP_KEY_STATUS *status, void *param)
{

}

static bool app_bt_scan_bdaddr_from_string(const char* param, uint32_t len, bt_bdaddr_t *out_addr)
{
    int bytes[sizeof(bt_bdaddr_t)] = {0};

    if (len < 17)
    {
        TRACE(0, "%s wrong len %d '%s'", __func__, len, param);
        return false;
    }

    TRACE(0,  "%s '%s'", __func__,param);


    if (6 != sscanf(param, "%x:%x:%x:%x:%x:%x", bytes+0, bytes+1, bytes+2, bytes+3, bytes+4, bytes+5))
    {
        TRACE(0, "%s parse address failed %s sscanf=%d", __func__, param,sscanf(param, "%x:%x:%x:%x:%x:%x", bytes+0, bytes+1, bytes+2, bytes+3, bytes+4, bytes+5));
        return false;
    }

    bt_bdaddr_t addr = {{
        (uint8_t)(bytes[0]&0xff),
        (uint8_t)(bytes[1]&0xff),
        (uint8_t)(bytes[2]&0xff),
        (uint8_t)(bytes[3]&0xff),
        (uint8_t)(bytes[4]&0xff),
        (uint8_t)(bytes[5]&0xff)}};

    *out_addr = addr;

    return true;
}

static bool char2hex(char ch, uint8_t *hex)
{
    if ((ch >= '0') && (ch <= '9')) {
        *hex = ch - '0';
    } else if ((ch >= 'a') && (ch <= 'f')) {
        *hex = ch - 'a' + 10;
    } else if ((ch >= 'A') && (ch <= 'F')) {
        *hex = ch - 'A' + 10;
    } else {
        return false;
    }
    return true;
}

POSSIBLY_UNUSED static bool strdec2uint16hex(const char*str, uint32 len, uint16_t *data)
{
    uint8_t id = 0;
    uint8_t arry_id = 0;
    uint16_t data_temp = 0;
    uint8_t carry_bit = 0;
    uint8_t arry[4] = {0};
    uint8_t remain_len = len;
    if (remain_len >4) {
        return false;
    }
    carry_bit = remain_len;
    for (;remain_len > 0; remain_len--) {
        if (!char2hex(str[id], &arry[arry_id])) {
            return false;
        }
        id++;
        arry_id++;
    }

    if (carry_bit == 4) {
        data_temp = arry[0]*1000 + arry[1]*100 + arry[2]*10 + arry[3];
    }
    else if (carry_bit == 3) {
        data_temp = arry[0]*100 + arry[1]*10 + arry[2];
    } else if (carry_bit == 2) {
        data_temp = arry[0]*10 + arry[1];
    } else if (carry_bit == 1) {
        data_temp = arry[0];
    } else {
        return false;
    }
    *data = data_temp;
    return true;
}

#define APP_BT_CMD_TABLE_MAX    20
typedef struct
{
    struct
    {
        uint8_t cmd_number;
        const app_bt_host_cmd_table_t* table_list;
    } cmd_table[APP_BT_CMD_TABLE_MAX];
} app_bt_cmd_env_t;

static app_bt_cmd_env_t app_bt_cmd_env = {0};

extern void bt_audio_local_volume_up();
extern void bt_audio_volume_down();

static void app_bt_trace_rx_sleep(const char* param, uint32_t param_len)
{
    TRACE(1, "%s", __func__);
    hal_trace_rx_sleep();
}

static void app_bt_shutdown_test(const char* param, uint32_t param_len)
{
    app_shutdown();
}

static void app_bt_flush_nv_test(const char* param, uint32_t param_len)
{
    nv_record_flash_flush();
}

static void app_bt_show_device_linkkey_test(const char* param, uint32_t param_len)
{
    TRACE(1, "%s", __func__);
    nv_record_all_ddbrec_print();
}

static void app_bt_hci_print_statistic(const char* param, uint32_t param_len)
{
    TRACE(1, "%s", __func__);
    btif_hci_print_statistic();
}

static void app_enable_hci_cmd_evt_debug(const char* param, uint32_t param_len)
{
    btif_hci_enable_cmd_evt_debug(true);
}

static void app_disable_hci_cmd_evt_debug(const char* param, uint32_t param_len)
{
    btif_hci_enable_cmd_evt_debug(false);
}

static void app_enable_hci_tx_flow_debug(const char* param, uint32_t param_len)
{
    btif_hci_enable_tx_flow_debug(true);
}

static void app_disable_hci_tx_flow_debug(const char* param, uint32_t param_len)
{
    btif_hci_enable_tx_flow_debug(false);
}

static void app_ota_mode_enter_test(const char* param, uint32_t param_len)
{
    app_otaMode_enter(NULL, NULL);
}

static void app_bt_sink_stop_sniff_test(const char* param, uint32_t param_len)
{
#ifndef BLE_ONLY_ENABLED
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (curr_device)
    {
        DUMP8("%02x ", &curr_device->remote, 6);
        btif_me_stop_sniff(curr_device->acl_conn_hdl);
    }
#endif
}

static void app_bt_source_stop_sniff_test(const char* param, uint32_t param_len)
{
#ifndef BLE_ONLY_ENABLED
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_SOURCE_DEVICE_ID_1);
    if (curr_device)
    {
        DUMP8("%02x ", &curr_device->remote, 6);
        btif_me_stop_sniff(curr_device->acl_conn_hdl);
    }
#endif
}

#ifdef APP_SPP_DEMO
extern void app_spp_main(const bt_bdaddr_t *addr);
extern bool app_spp_serial_port_send_data(bt_bdaddr_t *remote, const uint8_t* ptrData, uint16_t length);
extern void app_spp_example_set_rx_loopback_mode(bool flag);
static void app_bt_client_spp_test(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_main(&curr_device->remote);
}

static void app_service_spp_test(const char* param, uint32_t param_len)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_main(NULL);
}

static void app_spp_data_test(const char* param, uint32_t param_len)
{
    uint8_t test_data[]={'s', 'p', 'p', ' ', 's', 'e', 'n', 'd', ' ', 'd', 'a', 't', 'a', ' ', 't', 'e', 's', 't', '\0'};
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    app_spp_serial_port_send_data(&curr_device->remote, &test_data[0], 19);
}

static void app_spp_set_loopback_test(const char* param, uint32_t param_len)
{
    app_spp_example_set_rx_loopback_mode(true);
}

static void app_spp_clear_loopback_test(const char* param, uint32_t param_len)
{
    app_spp_example_set_rx_loopback_mode(false);
}
#endif

#ifndef BLE_ONLY_ENABLED
void app_bt_key_handle_func_click_test(const char* param, uint32_t param_len)
{
    bt_key_handle_func_click();
}

void app_bt_key_handle_func_doubleclick_test(const char* param, uint32_t param_len)
{
    bt_key_handle_func_doubleclick();
}

void app_bt_key_handle_func_longpress_test(const char* param, uint32_t param_len)
{
    bt_key_handle_func_longpress();
}

void app_bt_start_both_scan_test(const char* param, uint32_t param_len)
{
    app_bt_set_access_mode(BTIF_BAM_GENERAL_ACCESSIBLE);
}

void app_bt_stop_both_scan_test(const char* param, uint32_t param_len)
{
    app_bt_set_access_mode(BTIF_BAM_NOT_ACCESSIBLE);
}

void app_bt_disconnect_acl_link_test(const char* param, uint32_t param_len)
{
    app_bt_disconnect_acl_link();
}

void app_bt_pts_rfc_register_channel_test(const char* param, uint32_t param_len)
{
    btif_pts_rfc_register_channel();
}

void app_bt_pts_pts_rfc_close_test(const char* param, uint32_t param_len)
{
    btif_pts_rfc_close();
}
#endif

#if defined(BT_HFP_TEST_SUPPORT)
static void app_bt_pts_create_hf_channel_test(const char* param, uint32_t param_len)
{
    app_bt_reconnect_hfp_profile(app_bt_get_pts_address());
}

static void app_bt_pts_hf_disc_service_link_test(const char* param, uint32_t param_len)
{
    app_pts_hf_disc_service_link();
}

static void app_bt_pts_hf_create_audio_link_test(const char* param, uint32_t param_len)
{
    app_pts_hf_create_audio_link();
}

static void app_bt_pts_hf_disc_audio_link_test(const char* param, uint32_t param_len)
{
    app_pts_hf_disc_audio_link();
}

static void app_bt_pts_hf_send_key_pressed_test(const char* param, uint32_t param_len)
{
    app_pts_hf_send_key_pressed();
}

static void app_bt_pts_hf_answer_call_test(const char* param, uint32_t param_len)
{
    app_pts_hf_answer_call();
}

static void app_bt_pts_hf_hangup_call_test(const char* param, uint32_t param_len)
{
    app_pts_hf_hangup_call();
}

static void app_bt_pts_hf_dial_number_test(const char* param, uint32_t param_len)
{
    app_pts_hf_dial_number();
}

static void app_bt_pts_hf_dial_number_memory_index_test(const char* param, uint32_t param_len)
{
    app_pts_hf_dial_number_memory_index();
}

static void app_bt_pts_hf_dial_number_invalid_memory_index_test(const char* param, uint32_t param_len)
{
    app_pts_hf_dial_number_invalid_memory_index();
}

static void app_bt_pts_hf_redial_call_test(const char* param, uint32_t param_len)
{
    app_pts_hf_redial_call();
}

static void app_bt_pts_hf_release_active_call_test(const char* param, uint32_t param_len)
{
    app_pts_hf_release_active_call();
}

static void app_bt_pts_hf_release_active_call_2_test(const char* param, uint32_t param_len)
{
    app_pts_hf_release_active_call_2();
}

static void app_bt_pts_hf_hold_active_call_test(const char* param, uint32_t param_len)
{
    app_pts_hf_hold_active_call();
}

static void app_bt_pts_hf_hold_active_call_2_test(const char* param, uint32_t param_len)
{
    app_pts_hf_hold_active_call_2();
}

static void app_bt_pts_hf_hold_call_transfer_test(const char* param, uint32_t param_len)
{
    app_pts_hf_hold_call_transfer();
}

static void app_bt_pts_hf_vr_enable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_vr_enable();
}

static void app_bt_pts_hf_vr_disable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_vr_disable();
}

static void app_bt_pts_hf_list_current_calls_test(const char* param, uint32_t param_len)
{
    app_pts_hf_list_current_calls();
}

static void app_bt_pts_hf_report_mic_volume_test(const char* param, uint32_t param_len)
{
    app_pts_hf_report_mic_volume();
}

static void app_bt_pts_hf_attach_voice_tag_test(const char* param, uint32_t param_len)
{
    app_pts_hf_attach_voice_tag();
}

static void app_bt_pts_hf_update_ind_value_test(const char* param, uint32_t param_len)
{
    app_pts_hf_update_ind_value();
}

static void app_bt_pts_hf_ind_activation_test(const char* param, uint32_t param_len)
{
    app_pts_hf_ind_activation();
}

static void app_bt_pts_hf_acs_bv_09_i_set_enable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_acs_bv_09_i_set_enable();
}

static void app_bt_pts_hf_acs_bv_09_i_set_disable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_acs_bv_09_i_set_disable();
}

static void app_bt_pts_hf_acs_bi_13_i_set_enable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_acs_bi_13_i_set_enable();
}

static void app_bt_pts_hf_acs_bi_13_i_set_disable_test(const char* param, uint32_t param_len)
{
    app_pts_hf_acs_bi_13_i_set_disable();
}

static void app_bt_pts_hf_siri_voice_enable_test(const char* param, uint32_t param_len)
{
    app_pts_hfp_siri_voice_enable();
}

static void app_bt_pts_hf_siri_voice_disable_test(const char* param, uint32_t param_len)
{
    app_pts_hfp_siri_voice_disable();
}

#if defined(BT_HFP_AG_ROLE)
static void app_bt_pts_hf_ag_set_connectable_state(const char* param, uint32_t param_len)
{
    app_bt_source_set_connectable_state(true);
}

static void app_bt_pts_create_hf_ag_channel(const char* param, uint32_t param_len)
{
    //app_bt_source_set_connectable_state(true);
    bt_source_reconnect_hfp_profile(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_create_audio_link(const char* param, uint32_t param_len)
{
    bt_source_create_audio_link(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_disc_audio_link(const char* param, uint32_t param_len)
{
    bt_source_disc_audio_link(app_bt_get_pts_address());
}

static void app_bt_pts_hf_ag_send_mobile_signal_level(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_signal_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_send_mobile_signal_level_0(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_signal_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_service_status(const char* param, uint32_t param_len)
{
    btif_ag_send_service_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_send_service_status_0(const char* param, uint32_t param_len)
{
    btif_ag_send_service_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_call_active_status(const char* param, uint32_t param_len)
{
    btif_ag_send_call_active_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_hangup_call(const char* param, uint32_t param_len)
{
    btif_ag_send_call_active_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_callsetup_status(const char* param, uint32_t param_len)
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,1);
}

static void app_bt_pts_hf_ag_send_callsetup_status_0(const char* param, uint32_t param_len)
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_callsetup_status_2(const char* param, uint32_t param_len)
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,2);
}

static void app_bt_pts_hf_ag_send_callsetup_status_3(const char* param, uint32_t param_len)
{
    btif_ag_send_callsetup_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_enable_roam(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_roam_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_disable_roam(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_roam_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_mobile_battery_level(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,3);
}

static void app_bt_pts_hf_ag_send_full_battery_level(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,5);
}

static void app_bt_pts_hf_ag_send_battery_level_0(const char* param, uint32_t param_len)
{
    btif_ag_send_mobile_battery_level(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_calling_ring(const char* param, uint32_t param_len)
{
    //const char* number = NULL;
    char number[] = "1234567";
    btif_ag_send_calling_ring(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,number);
}

static void app_bt_pts_hf_ag_enable_inband_ring_tone(const char* param, uint32_t param_len)
{
    btif_ag_set_inband_ring_tone(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,true);
}

static void app_bt_pts_hf_ag_place_a_call(const char* param, uint32_t param_len)
{
    app_bt_pts_hf_ag_send_callsetup_status(param, param_len);
    app_bt_pts_hf_ag_send_calling_ring(param, param_len);
    app_bt_pts_hf_ag_send_call_active_status(param, param_len);
    app_bt_pts_hf_ag_send_callsetup_status_0(param, param_len);
    app_bt_pts_hf_ag_create_audio_link(param, param_len);
}

static void app_bt_pts_hf_ag_ongoing_call(const char* param, uint32_t param_len)
{
    app_bt_pts_hf_ag_send_callsetup_status_2(param, param_len);
    app_bt_pts_hf_ag_send_callsetup_status_3(param, param_len);
}

static void app_bt_pts_hf_ag_ongoing_call_setup(const char* param, uint32_t param_len)
{
    app_bt_pts_hf_ag_send_callsetup_status_2(param, param_len);
    app_bt_pts_hf_ag_create_audio_link(param, param_len);
    osDelay(100);
    app_bt_pts_hf_ag_send_callsetup_status_3(param, param_len);
    app_bt_pts_hf_ag_send_call_active_status(param, param_len);
    app_bt_pts_hf_ag_send_callsetup_status_0(param, param_len);
}

static void app_bt_pts_hf_ag_answer_incoming_call(const char* param, uint32_t param_len)
{
    app_bt_pts_hf_ag_send_callsetup_status(param, param_len);
    app_bt_pts_hf_ag_send_call_active_status(param, param_len);
    app_bt_pts_hf_ag_send_callsetup_status_0(param, param_len);
    app_bt_pts_hf_ag_create_audio_link(param, param_len);
}

static void app_bt_pts_hf_ag_clear_last_dial_number(const char* param, uint32_t param_len)
{
    btif_ag_set_last_dial_number(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,false);
}

static void app_bt_pts_hf_ag_send_call_waiting_notification(const char* param, uint32_t param_len)
{
    char number[] = "7654321";
    btif_ag_send_call_waiting_notification(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,number);
}

static void app_bt_pts_hf_ag_send_callheld_status(const char* param, uint32_t param_len)
{
    btif_ag_send_callheld_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,1);
}

static void app_bt_pts_hf_ag_send_callheld_status_0(const char* param, uint32_t param_len)
{
    btif_ag_send_callheld_status(app_bt_source_find_device(app_bt_get_pts_address())->base_device->hf_channel,0);
}

static void app_bt_pts_hf_ag_send_status_3_0_4_1(const char* param, uint32_t param_len)
{
    app_bt_pts_hf_ag_send_callsetup_status_0(param, param_len);
    app_bt_pts_hf_ag_send_callheld_status(param, param_len);
}

static void app_bt_pts_hf_ag_set_pts_enable(const char* param, uint32_t param_len)
{
    app_bt_source_set_hfp_ag_pts_enable(true);
}

static void app_bt_pts_hf_ag_set_pts_ecs_01(const char* param, uint32_t param_len)
{
    app_bt_source_set_hfp_ag_pts_esc_01_enable(true);
}

static void app_bt_pts_hf_ag_set_pts_ecs_02(const char* param, uint32_t param_len)
{
    app_bt_source_set_hfp_ag_pts_esc_02_enable(true);
}

static void app_bt_pts_hf_ag_set_pts_ecc(const char* param, uint32_t param_len)
{
    app_bt_source_set_hfp_ag_pts_ecc_enable(true);
}
#endif
#endif

#if defined(BT_A2DP_TEST_SUPPORT)
static void app_bt_pts_create_av_channel_test(const char* param, uint32_t param_len)
{
    btif_pts_av_create_channel(app_bt_get_pts_address());
}

static void app_bt_pts_av_disc_channel_test(const char* param, uint32_t param_len)
{
    app_pts_av_disc_channel();
}

static void app_bt_enable_tone_intrrupt_a2dp_test(const char* param, uint32_t param_len)
{
    app_bt_manager.config.a2dp_prompt_play_only_when_avrcp_play_received = false;
}

static void app_bt_disable_tone_intrrupt_a2dp_test(const char* param, uint32_t param_len)
{
    app_bt_manager.config.a2dp_prompt_play_only_when_avrcp_play_received = true;
}

static void app_bt_enable_a2dp_delay_prompt_test(const char* param, uint32_t param_len)
{
    app_bt_manager.config.a2dp_delay_prompt_play = true;
}

static void app_bt_disable_a2dp_delay_prompt_test(const char* param, uint32_t param_len)
{
    app_bt_manager.config.a2dp_delay_prompt_play = false;
}

static void app_bt_disable_a2dp_aac_codec_test(const char* param, uint32_t param_len)
{
    app_bt_a2dp_disable_aac_codec(true);
}

static void app_bt_disable_a2dp_vendor_codec_test(const char* param, uint32_t param_len)
{
    app_bt_a2dp_disable_vendor_codec(true);
}

static void app_bt_pts_av_create_media_channel_test(const char* param, uint32_t param_len)
{
    btif_pts_av_create_media_channel();
}

static void app_bt_pts_av_close_channel_test(const char* param, uint32_t param_len)
{
    app_pts_av_close_channel();
}

static void app_bt_pts_av_send_getconf_test(const char* param, uint32_t param_len)
{
    btif_pts_av_send_getconf();
}

static void app_bt_pts_av_send_reconf_test(const char* param, uint32_t param_len)
{
    btif_pts_av_send_reconf();
}

static void app_bt_pts_av_send_open_test(const char* param, uint32_t param_len)
{
    btif_pts_av_send_open();
}

static void app_bt_pts_av_send_start_test(const char* param, uint32_t param_len)
{
    btif_pts_av_send_start();
}
#endif

#if defined(BT_AVRCP_TEST_SUPPORT) || defined(BT_HFP_TEST_SUPPORT)
#ifndef BTH_SUBSYS_ONLY
static void app_bt_audio_local_volume_up_test(const char* param, uint32_t param_len)
{
    bt_audio_local_volume_up();
}

static void app_bt_audio_volume_down_test(const char* param, uint32_t param_len)
{
    bt_audio_volume_down();
}
#endif
#endif

#ifdef BQB_PROFILE_TEST
static void app_bt_pts_reject_invalid_object_type_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_OBJECT_TYPE();
}

static void app_bt_pts_reject_invalid_channels_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_CHANNELS();
}

static void app_bt_pts_reject_invalid_sampling_freq_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_SAMPLING_FREQUENCY();
}

static void app_bt_pts_reject_invalid_drc_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_DRC();
}

static void app_bt_pts_reject_not_supp_obj_type_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_NOT_SUPPORTED_OBJECT_TYPE();
}

static void app_bt_pts_reject_not_supp_channel_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_NOT_SUPPORTED_CHANNELS();
}

static void app_bt_pts_reject_not_supp_sampling_freq_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_NOT_SUPPORTED_SAMPLING_FREQUENCY();
}

static void app_bt_pts_reject_not_supp_drc_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_NOT_SUPPORTED_DRC();
}

static void app_bt_pts_reject_invalid_codec_type_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_CODEC_TYPE();
}

static void app_bt_pts_reject_invalid_channel_mode_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_CHANNEL_MODE();
}

static void app_bt_pts_reject_invalid_subbands_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_SUBBANDS();
}

static void app_bt_pts_reject_invalid_allocation_method_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_ALLOCATION_METHOD();
}

static void app_bt_pts_reject_invalid_min_bitpoll_value_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_MINIMUM_BITPOOL_VALUE();
}

static void app_bt_pts_reject_invalid_max_bitpoll_value_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_MAXIMUM_BITPOOL_VALUE();
}

static void app_bt_pts_reject_invalid_block_length_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_BLOCK_LENGTH();
}

static void app_bt_pts_reject_invalid_cp_type_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_CP_TYPE();
}

static void app_bt_pts_reject_invalid_cp_format_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_INVALID_CP_FORMAT();
}

static void app_bt_pts_reject_not_supp_codec_type_test(const char* param, uint32_t param_len)
{
    btif_pts_reject_NOT_SUPPORTED_CODEC_TYPE();
}
#endif

#ifdef BT_USE_COHEAP_ALLOC
static void app_bt_coheap_statistics_dump(const char* param, uint32_t param_len)
{
    cobuf_print_statistic();
}

static void app_bt_coheap_enable_debug(const char* param, uint32_t param_len)
{
    cobuf_enable_debug(true);
}

static void app_bt_coheap_disable_debug(const char* param, uint32_t param_len)
{
    cobuf_enable_debug(false);
}
#endif

#if defined(NORMAL_TEST_MODE_SWITCH)
static void app_bt_enter_signal_testmode_test(const char* param, uint32_t param_len)
{
    app_enter_signal_testmode();
}

static void app_bt_exit_signal_testmode_test(const char* param, uint32_t param_len)
{
    app_exit_signal_testmode();
}

static void app_bt_reboot_and_enter_nosignal_test(const char* param, uint32_t param_len)
{
    app_reboot_and_enter_nosignal_testmode();
}

static void app_bt_reboot_and_enter_signal_test(const char* param, uint32_t param_len)
{
    app_reboot_and_enter_signal_testmode();
}

static void app_bt_enter_nosignal_tx_test(const char* param, uint32_t param_len)
{
    app_enter_nosignal_tx_testmode();
}

static void app_bt_enter_nosignal_rx_test(const char* param, uint32_t param_len)
{
    app_enter_nosignal_rx_testmode();
}

static void app_bt_exit_nosignal_trx_test(const char* param, uint32_t param_len)
{
    app_exit_nosignal_trx_testmode();
}

static void app_bt_enter_ble_tx_v1_test(const char* param, uint32_t param_len)
{
    app_enter_ble_tx_v1_testmode();
}

static void app_bt_enter_ble_rx_v1_test(const char* param, uint32_t param_len)
{
    app_enter_ble_rx_v1_testmode();
}

static void app_bt_enter_ble_tx_v2_test(const char* param, uint32_t param_len)
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_enter_ble_rx_v2_test(const char* param, uint32_t param_len)
{
    app_enter_ble_rx_v2_testmode();
}

static void app_bt_enter_ble_tx_v3_test(const char* param, uint32_t param_len)
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_enter_ble_rx_v3_test(const char* param, uint32_t param_len)
{
    app_enter_ble_rx_v2_testmode();
}

static void app_bt_enter_ble_tx_v4_test(const char* param, uint32_t param_len)
{
    app_enter_ble_tx_v2_testmode();
}

static void app_bt_exit_ble_trx_test(const char* param, uint32_t param_len)
{
    app_exit_ble_trx_testmode();
}

static void app_bt_enter_normal_mode_test(const char* param, uint32_t param_len)
{
    app_enter_normal_mode();
}
#endif

#if defined(BT_SOURCE)
POSSIBLY_UNUSED static void app_bt_a2dp_source_pts_enable(const char* param, uint32_t param_len)
{
    besbt_cfg.a2dp_source_pts_test = true;
}

POSSIBLY_UNUSED static bool app_bt_a2dp_source_pts_is_enabled(const char* param, uint32_t param_len)
{
    return besbt_cfg.a2dp_source_pts_test;
}

static void app_bt_source_start_search_test(const char* param, uint32_t param_len)
{
    app_bt_stop_inquiry();
    app_bt_source_search_device();
}

static void app_bt_source_stop_search_test(const char* param, uint32_t param_len)
{
    app_bt_stop_inquiry();
}

static void app_bts_start_search_test(const char* param, uint32_t param_len)
{
    app_bt_stop_inquiry();
#if defined(BT_WATCH_SERVICE_ON) && defined(BT_WATCH_SERVICE_DISTRIBUTE)
    bts_a2dp_source_search();
#endif
}

static void app_bts_stop_search_test(const char* param, uint32_t param_len)
{
    //sbts_a2dp_source_stop_search();
}

static void app_bts_slow_search_test(const char* param, uint32_t param_len)
{
    besbt_cfg.force_normal_search = false;
    besbt_cfg.watch_is_sending_spp = true;
    app_bts_start_search_test(param, param_len);
    besbt_cfg.watch_is_sending_spp = false;
}

static void app_bts_force_normal_search_test(const char* param, uint32_t param_len)
{
    besbt_cfg.watch_is_sending_spp = false;
    besbt_cfg.force_normal_search = true;
    app_bts_start_search_test(param, param_len);
    besbt_cfg.force_normal_search = false;
}

static void app_bts_boost_up_test(const char* param, uint32_t param_len)
{

}

static void app_bt_set_curr_nv_source(const char* param, uint32_t param_len)
{
    uint8_t device_id = app_bt_find_connected_device();
    struct BT_DEVICE_T *curr_device = NULL;
    btif_device_record_t record;

    if (device_id != BT_DEVICE_INVALID_ID)
    {
        curr_device = app_bt_get_device(device_id);
        if (ddbif_find_record(&curr_device->remote, &record) == BT_STS_SUCCESS)
        {
            ddbif_delete_record(&record.bdAddr);
            record.for_bt_source = true;
            ddbif_add_record(&record);
        }
        else
        {
            TRACE(1, "%s no record", __func__);
        }
    }
    else
    {
        TRACE(1, "%s device not found", __func__);
    }
}

static void app_bt_clear_curr_nv_source(const char* param, uint32_t param_len)
{
    uint8_t device_id = app_bt_find_connected_device();
    struct BT_DEVICE_T *curr_device = NULL;
    btif_device_record_t record;

    if (device_id != BT_DEVICE_INVALID_ID)
    {
        curr_device = app_bt_get_device(device_id);
        if (ddbif_find_record(&curr_device->remote, &record) == BT_STS_SUCCESS)
        {
            ddbif_delete_record(&record.bdAddr);
            record.for_bt_source = false;
            ddbif_add_record(&record);
        }
        else
        {
            TRACE(1, "%s no record", __func__);
        }
    }
    else
    {
        TRACE(1, "%s device not found", __func__);
    }
}

static bool a2dp_source_inited = false;
typedef int wal_status_t;
extern "C" void bts_a2dp_source_test_start_cmd(char* param);
extern "C" wal_status_t mock_music_init(void);

static void app_bt_start_a2dp_source_test(const char* param, uint32_t param_len)
{
    if (!a2dp_source_inited)
    {
        //sbts_a2dp_source_test_start_cmd(NULL);
        //mock_music_init();
        //a2dp_source_inited = true;
    }
}

static void app_bt_source_music_test(const char* param, uint32_t param_len)
{
    app_bt_start_a2dp_source_test(NULL, 0);
}

static void app_bt_local_music_test(const char* param, uint32_t param_len)
{
    app_bt_start_a2dp_source_test(NULL, 0);
}

static void app_bt_connect_earbud_link_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_start_a2dp_source_test(NULL, 0);

    bt_source_perform_profile_reconnect(&addr);
}

static void app_bt_connect_earbud_a2dp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_start_a2dp_source_test(NULL, 0);

    bt_source_reconnect_a2dp_profile(&addr);
}

static void app_bt_connect_earbud_avrcp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_avrcp_profile(&addr);
}

static void app_bt_connect_earbud_hfp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_hfp_profile(&addr);
}

static void app_bt_connect_mobile_link_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_a2dp_profile(&addr);

    app_bt_reconnect_hfp_profile(&addr);
}

static void app_bt_connect_mobile_a2dp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_a2dp_profile(&addr);
}

static void app_bt_connect_mobile_avrcp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_avrcp_profile(&addr);
}

static void app_bt_connect_mobile_hfp_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    app_bt_reconnect_hfp_profile(&addr);
}

#if defined(BT_HFP_AG_ROLE)
static void app_bt_ag_create_audio_link_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_create_audio_link(&addr);
}

static void app_bt_ag_disc_audio_link_test(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_disc_audio_link(&addr);
}
#endif

#if defined(mHDT_SUPPORT)
static bool app_bt_scan_tx_rx_rate_from_string(const char* param, uint32_t len, uint8_t *tx_rate, uint8_t *rx_rate)
{
    //"tx:4rx:4"; at least 8 tyte
    if (len < 8)
    {
        TRACE(0, "%s wrong len %d '%s'", __func__, len, param);
        return false;
    }

    int tx_rate_temp;
    int rx_rate_temp;

    TRACE(0,  "%s '%s'", __func__,param);

    if (2 != sscanf(param, "tx:%d rx:%x",&tx_rate_temp, &rx_rate_temp))
    {
        TRACE(0, "%s parse rate failed %s", __func__, param);
        return false;
    }
    *tx_rate = (uint8_t)tx_rate_temp;
    *rx_rate = (uint8_t)rx_rate_temp;

    return true;
}

static void app_bt_source_enter_mhdt_mode_test(const char* param, uint32_t param_len)
{
    uint8 tx_rates;
    uint8 rx_rates;
    if (!app_bt_scan_tx_rx_rate_from_string(param, param_len, &tx_rates, &rx_rates))
    {
        return;
    }
    app_a2dp_source_enter_mhdt_mode(tx_rates, rx_rates);
}

static void app_bt_source_exit_mhdt_mode_test(const char* param, uint32_t param_len)
{
    TRACE(0,  "%s '%s'", __func__,param);
    app_a2dp_source_exit_mhdt_mode();
}

#endif  //mHDT_SUPPORT

static void app_bt_source_disconnect_link(const char* param, uint32_t param_len)
{
    app_bt_source_disconnect_all_connections(true);
}

extern void a2dp_source_pts_send_sbc_packet(void);
static void app_bt_pts_source_send_sbc_packet(const char* param, uint32_t param_len)
{
    a2dp_source_pts_send_sbc_packet();
}

static void app_bt_pts_source_cretae_media_channel_test(const char* param, uint32_t param_len)
{
    btif_pts_source_cretae_media_channel();
}

static void app_bt_pts_source_send_close_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_close_cmd();
}

static void app_bt_pts_source_send_get_configuration_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_get_configuration_cmd();
}

static void app_bt_pts_source_send_reconfigure_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_reconfigure_cmd();
}

static void app_bt_pts_source_send_abort_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_abort_cmd();
}

static void app_bt_pts_source_send_suspend_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_suspend_cmd();
}

static void app_bt_source_reconfig_codec_to_sbc(const char* param, uint32_t param_len)
{
    struct BT_SOURCE_DEVICE_T *source_device = app_bt_source_get_device(BT_SOURCE_DEVICE_ID_1);
    app_bt_a2dp_reconfig_to_sbc(source_device->base_device->a2dp_connected_stream);
}

static void app_bt_pts_source_set_get_all_cap_flag(const char* param, uint32_t param_len)
{
    app_bt_source_set_source_pts_get_all_cap_flag(true);
}

void app_bt_pts_source_set_suspend_err_flag(const char* param, uint32_t param_len)
{
    app_bt_source_set_source_pts_suspend_err_flag(true);
}

static void app_bt_pts_source_set_unknown_cmd_flag(const char* param, uint32_t param_len)
{
    app_bt_source_set_source_pts_unknown_cmd_flag(true);
}

static void app_bt_pts_source_send_start_cmd_test(const char* param, uint32_t param_len)
{
    btif_pts_source_send_start_cmd();
}

#if defined(BT_HID_DEVICE)
static void app_bt_hid_host_connect_dev_req(const char* param, uint32_t param_len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, param_len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    bt_source_reconnect_hid_profile(&addr);
}
#endif // BT_HID_DEVICE

#endif // BT_SOURCE

static void app_bt_delete_device_linkkey_test(const char* param, uint32_t len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    nv_record_ddbrec_delete(&addr);

    TRACE(0, "nv devices after delete:");
    nv_record_all_ddbrec_print();
}

void app_bt_set_linkey_test(const char* param, uint32 len)
{
    btif_device_record_t record = {{{0}},0};
    uint32_t param_key[16] = {0};
    uint32_t param_addr[6] = {0};
    uint8_t linkkey[16] = {0};
    uint8_t address[6] = {0};
    uint8_t cod[3] = {0};
    const char* pos = param + 32;
    memcpy(param_key, param, 32);
    memcpy(param_addr, pos, 12);
    sscanf(param, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", &param_key[0], &param_key[1],
        &param_key[2], &param_key[3],&param_key[4], &param_key[5],&param_key[6], &param_key[7],&param_key[8], &param_key[9],
        &param_key[10], &param_key[11],&param_key[12], &param_key[13],&param_key[14], &param_key[15]);
    sscanf(pos, "%02x%02x%02x%02x%02x%02x",&param_addr[0], &param_addr[1], &param_addr[2], &param_addr[3],
        &param_addr[4], &param_addr[5]);
    for(int i=0;i<16; i++){
        linkkey[i] = param_key[i];
    }
    for(int i=0;i<6; i++){
        address[i] = param_addr[i];
    }
    //DUMP8("%02x ", linkkey,16);
    //DUMP8("%02x ", address,6);

    memcpy(record.bdAddr.address, address, 6);
    memcpy(record.linkKey, linkkey, 16);
    memcpy(record.cod, cod, 3);

    record.keyType = 0x7;
    record.trusted = true;

    nv_record_add(section_usrdata_ddbrecord,(void *)&record);
}


#ifndef BLE_ONLY_ENABLED
static bool app_bt_is_pts_address(void *remote)
{
    return memcmp(remote, g_app_bt_pts_addr.address, sizeof(g_app_bt_pts_addr)) == 0;
}
#endif

static void app_bt_pts_set_address(const char* param, uint32 len)
{
    bt_bdaddr_t addr;

    if (!app_bt_scan_bdaddr_from_string(param, len, &addr))
    {
        return;
    }

    TRACE(2, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            addr.address[0], addr.address[1], addr.address[2],
            addr.address[3], addr.address[4], addr.address[5]);

    g_app_bt_pts_addr = addr;

#ifndef BLE_ONLY_ENABLED
    btif_register_is_pts_address_check_callback(app_bt_is_pts_address);
#endif
}

#ifndef BLE_ONLY_ENABLED
static void app_bt_set_access_mode_test(const char* param, uint32 len)
{
    int mode = 0;

    if (!param || !len)
    {
        return;
    }

    if (1 != sscanf(param, "%d", &mode))
    {
        TRACE(2, "%s invalid param %s", __func__, param);
        return;
    }

    app_bt_set_access_mode((btif_accessible_mode_t)mode);
}

static void app_bt_access_mode_set_test(const char* param, uint32 len)
{
    int mode = 0;

    if (!param || !len)
    {
        return;
    }

    if (1 != sscanf(param, "%d", &mode))
    {
        TRACE(2, "%s invalid param %s", __func__, param);
        return;
    }

    app_bt_set_access_mode((btif_accessible_mode_t)mode);

}

#endif /* BLE_ONLY_ENABLED */

#ifdef AUDIO_LINEIN
static void app_bt_test_linein_start(const char* param, uint32_t param_len)
{
    Audio_device_t audio_device;
    audio_device.audio_device.device_id = BT_DEVICE_ID_1;
    audio_device.audio_device.device_type = AUDIO_TYPE_BT;
    audio_device.aud_id = MAX_RECORD_NUM;
    audio_player_play(BT_STREAM_LINEIN, &audio_device);
}

static void app_bt_test_linein_stop(const char* param, uint32_t param_len)
{
    Audio_device_t audio_device;
    audio_device.audio_device.device_id = BT_DEVICE_ID_1;
    audio_device.audio_device.device_type = AUDIO_TYPE_BT;
    audio_device.aud_id = MAX_RECORD_NUM;
    audio_player_playback_stop(BT_STREAM_LINEIN, &audio_device);
}
#endif

#ifdef APP_USB_A2DP_SOURCE
static void app_a2dp_set_a2dp_source_test(const char* param, uint32_t param_len)
{
    app_a2dp_set_a2dp_source();
}
#endif

static void app_bt_sink_start_sniff_test(const char* param, uint32 len)
{
#ifndef BLE_ONLY_ENABLED
    uint16_t val = 0;
    btif_sniff_info_t sniff_info;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    strdec2uint16hex(param, len, &val);
    sniff_info.minInterval = val;
    sniff_info.maxInterval = val;
    if (val == 0)
    {
        sniff_info.minInterval = 800;
        sniff_info.maxInterval = 800;
    }
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;
    DUMP8("%02x ", &curr_device->remote, 6);
    btif_me_start_sniff(curr_device->acl_conn_hdl, &sniff_info);
#endif
}

static void app_bt_source_start_sniff_test(const char* param, uint32 len)
{
#ifndef BLE_ONLY_ENABLED
    uint16_t val = 0;
    btif_sniff_info_t sniff_info;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(BT_SOURCE_DEVICE_ID_1);
    if (!curr_device)
    {
        return;
    }
    strdec2uint16hex(param, len, &val);
    sniff_info.minInterval = val;
    sniff_info.maxInterval = val;
    if (val == 0)
    {
        sniff_info.minInterval = 800;
        sniff_info.maxInterval = 800;
    }
    sniff_info.attempt     = 3;
    sniff_info.timeout     = 1;
    DUMP8("%02x ", &curr_device->remote, 6);
    btif_me_start_sniff(curr_device->acl_conn_hdl, &sniff_info);
#endif
}

#if defined(BT_SPP_SUPPORT)
#if defined(BES_OTA)
static void app_bt_spp_dev_conn_channel(uint8_t device_id, void* remote, bool succ, uint8_t errcode)
{
    TRACE(0, "app_bt_spp_dev_conn_channel d(%d) succ %d", device_id, succ);
    uint8_t remote_uuid[] = {BT_SDP_SPLIT_16BITS_BE(SC_SERIAL_PORT)};
    osDelay(1000);
    btif_spp_connect(app_bt_get_pts_address(), RFCOMM_CHANNEL_BES_OTA, remote_uuid, sizeof(remote_uuid));
}
#endif // BES_OTA

static void app_bt_pts_spp_conn_channel_test(const char* param, uint32_t param_len)
{
#if defined(BES_OTA)
    btif_me_wait_acl_complete(app_bt_get_pts_address(), app_bt_spp_dev_conn_channel);
#else
    TRACE(0, "not defined BES_OTA");
#endif
}
#endif // BT_SPP_SUPPORT

#ifdef APP_CHIP_BRIDGE_MODULE
static void app_test_uart_relay(const char* param, uint32_t param_len)
{
    char test_str[] = "pong";
    //app_uart_send_data((uint8_t *)test_str, strlen(test_str));
    app_chip_bridge_send_cmd(SET_TEST_MODE_CMD, (uint8_t *)test_str, strlen(test_str));
}
#endif

#if defined(LOCAL_AUDIO_SUPPORT)
static void app_usb_mtp_enter_sys_usb_mtp_download_mode(const char* param, uint32_t param_len)
{
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_USB_MTP_OPERATION);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_USB_MTP_OPERATION);
    app_reset();
}

#if defined LOCAL_AUDIO_TEST_ENABLE
static void app_local_audio_enter_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_enter_local_mode();
}

static void app_local_audio_exit_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_exit_local_mode();
}

static void app_local_audio_request_to_stop_and_exit_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_request_to_stop_and_exit_local_mode();
}

static void app_local_audio_request_to_pause_and_exit_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_request_to_pause_and_exit_local_mode();
}

static void app_local_audio_play_song_test(const char* param, uint32_t param_len)
{
    app_local_audio_play_song();
}

static void app_local_audio_play_next_song_test(const char* param, uint32_t param_len)
{
    app_local_audio_play_next_song();
}

static void app_local_audio_play_prev_song_test(const char* param, uint32_t param_len)
{
    app_local_audio_play_prev_song();
}

static void app_local_audio_pause_song_without_exit_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_pause_song_without_exit_local_mode();
}

static void app_local_audio_stop_song_without_exit_local_mode_test(const char* param, uint32_t param_len)
{
    app_local_audio_stop_song_without_exit_local_mode();
}
#endif
#endif // LOCAL_AUDIO_SUPPORT

const static app_bt_host_cmd_table_t app_bt_host_cmd_table_handle[]=
{
#ifdef APP_CHIP_BRIDGE_MODULE
    {"app_test_uart_relay",     app_test_uart_relay},
#endif
    {"bt_trace_rx_sleep",       app_bt_trace_rx_sleep},
    {"bt_shutdown",             app_bt_shutdown_test},
    {"bt_flush_nv",             app_bt_flush_nv_test},
    {"bt_show_link_key",        app_bt_show_device_linkkey_test},
    {"hci_check_state",         app_bt_hci_print_statistic},
    {"hci_en_cmd_evt_debug",    app_enable_hci_cmd_evt_debug},
    {"hci_de_cmd_evt_debug",    app_disable_hci_cmd_evt_debug},
    {"hci_en_tx_flow_debug",    app_enable_hci_tx_flow_debug},
    {"hci_de_tx_flow_debug",    app_disable_hci_tx_flow_debug},
    {"ota_mode_enter",          app_ota_mode_enter_test},
    {"bt_sink_stop_sniff",      app_bt_sink_stop_sniff_test},
    {"bt_source_stop_sniff",    app_bt_source_stop_sniff_test},
#ifdef APP_SPP_DEMO
    {"bt_client_spp_test",      app_bt_client_spp_test},
    {"bt_service_spp_test",     app_service_spp_test},
    {"bt_spp_send_data_test",   app_spp_data_test},
    {"bt_spp_set_loopback",     app_spp_set_loopback_test},
    {"bt_spp_clear_loopback",   app_spp_clear_loopback_test},
#endif
#ifndef BLE_ONLY_ENABLED
    {"bt_key_click",            app_bt_key_handle_func_click_test},
    {"bt_key_double_click",     app_bt_key_handle_func_doubleclick_test},
    {"bt_key_long_click",       app_bt_key_handle_func_longpress_test},
    {"bt_start_both_scan",      app_bt_start_both_scan_test},
    {"bt_stop_both_scan",       app_bt_stop_both_scan_test},
    {"bt_disc_acl_link",        app_bt_disconnect_acl_link_test},
    {"rfc_register",            app_bt_pts_rfc_register_channel_test},
    {"rfc_close",               app_bt_pts_pts_rfc_close_test},
#endif
#if defined(BT_HFP_TEST_SUPPORT)
    {"hf_create_service_link",  app_bt_pts_create_hf_channel_test},
    {"hf_disc_service_link",    app_bt_pts_hf_disc_service_link_test},
    {"hf_create_audio_link",    app_bt_pts_hf_create_audio_link_test},
    {"hf_disc_audio_link",      app_bt_pts_hf_disc_audio_link_test},
    {"hf_send_key_pressed",     app_bt_pts_hf_send_key_pressed_test},
    {"hf_answer_call",          app_bt_pts_hf_answer_call_test},
    {"hf_hangup_call",          app_bt_pts_hf_hangup_call_test},
    {"hf_dial_number",          app_bt_pts_hf_dial_number_test},
    {"hf_dial_number_memory_index",app_bt_pts_hf_dial_number_memory_index_test},
    {"hf_dial_number_invalid_index",app_bt_pts_hf_dial_number_invalid_memory_index_test},
    {"hf_redial_call",          app_bt_pts_hf_redial_call_test},
    {"hf_release_call",         app_bt_pts_hf_release_active_call_test},
    {"hf_release_active_call_2",app_bt_pts_hf_release_active_call_2_test},
    {"hf_hold_call",            app_bt_pts_hf_hold_active_call_test},
    {"hf_hold_active_call_2",   app_bt_pts_hf_hold_active_call_2_test},
    {"hf_hold_transfer",        app_bt_pts_hf_hold_call_transfer_test},
    {"hf_vr_enable",            app_bt_pts_hf_vr_enable_test},
    {"hf_vr_disable",           app_bt_pts_hf_vr_disable_test},
    {"hf_list_current_calls",   app_bt_pts_hf_list_current_calls_test},
    {"hf_report_mic_volume",    app_bt_pts_hf_report_mic_volume_test},
    {"hf_attach_voice_tag",     app_bt_pts_hf_attach_voice_tag_test},
    {"hf_update_ind_value",     app_bt_pts_hf_update_ind_value_test},
    {"hf_ind_activation",       app_bt_pts_hf_ind_activation_test},
    {"hf_acs_bv_09_i_enable",   app_bt_pts_hf_acs_bv_09_i_set_enable_test},
    {"hf_acs_bv_09_i_disable",  app_bt_pts_hf_acs_bv_09_i_set_disable_test},
    {"hf_acs_bi_13_i_enable",   app_bt_pts_hf_acs_bi_13_i_set_enable_test},
    {"hf_acs_bi_13_i_disable",  app_bt_pts_hf_acs_bi_13_i_set_disable_test},
    {"hf_siri_voice_enable",    app_bt_pts_hf_siri_voice_enable_test},
    {"hf_siri_voice_disable",   app_bt_pts_hf_siri_voice_disable_test},
#if defined(BT_HFP_AG_ROLE)
    {"ag_set_connect_state",    app_bt_pts_hf_ag_set_connectable_state},
    {"ag_create_service_link",  app_bt_pts_create_hf_ag_channel},
    {"ag_create_audio_link",    app_bt_pts_hf_ag_create_audio_link},
    {"ag_disc_audio_link",      app_bt_pts_hf_ag_disc_audio_link},
    {"ag_send_mobile_signal",   app_bt_pts_hf_ag_send_mobile_signal_level},
    {"ag_send_mobile_signal_0", app_bt_pts_hf_ag_send_mobile_signal_level_0},
    {"ag_send_service_status",  app_bt_pts_hf_ag_send_service_status},
    {"ag_send_service_status_0",app_bt_pts_hf_ag_send_service_status_0},
    {"ag_send_callsetup_status",app_bt_pts_hf_ag_send_callsetup_status},
    {"ag_send_callsetup_status_0",app_bt_pts_hf_ag_send_callsetup_status_0},
    {"ag_send_callactive_status",app_bt_pts_hf_ag_send_call_active_status},
    {"ag_send_hangup_call",     app_bt_pts_hf_ag_hangup_call},
    {"ag_send_enable_roam",     app_bt_pts_hf_ag_enable_roam},
    {"ag_send_disable_roam",    app_bt_pts_hf_ag_disable_roam},
    {"ag_send_batt_level",      app_bt_pts_hf_ag_send_mobile_battery_level},
    {"ag_send_full_batt_level", app_bt_pts_hf_ag_send_full_battery_level},
    {"ag_send_batt_level_0",    app_bt_pts_hf_ag_send_battery_level_0},
    {"ag_send_calling_ring",    app_bt_pts_hf_ag_send_calling_ring},
    {"ag_enable_inband_ring",   app_bt_pts_hf_ag_enable_inband_ring_tone},
    {"ag_place_a_call",         app_bt_pts_hf_ag_place_a_call},
    {"ag_ongoing_call",         app_bt_pts_hf_ag_ongoing_call},
    {"ag_ongoing_call_setup",   app_bt_pts_hf_ag_ongoing_call_setup},
    {"ag_clear_dial_num",       app_bt_pts_hf_ag_clear_last_dial_number},
    {"ag_send_ccwa",            app_bt_pts_hf_ag_send_call_waiting_notification},
    {"ag_send_callheld_status", app_bt_pts_hf_ag_send_callheld_status},
    {"ag_send_callheld_status_0",app_bt_pts_hf_ag_send_callheld_status_0},
    {"ag_send_status_3_0_4_1",  app_bt_pts_hf_ag_send_status_3_0_4_1},
    {"ag_set_pts_enable",       app_bt_pts_hf_ag_set_pts_enable},
    {"ag_answer_incoming_call", app_bt_pts_hf_ag_answer_incoming_call},
    {"ag_set_pts_ecs_01",       app_bt_pts_hf_ag_set_pts_ecs_01},
    {"ag_set_pts_ecs_02",       app_bt_pts_hf_ag_set_pts_ecs_02},
    {"ag_set_pts_ecc",          app_bt_pts_hf_ag_set_pts_ecc},
#endif /* BT_HFP_AG_ROLE */
#endif /* BT_HFP_TEST_SUPPORT */
#if defined(BT_A2DP_TEST_SUPPORT)
    {"av_create_channel",       app_bt_pts_create_av_channel_test},
    {"av_disc_channel",         app_bt_pts_av_disc_channel_test},
    {"av_en_tone_interrupt",    app_bt_enable_tone_intrrupt_a2dp_test},
    {"av_de_tone_interrupt",    app_bt_disable_tone_intrrupt_a2dp_test},
    {"av_en_delay_prompt",      app_bt_enable_a2dp_delay_prompt_test},
    {"av_de_delay_prompt",      app_bt_disable_a2dp_delay_prompt_test},
    {"av_de_aac_codec",         app_bt_disable_a2dp_aac_codec_test},
    {"av_de_vnd_codec",         app_bt_disable_a2dp_vendor_codec_test},
    {"av_create_media_channel", app_bt_pts_av_create_media_channel_test},
    {"av_close_channel",        app_bt_pts_av_close_channel_test},
    {"av_send_getconf",         app_bt_pts_av_send_getconf_test},
    {"av_send_reconf",          app_bt_pts_av_send_reconf_test},
    {"av_send_open",            app_bt_pts_av_send_open_test},
    {"av_send_start",           app_bt_pts_av_send_start_test},
    {"av_send_discover",        btif_pts_av_send_discover},
    {"av_send_getcap",          btif_pts_av_send_getcap},
    {"av_send_setconf",         btif_pts_av_send_setconf},
    {"av_send_getallcap",       btif_pts_av_send_getallcap},
    {"av_send_suspend",         btif_pts_av_send_suspend},
    {"av_send_abort",           btif_pts_av_send_abort},
    {"av_send_security_control",btif_pts_av_send_security_control},
#endif /* BT_A2DP_TEST_SUPPORT */
#if defined(BT_AVRCP_TEST_SUPPORT) || defined(BT_HFP_TEST_SUPPORT)
#ifndef BTH_SUBSYS_ONLY
    {"bt_sink_volume_up",       app_bt_audio_local_volume_up_test},
    {"bt_sink_volume_down",     app_bt_audio_volume_down_test},

#endif
#endif
#ifdef BQB_PROFILE_TEST
    //A2DP/SNK/AVP/BI-01-C
    {"av_INVALID_OBJECT_TYPE", app_bt_pts_reject_invalid_object_type_test},
    //A2DP/SNK/AVP/BI-02-C
    {"av_INVALID_CHANNELS", app_bt_pts_reject_invalid_channels_test},
    //A2DP/SNK/AVP/BI-03-C
    {"av_INVALID_SAMPLING_FREQUENCY", app_bt_pts_reject_invalid_sampling_freq_test},
    //A2DP/SNK/AVP/BI-04-C
    {"av_INVALID_DRC", app_bt_pts_reject_invalid_drc_test},
    //A2DP/SNK/AVP/BI-06-C
    {"av_NOT_SUPPORTED_OBJECT_TYPE", app_bt_pts_reject_not_supp_obj_type_test},
    //A2DP/SNK/AVP/BI-07-C
    {"av_NOT_SUPPORTED_CHANNELS", app_bt_pts_reject_not_supp_channel_test},
    //A2DP/SNK/AVP/BI-08-C
    {"av_NOT_SUPPORTED_SAMPLING_FREQUENCY", app_bt_pts_reject_not_supp_sampling_freq_test},
    //A2DP/SNK/AVP/BI-09-C
    {"av_NOT_SUPPORTED_DRC", app_bt_pts_reject_not_supp_drc_test},
    //A2DP/SNK/AVP/BI-10-C
    {"av_INVALID_CODEC_TYPE", app_bt_pts_reject_invalid_codec_type_test},
    //A2DP/SNK/AVP/BI-11-C
    {"av_INVALID_CHANNEL_MODE", app_bt_pts_reject_invalid_channel_mode_test},
    //A2DP/SNK/AVP/BI-12-C
    {"av_INVALID_SUBBANDS", app_bt_pts_reject_invalid_subbands_test},
    //A2DP/SNK/AVP/BI-13-C
    {"av_INVALID_ALLOCATION_METHOD", app_bt_pts_reject_invalid_allocation_method_test},
    //A2DP/SNK/AVP/BI-14-C
    {"av_INVALID_MINIMUM_BITPOOL_VALUE", app_bt_pts_reject_invalid_min_bitpoll_value_test},
    //A2DP/SNK/AVP/BI-15-C
    {"av_INVALID_MAXIMUM_BITPOOL_VALUE", app_bt_pts_reject_invalid_max_bitpoll_value_test},
    //A2DP/SNK/AVP/BI-16-C
    {"av_INVALID_BLOCK_LENGTH", app_bt_pts_reject_invalid_block_length_test},
    //A2DP/SNK/AVP/BI-17-C
    {"av_INVALID_CP_TYPE", app_bt_pts_reject_invalid_cp_type_test},
    //A2DP/SNK/AVP/BI-18-C
    {"av_INVALID_CP_FORMAT", app_bt_pts_reject_invalid_cp_format_test},
    //A2DP/SNK/AVP/BI-20-C
    {"av_NOT_SUPPORTED_CODEC_TYPE", app_bt_pts_reject_not_supp_codec_type_test},
#endif

#if defined(BT_USE_COHEAP_ALLOC)
    {"coheap_dump",             app_bt_coheap_statistics_dump},
    {"coheap_enable_debug",     app_bt_coheap_enable_debug},
    {"coheap_disable_debug",    app_bt_coheap_disable_debug},
#endif

#if defined(NORMAL_TEST_MODE_SWITCH)
    {"enter_signal_testmode",   app_bt_enter_signal_testmode_test},
    {"exit_signal_testmode",    app_bt_exit_signal_testmode_test},
    
    {"reboot_and_enter_nosignal_testmode", app_bt_reboot_and_enter_nosignal_test},
    {"reboot_and_enter_signal_testmode",   app_bt_reboot_and_enter_signal_test},

    {"enter_nosignal_tx_testmode",app_bt_enter_nosignal_tx_test},
    {"enter_nosignal_rx_testmode",app_bt_enter_nosignal_rx_test},
    {"exit_nosignal_trx_testmode",app_bt_exit_nosignal_trx_test},
    {"enter_ble_tx_v1",         app_bt_enter_ble_tx_v1_test},
    {"enter_ble_rx_v1",         app_bt_enter_ble_rx_v1_test},
    {"enter_ble_tx_v2",         app_bt_enter_ble_tx_v2_test},
    {"enter_ble_rx_v2",         app_bt_enter_ble_rx_v2_test},
    {"enter_ble_tx_v3",         app_bt_enter_ble_tx_v3_test},
    {"enter_ble_rx_v3",         app_bt_enter_ble_rx_v3_test},
    {"enter_ble_tx_v4",         app_bt_enter_ble_tx_v4_test},
    {"exit_ble_trx",            app_bt_exit_ble_trx_test},
    {"enter_normal_mode",       app_bt_enter_normal_mode_test},
#endif

#if defined(BT_SOURCE)
    {"bt_start_search",         app_bt_source_start_search_test},
    {"bt_stop_search",          app_bt_source_stop_search_test},
    {"bts_start_search",        app_bts_start_search_test},
    {"bts_stop_search",         app_bts_stop_search_test},
    {"bts_slow_search",         app_bts_slow_search_test},
    {"bts_normal_search",       app_bts_force_normal_search_test},

    {"bts_boost_up_test",       app_bts_boost_up_test},
    {"bt_set_nv_source",        app_bt_set_curr_nv_source},
    {"bt_clear_nv_source",      app_bt_clear_curr_nv_source},
    {"bt_source_music",         app_bt_source_music_test},
    {"bt_local_music",          app_bt_local_music_test},

    {"source_disc_link",        app_bt_source_disconnect_link},
    {"reconfig_to_sbc",         app_bt_source_reconfig_codec_to_sbc},
    {"source_send_sbc_pkt",     app_bt_pts_source_send_sbc_packet},
    {"source_create_media_chnl",app_bt_pts_source_cretae_media_channel_test},
    {"source_send_close_cmd",   app_bt_pts_source_send_close_cmd_test},
    {"source_send_get_config_cmd",app_bt_pts_source_send_get_configuration_cmd_test},
    {"source_send_reconfigure_cmd",app_bt_pts_source_send_reconfigure_cmd_test},
    {"source_send_abort_cmd",   app_bt_pts_source_send_abort_cmd_test},
    {"source_send_suspend_cmd", app_bt_pts_source_send_suspend_cmd_test},
    {"source_set_get_all_cap_flag",app_bt_pts_source_set_get_all_cap_flag},
    {"source_set_suspend_err_flag",app_bt_pts_source_set_suspend_err_flag},
    {"source_set_unknown_cmd_flag",app_bt_pts_source_set_unknown_cmd_flag},
    {"source_send_start_cmd",   app_bt_pts_source_send_start_cmd_test},

    {"bt_conn_earbud_link",     app_bt_connect_earbud_link_test},   // bt_conn_earbud_link|af:19:b0:bb:22:74
    {"bt_conn_earbud_a2dp",     app_bt_connect_earbud_a2dp_test},   // bt_conn_earbud_a2dp|af:19:b0:bb:22:74
    {"bt_conn_earbud_avrcp",    app_bt_connect_earbud_avrcp_test},
    {"bt_conn_earbud_hfp",      app_bt_connect_earbud_hfp_test},
    {"bt_conn_mobile_link",     app_bt_connect_mobile_link_test},
    {"bt_conn_mobile_a2dp",     app_bt_connect_mobile_a2dp_test},
    {"bt_conn_mobile_avrcp",    app_bt_connect_mobile_avrcp_test},
    {"bt_conn_mobile_hfp",      app_bt_connect_mobile_hfp_test},
#if defined(BT_HFP_AG_ROLE)
    {"ag_create_audio_link",    app_bt_ag_create_audio_link_test},
    {"ag_disc_audio_link",      app_bt_ag_disc_audio_link_test},
#endif
#if defined(mHDT_SUPPORT)
    {"enter_mhdt_mode",         app_bt_source_enter_mhdt_mode_test},
    {"exit_mhdt_mode",          app_bt_source_exit_mhdt_mode_test},
#endif
#if defined(BT_HID_DEVICE)
    {"hid_host_conn_device",    app_bt_hid_host_connect_dev_req},
#endif // BT_HID_DEVICE
#endif // BT_SOURCE

#ifdef AUDIO_LINEIN
    {"linein_start",            app_bt_test_linein_start},
    {"linein_stop",             app_bt_test_linein_stop},
#endif

#ifdef APP_USB_A2DP_SOURCE
    {"reg_usb_btaudio",         app_a2dp_set_a2dp_source_test},
#endif
#if defined(LOCAL_AUDIO_SUPPORT)
    //enter usb m55 usb mtp download mode
    {"enter_sys_usb_download_mode", app_usb_mtp_enter_sys_usb_mtp_download_mode},
#if defined LOCAL_AUDIO_TEST_ENABLE
    {"local_audio_enter_local_mode",        app_local_audio_enter_local_mode_test},
    {"local_audio_exit_local_mode",         app_local_audio_exit_local_mode_test},
    {"local_audio_pause_and_exit",          app_local_audio_request_to_pause_and_exit_local_mode_test},
    {"local_audio_stop_and_exit",           app_local_audio_request_to_stop_and_exit_local_mode_test},
    {"local_audio_pause_and_no_exit",       app_local_audio_pause_song_without_exit_local_mode_test},
    {"local_audio_stop_and_no_exit",        app_local_audio_stop_song_without_exit_local_mode_test},
    {"local_audio_play_song",               app_local_audio_play_song_test},
    {"local_audio_play_next",               app_local_audio_play_next_song_test},
    {"local_audio_play_prev",               app_local_audio_play_prev_song_test},
#endif // LOCAL_AUDIO_TEST_ENABLE
#endif // LOCAL_AUDIO_SUPPORT

    ///param cmd
#ifndef BLE_ONLY_ENABLED
    {"bt_set_access_mode",      app_bt_set_access_mode_test},       // bt_set_access_mode|2
    {"bt_access_mode_set",      app_bt_access_mode_set_test},
#endif
    {"bt_sink_start_sniff",     app_bt_sink_start_sniff_test},
    {"bt_source_start_sniff",   app_bt_source_start_sniff_test},
    {"bt_set_pts_address",      app_bt_pts_set_address},            // bt_set_pts_address|ed:b6:f4:dc:1b:00
    {"bt_delete_link_key",      app_bt_delete_device_linkkey_test}, // bt_delete_link_key|ed:b6:f4:dc:1b:00
    {"bt_set_linkey",           app_bt_set_linkey_test},   //bt_set_linkey|e7ab95d59015ae53fb2d800d340e23412c1d7297d658  linkeydevicebtaddr
#if defined(BT_SPP_SUPPORT)
    {"spp_conn_test",           app_bt_pts_spp_conn_channel_test},
#endif // BT_SPP_SUPPORT
};

unsigned int app_bt_host_cmd_callback(unsigned char *buf, unsigned int length)
{
    // Check len
    TRACE(0, "[%s] cmd: %s", __func__, buf);
    uint8_t cmd_number = 0;
    int cmd_count = 0;
    char *para_addr=NULL;
    unsigned int para_len=0;
    const app_bt_host_cmd_table_t* cmd_table;

    for (int tab_num=0; tab_num<APP_BT_CMD_TABLE_MAX; tab_num++)
    {
        if (!app_bt_cmd_env.cmd_table[tab_num].table_list)
        {
            continue;
        }

        cmd_number = app_bt_cmd_env.cmd_table[tab_num].cmd_number;
        cmd_table  = app_bt_cmd_env.cmd_table[tab_num].table_list;
        for (cmd_count = 0; cmd_count < cmd_number; cmd_count++)
        {
            if ((strncmp((char*)buf, cmd_table[cmd_count].string, strlen(cmd_table[cmd_count].string)) == 0) ||
                    strstr(cmd_table[cmd_count].string, (char*)buf))
            {
                para_addr = strstr((char*)buf, "|");
                if(para_addr != NULL)
                {
                    para_addr++;
                    para_len = length - (para_addr - (char *)buf);
                }

                cmd_table[cmd_count].cmd_function(para_addr, para_len);
                break;
            }
        }

        if (cmd_count < cmd_number)
        {
            break;
        }
    }

    if (cmd_count >= cmd_number)
    {
        TRACE(0,"bt host not found,cmd=%s", buf);
    }

    return 0;
}
#endif // BT_HOST_TEST_CMD_ENABLE

void app_bt_host_cmd_init(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0, "bt host cmd init");
    app_trace_rx_register("BT_HOST", app_bt_host_cmd_callback);
    app_bt_host_add_cmd_table(sizeof(app_bt_host_cmd_table_handle)/sizeof(app_bt_host_cmd_table_handle[0]),
        app_bt_host_cmd_table_handle);
#else
    TRACE(0, "bt host cmd init not open APP_TRACE_RX_ENABLE");
#endif

#ifdef BLE_HOST_SUPPORT
    app_ble_test_cmd_init();
#endif

    return;
}

void app_bt_host_cmd_deinit(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0, "bt host cmd deinit");
    app_trace_rx_deregister("BT_HOST");
#else
    TRACE(0, "bt host cmd deinit not open APP_TRACE_RX_ENABLE");
#endif

#ifdef BLE_HOST_SUPPORT
    app_ble_test_cmd_deinit();
#endif
}

bool app_bt_host_add_cmd_table(uint8_t cmd_number, const app_bt_host_cmd_table_t *cmd_table)
{
#ifdef APP_TRACE_RX_ENABLE
    for (int i=0; i<APP_BT_CMD_TABLE_MAX; ++i)
    {
        if (!app_bt_cmd_env.cmd_table[i].table_list)
        {
            app_bt_cmd_env.cmd_table[i].cmd_number = cmd_number;
            app_bt_cmd_env.cmd_table[i].table_list = cmd_table;
            return true;
        }
    }

    TRACE(0, "bt host add add cmd table fail! table=%p", cmd_table);
    return false;
#else
    TRACE(0, "bt host cmd add table, not open APP_TRACE_RX_ENABLE");
    return false;
#endif
}

bool app_bt_host_delete_cmd_table(const app_bt_host_cmd_table_t* cmd_table)
{
#ifdef APP_TRACE_RX_ENABLE
    for (int i=0; i<APP_BT_CMD_TABLE_MAX; ++i)
    {
        if (app_bt_cmd_env.cmd_table[i].table_list == cmd_table)
        {
            app_bt_cmd_env.cmd_table[i].cmd_number = 0;
            app_bt_cmd_env.cmd_table[i].table_list = NULL;
            return true;
        }
    }

    TRACE(0, "bt host delete cmd table fail! table=%p", cmd_table);
    return false;
#else
    TRACE(0, "bt host cmd delete table, not open APP_TRACE_RX_ENABLE");
    return false;
#endif
}


