/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#include "plat_addr_map.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_mcu2sens.h"
#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
#include "hal_norflash.h"
#endif
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "sens_comp_test.h"
#include "sens_mem.h"
#include "sens_trc_to_mcu.h"
#include "sensor_hub.h"
#include "sensor_hub_core.h"
#include "sensor_hub_core_app.h"
#ifdef SENSOR_TEST
#include "sensor_test.h"
#endif

#if defined(BT_CONN_TEST) || defined(I2S_TEST) || defined(SENSOR_ENGINE_TEST) || defined(MAX_POWER_TEST) || defined(UART_TEST)  || defined(BECO_TEST)

#include "hwtest.h"
#endif
#ifdef VD_TEST
#include "voice_detector.h"
#endif
#ifdef MINIMA_TEST
#include "minima_test.h"
#endif
#ifdef BSP_TEST_1600
#include "test_hal_main.h"
#endif
#ifdef PT_APP
#include "pt_app.h"
#endif

#if defined(__BT_APP__)
#include "norflash_api.h"

static osThreadId main_thread_tid = NULL;

int system_shutdown(void)
{
    TR_INFO(TR_MOD(MAIN), "system_shutdown!!");
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    return 0;
}

int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x8);
    return 0;
}

int signal_send_to_main_thread(uint32_t signals)
{
    osSignalSet(main_thread_tid, signals);
    return 0;
}

#endif

#ifdef VD_TEST
void vad_test_env_init(void);
#endif

#ifdef SENSOR_HUB_MISC_TEST
void sensor_hub_misc_test(void);
#endif

static sensor_hub_core_rx_irq_handler_t sensorHubCoreRxIrqHandler = NULL;
static sensor_hub_core_tx_done_irq_handler_t sensorHubCoreTxDoneIrqHandler = NULL;

void sensor_hub_core_register_rx_irq_handler(sensor_hub_core_rx_irq_handler_t irqHandler)
{
    sensorHubCoreRxIrqHandler = irqHandler;
}

void sensor_hub_core_register_tx_done_irq_handler(sensor_hub_core_tx_done_irq_handler_t irqHandler)
{
    sensorHubCoreTxDoneIrqHandler = irqHandler;
}

#ifdef NOAPP
#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
SRAM_RODATA_LOC
#endif
static const uint8_t handshake_msg[] = "Hi";
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
static HWTIMER_ID wakeup_timer;
static uint32_t wakeup_cnt;
static const uint32_t timer_ms = 11;

static void timer_handler(void *param)
{
    sensor_hub_send(&wakeup_cnt, sizeof(wakeup_cnt));
    wakeup_cnt++;
    hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
}
#endif

static unsigned int rx_handler(const void *data, unsigned int len)
{
    if (sensorHubCoreRxIrqHandler) {
        return sensorHubCoreRxIrqHandler(data, len);
    }

    return len;
}

static void tx_handler(const void *data, unsigned int len)
{
    if (sensorHubCoreTxDoneIrqHandler) {
        sensorHubCoreTxDoneIrqHandler(data, len);
        return;
    }
}

int sensor_hub_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_mcu2sens_send_seq(HAL_MCU2SENS_ID_0, data, len, seq);
}

int sensor_hub_send(const void *data, unsigned int len)
{
    return hal_mcu2sens_send(HAL_MCU2SENS_ID_0, data, len);
}

int sensor_hub_tx_active(unsigned int seq)
{
    return hal_mcu2sens_tx_active(HAL_MCU2SENS_ID_0, seq);
}

#if (!defined(NOAPP))&&defined(OS_THREAD_TIMING_STATISTICS_ENABLE)
#if defined(KERNEL_RTX5)
extern void rtx_show_all_threads_usage(void);
#else
extern void freertos_show_all_threads(void);
#endif
static void cpu_usage_timer_handler(void const *param);
osTimerDef(cpu_usage_timer, cpu_usage_timer_handler);
static osTimerId cpu_usage_timer_id = NULL;
static void cpu_usage_timer_handler(void const *param)
{
#if defined(KERNEL_RTX5)
    rtx_show_all_threads_usage();
#elif defined(__NuttX__)
    nx_show_all_threads_usage();
#else
    freertos_show_all_threads();
#endif
}
#endif

