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
#include "string.h"
#include "app_utils.h"
#include "hal_trace.h"
#include "hal_aud.h"
#include "hal_timer.h"
#include "anc_process.h"
#include "audioflinger.h"
#include "hwtimer_list.h"
#include "app_anc_utils.h"
#include "anc_assist_tws_sync.h"

// NOTE: Perhaps can move tuning gain to app_anc_fade.
extern bool app_anc_work_status_is_on(void);
extern bool app_anc_work_status(void);
extern uint32_t app_anc_get_curr_types(void);

#if defined(ANC_HW_GAIN_SMOOTHING)
static void anc_set_gain_with_hw_smooth(anc_gain_user_t user, float gain_coef_l, float gain_coef_r, enum ANC_TYPE_T type)
{
    TRACE(0, "[%s] type = %d, gain_coef_l(*100) = %d, gain_coef_r(*100) = %d",
        __func__, type, (int32_t)(gain_coef_l * 100), (int32_t)(gain_coef_r * 100));

    if (type == ANC_FEEDFORWARD) {
        // Tuning PSAP or FF.
        uint32_t types = app_anc_get_curr_types();
        if (types & ANC_FEEDFORWARD) {
            app_anc_set_gain_f32(user, ANC_FEEDFORWARD, gain_coef_l, gain_coef_r);
        } else if (types & PSAP_FEEDFORWARD) {
            app_anc_set_gain_f32(user, PSAP_FEEDFORWARD, gain_coef_l, gain_coef_r);
        } else {
            TRACE(0, "[%s] WARNING: types(0x%x) is invalid for ANC_FEEDFORWARD", __func__, types);
        }
    } else if (type == ANC_TALKTHRU) {
        app_anc_set_gain_f32(user, ANC_TALKTHRU, gain_coef_l, gain_coef_r);
    } else if (type == ANC_FEEDBACK) {
        app_anc_set_gain_f32(user, ANC_FEEDBACK, gain_coef_l, gain_coef_r);
    } else {
        ASSERT(0, "[%s] type(%d) is invalid", __func__, type);
    }
}

#else
#define DEFAULT_ANC_GAIN        (512)
#define ANC_TIMER_PERIOD        (50)

// The anc total gain is same for FF and TT
static uint32_t default_ff_gain_l = DEFAULT_ANC_GAIN;
static uint32_t default_ff_gain_r = DEFAULT_ANC_GAIN;

static uint32_t default_fb_gain_l = DEFAULT_ANC_GAIN;
static uint32_t default_fb_gain_r = DEFAULT_ANC_GAIN;

static int32_t tgt_ff_gain_l = 0, tgt_ff_gain_r = 0;
static int32_t curr_ff_gain_l = 0, curr_ff_gain_r = 0;
static int32_t step_ff_gain_l = 0, step_ff_gain_r = 0;

static int32_t tgt_fb_gain_l = 0, tgt_fb_gain_r = 0;
static int32_t curr_fb_gain_l = 0, curr_fb_gain_r = 0;
static int32_t step_fb_gain_l = 0, step_fb_gain_r = 0;

struct anc_tuning_ctrl {
    uint8_t timer_init;
    uint8_t timer_ff_gain_run;
	uint8_t timer_fb_gain_run;
    const struct_anc_cfg *cfg;
    HWTIMER_ID timer_ff_gain_id;
	HWTIMER_ID timer_fb_gain_id;
};

static struct anc_tuning_ctrl anc_tctrl;


void anc_with_timer_reset(void)
{
    tgt_ff_gain_l = 0;
    tgt_ff_gain_r = 0;
    curr_ff_gain_l = 0;
    curr_ff_gain_r = 0;
    step_ff_gain_l = 0;
    step_ff_gain_r = 0;

    tgt_fb_gain_l = 0;
    tgt_fb_gain_r = 0;
    curr_fb_gain_l = 0;
    curr_fb_gain_r = 0;
    step_fb_gain_l = 0;
    step_fb_gain_r = 0;
}

