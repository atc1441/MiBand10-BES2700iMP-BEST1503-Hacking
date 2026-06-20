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
#ifdef BT_HFP_SUPPORT
#undef MOUDLE
#define MOUDLE HFP_APP
#include <stdio.h>
#include "cmsis_os.h"
#include "cmsis.h"
#include "hal_codec.h"
#include "hal_uart.h"
#include "hal_timer.h"
#include "audioflinger.h"
#include "lockcqueue.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "hal_chipid.h"
#include "analog.h"
#include "me_api.h"
#include "spp_api.h"
#include "app_audio.h"
#include "app_battery.h"
#include "app_trace_rx.h"
#include "app_status_ind.h"
#include "bluetooth.h"
#include "nvrecord_bt.h"
#include "nvrecord_env.h"
#include "nvrecord_dev.h"
#include "audio_policy.h"
#include "hfp_api.h"
#include "sco_api.h"
#include "besbt.h"
#include "cqueue.h"
#include "btapp.h"
#include "app_bt.h"
#include "apps.h"
#include "bt_if.h"
#include "app_hfp.h"
#if defined(BT_HFP_AG_ROLE)
#include "app_hfp_ag.h"
#endif

#include "besbt_cfg.h"
#include "app_bt_func.h"
#ifdef BT_USB_AUDIO_DUAL_MODE
#include "btusb_audio.h"
#endif
#ifdef __THIRDPARTY
#include "app_thirdparty.h"
#endif

#include "bt_drv_interface.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif

#ifdef __AI_VOICE__
#include "app_ai_if.h"
#endif

#if defined(IBRT)
#include "app_ibrt_internal.h"
#include "app_tws_ibrt_cmd_sync_hfp_status.h"
#include "besaud_api.h"
#include "app_tws_profile_sync.h"
#include "app_tws_ctrl_thread.h"
#include "app_tws_besaud.h"
#endif

#include "bt_drv_reg_op.h"

#ifdef BISTO_ENABLED
#include "gsound_custom_bt.h"
#endif

#ifdef GFPS_ENABLED
#include "gfps.h"
#endif

#if defined(BT_MAP_SUPPORT)
#include "app_map.h"
#endif

#if defined(__BIXBY_VOICE__) || defined(__BIXBY)
#include "app_bixby_thirdparty_if.h"
#endif

#include "app_bt_stream.h"
#include "app_bt_media_manager.h"
#include "app_audio_active_device_manager.h"
#include "audio_player_adapter.h"
#include "system_utils.h"

#define ATCMD_SZ 42

#define APP_HFP_SCO_ERROR_REJ_PERSONAL_DEV         0x0F //sync HCI_ERR_HOST_REJ_PERSONAL_DEV
#define APP_HFP_SCO_ERROR_INVALID_LMP_PARM         0x1E //SYNC HCI_ERR_INVALID_LMP_PARM

#ifdef __INTERCONNECTION__
#define HF_COMMAND_HUAWEI_BATTERY_HEAD              "AT+HUAWEIBATTERY="
#define BATTERY_REPORT_NUM                          2
#define BATTERY_REPORT_TYPE_BATTERY_LEVEL           1
#define BATTERY_REPORT_KEY_LEFT_BATTERY_LEVEL       2
#define BATTERY_REPORT_KEY_LEFT_CHARGE_STATE        3
static const char* huawei_self_defined_command_response = "+HUAWEIBATTERY=OK";
#endif

/* hfp */
int store_voicebtpcm_m2p_buffer(unsigned char *buf, unsigned int len);
int get_voicebtpcm_p2m_frame(unsigned char *buf, unsigned int len);

int store_voicecvsd_buffer(unsigned char *buf, unsigned int len);
int store_voicemsbc_buffer(unsigned char *buf, unsigned int len);

void btapp_hfp_mic_need_skip_frame_set(int32_t skip_frame);

extern "C" bool bt_media_cur_is_bt_stream_media(void);
bool bt_is_sco_media_open();
extern bool app_audio_list_playback_exist(void);
extern void app_hfp_ag_event_callback(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx);

app_hfp_hf_callback_t g_app_bt_hfp_hf_cb = NULL;
void app_bt_audio_hfp_recon_timeout_handler(void const *param);
osTimerDef (APP_HFP_GET_CLCC_TIMER, app_bt_audio_hfp_get_clcc_timeout_handler);
osTimerDef (APP_HFP_RECONN_TIMER0, app_bt_audio_hfp_recon_timeout_handler);
osTimerDef (APP_HFP_RECONN_TIMER1, app_bt_audio_hfp_recon_timeout_handler);

#define APP_BT_HFP_HF_CB(event,param) \
    do { \
        if (g_app_bt_hfp_hf_cb != NULL) \
            g_app_bt_hfp_hf_cb(event, param); \
    } while (0)

extern void hfcall_next_sta_handler(uint8_t device_id, hf_event_t event);
#ifndef _SCO_BTPCM_CHANNEL_
struct hf_sendbuff_control  hf_sendbuff_ctrl;
#endif
#ifdef __INTERACTION__
const char* oppo_self_defined_command_response = "+VDSF:7";
#endif
#if defined(SCO_LOOP)
#define HF_LOOP_CNT (20)
#define HF_LOOP_SIZE (360)

static uint8_t hf_loop_buffer[HF_LOOP_CNT*HF_LOOP_SIZE];
static uint32_t hf_loop_buffer_len[HF_LOOP_CNT];
static uint32_t hf_loop_buffer_valid = 1;
static uint32_t hf_loop_buffer_size = 0;
static char hf_loop_buffer_w_idx = 0;
#endif

#ifdef __IAG_BLE_INCLUDE__
static void app_hfp_resume_ble_adv(void);
#endif

#define HFP_AUDIO_CLOSED_DELAY_RESUME_ADV_IN_MS     1500
static void app_hfp_audio_closed_delay_resume_ble_adv_timer_cb(void const *n);
osTimerDef (APP_HFP_AUDIO_CLOSED_DELAY_RESUME_BLE_ADV_TIMER,
                        app_hfp_audio_closed_delay_resume_ble_adv_timer_cb);
osTimerId   app_hfp_audio_closed_delay_resume_ble_adv_timer_id = NULL;

static uint8_t report_battery_level = 0xff;

WEAK void app_bt_audio_check_hf_pause_a2dp_timer_handler(void const *param)
{

}

WEAK void app_bt_delay_send_hfp_hold_callback(void const *param)
{
 
}

WEAK bool app_audio_manager_hfp_is_active(int id)
{
    return false;
}

WEAK int app_audio_manager_ctrl_volume(enum APP_AUDIO_MANAGER_VOLUME_CTRL_T volume_ctrl, uint16_t volume_level)
{
    return 0;
}

void app_bt_audio_hfp_recon_timeout_handler(void const *param)
{
    struct BT_DEVICE_T *curr_device = (struct BT_DEVICE_T*)param;

    if(curr_device && curr_device->acl_is_connected && (curr_device->hf_conn_flag == false))
    {
#if defined(IBRT_UI)
        if (app_ibrt_conn_get_mobile_conn_state(&(curr_device->remote)) == IBRT_CONN_ACL_DISCONNECTING)
        {
            TRACE(2,"(d%x) acl disconnecting not reconnect hfp", curr_device->device_id);
        }
        else
#endif // IBRT_UI
        {
            app_bt_reconnect_hfp_profile(&(curr_device->remote));
        }
    }
    return;
}

void app_hfp_set_battery_level(uint8_t level)
{
    if (report_battery_level == 0xff) {
        report_battery_level = level;
        btif_osapi_notify_evm();
    }
}

#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)

int app_hfp_battery_report_reset(uint8_t bt_device_id)
{
    app_bt_get_device(bt_device_id)->battery_level = 0xff;
    return 0;
}

#ifdef __INTERACTION_CUSTOMER_AT_COMMAND__
#define HF_COMMAND_BATTERY_HEAD              		"AT+VDBTY="
#define HF_COMMAND_VERSION_HEAD              		"AT+VDRV="
#define HF_COMMAND_FEATURE_HEAD              		"AT+VDSF="
#define REPORT_NUM                          		3
#define LEFT_UNIT_REPORT					       	1
#define RIGHT_UNIT_REPORT      						2
#define BOX_REPORT						        	3

bt_status_t Send_customer_battery_report_AT_command(btif_hf_channel_t* chan_h, uint8_t level)
{
    char at_cmd[ATCMD_SZ] = {'\0'};
    char buf[20] = {'\0'};
    uint8_t len;

    DEBUG_INFO(0,"Send battery report at commnad.");
    /// head and keyNumber
    sprintf(at_cmd, "%s%d", HF_COMMAND_BATTERY_HEAD, REPORT_NUM);
    /// keys and corresponding values
    sprintf(buf, ",%d,%d,%d,%d,%d,%d\r", LEFT_UNIT_REPORT, level,
            RIGHT_UNIT_REPORT, level, BOX_REPORT, 9);

    len = strlen(at_cmd);
    if ((len + strlen(buf)) > ATCMD_SZ)
        return -1;

    strcpy(&at_cmd[len], buf);
    /// send AT command
    return btif_hf_send_at_cmd(chan_h, at_cmd);
}

bt_status_t Send_customer_phone_feature_support_AT_command(btif_hf_channel_t* chan_h, uint8_t val)
{
    char at_cmd[ATCMD_SZ] = {'\0'};

    DEBUG_INFO(0,"Send_customer_phone_feature_support_AT_command.");
	/// keys and corresponding values
    sprintf(at_cmd, "%s%d\r", HF_COMMAND_FEATURE_HEAD, val);
	/// send AT command
    return btif_hf_send_at_cmd(chan_h, at_cmd);
}

