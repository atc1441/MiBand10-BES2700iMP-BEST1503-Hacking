/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __REG_GPIO_V2_H__
#define __REG_GPIO_V2_H__

#include "plat_types.h"

struct GPIO_PORT_T {
    __IO uint32_t GPIO_DR;                              // 0x00
    __IO uint32_t GPIO_DDR;                             // 0x04
    __IO uint32_t GPIO_CTL;                             // 0x08
};

struct GPIO_BANK_T {
    __IO uint32_t GPIO_DR_SET;                          // 0x00
    __IO uint32_t GPIO_DDR_SET;                         // 0x04
    __IO uint32_t GPIO_CTL_SET;                         // 0x08
    __IO uint32_t GPIO_DR_CLR;                          // 0x0C
    __IO uint32_t GPIO_DDR_CLR;                         // 0x10
    __IO uint32_t GPIO_CTL_CLR;                         // 0x14
    __IO uint32_t GPIO_INTEN_CLR;                       // 0x18
    __IO uint32_t GPIO_INTMASK_CLR;                     // 0x1C
    __IO uint32_t GPIO_INTTYPE_LEVEL_CLR;               // 0x20
    __IO uint32_t GPIO_INT_POLARITY_CLR;                // 0x24
    __IO uint32_t GPIO_DEBOUNCE_CLR;                    // 0x28
    __IO uint32_t RESERVED_2C;                          // 0x2C
    __IO uint32_t GPIO_INTEN_SET;                       // 0x30
    __IO uint32_t GPIO_INTMASK_SET;                     // 0x34
    __IO uint32_t GPIO_INTTYPE_LEVEL_SET;               // 0x38
    __IO uint32_t GPIO_INT_POLARITY_SET;                // 0x3C
    __I  uint32_t GPIO_INTSTATUS;                       // 0x40
    __I  uint32_t GPIO_RAW_INTSTATUS;                   // 0x44
    __IO uint32_t GPIO_DEBOUNCE_SET;                    // 0x48
    __O  uint32_t GPIO_PORTA_EOI;                       // 0x4C
    __I  uint32_t GPIO_EXT_PORT;                        // 0x50
    __IO uint32_t RESERVED_54[3];                       // 0x54
    __IO uint32_t GPIO_LS_SYNC;                         // 0x60
};

#endif