static void _ff_gain_timer_handler(void *param)
{
    int32_t gain_l = 0;
    int32_t gain_r = 0;

    if (curr_ff_gain_l != tgt_ff_gain_l) {
        gain_l = curr_ff_gain_l + step_ff_gain_l;
    } else {
        gain_l = tgt_ff_gain_l;
    }

    if (curr_ff_gain_r != tgt_ff_gain_r) {
        gain_r = curr_ff_gain_r + step_ff_gain_r;
    } else {
        gain_r = tgt_ff_gain_r;
    }

    anc_set_gain(gain_l, gain_r, ANC_FEEDFORWARD);
    curr_ff_gain_l = gain_l;
    curr_ff_gain_r = gain_r;

    if (curr_ff_gain_l == tgt_ff_gain_l) {
        step_ff_gain_l = 0;
    }

    if (curr_ff_gain_r == tgt_ff_gain_r) {
        step_ff_gain_r = 0;
    }

    // If any step is not 0, need to set anc gain and restart timer.
    struct anc_tuning_ctrl *ctrl = (struct anc_tuning_ctrl *)param;
    if (step_ff_gain_l || step_ff_gain_r) {
        hwtimer_start(ctrl->timer_ff_gain_id, US_TO_TICKS(ANC_TIMER_PERIOD));      
    } else {
        ctrl->timer_ff_gain_run = 0;
    }
}

static void _fb_gain_timer_handler(void *param)
{
    int32_t gain_l = 0;
    int32_t gain_r = 0;

    if (curr_fb_gain_l != tgt_fb_gain_l) {
        gain_l = curr_fb_gain_l + step_fb_gain_l;
    } else {
        gain_l = tgt_fb_gain_l;
    }

    if (curr_fb_gain_r != tgt_fb_gain_r) {
        gain_r = curr_fb_gain_r + step_fb_gain_r;
    } else {
        gain_r = tgt_fb_gain_r;
    }

    anc_set_gain(gain_l, gain_r, ANC_FEEDBACK);
    curr_fb_gain_l = gain_l;
    curr_fb_gain_r = gain_r;

    if (curr_fb_gain_l == tgt_fb_gain_l) {
        step_fb_gain_l = 0;
    }

    if (curr_fb_gain_r == tgt_fb_gain_r) {
        step_fb_gain_r = 0;
    }

    // If any step is not 0, need to set anc gain and restart timer.
    struct anc_tuning_ctrl *ctrl = (struct anc_tuning_ctrl *)param;
    if (step_fb_gain_l || step_fb_gain_r) {
        hwtimer_start(ctrl->timer_fb_gain_id, US_TO_TICKS(ANC_TIMER_PERIOD));      
    } else {
        ctrl->timer_fb_gain_run = 0;
    }
}

static int32_t _anc_get_step_gain(int32_t delta_gain)
{
    int32_t step_gain = 0;

    if (delta_gain > 0) {
        step_gain = 1;
    } else if (delta_gain < 0) {
        step_gain = -1;
    } else {
        step_gain = 0;
    }

    return step_gain;
}

static void anc_set_ff_gain_with_timer(float gain_coef_l, float gain_coef_r, enum ANC_TYPE_T type)
{
    struct anc_tuning_ctrl *ctrl = &anc_tctrl;

    if (ctrl->timer_ff_gain_run) {
        hwtimer_stop(ctrl->timer_ff_gain_id);
        ctrl->timer_ff_gain_run = 0;
    }

	anc_get_gain(&curr_ff_gain_l, &curr_ff_gain_r, ANC_FEEDFORWARD);
	TRACE(3,"[%s] FF: curr_ff_gain_l = %d, curr_ff_gain_r = %d.", __func__, curr_ff_gain_l, curr_ff_gain_r);

	tgt_ff_gain_l = (int32_t)(default_ff_gain_l * gain_coef_l);
	tgt_ff_gain_r = (int32_t)(default_ff_gain_r * gain_coef_r);
	TRACE(3,"[%s] FF: tgt_ff_gain_l = %d, tgt_ff_gain_r = %d.", __func__, tgt_ff_gain_l, tgt_ff_gain_r);

	int32_t delta_gain_l = tgt_ff_gain_l - curr_ff_gain_l;
	int32_t delta_gain_r = tgt_ff_gain_r - curr_ff_gain_r;

    step_ff_gain_l = _anc_get_step_gain(delta_gain_l);
    step_ff_gain_r = _anc_get_step_gain(delta_gain_r);

    TRACE(3,"[%s] FF: delta_gain_l = %d, delta_gain_r = %d.", __func__, delta_gain_l, delta_gain_r);

    if (step_ff_gain_l || step_ff_gain_r) {
        hwtimer_start(ctrl->timer_ff_gain_id, MS_TO_TICKS(ANC_TIMER_PERIOD));
        ctrl->timer_ff_gain_run = 1;
    }
}

