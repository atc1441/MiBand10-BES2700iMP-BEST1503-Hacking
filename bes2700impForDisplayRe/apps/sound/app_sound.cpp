/***************************************************************************
 *
 * @copyright 2023-2033 BES.
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
 * @author dazhaoyang
 * @date 2023.06.15         v1.0
 *
 ****************************************************************************/
#include "app_sound.h"
#include "SoundDebug.h"
#include "SoundApi.h"
#include "bes_gap_api.h"
#include "factory_section.h"
#include "app_bt_func.h"

#if defined(NEW_NV_RECORD_ENABLED)
#include "nvrecord_env.h"
#endif
#include "app_key.h"

#if defined(WIFI_SOUND_UI_ENABLE)
#include "AppWifiSoundCustom.h"
#endif

#include "audio_player_adapter.h"
#include "app_audio_control.h"

#ifdef APP_SOUND_UI_ENABLE

static struct app_sound_ctrl_t g_sound_ctrl = {0};
static bool sound_is_initialized = false;

/*
 * handling timer idle state timeout shutdown
 */
//static void app_sound_timer_checker_callback(void const *param);
//osTimerDef(SOUND_CHECKER_TIMER, app_sound_timer_checker_callback);

 /*
  * Delay little time for handling UI event to avoid quickly ping-pong operations
  */
//static  void app_sound_ui_wait_ble_scanning_device_timer_cb(void const *n);
//osTimerDef (SOUND_BLE_ADV_DEVICE_TIMER, app_sound_ui_wait_ble_scanning_device_timer_cb);

static void app_sound_circle_click_handle(void);
static void app_sound_circle_double_click_handle(void);
void app_sound_ui_clear_pairing_recoder_event(bool is_sync_event);

struct app_sound_ctrl_t *app_sound_get_ctrl(void)
{
    return &g_sound_ctrl;
}

static struct app_sound_bt_device_info_t *app_sound_get_bt_device_info(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    return &app_sound_ctrl->bt_device_info;
}

static struct app_sound_wifi_device_info_t *app_sound_get_wifi_device_info(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    return &app_sound_ctrl->wifi_device_info;
}

static struct app_sound_tws_info_t *app_sound_get_tws_info(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    return &app_sound_ctrl->bt_device_info.tws_info;
}

static sound_tws_config_adv_info_t *app_sound_get_tws_config(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    return &app_sound_ctrl->tws_config_info;
}

static struct app_sound_factory_device_info_t *app_sound_get_factory_info(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    return &app_sound_ctrl->factory_device_info;
}

void app_sound_set_business_device_type(uint8_t device_type)
{
    app_sound_get_ctrl()->business_device_type = device_type;
}

uint8_t app_sound_get_business_device_type(void)
{
    return app_sound_get_ctrl()->business_device_type;
}

static void app_sound_timer_checker_callback(void* param)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (app_sound_ctrl->business_device_type == BT_DEVICE_TYPE)
    {
        soundShutdown();
    }
    else if (app_sound_ctrl->business_device_type == WIFI_DEVICE_TYPE)
    {

    }
    else
    {

    }
}

void app_sound_updata_checker_timer(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (app_sound_ctrl->sound_timer)
    {
        osTimerStop(app_sound_ctrl->sound_timer);
        osTimerStart(app_sound_ctrl->sound_timer, SOUND_WAIT_SHUTDOWN_TIMEOUT);
        app_sound_ctrl->sound_timer_start = true;
    }
}

void app_sound_checker_timer_manager(SOUND_CHECK_TIMER_EVENT_E type, uint8_t device_id, bool status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    LOG_I("sound_timer_status=%04x type=%d device_id=%d status=%d", app_sound_ctrl->sound_timer_status, type, device_id, status);

    if (type == SOUND_TIMER_BT_A2DP_STREAMING)
    {
        if (device_id < BT_REMOTE_DEVICE_NUM)
        {
            uint16_t status = 0;
            status = BT_REMOTE_DEVICE_NUM * (1 << (device_id * 4));
            if (status)
            {
                app_sound_ctrl->sound_timer_status |= status;
            }
            else
            {
                app_sound_ctrl->sound_timer_status &= (~status);
            }
        }
    }
    else if (type == SOUND_TIMER_BT_HFP_CALL)
    {
        if (device_id < BT_REMOTE_DEVICE_NUM)
        {
            uint16_t status = 0;
            status = BT_REMOTE_DEVICE_NUM * (1 << (device_id * 4));
            if (status)
            {
                app_sound_ctrl->sound_timer_status |= status;
            }
            else
            {
                app_sound_ctrl->sound_timer_status &= (~status);
            }
        }
    }
    else if (type == SOUND_TIMER_WIFI_STREAMING)
    {
        if (status)
        {
            app_sound_ctrl->sound_timer_status |= SOUND_TIMER_WIFI_STREAMING;
        }
        else
        {
            app_sound_ctrl->sound_timer_status &= (~SOUND_TIMER_WIFI_STREAMING);
        }
    }
    else if (type == SOUND_TIMER_AUX_STREAMING)
    {
        if (status)
        {
            app_sound_ctrl->sound_timer_status |= SOUND_TIMER_AUX_STREAMING;
        }
        else
        {
            app_sound_ctrl->sound_timer_status &= (~SOUND_TIMER_AUX_STREAMING);
        }
    }

    LOG_I("sound_timer_status=%04x timer_start=%d", app_sound_ctrl->sound_timer_status, app_sound_ctrl->sound_timer_start);

    if (app_sound_ctrl->sound_timer_status == 0)
    {
        app_sound_updata_checker_timer();
    }
    else
    {
        if (app_sound_ctrl->sound_timer)
        {
            osTimerStop(app_sound_ctrl->sound_timer);
            app_sound_ctrl->sound_timer_start = false;
        }
    }
}

void app_sound_ui_stop_ble_scanning_device_timer(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->sound_adv_timer != NULL)
    {
        osTimerStop(app_sound_ctrl->sound_adv_timer);
    }
}

static void app_sound_ui_wait_ble_scanning_device_timer_cb(void* param)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_DISCOVERY)
    {
        app_sound_ui_event_handler(SOUND_ACTION_TWS_DISCOVERY,SOUND_UI_RSP_TIMEOUT);
    }
    else if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_FORMATION)
    {
        app_sound_ui_event_handler(SOUND_ACTION_TWS_FORMATION,SOUND_UI_RSP_TIMEOUT);
    }
    else if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_PREHANDLE)
    {
        app_sound_ui_event_handler(SOUND_ACTION_TWS_PREHANDLE,SOUND_UI_RSP_TIMEOUT);
    }
}

static void app_sound_ui_event_queue_init(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    memset(&app_sound_ctrl->event_q_manager, 0, sizeof(sound_event_manager_t));
}

SOUND_UI_ERROR_E app_sound_ui_event_queue_insert(SOUND_UI_TYPE_E event)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_event_manager_t *p_event_queue = &app_sound_ctrl->event_q_manager;
    if (((p_event_queue->rear+1)%SOUND_EVENT_Q_LENGTH) == p_event_queue->front)
    {
        //event queue full
        LOG_I("inster ibrt event queue error");
        return SOUND_UI_STATUS_ERROR;
    }
    else
    {
        p_event_queue->sound_event_q[p_event_queue->rear] = event;
        p_event_queue->rear = (p_event_queue->rear+1)%SOUND_EVENT_Q_LENGTH;

        LOG_I("insert event queue, rear=%d, front=%d", \
                                p_event_queue->rear, p_event_queue->front);

        return SOUND_UI_NO_ERROR;
    }
}

SOUND_UI_ERROR_E app_sound_ui_event_queue_get_front(SOUND_UI_TYPE_E *p_event)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_event_manager_t *p_event_queue = &app_sound_ctrl->event_q_manager;

    if (p_event_queue->rear == p_event_queue->front)
    {
        //event queue empty
        return SOUND_UI_STATUS_ERROR;
    }
    else
    {
        *p_event = p_event_queue->sound_event_q[p_event_queue->front];
        return SOUND_UI_NO_ERROR;
    }
}

SOUND_UI_ERROR_E app_sound_ui_event_queue_delete(SOUND_UI_TYPE_E *p_event)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_event_manager_t *p_event_queue = &app_sound_ctrl->event_q_manager;
    if (!p_event)
    {
        return SOUND_UI_STATUS_ERROR;
    }
    if (p_event_queue->rear == p_event_queue->front)
    {
        //event queue empty
        LOG_I("delete event queue error");
        return SOUND_UI_STATUS_ERROR;
    }
    else
    {
        *p_event = p_event_queue->sound_event_q[p_event_queue->front];
        p_event_queue->front = (p_event_queue->front+1)%SOUND_EVENT_Q_LENGTH;
        LOG_I("delete event queue, rear=%d, front=%d", p_event_queue->rear, p_event_queue->front);
        return SOUND_UI_NO_ERROR;
    }
}

void app_sound_ui_event_show_in_led(SOUND_EVENT_TYPE evt_type, SOUND_UI_ERROR_E status)
{

}

extern uint8_t app_audio_adm_get_bt_active_device();

void app_sound_updata_bt_active_device(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    p_bt_device->active_device_id = 0;//app_audio_adm_get_bt_active_device();
}

void app_sound_wifi_acl_state_handle(SOUND_ACL_STATUS_TYPE_E status)
{
    app_sound_wifi_device_info_t *p_wifi_device = app_sound_get_wifi_device_info();
    LOG_I("app_sound_wifi_acl_state_handle: status %d=>%d!", p_wifi_device->acl_state, status);
    p_wifi_device->acl_state = status;
    if (status == SOUND_ACL_CONNECTED)
    {
    }
    else if(status == SOUND_ACL_DISCONNECTED)
    {
    }
}

void app_sound_airplay_state_handle(SOUND_WIFI_AIRPLAY_STATUS_TYPE_E status)
{
    app_sound_wifi_device_info_t *p_wifi_device = app_sound_get_wifi_device_info();

    LOG_I("app_sound_airplay_state_handle: status %d=>%d!", p_wifi_device->profile_state, status);
    p_wifi_device->profile_state = status;
    if (status == SOUND_WIFI_AIRPLAY_STREAM_STARTED)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_WIFI_STREAMING, 0, true);
    }
    else if (status == SOUND_WIFI_AIRPLAY_STREAM_CLOSE)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_WIFI_STREAMING, 0, false);
    }
}

void app_sound_bredr_acl_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_ACL_STATUS_TYPE_E status, uint8_t reason_code)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;

    if (device_id >= BT_REMOTE_DEVICE_NUM)
    {
        for (uint8_t i = 0; i < BT_REMOTE_DEVICE_NUM; i++)
        {
            device_addr_t *local_bt_addr = &p_bt_device->remote_info[i].remote_addr;
            if (memcmp((uint8_t *)local_bt_addr, (uint8_t *)addr, sizeof(device_addr_t)) == 0)
            {
                p_remote_info = &p_bt_device->remote_info[i];
                device_id = i;
                break;
            }
        }
    }
    else
    {
        p_remote_info = &p_bt_device->remote_info[device_id];
    }

    if (device_id >= BT_REMOTE_DEVICE_NUM)
    {
        LOG_I("app_sound_acl_state_handle: Warning!");
        return;
    }

    if (status == SOUND_ACL_CONNECTED)
    {
        if (reason_code)
        {
            app_sound_ui_event_handler(SOUND_ACTION_MOBILE_CONNECTED, SOUND_UI_CONNECT_FAILED);
        }
        else
        {
            if (p_remote_info->is_used == false)
            {
                p_remote_info->is_used = true;
            }
            memcpy((uint8_t *)&p_remote_info->remote_addr, (uint8_t *)addr, sizeof(device_addr_t));
            app_sound_ui_event_handler(SOUND_ACTION_MOBILE_CONNECTED, SOUND_UI_NO_ERROR);
        }
    }
    else if (status == SOUND_ACL_DISCONNECTED)
    {
        p_remote_info->is_used = false;
        memset((uint8_t *)&p_remote_info->remote_addr, 0, sizeof(device_addr_t));
        app_sound_ui_event_handler(SOUND_ACTION_MOBILE_DISCONNECTED, SOUND_UI_NO_ERROR);
    }
    p_remote_info->bredr_acl_state = status;
    app_sound_updata_bt_active_device();
}

void app_sound_a2dp_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_A2DP_STATUS_TYPE_E status)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;

    LOG_I("app_sound_a2dp_state_handle: device_id=%d status=%d!", device_id, status);

    if (!addr)
    {
        return;
    }

    if (device_id >= BT_REMOTE_DEVICE_NUM)
    {
        return;
    }
    p_remote_info = &p_bt_device->remote_info[device_id];
    if (p_remote_info->is_used == false)
    {
        return;
    }

    if (memcmp((void *)&p_remote_info->remote_addr, (void *)addr, sizeof(device_addr_t)))
    {
        return;
    }

    p_remote_info->a2dp_state = status;
    if (status == SOUND_BT_A2DP_STREAM_STARTED)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_BT_A2DP_STREAMING,device_id,  true);
    }
    else if (status == SOUND_BT_A2DP_STREAM_CLOSE)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_BT_A2DP_STREAMING,device_id,  false);
    }
    app_sound_updata_bt_active_device();
}

void app_sound_hfp_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_HFP_STATUS_TYPE_E status)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;
    LOG_I("app_sound_hfp_state_handle: device_id=%d status=%d!", device_id, status);

    if (!addr)
    {
        return;
    }

    if (device_id >= BT_REMOTE_DEVICE_NUM)
    {
        return;
    }
    p_remote_info = &p_bt_device->remote_info[device_id];

    if (p_remote_info->is_used == false)
    {
        return;
    }

    if (memcmp((void *)&p_remote_info->remote_addr, (void *)addr, sizeof(device_addr_t)))
    {
        return;
    }

    p_remote_info->hfp_state = status;

    app_sound_updata_bt_active_device();
}

