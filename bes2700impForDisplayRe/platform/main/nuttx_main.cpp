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
//#define main(...) _sdk_main(...)
//#include "main.cpp"
#include "nuttx/init.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_sdmmc.h"
#include "tool_msg.h"
#include "random.h"
#if defined(CONFIG_ARCH_TRUSTZONE_SECURE)
#include "hal_sec.h"
#include <arm_cmse.h>
#include "partition_ARMCM.h"
#include "mpc.h"
#endif
#include "plat_addr_map.h"
#include "analog.h"
#include "apps.h"
#include "app_trace_rx.h"
#include "app_utils.h"
#include "cmsis.h"
#include "factory_section.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
#include "hal_evr.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_wdt.h"
#include "hwtimer_list.h"
#include "mpu_cfg.h"
#include "norflash_api.h"
#include "pmu.h"
#include "random.h"
#include "stdlib.h"
#include "tgt_hardware.h"
#include "watchdog/watchdog.h"
#include "reboot_param.h"

#ifdef RTOS
#include "cmsis_os.h"
#include "app_factory.h"
#endif

#ifdef RAM_DUMP_TO_FLASH
#include "ramdump_section.h"
#endif

#ifdef CORE_DUMP_TO_FLASH
#include "coredump_section.h"
#endif

#ifdef BTH_IN_ROM
#include "bt_rom_init.h"
#endif

#ifdef __cplusplus
#define EXTERN_C                        extern "C"
#else
#define EXTERN_C                        extern
#endif

#if defined(ROM_UTILS_ON)
void rom_utils_init(void);
#endif

EXTERN_C void Jtag_hook(void);
EXTERN_C void btdrv_start_bt(void);
EXTERN_C int platform_trace_enable();
EXTERN_C BOOT_BSS_LOC volatile bool btdrv_init_ok = false;
EXTERN_C BOOT_BSS_LOC volatile bool wifidrv_init_ok = false;

#define PROP_NAME_MAX      127
#define PROP_BDMAC         "ro.factory.mac_bt"
#define PROP_BTMAC         "ro.factory.mac_wifi"

uint8_t btdrv_bdaddr[6] =       { 0x66, 0x55, 0x44, 0x33, 0x22, 0x11 };
uint8_t btdrv_btaddr[7] = { 0x01, 0x66, 0x55, 0x44, 0x33, 0x22, 0x12 };

extern uint8_t bt_global_addr[6];
extern uint8_t ble_global_addr[6];

static BOOT_BSS_LOC bool  is_chip_init_early_done = false;
#ifdef CONFIG_BES_LPUART
static void trace_enable_early()
{
#if defined(DEBUG)
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#elif (DEBUG_PORT == 2)
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
    //hal_iomux_set_analog_i2c();
#elif (DEBUG_PORT == 3)
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#endif
#endif
}
#endif /*#CONFIG_BES_LPUART*/

EXTERN_C void pmu_open_irq(void);
EXTERN_C int pmu_gpio_irq_init(void);
EXTERN_C void bes_chip_flash_init();

static int tgt_hardware_setup(void)
{
#ifdef __APP_USE_LED_INDICATE_IBRT_STATUS__
    for (uint8_t i=0;i<3;i++){
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&cfg_ibrt_indication_pinmux_pwl[i], 1);
        if(i==0)
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_ibrt_indication_pinmux_pwl[i].pin, HAL_GPIO_DIR_OUT, 0);
        else
            hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)cfg_ibrt_indication_pinmux_pwl[i].pin, HAL_GPIO_DIR_OUT, 1);
    }
#endif

#if CFG_HW_PWL_NUM > 0
    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)cfg_hw_pinmux_pwl, sizeof(cfg_hw_pinmux_pwl)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP));
#endif

#ifdef APP_BATTERY_ENABLE
    if (app_battery_ext_charger_indicator_cfg.pin != HAL_IOMUX_PIN_NUM){
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_indicator_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_indicator_cfg.pin, HAL_GPIO_DIR_IN, 1);
    }
#endif
    return 0;
}

EXTERN_C void a7_dsp_boot(void);

