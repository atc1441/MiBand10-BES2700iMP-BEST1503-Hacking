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
#include "plat_addr_map.h"
#include "analog.h"
#include "apps.h"
#include "app_bt_stream.h"
#include "app_trace_rx.h"
#include "cmsis.h"
#include "hal_bootmode.h"
#include "hal_cmu.h"
#include "hal_dma.h"
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
#include "stdlib.h"
#include "tgt_hardware.h"
#include "app_utils.h"
#include "watchdog/watchdog.h"
#include "hal_evr.h"
#include "factory_section.h"

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
#include "bth_rom_init.h"
#endif

#include "string.h"

#if defined(ARM_CMNS)
extern "C" {
#include "tz_trace_ns.h"
#include "tz_interface.h"
}
#endif

#if defined(SPA_AUDIO_SEC)
#include "parser.h"
#endif

extern "C" void log_dump_init(void);
extern "C" void crash_dump_init(void);
#if defined(DISPLAY_RM690B0)
extern "C" int up_fbinitialize(int display);   /* RM690B0 display bring-up */
#endif
#if defined(HW_SELFTEST)
extern "C" void hw_selftest(void);             /* on-SoC hardware self-test */
#endif
#ifdef USER_SECURE_BOOT
extern "C" int user_secure_boot_check(void);
extern "C" void security_boot_jump_to_next_entry(void);
#endif

#ifdef FIRMWARE_REV
#define SYS_STORE_FW_VER(x) \
      if(fw_rev_##x) { \
        *fw_rev_##x = fw.softwareRevByte##x; \
      }

typedef struct
{
    uint8_t softwareRevByte0;
    uint8_t softwareRevByte1;
    uint8_t softwareRevByte2;
    uint8_t softwareRevByte3;
} FIRMWARE_REV_INFO_T;

static FIRMWARE_REV_INFO_T fwRevInfoInFlash __attribute((section(".fw_rev"))) = {0, 0, 1, 0};
FIRMWARE_REV_INFO_T fwRevInfoInRam;

extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1,
    uint8_t *fw_rev_2, uint8_t *fw_rev_3)
{
  FIRMWARE_REV_INFO_T fw = fwRevInfoInFlash;

  SYS_STORE_FW_VER(0);
  SYS_STORE_FW_VER(1);
  SYS_STORE_FW_VER(2);
  SYS_STORE_FW_VER(3);
}
#endif

#if defined(_AUTO_TEST_)
static uint8_t fwversion[4] = {0,0,1,0};

void system_get_fwversion(uint8_t *fw_rev_0, uint8_t *fw_rev_1,
    uint8_t *fw_rev_2, uint8_t *fw_rev_3)
{
    *fw_rev_0 = fwversion[0];
    *fw_rev_1 = fwversion[1];
    *fw_rev_2 = fwversion[2];
    *fw_rev_3 = fwversion[3];
}
#endif

static osThreadId main_thread_tid = NULL;

extern "C" int system_shutdown(void)
{
    TR_INFO(TR_MOD(MAIN), "system_shutdown!!");
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    return 0;
}

extern "C" int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x8);
    return 0;
}

extern "C" int signal_send_to_main_thread(uint32_t signals)
{
    osSignalSet(main_thread_tid, signals);
    return 0;
}

int tgt_hardware_setup(void)
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

    if (app_battery_ext_charger_indicator_cfg.pin != HAL_IOMUX_PIN_NUM){
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&app_battery_ext_charger_indicator_cfg, 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)app_battery_ext_charger_indicator_cfg.pin, HAL_GPIO_DIR_IN, 1);
    }
    return 0;
}

#if defined(ROM_UTILS_ON)
void rom_utils_init(void);
#endif

#ifdef FPGA
uint32_t a2dp_audio_more_data(uint8_t *buf, uint32_t len);
uint32_t a2dp_audio_init(void);
extern "C" void app_audio_manager_open(void);
extern "C" uint32_t hal_iomux_init(const struct HAL_IOMUX_PIN_FUNCTION_MAP *map, uint32_t count);
void app_overlay_open(void);

extern "C" int app_os_init(void);
extern "C" uint32_t af_open(void);
extern "C" int list_init(void);
extern "C" void app_audio_open(void);


volatile uint32_t ddddd = 0;

#if defined(AAC_TEST)
#include "app_overlay.h"
int decode_aac_frame_test(unsigned char *pcm_buffer, unsigned int pcm_len);
#define AAC_TEST_PCM_BUFF_LEN (4096)
unsigned char aac_test_pcm_buff[AAC_TEST_PCM_BUFF_LEN];
#endif

#endif