void app_sound_sco_state_handle(uint8_t device_id, device_addr_t *addr, SOUND_BT_SCO_STATUS_TYPE_E status)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;

    LOG_I("app_sound_sco_state_handle: device_id=%d status=%d!", device_id, status);

    if (!addr)
    {
        return;
    }

    if (device_id >= BT_REMOTE_DEVICE_NUM)
    {
        return;
    }
    p_remote_info = &p_bt_device->remote_info[device_id];

    if (p_remote_info->is_used == false)
    {
        return;
    }

    if (memcmp((void *)&p_remote_info->remote_addr, (void *)addr, sizeof(device_addr_t)))
    {
        return;
    }

    p_remote_info->sco_state = status;
    if (status == SOUND_BT_SCO_CONNECTED)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_BT_HFP_CALL,device_id,  true);
    }
    else if (status == SOUND_BT_SCO_DISCONNECTED)
    {
        app_sound_checker_timer_manager(SOUND_TIMER_BT_HFP_CALL,device_id,  false);
    }
    app_sound_updata_bt_active_device();
}

bool app_sound_sync_send(uint8_t *buf, uint16_t len)
{
    if (!buf || (len == 0))
    {
        return false;
    }
    return soundTwsSendData(buf, len);
}

void app_sound_sync_active_event(SOUND_EVENT_TYPE event_type)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
    {
        struct app_sound_sync_event_t sync_data;
        sync_data.opcode = 0xff;
        sync_data.data_len = 1;
        sync_data.event = event_type;
        app_sound_sync_send((uint8_t*)&sync_data, sizeof(struct app_sound_sync_event_t));
    }
}

void app_sound_sync_info_handle(uint8_t *buf, uint16_t len)
{
    if (!buf)
    {
        return;
    }

    if (len < 3)
    {
        LOG_I("%s:rec data len too short!", __func__);
        return;
    }

    if ((buf[0] != 0xff) || (buf[1] != 1))//opcode and data len
    {
        LOG_I("%s:The sync cmd format is incorrect!", __func__);
        return;
    }

    switch (buf[2])
    {
        case SOUND_UI_POWEROFF_EVENT:
            app_sound_ui_event_entry(SOUND_UI_POWEROFF_EVENT);
            break;

        case SOUND_UI_TWS2FREEMAN_EVENT:
            app_sound_ui_event_entry(SOUND_UI_TWS2FREEMAN_EVENT);
            break;

        case SOUND_UI_TWS2BIS_EVENT:
            app_sound_ui_event_entry(SOUND_UI_TWS2BIS_EVENT);
            break;

        case SOUND_CUSTOM_CIRCLE_CLICK_EVENT:
            app_sound_circle_click_handle();
            break;

        case SOUND_CUSTOM_CIRCLE_DOUBLE_CLICK_EVENT:
            app_sound_circle_double_click_handle();
            break;

        case SOUND_CUSTOM_VOLUME_UP_EVENT:
            app_audio_control_streaming_volume_up();
            break;

        case SOUND_CUSTOM_VOLUME_DOWN_EVENT:
            app_audio_control_streaming_volume_down();
            break;

        case SOUND_CUSTOM_CONTROL_MEDIA_FORWARD_EVENT:
            app_audio_control_media_forward();
            break;

        case SOUND_CUSTOM_CONTROL_MEDIA_BACKWARD_EVENT:
            app_audio_control_media_backward();
            break;

        case SOUND_CUSTOM_BT_LONG_PRESS_EVENT:
            app_sound_ui_clear_pairing_recoder_event(true);
            break;

        default:
            break;
    }

    return;
}

static uint8_t crc8_cal(uint8_t *data, uint32_t length)
{
    uint8_t i;
    uint8_t crc = 0;        // Initial value
    while(length--)
    {
        crc ^= *data++;        // crc ^= *data; data++;
        for ( i = 0; i < 8; i++ )
        {
            if ( crc & 0x80 )
                crc = (crc << 1) ^ 0x07;
            else
                crc <<= 1;
        }
    }
    return crc;
}

static void app_sound_ble_adv_data_stop(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    LOG_I("%s adv_state:%d", __func__, p_bt_device->ble_adv_state);
    if (p_bt_device->ble_adv_state != SOUND_BLE_ADV_OP_IDLE)
    {
        p_bt_device->ble_adv_state = SOUND_BLE_ADV_OP_IDLE;
        bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_0);
    }
}

static void app_sound_ble_scan_device_start(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    LOG_I("%s scan_state:%d", __func__, p_bt_device->ble_scan_state);
    if (p_bt_device->ble_scan_state == SOUND_BLE_SCAN_STATE_STOPPED)
    {
        bes_ble_scan_param_t scan_param = {0};

        scan_param.scanFolicyType = BLE_SCAN_ALLOW_ADV_ALL;
        scan_param.scanWindowMs   = SOUND_BLE_SCAN_WINDOWN_MS;
        scan_param.scanIntervalMs   = SOUND_BLE_SCAN_INTERVAL_MS;
        bes_ble_gap_start_scan(&scan_param);
        p_bt_device->ble_scan_state = SOUND_BLE_SCAN_STATE_STARTED;
    }
}

void app_sound_ble_scan_device_stop(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    LOG_I("%s scan_state:%d", __func__, p_bt_device->ble_scan_state);
    if (p_bt_device->ble_scan_state != SOUND_BLE_SCAN_STATE_STOPPED)
    {
        bes_ble_gap_stop_scan();
        p_bt_device->ble_scan_state = SOUND_BLE_SCAN_STATE_STOPPED;
    }
}

static void app_sound_add_ble_scan_tws_discovery_data(device_addr_t *addr)
{
    if (!addr)
    {
        return;
    }

    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_ble_scan_data_store_cache_table_t *p_data = &app_sound_ctrl->ble_scan_data_cache[0];

    p_data->peer_scan_recv_cnt = SOUND_BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD;
    memcpy(&p_data->peer_device_addr, addr, sizeof(device_addr_t));
}

static void app_sound_tws_discovery_data_save(int8_t rssi_value, uint8_t *adv_data, uint8_t list_id)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_ble_scan_data_store_cache_table_t *p_data = &app_sound_ctrl->ble_scan_data_cache[list_id];
    sound_ble_adv_data_tws_discover_ind_t *adv_data_req = (sound_ble_adv_data_tws_discover_ind_t *)adv_data;

    p_data->peer_rssi_value = rssi_value;
    memcpy(&p_data->peer_product_info, &adv_data_req->product_info, sizeof(sound_product_info_t));
    memcpy(&p_data->peer_device_addr, &adv_data_req->device_addr, sizeof(device_addr_t));

    for (uint8_t i = 0; i < BT_REMOTE_DEVICE_NUM; i++)
    {
        memcpy(&p_data->peer_bt_remote_info, &adv_data_req->bt_remote_info, sizeof(sound_share_bt_info_t));
    }
    memcpy(&p_data->peer_wifi_status, &adv_data_req->wifi_status, sizeof(sound_share_wifi_info_t));
}

static bool app_sound_recv_scan_data_count(bool start_flage, uint8_t index)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_ble_scan_data_store_cache_table_t *p_data_cache = &app_sound_ctrl->ble_scan_data_cache[index];

    bool ret = false;
    p_data_cache->is_use = true;
    if (start_flage)
    {
        p_data_cache->peer_scan_recv_cnt = 1;
    }
    else
    {
        p_data_cache->peer_scan_recv_cnt += 1;
    }

    if (p_data_cache->peer_scan_recv_cnt >= SOUND_BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD)
    {
        p_data_cache->peer_scan_recv_cnt = SOUND_BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD;
        ret = true;
    }
    return ret;
}

static bool app_sound_scan_discovery_handle(bool start_flage, uint8_t index, int8_t rssi, uint8_t *adv_data)
{
    bool ret = false;
    if (index < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE)
    {
        app_sound_tws_discovery_data_save(rssi, adv_data, index);
        app_sound_recv_scan_data_count(start_flage, index);
        ret = true;
    }
    return ret;
}

static bool app_sound_ble_scan_data_handle_tws_discovery_ind(int8_t rssi_value, uint8_t *adv_data, uint8_t data_len)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_ble_adv_data_tws_discover_ind_t *stereo_adv_data_req = (sound_ble_adv_data_tws_discover_ind_t*)adv_data;
    sound_ble_scan_data_store_cache_table_t *p_data_cache = NULL;
    uint8_t crc_value_origin = 0;
    uint8_t crc_value_check = 0;
    uint8_t index = 0;

    crc_value_origin = stereo_adv_data_req->crc_check;
    crc_value_check = crc8_cal(adv_data, sizeof(sound_ble_adv_data_tws_discover_ind_t)-1);
    if (crc_value_origin != crc_value_check)
    {
        return false;
    }

    for (index = 0; index < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; index++)
    {
        p_data_cache = &app_sound_ctrl->ble_scan_data_cache[index];
        if (memcmp(&p_data_cache->peer_device_addr, &stereo_adv_data_req->device_addr, sizeof(device_addr_t)) == 0)
        {
            return app_sound_scan_discovery_handle(false, index, rssi_value, (uint8_t *)stereo_adv_data_req);
        }
    }

    for (index = 0; index < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; index++)
    {
        p_data_cache = &app_sound_ctrl->ble_scan_data_cache[index];
        if (!p_data_cache->is_use)
        {
            return app_sound_scan_discovery_handle(true, index, rssi_value, (uint8_t *)stereo_adv_data_req);
        }
    }

    for (index = 0; index < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; index++)
    {
        p_data_cache = &app_sound_ctrl->ble_scan_data_cache[index];
        int8_t min_rssi = p_data_cache->peer_rssi_value;
        if (rssi_value >= min_rssi)
        {
            return app_sound_scan_discovery_handle(true, index, rssi_value, (uint8_t *)stereo_adv_data_req);
        }
    }
    return false;
}

static void app_sound_set_sound_tws_info(SoundTWSRole role, uint8_t name_len, uint8_t *name, device_addr_t *addr)
{
    SoundTWSInfo info;
    memset(&info, 0, sizeof(SoundTWSInfo));
    if (!name || !addr)
    {
        return;
    }
    info.role = role;
    info.side = (SoundTWSSide)role;
    info.nameLen = name_len;
    memcpy(&info.nameStr, name, info.nameLen);
    memcpy(&info.btaddr, addr, sizeof(device_addr_t));
    soundEnterTws(&info);
}

static void app_sound_scan_formation_handle(SOUND_ROLE_E role, uint8_t *data, bool is_formation)
{
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();
    sound_tws_config_adv_info_t *p_tws_config = app_sound_get_tws_config();

    if (!data)
    {
        return;
    }

    p_tws_info->role = role;
    sound_tws_config_adv_info_t *sound_tws_config = (sound_tws_config_adv_info_t *)data;

    if (role == SOUND_ROLE_MASTER)
    {
        app_sound_set_sound_tws_info(RoleMaster, p_factory->name_len, &p_factory->device_name[0], &p_factory->device_addr);
    }
    else
    {
        app_sound_set_sound_tws_info(RoleSlave, sound_tws_config->name_len, &sound_tws_config->device_name[0], &sound_tws_config->master_addr);

        memcpy(&p_tws_config->master_addr, &sound_tws_config->master_addr, sizeof(device_addr_t));
        memcpy(&p_tws_config->slave_addr, &p_factory->device_addr, sizeof(device_addr_t));
        p_tws_config->name_len = sound_tws_config->name_len;
        memcpy(&p_tws_config->device_name[0], &sound_tws_config->device_name[0], sound_tws_config->name_len);
    }
}

static bool app_sound_ble_scan_data_handle_tws_perhandle_ind(uint8_t *adv_data, uint8_t data_len)
{
    sound_adv_data_tws_perhandle_t *sound_tws_perhandle = (sound_adv_data_tws_perhandle_t*)adv_data;
    sound_tws_config_adv_info_t *p_config = &sound_tws_perhandle->tws_config_info;
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();
    bool tws_perhandle = false;
    uint8_t crc_value_origin = sound_tws_perhandle->crc_check;
    uint8_t crc_value_check = crc8_cal(adv_data, sizeof(sound_adv_data_tws_perhandle_t)-1);

    if (!adv_data)
    {
        return false;
    }

    if (crc_value_origin == crc_value_check)
    {
        if ((memcmp(&p_factory->device_addr, &p_config->master_addr, sizeof(device_addr_t)) == 0) || \
            (memcmp(&p_factory->device_addr, &p_config->slave_addr, sizeof(device_addr_t)) == 0))
        {
            tws_perhandle = true;
        }
    }
    return tws_perhandle;
}


static uint8_t app_sound_scan_cache_table_max_recv_cnt_index_get(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_ble_scan_data_store_cache_table_t *p_data = &app_sound_ctrl->ble_scan_data_cache[0];
    uint16_t max_recv_cnt = p_data->peer_scan_recv_cnt;
    uint8_t  max_cnt_index = 0;
    for (uint8_t i = 1; i < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; i++)
    {
        p_data = &app_sound_ctrl->ble_scan_data_cache[i];
        if (p_data->peer_scan_recv_cnt > max_recv_cnt)
        {
            max_cnt_index = i;
            max_recv_cnt = p_data->peer_scan_recv_cnt;
        }
    }
    return max_cnt_index;
}

