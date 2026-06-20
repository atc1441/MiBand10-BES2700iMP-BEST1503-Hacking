/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#ifndef __BLE_AHP_H__
#define __BLE_AHP_H__
#ifdef CFG_APP_AHP_SERVER
#include "gatt_service.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef enum ahp_service_char_ahp_role
{
    AHS_AHP_ROLE_AMG           = 0x01,
    AHS_AHP_ROLE_AMT           = 0x02,
    AHS_AHP_ROLE_MASK          = 0x03,
} __attribute__((packed)) ahp_service_char_ahp_role_t;

typedef enum ahp_service_char_amg_features
{
    AHS_AMG_FEATURES_64K_SOURCE_SUPPORTED           = 0x01,
    AHS_AMG_FEATURES_96K_SOURCE_SUPPORTED           = 0x02,
    AHS_AMG_FEATURES_MASK                           = 0x03,
} __attribute__((packed)) ahp_service_char_amg_features_t;

typedef enum ahp_service_char_amt_features
{
    AHS_AMT_FEATURES_SOURCE_SUPPORTED               = 0x01,
    AHS_AMT_FEATURES_48K_SOURCE_SUPPORTED           = 0x02,
    AHS_AMT_FEATURES_64K_SOURCE_SUPPORTED           = 0x04,
    AHS_AMT_FEATURES_64K_SINK_SUPPORTED             = 0x08,
    AHS_AMT_FEATURES_96k_SINK_SUPPORTED             = 0x10,
    AHS_AMT_FEATURES_MASK                           = 0x1F,
} __attribute__((packed)) ahp_service_char_amt_features_t;

#ifdef __cplusplus
    }
#endif
#endif /* CFG_APP_AHP_SERVER */
#endif /* __BLE_AHP_H__ */