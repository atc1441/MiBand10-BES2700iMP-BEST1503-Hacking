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
#ifdef BT_HID_DEVICE
#undef MOUDLE
#define MOUDLE APP_BT
#include <stdio.h>
#include "cmsis_os.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "app_audio.h"
#include "app_bt_cmd.h"
#include "app_battery.h"
#include "app_trace_rx.h"
#include "app_status_ind.h"
#include "bluetooth.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "besbt.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "bt_if.h"
#include "app_bt_func.h"
#include "besbt_cfg.h"
#include "me_api.h"
#include "app_bt_hid.h"
#include "hid_api.h"
#include "bt_sys_config.h"

#ifdef IBRT
#include "app_tws_besaud.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_internal.h"
#include "app_ibrt_keyboard.h"
#endif

#ifdef BT_SOURCE
#include "bt_source.h"
#endif // BT_SOURCE

#define BT_HID_CAPTURE_WAIT_SEND_MS 2000
static void app_bt_hid_send_capture_handler(const void *param);
osTimerDef (BT_HID_CAPTURE_WAIT_TIMER0, app_bt_hid_send_capture_handler);
#if BT_DEVICE_NUM > 1
osTimerDef (BT_HID_CAPTURE_WAIT_TIMER1, app_bt_hid_send_capture_handler);
#endif
#if BT_DEVICE_NUM > 2
osTimerDef (BT_HID_CAPTURE_WAIT_TIMER2, app_bt_hid_send_capture_handler);
#endif

#define BT_HID_DISC_CHANNEL_WAIT_MS (30*1000)
static void app_bt_hid_disc_chan_timer_handler(const void *param);
osTimerDef (BT_HID_DISC_CHAN_TIMER0, app_bt_hid_disc_chan_timer_handler);
#if BT_DEVICE_NUM > 1
osTimerDef (BT_HID_DISC_CHAN_TIMER1, app_bt_hid_disc_chan_timer_handler);
#endif
#if BT_DEVICE_NUM > 2
osTimerDef (BT_HID_DISC_CHAN_TIMER2, app_bt_hid_disc_chan_timer_handler);
#endif

static void app_bt_hid_start_disc_wait_timer(uint8_t device_id);

static void _bt_hid_send_capture(uint32_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

#if 0
    btif_hid_keyboard_input_report(curr_device->hid_channel, HID_MOD_KEY_NULL, HID_KEY_CODE_ENTER);
    btif_hid_keyboard_input_report(curr_device->hid_channel, HID_MOD_KEY_NULL, HID_KEY_CODE_NULL);
    btif_hid_keyboard_send_ctrl_key(curr_device->hid_channel, HID_CTRL_KEY_VOLUME_INC);
    btif_hid_keyboard_send_ctrl_key(curr_device->hid_channel, HID_CTRL_KEY_NULL);
#else
    static uint8_t vol_ctrl_key = HID_CTRL_KEY_VOLUME_INC;
    vol_ctrl_key = (vol_ctrl_key == HID_CTRL_KEY_VOLUME_INC) ? HID_CTRL_KEY_VOLUME_DEC : HID_CTRL_KEY_VOLUME_INC;
    btif_hid_keyboard_send_ctrl_key(curr_device->hid_channel, vol_ctrl_key);
    btif_hid_keyboard_send_ctrl_key(curr_device->hid_channel, HID_CTRL_KEY_NULL);
#endif

    if (app_bt_manager.config.hid_capture_non_invade_mode)
    {
        app_bt_hid_start_disc_wait_timer((uint8_t)device_id);
    }
}

static void app_bt_hid_send_capture_handler(const void *param)
{
    uint8_t device_id = (uint8_t)(uintptr_t)param;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device && curr_device->hid_conn_flag)
    {
        TRACE(2, "(d%x) %s", device_id, __func__);
        app_bt_start_custom_function_in_bt_thread((uint32_t)device_id, (uint32_t)NULL, (uint32_t)_bt_hid_send_capture);
    }
    else
    {
        TRACE(2, "(d%x) %s failed", device_id, __func__);
    }
}

