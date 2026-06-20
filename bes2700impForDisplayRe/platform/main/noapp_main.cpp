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
#include "cmsis.h"
#include "analog.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_key.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "mpu_cfg.h"
#include "pmu.h"
#include "hal_evr.h"

#ifdef SENSOR_TEST
#include "sensor_test.h"
#endif

#ifdef RTOS
#include "cmsis_os.h"
#ifdef KERNEL_RTX
#include "rt_Time.h"
#endif
#endif

#ifdef HWTEST
#include "hwtest.h"
#ifdef VD_TEST
#include "voice_detector.h"
#endif
#endif

#ifdef DSP_TEST
#include "dsp_loader.h"
#endif

#ifdef LIS25BA_TEST
#include "sensor_test.h"
#endif

#ifdef I2S_HOST_TEST
#include "i2s_host.h"
#endif

#ifdef CHIP_FUNCTION_TEST
#include "chip_function_test.h"
#endif

#ifdef SLEEP_SIMU
#define TIMER_IRQ_PERIOD_MS             3
#define DELAY_PERIOD_MS                 4
#elif defined(SLEEP_TEST) && (SLEEP_TEST)
#define TIMER_IRQ_PERIOD_MS             3000
#define DELAY_PERIOD_MS                 4000
#else
#define TIMER_IRQ_PERIOD_MS             1000
#define DELAY_PERIOD_MS                 2000
#endif

#ifndef FLASH_FILL
#define FLASH_FILL                      1
#endif

#if defined(KERNEL_RTX) || defined(KERNEL_RTX5)
#define OS_TIME_STR                     "[%2u/%u]"
#define OS_CUR_TIME                     , SysTick->VAL, osKernelGetTickCount()
#else
#define OS_TIME_STR
#define OS_CUR_TIME
#endif

#if defined(MS_TIME)
#define TIME_STR                        "[%u]" OS_TIME_STR
#define CUR_TIME                        TICKS_TO_MS(hal_sys_timer_get())  OS_CUR_TIME
#elif defined(RAW_TIME)
#define TIME_STR                        "[0x%X]" OS_TIME_STR
#define CUR_TIME                        hal_sys_timer_get()  OS_CUR_TIME
#else
#define TIME_STR                        "[%u/0x%X]" OS_TIME_STR
#define CUR_TIME                        TICKS_TO_MS(hal_sys_timer_get()), hal_sys_timer_get() OS_CUR_TIME
#endif

#ifdef DEBUG_TIME
#define TR_INFO_TIME(attr, str, ...)    TRACE(attr, TIME_STR " " str, CUR_TIME, ##__VA_ARGS__)
#else
#define TR_INFO_TIME(attr, str, ...)    TRACE(attr, str, ##__VA_ARGS__)
#endif

const static unsigned char bytes[FLASH_FILL] = { 0x1, };

#if defined(DEBUG) && (DEBUG_PORT == 1)
static bool i2c_flag = false;

static void switch_between_i2c_and_uart(bool to_i2c, bool flush_trace)
{
    uint32_t lock;

    lock = int_lock();

    if (to_i2c) {
        TR_INFO(TR_MOD(MAIN), "Switch to I2C");
        if (flush_trace) {
            TRACE_FLUSH();
        }
        while (hal_trace_busy());
        hal_trace_pause();

        hal_iomux_set_analog_i2c();
    } else {
        hal_iomux_set_uart0();

        hal_trace_continue();
        if (flush_trace) {
            TRACE_FLUSH();
        }
        TR_INFO(TR_MOD(MAIN), "Switch to UART0");
    }

    int_unlock(lock);
}
#endif

static HWTIMER_CALLBACK_T timer_cb[3];

#ifdef __cplusplus
extern "C"
#endif
void sleep_test_register_timer_callback(HWTIMER_CALLBACK_T cb)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (!timer_cb[i]) {
            timer_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_timer_callback(void *param)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(timer_cb); i++) {
        if (timer_cb[i]) {
            timer_cb[i](param);
        }
    }
}