static void app_sound_full_local_config_info(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();
    sound_tws_config_adv_info_t *p_config = app_sound_get_tws_config();
    uint8_t sel_index = app_sound_scan_cache_table_max_recv_cnt_index_get();
    sound_ble_scan_data_store_cache_table_t *p_data = &app_sound_ctrl->ble_scan_data_cache[sel_index];

    memcpy(&p_config->slave_addr, &p_data->peer_device_addr, sizeof(device_addr_t));

    memcpy(&p_config->master_addr, &p_factory->device_addr, sizeof(device_addr_t));

    p_config->name_len = p_factory->name_len;

    memcpy(&p_config->device_name[0], &p_factory->device_name[0], p_config->name_len);
}

static void app_sound_check_local_device_status(sound_device_status_info_t *device_status)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_wifi_device_info_t *p_wifi_device = app_sound_get_wifi_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;
    uint8_t device_id = 0;
    bool is_sco_audio = false;
    bool is_a2dp_audio = false;
    bool is_wifi_audio = false;

    if (!device_status)
    {
        return;
    }

    for(device_id = 0; device_id < BT_REMOTE_DEVICE_NUM; device_id++)
    {
        p_remote_info = &p_bt_device->remote_info[device_id];
        if ((p_remote_info->hfp_state == SOUND_BT_HFP_CALLNG) && (p_remote_info->sco_state  == SOUND_BT_SCO_CONNECTED))
        {
            device_status->audio_status = SOUND_AUDIO_CALLING;
            device_status->audio_device_type = BT_DEVICE_TYPE;
            is_sco_audio = true;
            break;
        }
    }

    if (is_sco_audio == false)
    {
        if ((p_wifi_device->profile_state == true) && (p_wifi_device->acl_state  == SOUND_ACL_CONNECTED))
        {
            device_status->audio_status = SOUND_AUDIO_WIFI_MUSIC;
            device_status->audio_device_type = WIFI_DEVICE_TYPE;
            is_wifi_audio = true;
        }
    }

    if ((is_sco_audio == false) && (is_wifi_audio == false))
    {
        for(device_id = 0; device_id < BT_REMOTE_DEVICE_NUM; device_id++)
        {
            p_remote_info = &p_bt_device->remote_info[device_id];
            if (p_remote_info->a2dp_state == SOUND_BT_A2DP_STREAM_STARTED)
            {
                device_status->audio_status = SOUND_AUDIO_BT_A2DP;
                device_status->audio_device_type = BT_DEVICE_TYPE;
                is_a2dp_audio = true;
                break;
            }
        }
    }
    if ((is_sco_audio == false) && (is_wifi_audio == false) && (is_a2dp_audio == false))
    {
        device_status->audio_status = SOUND_AUDIO_IDLE;
        device_status->audio_device_type = AUDIO_DEVICE_TYPE_UNKNOW;
    }

    device_status->bt_acl_connected_num = 0;
    for(device_id = 0; device_id < BT_REMOTE_DEVICE_NUM; device_id++)
    {
        p_remote_info = &p_bt_device->remote_info[device_id];
        if ((p_remote_info->bredr_acl_state == SOUND_ACL_CONNECTING) ||\
                    (p_remote_info->bredr_acl_state == SOUND_ACL_CONNECTED))
        {
            device_status->bt_acl_connected_num++;
        }
    }

    device_status->wifi_acl_status = p_wifi_device->acl_state;
}

static void app_sound_ble_adv_data_start_init(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();

    p_tws_info->role = SOUND_ROLE_UNKNOW;
    app_sound_ctrl->sound_tws_adv_cfg_cnt = 0;

    for (uint8_t i = 0; i < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; i++)
    {
        memset((void *)&app_sound_ctrl->ble_scan_data_cache[i], 0, sizeof(sound_ble_scan_data_store_cache_table_t));
    }

    app_sound_full_local_config_info();
}

static void app_sound_ble_adv_data_start(SOUND_ADV_TYPE_E adv_type)
{
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();
    app_sound_wifi_device_info_t *p_wifi_device = app_sound_get_wifi_device_info();
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    sound_tws_info_t *p_tws_adv = NULL;

    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    bes_ble_gap_cus_adv_param_t param;
    memset(&param, 0, sizeof(bes_ble_gap_cus_adv_param_t));
    memset(app_sound_ctrl->adv_data_info, 0, SOUND_ADV_DATA_MAX_LEN);
    param.actv_user = BLE_ADV_ACTIVITY_USER_0;
    param.is_custom_adv_flags = true;
    param.type = BLE_ADV_PUBLIC_STATIC;
    param.local_addr = &p_factory->device_addr.addr[0];
    param.peer_addr = NULL;
    param.adv_interval = SOUND_BLE_ADV_INTERVAL_DISCOVERY_TWS;
    param.adv_type = ADV_TYPE_CONN_EXT_ADV;
    param.adv_mode = ADV_MODE_EXTENDED;
    param.tx_power_dbm = SOUND_BLE_ADV_TX_POWER;

    if (adv_type == SOUND_DISCOVERY_TWS_ADV)
    {
        app_sound_bt_remote_info_t *p_remote_info = NULL;
        sound_product_info_t *p_product_adv = NULL;
        sound_share_bt_info_t *p_remote_adv = NULL;
        sound_share_wifi_info_t *p_wifi_adv = NULL;

        app_sound_ctrl->adv_data_info[0] = sizeof(sound_ble_adv_data_tws_discover_ind_t) + 1;
        app_sound_ctrl->adv_data_info[1] = BLE_ADV_DATA_MANUFACTURER_TYPE_ID;
        sound_ble_adv_data_tws_discover_ind_t *ble_scan_data_req = (sound_ble_adv_data_tws_discover_ind_t *)&app_sound_ctrl->adv_data_info[2];
        p_tws_adv = &ble_scan_data_req->tws_info;
        p_product_adv = &ble_scan_data_req->product_info;
        p_wifi_adv = &ble_scan_data_req->wifi_status;

        p_tws_adv->operation = SOUND_BLE_ADV_OP_SETUP_STEREO_REQ;
        p_tws_adv->sound_role = SOUND_ROLE_UNKNOW;

        //fill the product info adv data
        p_product_adv->product_type = LOCAL_SOUND_PRODUCT_TYPE;

        //fill the factory device addr info adv data
        memcpy(&ble_scan_data_req->device_addr, &p_factory->device_addr, sizeof(device_addr_t));

        //fill the bt remote info adv data
        for(uint8_t i = 0; i < BT_REMOTE_DEVICE_NUM; i++)
        {
            p_remote_adv = &ble_scan_data_req->bt_remote_info[i];
            p_remote_info = &app_sound_ctrl->bt_device_info.remote_info[i];
            p_remote_adv->acl_state = p_remote_info->bredr_acl_state;
            p_remote_adv->a2dp_state = p_remote_info->a2dp_state;
            p_remote_adv->hfp_state = p_remote_info->hfp_state;
            p_remote_adv->sco_state = p_remote_info->sco_state;
        }

        //fill the wifi info adv data
        p_wifi_adv->acl_state = p_wifi_device->acl_state;
        p_wifi_adv->profile_state = p_wifi_device->profile_state;

        ble_scan_data_req->crc_check = crc8_cal(&app_sound_ctrl->adv_data_info[2], sizeof(sound_ble_adv_data_tws_discover_ind_t)-1);
        DUMP8("%02x", &app_sound_ctrl->adv_data_info[0], sizeof(sound_ble_adv_data_tws_discover_ind_t));
        p_bt_device->ble_adv_state = SOUND_BLE_ADV_OP_SETUP_STEREO_REQ;
    }
    else if (adv_type == SOUND_FORMATION_TWS_ADV)
    {
        app_sound_ctrl->adv_data_info[0] = sizeof(sound_ble_adv_data_tws_comfirm_dev_t) + 1;
        app_sound_ctrl->adv_data_info[1] = BLE_ADV_DATA_MANUFACTURER_TYPE_ID;
        sound_ble_adv_data_tws_comfirm_dev_t *ble_adv_data = (sound_ble_adv_data_tws_comfirm_dev_t *)&app_sound_ctrl->adv_data_info[2];
        p_tws_adv = &ble_adv_data->tws_info;

        p_tws_adv->operation = SOUND_BLE_ADV_OP_SETUP_STEREO_CFM;
        p_tws_adv->sound_role = SOUND_ROLE_UNKNOW;

        app_sound_full_local_config_info();
        memcpy((uint8_t *)&ble_adv_data->tws_config_info, (uint8_t *)&app_sound_ctrl->tws_config_info, sizeof(sound_tws_config_adv_info_t));
        app_sound_check_local_device_status(&ble_adv_data->device_status_info);
        ble_adv_data->crc_check = crc8_cal(&app_sound_ctrl->adv_data_info[2], sizeof(sound_ble_adv_data_tws_comfirm_dev_t)-1);
        DUMP8("%02x", &app_sound_ctrl->adv_data_info[0], sizeof(sound_ble_adv_data_tws_comfirm_dev_t));

        p_bt_device->ble_adv_state = SOUND_BLE_ADV_OP_SETUP_STEREO_CFM;
    }
    else if (adv_type == SOUND_PERHANDLE_TWS_ADV)
    {
        app_sound_ctrl->adv_data_info[0] = sizeof(sound_adv_data_tws_perhandle_t) + 1;
        app_sound_ctrl->adv_data_info[1] = BLE_ADV_DATA_MANUFACTURER_TYPE_ID;
        sound_adv_data_tws_perhandle_t *ble_adv_data = (sound_adv_data_tws_perhandle_t *)&app_sound_ctrl->adv_data_info[2];
        p_tws_adv = &ble_adv_data->tws_info;

        p_tws_adv->operation = SOUND_BLE_ADV_OP_SETUP_STEREO_PHD;
        p_tws_adv->sound_role = app_sound_ctrl->bt_device_info.tws_info.role;

        memcpy((uint8_t *)&ble_adv_data->tws_config_info, (uint8_t *)&app_sound_ctrl->tws_config_info, sizeof(sound_tws_config_adv_info_t));
        ble_adv_data->crc_check = crc8_cal(&app_sound_ctrl->adv_data_info[2], sizeof(sound_adv_data_tws_perhandle_t)-1);
        DUMP8("%02x", &app_sound_ctrl->adv_data_info[0], sizeof(sound_tws_config_adv_info_t));

        p_bt_device->ble_adv_state = SOUND_BLE_ADV_OP_SETUP_STEREO_PHD;
    }
    else
    {
        return;
    }

    param.adv_data = (uint8_t *)app_sound_ctrl->adv_data_info;
    param.adv_data_size = app_sound_ctrl->adv_data_info[0]+1;

    bes_ble_gap_custom_adv_write_data(&param);
    bes_ble_gap_custom_adv_start(BLE_ADV_ACTIVITY_USER_0);
}

static bool app_sound_ble_confirm_peer_device_match_success(device_addr_t *local_device)
{
    bool ret = false;
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();
    if (!local_device)
    {
        return false;
    }
    if (memcmp((uint8_t *)&p_factory->device_addr, (uint8_t *)local_device, sizeof(device_addr_t)) == 0)
    {
        ret = true;
    }
    return ret;
}

static uint8_t app_sound_decision_local_device_role(sound_ble_adv_data_tws_comfirm_dev_t *peer_adv_cfm)
{
    if (!peer_adv_cfm)
    {
        return SOUND_ROLE_UNKNOW;
    }
    sound_device_status_info_t *peer_status_info = &peer_adv_cfm->device_status_info;
    uint8_t config_role = SOUND_ROLE_UNKNOW;

    sound_device_status_info_t sound_device_status;
    app_sound_check_local_device_status(&sound_device_status);
    LOG_D("app_sound_decision_local_device_role: audio_status=%d audio_status=%d audio_device_type=%d wifi_acl=%d",
            sound_device_status.audio_status,
            sound_device_status.audio_device_type,
            sound_device_status.bt_acl_connected_num,
            sound_device_status.wifi_acl_status);

    LOG_D("app_sound_decision_local_device_role: audio_status=%d audio_status=%d audio_device_type=%d wifi_acl=%d",
            peer_status_info->audio_status,
            peer_status_info->audio_device_type,
            peer_status_info->bt_acl_connected_num,
            peer_status_info->wifi_acl_status);

    if ((sound_device_status.audio_status != SOUND_AUDIO_IDLE) && (peer_status_info->audio_status != SOUND_AUDIO_IDLE))
    {
        if (sound_device_status.audio_status > peer_status_info->audio_status)
        {
            config_role = SOUND_ROLE_MASTER;
        }
        else if (sound_device_status.audio_status < peer_status_info->audio_status)
        {
            config_role = SOUND_ROLE_SLAVE;
        }
        else
        {
            if (sound_device_status.bt_acl_connected_num > peer_status_info->bt_acl_connected_num)
            {
                config_role = SOUND_ROLE_MASTER;
            }
            else if (sound_device_status.bt_acl_connected_num < peer_status_info->bt_acl_connected_num)
            {
                config_role = SOUND_ROLE_SLAVE;
            }
            else
            {
                if (sound_device_status.wifi_acl_status > peer_status_info->wifi_acl_status)
                {
                    config_role = SOUND_ROLE_MASTER;
                }
                else if (sound_device_status.wifi_acl_status < peer_status_info->wifi_acl_status)
                {
                    config_role = SOUND_ROLE_SLAVE;
                }
            }
        }
    }
    else if(peer_status_info->audio_status != SOUND_AUDIO_IDLE)
    {
        config_role = SOUND_ROLE_SLAVE;
    }
    else if(sound_device_status.audio_status != SOUND_AUDIO_IDLE)
    {
        config_role = SOUND_ROLE_MASTER;
    }
    else
    {
        if (sound_device_status.bt_acl_connected_num > peer_status_info->bt_acl_connected_num)
        {
            config_role = SOUND_ROLE_MASTER;
        }
        else if (sound_device_status.bt_acl_connected_num < peer_status_info->bt_acl_connected_num)
        {
            config_role = SOUND_ROLE_SLAVE;
        }
        else
        {
            if (sound_device_status.wifi_acl_status > peer_status_info->wifi_acl_status)
            {
                config_role = SOUND_ROLE_MASTER;
            }
            else if (sound_device_status.wifi_acl_status < peer_status_info->wifi_acl_status)
            {
                config_role = SOUND_ROLE_SLAVE;
            }
        }
    }
    return config_role;
}

