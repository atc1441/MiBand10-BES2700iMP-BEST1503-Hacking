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
#include "plat_types.h"
#include "string.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "aud_section.h"
#include "audioflinger.h"
#include "anc_process.h"
#include "hwtimer_list.h"
#include "app_thread.h"
#include "tgt_hardware.h"
#include "hal_bootmode.h"
#include "hal_codec.h"
#include "app_anc.h"
#include "app_anc_utils.h"
#include "app_anc_table.h"
#include "app_anc_fade.h"
#include "app_anc_sync.h"
#include "app_anc_assist.h"
#include "app_status_ind.h"
#include "hal_aud.h"
#include "anc_assist_tws_sync.h"
#ifdef VOICE_DETECTOR_EN
#include "app_voice_detector.h"
#endif

#if defined(PSAP_APP)
#include "psap_process.h"
// TODO: Clean up this code...
#ifndef CODEC_OUTPUT_DEV
#define CODEC_OUTPUT_DEV                    CFG_HW_AUD_OUTPUT_PATH_SPEAKER_DEV
#endif
#endif

#if defined(VOICE_ASSIST_FF_FIR_LMS)
#include "app_voice_assist_fir_lms.h"
#endif

#if defined(VOICE_ASSIST_ADA_IIR)
#include "app_voice_assist_ada_iir.h"
#endif

// #define ANC_BLOCK_ANC_SYNC_SWITCH_CMD

#define ANC_SMOOTH_SWITCH_GAIN_MS   (500)
#define ANC_OPEN_DELAY_MS           (600)

typedef struct {
    uint32_t id;
    uint32_t param0;
    uint32_t param1;
    uint32_t param2;
} ANC_MESSAGE_T;

typedef enum {
    ANC_STATUS_OFF = 0,
    ANC_STATUS_ON
} anc_status_t;

typedef enum {
    ANC_EVENT_SWITCH_MODE = 0,
    ANC_EVENT_SYNC_SWITCH_MODE,
    ANC_EVENT_SET_GAIN,
    ANC_EVENT_QTY
} anc_event_t;

static anc_status_t g_anc_work_status = ANC_STATUS_OFF;
static bool g_anc_init_flag = false;
static bool g_enable_assist = true;
static app_anc_mode_t g_app_anc_mode = APP_ANC_MODE_OFF;
static enum AUD_SAMPRATE_T g_sample_rate_series = AUD_SAMPRATE_NULL;

const WEAK anc_process_cfg anc_cfg = {
    .howling_check_cfg = {
        .howling_recover_time = 5000,  //ms
        .howling_interval_time = 400,  //ms

        .howling_set_gain_level = 0,    //0:-3dB  1:-6dB  2:-9dB  3:mute
    },
};

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
struct anc_event_msg_t {
    anc_event_t     event;
    app_anc_mode_t  mode;
};

static bool g_event_processing_flag = false;
static struct anc_event_msg_t g_event_msg_list[ANC_EVENT_QTY];
#endif

#define ANC_THREAD_STACK_SIZE   (1024 * 1)
static osThreadId anc_thread_tid;
static void anc_thread(void const *argument);
osThreadDef(anc_thread, osPriorityNormal, 1, ANC_THREAD_STACK_SIZE, "app_anc");
static void anc_sample_rate_change(enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T rate, enum AUD_SAMPRATE_T *new_play, enum AUD_SAMPRATE_T *new_cap);

#define ANC_MAILBOX_MAX (20)
osMailQDef (anc_mailbox, ANC_MAILBOX_MAX, ANC_MESSAGE_T);
static osMailQId anc_mailbox = NULL;
static uint8_t anc_mailbox_cnt = 0;

static void anc_mailbox_init(void)
{
    anc_mailbox = osMailCreate(osMailQ(anc_mailbox), NULL);
    ASSERT(anc_mailbox, "Failed to create app_anc mailbox");
    anc_mailbox_cnt = 0;
}

static int32_t anc_mailbox_free(ANC_MESSAGE_T* msg_p)
{
    osStatus status;

    status = osMailFree(anc_mailbox, msg_p);
    if (osOK == status) {
        anc_mailbox_cnt--;
    }

    return (int32_t)status;
}

static int32_t anc_mailbox_get(ANC_MESSAGE_T** msg_p)
{
    osEvent evt;
    evt = osMailGet(anc_mailbox, osWaitForever);
    if (evt.status == osEventMail) {
        *msg_p = (ANC_MESSAGE_T *)evt.value.p;
        return 0;
    }
    return -1;
}