bt_status_t Send_customer_version_report_AT_command(btif_hf_channel_t* chan_h)
{
    char at_cmd[ATCMD_SZ] = {'\0'};
    char buf[30] = {'\0'};
    uint8_t len;

    DEBUG_INFO(0,"Send version report at commnad.");
    /// head and keyNumber
    sprintf(at_cmd, "%s%d", HF_COMMAND_VERSION_HEAD, REPORT_NUM);
    /// keys and corresponding values
    sprintf(buf, ",%d,%d,%d,%d,%d,%d\r", LEFT_UNIT_REPORT, 0x1111,
            RIGHT_UNIT_REPORT, 0x2222, BOX_REPORT, 0x3333);
    len = strlen(at_cmd);
    if ((len + strlen(buf)) > ATCMD_SZ)
        return -1;

    strcpy(&at_cmd[len], buf);
    /// send AT command
    return btif_hf_send_at_cmd(chan_h, at_cmd);
}

#endif

#ifdef __INTERCONNECTION__
uint8_t ask_is_selfdefined_battery_report_AT_command_support(void)
{
    char at_cmd[ATCMD_SZ] = {'\0'};

    DEBUG_INFO(0,"ask if mobile support self-defined at commnad.");
    uint8_t *support = app_battery_get_mobile_support_self_defined_command_p();
    *support = 0;

    sprintf(at_cmd, "%s?\r", HF_COMMAND_HUAWEI_BATTERY_HEAD);
    btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, at_cmd);

    return 0;
}

uint8_t send_selfdefined_battery_report_AT_command(uint8_t device_id)
{
    uint8_t *support = app_battery_get_mobile_support_self_defined_command_p();
    uint8_t batteryInfo = 0;
    char at_cmd[ATCMD_SZ] = {'\0'};
    char buf[30] = {'\0'};

    if(*support)
    {
        app_battery_get_info(NULL, &batteryInfo, NULL);

        /// head and keyNumber
        sprintf(at_cmd, "%s%d", HF_COMMAND_HUAWEI_BATTERY_HEAD, BATTERY_REPORT_NUM);

        /// keys and corresponding values
        sprintf(buf, ",%d,%d,%d,%d\r", BATTERY_REPORT_KEY_LEFT_BATTERY_LEVEL,
                batteryInfo & 0x7f,
                BATTERY_REPORT_KEY_LEFT_CHARGE_STATE,
                (batteryInfo & 0x80) ? 1 : 0);
        uint8_t len;

        len = strlen(at_cmd);

        if ((len + strlen(buf)) > ATCMD_SZ)
            return -1;

        strcpy(&at_cmd[len], buf);

        /// send AT command
        btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(device_id)->hf_channel, at_cmd);
    }
    return 0;
}
#endif
#endif

int app_hfp_battery_report(uint8_t level)
{
    bt_status_t status = BT_STS_FAILED;
    btif_hf_channel_t* chan;

    uint8_t i;
    int nRet = 0;

    if (level>9)
        return -1;

    for(i=0; i<BT_DEVICE_NUM; i++)
    {
        chan = app_bt_get_device(i)->hf_channel;
        if (btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN)
        {
            if (!besbt_cfg.apple_hf_at_support || btif_hf_is_hf_indicators_support(chan))
            {
                if (app_bt_get_device(i)->battery_level != level)
                {
                    status = btif_hf_update_indicators_batt_level(chan, level*10); //battery range 0~100
                }
            }
            else if (btif_hf_is_batt_report_support(chan))
            {
                if (app_bt_get_device(i)->battery_level != level)
                {
                #ifdef GFPS_ENABLED
                    gfps_send_battery_levels(SET_BT_ID(i));
                #endif
#ifdef __INTERACTION_CUSTOMER_AT_COMMAND__
                    status = Send_customer_battery_report_AT_command(chan, level);
#endif
                    status = btif_hf_batt_report(chan, level); //iphoneaccev battery range 0~9
                }
            }
            if (BT_STS_PENDING == status){
                app_bt_get_device(i)->battery_level = level;
            }
            else
            {
                nRet = -1;
            }
        }
        else
        {
             app_bt_get_device(i)->battery_level = 0xff;
             nRet = -1;
        }
    }
#if defined(BT_HFP_AG_ROLE)
    app_hfp_ag_battery_report(level);
#endif
    return nRet;
}

void app_bt_hf_report_batt_level(btif_hf_channel_t* chan_h, uint8_t level)
{
    if (!chan_h || btif_get_hf_chan_state(chan_h) != BT_HFP_CHAN_STATE_OPEN)
    {
        return;
    }

    if (btif_hf_is_hf_indicators_support(chan_h))
    {
        btif_hf_update_indicators_batt_level(chan_h, level); //battery range 0~100
        return;
    }

    // battery range change to 0~9
    level = level / 10;
    level = level > 9 ? 9 : level;

    if (besbt_cfg.apple_hf_at_support)
    {
        btif_hf_batt_report(chan_h, level); //iphoneaccev battery range 0~9
        return;
    }
}

void app_bt_hfp_enahnced_battery_report(uint8_t level) // level range 0~100
{
    btif_hf_channel_t* chan = NULL;

    for(int i = 0; i < BT_DEVICE_NUM; i++)
    {
        chan = app_bt_get_device(i)->hf_channel;
        if (btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN)
        {
            if (app_bt_get_device(i)->battery_level != level)
            {
                app_bt_hf_report_batt_level(chan, level);
                app_bt_get_device(i)->battery_level = level;
            }
        }
        else
        {
            app_bt_get_device(i)->battery_level = 0xff;
        }
    }

    // battery range change to 0~9
    level = level / 10;
    level = level > 9 ? 9 : level;

#if defined(BT_HFP_AG_ROLE)
    app_hfp_ag_battery_report(level);
#endif
}

void app_hfp_battery_report_proc(void)
{
    if(report_battery_level != 0xff)
    {
        app_hfp_battery_report(report_battery_level);
        report_battery_level = 0xff;
    }
}

bt_status_t app_hfp_send_at_command(const char *cmd)
{
    bt_status_t ret = 0;
    //send AT command
    ret = btif_hf_send_at_cmd((btif_hf_channel_t*)app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, cmd);

    return ret;
}

static void app_hfp_handle_call_hold(btif_hf_channel_t* chan_h, uint32_t action)
{
    btif_hf_call_hold(chan_h, (btif_hf_hold_call_t)action, 0);
}

void app_hfp_send_call_hold_request(uint8_t device_id, btif_hf_hold_call_t action)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    if (curr_device)
    {
#ifdef IBRT_UI
        if (tws_besaud_is_connected() &&
            (IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote)))
        {
            app_hfp_hold_call_t hfp_hold_call;
            hfp_hold_call.action = action;
            hfp_hold_call.device_id = device_id;
            tws_ctrl_send_cmd(APP_TWS_CMD_LET_MASTER_SEND_AT_CHLD, (uint8_t*)&hfp_hold_call, sizeof(hfp_hold_call));
        }
        else
#endif
        {
            app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->hf_channel,
                (uint32_t)action, (uint32_t)(uintptr_t)app_hfp_handle_call_hold);
        }
    }
}

void app_hfp_report_battery_hf_indicator(uint8_t device_id, uint8_t level)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->hf_channel,
            (uint32_t)level, (uint32_t)(uintptr_t)btif_hf_update_indicators_batt_level);
    }
}

void app_hfp_report_enhanced_safety(uint8_t device_id, uint8_t value)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (curr_device)
    {
        app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->hf_channel,
            (uint32_t)value, (uint32_t)(uintptr_t)btif_hf_report_enhanced_safety);
    }
}

void app_bt_hf_send_at_command(uint8_t device_id, const char* at_str)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (!curr_device || !curr_device->hf_conn_flag)
    {
        DEBUG_INFO(2, "(d%x) %s invalid device", device_id, __func__);
        return;
    }
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->hf_channel, (uint32_t)at_str, (uint32_t)(uintptr_t)btif_hf_send_at_cmd);
}

void app_bt_hf_create_sco_directly(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if (!curr_device || !curr_device->hf_conn_flag)
    {
        DEBUG_INFO(2, "(d%x) %s invalid device", device_id, __func__);
        return;
    }
    app_bt_start_custom_function_in_bt_thread((uint32_t)(uintptr_t)curr_device->hf_channel, (uint32_t)NULL, (uint32_t)(uintptr_t)btif_hf_create_sco_directly);
}

bool app_bt_is_hfp_disconnected(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    return curr_device && (0 == curr_device->hf_conn_flag);
}

bool app_hfp_curr_audio_up(btif_hf_channel_t* hfp_chnl)
{
    int i = 0;
    struct BT_DEVICE_T* curr_device = NULL;
    for(i=0;i<BT_DEVICE_NUM;i++)
    {
        curr_device = app_bt_get_device(i);
        if (curr_device->hf_channel == hfp_chnl) {
            return curr_device->hf_conn_flag && curr_device->hf_audio_state == BT_HFP_AUDIO_CON;
        }
    }
    return false;
}

#ifdef SUPPORT_SIRI
extern int open_siri_flag ;
int app_hfp_siri_voice(bool en)
{
    static int device_id = BT_DEVICE_INVALID_ID;
    struct BT_DEVICE_T* curr_device = NULL;

    device_id = app_audio_adm_get_bt_active_device();

    if(device_id == BT_DEVICE_INVALID_ID)
    {
        DEBUG_INFO(0, "active device is null");
        return -1;
    }

    curr_device = app_bt_get_device(device_id);

    if(open_siri_flag == 1)
    {
        if(btif_hf_is_voice_rec_active(curr_device->hf_channel) == false)
        {
            open_siri_flag = 0;
            DEBUG_INFO(0,"end auto");
        }
        else
        {
            DEBUG_INFO(0,"need close");
            en = false;
        }
    }

    if(open_siri_flag == 0)
    {
        if((btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN)) 
        {
            btif_hf_enable_voice_recognition(curr_device->hf_channel, en);
        }
    }

    DEBUG_INFO(4,"[%s]id =%d/%d/%d",__func__,device_id,open_siri_flag,en);

    return 0;
}
#endif

