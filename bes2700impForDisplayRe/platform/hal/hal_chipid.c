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
#include "hal_chipid.h"
#include "hal_location.h"
#include "export_fn_rom.h"

enum HAL_CHIP_METAL_ID_T BOOT_BSS_LOC metal_id;
#ifdef CHIP_BEST1306
#define EXPORT_FN_ROM_ADDR    (ROM_BASE + ROM_SIZE - ROM_BUILD_INFO_SECTION_SIZE - ROM_EXPORT_FN_SECTION_SIZE)
BOOT_BSS_LOC struct EXPORT_FN_ROM_T* export_fn_rom = NULL;
#endif

uint32_t WEAK BOOT_TEXT_FLASH_LOC read_hw_metal_id(void)
{
    return HAL_CHIP_METAL_ID_0;
}

void WEAK BOOT_TEXT_FLASH_LOC save_hw_metal_id(uint32_t id)
{
}

void BOOT_TEXT_FLASH_LOC hal_chipid_init(void)
{
    metal_id = read_hw_metal_id();
#ifdef CHIP_BEST1306
    if(metal_id >= HAL_CHIP_METAL_ID_1)
        export_fn_rom = (struct EXPORT_FN_ROM_T*)(EXPORT_FN_ROM_ADDR - 0x7000);
    else
        export_fn_rom = (struct EXPORT_FN_ROM_T*)(EXPORT_FN_ROM_ADDR);
#endif
    // Main CPU saves the id to a pre-defined location so that other subsys can get it when they startup
    save_hw_metal_id(metal_id);
}

enum HAL_CHIP_METAL_ID_T BOOT_TEXT_SRAM_LOC hal_get_chip_metal_id(void)
{
#ifdef FPGA
    return HAL_CHIP_METAL_ID_15;
#else
    return metal_id;
#endif
}

enum HAL_BT_CHIP_SERIES_T hal_get_bt_chip_series(void)
{
#if (defined(CHIP_BEST1000)) || defined(CHIP_BEST2000)
    return HAL_BT_CHIP_SERIES_2000;
#elif defined(CHIP_BEST2500)
    return HAL_BT_CHIP_SERIES_2500;
#else// (defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || defined(CHIP_BEST1400) || defined(CHIP_BEST1402))
    return HAL_BT_CHIP_SERIES_2300;
#endif
}

