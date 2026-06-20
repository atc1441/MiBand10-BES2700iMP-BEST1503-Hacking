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

#ifdef RB_CODEC
#define HEAP_SECTION_SIZE			0x1000
#else
#ifdef VOICE_ASSIST_ADA_IIR
	#ifndef RESOLUTION_2048
#define HEAP_SECTION_SIZE			0x10000
	#else
#define HEAP_SECTION_SIZE			0x20000
	#endif
#else
#define HEAP_SECTION_SIZE			0x1000
#endif
#endif

#if defined(__NuttX__)
#include "nuttx/config.h"
#define NUTTX_HEAP_SECTION_SIZE		0x30000
#define STACK_SECTION_SIZE			CONFIG_IDLETHREAD_STACKSIZE
#else
#ifdef ARM_CMSE
#define STACK_SECTION_SIZE			0x2100
#else
#define STACK_SECTION_SIZE			0x1100
#endif
#endif
#define CP_STACK_SECTION_SIZE		0x1000

#if defined(ROM_UTILS_ON)
#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define ROM_UTILS_RESV_RAM_SIZE     0x64
#else
#define ROM_UTILS_RESV_RAM_SIZE     0x0
#endif
#endif

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
#define OVERLAY_DATA_SECTION_SIZE	0x10000
#elif defined(RB_CODEC)
#define OVERLAY_DATA_SECTION_SIZE	0xA000
#elif (BLE_AUDIO_ENABLED)
#define OVERLAY_DATA_SECTION_SIZE	0x10000
#else
#define OVERLAY_DATA_SECTION_SIZE	0x8000
#endif

#if !defined(FAST_XRAM_SECTION_SIZE)
#if (RAM_SIZE >= 0x48000)
#define RTOS_IN_RAM
#if defined(BTUSB_AUDIO_MODE)
#define FAST_XRAM_SECTION_SIZE		0x20000
#elif (BLE_AUDIO_ENABLED)
#define FAST_XRAM_SECTION_SIZE		0x30000
#elif (defined(CHIP_BEST1402) && defined(SPEECH_TX_2MIC_NS2))
#define FAST_XRAM_SECTION_SIZE		0xF800
#elif defined(DUAL_MIC_RECORDING)
#define FAST_XRAM_SECTION_SIZE		0x20000
#else
#define FAST_XRAM_SECTION_SIZE		0x16000
#endif
#elif (RAM_SIZE >= 0x40000)
#if defined(CHIP_BEST1402) || defined(CHIP_BEST1400)
#define FAST_XRAM_SECTION_SIZE		0x5200
#else
#define FAST_XRAM_SECTION_SIZE		0x3800
#endif
#else
#define FAST_XRAM_SECTION_SIZE		0x6000
#endif
#endif
#if (FAST_XRAM_SECTION_SIZE & (32 - 1))
#error "FAST_XRAM_SECTION_SIZE should align with 32 bytes"
#endif

#if !defined(OTA_BOOT_SIZE) && defined(OTA_CODE_OFFSET)
#define FLASH_REGION_BASE			(FLASH_BASE + OTA_CODE_OFFSET)
#elif defined(OTA_BOOT_OFFSET)
#define FLASH_REGION_BASE			(FLASH_BASE + OTA_BOOT_OFFSET)
#else
#define FLASH_REGION_BASE			(FLASH_BASE)
#endif

#ifndef FLASH_REGION_SIZE
#define FLASH_REGION_SIZE			(FLASH_SIZE - (FLASH_REGION_BASE - FLASH_BASE))
#endif
/* For FLASH image size check */
#ifndef FLASH_IMAGE_SIZE
#define FLASH_IMAGE_SIZE			FLASH_REGION_SIZE
#endif

#if (FLASH_REGION_BASE + FLASH_REGION_SIZE) > (FLASH_BASE + FLASH_SIZE)
#error "(FLASH_REGION_BASE + FLASH_REGION_SIZE) should <= (FLASH_BASE + FLASH_SIZE)"
#endif

#define FLASH_NC_REGION_BASE		FLASH_C_TO_NC(FLASH_REGION_BASE)
#define FLASHX_REGION_BASE			FLASH_TO_FLASHX(FLASH_REGION_BASE)

#ifdef RAM_NC_BASE
#define REGION_VECTOR				RAM_NC
#else
#define REGION_VECTOR				RAM
#endif

#ifdef USE_MULTI_FLASH

#ifndef FLASH1_REGION_BASE
#define FLASH1_REGION_BASE			(FLASH1_BASE)
#endif

#ifndef FLASH1_REGION_SIZE
#define FLASH1_REGION_SIZE			(FLASH1_SIZE)
#endif

#if (FLASH1_REGION_BASE + FLASH1_REGION_SIZE) > (FLASH1_BASE + FLASH1_SIZE)
#error "(FLASH1_REGION_BASE + FLASH1_REGION_SIZE) should <= (FLASH1_BASE + FLASH1_SIZE)"
#endif

#define FLASH1_NC_REGION_BASE		FLASH1_C_TO_NC(FLASH1_REGION_BASE)

#endif /* USE_MULTI_FLASH */

#ifndef PSRAM_REGION_SIZE
#define PSRAM_REGION_SIZE			(PSRAM_SIZE)
#endif

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
#define OVERLAY_FOR_CP
#endif

#ifdef OVERLAY_FOR_CP
#define REGION_OVERLAY_TEXT			RAMCPX
#define REGION_OVERLAY_DATA			RAMCP
#else
#define REGION_OVERLAY_TEXT			FRAMX
#define REGION_OVERLAY_DATA			RAM
#endif

#if (FLASH_BASE == FLASHX_BASE)
#define FLASHX						FLASH
#endif
#if defined(PSRAM_BASE) && (PSRAM_BASE == PSRAMX_BASE)
#define PSRAMX						PSRAM
#endif
#if defined(PSRAMUHS_BASE) && (PSRAMUHS_BASE == PSRAMUHSX_BASE)
#define PSRAMUHSX					PSRAMUHS
#endif

#ifdef MAIN_RAM_USE_TCM
#define FRAMX						RAMX
#endif

/* Linker script to configure memory regions. */
/* See plat_addr_map.h and common.mk for the macro definitions */
MEMORY
{
	ROM			(rx)	: ORIGIN = ROM_BASE,				LENGTH = ROM_SIZE
	FLASH 		(r)		: ORIGIN = FLASH_REGION_BASE,		LENGTH = FLASH_REGION_SIZE
	FLASH_NC	(r)		: ORIGIN = FLASH_NC_REGION_BASE,	LENGTH = FLASH_REGION_SIZE
#ifdef USE_MULTI_FLASH
	FLASH1 		(r)		: ORIGIN = FLASH1_REGION_BASE,		LENGTH = FLASH1_REGION_SIZE
	FLASH1_NC	(r)		: ORIGIN = FLASH1_NC_REGION_BASE,	LENGTH = FLASH1_REGION_SIZE
#endif
#ifndef FLASHX
	FLASHX		(rx)	: ORIGIN = FLASHX_REGION_BASE,		LENGTH = FLASH_REGION_SIZE
#endif
#ifdef MAIN_RAM_USE_TCM
	RAM			(rwx)	: ORIGIN = RAM_BASE,				LENGTH = RAM_SIZE
	RAMX		(rx)	: ORIGIN = RAMX_BASE,				LENGTH = RAMX_SIZE
#else
#if (CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE > 0)
	RAM			(rwx)	: ORIGIN = RAM_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE - CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE - CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE
	RAMX		(rx)	: ORIGIN = RAMX_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE - CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE - CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE
	FRAMX		(rwx)	: ORIGIN = RAMX_BASE + RAM_SIZE - FAST_XRAM_SECTION_SIZE,	LENGTH = FAST_XRAM_SECTION_SIZE
	CUSTOMER_LOAD_RAM_DATA_SECTION		(rwx)	: ORIGIN = RAM_BASE + RAM_SIZE - FAST_XRAM_SECTION_SIZE - CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE - CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE,	LENGTH = CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE
	CUSTOMER_LOAD_SRAM_TEXT_SECTION		(rwx)	: ORIGIN = RAMX_BASE + RAM_SIZE - FAST_XRAM_SECTION_SIZE - CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE,	LENGTH = CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE
#else
	RAM			(rwx)	: ORIGIN = RAM_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE
	RAMX		(rx)	: ORIGIN = RAMX_BASE,				LENGTH = RAM_SIZE - FAST_XRAM_SECTION_SIZE
	FRAMX		(rwx)	: ORIGIN = RAMX_BASE + RAM_SIZE - FAST_XRAM_SECTION_SIZE,	LENGTH = FAST_XRAM_SECTION_SIZE
#endif
#endif
#ifdef RAM_NC_BASE
	RAM_NC		(rw)	: ORIGIN = RAM_NC_BASE,				LENGTH = RAM_NC_SIZE
#endif
#if defined(ARM_CMSE) && defined(INTSRAM_RUN)
	RAMX_NSC	(rx)	: ORIGIN = RAMX_BASE + RAM_S_SIZE,	LENGTH = RAM_NSC_SIZE
#endif
#if (defined(PSRAM_ENABLE) || defined(PSRAM_HEAP_ONLY)) && defined(PSRAM_BASE) && (PSRAM_SIZE > 0)
	PSRAM 		(rwx)	: ORIGIN = PSRAM_BASE,				LENGTH = PSRAM_REGION_SIZE
	PSRAM_NC	(rwx)	: ORIGIN = PSRAM_NC_BASE,			LENGTH = PSRAM_REGION_SIZE
#ifndef PSRAMX
	PSRAMX		(rwx)   : ORIGIN = PSRAMX_BASE,				LENGTH = PSRAM_REGION_SIZE
#endif
#endif
#if defined(PSRAMUHS_ENABLE) && defined(PSRAMUHS_BASE) && (MCU_PSRAMUHS_SIZE > 0)
	PSRAMUHS	(rwx)	: ORIGIN = MCU_PSRAMUHS_BASE,			LENGTH = MCU_PSRAMUHS_SIZE
	PSRAMUHS_NC	(rwx)	: ORIGIN = MCU_PSRAMUHS_NC_BASE,		LENGTH = MCU_PSRAMUHS_SIZE
#ifndef PSRAMUHSX
	PSRAMUHSX	(rx)	: ORIGIN = MCU_PSRAMUHSX_BASE,			LENGTH = MCU_PSRAMUHS_SIZE
#endif
#endif
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
	RAMCP 		(rwx)	: ORIGIN = RAMCP_BASE,				LENGTH = RAMCP_SIZE
	RAMCPX 		(rwx)	: ORIGIN = RAMCPX_BASE,				LENGTH = RAMCPX_SIZE
#endif
#ifdef MEM_POOL_BASE
	MEM_POOL	(rwx)	: ORIGIN = MEM_POOL_BASE,			LENGTH = MEM_POOL_SIZE
#endif
#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)
	ARM_CMSE_RAM_DATA_LEND 		(rwx)	: ORIGIN = RAM2_BASE,				LENGTH = RAM2_BASE - RAM1_BASE
	ARM_CMSE_RAMX_TEXT_LEND 		(rwx)	: ORIGIN = RAMX3_BASE,				LENGTH = RAM3_BASE - RAM2_BASE
#endif
}

/* Library configurations */
#if !(defined(nuttx_build) || defined(ohos_build))
GROUP(libgcc.a libc.a libm.a libnosys.a)
#endif

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __export_fn_rom
 *   __exidx_start
 *   __exidx_end
 *   __copy_table_start__
 *   __copy_table_end__
 *   __zero_table_start__
 *   __zero_table_end__
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 *   __free_ram
 *   __factory_start
 *   __factory_end
 *   __calib_start
 *   __calib_end
 *   __flash_start
 *   __flash_end
 *   __free_flash
 *   __flash1_start
 *   __flash1_end
 *   __free_flash1
 *   __boot_sram_start_flash__
 *   __boot_sram_end_flash__
 *   __boot_sram_start__
 *   __boot_bss_sram_start__
 *   __boot_bss_sram_end__
 *   __sram_text_data_start_flash__
 *   __sram_text_data_end_flash__
 *   __fast_sram_text_data_start__
 *   __fast_sram_text_data_end__
 *   __fast_sram_text_data_start_flash__
 *   __fast_sram_text_data_end_flash__
 *   __sram_text_data_start__
 *   __sram_bss_start__
 *   __sram_bss_end__
 *   __overlay_text_start__
 *   __overlay_text_exec_start__
 *   __overlay_data_start__
 */
ENTRY(Boot_Loader)