static int32_t anc_mailbox_free_cnt(uint32_t cnt)
{
    ANC_MESSAGE_T *msg_p = NULL;
    int32_t POSSIBLY_UNUSED status = osOK;
    osEvent evt;

    for (uint8_t i=0; i<cnt; i++) {
        evt = osMailGet(anc_mailbox, 0);
        if (evt.status == osEventMail) {
            msg_p = (ANC_MESSAGE_T *)evt.value.p;
            TRACE(3, "[%s] id: %d, param: %d", __func__, msg_p->id, msg_p->param0);
            status = anc_mailbox_free(msg_p);
        } else {
            TRACE(2, "[%s]: get mailbox failed: %d", __func__, evt.status);
        }
   }

   return 0;
}

static int32_t anc_mailbox_put(ANC_MESSAGE_T *msg)
{
    osStatus status;

    ANC_MESSAGE_T *msg_new = NULL;

    msg_new = (ANC_MESSAGE_T*)osMailAlloc(anc_mailbox, 0);

    if (msg_new == NULL) {
        TRACE(0, "[%s] cnt: %d, Free some message...", __func__, anc_mailbox_cnt);
        anc_mailbox_free_cnt(anc_mailbox_cnt/2);
        msg_new = (ANC_MESSAGE_T*)osMailAlloc(anc_mailbox, 0);
        ASSERT(msg_new, "app_anc: osMailCAlloc fail");
    }

    memcpy(msg_new, msg, sizeof(ANC_MESSAGE_T));

    status = osMailPut(anc_mailbox, msg_new);
    if (osOK == status) {
        anc_mailbox_cnt++;
    } else {
        TRACE(0, "app_anc: osMailPut fail, status: 0x%x", status);
    }

    return (int32_t)status;
}

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
osMutexId _anc_mutex_id = NULL;
osMutexDef(_anc_mutex);
static void _anc_create_lock(void)
{
    if (_anc_mutex_id == NULL) {
        _anc_mutex_id = osMutexCreate((osMutex(_anc_mutex)));
    }
}

static void _anc_lock(void)
{
    osMutexWait(_anc_mutex_id, osWaitForever);
}

static void _anc_unlock(void)
{
    osMutexRelease(_anc_mutex_id);
}
#endif


bool app_anc_work_status(void)
{
    return (g_anc_work_status == ANC_STATUS_ON);
}

app_anc_mode_t app_anc_get_curr_mode(void)
{
    return g_app_anc_mode;
}

uint32_t app_anc_get_curr_types(void)
{
    return app_anc_table_get_types(g_app_anc_mode);
}

WEAK int32_t psap_ramp_get_time(void)
{
    return ANC_SMOOTH_SWITCH_GAIN_MS;
}

WEAK int32_t anc_ramp_get_time(void)
{
    return ANC_SMOOTH_SWITCH_GAIN_MS;
}

WEAK int32_t anc_hw_notch_set_status(bool flag)
{
    return 0;
}

WEAK int32_t anc_fir_set_status(bool flag)
{
    return 0;
}

WEAK int32_t psap_hw_notch_set_status(bool flag)
{
    return 0;
}

WEAK int32_t anc_ramp_set_time(uint32_t ms)
{
    return 0;
}

WEAK int32_t psap_ramp_set_time(uint32_t ms)
{
    return 0;
}

WEAK int32_t anc_process_set_cfg(const anc_process_cfg * anc_cfg)
{
    TRACE(0, "%s, WEAK function !!!", __func__);

    return 0;
}

static void app_anc_af_open(enum ANC_TYPE_T type)
{
    af_anc_open(type, g_sample_rate_series, g_sample_rate_series, anc_sample_rate_change);
}

static void app_anc_select_coef(enum ANC_TYPE_T type, uint32_t index)
{
    if (0) {
#if defined(PSAP_APP)
    } else if (type == PSAP_FEEDFORWARD) {
        psap_select_coef(g_sample_rate_series, index);
#endif
#if defined(AUDIO_ANC_SPKCALIB_HW)
    } else if (type == ANC_SPKCALIB) {
        spkcalib_select_coef(g_sample_rate_series, index);
#endif
    } else {
#if defined(AUDIO_ANC_FB_MC_HW)
        if (type == ANC_FEEDBACK) {
            type |= ANC_MUSICCANCLE;
        }
#endif
        anc_select_coef(g_sample_rate_series, index, type, ANC_GAIN_DELAY);
    }
}