/* Note: irq is disabled when bes_chip_init_early*/
EXTERN_C void bes_chip_init_early()
{
    //TRACE(0, "bes_chip_init_early done:%d",is_chip_init_early_done);
    if (is_chip_init_early_done)
        return;
    //hal_iomux_set_analog_i2c();
    tgt_hardware_setup();
#if defined(ROM_UTILS_ON)
    rom_utils_init();
#endif

#if !defined(CONFIG_USE_SLOW_TIMER) || defined(CONFIG_BES_ONESHOT_SYSTICK)
    hwtimer_init();
#endif

    hal_dma_set_delay_func((HAL_DMA_DELAY_FUNC)osDelay);
    hal_audma_open();
    hal_gpdma_open();
#if defined(CONFIG_HIFI4_USE_AUDMA)
    //hal_dma_close_audma_inst();
#endif
    //mpu_cfg();

#if !defined(CONFIG_ARCH_TRUSTZONE_SECURE) && !defined(CHIP_SUBSYS_SENS)
    pmu_open();
    analog_open();
#endif
#if defined(CHIP_SUBSYS_SENS) && defined(SENS_TRC_TO_MCU)

#else
    trace_enable_early();
#endif
    hal_trace_set_log_level(TR_LEVEL_VERBOSE);
    hal_trace_enable_log_module(0);
    hal_sleep_start_stats(15000, 15000);

    is_chip_init_early_done = true;
#if defined(CONFIG_ARCH_TRUSTZONE_SECURE)
    hal_sec_init();
    int ret = mpc_init();
    ASSERT(ret==0, "mpc init fail. ret:%d", ret);
    TZ_SAU_Setup();
    __ISB();
#endif
#if (defined(PSRAM_ENABLE) || defined(PSRAMUHS_ENABLE)) && defined(PSRAM_INIT_CHECK)
    hal_wdt_stop(HAL_WDT_ID_0);
#endif
    //TRACE(0, "bes_chip_init_early done:%d",is_chip_init_early_done);
}

EXTERN_C void nuttx_norflash_get_size(void);
EXTERN_C void nuttx_norflash_sec_get_size(void);
EXTERN_C int factory_section_open(void);
EXTERN_C void factory_section_init(void);
EXTERN_C bool nvrec_dev_data_open(void);
EXTERN_C bool is_wifi_test_inprocess;

EXTERN_C void bes_chip_flash_init()
{
    TRACE(0, "bes_chip_flash_init");
#if defined(CONFIG_MTD)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);

    // Software will load the factory data and user data from the bottom TWO sectors from the flash,
    // the FLASH_SIZE defined is the common.mk must be equal or greater than the actual chip flash size,
    // otherwise the ota will load the wrong information
    uint32_t actualFlashSize = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0);
    if (FLASH_SIZE > actualFlashSize)
    {
        TRACE_IMM(0,"Wrong FLASH_SIZE defined in target.mk!");
        TRACE_IMM(2,"FLASH_SIZE is defined as 0x%x while the actual chip flash size is 0x%lx!", FLASH_SIZE, actualFlashSize);
        TRACE_IMM(1,"Please change the FLASH_SIZE in common.mk to 0x%lx to enable the OTA feature.", actualFlashSize);
        ASSERT(false, " ");
    }

    norflash_api_init();
    nuttx_norflash_get_size();
#if defined(CONFIG_BES_SECOND_FLASH)
    hal_norflash_init(HAL_FLASH_ID_1);
    nuttx_norflash_sec_get_size();
#endif
#elif defined(CONFIG_USE_BES_BT_STACK)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);

    // Software will load the factory data and user data from the bottom TWO sectors from the flash,
    // the FLASH_SIZE defined is the common.mk must be equal or greater than the actual chip flash size,
    // otherwise the ota will load the wrong information
    uint32_t actualFlashSize = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0);
    if (FLASH_SIZE > actualFlashSize)
    {
        TRACE_IMM(0,"Wrong FLASH_SIZE defined in target.mk!");
        TRACE_IMM(2,"FLASH_SIZE is defined as 0x%x while the actual chip flash size is 0x%lx!", FLASH_SIZE, actualFlashSize);
        TRACE_IMM(1,"Please change the FLASH_SIZE in common.mk to 0x%lx to enable the OTA feature.", actualFlashSize);
        ASSERT(false, " ");
    }
    norflash_api_init();
    factory_section_open();
#endif
}

EXTERN_C void bes_chip_init_later()
{
#if defined(CHIP_SUBSYS_SENS) && defined(SENS_TRC_TO_MCU)
        extern void sens_trace_to_mcu();
        sens_trace_to_mcu();
#ifndef CONFIG_BES_LPUART
        hal_trace_open(HAL_TRACE_TRANSPORT_NULL);
#endif
#endif
    TRACE(0, "bes_chip_init_later");
    //random_open();
    srand(hal_sys_timer_get());
#if !defined(SUBSYS_FLASH_BOOT)
    hal_iomux_ispi_access_init();
#endif
    bes_chip_flash_init();
#if defined(CONFIG_A7_DSP_ENABLE) && defined(CONFIG_A7_DSP_BOOT_EARLY) && !defined(TX_PULLING_CAL)
    if (!is_wifi_test_inprocess)
        a7_dsp_boot();
#endif
}

