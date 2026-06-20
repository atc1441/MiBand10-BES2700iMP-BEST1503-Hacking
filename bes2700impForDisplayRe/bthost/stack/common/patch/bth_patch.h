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
#ifndef __BT_PATCH_H__
#define __BT_PATCH_H__

#include "bluetooth.h"

#include "bth_patch_basic.h"
#if BLE_AUDIO_ENABLED
#include "bth_patch_gaf.h"
#endif
#include "bth_patch_l2cap.h"
#include "bth_patch_hci.h"
#include "bth_patch_btprf.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined (BUILD_BTH_ROM)

#define __BTH_STATIC
#define call_wrapper(module, func, ...)         (((func##_func_t)(g_patch_tbl_##module[FUNC_ID_##module##_##func]))(__VA_ARGS__))
#define call_get_func(module, func)             ((func##_func_t)(g_patch_tbl_##module[FUNC_ID_##module##_##func]))
#else
#define __BTH_STATIC                            static
#define call_wrapper(module, func, ...)         func(__VA_ARGS__)
#define call_get_func(module, func)             func

#endif  /* BUILD_BTH_ROM */

#define patch_place_func(module, func, patch)   do { \
                                                        g_patch_tbl_##module[FUNC_ID_##module##_##func] = (uint32_t)patch; \
                                                } while (0)

#ifdef __cplusplus
}
#endif

#endif /* __BT_PATCH_H__ */