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
#if defined(VOICE_DETECTOR_EN) && defined(VAD_ADPT_TEST_ENABLE)
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hwtimer_list.h"
#include "sens_msg.h"
#include "sensor_hub_core.h"
#include "sensor_hub_vad_core.h"
#include "sensor_hub_vad_adapter.h"
#include "sensor_hub_vad_adapter_tc.h"
#ifdef THIRDPARTY_LIB_GG
#include "gsound_lib.h"
#endif

#define ADPT_TC_VERBOSE 1

#if (ADPT_TC_VERBOSE > 0)
#define ADPT_TC_LOG TRACE
#else
#define ADPT_TC_LOG(...) do{}while(0)
#endif

#ifndef VAD_ADPT_TEST_ID
#define VAD_ADPT_TEST_ID APP_VAD_ADPT_ID_DEMO
#endif

#define ADPT_TC_MEM_TEST
#define ADPT_TC_MIC_DATA
#define ADPT_TC_CPU_TEST
#define ADPT_TC_KWS_TEST

#if defined(ADPT_TC_MEM_TEST)
#define MBUF_SIZE (8*1024)
#define SCAN_NUM  (10)
#define SCAN_DLY  (2)
static uint32_t mem_test_cnt = 0;
static uint8_t mem_test_buf[MBUF_SIZE];

POSSIBLY_UNUSED static void do_mem_test(void *param)
{
    uint32_t i, k, sum = 0;

    for (k = 0; k < SCAN_NUM; k++) {
        uint32_t *pd = (uint32_t *)mem_test_buf;

        for (i = 0; i < MBUF_SIZE/4; i += 8) {
            pd[i+0] = i+k+0;
            pd[i+1] = i+k+1;
            pd[i+2] = i+k+2;
            pd[i+3] = i+k+3;
            pd[i+4] = i+k+4;
            pd[i+5] = i+k+5;
            pd[i+6] = i+k+6;
            pd[i+7] = i+k+7;
        }

        for (i = 0; i < MBUF_SIZE/4; i += 8) {
            sum += pd[i+0];
            sum += pd[i+1];
            sum += pd[i+2];
            sum += pd[i+3];
            sum += pd[i+4];
            sum += pd[i+5];
            sum += pd[i+6];
            sum += pd[i+7];
        }
#if (ADPT_TC_VERBOSE > 1)
        ADPT_TC_LOG(0, "MEM_SCAN:addr=%x, size=%d, sum=%x, cnt=%d", (uint32_t)pd, MBUF_SIZE, sum, ++mem_test_cnt);
#endif
    }
    /*The threads maybe switched when call osDelay() */
    osDelay(SCAN_DLY);

    if (param) {
        *(uint32_t *)param = sum;
    }
}

static int app_vad_adapter_tc_mem_test(bool on)
{
    ADPT_TC_LOG(0, "%s: on=%d", __func__, on);

    if (on) {
        mem_test_cnt = 0;
#ifdef FULL_WORKLOAD_MODE_ENABLED
        sensor_hub_add_workload(WORKLOAD_ID_0, do_mem_test, NULL);
        sensor_hub_enable_workload(WORKLOAD_ID_0);
#endif
    } else {
#ifdef FULL_WORKLOAD_MODE_ENABLED
        sensor_hub_disable_workload(WORKLOAD_ID_0);
        sensor_hub_remove_workload(WORKLOAD_ID_0);
#endif
    }
    return 0;
}
#endif /* defined(ADPT_TC_MEM_TEST) */

#if defined(ADPT_TC_MIC_DATA)
static bool vad_adpt_mic_data_test = false;
static int app_vad_adapter_tc_mic_data(bool on)
{
    vad_adpt_mic_data_test = on;
    return 0;
}
#endif

#ifdef ADPT_TC_CPU_TEST
inline int smmul (const int a, const int b)
{
    uint32_t result ;
    __asm__ ("smmul %0, %1, %2"
            : "=r" (result)
            : "r" (a), "r" (b)) ;
    return result ;
}

inline int smull (const int a, const int b)
{
    uint32_t result ;
    __asm__ ("smull %0, %1, %2"
            : "=r" (result)
            : "r" (a), "r" (b)) ;
    return result ;
}

inline int smmulx (const int a, const int b)
{
    int sa = a<0, sb = b<0;

    uint32_t ua = sa? -a : a;
    uint32_t ub = sb? -b : b;
    uint64_t acc = 0;
    for (int i=0; i<32; i++) {
        acc += (ua & (1 << i))? (uint64_t)ub << i: 0;
    }

    return ((sa ^ sb)? -acc: acc) >> 32;
}