#if defined(_AUTO_TEST_)
extern int32_t at_Init(void);
#endif

#ifdef DEBUG_MODE_USB_DOWNLOAD
static void process_usb_download_mode(void)
{
    if (pmu_charger_get_status() == PMU_CHARGER_PLUGIN && hal_pwrkey_pressed()) {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_FORCE_USB_DLD);
        pmu_reboot();
    }
}
#endif

#if RTT_APP_SUPPORT
extern "C" void platform_console_init(void)
{
    hwtimer_init();
    hal_audma_open();
    hal_gpdma_open();

#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    {
        hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
    }
#endif

#if (DEBUG_PORT == 2)
    {
        hal_iomux_set_analog_i2c();
    }
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif
#endif
}
#endif

WEAK void hw_revision_checker(void)
{

}

void app_secure_nse_init(void)
{
    TRACE(1,"%s line:%d",__func__,__LINE__);
    uint32_t s_time;
    uint32_t e_time;

    s_time = hal_fast_sys_timer_get();

    //TO DO
    for(uint32_t i = 0;i<2000;i++){
        __NOP();
    }

    e_time = hal_fast_sys_timer_get();

    TRACE(1,"time cost %d",FAST_TICKS_TO_US(e_time - s_time));
}

#if defined(ARM_CMNS)
static int tz_flash_read(void)
{
    enum HAL_NORFLASH_RET_T ret;
    uint32_t magic_num;
    ret = hal_norflash_read(HAL_FLASH_ID_0, FLASH_BASE, (uint8_t *)(&magic_num), sizeof(magic_num));
    if (ret != HAL_NORFLASH_OK) {
        TRACE(0, "FLASH read error:%d", ret);
        return -1;
    }

    TRACE(0, "FLASH read success. magic_num:0x%x", magic_num);
    return 0;
}


void boot_secure_nse_boot_init(void)
{
    int ret;
    char str[30];

    cmns_trace_init();

    TRACE(0, "%s, cmns_trace_init done", __func__);

    // Let idle task run
    osDelay(10);

    memset(str, 0, sizeof(str));

    ret = get_demo_string(str, sizeof(str));
    TRACE(0, "cmse demo string(ret:%d):%s", ret, str);

    tz_flash_read();

}
#endif

/* ---- best1503 bring-up: standalone boot checkpoint logging over a raw PL011
 * poll-TX HW-UART, no SDK deps, so it works even before hal_trace_open() and
 * survives a stuck chip-specific init.
 *
 * Which UART/pin is selected by DEBUG_PORT -- the SAME selector the SDK trace
 * uses (config/best1503/target.mk), so early checkpoints and hal_trace land on
 * the same wire:
 *     DEBUG_PORT == 2 -> UART1, TXD on P2_1 = GPIO17
 *     otherwise       -> UART0, TXD on P2_3 = GPIO19
 *
 * On best1503 the UART0_TX pad is silicon-fixed to P2_3/GPIO19 (it is the only
 * pin carrying UART0_TX in pin_func_map); only UART1_TX can reach P2_1/GPIO17.
 * The boot ROM only brings up UART0, so for the UART1 case we *clone* UART0's
 * bootrom-configured clock source and baud/line registers onto UART1 -- that
 * way we need no UARTCLK constant and inherit the exact working 8N1 baud. */
