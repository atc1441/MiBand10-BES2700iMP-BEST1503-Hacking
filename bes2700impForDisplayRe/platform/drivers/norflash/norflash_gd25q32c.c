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
#include "plat_types.h"
#include "norflash_drv.h"
#include "hal_norflaship.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "norflash_cfg.h"
#include "norflash_gd25q32c.h"

static void gd25q32c_write_status_s8_s15(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint8_t status)
{
    bool volatile_reg = true;

    if (type == DRV_NORFLASH_W_STATUS_LB || type == DRV_NORFLASH_W_STATUS_BP_INIT) {
        volatile_reg = false;
    }
    norflash_write_reg(id, volatile_reg, GD25Q32C_CMD_WRITE_STATUS_S8_S15, &status, 1);
}

static void gd25q32c_write_status_s0_s7(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint8_t status)
{
    bool volatile_reg = true;

    if (type == DRV_NORFLASH_W_STATUS_LB || type == DRV_NORFLASH_W_STATUS_BP_INIT) {
        volatile_reg = false;
    }
    norflash_write_reg(id, volatile_reg, GD25Q32C_CMD_WRITE_STATUS_S0_S7, &status, 1);
}

static int gd25q32c_write_status(enum HAL_FLASH_ID_T id, enum DRV_NORFLASH_W_STATUS_T type, uint32_t param)
{
    uint8_t status_s0_s7;
    uint8_t status_s8_s15;
    uint32_t bp_mask = 0;
    union DRV_NORFLASH_SEC_REG_CFG_T cfg;

    if (type >= DRV_NORFLASH_W_STATUS_QTY) {
        return 1;
    }

    if (type == DRV_NORFLASH_W_STATUS_INIT) {
        gd25q32c_write_status_s0_s7(id, type, param & 0xFF);
        gd25q32c_write_status_s8_s15(id, type, (param >> 8) & 0xFF);
        return 0;
    }

    if (type == DRV_NORFLASH_W_STATUS_BP_INIT || type == DRV_NORFLASH_W_STATUS_BP) {
        bp_mask = norflash_get_block_protect_mask(id);
        status_s0_s7 = norflash_read_status_s0_s7(id);
        status_s0_s7 = (status_s0_s7 & ~bp_mask) | (param & bp_mask);
        gd25q32c_write_status_s0_s7(id, type, status_s0_s7);
        if ((bp_mask & ~0xFF) == 0) {
            return 0;
        }
    }

    status_s8_s15 = norflash_read_status_s8_s15(id);

    if (type == DRV_NORFLASH_W_STATUS_QE) {
        if (param) {
            status_s8_s15 |= GD25Q32C_QE_BIT_MASK;
        } else {
            status_s8_s15 &= ~(GD25Q32C_QE_BIT_MASK);
        }
    } else if (type == DRV_NORFLASH_W_STATUS_BP_INIT || type == DRV_NORFLASH_W_STATUS_BP) {
        param >>= 8;
        bp_mask >>= 8;
        status_s8_s15 = (status_s8_s15 & ~bp_mask) | (param & bp_mask);
    } else if (type == DRV_NORFLASH_W_STATUS_LB) {
        cfg = norflash_get_security_register_config(id);
        if (!cfg.s.enabled) {
            return 2;
        }
        if (cfg.s.lb == SEC_REG_LB_S11_S13) {
            if (param & ~7) {
                return 3;
            }
            status_s8_s15 = SET_BITFIELD(status_s8_s15, STATUS_LB_S11_S13_BIT, param);
        } else if (cfg.s.lb == SEC_REG_LB_S10) {
            if (param != 1) {
                return 3;
            }
            status_s8_s15 |= STATUS_LB_S10_BIT_MASK;
        } else if (cfg.s.lb == SEC_REG_LB_S10_S13) {
            if (param & ~0xF) {
                return 3;
            }
            status_s8_s15 = SET_BITFIELD(status_s8_s15, STATUS_LB_S10_S13_BIT, param);
        } else if (cfg.s.lb == SEC_REG_LB_S12_S13) {
            if (param & ~3) {
                return 3;
            }
            status_s8_s15 = SET_BITFIELD(status_s8_s15, STATUS_LB_S12_S13_BIT, param);
        } else {
            return 4;
        }
    }

    gd25q32c_write_status_s8_s15(id, type, status_s8_s15);

    return 0;
}

