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
#ifndef __HAL_LOCATION_H__
#define __HAL_LOCATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_addr_map.h"

#define HAL_SEC_CONCAT_2(a, b)          a.b
#define HAL_SEC_CONCAT_3(a, b, c)       a.b.c

#define HAL_SEC_TO_STR_A(s)             #s
#define HAL_SEC_TO_STR(s)               HAL_SEC_TO_STR_A(s)

#define HAL_SEC_NAME_ATTR(s)            __attribute__((section(s)))

#define HAL_SEC_DEF(section, name)      HAL_SEC_NAME_ATTR(HAL_SEC_TO_STR(HAL_SEC_CONCAT_2(section, name))) name
#ifdef __COUNTER__
#define HAL_SEC_LOC(section)            HAL_SEC_NAME_ATTR(HAL_SEC_TO_STR(HAL_SEC_CONCAT_3(section, __LINE__, __COUNTER__)))
#else
#define HAL_SEC_LOC(section)            HAL_SEC_NAME_ATTR(HAL_SEC_TO_STR(HAL_SEC_CONCAT_2(section, __LINE__)))
#endif

#if defined(__GNUC__) && !((defined(ROM_BUILD) && !defined(ROM_IN_FLASH)) || defined(PROGRAMMER)) && !defined(NANDFLASH_BUILD)

#define BOOT_TEXT_SRAM_LOC              HAL_SEC_LOC(.boot_text_sram)
#define BOOT_TEXT_SRAM_DEF(n)           HAL_SEC_DEF(.boot_text_sram, n)
#define BOOT_TEXT_FLASH_LOC             HAL_SEC_LOC(.boot_text_flash)
#define BOOT_TEXT_FLASH_DEF(n)          HAL_SEC_DEF(.boot_text_flash, n)
#define BOOT_RODATA_SRAM_LOC            HAL_SEC_LOC(.boot_rodata_sram)
#define BOOT_RODATA_SRAM_DEF(n)         HAL_SEC_DEF(.boot_rodata_sram, n)
#define BOOT_RODATA_FLASH_LOC           HAL_SEC_LOC(.boot_rodata_flash)
#define BOOT_RODATA_FLASH_DEF(n)        HAL_SEC_DEF(.boot_rodata_flash, n)
#define BOOT_DATA_LOC                   HAL_SEC_LOC(.boot_data)
#define BOOT_DATA_DEF(n)                HAL_SEC_DEF(.boot_data, n)
#ifdef __ARMCC_VERSION
#define BOOT_BSS_LOC                    HAL_SEC_LOC(.bss.boot_bss)
#define BOOT_BSS_DEF(n)                 HAL_SEC_DEF(.bss.boot_bss, n)
#else
#define BOOT_BSS_LOC                    HAL_SEC_LOC(.boot_bss)
#define BOOT_BSS_DEF(n)                 HAL_SEC_DEF(.boot_bss, n)
#endif

#define SRAM_TEXT_LOC                   HAL_SEC_LOC(.sram_text)
#define SRAM_TEXT_DEF(n)                HAL_SEC_DEF(.sram_text, n)
#define SRAM_RODATA_LOC                 HAL_SEC_LOC(.sram_rodata)
#define SRAM_RODATA_DEF(n)              HAL_SEC_DEF(.sram_rodata, n)
#define SRAM_DATA_LOC                   HAL_SEC_LOC(.sram_data)
#define SRAM_DATA_DEF(n)                HAL_SEC_DEF(.sram_data, n)
#define SRAM_STACK_LOC                  ALIGNED(8) HAL_SEC_LOC(.sram_data)
#define SRAM_STACK_DEF(n)               ALIGNED(8) HAL_SEC_LOC(.sram_data, n)
#ifdef __ARMCC_VERSION
#define SRAM_BSS_LOC                    HAL_SEC_LOC(.bss.sram_bss)
#define SRAM_BSS_DEF(n)                 HAL_SEC_DEF(.bss.sram_bss, n)
#define SRAM_NC_BSS_LOC                 HAL_SEC_LOC(.bss.sram_nc_bss)
#define SRAM_NC_BSS_DEF(n)              HAL_SEC_DEF(.bss.sram_nc_bss, n)
#else
#define SRAM_BSS_LOC                    HAL_SEC_LOC(.sram_bss)
#define SRAM_BSS_DEF(n)                 HAL_SEC_DEF(.sram_bss, n)
#define SRAM_NC_BSS_LOC                 HAL_SEC_LOC(.sram_nc_bss)
#define SRAM_NC_BSS_DEF(n)              HAL_SEC_DEF(.sram_nc_bss, n)
#endif