#define DBG_IOMUX_BASE 0x40086000u
#define DBG_CMU_BASE   0x40000000u
#define DBG_UART0_BASE 0x4000B000u
#define DBG_UART1_BASE 0x4000C000u
#if (DEBUG_PORT == 2)
#define DBG_UART_BASE  DBG_UART1_BASE   /* P2_1 = GPIO17 */
#else
#define DBG_UART_BASE  DBG_UART0_BASE   /* P2_3 = GPIO19 */
#endif
static void dbg_boot_init(void)
{
    volatile unsigned int *io  = (volatile unsigned int *)DBG_IOMUX_BASE;
#if (DEBUG_PORT == 2)
    volatile unsigned int *cmu = (volatile unsigned int *)DBG_CMU_BASE;
    volatile unsigned int *u0  = (volatile unsigned int *)DBG_UART0_BASE;
    volatile unsigned int *u1  = (volatile unsigned int *)DBG_UART1_BASE;

    /* 1. Clone UART0's clock cfg (div/sel/en = bits[4:0]) into UART1's field
     *    (bits[9:5]) of CMU UART_CLK @0x70 -> identical UARTCLK for UART1. */
    {
        unsigned int uc = cmu[0x070/4];
        cmu[0x070/4] = (uc & ~0x3E0u) | ((uc & 0x1Fu) << 5);
    }
    /* 2. Enable UART1 PCLK (bit 11) and OCLK (bit 13); both are set-only regs
     *    (PCLK_ENABLE @0x08, OCLK_ENABLE @0x10). */
    cmu[0x008/4] = (1u << 11);
    cmu[0x010/4] = (1u << 13);

    /* 3. Mux P2_1 (GPIO17) -> UART1_TX. REG_00C is the P2 group func-select,
     *    4 bits/pin; P2_1 occupies bits[7:4]. best1503 uses func value 2 for
     *    the MCU UART alt-func (same +1 offset as UART0, NOT the SDK table's 1
     *    -- see tests/programmer_inflash/main.c). */
    io[0x00C/4] = (io[0x00C/4] & ~0x00F0u) | 0x0020u;
    io[0x02C/4] &= ~(1u<<17);                         /* no pull on TXD (GPIO17) */
    io[0x030/4] &= ~(1u<<17);                         /* clear pull-down         */

    /* 4. Clone UART0's working baud + line control onto UART1 (UARTEN still 0,
     *    so IBRD/FBRD/LCR_H may be programmed; LCR_H written last). */
    u1[0x024/4] = u0[0x024/4];                        /* IBRD                    */
    u1[0x028/4] = u0[0x028/4];                        /* FBRD                    */
    u1[0x04C/4] = u0[0x04C/4];                        /* OVSAMP (BES oversample) */
    u1[0x02C/4] = u0[0x02C/4];                        /* LCR_H (8N1 + FIFO)      */

    /* 5. Enable UART1. */
    u1[0x030/4] |= (1u<<0)|(1u<<8);                   /* UARTCR: UARTEN | TXE    */
#else
    /* UART0 / P2_3 (GPIO19): boot ROM left it configured, just re-assert the
     * func-2 mux + pulls and enable TX (same trick as tests/programmer_inflash). */
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART0_BASE;
    io[0x050/4] |= 3u;
    io[0x00C/4] = (io[0x00C/4] & ~0xFF00u) | 0x2200u; /* P2_2/P2_3 -> func 2 (UART0) */
    io[0x02C/4] |= (1u<<18); io[0x02C/4] &= ~(1u<<19);
    io[0x030/4] &= ~((1u<<18)|(1u<<19));
    u[0x030/4] |= (1u<<0)|(1u<<8);                    /* UARTCR: UARTEN | TXE    */
#endif
}
static void dbg_putc(char c)
{
    volatile unsigned int *u = (volatile unsigned int *)DBG_UART_BASE;
    unsigned int g = 0;
    while ((u[0x018/4] & (1u<<5)) && ++g < 0x100000u) {}  /* wait TXFF clear */
    u[0] = (unsigned int)(unsigned char)c;
}
static void dbg_puts(const char *s) { while (*s) { if (*s=='\n') dbg_putc('\r'); dbg_putc(*s++); } }
#define DBG_CP(msg) dbg_puts("[CP] " msg "\n")
extern "C" void dbg_cp_n(int);   /* global robust checkpoint (platform/main/dbg_boot_cp.c) */

#if RTT_APP_SUPPORT || defined(CHIP_SUBSYS_SENS)
int main_enter(void)
#else
int main(void)
#endif
{
    uint8_t sys_case = 0;
    int ret = 0;

    dbg_cp_n(51);   /* CP'j': main() reached (global robust dbg) */
    dbg_boot_init();
    dbg_puts("\n\n===== best1503 fw boot (best1306-substituted) =====\n");
    DBG_CP("main() entered");

    DBG_CP("pre hw_revision_checker"); hw_revision_checker();
    DBG_CP("pre app_wdt_open");

#if !defined(BLE_ONLY_ENABLED)
    app_wdt_open(15);
#else
    app_wdt_open(30);
#endif

#ifdef __FACTORY_MODE_SUPPORT__
    uint32_t bootmode = hal_sw_bootmode_get();
#endif

#ifdef DEBUG_MODE_USB_DOWNLOAD
    process_usb_download_mode();
#endif

    DBG_CP("pre tgt_hardware_setup"); tgt_hardware_setup();

#if defined(ROM_UTILS_ON)
    DBG_CP("pre rom_utils_init"); rom_utils_init();
#endif
    DBG_CP("post rom_utils_init");

    main_thread_tid = osThreadGetId();

    DBG_CP("pre hwtimer_init"); hwtimer_init();

    hal_dma_set_delay_func((HAL_DMA_DELAY_FUNC)osDelay);
    DBG_CP("pre hal_audma_open"); hal_audma_open();
    hal_gpdma_open();
    DBG_CP("pre norflash_api_init"); norflash_api_init();
#if defined(DUMP_LOG_ENABLE)
    log_dump_init();
#endif
#if (defined(DUMP_CRASH_LOG) || defined(TOTA_CRASH_DUMP_TOOL_ENABLE))
    crash_dump_init();
#endif

#if defined(RAM_DUMP_TO_FLASH)
    ramdump_to_flash_init();
#endif

#ifdef CORE_DUMP_TO_FLASH
    coredump_to_flash_init();
#endif

    app_reset_gpio_status();

#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
#ifdef __FACTORY_MODE_SUPPORT__
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY))
#endif
    {
        hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
    }