static void app_bt_hid_start_capture_wait_timer(uint8_t device_id, hid_channel_t chan)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device->capture_wait_timer_id)
    {
        TRACE(1, "%s timer is NULL", __func__);
        return;
    }

    TRACE(3, "%s channel %p %p", __func__, curr_device->hid_channel, chan);

    osTimerStop(curr_device->capture_wait_timer_id);

    osTimerStart(curr_device->capture_wait_timer_id, BT_HID_CAPTURE_WAIT_SEND_MS);
}

static int app_bt_hid_device_callback(const bt_bdaddr_t *remote, bt_hid_event_t event, bt_hid_callback_param_t param)
{
    struct BT_DEVICE_T *curr_device = NULL;
    uint8_t device_id = param.opened->device_id;
    uint8_t error_code = param.opened->error_code;
    hid_channel_t chan = (hid_channel_t)param.opened->channel;

    if (device_id == BT_DEVICE_INVALID_ID && event == BT_HID_EVENT_CLOSED)
    {
        // hid profile is closed due to acl created fail
        TRACE(0, "::BT_HID_EVENT_CLOSED acl created error %x", param.closed->error_code);
        return 0;
    }

    curr_device = app_bt_get_device(device_id);
    ASSERT(device_id < BT_DEVICE_NUM && curr_device->hid_channel == chan, "hid device channel must match");

    TRACE(0, "(d%x) %s channel %p event %04x errno %02x %02x:%02x:***:%02x",
                device_id, __func__, chan, event, error_code,
                remote->address[0], remote->address[1], remote->address[5]);

    switch (event)
    {
        case BT_HID_EVENT_OPENED:
        {
            curr_device->hid_conn_flag = true;
            if (curr_device->wait_send_capture_key)
            {
                app_bt_hid_start_capture_wait_timer(device_id, chan);
                curr_device->wait_send_capture_key = false;
            }
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_OPENED, param.opened);
#if defined(IBRT)
            app_tws_ibrt_profile_callback(device_id, BTIF_APP_HID_PROFILE_ID, (void *)BT_EVENT_HID_OPENED, chan, &curr_device->remote);
#endif
        }
        break;
        case BT_HID_EVENT_CLOSED:
        {
            curr_device->hid_conn_flag = false;
            curr_device->wait_send_capture_key = false;
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_CLOSED, param.closed);
#if defined(IBRT)
            app_tws_ibrt_profile_callback(device_id, BTIF_APP_HID_PROFILE_ID, (void *)BT_EVENT_HID_CLOSED, chan, &curr_device->remote);
#endif
        }
        break;
        case BT_HID_EVENT_SENSOR_STATE_CHANGED:
        {
            uint8_t sensor_state = param.changed->sensor_state;
            uint8_t sensor_power_state = (sensor_state & 0x02) ? true : false;
            uint8_t sensor_reporting_state = (sensor_state & 0x01) ? true : false;
            uint8_t sensor_interval = 10 + ((sensor_state & 0xfc) >> 2);
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_SENSOR_STATE_CHANGED, param.changed);
            TRACE(0, "(d%x) %s sesnor power %d reporting %d interval %d ms", device_id, __func__,
                sensor_power_state, sensor_reporting_state, sensor_interval);
        }
        break;
        case BT_HID_EVENT_TXDONE:
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_TXDONE, param.txdone);
        break;
        case BT_HID_EVENT_RCV_CTL_DATA:
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_RCV_CTL_DATA, param.ctl_data);
        break;
        case BT_HID_EVENT_RCV_INT_DATA:
            btif_report_bt_event(&curr_device->remote, BT_EVENT_HID_RCV_INT_DATA, param.int_data);
        break;
    default:
        break;
    }

    return 0;
}

