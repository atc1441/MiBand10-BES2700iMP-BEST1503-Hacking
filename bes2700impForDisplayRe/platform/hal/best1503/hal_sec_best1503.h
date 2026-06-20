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
#ifndef __HAL_SEC_BEST1306_H__
#define __HAL_SEC_BEST1306_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

void hal_sec_set_sens2mcu_nonsec(bool nonsec);
void hal_sec_set_sens2aon_nonsec(bool nonsec);

void hal_sec_cfg_nonsec_bypass(bool bypass);

void hal_sec_set_sram0_nosec(bool nonsec);
void hal_sec_set_sram1_nosec(bool nonsec);
void hal_sec_set_sram2_nosec(bool nonsec);
void hal_sec_set_sram3_nosec(bool nonsec);
void hal_sec_set_sram4_nosec(bool nonsec);
void hal_sec_set_sram5_nosec(bool nonsec);
void hal_sec_set_sram6_nosec(bool nonsec);
void hal_sec_set_sram7_nosec(bool nonsec);
void hal_sec_set_sram8_nosec(bool nonsec);
void hal_sec_set_mcu2sens_nosec(bool nonsec);

#ifdef __cplusplus
}
#endif

#endif

