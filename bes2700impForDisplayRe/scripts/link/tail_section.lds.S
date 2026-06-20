/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#ifdef FLASH_DUAL_CHIP
#if (FLASH_DUAL_CHIP & (1 << 0))
#define FLASH0_SECTION_ALIGN				8K
#else
#define FLASH0_SECTION_ALIGN				4K
#endif
#ifdef USE_MULTI_FLASH
#if (FLASH_DUAL_CHIP & (1 << 1))
#define FLASH1_SECTION_ALIGN				8K
#else
#define FLASH1_SECTION_ALIGN				4K
#endif
#endif
#else
#define FLASH0_SECTION_ALIGN				4K
#ifdef USE_MULTI_FLASH
#define FLASH1_SECTION_ALIGN				4K
#endif
#endif

#define OTA_BOOT_INFO_SECTION_LOC			0
#define OTA_BOOT_REV_SECTION_LOC			0
#define PROMPT_SECTION_LOC					0
#define AUD_SECTION_LOC						0
#define ANC_SECTION_LOC						0
#define FACTORY_SECTION_LOC					0
#ifdef USE_MULTI_FLASH
#define COMBO_SECTION_LOC					1
#define HOTWORD_SECTION_LOC					1
#define RAM_DUMP_SECTION_LOC				1
#define BT_CONTROLLER_RAM_DUMP_SECTION_LOC	1
#define CORE_DUMP_SECTION_LOC				1
#define OTA_UPGRADE_SECTION_LOC				1
#define LOG_DUMP_SECTION_LOC				1
#define CRASH_DUMP_SECTION_LOC				1
#define CUSTOM_PARAMETER_SECTION_LOC		1
#define LHDC_LICENSE_SECTION_LOC			1
#define USERDATA_SECTION_LOC				1
#define RESERVED_SECTION_LOC				1
#else
#define COMBO_SECTION_LOC					0
#define HOTWORD_SECTION_LOC					0
#define RAM_DUMP_SECTION_LOC				0
#define BT_CONTROLLER_RAM_DUMP_SECTION_LOC	0
#define CORE_DUMP_SECTION_LOC				0
#define OTA_UPGRADE_SECTION_LOC				0
#define LOG_DUMP_SECTION_LOC				0
#define CRASH_DUMP_SECTION_LOC				0
#define CUSTOM_PARAMETER_SECTION_LOC		0
#define LHDC_LICENSE_SECTION_LOC			0
#define USERDATA_SECTION_LOC				0
#define RESERVED_SECTION_LOC				0
#endif

#ifdef USE_MULTI_FLASH
#define FLASH0_NC							FLASH_NC
#define FLASH_LOC(s)						CONCAT_NAME(CONCAT_NAME(FLASH, CONCAT_NAME(s, _SECTION_LOC)), _NC)
#define SECTION_ALIGN(s)					CONCAT_NAME(CONCAT_NAME(FLASH, CONCAT_NAME(s, _SECTION_LOC)), _SECTION_ALIGN)
#define SECTION_SIZE_LOC(n, s)				((n == CONCAT_NAME(s, _SECTION_LOC)) ? SECTION_SIZE(s) : 0)
#else
#define FLASH_LOC(s)						FLASH_NC
#define SECTION_ALIGN(s)					FLASH0_SECTION_ALIGN
#define SECTION_SIZE_LOC(n, s)				SECTION_SIZE(s)
#endif
#define SECTION_SIZE(s)						((CONCAT_NAME(s, _SECTION_SIZE) + SECTION_ALIGN(s) - 1) & ~(SECTION_ALIGN(s) - 1))

	__flash_tail_sec_start = ORIGIN(FLASH_NC) + LENGTH(FLASH_NC) -
							SECTION_SIZE_LOC(0, FACTORY) -
							SECTION_SIZE_LOC(0, RESERVED) -
							SECTION_SIZE_LOC(0, ANC) -
							SECTION_SIZE_LOC(0, USERDATA) * 2 -
							SECTION_SIZE_LOC(0, CUSTOM_PARAMETER) -
							SECTION_SIZE_LOC(0, LHDC_LICENSE) -
							SECTION_SIZE_LOC(0, CRASH_DUMP) -
							SECTION_SIZE_LOC(0, LOG_DUMP) -
							SECTION_SIZE_LOC(0, OTA_UPGRADE) -
							SECTION_SIZE_LOC(0, CORE_DUMP) -
							SECTION_SIZE_LOC(0, BT_CONTROLLER_RAM_DUMP) -
							SECTION_SIZE_LOC(0, RAM_DUMP) -
							SECTION_SIZE_LOC(0, HOTWORD) -
							SECTION_SIZE_LOC(0, PROMPT) -
							SECTION_SIZE_LOC(0, COMBO) -
							SECTION_SIZE_LOC(0, AUD) -
							SECTION_SIZE_LOC(0, OTA_APP_BOOT_INFO) -