static void (*loop_cb[3])(void);

#ifdef __cplusplus
extern "C"
#endif
void sleep_test_register_loop_callback(void (*cb)(void))
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (!loop_cb[i]) {
            loop_cb[i] = cb;
            return;
        }
    }

    ASSERT(false, "%s: failed", __func__);
}

POSSIBLY_UNUSED
static void sleep_test_exec_loop_callback(void)
{
    for (uint32_t i = 0; i < ARRAY_SIZE(loop_cb); i++) {
        if (loop_cb[i]) {
            loop_cb[i]();
        }
    }
}

#ifdef NO_TIMER
#if defined(SLEEP_TEST) && (SLEEP_TEST)
static bool wakeup_flag = false;
#endif
#else
static HWTIMER_ID hw_timer = NULL;
static void timer_handler(void *param)
{
    TR_INFO_TIME(TR_MOD(MAIN), "Timer handler: %u", (uint32_t)param);

    sleep_test_exec_timer_callback(param);

    hwtimer_start(hw_timer, MS_TO_TICKS(TIMER_IRQ_PERIOD_MS));
    TR_INFO_TIME(TR_MOD(MAIN), "Start timer %u ms", TIMER_IRQ_PERIOD_MS);
}
#endif

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    TR_INFO(TR_MOD(MAIN), "%s: code=0x%X event=%u", __FUNCTION__, key_code, key_event);

    if (key_code == HAL_KEY_CODE_PWR) {
        if (0) {
#if (defined(SLEEP_TEST) && (SLEEP_TEST)) && defined(NO_TIMER)
        } else if (key_event == HAL_KEY_EVENT_CLICK) {
            enum HAL_CMU_FREQ_T sys_freq;

            wakeup_flag = !wakeup_flag;
            if (wakeup_flag) {
                sys_freq = HAL_CMU_FREQ_26M;
            } else {
                sys_freq = HAL_CMU_FREQ_32K;
            }
            hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, sys_freq);

            TR_INFO(TR_MOD(MAIN), "%s: sleep=%d", __FUNCTION__, !wakeup_flag);
#endif
#if defined(DEBUG) && (DEBUG_PORT == 1)
        } else if (key_event == HAL_KEY_EVENT_LONGPRESS) {
            i2c_flag = !i2c_flag;
            switch_between_i2c_and_uart(i2c_flag, true);
#endif
        }
    }

    return 0;
}

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.

int MAIN_ENTRY(void)
{
    int POSSIBLY_UNUSED ret;

    hwtimer_init();
    hal_audma_open();
    hal_gpdma_open();
#ifdef DEBUG
#if (DEBUG_PORT == 3)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#elif (DEBUG_PORT == 2)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#else
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
#endif

#ifdef EVR_TRACING
    hal_evr_init(0);
#endif

#if !defined(SIMU) && !defined(FPGA) && !defined(BT_CONN_SIMU)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);
#endif

    TR_INFO_TIME(TR_MOD(MAIN), "main started: filled@0x%08x", (uint32_t)bytes);

    mpu_cfg();
#ifndef NO_PMU
    ret = pmu_open();
    ASSERT(ret == 0, "Failed to open pmu");
#endif
    analog_open();

#ifdef REBOOT_TEST
    if (!hal_hw_bootmode_get().global) {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_FLASH_BOOT);
        pmu_reboot();
        ASSERT(false, "%s: Reboot failed", __func__);
    }
#endif

    hal_cmu_simu_pass();

    enum HAL_CMU_FREQ_T sys_freq;

#if defined(SLEEP_TEST) && (SLEEP_TEST)
    sys_freq = HAL_CMU_FREQ_32K;
#elif defined(ULTRA_LOW_POWER)
    sys_freq = HAL_CMU_FREQ_52M;
#else
    sys_freq = HAL_CMU_FREQ_104M;
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, sys_freq);

    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

#ifdef CHIP_WAKE_TEST
    hal_chip_wake_lock(HAL_CHIP_WAKE_LOCK_USER_31);