#if defined(BT_SOURCE)
static int app_bt_hid_host_callback(const bt_bdaddr_t *remote, bt_hid_event_t event, bt_hid_callback_param_t param)
{
    struct BT_SOURCE_DEVICE_T* source_dev = NULL;

    uint8_t device_id = param.opened->device_id;
    uint8_t error_code = param.opened->error_code;
    hid_channel_t chan = (hid_channel_t)param.opened->channel;

    if (device_id == BT_DEVICE_INVALID_ID && event == BT_HID_EVENT_CLOSED)
    {
        // hid profile is closed due to acl created fail
        TRACE(0, "::BT_HID_EVENT_CLOSED acl created error %x", param.closed->error_code);
        return 0;
    }

    source_dev = app_bt_source_get_device(device_id);

    ASSERT(source_dev->base_device->hid_channel == chan, "hid device channel must match");

    TRACE(0, "(d%x) %s channel %p event 0x%04x errno %02x %02x:%02x:***:%02x",
                device_id, __func__, chan, event, error_code,
                remote->address[0], remote->address[1], remote->address[5]);

    switch (event)
    {
        case BT_HID_EVENT_OPENED:
        {
            source_dev->base_device->hid_conn_flag = true;
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_OPENED, param.opened);
        }
        break;
        case BT_HID_EVENT_CLOSED:
        {
            source_dev->base_device->hid_conn_flag = false;
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_CLOSED, param.closed);
        }
        break;
        case BT_HID_EVENT_TXDONE:
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_TXDONE, param.txdone);
        break;
        case BT_HID_EVENT_RCV_CTL_DATA:
            TRACE(0, "BT_HID_EVENT_RCV_CTL_DATA");
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_RCV_CTL_DATA, param.ctl_data);
        break;
        case BT_HID_EVENT_RCV_INT_DATA:
            TRACE(0, "BT_HID_EVENT_RCV_INT_DATA");
            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_RCV_INT_DATA, param.int_data);
        break;
        case BT_HID_EVENT_RCV_SDP_DATA:
            TRACE(0, "BT_HID_EVENT_RCV_SDP_DATA");

            btif_report_bt_event(&source_dev->base_device->remote, BT_EVENT_HID_SDP_REC_DATA, param.sdp_data);
        break;
            break;
    default:
        break;
    }

    return 0;
}
#endif // BT_SOURCE

static bool bt_hid_initialized = false;

#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_LEN (52+26)
#define HID_REPORT_MODE_MOUSE_DESCRIPTOR_DATA \
    HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
    HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_MOUSE), \
    HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_APPLICATION), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECLK_INPUT_REPORT_ID), \
        HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_POINTER), \
        HID_MAIN_ITEM_COLLECTION(HID_COLLECTION_PHYSICAL), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_GLOBAL_ITEM_REPORT_COUNT(3), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_BUTTONS), \
            HID_LOCAL_ITEM_USAGE_MIN(HID_USAGE_ID_BUTTON_1), \
            HID_LOCAL_ITEM_USAGE_MAX(HID_USAGE_ID_BUTTON_3), \
            HID_GLOBAL_ITEM_LOGICAL_MIN(0), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(1), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_ABS),      /* button report */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(5), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
                HID_MAIN_ITEM_INPUT(HID_CNST),                      /* button report padding */ \
            HID_GLOBAL_ITEM_REPORT_SIZE(8), \
            HID_GLOBAL_ITEM_REPORT_COUNT(2), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_GENERIC_DESKTOP), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_X), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_Y), \
            HID_GLOBAL_ITEM_LOGICAL_MIN((uint8_t)(int8_t)(-127)), \
            HID_GLOBAL_ITEM_LOGICAL_MAX(127), \
                HID_MAIN_ITEM_INPUT(HID_DATA|HID_VAR|HID_REL), \
        HID_MAIN_ITEM_END_COLLECTION(), \
        HID_GLOBAL_ITEM_REPORT_ID(HID_MOUSECTL_INPUT_REPORT_ID), \
            HID_GLOBAL_ITEM_USAGE_PAGE(HID_USAGE_PAGE_CONSUMER), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_INC), \
            HID_LOCAL_ITEM_USAGE_ID(HID_USAGE_ID_VOLUME_DEC), \
            HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_BACK), \
            HID_LOCAL_ITEM_USAGE_ID_2B(HID_USAGE_ID_HOME), \
            HID_GLOBAL_ITEM_REPORT_COUNT(4), \
            HID_GLOBAL_ITEM_REPORT_SIZE(1), \
            HID_MAIN_ITEM_INPUT(HID_VAR), \
            HID_GLOBAL_ITEM_REPORT_COUNT(1), \
            HID_GLOBAL_ITEM_REPORT_SIZE(4), \
            HID_MAIN_ITEM_INPUT(HID_CNST), \
        HID_MAIN_ITEM_END_COLLECTION()

