/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __BT_DRV_INTERNAL_H__
#define __BT_DRV_INTERNAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#undef __INLINE
#define __INLINE __STATIC_FORCEINLINE
/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "stdint.h"
#include "stdbool.h"

/*
 * DEFINES
 ****************************************************************************************
 */

/// Bluetooth technologies version
#define BT40_VERSION                      (6)
#define BT41_VERSION                      (7)
#define BT42_VERSION                      (8)
#define BT50_VERSION                      (9)
#define BT51_VERSION                      (10)
#define BT52_VERSION                      (11)
#define BT53_VERSION                      (12)
#define BT54_VERSION                      (13)

#ifndef ASSERT_ERR
#define ASSERT_ERR(cond)                             { if (!(cond)) { BT_DRV_TRACE(2,"line is %d file is %s", __LINE__, __FILE__); } }
#endif

#define BT_DRV_REG_OP_ENTER()    do{ uint32_t stime, spent_time; stime= hal_sys_timer_get();
#define BT_DRV_REG_OP_EXIT()     spent_time = TICKS_TO_US(hal_sys_timer_get()-stime);if (spent_time>1000)TRACE(2,"%s exit, %dus",__func__, spent_time);}while(0);

#define BTPCM_MASTER_MODE  (0)
#define BTPCM_SLAVE_MODE   (1)

/// Linear format
#define LINEAR_FORMAT_8_BITS    0x00
#define LINEAR_FORMAT_13_BITS   0x01
#define LINEAR_FORMAT_14_BITS   0x02
#define LINEAR_FORMAT_16_BITS   0x03

//MONO_LR_SEL
#define LEFT_CHANNEL_MONO    0x0
#define RIGHT_CHANNEL_MONO   0X1

//MONO_STEREO
#define MONO_SAMPLE_MODE 0
#define MONO_STEREO_MODE 1

#define B(byte, feat)     ((FEAT_##feat##_SUPP << B##byte##_##feat##_POS) & B##byte##_##feat##_MSK)
/// Extended feature mask definition page 2 LMP:3.3
#define B1_SEC_CON_CTRL_POS         0
#define B1_SEC_CON_CTRL_MSK         0x01
#define B1_PING_POS                 1
#define B1_PING_MSK                 0x02
#define B1_SAM_POS                  2
#define B1_SAM_MSK                  0x04
#define B1_TRAIN_NUDGING_POS        3
#define B1_TRAIN_NUDGING_MSK        0x08

#define BT_DRIVER_GET_U8_REG_VAL(regAddr)       (*(uint8_t *)(uintptr_t)(regAddr))
#define BT_DRIVER_GET_U16_REG_VAL(regAddr)      (*(uint16_t *)(uintptr_t)(regAddr))
#define BT_DRIVER_GET_U32_REG_VAL(regAddr)      (*(uint32_t *)(uintptr_t)(regAddr))

#define BT_DRIVER_PUT_U8_REG_VAL(regAddr, val)      *(uint8_t *)(uintptr_t)(regAddr) = (val)
#define BT_DRIVER_PUT_U16_REG_VAL(regAddr, val)     *(uint16_t *)(uintptr_t)(regAddr) = (val)
#define BT_DRIVER_PUT_U32_REG_VAL(regAddr, val)     *(uint32_t *)(uintptr_t)(regAddr) = (val)

#define BT_SUB_SYS_TYPE     0
#define MCU_SYS_TYPE        1
#define BT_EM_AREA_1_TYPE   2
#define BT_EM_AREA_2_TYPE   3

#define BT_FA_INVERT_EN   1
#define BT_FA_INVERT_DISABLE   0

/***************************************************************************
 *multi IBRT config
 ****************************************************************************/
#define MULTI_IBRT_BG_SLICE 28

#define MULTI_IBRT_FG_SLICE 112

