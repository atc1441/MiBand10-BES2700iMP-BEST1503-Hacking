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
#ifndef __BLE_COMMON_DEFINE_H__
#define __BLE_COMMON_DEFINE_H__
#ifdef BLE_HOST_SUPPORT
#include "ble_device_info.h"
#ifdef __cplusplus
extern "C" {
#endif

/// APS3 is little endian
#define CPU_LE          1

typedef enum
{
    BLE_CORE_ACC_MCC,
    BLE_CORE_ACC_TBC,
} BLE_CORE_ACC_TYPE_E;

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BLE_COMMON_DEFINE_H__ */
