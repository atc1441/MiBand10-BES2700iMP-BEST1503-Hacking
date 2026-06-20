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
#include "bluetooth.h"
#include "btapp.h"
#include "me_api.h"
#include "app_ibrt_internal.h"
#include "earbud_ux_duplicate_api.h"
#include "app_tws_ibrt.h"
#include "app_ibrt_nvrecord.h"
#include "btspeaker_main.h"
#include "app_utils.h"
#include "app_ibrt_customif_ui.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "factory_section.h"
#include "nvrecord_extension.h"
#include "me_api.h"
#ifdef APP_BT_SPEAKER
#include "bluetooth_ble_api.h"
#include "ibrt_mgr_evt.h"
#include "cmsis_os.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_ibrt_cmd_handler.h"
#endif
#include "btspeaker_trace_cfg.h"
#include "app_ibrt_customif_cmd.h"
#include "apps.h"

#ifdef APP_BT_SPEAKER
typedef enum {
    BLE_SCAN_STATE_STOPPED,
    BLE_SCAN_STATE_STARTED,
} BLE_SCAN_STATE_E;

#define     LOCAL_SPEAKER_PRODUCT_TYPE              (SPEAKER_PRODUCT_TYPE_BESBOARD)
#define     LOCAL_SPEAKER_PRODUCT_COLOR             (SPEAKER_PRODUCT_COLOR_BLACK)
#define     BLE_SCAN_WINDOWN_MS                     (100) //ms of unit
#define     BLE_SCAN_INTERVAL_MS                    (200) //ms of unit
#define     BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD  (5)
#define     BLE_ADV_DATA_MANUFACTURER_TYPE_ID       (0xFF)

static uint16_t g_tws_setup_relative_tm = 0;
static uint8_t g_speaker_adv_data_info[31] = {0};
static uint8_t ble_adv_state = SPEAKER_BLE_ADV_OP_IDLE;
static BLE_SCAN_STATE_E ble_scan_state = BLE_SCAN_STATE_STOPPED;
static speaker_app_ibrt_ui_t g_speaker_ibrt_ui;

#define BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE        (3)
static SPEAKER_BLE_SCAN_DATA_STORE_CACHE_TABLE_T ble_scan_data_cache_table[BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE]= {0};

static void ble_scan_handle_thread(void const *argument);
osThreadDef(ble_scan_handle_thread, osPriorityAboveNormal, 1, 4096, "ble_scan_handle_thread");

void app_ibrt_ui_wait_ble_scanning_device_timer_cb(void const *n);
osTimerId  ibrt_ui_ble_scanning_device_timeout_timer_id = NULL;
osTimerDef (IBRT_UI_BLE_ADV_DEVICE_TIMER, app_ibrt_ui_wait_ble_scanning_device_timer_cb);

#define BLE_SCAN_MSG_MAILBOX_MAX     (5)
typedef struct {
    int8_t          rssi;
    uint8_t         data_len;
    uint8_t         data[BLE_ADV_REPORT_MAX_LEN];
} __attribute__((packed))BLE_SCAN_RESULT_MSG_BLOCK;


static osMailQId ble_scan_msg_mailbox = NULL;
static uint8_t ble_scan_msg_mailbox_cnt = 0;
osMailQDef (ble_scan_msg_mailbox, BLE_SCAN_MSG_MAILBOX_MAX, BLE_SCAN_RESULT_MSG_BLOCK);

speaker_app_ibrt_ui_t *speaker_app_ibrt_ui_get_ctx(void)
{
    return (speaker_app_ibrt_ui_t*)&g_speaker_ibrt_ui;
}

void app_ibrt_ui_wait_ble_scanning_device_timer_cb(void const *n)
{
    speaker_app_ibrt_ui_t *p_ibrt_ui = speaker_app_ibrt_ui_get_ctx();
    if (p_ibrt_ui->super_state == IBRT_UI_TWS_DISCOVERY)
    {
        p_ibrt_ui->error_state = SPEAKER_IBRT_UI_RSP_TIMEOUT;
        app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
    }
    else if (p_ibrt_ui->super_state == IBRT_UI_TWS_FORMATION)
    {
        p_ibrt_ui->error_state = SPEAKER_IBRT_UI_RSP_TIMEOUT;
        app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
    }
    else if (p_ibrt_ui->super_state == IBRT_UI_TWS_PREHANDLE)
    {
        app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
    }
}

void speaker_ibrt_ui_stop_ble_scanning_device_timer(void)
{
    if (ibrt_ui_ble_scanning_device_timeout_timer_id != NULL)
    {
        osTimerStop(ibrt_ui_ble_scanning_device_timeout_timer_id);
    }
}