EXTERN_C void bes_chip_bt_drv_init()
{
#if !defined(CONFIG_USE_BES_BT_STACK) && (defined(CONFIG_BT) || defined(CONFIG_LIB_FLUORIDE) || defined(CONFIG_BTSAK) || defined(CONFIG_LIB_BLUELET))
    factory_section_init();
    nvrec_dev_data_open();
    factory_section_open();

    struct ether_addr
    {
      uint8_t ether_addr_octet[6];
    };
    extern int property_get(const char* key, char* value, const char* default_value);
    extern struct ether_addr *ether_aton_r(const char *asc, struct ether_addr *addr);
    extern void syslog(int priority, const char *fmt, ...);
    unsigned char tmp;
    int i;
#ifdef CONFIG_KVDB
    char value[PROP_NAME_MAX];
    struct ether_addr addr;
    int ret = property_get(PROP_BDMAC, value, NULL);
    if (ret > 0 && ether_aton_r(value, &addr) != NULL) {
      memcpy(btdrv_bdaddr, addr.ether_addr_octet, sizeof(btdrv_bdaddr));
      memcpy(&btdrv_btaddr[1], addr.ether_addr_octet, sizeof(btdrv_btaddr) - 1);
    }
    else
#endif
    {
        memcpy(btdrv_bdaddr, bt_global_addr, sizeof(btdrv_bdaddr));
        memcpy(&btdrv_btaddr[1], ble_global_addr, sizeof(btdrv_btaddr) - 1);
    }
    syslog(1, "BT  Address : [ %02x:%02x:%02x:%02x:%02x:%02x ]\n",
        btdrv_bdaddr[0], btdrv_bdaddr[1], btdrv_bdaddr[2],
        btdrv_bdaddr[3], btdrv_bdaddr[4], btdrv_bdaddr[5]);

    /* Reverse bt address */

    for (i = 0; i < 3; i++) {
      tmp = btdrv_bdaddr[i];
      btdrv_bdaddr[i] = btdrv_bdaddr[6 - i - 1];
      btdrv_bdaddr[6 - i - 1] = tmp;
    }

    syslog(1, "BLE Address : [ %02x:%02x:%02x:%02x:%02x:%02x ]\n",
        btdrv_btaddr[1], btdrv_btaddr[2], btdrv_btaddr[3],
        btdrv_btaddr[4], btdrv_btaddr[5], btdrv_btaddr[6]);

    /* Reverse ble address */

    for (i = 0; i < 3; i++) {
      tmp = btdrv_btaddr[i + 1];
      btdrv_btaddr[i + 1] = btdrv_btaddr[7 - i - 1];
      btdrv_btaddr[7 - i - 1] = tmp;
    }
#endif

    syslog(1, "BT  Address : [ %02x:%02x:%02x:%02x:%02x:%02x ]\n",
        btdrv_bdaddr[0], btdrv_bdaddr[1], btdrv_bdaddr[2],
        btdrv_bdaddr[3], btdrv_bdaddr[4], btdrv_bdaddr[5]);

    /* Reverse bt address */

    for (i = 0; i < 3; i++) {
      tmp = btdrv_bdaddr[i];
      btdrv_bdaddr[i] = btdrv_bdaddr[6 - i - 1];
      btdrv_bdaddr[6 - i - 1] = tmp;
    }

    syslog(1, "BLE Address : [ %02x:%02x:%02x:%02x:%02x:%02x ]\n",
        btdrv_btaddr[1], btdrv_btaddr[2], btdrv_btaddr[3],
        btdrv_btaddr[4], btdrv_btaddr[5], btdrv_btaddr[6]);

    /* Reverse ble address */

    for (i = 0; i < 3; i++) {
      tmp = btdrv_btaddr[i + 1];
      btdrv_btaddr[i + 1] = btdrv_btaddr[7 - i - 1];
      btdrv_btaddr[7 - i - 1] = tmp;
    }

#if !defined(__BOOT_SELECT)
    //nvrec_dev_data_open();
    btdrv_start_bt();
#endif
    btdrv_init_ok = true;

#elif !defined(CONFIG_USE_BES_BT_STACK)
#if defined(CONFIG_BES1502X_AP)
    btdrv_start_bt();
#endif
    btdrv_init_ok = true;
#endif
}

