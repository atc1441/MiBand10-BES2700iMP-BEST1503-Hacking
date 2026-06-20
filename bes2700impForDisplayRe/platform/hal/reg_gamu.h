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


#ifndef __GAMU_REG_DEFINE_H
#define __GAMU_REG_DEFINE_H

#include "plat_types.h"
#include "plat_addr_map.h"

#define GAMU_REG_BASE               GAMU_BASE
#define GAMU_VIRTUAL_ADDRSPACE_BASE 0x60000000
#define GAMU_VIRTUAL_ADDRSPACE_TOP  0x6FFF0000

/*
GAMU_LUT_ENTRY_X_CR
*/
#define GAMU_LUT_ENTRY_BASE_ADDR ( GAMU_REG_BASE )
#define GAMU_LUT_ENTRY_X_ADDR(x) (GAMU_LUT_ENTRY_BASE_ADDR + 4 * x)

/*
GAMU_BUFFERX_PHYADDR_CR
*/
#define GAMU_BUFFER_PHYADDR_BASE_ADDR ( GAMU_REG_BASE  + 0x800)
#define GAMU_BUFFERx_PHYADDR_CR_ADDR(x) ( GAMU_BUFFER_PHYADDR_BASE_ADDR  + 4*x)

/*
GAMU_CR
*/
#define GAMU_CR_ADDR ( GAMU_REG_BASE  + 0x810)

/*
GAMU_HADDR_UNVALID_CR
*/
#define GAMU_HADDR_UNVALID_CR_ADDR ( GAMU_REG_BASE  + 0x814)

/* GAMU_LUT_ENTRY_X_CR */
#define GAMU_LUT_ENTRY_X_BLOCK_OFFSET_SHIFT   (16)    /* xth line block offset bits(16:31)*/
#define GAMU_LUT_ENTRY_X_BLOCK_OFFSET_MASK    (0xFFFF << GAMU_LUT_ENTRY_X_BLOCK_OFFSET_SHIFT)
#define GAMU_LUT_ENTRY_X_BLOCK_OFFSET(n)      ((n & 0xffff) << GAMU_LUT_ENTRY_X_BLOCK_OFFSET_SHIFT)
#define GAMU_LUT_ENTRY_X_ENA_SHIFT            (15)    /* xth line enable flag bits(15)*/
#define GAMU_LUT_ENTRY_X_ENA_MASK             (0x1<< GAMU_LUT_ENTRY_X_ENA_SHIFT)
#define GAMU_LUT_ENTRY_X_ENA(n)              ((n & 0x01) <<GAMU_LUT_ENTRY_X_ENA_SHIFT)
#define GAMU_LUT_ENTRY_X_END_BLOCK_SHIFT      (7)    /* xth line start block bits(7:13)*/
#define GAMU_LUT_ENTRY_X_END_BLOCK_MASK       (0x7F<< GAMU_LUT_ENTRY_X_END_BLOCK_SHIFT)
#define GAMU_LUT_ENTRY_X_END_BLOCK(n)         ((n & 0x7f) <<GAMU_LUT_ENTRY_X_END_BLOCK_SHIFT)
#define GAMU_LUT_ENTRY_X_START_BLOCK_SHIFT    (0)    /* xth line start block bits(6:0) */
#define GAMU_LUT_ENTRY_X_START_BLOCK_MASK     (0x7F<< GAMU_LUT_ENTRY_X_START_BLOCK_SHIFT)
#define GAMU_LUT_ENTRY_X_START_BLOCK(n)       ((n & 0x7f) <<GAMU_LUT_ENTRY_X_START_BLOCK_SHIFT)

/* GAMU BUFFERX Register */
#define GAMU_BUFFER_X_PHYADDR_SHIFT    (0)    /* xth buffer physical memory address (31:0) */
#define GAMU_BUFFER_X_PHYADDR_MASK     (0xFFFFFFFF<< GAMU_BUFFER_X_PHYADDR_SHIFT)
#define GAMU_BUFFER_X_PHYADDR(n)       ((n & 0xffffffff) <<GAMU_BUFFER_X_PHYADDR_SHIFT)

/* GAMU_CONTROL Register */
#define GAMU_CROSS_1K_ENA_SHIFT        (6)    /* cross 1k enable bits(6) always 1*/
#define GAMU_CROSS_1KENA_MASK          (0x1<< GAMU_CROSS_1K_ENA_SHIFT)
#define GAMU_CROSS_1K_ENA(n)           ((n & 0x01) << GAMU_CROSS_1K_ENA_SHIFT)
#define GAMU_VVM_ENA_SHIFT             (5)    /* vvm enable bits(5)  always 1 */
#define GAMU_VVM_ENA_MASK              (0x1<< GAMU_VVM_ENA_SHIFT)
#define GAMU_VVM_ENA(n)                ((n & 0x01) << GAMU_VVM_ENA_SHIFT)
#define GAMU_RGB_INPUT_FMT_SHIFT       (3)    /* rgb input format bits(4:3) 00-32bit 01-24bit 10-16bit */
#define GAMU_RGB_INPUT_FMT_MASK        (0x3 << GAMU_RGB_INPUT_FMT_SHIFT)
#define GAMU_RGB_INPUT_FMT(n)          ((n & 0x03) << GAMU_RGB_INPUT_FMT_SHIFT)
#define GAMU_PHYADDR_SELECT_ENA_SHIFT  (2)    /* physical address select enable bits(2) always 0*/
#define GAMU_PHYADDR_SELECT_ENA_MASK   (0x1<< GAMU_PHYADDR_SELECT_ENA_SHIFT)
#define GAMU_PHYADDR_SELECT_ENA(n)     ((n & 0x01) << GAMU_PHYADDR_SELECT_ENA_SHIFT)
#define GAMU_PHYADDR_SELECT_SHIFT      (0)    /* physical address select bits(1:0) */
#define GAMU_PHYADDR_SELECT_MASK       (0x3 << GAMU_PHYADDR_SELECT_SHIFT)
#define GAMU_PHYADDR_SELECT(n)         ((n & 0x3) << GAMU_PHYADDR_SELECT_SHIFT)

#endif /* __GAMU_REG_DEFINE_H */
