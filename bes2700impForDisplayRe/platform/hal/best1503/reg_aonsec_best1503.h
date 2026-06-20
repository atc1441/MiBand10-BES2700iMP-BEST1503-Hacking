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
#ifndef __REG_AONSEC_BEST1306_H__
#define __REG_AONSEC_BEST1306_H__

#include "plat_types.h"

struct AONSEC_T {
    __I  uint32_t NONSEC_I2C;           // 0x00
    __IO uint32_t NONSEC_PROT;          // 0x04
    __IO uint32_t NONSEC_BYPASS_PROT;   // 0x08
    __IO uint32_t SEC_BOOT_ACC;         // 0x0C
};

// reg_00
#define AON_SEC_CFG_NONSEC_I2C_SLV                          (1 << 0)
#define AON_SEC_CFG_NONSEC_SENS                             (1 << 1)

// reg_04
#define AON_SEC_CFG_NONSEC_PROT_APB0(n)                     (((n) & 0x3FFFFF) << 0)
#define AON_SEC_CFG_NONSEC_PROT_APB0_MASK                   (0x3FFFFF << 0)
#define AON_SEC_CFG_NONSEC_PROT_APB0_SHIFT                  (0)

// reg_08
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0(n)              (((n) & 0x3FFFFF) << 0)
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0_MASK            (0x3FFFFF << 0)
#define AON_SEC_CFG_NONSEC_BYPASS_PROT_APB0_SHIFT           (0)

// reg_0c
#define AON_SEC_SECURE_BOOT_JTAG                            (1 << 0)
#define AON_SEC_SECURE_BOOT_I2C                             (1 << 1)

#endif