EXTERN_C int hal_uart_printf_init(void);
EXTERN_C int hal_uart_printf(const char *fmt, ...);
EXTERN_C int bes_wl_initialize(void);
EXTERN_C int brcm_wifi_init(void);
EXTERN_C void bes_userspace(void);

extern volatile int trace_enable_flag;
#if defined(CONFIG_ARCH_TRUSTZONE_SECURE) || defined(CONFIG_ARCH_TRUSTZONE_NONSECURE)
static int cmse_trace_global_tag_handler(char *buf, unsigned int buf_len)
{
    unsigned int len;
#if defined(CONFIG_ARCH_TRUSTZONE_SECURE)
    const char tag[] = "SE/";
#else
    const char tag[] = "NS/";
#endif
    const unsigned int tag_len = sizeof(tag) - 1;

    if (buf_len) {
        len = (buf_len <= tag_len) ? buf_len : tag_len;
        memcpy(buf, tag, len);
        return len;
    }

    return 0;
}
#endif

EXTERN_C int bes_global_shmem_setup_syslog_buffer(void);
EXTERN_C void BootSelect(void);
EXTERN_C void main_bl_enter();

EXTERN_C int _start(void)
{
#ifndef CONFIG_ARMV8M_LAZYFPU
    volatile double testfpu = 0.0;
    volatile double test1= 2.0;
    volatile double test2 = 3.0;
    testfpu = test1*test2;
#endif

#ifdef CONFIG_BES_LPUART
    hal_uart_printf_init();
    hal_uart_printf("system power on....");
#endif

#ifdef CONFIG_JTAG_ENABLE
    hal_iomux_set_jtag();
    hal_cmu_jtag_clock_enable();
    //Jtag_hook();
//#ifdef JTAG_DSP
//    hal_cmu_jtag_select_sys_dsp();
//#endif
#endif

#if defined(CONFIG_SYSLOG_RPMSG)
    bes_global_shmem_setup_syslog_buffer();
#endif

#if defined(CONFIG_BOARD_CRASHDUMP)
    extern void nuttx_showtasks(void);
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_SYS,nuttx_showtasks);
#endif

#ifdef CONFIG_BES_LPUART
#if defined(CONFIG_ARCH_TRUSTZONE_SECURE) || defined(CONFIG_ARCH_TRUSTZONE_NONSECURE)
    hal_trace_global_tag_register(cmse_trace_global_tag_handler);
#endif
#endif

#if defined(CONFIG_WITHOUT_OS)
    trace_enable_early();
    hal_trace_set_log_level(TR_LEVEL_VERBOSE);
    main_bl_enter();
#else
#if defined(__BOOT_SELECT)
#ifndef BOOT_LOADER_ENTRY_HOOK
    BootSelect();
#endif
#endif

#ifdef CONFIG_SCHED_BACKTRACE
    extern void hal_trace_init_program_regions(void);
    hal_trace_init_program_regions();
#endif

#ifdef CONFIG_BUILD_PROTECTED
    bes_userspace();
#endif
    nx_start();
#endif
    return 0;
}

#if !defined(CONFIG_WITHOUT_OS)

EXTERN_C void app_trace_rx_open(void);
EXTERN_C void cp_boot(void);
EXTERN_C uint32_t af_open(void);

#if !defined(CONFIG_BES_HAVE_DVFS)
EXTERN_C void nx_sysfreq_idle(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
}

#if defined(CONFIG_ARCH_CHIP_BES1600)
EXTERN_C void nx_sysfreq_boost(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
}
#else
EXTERN_C void nx_sysfreq_boost(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
}
#endif
#endif

EXTERN_C int bwifi_set_ps_mode(int ps);
EXTERN_C int bwifi_set_low_power(int lowpower);

#ifdef __WIFI_APP_SUPPORT__
#include "bwifi_interface.h"
EXTERN_C void btdrv_reg_op_config_coex_mode(uint8_t mode);
/* config wifi/bt coex mode
@mode: coex mode, 0 - tdd, 1 - fdd
*/
static void POSSIBLY_UNUSED config_coex_mode(uint8_t mode)
{
    TRACE(0, "config_coex_mode %d", mode);
    /* set bt traffic ignore pta, should be set after bt drv started */
    btdrv_reg_op_config_coex_mode(mode);
}