static void app_anc_open_items(enum ANC_TYPE_T type)
{
    if (type == ANC_FEEDFORWARD) {
        app_anc_af_open(ANC_FEEDFORWARD);
        anc_open(ANC_FEEDFORWARD);
#if defined(ANC_ASSIST_ENABLED) && !defined(AUDIO_EQ_TUNING) && !defined(VOICE_ASSIST_FF_FIR_LMS)
        if (g_enable_assist) {
            app_anc_assist_open(ANC_ASSIST_USER_ANC);
        }
#endif
    } else if (type == ANC_FEEDBACK) {
        app_anc_af_open(ANC_FEEDBACK);
        anc_open(ANC_FEEDBACK);
#if defined(AUDIO_ANC_FB_MC_HW)
        anc_open(ANC_MUSICCANCLE);
        // NOTE: FB Gain is after MC. So disable FB will disable MC
        app_anc_enable_gain(ANC_MUSICCANCLE);
#endif
    } else if (type == ANC_TALKTHRU) {
        app_anc_af_open(ANC_TALKTHRU);
        anc_open(ANC_TALKTHRU);
#if defined(PSAP_APP)
    } else if (type == PSAP_FEEDFORWARD) {
        app_anc_af_open(ANC_TALKTHRU);  // NOTE: PSAP use TT configure
        psap_open(CODEC_OUTPUT_DEV);
#if defined(ANC_ASSIST_ENABLED) && !defined(AUDIO_EQ_TUNING)
        if (g_enable_assist) {
            app_anc_assist_open(ANC_ASSIST_USER_PSAP);
        }
#endif
#endif
#if defined(AUDIO_ANC_SPKCALIB_HW)
    } else if (type == ANC_SPKCALIB) {
        // app_anc_af_open(ANC_SPKCALIB);
        anc_open(ANC_SPKCALIB);
#endif
    } else {
        ASSERT(0, "[%s] type(0x%x) is invalid!", __func__, type);
    }
}

static void app_anc_close_items(enum ANC_TYPE_T type)
{
    if (type == ANC_FEEDFORWARD) {
#if defined(ANC_ASSIST_ENABLED) && !defined(AUDIO_EQ_TUNING) && !defined(VOICE_ASSIST_FF_FIR_LMS)
        if (g_enable_assist) {
            app_anc_assist_close(ANC_ASSIST_USER_ANC);
        }
#endif
        anc_close(ANC_FEEDFORWARD);
        af_anc_close(ANC_FEEDFORWARD);
    } else if (type == ANC_FEEDBACK) {
        anc_close(ANC_FEEDBACK);
        af_anc_close(ANC_FEEDBACK);
#if defined(AUDIO_ANC_FB_MC_HW)
        app_anc_disable_gain(ANC_MUSICCANCLE);
        anc_close(ANC_MUSICCANCLE);
#endif
    } else if (type == ANC_TALKTHRU) {
        anc_close(ANC_TALKTHRU);
        af_anc_close(ANC_TALKTHRU);
#if defined(PSAP_APP)
    } else if (type == PSAP_FEEDFORWARD) {
#if defined(ANC_ASSIST_ENABLED) && !defined(AUDIO_EQ_TUNING)
        if (g_enable_assist) {
            app_anc_assist_close(ANC_ASSIST_USER_PSAP);
        }
#endif
        psap_close();
        af_anc_close(ANC_TALKTHRU);     // NOTE: PSAP use TT configure
#endif
#if defined(AUDIO_ANC_SPKCALIB_HW)
    } else if (type == ANC_SPKCALIB) {
        // af_anc_close(ANC_SPKCALIB);
        anc_close(ANC_SPKCALIB);
#endif
    } else {
        ASSERT(0, "[%s] type(0x%x) is invalid!", __func__, type);
    }
}

void app_anc_update_coef(void)
{
    app_anc_coef_index_cfg_t index_cfg = app_anc_table_get_cfg(g_app_anc_mode);

#if defined(ANC_FF_ENABLED)
    if (index_cfg.ff != ANC_INVALID_COEF_INDEX) {
        app_anc_select_coef(ANC_FEEDFORWARD, index_cfg.ff);
    }
#endif
#if defined(AUDIO_ANC_TT_HW)
    if (index_cfg.tt != ANC_INVALID_COEF_INDEX) {
        app_anc_select_coef(ANC_TALKTHRU, index_cfg.tt);
    }
#endif
#if defined(ANC_FB_ENABLED)
    if (index_cfg.fb != ANC_INVALID_COEF_INDEX) {
        app_anc_select_coef(ANC_FEEDBACK, index_cfg.fb);
    }
#endif
#if defined(PSAP_APP)
    if (index_cfg.psap != ANC_INVALID_COEF_INDEX) {
        app_anc_select_coef(PSAP_FEEDFORWARD, index_cfg.psap);
    }
#endif
#if defined(AUDIO_ANC_SPKCALIB_HW)
    if (index_cfg.spk_calib != ANC_INVALID_COEF_INDEX) {
        app_anc_select_coef(ANC_SPKCALIB, index_cfg.spk_calib);
    }
#endif
}