SECTIONS
{
	__export_fn_rom = (ORIGIN(ROM) + LENGTH(ROM) - ROM_BUILD_INFO_SECTION_SIZE - ROM_EXPORT_FN_SECTION_SIZE);

	.boot_struct (ORIGIN(FLASH)) :
	{
		__flash_start = .;
		KEEP(*(.boot_struct))
		. = ALIGN(4);
	} > FLASH

	. = FLASH_TO_FLASHX(.);

	.boot_text_flash (.) : AT (FLASHX_TO_FLASH(ADDR(.boot_text_flash)))
	{
		__flashx_start_addr__ = .;
		*(.boot_loader)
		*(.boot_text_flash*)
		*(.boot_rodata_flash*)
		. = ALIGN(4);
	} > FLASHX

#if defined(__BES_OTA_MODE__)
	.ota_boot_info (ALIGN(4096)) :
	{
		__ota_boot_info_start = .;
		*(.ota_boot_info)
		. = (0x1000);
	} > FLASHX

	.ota_boot_rev (ALIGN(4096)) :
	{
		__ota_boot_rev_start = .;
		*(.ota_boot_rev)
		. = (0x1000);
	} > FLASHX
#endif

	. = FLASHX_TO_FLASH(.);

	.got_info (.) :
	{
		__got_info_start = .;
		__got_plt_start = .;
		*(.got.plt)
		__got_plt_end = .;
		__got_start = .;
		*(.got)
		__got_end = .;
		. = ALIGN(4);
	} > FLASH

	.vector_table (ORIGIN(REGION_VECTOR)) (NOLOAD) :
	{
		__vector_table_start = .;
		KEEP(*(.vector_table))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > REGION_VECTOR

	.reboot_param (NOLOAD) :
	{
		KEEP(*(.reboot_param))
		. = REBOOT_PARAM_SECTION_SIZE;
		. = ALIGN(4);
#if defined(ASIC_SIMU)
		. = ALIGN(1024*4);
#endif
	} > RAM

#if defined(ASIC_SIMU)
	.share_memory_pool (NOLOAD) :
	{
		__share_memory_pool_start = .;
        . = SHARE_MEMORY_POOL;
		__share_memory_pool_end = .;
		. = ALIGN(4);;
	} > RAM
#endif

	.userdata_pool (NOLOAD) :
	{
		*(.userdata_pool)
		. = ALIGN(4);
		__userdata_pool_end__ = .;
	} > RAM

	__boot_sram_start_flash__ = LOADADDR(.got_info) + SIZEOF(.got_info);
#ifndef MAIN_RAM_USE_TCM
	__boot_sram_start__ = ALIGN(__userdata_pool_end__, 32);
#endif

	.boot_text_sram
#ifdef MAIN_RAM_USE_TCM
		(ALIGN(ORIGIN(RAMX), 32))
#else
		(RAM_TO_RAMX(__boot_sram_start__))
#endif
		: AT (__boot_sram_start_flash__)
	{
		__boot_text_sram_start__ = .;
		_stext = __boot_text_sram_start__;
		/* memcpy.o/memcpy_move-kp.o or libc_nano.a(lib_a-memcpy.o/lib_a-memcpy-stub.o) */
		*:memcpy*.o(.text*)
		*:lib_a-memcpy*.o(.text*)
#ifndef MTEST_IN_FLASH
		*:libc_rom.o(.text*)
#endif
		*:libc_rom_mem.o(.text*)

		/* memset.o/memset-kp.o or libc_nano.a(lib_a-memset.o) */
		*:memset*.o(.text*)
		*:aeabi_memset.o(.text*)
		*:aeabi_memset-soft.o(.text*)
		*:lib_a-memset*.o(.text*)
#if defined(AAC_BECO_FFT)
		*:hal_cmu_best1306.o(.text.hal_cmu_beco_enable)
		*:hal_cmu_best1306.o(.text.aocmu_reg_update_wait)
		*:hal_cmu_best1306.o(.text.hal_cmu_beco_disable)
#endif
		*:hal_norflash*.o(.text*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .text*)

#ifdef BECO
		*:hal_*cmu_*.o(.text.hal_cmu_beco_enable)
		*:hal_*cmu_*.o(.text.hal_cmu_beco_disable)
		*:hal_*cmu_*.o(.text.hal_cmu_cp_beco_enable)
		*:hal_*cmu_*.o(.text.hal_cmu_cp_beco_disable)
		*:system_ARMCM.o(.text.set_beco_coprocessor)
#endif

#ifdef BOOT_CODE_IN_RAM
		*:hal_*cmu_*.o(.text*)
		*:stack_protector*.o(.text*)
		*:hal_iomux_*.o(.text*)
		*:hal_analogif_*.o(.text*)
		*:hal_spi.o(.text*)
		*:hal_timer.o(.text*)
		*:hal_sysfreq.o(.text*)
#if defined(PSRAM_ENABLE) || defined(PSRAMUHS_ENABLE)
		*:hal_psram*.o(.text*)
		*:hal_trace.o(.text*)
		*:pmu_*.o(.text*)
#endif
#endif
#ifdef TRACE_CODE_IN_RAM
		*:hal_trace.o(.text*)
		*:hal_trace_mod.o(.text*)
#else
		*:hal_trace.o(.text.hal_trace_printf)
		*:hal_trace.o(.text.hal_trace_output)
		*:hal_trace.o(.text.hal_trace_output_linefeed)
		*:hal_trace.o(.text.hal_trace_flush_buffer)
		*:hal_trace.o(.text.hal_trace_send)
		*:hal_trace.o(.text.hal_trace_dump)
#if defined(CP_FAULT_DUMP) && !defined(CP_TRACE_ENABLE)
		*:hal_trace.o(.text.hal_trace_fault_dump_cp)
		*:hal_trace.o(.text.hal_trace_fault_cp_handler)
		*:hal_trace.o(.text.ARM_MPU_Disable)
#endif
#endif

#ifdef ROM_IN_FLASH
		*:hal_cmu_*.o(.text.hal_cmu_flash*_set_freq .text.hal_cmu_reset_set .text.hal_cmu_reset_clear)
		*:hal_cmu_common.o(.text.hal_cmu_get_crystal_freq)
		*:hal_timer.o(.text.hal_sys_timer_delay .text.hal_sys_timer_delay_us
			.text.hal_sys_timer_delay_ns .text.hal_sys_timer_get
			.text.hal_fast_sys_timer_get)
#endif
#if defined(ARM_CMSE) && defined(INTSRAM_RUN)
		/* BEST1501 SAU set FLASH_BASE region as nonsecure region becase of the SAU region number limitation,
		CPU can't access FLASH_BASE region in secure state, so the text should be located in SRAM */
#if (defined(CMSE_RAM_RAMX_LEND_NSE))
		EXCLUDE_FILE(*:rsa_test.o *:user_secure_boot.o *:crypt_lib.o *:bin_aes_crypt.o *:parser.o)
#elif defined(CMSE_CRYPT_TEST_DEMO)
		EXCLUDE_FILE(*:rsa_test.o)
#endif
		*(EXCLUDE_FILE(*crt*) .text* .flash_text*)
#endif
		*(.boot_text_sram*)
		. = ALIGN(32);
		__boot_text_sram_end__ = .;
	} > RAMX

	__boot_text_sram_end_flash__ = __boot_sram_start_flash__ + SIZEOF(.boot_text_sram);
	__boot_data_sram_start_flash__ = __boot_text_sram_end_flash__;

	.boot_data_sram
#ifndef MAIN_RAM_USE_TCM
		(RAMX_TO_RAM(ADDR(.boot_text_sram) + SIZEOF(.boot_text_sram)))
#endif
		: AT (__boot_data_sram_start_flash__)
	{
		__boot_data_sram_start__ = .;
#if defined(AAC_BECO_FFT)
		*:hal_cmu_best1306.o(.data* .rodata*)
#endif
		*:hal_norflash*.o(.data* .rodata*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .data* EXCLUDE_FILE(*:norflash_api.o) .rodata*)
		*:libc_rom.o(.rodata*)
		*:libc_rom_mem.o(.rodata*)

#ifdef BOOT_CODE_IN_RAM
		*:hal_*cmu_*.o(.data* .rodata*)
		*:stack_protector*.o(.data* .rodata*)
		*:hal_iomux_*.o(.data* .rodata*)
		*:hal_analogif_*.o(.data* .rodata*)
		*:hal_spi.o(.data* .rodata*)
		*:hal_timer.o(.data* .rodata*)
		*:hal_sysfreq.o(.data* .rodata*)
#if defined(PSRAM_ENABLE) || defined(PSRAMUHS_ENABLE)
		*:hal_psram*.o(.data* .rodata*)
		*:hal_trace.o(.data* .rodata*)
		*:pmu_*.o(.data* .rodata*)
#endif
#endif
#ifdef TRACE_CODE_IN_RAM
		*:hal_trace.o(.data* .rodata*)
		*:hal_trace_mod.o(.data* .rodata*)
#else
		*:hal_trace.o(.rodata.hal_trace_printf.cst4)
#endif

#if defined(ARM_CMSE) && defined(INTSRAM_RUN)
		/* BEST1501 SAU set FLASH_BASE region as nonsecure region becase of the SAU region number limitation,
		CPU can't access FLASH_BASE region in secure state, so the data and rodata should be located in SRAM */
#if (defined(CMSE_RAM_RAMX_LEND_NSE))
		EXCLUDE_FILE(*:rsa_test.o *:user_secure_boot.o *:crypt_lib.o *:bin_aes_crypt.o *:parser.o)
#elif defined(CMSE_CRYPT_TEST_DEMO)
		EXCLUDE_FILE(*:rsa_test.o)
#endif
		*(.data* .rodata*)
#endif

		/* Stack protector instrumentation rodata for functions in special sections */
		*(.rodata.cst4)

		*(.boot_rodata_sram*)
		*(.boot_data*)
		. = ALIGN(4);
		__boot_data_sram_end__ = .;
	} > RAM

	__boot_sram_end_flash__ = __boot_data_sram_start_flash__ + SIZEOF(.boot_data_sram);

	.boot_bss_sram (.) (NOLOAD) : AT (ADDR(.boot_bss_sram))
	{
		__boot_bss_sram_start__ = .;
		*:hal_norflash*.o(.bss*)
		*:norflash_*.o(EXCLUDE_FILE(*:norflash_api.o) .bss*)

#ifdef BOOT_CODE_IN_RAM
		*:hal_*cmu_*.o(.bss*)
		*:stack_protector*.o(.bss*)
		*:hal_iomux_*.o(.bss*)
		*:hal_analogif_*.o(.bss*)
		*:hal_spi.o(.bss*)
		*:hal_timer.o(.bss*)
		*:hal_sysfreq.o(.bss*)
#if defined(PSRAM_ENABLE) || defined(PSRAMUHS_ENABLE)
		*:hal_psram*.o(.bss*)
		*:hal_trace.o(.bss*)
		*:pmu_*.o(.bss*)
#endif
#endif
		*(.boot_bss*)
		. = ALIGN(4);
		__boot_bss_sram_end__ = .;
	} > RAM

#ifdef RAM_NC_BASE
	.sram_nc_bss (NOLOAD) : AT (ADDR(.sram_nc_bss))
	{
		__sram_nc_bss_start__ = .;
		*(.sram_nc_bss*)
		*(.sync_flags_sram*)
		. = ALIGN(4);
		__sram_nc_bss_end__ = .;
	} > RAM_NC

	__ram_nc_top = ORIGIN(RAM_NC) + LENGTH(RAM_NC);
	__free_ram_nc = __ram_nc_top - __sram_nc_bss_end__;
#endif

	__sram_text_data_start_flash__ = __boot_sram_end_flash__;
#ifdef MAIN_RAM_USE_TCM
	__sram_text_data_start__ = ALIGN(__boot_text_sram_end__, 32);
#else
	__sram_text_data_start__ = ALIGN(__boot_bss_sram_end__, 32);
#endif

	.sram_text (RAM_TO_RAMX(__sram_text_data_start__)) : AT (__sram_text_data_start_flash__)
	{
		__sram_text_start__ = .;
#ifdef RTOS_IN_RAM
		*:rtos_lib.o(.text*)
#endif
#if defined(SPEECH_TX_3MIC_NS)
		*:lib_a-s_fabs.o(.text*)
		*:lib_a-s_tanh.o(.text*)
		*:lib_a-s_expm1.o(.text*)
		*:lib_a-wf_log10.o(.text*)
		*:lib_a-ef_log10.o(.text*)
		*:lib_a-ef_log.o(.text*)
		*:lib_a-sf_frexp.o(.text*)
		*:lib_a-wf_exp.o(.text*)
		*:lib_a-ef_exp.o(.text*)
		*:lib_a-sf_tanh.o(.text*)
		*:lib_a-sf_fabs.o(.text*)
		*:lib_a-sf_finite.o(.text*)
		*:lib_a-sf_expm1.o(.text*)
		*:lib_a-wf_log10.o(.text*)
		*:lib_a-ef_log10.o(.text*)
		*:lib_a-ef_log.o(.text*)
		*:lib_a-wf_pow.o(.text*)
		*:lib_a-ef_pow.o(.text*)
		*:lib_a-s_matherr.o(.text*)
		*:app_anc.o(.text.app_anc_get_curr_mode)
#endif
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
		*:hal_memsc.o(.text*)
		*:mpu_armv*.o(.text*)
		*:codec_sbc.o(.text*)
		*:multi_heap.o(.text*)
		*:pool_api.o(.text*)
		*:_arm_addsubdf3.o(.text*)
		*:_arm_cmpdf2.o(.text*)
		*:_arm_fixdfsi.o(.text*)
#if defined(SPEECH_TX_3MIC_NS)
		*:_arm_muldivdf3.o(.text*)
		*:_arm_truncdfsf2.o(.text*)
#endif
#if !defined(LIBC_ROM)
		*:vsprintf.o(.text*)
#endif
#if defined(A2DP_LHDC_ON)
		*libm.a:(.text*)
		*:_fixsfdi.o(.text*)
		*:_fixunssfdi.o(.text*)
		*:_arm_muldivdf3.o(.text*)
		*:_arm_fixunsdfsi.o(.text*)
#endif
#if defined(XIAOMI_TX_3MIC_ENABLE)
		/* for SCO_CP_ACCEL=1 */
		*:cmsis_dsp.o(.text*)
		*libm.a:(.text*)
		*libgcc.a(.text*)
		*libc_nano.a(.text*)

		*:_fixunssfdi.o(.text*)
		*:_arm_muldivdf3.o(.text*)
		*:_arm_fixunsdfsi.o(.text*)
		*:_arm_truncdfsf2.o(.text*)
		*:_arm_addsubsf3.o(.text*)
#endif
#endif
		*:_arm_unorddf2.o(.text*)
		*:hal_overlay.o(.text*)
#if defined(BLE_AUDIO_ENABLED)
		*:lc3_overlay.o(.text*)
#endif
		*:fftwrap.o(.text*)
#if defined(GLOBAL_SRAM_CMSIS_FFT)
		*:cmsis_dsp.o(.text.arm_radix8_butterfly_f32)
		*:cmsis_dsp.o(.text.arm_cfft_radix8by2_f32)
		*:cmsis_dsp.o(.text.arm_cfft_radix8by4_f32)
		*:cmsis_dsp.o(.text.arm_cfft_f32)
		*:cmsis_dsp.o(.text.stage_rfft_f32)
		*:cmsis_dsp.o(.text.merge_rfft_f32)
		*:cmsis_dsp.o(.text.arm_rfft_fast_f32)
#endif
		*:speech_win.o(.text*)
		*(.sram_text*)
		/*
		   If you want protect one section by armv8's mpu, the section should
		   be aligned to 32bytes
		 */
		. = ALIGN(32);
		__sram_text_end__ = .;
	} > RAMX

	__sram_text_end_flash__ = __sram_text_data_start_flash__ + SIZEOF(.sram_text);
	__sram_data_start_flash__ = __sram_text_end_flash__;

	.sram_data
#ifndef MAIN_RAM_USE_TCM
		(RAMX_TO_RAM(ADDR(.sram_text) + SIZEOF(.sram_text)))
#endif
		: AT (__sram_data_start_flash__)
	{
		__sram_data_start__ = .;
#ifdef RTOS_IN_RAM
		*:rtos_lib.o(.data* .rodata*)
#endif
#if defined(A2DP_LHDC_ON)
		*libm.a:(.data* .rodata*)
		*:_fixsfdi.o(.data* .rodata*)
		*:_fixunssfdi.o(.data* .rodata*)
		*:_arm_muldivdf3.o(.data* .rodata*)
		*:_arm_fixunsdfsi.o(.data* .rodata*)
#endif
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
		*:hal_memsc.o(.data* .rodata*)
		*:mpu_armv*.o(.data* .rodata*)
		*:multi_heap.o(.data* .rodata*)
		*:pool_api.o(.data* .rodata*)
		*:sbc.o(.data* .rodata*)
		*:sbc_api.o(.data* .rodata*)
		*:aac_tool.o(.data* .rodata*)
#if !defined(LIBC_ROM)
		*:vsprintf.o(.rodata*)
#endif
#if defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_2MIC_NS8)
		*:lib_a-wf_sqrt.o(.data* .rodata*)
		*:lib_a-ef_sqrt.o(.data* .rodata*)
		*:lib_a-sf_fabs.o(.data* .rodata*)
		*:lib_a-sf_frexp.o(.data* .rodata*)
		*:beco.o(.data* .rodata*)
#endif
#if defined(XIAOMI_TX_3MIC_ENABLE)
		/* for SCO_CP_ACCEL=1 */
		*:cmsis_dsp.o(.rodata*)
		*libm.a:(.rodata*)
		*libgcc.a(.rodata*)
		*libc_nano.a(.rodata*)
#endif
#endif
		*:_arm_unorddf2.o(.data* .rodata*)
		*:fftwrap.o(.data* .rodata*)
		*:hal_overlay.o(.data* .rodata*)
#if defined(BLE_AUDIO_ENABLED)
		*:lc3_overlay.o(.data* .rodata*)
#endif
		*:speech_win.o(.data* .rodata*)
#if RTT_APP_SUPPORT > 0
    /* section information for finsh shell */
        . = ALIGN(4);
        __fsymtab_start = .;
        KEEP(*(FSymTab))
        __fsymtab_end = .;

        . = ALIGN(4);
        __vsymtab_start = .;
        KEEP(*(VSymTab))
        __vsymtab_end = .;

        /* section information for initial. */
        . = ALIGN(4);
        __rt_init_start = .;
        KEEP(*(SORT(.rti_fn*)))
        __rt_init_end = .;
#endif
		_hdf_drivers_start = .;
		KEEP(*(.hdf.driver))
		_hdf_drivers_end = .;

#ifdef ARM_CMSE
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);

		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		. = ALIGN(4);
		__cust_cmd_hldr_tbl_start = .;
		KEEP(*(.cust_cmd_hldr_tbl))
		__cust_cmd_hldr_tbl_end = .;
		. = ALIGN(4);
		__ai_gatt_server_table_start = .;
		KEEP(*(.ai_gatt_server_table))
		__ai_gatt_server_table_end = .;
		. = ALIGN(4);
		__ai_ble_handler_table_start = .;
		KEEP(*(.ai_ble_handler_table))
		__ai_ble_handler_table_end = .;
		. = ALIGN(4);
		__ai_spp_register_table_start = .;
		KEEP(*(.ai_spp_register_table))
		__ai_spp_register_table_end = .;
		. = ALIGN(4);
		__tota_handler_table_start = .;
		KEEP(*(.tota_handler_table))
		__tota_handler_table_end = .;
		. = ALIGN(4);
		__ai_handler_function_table_start = .;
		KEEP(*(.ai_handler_function_table))
		__ai_handler_function_table_end = .;
		. = ALIGN(4);
		__custom_handler_table_start = .;
		KEEP(*(.custom_handler_table))
		__custom_handler_table_end = .;
		. = ALIGN(4);
		__mixprompt_property_table_start = .;
		KEEP(*(.mixprompt_property_table))
		__mixprompt_property_table_end = .;
		. = ALIGN(4);
		__core_bridge_task_cmd_table_start = .;
		KEEP(*(.core_bridge_task_cmd_table))
		__core_bridge_task_cmd_table_end = .;
		. = ALIGN(4);
		__core_bridge_instant_cmd_table_start = .;
		KEEP(*(.core_bridge_instant_cmd_table))
		__core_bridge_instant_cmd_table_end = .;
		. = ALIGN(4);
		__rpc_bth_dsp_cmd_table_start = .;
		KEEP(*(.rpc_bth_dsp_cmd_table))
		__rpc_bth_dsp_cmd_table_end = .;
		. = ALIGN(4);
		__app_bt_sync_command_handler_table_start = .;
		KEEP(*(.app_bt_sync_command_handler_table))
		__app_bt_sync_command_handler_table_end = .;
		. = ALIGN(4);
		KEEP(*(.eh_frame*))
		*(.note.gnu.build-id)
#endif

		*(.sram_rodata*)
		*(.sram_data*)
#if (RAMCP_SIZE == 0)
		*(.cp_data*)
#endif
		. = ALIGN(4);
		__sram_data_end__ = .;
	} > RAM

	__sram_text_data_end_flash__ = __sram_data_start_flash__ + SIZEOF(.sram_data);

	.sram_bss (.) (NOLOAD) : AT (ADDR(.sram_bss))
	{
		__sram_bss_start__ = .;
		*:utils.o(.bss*)
#if defined(_CVSD_BYPASS_) || defined(CVSD_BYPASS)
		*:Pcm8k_Cvsd.o(.bss*)
		*:cvsd_codec.o(.bss*)
#endif

#if defined(VOICE_TX_AEC)
		*:ssp_aec.o(.bss*)
#endif

#if defined(APP_NOISE_ESTIMATION)
		*:noise_estimator.o(.bss*)
#endif
		*(.sram_bss*)
#if (RAMCP_SIZE == 0)
		*(.cp_bss*)
#endif

#ifdef BLE_BIS_TRANSPORT
		. = ALIGN(4);
		__ble_bis_tran_mem_pool_start__ = .;
		. += BLE_BIS_TRANSPORT_BUF_SIZE;
		__ble_bis_tran_mem_pool_end__ = .;
#endif
		. = ALIGN(32);
		__sram_bss_end__ = .;
	} > RAM


#if defined(ARM_CMSE) && defined(INTSRAM_RUN)
	__sram_nsc_text_start_flash__ = __sram_text_data_end_flash__;
	.gnu.sgstubs (ORIGIN(RAMX_NSC)) : AT (__sram_nsc_text_start_flash__)
	{
		__sram_nsc_text_start__ = .;
		*(.gnu.sgstubs*)
		. = ALIGN(4);
	} > RAMX_NSC
	__sram_nsc_text_end_flash__ = __sram_nsc_text_start_flash__ + SIZEOF(.gnu.sgstubs);
	__fast_sram_text_data_start_flash__ = __sram_nsc_text_end_flash__;
#else
	__fast_sram_text_data_start_flash__ = __sram_text_data_end_flash__;
#endif

	.fast_text_sram : AT (__fast_sram_text_data_start_flash__)
	{
		__fast_sram_text_exec_start__ = .;
#if defined(GLOBAL_SRAM_KISS_FFT)
		*:fftwrap.o(.text*)
		*:kiss_fft.o(.text*)
		*:kiss_fftr.o(.text*)
#endif
#if defined(AAC_BECO_FFT)
		*:beco.o(.text*)
		*:system_ARMCM.o(.text*)
#endif
#if defined(__VIRTUAL_SURROUND__) || defined(__VIRTUAL_SURROUND_STEREO__)
			*:cmsis_dsp.o(.text.arm_copy_f32)
#endif
#if defined(__VIRTUAL_SURROUND_CP__)
			*:cmsis_dsp.o(.text.arm_copy_f32)
			*:virtual_surround_process.o(.text*)
#endif
#if defined(VOICE_ALGO_CP_ACCEL)
		*:cmsis_fftwrap.o(.text*)
		*:arm_rfft_fast_f32.o(.text*)
		*:arm_cfft_f32.o(.text*.arm_cfft_f32)
		*:arm_bitreversal2.o(.text*.arm_bitreversal_32)
		*:arm_cfft_radix8_f32.o(.text*.arm_radix8_butterfly_f32)

		*:voice_algo.o(.text*.voice_algo_process_impl)
		*:voice_algo.o(.text*.algo_process_impl)
#if defined(VOICE_TX_AEC)
		*:voice_tx_aec.o(.text*.voice_tx_aec_process)
		*:ssp_aec.o(.text*)
#endif
#if defined(APP_NOISE_ESTIMATION)
		*:voice_noise_est.o(.text*.voice_noise_est_process)
		*:noise_estimator.o(.text*)
#endif
#endif


#if defined(ROM_UTILS_ON)
		*:rom_utils.o(.text*)
#endif

#if defined(BTUSB_AUDIO_MODE)
		*:hal_dma.o(.text*)
		*:hal_uart.o(.text*)
		*:hal_trace.o(.text*)
#endif

#if defined(BTUSB_AUDIO_MODE)
		*:hal_usb.o(.text*)

		*:usb_audio.o(.text*)
		*:usb_audio_sync.o(.text*)
		*:usb_audio_cdc.o(.text*)

		*:usbaudio_entry.o(.text*)
		*:usb_audio_app.o(.text*)
#endif

		/* memmove.o or libc_nano.a(lib_a-memmove.o) */
		*:memmove*.o(.text*)
		*:lib_a-memmove.o(.text*)
		*:utils.o(.text*)
		*:cqueue.o(.text*)
		*:audio_resample_ex.o(.text*)
#if defined(MIX_MIC_DURING_MUSIC)
		*:app_bt_stream.o(.text*)
#endif
#if defined(__SBC_FUNC_IN_ROM__) || (defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0))
		*:sbc.o(.text*)
#else
		*:sbc.o(.text*.sbc_is_stream_info_valid)
		*:sbc.o(.text*.sbc_frame_length_get)
		*:sbc.o(.text*.sbc_4bit_crc_calc)
		*:sbc.o(.text*.sbc_bits_for_mono_alloc)
		*:sbc.o(.text*.sbc_bits_for_stereo_alloc)
		*:sbc.o(.text*.sbc_header_process)
		*:sbc.o(.text*.sbc_factors_process)
		*:sbc.o(.text*.sbc_sample_extract)
		*:sbc.o(.text*.sbc_sbsamples_make)
		*:sbc.o(.text*.sbc_joint_process)
		*:sbc.o(.text*.sbc_4band_synth_filter)
		*:sbc.o(.text*.sbc_8band_synth_filter)
		*:sbc.o(.text*.sbc_decoder_state_reset)
		*:sbc.o(.text*.sbc_frame_mute)
		*:sbc.o(.text*.sbc_frames_decode)
		*:sbc.o(.text*.sbc_frames_decode_do)
		*:sbc.o(.text*.sbc_frames_decode_select_channel)
		*:sbc.o(.text*.sbc_frames_decode)
		*:sbc.o(.text*.sbc_samples_quant)
		*:sbc.o(.text*.sbc_decoder_init)
		*:sbc.o(.text*.sbc_encoder_init)
		*:sbc.o(.text*.sbc_frames_encode)
#endif
		*:sbc_math.o(.text*)
		*:sbc_api.o(.text*)
		*:aac_tool.o(.text*)
#if defined(A2DP_AAC_ON)
		*:fixpoint_math.o(.text*)
		*:FDK_bitbuffer.o(.text*)
		*:FDK_tools_rom.o(.text*FDKgetWindowSlope*)
		*:FDK_tools_rom.o(.text*getBitstreamElementList*)
		*:dct.o(.text*dct_IV*)
#if !defined(__SBC_FUNC_IN_ROM_VBEST2000__)
		*:fft.o(.text*._Z3fftiPlPi)
		*:fft_rad2.o(.text*dit_fft*)
		*:dct.o(.text*getTables*)
#endif
#if defined(OPUS_IN_OVERLAY)
		*:voice_opus.o(.text*)
#elif defined(VOICE_DATAPATH) || defined(__ALEXA_WWE) || defined(AQE_KWS_ALEXA)
		*:opus_encoder.o(.text*)
#elif defined(__AI_VOICE__) && (RAM_SIZE >= 0x48000)
		*:voice_opus.o(.text*)
		/**:opus_encoder.o(.text*)*/
		*:opus_encoder.o(.text*.frame_size_select)
		*:opus_encoder.o(.text*.opus_encode_native)
		*:opus_encoder.o(.text*.user_bitrate_to_bitrate)
		*:opus_encoder.o(.text*.compute_equiv_rate)
		*:opus_encoder.o(.text*.decide_fec)
		*:opus_encoder.o(.text*.dc_reject)
		*:opus_encoder.o(.text*.gen_toc)
		*:opus_encoder.o(.text*.opus_encode)
		*:opus_encoder.o(.text*.opus_encoder_create)
		*:opus_encoder.o(.text*.opus_encoder_get_size)
		*:opus_encoder.o(.text*.opus_encoder_init)
		*:opus_encoder.o(.text*.opus_encoder_ctl)
		*:analysis.o(.text*)
		/**:celt_encoder.o(.text*)*/
		*:celt_encoder.o(.text*.opus_custom_encoder_ctl)
		*:celt_encoder.o(.text*.celt_encoder_get_size)
		*:celt_encoder.o(.text*.opus_custom_encoder_get_size)
		*:celt_encoder.o(.text*.celt_encoder_init)
		*:celt_encoder.o(.text*.opus_custom_encoder_init_arch)
		/**:repacketizer.o(.text*)*/
		*:repacketizer.o(.text*.opus_packet_pad)
		*:repacketizer.o(.text*.opus_repacketizer_init)
		*:repacketizer.o(.text*.opus_repacketizer_cat)
		*:repacketizer.o(.text*.opus_repacketizer_cat_impl)
		*:repacketizer.o(.text*.opus_repacketizer_out_range_impl)
		*:opus.o(.text*)
		*:enc_API.o(.text*)
		/**:entenc.o(.text*)*/
		*:entenc.o(.text*.ec_enc_init)
		*:entenc.o(.text*.ec_enc_icdf)
		*:entenc.o(.text*.ec_enc_normalize)
		*:entenc.o(.text*.ec_enc_carry_out)
		*:entenc.o(.text*.ec_write_byte)
		*:entenc.o(.text*.ec_enc_patch_initial_bits)
		*:entenc.o(.text*.ec_enc_done)
		*:control_audio_bandwidth.o(.text*)
		*:encode_pulses.o(.text*)
		*:LPC_analysis_filter.o(.text*)
		*:burg_modified_FIX.o(.text*.silk_burg_modified_c)
		*:vector_ops_FIX.o(.text*.silk_scale_copy_vector16)
		*:vector_ops_FIX.o(.text*.silk_inner_prod_aligned)
		*:vector_ops_FIX.o(.text*.silk_inner_prod16_aligned_64_c)
		*:NLSF_del_dec_quant.o(.text*.silk_NLSF_del_dec_quant)
		*:encode_frame_FIX.o(.text*.silk_encode_frame_FIX)
		*:encode_frame_FIX.o(.text*.silk_encode_do_VAD_FIX)
		*:encode_frame_FIX.o(.text*.silk_LBRR_encode_FIX)
		/**:pitch.o(.text*)*/
		*:pitch.o(.text*.celt_pitch_xcorr_c)
		*:resampler.o(.text*)
		*:VAD.o(.text*)
		*:encode_indices.o(.text*)
		/**:control_codec.o(.text*)*/
		*:control_codec.o(.text*.silk_control_encoder)
		*:control_codec.o(.text*.silk_setup_resamplers)
		*:control_codec.o(.text*.silk_setup_fs)
		*:control_codec.o(.text*.silk_setup_complexity)
		*:control_codec.o(.text*.silk_setup_LBRR)
		/**:celt_lpc.o(.text*)*/
		*:celt_lpc.o(.text*._celt_autocorr)
		/**:shell_coder.o(.text*)*/
		*:shell_coder.o(.text*.combine_pulses)
		*:shell_coder.o(.text*.silk_shell_encoder)
		*:shell_coder.o(.text*.encode_split)
		*:init_encoder.o(.text*)
		*:log2lin.o(.text*)
		*:biquad_alt.o(.text*)
		*:stereo_encode_pred.o(.text*)
		*:NLSF_unpack.o(.text*)
		*:code_signs.o(.text*)
		*:lin2log.o(.text*)
		*:HP_variable_cutoff.o(.text*)
		*:sum_sqr_shift.o(.text*)
		*:inner_prod_aligned.o(.text*)
		*:stereo_quant_pred.o(.text*)
		*:entcode.o(.text*)
		*:celt.o(.text*)
		*:laplace.o(.text*)
		/**:NSQ.o(.text*)*/ /*complexity 1*/
		/**:NSQ_del_dec.o(.text*)*/ /*complexity 2*/
#endif
#endif

#if defined(AI_CAPTURE_DATA_AEC)
		*:coherence.o(.text*)
		*:echo_canceller.o(.text*)
		*:echo_suppression.o(.text*)
		*:lcmmse.o(.text*)
		*:logmmse.o(.text*)
		*:mdf2.o(.text*)
		*:noise_generator.o(.text*)
		*:pink_noise_gen.o(.text*)
		*:white_noise_gen.o(.text*)
#endif

#if defined(__AUDIO_SPECTRUM__)
		*:spectrum_fix.o(.text*)
		*:audio_spectrum.o(.text*)
#endif
#if defined(A2DP_SCALABLE_ON) && defined(A2DP_AAC_ON)
		*:mdct.o(.text*)
#endif
		*:crossover_filter.o(.text*)
		*:data_compressor.o(.text*)
		*:drc.o(.text*)
		*:limiter.o(.text*)
#if defined(AQE_KWS)
		*:arm_*.o(.text*)
		*:aqe_mfcc.o(.text*)
		*:kws_nn.o(.text*)
		*:command_recognition.o(.text*)
		*:arm_radix8_butterfly_f32.o(.text*)
		*:arm_bitreversal_32.o(.text*)
		*:arm_bitreversal_16.o(.text*)
		*:dct.o(.text*dct_IV*)
		*:fft.o(.text*._Z3fftiPlPi)
		*:fft_rad2.o(.text*dit_fft*)
		*:dct.o(.text*getTables*)
		*:arm_cfft_radix8_f32.o(.text*)
		*:arm_bitreversal2.o(.text*)
		*:arm_mult_f32.o(.text*)
		*:arm_cmplx_mag_squared_f32.o(.text*)
		*:arm_cmplx_mult_cmplx_f32.o(.text*)
		*:lib_a-wf_sqrt.o(.text*)
		*:arm_abs_f32.o(.text*)
		*:arm_cos_f32.o(.text*)
		*:arm_sin_f32.o(.text*)

		*:coherence.o(.text*)
		*:echo_canceller.o(.text*)
		*:echo_suppression.o(.text*)
		*:lcmmse.o(.text*)
		*:logmmse.o(.text*)
		*:mdf2.o(.text*)
		*:noise_generator.o(.text*)
		*:pink_noise_gen.o(.text*)
		*:white_noise_gen.o(.text*)

		*:ae_common.o(.text*)
		*:ae_rand.o(.text*)
		*:fastmath.o(.text*)
		*:integer_delay.o(.text*)
		*:ringbuf.o(.text*)
		*:scale_int16.o(.text*)
		*:speech_conv.o(.text*)
		*:speech_trans_buf.o(.text*)

		*:ns3.o(.text*)
		*:noise_suppression_x.o(.text*)
		*:nsx_core.o(.text*)
		*:nsx_core_c.o(.text*)
		*:copy_set_operations.o(.text*)
		*:cross_correlation.o(.text*)
		*:division_operations.o(.text*)
		*:dot_product_with_scale.o(.text*)
		*:downsample_fast.o(.text*)
		*:energy.o(.text*)
		*:get_scaling_square.o(.text*)
		*:min_max_operations.o(.text*)
		*:real_fft.o(.text*)
		*:resample_by_2.o(.text*)
		*:resample_by_2_internal.o(.text*)
		*:spl_init.o(.text*)
		*:spl_sqrt.o(.text*)
		*:spl_sqrt_floor.o(.text*)
		*:vector_scaling_operations.o(.text*)
#endif
#if defined(SPEECH_TX_NN_NS2)
	*:nn_ns_pitch.o(.text*)
	*:nn_ns_celt_lpc.o(.text*)
	*:nn_ns_model.o(.text*)
	*:nn_ns2.o(.text*)
#endif


#if !defined(SCO_OPTIMIZE_FOR_RAM)
		:audio_prompt_sbc.o(.text*)
#endif

		*:sbcplc.o(.text*)
		*:iirfilt.o(.text*)
#if defined(SPEECH_TX_2MIC_NS7) || defined(SPEECH_TX_2MIC_NS8)
		*:lib_a-wf_sqrt.o(.text*)
		*:lib_a-ef_sqrt.o(.text*)
		*:lib_a-sf_fabs.o(.text*)
		*:lib_a-sf_frexp.o(.text*) /* besnet_utils.c */
		*:beco.o(.text*)
		*:fastmath_approx.o(.text*)
#endif
		*:cmsis_dsp.o(.text.arm_scale_f32)
#if 0
		*:frame_resize.o(.text*)
		*:buffer_manager.o(.text*)
#if defined(_CVSD_BYPASS_) || defined(CVSD_BYPASS)
		*:Pcm8k_Cvsd.o(.text*)
#endif
		*:lpc_plc.o(.text*)
#endif

#if defined(A2DP_SCALABLE_ON)
		*:util.o(.text*)
#endif
		*(.fast_text_sram*)
#if (RAMCP_SIZE == 0)
		*(.cp_text_sram*)
#endif
		/*
		   If you want protect one section by armv8's mpu, the section should
		   be aligned to 32bytes
		 */
		. = ALIGN(32);
		__fast_sram_text_exec_end__ = .;
	} > FRAMX

	__fast_sram_text_data_end_flash__ = __fast_sram_text_data_start_flash__ + SIZEOF(.fast_text_sram);
	__fast_sram_text_data_start__ = RAMX_TO_RAM(__fast_sram_text_exec_start__);
	__fast_sram_text_data_end__ = RAMX_TO_RAM(__fast_sram_text_exec_end__);

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
	__cp_text_sram_start_flash__ = __fast_sram_text_data_end_flash__;

	.cp_text_sram (ORIGIN(RAMCPX)) : AT (__cp_text_sram_start_flash__)
	{
		__cp_text_sram_exec_start__ = .;
		*(.cp_text_sram*)
		*:cp_queue.o(.text*)
#if defined(GSOUND_HOTWORD_EXTERNAL)
		//*:libspeech*.o(.text*)
#endif
		. = ALIGN(4);
		__cp_text_sram_exec_end__ = .;
	} > RAMCPX

	__cp_text_sram_start = RAMX_TO_RAM(__cp_text_sram_exec_start__);
	__cp_text_sram_end = RAMX_TO_RAM(__cp_text_sram_exec_end__);

	.vector_table_cp (ORIGIN(RAMCP)) (NOLOAD) :
	{
		KEEP(*(.vector_table_cp))
		. = VECTOR_SECTION_SIZE;
		. = ALIGN(4);
	} > RAMCP

	__cp_data_sram_start_flash__ = __cp_text_sram_start_flash__ + SIZEOF(.cp_text_sram);

	.cp_data_sram (.) : AT (__cp_data_sram_start_flash__)
	{
		__cp_data_sram_start = .;
		*(.cp_data*)
		*:cp_queue.o(.data* .rodata*)
#if defined(GSOUND_HOTWORD_EXTERNAL)
		//*:libspeech*.o(.data* .rodata*)
#endif
		. = ALIGN(4);
		__cp_data_sram_end = .;
	} > RAMCP

	__cp_sram_end_flash__ = __cp_data_sram_start_flash__ + SIZEOF(.cp_data_sram);

	.cp_bss_sram (.) (NOLOAD) : AT (ADDR(.cp_bss_sram))
	{
		__cp_bss_sram_start = .;
		*(.cp_bss*)
		. = ALIGN(4);
		__cp_bss_sram_end = .;
	} > RAMCP

	__overlay_text_start_flash__ = __cp_sram_end_flash__;
#else
	__overlay_text_start_flash__ = __fast_sram_text_data_end_flash__;
#endif

#ifdef OVERLAY_FOR_CP
	__overlay_text_start__ = ALIGN(__cp_text_sram_end, 32);
#else
	__overlay_text_start__ = ALIGN(__fast_sram_text_data_end__, 32);
#endif
	__overlay_text_exec_start__ = RAM_TO_RAMX(__overlay_text_start__);

	OVERLAY (__overlay_text_exec_start__) : NOCROSSREFS AT (__overlay_text_start_flash__)
	{
#ifndef NO_OVERLAY
		.overlay_text0
		{
			LONG(0);
			*:smallft.o(.text*)
			*:lib_a-qsort.o(.text*)
#ifndef KARAOKE_ALGO_NS
			*:filterbank_pflt.o(.text*)
#endif
			*:integer_delay.o(.text*)
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
			*:bt_sco_chain.o( .text.speech_tx_process)
			*:bt_sco_chain.o( .text.speech_rx_process)
			*:bt_sco_chain.o( .text._speech_tx_process_pre)
			*:bt_sco_chain.o( .text._speech_rx_process_pre)
			*:bt_sco_chain_cp.o(.text*)
#endif
#ifndef KARAOKE_ALGO_NS
			*:cmsis_fftwrap.o(.text*)
#if !defined(GLOBAL_SRAM_CMSIS_FFT)
			*:cmsis_dsp.o(.text.arm_radix8_butterfly_f32)
			*:cmsis_dsp.o(.text.arm_cfft_radix8by2_f32)
			*:cmsis_dsp.o(.text.arm_cfft_radix8by4_f32)
			*:cmsis_dsp.o(.text.arm_cfft_f32)
			*:cmsis_dsp.o(.text.stage_rfft_f32)
			*:cmsis_dsp.o(.text.merge_rfft_f32)
			*:cmsis_dsp.o(.text.arm_rfft_fast_f32)
#endif
#endif
#ifndef KARAOKE_ALGO_EQ
			*:cmsis_dsp.o(.text.arm_power_f32)
#if !defined(__VIRTUAL_SURROUND__)
			*:cmsis_dsp.o(.text.arm_copy_f32)
#endif
#ifndef AUDIO_BASS_ENHANCER
			*:cmsis_dsp.o(.text.arm_biquad_cascade_df1_ex_f32)
#endif
#endif

#if defined(SPEECH_TX_2MIC_NS4)
			*:sensormic_denoise.o(.text*)
#endif
#if (defined(SPEECH_TX_NS2FLOAT) || defined(SPEECH_RX_NS2FLOAT)) && !defined(KARAOKE_ALGO_NS)
			*:lc_mmse_ns_float.o(.text*)
#endif
#if defined(SPEECH_TX_NS3) || defined(SPEECH_RX_NS3) || defined(SPEECH_TX_2MIC_NS4)
			*:ns3.o(.text*)
			*:noise_suppression_x.o(.text*)
			*:nsx_core.o(.text*)
			*:nsx_core_c.o(.text*)
			*:noise_suppression.o(.text*)
			*:ns_core.o(.text*)
			*:copy_set_operations.o(.text*)
			*:cross_correlation.o(.text*)
			*:division_operations.o(.text*)
			*:dot_product_with_scale.o(.text*)
			*:downsample_fast.o(.text*)
			*:energy.o(.text*)
			*:get_scaling_square.o(.text*)
			*:min_max_operations.o(.text*)
			*:real_fft.o(.text*)
			*:resample_by_2.o(.text*)
			*:resample_by_2_internal.o(.text*)
			*:spl_init.o(.text*)
			*:spl_sqrt.o(.text*)
			*:spl_sqrt_floor.o(.text*)
			*:vector_scaling_operations.o(.text*)
#endif

#if defined(SPEECH_TX_1MIC_NS) || defined(SPEECH_TX_2MIC_NS8) || defined(SPEECH_TX_3MIC_NS)
			*:beco.o(.text*)
			*:speech_1mic_ns.o(.text*)
			/* stft */
			*:speech_nn_stft.o(.text*)
			*:speech_win.o(.text*)
			/* nn */
			*:speech_nn_ns95l.o(.text*)
			*:speech_nn_ns94l.o(.text*)
			*:speech_nn_ns93l.o(.text*)
			*:speech_nn_ns91s.o(.text*)
			*:speech_nn_ns95s.o(.text*)
			*:speech_nn_ns100l.o(.text*)
			*:speech_nn_ns101l.o(.text*)
			*:speech_nn_ns102l.o(.text*)
			*:filterbank_pflt.o(.text*)
			*:lcmmse.o(.text*)
			*:logmmse.o(.text*)
			/* layer */
			*:besnet_add_layer.o(.text*)
			*:besnet_base_layer.o(.text*)
			*:besnet_common.o(.text*)
			*:besnet_conv1x1_layer.o(.text*)
			*:besnet_gru_layer.o(.text*)
			*:besnet_instance_norm_layer.o(.text*)
			*:besnet_instance_norm1_layer.o(.text*)
			*:besnet_prelu_layer.o(.text*)
			*:besnet_separable_conv_layer.o(.text*)
			*:besnet_separable_transposed_conv_layer.o(.text*)
			/* kernel */
			*:arm_instance_norm_f32.o(.text*)
			*:arm_depthwise_separable_conv_ns_s16.o(.text*)
			*:arm_depthwise_transposed_conv_ns_s16.o(.text*)
#if defined(SPEECH_NS_BECO)
			*:beco_support_functions.o(.text*)
			*:beco_convolve_1x1_ns_s16.o(.text*)
			*:beco_convolve_1x1_norelu_ns_s16.o(.text*)
			*:beco_convolve_1x1och1_ns_s16.o(.text*)
			*:beco_fully_connected_ns_s16.o(.text*)
			*:beco_gru_ns_s16.o(.text*)
#endif
#if defined(SPEECH_NS_CMSIS)
			*:arm_convolve_1x1_ns_s16.o(.text*)
			*:arm_convolve_1x1och1_ns_s16.o(.text*)
			*:arm_fully_connected_ns_s16.o(.text*)
			*:arm_gru_ns_s16.o(.text*)
#endif
#if defined(SPEECH_NS_NAIVE)
			*:naive_convolve_1x1_ns_s16.o(.text*)
			*:naive_convolve_1x1och1_ns_s16.o(.text*)
			*:naive_fully_connected_ns_s16.o(.text*)
			*:naive_gru_ns_s16.o(.text*)
#endif
			/* others */
			*:spectral_wdrc.o(.text*)
			*:fastmath_approx.o(.text*)
			*:arm_nn_activations_q15.o(.text*)
			*:arm_nntables.o(.text*)
			*:cmsis_dsp.o(.text.arm_scale_q15)
			*:cmsis_dsp.o(.text.arm_mat_trans_q15)
			*:cmsis_dsp.o(.text.arm_add_q15)
			*:cmsis_dsp.o(.text.arm_mult_q15)
			*:cmsis_dsp.o(.text.arm_offset_q15)
			*:cmsis_dsp.o(.text.arm_negate_q15)
			*:cmsis_dsp.o(.text.arm_scale_f32)
			*:cmsis_dsp.o(.text.arm_mean_f32)
			*:cmsis_dsp.o(.text.arm_cmplx_mag_f32)
			*:cmsis_dsp.o(.text.arm_dot_prod_f32)
			*:cmsis_dsp.o(.text.arm_add_f32)
			*:cmsis_dsp.o(.text.arm_fill_f32)
#endif

#if defined(SPEECH_TX_NS4) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_AEC2FLOAT)
			*:speech_ns4.o(.text*)
			*:speech_ns4_core.o(.text*)
#endif
#if defined(SPEECH_TX_2MIC_NS7)
			/*:integer_delay.o(.text*)*/
			*:noise_suppression.o(.text*)
			*:ns_core.o(.text*)
			*:speech_2mic_ns7.o(.text*)
			*:spectral_wdrc.o(.text*)
			*:base_layer.o(.text*)
			*:logmmse.o(.text*)
			*:lcmmse.o(.text*)
			*:besnet_utils.o(.text*)
			*:dw_conv_layer.o(.text*)
			*:dw_deconv_layer.o(.text*)
			*:fully_connected_layer.o(.text*)
			*:gru_layer.o(.text*)
			*:output_layer.o(.text*)
			*:speech_ns7.o(.text*)
			*:filterbank_pflt.o(.text*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.text*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.text*)
			*:arm_depthwise_transposed_conv2x4_q15_q7.o(.text*)
			*:arm_fully_connected_q15_q7_q31.o(.text*)
			*:cmsis_dsp.o(.text.arm_mat_trans_q15)
			*:cmsis_dsp.o(.text.arm_mat_inverse_f64)
			*:arm_depthwise_transposedconv_2x4_q15_q7.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:coherence.o(.text*)
			*:mdf2.o(.text*)
			*:noise_generator.o(.text*)
			*:pink_noise_gen.o(.text*)
			*:white_noise_gen.o(.text*)
			*:rls.o(.text*)
			*:stft.o(.text*)
			*:coherence.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:wind_detection.o(.text*)
			*:cmsis_dsp.o(.text.arm_scale_f32)
			*:floatlimiter.o(.text.ApplyFloatLimiter)
#endif
#if defined(SPEECH_TX_2MIC_NS8)
			*:speech_2mic_ns8.o(.text*)
			*:fixed_beamforming.o(.text*)
			*:stft.o(.text*)
			*:rls.o(.text*)
			*:coherence.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:spectral_wdrc.o(.text*)
			*:wind_detection.o(.text*)
			*:cmsis_dsp.o(.text.arm_scale_f32)
			*:floatlimiter.o(.text.ApplyFloatLimiter)
			*:cmsis_dsp.o(.text.arm_fill_q31)
			*:cmsis_dsp.o(.text.arm_absmax_f32)
#endif
#ifndef KARAOKE_ALGO_NS
			*:cmsis_fftwrap.o(.text*)
			*:arm_rfft_fast_f32.o(.text*)
			*:arm_cfft_f32.o(.text*)
#endif
#if defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_2MIC_NS4)
			*:rnn.o(.text*)
			*:bes_fully_connected_mat_q7_vec_f32.o(.text.bes_fully_connected_mat_q7_vec_f32_no_bias)
			*:bes_fully_connected_mat_q7_vec_f32.o(.text.bes_fully_connected_mat_q7_vec_f32)
			*:arm_fully_connected_q7.o(.text.arm_fully_connected_q7_bes_q31)
			*:arm_q7_to_q15_reordered_no_shift.o(.text.arm_q7_to_q15_reordered_no_shift)
#endif
#if (defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_2MIC_NS4)) && !defined(AUDIO_ANC_FB_ADJ_MC) && !defined(AUDIO_ADJ_EQ)
			*:coherence.o(.text*)
			*:echo_canceller.o(.text*)
			*:echo_suppression.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:lcmmse.o(.text*)
			*:logmmse.o(.text*)
			*:mdf.o(.text*)
			*:mdf1.o(.text*)
			*:mdf2.o(.text*)
			*:mdf3.o(.text*)
			*:ssp_aec.o(.text*)
			*:noise_generator.o(.text*)
			*:pink_noise_gen.o(.text*)
			*:white_noise_gen.o(.text*)
#endif
#if defined(SPEECH_TX_3MIC_NS)
			*:coherence.o(.text*)
			*:echo_suppression.o(.text*)
			*:echo_suppression_core.o(.text*)
			*:lcmmse.o(.text*)
			*:logmmse.o(.text*)
			*:mdf5.o(.text*)
			*:speech_3mic_ns.o(.text*)
			*:filterbank_pflt.o(.text*)
			*:lib_a-sf_frexp.o(.text*)
			*:libm.a(.text*)
			*:spectral_wdrc.o(.text*)
			*:fixed_beamforming.o(.text*)
			*:ns3.o(.text*)
			*:noise_suppression_x.o(.text*)
			*:nsx_core.o(.text*)
			*:nsx_core_c.o(.text*)
			*:noise_suppression.o(.text*)
			*:ns_core.o(.text*)
			*:stft.o(.text*)
			*:rls.o(.text*)
			*:wind_detection_in_3mic.o(.text*)
			*:spectral_wdrc.o(.text*)
			*:cmsis_dsp.o(.text.arm_scale_f32)
			*:cmsis_dsp.o(.text.arm_fill_q31)
#if !defined(AUDIO_LIMITER)
			*:floatlimiter.o(.text.ApplyFloatLimiter)
#endif
#endif
#if defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_3MIC_NS)
			*:fftfilt.o(.text*)
#endif
#if defined(SPEECH_TX_COMPEXP)&& !defined(AUDIO_ADJ_EQ)
			*:compexp.o(.text*)
#endif
#if (defined(SPEECH_TX_EQ) || defined(SPEECH_RX_EQ) || defined(SPEECH_CODEC_FIXED_SAMPLE_RATE)) && !defined(KARAOKE_ALGO_EQ) && !defined(AUDIO_BASS_ENHANCER)
			*:speech_eq.o(.text*)
			*:speech_arm_eq.o(.text*)
			*:arm_biquad_cascade_df1_f32.o(.text*)
			*:arm_biquad_cascade_df1_ex_f32.o(.text*)
#endif
#if defined(SPEECH_TX_POST_GAIN) || defined(SPEECH_RX_POST_GAIN)
			*:speech_gain.o(.text*)
#endif

			*(.overlay_text0)
			. = ALIGN(4);
		}
		.overlay_text1
		{
			LONG(0);
#if !defined(RB_CODEC)
			*:fixedpoint.o(.text*)
			*:replaygain.o(.text*)
			*:dsp_core.o(.text*)
			*:dsp_misc.o(.text*)
			*:dsp_filter.o(.text*)
			*:dsp_sample_input.o(.text*)
			*:dsp_sample_output.o(.text*)
			*:eq.o(.text*)
			*:pga.o(.text*)
#if !defined(A2DP_AAC_ON) && !defined(A2DP_LHDC_ON)
			*:fir_process.o(.text*)
			*:iir_process.o(.text*.iir_run .text*.*iir_run_per_*)
#endif
#endif
			*:a2dp_decoder_sbc.o(.text*)
			*(.overlay_a2dp_sbc*)
			*(.overlay_text1)
			. = ALIGN(4);
		}
		.overlay_text2
		{
			LONG(0);
#if 0
		    *:lc3_com.o(.text*)
		    *:lc3_dec.o(.text*)
		    *:lc3_enc.o(.text*)
#endif
		    *(.overlay_a2dp_lc3*)
			*(.overlay_text2)
			. = ALIGN(4);
		}
		.overlay_text3
		{
			LONG(0);
			*:fft128dot.o(.text*)
			*(.overlay_text3)
			. = ALIGN(4);
		}
		.overlay_text4
		{
			LONG(0);
#if !defined(PROMPT_USE_AAC)
#ifdef A2DP_SOURCE_AAC_ON
			*:adj_thr.o(.text.*FDKaacEnc_AdjThrClose*)
			*:bit_cnt.o(.text*)
			*:quantize.o(.text*FDKaacEnc_quantizeLines*)
			*:adj_thr.o(.text*CalcRedValPower*)
			*:tpenc_lib.o(.text.*transportEnc_GetStaticBits*)
			*:tpenc_latm.o(.text.*transportEnc_LatmCountTotalBitDemandHeader*)
			*:tpenc_lib.o(.text.*transportEnc_GetBitstream*)
			*:line_pe.o(.text.*FDKaacEnc_calcSfbPe*)
			*:adj_thr.o(.text.*FDKaacEnc_calcPe*)
			*:spreading.o(.text.*FDKaacEnc_SpreadingMax*)
			*:bitenc.o(.text.*FDKaacEnc_writeExtensionData*)
			*:bitenc.o(.text.*FDKaacEnc_ChannelElementWrite*)
			*:qc_main.o(.text.*FDKaacEnc_getTotalConsumedBits*)
			*:adj_thr.o(.text.*FDKaacEnc_reduceThresholdsCBR*)
			*:band_nrg.o(.text.*FDKaacEnc_CalcSfbMaxScaleSpec*)
			*:aacenc_tns.o(.text.*FDKaacEnc_AutoToParcor*)
			*:aacenc_tns.o(.text.*FDKaacEnc_Parcor2Index*)
			*:aacenc_tns.o(.text.*FDKaacEnc_TnsDetect*)
			*:aacenc_tns.o(.text.*FDKaacEnc_TnsEncode*)
			*:pre_echo_control.o(.text.*FDKaacEnc_PreEchoControl*)
			*:transform.o(.text.*FDKaacEnc_Transform_Real*)
			*:band_nrg.o(.text.*FDKaacEnc_CheckBandEnergyOptim*)
			*:tpenc_latm.o(.text.*AdvanceAudioMuxElement*)
			*:aacenc.o(.text.*FDKaacEnc_EncodeFrame*)
			*:aacenc_pns.o(.text.*FDKaacEnc_PnsDetect*)
#else
#if defined(A2DP_AAC_ON)
			*:block.o(.text*.*CBlock_GetEscape*)
			*:tpdec_lib.o(.text*.*transportDec_GetBitstream*)
			*:tpdec_lib.o(.text*.*transportDec_GetAuBitsRemaining*)
			*:tpdec_latm.o(.text*.*CLatmDemux_GetValue*)
			*:tpdec_latm.o(.text*.*CLatmDemux_Read*)
			*:tpdec_lib.o(.text*.*transportDec_FillData*)
			*:tpdec_lib.o(.text*.*transportDec_ReadAccessUnit*)
			*:tpdec_lib.o(.text*.*transportDec_readHeader*)
			*:block.o(.text*.*ApplyTools*)
			*:aacdec_pns.o(.text*.*CPns_Apply*)
			*:aacdec_tns.o(.text*.*CTns_Apply*)
			*:aacdec_tns.o(.text*.*CTns_Reset*)
			*:aacdec_pns.o(.text*.*CPns_ResetData*)
			*:pulsedata.o(.text*.*CPulseData_Read*)
#if !defined(AAC_TEXT_PARTIAL_IN_FLASH)
			*:channel.o(.text*.*CChannelElement_Read*)
			*:tpdec_asc.o(.text*.*CProgramConfig_Reset*)
			*:tpdec_lib.o(.text*.*transportDec_CrcCheck*)
			*:block.o(.text*.*CBlock_FrequencyToTime*)
			*:block.o(.text*.*CBlock_ReadSectionData*)
			*:channel.o(.text*.*CChannelElement_Decode*)
			*:tpdec_lib.o(.text*.*transportDec_CrcEndReg*)
			*:aacdecoder.o(.text*.*CAacDecoder_DecodeFrame*)
			*:aac_mdct.o(.text*)
#endif

#if !defined(__SBC_FUNC_IN_ROM_VBEST2000__) && !defined(AAC_TEXT_PARTIAL_IN_FLASH)
			*:block.o(.text*.*CBlock_ReadSpectralData*)
			*:block.o(.text*.*CBlock_ScaleSpectralData*)
			*:aacdec_tns.o(.text*.*CTns_ReadDataPresentFlag*)
			*:tpdec_lib.o(.text*.*transportDec_CrcStartReg*)
			*:block.o(.text*.*CBlock_ReadScaleFactorData*)
			*:tpdec_lib.o(.text*.*transportDec_EndAccessUnit*)
			*:tpdec_latm.o(.text*.*CLatmDemux_GetNrOfSubFrames*)
			*:tpdec_lib.o(.text*.*transportDec_GetAuBitsTotal*)
			*:tpdec_asc.o(.text*.*CProgramConfig_LookupElement*)
			*:tpdec_latm.o(.text*.*CLatmDemux_GetFrameLengthInBits*)
			*:tpdec_latm.o(.text*.*CLatmDemux_ReadPayloadLengthInfo*)
			*:block.o(.text*.*CBlock_InverseQuantizeSpectralData*)
			*:channelinfo.o(.text*.*IcsRead*)
			*:aacdec_tns.o(.text*.*CTns_Read*)
			*:tpdec_lib.o(.text*.*transportDec_AdjustEndOfAccessUnit*)
			*:pulsedata.o(.text*.*CPulseData_Apply*)
			*:tpdec_asc.o(.text*.*AudioSpecificConfig_Init*)
			*:tpdec_asc.o(.text*.*AudioSpecificConfig_Parse*)
			*:tpdec_latm.o(.text*.*CLatmDemux_ReadStreamMuxConfig*)
			*:tpdec_asc.o(.text*.*getSampleRate*)
			*:tpdec_asc.o(.text*.*getAOT*)
			*:aacdecoder.o(.text*.*CAacDecoder_Init*)
			*:tpdec_asc.o(.text*.*CProgramConfig_Init*)
			*:aacdecoder_lib.o(.text*.*aacDecoder_ConfigCallback*)
			*:aacdecoder.o(.text*.*CAacDecoder_ExtPayloadParse*)
			*:stereo.o(.text*.*CJointStereo_Read*)
#endif
#endif
#if defined(AAC_BECO_FFT)
			*:fdkaac_beco_fft.o(.text*)
#endif
			*:aac_com.o(.text*)
			*:aac_enc.o(.text*)
			*:aac_dec.o(.text*)
			*:aac_api.o(.text*)
#endif
			*:a2dp_decoder_aac_lc.o(.text*.*a2dp_audio_aac_lc_decoder_init*)
			*:a2dp_decoder_aac_lc.o(.text*.*a2dp_audio_aac_lc_decoder_reinit*)
			*:a2dp_decoder_aac_lc.o(.text*.*a2dp_audio_aac_lc_channel_select*)
			*:a2dp_decoder_aac_lc.o(.text*.*aac_malloc*)
			*:a2dp_decoder_aac_lc.o(.text*.*aac_free*)
#endif
			*(.overlay_a2dp_aac*)
			*(.overlay_text4)
			. = ALIGN(4);
		}
		.overlay_text5
		{
			LONG(0);
			*:mpa.o(.text*)
			*:bit.o(.text*)
			*:frame.o(.text*)
			*:huffman.o(.text*)
			*:layer12.o(.text*)
			*:layer3.o(.text*)
			*:stream.o(.text*)
			*:synth.o(.text*)
			*(.overlay_text5)
			. = ALIGN(4);
		}
		.overlay_text6
		{
			LONG(0);
			*:mp3_dec.o(.text*)
			*:mp3_enc.o(.text*)
#if defined(A2DP_SCALABLE_ON)
			*:rc_decode.o(.text*)
			*:audio_int32.o(.text*)
			*:ssc_ifft.o(.text*)
			*:vector_quant.o(.text*)
			*:ssc_quant.o(.text*)
			*:ssc_pulsealloc.o(.text*)
			*:audio.o(.text*)
			*:set_sbm.o(.text*)
			*:rc_code.o(.text*)
			*:SBM_pitchBased.o(.text*)
			*:ssc_imdct.o(.text*)
			*:audio_decoder.o(.text*)
			*:SoundBooster_DRC_classic.o(.text*)
			*:ssc_cwrs.o(.text*)
			*:ssc_mode.o(.text*)
			*:ssc_math.o(.text*)
			*:ssc_lap.o(.text*)
			*:SeamlessBufferManagement.o(.text*)
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_scalable.o(.text*)
#endif
			*(.overlay_a2dp_ssc*)
#endif

#if defined(A2DP_LHDC_ON)
			*(.overlay_a2dp_lhdcv3*)
			/* LLAC */
			*:llac_dec_alg.o(.text*)
			*:llac_dec_alg_code.o(.text*)
			*:llac_dec_alg_mdct.o(.text*)
			*:llac_dec_alg_quant.o(.text*)
			*:llac_dec_alg_signal.o(.text*)
			/* *:llac_dec_const.o(.text*) */
			*:llac_dec_init.o(.text*)
			*:llac_dec_mem_alloc.o(.text*)
			*:llac_dec_wrapper.o(.text*)
			*:sv_math.o(.text*)
			*:uhead_api.o(.text*)

			/* LHDC */
			*:a2dp_decoder_lhdc.o(.text*)
			*:testkey.o(.text*)
			*:lhdcUtil.o(.text*)
			*:lzh_dec.o(.text*)
			*:licdecrypt.o(.text*)
			*:lic_check.o(.text*)
			*:fast_upsampling_2x.o(.text*)
			*:encdec.o(.text*)
			*:dec.o(.text*)
			*:cirbuf.o(.text*)
			*:DecodeUps2xLPF.o(.text*)
			/**:arm_biquad_cascade_df2T_init_f32.o(.text*)*/
#endif
#if defined(A2DP_SOURCE_LHDC_ON)
			*(.overlay_a2dp_lhdc_encoder*)
			*:EncodeLPF.o(.text*)
			*:enc.o(.text*)
			*:encdec.o(.text*)
			*:lhdc_enc_api.o(.text*)
			*:lhdc_process.o(.text*)
			*:lic_check.o(.text*)
			*:licdecrypt.o(.text*)
			*:lzh_enc.o(.text*)
			*:win_sinc.o(.text*)
			*:a2dp_encoder_lhdc_cp.o(.text*)
#endif
			*(.overlay_text6)
			. = ALIGN(4);
		}
		.overlay_text7
		{
			LONG(0);
#if defined(A2DP_LDAC_ON)
			*:a2dp_decoder_ldac.o(.text*)
			*:bitalloc_ldac.o(.text*)
			*:bitalloc_sub_ldac.o(.text*)
			*:encode_ldac.o(.text*)
			*:ldacBT.o(.text*)
			*:ldaclib.o(.text*)
			*:pack_ldac.o(.text*)
			*:quant_ldac.o(.text*)
			*:mdct_ldac.o(.text*)
			*:ldacBT_api.o(.text*)
			*:decode_ldac.o(.text*)
			*:dequant_ldac.o(.text*)
			*:imdct_ldac.o(.text*)
			*:ldacBT_internal.o(.text*)
			*:ldaclib_api.o(.text*)
			*:memory_ldac.o(.text*)
			*:setpcm_ldac.o(.text*)
			*:sigana_ldac.o(.text*)
			*:tables_ldac.o(.text*)
			*:tables_sigproc_ldac.o(.text*)
			*:unpack_ldac.o(.text*)
			*(.overlay_a2dp_ldac*)
#endif
			*(.overlay_text7)
			. = ALIGN(4);
		}
		.overlay_text8
		{
			LONG(0);
#if defined(A2DP_LHDCV5_ON)
			/* LHDC V5*/
			*(.overlay_a2dp_lhdcv5*)
			*:a2dp_decoder_lhdcv5.o(.text*)
			*:lhdc_v5_kiss_fft.o(.text*)
			*:lhdc_v5_dec_process.o(.text*)
			*:lhdcv5_util_dec.o(.text*)
			*:lhdc_v5_dec_time_process.o(.text*)
			*:lhdc_v5_global_table.o(.text*)
			*:svlice_v5_encdec.o(.text*)
			*:lhdc_v5_arith.o(.text*)
			*:svlice_v5_lic_check.o(.text*)
			*:lhdc_v5_dec_workspace.o(.text*)
			*:lhdc_v5_level.o(.text*)
			*:lhdc_log.o(.text*)
			*:lhdc_v5_dec_header.o(.text*)
			*:lhdc_v5_accelerate.o(.text*)
			*:lhdc_v5_bit_inputter.o(.text*)
			*:svlice_v5_licdecrypt.o(.text*)
			*:lhdc_v5_freq_shift.o(.text*)
#endif
			*(.overlay_text8)
			. = ALIGN(4);
		}
		.overlay_text9
		{
			LONG(0);
#if defined(OPUS_OPT_OVERLAY)
			/**:voice_opus.o(.text*)*/
			*:opus_encoder.o(.text*)
			*:analysis.o(.text*)
			/**:celt_encoder.o(.text*)*/
			*:celt_encoder.o(.text*.opus_custom_encoder_ctl)
			*:celt_encoder.o(.text*.celt_encoder_get_size)
			*:celt_encoder.o(.text*.opus_custom_encoder_get_size)
			*:celt_encoder.o(.text*.celt_encoder_init)
			*:celt_encoder.o(.text*.opus_custom_encoder_init_arch)
			*:celt_encoder.o(.text*.transient_analysis)
			*:celt_encoder.o(.text*.patch_transient_decision)
			*:celt_encoder.o(.text*.celt_preemphasis)
			*:celt_encoder.o(.text*.l1_metric)
			*:celt_encoder.o(.text*.tf_analysis)
			*:celt_encoder.o(.text*.tf_encode)
			*:celt_encoder.o(.text*.alloc_trim_analysis)
			*:celt_encoder.o(.text*.stereo_analysis)
			*:celt_encoder.o(.text*.median_of_5)
			*:celt_encoder.o(.text*.median_of_3)
			*:celt_encoder.o(.text*.dynalloc_analysis)
			*:celt_encoder.o(.text*.run_prefilter)
			*:celt_encoder.o(.text*.compute_vbr)
			*:celt_encoder.o(.text*.celt_encode_with_ec)
			*:celt_encoder.o(.text*.opus_custom_encoder_ctl)
			*:repacketizer.o(.text*)
			*:opus.o(.text*)
			*:enc_API.o(.text*)
			*:entenc.o(.text*)
			*:control_audio_bandwidth.o(.text*)
			*:encode_pulses.o(.text*)
			*:LPC_analysis_filter.o(.text*)
			*:burg_modified_FIX.o(.text*)
			*:vector_ops_FIX.o(.text*)
			*:NLSF_del_dec_quant.o(.text*)
			*:encode_frame_FIX.o(.text*)
			*:pitch.o(.text*)
			*:resampler.o(.text*)
			*:VAD.o(.text*)
			*:encode_indices.o(.text*)
			*:control_codec.o(.text*)
			*:celt_lpc.o(.text*)
			*:shell_coder.o(.text*)
			*:init_encoder.o(.text*)
			*:log2lin.o(.text*)
			*:biquad_alt.o(.text*)
			*:stereo_encode_pred.o(.text*)
			*:NLSF_unpack.o(.text*)
			*:code_signs.o(.text*)
			*:lin2log.o(.text*)
			*:HP_variable_cutoff.o(.text*)
			*:sum_sqr_shift.o(.text*)
			*:inner_prod_aligned.o(.text*)
			*:stereo_quant_pred.o(.text*)
			*:entcode.o(.text*)
			*:celt.o(.text*)
			*:laplace.o(.text*)
			*:NSQ.o(.text*) /*complexity 1*/
			*:NSQ_del_dec.o(.text*) /*complexity 2*/
#else
			*:opus_com.o(.text*)
#endif
			*(.overlay_text9)
			. = ALIGN(4);
		}
		.overlay_text10
		{
			LONG(0);
			*:ogg_com.o(.text*)
			*:vorbis_com.o(.text*)
			*(.overlay_text10)
			. = ALIGN(4);
		}
		.overlay_text11
		{
			LONG(0);
			*:alac_com.o(.text*)
			*(.overlay_text11)
			. = ALIGN(4);
		}
		.overlay_text12
		{
			LONG(0);
			*:flac_com.o(.text*)
			*(.overlay_text12)
			. = ALIGN(4);
		}
		.overlay_text13
		{
			LONG(0);
			*(.overlay_text13)
			. = ALIGN(4);
		}
#endif
		.overlay_text_last
		{
			LONG(0);
			. = ALIGN(4);
		}
	} > REGION_OVERLAY_TEXT

	.dummy_overlay_text_end (.) (NOLOAD) : AT (ADDR(.dummy_overlay_text_end))
	{
		__overlay_text_exec_end__ = .;
	} > REGION_OVERLAY_TEXT

#ifdef OVERLAY_FOR_CP
	__fast_sram_end__ = __fast_sram_text_data_end__;
#else
	__fast_sram_end__ = RAMX_TO_RAM(__overlay_text_exec_end__);
#endif

#ifdef MAIN_RAM_USE_TCM
	__ramx_end = __fast_sram_end__;
	__ramx_top = ORIGIN(RAMX) + LENGTH(RAMX);
	__free_ramx = __ramx_top - __ramx_end;
#else
	ASSERT(__fast_sram_end__ - __fast_sram_text_data_start__ <= FAST_XRAM_SECTION_SIZE, "fast xram sections too large")
	__free_fram = FAST_XRAM_SECTION_SIZE - (__fast_sram_end__ - __fast_sram_text_data_start__);
#endif

#ifdef OVERLAY_FOR_CP
	__overlay_data_start__ = __cp_bss_sram_end;
#else
	__overlay_data_start__ = __sram_bss_end__;
#endif

	OVERLAY (__overlay_data_start__) : NOCROSSREFS AT (__load_stop_overlay_text_last)
	{
#ifndef NO_OVERLAY
		.overlay_data0
		{
			/* Explicitly place 4 bytes at section start to avoid "section type changed to PROGBITS" warnings */
			LONG(0);
			*:smallft.o(.data* .rodata*)
			*:lib_a-qsort.o(.data* .rodata*)
			*:integer_delay.o(.data* .rodata*)
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
			*:bt_sco_chain.o(.data* .rodata*)
			*:bt_sco_chain_cp.o(.data* .rodata*)
#endif
			*:cmsis_dsp.o(.rodata.armBitRevIndexTable256)
			*:cmsis_dsp.o(.rodata.twiddleCoef_256)
			*:cmsis_dsp.o(.rodata.twiddleCoef_rfft_512)
#if defined(SPEECH_TX_2MIC_NS4)
			*:sensormic_denoise.o(.data* .rodata*)
			*:echo_suppression_core.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_NS3) || defined(SPEECH_RX_NS3) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_3MIC_NS)
			*:nsx_core.o(.data* .rodata*)
			*:ns_core.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_NS4) || defined(SPEECH_TX_2MIC_NS4) || defined(SPEECH_TX_AEC2FLOAT)
			*:speech_ns4.o(.data* .rodata*)
			*:speech_ns4_core.o(.data* .rodata*)