#endif

    hal_sleep_start_stats(10000, 10000);

#ifdef HWTEST

#ifdef CMSIS_TEST
    cmsis_test();
#endif

#ifdef SBC_ROM_TEST
    sbc_rom_test();
#endif
#ifdef LC3_TEST
    lc3_test();
#endif

#ifdef SBC_TEST
    sbc_test();
#endif

#ifdef ACCDEC_TEST
    acc_dec_test();
#endif

#ifdef USB_SERIAL_TEST
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    usb_serial_test();
#endif

#ifdef USB_SERIAL_DIRECT_XFER_TEST
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    usb_serial_direct_xfer_test();
#endif

#ifdef USB_AUDIO_TEST
    pmu_usb_config(PMU_USB_CONFIG_TYPE_DEVICE);
    usb_audio_test();
#endif

#ifdef I2C_TEST
    i2c_test();
#endif

#ifdef I2C_DEBUG_SLAVE_TEST
    i2c_debug_slave_test();
#endif

#ifdef I2C_SLAVE_TEST
    i2c_slave_test();
#endif

#ifdef AF_TEST
    af_test();
#endif

#ifdef VD_TEST
    voice_detector_test();
#endif

#ifdef CP_TEST
    cp_test();
#endif

#ifdef SEC_ENG_TEST
    sec_eng_test();
#endif

#ifdef TDM_TEST
    tdm_test();
#endif

#ifdef A7_DSP_TEST
    a7_dsp_test();
#endif

#ifdef TRANSQ_TEST
    transq_test();
#endif

#ifdef FLASH_TEST
    flash_test();
#endif

#ifdef NANDFLASH_TEST
    nandflash_test();
#endif

#ifdef PSRAM_TEST
    psram_test();
#endif

#ifdef PSRAM_WINDOW_TEST
    psram_window_test();
#endif

#ifdef PSRAM_DUAL_PORT_TEST
    psram_dual_port_test();
#endif

#ifdef PSRAMUHS_TEST
    psramuhs_test();
#endif

#ifdef PSRAMUHS_WINDOW_TEST
    psramuhs_window_test();
#endif

#ifdef MBW_TEST
    mbw_test();
#endif

#ifdef SPI_NORFLASH_TEST
    spi_norflash_test();
#endif

#ifdef SENSOR_HUB_TEST
    sensor_hub_test();
#endif

#ifdef BT_HOST_TEST
    bt_host_test();
#endif

#ifdef DSP_HIFI4_TEST
    dsp_hifi4_test();
#ifdef HIFI4_ADDA_LOOP_TEST
    adda_loop_start_demo();
#endif
#endif

#ifdef DSP_M55_TEST
    dsp_m55_test();
#endif

#ifdef CODEC_SIMU
    codec_test();
#endif

#ifdef I2S_TEST
    i2s_test();
#endif

#ifdef SENSOR_TEST
    sensor_test();
#endif

#ifdef BECO_TEST
    beco_test();
#endif

#ifdef TZ_TEST
    tz_test();
#endif

#ifdef BT_CONN_TEST
    bt_conn_test();
#endif

#ifdef BT_CONN_SIMU
    bt_conn_simu();
#endif

#ifdef FT_TEST
    ft_test_main();
#endif

#ifdef LIS25BA_TEST
    lis25ba_test();
#endif

#ifdef TRNG_TEST
    trng_test();
#endif

#ifdef PWM_TEST
    pwm_test();
#endif

#ifdef BREATHING_LED_TEST
    breathing_led_test();
#endif

#ifdef IR_TEST
    ir_test();
#endif

#ifdef RSA_TEST
    test_rsa();
#endif

#ifdef DAC_TEST
    dac_test();
#endif

#ifdef ADC2_TEST
    adc2_test();
#endif

#ifdef SDMMC_TEST
    sdmmc_test();
#endif

#ifdef SDEMMC_TEST
    sdemmc_test();
#endif