POSSIBLY_UNUSED static void test_smmul_1(void *param)
{
    int error = 0;
    uint32_t i, j;
    uint32_t x,y;

//    ADPT_TC_LOG(1,"%s", __func__);
    for(i=0; i<32; i++) {
        for(j=0; j<32; j++) {
            int a = 1<<i;
            int b = 1<<j;
            if ((x = smmulx(a,b)) != (y = smmul(a, b))) {
                ADPT_TC_LOG(4,"1<<%d (*>>32) 1<<%d = 0x%08x/0x%08x\n", i,j, y, x);
                error = 1;
            }
        }
    }
    if (param) {
        *(uint32_t *)param = error;
    }
}

static int app_vad_adapter_tc_cpu_test(bool on)
{
    ADPT_TC_LOG(0, "%s: on=%d", __func__, on);

    if (on) {
#ifdef FULL_WORKLOAD_MODE_ENABLED
        sensor_hub_add_workload(WORKLOAD_ID_1, test_smmul_1, NULL);
        sensor_hub_enable_workload(WORKLOAD_ID_1);
#endif
    } else {
#ifdef FULL_WORKLOAD_MODE_ENABLED
        sensor_hub_disable_workload(WORKLOAD_ID_1);
        sensor_hub_remove_workload(WORKLOAD_ID_1);
#endif
    }
    return 0;
}
#endif /* ADPT_TC_CPU_TEST */

#ifdef ADPT_TC_KWS_TEST
static POSSIBLY_UNUSED bool vad_adpt_kws_test = false;

#if (VAD_ADPT_CLOCK_FREQ == 4)
static POSSIBLY_UNUSED bool cpu_low_sys_clk = true;
#else
static POSSIBLY_UNUSED bool cpu_low_sys_clk = false;
#endif

#ifdef THIRDPARTY_LIB_GG
static void vad_adpt_gs_kws_handler(void *param, uint16_t len)
{
    TRACE(0, "%s============> OK GOOGLE", __func__);
    app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_VOICE_CMD, 0, 0, 0);
}
#endif

static int app_vad_adapter_tc_kws_test(bool on)
{
    ADPT_TC_LOG(0, "%s: on=%d", __func__, on);
#ifdef THIRDPARTY_LIB_GG
    gsound_lib_init(on, vad_adpt_gs_kws_handler);
    vad_adpt_kws_test = on;
#endif
    return 0;
}
#endif /* ADPT_TC_KWS_TEST */

int app_vad_adapter_test_handler(enum APP_VAD_ADPT_ID_T id, uint32_t scmd, uint32_t arg1, uint32_t arg2)
{
    ADPT_TC_LOG(0, "%s:id=%d,%d/%d/%d", __func__, id, scmd, arg1, arg2);

    if (id != VAD_ADPT_TEST_ID) {
        TRACE(0, "%s: WARNING: bad adpt id %d", __func__, id);
        return -1;
    }
    switch (scmd) {
#if defined(ADPT_TC_MEM_TEST)
    case SENS_VAD_TEST_SCMD_MEM_TEST:
        app_vad_adapter_tc_mem_test(!!arg1);
        break;
#endif
#if defined(ADPT_TC_CPU_TEST)
    case SENS_VAD_TEST_SCMD_CPU_TEST:
        app_vad_adapter_tc_cpu_test(!!arg1);
        break;
#endif
#if defined(ADPT_TC_KWS_TEST)
    case SENS_VAD_TEST_SCMD_KWS_TEST:
        app_vad_adapter_tc_kws_test(!!arg1);
        break;
#endif
#if defined(ADPT_TC_MIC_DATA)
    case SENS_VAD_TEST_SCMD_MIC_DATA:
        app_vad_adapter_tc_mic_data(!!arg1);
        break;
#endif
    default:
        TRACE(0, "%s: WARNING: unknown test cmd %d", __func__, scmd);
        break;
    }
    return 0;
}

uint32_t vad_adpt_test_cap_stream_handler(uint8_t *buf, uint32_t len)
{
#ifdef ADPT_TC_KWS_TEST
    if (vad_adpt_kws_test) {
        if (cpu_low_sys_clk) {
            hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, HAL_CMU_FREQ_52M);
        }
#ifdef THIRDPARTY_LIB_GG
        gsound_lib_parse(buf, len);
#endif
        if (cpu_low_sys_clk) {
            hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, HAL_CMU_FREQ_32K);
        }
    }
#endif

#ifdef ADPT_TC_MIC_DATA
    if (vad_adpt_mic_data_test) {
        app_sensor_hub_core_vad_send_evt_msg(SENS_EVT_ID_VAD_IRQ_TRIG, 0, 0, 0);
    }
#endif
    return 0;
}
#endif