#define HID_DEVICE_SUBCLASS CFG_COD_MINOR_PERIPH_POINTING_DEVICE
#define HID_DESCRIPTOR_LEN  HID_REPORT_MODE_MOUSE_DESCRIPTOR_LEN
#define HID_DESCRIPTOR_DATA HID_REPORT_MODE_MOUSE_DESCRIPTOR_DATA

static const uint8_t g_app_bt_hid_device_custom_descriptor_list[] = {
    SDP_DE_DESEQ_8BITSIZE_H2_D(HID_DESCRIPTOR_LEN+6), // attr value head
        SDP_DE_DESEQ_8BITSIZE_H2_D(HID_DESCRIPTOR_LEN+4), // attr value head
            SDP_DE_UINT_H1_D1,
                HID_DESCRIPTOR_TYPE, // Report Descriptor
            SDP_DE_TEXTSTR_8BITSIZE_H2_D(HID_DESCRIPTOR_LEN), // Text String, n octet Report Descripotr
                HID_DESCRIPTOR_DATA,
};

POSSIBLY_UNUSED static const bt_hid_custom_descriptor_t g_app_bt_hid_custom_descriptor = {
    HID_DEVICE_SUBCLASS,
    sizeof(g_app_bt_hid_device_custom_descriptor_list),
    g_app_bt_hid_device_custom_descriptor_list
};

static const bt_hid_custom_descriptor_t *app_bt_hid_get_custom_descriptor(void)
{
#if 0
    return &g_app_bt_hid_custom_descriptor;
#else
    return NULL; // default no custom descriptor
#endif
}

bt_status_t app_bt_hid_profile_connect(const bt_bdaddr_t *remote, int capture)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(remote);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device == NULL || !curr_device->acl_is_connected)
    {
        TRACE(3, "(d%x) %s acl is not connected %d", device_id, __func__, capture);
        return BT_STS_FAILED;
    }

    TRACE(8, "(d%x) %s %d %02x:%02x:%02x:%02x:%02x:%02x", device_id, __func__, capture,
            curr_device->remote.address[0], curr_device->remote.address[1], curr_device->remote.address[2],
            curr_device->remote.address[3], curr_device->remote.address[4], curr_device->remote.address[5]);

    curr_device->wait_send_capture_key = capture ? true : false;

    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)&curr_device->remote, (uint32_t)NULL, (uint32_t)btif_hid_connect);

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_profile_connect_v1(const bt_bdaddr_t *remote)
{
    TRACE(8, "%s %02x:%02x:%02x:%02x:%02x:%02x", __func__,
            remote->address[0], remote->address[1], remote->address[2],
            remote->address[3], remote->address[4], remote->address[5]);

    struct bt_defer_param_t param_a = bt_alloc_param_size(remote, 6);

    bt_defer_call_func_1(btif_hid_connect, param_a);

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_profile_disconnect(const bt_bdaddr_t *remote)
{
    uint8_t device_id = app_bt_get_connected_device_id_byaddr(remote);
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    TRACE(1, "(d%x) %s", device_id, __func__);

    if (curr_device)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device->hid_channel, (uint32_t)NULL, (uint32_t)btif_hid_disconnect);
    }

    return BT_STS_SUCCESS;
}

int app_bt_nvrecord_get_latest_device_addr(bt_bdaddr_t *addr)
{
    btif_device_record_t record;
    int found_addr_count = 0;
    int paired_dev_count = nv_record_get_paired_dev_count();

    if (paired_dev_count > 0 && BT_STS_SUCCESS == nv_record_enum_dev_records(0, &record))
    {
        *addr = record.bdAddr;
        found_addr_count = 1;
    }

    return found_addr_count;
}