#if defined(APP_DEBUG_TOOL_BT_HFP_AT)
/**
 * Input cmd: "+ANDROID: xxx"
 * Return: "AT+ANDROID=f00,3"
 */
#include <stdarg.h>
#include "stdio.h"

#define BT_DEBUG_RESERVED_SIZE      (20)
#define BT_DEBUG_DATA_SIZE          (1024)
static char g_bt_debug_tx_buf[BT_DEBUG_RESERVED_SIZE + BT_DEBUG_DATA_SIZE];
static uint32_t g_at_cmd_debug_device_id = 0;

static void _strrpl(char *buf, char s1, char s2)
{
    uint32_t len = strlen(buf);

    for (uint32_t i=0; i<len; i++) {
        if (buf[i] == s1) {
            buf[i] = s2;
        }
    }
}

static void app_debug_tool_send(char *buf)
{
    _strrpl(buf, ' ', '^');
    /* btif_hf_send_at_cmd needs more delay than app_bt_hf_send_at_command */
    btif_hf_send_at_cmd(app_bt_get_device(g_at_cmd_debug_device_id)->hf_channel, buf);
    // app_bt_hf_send_at_command(device_id, cmd);
}

extern "C" void app_debug_tool_printf(const char *fmt, ...)
{
    va_list arg;
    char *buf_ptr = g_bt_debug_tx_buf;

    memset(g_bt_debug_tx_buf, 0, sizeof(g_bt_debug_tx_buf));
    memcpy(buf_ptr, "AT+ANDROID=", strlen("AT+ANDROID="));
    buf_ptr += strlen("AT+ANDROID=");

    va_start(arg, fmt);
    vsprintf(buf_ptr, fmt, arg);
    va_end(arg);

    strcat(buf_ptr, "\r");

    ASSERT(strlen(g_bt_debug_tx_buf) < sizeof(g_bt_debug_tx_buf),
        "[%s] Invalid length: %d", __func__, strlen(g_bt_debug_tx_buf));

    DEBUG_INFO(2, "[%s] %s", __func__, g_bt_debug_tx_buf);
    app_debug_tool_send(g_bt_debug_tx_buf);
}

extern "C" uint32_t app_trace_rx_process(uint8_t *buf, uint32_t len);
static void app_debug_tool_hfp_at_cmd_receive(uint32_t device_id, const char *cmd)
{
    uint32_t ret = 0;

    DEBUG_INFO(3, "[%s] id: %d, cmd: %s", __func__, device_id, cmd);

    if (strncmp("+ANDROID: ", cmd, strlen("+ANDROID: ")) != 0) {
        DEBUG_INFO(0, "[%s] WARNING: Can not find cmd head", __func__);
        return;
    }

    g_at_cmd_debug_device_id = device_id;

    cmd += strlen("+ANDROID: ");

    ret = app_trace_rx_process((uint8_t *)cmd, strlen(cmd) + 1);

    if (ret) {
        app_debug_tool_printf("cmd, ERROR: %d", ret);
    } else {
        // app_debug_tool_printf("cmd, Rx cmd");
    }
}
#endif

uint8_t hfp_volume_local_get(int id)
{
    uint8_t localVol = hal_codec_get_default_dac_volume_index();
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && !nv_record_btdevicerecord_find(&curr_device->remote, &record))
    {
        localVol = record->device_vol.hfp_vol;
    }

    return localVol;
}

uint8_t hfp_convert_local_vol_to_bt_vol(uint8_t localVol)
{
    return unsigned_range_value_map(localVol, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX, 0, MAX_HFP_VOL);
}

uint8_t hfp_volume_get(int id)
{
    int8_t localVol = hfp_volume_local_get(id);

    return hfp_convert_local_vol_to_bt_vol(localVol);
}

void hfp_volume_local_set(int id, uint8_t vol)
{
    nvrec_btdevicerecord *record = NULL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(id);

    if (curr_device && curr_device->acl_is_connected)
    {
        DEBUG_INFO(3, "(d%x) %s hfp_vol local %d", id, __func__, vol);

        if (!nv_record_btdevicerecord_find(&curr_device->remote, &record) && record->device_vol.hfp_vol != vol)
        {
            nv_record_btdevicerecord_set_hfp_vol(record, vol);
#ifndef FPGA
            nv_record_touch_cause_flush();
#endif
        }
    }
}

uint8_t hfp_convert_bt_vol_to_local_vol(uint8_t btVol)
{
    return unsigned_range_value_map(btVol, 0, MAX_HFP_VOL, TGT_VOLUME_LEVEL_MUTE, TGT_VOLUME_LEVEL_MAX);
}

void hfp_update_local_volume(int id, uint8_t localVol)
{
    // update nv record
    hfp_volume_local_set(id, localVol);

    // update codec if the codec is active
    if (app_audio_manager_hfp_is_active(id))
    {
        app_audio_manager_ctrl_volume(APP_AUDIO_MANAGER_VOLUME_CTRL_SET, localVol);
    }
}

void hfp_volume_set(int id, uint8_t btVol)
{
    uint8_t localVol = hfp_convert_bt_vol_to_local_vol(btVol);

    hfp_update_local_volume(id, localVol);

    DEBUG_INFO(2, "Set hfp bt vol %d local vol %d", btVol, localVol);
}

static void hfp_remote_not_support_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    DEBUG_INFO(1,"(d%x) ::HF_EVENT_REMOTE_NOT_SUPPORT\n", device_id);

    app_bt_profile_connect_manager_hf(device_id, chan, ctx);
}

static bool hfp_current_is_idle_state(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device && curr_device->hf_audio_state == BT_HFP_AUDIO_DISCON)
    {
        if(!curr_device->hfchan_call && !curr_device->hfchan_callSetup && !curr_device->hf_callheld){
            return true;
        }
    }
    return false;
}

static void hfp_connected_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    bt_bdaddr_t peer_addr;
    app_hfp_hf_event_param_t param;
    uint8_t hfp_volume = MAX_HFP_VOL;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(1,"(d%x) ::HF_EVENT_SERVICE_CONNECTED\n", device_id);

#ifdef GFPS_ENABLED
    app_exit_fastpairing_mode();
#endif

    // 20210423:move bixby resume on to here from a2dp connect. Because during voice calling,
    // there is a probability for receiving a2dp close and a2dp connect, it will lead to not resume bixby on after
    // voice calling hung up.
#ifdef SS_BIXBY_INTEGRATED
    app_bixby_on_resume_handle();
#endif

    app_bt_clear_connecting_profiles_state(device_id);

    curr_device->switch_sco_to_earbud = 1;
    curr_device->hf_conn_flag = 1;

#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
    uint8_t battery_level;
    app_hfp_battery_report_reset(device_id);
    app_battery_get_info(NULL, &battery_level, NULL);
    app_hfp_set_battery_level(battery_level);
#endif

    //app_bt_stream_hfpvolume_reset();
    hfp_volume = hfp_volume_get((enum BT_DEVICE_ID_T)device_id);
    /*
    if(hfp_volume == MAX_HFP_VOL || hfp_volume == MIN_HFP_VOL)
    {
        hfp_volume_set(device_id, app_bt_manager.config.hfp_default_volume);
        hfp_volume = hfp_volume_get((enum BT_DEVICE_ID_T)device_id);
    }
    */
    DEBUG_INFO(1,"hfp connect report volume:%d", hfp_volume);
    btif_hf_report_speaker_volume(chan, hfp_volume);
#if defined(HFP_DISABLE_NREC)
    btif_hf_disable_nrec(chan);
#endif

#if defined(BT_MAP_SUPPORT)
    bt_map_connect(&curr_device->remote);
#endif

    app_bt_profile_connect_manager_hf(device_id, chan, ctx);

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SERVICE_CONNECTED, 0);

#if defined(IBRT) && defined(IBRT_UI)
    app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_HFP_PROFILE_ID);
    app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_HFP_PROFILE_ID);
#endif

    btif_hf_get_remote_bdaddr(chan, &peer_addr);
    param.p.service_connected.device_id = device_id;
    param.p.service_connected.addr      = &peer_addr;
    param.p.service_connected.channel   = chan;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_SERVICE_CONNECTED, &param);

#ifdef __INTERCONNECTION__
    ask_is_selfdefined_battery_report_AT_command_support();
#endif

#ifdef __INTERACTION_CUSTOMER_AT_COMMAND__
    Send_customer_phone_feature_support_AT_command(chan,7);
    Send_customer_battery_report_AT_command(chan, battery_level);
#endif
}

static void hfp_disconnected_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    if (BT_ECODE_IBRT_SLAVE_CLEANUP == ctx->disc_reason)
    {
        return;
    }
    app_hfp_hf_event_param_t param;
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint32_t sco_is_connected = (curr_device->hf_audio_state == BT_HFP_AUDIO_CON);

    TRACE(2,"(d%x) ::HF_EVENT_SERVICE_DISCONNECTED reason=%x\n", device_id, ctx->disc_reason);

    btif_hf_set_negotiated_codec(chan, BT_HFP_SCO_CODEC_CVSD);
    curr_device->hf_conn_flag = 0;
    curr_device->hfchan_call = 0;
    curr_device->hfchan_callSetup = 0;
    curr_device->hf_callheld = 0;
    curr_device->hf_audio_state = BT_HFP_AUDIO_DISCON;
    osTimerStop(curr_device->clcc_timer);
    app_bt_profile_connect_manager_hf(device_id, chan, ctx);

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SERVICE_DISCONNECTED, sco_is_connected);

#if defined(IBRT) && defined(IBRT_UI)
    app_ibrt_clear_profile_connect_protect(device_id, APP_IBRT_HFP_PROFILE_ID);
    app_ibrt_clear_profile_disconnect_protect(device_id, APP_IBRT_HFP_PROFILE_ID);
