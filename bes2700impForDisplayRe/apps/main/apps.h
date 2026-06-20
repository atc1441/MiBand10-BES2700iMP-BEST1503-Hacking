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
#ifndef __APPS_H__
#define __APPS_H__

#include "app_status_ind.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

int app_init(void);

int app_deinit(int deinit_case);

int app_shutdown(void);

int app_reset(void);

void app_10_second_timer_check(void);

int app_switch_mode(int mode, bool reboot);

int app_voice_stop(APP_STATUS_INDICATION_T status, uint8_t device_id);

void app_enter_signalingtest_mode(void);
void app_enter_non_signalingtest_mode(void);


/*FixME*/
void app_status_set_num(const char* p);

////////////10 second tiemr///////////////
#define APP_FAST_PAIRING_TIMEOUT_IN_SECOND  120

#define APP_PAIR_TIMER_ID       0
#define APP_POWEROFF_TIMER_ID   1
#define APP_FASTPAIR_LASTING_TIMER_ID   2

void app_stop_10_second_timer(uint8_t timer_id);
void app_start_10_second_timer(uint8_t timer_id);

void app_start_postponed_reset(void);
#ifdef PROMPT_IN_FLASH
void app_start_ota_language_reset(void);
#endif

bool app_is_power_off_in_progress(void);

#define CHIP_ID_C     1
#define CHIP_ID_D     2

bool app_is_stack_ready(void);

void app_start_power_consumption_thread(void);

void app_reset_gpio_status(void);

int app_init_btc(void);
int app_deinit_btc(int deinit_case);

void app_application_ready_to_start_callback(void);

void system_power_on_callback(void);

void system_power_off_callback(uint8_t sys_case);

void hw_revision_checker(void);

void app_enter_fastpairing_mode(void);

void app_exit_fastpairing_mode(void);


////////////////////


#ifdef __cplusplus
}
#endif
#endif//__FMDEC_H__