static void anc_set_fb_gain_with_timer(float gain_coef_l, float gain_coef_r, enum ANC_TYPE_T type)
{
    struct anc_tuning_ctrl *ctrl = &anc_tctrl;

    if (ctrl->timer_fb_gain_run) {
        hwtimer_stop(ctrl->timer_fb_gain_id);
        ctrl->timer_fb_gain_run = 0;
    }

	anc_get_gain(&curr_fb_gain_l, &curr_fb_gain_r, ANC_FEEDBACK);
	TRACE(3,"[%s] FB: curr_fb_gain_l = %d, curr_fb_gain_r = %d.", __func__, curr_fb_gain_l, curr_fb_gain_r);

	tgt_fb_gain_l = (int32_t)(default_fb_gain_l * gain_coef_l);
	tgt_fb_gain_r = (int32_t)(default_fb_gain_r * gain_coef_r);
	TRACE(3,"[%s] FB: tgt_fb_gain_l = %d, tgt_fb_gain_r = %d.", __func__, tgt_fb_gain_l, tgt_fb_gain_r);

	int32_t delta_gain_l = tgt_fb_gain_l - curr_fb_gain_l;
	int32_t delta_gain_r = tgt_fb_gain_r - curr_fb_gain_r;

    step_fb_gain_l = _anc_get_step_gain(delta_gain_l);
    step_fb_gain_r = _anc_get_step_gain(delta_gain_r);

    TRACE(3,"[%s] FB: delta_gain_l = %d, delta_gain_r = %d.", __func__, delta_gain_l, delta_gain_r);

    if (step_fb_gain_l || step_fb_gain_r) {
        hwtimer_start(ctrl->timer_fb_gain_id, MS_TO_TICKS(ANC_TIMER_PERIOD));
        ctrl->timer_fb_gain_run = 1;
    } 
}

void anc_set_gain_with_timer(float gain_coef_l, float gain_coef_r, enum ANC_TYPE_T type)
{
    if (type == ANC_FEEDFORWARD) {
        anc_set_ff_gain_with_timer(gain_coef_l, gain_coef_r, ANC_FEEDFORWARD);
    } else if (type == ANC_FEEDBACK) {
        anc_set_fb_gain_with_timer(gain_coef_l, gain_coef_r, ANC_FEEDBACK);
    } else {
        ASSERT(0, "[%s] type(%d) is invalid", __func__, type);
    }
}

void anc_with_timer_init(void)
{
    struct anc_tuning_ctrl *ctrl = &anc_tctrl;

    anc_with_timer_reset();
   
    if (!ctrl->timer_init) {
        ctrl->timer_ff_gain_id = hwtimer_alloc(_ff_gain_timer_handler, (void *)ctrl);
        ctrl->timer_ff_gain_run = 0;

        ctrl->timer_fb_gain_id = hwtimer_alloc(_fb_gain_timer_handler, (void *)ctrl);
        ctrl->timer_fb_gain_run = 0;
    }

    ctrl->timer_init = 1;
}

void anc_with_timer_deinit(void)
{
	struct anc_tuning_ctrl *ctrl = &anc_tctrl;

	if (ctrl->timer_ff_gain_run) {
    	hwtimer_stop(ctrl->timer_ff_gain_id);
        ctrl->timer_ff_gain_run = 0;
    }
	hwtimer_free(ctrl->timer_ff_gain_id);

    if (ctrl->timer_fb_gain_run) {
        hwtimer_stop(ctrl->timer_fb_gain_id);
        ctrl->timer_fb_gain_run = 0;
    }
    hwtimer_free(ctrl->timer_fb_gain_id);

	ctrl->timer_init = 0;
}
#endif