#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE)
static void slow_timer_calibration_thread(const void *arg)
{
    int count = 0;
    while(1) {
        hal_sys_timer_calib();
        if (count < 10) {
            //calibrate slow time more often after power up, utils 32K stabilized
            osDelay(500);
            count ++;
        } else {
            osDelay(3000);
        }
    }
}

osThreadDef(slow_timer_calibration_thread, osPriorityBelowNormal, 1,
    APP_THREAD_TIMER_CALIB_STACK_SIZE, "slow_timer_calibration_thread");

static osThreadId slow_timer_thread_id = NULL;

static void sensor_hub_init_slow_timer_calib_thread(void)
{
    if (slow_timer_thread_id == NULL) {
        slow_timer_thread_id = osThreadCreate(osThread(slow_timer_calibration_thread), NULL);
    }
}
#endif

int MAIN_ENTRY(void)
{
    int ret;
#if !defined(__NuttX__)
    hwtimer_init();
    hal_dma_open();

#ifdef SENS_TRC_TO_MCU
    sens_trace_to_mcu();
#endif

#ifdef APP_MCPP_SRV
    extern void mcpp_srv_open(void);
    mcpp_srv_open();
#endif

#if defined(DEBUG)
    enum HAL_TRACE_TRANSPORT_T transport;

#ifdef SENS_TRC_TO_MCU
    transport = HAL_TRACE_TRANSPORT_NULL;
#elif (DEBUG_PORT == 1)
    hal_iomux_set_uart0();

    transport = HAL_TRACE_TRANSPORT_UART0;
#elif (DEBUG_PORT == 2)
    hal_iomux_set_uart1();

    transport = HAL_TRACE_TRANSPORT_UART1;
#else
    transport = HAL_TRACE_TRANSPORT_QTY;
#endif
    hal_trace_open(transport);
#endif

    ret = hal_mcu2sens_open(HAL_MCU2SENS_ID_0, rx_handler, tx_handler, false);
    ASSERT(ret == 0, "hal_mcu2sens_open failed: %d", ret);

    ret = hal_mcu2sens_start_recv(HAL_MCU2SENS_ID_0);
    ASSERT(ret == 0, "hal_mcu2sens_start_recv failed: %d", ret);
#endif

#if defined(SUBSYS_FLASH_BOOT) && !defined(NO_FLASH_INIT)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);
#endif

    hal_sleep_start_stats(10000, 10000);

#ifdef APP_MCPP_SRV
    extern void mcpp_srv_open(void);
    mcpp_srv_open();
#endif

#ifdef NOAPP
    ret = hal_mcu2sens_send(HAL_MCU2SENS_ID_0, handshake_msg, (sizeof(handshake_msg) - 1));
    ASSERT(ret == 0, "hal_mcu2sens_send failed: %d", ret);
#else
    sensor_hub_core_app_init();
#endif

#ifdef MCU2SENS_MSG_TEST
    mcu2sens_msg_test();
#endif

#ifdef BECO_TEST
    beco_test();
#endif

#if defined(SLEEP_TEST) && !defined(NO_TIMER)
    wakeup_timer = hwtimer_alloc(timer_handler, NULL);
    ASSERT(wakeup_timer, "Failed to alloc wakeup_timer");

    ret = hwtimer_start(wakeup_timer, MS_TO_TICKS(timer_ms));
#endif

#ifdef PT_APP
    pt_app_entry(NULL);
#endif

#ifdef BT_CONN_TEST
    bt_conn_test();
#endif

#ifdef I2S_TEST
    i2s_test();
#endif

#ifdef SENSOR_TEST
    sensor_test();
#endif

#ifdef VAD_CODEC_TEST
    vad_codec_test();
#endif

#ifdef VD_TEST
    vad_test_env_init();
    voice_detector_test();
#endif

#if (!defined(NOAPP))&&defined(OS_THREAD_TIMING_STATISTICS_ENABLE)
    cpu_usage_timer_id = osTimerCreate(osTimer(cpu_usage_timer), osTimerPeriodic, NULL);
    if (cpu_usage_timer_id != NULL) {
        osTimerStart(cpu_usage_timer_id, OS_THREAD_TIMING_STATISTICS_PEROID_MS);
    }
#endif

#ifdef FULL_WORKLOAD_MODE_ENABLED
    sensor_hub_init_workload_thread();
#endif

#if defined(RTOS) && defined(FAST_TIMER_COMPENSATE)
    sensor_hub_init_slow_timer_calib_thread();
#endif

#if defined(SLEEP_TEST) || !defined(NOAPP)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
#endif
    hal_sysfreq_print_user_freq();

#ifdef MINIMA_TEST
    minima_test();
#endif

#ifdef SENSOR_HUB_MISC_TEST
    sensor_hub_misc_test();
#endif

#ifdef SENSOR_ENGINE_TEST
    sensor_engine_test();
#endif

#ifdef MAX_POWER_TEST
    max_power_test();
#endif

#ifdef BSP_TEST_1600
    test_hal_pattern();
#endif

#if defined(VPU_CFG_TEST)
    extern int32_t vpu_cfg_init(void);
    vpu_cfg_init();
#endif

#if defined(__BT_APP__)
    extern int app_init(void);
    main_thread_tid = osThreadGetId();
#ifndef RAM_NV_RECORD
    norflash_api_init();
#endif
    ret = app_init();
    TRACE(0,"app_init done:%d",ret);
#endif

#ifdef __NuttX__
    struct mallinfo minfo;
    minfo = kumm_mallinfo();
    TRACE(4, "nuttx heap total:%d, used:%d, free:%d-%d", minfo.arena, minfo.uordblks, minfo.fordblks, minfo.mxordblk);
#endif

    while (1) {
#ifdef NOAPP
#ifdef RTOS
        osDelay(2);
#else
        hal_sleep_enter_sleep();
#endif
#else
        osSignalWait(0x0, osWaitForever);
#endif
    }

    return 0;
}

