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
#ifndef _APP_HAP_HAC_MSG_H_
#define _APP_HAP_HAC_MSG_H_
#include "bluetooth.h"
#ifdef __cplusplus
extern "C" {
#endif
#ifdef AOB_MOBILE_ENABLED
int app_hap_hac_msg_set_preset_name(uint8_t con_lid, uint8_t preset_idx, uint8_t length, char *name);
int app_hap_hac_msg_get_preset(uint8_t con_lid, uint8_t preset_idx, bool is_read_all);
int app_hap_hac_msg_set_active_preset(uint8_t con_lid, uint8_t opcode, uint8_t preset_idx);
#endif
#ifdef __cplusplus
}
#endif
#endif