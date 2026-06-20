/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __APP_AVRCP_TARGET_H__
#define __APP_AVRCP_TARGET_H__
#include "avrcp_api.h"

#ifdef __cplusplus
extern "C" {
#endif

void avrcp_target_send_play_status_change_notify(uint8_t device_id);

#ifdef __cplusplus
}
#endif

#endif /* __APP_AVRCP_TARGET_H__ */