void speaker_ibrt_ui_start_ble_scanning_device_timer(uint32_t ms)
{
    if (ibrt_ui_ble_scanning_device_timeout_timer_id != NULL)
    {
        osTimerStart(ibrt_ui_ble_scanning_device_timeout_timer_id, ms);
    }
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

static int ble_scan_mailbox_init(void)
{
    ble_scan_msg_mailbox = osMailCreate(osMailQ(ble_scan_msg_mailbox), NULL);
    if (ble_scan_msg_mailbox == NULL)
    {
        // shall increase OS_DYNAMIC_MEM_SIZE if it's rtx5
        ASSERT(false, "Failed to Create ble_scan_mailbox\n");
        return -1;
    }
    ble_scan_msg_mailbox_cnt = 0;
    return 0;
}

static int ble_scan_mailbox_free(BLE_SCAN_RESULT_MSG_BLOCK* msg_p)
{
    osStatus status;

    status = osMailFree(ble_scan_msg_mailbox, msg_p);
    if (osOK != status)
    {
        TRACE(2,"%s error status 0x%02x", __func__, status);
        return (int)status;
    }

    if (ble_scan_msg_mailbox_cnt)
    {
        ble_scan_msg_mailbox_cnt--;
    }

    return (int)status;
}

static int ble_scan_mailbox_free_all()
{
    BLE_SCAN_RESULT_MSG_BLOCK *msg_p = NULL;
    int status = osOK;
    osEvent evt;

    for (uint8_t i=0; i<BLE_SCAN_MSG_MAILBOX_MAX; i++)
    {
        evt = osMailGet(ble_scan_msg_mailbox, 100);
        if (evt.status == osEventMail)
        {
            msg_p = (BLE_SCAN_RESULT_MSG_BLOCK *)evt.value.p;
            status = ble_scan_mailbox_free(msg_p);
        }
        else
        {
            LOG_E("%s get mailbox timeout!!!", __func__);
            continue;
        }
   }
   return status;
}

static osStatus ble_scan_mailbox_put(int8_t rssi, uint8_t *scan_data, uint8_t scan_data_len)
{
    osStatus status = osOK;

    BLE_SCAN_RESULT_MSG_BLOCK *msg_p = NULL;

    if (ble_scan_msg_mailbox_cnt > BLE_SCAN_MSG_MAILBOX_MAX - 1)
    {
        LOG_W("%s arrive maximum", __func__);
        return (osStatus)osErrorValue;
    }

    msg_p = (BLE_SCAN_RESULT_MSG_BLOCK*)osMailAlloc(ble_scan_msg_mailbox, 0);
    if (msg_p == NULL)
    {
        ble_scan_mailbox_free_all();
        msg_p = (BLE_SCAN_RESULT_MSG_BLOCK*)osMailAlloc(ble_scan_msg_mailbox, 0);
        ASSERT(msg_p, "osMailAlloc error");
    }
    msg_p->rssi = rssi;
    msg_p->data_len = scan_data_len;
    memcpy(msg_p->data, scan_data, scan_data_len);
    status = osMailPut(ble_scan_msg_mailbox, msg_p);
    if (osOK != status)
    {
        LOG_E("%s error status 0x%02x", __func__, status);
        return status;
    }

    ble_scan_msg_mailbox_cnt++;

    return status;
}

static int ble_scan_mailbox_get(BLE_SCAN_RESULT_MSG_BLOCK** msg_p)
{
    osEvent evt;

    evt = osMailGet(ble_scan_msg_mailbox, osWaitForever);
    if (evt.status == osEventMail)
    {
        *msg_p = (BLE_SCAN_RESULT_MSG_BLOCK *)evt.value.p;
        return 0;
    }
    else
    {
       LOG_E("%s evt.status 0x%02x", __func__, evt.status);
        return -1;
    }
}

static void app_speaker_ble_user_data_fill_handler(void *param)
{
    BLE_ADV_PARAM_T *advInfo = (BLE_ADV_PARAM_T *)param;
    bool adv_enable = false;

    LOG_D("------> adv data offset:%d, scan response data offset:%d",
                  advInfo->advDataLen,
                  advInfo->scanRspDataLen);

    if (SPEAKER_BLE_ADV_OP_IDLE != ble_adv_state)
    {
        adv_enable = true;
        memcpy(&advInfo->advData[advInfo->advDataLen], &g_speaker_adv_data_info[0], g_speaker_adv_data_info[0] + 1);
        advInfo->advDataLen = advInfo->advDataLen + g_speaker_adv_data_info[0] + 1;
        LOG_I("bt speaker adv len:%d", g_speaker_adv_data_info[0] + 1);
    }

    app_ble_data_fill_enable(USER_SPEAKER_TWS, adv_enable);
}

void app_speaker_ble_scan_device_start(void)
{
    LOG_I("%s scan_state:%d", __func__, ble_scan_state);
    if (ble_scan_state == BLE_SCAN_STATE_STOPPED)
    {
        bes_ble_scan_param_t scan_param = {0};

        scan_param.scanFolicyType = BLE_SCAN_ALLOW_ADV_ALL;
        scan_param.scanWindowMs   = BLE_SCAN_WINDOWN_MS;
        scan_param.scanIntervalMs   = BLE_SCAN_INTERVAL_MS;

        bes_ble_gap_start_scan(&scan_param);
        ble_scan_state = BLE_SCAN_STATE_STARTED;
    }
}

void app_speaker_ble_scan_device_stop(void)
{
    LOG_I("%s scan_state:%d", __func__, ble_scan_state);
    if (ble_scan_state != BLE_SCAN_STATE_STOPPED)
    {
        bes_ble_gap_stop_scan();
        ble_scan_state = BLE_SCAN_STATE_STOPPED;
    }
}

static bool app_speaker_ble_confirm_peer_device_match_success(uint8_t *peer_device, uint8_t *local_device)
{
    bool ret = false;
    uint8_t local_factory_bt_addr[6] = {0};
    factory_section_original_btaddr_get(local_factory_bt_addr);
    if (memcmp(local_factory_bt_addr, local_device, 6) == 0)
    {
        ret = true;
    }
    return ret;
}

static uint8_t app_speaker_decision_local_device_role(SPEAKER_CONNECTION_STATUS_INFO_T *peer_conn_status, SPEAKER_CONNECTION_STATUS_INFO_T *local_conn_status)
{
    uint8_t config_role = SPEAKER_ROLE_UNKNOW;
    if (peer_conn_status && local_conn_status)
    {
        LOG_V("rmt_time:%d, loc_time:%d", peer_conn_status->rtc_time, local_conn_status->rtc_time);
        if (peer_conn_status->app_profile_status != SPEAKER_APP_ID_INVALID && local_conn_status->app_profile_status != SPEAKER_APP_ID_INVALID)
        {
            if (peer_conn_status->mobile_connection_status && local_conn_status->mobile_connection_status)
            {
                if (peer_conn_status->rtc_time > local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_MASTER;
                }
                else if (peer_conn_status->rtc_time < local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_SLAVE;
                }
            }
            else if (peer_conn_status->mobile_connection_status)
            {
                config_role = SPEAKER_ROLE_SLAVE;
            }
            else if (local_conn_status->mobile_connection_status)
            {
                config_role = SPEAKER_ROLE_MASTER;
            }
            else
            {
                if (peer_conn_status->rtc_time > local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_MASTER;
                }
                else if (peer_conn_status->rtc_time < local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_SLAVE;
                }
            }
        }
        else if (peer_conn_status->app_profile_status != SPEAKER_APP_ID_INVALID)
        {
            config_role = SPEAKER_ROLE_SLAVE;
        }
        else if (local_conn_status->app_profile_status != SPEAKER_APP_ID_INVALID)
        {
            config_role = SPEAKER_ROLE_MASTER;
        }
        else
        {
            if (peer_conn_status->mobile_connection_status && local_conn_status->mobile_connection_status)
            {
                if (peer_conn_status->rtc_time > local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_MASTER;
                }
                else if (peer_conn_status->rtc_time < local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_SLAVE;
                }
            }
            else if (peer_conn_status->mobile_connection_status)
            {
                config_role = SPEAKER_ROLE_SLAVE;
            }
            else if (local_conn_status->mobile_connection_status)
            {
                config_role = SPEAKER_ROLE_MASTER;
            }
            else
            {
                if (peer_conn_status->rtc_time > local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_MASTER;
                }
                else if (peer_conn_status->rtc_time < local_conn_status->rtc_time)
                {
                    config_role = SPEAKER_ROLE_SLAVE;
                }
            }
        }
    }
    return config_role;
}

static bool app_speaker_ble_scan_data_handle_tws_discovery_ind(int8_t rssi_value, uint8_t *adv_data, uint8_t data_len)
{
    SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T *stereo_adv_data_req = (SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T*)adv_data;
    int8_t min_rssi = ble_scan_data_cache_table[0].peer_rssi_value;
    uint8_t found_index = 0;
    bool confirm_scan_device = false;
    uint8_t index = 0;
    uint8_t crc_value_origin = stereo_adv_data_req->crc_check;
    uint8_t crc_value_check = crc8_cal(adv_data, sizeof(SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T)-1);
    if (crc_value_origin == crc_value_check)
    {
        if (stereo_adv_data_req->product_info.product_type == LOCAL_SPEAKER_PRODUCT_TYPE)
        {
            for (index = 0; index < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; index++)
            {
                if (!ble_scan_data_cache_table[index].is_use)
                {
                    ble_scan_data_cache_table[index].is_use = true;
                    ble_scan_data_cache_table[index].peer_rssi_value = rssi_value;
                    memcpy(&ble_scan_data_cache_table[index].peer_device_info, &stereo_adv_data_req->device_info, sizeof(SPEAKER_DEVICE_INFO));
                    memcpy(&ble_scan_data_cache_table[index].peer_connection_status_info, &stereo_adv_data_req->connection_info, sizeof(SPEAKER_CONNECTION_STATUS_INFO_T));
                    ble_scan_data_cache_table[index].peer_scan_recv_cnt = 1;
                    if (ble_scan_data_cache_table[index].peer_scan_recv_cnt >= BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD)
                    {
                        confirm_scan_device = true;
                    }
                    found_index = index;
                    break;
                }
                else
                {
                    if (!memcmp(ble_scan_data_cache_table[index].peer_device_info.bt_mac_addr, stereo_adv_data_req->device_info.bt_mac_addr, 6))
                    {
                        ble_scan_data_cache_table[index].peer_rssi_value = rssi_value;
                        memcpy(&ble_scan_data_cache_table[index].peer_connection_status_info, &stereo_adv_data_req->connection_info, sizeof(SPEAKER_CONNECTION_STATUS_INFO_T));
                        ble_scan_data_cache_table[index].peer_scan_recv_cnt += 1;
                        if (ble_scan_data_cache_table[index].peer_scan_recv_cnt >= BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD)
                        {
                            confirm_scan_device = true;
                        }
                        found_index = index;
                        break;
                    }
                    else if (ble_scan_data_cache_table[index].peer_rssi_value < min_rssi)
                    {
                        min_rssi = ble_scan_data_cache_table[index].peer_rssi_value;
                        found_index = index;
                    }
                }
            }

            if (index >= BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE)
            {
                if (rssi_value >= min_rssi)
                {
                    ble_scan_data_cache_table[found_index].peer_rssi_value = rssi_value;
                    memcpy(&ble_scan_data_cache_table[found_index].peer_device_info, \
                            &stereo_adv_data_req->device_info, sizeof(SPEAKER_DEVICE_INFO));
                    memcpy(&ble_scan_data_cache_table[found_index].peer_connection_status_info, \
                            &stereo_adv_data_req->connection_info, sizeof(SPEAKER_CONNECTION_STATUS_INFO_T));
                    ble_scan_data_cache_table[found_index].peer_scan_recv_cnt = 1;
                    if (ble_scan_data_cache_table[found_index].peer_scan_recv_cnt >= BLE_SCAN_SAME_DEVICE_MAX_CNT_THRESHOLD)
                    {
                        confirm_scan_device = true;
                    }
                }
            }
            LOG_D("scan tws req: update_index=%d recv_cnt=%d", found_index, ble_scan_data_cache_table[found_index].peer_scan_recv_cnt);
        }
    }
    return confirm_scan_device;
}

static uint8_t app_speaker_ble_scan_data_handle_tws_discovery_cfm(uint8_t *adv_data, uint8_t data_len)
{
    SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T *stereo_adv_data_cfm = (SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T*)adv_data;
    uint8_t local_config_role = SPEAKER_ROLE_UNKNOW;
    uint8_t crc_value_origin = stereo_adv_data_cfm->crc_check;
    uint8_t crc_value_check = crc8_cal(adv_data, sizeof(SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T)-1);

    if (crc_value_origin == crc_value_check)
    {
        if (app_speaker_ble_confirm_peer_device_match_success(stereo_adv_data_cfm->local_device_info.bt_mac_addr, stereo_adv_data_cfm->peer_device_info.bt_mac_addr))
        {
            local_config_role = app_speaker_decision_local_device_role(&stereo_adv_data_cfm->local_connection_info, &stereo_adv_data_cfm->peer_connection_info);
            if (local_config_role == SPEAKER_ROLE_UNKNOW)
            {
                TRACE(0, "peer device addr:");
                DUMP8("%x ", stereo_adv_data_cfm->local_device_info.bt_mac_addr, 6);
                TRACE(0, "local device addr:");
                DUMP8("%x ", stereo_adv_data_cfm->peer_device_info.bt_mac_addr, 6);
                if (memcmp(stereo_adv_data_cfm->local_device_info.bt_mac_addr, stereo_adv_data_cfm->peer_device_info.bt_mac_addr, 6) > 0)
                {
                    local_config_role = SPEAKER_ROLE_SLAVE;
                }
                else
                {
                    local_config_role = SPEAKER_ROLE_MASTER;
                }
            }
        }
    }
    LOG_D("scan tws cfm: ux_role=%d", local_config_role);
    return local_config_role;
}

static void app_speaker_ble_scan_data_report_handler(int8_t rssi_value, uint8_t *adv_data, uint8_t data_len)
{
    //DUMP8("%02x ", adv_data, data_len);
    uint8_t adv_len = 0;
    uint8_t adv_type = 0;
    uint8_t *ptr_section_data = adv_data;
    uint8_t start_index = 0;
    bool loop_continue = true;
    speaker_app_ibrt_ui_t *p_ibrt_ui = speaker_app_ibrt_ui_get_ctx();
    uint8_t ret = 0;
    app_sysfreq_req(APP_SYSFREQ_USER_PARSE_BLE_SCAN_DATA, APP_SYSFREQ_104M);
    TRACE(0, "app_speaker_ble_scan_data_report_handler ===> rssi_value= %d", rssi_value);
    DUMP8("%x ", adv_data, data_len);
    while (loop_continue && adv_data && (start_index <= data_len - 3))
    {
        adv_len = ptr_section_data[0];
        adv_type = ptr_section_data[1];
        TRACE(0, "adv_len = %d adv_type = %d", adv_len, adv_type);
        if (adv_type == BLE_ADV_DATA_MANUFACTURER_TYPE_ID)
        {
            SPEAKER_TWS_INFO_T *cmd_operation = (SPEAKER_TWS_INFO_T*)&ptr_section_data[2];
            TRACE(0, "tws_operation = %d", cmd_operation->tws_operation);
            switch (cmd_operation->tws_operation)
            {
                case SPEAKER_BLE_ADV_OP_IDLE:
                    break;
                case SPEAKER_BLE_ADV_OP_SETUP_STEREO_REQ:
                    if (adv_len == sizeof(SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T) + 1)
                    {
                        ret = app_speaker_ble_scan_data_handle_tws_discovery_ind(rssi_value, &ptr_section_data[2], data_len);
                        if (ret)
                        {
                            if (IBRT_UI_TWS_DISCOVERY == p_ibrt_ui->super_state)
                            {
                                speaker_ibrt_ui_stop_ble_scanning_device_timer();
                                p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
                                app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
                            }
                        }
                        loop_continue = false;
                    }else{
                        LOG_E("adv_len %d != %d", adv_len, sizeof(SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T) + 1);
                    }
                    break;
                case SPEAKER_BLE_ADV_OP_SETUP_STEREO_CFM:
                    if (adv_len == sizeof(SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T) + 1)
                    {
                        SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T *stereo_adv_data_cfm = (SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T*)&ptr_section_data[2];
                        ret = app_speaker_ble_scan_data_handle_tws_discovery_cfm((uint8_t*)stereo_adv_data_cfm, data_len);
                        if (ret != SPEAKER_ROLE_UNKNOW)
                        {
                            speaker_ibrt_ui_stop_ble_scanning_device_timer();
                            if (IBRT_UI_TWS_FORMATION == p_ibrt_ui->super_state)
                            {
                                app_speaker_ble_scan_device_stop();
                                if (ret == SPEAKER_ROLE_MASTER)
                                {
#ifdef IBRT_RIGHT_MASTER
                                    app_tws_ibrt_reconfig_role(IBRT_MASTER, stereo_adv_data_cfm->peer_device_info.bt_mac_addr, 
                                                stereo_adv_data_cfm->local_device_info.bt_mac_addr, true);
#else
                                    app_tws_ibrt_reconfig_role(IBRT_MASTER, stereo_adv_data_cfm->peer_device_info.bt_mac_addr, 
                                                stereo_adv_data_cfm->local_device_info.bt_mac_addr, false);
#endif
                                }
                                else
                                {
#ifdef IBRT_RIGHT_MASTER
                                    app_tws_ibrt_reconfig_role(IBRT_SLAVE, stereo_adv_data_cfm->local_device_info.bt_mac_addr, 
                                                stereo_adv_data_cfm->peer_device_info.bt_mac_addr, true);
#else
                                    app_tws_ibrt_reconfig_role(IBRT_SLAVE, stereo_adv_data_cfm->local_device_info.bt_mac_addr, 
                                                stereo_adv_data_cfm->peer_device_info.bt_mac_addr, false);
#endif
                                }
                                p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
                                app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
                            }
                            else if (IBRT_UI_TWS_DISCOVERY == p_ibrt_ui->super_state)
                            {
                                p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
                                app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
                            }
                        }
                        loop_continue = false;
                    }
                    break;
                case SPEAKER_BLE_ADV_OP_EXIT_STEREO_REQ:
                    // Notice: use ble adv solution to indicate peer device to exit stereo mode, currently it's not supported.
                    loop_continue = false;
                    break;
                case SPEAKER_BLE_ADV_OP_EXIT_STEREO_CFM:
                    // Notice: use ble adv solution to indicate peer device to exit stereo mode, currently it's not supported.
                    loop_continue = false;
                    break;
                default:
                    break;
            }
        }
        start_index = start_index + adv_len + 1;
        ptr_section_data += start_index;
    }
    app_sysfreq_req(APP_SYSFREQ_USER_PARSE_BLE_SCAN_DATA, APP_SYSFREQ_32K);
}

static uint8_t app_speaker_scan_cache_table_max_recv_cnt_index_get()
{
    uint16_t max_recv_cnt = ble_scan_data_cache_table[0].peer_scan_recv_cnt;
    uint8_t  max_cnt_index = 0;
    for (uint8_t i = 1; i < BLE_SCAN_DATA_STORE_CACHE_TABLE_SIZE; i++)
    {
        if (ble_scan_data_cache_table[i].peer_scan_recv_cnt > max_recv_cnt)
        {
            max_cnt_index = i;
            max_recv_cnt = ble_scan_data_cache_table[i].peer_scan_recv_cnt;
        }
    }
    return max_cnt_index;
}

void app_speaker_ble_adv_data_start_tws_discovery_ind(void)
{
    LOG_I("%s", __func__);
    memset(g_speaker_adv_data_info, 0, sizeof(g_speaker_adv_data_info)/sizeof(g_speaker_adv_data_info[0]));
    SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T *ble_scan_data_req = (SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T *)&g_speaker_adv_data_info[2];

    g_speaker_adv_data_info[0] = sizeof(SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T) + 1;
    g_speaker_adv_data_info[1] = BLE_ADV_DATA_MANUFACTURER_TYPE_ID;

    //fill the tws info adv data
    ble_scan_data_req->tws_info.tws_operation = SPEAKER_BLE_ADV_OP_SETUP_STEREO_REQ;
    ble_scan_data_req->tws_info.speaker_role = SPEAKER_ROLE_UNKNOW;

    //fill the product info adva data
    ble_scan_data_req->product_info.product_type = LOCAL_SPEAKER_PRODUCT_TYPE;
    ble_scan_data_req->product_info.product_color = LOCAL_SPEAKER_PRODUCT_COLOR;

    //fill the connection info adv data
    ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
    if (app_tws_ibrt_mobile_link_connected(&p_mobile_info->mobile_addr))
        ble_scan_data_req->connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_CONNECTED;
    //else if (app_ibrt_ui_is_mobile_connecting())
    //    ble_scan_data_req->connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_CONNECTING;
    else
        ble_scan_data_req->connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_DISCONNECTED;
    ble_scan_data_req->connection_info.wifi_connection_status = SPEAKER_CONNECTION_STATUS_DISCONNECTED;
    ble_scan_data_req->connection_info.app_profile_status = SPEAKER_APP_ID_INVALID;
    ble_scan_data_req->connection_info.rtc_time = g_tws_setup_relative_tm;

    //fill the device infor adv data
    factory_section_original_btaddr_get(ble_scan_data_req->device_info.bt_mac_addr);

    ble_scan_data_req->crc_check = crc8_cal(&g_speaker_adv_data_info[2], sizeof(SPEAKER_BLE_ADV_DATA_TWS_DISCOVE_IND_T)-1);

    //clear the cache table store data and restart ble adv
    ble_adv_state = SPEAKER_BLE_ADV_OP_SETUP_STEREO_REQ;
    memset(ble_scan_data_cache_table, 0, sizeof(ble_scan_data_cache_table));
    bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_TWS_STM,USER_SPEAKER_TWS,BTSPEAKER_BLE_ADV_INTERVAL_DISCOVERY_TWS);
    bes_ble_gap_refresh_adv_state(BTSPEAKER_BLE_ADV_INTERVAL_DISCOVERY_TWS);
}