static void app_anc_switch_coef(app_anc_mode_t mode_old, app_anc_mode_t mode_new, bool opt_with_gain)
{
    TRACE(0, "[%s] mode_old: %d, mode_new: %d", __func__, mode_old, mode_new);


    app_anc_coef_index_cfg_t index_cfg_old = app_anc_table_get_cfg(mode_old);
    app_anc_coef_index_cfg_t index_cfg_new = app_anc_table_get_cfg(mode_new);

#if defined(AUDIO_ANC_TT_HW)
    if (index_cfg_old.tt != index_cfg_new.tt) {
        if (index_cfg_old.tt == ANC_INVALID_COEF_INDEX) {
            // open
            app_anc_open_items(ANC_TALKTHRU);
            app_anc_select_coef(ANC_TALKTHRU, index_cfg_new.tt);
            if (opt_with_gain) {
                app_anc_enable_gain(ANC_TALKTHRU);
            }
        } else if (index_cfg_new.tt == ANC_INVALID_COEF_INDEX) {
            // close
            if (opt_with_gain) {
                app_anc_disable_gain(ANC_TALKTHRU);
                osDelay(ANC_SMOOTH_SWITCH_GAIN_MS);
            }
            app_anc_close_items(ANC_TALKTHRU);
        } else {
            // switch
            app_anc_select_coef(ANC_TALKTHRU, index_cfg_new.tt);
        }
    }
#endif

#if defined(ANC_FF_ENABLED)
    if (index_cfg_old.ff != index_cfg_new.ff) {
        if (index_cfg_old.ff == ANC_INVALID_COEF_INDEX) {
            // open
            app_anc_open_items(ANC_FEEDFORWARD);
            app_anc_select_coef(ANC_FEEDFORWARD, index_cfg_new.ff);
            if (opt_with_gain) {
                app_anc_enable_gain(ANC_FEEDFORWARD);
            }
        } else if (index_cfg_new.ff == ANC_INVALID_COEF_INDEX) {
            // close
            if (opt_with_gain) {
                app_anc_disable_gain(ANC_FEEDFORWARD);
                osDelay(ANC_SMOOTH_SWITCH_GAIN_MS);
            }
            app_anc_close_items(ANC_FEEDFORWARD);
        } else {
            // switch
            app_anc_select_coef(ANC_FEEDFORWARD, index_cfg_new.ff);
        }
    }
#endif

#if defined(ANC_FB_ENABLED)
    if (index_cfg_old.fb != index_cfg_new.fb) {
        if (index_cfg_old.fb == ANC_INVALID_COEF_INDEX) {
            // open
            app_anc_open_items(ANC_FEEDBACK);
            app_anc_select_coef(ANC_FEEDBACK, index_cfg_new.fb);
            if (opt_with_gain) {
                app_anc_enable_gain(ANC_FEEDBACK);
            }
        } else if (index_cfg_new.fb == ANC_INVALID_COEF_INDEX) {
            // close
            if (opt_with_gain) {
                app_anc_disable_gain(ANC_FEEDBACK);
                osDelay(ANC_SMOOTH_SWITCH_GAIN_MS);
            }
            app_anc_close_items(ANC_FEEDBACK);
        } else {
            // switch
            app_anc_select_coef(ANC_FEEDBACK, index_cfg_new.fb);
        }
    }
#endif

#if defined(PSAP_APP)
    if (index_cfg_old.psap != index_cfg_new.psap) {
        if (index_cfg_old.psap == ANC_INVALID_COEF_INDEX) {
            // open
            app_anc_open_items(PSAP_FEEDFORWARD);
            app_anc_select_coef(PSAP_FEEDFORWARD, index_cfg_new.psap);
            if (opt_with_gain) {
                app_anc_enable_gain(PSAP_FEEDFORWARD);
            }
        } else if (index_cfg_new.psap == ANC_INVALID_COEF_INDEX) {
            // close
            if (opt_with_gain) {
                app_anc_disable_gain(PSAP_FEEDFORWARD);
                osDelay(ANC_SMOOTH_SWITCH_GAIN_MS);
            }
            app_anc_close_items(PSAP_FEEDFORWARD);
        } else {
            // switch
            app_anc_select_coef(PSAP_FEEDFORWARD, index_cfg_new.psap);
        }
    }
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
    if (index_cfg_old.spk_calib != index_cfg_new.spk_calib) {
        if (index_cfg_old.spk_calib == ANC_INVALID_COEF_INDEX) {
            // open
            app_anc_open_items(ANC_SPKCALIB);
            app_anc_select_coef(ANC_SPKCALIB, index_cfg_new.spk_calib);
        } else if (index_cfg_new.spk_calib == ANC_INVALID_COEF_INDEX) {
            // close
            app_anc_close_items(ANC_SPKCALIB);
        } else {
            // switch
            app_anc_select_coef(ANC_SPKCALIB, index_cfg_new.spk_calib);
        }
    }
#endif
}