static void POSSIBLY_UNUSED coex_mode_evt_handler(WIFI_USER_EVT_ID evt_id, void *arg)
{
    if (evt_id == WIFI_USER_EVT_COEX_MODE_CHANGE) {
        if (arg) {
            uint8_t mode = *(uint8_t *)arg;
            config_coex_mode(mode);
        }
    }
}
#endif

void static freq_check(HAL_CMU_FREQ_T init_freq)
{
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    //TRACE(1,"sys freq calc 208: %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_104M);
    //TRACE(1,"sys freq calc 104: %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_52M);
    //TRACE(1,"sys freq calc 52: %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_26M);
    //TRACE(1,"sys freq calc 26: %d\n", hal_sys_timer_calc_cpu_freq(5, 0));
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    //hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
}


static osThreadId_t main_thread_tid = NULL;
EXTERN_C int bes_main(void)
{
    uint8_t sys_case = 0;
    HAL_CMU_FREQ_T init_freq = HAL_CMU_FREQ_52M;
    main_thread_tid = osThreadGetId();
    int ret = 0;

#if !defined(CONFIG_BES_HAVE_DVFS)
#if defined(CONFIG_ARCH_CHIP_BES1600) || defined(CONFIG_BES1502X_AP)
    init_freq = HAL_CMU_FREQ_52M;
#elif defined(CONFIG_ARCH_CHIP_BES1600_BTH)
    init_freq = HAL_CMU_FREQ_208M;
#elif defined(CONFIG_ARCH_CHIP_BES2003)
    init_freq = HAL_CMU_FREQ_390M;
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, init_freq);
    TRACE(1,"sys freq calc : %ld\n", hal_sys_timer_calc_cpu_freq(5, 0));
#endif

#if defined(CONFIG_USE_BES_BT_STACK)
    ret = app_init();
#if !defined(CONFIG_BES_HAVE_DVFS)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, init_freq);
#endif
#else
#if defined(CONFIG_AUDIO) && !defined(CONFIG_AUDIO_RPMSG_CLIENT)
    af_open();
#endif
#endif

#if defined(CONFIG_A7_DSP_ENABLE) && defined(CONFIG_A7_DSP_BOOT_EARLY) && defined(TX_PULLING_CAL)
    if (!is_wifi_test_inprocess)
        a7_dsp_boot();
#endif

#if defined(__WIFI_APP_SUPPORT__)
#if __COEX_MODE_FDD__
    config_coex_mode(1);
#endif
#if !__COEX_MODE_FDD__ || __COEX_MODE_HYBRID__
    /* COEX code: register it in tdd mode, 5G force FDD mode */
    bwifi_reg_user_evt_handler(WIFI_USER_EVT_COEX_MODE_CHANGE, coex_mode_evt_handler);
#endif
    void net_system_init(void);
    net_system_init();
#endif
    wifidrv_init_ok = true;

#if defined(CONFIG_BES_AUDIO_DUMP)
    app_trace_rx_open();
#endif
#if defined(CONFIG_MIWEAR_BRCM_WIFI)
    brcm_wifi_init();
#endif
#if defined(__WIFI_APP_SUPPORT__) || defined(CONFIG_IEEE80211_BESTECHNIC_NETDEV_SDIO)
    if (!is_wifi_test_inprocess)
        bes_wl_initialize();
#endif
#if defined(CONFIG_USE_BES_BT_STACK)
#if defined(USE_BT_ADAPTER)
    extern "C" void bes_bt_init(void);
    bes_bt_init();
#endif

    btdrv_init_ok = true;
#endif
#if defined(CONFIG_BES_HAVE_DVFS)
#if defined(__WIFI_APP_SUPPORT__)
    osDelay(10);
    bwifi_set_ps_mode(1);
    bwifi_set_low_power(1);
    app_sysfreq_req(APP_SYSFREQ_USER_WIFI, APP_SYSFREQ_32K);
#endif
    app_sysfreq_req(APP_SYSFREQ_USER_APP_INIT, APP_SYSFREQ_32K);
#endif
    if (!ret)
    {
        while(1)
        {
            osEvent evt;
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
            osThreadFlagsClear(0x0f);
#endif
            uint32_t flags;

            flags = osThreadFlagsWait(CMSIS_V1_SIGNAL_ANY, osFlagsWaitAny, osWaitForever);

            if ((flags > 0U) && (flags < 0x80000000U)) {
                if (flags & 0x04) {
                    sys_case = 1;
                    break;
                } else if (flags & 0x08) {
                    sys_case = 2;
                    break;
                }
            } else {
                sys_case = 1;
                break;
            }
         }
    }