static uint8_t app_sound_ble_scan_data_handle_tws_discovery_cfm(sound_ble_adv_data_tws_comfirm_dev_t *adv_data)
{
    app_sound_factory_device_info_t *p_factory = app_sound_get_factory_info();

    sound_ble_adv_data_tws_comfirm_dev_t *adv_data_cfm = adv_data;
    uint8_t local_config_role = SOUND_ROLE_UNKNOW;
    uint8_t crc_value_origin = adv_data_cfm->crc_check;
    uint8_t crc_value_check = crc8_cal((uint8_t *)adv_data, sizeof(sound_ble_adv_data_tws_comfirm_dev_t)-1);
    if (!adv_data)
    {
        return SOUND_ROLE_UNKNOW;
    }

    if (crc_value_origin != crc_value_check)
    {
        LOG_D("scan tws cfm: crc check fail!");
        return local_config_role;
    }
    if (app_sound_ble_confirm_peer_device_match_success(&adv_data_cfm->tws_config_info.slave_addr) == false)
    {
        LOG_D("scan tws cfm: peer device mismatch!");
        return local_config_role;
    }
    local_config_role = app_sound_decision_local_device_role(adv_data_cfm);
    if (local_config_role == SOUND_ROLE_UNKNOW)
    {
        if (memcmp((uint8_t *)&adv_data_cfm->tws_config_info.master_addr, (uint8_t *)&p_factory->device_addr, sizeof(device_addr_t)) > 0)
        {
            local_config_role = SOUND_ROLE_SLAVE;
        }
        else
        {
            local_config_role = SOUND_ROLE_MASTER;
        }
    }
    LOG_D("scan tws cfm: ux_role=%d", local_config_role);
    return local_config_role;
}

static void app_sound_ble_scan_data_report_handler(bes_ble_bdaddr_t *bleAddr, int8_t rssi, uint8_t evt_type, uint8_t *adv_buf, uint8_t len)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    sound_tws_config_adv_info_t *p_tws_config = NULL;
    uint8_t *ptr_section_data = adv_buf;
    uint8_t adv_len = ptr_section_data[0];
    uint8_t adv_type = ptr_section_data[1];
    uint8_t ret = 0;
    if (!adv_buf)
    {
        return;
    }

    if (adv_type == BLE_ADV_DATA_MANUFACTURER_TYPE_ID)
    {
        DUMP8("%02x ", adv_buf, len);

        sound_tws_info_t *cmd_operation = (sound_tws_info_t*)&ptr_section_data[2];
        switch (cmd_operation->operation)
        {
            case SOUND_BLE_ADV_OP_IDLE:
                break;
            case SOUND_BLE_ADV_OP_SETUP_STEREO_REQ:
                if (adv_len == sizeof(sound_ble_adv_data_tws_discover_ind_t) + 1)
                {
                    ret = app_sound_ble_scan_data_handle_tws_discovery_ind(rssi, &ptr_section_data[2], len);
                    if (ret)
                    {
                        if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_DISCOVERY)
                        {
                            app_sound_ui_stop_ble_scanning_device_timer();
                            app_sound_ui_event_handler(SOUND_ACTION_TWS_DISCOVERY, SOUND_UI_NO_ERROR);
                        }
                    }
                }
                break;
            case SOUND_BLE_ADV_OP_SETUP_STEREO_CFM:
                if (adv_len == sizeof(sound_ble_adv_data_tws_comfirm_dev_t) + 1)
                {
                    sound_ble_adv_data_tws_comfirm_dev_t *adv_data_cfm = (sound_ble_adv_data_tws_comfirm_dev_t*)&ptr_section_data[2];
                    ret = app_sound_ble_scan_data_handle_tws_discovery_cfm(adv_data_cfm);
                    if (ret != SOUND_ROLE_UNKNOW)
                    {
                        p_tws_config = &adv_data_cfm->tws_config_info;
                        if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_FORMATION)
                        {
                            if (++app_sound_ctrl->sound_tws_adv_cfg_cnt > SOUND_BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD)
                            {
                                app_sound_scan_formation_handle((SOUND_ROLE_E)ret, (uint8_t *)p_tws_config, true);
                                app_sound_ui_stop_ble_scanning_device_timer();
                                app_sound_ui_event_handler(SOUND_ACTION_TWS_FORMATION, SOUND_UI_NO_ERROR);
                            }
                        }
                        else if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_DISCOVERY)
                        {
                            app_sound_add_ble_scan_tws_discovery_data(&p_tws_config->master_addr);
                            app_sound_ui_event_handler(SOUND_ACTION_TWS_DISCOVERY, SOUND_UI_NO_ERROR);
                        }
                    }
                }
                break;
            case SOUND_BLE_ADV_OP_SETUP_STEREO_PHD:
                if (adv_len == sizeof(sound_adv_data_tws_perhandle_t) + 1)
                {
                    if (app_sound_ble_scan_data_handle_tws_perhandle_ind(&ptr_section_data[2], len))
                    {
                        if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_PREHANDLE)
                        {
                            app_sound_ble_scan_device_stop();
                            app_sound_ui_stop_ble_scanning_device_timer();
                            app_sound_ui_event_handler(SOUND_ACTION_TWS_PREHANDLE, SOUND_UI_NO_ERROR);
                        }
                        else if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_FORMATION)
                        {
                            sound_adv_data_tws_perhandle_t *sound_tws_perhandle = (sound_adv_data_tws_perhandle_t*)&ptr_section_data[2];
                            if (sound_tws_perhandle->tws_info.sound_role != SOUND_ROLE_UNKNOW)
                            {
                                p_tws_config = &sound_tws_perhandle->tws_config_info;
                                if (sound_tws_perhandle->tws_info.sound_role == SOUND_ROLE_MASTER)
                                {
                                    app_sound_scan_formation_handle(SOUND_ROLE_SLAVE, (uint8_t *)p_tws_config, false);
                                }
                                else
                                {
                                    app_sound_scan_formation_handle(SOUND_ROLE_MASTER, (uint8_t *)p_tws_config, false);
                                }
                                app_sound_ui_stop_ble_scanning_device_timer();
                                app_sound_ui_event_handler(SOUND_ACTION_TWS_FORMATION, SOUND_UI_NO_ERROR);
                            }
                        }
                    }
                }
                else
                {

                }
                break;

            case SOUND_BLE_ADV_OP_EXIT_STEREO_REQ:
                break;
            case SOUND_BLE_ADV_OP_EXIT_STEREO_CFM:
                break;

            default:
                break;
        }
    }
}

static void app_sound_ui_set_bt_super_state(SOUND_UI_STATE_E state)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (SOUND_IS_BT_AND_WIFI_DEVICE(app_sound_ctrl->business_device_type))
    {
        app_sound_ctrl->sub_btsuper_state = state;
    }
    else
    {
        app_sound_ctrl->super_state = state;
    }
}

static bool app_sound_ui_tws_is_connected(void)
 {
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();
    bool ret = false;

    if (p_tws_info->connect_status == SOUND_ACL_CONNECTED)
    {
        ret = true;
    }
    return ret;
}

static bool app_sound_ui_has_bt_mobile_connected(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;
    bool ret = false;

    for(uint8_t device_id = 0; device_id < BT_REMOTE_DEVICE_NUM; device_id++)
    {
        p_remote_info = &p_bt_device->remote_info[device_id];
        if (p_remote_info->bredr_acl_state == SOUND_ACL_CONNECTED)
        {
            ret = true;
        }
    }
    return ret;
}

static device_addr_t *app_sound_get_connected_mobile_addr(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_bt_remote_info_t *p_remote_info = NULL;
    device_addr_t *p_device_addr = NULL;
    uint8_t device_id = 0;

    for(; device_id < BT_REMOTE_DEVICE_NUM; device_id++)
    {
        p_remote_info = &p_bt_device->remote_info[device_id];
        if (p_remote_info->bredr_acl_state == SOUND_ACL_CONNECTED)
        {
            p_device_addr = &p_remote_info->remote_addr;
            break;
        }
    }
    return p_device_addr;
}

static uint8_t app_sound_get_nvrecord_count(void)
{
    bt_bdaddr_t device_list[10];
    uint8_t count = 0;
    soundBtGetPairedDeviceList(device_list, 10, &count);
    return count;
}

static bool app_sound_reconnect_mobile_by_nvrecord(bool is_first_mobile)
{
    bool ret = false;
    bt_bdaddr_t device_list[10];
    uint8_t count = 0;
    soundBtGetPairedDeviceList(device_list, 10, &count);
    LOG_I("record count=%d", count);
    if (count == 0)
    {
        return ret;
    }
    if (is_first_mobile)
    {
        if (soundBtConnectDevice(&device_list[0]))
        {
            ret = true;
        }
    }
    else if (count >1)
    {
        device_addr_t *p_connected_addr = app_sound_get_connected_mobile_addr();
        if (p_connected_addr == NULL)
        {
            if (soundBtConnectDevice(&device_list[1]))
            {
                ret = true;
            }
        }
        else if (memcmp((uint8_t *)&device_list[0], (uint8_t *)p_connected_addr, sizeof(device_addr_t)) == 0)
        {
            if (soundBtConnectDevice(&device_list[1]))
            {
                ret = true;
            }
        }
        else
        {
            if (soundBtConnectDevice(&device_list[0]))
            {
                ret = true;
            }
        }
    }
    return ret;
}

static bool app_sound_ui_bt_enable_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info =  &p_bt_device->tws_info;
    bool ret = false;
    uint8_t count = 0;
    if (app_sound_ctrl->active_event != SOUND_UI_POWERON_EVENT)
    {
        return ret;
    }

    app_sound_ctrl->onoff_status = SOUND_ONOFF_OPENED_MODE;
    count = app_sound_get_nvrecord_count();
    LOG_I("nv record count=%d", count);
    /*
    *If there is a pairing record, check whether the connection is successful,
    *and enter the pairing mode if there is no pairing record
    */
    if (count)
    {
        if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
        {
            if (p_tws_info->connect_status != SOUND_ACL_CONNECTED)
            {
                if (app_sound_ctrl->tws_connect_fail_enter_freeman)
                {
                    p_bt_device->mode_type = SOUND_BT_TWS2FREEMAN_MODE;
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_ENTER_FREEMAN);
                    soundEnterFreeman();
                }
                else
                {
                    if (app_sound_ui_has_bt_mobile_connected())
                    {
                        ret = true;
                    }
                    else
                    {
                        if (app_sound_reconnect_mobile_by_nvrecord(true))
                        {
                            app_sound_ui_set_bt_super_state(SOUND_UI_W4_MOBILE_CONNECTED);
                            ret = true;
                        }
                    }
                }
            }
            else
            {
                ret = true;
            }
        }
        else
        {
            if (app_sound_ui_has_bt_mobile_connected())
            {
                ret = true;
            }
            else
            {
                if (app_sound_reconnect_mobile_by_nvrecord(true))
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_MOBILE_CONNECTED);
                    ret = true;
                }
            }
        }
    }
    else
    {
        if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
        {
            if ((app_sound_ui_tws_is_connected() == false) && app_sound_ctrl->tws_connect_fail_enter_freeman)
            {
                p_bt_device->mode_type = SOUND_BT_TWS2FREEMAN_MODE;
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_ENTER_FREEMAN);
                soundEnterFreeman();
            }
            else
            {
                soundBtPairing();
                ret = true;
            }
        }
        else
        {
            soundBtPairing();
            ret = true;
        }
    }
    return ret;
}

bool app_sound_ui_execute_pending_event(void);