bt_status_t app_bt_hid_virtual_cable_unplug(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);

    if (!curr_device || !curr_device->hid_conn_flag)
    {
        TRACE(3, "(d%x) %s hid not connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

    app_bt_start_custom_function_in_bt_thread((uint32_t)curr_device->hid_channel, (uint32_t)NULL, (uint32_t)btif_hid_send_virtual_cable_unplug);

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_capture_process(const bt_bdaddr_t *bd_addr)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(bd_addr);

    if (!curr_device || !curr_device->hid_conn_flag)
    {
        TRACE(3, "(d%x) %s hid not connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

    curr_device->wait_send_capture_key = false;

#if defined(IBRT_UI)
    if (tws_besaud_is_connected())
    {
        uint8_t ibrt_role = app_tws_get_ibrt_role(&curr_device->remote);
        TRACE(3, "(d%x) %s role %d", curr_device->device_id, __func__, ibrt_role);

        if (IBRT_MASTER == ibrt_role)
        {
            app_bt_hid_send_capture_handler((void *)(uintptr_t)curr_device->device_id);
        }
        else if (IBRT_SLAVE == ibrt_role)
        {
            app_ibrt_if_start_user_action_v2(curr_device->device_id, IBRT_ACTION_HID_SEND_CAPTURE, 0, 0);
        }
    }
    else
    {
        app_bt_hid_send_capture_handler((void *)(uintptr_t)curr_device->device_id);
    }
#else
    app_bt_hid_send_capture_handler((void *)(uintptr_t)curr_device->device_id);
#endif

    return BT_STS_SUCCESS;
}

static void app_bt_hid_open_shutter_mode(uint8_t device_id, bool capture)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device->acl_is_connected)
    {
        TRACE(3, "(d%x) %s acl is not connected %d", device_id, __func__, capture);
        return;
    }

    if (!curr_device->hid_conn_flag)
    {
#if defined(IBRT_UI)
        app_ibrt_internal_profile_connect(device_id, APP_IBRT_HID_PROFILE_ID, capture);
#else
        app_bt_hid_profile_connect(&curr_device->remote, capture);
#endif
        return;
    }

    if (capture)
    {
        app_bt_hid_capture_process(&curr_device->remote);
    }
}

static void app_bt_hid_close_shutter_mode(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    TRACE(3, "(d%x) %s hid_conn_flag %d", device_id, __func__, curr_device->hid_conn_flag);

    if (curr_device->hid_conn_flag)
    {
#if defined(IBRT_UI)
        app_ibrt_internal_profile_disconnect(device_id, APP_IBRT_HID_PROFILE_ID);
#else
        app_bt_hid_profile_disconnect(&curr_device->remote);
#endif
    }

    curr_device->wait_send_capture_key = false;
}

static void app_bt_hid_disc_chan_timer_handler(const void *param)
{
    uint8_t device_id = (uint8_t)(uintptr_t)param;

    TRACE(2, "(d%x) %s", device_id, __func__);

    app_bt_hid_close_shutter_mode(device_id);
}

static void app_bt_hid_start_disc_wait_timer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device->hid_wait_disc_timer_id)
    {
        TRACE(1, "%s timer is NULL", __func__);
        return;
    }

    TRACE(2, "(d%x) %s", device_id, __func__);

    osTimerStop(curr_device->hid_wait_disc_timer_id);

    osTimerStart(curr_device->hid_wait_disc_timer_id, BT_HID_DISC_CHANNEL_WAIT_MS);
}

static void app_bt_hid_stop_wait_disc_chan_timer(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (!curr_device->hid_wait_disc_timer_id)
    {
        return;
    }

    osTimerStop(curr_device->hid_wait_disc_timer_id);
}

bool app_bt_hid_is_in_shutter_mode(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);

        if (curr_device->hid_conn_flag)
        {
            return true;
        }
    }

    return false;
}

void app_bt_hid_enter_shutter_mode(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    int i = 0;

    TRACE(1, "%s", __func__);

    for (i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (!curr_device->acl_is_connected)
        {
            TRACE(2, "%s acl d%x is not connected", __func__, i);
            continue;
        }
        app_bt_hid_open_shutter_mode(i, false);
    }
}

void app_bt_hid_exit_shutter_mode(void)
{
    int i = 0;

    TRACE(1, "%s", __func__);

    for (i = 0; i < BT_DEVICE_NUM; ++i)
    {
        app_bt_hid_stop_wait_disc_chan_timer(i);

        app_bt_hid_close_shutter_mode(i);
    }
}

void app_bt_hid_disconnect_all_channels(void)
{
    struct BT_DEVICE_T *curr_device = NULL;

    TRACE(1, "%s", __func__);

    for (int i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);

        if (curr_device->hid_conn_flag)
        {
            app_bt_hid_profile_disconnect(&curr_device->remote);
        }

        app_bt_hid_stop_wait_disc_chan_timer(i);

        curr_device->wait_send_capture_key = false;
    }
}