#if defined(CONFIG_USE_BES_BT_STACK)// || defined(__WIFI_APP_SUPPORT__)
    system_shutdown_wdt_config(10);
    app_deinit(ret);
#endif
    TR_INFO(TR_MOD(MAIN), "byebye~~~ %d\n", sys_case);
#if !defined(CHIP_SUBSYS_SENS)
    if ((sys_case == 1)||(sys_case == 0)){
        TR_INFO(TR_MOD(MAIN), "shutdown\n");
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pmu_shutdown();
    }else if (sys_case == 2){
        TR_INFO(TR_MOD(MAIN), "reset\n");
        pmu_reboot();
    }
#endif
    return ret;
}

static uint64_t os_thread_def_stack_bes_main [ROUND_UP(4096, 8) / sizeof(uint64_t)];
static const osThreadAttr_t ThreadAttr_bes_main = {
    .name = "bes_main",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_mem = os_thread_def_stack_bes_main,
    .stack_size = ROUND_UP(4096, 8),
    .priority = osPriorityAboveNormal,
    .tz_module = 1U,                  // indicate calls to secure mode
    .reserved = 0U,
};

EXTERN_C __attribute__((weak)) void bes_app_initialize()
{
    bes_chip_init_early();
    bes_chip_init_later();
    bes_chip_bt_drv_init();
    osThreadNew(bes_main, NULL, &ThreadAttr_bes_main);
}

EXTERN_C int system_shutdown(void)
{
    TR_INFO(TR_MOD(MAIN), "system_shutdown!!");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    while (1) {
        TRACE(0, "system_shutdown");
        osDelay(1000);
    }
    return 0;
}

EXTERN_C int system_reset(void)
{
    TR_INFO(TR_MOD(MAIN), "system_reset");
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_clear(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    pmu_reboot();
    while (1) {
        hal_sys_timer_delay(MS_TO_TICKS(1000));
    }
    return 0;
}

EXTERN_C int signal_send_to_main_thread(uint32_t signals)
{
    osThreadFlagsSet(main_thread_tid, signals);
    return 0;
}

EXTERN_C int system_sw_shutdown(int status)
{
    TR_INFO(TR_MOD(MAIN), "system_sw_shutdown status=%d", status);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    switch (status) {
        case 0:
            reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_LOW_BATT_MODE);
            reboot_param_set(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_POWEROFF_MODE);
            pmu_reboot();
            break;
        case 1:
            reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_POWEROFF_MODE);
            reboot_param_set(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_LOW_BATT_MODE);
            pmu_reboot();
            break;
        default:
            break;
    }
    system_shutdown();
    return 0;
}
#if !defined(CONFIG_USE_BES_BT_STACK)
EXTERN_C void sys_otaMode_enter()
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    pmu_reboot();
}
#endif

EXTERN_C void sys_clearMode_enter(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    reboot_param_set(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_FACTORY_RESET_NORMAL);
    pmu_reboot();
}

EXTERN_C void sys_inquiryClearMode_enter(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    reboot_param_clear(REBOOT_PARAM_ID_BOOTMODE, 0xffffffff);
    reboot_param_set(REBOOT_PARAM_ID_BOOTMODE, REBOOT_PARAM_BOOTMODE_FACTORY_RESET_INQUIRY);
    pmu_reboot();
}
#endif

#if __BOOT_SELECT
typedef void (*FLASH_ENTRY)(void);
#define BOOT_MAGIC_NUMBER       0xBE57EC1C
#define NORMAL_BOOT             BOOT_MAGIC_NUMBER
#define COPY_NEW_IMAGE          0x5a5a5a5a
EXTERN_C void NVIC_DisableAllIRQs(void);

#define EXEC_CODE_BASE (FLASH_NC_BASE+OTA_CODE_OFFSET)

#ifdef SECURE_BOOT
static const struct secure_boot_struct_t * secure_boot_struct_app =
    (const struct secure_boot_struct_t *)EXEC_CODE_BASE;
#else
static const struct boot_struct_t * boot_struct_app =
    (const struct boot_struct_t *)EXEC_CODE_BASE;
#endif