#define TX_PWR_16DBM 16
#define TX_PWR_15DBM 15
#define TX_PWR_14DBM 14
#define TX_PWR_13DBM 13
#define TX_PWR_12DBM 12
#define TX_PWR_11DBM 11
#define TX_PWR_10DBM 10
#define TX_PWR_9DBM  9
#define TX_PWR_8DBM  8
#define TX_PWR_7DBM  7
#define TX_PWR_6DBM  6
#define TX_PWR_5DBM  5
#define TX_PWR_4DBM  4
#define TX_PWR_3DBM  3
#define TX_PWR_2DBM  2
#define TX_PWR_1DBM  1
#define TX_PWR_0DBM  0
#define TX_PWR_N1DBM -1
#define TX_PWR_N2DBM -2
#define TX_PWR_N3DBM -3
#define TX_PWR_N4DBM -4
#define TX_PWR_N5DBM -5
#define TX_PWR_N6DBM -6
#define TX_PWR_N7DBM -7
#define TX_PWR_N8DBM -8
#define TX_PWR_N9DBM -9
#define TX_PWR_N10DBM -10
#define TX_PWR_N11DBM -11
#define TX_PWR_N12DBM -12
#define TX_PWR_N13DBM -13
#define TX_PWR_N14DBM -14

#define GET_32BIT_0BYTE(value) ((uint8_t)(value & 0xFF))

#define GET_32BIT_1BYTE(value) ((uint8_t)((value >> 8) & 0xFF))

#define GET_32BIT_2BYTE(value) ((uint8_t)((value >> 16) & 0xFF))

#define GET_32BIT_3BYTE(value) ((uint8_t)((value >> 24) & 0xFF))
/*
 * ENUMERATION
 ****************************************************************************************
 */

#ifdef MCU_WAKEUP_BT_V2
#define MCU_WAKEUP_BT_MSG_LEN (8)

enum WAKEUP_STATUS_T
{
    INTERSYS_WAIT_BTC_RESPONSE = 0,
    INTERSYS_TX_DONE = 1,
};

enum WAKEUP_BT_USER_T
{
    WAKEUP_BT_USER_DRV = 1,
    WAKEUP_BT_USER_TRIG,

    WAKEUP_BT_USER_QTY
};

enum WAKEUP_BT_MSG_T
{
    // disable sleep (1)  resume sleep (2) wakeup BT(3)
    MSG_WAKEUP_BT_AND_DIS_SLP = 1,
    MSG_WAKEUP_BT_AND_EN_SLP,
    MSG_ONLY_WAKEUP_BT,

    WAKEUP_BT_MSG_QTY
};
#endif //MCU_WAKEUP_BT_V2

enum bes_btc_tport_level
{
    SNOOP_LEVEL                  = 0x1,
    ACL_LEVEL                    = 0x2,
    SCO_LEVEL                    = 0x4,
    BLE_LEVEL                    = 0x8,
    IBRT_LMP_LEVEL               = 0x10,
    SCH_LEVEL                    = 0x20,
    ISR_LEVEL                    = 0x40,
    PATCH_LEVEL                  = 0x80,
    TRANSQ_LEVEL                 = 0x100,
    PD_LEVEL                     = 0x200,
};

enum btc_slp_cfg
{
    /// Boolean indicating whether cmu intmask
    BES_CMU_INTMASK_POS       = 0,
    BES_CMU_INTMASK_BIT       = 0x01,
    /// Boolean indicating whether to use external 32K LPO
    BES_EXT_LPO_POS           = 1,
    BES_EXT_LPO_BIT           = 0x02,
    /// Boolean indicating RF SPI BIT NUMBER
    BES_RF_SPI_BIT_NB_LSB     = 2,
    BES_RF_SPI_BIT_NB_MASK    = 0xC,
};
/*
 * COMMON FUNCTION DECLARATIONS
 ****************************************************************************************
 */
inline uint32_t co_mod(uint32_t val, uint32_t div)
{
   ASSERT_ERR(div);
   return ((val) % (div));
}
#define CO_MOD(val, div) co_mod(val, div)

inline uint32_t co_max(uint32_t a, uint32_t b)
{
    return a > b ? a : b;
}