#define FRAM_TEXT_LOC                   HAL_SEC_LOC(.fast_text_sram)
#define FRAM_TEXT_DEF(n)                HAL_SEC_DEF(.fast_text_sram, n)

#define CP_TEXT_SRAM_LOC                HAL_SEC_LOC(.cp_text_sram)
#define CP_TEXT_SRAM_DEF(n)             HAL_SEC_DEF(.cp_text_sram, n)
#define CP_DATA_LOC                     HAL_SEC_LOC(.cp_data)
#define CP_DATA_DEF(n)                  HAL_SEC_DEF(.cp_data, n)
#ifdef __ARMCC_VERSION
#define CP_BSS_LOC                      HAL_SEC_LOC(.bss.cp_bss)
#define CP_BSS_DEF(n)                   HAL_SEC_DEF(.bss.cp_bss, n)
#else
#define CP_BSS_LOC                      HAL_SEC_LOC(.cp_bss)
#define CP_BSS_DEF(n)                   HAL_SEC_DEF(.cp_bss, n)
#endif

#define FLASH_TEXT_LOC                  HAL_SEC_LOC(.flash_text)
#define FLASH_TEXT_DEF(n)               HAL_SEC_DEF(.flash_text, n)
#define FLASH_RODATA_LOC                HAL_SEC_LOC(.flash_rodata)
#define FLASH_RODATA_DEF(n)             HAL_SEC_DEF(.flash_rodata, n)

#define REBOOT_CUSTOM_PARAM_LOC         HAL_SEC_LOC(.reboot_custom_param)
#define REBOOT_CUSTOM_PARAM_DEF         HAL_SEC_DEF(.reboot_custom_param)

#ifdef PSRAM_BASE
#define PSRAM_TEXT_LOC                  HAL_SEC_LOC(.psram_text)
#define PSRAM_TEXT_DEF(n)               HAL_SEC_DEF(.psram_text, n)
#define PSRAM_RODATA_LOC                HAL_SEC_LOC(.psram_rodata)
#define PSRAM_RODATA_DEF(n)             HAL_SEC_DEF(.psram_rodata, n)
#define PSRAM_DATA_LOC                  HAL_SEC_LOC(.psram_data)
#define PSRAM_DATA_DEF(n)               HAL_SEC_DEF(.psram_data, n)
#define PSRAM_STACK_LOC                 ALIGNED(8) HAL_SEC_LOC(.psram_data)
#define PSRAM_STACK_DEF(n)              ALIGNED(8) HAL_SEC_LOC(.psram_data, n)
#ifdef __ARMCC_VERSION
#define PSRAM_BSS_LOC                   HAL_SEC_LOC(.bss.psram_bss)
#define PSRAM_BSS_DEF(n)                HAL_SEC_DEF(.bss.psram_bss, n)
#define PSRAM_NC_BSS_LOC                HAL_SEC_LOC(.bss.psram_nc_bss)
#define PSRAM_NC_BSS_DEF(n)             HAL_SEC_DEF(.bss.psram_nc_bss, n)
#else
#define PSRAM_BSS_LOC                   HAL_SEC_LOC(.psram_bss)
#define PSRAM_BSS_DEF(n)                HAL_SEC_DEF(.psram_bss, n)
#define PSRAM_NC_BSS_LOC                HAL_SEC_LOC(.psram_nc_bss)
#define PSRAM_NC_BSS_DEF(n)             HAL_SEC_DEF(.psram_nc_bss, n)
#endif
#define SYNC_FLAGS_PSRAM_LOC            HAL_SEC_LOC(.sync_flags_psram)
#define SYNC_FLAGS_PSRAM_DEF(n)         HAL_SEC_DEF(.sync_flags_psram, n)
#endif