void app_sound_bt_ui_common_sm(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    bool new_event_selected = false;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    TRACE(2,"active_event=%d, super_state=%d", app_sound_ctrl->active_event, app_sound_ctrl->super_state);
    switch (app_sound_ctrl->super_state)
    {
        case SOUND_UI_IDLE:
            goto sound_ui_common_sm_end;
            break;

        case SOUND_UI_W4_TWS_DISCONNECT:
            if (action == SOUND_ACTION_TWS_DISCONNECTED)
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_BT_DISABLE;
                soundBtDisable();
            }
            break;

        case SOUND_UI_W4_CLEAR_PAIRING:
            if (app_sound_ctrl->active_event == SOUND_UI_CLEAR_PAIRING_EVENT)
            {
                goto sound_ui_common_sm_end;
            }
            break;

        case SOUND_UI_W4_ENTER_FREEMAN:
            app_sound_ctrl->bt_device_info.mode_type = SOUND_BT_FREEMAN_MODE;
            if (status != SOUND_UI_NO_ERROR)
            {
                goto sound_ui_common_sm_end;
            }
            else
            {
                if (app_sound_ui_has_bt_mobile_connected())
                {
                    goto sound_ui_common_sm_end;
                }

                if (app_sound_get_nvrecord_count() == 0)
                {
                    soundBtPairing();
                    goto sound_ui_common_sm_end;
                }

                if (app_sound_reconnect_mobile_by_nvrecord(true))
                {
                    app_sound_ctrl->super_state = SOUND_UI_W4_MOBILE_CONNECTED;
                }
                else
                {
                    goto sound_ui_common_sm_end;
                }
            }
            break;

        case SOUND_UI_W4_MOBILE_CONNECTED:
            if (action == SOUND_ACTION_MOBILE_CONNECTED)
            {
                if (app_sound_get_nvrecord_count() > 1)
                {
                    if (app_sound_reconnect_mobile_by_nvrecord(false))
                    {
                        app_sound_ctrl->super_state = SOUND_UI_W4_MOBILE_CONNECTED;
                    }
                }
                else
                {
                    goto sound_ui_common_sm_end;
                }
            }
            break;

        default:
            break;

        sound_ui_common_sm_end:
            //app_sound_ble_adv_data_stop();
            //app_sound_ble_scan_device_stop();
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_bt_ui_common_sm, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_open_event_handler
 Description  : Handles actions and states related to open events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_open_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_open_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);
    LOG_I("app_sound_open_event_handler:device_type=%d, mode_type=%d, connect_status=%d",app_sound_ctrl->business_device_type, p_bt_device->mode_type, p_bt_device->tws_info.connect_status);

    switch (app_sound_ctrl->super_state)
    {
        case SOUND_UI_IDLE:
            if (app_sound_ctrl->business_device_type == BT_DEVICE_TYPE)
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_BT_EBABLE;
                soundBtEnable();
            }
            else if (app_sound_ctrl->business_device_type == WIFI_DEVICE_TYPE)
            {
                //call wifi power on
#ifdef WIFI_SOUND_UI_ENABLE
                app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_AIRPLAY_PAIRING, 0);
                soundWifiPowerOn();
#endif
                app_sound_ctrl->onoff_status = SOUND_ONOFF_OPENED_MODE;
                goto sound_ui_open_sm_end;
            }
            else
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_ALL_DEVICE_READY;
                app_sound_ctrl->sub_btsuper_state = SOUND_UI_W4_BT_EBABLE;
                soundBtEnable();
                app_sound_ctrl->sub_wifisuper_state = SOUND_UI_W4_SM_STOP;
                //call wifi power on
#ifdef WIFI_SOUND_UI_ENABLE
                app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_AIRPLAY_PAIRING, 0);
                soundWifiPowerOn();
#endif
            }
            break;
        case SOUND_UI_W4_BT_EBABLE:
            if (app_sound_ui_bt_enable_handle())
            {
                goto sound_ui_open_sm_end;
            }
            break;

        case SOUND_UI_W4_ALL_DEVICE_READY:
            // bt device event handle
            if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_BT_EBABLE)
            {
                if (app_sound_ui_bt_enable_handle())
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                }
            }
            else if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_ENTER_PAIRING)
            {
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
            }
            else if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_ENTER_FREEMAN)
            {
                p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
                if (status != SOUND_UI_NO_ERROR)
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                }
                else
                {
                    if (app_sound_get_nvrecord_count() == 0)
                    {
                        soundBtPairing();
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                        goto all_device_ready_end;
                    }
                    if (app_sound_ui_has_bt_mobile_connected())
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                        goto all_device_ready_end;
                    }
                    if (app_sound_reconnect_mobile_by_nvrecord(true))
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_MOBILE_CONNECTED);
                    }
                    else
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                        goto all_device_ready_end;
                    }
                }
            }
            else if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_MOBILE_CONNECTED)
            {
                uint8_t count = app_sound_get_nvrecord_count();

                if (count > 1)
                {
                    if (app_sound_reconnect_mobile_by_nvrecord(false))
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_MOBILE_CONNECTED);
                    }
                    else
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                    }
                }
                else
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                }
            }

            // wifi device event handle reseved

            // check bt and wifi devices ready
        all_device_ready_end:
            if ((app_sound_ctrl->sub_wifisuper_state == SOUND_UI_W4_SM_STOP) && (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_SM_STOP))
            {
                goto sound_ui_open_sm_end;
            }
            break;

        default:
            app_sound_bt_ui_common_sm(action, status);
            break;

        sound_ui_open_sm_end:
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_open_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d, sub_bt=%d, sub_wifi=%d ", super_state_enter, app_sound_ctrl->super_state, app_sound_ctrl->sub_btsuper_state, app_sound_ctrl->sub_wifisuper_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_close_event_handler
 Description  : Handles actions and states related to close events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_close_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_close_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);
    LOG_I("app_sound_close_event_handler:device_type=%d, mode_type=%d, connect_status=%d",
            app_sound_ctrl->business_device_type,
            p_bt_device->mode_type,
            p_tws_info->connect_status);

    switch (app_sound_ctrl->super_state)
    {
        case SOUND_UI_IDLE:
            if (app_sound_ctrl->business_device_type == BT_DEVICE_TYPE)
            {
                if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
                {
                    app_sound_sync_active_event(app_sound_ctrl->active_event);
                }
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_BT_DISABLE);
                soundBtDisable();
            }
            else if (app_sound_ctrl->business_device_type == WIFI_DEVICE_TYPE)
            {
#ifdef WIFI_SOUND_UI_ENABLE
                soundWifiPowerOff();
#endif
                app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTED_MODE;
                goto sound_ui_close_sm_end;
            }
            else
            {
                if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
                {
                    app_sound_sync_active_event(app_sound_ctrl->active_event);
                    if (p_tws_info->role == SOUND_ROLE_MASTER)
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_BT_DISABLE);
                        soundBtDisable();
                    }
                    else
                    {
                        app_sound_ui_set_bt_super_state(SOUND_UI_W4_TWS_DISCONNECT);
                    }
                }
                else
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_BT_DISABLE);
                    soundBtDisable();
                }
#ifdef WIFI_SOUND_UI_ENABLE
                soundWifiPowerOff();
                app_sound_ctrl->sub_wifisuper_state = SOUND_UI_W4_SM_STOP;
#endif
                app_sound_ctrl->super_state = SOUND_UI_W4_ALL_DEVICE_READY;
            }
            break;

        case SOUND_UI_W4_BT_DISABLE:
            app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTED_MODE;
            goto sound_ui_close_sm_end;
            break;

        case SOUND_UI_W4_ALL_DEVICE_READY:
            // bt device event handle
            if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_BT_DISABLE)
            {
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
            }
            // wifi device event handle reseved

            // check bt and wifi devices ready
            if ((app_sound_ctrl->sub_wifisuper_state == SOUND_UI_W4_SM_STOP) && (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_SM_STOP))
            {
                app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTED_MODE;
                goto sound_ui_close_sm_end;
            }
            break;

        default:
            app_sound_bt_ui_common_sm(action, status);
            break;

        sound_ui_close_sm_end:
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_close_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d, sub_bt=%d, sub_wifi=%d ", super_state_enter, app_sound_ctrl->super_state, app_sound_ctrl->sub_btsuper_state, app_sound_ctrl->sub_wifisuper_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_freeman_to_tws_event_handler
 Description  : Handles actions and states related to freeman2tws events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_freeman_to_tws_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_freeman_to_tws_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);
    LOG_I("app_sound_ui_freeman_to_tws_event_handler:device_type=%d, mode_type=%d, connect_status=%d",
            app_sound_ctrl->business_device_type,
            p_bt_device->mode_type,
            p_tws_info->connect_status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_TWS_MODE_START, 0);
            p_bt_device->mode_type = SOUND_BT_FREEMAN2TWS_MODE;
            app_sound_ctrl->super_state = SOUND_UI_W4_TWS_DISCOVERY;
            app_sound_ble_adv_data_start_init();
            osTimerStart(app_sound_ctrl->sound_adv_timer, 10000);
            app_sound_ble_adv_data_start(SOUND_DISCOVERY_TWS_ADV);
            app_sound_ble_scan_device_start();
            break;

        case SOUND_ACTION_TWS_DISCOVERY:
            if (status == SOUND_UI_NO_ERROR)
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_TWS_FORMATION;
                osTimerStart(app_sound_ctrl->sound_adv_timer, 10000);
                app_sound_ble_adv_data_start(SOUND_FORMATION_TWS_ADV);
            }
            else
            {
                p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
                goto sound_ui_freeman_to_tws_sm_end;
            }
            break;
        case SOUND_ACTION_TWS_FORMATION:
            if (status != SOUND_UI_NO_ERROR)
            {
                goto sound_ui_freeman_to_tws_sm_end;
            }
            else
            {
                p_bt_device->mode_type = SOUND_BT_TWS_MODE;
                app_sound_ctrl->super_state = SOUND_UI_W4_TWS_PREHANDLE;
                osTimerStart(app_sound_ctrl->sound_adv_timer, 10000);
                app_sound_ble_adv_data_start(SOUND_PERHANDLE_TWS_ADV);
            }
            break;
        case SOUND_ACTION_TWS_PREHANDLE:
            if (status != SOUND_UI_NO_ERROR)
            {
                goto sound_ui_freeman_to_tws_sm_end;
            }
            else
            {
                p_bt_device->mode_type = SOUND_BT_TWS_MODE;
                app_sound_ctrl->super_state = SOUND_UI_W4_TWS_CONNECT;
                app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_TWS_MODE_END, 0);
                p_tws_info->tws_reconnect_cnt = 0;
                LOG_I("app_sound_ui_freeman_to_tws_event_handler:mode_type=%d, connect_status=%d", p_bt_device->mode_type, p_tws_info->connect_status);
                soundBtConnectTws();
            }
            break;

        case SOUND_ACTION_TWS_CONNECTE_SIG:
            if (status != SOUND_UI_NO_ERROR)
            {
                p_bt_device->mode_type = SOUND_BT_TWS_MODE;
                app_sound_ctrl->super_state = SOUND_UI_W4_TWS_CONNECT;
                app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_TWS_MODE_END, 0);
                if (++p_tws_info->tws_reconnect_cnt < SOUND_ENTER_TWS_MODE_RECONNECT_TWS_MAX_TIMES)
                {
                    soundBtConnectTws();
                }
                else
                {
                    goto sound_ui_freeman_to_tws_sm_end;
                }
            }
            else
            {
                p_bt_device->mode_type = SOUND_BT_TWS_MODE;
                app_sound_ctrl->super_state = SOUND_UI_W4_TWS_CONNECT;
                goto sound_ui_freeman_to_tws_sm_end;
            }
            break;

        default:
            //app_sound_bt_ui_common_sm(action, status);
            break;

        sound_ui_freeman_to_tws_sm_end:
            LOG_I("app_sound_ui_freeman_to_tws_event_handler:mode_type=%d, connect_status=%d",
                    p_bt_device->mode_type,
                    p_tws_info->connect_status);
            if (p_bt_device->mode_type == SOUND_BT_FREEMAN2TWS_MODE)
            {
                soundEnterFreeman();
            }

            app_sound_ble_adv_data_stop();
            app_sound_ble_scan_device_stop();
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();

            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_freeman_to_tws_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_freeman_to_bis_event_handler
 Description  : Handles actions and states related to freeman2bis events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_freeman_to_bis_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_freeman_to_bis_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_BIS_MODE_START, 0);
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_BIS;
            if (soundEnterBis() == false)
            {
                goto sound_ui_freeman_to_bis_sm_end;
            }
            break;
        case SOUND_ACTION_ENTER_BIS:
            p_bt_device->mode_type = SOUND_BT_BIS_MODE;
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_BIS_MODE_END, 0);
            goto sound_ui_freeman_to_bis_sm_end;
            break;

        default:
            break;
        sound_ui_freeman_to_bis_sm_end:
            LOG_I("app_sound_ui_freeman_to_bis_event_handler:mode_type=%d", p_bt_device->mode_type);
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_freeman_to_bis_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_ibrt_ui_tws_to_freeman_event_handler
 Description  : Handles actions and states related to tws2freeman events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_tws_to_freeman_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_tws_to_freeman_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_START, 0);
            app_sound_sync_active_event(app_sound_ctrl->active_event);
            p_bt_device->mode_type = SOUND_BT_TWS2FREEMAN_MODE;
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_FREEMAN;
            soundEnterFreeman();
            break;
        case SOUND_ACTION_ENTER_FREEMAN:
            {
                bool is_reconnecting = false;
                p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
                app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_END, 0);

                if (app_sound_ui_has_bt_mobile_connected())
                {
                    goto sound_ui_tws2freeman_sm_end;
                }
                if (app_sound_get_nvrecord_count() == 0)
                {
                    soundBtPairing();
                    goto sound_ui_tws2freeman_sm_end;
                }

                is_reconnecting = app_sound_reconnect_mobile_by_nvrecord(true);
                if (is_reconnecting)
                {
                    app_sound_ctrl->super_state = SOUND_UI_W4_MOBILE_CONNECTED;
                }
                else
                {
                    goto sound_ui_tws2freeman_sm_end;
                }
            }
            break;

        default:
            app_sound_bt_ui_common_sm(action, status);
            break;

        sound_ui_tws2freeman_sm_end:
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_tws_to_freeman_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_tws_to_bis_event_handler
 Description  : Handles actions and states related to tws2bis events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_tws_to_bis_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_tws_to_bis_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_BIS_MODE_START, 0);
            app_sound_sync_active_event(app_sound_ctrl->active_event);
            p_bt_device->mode_type = SOUND_BT_TWS2FREEMAN_MODE;
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_FREEMAN;
            soundEnterFreeman();
            break;
        case SOUND_ACTION_ENTER_FREEMAN:
            p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_BIS;
            if (soundEnterBis() == false)
            {
                goto sound_ui_tws_to_bis_sm_end;
            }
            break;
        case SOUND_UI_W4_ENTER_BIS:
            p_bt_device->mode_type = SOUND_BT_BIS_MODE;
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_BIS_MODE_END, 0);
            goto sound_ui_tws_to_bis_sm_end;
            break;

        default:
            //app_sound_bt_ui_common_sm(action, status);
            break;
        sound_ui_tws_to_bis_sm_end:
            LOG_I("app_sound_ui_tws_to_bis_event_handler:mode_type=%d", p_bt_device->mode_type);
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_tws_to_bis_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_bis_to_freeman_event_handler
 Description  : Handles actions and states related to bis2freeman events
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_bis_to_freeman_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_bis_to_freeman_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_START, 0);
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_FREEMAN;
            soundEnterFreeman();
            break;
        case SOUND_ACTION_ENTER_FREEMAN:
            p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_END, 0);
            goto sound_ui_bis_to_freeman_sm_end;
            break;

        default:
            break;
        sound_ui_bis_to_freeman_sm_end:
            LOG_I("app_sound_ui_bis_to_freeman_event_handler:mode_type=%d", p_bt_device->mode_type);
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_bis_to_freeman_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_bis_to_tws_event_handler
 Description  : Handles actions and states related to bis2tws events
                (Note:Reseved, Not yet implemented)
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_bis_to_tws_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_bis_to_tws_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_FREEMAN;
            soundEnterFreeman();
            break;
        case SOUND_ACTION_ENTER_FREEMAN:
            p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
            goto sound_ui_bis_to_freeman_sm_end;
            break;

        default:
            break;
        sound_ui_bis_to_freeman_sm_end:
            LOG_I("app_sound_ui_bis_to_tws_event_handler:mode_type=%d", p_bt_device->mode_type);
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_bis_to_tws_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_factory_reset_event_handler
 Description  : Clear the pairing record of Bluetooth mode, WiFi mode,
                and TWS mode and shut down
 Input        : SOUND_ACTION_E action
                SOUND_UI_ERROR_E status
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/27
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/
void app_sound_ui_factory_reset_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    uint8_t super_state_enter = app_sound_ctrl->super_state;
    bool new_event_selected = false;
    LOG_I("app_sound_ui_factory_reset_event_handler:action=%d, super_state=%d status=%d", action, app_sound_ctrl->super_state, status);

    switch (action)
    {
        case SOUND_ACTION_IDLE:
            app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_POWER_OFF, 0);
            if (app_sound_ctrl->business_device_type == BT_DEVICE_TYPE)
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_BT_DISABLE;
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_BT_DISABLE);
                soundBtDisable();
            }
            else if (app_sound_ctrl->business_device_type == WIFI_DEVICE_TYPE)
            {
                //call wifi power on
#ifdef WIFI_SOUND_UI_ENABLE
                soundWifiPairingModeExit();
#endif
                app_sound_ctrl->onoff_status = SOUND_ONOFF_OPENED_MODE;
                goto sound_ui_factory_reset_sm_end;
            }
            else
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_ALL_DEVICE_READY;
                app_sound_ctrl->sub_btsuper_state = SOUND_UI_W4_BT_DISABLE;
                soundBtDisable();
                //call wifi power on
