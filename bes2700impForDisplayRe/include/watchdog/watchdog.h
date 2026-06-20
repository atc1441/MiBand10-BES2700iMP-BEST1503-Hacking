
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
#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef RTOS
# ifdef __WATCHER_DOG_RESET__
# define APP_WATCHDOG
# endif
#endif

#ifdef APP_WATCHDOG
int app_wdt_open(int seconds);

int app_wdt_reopen(int seconds);

int app_wdt_close(void);

int app_wdt_set_ping_time(int seconds);

/*
 * when enable, the watchdog ping timer will also check the ilde task's heath,
 * and if the idle task was not schedule for a period time,  there maybe
 * something wrong with system, and assert will happen.
 */
int app_wdt_hung_task_check_enable(int enable);

#else
static inline int app_wdt_open(int seconds) {return 0;}

static inline int app_wdt_reopen(int seconds) {return 0;}

static inline int app_wdt_close(void) {return 0;}

static inline int app_wdt_set_ping_time(int seconds) {return 0;}

static inline int app_wdt_hung_task_check_enable(int enable) {return 0;}

#endif


#ifdef __WATCHER_DOG_RESET__

#define  system_shutdown_wdt_config(seconds) \
                        do{ \
                            hal_wdt_stop(HAL_WDT_ID_0); \
                            hal_wdt_set_irq_callback(HAL_WDT_ID_0, NULL); \
                            hal_wdt_set_timeout(HAL_WDT_ID_0, seconds); \
                            hal_wdt_start(HAL_WDT_ID_0); \
                            hal_sleep_set_sleep_hook(HAL_SLEEP_HOOK_USER_OTA, NULL); \
                        }while(0)

int watchdog_hw_stop(void);

int watchdog_hw_start(int seconds);

void watchdog_ping(void);
#else

#define  system_shutdown_wdt_config(seconds)

static inline int watchdog_hw_stop(void) {return 0;}

static inline int watchdog_hw_start(int seconds) {return 0;}

static inline void watchdog_ping(void) {}

#endif

#ifdef __cplusplus
}
#endif

#endif  /*_ __WATCHDOG_H__*/