#endif


#if defined(SPEECH_TX_1MIC_NS) || defined(SPEECH_TX_2MIC_NS8) || defined(SPEECH_TX_3MIC_NS)
			*:beco.o(.data* .rodata*)
			*:speech_1mic_ns.o(.data* .rodata*)
			/* stft */
			*:speech_nn_stft.o(.data* .rodata*)
			*:speech_win.o(.data* .rodata*)
			/* nn */
			*:speech_nn_ns95l.o(.data* .rodata*)
			*:speech_nn_ns94l.o(.data* .rodata*)
			*:speech_nn_ns93l.o(.data* .rodata*)
			*:speech_nn_ns91s.o(.data* .rodata*)
			*:speech_nn_ns95s.o(.data* .rodata*)
			*:speech_nn_ns100l.o(.data* .rodata*)
			*:speech_nn_ns101l.o(.data* .rodata*)
			*:speech_nn_ns102l.o(.data* .rodata*)
			*:filterbank_pflt.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			/* layer */
			*:besnet_add_layer.o(.data* .rodata*)
			*:besnet_base_layer.o(.data* .rodata*)
			*:besnet_common.o(.data* .rodata*)
			*:besnet_conv1x1_layer.o(.data* .rodata*)
			*:besnet_gru_layer.o(.data* .rodata*)
			*:besnet_instance_norm_layer.o(.data* .rodata*)
			*:besnet_instance_norm1_layer.o(.data* .rodata*)
			*:besnet_prelu_layer.o(.data* .rodata*)
			*:besnet_separable_conv_layer.o(.data* .rodata*)
			*:besnet_separable_transposed_conv_layer.o(.data* .rodata*)
			/* kernel */
			*:arm_instance_norm_f32.o(.data* .rodata*)
			*:arm_depthwise_separable_conv_ns_s16.o(.data* .rodata*)
			*:arm_depthwise_transposed_conv_ns_s16.o(.data* .rodata*)