/**
 ****************************************************************************************
 * @brief Read a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @return The 16 bits value.
 ****************************************************************************************
 */
inline uint16_t co_read16p(void const *ptr16)
{
    uint16_t value = ((uint8_t *)ptr16)[0] | ((uint8_t *)ptr16)[1] << 8;
    return value;
}

/**
 ****************************************************************************************
 * @brief Read a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @return The 32 bits value.
 ****************************************************************************************
 */
inline uint32_t co_read32p(void const *ptr32)
{
    uint16_t addr_l, addr_h;
    addr_l = co_read16p((uint16_t *)ptr32);
    addr_h = co_read16p((uint16_t *)ptr32 + 1);
    return ((uint32_t)addr_l | (uint32_t)addr_h << 16);
}

/**
 ****************************************************************************************
 * @brief Write a packed 32 bits word.
 * @param[in] ptr32 The address of the first byte of the 32 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
inline void co_write32(void const *ptr32, uint32_t value)
{
    uint8_t volatile *ptr=(uint8_t*)ptr32;

    *ptr++ = (uint8_t)(value&0xff);
    *ptr++ = (uint8_t)((value&0xff00)>>8);
    *ptr++ = (uint8_t)((value&0xff0000)>>16);
    *ptr = (uint8_t)((value&0xff000000)>>24);
}

/**
 ****************************************************************************************
 * @brief Write a packed 16 bits word.
 * @param[in] ptr16 The address of the first byte of the 16 bits word.
 * @param[in] value The value to write.
 ****************************************************************************************
 */
inline void co_write16(void const *ptr16, uint16_t value)
{
    uint8_t volatile *ptr=(uint8_t*)ptr16;

    *ptr++ = value&0xff;
    *ptr = (value&0xff00)>>8;
}

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
void btdrv_poweron(uint8_t en);
uint8_t btdrv_rf_init(void);
void btdrv_test_mode_rf_txpwr_init(void);
void btdrv_ins_patch_init(void);
void btdrv_data_patch_init(void);
void btdrv_patch_en(uint8_t en);
void btdrv_config_init(void);
void btdrv_config_end(void);
void btdrv_testmode_config_init(void);
void btdrv_bt_spi_rawbuf_init(void);
void btdrv_bt_spi_xtal_init(void);
void btdrv_rf_rx_gain_adjust_req(uint32_t user, bool lowgain);
void btdrv_btc_fault_dump(void);
void btdrv_dump_mem(uint8_t *dump_mem_start, uint32_t dump_length, uint8_t dump_type);
void btdrv_fast_lock_config(bool fastlock_on);
void btdrv_ecc_config(void);
void btdrv_btc_fault_dump(void);
void btdrv_fa_config(void);
void btdrv_hciprocess(void);
void bt_drv_extra_config_after_init(void);
void bt_drv_reg_op_multi_ibrt_sche_dump(void);
void bt_drv_reg_op_dump_rx_record(void);
int8_t btdrv_get_rssi_avg_thr(bool tws_link);
void bt_drv_reg_op_enable_3wire_tports(void);
void bt_drv_reg_op_enable_2wire_tports(void);
void bt_drv_reg_op_multi_ibrt_sche_dump(void);

void bt_drv_heap_init(void);
void btdrv_common_init(void);
void *bt_drv_malloc(unsigned int size);
void bt_drv_free(void *ptr);
void bt_tester_cmd_receive_evt_analyze(const unsigned char *data, unsigned int len);
uint32_t bt_drv_get_btc_sw_version(void);
void bt_drv_bt_tport_type_config(void);

#ifdef MCU_WAKEUP_BT_V2
void bt_intersys_oper_btcore(enum WAKEUP_BT_USER_T user, enum WAKEUP_BT_MSG_T msg);

void bt_intersys_oper_wait_bt_response(void);
#endif // MCU_WAKEUP_BT_V2

#ifdef __cplusplus
}
#endif

#endif //__BT_DRV_INTERNAL_H__
