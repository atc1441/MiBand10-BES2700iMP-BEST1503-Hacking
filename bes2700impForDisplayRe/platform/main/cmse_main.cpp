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
#include "hal_dma.h"
#include "hal_iomux.h"
#include "hal_norflash.h"
#include "hal_sysfreq.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sec.h"
#include "hal_cmu.h"
#include "hwtimer_list.h"
#include "main_entry.h"
#include "pmu.h"
#ifdef RTOS
#include "rtx_os.h"
#endif
#include "tool_msg.h"
#include "norflash_cfg.h"
#include <arm_cmse.h>
#include "cmse/tz_trace_s.h"
#include "cmse/tz_flash.h"

#include "partition_ARMCM33.h"
#include "mpc.h"

#ifdef MBEDTLS_TEST
#include "hwtest.h"
#endif

#ifdef RTOS
extern "C" void rtx_show_all_threads(void);
#endif
#ifdef USER_SECURE_BOOT
extern  "C" int user_secure_boot_check(void);
#endif

#if defined(SE_OTP_DEMO_TEST)
#include "cmse/tz_otp_handler_demo.h"
#endif

#if defined(SPA_AUDIO_SEC)
#include "parser.h"
extern "C"     void rsa_test(void);
#endif

#define NS_CALL                             __attribute__((cmse_nonsecure_call))

/* typedef for non-secure callback functions */
typedef void (*funcptr_void) (void) NS_CALL;

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.

int MAIN_ENTRY(void)
{
    int POSSIBLY_UNUSED ret;
    uint32_t ns_app_start_addr = (FLASHX_BASE + NS_APP_START_OFFSET);

    hal_audma_open();
    hal_gpdma_open();
#ifdef DEBUG
    enum HAL_TRACE_TRANSPORT_T transport = HAL_TRACE_TRANSPORT_QTY;

    cmse_trace_init();
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    transport = HAL_TRACE_TRANSPORT_UART0;
#else
    hal_iomux_set_analog_i2c();
#if (DEBUG_PORT == 2)
    hal_iomux_set_uart1();
    transport = HAL_TRACE_TRANSPORT_UART1;
#elif (DEBUG_PORT == 3)
    hal_iomux_set_uart2();
    transport = HAL_TRACE_TRANSPORT_UART2;
#elif (DEBUG_PORT == 4)
    hal_iomux_set_uart3();
    transport = HAL_TRACE_TRANSPORT_UART3;
#elif (DEBUG_PORT == 5)
    hal_iomux_set_uart4();
    transport = HAL_TRACE_TRANSPORT_UART4;
#endif
#endif
    hal_trace_open(transport);
#endif

    sec_flash_init(HAL_FLASH_ID_0);

#ifdef RTOS
#if !(defined(ROM_BUILD) || defined(PROGRAMMER))
    ret = hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_SYS, rtx_show_all_threads);
    ASSERT(ret == 0, "IdleTask: Failed to register crash dump callback");
#endif
#endif

#if !defined(SIMU) && !defined(FPGA)
    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);
#endif

#ifndef NO_PMU
    ret = pmu_open();
    ASSERT(ret == 0, "Failed to open pmu");
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_52M);
    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

#ifndef  CMSE_RAM_RAMX_LEND_NSE
    hal_sec_init();

    ret = mpc_init();
    ASSERT(ret==0, "mpc init fail. ret:%d", ret);

    TZ_SAU_Setup();
    __ISB();
#endif


    //hal_page_spy_set(SPY_SRAM0_BASE, 0, 0x20001000, 0x1000, HAL_PAGE_SPY_FLAG_READ);

#ifdef RTOS
    osKernelSuspend();
#endif

#ifdef SE_OTP_DEMO_TEST
    //se_otp_demo_test();
    se_otp_demo_test_bes();
#endif

#ifdef MBEDTLS_TEST
    mbedtls_test();
#endif

#ifdef CMSE_RAM_RAMX_LEND_NSE
    TRACE(0,"CMSE_RAM_RAMX_LEND_NSE start");

    cmse_flash_load_into_sram_if((unsigned int )__cmse_ramx_lend_text_section_start_flash__,(unsigned int )__cmse_ramx_lend_text_section_end_flash__,
                                (unsigned int )__cmse_ramx_lend_text_section_ram_start__,(unsigned int )__cmse_ramx_lend_text_section_ram_end__);
    cmse_flash_load_into_sram_if((unsigned int )__cmse_ram_lend_data_section_start_flash__,(unsigned int )__cmse_ram_ramx_data_text_end_flash__,
                                (unsigned int )__cmse_ram_lend_data_section_exec_start__,(unsigned int )__cmse_ram_lend_data_section_exec_end__);

    TRACE(0,"CMSE_RAM_RAMX_LEND_NSE finished");
//    DUMP8("%02x ",__cmse_ramx_lend_text_section_ram_start__,16);
//    DUMP8("%02x ",__cmse_ram_lend_data_section_exec_start__,16);
#endif

#ifdef CMSE_CRYPT_TEST_DEMO
    tz_customer_load_ram_ramx_section_info_t info = {0};
    info.flash_addr = (unsigned int )inc_enc_bin_sec_start;
    info.dst_text_sramx_addr_start = (unsigned int )__customer_load_sram_text_start__;
    info.dst_text_sramx_addr_end = (unsigned int )__customer_load_sram_text_end__;
    info.dst_data_sram_addr_start = (unsigned int )__customer_load_ram_data_start__;
    info.dst_data_sram_addr_end = (unsigned int )__customer_load_ram_data_end__;
    if(sec_decrypted_section_load_init((void *)&info) == 0){
        //rsa_test();
    }
#endif

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    TR_INFO(TR_MOD(MAIN), "Release CPU freq for sleep");

    funcptr_void NonSecure_ResetHandler;

    /* Add user setup code for secure part here*/

    /* Set non-secure main stack (MSP_NS) */
    //__TZ_set_MSP_NS(*((uint32_t *)(ns_app_start_addr)));

    ///TODO: check SECURE_BOOT
    /* Get non-secure reset handler */
#ifdef USER_SECURE_BOOT
    if(user_secure_boot_check() !=0){
        ASSERT(0,"security check fail");
        return 0;
    }
#endif
#ifdef CMSE_RAM_RAMX_LEND_NSE
    hal_sec_init();

    ret = mpc_init();
    ASSERT(ret==0, "mpc init fail. ret:%d", ret);

    TZ_SAU_Setup();
    __ISB();
#endif

#ifdef SECURE_BOOT
    NonSecure_ResetHandler = (funcptr_void)((uint32_t)ns_app_start_addr + sizeof(struct boot_struct_t) + sizeof(struct code_sig_struct_t) +
                                            sizeof(struct norflash_cfg_struct_t) + sizeof(struct code_ver_struct_t));
#else
    NonSecure_ResetHandler = (funcptr_void)((uint32_t)(&((struct boot_struct_t *)ns_app_start_addr)->hdr + 1));
#endif
    if (*(uint32_t *)ns_app_start_addr != BOOT_MAGIC_NUMBER) {
        TRACE(0, "nonsec image(0x%08x) magic(0x%08x) error", ns_app_start_addr, *(uint32_t *)ns_app_start_addr);
    } else {
        TRACE(0, "Call nonsec App. start addr:0x%08x", ns_app_start_addr);
        /* Start non-secure state software application */
#ifdef DEBUG
        cmse_set_ns_start_flag(true);
#endif
        NonSecure_ResetHandler();
    }
    /* Non-secure software does not return, this code is not executed */
    SAFE_PROGRAM_STOP();
    return 0;
}