#if defined(SPEECH_NS_BECO)
			*:beco_support_functions.o(.data* .rodata*)
			*:beco_convolve_1x1_ns_s16.o(.data* .rodata*)
			*:beco_convolve_1x1_norelu_ns_s16.o(.data* .rodata*)
			*:beco_convolve_1x1och1_ns_s16.o(.data* .rodata*)
			*:beco_fully_connected_ns_s16.o(.data* .rodata*)
			*:beco_gru_ns_s16.o(.data* .rodata*)
#endif
#if defined(SPEECH_NS_CMSIS)
			*:arm_convolve_1x1_ns_s16.o(.data* .rodata*)
			*:arm_convolve_1x1och1_ns_s16.o(.data* .rodata*)
			*:arm_fully_connected_ns_s16.o(.data* .rodata*)
			*:arm_gru_ns_s16.o(.data* .rodata*)
#endif
#if defined(SPEECH_NS_NAIVE)
			*:naive_convolve_1x1_ns_s16.o(.data* .rodata*)
			*:naive_convolve_1x1och1_ns_s16.o(.data* .rodata*)
			*:naive_fully_connected_ns_s16.o(.data* .rodata*)
			*:naive_gru_ns_s16.o(.data* .rodata*)
#endif
			/* others */
			*:spectral_wdrc.o(.data* .rodata*)
			*:fastmath_approx.o(.data* .rodata*)
			*:arm_nn_activations_q15.o(.data* .rodata*)
			*:arm_nntables.o(.data* .rodata*)
