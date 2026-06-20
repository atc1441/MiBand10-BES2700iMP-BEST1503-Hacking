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
#ifdef ARM_CMSE
#include "plat_types.h"
#include "plat_addr_map.h"
#include "arm_cmse.h"
#include "cmsis.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "string.h"

#if 0
// Not supported: avoid returning a secured pointer
CMSE_API const struct HAL_NORFLASH_CONFIG_T *hal_norflash_get_init_config_se(void)
{
    return hal_norflash_get_init_config();
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_init_config_ex_se(struct HAL_NORFLASH_CONFIG_T *cfg)
{
    if (cfg) {
        cfg = cmse_check_address_range((void *)cfg, sizeof(*cfg), CMSE_NONSECURE);
        if (!cfg) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    memcpy(cfg, hal_norflash_get_init_config(), sizeof(*cfg));
    return HAL_NORFLASH_OK;
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_init_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_init(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_open_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg)
{
    if (cfg) {
        cfg = cmse_check_address_range((void *)cfg, sizeof(*cfg), CMSE_NONSECURE);
        if (!cfg) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_open(id, cfg);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_reopen_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg)
{
    if (cfg) {
        cfg = cmse_check_address_range((void *)cfg, sizeof(*cfg), CMSE_NONSECURE);
        if (!cfg) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_reopen(id, cfg);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_apply_config_se(enum HAL_FLASH_ID_T id, const struct HAL_NORFLASH_CONFIG_T *cfg, const struct HAL_NORFLASH_TIMING_CONFIG_T *timing)
{
    if (cfg) {
        cfg = cmse_check_address_range((void *)cfg, sizeof(*cfg), CMSE_NONSECURE);
        if (!cfg) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (timing) {
        timing = cmse_check_address_range((void *)timing, sizeof(*timing), CMSE_NONSECURE);
        if (!timing) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_apply_config(id, cfg, timing);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_timing_config_se(enum HAL_FLASH_ID_T id, struct HAL_NORFLASH_TIMING_CONFIG_T *timing)
{
    if (timing) {
        timing = cmse_check_address_range((void *)timing, sizeof(*timing), CMSE_NONSECURE);
        if (!timing) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_timing_config(id, timing);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_id_se(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    if (value) {
        value = cmse_check_address_range(value, len, CMSE_NONSECURE);
        if (!value) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_id(id, value, len);
}

CMSE_API enum HAL_FLASH_ID_T hal_norflash_addr_to_id_se(uint32_t addr)
{
    return hal_norflash_addr_to_id(addr);
}

CMSE_API uint32_t hal_norflash_get_addr_offset_se(uint32_t addr)
{
    return hal_norflash_get_addr_offset(addr);
}

static int hal_norflash_addr_is_secured(enum HAL_FLASH_ID_T id, uint32_t addr, uint32_t len)
{
    // Flash memory address has too many aliases and is hard to fully cover by cmse_check_address_range()
    uint32_t offset;
    uint32_t size = 0;

    if (id != HAL_FLASH_ID_0) {
        return false;
    }

    hal_norflash_get_size(id, &size, NULL, NULL, NULL);

    offset = hal_norflash_get_addr_offset(addr);
    if (offset >= size) {
        if (addr < size) {
            offset = addr;
        } else {
            // Bad addr and consider it as secured
            return true;
        }
    }
    if (len > size || offset + len > size) {
        // Bad len and consider it as secured
        return true;
    }
    return (offset < FLASH_S_SIZE);
}

CMSE_API void hal_norflash_show_id_state_se(enum HAL_FLASH_ID_T id, int assert_on_error)
{
    hal_norflash_show_id_state(id, assert_on_error);
}

CMSE_API uint32_t hal_norflash_get_flash_total_size_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_get_flash_total_size(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_size_se0(uint32_t *total_size,
                                              uint32_t *block_size, uint32_t *sector_size,
                                              uint32_t *page_size)
{
    if (total_size) {
        total_size = cmse_check_address_range(total_size, sizeof(*total_size), CMSE_NONSECURE);
        if (!total_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (block_size) {
        block_size = cmse_check_address_range(block_size, sizeof(*block_size), CMSE_NONSECURE);
        if (!block_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (sector_size) {
        sector_size = cmse_check_address_range(sector_size, sizeof(*sector_size), CMSE_NONSECURE);
        if (!sector_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (page_size) {
        page_size = cmse_check_address_range(page_size, sizeof(*page_size), CMSE_NONSECURE);
        if (!page_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_size(HAL_FLASH_ID_0, total_size, block_size, sector_size, page_size);
}

#ifdef FLASH1_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_size_se1(uint32_t *total_size,
                                              uint32_t *block_size, uint32_t *sector_size,
                                              uint32_t *page_size)
{
    if (total_size) {
        total_size = cmse_check_address_range(total_size, sizeof(*total_size), CMSE_NONSECURE);
        if (!total_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (block_size) {
        block_size = cmse_check_address_range(block_size, sizeof(*block_size), CMSE_NONSECURE);
        if (!block_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (sector_size) {
        sector_size = cmse_check_address_range(sector_size, sizeof(*sector_size), CMSE_NONSECURE);
        if (!sector_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (page_size) {
        page_size = cmse_check_address_range(page_size, sizeof(*page_size), CMSE_NONSECURE);
        if (!page_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_size(HAL_FLASH_ID_1, total_size, block_size, sector_size, page_size);
}
#endif

#ifdef FLASH2_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_size_se2(uint32_t *total_size,
                                              uint32_t *block_size, uint32_t *sector_size,
                                              uint32_t *page_size)
{
    if (total_size) {
        total_size = cmse_check_address_range(total_size, sizeof(*total_size), CMSE_NONSECURE);
        if (!total_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (block_size) {
        block_size = cmse_check_address_range(block_size, sizeof(*block_size), CMSE_NONSECURE);
        if (!block_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (sector_size) {
        sector_size = cmse_check_address_range(sector_size, sizeof(*sector_size), CMSE_NONSECURE);
        if (!sector_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (page_size) {
        page_size = cmse_check_address_range(page_size, sizeof(*page_size), CMSE_NONSECURE);
        if (!page_size) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_size(HAL_FLASH_ID_2, total_size, block_size, sector_size, page_size);
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_boundary_se(enum HAL_FLASH_ID_T id, uint32_t address,
                                                  uint32_t *block_boundary, uint32_t *sector_boundary)
{
    if (block_boundary) {
        block_boundary = cmse_check_address_range(block_boundary, sizeof(*block_boundary), CMSE_NONSECURE);
        if (!block_boundary) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (sector_boundary) {
        sector_boundary = cmse_check_address_range(sector_boundary, sizeof(*sector_boundary), CMSE_NONSECURE);
        if (!sector_boundary) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_boundary(id, address, block_boundary, sector_boundary);
}

CMSE_API void hal_norflash_show_calib_result_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_show_calib_result(id);
}

CMSE_API void hal_norflash_show_sfdp_check_param_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_show_sfdp_check_param(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_erase_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len)
{
    if (hal_norflash_addr_is_secured(id, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    return hal_norflash_erase(id, start_address, len);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_write_se(enum HAL_FLASH_ID_T id, uint32_t start_address, const uint8_t *buffer, uint32_t len)
{
    if (hal_norflash_addr_is_secured(id, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (buffer) {
        buffer = cmse_check_address_range((void *)buffer, len, CMSE_NONSECURE);
        if (!buffer) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_write(id, start_address, buffer, len);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_read_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len)
{
    if (hal_norflash_addr_is_secured(id, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (buffer) {
        buffer = cmse_check_address_range((void *)buffer, len, CMSE_NONSECURE);
        if (!buffer) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    } else {
        return HAL_NORFLASH_BUFFER_NULL;
    }
#ifdef CHIP_BEST2003
    //2003 access FLASH secure region by FLASHX_BASE
    uint32_t total_size = hal_norflash_get_flash_total_size(id);
    start_address = (start_address & (total_size -1)) | (FLASHX_BASE);
    memcpy((void *)buffer, (void *)start_address, len);
#else
    return hal_norflash_read(id, start_address, buffer, len);
#endif
    return HAL_NORFLASH_OK;
}

CMSE_API void hal_norflash_sleep_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_sleep(id);
}

CMSE_API void hal_norflash_wakeup_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_wakeup(id);
}

CMSE_API int hal_norflash_opened_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_opened(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_open_state_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_get_open_state(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se0(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_0, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (buffer) {
        buffer = cmse_check_address_range((void *)buffer, len, CMSE_NONSECURE);
        if (!buffer) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_write_suspend(HAL_FLASH_ID_0, start_address, buffer, len, suspend);
}

#ifdef FLASH1_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se1(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_1, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (buffer) {
        buffer = cmse_check_address_range((void *)buffer, len, CMSE_NONSECURE);
        if (!buffer) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_write_suspend(HAL_FLASH_ID_1, start_address, buffer, len, suspend);
}
#endif

#ifdef FLASH2_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_write_suspend_se2(uint32_t start_address, const uint8_t *buffer, uint32_t len, int suspend)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_2, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (buffer) {
        buffer = cmse_check_address_range((void *)buffer, len, CMSE_NONSECURE);
        if (!buffer) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_write_suspend(HAL_FLASH_ID_2, start_address, buffer, len, suspend);
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_erase_suspend_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len, int suspend)
{
    if (hal_norflash_addr_is_secured(id, start_address, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    return hal_norflash_erase_suspend(id, start_address, len, suspend);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_write_resume_se(enum HAL_FLASH_ID_T id, int suspend)
{
    return hal_norflash_write_resume(id, suspend);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_erase_resume_se(enum HAL_FLASH_ID_T id, int suspend)
{
    return hal_norflash_erase_resume(id, suspend);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_irq_se(enum HAL_FLASH_ID_T id, uint32_t irq_num, int enable)
{
    return hal_norflash_suspend_check_irq(id, irq_num, enable);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_suspend_check_flash_read_se(enum HAL_FLASH_ID_T id, int enable)
{
    return hal_norflash_suspend_check_flash_read(id, enable);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_enable_remap_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id)
{
    return hal_norflash_enable_remap(id, remap_id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_disable_remap_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id)
{
    return hal_norflash_disable_remap(id, remap_id);
}

CMSE_API int hal_norflash_get_remap_status_se(enum HAL_FLASH_ID_T id, enum HAL_NORFLASH_REMAP_ID_T remap_id)
{
    return hal_norflash_get_remap_status(id, remap_id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se0(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_0, addr, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_0, addr + offset, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    return hal_norflash_config_remap(HAL_FLASH_ID_0, remap_id, addr, len, offset);
}

#ifdef FLASH1_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se1(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_1, addr, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_1, addr + offset, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    return hal_norflash_config_remap(HAL_FLASH_ID_1, remap_id, addr, len, offset);
}
#endif

#ifdef FLASH2_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_config_remap_se2(enum HAL_NORFLASH_REMAP_ID_T remap_id, uint32_t addr, uint32_t len, uint32_t offset)
{
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_2, addr, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    if (hal_norflash_addr_is_secured(HAL_FLASH_ID_2, addr + offset, len)) {
        return HAL_NORFLASH_BAD_NS_ADDR;
    }
    return hal_norflash_config_remap(HAL_FLASH_ID_2, remap_id, addr, len, offset);
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_enable_protection_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_enable_protection(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_disable_protection_se(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_disable_protection(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_set_protection_se(enum HAL_FLASH_ID_T id, uint32_t bp)
{
    // TODO: Check bp for secured address?
    return hal_norflash_set_protection(id, bp);
}

#ifdef FLASH_UNIQUE_ID
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_unique_id_se(enum HAL_FLASH_ID_T id, uint8_t *value, uint32_t len)
{
    if (value) {
        value = cmse_check_address_range(value, len, CMSE_NONSECURE);
        if (!value) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_unique_id(id, value, len);
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_read_sfdp_reg_se(enum HAL_FLASH_ID_T id, uint8_t addr, uint8_t *value, uint32_t len)
{
    if (value) {
        value = cmse_check_address_range(value, len, CMSE_NONSECURE);
        if (!value) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_read_sfdp_reg(id, addr, value, len);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se0(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len)
{
    if (value0) {
        value0 = cmse_check_address_range(value0, len, CMSE_NONSECURE);
        if (!value0) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (value1) {
        value1 = cmse_check_address_range(value1, len, CMSE_NONSECURE);
        if (!value1) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_dual_chip_read_sfdp_reg(HAL_FLASH_ID_0, addr, value0, value1, len);
}

#ifdef FLASH1_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se1(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len)
{
    if (value0) {
        value0 = cmse_check_address_range(value0, len, CMSE_NONSECURE);
        if (!value0) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (value1) {
        value1 = cmse_check_address_range(value1, len, CMSE_NONSECURE);
        if (!value1) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_dual_chip_read_sfdp_reg(HAL_FLASH_ID_1, addr, value0, value1, len);
}
#endif

#ifdef FLASH2_CTRL_BASE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_dual_chip_read_sfdp_reg_se2(uint8_t addr, uint8_t *value0, uint8_t *value1, uint32_t len)
{
    if (value0) {
        value0 = cmse_check_address_range(value0, len, CMSE_NONSECURE);
        if (!value0) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (value1) {
        value1 = cmse_check_address_range(value1, len, CMSE_NONSECURE);
        if (!value1) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_dual_chip_read_sfdp_reg(HAL_FLASH_ID_2, addr, value0, value1, len);
}
#endif

#ifdef FLASH_DEC_ENABLE
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_dec_enable_sec(enum HAL_FLASH_ID_T id)
{
    return hal_norflash_dec_enable(id);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_dec_disable_sec(enum HAL_FLASH_ID_T id);
{
    return hal_norflash_dec_disable(id);
}
#endif

#ifdef FLASH_SECURITY_REGISTER
CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_security_register_lock_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint32_t len)
{
    return HAL_NORFLASH_BAD_PRIVILEGE;
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_security_register_read_se(enum HAL_FLASH_ID_T id, uint32_t start_address, uint8_t *buffer, uint32_t len)
{
    return HAL_NORFLASH_BAD_PRIVILEGE;
}
#endif

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_set_dual_chip_mode_se(enum HAL_FLASH_ID_T id, int dual_flash, int dual_sec_reg)
{
    return hal_norflash_set_dual_chip_mode(id, dual_flash, dual_sec_reg);
}

CMSE_API enum HAL_NORFLASH_RET_T hal_norflash_get_dual_chip_mode_se(enum HAL_FLASH_ID_T id, int *dual_flash, int *dual_sec_reg)
{
    if (dual_flash) {
        dual_flash = cmse_check_address_range(dual_flash, sizeof(*dual_flash), CMSE_NONSECURE);
        if (!dual_flash) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    if (dual_sec_reg) {
        dual_sec_reg = cmse_check_address_range(dual_sec_reg, sizeof(*dual_sec_reg), CMSE_NONSECURE);
        if (!dual_sec_reg) {
            return HAL_NORFLASH_BAD_NS_ADDR;
        }
    }
    return hal_norflash_get_dual_chip_mode(id, dual_flash, dual_sec_reg);
}

#endif
