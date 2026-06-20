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
#ifdef RAM_DUMP_TO_FLASH
#include <stdio.h>
#include <string.h>
#include <string.h>
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_norflash.h"
#include "norflash_api.h"
#include "watchdog.h"
#include "ramdump_section.h"

extern uint32_t __ramdump_section_start;
extern uint32_t __ramdump_section_end;

static const uint32_t ramdump_flash_start_addr = (uint32_t)&__ramdump_section_start;
static const uint32_t ramdump_flash_end_addr = (uint32_t)&__ramdump_section_end;
static uint32_t ramdump_ram_start = RAM_BASE;
static uint32_t ramdump_ram_size = RAM_SIZE;

extern uint32_t __bt_controller_ramdump_section_start;
extern uint32_t __bt_controller_ramdump_section_end;

static const uint32_t bt_controller_ramdump_flash_start_addr = (uint32_t)&__bt_controller_ramdump_section_start;
static const uint32_t bt_controller_ramdump_flash_end_addr = (uint32_t)&__bt_controller_ramdump_section_end;

enum HAL_TRACE_STATE_T ramdump_execute_state_machine = HAL_TRACE_STATE_CRASH_UNINITIALIZED;
void ramdump_update_execute_state_machine(enum HAL_TRACE_STATE_T ramdump_execute_state)
{
    ramdump_execute_state_machine = ramdump_execute_state;
}

enum HAL_TRACE_STATE_T ramdump_get_execute_state_machine(void)
{
    return ramdump_execute_state_machine;
}

void ramdump_to_flash_init(void)
{
    uint32_t block_size = 0;
    uint32_t sector_size = 0;
    uint32_t page_size = 0;
    uint32_t buffer_len = 0;
    enum NORFLASH_API_RET_T result;

    hal_norflash_get_size(HAL_FLASH_ID_0,NULL, &block_size, &sector_size, &page_size);
    buffer_len = sector_size;
    RAM_DUMP_TRACE(4,RAM_DUMP_PREFIX"%s: ramdump_flash_start_addr = 0x%x, ramdump_flash_end_addr = 0x%x, buff_len = 0x%x.",
                    __func__,
                    ramdump_flash_start_addr,
                    ramdump_flash_end_addr,
                    buffer_len);

    result = norflash_api_register(
                    NORFLASH_API_MODULE_ID_RAMDUMP,
                    HAL_FLASH_ID_0,
                    ramdump_flash_start_addr,
                    (ramdump_flash_end_addr - ramdump_flash_start_addr) +
                    (bt_controller_ramdump_flash_end_addr - bt_controller_ramdump_flash_start_addr),
                    block_size,
                    sector_size,
                    page_size,
                    buffer_len,
                    NULL
                    );
    if(result == NORFLASH_API_OK)
    {
        RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"ramdump_to_flash_init ok.");
    }
    else
    {
        RAM_DUMP_TRACE(4,RAM_DUMP_PREFIX"ramdump_to_flash_init failed,result = %d.",result);
    }

#if !(defined(DUMP_LOG_ENABLE) || defined(DUMP_CRASH_ENABLE) || defined(TOTA_CRASH_DUMP_TOOL_ENABLE))
    hal_trace_app_register(HAL_TRACE_APP_REG_ID_2, ramdump_to_flash_handler, NULL);
#endif

    ramdump_update_execute_state_machine(HAL_TRACE_STATE_CRASH_END);
}

static void ramdump_flash_write(uint32_t startFlashAddress, uint32_t sourceRamStartAddress, uint32_t length)
{
    uint32_t i;
    uint32_t sector_count;
    uint32_t sector_size = 0;
    uint32_t addr, len, offs;
    uint8_t *pbuff;
    enum NORFLASH_API_RET_T result;

    hal_norflash_get_size(HAL_FLASH_ID_0,NULL, NULL, &sector_size, NULL);

    sector_count = length/sector_size;
    for(i = 0; i < sector_count; i++)
    {
        offs = i * sector_size;
        addr = startFlashAddress + offs;
        pbuff = (uint8_t*)sourceRamStartAddress + offs;

        len = offs + sector_size <= length ? sector_size : length - offs;
        if(offs + sector_size <= length )
        {
            len = sector_size;
        }
        else
        {
            if(offs < length)
            {
                len = length - offs;
            }
            else
            {
                // RAM dump completed.
                len = 0;
                break;
            }
        }
        result = norflash_api_erase(NORFLASH_API_MODULE_ID_RAMDUMP,
                                    addr,
                                    sector_size,
                                    false);
        if(result != NORFLASH_API_OK)
        {
            RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"ram dump: erase failed, result = %d, addr = 0x%x", result,addr);
            break;
        }

        result = norflash_api_write(NORFLASH_API_MODULE_ID_RAMDUMP,
                                    addr,
                                    pbuff,
                                    len,
                                    false);
        if(result != NORFLASH_API_OK)
        {
            RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"ram dump: write failed, result = %d, addr = 0x%x", result,addr);
            break;
        }
    }
}

void ramdump_to_flash_handler(enum HAL_TRACE_STATE_T ramdump_executor)
{
    if (HAL_TRACE_STATE_CRASH_END == ramdump_executor)
    {
        RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"%s: end ramdump!", __func__);
        return;
    }

    RAM_DUMP_TRACE(1,RAM_DUMP_PREFIX"%s: RAM: 0x%x-0x%x,FLASH: 0x%x-0x%x",
            __func__,
            ramdump_ram_start,
            ramdump_ram_start + ramdump_ram_size,
            ramdump_flash_start_addr,
            ramdump_flash_end_addr
            );

    if (HAL_TRACE_STATE_CRASH_UNINITIALIZED == ramdump_get_execute_state_machine())
    {
        RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"%s: uninit!", __func__);
        return;
    }

    if (HAL_TRACE_STATE_CRASH_END != ramdump_get_execute_state_machine())
    {
        RAM_DUMP_TRACE(0,RAM_DUMP_PREFIX"%s: execution may ERROR!!!", __func__);
        return;
    }

    ramdump_update_execute_state_machine(ramdump_executor);

    watchdog_hw_stop();
    ramdump_flash_write(ramdump_flash_start_addr, ramdump_ram_start, ramdump_ram_size);

#ifdef __BT_RAMRUN_NEW__
    uint32_t bt_controller_ram_dump_start_addr = BTC_ROM_RAM_BASE;
    uint32_t bt_dump_len_max                   = BTC_ROM_RAM_SIZE;
#else
    uint32_t bt_controller_ram_dump_start_addr = BT_RAM_BASE;
    uint32_t bt_dump_len_max                   = BT_RAM_SIZE;
#endif

    uint32_t em_dump_area_2_start = BT_EXCH_MEM_BASE;
    uint32_t em_area_2_len_max    = BT_EXCH_MEM_SIZE;

    ramdump_flash_write(bt_controller_ramdump_flash_start_addr, bt_controller_ram_dump_start_addr,
        bt_dump_len_max);

    ramdump_flash_write(bt_controller_ramdump_flash_start_addr+bt_dump_len_max, em_dump_area_2_start,
        em_area_2_len_max);

    watchdog_hw_start(15);

    ramdump_update_execute_state_machine(HAL_TRACE_STATE_CRASH_END);
    RAM_DUMP_TRACE(1,RAM_DUMP_PREFIX"%s done", __func__);

}

#endif /*RAM_DUMP_TO_FLASH*/

