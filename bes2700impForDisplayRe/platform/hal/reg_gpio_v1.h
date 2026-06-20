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
#ifndef __REG_GPIO_V1_H__
#define __REG_GPIO_V1_H__

#include "plat_types.h"

#define HAL_GPIO_PORT_NUM                               1

struct GPIO_PORT_T {
    __IO uint32_t GPIO_DR;                              // 0x00
    __IO uint32_t GPIO_DDR;                             // 0x04
    __IO uint32_t GPIO_CTL;                             // 0x08
};

struct GPIO_BANK_T {
    struct GPIO_PORT_T port[HAL_GPIO_PORT_NUM];
    struct GPIO_PORT_T _port_reserved[3];
    __IO uint32_t GPIO_INTEN;                           // 0x30
    __IO uint32_t GPIO_INTMASK;                         // 0x34
    __IO uint32_t GPIO_INTTYPE_LEVEL;                   // 0x38
    __IO uint32_t GPIO_INT_POLARITY;                    // 0x3C
    __I  uint32_t GPIO_INTSTATUS;                       // 0x40
    __I  uint32_t GPIO_RAW_INTSTATUS;                   // 0x44
    __IO uint32_t GPIO_DEBOUNCE;                        // 0x48
    __O  uint32_t GPIO_PORTA_EOI;                       // 0x4C
    __I  uint32_t GPIO_EXT_PORT[HAL_GPIO_PORT_NUM];     // 0x50
    __I  uint32_t GPIO_EXT_PORT_reserved[3];
    __IO uint32_t GPIO_LS_SYNC;                         // 0x60
};

#endif