static void app_anc_post_msg(anc_event_t event, uint8_t param)
{
    TRACE(0, "[%s] event: %d, param: %d", __func__, event, param);

    if (g_anc_init_flag == false) {
        return;
    }

    ANC_MESSAGE_T msg;
    msg.id = event;
    msg.param0 = param;
    anc_mailbox_put(&msg);
}

int32_t app_anc_thread_set_gain(enum ANC_TYPE_T type, float gain_l, float gain_r)
{
    TRACE(2, "[%s] type: 0x%x", __func__, type);

    if (g_anc_init_flag == false) {
        return -1;
    }

    ANC_MESSAGE_T msg;
    msg.id = ANC_EVENT_SET_GAIN;
    msg.param0 = type;
    memcpy(&msg.param1, &gain_l, sizeof(float));
    memcpy(&msg.param2, &gain_r, sizeof(float));
    anc_mailbox_put(&msg);

    return 0;
}

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
static uint32_t _delete_event_msg_from_list(anc_event_t event)
{
    uint32_t i = 0;
    /* Found event */
    for (i=0; i<ANC_EVENT_QTY; i++) {
        if ((g_event_msg_list[i].event == ANC_EVENT_QTY) || (g_event_msg_list[i].event == event)) {
            break;
        }
    }

    /* Delete event */
    for (; i<ANC_EVENT_QTY-1; i++) {
        if (g_event_msg_list[i + 1].event == ANC_EVENT_QTY) {
            break;
        } else {
            g_event_msg_list[i] = g_event_msg_list[i+1];
        }
    }

    ASSERT(i<ANC_EVENT_QTY, "[%s] %d is invalid", __func__, i);

    g_event_msg_list[i].event = ANC_EVENT_QTY;

    return i;
}

/* Put event to the end of list*/
static void _update_event_msg_list_with_mode(anc_event_t event, app_anc_mode_t mode)
{
    _anc_lock();
    if (g_event_processing_flag) {
        uint32_t end = _delete_event_msg_from_list(event);
        g_event_msg_list[end].event = event;
        g_event_msg_list[end].mode = mode;
    } else {
        app_anc_post_msg(event, mode);
    }
    _anc_unlock();
}
#endif

/************************************************************************
**                  anc_sample_rate_change()
**
** this function is callback. it is passed as a parameter to af_anc_open()
** function. it is invoked that used to change anc filter if anc sample rate
** currently is not equal to playback stream sample rate incoming during ANC
** ON status. so it can cause POP voice during changing anc filter. so we not
** recommend to change anc filter if POP voice has already occurred. so we just
** pass NULL paramter to af_anc_open() function.
**
************************************************************************/
static void anc_sample_rate_change(enum AUD_STREAM_T stream, enum AUD_SAMPRATE_T rate, enum AUD_SAMPRATE_T *new_play, enum AUD_SAMPRATE_T *new_cap)
{
    ASSERT(new_play == NULL, "[%s] new_play != NULL", __func__);
    ASSERT(new_cap == NULL, "[%s] new_cap != NULL", __func__);

    if (g_sample_rate_series != rate) {
        g_sample_rate_series = rate;
        TRACE(0, "[%s] Update g_sample_rate_series = %d", __func__, g_sample_rate_series);
        // TODO: Send to thread to process
        app_anc_update_coef();
    }
}

static void app_anc_open_impl(app_anc_mode_t mode)
{
    TRACE(0, "[%s] mode: %d", __func__, mode);
    (void)mode;

#if defined(VOICE_DETECTOR_EN)
    app_voice_detector_capture_disable_vad(VOICE_DETECTOR_ID_0, VOICE_DET_USER_ANC);
#endif
}

