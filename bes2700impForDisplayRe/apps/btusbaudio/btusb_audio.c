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
#include "stdio.h"
#include "cmsis_os.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "app_audio.h"
#include "bluetooth_bt_api.h"
#include "a2dp_api.h"
#include "app_bt.h"
#include "btapp.h"
#include "usb_audio_app.h"
#include "btusb_audio.h"
#include "usbaudio_thread.h"
#include "hal_usb.h"

extern void btusbaudio_entry(void);
extern void btusbaudio_exit(void);
extern a2dp_stream_t* app_bt_get_steam(int id);
extern int app_bt_get_bt_addr(int id,bt_bdaddr_t *bdaddr);
extern bool app_bt_a2dp_service_is_connected(uint8_t device_id);
extern int app_bt_A2DP_OpenStream(a2dp_stream_t *Stream, bt_bdaddr_t *Addr);
extern int app_bt_A2DP_CloseStream(a2dp_stream_t *Stream);

extern bool btapp_hfp_is_call_active(void);
static bool btusb_usb_is_on = false;
static enum BTUSB_MODE btusb_mode = BTUSB_MODE_INVALID;
static bool btusb_bt_audio_is_suspend = false;
static bool btusb_usb_is_init = false;

#if 1
#define BTUSB_DEBUG TRACE
#else
#define BTUSB_DEBUG(...)
#endif

typedef struct
{
    uint8_t isUsbAudioOpenStreamStatus : 1;
    uint8_t isUsbAudioCloseStreamStatus : 1;
    uint8_t isUsbAudioMobileLinkConnect : 1;
    uint8_t isUsbAudioMobileLinkDisconnect : 1;
    uint8_t isUsbAudioA2dpConnected : 1;
    uint8_t isUsbAudioA2dpDisconnect : 1;
    uint8_t isUSBAudioPlugIn : 1;
    uint8_t isUsbAudioPlugInCheck : 1;
} APP_USB_AUDIO_ENV_T;

static APP_USB_AUDIO_ENV_T app_usb_audio_env =
{
    0, //isUsbAudioOpenStreamStatus
    0, //isUsbAudioCloseStreamStatus
    0, //isUsbAudioMobileLinkConnect
    0, //isUsbAudioMobileLinkDisconnect
    0, //isUsbAudioA2dpConnected
    0, //isUsbAudioA2dpDisconnect
    0, //isUSBAudioPlugIn
    0, //isUsbAudioCheck
};
extern void btusb_a2dp_disconnect_timer_cb(void const *n);
#define BTUSB_A2DP_DISCONNECT_DELAY_CHECK_MS 200
osTimerId   btusb_a2dp_disconnect_timer_id;
bool        btusb_a2dp_disconnect_status_timer_on = false;
osTimerDef(BTUSB_A2DP_DISCONNECT__STATUS_CHECK_TIMER, btusb_a2dp_disconnect_timer_cb);

extern void btusb_a2dp_connect_timer_cb(void const *n);
#define BTUSB_A2DP_CONNECT_DELAY_CHECK_MS 200
osTimerId   btusb_a2dp_connect_timer_id;
bool        btusb_a2dp_connect_status_timer_on = false;
osTimerDef(BTUSB_A2DP_CONNECT_STATUS_CHECK_TIMER, btusb_a2dp_connect_timer_cb);

extern void btusb_switch_to_usb_timer_cb(void const *n);
#define BTUSB_SWITCH_TO_USB_DELAY_MS  1000
osTimerId   btusb_switch_to_usb_timer_id;
osTimerDef(BTUSB_SWITCH_TO_USB_TIMER, btusb_switch_to_usb_timer_cb);

