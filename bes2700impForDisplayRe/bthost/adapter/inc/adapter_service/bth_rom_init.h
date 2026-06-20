/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __BT_ROM_INIT_H__
#define __BT_ROM_INIT_H__

#ifdef BTH_IN_ROM
#ifdef __cplusplus
extern "C" {
#endif

void bth_rom_init(void);

void bth_stack_patch_init(void);

#ifdef __cplusplus
}
#endif
#endif /* BTH_IN_ROM */

#endif /* __BT_ROM_INIT_H__ */