static void app_anc_close_impl(app_anc_mode_t mode)
{
    TRACE(0, "[%s] mode: %d", __func__, mode);

    app_anc_coef_index_cfg_t index_cfg = app_anc_table_get_cfg(mode);

#if defined(ANC_FF_ENABLED)
    if (index_cfg.ff != ANC_INVALID_COEF_INDEX) {
        app_anc_close_items(ANC_FEEDFORWARD);
    }
#endif

#if defined(AUDIO_ANC_TT_HW)
    if (index_cfg.tt != ANC_INVALID_COEF_INDEX) {
        app_anc_close_items(ANC_TALKTHRU);
    }
#endif

#if defined(ANC_FB_ENABLED)
    if (index_cfg.fb != ANC_INVALID_COEF_INDEX) {
        app_anc_close_items(ANC_FEEDBACK);
    }
#endif

#if defined(PSAP_APP)
    if (index_cfg.psap != ANC_INVALID_COEF_INDEX) {
        app_anc_close_items(PSAP_FEEDFORWARD);
    }
#endif

#if defined(AUDIO_ANC_SPKCALIB_HW)
    if (index_cfg.spk_calib != ANC_INVALID_COEF_INDEX) {
        app_anc_close_items(ANC_SPKCALIB);
    }
#endif

#if defined(VOICE_DETECTOR_EN)
    app_voice_detector_capture_enable_vad(VOICE_DETECTOR_ID_0, VOICE_DET_USER_ANC);
#endif
}

static anc_status_t app_anc_switch_mode_impl(anc_status_t status, app_anc_mode_t mode)
{
    TRACE(0, "[%s] status: %d, old mode: %d, new mode: %d", __func__, status, g_app_anc_mode, mode);
    int32_t fade_out_ms = 0;

    if (g_app_anc_mode != mode) {
#if defined(VOICE_ASSIST_FF_FIR_LMS)
        uint32_t custom_mode_old = app_anc_table_get_custom_mode(g_app_anc_mode);
        uint32_t custom_mode_new = app_anc_table_get_custom_mode(mode);

        if (g_enable_assist) {
            if (custom_mode_old == ANC_CUSTOM_MODE_FIR_ANC) {
                app_voice_assist_fir_lms_set_fir_status(false);
                if ((custom_mode_new != ANC_CUSTOM_MODE_FIR_ANC) && ( custom_mode_new !=  APP_ANC_MODE_OFF)) {
                    app_voice_assist_fir_lms_stop_fir();
                }
            } else if (custom_mode_old == ANC_CUSTOM_MODE_FIR_TT) {
                app_voice_assist_fir_lms_set_tt_algo_status(0);
            }
        }
#endif

        if (g_app_anc_mode == APP_ANC_MODE_OFF) {
            // Open ...
            if (status == ANC_STATUS_OFF) {
#if defined(VOICE_ASSIST_FF_FIR_LMS)
                anc_fir_set_status(true); //enable fir before anc_open.
                anc_fir_step_sample_mode(12); // set fir step before anc_open
#endif
                app_anc_switch_coef(g_app_anc_mode, mode, false);   // Just open anc and set coef. gain is zero
#if defined(VOICE_ASSIST_FF_FIR_LMS)
                app_voice_assist_fir_lms_set_fir_HW_work_mode(custom_mode_new);
#endif
                app_anc_open_impl(mode);
                osDelay(ANC_OPEN_DELAY_MS);
                app_anc_fadein(app_anc_table_get_types(mode));
                status = ANC_STATUS_ON;
            } else {
                ASSERT(0, "[%s] Open: status(%d) is invalid", __func__, status);
            }
        } else if (mode == APP_ANC_MODE_OFF) {
            // Close ...
            if (status == ANC_STATUS_ON) {
                app_anc_fadeout(app_anc_table_get_types(g_app_anc_mode));
                fade_out_ms = psap_ramp_get_time() > anc_ramp_get_time() ? (psap_ramp_get_time()+20) : (anc_ramp_get_time()+20);
                osDelay(fade_out_ms);
                app_anc_close_impl(g_app_anc_mode);
                status = ANC_STATUS_OFF;
            } else {
                ASSERT(0, "[%s] Close: status(%d) is invalid", __func__, status);
            }
        } else {
            // Switch ...
            if (status == ANC_STATUS_ON) {
#if (!defined VOICE_ASSIST_ADA_IIR) || (defined ONLY_FF_IN_IIR)
                app_anc_fadeout(app_anc_table_get_types(g_app_anc_mode));
                fade_out_ms = psap_ramp_get_time() > anc_ramp_get_time() ? (psap_ramp_get_time()+20) : (anc_ramp_get_time()+20);
                osDelay(fade_out_ms);
#if defined(SWITCH_MODE_WITHOUT_HW_FADE)
                app_anc_close_impl(g_app_anc_mode);
                app_anc_open_impl(mode);
                app_anc_switch_coef(APP_ANC_MODE_OFF, mode, false);
                osDelay(ANC_OPEN_DELAY_MS);
#else
                app_anc_switch_coef(g_app_anc_mode, mode, false);
#endif
#if defined(VOICE_ASSIST_FF_FIR_LMS)
                app_voice_assist_fir_lms_set_fir_HW_work_mode(custom_mode_new);
#endif
                app_anc_fadein(app_anc_table_get_types(mode));
#else
                app_anc_switch_coef(g_app_anc_mode, mode, true);
#endif
                app_anc_assist_algo_reset();

            } else {
                ASSERT(0, "[%s] Switch: status(%d) is invalid", __func__, status);
            }
        }

#if defined(VOICE_ASSIST_FF_FIR_LMS)
        if (g_enable_assist) {
            app_voice_assist_fir_lms_set_anc_mode(mode);
            if (custom_mode_new == ANC_CUSTOM_MODE_FIR_ANC) {
                app_voice_assist_fir_lms_set_fir_status(true);
                if ((custom_mode_old != ANC_CUSTOM_MODE_FIR_ANC)&&( custom_mode_old !=  APP_ANC_MODE_OFF)) {
                    app_voice_assist_fir_lms_start_fir();
                }
            } else if (custom_mode_new == ANC_CUSTOM_MODE_FIR_TT) {
                app_voice_assist_fir_lms_set_tt_algo_status(1);
            }
        }
#endif
        g_app_anc_mode = mode;
    }

    return status;
}