osSemaphoreId btusb_wait_switch_to_usb = NULL;
osSemaphoreDef(btusb_wait_switch_to_usb);
static void _bt_stream_open(unsigned int timeout_ms)
{
    a2dp_stream_t *stream = NULL;
    bt_bdaddr_t bdaddr;
    uint32_t stime = 0;
    uint32_t etime = 0;
    bool has_remove_dev;
    bool is_connected;
    stime = hal_sys_timer_get();
    //BT_USB_DEBUG();
    stream = (a2dp_stream_t*)app_bt_get_steam(BT_DEVICE_ID_1);

    //stream = app_bt_get_mobile_stream(BT_DEVICE_ID_1);
    if(stream != NULL)
    {
        //osDelay(10);
        app_usb_audio_env.isUsbAudioOpenStreamStatus = false;
        has_remove_dev = app_bt_get_bt_addr(BT_DEVICE_ID_1,&bdaddr);
        is_connected = app_bt_is_device_profile_connected(BT_DEVICE_ID_1);
        if(has_remove_dev && is_connected)
        {
            if(!app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1))
            {
                BTUSB_DEBUG(1,"%s,btaddr:", __func__);
                DUMP8("0x%x,",(uint8_t*)bdaddr.address,BT_ADDR_OUTPUT_PRINT_NUM);
                app_usb_audio_env.isUsbAudioMobileLinkConnect = true;
                app_bt_A2DP_OpenStream(stream,&bdaddr);
                app_usb_audio_env.isUsbAudioOpenStreamStatus = true;
                app_usb_audio_env.isUsbAudioA2dpConnected = false;
            }
            else
            {
                BTUSB_DEBUG(1,"%s: a2dp is no disconnected!", __func__);
                app_usb_audio_env.isUsbAudioMobileLinkConnect = true;
                app_usb_audio_env.isUsbAudioA2dpConnected = false;
                return;
            }
        }
        else
        {
            BTUSB_DEBUG(1,"%s: mobile is no connected!", __func__);
            app_usb_audio_env.isUsbAudioMobileLinkConnect = false;
            app_usb_audio_env.isUsbAudioA2dpConnected = true; //mobile disconnect, do not connect a2dp
            return;
        }
    }
    else
    {
        BTUSB_DEBUG(1,"%s: stream is null!", __func__);
        app_usb_audio_env.isUsbAudioOpenStreamStatus = false;
        return;
    }
    while(1)
    {
        if(app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1)){
            etime = hal_sys_timer_get();
            BTUSB_DEBUG(1,"a2dp service connected, wait time = 0x%x.",TICKS_TO_MS(etime - stime));
            app_usb_audio_env.isUsbAudioA2dpConnected = true;
            app_usb_audio_env.isUsbAudioOpenStreamStatus = true;
            break;
        }
        else
        {
            etime = hal_sys_timer_get();
            if(TICKS_TO_MS(etime - stime) >= timeout_ms)
            {
                BTUSB_DEBUG(1,"a2dp service connect timeout = 0x%x.",
                       TICKS_TO_MS(etime - stime));
                app_usb_audio_env.isUsbAudioA2dpConnected = false;
                break;
            }
            osDelay(10);
        }
    }
    //BT_USB_DEBUG();
}

static void _bt_stream_close(unsigned int timeout_ms)
{
    a2dp_stream_t *stream = NULL;
    uint32_t stime = 0;
    uint32_t etime = 0;

    BTUSB_DEBUG(1,"%s", __func__);
    stime = hal_sys_timer_get();
    stream = (a2dp_stream_t*)app_bt_get_steam(BT_DEVICE_ID_1);
    if(stream)
    {
        BTUSB_DEBUG(1,"%s,stream = %p", __func__,stream);
        app_usb_audio_env.isUsbAudioCloseStreamStatus = false;
        if(app_bt_is_device_profile_connected(BT_DEVICE_ID_1))
        {
            BTUSB_DEBUG(1,"%s,app_bt_is_device_profile_connected", __func__);
            app_usb_audio_env.isUsbAudioMobileLinkConnect = true;
            if(app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1))
            {
                BTUSB_DEBUG(1,"%s,app_bt_A2DP_CloseStream", __func__);
                //a2dp_handleKey(AVRCP_KEY_PAUSE);
                //btif_a2dp_close_stream(stream);
                app_bt_A2DP_CloseStream(stream);
                app_usb_audio_env.isUsbAudioCloseStreamStatus = true;
                app_usb_audio_env.isUsbAudioA2dpDisconnect = false;
            }
            else
            {
                BTUSB_DEBUG(1,"%s,not connected!", __func__);
                app_usb_audio_env.isUsbAudioA2dpDisconnect = false;
                return;
            }
        }
        else
        {
            BTUSB_DEBUG(1,"%s: mobile is no connected!", __func__);
            app_usb_audio_env.isUsbAudioMobileLinkConnect = false;
            app_usb_audio_env.isUsbAudioA2dpDisconnect = true;
            return;
        }
    }
    else
    {
        BTUSB_DEBUG(1,"%s: stream is null!", __func__);
        app_usb_audio_env.isUsbAudioCloseStreamStatus = true;
        app_usb_audio_env.isUsbAudioMobileLinkConnect = false;
        app_usb_audio_env.isUsbAudioA2dpDisconnect = true;
        return;
    }
    stime = hal_sys_timer_get();
    while(1)
    {
        if(!app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1)){
            etime = hal_sys_timer_get();
            BTUSB_DEBUG(1,"a2dp service diconnected, wait time = 0x%x.",
                  TICKS_TO_MS(etime - stime));
            app_usb_audio_env.isUsbAudioA2dpDisconnect = true;
            app_usb_audio_env.isUsbAudioCloseStreamStatus = true;
            break;
        }
        else
        {
            etime = hal_sys_timer_get();
            if(TICKS_TO_MS(etime - stime) >= timeout_ms)
            {
                BTUSB_DEBUG(1,"a2dp service diconnect timeout = 0x%x.",
                       TICKS_TO_MS(etime - stime));
                app_usb_audio_env.isUsbAudioA2dpDisconnect = false;
                break;
            }
            osDelay(10);
        }
    }
}