#if defined(__NuttX__)
#ifdef __cplusplus
#define EXTERN_C                        extern "C"
#else
#define EXTERN_C                        extern
#endif

EXTERN_C bool btdrv_init_ok = 1;
EXTERN_C bool wifidrv_init_ok = 1;

EXTERN_C void bes_chip_init_early()
{
    int ret = 0;

    hal_dma_open();

#ifdef SENS_TRC_TO_MCU
    sens_trace_to_mcu();
#endif

#if defined(DEBUG)
    enum HAL_TRACE_TRANSPORT_T transport;

#ifdef SENS_TRC_TO_MCU
    transport = HAL_TRACE_TRANSPORT_NULL;
#elif (DEBUG_PORT == 1)
    hal_iomux_set_uart0();

    transport = HAL_TRACE_TRANSPORT_UART0;
#elif (DEBUG_PORT == 2)
    hal_iomux_set_uart1();

    transport = HAL_TRACE_TRANSPORT_UART1;
#else
    transport = HAL_TRACE_TRANSPORT_QTY;
#endif
    hal_trace_open(transport);
#endif

    hal_sleep_start_stats(10000, 10000);

    ret = hal_mcu2sens_open(HAL_MCU2SENS_ID_0, rx_handler, tx_handler, false);
    ASSERT(ret == 0, "hal_mcu2sens_open failed: %d", ret);

    ret = hal_mcu2sens_start_recv(HAL_MCU2SENS_ID_0);
    ASSERT(ret == 0, "hal_mcu2sens_start_recv failed: %d", ret);
#ifdef CONFIG_ARCH_HAVE_BACKTRACE
    extern void hal_trace_init_program_regions(void);
    hal_trace_init_program_regions();
#endif
}

EXTERN_C int hal_uart_printf_init(void);
EXTERN_C void hal_uart_printf(const char *fmt, ...);
EXTERN_C void nx_start(void);

void hal_uart_printf_output(const uint8_t *buf, uint32_t len);

EXTERN_C void nx_sysfreq_idle(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
}

EXTERN_C void nx_sysfreq_boost(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);
}

EXTERN_C int bes_global_shmem_setup_syslog_buffer(void);
EXTERN_C int _start(void)
{
#if 0//def DEBUG
    hal_uart_printf_init();
    hal_uart_printf("sens system power on....");
#endif
    //bes_global_shmem_setup_syslog_buffer();
    //hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
    nx_start();
    return 0;
}

osThreadDef(bes_main, (osPriorityAboveNormal), 1, (1024), "bes_main");

EXTERN_C void bes_app_initialize()
{
#ifdef CONFIG_SYSTEM_NSH
    osThreadCreate(osThread(bes_main), NULL);
#endif
}

#ifndef CONFIG_SYSTEM_NSH
EXTERN_C int nsh_main(int argc, FAR char *argv[])
{
    osThreadCreate(osThread(bes_main), NULL);
    while (1)
        sleep(10);
    return 0;
}
#endif

#endif