#ifdef PSRAMUHS_BASE
#define PSRAMUHS_TEXT_LOC               HAL_SEC_LOC(.psramuhs_text)
#define PSRAMUHS_TEXT_DEF(n)            HAL_SEC_DEF(.psramuhs_text, n)
#define PSRAMUHS_RODATA_LOC             HAL_SEC_LOC(.psramuhs_rodata)
#define PSRAMUHS_RODATA_DEF(n)          HAL_SEC_DEF(.psramuhs_rodata, n)
#define PSRAMUHS_DATA_LOC               HAL_SEC_LOC(.psramuhs_data)
#define PSRAMUHS_DATA_DEF(n)            HAL_SEC_DEF(.psramuhs_data, n)
#define PSRAMUHS_STACK_LOC              ALIGNED(8) HAL_SEC_LOC(.psramuhs_data)
#define PSRAMUHS_STACK_DEF(n)           ALIGNED(8) HAL_SEC_LOC(.psramuhs_data, n)
#ifdef __ARMCC_VERSION
#define PSRAMUHS_BSS_LOC                HAL_SEC_LOC(.bss.psramuhs_bss)
#define PSRAMUHS_BSS_DEF(n)             HAL_SEC_DEF(.bss.psramuhs_bss, n)
#define PSRAMUHS_NC_BSS_LOC             HAL_SEC_LOC(.bss.psramuhs_nc_bss)
#define PSRAMUHS_NC_BSS_DEF(n)          HAL_SEC_DEF(.bss.psramuhs_nc_bss, n)
#else
#define PSRAMUHS_BSS_LOC                HAL_SEC_LOC(.psramuhs_bss)
#define PSRAMUHS_BSS_DEF(n)             HAL_SEC_DEF(.psramuhs_bss, n)
#define PSRAMUHS_NC_BSS_LOC             HAL_SEC_LOC(.psramuhs_nc_bss)
#define PSRAMUHS_NC_BSS_DEF(n)          HAL_SEC_DEF(.psramuhs_nc_bss, n)
#endif
#define SYNC_FLAGS_PSRAMUHS_LOC         HAL_SEC_LOC(.sync_flags_psramuhs)
#define SYNC_FLAGS_PSRAMUHS_DEF(n)      HAL_SEC_DEF(.sync_flags_psramuhs, n)
#endif

#ifdef ARM_CMSE
#ifdef INTSRAM_RUN // best1501
#define CMSE_API_TEXT_LOC               SRAM_TEXT_LOC
#else
#define CMSE_API_TEXT_LOC               FLASH_TEXT_LOC
#endif
#define CMSE_API                        CMSE_API_TEXT_LOC __attribute__((cmse_nonsecure_entry))
#endif

#define CONST_STR_LOC(s, L)             ({ static const char L __cstrname[] = (s); __cstrname; })

#else // !__GNUC__ || ROM_BUILD || PROGRAMMER

#if defined(__GNUC__) && defined(ROM_SRAM_TEXT_SIMU)
#define BOOT_TEXT_SRAM_LOC              HAL_SEC_LOC(.rom_ramx)
#define BOOT_TEXT_SRAM_DEF(n)           HAL_SEC_DEF(.rom_ramx, n)
#else
#define BOOT_TEXT_SRAM_LOC
#define BOOT_TEXT_SRAM_DEF(n)           n
#endif
#if defined(__GNUC__) && defined(PROGRAMMER_INFLASH)
#define BOOT_TEXT_FLASH_LOC             HAL_SEC_LOC(.boot_text_flash)
#define BOOT_TEXT_FLASH_DEF(n)          HAL_SEC_DEF(.boot_text_flash, n)
#define BOOT_RODATA_FLASH_LOC           HAL_SEC_LOC(.boot_rodata_flash)
#define BOOT_RODATA_FLASH_DEF(n)        HAL_SEC_DEF(.boot_rodata_flash, n)
#else
#define BOOT_TEXT_FLASH_LOC
#define BOOT_TEXT_FLASH_DEF(n)          n
#define BOOT_RODATA_FLASH_LOC
#define BOOT_RODATA_FLASH_DEF(n)        n
#endif
#if defined(__GNUC__) && defined(ROM_SRAM_TEXT_SIMU)
#define BOOT_RODATA_SRAM_LOC            HAL_SEC_LOC(.data)
#define BOOT_RODATA_SRAM_DEF(n)         HAL_SEC_DEF(.data, n)
#else
#define BOOT_RODATA_SRAM_LOC
#define BOOT_RODATA_SRAM_DEF(n)         n
#endif
#define BOOT_DATA_LOC
#define BOOT_DATA_DEF(n)                n
#define BOOT_BSS_LOC
#define BOOT_BSS_DEF(n)                 n

#if defined(__GNUC__) && defined(ROM_SRAM_TEXT_SIMU)
#define SRAM_TEXT_LOC                   HAL_SEC_LOC(.rom_ramx)
#define SRAM_TEXT_DEF(n)                HAL_SEC_DEF(.rom_ramx, n)
#else
#define SRAM_TEXT_LOC
#define SRAM_TEXT_DEF(n)                n
#endif
#define SRAM_RODATA_LOC
#define SRAM_RODATA_DEF(n)              n
#define SRAM_DATA_LOC
#define SRAM_DATA_DEF(n)                n
#define SRAM_STACK_LOC
#define SRAM_STACK_DEF(n)               n
#define SRAM_BSS_LOC
#define SRAM_BSS_DEF(n)                 n
#define SRAM_NC_BSS_LOC
#define SRAM_NC_BSS_DEF(n)              n