#ifdef WIFI_SOUND_UI_ENABLE
                soundWifiPairingModeExit();
#endif
            }
            break;

        case SOUND_ACTION_BT_DISABLE_EVENT:
            soundBtCleanPairedDeviceList();
            if (p_bt_device->mode_type != SOUND_BT_FREEMAN_MODE)
            {
                app_sound_ctrl->super_state = SOUND_UI_W4_ENTER_FREEMAN;
                soundEnterFreeman();
            }
            else
            {
                goto  sound_ui_factory_reset_sm_end;
            }
            break;

        case SOUND_UI_W4_ENTER_FREEMAN:
            p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
            soundBtPairing();
            goto sound_ui_factory_reset_sm_end;
            break;

        case SOUND_UI_W4_ALL_DEVICE_READY:
            // bt device event handle
            if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_TWS_DISCONNECT)
            {
                soundBtCleanPairedDeviceList();
                if (p_bt_device->mode_type != SOUND_BT_FREEMAN_MODE)
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_ENTER_FREEMAN);
                    soundEnterFreeman();
                }
                else
                {
                    app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
                }
            }
            else if (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_ENTER_FREEMAN)
            {
                app_sound_ui_set_bt_super_state(SOUND_UI_W4_SM_STOP);
            }
            // wifi device event handle reseved

            // check bt and wifi devices ready
            if ((app_sound_ctrl->sub_wifisuper_state == SOUND_UI_W4_SM_STOP) && (app_sound_ctrl->sub_btsuper_state == SOUND_UI_W4_SM_STOP))
            {
                goto sound_ui_factory_reset_sm_end;
            }
            break;

        default:
            //app_sound_bt_ui_common_sm(action, status);
            break;
        sound_ui_factory_reset_sm_end:
            LOG_I("app_sound_ui_factory_reset_event_handler:mode_type=%d", p_bt_device->mode_type);
            app_sound_ctrl->sm_running = false;
            app_sound_ctrl->super_state = SOUND_UI_W4_SM_STOP;
            new_event_selected = app_sound_ui_execute_pending_event();
            break;
    }

    if (!new_event_selected)
    {
        LOG_I("entry=app_sound_ui_factory_reset_event_handler, action=%d, status=0x%x", action, status);
        LOG_I("enter=%d, exit=%d", super_state_enter, app_sound_ctrl->super_state);
    }
}

bool app_sound_false_trigger_protect(SOUND_UI_TYPE_E evt_type)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    bool ret = false;
    if (app_sound_ctrl->active_event == evt_type)
    {
        switch(evt_type)
        {
            case SOUND_UI_POWERON_EVENT:
            case SOUND_UI_POWEROFF_EVENT:
            case SOUND_UI_FREEMAN2TWS_EVENT:
            case SOUND_UI_FREEMAN2TBIS_EVENT:
            case SOUND_UI_TWS2FREEMAN_EVENT:
            case SOUND_UI_TWS2BIS_EVENT:
            case SOUND_UI_BIS2FREEMAN_EVENT:
            case SOUND_UI_BIS2TWS_EVENT:
                    ret = true;
                    break;
                default :
                    break;
        }
    }
    return ret;
}

