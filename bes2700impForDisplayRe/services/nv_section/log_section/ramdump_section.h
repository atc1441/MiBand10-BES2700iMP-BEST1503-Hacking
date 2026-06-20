/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef _RAMDUMP_SECTION_H
#define _RAMDUMP_SECTION_H

#if defined(__cplusplus)
extern "C" {
#endif

#define RAM_DUMP_PREFIX     "__RAM_DUMP:"

#if 1
#define RAM_DUMP_TRACE(num,fmt, ...)  TRACE(num,fmt, ##__VA_ARGS__)
#else
#define RAM_DUMP_TRACE(num,fmt, ...)
#endif

void ramdump_update_execute_state_machine(enum HAL_TRACE_STATE_T ramdump_execute_state);

enum HAL_TRACE_STATE_T ramdump_get_execute_state_machine(void);

void ramdump_to_flash_init(void);

void ramdump_to_flash_handler(enum HAL_TRACE_STATE_T ramdump_executor);

#if defined(__cplusplus)
}
#endif

#endif // _RAMDUMP_SECTION_H