// ----------------------
// GigaDevice
// ----------------------

const struct NORFLASH_CFG_T gd25q32c_cfg = {
    .id = { 0xC8, 0x40, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_5_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = GD25Q32C_TOTAL_SIZE,
#ifdef FLASH_HPM
    .max_speed = 120 * 1000 * 1000,
#else
    // No high performance mode for gd25q32e
    .max_speed = 104 * 1000 * 1000,
#endif
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
#ifdef FLASH_HPM
                HAL_NORFLASH_OP_MODE_HIGH_PERFORMANCE |
#endif
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

// ----------------------
// Puya
// ----------------------

const struct NORFLASH_CFG_T p25q256l_cfg =
{
    .id = { 0x85,0x40,0x19, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q256L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 85 * 1000 * 1000,
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_SFDP_CFG_T p25q128l_sfdp_cfg[] = {
    // PY25Q128LAC-T
    {
        .param = { 0xC8, },
        .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
            },
        },
        .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH,
            .dummy_cycles = 9,
            },
        },
        .max_speed = 133 * 1000 * 1000,  // 133MHz STD_READ=133MHz DTR_READ=85MHz
    },

    // P25Q128L
    {
         .param = { 0xE8, },
         .speed_ratio = {
            .s = {
                .std_read = SPEED_RATIO_3_EIGHTH,
                .others = SPEED_RATIO_8_EIGHTH,
            },
         },
         .dtr_quad_cfg = {
            .s = {
                .speed_ratio = SPEED_RATIO_4_EIGHTH,
                .dummy_cycles = 7,
            },
         },
         .max_speed = 85 * 1000 * 1000,  // 85MHz STD_READ=33MHz DTR_READ=43MHz
    },
};

const struct NORFLASH_SFDP_LIST_T p25q128l_sfdp_list = {
    .id = { 0x85, 0x60, 0x18, },
    .cfg_cnt = ARRAY_SIZE(p25q128l_sfdp_cfg),
    .sfdp_check_offset = 0x69,
    .sfdp_check_len = 1,
    .cfg = (struct NORFLASH_SFDP_CFG_T*)p25q128l_sfdp_cfg,
};

// P25Q128L,PY25Q128LAC-T(temporay version) use same id.
// Normal: P25Q128L
struct NORFLASH_CFG_T p25q128l_cfg = {
    .id = { 0x85, 0x60, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q128L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND |
                HAL_NORFLASH_OP_MODE_CHECK_SFDP),
    .max_speed = 128 * 1000 * 1000,
    .write_status = gd25q32c_write_status,
};

// PY25Q128LAC
struct NORFLASH_CFG_T py25q128lac_cfg = {
    .id = { 0x85, 0x65, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q128L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 128 * 1000 * 1000, // 133MHz STD_READ=80MHz DTR_READ=85MHz
    .write_status = gd25q32c_write_status,
};

// PY25Q128LAA
const struct NORFLASH_CFG_T py25q128laa_cfg = {
    .id = { 0x85, 0x20, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q128L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 128 * 1000 * 1000, // 133Mhz STD_READ=80MHz DTR_READ=85MHz
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_SFDP_CFG_T p25q64l_sfdp_cfg[] = {
    // P25Q64SLEWF
    {
        .param = { 0x00, 0x20, 0x50, 0x16, },
        .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_2_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
            },
        },
        .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_6_EIGHTH,
            .dummy_cycles = 7,
            },
        },
        .max_speed = 104 * 1000 * 1000,  // STD_READ=33M DTR_READ=85M
    },

    // P25Q64SLDWF-HX2
    {
         .param = { 0x00, 0x20, 0x00, 0x17, },
         .speed_ratio = {
            .s = {
                .std_read = SPEED_RATIO_3_EIGHTH,
                .others = SPEED_RATIO_8_EIGHTH,
            },
         },
         .dtr_quad_cfg = {
            .s = {
                .speed_ratio = SPEED_RATIO_5_EIGHTH,
                .dummy_cycles = 7,
            },
         },
         .max_speed = 85 * 1000 * 1000,  // STD_READ=33M DTR_READ=55M
    },

    // P25Q65SLA-HX
    {
        .param = { 0x00, 0x36, 0x50, 0x16, },
        .speed_ratio = {
           .s = {
               .std_read = SPEED_RATIO_3_EIGHTH,
               .others = SPEED_RATIO_8_EIGHTH,
           },
        },
        .dtr_quad_cfg = {
           .s = {
               .speed_ratio = SPEED_RATIO_4_EIGHTH,
               .dummy_cycles = 7,
           },
        },
        .max_speed = 70 * 1000 * 1000,  // STD_READ=33M DTR_READ=35M
    },
};

