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
#ifndef __BT_SERVICE_BUF_H__
#define __BT_SERVICE_BUF_H__

/*****************************header include********************************/
#include <stdint.h>

/*********************external declaration*************************/

/**********************private function declaration*************************/

/************************private macro defination***************************/

/************************private variable defination************************/

void bt_svc_base_buf_init();

void bt_svc_base_buf_deinit();

uint8_t* bt_svc_base_buf_malloc(uint32_t size);

void bt_svc_base_buf_free(void *p_buf);

#endif