void app_bt_hid_send_capture(void)
{
    struct BT_DEVICE_T *curr_device = NULL;
    int i = 0;

    TRACE(1, "%s", __func__);

    for (i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (!curr_device->acl_is_connected)
        {
            TRACE(2, "%s acl d%x is not connected", __func__, i);
            continue;
        }
        app_bt_hid_stop_wait_disc_chan_timer(i);
        app_bt_hid_open_shutter_mode(i, true);
    }
}

void app_bt_hid_send_consumer_ctrl_key(uint8_t ctrl_key)
{
    struct BT_DEVICE_T *curr_device = NULL;
    int i = 0;

    TRACE(1, "%s", __func__);

    for (i = 0; i < BT_DEVICE_NUM; ++i)
    {
        curr_device = app_bt_get_device(i);
        if (!curr_device->acl_is_connected)
        {
            TRACE(2, "%s acl d%x is not connected", __func__, i);
            continue;
        }
        btif_hid_consumer_send_ctrl_key(curr_device->hid_channel, ctrl_key);
        btif_hid_consumer_send_ctrl_key(curr_device->hid_channel, HID_CONSUMER_CTRL_KEY_NULL);
    }
}

bt_status_t app_bt_hid_send_sensor_report(const bt_bdaddr_t *remote, const struct bt_hid_sensor_report_t *report)
{
    uint32_t a, b, c, d;
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (!curr_device)
    {
        TRACE(0, "app_bt_hid_send_sensor_report: %02x:%02x:***:%02x not connected",
            remote->address[0], remote->address[1], remote->address[5]);
        return BT_STS_FAILED;
    }

    a = (((uint32_t)((uint16_t)report->ry)) << 16) | ((uint16_t)report->rx);
    b = (((uint32_t)((uint16_t)report->vx)) << 16) | ((uint16_t)report->rz);
    c = (((uint32_t)((uint16_t)report->vz)) << 16) | ((uint16_t)report->vy);
    d = (((uint32_t)curr_device->device_id) << 8) | report->counter;

    app_bt_call_func_in_bt_thread(a, b, c, d, (uint32_t)(uintptr_t)btif_hid_sensor_send_input_report);
    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_process_sensor_report(const bt_bdaddr_t *remote, const struct bt_hid_sensor_report_t *report)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);

    if (!curr_device || !curr_device->hid_conn_flag)
    {
        TRACE(0, "app_bt_hid_process_sensor_report: not connected");
        return BT_STS_FAILED;
    }

    curr_device->wait_send_capture_key = false;

#if defined(IBRT_UI)
    if (tws_besaud_is_connected())
    {
        uint8_t ibrt_role = app_tws_get_ibrt_role(&curr_device->remote);
        TRACE(3, "(d%x) %s role %d", curr_device->device_id, __func__, ibrt_role);

        if (IBRT_MASTER == ibrt_role)
        {
            app_bt_hid_send_sensor_report(remote, report);
        }
        else if (IBRT_SLAVE == ibrt_role)
        {
            app_ibrt_if_start_user_action_v2(curr_device->device_id, IBRT_ACTION_HID_SENSOR_REPORT, (uint32_t)(uintptr_t)report, 0);
        }
    }
    else
    {
        app_bt_hid_send_sensor_report(remote, report);
    }
#else
    app_bt_hid_send_sensor_report(remote, report);