static void btusb_usbaudio_entry(void)
{
    btusbaudio_entry();
    btusb_usb_is_on = true ;
}

static bool _btusb_usb_is_plus(void)
{
    uint32_t start_time;
    uint32_t end_time;
    bool is_plug = false;

    start_time = hal_sys_timer_get();
    while(1)
    {
        end_time = hal_sys_timer_get();
        if(end_time > start_time)
        {
            if(TICKS_TO_MS(end_time - start_time) >= 500)
            {
                break;
            }
        }
        else
        {
            if(TICKS_TO_MS(0xffffffff - start_time + end_time) >= 500)
            {
                break;
            }
        }
        if(hal_usb_configured())
        {
            is_plug = true;
            break;
        }
    }
    return is_plug;
}

void btusb_usbaudio_open(void)
{
    if(!btusb_usb_is_on)
    {
        btusb_usbaudio_entry();
    }
    else
    {
        usb_audio_app(1);
    }
}

void btusb_usbaudio_close(void)
{
    if(btusb_usb_is_on)
    {
        usb_audio_app(0);
    }
}

void btusb_btaudio_close(bool is_wait)
{
    if(is_wait)
    {
        BTUSB_DEBUG(1,"%s: is_wait = true.",__func__);
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_CLOSEALL, 0);
        _bt_stream_close(BTUSB_OUTTIME_MS);
    }
    else
    {
        BTUSB_DEBUG(1,"%s: is_wait = false.",__func__);
        _bt_stream_close(0);
        btusb_bt_audio_is_suspend = true;
    }
}

void btusb_btaudio_open(bool is_wait)
{
    if(is_wait)
    {
        _bt_stream_open(BTUSB_OUTTIME_MS);
#if (!defined(PROMPT_SELF_MANAGEMENT))
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_CLOSE, 0);
#else            
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, APP_BT_SETTING_STOP_PROMPT, 0);
#endif
        app_audio_sendrequest(APP_PLAY_BACK_AUDIO, (uint8_t)APP_BT_SETTING_SETUP, 0);
    }
    else
    {
        _bt_stream_open(0);
    }
    btusb_bt_audio_is_suspend = false;
}

void btusb_switch_to_bt_mode(void)
{
    if(btusb_a2dp_connect_status_timer_on == true)
    {
        osTimerStop(btusb_a2dp_connect_timer_id);
        btusb_a2dp_connect_status_timer_on = false;
    }

    BTUSB_DEBUG(1,"%s: 2 switch to BT mode.",__func__);
    if(btusb_usb_is_on)
    {
        BTUSB_DEBUG(1,"%s: 2 btusb_usbaudio_close.",__func__);
        btusb_usbaudio_close();
        BTUSB_DEBUG(1,"%s:2 btusb_usbaudio_close done.",__func__);
        osDelay(500);
    }

    btusb_mode = BTUSB_MODE_BT;
    btusb_btaudio_open(true);
    if(app_usb_audio_env.isUsbAudioA2dpConnected == false)
    {
        osTimerStart(btusb_a2dp_connect_timer_id,BTUSB_A2DP_CONNECT_DELAY_CHECK_MS);
        btusb_a2dp_connect_status_timer_on = true;
    }

    BTUSB_DEBUG(1,"%s:2 switch to BT mode done.",__func__);

    return;
}