#endif

    param.p.service_disconnected.device_id = device_id;
    param.p.service_disconnected.channel   = chan;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_SERVICE_DISCONNECTED, &param);
}

static void hfp_audio_data_sent_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
#if defined(SCO_LOOP)
    hf_loop_buffer_valid = 1;
#endif
}

static void hfp_audio_data_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
#if defined(SCO_LOOP)
    memcpy(hf_loop_buffer + hf_loop_buffer_w_idx*HF_LOOP_SIZE, Info->p.audioData->data, Info->p.audioData->len);
    hf_loop_buffer_len[hf_loop_buffer_w_idx] = Info->p.audioData->len;
    hf_loop_buffer_w_idx = (hf_loop_buffer_w_idx+1)%HF_LOOP_CNT;
    ++hf_loop_buffer_size;

    if (hf_loop_buffer_size >= 18 && hf_loop_buffer_valid == 1) {
        hf_loop_buffer_valid = 0;
        idx = hf_loop_buffer_w_idx-17<0?(HF_LOOP_CNT-(17-hf_loop_buffer_w_idx)):hf_loop_buffer_w_idx-17;
        pkt.flags = BTP_FLAG_NONE;
        pkt.dataLen = hf_loop_buffer_len[idx];
        pkt.data = hf_loop_buffer + idx*HF_LOOP_SIZE;
        HF_SendAudioData(Chan, &pkt);
    }
#endif
}

static void hfp_call_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    uint8_t prev_hf_call = 0;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(4,"(d%x) ::HF_EVENT_CALL_IND %d curr_hfp_device %x\n", device_id, ctx->call, app_bt_audio_get_curr_hfp_device());

    prev_hf_call = curr_device->hfchan_call;
    curr_device->hfchan_call = ctx->call;
    if(prev_hf_call == BT_HFP_CALL_ACTIVE && ctx->call == BT_HFP_CALL_NONE)
    {
        DEBUG_INFO(1,"d%x hungup call", device_id);
    }
    DEBUG_INFO(2, "(d%x) hfp state: %s\n", device_id, app_bt_hf_get_all_device_state());

    if(ctx->call == BT_HFP_CALL_NONE)
    {
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("Call hangup ok.");
#endif
        curr_device->hf_callheld = BT_HFP_CALL_HELD_NONE;
    }
    else if(ctx->call == BT_HFP_CALL_ACTIVE)
    {
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("Call setup ok.");
#endif
    }
    else
    {
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("Call hangup ok.");
#endif
    }

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALL_IND, prev_hf_call);
    if(prev_hf_call == BT_HFP_CALL_ACTIVE && hfp_current_is_idle_state(device_id))
    {
        TRACE(2,"d%x%s hungup call", device_id, __func__);
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HUNGUP_ACTIVE_CALL, 0);
    }
}

static void hfp_callsetup_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    uint8_t prev_hf_callsetup = 0;

    if (BT_HFP_CALL_SETUP_NONE != ctx->call_setup)
    {
        // exit sniff mode and stay active
        app_bt_set_keep_active_mode(true, BT_ACTIVE_MODE_KEEP_USER_SCO_STREAMING, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
    }
    else
    {
        // resume sniff mode
        app_bt_set_keep_active_mode(false, BT_ACTIVE_MODE_KEEP_USER_SCO_STREAMING, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
    }

    if(BT_HFP_CALL_SETUP_OUT == ctx->call_setup || BT_HFP_CALL_SETUP_ALERT == ctx->call_setup)
    {
        app_hf_send_current_call_at_commond(chan);
        osTimerStart(curr_device->clcc_timer, APP_BT_HFP_GET_CLCC_TIMEOUT);
        DEBUG_INFO(0,"###check virtual call send CLCC");
    }
    //call is alert so remember this time
    if(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT) {
        TRACE(1,"HF CALLSETUP TIME=%d",hal_sys_timer_get());
        //curr_device->hf_callsetup_alert_time = hal_sys_timer_get();
    }

    prev_hf_callsetup = curr_device->hfchan_callSetup;
    curr_device->hfchan_callSetup = ctx->call_setup;

    DEBUG_INFO(3, "(d%x) ::HF_EVENT_CALLSETUP_IND %d %s\n", device_id, ctx->call_setup, app_bt_hf_get_all_device_state());
#ifndef SASS_ENABLED
    if(curr_device->hfchan_callSetup == BT_HFP_CALL_SETUP_IN) {
        btif_hf_list_current_calls(chan);
    }
#endif

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALLSETUP_IND, prev_hf_callsetup);
    if(prev_hf_callsetup == BT_HFP_CALL_SETUP_NONE && hfp_current_is_idle_state(device_id))
    {
        TRACE(2,"d%x%s hungup inconming call", device_id, __func__);
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HUNGUP_INCOMING_CALL, 0);
    }
}

static void hfp_call_held_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    uint8_t prev_hf_callheld = 0;

    prev_hf_callheld = curr_device->hf_callheld;
    curr_device->hf_callheld = ctx->call_held;

    DEBUG_INFO(3,"(d%x) ::HF_EVENT_CALLHELD_IND %d %s\n", device_id, ctx->call_held, app_bt_hf_get_all_device_state());

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALLHELD_IND, prev_hf_callheld);
}

static void hfp_ring_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    app_hfp_hf_event_param_t param;
    POSSIBLY_UNUSED struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(2,"(d%x) ::HF_EVENT_RING_IND in-band-ring-tone %d\n", device_id, btif_hf_is_inbandring_enabled(curr_device->hf_channel));
    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_RING_IND, 0);
    param.p.ring_ind.channel   = chan;
    param.p.ring_ind.device_id = device_id;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_RING_IND, &param);
}

static void hfp_current_call_state_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    app_hfp_hf_event_param_t param;
    DEBUG_INFO(2,"(d%x) ::HF_EVENT_CURRENT_CALL_STATE call_number:%s\n", device_id, ctx->call_number);

    const char *buffer1 = "10000000";
    const char *buffer2 = "00000000000";

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        DEBUG_INFO(0, "d(%d) device NULL", device_id);
        return;
    }

    osTimerStop(curr_device->clcc_timer);

    if(memcmp(buffer1, ctx->call_number, 8) == 0 ||
    memcmp(buffer2, ctx->call_number, 8) == 0)
    {
        DEBUG_INFO(0,"virtual_call!!!");
        btif_hf_set_virtual_call_enable(curr_device->hf_channel);
    }

    char *ret = NULL;
    DEBUG_INFO(0, "card_number %s call_number %s", curr_device->card_number.number, ctx->call_number);
    if(strlen(curr_device->card_number.number) != 0 && strlen(ctx->call_number) != 0)
    {
        //Does it include each other:maybe cnum:+86xx call_number:86xx or cnum:86xx  call_number: +86xx
        ret = strstr(curr_device->card_number.number, ctx->call_number);
        if (ret)
        {
            TRACE(0, "callnumber is card number, virtual_call!!!");
            btif_hf_set_virtual_call_enable(chan);
        }
        ret = strstr(ctx->call_number, curr_device->card_number.number);
        if (ret)
        {
            TRACE(0, "callnumber is card number 01, virtual_call!!!");
            btif_hf_set_virtual_call_enable(chan);
        }
    }
    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CLCC_IND, 0);

    param.p.caller_id_ind.number           = ctx->call_number;
    if (ctx->call_number == NULL)
        param.p.caller_id_ind.number_len   = 0;
    else
        param.p.caller_id_ind.number_len   = strlen(ctx->call_number);
    param.p.caller_id_ind.channel          = chan;
    param.p.caller_id_ind.device_id        = device_id;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_CALLER_ID_IND, &param);
}

static void hfp_cnum_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (curr_device == NULL)
    {
        DEBUG_INFO(0, "d(%d) device NULL", device_id);
        return;
    }

    uint8_t sim_card_number_len = strlen(ctx->sim_card_number);
    DEBUG_INFO(2,"d(%d) hfp_cnum_ind_handler len %d number %s \n", device_id, sim_card_number_len, ctx->sim_card_number);

    memcpy(curr_device->card_number.number, ctx->sim_card_number, sim_card_number_len+1); //+1 is for \0

    DEBUG_INFO(2,"d(%d) hfp_cnum_ind_handler len %d number %s \n", device_id, sim_card_number_len, curr_device->card_number.number);
}

void app_hfp_mute_upstream(uint8_t devId, bool isMute);

void app_hfp_receive_peer_sco_codec(uint8_t device_id, void *chan, uint8_t codec_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    ASSERT(device_id < BT_DEVICE_NUM && curr_device->hf_channel == chan, "hfp device channel must match");
    btif_hf_set_negotiated_codec(curr_device->hf_channel, (hfp_sco_codec_t)codec_id);
    bt_adapter_set_hfp_sco_codec_type(device_id, codec_id);
    app_bt_audio_peer_sco_codec_received(device_id);
}

static void hfp_audio_connected_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    app_hfp_hf_event_param_t param;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    uint8_t codec_id = 0;
    if(ctx->status != BT_STS_SUCCESS)
        return;

#ifdef __AI_VOICE__
    app_ai_if_hfp_connected_handler(device_id);
#endif

    btapp_hfp_mic_need_skip_frame_set(2);
#if defined(IBRT)
    if (ctx->sco_codec != 0xff) {
        codec_id = ctx->sco_codec;
        btif_hf_set_negotiated_codec(curr_device->hf_channel, (hfp_sco_codec_t)codec_id);
        DEBUG_INFO(2,"(d%x) ::HF_EVENT_AUDIO_CONNECTED codec_id:%d\n", device_id, codec_id);
    } else {
        codec_id = btif_hf_get_negotiated_codec(curr_device->hf_channel);
        DEBUG_INFO(2,"(d%x) ::HF_EVENT_AUDIO_CONNECTED codec_id:%d\n", device_id, codec_id);
    }
