#include <stdio.h>
#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "app_utils.h"
#include "app_bt.h"
#include "app_audio.h"
#include "audio_trigger_checker.h"
#include "a2dp_decoder.h"
#if defined(IBRT)
#include "app_ibrt_internal.h"
#endif

#define APP_BT_STREAM_TRIGGER_TIMEROUT (2000)

#define SYNCHRONIZE_NEED_DISCARDS_DMA_CNT 4


static bool app_bt_stream_trigger_enable = 0;
static uint32_t app_bt_stream_trigger_checker = 0;
static APP_BT_STREAM_TRIGGER_CHECKER_TRIGOK_CB app_bt_stream_trigger_checker_trigok_fn = NULL;

static void app_bt_stream_trigger_timeout_cb(void const *n);
osTimerDef(APP_BT_STREAM_TRIGGER_TIMEOUT, app_bt_stream_trigger_timeout_cb);
osTimerId app_bt_stream_trigger_timeout_id = NULL;

static void app_bt_stream_trigger_timeout_cb(void const *n)
{
    TRACE_AUD_STREAM_I("[STRM_TRIG][CHK]timeout_cb\n");
    a2dp_audio_retrigger_set_on_process(false);
    app_sysfreq_req(APP_SYSFREQ_USER_TRIGGER, APP_SYSFREQ_32K);
    if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
        TRACE_AUD_STREAM_I("[STRM_TRIG][CHK]-->A2DP_SBC\n");
#if defined(IBRT)
        app_tws_ibrt_request_modify_tws_bandwidth(TWS_TIMING_CONTROL_USER_FAST_COMMUNICATION, false);
        app_ibrt_if_force_audio_retrigger(RETRIGGER_BY_TRIGGER_FAIL);
#else
        app_audio_sendrequest_param(APP_BT_STREAM_A2DP_SBC, (uint8_t)APP_BT_SETTING_RESTART, 0, 0);
#endif
    }else if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
        TRACE_AUD_STREAM_I("[STRM_TRIG][CHK]-->HFP_PCM\n");
#ifndef HFP_AG_TEST
        app_audio_sendrequest(APP_BT_STREAM_HFP_PCM, (uint8_t)APP_BT_SETTING_RESTART, 0);
#endif
    }

}

extern void bt_sco_clear_trigger(void);
int app_bt_stream_trigger_checker_handler(uint32_t trigger_checker)
{
    bool trigger_ok = false;

    if (app_bt_stream_trigger_enable){
        app_bt_stream_trigger_checker |= trigger_checker;
        if (app_bt_stream_isrun(APP_BT_STREAM_A2DP_SBC)){
            if (app_bt_stream_trigger_checker == TRIGGER_CHECKER_A2DP_DONE){
                trigger_ok = true;                
                a2dp_audio_retrigger_set_on_process(false);
            }
        }else if (app_bt_stream_isrun(APP_BT_STREAM_HFP_PCM)){
            if (app_bt_stream_trigger_checker == TRIGGER_CHECKER_HFP_DONE){
                trigger_ok = true;
                bt_sco_clear_trigger();
            }
        }
        if (trigger_ok){
            TRACE_AUD_STREAM_I("[STRM_TRIG][CHK] ok\n");
            if (app_bt_stream_trigger_checker_trigok_fn){
                app_bt_stream_trigger_checker_trigok_fn(trigger_checker);
            }
            app_bt_stream_trigger_checker_stop();
        }
    }
    return 0;
}

int app_bt_stream_trigger_checker_set_cb(APP_BT_STREAM_TRIGGER_CHECKER_TRIGOK_CB trigok_cb_fn)
{
    app_bt_stream_trigger_checker_trigok_fn = trigok_cb_fn;
    return 0;
}

int app_bt_stream_trigger_checker_init(void)
{
    if (app_bt_stream_trigger_timeout_id == NULL){
        app_bt_stream_trigger_enable = false;
        app_bt_stream_trigger_checker = 0;
        app_bt_stream_trigger_timeout_id = osTimerCreate(osTimer(APP_BT_STREAM_TRIGGER_TIMEOUT), osTimerOnce, NULL);
    }

    return 0;
}

int app_bt_stream_trigger_checker_start(void)
{
    TRACE_AUD_STREAM_I("%s", __func__);
#ifdef IBRT
    app_ibrt_set_audio_trigger_protect();
#endif
    app_bt_stream_trigger_checker = 0;
    app_bt_stream_trigger_enable = true;
    osTimerStart(app_bt_stream_trigger_timeout_id, APP_BT_STREAM_TRIGGER_TIMEROUT);
    return 0;
}

int app_bt_stream_trigger_checker_stop(void)
{
    TRACE_AUD_STREAM_I("%s", __func__);
#ifdef IBRT
    app_ibrt_clear_audio_trigger_protect();
#endif
    app_bt_stream_trigger_enable = false;
    app_bt_stream_trigger_checker = 0;
    osTimerStop(app_bt_stream_trigger_timeout_id);
    return 0;
}