void btusb_switch_to_usb_mode(void)
{
    if(btusb_a2dp_disconnect_status_timer_on == true)
    {
        osTimerStop(btusb_a2dp_disconnect_timer_id);
        btusb_a2dp_disconnect_status_timer_on = false;
    }

    if(btapp_hfp_is_call_active() == 1)
    {
        BTUSB_DEBUG(1,"%s: 2 hfp is call active.",__func__);
        return;
    }
    BTUSB_DEBUG(1,"%s: 2 switch to USB mode.",__func__);
    btusb_btaudio_close(true);
    if(app_usb_audio_env.isUsbAudioA2dpDisconnect == false)
    {
        osTimerStart(btusb_a2dp_disconnect_timer_id,BTUSB_A2DP_CONNECT_DELAY_CHECK_MS);
        btusb_a2dp_disconnect_status_timer_on = true;
    }

    BTUSB_DEBUG(1,"%s: 2 btusb_btaudio_close done.",__func__);
    osDelay(500);
    btusb_usbaudio_open();
    btusb_mode = BTUSB_MODE_USB;
    BTUSB_DEBUG(1,"%s: 2 switch to USB mode done.",__func__);

    return;
}

extern void btusb_usb_pre_open(void);

void btusb_switch_process(enum BTUSB_MODE mode)
{
    BTUSB_DEBUG(1,"%s: 1 switch to mode-%d.",__func__,mode);
    if(btusb_a2dp_disconnect_status_timer_on == true)
    {
        BTUSB_DEBUG(1,"%s: btusb_a2dp_disconnect_status_timer_on.",__func__);
        osTimerStop(btusb_a2dp_disconnect_timer_id);
        btusb_a2dp_disconnect_status_timer_on = false;
    }

    if(btusb_a2dp_connect_status_timer_on == true)
    {
        BTUSB_DEBUG(1,"%s: btusb_a2dp_connect_status_timer_on.",__func__);
        osTimerStop(btusb_a2dp_connect_timer_id);
        btusb_a2dp_connect_status_timer_on = false;
    }

    if(mode == BTUSB_MODE_BT)
    {
        BTUSB_DEBUG(1,"%s: mode == BTUSB_MODE_BT.",__func__);
        btusb_switch_to_bt_mode();
    }
    else
    {
        if(!btusb_usb_is_on)
        {
            BTUSB_DEBUG(1,"%s: btusb_usb_is_on = false.",__func__);
            btusb_usb_pre_open();
            btusb_usb_is_on = true;
        }

        btusb_switch_to_usb_mode();
        if(!_btusb_usb_is_plus())
        {
           BTUSB_DEBUG(1,"%s: _btusb_usb_is_plus = fasle.", __func__);
           // btusb_switch_to_bt_mode();
        }
    }

    return;
}

void btusb_a2dp_connect_timer_cb(void const *n)
{
    static uint8_t timer_cb_times = 0;

    BTUSB_DEBUG(1," [%s] ", __func__);
    btusb_a2dp_connect_status_timer_on = false;

    //btusb_btaudio_open(true);
    if((app_usb_audio_env.isUsbAudioOpenStreamStatus == false) && app_bt_is_device_profile_connected(BT_DEVICE_ID_1))
    {
        BTUSB_DEBUG(1,"%s _btusb_stream_close.", __func__);
        _bt_stream_open(0);
        app_usb_audio_env.isUsbAudioA2dpConnected = false;
    }

    if(app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1)){
        BTUSB_DEBUG(1,"%s a2dp service connected.timer_cb_times = %d",__func__,timer_cb_times);
        app_usb_audio_env.isUsbAudioA2dpConnected = true;
        timer_cb_times = 0;
    }
    else
    {
        BTUSB_DEBUG(1,"%s a2dp service connect timeout.timer_cb_times = %d", __func__,timer_cb_times);
        app_usb_audio_env.isUsbAudioA2dpConnected = false;
    }

    if((app_usb_audio_env.isUsbAudioA2dpConnected == false) && app_bt_is_device_profile_connected(BT_DEVICE_ID_1) && (timer_cb_times < 20))
    {
        osTimerStart(btusb_a2dp_connect_timer_id,BTUSB_A2DP_CONNECT_DELAY_CHECK_MS);
        btusb_a2dp_connect_status_timer_on = true;
        timer_cb_times ++;
    }

    return;
}

