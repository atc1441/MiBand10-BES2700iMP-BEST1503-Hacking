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
#ifndef __BT_DRV_H__
#define  __BT_DRV_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_timer.h"

#include "hal_analogif.h"
#include "hal_trace.h"

#define SBC_PKT_TYPE_DM1   0x3
#define SBC_PKT_TYPE_2EV3  0x6
#define SBC_PKT_TYPE_2DH5  0xe

#define BT_ACL_CONHDL_BIT       (0x80)


#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || \
    defined(CHIP_BEST2300A) || defined(CHIP_BEST2300A)
#define BTDRV_ISPI_RF_REG(reg)                  (reg)
#else
#define BTDRV_ISPI_RF_REG(reg)                  (((reg) & 0xFFF) | 0x2000)
#endif

// NOTE: Measure this value for every CHIP Version
#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || \
    defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define CVSD_OFFSET_BYTES (120 - 2)
#else
#define CVSD_OFFSET_BYTES (120)
#endif

// NOTE: Measure this value for every CHIP Version
#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300) || defined(CHIP_BEST2300P) || \
    defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define MSBC_OFFSET_BYTES (1)
#else
#define MSBC_OFFSET_BYTES (0)
#endif

#if defined(CHIP_BEST1000) || defined(CHIP_BEST2000) || defined(CHIP_BEST2300)
#define MSBC_MUTE_PATTERN (0x00)
#else
#define MSBC_MUTE_PATTERN (0x55)
#endif

#define btdrv_read_rf_reg(reg,val)              hal_analogif_reg_read(BTDRV_ISPI_RF_REG(reg),val)
#define btdrv_write_rf_reg(reg,val)             hal_analogif_reg_write(BTDRV_ISPI_RF_REG(reg),val)

#define btdrv_delay(ms)                         hal_sys_timer_delay(MS_TO_TICKS(ms))
#define btdrv_delay_us(us)                         hal_sys_timer_delay(US_TO_TICKS(us))


#define BTDIGITAL_REG(a)                        (*(volatile uint32_t *)(a))
#define BTDIGITAL_REG_WR(addr, value)         (*(volatile uint32_t *)(addr)) = (value)

#define BTDIGITAL_BT_EM(a)                      (*(volatile uint16_t *)(a))
/// Macro to write a BT control structure field (16-bit wide)
#define BTDIGITAL_EM_BT_WR(addr, value)         (*(volatile uint16_t *)(addr)) = (value)

#define BTDIGITAL_REG_SET_FIELD(reg, mask, shift, v)\
                                                do{ \
                                                    volatile unsigned int tmp = *(volatile unsigned int *)(reg); \
                                                    tmp &= ~(mask<<shift); \
                                                    tmp |= (v<<shift); \
                                                    *(volatile unsigned int *)(reg) = tmp; \
                                                }while(0)

#define BTDIGITAL_REG_GET_FIELD(reg, mask, shift, v)\
                                                do{ \
                                                    volatile unsigned int tmp = *(volatile unsigned int *)(reg); \
                                                    v = (tmp>>shift)&mask; \
                                                }while(0)

#define BTRF_REG_SET_FIELD(reg, mask, shift, v)\
                                                do{ \
                                                    unsigned short tmp; \
                                                    btdrv_read_rf_reg(reg,&tmp); \
                                                    tmp &= ~(mask<<shift); \
                                                    tmp |= (v<<shift); \
                                                    btdrv_write_rf_reg(reg,tmp); \
                                                }while(0)

#define BTRF_REG_GET_FIELD(reg, mask, shift, v)\
                                                do{ \
                                                    unsigned short tmp; \
                                                    btdrv_read_rf_reg(reg,&tmp); \
                                                    v = (tmp>>shift)&mask; \
                                                }while(0)

#define getbit(x,y)   ((x) >> (y)&1)

#define BT_DRV_DEBUG  1
#if BT_DRV_DEBUG
#define BT_DRV_TRACE(n, fmt, ...) TRACE(n, fmt, ##__VA_ARGS__)
#define BT_DRV_DUMP(s,buff,len) DUMP8(s,buff,len)
#ifndef TOTA_CRASH_DUMP_TOOL_ENABLE
#define BT_DRV_TRACE_CRASH_DUMP(n, fmt, ...)  TRACE(n, fmt, ##__VA_ARGS__)
#else
#define BT_DRV_TRACE_CRASH_DUMP(n, fmt, ...)  REL_TRACE_IMM(n, fmt, ##__VA_ARGS__)
#endif
#else
#define BT_DRV_TRACE(n, fmt, ...) hal_trace_dummy(NULL, ##__VA_ARGS__)
#define BT_DRV_DUMP(s,buff,len)
#define BT_DRV_TRACE_CRASH_DUMP(n, fmt, ...) hal_trace_dummy(NULL, ##__VA_ARGS__)
#endif

#ifndef HCI_HOST_NB_CMP_PKTS_CMD_OPCODE
#define HCI_HOST_NB_CMP_PKTS_CMD_OPCODE         0x0C35
#endif
#ifndef HCI_NB_CMP_PKTS_EVT_CODE
#define HCI_NB_CMP_PKTS_EVT_CODE                0x13
#endif
#define MAX_NUM_SCO_LINKS (2)
#define  TEST_MODE_MAX_NB_ACTIVE_ACL 3


#if defined(BT_CLK_UNIT_IN_HSLOT)
//bt max slot clock
#define MAX_SLOT_CLOCK      ((1L<<28) - 1)
#else
//bt max slot clock
#define MAX_SLOT_CLOCK      ((1L<<27) - 1)
#endif
 // A slot is 625 us
#define SLOT_SIZE           625
#define HALF_SLOT_SIZE       625                    // A half slot is 312.5 us (in half us)

//#define __PASS_CI_TEST_SETTING__
#define  BT_LOW_POWER_MODE  1
#define  BT_HIGH_PERFORMANCE_MODE   2
//#define __ENABLE_LINK_POWER_CONTROL__

#define BT_RFMODE BT_LOW_POWER_MODE
//#define BT_RFMODE BT_HIGH_PERFORMANCE_MODE

#define BT_POWERON            1
#define BT_POWEROFF           0

/// 8 bit access types
#define _8_Bit                              8
/// 16 bit access types
#define _16_Bit                             16
/// 32 bit access types
#define _32_Bit                             32

#define BTDRV_PATCH_WRITING  0x0
#define BTDRV_PATCH_DONE         0x1

#define BTDRV_CFG_WRITING   0x0
#define BTDRV_CFG_DONE      0x1

struct bt_cmd_chip_config_t
{
    uint16_t hci_dbg_set_sync_config_cmd_opcode;
    uint16_t hci_dbg_set_sco_switch_cmd_opcode;
};

extern struct bt_cmd_chip_config_t g_bt_drv_btstack_chip_config;

extern void btdrv_hciopen(void);

void btdrv_nosignal_tx_test(void);

#ifdef __cplusplus
}
#endif

#endif