#endif

#if (DEBUG_PORT == 2)
#ifdef __FACTORY_MODE_SUPPORT__
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY))
#endif
    {
        hal_iomux_set_analog_i2c();
    }
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif

#if (DEBUG_PORT == 3)
#ifdef __FACTORY_MODE_SUPPORT__
    if (!(bootmode & HAL_SW_BOOTMODE_FACTORY))
#endif
    {
        hal_iomux_set_analog_i2c();
    }
    hal_iomux_set_uart2();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART2);
#endif

    hal_sleep_start_stats(10000, 10000);
    hal_trace_set_log_level(TR_LEVEL_DEBUG);
#endif

    DBG_CP("trace_open done; pre hal_iomux_ispi_access_init"); hal_iomux_ispi_access_init();

    DBG_CP("pre system_power_on_callback"); system_power_on_callback();

#ifndef FPGA
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);

    // Software will load the factory data and user data from the bottom TWO sectors from the flash,
    // the FLASH_SIZE defined is the common.mk must be equal or greater than the actual chip flash size,
    // otherwise the ota will load the wrong information
    uint32_t actualFlashSize = hal_norflash_get_flash_total_size(HAL_FLASH_ID_0);
    if (FLASH_SIZE > actualFlashSize)
    {
        TRACE_IMM(0,"Wrong FLASH_SIZE defined in target.mk!");
        TRACE_IMM(2,"FLASH_SIZE is defined as 0x%x while the actual chip flash size is 0x%x!", FLASH_SIZE, actualFlashSize);
        TRACE_IMM(1,"Please change the FLASH_SIZE in common.mk to 0x%x to enable the OTA feature.", actualFlashSize);
        ASSERT(false, " ");
    }
#endif

    DBG_CP("pre mpu_cfg"); mpu_cfg();
#if !defined(__SUBSYS_ONLY__)
    DBG_CP("pre pmu_open"); pmu_open();
    DBG_CP("pre analog_open"); analog_open();
#endif
    DBG_CP("post analog_open");

#ifdef BTH_IN_ROM
    DBG_CP("pre bth_rom_init"); bth_rom_init();
#endif
    DBG_CP("post bth_rom_init");

#if defined(HW_SELFTEST)
    DBG_CP("pre hw_selftest"); hw_selftest(); DBG_CP("post hw_selftest");
#endif

#if defined(DISPLAY_RM690B0)
    /* best1503/MiBand9 RM690B0 AMOLED bring-up: run the (placeholder) display init.
     * This pulls hal_dsi/hal_lcdc + the RM690B0 panel driver into the image and
     * exercises the init path. With placeholder DSI_BASE/LCDC_BASE/clocks it will
     * NOT light the panel yet (see DISPLAY_BRINGUP.md) — the checkpoints just prove
     * the path is wired and reached. */
    DBG_CP("pre display up_fbinitialize (RM690B0)");
    up_fbinitialize(0);
    DBG_CP("post display up_fbinitialize");
#endif

    srand(hal_sys_timer_get());

#ifdef EVR_TRACING
    hal_evr_init(0);
#endif

#if defined(_AUTO_TEST_)
    at_Init();
#endif
    factory_section_open();

#ifdef FPGA

    TR_INFO(TR_MOD(MAIN), "\n[best of best of best...]\n");
    TR_INFO(TR_MOD(MAIN), "\n[ps: w4 0x%x,2]", &ddddd);

    ddddd = 1;
    while (ddddd == 1);
    TR_INFO(TR_MOD(MAIN), "bt start");

    list_init();

    app_os_init();
#ifndef DOOM
    /* DOOM build: the device has no audio hardware and hw_selftest()->doom never
     * returns, so these audio-init calls are dead. Dropping the references lets
     * --gc-sections strip the whole audio subsystem (codecs, audio process,
     * media player, audioflinger ...) from flash. */
    a2dp_audio_init();

    af_open();
    app_audio_open();
    app_audio_manager_open();
    app_overlay_open();