static inline int32_t anc_thread_process(ANC_MESSAGE_T *msg)
{
    uint32_t evt = msg->id;
    uint32_t arg0 = msg->param0;

    TRACE(4, "[%s] evt: %d, arg0: %d, anc status :%d", __func__, evt, arg0, g_anc_work_status);

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    g_event_processing_flag = true;
#endif

    switch (evt) {
        case ANC_EVENT_SYNC_SWITCH_MODE:
            if (g_app_anc_mode != arg0) {
#if defined(IBRT)
                app_anc_sync_mode(arg0);
#endif

#ifdef APP_ANC_TRIGGER_SYNC
                TRACE(0,"[%s]Choose trigger sync method",__func__);
#else
                g_anc_work_status = app_anc_switch_mode_impl(g_anc_work_status, arg0);
#endif
            } else {
                TRACE(0, "[%s] WARNING: Same mode: %d", __func__, arg0);
            }
            break;
        case ANC_EVENT_SWITCH_MODE:
            g_anc_work_status = app_anc_switch_mode_impl(g_anc_work_status, arg0);
            break;
        case ANC_EVENT_SET_GAIN: {
            enum ANC_TYPE_T type = (enum ANC_TYPE_T)msg->param0;
            float gain_l = 1.0;
            float gain_r = 1.0;
            memcpy(&gain_l, &msg->param1, sizeof(float));
            memcpy(&gain_r, &msg->param2, sizeof(float));
            TRACE(4, "[%s] type: 0x%x, gain(x100): %d, %d", __func__, type, (uint32_t)(gain_l * 100), (uint32_t)(gain_r * 100));
            app_anc_set_gain_f32(ANC_GAIN_USER_APP_ANC, type, gain_l, gain_r);
        }
            break;
        default:
            break;
    }

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    _anc_lock();
    g_event_processing_flag = false;

    for (uint32_t i=0; i<ANC_EVENT_QTY; i++) {
        if (g_event_msg_list[i].event != ANC_EVENT_QTY) {
            app_anc_post_msg(g_event_msg_list[i].event, g_event_msg_list[i].mode);
            g_event_msg_list[i].event = ANC_EVENT_QTY;
        } else {
            break;
        }
    }
    _anc_unlock();
#endif

    return 0;
}

static void anc_thread(void const *argument)
{
    while(1) {
        ANC_MESSAGE_T *msg = NULL;
        if (!anc_mailbox_get(&msg)) {
            anc_thread_process(msg);
            anc_mailbox_free(msg);
        }
    }
}

int32_t app_anc_switch(app_anc_mode_t mode)
{
    TRACE(0, "[%s] Mode: %d --> %d", __func__, g_app_anc_mode, mode);

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    _update_event_msg_list_with_mode(ANC_EVENT_SYNC_SWITCH_MODE, mode);
#else
    app_anc_post_msg(ANC_EVENT_SYNC_SWITCH_MODE, mode);
#endif

    return 0;
}

