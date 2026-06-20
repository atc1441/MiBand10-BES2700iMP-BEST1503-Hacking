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
#ifndef __APP_BT_CMD_H__
#define __APP_BT_CMD_H__
#include "bluetooth.h"
#include "me_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    const char* string;
    void (*cmd_function)(const char* param, uint32_t param_len);
} app_bt_host_cmd_table_t;

void app_bt_host_cmd_init(void);

void app_bt_host_cmd_deinit(void);

bool app_bt_host_add_cmd_table(uint8_t cmd_number, const app_bt_host_cmd_table_t* cmd_table);

bool app_bt_host_delete_cmd_table(const app_bt_host_cmd_table_t* cmd_table);

bt_bdaddr_t *app_bt_get_pts_address(void);

unsigned int app_bt_host_cmd_callback(unsigned char *buf, unsigned int length);

#ifdef __cplusplus
}
#endif

#endif // __APP_BT_CMD_H__