#endif

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_connect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);

    if (!curr_device)
    {
        TRACE(0, "%s: no connection", __func__);
        return BT_STS_FAILED;
    }

    if (curr_device->hid_conn_flag)
    {
        TRACE(0, "(d%x) %s: hid already connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

#if defined(IBRT_UI)
    app_ibrt_internal_profile_connect(curr_device->device_id, APP_IBRT_HID_PROFILE_ID, false);
#else
    app_bt_hid_profile_connect(&curr_device->remote, false);
#endif

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_disconnect(const bt_bdaddr_t *remote)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);

    if (!curr_device)
    {
        TRACE(0, "app_bt_hid_disconnect: no connection");
        return BT_STS_FAILED;
    }

    if (!curr_device->hid_conn_flag)
    {
        TRACE(0, "(d%x) %s: hid not connected", curr_device->device_id, __func__);
        return BT_STS_FAILED;
    }

#if defined(IBRT_UI)
    app_ibrt_internal_profile_disconnect(curr_device->device_id, APP_IBRT_HID_PROFILE_ID);
#else
    app_bt_hid_profile_disconnect(&curr_device->remote);
#endif

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_send_keyboard_report(const bt_bdaddr_t *remote, uint8_t modifier_key, uint8_t key_code)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (!curr_device)
    {
        TRACE(0, "app_bt_hid_send_keyboard_report: %02x:%02x:***:%02x not connected",
            remote->address[0], remote->address[1], remote->address[5]);
        return BT_STS_FAILED;
    }

    app_bt_call_func_in_bt_thread((uint32_t)(uintptr_t)curr_device->hid_channel,
        modifier_key, key_code, 0, (uint32_t)(uintptr_t)btif_hid_keyboard_input_report);
    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_send_mouse_report(const bt_bdaddr_t *remote, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (!curr_device)
    {
        TRACE(0, "app_bt_hid_send_mouse_report: %02x:%02x:***:%02x not connected",
            remote->address[0], remote->address[1], remote->address[5]);
        return BT_STS_FAILED;
    }

    app_bt_call_func_in_bt_thread((uint32_t)(uintptr_t)curr_device->hid_channel,
        (uint32_t)x_pos, (uint32_t)y_pos, clk_buttons, (uint32_t)(uintptr_t)btif_hid_mouse_input_report);

    if (app_bt_manager.config.hid_capture_non_invade_mode)
    {
        app_bt_hid_start_disc_wait_timer((uint8_t)curr_device->device_id);
    }

    return BT_STS_SUCCESS;
}

bt_status_t app_bt_hid_send_mouse_control_report(const bt_bdaddr_t *remote, uint8_t ctl_buttons)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_connected_device_byaddr(remote);
    if (!curr_device)
    {
        TRACE(0, "app_bt_hid_send_mouse_control_report: %02x:%02x:***:%02x not connected",
            remote->address[0], remote->address[1], remote->address[5]);
        return BT_STS_FAILED;
    }

    app_bt_call_func_in_bt_thread((uint32_t)(uintptr_t)curr_device->hid_channel,
        (uint32_t)ctl_buttons, 0, 0, (uint32_t)(uintptr_t)btif_hid_mouse_control_report);

    if (app_bt_manager.config.hid_capture_non_invade_mode)
    {
        app_bt_hid_start_disc_wait_timer((uint8_t)curr_device->device_id);
    }

    return BT_STS_SUCCESS;
}

static void app_bt_hid_enter_shutter_mode_test(const char* param, uint32_t param_len)
{
    app_bt_hid_enter_shutter_mode();
}

static void app_bt_hid_exit_shutter_mode_test(const char* param, uint32_t param_len)
{
    app_bt_hid_exit_shutter_mode();
}

static void app_bt_hid_send_consumer_play_test(const char* param, uint32_t param_len)
{
    app_bt_hid_send_consumer_ctrl_key(0x01);
}

static void app_bt_hid_send_capture_test(const char* param, uint32_t param_len)
{
    app_bt_hid_send_capture();
}

static app_bt_host_cmd_table_t app_hid_test_cmd_table[] =
{
    {"hid_enter",               app_bt_hid_enter_shutter_mode_test},
    {"hid_exit",                app_bt_hid_exit_shutter_mode_test},
    {"hid_send_play",           app_bt_hid_send_consumer_play_test},
    {"hid_capture",             app_bt_hid_send_capture_test},
};