extern int32_t anc_assist_send_sync_info(void);
int32_t anc_assist_anc_set_gain_coef_impl(anc_gain_user_t user, float gain_coef_l, float gain_coef_r, enum ANC_TYPE_T type)
{
    ASSERT(gain_coef_l >= 0.0 || gain_coef_l <= 1.0, "[%s] gain_coef_l(%d) is invalid.", __func__, (uint32_t)(gain_coef_l * 100));
    ASSERT(gain_coef_r >= 0.0 || gain_coef_r <= 1.0, "[%s] gain_coef_r(%d) is invalid.", __func__, (uint32_t)(gain_coef_r * 100));

    if (app_anc_work_status() == false) {
        TRACE(0, "%s: ANC not ready ！！！", __func__);
#ifdef IBRT
        anc_assist_send_sync_info();
#endif
        return 1;
    }

#if defined(ANC_HW_GAIN_SMOOTHING)
    anc_set_gain_with_hw_smooth(user, gain_coef_l, gain_coef_r, type);
#else
    anc_set_gain_with_timer(gain_coef_l, gain_coef_r, type);
#endif

    return 0;
}

int32_t anc_assist_anc_set_gain_coef(uint32_t *ff_gain_changed, anc_assist_algo_id_t *ff_id, float *ff_gain, uint8_t ff_ch_num,
                                     uint32_t *fb_gain_changed, anc_assist_algo_id_t *fb_id, float *fb_gain, uint8_t fb_ch_num)
{
    // anc_assist_anc_set_gain_coef_impl(gain_coef, type);
    anc_assist_tws_sync_set_anc_gain_coef(ff_gain_changed, ff_id, ff_gain, ff_ch_num,
                                          fb_gain_changed, fb_id, fb_gain, fb_ch_num);

    return 0;
}

// TODO: Will clean up these code.
// extern const struct_anc_cfg * anc_coef_list_50p7k[1];

int32_t anc_assist_anc_switch_curve_impl(uint32_t index)
{
    TRACE(2,"[%s] index = %d", __func__, index);

    // anc_set_cfg(anc_coef_list_50p7k[index], ANC_FEEDFORWARD, ANC_GAIN_NO_DELAY);
    // anc_set_cfg(anc_coef_list_50p7k[index], ANC_FEEDBACK, ANC_GAIN_NO_DELAY);

    return 0;
}

int32_t anc_assist_anc_switch_curve(anc_assist_algo_id_t id, uint32_t index)
{
    // anc_assist_anc_switch_curve_impl(index);
    anc_assist_tws_sync_set_anc_curve(id, index);

    return 0;
}

extern int32_t app_voice_assist_fir_lms_set_fir_flag(uint32_t flag);

int32_t anc_assist_anc_switch_fir_flag_impl(uint32_t flag)
{
#if defined(VOICE_ASSIST_FF_FIR_LMS)
    app_voice_assist_fir_lms_set_fir_flag(flag);
#endif
    return 0;
}

int32_t anc_assist_anc_switch_fir_flag(anc_assist_algo_id_t id, uint32_t flag)
{
    anc_assist_tws_sync_set_fir_flag(id, flag);

    return 0;
}

int32_t anc_assist_anc_reset(void)
{
    TRACE(1, "[%s] ...", __func__);

#if defined(ANC_HW_GAIN_SMOOTHING)
    // TODO ...
#else
    anc_with_timer_reset();
#endif

    return 0;
}

int32_t anc_assist_anc_init(void)
{
    // TRACE(0, "[%s] ...", __func__);

#if defined(ANC_HW_GAIN_SMOOTHING)
    // TODO ...
#else
    anc_with_timer_init();
#endif

    // NOTE: Need to reset anc gain?

    return 0;
}

int32_t anc_assist_anc_deinit(void)
{
    // TRACE(0, "[%s] ...", __func__);

#if defined(ANC_HW_GAIN_SMOOTHING)
    // TODO ...
#else
    anc_with_timer_deinit();
#endif

    return 0;
}