const struct NORFLASH_SFDP_LIST_T p25q64l_sfdp_list = {
    .id = { 0x85, 0x60, 0x17, },
    .cfg_cnt = ARRAY_SIZE(p25q64l_sfdp_cfg),
    .sfdp_check_offset = 0x60,
    .sfdp_check_len = 4,
    .cfg = (struct NORFLASH_SFDP_CFG_T*)p25q64l_sfdp_cfg,
};

struct NORFLASH_CFG_T p25q64l_cfg = {
    .id = { 0x85, 0x60, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q64L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND |
                HAL_NORFLASH_OP_MODE_CHECK_SFDP),

    .max_speed = 70 * 1000 * 1000,  // STD_READ=33M DTR_READ=35M
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q64ha_cfg = {
    .id = { 0x85, 0x20, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_6_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q64L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 128 * 1000 * 1000,  // STD_READ=33M DTR_READ=80M
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q64sn_cfg = {
    .id = { 0x85, 0x66, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q64L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND |
                HAL_NORFLASH_OP_MODE_CHECK_SFDP),

    .max_speed = 104 * 1000 * 1000,  // STD_READ=33M DTR_READ=35M
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q32l_cfg = {
    .id = { 0x85, 0x60, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_4_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH, // 43M
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q32L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 62 * 1000 * 1000, // P25Q32L=62.5M, P25Q32SL=70M
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q32sn_cfg = {
    .id = { 0x85, 0x66, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_4_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH, // 43M
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q32L_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_DUAL_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 104 * 1000 * 1000,
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q32sl_cfg = {
    .id = { 0x85, 0x42, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q32SL_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_PAGE_SIZE_1K |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 85 * 1000 * 1000,
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T p25q16sl_cfg = {
    .id = { 0x85, 0x42, 0x15, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_3_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_1024,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = P25Q16SL_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 85 * 1000 * 1000,
    .write_status = gd25q32c_write_status,
};

// ----------------------
// Xinxin
// ----------------------

// Additionally, the device supports JEDEC standard manufacturer and device ID and SFDP Register,
// a 64-bit Unique Serial Number and three 256-bytes Security Registers.

const struct NORFLASH_CFG_T xm25qu256c_cfg = {
    .id = { 0x20, 0x41, 0x19, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_256,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = XM25QU256C_TOTAL_SIZE,
    .max_speed = 128 * 1000 * 1000, // 128M (std_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_SUSPEND |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                0),
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T xm25qu128c_cfg = {
    .id = { 0x20, 0x41, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_256,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = XM25QU128C_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000, // 108M (std_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T xm25lu64c_cfg = {
    .id = { 0x20, 0x41, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_5_EIGHTH, // 66M
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_256,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = XM25LU64C_TOTAL_SIZE,
    .max_speed = 128 * 1000 * 1000, // 128M (std_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T xm25qh16c_cfg = {
    .id = { 0x20, 0x40, 0x15, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_256,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = XM25QH16C_TOTAL_SIZE,
    .max_speed = 104 * 1000 * 1000, // 108M (std_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T xm25qh80b_cfg = {
    .id = { 0x20, 0x40, 0x14, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_7_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_256,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = XM25QH80B_TOTAL_SIZE,
    .max_speed = 60 * 1000 * 1000, // 104M (std_read=50M or 3/8) when HFM=1 (S20 or SR3-bit4)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

// ----------------------
// Zbit
// ----------------------

const struct NORFLASH_CFG_T zb25vq128b_cfg = {
    .id = { 0x5E, 0x40, 0x18, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_6_EIGHTH,
            .others = SPEED_RATIO_6_EIGHTH,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = ZB25VQ128B_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 104 * 1000 * 1000, // HFM=1 by default
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T zb25lq64a_cfg = {
    .id = { 0x5E, 0x70, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_6_EIGHTH,
            .others = SPEED_RATIO_6_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_6_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = ZB25LQ64A_TOTAL_SIZE,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .max_speed = 104 * 1000 * 1000, // HFM=1 by default
    .write_status = gd25q32c_write_status,
};

// ----------------------
// Windbond
// ----------------------

const struct NORFLASH_CFG_T w25q32fw_cfg = {
    .id = { 0xEF, 0x60, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_6_EIGHTH,
            .dummy_cycles = 9,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = W25Q32FW_TOTAL_SIZE,
    .max_speed = 120 * 1000 * 1000,
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                //HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                //HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_DTR |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
                HAL_NORFLASH_OP_MODE_READ_WRAP |
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                //HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

// ----------------------
// SiliconKaiser
// ----------------------

const struct NORFLASH_CFG_T sk25le064_cfg = {
    .id = { 0x25, 0x70, 0x17, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_6_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = SK25LE064_TOTAL_SIZE,
    .max_speed = 128 * 1000 * 1000, // 133M (std_read=80M dtr_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
#ifdef FLASH_DTR
                HAL_NORFLASH_OP_MODE_DTR |
#else
                HAL_NORFLASH_OP_MODE_READ_WRAP |
#endif
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

const struct NORFLASH_CFG_T sk25le032_cfg = {
    .id = { 0x25, 0x70, 0x16, },
    .speed_ratio = {
        .s = {
            .std_read = SPEED_RATIO_5_EIGHTH,
            .others = SPEED_RATIO_8_EIGHTH,
        },
    },
    .dtr_quad_cfg = {
        .s = {
            .speed_ratio = SPEED_RATIO_4_EIGHTH,
            .dummy_cycles = 7,
        },
    },
    .crm_en_bits = (1 << 5) | (0 << 4),
    .crm_dis_bits = 0,
    .block_protect_mask = 0x407C,
    .sec_reg_cfg = {
        .s = {
            .enabled = true,
            .base = SEC_REG_BASE_0X1000,
            .size = SEC_REG_SIZE_1024,
            .offset = SEC_REG_OFFSET_0X1000,
            .cnt = SEC_REG_CNT_3,
            .pp = SEC_REG_PP_256,
            .lb = SEC_REG_LB_S11_S13,
        },
    },
    .total_size = SK25LE032_TOTAL_SIZE,
    .max_speed = 128 * 1000 * 1000, // 133M (std_read=80M dtr_read=66M)
    .mode = (HAL_NORFLASH_OP_MODE_STAND_SPI |
                HAL_NORFLASH_OP_MODE_FAST_SPI |
                HAL_NORFLASH_OP_MODE_DUAL_OUTPUT |
                HAL_NORFLASH_OP_MODE_DUAL_IO |
                HAL_NORFLASH_OP_MODE_QUAD_OUTPUT |
                HAL_NORFLASH_OP_MODE_QUAD_IO |
#ifdef FLASH_CONT_MODE_SK25LE032
                HAL_NORFLASH_OP_MODE_CONTINUOUS_READ |
#endif
#ifdef FLASH_DTR
                HAL_NORFLASH_OP_MODE_DTR |
#else
                HAL_NORFLASH_OP_MODE_READ_WRAP |
#endif
                HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_QUAD_PAGE_PROGRAM |
                HAL_NORFLASH_OP_MODE_SUSPEND),
    .write_status = gd25q32c_write_status,
};

