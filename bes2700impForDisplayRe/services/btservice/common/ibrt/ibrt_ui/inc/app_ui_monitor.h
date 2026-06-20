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
#ifndef __APP_UI_MONITOR_H__
#define __APP_UI_MONITOR_H__

#include <stdint.h>

#define IO_EVENT        (1)
#define SUPER_SM        (2)
#define BTTWS_SM        (3)
#define BTMOB_SM        (4)
#define LEMOB_SM        (5)

void app_ui_mnt_record(uint32_t target, uint32_t event, uint32_t ext_event = 0);
void app_ui_mnt_record(uint32_t target, uint32_t state, uint32_t event, uint32_t ext_event);
void app_ui_mnt_record(uint32_t target, uint32_t s_src, uint32_t s_dst, uint32_t event, uint32_t ext_event);
void app_ui_mnt_start();
void app_ui_mnt_stop();
void app_ui_mnt_dump();

#endif /* __APP_UI_MONITOR_H__ */