static int app_is_otaMode(void)
{
    uint32_t bootmode = hal_sw_bootmode_get();

    return (bootmode & HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
}

#ifdef SECURE_BOOT
static bool app_is_security_enable(void)
{
    union SECURITY_VALUE_T security;

    pmu_get_security_value(&security);
    return security.root.security_en;
}

#include "rsa.h"
#include "sha256.h"

static const RSAPublicKey pubkey = {
        0xa6d05c9b,
        {
            0xc80dd66d, 0x1bd4d23e, 0x504df850, 0x533c737b,
            0x1e2f49e2, 0xc219c16e, 0x9f995900, 0x2b296aa7,
            0x1421654d, 0x6941f620, 0xd080075e, 0x7815cf32,
            0x423a4a64, 0x0b5cc476, 0x099ad291, 0x1cc99963,
            0xaadc7e6e, 0xd9a2c290, 0xf4a8ea77, 0x2c6951cd,
            0x6435fa06, 0x34cc9d31, 0xa7dc9112, 0x7bfc62f2,
            0xca478382, 0x02047e85, 0xcca2d4aa, 0xe424ac9b,
            0x63ac8a24, 0x83c9fc5a, 0xa05670d7, 0x62571bd1,
            0x4c5a9846, 0x909bee90, 0x575b8125, 0xa27cb715,
            0x159ec64b, 0x3059e7df, 0xb0e2cb7d, 0xf99168a2,
            0xe07603f3, 0x3856d5fe, 0x9d4764c6, 0xc2bfe583,
            0xf61f9e54, 0x58c74197, 0xbe793f44, 0xa107362f,
            0xd33bacbd, 0x04420f1c, 0xadc3e3df, 0xab1cbd82,
            0x8adfa69f, 0xd5cdef46, 0x478f8a73, 0xc5a4b8ff,
            0xfb768575, 0x8443782a, 0x4af64644, 0xf5b4c117,
            0x7211a214, 0xf2a91e63, 0xafffd136, 0x929ad92b,
            0x76adf2a8, 0x04b397cb, 0xc969bd12, 0x981e66e9,
            0x7e76dc33, 0xea876f1d, 0x8d2377de, 0x17892e81,
            0x30a06967, 0x1ee362e4, 0x97de944a, 0x86b89305,
            0x93416053, 0xc5a15ded, 0x855b0960, 0xa112817e,
            0x04eeaa2f, 0x7e6086b0, 0x9b94809a, 0x9eceb981,
            0xb833f021, 0x5dd3b84c, 0x029c5b19, 0xe86b10f7,
            0x9868080d, 0x3d27ba6b, 0x492661b4, 0x019e4159,
            0x2c9db783, 0xeb26d3fe, 0xd914cddd, 0xc232e94e,
        },
        {
            0x39fd219a, 0x8533bd6c, 0x8eb78699, 0x4770b0f7,
            0x5bf08ee4, 0x6b944a5e, 0xcd8e5e49, 0x99a08232,
            0xfcb1a6bf, 0xa35ffc43, 0xd8884a98, 0x03c3a135,
            0x1db99139, 0x37e4d4ef, 0xd87120a7, 0x7fee54d7,
            0x6864d566, 0x63eaa05b, 0x2f58bdb9, 0x067bda59,
            0x45c3da17, 0x7aff8211, 0x0fe87ba5, 0xdbd4a631,
            0x5ff87c83, 0xf6c3057b, 0xb7907ff3, 0xd0b0e0f8,
            0x0c6bc5ed, 0x4d5976d7, 0x54f56456, 0x1bb28630,
            0x38674fda, 0xef4bb73d, 0x88dbe388, 0xe298faf8,
            0xc57904c1, 0x461ecdaa, 0xb698f6a0, 0x7fe4c5f2,
            0x8b230850, 0x25f782c6, 0xe40d9838, 0xe66abb4b,
            0x02aaa1ed, 0x4eae45dc, 0x4bc90876, 0x2f346ecd,
            0xd7c2e233, 0x2ec41934, 0x55c60535, 0xd1e3644b,
            0x88c7426e, 0xbb53773a, 0x2baea13d, 0xeb09032f,
            0x4107cc50, 0x891e2a98, 0x397b73f0, 0x9ad9596f,
            0x38abb7ee, 0x813bbc5e, 0xe8c2c908, 0xdc0b0f48,
            0x656c8c3a, 0xb6484d7b, 0xfac464ac, 0x3c5876cf,
            0x435c8230, 0x135562e1, 0x900ef90e, 0xd32e822f,
            0xc9c47406, 0xebb3586f, 0x39f5f850, 0x8f5eb237,
            0x481f7920, 0xaec60d1a, 0x603586fc, 0x05b60ee9,
            0xe9051d5f, 0xa5ff279e, 0x8cf8f2e2, 0x5314771f,
            0x803a28d8, 0xc7a610b0, 0xb352c6ab, 0x77bcd097,
            0x9dc82227, 0x3eebd068, 0xf45ee047, 0xc6401065,
            0x3613da18, 0xe773339b, 0xc1d90b20, 0x46378571,
        },
};

int verify_tee_image(void)
{
    int ret = 0;
    uint32_t time;
    const RSAPublicKey *key = &pubkey;
    const unsigned char *signature = secure_boot_struct_app->code_sig_struct.sig;
    const unsigned char *message = (const unsigned char *) &(secure_boot_struct_app->norflash_cfg);
    const uint32_t mlen = secure_boot_struct_app->code_sig_struct.code_size;

    TRACE(0, "norflash_cfg %p", message);
    TRACE(0, "signature %p", signature);
    TRACE(0, "code_size %ld", mlen);
    TRACE(0, "signature size %d", MAX_SIG_LEN);

    time = hal_fast_sys_timer_get();

    ret = RSA_verify(0, key, signature, MAX_SIG_LEN, message, mlen);
    if (ret != 1)
        TRACE(0, "RSA verify fail");

    TRACE(0, "RSA verify done. msg_len:%ld, time:%ld us", mlen, FAST_TICKS_TO_US(hal_fast_sys_timer_get()-time));
    return ret;
}
#endif

#ifdef SECURE_BOOT
    if (app_is_security_enable())
    {
        if (verify_tee_image() != 1)
            return;
        TRACE_IMM(0, "security enabled, verify tee image successfully");
    }
#endif

#ifdef SECURE_BOOT
    if ((NORMAL_BOOT == secure_boot_struct_app->boot_struct.hdr.magic))
#else
    if ((NORMAL_BOOT == boot_struct_app->hdr.magic))
#endif
    {
#ifndef BOOT_LOADER_ENTRY_HOOK
        hal_trace_close();
#endif
        // Disable all IRQs
        NVIC_DisableAllIRQs();
        // Ensure in thumb state
#ifdef SECURE_BOOT
        entry = (FLASH_ENTRY)FLASH_TO_FLASHX(FLASH_NC_TO_C((uint32_t)(&secure_boot_struct_app->code_ver_struct + 1)));
#else
        entry = (FLASH_ENTRY)FLASH_TO_FLASHX(FLASH_NC_TO_C((uint32_t)(&boot_struct_app->hdr + 1)));
#endif
        entry = (FLASH_ENTRY)((uint32_t)entry | 1);
        entry();
    }
}
#endif

