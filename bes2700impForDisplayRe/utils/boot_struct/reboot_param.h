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
#ifndef __REBOOT_PARAM_H__
#define __REBOOT_PARAM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

enum REBOOT_PARAM_ID_T {
    REBOOT_PARAM_ID_BOOTMODE = 0,
    REBOOT_PARAM_ID_GPIO,
    REBOOT_PARAM_ID_2,
    REBOOT_PARAM_ID_3,
    REBOOT_PARAM_ID_QTY,
};

#define REBOOT_PARAM_BOOTMODE_NORMAL                 (1 << 0)
#define REBOOT_PARAM_BOOTMODE_KEYEVENT               (1 << 1)
#define REBOOT_PARAM_BOOTMODE_RTC_ALARM              (1 << 2)
#define REBOOT_PARAM_BOOTMODE_FACTORY_RESET_NORMAL   (1 << 3)
#define REBOOT_PARAM_BOOTMODE_FACTORY_RESET_INQUIRY  (1 << 4)
#define REBOOT_PARAM_BOOTMODE_POWEROFF_MODE          (1 << 5)
#define REBOOT_PARAM_BOOTMODE_LOW_BATT_MODE          (1 << 6)

#define REBOOT_PARAM_GPIO_KEEP_STATUS_SIHFT (0)
#define REBOOT_PARAM_GPIO_KEEP_STATUS_MASK  (0xFF << REBOOT_PARAM_EXTBOOT_GPIO_KEEP_STATUS_SIHFT)

void reboot_param_init(void);
void reboot_param_set(enum REBOOT_PARAM_ID_T id, uint32_t flags);
void reboot_param_clear(enum REBOOT_PARAM_ID_T id, uint32_t flags);
uint32_t reboot_param_get(enum REBOOT_PARAM_ID_T id);

#ifdef __cplusplus
}
#endif

#endif

