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
#ifndef __BT_1306_REG_MAP_H__
#define __BT_1306_REG_MAP_H__

/***************************************************************************
 *BES ARM cortex M33 BT ROM Patch registers
 ****************************************************************************/

#define BTDRV_PATCH_EN_REG                  0xa01ffe00

//instruction patch compare src address
#define BTDRV_PATCH_INS_COMP_ADDR_START     0xa01ffe00

#define BTDRV_PATCH_INS_REMAP_ADDR_START    0xa01fff00

/***************************************************************************
 *BT SRAM define
 ****************************************************************************/

#define BT_SRAM_SIZE 0x10000

/***************************************************************************
 * BT EM define
 ****************************************************************************/

#define EM_BASE_ADDR 0xD0210000

#define EM_SIZE 0x10000

/***************************************************************************
 * BT CMU registers
 ****************************************************************************/
#define CMU_CLKREG_ADDR   0xD0330038

/***************************************************************************
 * HW SPI
 ****************************************************************************/
#define REG_SPI_EN_ADDR        0xd0340004
#define REG_SPI_TRIG_NUM_ADDR  0xd0340074
#define REG_SPI0_TRIG_POS_ADDR 0xd0340078
#define REG_SPI1_TRIG_POS_ADDR 0xd034007c
#define REG_HWSPI_LOOP_EN_ADDR 0xd0220d04

/***************************************************************************
 * IP ERROR state registers
 ****************************************************************************/
#define BLE_ERRORTYPESTAT_ADDR   (0xD0220860)

#define BT_ERRORTYPESTAT_ADDR   (0xD0220460)

/***************************************************************************
 * BT CORE registers
 ****************************************************************************/

#define BT_REG_CLKCAP_REG_ADDR   0xD0220E00
#define BT_BES_CLK_REG1_ADDR   0xD0220E04
#define BT_BES_CLK_REG2_ADDR   0xD0220E08
#define BT_REG_CLKNCNT_CAP4_REG_ADDR   0xD0220E34
#define BT_BES_FINECNT_CAP4_REG_ADDR   0xD0220E38
#define BT_REG_CLKNCNT_CAP5_REG_ADDR   0xD0220E3C
#define BT_BES_FINECNT_CAP5_REG_ADDR   0xD0220E40
#define BT_REG_CLKNCNT_CAP6_REG_ADDR   0xD0220E44
#define BT_BES_FINECNT_CAP6_REG_ADDR   0xD0220E48
#define BT_REG_CLKNCNT_CAP7_REG_ADDR   0xD0220E4C
#define BT_BES_FINECNT_CAP7_REG_ADDR   0xD0220E50
#define BT_BES_CNTL1_ADDR   0xD0220C08
#define BT_BES_CNTL2_ADDR   0xD0220C0C
#define BT_BES_CNTL3_ADDR   0xD0220C10
#define BT_BES_CNTL5_ADDR   0xD0220C18
#define BT_BES_CNTLX_ADDR   0xD0220C7C
#define BT_CAP_SEL_ADDR   0xd0220C50
#define BT_TRIGREG_ADDR   0xD0220C80
#define BT_TWSBTCLKREG_ADDR   0xD0220C84
#define BT_BES_PCMCNTL_ADDR   0xD0220C88
#define BT_BES_FACNTL0_ADDR   0xD0220C8C
#define BT_BES_FACNTL1_ADDR   0xD0220C90
#define BT_BES_TG_CLKNCNT_ADDR  0xD0220C94
#define BT_BES_TG_FINECNT_ADDR  0xD0220C98
#define BT_BES_TG_CLKNCNT1_ADDR  0xD0220D08
#define BT_BES_TG_FINECNT1_ADDR  0xD0220D0C
#define BT_BES_TG_CLKNCNT2_ADDR  0xD0220D10
#define BT_BES_TG_FINECNT2_ADDR  0xD0220D14
#define BT_BES_TG_CLKNCNT3_ADDR  0xD0220D18
#define BT_BES_TG_FINECNT3_ADDR  0xD0220D1C
#define BT_BES_SYNC_INTR_ADDR   0xD0220D28
#define BT_BES_TOG_CNTL_ADDR     0xD0220D70
#define BT_BES_PCM_DF_REG1_ADDR   0xD0220CA0
#define BT_BES_PCM_DF_REG_ADDR   0xD0220C9C
#define BT_BES_FA_SWREG0_ADDR   0xD0220CC4
#define BT_BES_FA_SWREG1_ADDR   0xD0220CC8
#define BT_E_SCOMUTECNTL_0_ADDR   0xD0220614
#define BT_E_SCOTRCNTL_ADDR   0xD0220624
#define BT_E_SCOMUTECNTL_1_ADDR   0xD0220634
#define BT_E_SCOCURRENTTXPTR_ADDR   0xD0220618
#define BT_AUDIOCNTL0_ADDR   0xD0220670
#define BT_AUDIOCNTL1_ADDR   0xD0220674
#define BT_PCMGENCNTL_ADDR   0xD0220680
#define BT_PCMPHYSCNTL1_ADDR   0xD0220688
#define BT_RADIOPWRUPDN_ADDR   0xD022048C
#define BT_RADIOTXRXTIM_ADDR   0xD0220490
#define BT_BES_ENHPCM_CNTL_ADDR   0xD0220CBC
#define BT_CURRENTRXDESCPTR_ADDR   0xD0220428
#define BT_AFH_MONITOR_ADDR        0xD0220D54