#else
    codec_id = btif_hf_get_negotiated_codec(curr_device->hf_channel);
    DEBUG_INFO(2,"(d%x) ::HF_EVENT_AUDIO_CONNECTED codec_id:%d\n", device_id, codec_id);
#endif
    bt_adapter_set_hfp_sco_codec_type(device_id, codec_id);
    curr_device->switch_sco_to_earbud = 0;
    curr_device->hf_audio_state = BT_HFP_AUDIO_CON;

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SCO_CONNECTED, 0);
    param.p.audio_connected.device_id = device_id;
    param.p.audio_connected.channel   = chan;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_AUDIO_CONNECTED, &param);

#if defined(__FORCE_REPORTVOLUME_SOCON__)
    btif_hf_report_speaker_volume(chan, hfp_volume_get(device_id));
#endif

#ifdef __IAG_BLE_INCLUDE__
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_HFP_ON, true);
#endif
}

static void hfp_audio_con_fail_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(1,"(d%x) ::HF_EVENT_AUDIO_CON_FAIL\n", device_id);

    curr_device->hf_audio_state = BT_HFP_AUDIO_DISCON;

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SCO_CON_FAIL_IND, ctx->sco_con_fail_reason);
}

static void hfp_audio_disconnected_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    app_hfp_hf_event_param_t param;
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(1,"(d%x) ::HF_EVENT_AUDIO_DISCONNECTED\n", device_id);


#ifdef BT_USB_AUDIO_DUAL_MODE
    if(!btusb_is_bt_mode())
    {
        DEBUG_INFO(0,"btusb_usbaudio_open doing.");
        btusb_usbaudio_open();
    }
#endif

    if(curr_device->hfchan_call == BT_HFP_CALL_ACTIVE) {
        curr_device->switch_sco_to_earbud = 1;
    }

    curr_device->hf_audio_state = BT_HFP_AUDIO_DISCON;

    //app_hfp_mute_upstream(device_id, true);

    /* Dont clear callsetup status when audio disc: press iphone volume button
       will disc audio link, but the iphone incoming call is still exist. The
       callsetup status will be reported after call rejected or answered. */
    //curr_device->hfchan_callSetup = BT_HFP_CALL_SETUP_NONE;

    bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SCO_DISCONNECTED, 0);

    param.p.audio_disconnected.device_id = device_id;
    param.p.audio_disconnected.channel   = chan;
    APP_BT_HFP_HF_CB(APP_HFP_HF_EVENT_AUDIO_DISCONNECTED, &param);

#ifdef __IAG_BLE_INCLUDE__
    if (!app_bt_is_in_reconnecting()) {
        app_hfp_resume_ble_adv();
    }
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_HFP_ON, false);
#endif

    app_bt_set_keep_active_mode(false, BT_ACTIVE_MODE_KEEP_USER_SCO_STREAMING, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
}

void hfp_speak_volume_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    DEBUG_INFO(2,"::HF_EVENT_SPEAKER_VOLUME  chan_id:%d,speaker gain = %d\n", device_id, ctx->speaker_volume);
    hfp_volume_set(device_id, (uint8_t)ctx->speaker_volume);
}

static void hfp_voice_rec_state_ind_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    DEBUG_INFO(2,"::HF_EVENT_VOICE_REC_STATE  chan_id:%d,voice_rec_state = %d\n",
                                    device_id, ctx->voice_rec_state);
}

static void hfp_bes_test_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    //DEBUG_INFO(0,"HF_EVENT_BES_TEST content =d", Info->p.ptr);
}

static void hfp_read_ag_ind_status_handler(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    DEBUG_INFO(1,"HF_EVENT_READ_AG_INDICATORS_STATUS %s\n", __func__);
}

static uint8_t skip_frame_cnt = 0;
void app_hfp_set_skip_frame(uint8_t frames)
{
    skip_frame_cnt = frames;
}
uint8_t app_hfp_run_skip_frame(void)
{
    if(skip_frame_cnt >0){
        skip_frame_cnt--;
        return 1;
    }
    return 0;
}
uint8_t hfp_is_service_connected(uint8_t device_id)
{
    return app_bt_get_device(device_id)->hf_conn_flag;
}

void app_hfp_mute_upstream(uint8_t devId, bool isMute)
{
}

static void app_hfp_audio_closed_delay_resume_ble_adv_timer_cb(void const *n)
{
#ifdef __IAG_BLE_INCLUDE__
    bes_ble_gap_refresh_adv_state(BLE_ADVERTISING_INTERVAL);
#endif
}

#ifdef __IAG_BLE_INCLUDE__
static void app_hfp_resume_ble_adv(void)
{
    if (!app_hfp_audio_closed_delay_resume_ble_adv_timer_id) {
        app_hfp_audio_closed_delay_resume_ble_adv_timer_id =
            osTimerCreate(osTimer(APP_HFP_AUDIO_CLOSED_DELAY_RESUME_BLE_ADV_TIMER),
            osTimerOnce, NULL);
    }

    osTimerStart(app_hfp_audio_closed_delay_resume_ble_adv_timer_id,
        HFP_AUDIO_CLOSED_DELAY_RESUME_ADV_IN_MS);
}
#endif

void app_hfp_bt_driver_callback(uint8_t device_id, hf_event_t event)
{
    struct bt_cb_tag* bt_drv_func_cb = bt_drv_get_func_cb_ptr();
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if(curr_device == NULL)
    {
        ASSERT(0, "hfp device error");
        return;
    }

    switch(event)
    {
        case BTIF_HF_EVENT_AUDIO_CONNECTED:
            if(bt_drv_func_cb->bt_switch_agc != NULL)
            {
                bt_drv_func_cb->bt_switch_agc(BT_HFP_WORK_MODE);
            }
            break;

        case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
            {
                if (app_bt_audio_count_connected_sco() == 0)
                {
                    if(bt_drv_func_cb->bt_switch_agc != NULL)
                    {
                        bt_drv_func_cb->bt_switch_agc(BT_IDLE_MODE);
                    }

                    uint16_t codec_id = btif_hf_get_negotiated_codec(curr_device->hf_channel);
                    bt_drv_reg_op_sco_txfifo_reset(codec_id);
                }

                bt_drv_reg_op_clean_flags_of_ble_and_sco();
            }
            break;

        default:
            break;
    }
}

void app_hfp_event_callback(uint8_t device_id, btif_hf_channel_t* chan, struct hfp_context *ctx)
{
    if (device_id == BT_DEVICE_INVALID_ID && ctx->event == BTIF_HF_EVENT_SERVICE_DISCONNECTED)
    {
        // hfp profile is closed due to acl created fail
        DEBUG_INFO(1,"::HF_EVENT_SERVICE_DISCONNECTED acl created error=%x\n", ctx->disc_reason);
        return;
    }

    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    ASSERT(device_id < BT_DEVICE_NUM && curr_device->hf_channel == chan, "hfp device channel must match");

    DEBUG_INFO(3,"(d%x) %s event %d", device_id, __func__, ctx->event);
    switch(ctx->event) {
    case BTIF_HF_EVENT_SERVICE_CONNECTED:
        curr_device->mock_hfp_after_force_disc = false;
        curr_device->ibrt_slave_force_disc_hfp = false;
        hfp_connected_ind_handler(device_id, chan, ctx);
        if (app_bt_is_a2dp_connected(device_id))
        {
            curr_device->profiles_connected_before = true;
        }
        break;
    case BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED:
        if (app_bt_is_a2dp_connected(device_id))
        {
            curr_device->profiles_connected_before = true;
        }
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SERVICE_MOCK_CONNECTED, 0);
        break;
    case BTIF_HF_EVENT_AUDIO_DATA_SENT:
        hfp_audio_data_sent_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_AUDIO_DATA:
        hfp_audio_data_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_SERVICE_DISCONNECTED:
        hfp_disconnected_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_AUDIO_CONNECTED:
        hfp_audio_connected_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_AUDIO_DISCONNECTED:
        hfp_audio_disconnected_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_AUDIO_CON_FAIL:
        hfp_audio_con_fail_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_BES_TEST:
        hfp_bes_test_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_READ_AG_INDICATORS_STATUS:
        hfp_read_ag_ind_status_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_RING_IND:
        hfp_ring_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_CURRENT_CALL_STATE:
        hfp_current_call_state_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_WAIT_NOTIFY:
        break;
    case BTIF_HF_EVENT_CALL_IND:
        hfp_call_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_CALLSETUP_IND:
        hfp_callsetup_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_CALLHELD_IND:
        hfp_call_held_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_SERVICE_IND:
        break;
    case BTIF_HF_EVENT_SIGNAL_IND:
        break;
    case BTIF_HF_EVENT_ROAM_IND:
        break;
    case BTIF_HF_EVENT_BATTERY_IND:
        break;
    case BTIF_HF_EVENT_SPEAKER_VOLUME:
        hfp_speak_volume_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_MIC_VOLUME:
        break;
    case BTIF_HF_EVENT_VOICE_REC_STATE:
        hfp_voice_rec_state_ind_handler(device_id, chan, ctx);
        break;
#ifdef SUPPORT_SIRI
    case BTIF_HF_EVENT_SIRI_STATUS:
        break;
#endif
    case BTIF_HF_EVENT_COMMAND_COMPLETE:
        break;
    case BTIF_HF_EVENT_AT_RESULT_DATA:
        DEBUG_INFO(2,"(d%x) received AT command: %s", device_id, ctx->ptr);
#ifdef __INTERACTION__
        if(!memcmp(oppo_self_defined_command_response, ctx->ptr, strlen(oppo_self_defined_command_response)))
        {
            for(int i=0; i<BT_DEVICE_NUM; i++)
            {
                chan = app_bt_get_device(i)->hf_channel;
                {
                    DEBUG_INFO(2,"hf state=%p %d",chan, btif_get_hf_chan_state(chan));
                    if (btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN)
                    {
                        //char firmwareversion[] = "AT+VDRV=3,1,9,2,9,3,9";
                        //sprintf(&firmwareversion[27], "%d", (int)NeonFwVersion[0]);
                        //sprintf(&firmwareversion[28], "%d", (int)NeonFwVersion[1]);
                        //sprintf(&firmwareversion[29], "%d", (int)NeonFwVersion[2]);
                        //btif_hf_send_at_cmd(chan,firmwareversion);
                    }
                }
            }
            DEBUG_INFO(0,"oppo_self_defined_command_response");
        }
#endif
#ifdef __INTERCONNECTION__
        if (!memcmp(huawei_self_defined_command_response, ctx->ptr, strlen(huawei_self_defined_command_response)+1))
        {
            uint8_t *support = app_battery_get_mobile_support_self_defined_command_p();
            *support = 1;

            DEBUG_INFO(0,"send self defined AT command to mobile.");
            send_selfdefined_battery_report_AT_command(device_id);
        }
#endif
#if defined(APP_DEBUG_TOOL_BT_HFP_AT)
        app_debug_tool_hfp_at_cmd_receive(device_id, ctx->ptr);
#endif
        break;

    case BTIF_HF_EVENT_REMOTE_NOT_SUPPORT:
#if defined(IBRT) && defined(IBRT_UI)
        bt_bdaddr_t bdaddr;
        if (btif_hf_get_remote_bdaddr(chan, &bdaddr) == true)
        {
            app_tws_profile_remove_from_basic_profiles(&bdaddr,BTIF_APP_HFP_PROFILE_ID);
        }
#endif
        hfp_remote_not_support_ind_handler(device_id, chan, ctx);
        break;
    case BTIF_HF_EVENT_SUBSCRIBER_NUMBER:
        hfp_cnum_ind_handler(device_id, chan, ctx);
        break;
    default:
        break;
    }

    app_hfp_bt_driver_callback(device_id, ctx->event);

    hfcall_next_sta_handler(device_id, ctx->event);