#ifdef BOOT_LOADER_ENTRY_HOOK
EXTERN_C void boot_loader_entry_hook(void) __attribute__((alias("BootSelect")));
#endif

#endif

#if defined(CONFIG_ARCH_TRUSTZONE_SECURE)
#define NS_CALL                             __attribute__((cmse_nonsecure_call))
/* typedef for non-secure callback functions */
typedef void (*funcptr_void) (void) NS_CALL;

EXTERN_C void bes_call_nonse(void)
{
    funcptr_void NonSecure_ResetHandler;
    uint32_t ns_app_start_addr = (FLASHX_BASE + NS_APP_START_OFFSET);

    /* Add user setup code for secure part here*/

    /* Set non-secure main stack (MSP_NS) */
    //__TZ_set_MSP_NS(*((uint32_t *)(ns_app_start_addr)));

    /* Get non-secure reset handler */
#ifdef SECURE_BOOT
    NonSecure_ResetHandler = (funcptr_void)((uint32_t)ns_app_start_addr + sizeof(struct boot_struct_t) + sizeof(struct code_sig_struct_t) +
                                            sizeof(struct norflash_cfg_struct_t) + sizeof(struct code_ver_struct_t));
#else
    NonSecure_ResetHandler = (funcptr_void)((uint32_t)(&((struct boot_struct_t *)ns_app_start_addr)->hdr + 1));
#endif
    if (0){//*(uint32_t *)ns_app_start_addr != BOOT_MAGIC_NUMBER) {
        TRACE(0, "nonsec image(0x%08lx) magic(0x%08lx) error", ns_app_start_addr, *(uint32_t *)ns_app_start_addr);
    } else {
        TRACE(0, "Call nonsec App. start addr:0x%08lx", ns_app_start_addr);
        /* Start non-secure state software application */
#if 0//def DEBUG
        cmse_set_ns_start_flag(true);
#endif
        NonSecure_ResetHandler();
    }
}
#endif

BOOT_DATA_LOC volatile uint8_t jtag_test = 1;
EXTERN_C void Jtag_hook(void)
{
    while (jtag_test != 123);
}
