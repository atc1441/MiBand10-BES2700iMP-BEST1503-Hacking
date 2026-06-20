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
#ifndef __BT_DRV_1306_CONFIG_H__
#define __BT_DRV_1306_CONFIG_H__
/***************************************************************************
 *BTC sleep and wakeup configration
 ****************************************************************************/

/*
*   TWOSC: Time to wake-up osc_en before deepsleep_time expiration
*   TWRM  : Time to wake-up radio module(no used)
*   TWEXT : Time to wake-up osc_en on external wake-up request
*/
//BTC power off section define
#define BTC_LP_OFF       (0x0000)
#define LOGIC_POWEROFF_EN      (0x0004)
#define EM0_POWEROFF_EN       (0x0008)
#define EM1_POWEROFF_EN       (0x0010)
#define RAM0_POWEROFF_EN      (0x0020)
#define RAM1_POWEROFF_EN      (0x0040)
#define RAM2_POWEROFF_EN      (0x0080)
#define RAM3_POWEROFF_EN      (0x0100)
#define RAM4_POWEROFF_EN      (0x0200)
#define RAM5_POWEROFF_EN      (0x0400)
#define EM2_POWEROFF_EN       (0x0800)
#define EM3_POWEROFF_EN       (0x1000)

#define IP_PROG_DELAY_DFT (3)

#define BTC_LP_MODE         (BTC_LP_OFF)

/***************************************************************************
 *BT phy configration
 ****************************************************************************/

#define PHY_TX_DELAY (0x20)
#define PHY_RX_DELAY (0x27)

/***************************************************************************
 *channel assessment config
 ****************************************************************************/
#define AFH_ASSESS_GAIN  2

/***************************************************************************
 *TX power configration
 ****************************************************************************/

#define BT_MAX_TX_PWR_IDX    (5)        //idx0~5
#define BT_INIT_TX_PWR_IDX    (3)

#ifdef __HW_AGC__
#define BTC_HW_AGC_ENABLE_FLAG 0x01
#else
#define BTC_HW_AGC_ENABLE_FLAG 0x00
#endif

#ifdef __HW_AGC__
#define HW_AGC_HIGH_PWEF_MODE   (0x0)
#define HW_AGC_PWR_SAVE_MODE     (0x1)
#endif

//RF BT TX register
#define RF_BT_TX_PWR_IDX0_REG (0x49)
#define RF_BT_TX_PWR_IDX1_REG (0x4A)
#define RF_BT_TX_PWR_IDX2_REG (0x4B)
#define RF_BT_TX_PWR_IDX3_REG (0x4C)
#define RF_BT_TX_PWR_IDX4_REG (0x4D)
#define RF_BT_TX_PWR_IDX5_REG (0x4E)
#define RF_BT_TX_PWR_IDX6_REG (0x4F)
//RF BLE TX register
#define RF_BLE_TX_PWR_IDX0_REG (0x49)
#define RF_BLE_TX_PWR_IDX1_REG (0x4A)
#define RF_BLE_TX_PWR_IDX2_REG (0x4B)
#define RF_BLE_TX_PWR_IDX3_REG (0x4C)
#define RF_BLE_TX_PWR_IDX4_REG (0x4D)
#define RF_BLE_TX_PWR_IDX5_REG (0x4E)
/***************************************************************************
 *RX gain configration
 ****************************************************************************/
/***************************************************************************
 *Fast ack configration
 ****************************************************************************/
#define FA_RX_WIN_SIZE      (0xA)
#define FA_RXPWRUP_TIMING   (0x37)
#define FA_TXPWRUP_TIMING   (0x28)
#define FA_BW2M_RXPWRUP_TIMING   (0x27)
#define FA_BW2M_TXPWRUP_TIMING   (0x28)

#if defined (CTKD_ENABLE)|| defined (__3M_PACK__)||defined(__FASTACK_ECC_ENABLE__)
#define FA_CNT_PKT_US   (0x1F)
#else
#define FA_CNT_PKT_US   (0x4)
#endif

#define FA_FIX_TX_GIAN_IDX  (0x4)
#define FA_FIX_RX_GIAN_IDX  (0x1)
#define FA_NEW_CORR_VALUE   (0x40)
//lager than 2 will causes error synchronization
#define FA_OLD_CORR_VALUE   (0x1)
#define FA_MULTI_TX_COUNT   (0x3)
#define INVALID_FA_FREQ_TABLE       (0)
/***************************************************************************
 *BTC activity number configration
 ****************************************************************************/
#define BLE_ACTIVITY_MAX ((hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_1)? 20:14)

#define MAX_NB_ACTIVE_ACL                   (5)
#define HCI_HANDLE_MIN         (0x80)
#define HCI_HANDLE_MAX         (HCI_HANDLE_MIN + MAX_NB_ACTIVE_ACL - 1)

#endif //__BT_DRV_1306_CONFIG_H__