void app_speaker_ble_adv_data_start_tws_discovery_cfm(void)
{
    LOG_I("%s", __func__);
    memset(g_speaker_adv_data_info, 0, sizeof(g_speaker_adv_data_info)/sizeof(g_speaker_adv_data_info[0]));
    SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T *ble_adv_data_cfm = (SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T *)&g_speaker_adv_data_info[2];

    g_speaker_adv_data_info[0] = sizeof(SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T) + 1;
    g_speaker_adv_data_info[1] = BLE_ADV_DATA_MANUFACTURER_TYPE_ID;

    //fill the tws info adv data
    ble_adv_data_cfm->tws_info.tws_operation = SPEAKER_BLE_ADV_OP_SETUP_STEREO_CFM;
    ble_adv_data_cfm->tws_info.speaker_role = SPEAKER_ROLE_UNKNOW;

    //fill the local device info adv data
    factory_section_original_btaddr_get(ble_adv_data_cfm->local_device_info.bt_mac_addr);

    //fill the local connection info adv data
    ibrt_mobile_info_t *p_mobile_info = app_ibrt_conn_get_mobile_info_ext();
    if (app_tws_ibrt_mobile_link_connected(&p_mobile_info->mobile_addr))
        ble_adv_data_cfm->local_connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_CONNECTED;
    //else if (app_ibrt_ui_is_mobile_connecting())
    //    ble_adv_data_cfm->local_connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_CONNECTING;
    else
        ble_adv_data_cfm->local_connection_info.mobile_connection_status = SPEAKER_CONNECTION_STATUS_DISCONNECTED;
    ble_adv_data_cfm->local_connection_info.wifi_connection_status = SPEAKER_CONNECTION_STATUS_DISCONNECTED;
    ble_adv_data_cfm->local_connection_info.app_profile_status = SPEAKER_APP_ID_INVALID;
    ble_adv_data_cfm->local_connection_info.rtc_time = g_tws_setup_relative_tm;

    uint8_t sel_index = app_speaker_scan_cache_table_max_recv_cnt_index_get();
    //fill the peer device info adv data
    memcpy(ble_adv_data_cfm->peer_device_info.bt_mac_addr, \
                    ble_scan_data_cache_table[sel_index].peer_device_info.bt_mac_addr, 6);

    //fill the peer device connection info adv data
    memcpy((uint8_t*)&ble_adv_data_cfm->peer_connection_info, \
                    (uint8_t*)&ble_scan_data_cache_table[sel_index].peer_connection_status_info, sizeof(SPEAKER_CONNECTION_STATUS_INFO_T));

    ble_adv_data_cfm->crc_check = crc8_cal(&g_speaker_adv_data_info[2], sizeof(SPEAKER_BLE_ADV_DATA_TWS_CONFIRM_DEV_T)-1);

    ble_adv_state = SPEAKER_BLE_ADV_OP_SETUP_STEREO_CFM;
    bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_TWS_STM,USER_SPEAKER_TWS,BTSPEAKER_BLE_ADV_INTERVAL_DISCOVERY_TWS);
    bes_ble_gap_refresh_adv_state(BTSPEAKER_BLE_ADV_INTERVAL_DISCOVERY_TWS);
}

