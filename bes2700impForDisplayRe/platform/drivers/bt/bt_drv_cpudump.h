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
#ifndef __BT_DRV_CPUDUMP_H__
#define __BT_DRV_CPUDUMP_H__

#ifdef __cplusplus
extern "C" {
#endif

void btdrv_cpudump_clk_enable(void);
void btdrv_cpudump_clk_disable(void);
void btdrv_cpudump_enable(void);
void btdrv_cpudump_disable(void);

#ifdef __cplusplus
}
#endif

#endif
