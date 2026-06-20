/**
 * @file gsound_target_ota.cpp
 * @author BES AI team
 * @version 0.1
 * @date 2020-04-17
 * 
 * @copyright Copyright (c) 2015-2020 BES Technic.
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
 */

/*****************************header include********************************/
#include "gsound_target_ota.h"
#include "gsound_dbg.h"
#include "gsound_custom_ota.h"

/*********************external function declearation************************/

/************************private macro defination***************************/
#ifdef GSOUND_HOTWORD_ENABLED
#define GSOUND_OTA_DEVICE_COUNT 2
#else
#define GSOUND_OTA_DEVICE_COUNT 1
#endif

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/

/****************************function defination****************************/
GSoundStatus GSoundTargetOtaBegin(const GSoundOtaSessionSettings *ota_settings)
{
    GSoundStatus ret = gsound_custom_ota_begin(ota_settings);

    return ret;
}

GSoundStatus GSoundTargetOtaData(const uint8_t *data,
                                 uint32_t length,
                                 uint32_t byte_offset,
                                 GSoundOtaDataResult *ota_status,
                                 GSoundOtaDeviceIndex device_index)
{
    ASSERT(NULL != data, "ota data ptr is null");
    ASSERT(NULL != ota_status, "ota status ptr is null");

    GSoundStatus ret = gsound_custom_ota_data(data,
                                              length,
                                              byte_offset,
                                              ota_status,
                                              device_index);

    return ret;
}

GSoundStatus GSoundTargetOtaApply(GSoundOtaDeviceIndex device_index,
                                  bool is_zero_day_ota)
{
    GSoundStatus ret = gsound_custom_ota_apply(device_index,
                                               is_zero_day_ota);

    return ret;
}

GSoundStatus GSoundTargetOtaAbort(GSoundOtaDeviceIndex device_index)
{
    GSoundStatus ret = gsound_custom_ota_abort(device_index);

    return ret;
}

void GSoundTargetOtaGetStatus(GSoundOtaStatus *target_ota_status,
                              GSoundOtaDeviceIndex device_index)
{
    GLOG_D("%s received device index:%d", __func__, device_index);

    // set receiving other flag to flase by default
    target_ota_status->is_receiving_other = false;

    if (GSOUND_TARGET_OTA_DEVICE_INDEX_FULL_FIRMWARE == device_index)
    {
        target_ota_status->is_receiving_other = gsound_custom_ota_is_receiving_other();
    }

    GLOG_D("is_receiving_other is:%d", target_ota_status->is_receiving_other);
}

void GSoundTargetOtaGetResumeInfo(GSoundOtaResumeInfo *target_resume_info)
{
    gsound_custom_ota_get_resume_info(target_resume_info);
}

GSoundStatus GSoundTargetOtaInit(const GSoundOtaInterface *handler,
                                 GSoundOtaConfiguration *ota_device_config)
{
    ota_device_config->ota_device_count = GSOUND_OTA_DEVICE_COUNT;
    ota_device_config->supports_automatic_ota = SUPPORT_AUTOMATIC_OTA;
    GLOG_D("%s device_cnt:%d, auto_ota:%d",
           __func__,
           ota_device_config->ota_device_count,
           ota_device_config->supports_automatic_ota);

    return GSOUND_STATUS_OK;
}