#ifdef EMMC_TEST
    emmc_test();
#endif

#ifdef SDIO_HOST_TEST
    sdio_host_test();
#endif

#ifdef SDIO_HOST_SIGNAL_TEST
    sdio_host_signal_test();
#endif

#ifdef SDIO_HOST_FIRMWARE_TEST
    sdio_host_firmware_test();
#endif

#ifdef SDIO_HOST_PROGRAMMER_TEST
    sdio_host_programmer_test();
#endif

#ifdef SDIO_DEVICE_TEST
    sdio_device_test();
#endif

#ifdef DSI_TEST
    dsi_test();
#endif

#ifdef CSI_TEST
    csi_test();
#endif

#ifdef QSPI_1501_TEST
    qspi_1501_test();
#endif

#ifdef QSPI_LCDC_TEST
    qspi_lcdc_test();
#endif

#ifdef I2S_HOST_TEST
    i2s_host_test();
#endif

#ifdef SPI_TEST
    spi_test();
#endif

#ifdef SPI2SDIO_TEST
    spi2sdio_test();
#endif

#ifdef SPI_DEBUG_SLAVE_TEST
    spi_debug_slave_test();
#endif

#ifdef UART_TEST
    uart_test();
#endif

#ifdef MAX_POWER_TEST
    max_power_test();
#endif

#ifdef GPIO_TEST
    gpio_test();
#endif

#ifdef GPU_TEST
    gpu_test();
#endif

#ifdef GRAPHIC_TEST
    run_graphic_test_cases();
#endif

#ifdef GPADC_TEST
    gpadc_test();
#endif

#ifdef WDT_TEST
    wdt_test();
#endif

#ifdef HEAP_TEST
    heap_test();
#endif

#ifdef CHARGER_TEST
    charger_test();
#endif

#ifdef COREMARK_TEST
    coremark_test();
#endif

#ifdef DHRYSTONE_TEST
    dhrystone_test();
#endif

#ifdef MBEDTLS_TEST
    mbedtls_test();
#endif

#ifdef CP_SUBSYS_TEST
    cp_subsys_test();
#endif

#if !(defined(SLEEP_TEST) && (SLEEP_TEST))
    SAFE_PROGRAM_STOP();
#endif

#endif // HWTEST

    hal_key_open(false, key_event_process);

#if defined(DEBUG) && (DEBUG_PORT == 1)
    TR_INFO(TR_MOD(MAIN), "TIP: Long press PwrKey to switch between I2C and UART0");
#endif

#ifdef CHIP_FUNCTION_TEST
    chip_function_test();
#endif

#ifdef NO_TIMER
#if defined(SLEEP_TEST) && (SLEEP_TEST)
#if (SLEEP_TEST == 2)
    wakeup_flag = true;
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
    TR_INFO(TR_MOD(MAIN), "Single click PwrKey to sleep ...");
#else
    TR_INFO(TR_MOD(MAIN), "Enter sleep (and single click PwrKey to wakeup) ...");
#endif
#endif
#else
    hw_timer = hwtimer_alloc(timer_handler, 0);
    hwtimer_start(hw_timer, MS_TO_TICKS(TIMER_IRQ_PERIOD_MS));
    TR_INFO(TR_MOD(MAIN), TIME_STR " Start timer %u ms", CUR_TIME, TIMER_IRQ_PERIOD_MS);
#endif

    while (1) {
        sleep_test_exec_loop_callback();
#if (defined(SLEEP_TEST) && (SLEEP_TEST)) && !defined(RTOS)
        hal_sleep_enter_sleep();
#else
#if defined(NO_TIMER) && defined(RTOS)
        osSignalWait(0x0, osWaitForever);
#elif !defined(NO_TIMER)
        osDelay(DELAY_PERIOD_MS);
        TR_INFO(TR_MOD(MAIN), TIME_STR " Delay %u ms done", CUR_TIME, DELAY_PERIOD_MS);
#endif
#endif
    }

    SAFE_PROGRAM_STOP();
    return 0;
}