bt_status_t app_bt_hid_init(void)
{
    TRACE(2, "%s sink_enable %d source_enable %d", __func__, besbt_cfg.bt_sink_enable, besbt_cfg.bt_source_enable);

    if (bt_hid_initialized)
    {
        return BT_STS_SUCCESS;
    }

    bt_hid_initialized = true;

    // init hid channle (include host and device)
    btif_hid_channel_init();

    // init some interface
    btif_hid_intf_init(app_bt_hid_device_callback, app_bt_hid_get_custom_descriptor);

    // init l2cap channel
    btif_hid_init_port();

    if (besbt_cfg.bt_sink_enable)
    {
        struct BT_DEVICE_T *curr_device = NULL;
        for (int i = 0; i < BT_DEVICE_NUM; ++i)
        {
            curr_device = app_bt_get_device(i);
            curr_device->hid_channel = btif_hid_channel_alloc_and_init(i, true, app_bt_hid_device_callback);
            if (i == 0)
            {
                curr_device->capture_wait_timer_id = osTimerCreate(osTimer(BT_HID_CAPTURE_WAIT_TIMER0), osTimerOnce, (void *)i);
                curr_device->hid_wait_disc_timer_id = osTimerCreate(osTimer(BT_HID_DISC_CHAN_TIMER0), osTimerOnce, (void *)i);
            }
#if BT_DEVICE_NUM > 1
            else if (i == 1)
            {
                curr_device->capture_wait_timer_id = osTimerCreate(osTimer(BT_HID_CAPTURE_WAIT_TIMER1), osTimerOnce, (void *)i);
                curr_device->hid_wait_disc_timer_id = osTimerCreate(osTimer(BT_HID_DISC_CHAN_TIMER1), osTimerOnce, (void *)i);
            }
#endif
#if BT_DEVICE_NUM > 2
            else if (i == 2)
            {
                curr_device->capture_wait_timer_id = osTimerCreate(osTimer(BT_HID_CAPTURE_WAIT_TIMER2), osTimerOnce, (void *)i);
                curr_device->hid_wait_disc_timer_id = osTimerCreate(osTimer(BT_HID_DISC_CHAN_TIMER2), osTimerOnce, (void *)i);
            }
#endif
            curr_device->hid_conn_flag = false;
            curr_device->wait_send_capture_key = false;
        }

        // hid device need to register sdp record
        btif_hid_init_sdp_record();
    }
#if defined(BT_SOURCE)
    if (besbt_cfg.bt_source_enable)
    {
        struct BT_SOURCE_DEVICE_T* source_dev = NULL;
        for (int i = BT_SOURCE_DEVICE_ID_1; i < BT_SOURCE_DEVICE_ID_N; i += 1)
        {
            source_dev = app_bt_source_get_device(i);
            if (source_dev)
            {
                source_dev->base_device->hid_channel = btif_hid_channel_alloc_and_init(i, false, app_bt_hid_host_callback);
                source_dev->base_device->capture_wait_timer_id = NULL;
                source_dev->base_device->hid_wait_disc_timer_id = NULL;
                source_dev->base_device->hid_conn_flag = false;
                source_dev->base_device->wait_send_capture_key = false;
            }
        }
    }
#endif // BT_SOURCE

    app_bt_host_add_cmd_table(sizeof(app_hid_test_cmd_table)/sizeof(app_hid_test_cmd_table[0]),
        app_hid_test_cmd_table);

    return BT_STS_SUCCESS;
}

bt_status_t bt_hid_virtual_cable_unplug(const bt_bdaddr_t *bd_addr)
    __attribute__((alias("app_bt_hid_virtual_cable_unplug")));

bt_status_t bt_hid_capture(const bt_bdaddr_t *bd_addr)
    __attribute__((alias("app_bt_hid_capture_process")));

bt_status_t bt_hid_send_sensor_report(const bt_bdaddr_t *bd_addr, const struct bt_hid_sensor_report_t *report)
    __attribute__((alias("app_bt_hid_process_sensor_report")));

bt_status_t bt_hid_send_keyboard_report(const bt_bdaddr_t *remote, uint8_t modifier_key, uint8_t key_code)
    __attribute__((alias("app_bt_hid_send_keyboard_report")));

bt_status_t bt_hid_send_mouse_report(const bt_bdaddr_t *remote, int8_t x_pos, int8_t y_pos, uint8_t clk_buttons)
    __attribute__((alias("app_bt_hid_send_mouse_report")));

bt_status_t bt_hid_send_mouse_control_report(const bt_bdaddr_t *remote, uint8_t ctl_buttons)
    __attribute__((alias("app_bt_hid_send_mouse_control_report")));

#endif /* BT_HID_DEVICE */
