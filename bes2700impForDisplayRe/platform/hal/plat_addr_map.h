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
#ifndef __PLAT_ADDR_MAP_H__
#define __PLAT_ADDR_MAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#if 0
#elif defined(CHIP_FPGA1000)
#define CHIP_ID_LITERAL                         fpga1000
#elif defined(CHIP_BEST1000)
#define CHIP_ID_LITERAL                         best1000
#elif defined(CHIP_BEST1305)
#define CHIP_ID_LITERAL                         best1305
#elif defined(CHIP_BEST1306)
#define CHIP_ID_LITERAL                         best1306
#elif defined(CHIP_BEST1306P)
#define CHIP_ID_LITERAL                         best1306p
#elif defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define CHIP_ID_LITERAL                         best1400
#elif defined(CHIP_BEST1501)
#define CHIP_ID_LITERAL                         best1501
#elif defined(CHIP_BEST1501P)
#define CHIP_ID_LITERAL                         best1501p
#elif defined(CHIP_BEST1502P)
#define CHIP_ID_LITERAL                         best1502p
#elif defined(CHIP_BEST1502X)
#define CHIP_ID_LITERAL                         best1502x
#elif defined(CHIP_BEST1503)
#define CHIP_ID_LITERAL                         best1503
#elif defined(CHIP_BEST1600)
#define CHIP_ID_LITERAL                         best1600
#elif defined(CHIP_BEST2000)
#define CHIP_ID_LITERAL                         best2000
#elif defined(CHIP_BEST2001)
#define CHIP_ID_LITERAL                         best2001
#elif defined(CHIP_BEST2003)
#define CHIP_ID_LITERAL                         best2003
#elif defined(CHIP_BEST2300)
#define CHIP_ID_LITERAL                         best2300
#elif defined(CHIP_BEST2300A)
#define CHIP_ID_LITERAL                         best2300a
#elif defined(CHIP_BEST2300P)
#define CHIP_ID_LITERAL                         best2300p
#elif defined(CHIP_BEST3001) || defined(CHIP_BEST3005)
#define CHIP_ID_LITERAL                         best3001
#elif defined(CHIP_BEST3003)
#define CHIP_ID_LITERAL                         best3003
#elif defined(CHIP_BEST1501SIMU)
#define CHIP_ID_LITERAL                         best1501simu
#elif defined(CHIP_BEST1600SIMU)
#define CHIP_ID_LITERAL                         best1600simu
#else
#error "Unknown chip ID"
#endif

#define _TO_STRING_A(s)                         # s
#define _TO_STRING(s)                           _TO_STRING_A(s)

#define CONCAT_NAME_A(a, b)                     a ## b
#define CONCAT_NAME(a, b)                       CONCAT_NAME_A(a, b)
#define CONCAT_SUFFIX(a, b)                     a.b

#define CONSTRUCT_HDR_NAME_A(d, n)              d/n ## _ ## d
#define CONSTRUCT_HDR_NAME(d, n)                CONSTRUCT_HDR_NAME_A(d, n)
#define CHIP_SPECIFIC_HDR(n)                    _TO_STRING(CONSTRUCT_HDR_NAME(CHIP_ID_LITERAL, n).h)

#include CHIP_SPECIFIC_HDR(plat_addr_map)

#ifndef ROMX_BASE
#define ROMX_BASE                               ROM_BASE
#endif

#ifndef ROM_EXT_SIZE
#define ROM_EXT_SIZE                            0
#endif

#define ROM_TOTAL_SIZE                          (ROM_SIZE + ROM_EXT_SIZE)

#ifdef MAIN_RAM_USE_TCM
#define RAM_TO_RAMX(d)                          (d)
#define RAMX_TO_RAM(d)                          (d)
#else
#define RAM_TO_RAMX(d)                          ((d) - RAM_BASE + RAMX_BASE)
#define RAMX_TO_RAM(d)                          ((d) - RAMX_BASE + RAM_BASE)
#endif

#ifdef RAM_SECURITY_FLAG
#define RAM_NS_TO_S(a)                          ((a) | RAM_SECURITY_FLAG)
#define RAM_S_TO_NS(a)                          ((a) & ~RAM_SECURITY_FLAG)
#else
#define RAM_NS_TO_S(a)                          (a)
#define RAM_S_TO_NS(a)                          (a)
#endif

#define FLASH_TO_FLASHX(d)                      ((d) - FLASH_BASE + FLASHX_BASE)
#define FLASHX_TO_FLASH(d)                      ((d) - FLASHX_BASE + FLASH_BASE)

#define PSRAM_TO_PSRAMX(d)                      ((d) - PSRAM_BASE + PSRAMX_BASE)
#define PSRAMX_TO_PSRAM(d)                      ((d) - PSRAMX_BASE + PSRAM_BASE)

#define PSRAMUHSX_TO_PSRAMUHS(d)                ((d) - PSRAMUHSX_BASE + PSRAMUHS_BASE)
#define PSRAMUHS_TO_PSRAMUHSX(d)                ((d) - PSRAMUHS_BASE + PSRAMUHSX_BASE)

#define FLASH_C_TO_NC(d)                        ((d) - FLASH_BASE + FLASH_NC_BASE)
#define FLASH_NC_TO_C(d)                        ((d) - FLASH_NC_BASE + FLASH_BASE)

#define FLASH1_C_TO_NC(d)                       ((d) - FLASH1_BASE + FLASH1_NC_BASE)
#define FLASH1_NC_TO_C(d)                       ((d) - FLASH1_NC_BASE + FLASH1_BASE)

#define FLASH2_C_TO_NC(d)                       ((d) - FLASH2_BASE + FLASH2_NC_BASE)
#define FLASH2_NC_TO_C(d)                       ((d) - FLASH2_NC_BASE + FLASH2_BASE)

#define PSRAM_C_TO_NC(d)                        ((d) - PSRAM_BASE + PSRAM_NC_BASE)
#define PSRAM_NC_TO_C(d)                        ((d) - PSRAM_NC_BASE + PSRAM_BASE)

#define PSRAMUHS_C_TO_NC(d)                     ((d) - PSRAMUHS_BASE + PSRAMUHS_NC_BASE)
#define PSRAMUHS_NC_TO_C(d)                     ((d) - PSRAMUHS_NC_BASE + PSRAMUHS_BASE)

#define BUILD_INFO_MAGIC                        0xBE57341D

#define TRACE_ID_STR_OFFSET                     0xFFC00000

#if (RAM_BASE & (0x400 - 1))
#error "RAM_BASE should be aligned on 0x400 boundary"
#endif
#if defined(RAMCP_BASE) && (RAMCP_BASE & (0x400 - 1))
#error "RAMCP_BASE should be aligned on 0x400 boundary"
#endif

#ifdef __cplusplus
}
#endif

#endif
