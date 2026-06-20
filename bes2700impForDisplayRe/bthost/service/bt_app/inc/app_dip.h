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
#ifndef __APP_DIP_H__
#define __APP_DIP_H__

#include "dip_common_define.h"

#ifdef __cplusplus
extern "C" {
#endif

void app_dip_sync_init(void);
void app_dip_init(void);

typedef void (*app_dip_info_queried_callback_func_t)(uint8_t *remote_addr, bt_dip_pnp_info_t *pnp_info);

void app_dip_register_dip_info_queried_callback(app_dip_info_queried_callback_func_t func);

void app_dip_get_remote_info(uint8_t device_id);

#ifdef __cplusplus
}
#endif

#endif

