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
#include "cmsis_os.h"
#include <string.h>
#include "bluetooth_bt_api.h"
#include "me_api.h"
#include "hal_trace.h"
#include "audioflinger.h"
#include "app_bt_stream.h"
#include "app_anc_assist_trigger.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "app_bt.h"
#ifdef IBRT
#include "app_ibrt_internal.h"
#endif
#include "hal_timer.h"
#include "app_voice_assist_ultrasound.h"
#include "audio_policy.h"

#define APP_ANC_ASSIST_SYNC_DELAY_US         (10*1000)
#define APP_ANC_ASSIST_TRIGGER_TIMEROUT_MS    (50)

static uint32_t app_anc_assist_trigger_enable = 0;

static void app_anc_assist_trigger_timeout_cb(void const *n);
osTimerDef(APP_ANC_ASSIST_TRIGGER_TIMEOUT, app_anc_assist_trigger_timeout_cb);
osTimerId app_anc_assist_trigger_timeout_id = NULL;

static app_anc_assist_trigger_device_t trigger_device =
{
    {
        {
            .device = AUD_STREAM_USE_INT_CODEC,
            .stream = AUD_STREAM_CAPTURE,
        },
        {
            .device = AUD_STREAM_USE_INT_CODEC2,
            .stream = AUD_STREAM_PLAYBACK,
        },
    },
    .stream_num = 2,
    .trigger_channel = 1,
};

#define ANC_ASSIST_TRIGGER_CHANNEL \
    (trigger_device.trigger_channel)

static uint32_t trigger_checker_start_time, codec_trigger_time;

static void app_anc_assist_trigger_timeout_cb(void const *n)
{
    uint32_t e_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
    TRACE(1,"[%s]-->APP_BT_SETTING_CLOSE, %dus, timer %dus", __func__, e_time, e_time - trigger_checker_start_time);
    TRACE(1,"[%s] curr tick %08x", __func__, bt_syn_get_curr_ticks(0));
    hal_iomux_tportclr(0);
    bt_drv_i2v_enable_sleep_for_bt_access();
    // bt_adapter_write_sleep_enable(1);
    app_voice_assist_ultrasound_close();
}

static int app_anc_assist_trigger_checker_init(void)
{
    if (app_anc_assist_trigger_timeout_id == NULL){
        app_anc_assist_trigger_timeout_id = osTimerCreate(osTimer(APP_ANC_ASSIST_TRIGGER_TIMEOUT), osTimerOnce, NULL);
    }
    return 0;
}

static int app_anc_assist_trigger_checker_start(void)
{
    app_anc_assist_trigger_enable = true;
    osTimerStart(app_anc_assist_trigger_timeout_id, APP_ANC_ASSIST_TRIGGER_TIMEROUT_MS);
    trigger_checker_start_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
    TRACE(2, "[%s] trigger_check_start: %d", __FUNCTION__, trigger_checker_start_time);
    TRACE(1, "[%s] curr tick %08x", __func__, bt_syn_get_curr_ticks(0));
    return 0;
}

static int app_anc_assist_trigger_checker_stop(void)
{
    app_anc_assist_trigger_enable = false;
    osTimerStop(app_anc_assist_trigger_timeout_id);
    return 0;
}

int app_anc_assist_trigger_checker(void)
{
    if (app_anc_assist_trigger_enable){
        uint32_t e_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
        TRACE(1,"[%s] trigger ok at %dus, timer %dus", __func__, e_time, e_time - trigger_checker_start_time);
        TRACE(1,"[%s] curr tick %08x", __func__, bt_syn_get_curr_ticks(0));
        app_anc_assist_trigger_enable = false;
        hal_iomux_tportclr(0);
        bt_drv_i2v_enable_sleep_for_bt_access();
        // bt_adapter_write_sleep_enable(1);
        osTimerStop(app_anc_assist_trigger_timeout_id);
    }
    return 0;
}

static void app_anc_assist_codec_sync_config(bool enable)
{
    if (enable) {
        codec_trigger_time = FAST_TICKS_TO_US(hal_fast_sys_timer_get());
        TRACE(2, "[%s] codec_trigger_time %d", __FUNCTION__, codec_trigger_time);
    }

    for (uint8_t i = 0; i < trigger_device.stream_num; i++) {
        app_anc_assist_trigger_stream_t *stream = &trigger_device.stream[i];

        af_codec_sync_device_config(stream->device, stream->stream, AF_CODEC_SYNC_TYPE_BT, enable);
        af_codec_set_device_bt_sync_source(stream->device, stream->stream, ANC_ASSIST_TRIGGER_CHANNEL);
    }
}

static void app_anc_assist_set_trigger_time(uint32_t tg_tick)
{
    if (tg_tick){
        btdrv_syn_clr_trigger(ANC_ASSIST_TRIGGER_CHANNEL);
        btdrv_enable_playback_triggler(ACL_TRIGGLE_MODE);
        bt_syn_set_tg_ticks(tg_tick, 0, BT_TRIG_MASTER_ROLE, ANC_ASSIST_TRIGGER_CHANNEL, true);
        hal_iomux_tportset(0);
        app_anc_assist_trigger_checker_start();
        TRACE(2,"[%s] set trigger tg_tick:%08x", __func__, tg_tick);
    }else{
        btdrv_syn_clr_trigger(ANC_ASSIST_TRIGGER_CHANNEL);
        bt_syn_cancel_tg_ticks(ANC_ASSIST_TRIGGER_CHANNEL);
        app_anc_assist_trigger_checker_stop();
        TRACE(1,"[%s] trigger clear", __func__);
    }
}

int app_anc_assist_trigger_init(void)
{
    uint32_t curr_ticks = 0;
    uint32_t tg_tick = 0;
    uint32_t tick_offset = 0;

    bt_drv_i2v_disable_sleep_for_bt_access();
    // bt_adapter_write_sleep_enable(0);

    hal_sys_timer_delay_us(5000);

    app_anc_assist_trigger_deinit();

    app_anc_assist_trigger_checker_init();

    curr_ticks = bt_syn_get_curr_ticks(0);

    tick_offset = APP_ANC_ASSIST_SYNC_DELAY_US;

    //wake up btcore to protect software trigger procedure
    tg_tick = curr_ticks + US_TO_BTCLKS(tick_offset);

    app_anc_assist_codec_sync_config(true);

    TRACE(1,"[%s]-->  curr bt tick %08x ,tg clk: %08x ", __func__, curr_ticks, tg_tick);
    app_anc_assist_set_trigger_time(tg_tick);

    return 0;
}

int app_anc_assist_trigger_deinit(void)
{
    app_anc_assist_codec_sync_config(false);
    app_anc_assist_set_trigger_time(0);

    return 0;
}