#endif

#if defined(SPEECH_TX_2MIC_NS7)
			/*:integer_delay.o(.data* .rodata*)*/
			*:ns_core.o(.data* .rodata*)
			*:noise_suppression.o(.data* .rodata*)
			*:speech_2mic_ns7.o(.data* .rodata*)
			/*:rnn.o(.data* .rodata*)*/
			*:spectral_wdrc.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:besnet_utils.o(.data* .rodata*)
			*:dw_conv_layer.o(.data* .rodata*)
			*:dw_deconv_layer.o(.data* .rodata*)
			*:fully_connected_layer.o(.data* .rodata*)
			*:gru_layer.o(.data* .rodata*)
			*:output_layer.o(.data* .rodata*)
			*:speech_ns7.o(.data* .rodata*)
			*:filterbank_pflt.o(.data* .rodata*)
			*:arm_depthwise_separable_conv2x4_q15_q7_q31.o(.data* .rodata*)
			*:arm_convolve_1x1_HWC_q15_q7_q31.o(.data* .rodata*)
			*:arm_depthwise_transposedconv_2x4_q15_q7.o(.data* .rodata*)
			*:rls.o(.data* .rodata*)
			*:stft.o(.data* .rodata*)
			*:coherence.o(.data* .rodata*)
			*:echo_suppression_core.o(.data* .rodata*)
			*:wind_detection.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_2MIC_NS8)
			*:speech_2mic_ns8.o(.data* .rodata*)
			*:fixed_beamforming.o(.data* .rodata*)
			*:stft.o(.data* .rodata*)
			*:rls.o(.data* .rodata*)
			*:coherence.o(.data* .rodata*)
			*:echo_suppression_core.o(.data* .rodata*)
			*:spectral_wdrc.o(.data* .rodata*)
			*:wind_detection.o(.data* .rodata*)
			*:cmsis_dsp.o(.data*.arm_fill_q31)
			*:cmsis_dsp.o(.data*.arm_absmax_f32)