void app_speaker_ble_adv_data_stop(void)
{
    LOG_I("%s adv_state:%d", __func__, ble_adv_state);
    if (ble_adv_state != SPEAKER_BLE_ADV_OP_IDLE)
    {
        ble_adv_state = SPEAKER_BLE_ADV_OP_IDLE;
        bes_ble_gap_refresh_adv_state(BTSPEAKER_BLE_ADV_INTERVAL_IDLE_DEFAULT);
    }
}

static void app_speaker_ble_scan_data_report_hook(const btif_ble_adv_report *report)
{
    if (report)
    {
        ble_scan_mailbox_put(report->rssi, (uint8_t*)report->data, report->data_len);
    }
}

static void ble_scan_handle_thread(void const *argument)
{
    BLE_SCAN_RESULT_MSG_BLOCK* msg_p = NULL;
    uint8_t scan_data[BLE_ADV_REPORT_MAX_LEN] = {0};
    uint8_t scan_data_len = 0;
    int8_t  rssi_value = 0;

    while(1)
    {
        if (ble_scan_mailbox_get(&msg_p) == 0)
        {
            memcpy(scan_data, msg_p->data, msg_p->data_len);
            scan_data_len = msg_p->data_len;
            rssi_value = msg_p->rssi;
            ble_scan_mailbox_free(msg_p);

            //handle the stored ble scan results
            app_speaker_ble_scan_data_report_handler(rssi_value, scan_data, scan_data_len);
        }
    }
}

