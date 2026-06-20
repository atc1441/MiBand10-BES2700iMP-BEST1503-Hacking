/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/

#ifndef __GSOUND_CUSTOM_BATTERY_H__
#define __GSOUND_CUSTOM_BATTERY_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "app_battery.h"
#include "gsound_target_battery.h"

/******************************macro defination*****************************/

/******************************type defination******************************/
typedef struct {
    uint8_t level;
    enum APP_BATTERY_STATUS_T status;
} BesBatteryStatus;

/****************************function declearation**************************/
void gsound_custom_battery_store_lib_interface(GSoundBatteryInterface *interface);

void gsound_custom_battery_callback(APP_BATTERY_MV_T currVolt,
                                    uint8_t currLevel,
                                    enum APP_BATTERY_STATUS_T currStatus,
                                    uint32_t status,
                                    union APP_BATTERY_MSG_PRAMS prams);

bool gsound_battery_equals_values(const uint8_t level_1,
                                  const enum APP_BATTERY_STATUS_T status_1,
                                  const uint8_t level_2,
                                  const enum APP_BATTERY_STATUS_T status_2);

bool gsound_battery_equals(BesBatteryStatus const *const value_1,
                           BesBatteryStatus const *const value_2);

void gsound_battery_conv(BesBatteryStatus const *const bes_battery,
                         GSoundBatteryInfo *const gsound_battery);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_BATTERY_H__ */