#if defined(IBRT)
    app_tws_ibrt_profile_callback(device_id, BTIF_APP_HFP_PROFILE_ID,(void *)chan, (void *)ctx,&curr_device->remote);
#endif

#if defined(IBRT)
    if (ctx->event == BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED)
    {
        bt_hf_opened_param_t param;
        param.error_code = 0;
        param.peer_feat = app_bt_hf_get_ag_features(curr_device->device_id);
        param.call_status = curr_device->hfchan_call;
        param.callsetup_status = curr_device->hfchan_callSetup;
        param.callhold_status = curr_device->hf_callheld;
        btif_report_bt_event(&curr_device->remote, BT_EVENT_HF_OPENED, &param);
    }
#endif
}

struct btif_hf_cind_value app_bt_hf_get_cind_service_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_service_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_call_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_call_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_callsetup_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_callsetup_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_callheld_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_callheld_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_signal_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_signal_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_roam_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_roam_value(curr_device->hf_channel);
    }
    return value;
}

struct btif_hf_cind_value app_bt_hf_get_cind_battchg_value(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    struct btif_hf_cind_value value = {0};
    if (curr_device)
    {
        value = btif_hf_get_cind_battchg_value(curr_device->hf_channel);
    }
    return value;
}

uint32_t app_bt_hf_get_ag_features(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    uint32_t ag_features = 0;
    if (curr_device)
    {
        ag_features = btif_hf_get_ag_features(curr_device->hf_channel);
    }
    return ag_features;
}

static uint8_t g_bt_hfp_reject_sco_dev= BT_DEVICE_INVALID_ID;

uint8_t app_bt_hf_get_reject_dev(void)
{
    return g_bt_hfp_reject_sco_dev;
}

void app_bt_hf_set_reject_dev(uint8_t device_id)
{
    g_bt_hfp_reject_sco_dev = device_id;
}

uint8_t btapp_hfp_get_call_setup(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++){
        if ((app_bt_get_device(i)->hfchan_callSetup != BT_HFP_CALL_SETUP_NONE)){
            return (app_bt_get_device(i)->hfchan_callSetup);
        }
    }
    return 0;
}

uint8_t btapp_hfp_incoming_calls(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++) {
        if (app_bt_get_device(i)->hfchan_callSetup == BT_HFP_CALL_SETUP_IN) {
            return 1;
        }
    }
    return 0;
}

bool btapp_hfp_is_pc_call_active(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++) {
        if (app_bt_get_device(i)->hf_audio_state == BT_HFP_AUDIO_CON) {
            struct BT_DEVICE_T *curr_device = app_bt_get_device(i);
            uint8_t cod = 0xFF;
            if(curr_device) {
                cod = btif_me_get_remote_class_of_device(curr_device->btm_conn);
                if(cod == APP_BT_COD_COMPUTER){
                    return true;
                }
            }
        }
    }
    return false;
}

bool btapp_hfp_is_call_active(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++){
        if (app_bt_get_device(i)->hfchan_call == BT_HFP_CALL_ACTIVE) {
            return true;
        }
    }
    return false;
}

bool btapp_hfp_is_sco_active(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++) {
        if (app_bt_get_device(i)->hf_audio_state == BT_HFP_AUDIO_CON) {
            return true;
        }
    }
    return false;
}

uint8_t btapp_hfp_get_call_active(void)
{
    uint8_t i;
    for (i = 0; i < BT_DEVICE_NUM; i++) {
        if ((app_bt_get_device(i)->hfchan_call == BT_HFP_CALL_ACTIVE) ||
            (app_bt_get_device(i)->hfchan_callSetup == BT_HFP_CALL_SETUP_ALERT)){

            return 1;
        }
    }
    return 0;
}

bool btapp_hfp_is_dev_sco_connected(uint8_t devId)
{
    return (app_bt_get_device(devId)->hf_audio_state == BT_HFP_AUDIO_CON);
}

bool btapp_hfp_current_is_virtual_call(uint8_t device_id)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    if(curr_device)
    {
        return btif_hf_is_virtual_call_enabled(curr_device->hf_channel);
    }
    return false;
}

void btapp_hfp_report_speak_gain_handler(void)
{
    uint8_t i;
    btif_remote_device_t *remDev = NULL;
    btif_link_mode_t mode = BTIF_BLM_SNIFF_MODE;
    btif_hf_channel_t* chan;

    for(i = 0; i < BT_DEVICE_NUM; i++) {
        remDev = (btif_remote_device_t *)btif_hf_cmgr_get_remote_device(app_bt_get_device(i)->hf_channel);
        if (remDev){
            mode = btif_me_get_current_mode(remDev);
        } else {
            mode = BTIF_BLM_SNIFF_MODE;
        }
        chan = app_bt_get_device(i)->hf_channel;
        DEBUG_INFO(2,"d%x report hfp volume:%d", i, mode == BTIF_BLM_ACTIVE_MODE);
        if ((btif_get_hf_chan_state(chan) == BT_HFP_CHAN_STATE_OPEN) && (mode == BTIF_BLM_ACTIVE_MODE) &&
             app_bt_audio_get_curr_playing_sco() == i)
	    {
            btif_hf_report_speaker_volume(chan, hfp_volume_get((enum BT_DEVICE_ID_T)i));
        }
    }
}

void btapp_hfp_report_speak_gain(void)
{
    app_bt_start_custom_function_in_bt_thread(0,0,(uint32_t)btapp_hfp_report_speak_gain_handler);
}

uint8_t btapp_hfp_need_mute(void)
{
    return app_bt_manager.hf_tx_mute_flag;
}

int32_t hfp_mic_need_skip_frame_cnt = 0;

bool btapp_hfp_mic_need_skip_frame(void)
{
    bool nRet;

    if (hfp_mic_need_skip_frame_cnt > 0) {
        hfp_mic_need_skip_frame_cnt--;
        nRet = true;
    } else {
        app_hfp_mute_upstream(0, false);
        nRet = false;
    }
    return nRet;
}

void btapp_hfp_mic_need_skip_frame_set(int32_t skip_frame)
{
    hfp_mic_need_skip_frame_cnt = skip_frame;
}

void app_hfp_sco_request_handler(uint8_t device_id, const uint8_t *remote)
{
    int accept_result = 0;

#if defined(__BT_ONE_BRING_TWO__)
    if (btif_sco_get_conn_number() >= 1)
    {
        DEBUG_INFO(0,"#User reject sco request!");
        btif_sco_connect_resp(device_id, 0);
        return;
    }
#elif defined(IBRT_UI)
    if (btif_sco_get_conn_number() >= 2)
    {
        DEBUG_INFO(0,"#User reject sco request!");
        btif_sco_connect_resp(device_id, 0);
        return;
    }

    if(!app_ui_incoming_sco_req_callback(device_id, (void *)remote))
    {
        DEBUG_INFO(0,"#User reject sco request!");
        btif_sco_connect_resp(device_id, 0);
        return;
    }
#endif

    accept_result = bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_SCO_CONNECT_REQ, (uint32_t)(uintptr_t)remote);
    switch (accept_result)
    {
        case APP_BT_SCO_ACCEPT_REQUEST:
            btif_sco_connect_resp(device_id, 1);
            break;
        case APP_BT_SCO_REJECT_REQUEST:
            btif_sco_connect_resp(device_id, 0);
            break;
        case APP_BT_SCO_LET_UPPER_LAYER_HANDLE:
            DEBUG_WARNING(1, "sco request let upper layer handle");
            break;
        default:
            btif_sco_connect_resp(device_id, 0);
            break;
    }
}