#define IP_SLOTCLK_ADDR         0xD0220100
#define IP_ISOCNTSAMP_ADDR      0xD02209C4

/**
 * @brief RADIOCNTL3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:30           RXRATE3CFG   0x3
 *  29:28           RXRATE2CFG   0x2
 *  27:26           RXRATE1CFG   0x1
 *  25:24           RXRATE0CFG   0x0
 *  22:20         GETRSSIDELAY   0x4
 *     18       RXSYNC_ROUTING   0
 *  17:16          RXVALID_BEH   0x0
 *  15:14           TXRATE3CFG   0x3
 *  13:12           TXRATE2CFG   0x2
 *  11:10           TXRATE1CFG   0x1
 *  09:08           TXRATE0CFG   0x0
 *  01:00          TXVALID_BEH   0x0
 * </pre>
 */
#define BLE_RADIOCNTL3_ADDR   0xD022087C
/**
 * @brief RADIOPWRUPDN0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION0   0x0
 *  23:16             RXPWRUP0   0x0
 *  14:08             TXPWRDN0   0x0
 *  07:00             TXPWRUP0   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN0_ADDR   0xD0220880
/**
 * @brief RADIOPWRUPDN1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION1   0x0
 *  23:16             RXPWRUP1   0x0
 *  14:08             TXPWRDN1   0x0
 *  07:00             TXPWRUP1   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN1_ADDR   0xD0220884
/**
 * @brief RADIOPWRUPDN2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24       SYNC_POSITION2   0x0
 *  23:16             RXPWRUP2   0x0
 *  14:08             TXPWRDN2   0x0
 *  07:00             TXPWRUP2   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN2_ADDR   0xD0220888
/**
 * @brief RADIOPWRUPDN3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  14:08             TXPWRDN3   0x0
 *  07:00             TXPWRUP3   0x0
 * </pre>
 */
#define BLE_RADIOPWRUPDN3_ADDR   0xD022088c
/**
 * @brief RADIOTXRXTIM0 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16            RFRXTMDA0   0x0
 *  14:08           RXPATHDLY0   0x0
 *  06:00           TXPATHDLY0   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM0_ADDR   0xD0220890
/**
 * @brief RADIOTXRXTIM1 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  22:16            RFRXTMDA1   0x0
 *  14:08           RXPATHDLY1   0x0
 *  06:00           TXPATHDLY1   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM1_ADDR   0xD0220894
/**
 * @brief RADIOTXRXTIM2 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24      RXFLUSHPATHDLY2   0x0
 *  23:16            RFRXTMDA2   0x0
 *  15:08           RXPATHDLY2   0x0
 *  06:00           TXPATHDLY2   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM2_ADDR   0xD0220898
/**
 * @brief RADIOTXRXTIM3 register definition
 * <pre>
 *   Bits           Field Name   Reset Value
 *  -----   ------------------   -----------
 *  31:24      RXFLUSHPATHDLY3   0x0
 *  22:16            RFRXTMDA3   0x0
 *  06:00           TXPATHDLY3   0x0
 * </pre>
 */
#define BLE_RADIOTXRXTIM3_ADDR   0xD022089C
#endif//__BT_1306_REG_MAP_H__
