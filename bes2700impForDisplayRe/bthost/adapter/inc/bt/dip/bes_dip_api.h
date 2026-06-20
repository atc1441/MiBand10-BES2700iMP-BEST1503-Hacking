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
#ifndef __BES_DIP_API_H__
#define __BES_DIP_API_H__
#include "dip_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

void bes_bt_dip_app_sync_init(void);

void bes_bt_dip_query_remote_info(uint8_t device_id);

bt_dip_pnp_info_t* bes_bt_dip_get_device_info(bt_bdaddr_t *remote);

bool bes_bt_dip_check_is_ios_device(uint16_t vend_id, uint16_t vend_id_source);

void bes_bt_dip_set_device_info_queried(const bt_bdaddr_t *remote, bool queried);

#ifdef __cplusplus
}
#endif
#endif /* __BES_DIP_API_H__ */