void  app_hfp_sco_connected_handler(uint8_t device_id, const uint8_t *remote, uint16_t conn_hdl, uint8_t codec_type)
{
    struct hfp_context ctx = {0};
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    hfp_set_sco_state_param_t set_sco_state = {0};

    if ((!curr_device) || (!curr_device->hf_channel))
    {
        return;
    }

    // set hfp stack sco state
    set_sco_state.device_id = device_id;
    set_sco_state.remote = (struct bdaddr_t *)remote;
    set_sco_state.connected = 1;
    set_sco_state.connected_codec = &codec_type;
    btif_hf_set_sco_path_status(&set_sco_state);

    // report user sco info
    ctx.event = BTIF_HF_EVENT_AUDIO_CONNECTED;
    ctx.status = BT_STS_SUCCESS;
    memcpy(&ctx.remote_dev_bdaddr.address, remote, 6);
    ctx.sco_codec = codec_type;

#ifdef BT_HFP_AG_ROLE
    if (btif_get_hf_is_ag_role(curr_device->hf_channel))
    {
        app_hfp_ag_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
    else
#endif
    {
        app_hfp_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
}

void app_hfp_sco_connect_fail_handler(uint8_t device_id, const uint8_t *remote, uint8_t status)
{
    struct hfp_context ctx = {0};
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    hfp_set_sco_state_param_t set_sco_state = {0};

    DEBUG_INFO(3,"%s: connection complete error. error code: %02x", __func__, status);
    if (status == APP_HFP_SCO_ERROR_REJ_PERSONAL_DEV)
    {
        if (curr_device && curr_device->acl_is_connected)
        {
            app_bt_audio_new_sco_is_rejected_by_controller(device_id);
        }
        return;
    }

    if (status != APP_HFP_SCO_ERROR_INVALID_LMP_PARM)
    {
        return;
    }

    if ((!curr_device) || (!curr_device->hf_channel))
    {
        return;
    }

    // set hfp stack sco state
    set_sco_state.device_id = device_id;
    set_sco_state.remote = (struct bdaddr_t *)remote;
    set_sco_state.connected = 0;
    btif_hf_set_sco_path_status(&set_sco_state);

    // report user sco info
    ctx.event = BTIF_HF_EVENT_AUDIO_CON_FAIL;
    ctx.status = BT_STS_SUCCESS;
    memcpy(&ctx.remote_dev_bdaddr.address, remote, 6);
    ctx.sco_con_fail_reason = status;
#ifdef BT_HFP_AG_ROLE
    if (btif_get_hf_is_ag_role(curr_device->hf_channel))
    {
        app_hfp_ag_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
    else
#endif
    {
        app_hfp_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
}

void app_hfp_sco_disconnected_handler(uint8_t device_id, const uint8_t *remote, uint16_t conn_hdl)
{
    struct hfp_context ctx = {0};
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    hfp_set_sco_state_param_t set_sco_state = {0};

    if ((!curr_device) || (!curr_device->hf_channel))
    {
        return;
    }

        // set hfp stack sco state
    set_sco_state.device_id = device_id;
    set_sco_state.remote = (struct bdaddr_t *)remote;
    set_sco_state.connected = 0;
    btif_hf_set_sco_path_status(&set_sco_state);

    // report user sco info
    ctx.event = BTIF_HF_EVENT_AUDIO_DISCONNECTED;
    ctx.status = BT_STS_SUCCESS;
    memcpy(&ctx.remote_dev_bdaddr.address, remote, 6);
#ifdef BT_HFP_AG_ROLE
    if (btif_get_hf_is_ag_role(curr_device->hf_channel))
    {
        app_hfp_ag_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
    else
#endif
    {
        app_hfp_event_callback(device_id, curr_device->hf_channel, &ctx);
    }
}

void app_hfp_sco_data_rx_handler(uint8_t dev_id, uint16_t sco_hdl, uint8_t ps_flg, const uint8_t* data, uint16_t data_len)
{
    if(app_bt_audio_get_curr_playing_sco() == dev_id)
    {
#ifndef _SCO_BTPCM_CHANNEL_
        uint32_t idx = 0;
        if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
            store_voicebtpcm_m2p_buffer((uint8_t*)data, data_len);

            idx = hf_sendbuff_ctrl.index % HF_SENDBUFF_MEMPOOL_NUM;
            get_voicebtpcm_p2m_frame(&(hf_sendbuff_ctrl.mempool[idx].buffer[0]), data_len);
            if(!app_bt_manager.hf_tx_mute_flag) {
                btif_sco_send_data(sco_hdl, &hf_sendbuff_ctrl.mempool[idx].buffer[0], data_len);
            }
            hf_sendbuff_ctrl.index++;
        }
#endif
    }
}

#ifdef IBRT
#if defined(IBRT_UI)
extern "C" bool app_ibrt_conn_is_ibrt_connected(const bt_bdaddr_t *addr);
#endif

static bool app_hfp_is_current_ibrt_slave_role(void *addr)
{
    if(app_tws_ibrt_tws_link_connected())
    {
        if (
#if defined(IBRT_UI)
            app_ibrt_conn_is_ibrt_connected((bt_bdaddr_t *)addr) &&
#endif
            app_tws_get_ibrt_role((bt_bdaddr_t *)addr) == 1)
        {
            DEBUG_INFO(0,"app_hfp_is_current_ibrt_slave_role: current_role=IBRT_SLAVE");
            return true;
        }
        else{
            return false;
        }
    }
    else
    {
        return false;
    }
}
#endif

static bool g_hfp_is_inited = false;

void app_hfp_init(void)
{
#ifdef IBRT
    btif_hfp_register_peer_sco_codec_receive_handler(app_hfp_receive_peer_sco_codec);
#endif

    btif_hfp_initialize();

#ifdef IBRT
    btif_register_tws_current_ibrt_slave_role_callback(app_hfp_is_current_ibrt_slave_role);
#endif

    if (g_hfp_is_inited)
    {
        return;
    }

    g_hfp_is_inited = true;

    if (besbt_cfg.bt_sink_enable)
    {
        for (uint8_t i = 0; i < BT_DEVICE_NUM; i++) {
            struct BT_DEVICE_T* curr_device = app_bt_get_device(i);
            curr_device->hf_channel = btif_hf_alloc_channel();
            curr_device->profile_mgr.chan = curr_device->hf_channel;
            if (!curr_device->hf_channel) {
                ASSERT(0, "Serious error: cannot alloc hf channel\n");
            }
            btif_hf_init_channel(curr_device->hf_channel);
            curr_device->hfchan_call = 0;
            curr_device->hfchan_callSetup = 0;
            curr_device->hf_callheld = 0;
            curr_device->hf_audio_state = BT_HFP_AUDIO_DISCON,
            curr_device->hf_conn_flag = false;
            curr_device->battery_level = 0xff;
            curr_device->clcc_timer =  osTimerCreate(osTimer(APP_HFP_GET_CLCC_TIMER), osTimerOnce, (void*)curr_device);
            if (i == 0)
            {
                curr_device->reconn_hfp_timer = osTimerCreate(osTimer(APP_HFP_RECONN_TIMER0), osTimerOnce, (void*)curr_device);
            }
            if (i == 1)
            {
                curr_device->reconn_hfp_timer = osTimerCreate(osTimer(APP_HFP_RECONN_TIMER1), osTimerOnce, (void*)curr_device);
            }
        }

        btif_hf_register_callback(app_hfp_event_callback);

        btif_sco_user_callbak_t sco_callback = {0};

        sco_callback.connect_req = app_hfp_sco_request_handler;
        sco_callback.connected    = app_hfp_sco_connected_handler;
        sco_callback.connect_fail = app_hfp_sco_connect_fail_handler;
        sco_callback.disconnected = app_hfp_sco_disconnected_handler;
        sco_callback.data_rx      = app_hfp_sco_data_rx_handler;
        btif_sco_init(&sco_callback);
    }

#if defined(SUPPORT_BATTERY_REPORT) || defined(SUPPORT_HF_INDICATORS)
    Besbt_hook_handler_set(BESBT_HOOK_USER_3, app_hfp_battery_report_proc);
#endif

    app_bt_manager.curr_hf_channel_id = BT_DEVICE_ID_1;
    app_bt_manager.hf_tx_mute_flag = 0;

    app_hfp_mute_upstream(app_bt_manager.curr_hf_channel_id, true);
}

void app_hfp_enable_audio_link(bool isEnable)
{
    return;
}

void app_hfp_hf_register_callback(app_hfp_hf_callback_t cb)
{
    g_app_bt_hfp_hf_cb = cb;
}

#define HF_COMMAND_CURRENT_CALL_HEAD  "AT+CLCC"
bt_status_t app_hf_send_current_call_at_commond(btif_hf_channel_t* chan_h)
{
    char clcc[20] = {'\0'};
    sprintf(clcc, "%s\r", HF_COMMAND_CURRENT_CALL_HEAD);
    return btif_hf_send_at_cmd(chan_h, clcc);
}


void app_pts_hfp_siri_voice_enable(void)
{
#ifdef SUPPORT_SIRI
    app_hfp_siri_voice(true);
#endif
}

void app_pts_hfp_siri_voice_disable(void)
{
#ifdef SUPPORT_SIRI
    app_hfp_siri_voice(false);
#endif
}

void app_pts_hf_acs_bi_13_i_set_enable(void)
{
    bt_drv_reg_op_set_ibrt_auto_accept_sco(0);
}

void app_pts_hf_acs_bi_13_i_set_disable(void)
{
    bt_drv_reg_op_set_ibrt_auto_accept_sco(1);
}

void app_pts_hf_acs_bv_09_i_set_enable(void)
{
    besbt_cfg.hfp_hf_pts_acs_bv_09_i = true;
}
void app_pts_hf_acs_bv_09_i_set_disable(void)
{
    besbt_cfg.hfp_hf_pts_acs_bv_09_i = false;
}
void app_pts_hf_create_service_link(bt_bdaddr_t *btaddr)
{
    btif_hf_create_service_link(btaddr);
}
void app_pts_hf_disc_service_link(void)
{
    btif_hf_disconnect_service_link(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_create_audio_link(void)
{
    btif_hf_create_audio_link(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_disc_audio_link(void)
{
    btif_hf_disc_audio_link(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_redial_call(void)
{
    btif_hf_redial_call(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_dial_number(void)
{
    char num_str[8];
    uint8_t number[8];
    sprintf(num_str, "7654321");
    memcpy(number,num_str,sizeof(num_str));
    btif_hf_dial_number(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,number,sizeof(number));
}

void app_pts_hf_dial_number_memory_index(void)
{
    char num_str[8];
    uint8_t number[8];
    sprintf(num_str, ">1");
    memcpy(number,num_str,sizeof(num_str));
    btif_hf_dial_number(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,number,sizeof(number));
}

void app_pts_hf_dial_number_invalid_memory_index(void)
{
    char num_str[8];
    uint8_t number[8];
    sprintf(num_str, ">9999");
    memcpy(number,num_str,sizeof(num_str));
    btif_hf_dial_number(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,number,sizeof(number));
}

void app_pts_hf_send_key_pressed(void)
{
    btif_hf_send_key_pressed(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}

void app_pts_hf_answer_call(void)
{
    btif_hf_answer_call(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_hangup_call(void)
{
    btif_hf_hang_up_call(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_vr_enable(void)
{
    btif_hf_enable_voice_recognition(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, true);
}
void app_pts_hf_vr_disable(void)
{
    btif_hf_enable_voice_recognition(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, false);
}
void app_pts_hf_list_current_calls(void)
{
    btif_hf_list_current_calls(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_hold_active_call(void)
{
    btif_hf_call_hold(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, BTIF_HF_HOLD_HOLD_ACTIVE_CALLS, 0);
}
void app_pts_hf_release_active_call_2(void)
{
    btif_hf_call_hold(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS,2);
}
void app_pts_hf_hold_active_call_2(void)
{
    btif_hf_call_hold(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,BTIF_HF_HOLD_HOLD_ACTIVE_CALLS,2);
}
void app_pts_hf_release_active_call(void)
{
    btif_hf_call_hold(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,BTIF_HF_HOLD_RELEASE_ACTIVE_CALLS,0);
}
void app_pts_hf_hold_call_transfer(void)
{
    btif_hf_call_hold(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel,BTIF_HF_HOLD_CALL_TRANSFER,0);
}
void app_pts_hf_send_ind_1(void)
{
    btif_hf_indicators_1(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_send_ind_2(void)
{
    btif_hf_indicators_2(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_send_ind_3(void)
{
    btif_hf_indicators_3(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_update_ind_value(void)
{
    btif_hf_update_indicators_batt_level(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, 50);
}
void app_pts_hf_report_mic_volume(void)
{
    btif_hf_report_mic_volume(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel, 0);
}
void app_pts_hf_attach_voice_tag(void)
{
    btif_hf_attach_voice_tag(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}
void app_pts_hf_ind_activation(void)
{
    btif_hf_ind_activation(app_bt_get_device(BT_DEVICE_ID_1)->hf_channel);
}

#if defined(IBRT)
static hfp_vol_sync_done_cb hfp_vol_sync_done_callback_func = NULL;
void app_ibrt_if_register_hfp_vol_sync_done_callback(hfp_vol_sync_done_cb callback)
{
    hfp_vol_sync_done_callback_func = callback;
}

static void hfp_ibrt_check_mock_hfp_status(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if(curr_device->hfchan_call)
    {
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALL_MOCK, 0);
    }
    else if(curr_device->hfchan_callSetup)
    {
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALLSETUP_MOCK, 0);
    }
    else if(curr_device->hf_callheld)
    {
        bt_audio_event_handler(device_id, APP_BT_AUDIO_EVENT_HFP_CALLHOLD_MOCK, 0);
    }
    else
    {
        DEBUG_INFO(0,"::hfp not in active state mock");
    }
}

int hfp_ibrt_service_connected_mock(uint8_t device_id)
{
    struct BT_DEVICE_T* curr_device = app_bt_get_device(device_id);
    if (btif_get_hf_chan_state(curr_device->hf_channel) == BT_HFP_CHAN_STATE_OPEN){
        DEBUG_INFO(0,"::HF_EVENT_SERVICE_CONNECTED mock");
        curr_device->hf_conn_flag = 1;
        struct hfp_context ctx = {0};
        ctx.status = BT_STS_SUCCESS;
        ctx.remote_dev_bdaddr = curr_device->remote;
        ctx.event = BTIF_HF_EVENT_SERVICE_MOCK_CONNECTED;
        ctx.call = curr_device->hfchan_call;
        ctx.call_setup = curr_device->hfchan_callSetup;
        ctx.state = BT_HFP_CHAN_STATE_OPEN;
        app_hfp_event_callback(device_id, curr_device->hf_channel, &ctx);
        hfp_ibrt_check_mock_hfp_status(device_id);
    }else{
        DEBUG_INFO(1,"::HF_EVENT_SERVICE_CONNECTED mock need check chan_state:%d", btif_get_hf_chan_state(curr_device->hf_channel));
    }

    return 0;
}

int hfp_ibrt_sync_get_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);
    hfp_status->audio_state = curr_device->hf_audio_state;
    hfp_status->localVolume = hfp_volume_local_get(device_id);
    hfp_status->callhold_state = curr_device->hf_callheld;
    hfp_status->callsetup_state = curr_device->hfchan_callSetup;
    hfp_status->call_state = curr_device->hfchan_call;
    return 0;
}

int hfp_ibrt_sync_set_status(uint8_t device_id,ibrt_hfp_status_t *hfp_status)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(device_id);

    DEBUG_INFO(4,"%s audio_state:%d localVolume:%d  sync_ctx %d", __func__, hfp_status->audio_state,
        hfp_status->localVolume, hfp_status->sync_ctx);

    /* ibrt slave also set this state when HF_EVENT_AUDIO_CONNECTED/DISCONNECTED
       so it is no need to sync this state, and if sync there is a problem:
       1. ibrt master send the audio state whent its sco connected
       2. but when slave received this command, the sco is already disc
       3. so reset this state to BT_HFP_AUDIO_CON is a mistake
     */

    if (curr_device && curr_device->acl_is_connected)
    {
        audio_player_restore_volume(curr_device->remote.address);
    }
    else
    {
        audio_player_restore_volume(NULL);
    }

    /* only sync volume when profile exchanged done, dont sync volume when sco connected
       in case the synced volume override the volume phone send
     */
    if(hfp_status->sync_ctx == HFP_SYNC_CTX_PROFILE_TXDONE)
    {
        hfp_update_local_volume(device_id, hfp_status->localVolume); //BT_DEVICE_ID_1
    }

    if (hfp_vol_sync_done_callback_func)
    {
        hfp_vol_sync_done_callback_func();
    }

    return 0;
}

int hfp_ibrt_sco_audio_connected(hfp_sco_codec_t codec, uint16_t sco_connhdl)
{
    return BT_STS_SUCCESS;
}

int hfp_ibrt_sco_audio_disconnected(void)
{
    return BT_STS_SUCCESS;
}

void hfp_ibrt_sync_status_sent_callback(void)
{
#if defined(IBRT_UI)
    app_hfp_restore_master_local_volume();
#endif
}


#if defined(IBRT_UI)
bool app_ibrt_if_is_hfp_status_sync_from_master(uint8_t deviceId)
{
    struct BT_DEVICE_T *curr_device = app_bt_get_device(deviceId);
    if (curr_device && curr_device->acl_is_connected)
    {
        if (tws_besaud_is_connected() &&
            IBRT_SLAVE == app_tws_get_ibrt_role(&curr_device->remote))
        {
            ibrt_mobile_info_t* mobileInfo =
                app_ibrt_conn_get_mobile_info_by_addr(&curr_device->remote);
            if (mobileInfo && (mobileInfo->is_waiting_for_hfp_status_from_master))
            {
                return false;
            }
        }
    }
    return true;
}

void app_hfp_restore_master_local_volume(void)
{
    // TODO: send bt sync cmd to restore hfp volume at the same time
    return;
    uint8_t curr_sco_device = app_bt_audio_get_curr_sco_device();
    if (BT_DEVICE_INVALID_ID != curr_sco_device)
    {
        struct BT_DEVICE_T *curr_device = app_bt_get_device(curr_sco_device);
        if (app_tws_ibrt_mobile_link_connected(&curr_device->remote)
           && app_audio_adm_call_stream_is_active(curr_sco_device))
        {
            uint8_t localHfpVol = hfp_volume_local_get((enum BT_DEVICE_ID_T)curr_sco_device);
            DEBUG_INFO(0, "Restore masterlocal vol after sync hfp state with slave.");
            audio_player_set_volume(localHfpVol);
        }
    }
}

#endif /* IBRT_UI */
#endif /* IBRT */
#else /* BT_HFP_SUPPORT */

#include "app_hfp.h"

void app_hfp_hf_register_callback(app_hfp_hf_callback_t cb)
{
    return;
}

void btapp_hfp_report_speak_gain(void)
{
    return;
}

uint8_t btapp_hfp_need_mute(void)
{
    return false;
}

bool btapp_hfp_mic_need_skip_frame(void)
{
    return false;
}

void hfp_volume_local_set(int id, uint8_t vol)
{
    return;
}

uint8_t app_bt_hfp_adjust_volume(uint8_t device_id, bool up, bool adjust_local_vol_level)
{
    return 0;
}

#endif /* BT_HFP_SUPPORT */

void bes_bt_register_hfp_hf_callback(app_hfp_hf_callback_t cb)
    __attribute__((alias("app_hfp_hf_register_callback")));