#endif
#ifndef KARAOKE_ALGO_NS
			*:cmsis_fftwrap.o(.rodata*)
			*:arm_rfft_fast_f32.o(.rodata*)
			*:arm_cfft_f32.o(.rodata*)
#endif
#if defined(SPEECH_TX_AEC2FLOAT) || defined(SPEECH_TX_2MIC_NS4)
			*:rnn.o(.data* .rodata*)
#endif
#if (defined(SPEECH_TX_NS2FLOAT) || defined(SPEECH_RX_NS2FLOAT)) && !defined(KARAOKE_ALGO_NS)
			*:lc_mmse_ns_float.o(.data* .rodata*)
#endif
#if defined(SPEECH_TX_3MIC_NS)
			*:speech_3mic_ns.o(.data* .rodata*)
			*:logmmse.o(.data* .rodata*)
			*:lcmmse.o(.data* .rodata*)
			*:filterbank_pflt.o(.data* .rodata*)
			*:arm_mat_trans_q15.o(.data* .rodata*)
			*:arm_mat_inverse_f64.o(.data* .rodata*)
			*:libm.a(.data* .rodata*)
			*:spectral_wdrc.o(.data* .rodata*)
			*:fixed_beamforming.o(.data* .rodata*)
			*:stft.o(.data* .rodata*)
			*:rls.o(.data* .rodata*)
			*:coherence.o(.data* .rodata*)
			*:echo_suppression_core.o(.data* .rodata*)
			*:spectral_wdrc.o(.data* .rodata*)
			*:wind_detection_in_3mic.o(.data* .rodata*)
			*:mdf5.o(.data* .rodata*)
			*:ns3.o(.data* .rodata*)
			*:noise_suppression_x.o(.data* .rodata*)
			*:noise_suppression.o(.data* .rodata*)
			*:ns_core.o(.data* .rodata*)
#endif
			*(.overlay_data0 .overlay_rodata0)
			. = ALIGN(4);
			*(.overlay_sco_cache*)
			. = ALIGN(4);
		}
		.overlay_data1
		{
			LONG(0);
#if !defined(RB_CODEC)
			*:fixedpoint.o(.data* .rodata*)
			*:replaygain.o(.data* .rodata*)
			*:dsp_core.o(.data* .rodata*)
			*:dsp_misc.o(.data* .rodata*)
			*:dsp_filter.o(.data* .rodata*)
			*:dsp_sample_input.o(.data* .rodata*)
			*:dsp_sample_output.o(.data* .rodata*)
			*:eq.o(.data* .rodata*)
			*:pga.o(.data* .rodata*)
#if !defined(A2DP_AAC_ON)
			*:fir_process.o(.data* .rodata*)
#endif
#endif
			*:a2dp_decoder_sbc.o(.data* .rodata*)
			*(.overlay_data1 .overlay_rodata1)
			. = ALIGN(4);
		}
		.overlay_data2
		{
			LONG(0);
#if 0
			*:lc3_com.o(.data* .rodata*)
			*:lc3_dec.o(.data* .rodata*)
			*:lc3_enc.o(.data* .rodata*)
#endif
			*(.overlay_data2 .overlay_rodata2)
			. = ALIGN(4);
		}
		.overlay_data3
		{
			LONG(0);
			*:fft128dot.o(.data* .rodata*)
			*(.overlay_data3 .overlay_rodata3)
			. = ALIGN(4);
		}
		.overlay_data4
		{
			LONG(0);
#if !defined(PROMPT_USE_AAC)
#ifdef A2DP_SOURCE_AAC_ON
			*:aacenc_lib.o(.data* .rodata*)
			*:aacenc.o(.data* .rodata*)
			*:adj_thr.o(.data* .rodata*)
			*:bit_cnt.o(.data* .rodata*)
			*:quantize.o(.data* .rodata*)
			*:tpenc_lib.o(.data* .rodata*)
			*:tpenc_latm.o(.data* .rodata*)
			*:line_pe.o(.data* .rodata*)
			*:spreading.o(.data* .rodata*)
			*:bitenc.o(.data* .rodata*)
			*:qc_main.o(.data* .rodata*)
			*:band_nrg.o(.data* .rodata*)
			*:aacenc_tns.o(.data* .rodata*)
			*:pre_echo_control.o(.data* .rodata*)
			*:transform.o(.data* .rodata*)
			*:dyn_bits.o(.data* .rodata*)
			*:aacenc_pns.o(.data* .rodata*)
			*:aacEnc_ram.o(.data* .rodata*)
			*:aacEnc_rom.o(.data* .rodata*)
			*:bandwidth.o(.data* .rodata*)
			*:block_switch.o(.data* .rodata*)
			*:channel_map.o(.data* .rodata*)
			*:chaosmeasure.o(.data* .rodata*)
			*:grp_data.o(.data* .rodata*)
			*:intensity.o(.data* .rodata*)
			*:metadata_compressor.o(.data* .rodata*)
			*:metadata_main.o(.data* .rodata*)
			*:ms_stereo.o(.data* .rodata*)
			*:noisedet.o(.data* .rodata*)
			*:pnsparam.o(.data* .rodata*)
			*:psy_configuration.o(.data* .rodata*)
			*:psy_main.o(.data* .rodata*)
			*:sf_estim.o(.data* .rodata*)
			*:tonality.o(.data* .rodata*)
#else
#if defined(A2DP_AAC_ON)
			*:block.o(.data* .rodata*)
			*:tpdec_lib.o(.data* .rodata*)
			*:tpdec_latm.o(.data* .rodata*)
			*:aacdec_pns.o(.data* .rodata*)
			*:aacdec_tns.o(.data* .rodata*)
			*:pulsedata.o(.data* .rodata*)
			*:channel.o(.data* .rodata*)
			*:tpdec_asc.o(.data* .rodata*)
			*:aacdecoder.o(.data* .rodata*)
			*:channelinfo.o(.data* .rodata*)
			*:aacdecoder_lib.o(.data* .rodata*)
			*:stereo.o(.data* .rodata*)
			*:fixpoint_math.o(.data* .rodata*)
			*:FDK_bitbuffer.o(.data* .rodata*)
#endif
#if defined(AAC_BECO_FFT)
		*:fdkaac_beco_fft.o(.data* .rodata*)
#endif
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
			*:aac_com.o(.data .rodata*)
			*:aac_dec.o(.data .rodata*)
			*:aac_enc.o(.data .rodata*)
			*:aac_api.o(.data .rodata*)
#endif
#endif
			*:a2dp_decoder_aac_lc.o(.data .rodata*.*a2dp_audio_aac_lc_decoder_init*)
			*:a2dp_decoder_aac_lc.o(.data .rodata*.*a2dp_audio_aac_lc_decoder_reinit*)
			*:a2dp_decoder_aac_lc.o(.data .rodata*.*a2dp_audio_aac_lc_channel_select*)
			*:a2dp_decoder_aac_lc.o(.data .rodata*.*aac_malloc*)
			*:a2dp_decoder_aac_lc.o(.data .rodata*.*aac_free*)
#endif
			*(.overlay_data4 .overlay_rodata4)
			. = ALIGN(4);
		}
		.overlay_data5
		{
			LONG(0);
			*:mpa.o(.data* .bss*)
			*:bit.o(.data* .bss*)
			*:frame.o(.data* .bss*)
			*:huffman.o(.data* .bss*)
			*:layer12.o(.data* .bss*)
			*:layer3.o(.data* .bss*)
			*:stream.o(.data* .bss*)
			*:synth.o(.data* .bss*)
			*(.overlay_data5 .overlay_rodata5)
			. = ALIGN(4);
		}
		.overlay_data6
		{
			LONG(0);
#if defined(A2DP_SCALABLE_ON)
			*:rc_decode.o(.data* .rodata*)
			*:audio_int32.o(.data* .rodata*)
			*:ssc_ifft.o(.data* .rodata*)
			*:vector_quant.o(.data* .rodata*)
			*:ssc_quant.o(.data* .rodata*)
			*:ssc_pulsealloc.o(.data* .rodata*)
			*:audio.o(.data* .rodata*)
			*:set_sbm.o(.data* .rodata*)
			*:rc_code.o(.data* .rodata*)
			*:SBM_pitchBased.o(.data* .rodata*)
			*:ssc_imdct.o(.data* .rodata*)
			*:audio_decoder.o(.data* .rodata*)
			*:SoundBooster_DRC_classic.o(.data* .rodata*)
			*:ssc_cwrs.o(.data* .rodata*)
			*:ssc_mode.o(.data* .rodata*)
			*:ssc_math.o(.data* .rodata*)
			*:ssc_lap.o(.data* .rodata*)
			*:SeamlessBufferManagement.o(.data* .rodata*)
#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
			*:a2dp_decoder_scalable.o(.rodata*)
			*:a2dp_decoder_scalable.o(.bss*ss_pcm_buff)
#endif
#endif
#if defined(A2DP_LHDC_ON)
			/*LLAC*/
			*:llac_dec_alg.o(.data* .rodata*)
			*:llac_dec_alg_code.o(.data* .rodata*)
			*:llac_dec_alg_mdct.o(.data* .rodata*)
			*:llac_dec_alg_quant.o(.data* .rodata*)
			*:llac_dec_alg_signal.o(.data* .rodata*)
			*:llac_dec_const.o(.data* .rodata*)
			*:llac_dec_init.o(.data* .rodata*)
			*:llac_dec_mem_alloc.o(.data* .rodata*)
			*:llac_dec_wrapper.o(.data* .rodata*)
			*:sv_math.o(.data* .rodata*)
			*:uhead_api.o(.data* .rodata*)

			/*LHDC*/
			*:a2dp_decoder_lhdc.o(.data* .rodata*)
			*:testkey.o(.data* .rodata*)
			*:lhdcUtil.o(.data* .rodata*)
			*:lzh_dec.o(.data* .rodata*)
			*:licdecrypt.o(.data* .rodata*)
			*:lic_check.o(.data* .rodata*)
			*:fast_upsampling_2x.o(.data* .rodata*)
			*:encdec.o(.data* .rodata*)
			*:dec.o(.data* .rodata*)
			*:cirbuf.o(.data* .rodata*)
			*:DecodeUps2xLPF.o(.data* .rodata*)
#endif
#if defined(A2DP_SOURCE_LHDC_ON)
			*:EncodeLPF.o(.data* .rodata*)
			*:enc.o(.data* .rodata*)
			*:encdec.o(.data* .rodata*)
			*:lhdc_enc_api.o(.data* .rodata*)
			*:lhdc_process.o(.data* .rodata*)
			*:lic_check.o(.data* .rodata*)
			*:licdecrypt.o(.data* .rodata*)
			*:lzh_enc.o(.data* .rodata*)
			*:win_sinc.o(.data* .rodata*)
			*:a2dp_encoder_lhdc_cp.o(.data* .rodata*)
#endif
			*:mp3_dec.o(.data* .rodata*)
			*:mp3_enc.o(.data* .rodata*)
			*(.overlay_data6 .overlay_rodata6)
			. = ALIGN(4);
		}
		.overlay_data7
		{
			LONG(0);
#if defined(A2DP_LDAC_ON)
			*:a2dp_decoder_ldac.o(.data* .rodata*)
			*:bitalloc_ldac.o(.data* .rodata*)
			*:bitalloc_sub_ldac.o(.data* .rodata*)
			*:encode_ldac.o(.data* .rodata*)
			*:ldacBT.o(.data* .rodata*)
			*:ldaclib.o(.data* .rodata*)
			*:pack_ldac.o(.data* .rodata*)
			*:quant_ldac.o(.data* .rodata*)
			*:mdct_ldac.o(.data* .rodata*)
			*:ldacBT_api.o(.data* .rodata*)
			*:decode_ldac.o(.data* .rodata*)
			*:dequant_ldac.o(.data* .rodata*)
			*:imdct_ldac.o(.data* .rodata*)
			*:ldacBT_internal.o(.data* .rodata*)
			*:ldaclib_api.o(.data* .rodata*)
			*:memory_ldac.o(.data* .rodata*)
			*:setpcm_ldac.o(.data* .rodata*)
			*:sigana_ldac.o(.data* .rodata*)
			*:tables_ldac.o(.data* .rodata*)
			*:tables_sigproc_ldac.o(.data* .rodata*)
			*:unpack_ldac.o(.data* .rodata*)
#endif
			*(.overlay_data7 .overlay_rodata7)
			. = ALIGN(4);
		}
		.overlay_data8
		{
			LONG(0);
#if defined(A2DP_LHDCV5_ON)
			/* LHDC V5*/
			*:a2dp_decoder_lhdcv5.o(.data* .rodata*)
			*:lhdc_v5_kiss_fft.o(.data* .rodata*)
			*:lhdc_v5_dec_process.o(.data* .rodata*)
			*:lhdcv5_util_dec.o(.data* .rodata*)
			*:lhdc_v5_dec_time_process.o(.data* .rodata*)
			*:lhdc_v5_global_table.o(.data* .rodata*)
			*:svlice_v5_encdec.o(.data* .rodata*)
			*:lhdc_v5_arith.o(.data* .rodata*)
			*:svlice_v5_lic_check.o(.data* .rodata*)
			*:lhdc_v5_dec_workspace.o(.data* .rodata*)
			*:lhdc_v5_level.o(.data* .rodata*)
			*:lhdc_log.o(.data* .rodata*)
			*:lhdc_v5_dec_header.o(.data* .rodata*)
			*:lhdc_v5_accelerate.o(.data* .rodata*)
			*:lhdc_v5_bit_inputter.o(.data* .rodata*)
			*:svlice_v5_licdecrypt.o(.data* .rodata*)
			*:lhdc_v5_freq_shift.o(.data* .rodata*)
#endif
			*(.overlay_data8 .overlay_rodata8)
			. = ALIGN(4);
		}
		.overlay_data9
		{
			LONG(0);
#if defined(OPUS_OPT_OVERLAY)
			*:voice_opus.o(.data* .rodata*)
			*:opus_encoder.o(.data* .rodata*)
			*:analysis.o(.data* .rodata*)
			*:celt_encoder.o(.data* .rodata*)
			*:repacketizer.o(.data* .rodata*)
			*:opus.o(.data* .rodata*)
			*:enc_API.o(data* .rodata)
			*:entenc.o(.data* .rodata*)
			*:control_audio_bandwidth.o(.data* .rodata*)
			*:encode_pulses.o(.data* .rodata*)
			*:LPC_analysis_filter.o(.data* .rodata*)
			*:burg_modified_FIX.o(.data* .rodata*)
			*:vector_ops_FIX.o(.data* .rodata*)
			*:NLSF_del_dec_quant.o(.data* .rodata*)
			*:encode_frame_FIX.o(.data* .rodata*)
			*:pitch.o(.data* .rodata*)
			*:resampler.o(.data* .rodata*)
			*:VAD.o(.data* .rodata*)
			*:encode_indices.o(.data* .rodata*)
			*:control_codec.o(.data* .rodata*)
			*:celt_lpc.o(.data* .rodata*)
			*:shell_coder.o(.data* .rodata*)
			*:init_encoder.o(.data* .rodata*)
			*:log2lin.o(.data* .rodata*)
			*:biquad_alt.o(.data* .rodata*)
			*:stereo_encode_pred.o(.data* .rodata*)
			*:NLSF_unpack.o(.data* .rodata*)
			*:code_signs.o(.data* .rodata*)
			*:lin2log.o(.data* .rodata*)
			*:HP_variable_cutoff.o(.data* .rodata*)
			*:sum_sqr_shift.o(.data* .rodata*)
			*:inner_prod_aligned.o(.data* .rodata*)
			*:stereo_quant_pred.o(.data* .rodata*)
			*:entcode.o(.data* .rodata*)
			*:celt.o(.data* .rodata*)
			*:laplace.o(.data* .rodata*)
			*:NSQ.o(.data* .rodata*) /*complexity 1*/
			*:NSQ_del_dec.o(.data* .rodata*) /*complexity 2*/
#else
			*:opus_com.o(.data* .rodata*)
#endif
			*(.overlay_data9 .overlay_rodata9)
			. = ALIGN(4);
		}
		.overlay_data10
		{
			LONG(0);
			*:ogg_com.o(.text*)
			*:vorbis_com.o(.data* .rodata*)
			*(.overlay_data10 .overlay_rodata10)
			. = ALIGN(4);
		}
		.overlay_data11
		{
			LONG(0);
			*:alac_com.o(.data* .rodata*)
			*(.overlay_data11 .overlay_rodata11)
			. = ALIGN(4);
		}
		.overlay_data12
		{
			LONG(0);
			*:flac_com.o(.data* .rodata*)
			*(.overlay_data12 .overlay_rodata12)
			. = ALIGN(4);
		}
		.overlay_data13
		{
			LONG(0);
			*(.overlay_data13 .overlay_rodata13)
			. = ALIGN(4);
		}
