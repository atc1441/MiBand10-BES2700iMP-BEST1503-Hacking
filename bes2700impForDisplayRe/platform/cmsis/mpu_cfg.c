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
#include "plat_addr_map.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "mpu.h"
#include "mpu_cfg.h"

#if (ROMX_BASE >= 0x20000)
// ROMX_BASE with security flag will be a large value
#define ROM_DATA_MAP_BASE                       0x20000
#elif (ROMX_BASE >= 0x800)
#define ROM_DATA_MAP_BASE                       ROMX_BASE
#else
#ifdef CHIP_BEST1000
#define ROM_DATA_MAP_BASE                       0x400
#else
#define ROM_DATA_MAP_BASE                       0x800
#endif
#endif

#ifndef RAM_TOTAL_SIZE
#define RAM_TOTAL_SIZE                          RAM_SIZE
#endif

BOOT_RODATA_FLASH_LOC
static const mpu_regions_t mpu_boot_table[] = {
#if defined(__ARM_ARCH_8M_MAIN__)
/* Never overlap the memory region for armv8 MPU, or there will be data abort
 * When access overlaped memory
 */
#ifdef MPU_INIT_TABLE
    MPU_INIT_TABLE
#elif defined(NO_MPU_DEFAULT_MAP)
#define RAM_RX_START                            ROM_DATA_MAP_BASE
#define RAM_RX_END                              0x20000000
#define RAM_RWX_START                           0x20000000
#define RAM_RWX_END                             0x40000000
    {RAM_RX_START,  (RAM_RX_END - RAM_RX_START),    MPU_ATTR_READ_EXEC,         MEM_ATTR_INT_SRAM },
    {RAM_RWX_START, (RAM_RWX_END - RAM_RWX_START),  MPU_ATTR_READ_WRITE_EXEC,   MEM_ATTR_INT_SRAM },
    {0x40000000,    (0xE0000000 - 0x40000000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
#else
    {0,             ROM_DATA_MAP_BASE,              MPU_ATTR_EXEC,              MEM_ATTR_INT_SRAM },
    {RAMX_BASE,     RAM_TOTAL_SIZE,                 MPU_ATTR_READ_EXEC,         MEM_ATTR_INT_SRAM },
#ifdef WIFI_PAS_BASE
    // Exchange memory end to wifi space end
    {0x82020000,    (0xA0000000 - 0x82020000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
#elif defined(WIFI_SUBSYS_BASE)
    // Register space start to wifi space end
    {0x90000000,    (0xA0000000 - 0x90000000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
#endif
#if 0 //def BT_SUBSYS_BASE
    // Register space 0 start to exchange memory 0 start
    {0xD0000000,    (0xD0210000 - 0xD0000000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
    // Register space 1 start to exchange memory 1 start
    {0xD0220000,    (0xD0230000 - 0xD0220000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
    // Register space 2 start to bt space end
    {0xD0240000,    (0xE0000000 - 0xD0230000),      MPU_ATTR_READ_WRITE,        MEM_ATTR_DEVICE   },
#endif
#endif
#else
/* Can overlap the memory region for armv7 MPU, the attribute of high number region
 * will override the low one
 */
    {0,             ROM_DATA_MAP_BASE,    MPU_ATTR_NO_ACCESS  },
    {RAMX_BASE,     RAM_TOTAL_SIZE,       MPU_ATTR_READ_EXEC  },
    {RAM_BASE,      RAM_TOTAL_SIZE,       MPU_ATTR_READ_WRITE },
#endif
};

#if defined(__ARM_ARCH_8M_MAIN__) && defined(MPU_INIT_TABLE)
BOOT_TEXT_FLASH_LOC
void mpu_boot_cfg(void)
{
    mpu_boot_setup(mpu_boot_table, ARRAY_SIZE(mpu_boot_table));
}
#endif

void mpu_cfg(void)
{
    int ret;
    uint32_t lock;

    lock = int_lock_global();
    ret = mpu_setup(mpu_boot_table, ARRAY_SIZE(mpu_boot_table));
    int_unlock_global(lock);
    if (ret) {
        TR_INFO(TR_MOD(MAIN), "Warning, MPU is not setup correctly: %d", ret);
    }
}
