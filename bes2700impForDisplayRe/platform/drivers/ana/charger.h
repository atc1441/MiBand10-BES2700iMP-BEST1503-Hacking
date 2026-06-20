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
#ifndef __CHARGER_H__
#define __CHARGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal_analogif.h"
#include "hal_cmu.h"
#include "hal_gpio.h"
#include "hal_gpadc.h"
#include "plat_addr_map.h"
#include CHIP_SPECIFIC_HDR(charger)

#define CHARGER_DEBUG_TRACE(n, s, ...)          TR_DUMMY(n, s, ##__VA_ARGS__)
#define CHARGER_INFO_TRACE_IMM(n, s, ...)       TR_INFO((n) | TR_ATTR_IMM, s, ##__VA_ARGS__)
#define CHARGER_INFO_TRACE(n, s, ...)           TR_INFO(n, s, ##__VA_ARGS__)

#ifndef ISPI_CHG_REG
#define ISPI_CHG_REG(reg)                       (reg)
#endif
#define chg_read(reg, val)                      hal_analogif_reg_read(ISPI_CHG_REG(reg), val)
#define chg_write(reg, val)                     hal_analogif_reg_write(ISPI_CHG_REG(reg), val)

#ifdef __cplusplus
}
#endif

#endif