#endif

#if defined(AAC_TEST)
    app_overlay_select(APP_OVERLAY_A2DP_AAC);
    decode_aac_frame_test(aac_test_pcm_buff, AAC_TEST_PCM_BUFF_LEN);
#endif

    SAFE_PROGRAM_STOP();

#else // !FPGA

#if defined( __FACTORY_MODE_SUPPORT__) && !defined(SLIM_BTC_ONLY)
    if (bootmode & HAL_SW_BOOTMODE_FACTORY){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_FACTORY);
        ret = app_factorymode_init(bootmode);

    }else if(bootmode & HAL_SW_BOOTMODE_CALIB){
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CALIB);
        ret = app_factorymode_calib_only();
    }
#ifdef __USB_COMM__
    else if(bootmode & HAL_SW_BOOTMODE_CDC_COMM)
    {
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_CDC_COMM);
        ret = app_factorymode_cdc_comm();
    }
#endif
    else
#endif
    {
//    demo_bin_aes_crypt_main();
#if defined(ARM_CMNS)
    boot_secure_nse_boot_init();
#endif
#ifdef SPA_AUDIO_SEC
    tz_customer_load_ram_ramx_section_info_t info = {0};
#if defined(ARM_CMNS)
    TRACE(0,"start to jump to trust zone");

    info.flash_addr = (unsigned int )inc_enc_bin_sec_start;
    info.dst_text_sramx_addr_start = (unsigned int )__customer_load_sram_text_start__;
    info.dst_text_sramx_addr_end = (unsigned int )__customer_load_sram_text_end__;
    info.dst_data_sram_addr_start = (unsigned int )__customer_load_ram_data_start__;
    info.dst_data_sram_addr_end = (unsigned int )__customer_load_ram_data_end__;
#endif
    uint32_t lock = int_lock();
    sec_decrypted_section_load_init((void*)&info);
    int_unlock(lock);
#endif

#ifdef FIRMWARE_REV
        fwRevInfoInRam = fwRevInfoInFlash;
        TR_INFO(TR_MOD(MAIN), "The Firmware rev is %d.%d.%d.%d",
        fwRevInfoInRam.softwareRevByte0,
        fwRevInfoInRam.softwareRevByte1,
        fwRevInfoInRam.softwareRevByte2,
        fwRevInfoInRam.softwareRevByte3);
#endif
#ifdef SLIM_BTC_ONLY
        ret = app_init_btc();
#else
        ret = app_init();
#endif

    }
    if (!ret){
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("BT Init ok.");
#endif
        while(1)
        {
            osEvent evt;
#ifndef __POWERKEY_CTRL_ONOFF_ONLY__
            osSignalClear (main_thread_tid, 0x0f);
#endif
            //wait any signal
            evt = osSignalWait(0x0, osWaitForever);

            //get role from signal value
            if(evt.status == osEventSignal)
            {
                if(evt.value.signals & 0x04)
                {
                    sys_case = 1;
                    break;
                }
                else if(evt.value.signals & 0x08)
                {
                    sys_case = 2;
                    break;
                }
            }else{
                sys_case = 1;
                break;
            }
         }
    }
    system_shutdown_wdt_config(10);
#ifdef SLIM_BTC_ONLY
    app_deinit_btc(ret);
#else
    app_deinit(ret);
#endif
    system_power_off_callback(sys_case);
    TR_INFO(TR_MOD(MAIN), "byebye~~~ %d\n", sys_case);
    if ((sys_case == 1)||(sys_case == 0)){
        TR_INFO(TR_MOD(MAIN), "shutdown\n");
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("System shutdown.");
        osDelay(50);
#endif
        hal_sw_bootmode_clear(HAL_SW_BOOTMODE_REBOOT);
        pmu_shutdown();
    }else if (sys_case == 2){
        TR_INFO(TR_MOD(MAIN), "reset\n");
#if defined(_AUTO_TEST_)
        AUTO_TEST_SEND("System reset.");
        osDelay(50);
#endif
        pmu_reboot();
    }

#endif // !FPGA

    return 0;
}

#if RTT_APP_SUPPORT
static void main_thread(void const *argument)
{
    main_enter();
}

osThreadDef(main_thread, osPriorityBelowNormal, 1, 3072, "main_thread");
void board_start_main_thread(void)
{
    osThreadCreate(osThread(main_thread), NULL);
}

int main(void)
{
    board_start_main_thread();
    return 0;
}
#endif

WEAK void app_reset_gpio_status(void)
{

}

WEAK void system_power_on_callback(void)
{

}

WEAK void system_power_off_callback(uint8_t sys_case)
{

}