int32_t app_anc_switch_locally(app_anc_mode_t mode)
{
    TRACE(0, "[%s] Mode: %d --> %d", __func__, g_app_anc_mode, mode);

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    _update_event_msg_list_with_mode(ANC_EVENT_SWITCH_MODE, mode);
#else
    app_anc_post_msg(ANC_EVENT_SWITCH_MODE, mode);
#endif

    return 0;
}

int32_t app_anc_loop_switch(void)
{
#if (defined VOICE_ASSIST_ADA_IIR) && (defined POWER_KEY_IN_IIR)
    static int cnt = 0;
    switch (cnt)
    {
    case 0:
        app_voice_assist_ada_iir_open();
        cnt = 1;
        break;
    case 1:
        app_voice_assist_ada_iir_close();
        cnt = 0;
    default:
        break;
    }
#else
    static app_anc_mode_t mode = APP_ANC_MODE_OFF;
    mode = (mode + 1) % APP_ANC_MODE_QTY;

    app_anc_switch(mode);
#endif

    return 0;
}

int32_t app_anc_init(void)
{
    TRACE(0, "[%s] ...", __func__);

    g_anc_init_flag = true;
    g_enable_assist = true;
    g_app_anc_mode = APP_ANC_MODE_OFF;

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    g_event_processing_flag = false;
    for (uint32_t i=0; i<ANC_EVENT_QTY; i++) {
        g_event_msg_list[i].event = ANC_EVENT_QTY;
        g_event_msg_list[i].mode = APP_ANC_MODE_OFF;
    }
#endif

    if (g_sample_rate_series == AUD_SAMPRATE_NULL) {
        g_sample_rate_series = hal_codec_anc_convert_rate(AUD_SAMPRATE_48000);
    }

    anc_thread_tid = osThreadCreate(osThread(anc_thread), NULL);
    anc_mailbox_init();

#if defined(ANC_CALIB_WITH_GAIN)
    int32_t gain_l = 512;
    int32_t gain_r = 512;

    // FIXME: get_ff_gain_from_nvrecord(&gain_l, &gain_r);
    anc_set_calib_gain(ANC_FEEDFORWARD, gain_l, gain_r);
    TRACE(3, "[%s] FF Calib Gain: %d, %d", __func__, gain_l, gain_r);

    // FIXME: get_fb_gain_from_nvrecord(&gain_l, &gain_r);
    anc_set_calib_gain(ANC_FEEDBACK, gain_l, gain_r);
    TRACE(3, "[%s] FB Calib Gain: %d, %d", __func__, gain_l, gain_r);

#elif defined(__AUDIO_SECTION_SUPPT__)
    anc_load_cfg();
#endif
    app_anc_table_init();
    app_anc_utils_init();
    app_anc_fade_init();
#if defined(PSAP_APP)
    psap_set_bands_same_gain_f32(1.0, 1.0, 0, 0);
    psap_set_bands_gain_f32(NULL, NULL);
#endif

#if defined(ANC_BLOCK_ANC_SYNC_SWITCH_CMD)
    _anc_create_lock();
#endif

    anc_hw_notch_set_status(false);  //true: ANF open
    psap_hw_notch_set_status(false);

    anc_fir_set_status(false); //disable fir by default.

    anc_ramp_set_time(ANC_SMOOTH_SWITCH_GAIN_MS); //HW smooth
    psap_ramp_set_time(ANC_SMOOTH_SWITCH_GAIN_MS);

#if defined(PSAP_IIR_LINEAR_SMOOTH_MODE)
    psap_ramp_set_mode(PSAP_RAMP_MODE_TIME_FIXED);
#endif
    anc_process_set_cfg(&anc_cfg);

    g_anc_work_status = ANC_STATUS_OFF;

    app_anc_sync_init();

    return 0;
}

int32_t app_anc_deinit(void)
{
    TRACE(0, "[%s] ...", __func__);

    app_anc_fade_deinit();
    app_anc_table_deinit();

    if (g_anc_work_status != ANC_STATUS_OFF) {
        g_anc_work_status = ANC_STATUS_OFF;
        app_anc_close_impl(g_app_anc_mode);
    }

    osThreadTerminate(anc_thread_tid);

    g_anc_init_flag = false;

    return 0;
}

int32_t app_anc_enable_assist(bool en)
{
    TRACE(0, "[%s] en: %d", __func__, en);

    g_enable_assist = en;

    return 0;
}

bool app_anc_assist_is_enable(void)
{
    return g_enable_assist;
}