#define FRAM_TEXT_LOC
#define FRAM_TEXT_DEF(n)                n

#if defined(__GNUC__) && defined(PROGRAMMER) && defined(PROGRAMMER_CP_SECTION) && !defined(__PIE__)
#define CP_TEXT_SRAM_LOC                HAL_SEC_LOC(.cp_text_sram)
#define CP_TEXT_SRAM_DEF(n)             HAL_SEC_DEF(.cp_text_sram, n)
#define CP_DATA_LOC                     HAL_SEC_LOC(.cp_data)
#define CP_DATA_DEF(n)                  HAL_SEC_DEF(.cp_data, n)
#ifdef __ARMCC_VERSION
#define CP_BSS_LOC                      HAL_SEC_LOC(.bss.cp_bss)
#define CP_BSS_DEF(n)                   HAL_SEC_DEF(.bss.cp_bss, n)
#else
#define CP_BSS_LOC                      HAL_SEC_LOC(.cp_bss)
#define CP_BSS_DEF(n)                   HAL_SEC_DEF(.cp_bss, n)
#endif
#else
#define CP_TEXT_SRAM_LOC
#define CP_TEXT_SRAM_DEF(n)             n
#define CP_DATA_LOC
#define CP_DATA_DEF(n)                  n
#define CP_BSS_LOC
#define CP_BSS_DEF(n)                   n
#endif

#define FLASH_TEXT_LOC
#define FLASH_TEXT_DEF(n)               n
#define FLASH_RODATA_LOC
#define FLASH_RODATA_DEF(n)             n

#define REBOOT_CUSTOM_PARAM_LOC
#define REBOOT_CUSTOM_PARAM_DEF(n)      n

#define PSRAM_TEXT_LOC
#define PSRAM_TEXT_DEF(n)               n
#define PSRAM_RODATA_LOC
#define PSRAM_RODATA_DEF(n)             n
#define PSRAM_DATA_LOC
#define PSRAM_DATA_DEF(n)               n
#define PSRAM_STACK_LOC
#define PSRAM_STACK_DEF(n)              n
#define PSRAM_BSS_LOC
#define PSRAM_BSS_DEF(n)                n
#define PSRAM_NC_BSS_LOC
#define PSRAM_NC_BSS_DEF(n)             n
#define SYNC_FLAGS_PSRAM_LOC
#define SYNC_FLAGS_PSRAM_DEF(n)         n

#define PSRAMUHS_TEXT_LOC
#define PSRAMUHS_TEXT_DEF(n)            n
#define PSRAMUHS_RODATA_LOC
#define PSRAMUHS_RODATA_DEF(n)          n
#define PSRAMUHS_DATA_LOC
#define PSRAMUHS_DATA_DEF(n)            n
#define PSRAMUHS_STACK_LOC
#define PSRAMUHS_STACK_DEF(n)           n
#define PSRAMUHS_BSS_LOC
#define PSRAMUHS_BSS_DEF(n)             n
#define PSRAMUHS_NC_BSS_LOC
#define PSRAMUHS_NC_BSS_DEF(n)          n
#define SYNC_FLAGS_PSRAMUHS_LOC
#define SYNC_FLAGS_PSRAMUHS_DEF(n)      n

#define CONST_STR_LOC(s, L)             s

#endif // !__GNUC__ || ROM_BUILD || PROGRAMMER

#if defined(__GNUC__) && (defined(__ARM_ARCH_ISA_ARM) || defined(RAM_NC_BASE))
#define SYNC_FLAGS_LOC                  HAL_SEC_LOC(.sync_flags_sram)
#define SYNC_FLAGS_DEF(n)               HAL_SEC_DEF(.sync_flags_sram, n)
#else
#define SYNC_FLAGS_LOC
#define SYNC_FLAGS_DEF(n)               n
#endif

#define CONST_STR_BOOT_SRAM(s)          CONST_STR_LOC(s, BOOT_RODATA_SRAM_LOC)
#define CONST_STR_SRAM(s)               CONST_STR_LOC(s, SRAM_RODATA_LOC)
#define CONST_STR_FLASH(s)              CONST_STR_LOC(s, FLASH_RODATA_LOC)
#define CONST_STR_PSRAM(s)              CONST_STR_LOC(s, PSRAM_RODATA_LOC)
#define CONST_STR_PSRAMUHS(s)           CONST_STR_LOC(s, PSRAMUHS_RODATA_LOC)

#ifdef __cplusplus
}
#endif

#endif