#endif
		.overlay_data_last
		{
			LONG(0);
			. = ALIGN(4);
		}
	} > REGION_OVERLAY_DATA

	.dummy_overlay_data_end (.) (NOLOAD) : AT (ADDR(.dummy_overlay_data_end))
	{
		__overlay_data_end__ = .;
	} > REGION_OVERLAY_DATA

	ASSERT(__overlay_data_end__ - __overlay_data_start__ <= OVERLAY_DATA_SECTION_SIZE, "overlay data sections too large")

#if defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0)
	.cp_ramx_last_dummy (NOLOAD) : AT (ADDR(.cp_ramx_last_dummy))
	{
		__cp_ramx_last_dummy_start = .;
	} > RAMCPX

	.cp_ram_last_dummy (NOLOAD) : AT (ADDR(.cp_ram_last_dummy))
	{
		. = ALIGN(8);
		__cp_ram_last_dummy_start = .;
	} > RAMCP

	__cp_stack_top = ORIGIN(RAMCP) + LENGTH(RAMCP);
	__cp_stack_limit = __cp_stack_top - CP_STACK_SECTION_SIZE;

	/* LD v2.24 (GCC 4.9) cannot handle address conversion correctly if involving immediate value.
	   E.g., <absolute_addr> + <immediate_val> + <relative_addr> */
	__free_ramcpx = ORIGIN(RAMCPX) + LENGTH(RAMCPX) - ABSOLUTE(__cp_ramx_last_dummy_start);
	ASSERT(__cp_stack_limit >= __cp_ram_last_dummy_start, "region RAMCP overflowed with stack")
	__free_ramcp = __cp_stack_limit - __cp_ram_last_dummy_start;

#ifdef MEM_POOL_IN_CP_RAM
	__cp_mem_pool_start__ = __cp_ram_last_dummy_start;
	__cp_mem_pool_end__ = __cp_ram_last_dummy_start;
	__mem_pool_start__ = __cp_ram_last_dummy_start;
	__mem_pool_end__ = __cp_stack_limit;
#else
	__cp_mem_pool_start__ = __cp_ram_last_dummy_start;
	__cp_mem_pool_end__ = __cp_stack_limit;
#endif
#endif

#if (CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE > 0)
    __customer_load_sram_text_start_flash__ = __load_stop_overlay_data_last;
	.customer_load_sram_text_section : AT(__customer_load_sram_text_start_flash__)
	{
        __customer_load_sram_text_exec_start__ = .;
#if defined(CMSE_CRYPT_TEST_DEMO)
		*:rsa_test.o(.text*)
#endif
		*:ree_audio_process.o(.text*)

        *(.customer_load_sram_text_section*)
        . = ALIGN(32);
        __customer_load_sram_text_exec_end__ = .;
	} > CUSTOMER_LOAD_SRAM_TEXT_SECTION

	__customer_load_sram_text_end_flash__ = __customer_load_sram_text_start_flash__ + SIZEOF(.customer_load_sram_text_section);

	__customer_load_sram_text_start__ = RAMX_TO_RAM(__customer_load_sram_text_exec_start__);
	__customer_load_sram_text_end__ = RAMX_TO_RAM(__customer_load_sram_text_exec_end__);

	ASSERT(__customer_load_sram_text_exec_end__ - __customer_load_sram_text_exec_start__ <= CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE, "customer_load_sram_text_section too large")
	__customer_free_load_sram_text = CUSTOMER_LOAD_SRAM_TEXT_RAMX_SECTION_SIZE - (__customer_load_sram_text_exec_end__ - __customer_load_sram_text_exec_start__);

    __customer_load_ram_data_start_flash__ = __customer_load_sram_text_end_flash__;

	.customer_load_ram_data_section : AT(__customer_load_ram_data_start_flash__)
	{
        __customer_load_ram_data_start__ = .;
#if defined(CMSE_CRYPT_TEST_DEMO)
		*:rsa_test.o(.data* .rodata*)
#endif
		*:ree_audio_process.o(.data* .rodata*)

        *(.customer_load_ram_data_section*)
        . = ALIGN(32);
        __customer_load_ram_data_end__ = .;
    } > CUSTOMER_LOAD_RAM_DATA_SECTION

	__customer_load_ram_data_end_flash__ = __customer_load_ram_data_start_flash__ + SIZEOF(.customer_load_ram_data_section);
	ASSERT(__customer_load_ram_data_end__ - __customer_load_ram_data_start__ <= CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE, "customer_load_ram_data_section too large")

	__customer_load_ram_data_text_total_flash__ = __customer_load_ram_data_end_flash__ - __customer_load_sram_text_start_flash__;
	__customer_free_load_ram_data = CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE - (__customer_load_ram_data_end__ - __customer_load_ram_data_start__);
#endif

#if (CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE > 0)
	__flash_text_data_start__ = __customer_load_ram_data_end_flash__;
#else
	__flash_text_data_start__ = __load_stop_overlay_data_last;
#endif
#if (defined(PSRAM_ENABLE) || defined(PSRAM_HEAP_ONLY)) && defined(PSRAM_BASE) && (PSRAM_SIZE > 0)
	__psram_text_data_start__ = ORIGIN(PSRAM);
	__psram_text_data_start_flash__ = __flash_text_data_start__;
	.psram_text (PSRAM_TO_PSRAMX(__psram_text_data_start__)) : AT (__psram_text_data_start_flash__)
	{
		__psramx_text_start__ = .;
#if !defined(PSRAM_HEAP_ONLY)
		*(.psram_text*)
#ifdef DEFAULT_CODE_IN_PSRAM
		*(.text*)
#endif
#endif
		. = ALIGN(32);
		__psramx_text_end__ = .;
	} > PSRAMX

	.psram_data (__psram_text_data_start__ + SIZEOF(.psram_text)) :
				AT (__psram_text_data_start_flash__ + SIZEOF(.psram_text))
	{
		__psram_data_start__ = .;
#if !defined(PSRAM_HEAP_ONLY)
		*(.psram_rodata*)
#ifdef DEFAULT_DATA_IN_PSRAM
		*(.rodata*)
#endif
		*(.psram_data*)
#ifdef DEFAULT_DATA_IN_PSRAM
		*(.data*)
#endif
#endif
		__psram_data_end__ = .;
		. = ALIGN(32);
	} > PSRAM

	__psram_text_data_end_flash__ = __psram_text_data_start_flash__ + SIZEOF(.psram_text)+ SIZEOF(.psram_data);

	.psram_bss (.) (NOLOAD) : AT (ADDR(.psram_bss))
	{
		__psram_bss_start__ = .;
#if !defined(PSRAM_HEAP_ONLY)
		*:*ffmpeg*.o(COMMON)
		*(.psram_bss*)
#ifdef DEFAULT_BSS_IN_PSRAM
		*(.bss*)
#endif
#endif
		. = ALIGN(32);
		__psram_bss_end__ = .;
	} > PSRAM

	.psram_nc_bss (PSRAM_C_TO_NC(.)) (NOLOAD) : AT (ADDR(.psram_nc_bss))
	{
		__psram_nc_bss_start__ = .;
#if !defined(PSRAM_HEAP_ONLY)
		*(.psram_nc_bss*)
		*(.sync_flags_psram*)
#endif
		. = ALIGN(32);
		__psram_nc_bss_end__ = .;
	} > PSRAM_NC

	.psram_heap_bss (PSRAM_NC_TO_C(.)) (NOLOAD) : AT (ADDR(.psram_heap_bss))
	{
		__psram_reserved_start__ = .;
		*(.psram_reserved*)
		__psram_reserved_end__ = .;
#if defined(__NuttX__)
		. = ALIGN(32);
		_nuttx_heap_start = .;
		_nuttx_heap_end = ABSOLUTE(PSRAM_BASE+PSRAM_REGION_SIZE);
#endif
	} > PSRAM

	__flash_text_data_start__ = __psram_text_data_end_flash__;
#endif

#if defined(PSRAMUHS_ENABLE) && defined(PSRAMUHS_BASE) && (MCU_PSRAMUHS_SIZE > 0)
	__psramuhs_text_data_start__ = ORIGIN(PSRAMUHS);
	__psramuhs_text_data_start_flash__ = __flash_text_data_start__;
	.psramuhs_text (PSRAMUHS_TO_PSRAMUHSX(__psramuhs_text_data_start__)) : AT (__psramuhs_text_data_start_flash__)
	{
	    __psramux_text_start__ = .;
	    *(.psramuhs_text*)
	    . = ALIGN(32);
	    __psramux_text_end__ = .;
	} > PSRAMUHSX

	.psramuhs_data (__psramuhs_text_data_start__ + SIZEOF(.psramuhs_text)) :
	            AT (__psramuhs_text_data_start_flash__ + SIZEOF(.psramuhs_text))
	{
		__psramuhs_data_start__ = .;
	    *(.psramuhs_rodata*)
	    *(.psramuhs_data*)
		__psramuhs_data_end__ = .;
	    . = ALIGN(32);
	} > PSRAMUHS

	__psramuhs_text_data_end_flash__ = __psramuhs_text_data_start_flash__ + SIZEOF(.psramuhs_text)+ SIZEOF(.psramuhs_data);

	.psramuhs_bss (.) (NOLOAD) : AT (ADDR(.psramuhs_bss))
	{
	    __psramuhs_bss_start__ = .;
	    *(.psramuhs_bss*)
	    . = ALIGN(32);
	    __psramuhs_bss_end__ = .;
	} > PSRAMUHS

	.psramuhs_nc_bss (PSRAMUHS_C_TO_NC(.)) (NOLOAD) : AT (ADDR(.psramuhs_nc_bss))
	{
	    __psramuhs_nc_bss_start__ = .;
	    *(.psramuhs_nc_bss*)
		*(.sync_flags_psramuhs*)
	    . = ALIGN(32);
	    __psramuhs_nc_bss_end__ = .;
	} > PSRAMUHS_NC

	__flash_text_data_start__ = __psramuhs_text_data_end_flash__;
#endif

#if defined(ARM_CMSE) && defined(CMSE_RAM_RAMX_LEND_NSE)

    __cmse_ramx_lend_text_section_start__ = ORIGIN(ARM_CMSE_RAMX_TEXT_LEND);
    __cmse_ramx_lend_text_section_start_flash__ = __flash_text_data_start__ ;
    __cmse_ramx_lend_text_section_start_flash__ = ALIGN(__cmse_ramx_lend_text_section_start_flash__, 32);
    .cmse_ramx_lend_text_section (__cmse_ramx_lend_text_section_start__) : AT (__cmse_ramx_lend_text_section_start_flash__)
    {
        __cmse_ramx_lend_text_section_exec_start__ = .;
        *(.cmse_ramx_lend_text_section*)
        *:user_secure_boot.o(.text*)
        *:crypt_lib.o(.text*)
        *:bin_aes_crypt.o(.text*)
        *:parser.o(.text*)
        . = ALIGN(32);
        __cmse_ramx_lend_text_section_exec_end__ = .;
    } > ARM_CMSE_RAMX_TEXT_LEND

    __cmse_ramx_lend_text_section_end_flash__ = __cmse_ramx_lend_text_section_start_flash__ + SIZEOF(.cmse_ramx_lend_text_section);

	__cmse_ramx_lend_text_section_ram_start__ = RAMX_TO_RAM(__cmse_ramx_lend_text_section_exec_start__);
	__cmse_ramx_lend_text_section_ram_end__ = RAMX_TO_RAM(__cmse_ramx_lend_text_section_exec_end__);

    __cmse_ram_lend_data_section_start__ = ORIGIN(ARM_CMSE_RAM_DATA_LEND);

    __cmse_ram_lend_data_section_start_flash__ = __cmse_ramx_lend_text_section_start_flash__ + SIZEOF(.cmse_ramx_lend_text_section);

    .cmse_ram_lend_data_section(__cmse_ram_lend_data_section_start__)  : AT (__cmse_ram_lend_data_section_start_flash__)
    {
        __cmse_ram_lend_data_section_exec_start__ = .;

        *(.cmse_ram_lend_data_section*)
		*:crypt_lib.o(.data* .rodata*)
        *:user_secure_boot.o(.data* .rodata*)
        *:bin_aes_crypt.o(.data* .rodata*)
        *:parser.o(.data* .rodata*)
        . = ALIGN(32);
        __cmse_ram_lend_data_section_exec_end__ = .;
    } > ARM_CMSE_RAM_DATA_LEND

	__cmse_ram_ramx_data_text_end_flash__ = __cmse_ramx_lend_text_section_start_flash__ + SIZEOF(.cmse_ramx_lend_text_section)+ SIZEOF(.cmse_ram_lend_data_section);

    .cmse_ram_lend_bss_sram_section (.) (NOLOAD) : AT (ADDR(.cmse_ram_lend_bss_sram_section))
    {
        __cmse_ram_lend_bss_sram_section_start = .;

        *(.cmse_ram_lend_bss_sram_section*)
		*:crypt_lib.o(.bss*)
        *:user_secure_boot.o(.bss*)
        *:bin_aes_crypt.o(.bss*)
        *:parser.o(.bss*)
#if 0
		*:rsa_test.o(.bss*)
#endif
        . = ALIGN(32);
        __cmse_ram_lend_bss_sram_section_end = .;
    } > ARM_CMSE_RAM_DATA_LEND

	__flash_text_data_start__ = __cmse_ram_ramx_data_text_end_flash__;
