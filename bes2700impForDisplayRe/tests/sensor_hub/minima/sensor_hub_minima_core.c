/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifdef SENSOR_HUB_MINIMA
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "pmu.h"
#include "analog.h"
#include "minima_avs.h"
#include "minima_dvfs_interface.h"
#include "DmIP_driver.h"
#include "initClockTop.h"
#include "sensor_hub_minima_core.h"
#if !defined(MINIMA_TEST)
#include "sensor_hub_minima_core_conf.h"
#endif

#define MINIMA_VERBOSE 0

POSSIBLY_UNUSED static uint32_t m_last_volt_op = 0;
POSSIBLY_UNUSED static uint32_t m_cur_volt_op  = 0;
POSSIBLY_UNUSED static uint32_t m_freq_volt_op_idx;
POSSIBLY_UNUSED static bool m_inited = false;
POSSIBLY_UNUSED static bool m_enabled = false;
POSSIBLY_UNUSED static enum HAL_CMU_FREQ_T sysfreq_cur_freq = HAL_CMU_FREQ_32K;
POSSIBLY_UNUSED static uint32_t sysfreq_req_cnt = 0;

void sensor_hub_minima_setup_freq(int idx, enum HAL_CMU_FREQ_T freq)
{
#if defined(MINIMA_TEST)
    return;
#endif
    if ((!m_inited) || (!m_enabled)) {
        return;
    }
    if (idx == 0) {
        sysfreq_req_cnt++;
    } else if (idx == 1) {
        sysfreq_cur_freq = freq;
        app_sensor_hub_minima_set_freq(freq);
    }
#if (MINIMA_VERBOSE > 0)
    TRACE(0, "%s:[%d],freq=%d,cur_freq=%d,cnt=%d",
        __func__,idx,freq,sysfreq_cur_freq,sysfreq_req_cnt);
#endif
}

static void minima_set_idle_freq(void)
{
    uint32_t freq_mhz, i;
    enum HAL_CMU_FREQ_T freq = hal_sysfreq_get();
    uint32_t arr[] = {26,6,13,26,52,78,104,208};
    MinimaDvfsFreqVoltageOP_t *p;

    ASSERT(freq < ARRAY_SIZE(arr),"%s: invalid freq=%d", __func__, freq);
    freq_mhz = arr[freq];

    TRACE(0, "%s:freq_mz=%d", __func__,freq_mhz);

    setDvfsIdleFreq(freq_mhz);
    for (i = 0; i <= getDvfsVoltageOPNum(); i++) {
        p = getDvfsVoltageOP(i);
        if (p) {
            TRACE(0,"DVFS OP[%d]: freq=%d,vol=[%d,%d]",i,p->freq,p->voltage,p->voltageMin);
            if (p->freq == freq_mhz) {
                minimaAvsSetSWVoltage(p->voltage);
            }
        }
    }
}

void app_sensor_hub_minima_init(void)
{
    if (m_inited) {
        return;
    }
    minima_set_idle_freq();
    initAvs();
    initDvfs();

    config_Clocktop(MINIMA_BYPASS,MINIMA_180DEG,MINIMA_FCLK_ENABLE);
#if !defined(MINIMA_TEST)
    m_last_volt_op = LAST_VOLT_OP_INI_VAL;
    m_cur_volt_op  = CUR_VOLT_OP_INI_VAL;
#endif
    m_enabled = true;
    m_inited = true;
    TRACE(0, "%s: done", __func__);
}

void app_sensor_hub_minima_exit(void)
{
    m_enabled = false;
    m_inited = false;
}

void app_sensor_hub_minima_enable(void)
{
    m_enabled = true;
}

void app_sensor_hub_minima_disable(void)
{
    m_enabled = false;
}

#if !defined(MINIMA_TEST)
static bool app_sensor_hub_minima_set_volt_op(uint32_t new_volt_op)
{
    bool set = false;

    if (new_volt_op != m_cur_volt_op) {
        m_last_volt_op = m_cur_volt_op;
        m_cur_volt_op = new_volt_op;
        set = true;
    }
    if (set) {
#if (MINIMA_VERBOSE > 0)
        TRACE(0, "%s:last_volt_op=%d,cur_volt_op=%d", __func__,m_last_volt_op,m_cur_volt_op);
#endif
        //return false;
        setCurrentOp(m_cur_volt_op);
        releaseOp(m_last_volt_op);
        clearIrqCounts();
    }
    return set;
}
#endif

bool app_sensor_hub_minima_set_freq(uint8_t freq)
{
#if !defined(MINIMA_TEST)
    uint32_t i;

    if (!m_inited) {
        return false;
    }
    if (!m_enabled) {
        return false;
    }
#if (MINIMA_VERBOSE > 0)
    TRACE(0, "%s:freq=%d", __func__,freq);
#endif

    // find the volt_op
    for (i = 0; i < ARRAY_SIZE(freq_volt_op_list); i++) {
        uint32_t volt_op;
        if (freq == freq_volt_op_list[i].freq) {
            volt_op = freq_volt_op_list[i].volt_op;
            m_freq_volt_op_idx = i;
            app_sensor_hub_minima_set_volt_op(volt_op);
            return true;;
        }
    }
    ASSERT(false, "%s: invalid freq(%d)", __func__, freq);
#endif
    return false;
}

#endif /* SENSOR_HUB_MINIMA */
