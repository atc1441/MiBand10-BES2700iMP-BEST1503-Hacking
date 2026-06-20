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

/*****************************header include********************************/
#include "gsound_dbg.h"
#include "gsound_custom_battery.h"

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/
static GSoundBatteryInterface *battery_interface;

/****************************function defination****************************/

/**
 * @brief store battery interface pointer comes from libgsound
 * 
 * @param interface pointer of battery interface
 */
void gsound_custom_battery_store_lib_interface(GSoundBatteryInterface *interface)
{
    battery_interface = interface;
}

/**
 * @brief battery callback function of gsound module
 * 
 * @param currVolt current battery voltage
 * @param currLevel current battery level
 * @param currStatus current battery status
 * @param status status
 * @param APP_BATTERY_MSG_PRAMS battery message param
 */
void gsound_custom_battery_callback(APP_BATTERY_MV_T currVolt,
                                    uint8_t currLevel,
                                    enum APP_BATTERY_STATUS_T currStatus,
                                    uint32_t status,
                                    union APP_BATTERY_MSG_PRAMS prams)
{
    static BesBatteryStatus last_battery = {
        APP_BATTERY_STATUS_INVALID,
        (APP_BATTERY_STATUS_T)(APP_BATTERY_LEVEL_MIN - 1),
    };

    BesBatteryStatus curr_battery = {
        currLevel,
        currStatus,
    };

    if (gsound_battery_equals(&last_battery, &curr_battery) == false)
    {
        GSoundBatteryInfo gsound_battery;
        last_battery = curr_battery;
        gsound_battery_conv(&curr_battery, &gsound_battery);

        GLOG_I("%s BATTERY CHANGE %d %d %d %d",
               __func__,
               curr_battery.level,
               curr_battery.status,
               gsound_battery.percent_full,
               gsound_battery.charging);

        battery_interface->gsound_battery_ready(&gsound_battery);
    }
}

/**
 * @brief judge if the two battery level/status is equal or not
 * 
 * @param level_1 first battery level
 * @param status_1 first battery status
 * @param level_2 second battery level
 * @param status_2 second battery status
 * @return true the two battery level/status is equal
 * @return false the two battery level/status is not equal
 */
bool gsound_battery_equals_values(const uint8_t level_1,
                                  const enum APP_BATTERY_STATUS_T status_1,
                                  const uint8_t level_2,
                                  const enum APP_BATTERY_STATUS_T status_2)
{
    const BesBatteryStatus val_1 = {
        level_1,
        status_1,
    };

    const BesBatteryStatus val_2 = {
        level_2,
        status_2,
    };

    return gsound_battery_equals(&val_1, &val_2);
}

/**
 * @brief : judge if the two battery level & status equals or not
 * 
 * @param value_1 the first battery level & status
 * @param value_2 the second battery level & status
 * @return true the two battery level & status equals
 * @return false the two battery level & status not equals
 */
bool gsound_battery_equals(BesBatteryStatus const *const value_1,
                           BesBatteryStatus const *const value_2)
{
    return ((value_2->level == value_1->level) &&
            (value_2->status == value_1->status));
}

/**
 * @brief convert battery info from bes format into gsound format
 * 
 * @param bes_battery battery info in bes format
 * @param gsound_battery battery info in gsound info
 */
void gsound_battery_conv(BesBatteryStatus const *const bes_battery,
                         GSoundBatteryInfo *const gsound_battery)
{
    int32_t percent = ((bes_battery->level + 1) * 100) +
                      ((APP_BATTERY_LEVEL_MAX + 1) / 2);
    percent /= (APP_BATTERY_LEVEL_MAX + 1);

    gsound_battery->percent_full = percent;
    gsound_battery->charging = ((bes_battery->status == APP_BATTERY_STATUS_CHARGING)
                                    ? true
                                    : false);
}