bool app_speaker_setup_bt_stereo(void)
{
    //app_voice_report(APP_STATUS_INDICATION_SETUP_STEREO_START, 0);
    uint32_t tws_setup_timestamp = (uint32_t)__SLIM_TICKS_TO_MS(hal_sys_timer_get());
    g_tws_setup_relative_tm = (uint16_t)(tws_setup_timestamp/1000);
    LOG_I("%s setup_tws_tm:%d", __func__, g_tws_setup_relative_tm);
    app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DEV_SCAN);
    return true;
}

bool app_speaker_exit_bt_stereo(void)
{
    LOG_I("%s", __func__);
    app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_DISCONNECT);
    
    return true;
}

static void app_speaker_reset_tws_pairing_config(void){
    LOG_I("==> %s", __func__);
    speaker_app_ibrt_ui_t *p_ibrt_ui = speaker_app_ibrt_ui_get_ctx();
    p_ibrt_ui->super_state = IBRT_UI_IDLE;
    p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
    app_speaker_ble_adv_data_stop();
    app_speaker_ble_scan_device_stop();
}

void app_speaker_tws_pairing_parse(void){
    speaker_app_ibrt_ui_t *p_ibrt_ui = speaker_app_ibrt_ui_get_ctx();
    LOG_I("==> %s %d", __func__, p_ibrt_ui->super_state);
    switch(p_ibrt_ui->super_state){
        case IBRT_UI_IDLE:{
            app_speaker_ble_adv_data_start_tws_discovery_ind();
            app_speaker_ble_scan_device_start();
            speaker_ibrt_ui_start_ble_scanning_device_timer(BTSPEAKER_BLE_DISCOVER_DEVICE_TIMEOUT);
            p_ibrt_ui->super_state = IBRT_UI_TWS_DISCOVERY;
            p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
            break;
        }
        case IBRT_UI_TWS_DISCOVERY:{
            if(p_ibrt_ui->error_state == SPEAKER_IBRT_UI_NO_ERROR){
                speaker_ibrt_ui_stop_ble_scanning_device_timer();
                p_ibrt_ui->super_state = IBRT_UI_TWS_FORMATION;
                p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
                app_speaker_ble_adv_data_start_tws_discovery_cfm();
                speaker_ibrt_ui_start_ble_scanning_device_timer(BTSPEAKER_BLE_CONFIRM_DEVICE_TIMEOUT);
            }
            else{
                LOG_E("====> discovery timeout !!!");
                app_speaker_reset_tws_pairing_config();
            }
            break;
        }
        case IBRT_UI_TWS_FORMATION:{
            if(p_ibrt_ui->error_state == SPEAKER_IBRT_UI_NO_ERROR){
                speaker_ibrt_ui_stop_ble_scanning_device_timer();
                p_ibrt_ui->super_state = IBRT_UI_TWS_PREHANDLE;
                p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;
                speaker_ibrt_ui_start_ble_scanning_device_timer(BTSPEAKER_BLE_TWS_CONN_PREHANDLE_TIMEOUT);
            }
            else{
                LOG_E("====> formation timeout !!!");
                app_speaker_reset_tws_pairing_config();
            }
            break;
        }
        case IBRT_UI_TWS_PREHANDLE:{
            p_ibrt_ui->super_state = IBRT_UI_IDLE;
            app_speaker_ble_adv_data_stop();
            LOG_I("%s===========> prepare to tws pair!!!", __func__);
            // enter tws pairing
            app_ibrt_if_init_open_box_state_for_evb();
            app_ibrt_if_event_entry(IBRT_MGR_EV_TWS_PAIRING);
            break;
        }
        default:break;
    }
}