/*****************************************************************************
 Prototype    : app_sound_ui_event_entry
 Description  : Sound ui events handle
 Input        : SOUND_UI_TYPE_E event
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/219
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_ui_event_entry(SOUND_UI_TYPE_E event)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (app_sound_false_trigger_protect(event))
    {
        LOG_I("entry event false trigger %d", event);
        return;
    }

    if ((event == SOUND_UI_POWEROFF_EVENT) || \
        (event == SOUND_UI_FACTORY_RESET_EVENT) || \
        (event == SOUND_UI_CLEAR_PAIRING_EVENT))
    {
        app_sound_ui_event_queue_init();
        if (app_sound_ctrl->sm_running)
        {
            app_sound_ctrl->sm_running = false;
            LOG_I("force reset,reason event=%d", event);
        }
    }

    if (app_sound_ctrl->sm_running)
    {
        SOUND_UI_ERROR_E status = app_sound_ui_event_queue_insert(event);
        LOG_I("entry return directly:sm_run=%d, event=%d, status=%d", app_sound_ctrl->sm_running, event, status);
        return;
    }

    app_sound_ctrl->active_event = event;
    app_sound_ctrl->super_state = SOUND_UI_IDLE;
    app_sound_ctrl->wait_state = SOUND_UI_IDLE;
    app_sound_ctrl->sm_running = true;
    app_sound_ctrl->sub_btsuper_state = SOUND_UI_IDLE;
    app_sound_ctrl->sub_wifisuper_state = SOUND_UI_IDLE;

    app_sound_checker_timer_manager(SOUND_TIMER_UI_EVENT, 0, false);
    app_sound_ui_event_show_in_led(event, SOUND_UI_NO_ERROR);

    switch(event)
    {
        case SOUND_UI_POWERON_EVENT:
            app_sound_ctrl->onoff_status = SOUND_ONOFF_OPENING;
            app_sound_bt_init();
            app_sound_open_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_POWEROFF_EVENT:
            app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTING;
            app_sound_close_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;

        case SOUND_UI_FREEMAN2TWS_EVENT:
            app_sound_ui_freeman_to_tws_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_FREEMAN2TBIS_EVENT:
            app_sound_ui_freeman_to_bis_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_TWS2FREEMAN_EVENT:
            app_sound_tws_to_freeman_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_TWS2BIS_EVENT:
            app_sound_ui_tws_to_bis_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_BIS2FREEMAN_EVENT:
            app_sound_ui_bis_to_freeman_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;
        case SOUND_UI_BIS2TWS_EVENT:
            app_sound_ui_bis_to_tws_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;

        case SOUND_UI_FACTORY_RESET_EVENT:
            app_sound_ui_factory_reset_event_handler(SOUND_ACTION_IDLE, SOUND_UI_NO_ERROR);
            break;

        default :
            break;
    }
}

static bool app_sound_ui_action_state_mismatch_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    bool ret = true;
    switch (action)
    {
        case SOUND_ACTION_IDLE:
            break;
        case SOUND_ACTION_BT_ENABLE_EVENT:
            break;
        case SOUND_ACTION_BT_DISABLE_EVENT:
            break;
        case SOUND_ACTION_BT_DISCONNECT_ALL:
            break;
        case SOUND_ACTION_TWS_DISCONNECTED:
            break;
        case SOUND_ACTION_TWS_CONNECTE_SIG:
            if (app_sound_ctrl->super_state == SOUND_UI_W4_TWS_PREHANDLE)
            {
                app_sound_ble_adv_data_stop();
                app_sound_ble_scan_device_stop();
                ret = false;
            }
            break;
        case SOUND_ACTION_BT_ENTERPAIRING_EVENT:
            break;
        case SOUND_ACTION_BT_CLEAR_PAIRING_EVENT:
            break;
        case SOUND_ACTION_TWS_DISCOVERY:
            break;
        case SOUND_ACTION_TWS_FORMATION:
            break;
        case SOUND_ACTION_TWS_PREHANDLE:
            break;
        case SOUND_ACTION_MOBILE_CONNECTED:
            break;
        case SOUND_ACTION_MOBILE_DISCONNECTED:
            break;
        case SOUND_ACTION_ENTER_FREEMAN:
            break;
        case SOUND_ACTION_ENTER_BIS:
            break;
        case SOUND_ACTION_W4_ALL_DEVICE_READY:
            break;
        case SOUND_ACTION_SM_STOP:
            break;
        default:
            break;
    }
    return ret;
}

extern  osThreadId besbt_tid;
void app_sound_ui_event_handler(SOUND_ACTION_E action, SOUND_UI_ERROR_E status)
{
    LOG_I("app_sound_ui_event_handler: action=%d status=%d!", action, status);

    if(osThreadGetId() != besbt_tid)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)action,(uint32_t)status,(uint32_t)app_sound_ui_event_handler);
        return;
    }
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if ((app_sound_ctrl->super_state != (SOUND_UI_STATE_E)action) && (app_sound_ctrl->sub_btsuper_state != (SOUND_UI_STATE_E)action))
    {
        if (app_sound_ui_action_state_mismatch_handler(action, status))
        {
            LOG_I("error action, action=%d, super_state=%d , sub_bt=%d, sub_wifi=%d",\
                              action, app_sound_ctrl->super_state, app_sound_ctrl->sub_btsuper_state, app_sound_ctrl->sub_wifisuper_state);
            return;
        }
    }

    switch (app_sound_ctrl->active_event)
    {
        case SOUND_UI_POWERON_EVENT:
            app_sound_open_event_handler(action,status);
            break;

        case SOUND_UI_POWEROFF_EVENT:
            app_sound_close_event_handler(action,status);
            break;

        case SOUND_UI_FREEMAN2TWS_EVENT:
            app_sound_ui_freeman_to_tws_event_handler(action,status);
            break;

        case SOUND_UI_FREEMAN2TBIS_EVENT:
            app_sound_ui_freeman_to_bis_event_handler(action,status);
            break;
        case SOUND_UI_TWS2FREEMAN_EVENT:
            app_sound_tws_to_freeman_event_handler(action,status);
            break;
        case SOUND_UI_TWS2BIS_EVENT:
            app_sound_ui_tws_to_bis_event_handler(action,status);
            break;
        case SOUND_UI_BIS2FREEMAN_EVENT:
            app_sound_ui_bis_to_freeman_event_handler(action,status);
            break;
        case SOUND_UI_BIS2TWS_EVENT:
            app_sound_ui_bis_to_tws_event_handler(action,status);
            break;

        case SOUND_UI_FACTORY_RESET_EVENT:
            app_sound_ui_factory_reset_event_handler(action,status);
            break;

        default:
            LOG_I("UI event unknown %d", app_sound_ctrl->active_event);
            break;
    }
}

bool app_sound_ui_execute_pending_event(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    bool new_event_selected = false;

    app_sound_ctrl->super_state = SOUND_UI_IDLE;
    app_sound_ctrl->sm_running = false;
    app_sound_ctrl->active_event = SOUND_ACTION_IDLE;

    //Go on do next pending event

    SOUND_UI_TYPE_E front_evt;
    SOUND_UI_ERROR_E status = app_sound_ui_event_queue_get_front(&front_evt);

    if (SOUND_UI_NO_ERROR == status)
    {
        app_sound_ui_event_queue_delete(&front_evt);
        LOG_I("[IO] execute_pending_event");
        app_sound_ui_event_entry(front_evt);
        if (SOUND_UI_NONE_EVENT != front_evt)
        {
            //select new pending event
            new_event_selected = true;
            LOG_I("execute_pending_event,active event=%d", app_sound_ctrl->active_event);
        }
    }

    return new_event_selected;
}

/*****************************************************************************
 Prototype    : app_sound_ui_clear_pairing_mode_event
 Description  : clear pairing mode event handle
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_ui_clear_pairing_recoder_event(bool is_sync_event)
{
    LOG_I("app_sound_ui_clear_pairing_recoder_event!");
    if (app_sound_ui_has_bt_mobile_connected() || app_sound_ui_tws_is_connected())
    {
        if (!is_sync_event)
        {
            app_sound_sync_active_event(SOUND_CUSTOM_BT_LONG_PRESS_EVENT);
        }
        soundBtDisconnectAll();
    }

    app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_CLEAR_PAIRING_MODE, 0);
    soundBtCleanPairedDeviceList();
}

/*****************************************************************************
 Prototype    : app_sound_ui_enter_pairing_mode_event
 Description  : enter pairing mode event handle
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_ui_enter_pairing_mode_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();
    app_sound_tws_info_t *p_tws_info = app_sound_get_tws_info();

    app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_BT_ENTER_PAIRING_MODE, 0);
    if ((p_bt_device->mode_type == SOUND_BT_TWS_MODE) && (p_tws_info->connect_status == SOUND_ACL_CONNECTED))
    {
        if (p_tws_info->role == SOUND_ROLE_MASTER)
        {
            soundBtPairing();
        }
    }
    else
    {
        soundBtPairing();
    }
}

/*****************************************************************************
 Prototype    : app_sound_ui_freeman_to_tws_event
 Description  : exit freeman mode, enter tws mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_ui_freeman_to_tws_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_FREEMAN_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }

    app_sound_ui_event_entry(SOUND_UI_FREEMAN2TWS_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_ui_freeman_to_bis_event
 Description  : exit freeman mode, enter bis mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_ui_freeman_to_bis_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_FREEMAN_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_FREEMAN2TBIS_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_ui_tws_to_freeman_event
 Description  : exit tws mode, enter freeman mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_ui_tws_to_freeman_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_TWS_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_TWS2FREEMAN_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_ui_tws_to_bis_event
 Description  : exit tws mode, enter bis mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_ui_tws_to_bis_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_TWS_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_TWS2BIS_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_ui_bis_to_freeman_event
 Description  : exit bis mode, enter freeman mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_ui_bis_to_freeman_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_BIS_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_BIS2FREEMAN_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_ui_bis_to_tws_event
 Description  : exit bis mode, enter tws mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_ui_bis_to_tws_event(void)
{
    app_sound_bt_device_info_t *p_bt_device = app_sound_get_bt_device_info();

    if (p_bt_device->mode_type != SOUND_BT_BIS_MODE)
    {
        LOG_I("bt mode error!");
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_BIS2TWS_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_power_key_click_handle
 Description  : power key click event handle,reserved
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_power_key_click_handle(void)
{
    LOG_D("%s: Not yet implemented!", __func__);
}

/*****************************************************************************
 Prototype    : app_sound_power_key_double_click_handle
 Description  : In Freeman mode, double-click the Power key to enter TWS mode;
                In TWS mode, double-click the Power key to enter Freeman mode;
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/20
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_power_key_double_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    if (app_sound_ctrl->onoff_status < SOUND_ONOFF_OPENING)
    {
        return;
    }

    if (SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type))
    {
        if (p_bt_device->mode_type == SOUND_BT_FREEMAN_MODE)
        {
            app_sound_ui_freeman_to_tws_event();
        }
        else if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
        {
            app_sound_ui_tws_to_freeman_event();
        }
    }
}

/*****************************************************************************
 Prototype    : app_sound_power_key_long_press_handle
 Description  : poweron event handler power on/off
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_power_key_long_press_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type))
    {
        if ((app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE) || (app_sound_ctrl->active_event == SOUND_UI_POWEROFF_EVENT))
        {
            app_sound_ui_event_entry(SOUND_UI_POWERON_EVENT);
        }
        else
        {
            app_sound_ui_event_entry(SOUND_UI_POWEROFF_EVENT);
        }
    }
    if (SOUND_ONLY_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type))
    {
#ifdef WIFI_SOUND_UI_ENABLE
        if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
        {
            soundWifiPowerOn();
            app_sound_ctrl->onoff_status = SOUND_ONOFF_OPENED_MODE;
        }
        else
        {
            soundWifiPowerOff();
            app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTED_MODE;
        }
#endif
    }
}

/*****************************************************************************
 Prototype    : app_sound_bt_key_long_click_handle
 Description  : bt key click  event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_bt_key_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }

    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        app_sound_ui_enter_pairing_mode_event();
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
        app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_WIFI_SWITCH_BT, 0);
    }
}

/*****************************************************************************
 Prototype    : app_sound_bt_key_long_click_handle
 Description  : bt key double click  event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_bt_key_double_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }

    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        soundEnableMultipoint();
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
        app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_WIFI_SWITCH_BT, 0);
    }
}

/*****************************************************************************
 Prototype    : app_sound_bt_key_long_click_handle
 Description  : bt key three click  event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_bt_key_three_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }
    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        if (p_bt_device->mode_type == SOUND_BT_FREEMAN_MODE)
        {
            app_sound_ui_freeman_to_bis_event();
        }
        else if (p_bt_device->mode_type == SOUND_BT_TWS_MODE)
        {
            app_sound_ui_tws_to_bis_event();
        }
        else if (p_bt_device->mode_type == SOUND_BT_BIS_MODE)
        {
            app_sound_ui_bis_to_freeman_event();
        }
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
        app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_WIFI_SWITCH_BT, 0);
    }
}

/*****************************************************************************
 Prototype    : app_sound_bt_key_long_click_handle
 Description  : bt key long click  event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_bt_key_long_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }

    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        app_sound_ui_clear_pairing_recoder_event(false);
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
        app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_WIFI_SWITCH_BT, 0);
    }
}

/*****************************************************************************
 Prototype    : app_sound_wifi_key_click_handle
 Description  : switch to wifi business
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/26
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_wifi_key_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }
    //first wifi disconnect,second clear wifi pair record
    if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
#ifdef WIFI_SOUND_UI_ENABLE
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_AIRPLAY_PAIRING, 0);
        soundWifiPairingModeEntry();
#endif
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        app_sound_ctrl->business_mode = WIFI_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_AIRPLAY_PAIRING, 0);
    }
}

/*****************************************************************************
 Prototype    : app_sound_wifi_key_double_click_handle
 Description  : reseved
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/28
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_wifi_key_double_click_handle(void)
{
    LOG_D("%s: Not yet implemented!", __func__);
}

/*****************************************************************************
 Prototype    : app_sound_wifi_key_long_click_handle
 Description  : clear wifi pair record
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/26
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_wifi_key_long_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }
    //first wifi disconnect,second clear wifi pair record
    if (SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type))
    {
        app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
        app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_CLEAR_WIFI_NV_RECORD, 0);
#ifdef WIFI_SOUND_UI_ENABLE
        soundWifiPairingModeExit();
#endif
    }
}

/*****************************************************************************
 Prototype    : app_sound_circle_click_handle
 Description  : circle click event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_circle_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_wifi_device_info_t *p_wifi_device_info = &app_sound_ctrl->wifi_device_info;
    uint8_t active_id = p_bt_device->active_device_id;
    app_sound_bt_remote_info_t *p_remote_info = &p_bt_device->remote_info[active_id];
    app_sound_tws_info_t *p_tws_info = &p_bt_device->tws_info;

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }

    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        LOG_I("active_device_id=%d hfp_state=%d a2dp_state=%d!", p_bt_device->active_device_id,
                                p_remote_info->hfp_state,
                                p_remote_info->a2dp_state);

        if ((p_bt_device->mode_type == SOUND_BT_TWS_MODE) && (p_tws_info->role == SOUND_ROLE_SLAVE))
        {
            app_sound_sync_active_event(SOUND_CUSTOM_CIRCLE_CLICK_EVENT);
        }

        if (p_remote_info->hfp_state == SOUND_BT_HFP_INCOMING_CALL)
        {
            //call bt hfp answer API
            app_audio_control_call_answer();
        }
        else if (p_remote_info->a2dp_state == SOUND_BT_A2DP_STREAM_STARTED)
        {
            //call bt music play API
            app_audio_control_media_pause();
        }
        else if ((p_remote_info->a2dp_state == SOUND_BT_A2DP_STREAM_CLOSE) || \
                    (p_remote_info->a2dp_state == SOUND_BT_A2DP_CONNECTED))
        {
            //call bt music play API
            app_audio_control_media_play();
        }
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
        LOG_I("state=%d !", p_wifi_device_info->profile_state);
        if (p_wifi_device_info->profile_state == SOUND_WIFI_AIRPLAY_STREAM_STARTED)
        {
            //call wifi music stop API
#ifdef WIFI_SOUND_UI_ENABLE
            soundWifiAirplayPause();
#endif
        }
        else if (p_wifi_device_info->profile_state == SOUND_WIFI_AIRPLAY_STREAM_CLOSE)
        {
            //call wifi music play API
#ifdef WIFI_SOUND_UI_ENABLE
            soundWifiAirplayPlay();
#endif
        }
    }
}

/*****************************************************************************
 Prototype    : app_sound_circle_double_click_handle
 Description  : circle double click event handler
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_circle_double_click_handle(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info = &p_bt_device->tws_info;
    app_sound_bt_remote_info_t *p_remote_info = &p_bt_device->remote_info[p_bt_device->active_device_id];

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }
    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        if ((p_bt_device->mode_type == SOUND_BT_TWS_MODE) && (p_tws_info->role == SOUND_ROLE_SLAVE))
        {
            app_sound_sync_active_event(SOUND_CUSTOM_CIRCLE_DOUBLE_CLICK_EVENT);
        }

        if (p_remote_info->hfp_state > SOUND_BT_HFP_CONNECTED)
        {
            app_audio_control_call_terminate();
        }
    }
}

/*****************************************************************************
 Prototype    : app_sound_vol_ctrl_event
 Description  : volum ctrl handle
 Input        : bool direction
                    true: add vol
                    false:sub vol
                uint8_t offset_vol_level : offset vol level
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_vol_ctrl_event(bool direction,uint8_t offset_vol_level)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info = &p_bt_device->tws_info;

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s sound is shut down status!", __func__);
        return;
    }

    if (app_sound_ctrl->aux_in)
    {
        //call aux playback api
        LOG_I("%s:current aux_in insert status!", __func__);
        if (direction)
        {
            audio_player_volume_up();
        }
        else
        {
            audio_player_volume_down();
        }
        return;
    }

    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        if ((p_bt_device->mode_type == SOUND_BT_TWS_MODE) && (p_tws_info->role == SOUND_ROLE_SLAVE))
        {
            if (direction)
            {
                app_sound_sync_active_event(SOUND_CUSTOM_VOLUME_UP_EVENT);
            }
            else
            {
                app_sound_sync_active_event(SOUND_CUSTOM_VOLUME_DOWN_EVENT);
            }
        }
        else
        {
            if (direction)
            {
                app_audio_control_streaming_volume_up();
            }
            else
            {
                app_audio_control_streaming_volume_down();
            }
        }
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
#ifdef WIFI_SOUND_UI_ENABLE
        if (direction)
        {
            soundWifiAirplayVolUp();
        }
        else
        {
            soundWifiAirplayVolDown();
        }
#endif
    }
}

/*****************************************************************************
 Prototype    : app_sound_playback_direction
 Description  : Controls the music that is played
 Input        : bool direction
                true: Perform playback of the previous music
                false:Perform playback of a piece of music after playing
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/19
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_playback_direction(bool direction)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_tws_info_t *p_tws_info = &p_bt_device->tws_info;

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s sound is shut down status!", __func__);
        return;
    }
    if (app_sound_ctrl->aux_in)
    {
        //call aux playback api
        LOG_I("%s:current aux_in insert status!", __func__);
        return;
    }
    if ((SOUND_IS_BT_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == BT_DEVICE_TYPE))
    {
        if ((p_bt_device->mode_type == SOUND_BT_TWS_MODE) && (p_tws_info->role == SOUND_ROLE_SLAVE))
        {
            if (direction)
            {
                app_sound_sync_active_event(SOUND_CUSTOM_CONTROL_MEDIA_FORWARD_EVENT);
            }
            else
            {
                app_sound_sync_active_event(SOUND_CUSTOM_CONTROL_MEDIA_BACKWARD_EVENT);
            }
        }
        else
        {
            if (direction)
            {
                app_audio_control_media_forward();
            }
            else
            {
                app_audio_control_media_backward();
            }
        }
    }
    else if ((SOUND_IS_WIFI_DEVICE(app_sound_ctrl->business_device_type)) && (app_sound_ctrl->business_mode == WIFI_DEVICE_TYPE))
    {
#ifdef WIFI_SOUND_UI_ENABLE
        if (direction)
        {
            soundWifiAirplayPrevious();
        }
        else
        {
            soundWifiAirplayNext();
        }
#endif
    }
}

/*****************************************************************************
 Prototype    : app_sound_bt_and_power
 Description  : Clear the pairing record of Bluetooth mode, WiFi mode,
                and TWS mode and shut down
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/27
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_bt_and_power(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();

    if (app_sound_ctrl->onoff_status == SOUND_ONOFF_SHOUTED_MODE)
    {
        LOG_I("%s: sound is shut down status!", __func__);
        return;
    }
    app_sound_ui_event_entry(SOUND_UI_FACTORY_RESET_EVENT);
}

/*****************************************************************************
 Prototype    : app_sound_circle_and_power
 Description  : Query the current version
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/27
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_circle_and_power(void)
{
    LOG_D("%s: Not yet implemented!", __func__);
    app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_SDK_VERSION, 0);
}

/*****************************************************************************
 Prototype    : app_sound_aux_in
 Description  : aux in mode
 Input        : None
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/27
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

static void app_sound_aux_in(bool aux_in)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_ctrl->aux_in = aux_in;
    app_sound_checker_timer_manager(SOUND_TIMER_AUX_STREAMING, 0, aux_in);
}


/*****************************************************************************
 Prototype    : app_sound_key_handle
 Description  : key events handle
 Input        : SOUND_EVENT_TYPE sound_key
 Output       : None
 Return Value :
 Calls        :
 Called By    :

 History        :
 Date         : 2023/6/12
 Author       : bestechnic
 Modification : Created function

*****************************************************************************/