void btusb_a2dp_disconnect_timer_cb(void const *n)
{
    static uint8_t timer_cb_times = 0;

    BTUSB_DEBUG(1," [%s] ", __func__);
    btusb_a2dp_disconnect_status_timer_on = false;

    if((app_usb_audio_env.isUsbAudioCloseStreamStatus == false) && app_bt_is_device_profile_connected(BT_DEVICE_ID_1))
    {
        BTUSB_DEBUG(1,"%s _bt_stream_close.", __func__);
        _bt_stream_close(0);
        app_usb_audio_env.isUsbAudioA2dpDisconnect = false;
    }

    if(!app_bt_a2dp_service_is_connected(BT_DEVICE_ID_1))
    {
        BTUSB_DEBUG(1,"%s a2dp service diconnected.", __func__);
        app_usb_audio_env.isUsbAudioA2dpDisconnect = true;
        timer_cb_times = 0;
    }
    else
    {
        BTUSB_DEBUG(1,"%s a2dp service diconnect timeout.",__func__);
        app_usb_audio_env.isUsbAudioA2dpDisconnect = false;
    }

    if(app_usb_audio_env.isUsbAudioA2dpDisconnect == false && app_bt_is_device_profile_connected(BT_DEVICE_ID_1))
    {
        osTimerStart(btusb_a2dp_disconnect_timer_id,BTUSB_A2DP_DISCONNECT_DELAY_CHECK_MS);
        btusb_a2dp_disconnect_status_timer_on = true;
        timer_cb_times ++;
    }

    return;
}

void btusb_switch_to_usb_timer_cb(void const *n)
{
    btusb_switch(BTUSB_MODE_USB);
    osTimerDelete(btusb_switch_to_usb_timer_id);
}

void btusb_init(void)
{
    BTUSB_DEBUG(1,"%s",__func__);
    usb_os_init();

    btusb_a2dp_disconnect_timer_id = \
        osTimerCreate(osTimer(BTUSB_A2DP_DISCONNECT__STATUS_CHECK_TIMER), osTimerOnce, NULL);

    btusb_a2dp_connect_timer_id = \
        osTimerCreate(osTimer(BTUSB_A2DP_CONNECT_STATUS_CHECK_TIMER), osTimerOnce, NULL);

    btusb_switch_to_usb_timer_id = \
        osTimerCreate(osTimer(BTUSB_SWITCH_TO_USB_TIMER), osTimerOnce, NULL);

    if (btusb_wait_switch_to_usb == NULL)
    {
        btusb_wait_switch_to_usb = osSemaphoreCreate(osSemaphore(btusb_wait_switch_to_usb), 0);
    }

    btusb_usb_is_init = true;
    return;
}

void btusb_switch(enum BTUSB_MODE mode)
{
    if (!btusb_usb_is_init)
        return;

#ifdef BLE_USB_AUDIO_SUPPORT
    if (mode == BTUSB_MODE_BT) {
        TRACE(0, "%s: %d no need to switch to bt", __func__, __LINE__);
        return;
    }
#endif

    if(mode != BTUSB_MODE_BT && mode != BTUSB_MODE_USB)
    {
        ASSERT(0, "%s:%d, mode = %d.",__func__,__LINE__,mode);
    }

    BTUSB_DEBUG(1,"[%s]: enter %d.",__func__, mode);

    if(btusb_mode == mode) {
        return;
    }

    if(btusb_mode == BTUSB_MODE_INVALID)
    {
        if(mode == BTUSB_MODE_BT) {
            BTUSB_DEBUG(1,"%s: 1 switch to BT mode.",__func__);
            btusb_mode = BTUSB_MODE_BT;
        }
        else {
            BTUSB_DEBUG(1,"%s: 1 switch to USB mode.",__func__);
            btusb_usbaudio_open();
            btusb_mode = BTUSB_MODE_USB;
        }
    }
    else
    {
        BTUSB_DEBUG(1,"%s: 1 switch to %d.",__func__,mode);
        btusb_switch_process(mode);
    }
}

void btusb_start_switch_to(enum BTUSB_MODE mode)
{
    if(mode == BTUSB_MODE_USB)
    {
        osTimerStart(btusb_switch_to_usb_timer_id,BTUSB_SWITCH_TO_USB_DELAY_MS);
    }
    else
    {
        // TODO
    }
}

bool btusb_is_bt_mode(void)
{
    return btusb_mode == BTUSB_MODE_BT ? true : false;
}

bool btusb_is_usb_mode(void)
{
    return btusb_mode == BTUSB_MODE_USB ? true : false;
}

#if defined(BT_USB_AUDIO_DUAL_MODE_TEST)
void test_btusb_switch(void)
{
    if(btusb_mode == BTUSB_MODE_BT)
    {
        btusb_switch(BTUSB_MODE_USB);
    }
    else
    {
        btusb_switch(BTUSB_MODE_BT);
    }
}

void test_btusb_switch_to_bt(void)
{
     btusb_switch(BTUSB_MODE_BT);
}

void test_btusb_switch_to_usb(void)
{
     btusb_switch(BTUSB_MODE_USB);
}
#endif

