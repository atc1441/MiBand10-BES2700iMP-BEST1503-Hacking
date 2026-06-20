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
#ifndef __APP_IBRT_BT_PROFILE_SYNC__
#define __APP_IBRT_BT_PROFILE_SYNC__

#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

uint8_t app_ibrt_pack_bt_profile_data(uint8_t* buf, uint16_t* len, uint16_t conhdl);
uint8_t app_ibrt_unpack_bt_profile_data(uint8_t* buf, uint16_t len);
#ifdef __cplusplus
}
#endif
#endif