void app_sound_key_handle(SOUND_EVENT_TYPE sound_key)
{
    switch(sound_key)
    {
        case SOUND_POWER_KEY_CLICK:
            app_sound_power_key_click_handle();
            break;
        case SOUND_POWER_KEY_DOUBLE_CLICK:
            app_sound_power_key_double_click_handle();
            break;

        case SOUND_POWER_KEY_LONG_PRESS:
            app_sound_power_key_long_press_handle();
            break;

        case SOUND_BT_KEY_CLICK:
            app_sound_bt_key_click_handle();
            break;

        case SOUND_BT_KEY_DOUBLE_CLICK:
            app_sound_bt_key_double_click_handle();
            break;

        case SOUND_BT_KEY_THREE_CLICK:
            app_sound_bt_key_three_click_handle();
            break;

        case SOUND_BT_KEY_LONG_PRESS:
            app_sound_bt_key_long_click_handle();
            break;

        case SOUND_WIFI_KEY_CLICK:
            app_sound_wifi_key_click_handle();
            break;

        case SOUND_WIFI_KEY_DOUBLE_CLICK:
            app_sound_wifi_key_double_click_handle();
            break;

        case SOUND_WIFI_KEY_LONG_PRESS:
            app_sound_wifi_key_long_click_handle();
            break;

        case SOUND_CIRCLE_KEY_CLICK:
            app_sound_circle_click_handle();
            break;

        case SOUND_CIRCLE_KEY_DOUBLE_CLICK:
            app_sound_circle_double_click_handle();
            break;

        case SOUND_CIRCLE_UP_KEY_CLICK:
            app_sound_vol_ctrl_event(true, 1);
            break;

        case SOUND_CIRCLE_DOWN_KEY_CLICK:
            app_sound_vol_ctrl_event(false, 1);
            break;

        case SOUND_CIRCLE_LEFT_KEY_CLICK:
            app_sound_playback_direction(true);
            break;

        case SOUND_CIRCLE_RIGHT_KEY_CLICK:
            app_sound_playback_direction(false);
            break;

        case SOUND_BT_AND_POWER:
            app_sound_bt_and_power();
            break;

        case SOUND_CIRCLE_AND_POWER:
            app_sound_circle_and_power();
            break;

        case SOUND_AUX_IN_INSERT:
            app_sound_aux_in(true);
            break;

        case SOUND_AUX_IN_EXTRACT:
            app_sound_aux_in(false);
            break;

        default:
            break;
    }
}

void app_sound_bt_init(void)
{
    uint8_t *sound_name = NULL;
    app_sound_bt_remote_info_t *p_remote_info = NULL;

    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_bt_device_info_t *p_bt_device = &app_sound_ctrl->bt_device_info;
    app_sound_factory_device_info_t *p_factory = &app_sound_ctrl->factory_device_info;
    app_sound_tws_info_t *p_tws_info = &p_bt_device->tws_info;

    for (uint8_t i = 0 ; i < BT_REMOTE_DEVICE_NUM; i++)
    {
        p_remote_info = &p_bt_device->remote_info[i];
        p_remote_info->is_used = false;
        memset(&p_remote_info->remote_addr, 0, sizeof(device_addr_t));
        p_remote_info->bredr_acl_state = SOUND_ACL_DISCONNECTED;
        p_remote_info->a2dp_state = SOUND_BT_A2DP_DISCONNECTED;
        p_remote_info->hfp_state = SOUND_BT_HFP_DISCONNECTED;
        p_remote_info->sco_state = SOUND_BT_SCO_DISCONNECTED;
    }
#if defined(NEW_NV_RECORD_ENABLED)
    if (nv_record_tws_is_enabled() == false)
    {
        p_bt_device->mode_type = SOUND_BT_FREEMAN_MODE;
    }
    else
    {
        p_bt_device->mode_type = SOUND_BT_TWS_MODE;
    }
#endif

    p_bt_device->active_device_id = BT_DEVICE_ID_UNKNOWN;

    factory_section_original_btaddr_get(&p_factory->device_addr.addr[0]);
    memset(&p_factory->device_name, 0, SOUND_NAME_MAX_LEN);
    sound_name = factory_section_get_bt_name();
    memcpy(&p_factory->device_name[0], sound_name, SOUND_NAME_MAX_LEN);
    p_factory->name_len = strlen((char *)&p_factory->device_name[0]);

    p_tws_info->connect_status = SOUND_ACL_DISCONNECTED;
    p_tws_info->role = SOUND_ROLE_UNKNOW;

    soundTwsRegisterRecvDataCallback(app_sound_sync_info_handle);

    LOG_I("app_sound_bt_init: mode_type is=%d!", p_bt_device->mode_type);
}

void app_sound_init(void)
{
    struct app_sound_ctrl_t *app_sound_ctrl = app_sound_get_ctrl();
    app_sound_wifi_device_info_t *p_wifi_device = app_sound_get_wifi_device_info();
    osTimerAttr_t timerattr;

    if (sound_is_initialized)
    {
        return;
    }
    sound_is_initialized = true;

#if defined(APP_SOUND_ENABLE) && defined(WIFI_SOUND_UI_ENABLE)
    app_sound_ctrl->business_device_type = BT_AND_WIFI_DEVICE_TYPE;
    app_sound_ctrl->business_mode = WIFI_DEVICE_TYPE;
#elif  defined(APP_SOUND_ENABLE)
    app_sound_ctrl->business_device_type = BT_DEVICE_TYPE;
    app_sound_ctrl->business_mode = BT_DEVICE_TYPE;
#elif  defined(WIFI_SOUND_UI_ENABLE)
    app_sound_ctrl->business_device_type = WIFI_DEVICE_TYPE;
    app_sound_ctrl->business_mode = WIFI_DEVICE_TYPE;
#endif

    app_sound_ctrl->onoff_status = SOUND_ONOFF_SHOUTED_MODE;
    app_sound_ctrl->tws_connect_fail_enter_freeman = true;

    p_wifi_device->acl_state = SOUND_ACL_DISCONNECTED;
    p_wifi_device->profile_state = SOUND_WIFI_AIRPLAY_DISCONNECTED;
    app_sound_bt_init();

    memset(&timerattr, 0, sizeof(timerattr));
    timerattr.name = "SOUND_CHECKER_TIMER";
    app_sound_ctrl->sound_timer = osTimerNew(app_sound_timer_checker_callback, osTimerOnce, NULL, &timerattr);
    app_sound_ctrl->sound_timer_start = false;
    if (app_sound_ctrl->sound_timer)
    {
        osTimerStart(app_sound_ctrl->sound_timer, SOUND_WAIT_SHUTDOWN_TIMEOUT);
        app_sound_ctrl->sound_timer_start = true;
    }

    memset(&timerattr, 0, sizeof(timerattr));
    timerattr.name = "SOUND_BLE_ADV_DEVICE_TIMER";
    app_sound_ctrl->sound_adv_timer = osTimerNew(app_sound_ui_wait_ble_scanning_device_timer_cb, osTimerOnce, NULL, &timerattr);
    if (app_sound_ctrl->sound_adv_timer)
    {
        LOG_I("sound adv timer creat success!");
    }

    bes_ble_gap_adv_report_callback_register(app_sound_ble_scan_data_report_handler);
    app_sound_ui_event_queue_init();
}

void app_sound_ui_test_key_io_event(APP_KEY_STATUS *status, void *param)
{
    if (!status || !param)
    {
        return;
    }

    switch(status->event)
    {
        case APP_KEY_EVENT_CLICK:
            if (status->code== APP_KEY_CODE_PWR)
            {
                app_sound_key_handle(SOUND_POWER_KEY_CLICK);
            }
            else if (status->code== APP_KEY_CODE_FN1)
            {
                app_sound_key_handle(SOUND_BT_KEY_CLICK);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_sound_key_handle(SOUND_WIFI_KEY_CLICK);
            }
            else
            {
            }
            break;

        case APP_KEY_EVENT_DOUBLECLICK:
            if (status->code== APP_KEY_CODE_PWR)
            {
                app_sound_key_handle(SOUND_POWER_KEY_DOUBLE_CLICK);
            }
            else if (status->code== APP_KEY_CODE_FN1)
            {
                app_sound_key_handle(SOUND_BT_KEY_DOUBLE_CLICK);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_sound_key_handle(SOUND_WIFI_KEY_DOUBLE_CLICK);
            }
            else
            {

            }
            break;

        case APP_KEY_EVENT_LONGPRESS:
            if (status->code== APP_KEY_CODE_PWR)
            {
                app_sound_key_handle(SOUND_POWER_KEY_LONG_PRESS);
            }
            else if (status->code== APP_KEY_CODE_FN1)
            {
                app_sound_key_handle(SOUND_BT_KEY_LONG_PRESS);
            }
            else if (status->code== APP_KEY_CODE_FN2)
            {
                app_sound_key_handle(SOUND_WIFI_KEY_LONG_PRESS);
            }
            else
            {
            }
            break;

        case APP_KEY_EVENT_TRIPLECLICK:
            if (status->code== APP_KEY_CODE_FN1)
            {
                app_sound_key_handle(SOUND_BT_KEY_THREE_CLICK);
            }
            else
            {
            }
            break;

        case HAL_KEY_EVENT_LONGLONGPRESS:
            break;

        case APP_KEY_EVENT_ULTRACLICK:
            break;

        case APP_KEY_EVENT_RAMPAGECLICK:
            break;
        default:
            break;
    }
}

static const APP_KEY_HANDLE  app_sound_ui_test_key_cfg[] =
{
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_CLICK},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_PWR,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN1,APP_KEY_EVENT_TRIPLECLICK},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN2,APP_KEY_EVENT_LONGPRESS},"app_ibrt_ui_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_CLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_DOUBLECLICK},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
    {{APP_KEY_CODE_FN3,APP_KEY_EVENT_LONGPRESS},"app_ibrt_service_test_key", app_sound_ui_test_key_io_event, NULL},
};

void app_sound_ui_test_key_init(void)
{
    LOG_I("app_sound_ui_test_key_init");
    app_key_handle_clear();
    for (uint8_t i=0; i<ARRAY_SIZE(app_sound_ui_test_key_cfg); i++)
    {
        app_key_handle_registration(&app_sound_ui_test_key_cfg[i]);
    }
}

void app_sound_ui_state_changed_evt_cb(SOUND_PROMPT_TYPE_E type, uint8_t reason_code)
{
    LOG_D("%s prompt_type=%d reason=%d", __func__, type, reason_code);
    AUD_ID_ENUM prompt_id = AUD_ID_INVALID;

    switch (type)
    {
        case SOUND_PROMPT_POWER_ON:
            break;
        case SOUND_PROMPT_POWER_OFF:
            prompt_id = AUD_ID_POWER_OFF;
            break;

        case SOUND_PROMPT_VOL_CHANGED:
            prompt_id = AUD_ID_BT_WARNING;
            break;

        case SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_START:
            prompt_id = AUD_ID_NUM_0;
            break;

        case SOUND_PROMPT_BT_ENTER_FREEMAN_MODE_END:
            prompt_id = AUD_ID_NUM_1;
            break;

        case SOUND_PROMPT_BT_ENTER_BIS_MODE_START:
            prompt_id = AUD_ID_NUM_2;
            break;

        case SOUND_PROMPT_BT_ENTER_BIS_MODE_END:
            prompt_id = AUD_ID_NUM_3;
            break;

        case SOUND_PROMPT_BT_ENTER_TWS_MODE_START:
            prompt_id = AUD_ID_NUM_4;
            break;

        case SOUND_PROMPT_BT_ENTER_TWS_MODE_END:
            prompt_id = AUD_ID_NUM_5;
            break;

        case SOUND_PROMPT_WIFI_SWITCH_BT:
            prompt_id = AUD_ID_NUM_9;
            break;

        case SOUND_PROMPT_BT_CLEAR_PAIRING_MODE:
            prompt_id = AUD_ID_NUM_8;
            break;

        case SOUND_PROMPT_BT_ENTER_PAIRING_MODE:
            prompt_id = AUD_ID_BT_PAIRING;
            break;

        case SOUND_PROMPT_AIRPLAY_PAIRING:
            prompt_id = AUD_ID_NUM_6;
            break;

        case SOUND_PROMPT_CLEAR_WIFI_NV_RECORD:
            prompt_id = AUD_ID_NUM_7;
            break;

        case SOUND_PROMPT_SDK_VERSION:
            prompt_id = AUD_ID_NUM_1;
            break;

        default:
            break;
    }

    if (prompt_id != AUD_ID_INVALID)
    {
        LOG_I("AUD_ID=%d", prompt_id);
#ifdef MEDIA_PLAYER_SUPPORT
        audio_player_play_prompt(prompt_id, 0);
#endif
    }
}
#endif //APP_SOUND_UI_ENABLE