#if defined(SPA_AUDIO_SEC_TEST)
							SECTION_SIZE_LOC(0, CUSTOMER_LOAD_ENC_DEC_RECORD) -
#endif
							0;

	.dummy_flash_tail (__flash_tail_sec_start - 16) (NOLOAD) : AT (ADDR(.dummy_flash_tail))
	{
		__dummy_flash_tail_start = .;
 		. = ALIGN(FLASH0_SECTION_ALIGN);
		__dummy_flash_tail_end = .;
	} > FLASH_NC
	ASSERT(__flash_tail_sec_start == __dummy_flash_tail_end, "Bad dummy_flash_tail")

#ifdef USE_MULTI_FLASH
	__flash1_tail_sec_start = ORIGIN(FLASH1_NC) + LENGTH(FLASH1_NC) -
							SECTION_SIZE_LOC(1, FACTORY) -
							SECTION_SIZE_LOC(1, RESERVED) -
							SECTION_SIZE_LOC(1, ANC) -
							SECTION_SIZE_LOC(1, USERDATA) * 2 -
							SECTION_SIZE_LOC(1, CUSTOM_PARAMETER) -
							SECTION_SIZE_LOC(1, LHDC_LICENSE) -
							SECTION_SIZE_LOC(1, CRASH_DUMP) -
							SECTION_SIZE_LOC(1, LOG_DUMP) -
							SECTION_SIZE_LOC(1, OTA_UPGRADE) -
							SECTION_SIZE_LOC(1, CORE_DUMP) -
							SECTION_SIZE_LOC(1, BT_CONTROLLER_RAM_DUMP) -
							SECTION_SIZE_LOC(1, RAM_DUMP) -
							SECTION_SIZE_LOC(1, HOTWORD) -
							SECTION_SIZE_LOC(1, PROMPT) -
							SECTION_SIZE_LOC(1, COMBO) -
							SECTION_SIZE_LOC(1, AUD) -
							SECTION_SIZE_LOC(1, OTA_APP_BOOT_INFO) -
							0;


	.dummy_flash1_tail (__flash1_tail_sec_start - 16) (NOLOAD) :
	{
		__dummy_flash1_tail_start = .;
		. = ALIGN(FLASH1_SECTION_ALIGN);
		__dummy_flash1_tail_end = .;
	} > FLASH1_NC
	ASSERT(__flash1_tail_sec_start == __dummy_flash1_tail_end, "Bad dummy_flash1_tail")