void app_speaker_tws_reset_parse(void){
    if(app_tws_ibrt_mobile_link_connected(&app_ibrt_conn_get_mobile_info_ext()->mobile_addr)){
        stop_ibrt_info_t stop_ibrt_buffer;
        stop_ibrt_buffer.opcode = IBRT_STOP_IBRT;
        stop_ibrt_buffer.error_code = IBRT_UI_NO_ERROR;
        stop_ibrt_buffer.mobile_addr = app_ibrt_conn_get_mobile_info_ext()->mobile_addr;
        stop_ibrt_buffer.ibrt_role = app_tws_get_ibrt_role(&app_ibrt_conn_get_mobile_info_ext()->mobile_addr);
        tws_ctrl_send_cmd(APP_TWS_CMD_STOP_IBRT, \
                         (uint8_t *)&stop_ibrt_buffer, \
                         sizeof(stop_ibrt_buffer));
    }
    
}

void app_speaker_tws_ui_init(void){
    speaker_app_ibrt_ui_t *p_ibrt_ui = speaker_app_ibrt_ui_get_ctx();
    p_ibrt_ui->super_state = IBRT_UI_IDLE;
    p_ibrt_ui->error_state = SPEAKER_IBRT_UI_NO_ERROR;

    ibrt_ui_ble_scanning_device_timeout_timer_id = osTimerCreate(osTimer(IBRT_UI_BLE_ADV_DEVICE_TIMER), \
                                          osTimerOnce, NULL);
}
void app_speaker_init(void){
    LOG_I("%s", __func__);
    app_speaker_tws_ui_init();
    app_ble_register_data_fill_handle(USER_SPEAKER_TWS, (BLE_DATA_FILL_FUNC_T)app_speaker_ble_user_data_fill_handler, false);
    btif_me_ble_receive_adv_report(app_speaker_ble_scan_data_report_hook);

#ifdef RTOS
    osThreadId ble_scan_thread_tid = NULL;
    ble_scan_mailbox_init();
    ble_scan_thread_tid = osThreadCreate(osThread(ble_scan_handle_thread), NULL);
    ASSERT(ble_scan_thread_tid, "create ble_scan_thread error");
#endif

}
#endif