#endif
	.text (FLASH_TO_FLASHX(__flash_text_data_start__)) : AT (FLASHX_TO_FLASH(ADDR(.text)))
	{
		*(.text*)
		*(.flash_text*)

		KEEP(*(.init))
		KEEP(*(.fini))
		_sinit = ABSOLUTE(.);
		#ifdef CONFIG_HAVE_CXX
		/* C++ Static constructors/destructors (eabi) */
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
		#endif
		/* .ctors */
		*crtbegin.o(.ctors)
		*crtbegin?.o(.ctors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
		*(SORT(.ctors.*))
		*(.ctors)
		_einit = ABSOLUTE(.);

		/* .dtors */
		*crtbegin.o(.dtors)
		*crtbegin?.o(.dtors)
		*(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
		*(SORT(.dtors.*))
		*(.dtors)
	} > FLASHX
#if defined(ARM_CMSE) && !defined(INTSRAM_RUN)
	. = ALIGN(32);
	__nsc_text_start__ = .;
	.gnu.sgstubs (.) : AT (FLASHX_TO_FLASH(ADDR(.gnu.sgstubs)))
	{
		*(.gnu.sgstubs*)
		. = ALIGN(4);
	} > FLASHX
	. = ALIGN(32);
	__nsc_text_end__ = .;
#endif
	.ARM.extab (.) : AT (FLASHX_TO_FLASH(ADDR(.ARM.extab)))
	{
		*(.ARM.extab* .gnu.linkonce.armextab.*)
	} > FLASHX

	__exidx_start = .;
	/* .ARM.exidx contains R_ARM_PREL31 (+-0x40000000) offset to functions, which means
	 * the session location cannot be too far away from the function addresses */
	.ARM.exidx (.) : AT (FLASHX_TO_FLASH(ADDR(.ARM.exidx)))
	{
		*(.ARM.exidx* .gnu.linkonce.armexidx.*)
		__flashx_text_end__ = .;
	} > FLASHX
	__exidx_end = .;

#ifdef ARM_CMSE
	/* BEST2003 SAU set FLASH_BASE region as nonsecure region becase of the SAU region number limitation,
	CPU can't access FLASH_BASE region in secure state, so the rodata should be located in SRAM or FLASHX*/
	.sec_rodata (.) : AT (FLASHX_TO_FLASH(ADDR(.sec_rodata)))
	{
		*(.rodata*)
	} > FLASHX
#endif

	. = FLASHX_TO_FLASH(.);

#ifdef TRACE_STR_SECTION
	.rodata_str (.):
	{
	    __rodata_str_start__ = .;
		*(.rodata.__func__.*)
		*(.rodata.*__func__)
		*(.rodata.str1*)
		*(.rodata.*.str1*)
		*(.rodata.__FUNCTION__.*)
		*(.rodata.*__FUNCTION__)
		__rodata_str_end__ = .;
	} > FLASH

	RODATA_ADDRESS = .;
	. = TRACE_ID_STR_OFFSET;
	.trc_str (.):
	{
	    __trace_str_start__ = .;
		*(.rodata.__func__.*)
		*(.rodata.*__func__)
		*(.rodata.__FUNCTION__.*)
		*(.rodata.*__FUNCTION__)
		*(.trc_str*)
	    __trace_str_end__ = .;
	}
	. = RODATA_ADDRESS;
#endif

	.rodata (.) :
	{
		*(.rodata*)
		*(.flash_rodata*)
		*(.constdata)
#ifndef CONFIG_HAVE_CXX
		. = ALIGN(4);
		/* preinit data */
		PROVIDE_HIDDEN (__preinit_array_start = .);
		KEEP(*(.preinit_array))
		PROVIDE_HIDDEN (__preinit_array_end = .);

		. = ALIGN(4);
		/* init data */
		PROVIDE_HIDDEN (__init_array_start = .);
		KEEP(*(SORT(.init_array.*)))
		KEEP(*(.init_array))
		PROVIDE_HIDDEN (__init_array_end = .);
#endif
		. = ALIGN(4);
		/* finit data */
		PROVIDE_HIDDEN (__fini_array_start = .);
		KEEP(*(SORT(.fini_array.*)))
		KEEP(*(.fini_array))
		PROVIDE_HIDDEN (__fini_array_end = .);

		. = ALIGN(4);
		__cust_cmd_hldr_tbl_start = .;
		KEEP(*(.cust_cmd_hldr_tbl))
		__cust_cmd_hldr_tbl_end = .;
		. = ALIGN(4);
		__ai_gatt_server_table_start = .;
		KEEP(*(.ai_gatt_server_table))
		__ai_gatt_server_table_end = .;
		. = ALIGN(4);
		__ai_ble_handler_table_start = .;
		KEEP(*(.ai_ble_handler_table))
		__ai_ble_handler_table_end = .;
		. = ALIGN(4);
		__ai_spp_register_table_start = .;
		KEEP(*(.ai_spp_register_table))
		__ai_spp_register_table_end = .;
		. = ALIGN(4);
		__tota_handler_table_start = .;
		KEEP(*(.tota_handler_table))
		__tota_handler_table_end = .;
		. = ALIGN(4);
		__ai_handler_function_table_start = .;
		KEEP(*(.ai_handler_function_table))
		__ai_handler_function_table_end = .;
		. = ALIGN(4);
		__custom_handler_table_start = .;
		KEEP(*(.custom_handler_table))
		__custom_handler_table_end = .;
		. = ALIGN(4);
		__mixprompt_property_table_start = .;
		KEEP(*(.mixprompt_property_table))
		__mixprompt_property_table_end = .;
		. = ALIGN(4);
		__core_bridge_task_cmd_table_start = .;
		KEEP(*(.core_bridge_task_cmd_table))
		__core_bridge_task_cmd_table_end = .;
		. = ALIGN(4);
		__core_bridge_instant_cmd_table_start = .;
		KEEP(*(.core_bridge_instant_cmd_table))
		__core_bridge_instant_cmd_table_end = .;
		. = ALIGN(4);
		__rpc_bth_dsp_cmd_table_start = .;
		KEEP(*(.rpc_bth_dsp_cmd_table))
		__rpc_bth_dsp_cmd_table_end = .;
		. = ALIGN(4);
		__eshell_command_table_start = .;
		KEEP(*(.eshell_command_table))
		__eshell_command_table_end = .;
		. = ALIGN(4);
		__app_bt_sync_command_handler_table_start = .;
		KEEP(*(.app_bt_sync_command_handler_table))
		__app_bt_sync_command_handler_table_end = .;
		. = ALIGN(4);
		. = ALIGN(4);
	 	KEEP(*(.eh_frame*))
		*(.note.gnu.build-id)

		. = ALIGN(4);
		_etext = .;
	} > FLASH

#if defined(FIRMWARE_REV)
	.firmware_rev (.) :
	{
		. = ALIGN(64);
		*(.fw_rev)
	} > FLASH
#endif
/* zInit code and data - will be freed after init */
	.zInit (.) :
	{
		__zinitcall_bsp_start = .;
		KEEP (*(.zinitcall.bsp0.init))
		KEEP (*(.zinitcall.bsp1.init))
		KEEP (*(.zinitcall.bsp2.init))
		KEEP (*(.zinitcall.bsp3.init))
		KEEP (*(.zinitcall.bsp4.init))
		__zinitcall_bsp_end = .;
		. = ALIGN(4);
		__zinitcall_device_start = .;
		KEEP (*(.zinitcall.device0.init))
		KEEP (*(.zinitcall.device1.init))
		KEEP (*(.zinitcall.device2.init))
		KEEP (*(.zinitcall.device3.init))
		KEEP (*(.zinitcall.device4.init))
		__zinitcall_device_end = .;
		. = ALIGN(4);
		__zinitcall_core_start = .;
		KEEP (*(.zinitcall.core0.init))
		KEEP (*(.zinitcall.core1.init))
		KEEP (*(.zinitcall.core2.init))
		KEEP (*(.zinitcall.core3.init))
		KEEP (*(.zinitcall.core4.init))
		__zinitcall_core_end = .;
		. = ALIGN(4);
		__zinitcall_sys_service_start = .;
		KEEP (*(.zinitcall.sys.service0.init))
		KEEP (*(.zinitcall.sys.service1.init))
		KEEP (*(.zinitcall.sys.service2.init))
		KEEP (*(.zinitcall.sys.service3.init))
		KEEP (*(.zinitcall.sys.service4.init))
		__zinitcall_sys_service_end = .;
		. = ALIGN(4);
		__zinitcall_sys_feature_start = .;
		KEEP (*(.zinitcall.sys.feature0.init))
		KEEP (*(.zinitcall.sys.feature1.init))
		KEEP (*(.zinitcall.sys.feature2.init))
		KEEP (*(.zinitcall.sys.feature3.init))
		KEEP (*(.zinitcall.sys.feature4.init))
		__zinitcall_sys_feature_end = .;
		. = ALIGN(4);
		__zinitcall_run_start = .;
		KEEP (*(.zinitcall.run0.init))
		KEEP (*(.zinitcall.run1.init))
		KEEP (*(.zinitcall.run2.init))
		KEEP (*(.zinitcall.run3.init))
		KEEP (*(.zinitcall.run4.init))
		__zinitcall_run_end = .;
		. = ALIGN(4);
		__zinitcall_app_service_start = .;
		KEEP (*(.zinitcall.app.service0.init))
		KEEP (*(.zinitcall.app.service1.init))
		KEEP (*(.zinitcall.app.service2.init))
		KEEP (*(.zinitcall.app.service3.init))
		KEEP (*(.zinitcall.app.service4.init))
		__zinitcall_app_service_end = .;
		. = ALIGN(4);
		__zinitcall_app_feature_start = .;
		KEEP (*(.zinitcall.app.feature0.init))
		KEEP (*(.zinitcall.app.feature1.init))
		KEEP (*(.zinitcall.app.feature2.init))
		KEEP (*(.zinitcall.app.feature3.init))
     		  KEEP (*(.zinitcall.app.feature4.init))
		__zinitcall_app_feature_end = .;
		. = ALIGN(4);
		__zinitcall_test_start = .;
		KEEP (*(.zinitcall.test0.init))
		KEEP (*(.zinitcall.test1.init))
		KEEP (*(.zinitcall.test2.init))
		KEEP (*(.zinitcall.test3.init))
		KEEP (*(.zinitcall.test4.init))
		__zinitcall_test_end = .;
		. = ALIGN(4);
		__zinitcall_exit_start = .;
		KEEP (*(.zinitcall.exit0.init))
		KEEP (*(.zinitcall.exit1.init))
		KEEP (*(.zinitcall.exit2.init))
		KEEP (*(.zinitcall.exit3.init))
		KEEP (*(.zinitcall.exit4.init))
		__zinitcall_exit_end = .;
		. = ALIGN(4);
	} > FLASH


	/* To copy multiple FLASH to RAM sections,
	 * uncomment .copy.table section and,
	 * define __STARTUP_COPY_MULTIPLE in startup_ARMCMx.S */
	/*
	.copy.table (.) :
	{
		. = ALIGN(4);
		__copy_table_start__ = .;
		LONG (__etext)
		LONG (__data_start__)
		LONG (__data_end__ - __data_start__)
		LONG (__etext2)
		LONG (__data2_start__)
		LONG (__data2_end__ - __data2_start__)
		__copy_table_end__ = .;
	} > FLASH
	*/

	/* To clear multiple BSS sections,
	 * uncomment .zero.table section and,
	 * define __STARTUP_CLEAR_BSS_MULTIPLE in startup_ARMCMx.S */
	/*
	.zero.table (.) :
	{
		. = ALIGN(4);
		__zero_table_start__ = .;
		LONG (__bss_start__)
		LONG (__bss_end__ - __bss_start__)
		LONG (__bss2_start__)
		LONG (__bss2_end__ - __bss2_start__)
		__zero_table_end__ = .;
	} > FLASH
	*/

	/* Location counter can end up 2byte aligned with narrow Thumb code but
	   __etext is assumed by startup code to be the LMA of a section in RAM
	   which must be 4byte aligned */
	__etext = ALIGN(4);

	/* The VMA is either the end of overlay_data or the end of sram bss */

	.data : AT (__etext)
	{
		__data_start__ = .;
		_sdata = .;
		*(vtable)
		. = ALIGN(4);
		*(.data*)

		KEEP(*(.jcr*))

		/* All data end */
		. = ALIGN(4);
		__data_end__ = .;
		_edata = .;
        __shell_root_cmds_start = .;
        KEEP(*(SORT(.shell_root_cmd_*)));
        __shell_root_cmds_end = .;
	} > RAM

	.bss (.) (NOLOAD) : AT (ADDR(.bss))
	{
		. = ALIGN(4);
		__bss_start__ = .;
		_sbss = .;
		*(.bss*)
		*(COMMON)
#ifdef OS_HEAP_LINK_NAME
		. = ALIGN(64);
		__os_heap_start__ = .;
		OS_HEAP_LINK_NAME = .;
		. += OS_HEAP_SIZE;
		__os_heap_end__ = .;
#endif
		. = ALIGN(4);
		__bss_end__ = .;
		_ebss = .;
	} > RAM

#if defined(__NuttX__) && (!defined(PSRAM_BASE) || (PSRAM_SIZE == 0)) && defined(CONFIG_USE_BES_BT_STACK) && !defined(PSRAM_HEAP_ONLY)
	.nuttx_heap (.) (NOLOAD) :
	{
		_nuttx_heap_start = .;
		. = ALIGN(8);
		. += NUTTX_HEAP_SECTION_SIZE;
		. = ALIGN(8);
		_nuttx_heap_end = .;
	} > RAM
#endif

	.framebuffer (.) (NOLOAD) :
	{
		. = ALIGN(64);
		KEEP(*(.framebuffer))
	} > RAM

	.reboot_custom_param (.) (NOLOAD) :
	{
		KEEP(*(.reboot_custom_param*))
		. = ALIGN(4);
		__reboot_custom_param_end__ = .;
	} > RAM

	.heap (.) (NOLOAD) :
	{
		. = ALIGN(8);
		__HeapBase = .;
		__end__ = .;
		end = __end__;
		. += HEAP_SECTION_SIZE;
		. = ALIGN(8);
		__HeapLimit = .;
	} > RAM

#ifdef ROM_IN_FLASH
	.scratch_buf (.) (NOLOAD) :
	{
		. = ALIGN(4);
		*(.scratch_buf)
		. = ALIGN(4);
	} > RAM
#endif

	/* .stack_dummy section doesn't contains any symbols. It is only
	 * used for linker to calculate size of stack sections, and assign
	 * values to stack symbols later */
	.stack_dummy (.) (COPY) :
	{
		. = STACK_SECTION_SIZE;
		. = ALIGN(8);
	} > RAM

	/* Set stack top to end of RAM, and stack limit move down by
	 * size of stack_dummy section */
#if defined(DOOM)
	/* GBADoom port: keep all system RAM (newlib heap, BT mem-pool, stack)
	 * BELOW the fixed framebuffer at 0x20080000 so the pool/stack can never
	 * collide with it. The RAM above the FB (0x200A0000+) is doom's own
	 * dedicated heap arena (see apps/doom/bes_glue.c). */
	__StackTop = 0x20080000;
#elif defined(ROM_UTILS_ON)
	__StackTop = ORIGIN(RAM) + LENGTH(RAM) - ROM_UTILS_RESV_RAM_SIZE;
#else
	__StackTop = ORIGIN(RAM) + LENGTH(RAM);
#endif
	__StackLimit = __StackTop - SIZEOF(.stack_dummy);
	PROVIDE(__stack = __StackTop);

#if defined(__NuttX__) && !defined(CONFIG_USE_BES_BT_STACK) && \
		!((defined(PSRAM_ENABLE) || defined(PSRAM_HEAP_ONLY)) && defined(PSRAM_BASE) && (PSRAM_SIZE > 0))
	_nuttx_heap_start = __HeapLimit;
	_nuttx_heap_end = __StackLimit - 4;
#endif

	/* Check if data + heap + stack exceeds RAM limit */
	ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")
	__free_ram = __StackLimit - __HeapLimit;

#ifdef MEM_POOL_BASE
	.mem_pool (ORIGIN(MEM_POOL)) (NOLOAD) :
	{
		__mem_pool_start__ = .;
		. = LENGTH(MEM_POOL);
		__mem_pool_end__ = .;
	} > MEM_POOL
#elif !(defined(CP_IN_SAME_EE) && (RAMCP_SIZE > 0) && defined(MEM_POOL_IN_CP_RAM))
	__mem_pool_start__ = __HeapLimit;
	__mem_pool_end__ = __StackLimit;
#endif

	. = __etext + SIZEOF(.data);

#if (CUSTOMER_LOAD_RAM_DATA_SECTION_SIZE > 0)
    .inc_enc_bin_section(.) :
    {
		. = ALIGN(32);
        inc_enc_bin_sec_start = .;
        . = CUSTOMER_LOAD_ENC_DEC_RECORD_SECTION_SIZE;
        inc_enc_bin_sec_end = .;
		. = ALIGN(32);
    } > FLASH

#endif
	. = FLASH_C_TO_NC(.);
	.system_info (.) : AT (FLASH_NC_TO_C(ADDR(.system_info)))
	{
		KEEP(*(.system_info_list_size))
		KEEP(*(.system_info_list))
		. = ALIGN(4);
	}  > FLASH_NC = 0x00000000

	.build_info (.) : AT (FLASH_NC_TO_C(ADDR(.build_info)))
	{
		KEEP(*(.build_info))
		. = ALIGN(4);
	} > FLASH_NC = 0x00000000
	. = FLASH_NC_TO_C(.);

#ifdef SLAVE_BIN_FLASH_OFFSET
	.slave_code (ORIGIN(FLASH) + SLAVE_BIN_FLASH_OFFSET):
	{
		KEEP(*(.slave_code_flash_rodata))
	} > FLASH
#endif

	/* The following section MUST be the last loaded section */
	.code_start_addr (.) :
	{
#ifdef TRACE_STR_SECTION
		LONG(ABSOLUTE(__rodata_str_start__));
		LONG(ABSOLUTE(__rodata_str_end__));
		LONG(0x80);  /* RODATA_STR_OFFSET, should same to --change-section-lma .rodata_str value in Makefile  */
		LONG(ABSOLUTE(__sram_data_start__));
		LONG(ABSOLUTE(__sram_data_end__));
		LONG(0x30000);  /* SRAM_STR_OFFSET, should same to --change-section-lma .sram_data value in Makefile  */
		LONG(__trace_str_start__ - TRACE_ID_STR_OFFSET);
		LONG(__trace_str_end__ - TRACE_ID_STR_OFFSET);
		LONG(0x60000);  /* TRC_STR_OFFSET, should same to --change-section-lma .trc_str value in Makefile  */
#else
		LONG(0);
		LONG(0);
#endif
		LONG(BUILD_INFO_MAGIC);
		LONG(FLASH_C_TO_NC(ABSOLUTE(__flash_start)));
	} > FLASH

	__flash_end = .;
	ASSERT(__flash_end <= __flash_start + FLASH_IMAGE_SIZE, "flash code size too large (should <= FLASH_IMAGE_SIZE)")

#include "tail_section.lds.S"

#ifdef OTA_BOOT_SIZE
#if (OTA_BOOT_SIZE > FLASH_REGION_SIZE)
#error "OTA_BOOT_SIZE should <= FLASH_REGION_SIZE"
#endif
#ifdef OTA_CODE_OFFSET
#if defined(OTA_BOOT_OFFSET) && ((OTA_BOOT_OFFSET + OTA_BOOT_SIZE) > OTA_CODE_OFFSET)
#error "(OTA_BOOT_OFFSET + OTA_BOOT_SIZE) should <= OTA_CODE_OFFSET"
#elif (OTA_BOOT_SIZE > OTA_CODE_OFFSET)
#error "OTA_BOOT_SIZE should <= OTA_CODE_OFFSET"
#endif
#endif
#ifdef OTA_BOOT_OFFSET
	__tail_section_start = FLASH_NC_BASE + OTA_BOOT_OFFSET + OTA_BOOT_SIZE;
#else
	__tail_section_start = FLASH_NC_BASE + OTA_BOOT_SIZE;
#endif
#else /* !OTA_BOOT_SIZE */
	__tail_section_start = __flash_tail_sec_start;
#endif /* !OTA_BOOT_SIZE */

	ASSERT(FLASH_NC_TO_C(__tail_section_start) >= __flash_end, "region FLASH overflowed")
	__free_flash = FLASH_NC_TO_C(__tail_section_start) - __flash_end;

#ifdef OTA_REMAP_OFFSET
#ifndef OTA_CODE_OFFSET
#error "OTA_CODE_OFFSET should be set along with OTA_REMAP_OFFSET"
#endif
#ifndef OTA_BOOT_SIZE
	/* This is not an OTA booting code, but a normal working code */
	ASSERT((FLASH_BASE & 0x03FFFFFF) == 0, "bad FLASH_BASE")
#ifdef CHIP_BEST2300P
	ASSERT((FLASH_SIZE & (FLASH_SIZE - 1)) == 0, "bad FLASH_SIZE")
	ASSERT(__flash_end <= FLASH_BASE + OTA_REMAP_OFFSET, "flash code size too large to remap")
	ASSERT(__flash_start - FLASH_BASE >= FLASH_NC_BASE + FLASH_SIZE - __tail_section_start, "flash code conflicted with tail sections in remap")
	ASSERT(__flash_start - FLASH_BASE >= FLASH_SIZE / (1 << (4 + 1)), "flash code cannot remap within 4 regions (max code size is half of flash size)")
	ASSERT((__flash_start - FLASH_BASE) % (FLASH_SIZE / (1 << (4 + 1))) == 0, "flash code start address not aligned with remap region boundary")
#else
	ASSERT(((OTA_REMAP_OFFSET) & (0x8000 - 1)) == 0, "OTA_REMAP_OFFSET not aligned with 32K sector boundary")
	ASSERT(__flash_end <= FLASH_BASE + (OTA_CODE_OFFSET + OTA_REMAP_OFFSET), "flash code size too large to remap")
	ASSERT((__tail_section_start & (0x1000 - 1)) == 0, "__tail_section_start not aligned with 4K sector boundary")
	ASSERT(__flash_end + (OTA_REMAP_OFFSET) <= FLASH_NC_TO_C(__tail_section_start), "flash code conflicted with tail sections in remap")
#endif
#endif
#endif

#ifdef USE_MULTI_FLASH
	ASSERT((FLASH1_BASE & 0x03FFFFFF) == 0, "bad FLASH1_BASE")
	ASSERT((FLASH1_SIZE & (FLASH1_SIZE - 1)) == 0, "bad FLASH1_SIZE")
#endif

	ASSERT((__vector_table_start & (1K - 1)) == 0, "vector table alignment error")

	/DISCARD/ :
	{
		*(.interp)
		*(.hash)
		*(.dynsym)
		*(.dynstr)
		*(.gnu.version)
		*(.gnu.version_d)
		*(.gnu.version_r)
		*(.rela.plt)
		*(.rela.data.rel.ro*)
		*(.rela.gnu.linkonce.d.rel.ro*)
		*(.rela.*)
		*(.plt)
		*(.dynamic)
	}
}