#endif

	.audio (NOLOAD) :
	{
		__aud_start = .;
		. = SECTION_SIZE(AUD);
		__aud_end = .;
	} > FLASH_LOC(AUD)

	.combo (NOLOAD):
	{
		__combo_start = .;
		. = SECTION_SIZE(COMBO);
		__combo_end = .;
	} > FLASH_LOC(COMBO)

	.prompt (NOLOAD):
	{
		__prompt_start = .;
		. = SECTION_SIZE(PROMPT);
		__prompt_end = .;
	} > FLASH_LOC(PROMPT)

	.hotword_model (NOLOAD):
	{
		__hotword_model_start = .;
		. = SECTION_SIZE(HOTWORD);
		__hotword_model_end = .;
	} > FLASH_LOC(HOTWORD)

	.ota_app_boot_info (NOLOAD) :
	{
		__ota_app_boot_info_start = .;
		. = SECTION_SIZE(OTA_APP_BOOT_INFO);
		__ota_app_boot_info_end = .;
	} > FLASH_LOC(OTA_APP_BOOT_INFO)

	.ramdump_section (NOLOAD) :
	{
		__ramdump_section_start = .;
		. = SECTION_SIZE(RAM_DUMP);
		__ramdump_section_end = .;
	} > FLASH_LOC(RAM_DUMP)

	.bt_controller_ramdump_section (NOLOAD) :
	{
		__bt_controller_ramdump_section_start = .;
		. = SECTION_SIZE(BT_CONTROLLER_RAM_DUMP);
		__bt_controller_ramdump_section_end = .;
	} > FLASH_LOC(BT_CONTROLLER_RAM_DUMP)

	.coredump_section (NOLOAD) :
	{
		__coredump_section_start = .;
		. = SECTION_SIZE(CORE_DUMP);
		__coredump_section_end = .;
	} > FLASH_LOC(CORE_DUMP)
#if defined(SPA_AUDIO_SEC_TEST)
	.inc_dec_bin_section (NOLOAD) :
	{
		inc_dec_bin_sec_start = .;
		. = SECTION_SIZE(CUSTOMER_LOAD_ENC_DEC_RECORD);
		inc_dec_bin_sec_end = .;
	} > FLASH_LOC(CUSTOMER_LOAD_ENC_DEC_RECORD)
#endif    

	.ota_upgrade_log (NOLOAD) :
	{
		__ota_upgrade_log_start = .;
		. = SECTION_SIZE(OTA_UPGRADE);
		__ota_upgrade_log_end = .;
	} > FLASH_LOC(OTA_UPGRADE)

	.log_dump (NOLOAD) :
	{
		__log_dump_start = .;
		. = SECTION_SIZE(LOG_DUMP);
		__log_dump_end = .;
	} > FLASH_LOC(LOG_DUMP)

	.crash_dump (NOLOAD) :
	{
		__crash_dump_start = .;
		. = SECTION_SIZE(CRASH_DUMP);
		__crash_dump_end = .;
	} > FLASH_LOC(CRASH_DUMP)

	.custom_parameter (NOLOAD) :
	{
		__custom_parameter_start = .;
		. = SECTION_SIZE(CUSTOM_PARAMETER);
		__custom_parameter_end = .;
	} > FLASH_LOC(CUSTOM_PARAMETER)

	.lhdc_license (NOLOAD) :
	{
		__lhdc_license_start = .;
		. = SECTION_SIZE(LHDC_LICENSE);
		__lhdc_license_end = .;
	} > FLASH_LOC(LHDC_LICENSE)

	.userdata (NOLOAD) :
	{
		__userdata_start = .;
		. = SECTION_SIZE(USERDATA) * 2;
		__userdata_end = .;
	} > FLASH_LOC(USERDATA)

	.anc (NOLOAD) :
	{
		__anc_start = .;
		. = SECTION_SIZE(ANC);
		__anc_end = .;
	} > FLASH_LOC(ANC)

	.reserved (NOLOAD) :
	{
		__reserved_start = .;
		. = SECTION_SIZE(RESERVED);
		__reserved_end = .;
	} > FLASH_LOC(RESERVED)

	.factory (NOLOAD) :
	{
		__factory_start = .;
		. = SECTION_SIZE(FACTORY);
		__factory_end = .;
	} > FLASH_LOC(FACTORY)
