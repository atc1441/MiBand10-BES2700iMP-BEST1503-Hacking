/**
 * @file gsound_custom_ota.h
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

#ifndef __GSOUND_CUSTOM_OTA_H__
#define __GSOUND_CUSTOM_OTA_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "gsound_target.h"
#include "ota_common.h"

#ifdef IBRT
#include "app_tws_ibrt_cmd_handler.h"
#endif

#ifdef GSOUND_HOTWORD_ENABLED
#include "gsound_target_hotword_common.h"
#endif

/******************************macro defination*****************************/

/******************************type defination******************************/
/**
 * @brief structure to pack the BISTO OTA specific info for command OTA_BEGIN.
 * 
 */
typedef struct
{
    /// OTA device index, @see GSoundOtaDeviceIndex to get more info
    GSoundOtaDeviceIndex index;
} GSOTA_BEGIN_PARAM_T;

/**
 * @brief structure to pack the BISTO OTA specific info for command OTA_DATA.
 * 
 */
typedef struct
{
    /// current OTA status, @see GSoundOtaDataResult to get more details
    GSoundOtaDataResult *status;

    /// OTA device index, @see GSoundOtaDeviceIndex to get more info
    GSoundOtaDeviceIndex index;
} GSOTA_DATA_PARAM_T;

/**
 * @brief structure to pack the BISTO OTA specific info for command OTA_APPLY.
 * 
 */
typedef struct
{
    /// OTA device index, @see GSoundOtaDeviceIndex to get more info
    GSoundOtaDeviceIndex index;
} GSOTA_APPLY_PARAM_T;

/**
 * @brief structure to pack the BISTO OTA specific info for command OTA_ABORT.
 * 
 */
typedef struct
{
    /// OTA device index, @see GSoundOtaDeviceIndex to get more info
    GSoundOtaDeviceIndex index;
} GSOTA_ABORT_PARAM_T;

/**
 * @brief The OTA handling data structure
 *
 */
typedef struct 
{
    /// pointer of common OTA env
    OTA_COMMON_ENV_T *common;

    /// gsound OTA device
    GSoundOtaDeviceIndex device;

    // used to determain if image should be accept by both tws sides on tws system
    // in some cases(e.g. hotword model update), it's not necessary to keep both 
    // master and slave receive update image
    bool syncTws;

} GSOTA_ENV_T;

/****************************function declearation**************************/
/**
 * @brief Judge if OTA module is currently receiving data from other user.
 * 
 * This function is used to judge if the OTA module is receiving upgrade data
 * from other user instead of BISTO.
 * 
 * @return true         OTA is receiving data from other user
 * @return false        OTA is not receiving data from other user
 */
bool gsound_custom_ota_is_receiving_other(void);

/**
 * @brief OTA_BEGIN command handler.
 * 
 * handle OTA_BEGIN command comes from libgsound and return the handling result
 * in libgsound format.
 * 
 * @param settings      OTA_BEGIN settings come from libgsound.
 * @return GSoundStatus OTA_BEGIN command handling result.
 */
GSoundStatus gsound_custom_ota_begin(const GSoundOtaSessionSettings *settings);

/**
 * @brief OTA_DATA command handler.
 * 
 * handle OTA_DATA command comes from libgsound and return the handling result
 * in libgsound format.
 * 
 * @param data          pointer of upgrade data.
 * @param length        length of upgrade data passed in.
 * @param offset        offset in whole image of the data passed in.
 * @param status        current staus of OTA progress @see GSoundOtaDataResult
 *                      to get more info.
 * @param index         device index to upgrade, @see GSoundOtaDeviceIndex to
 *                      get more info.
 * @return GSoundStatus OTA_DATA command handling result.
 */
GSoundStatus gsound_custom_ota_data(const uint8_t *data,
                                    uint32_t length,
                                    uint32_t offset,
                                    GSoundOtaDataResult *status,
                                    GSoundOtaDeviceIndex index);

/**
 * @brief OTA_APPLY command handler.
 * 
 * handle OTA_APPLY command comes from libgsound and return the handling result
 * in libgsound format.
 * 
 * @param index         OTA device index to apply the new image, 
 *                      @see GSoundOtaDeviceIndex to get more info.
 * @param zdOta         zero-day OTA flag, this flag indicates that the device
 *                      must immediately apply the OTA before proceeding any
 *                      further, refer to Target API file for more details
 * @return GSoundStatus OTA_APPLY command handling result.
 */
GSoundStatus gsound_custom_ota_apply(GSoundOtaDeviceIndex index,
                                     bool zdOta);

/**
 * @brief OTA_ABORT command handler.
 * 
 * handle OTA_ABORT command comes from libgosund and return the handling result
 * in libgsound format.
 * 
 * @param index         OTA device index to abort the OTA progress
 * @return GSoundStatus OTA_ABORT command handling result.
 */
GSoundStatus gsound_custom_ota_abort(GSoundOtaDeviceIndex index);

/**
 * @brief Exit the OTA progress.
 * 
 * This function is used to exit the OTA progress 
 * 
 */
void gsound_custom_ota_exit(OTA_PATH_E otaPath);

/**
 * @brief Get BISTO OTA resume info from the flash.
 * 
 * @param info          pointer of data to retrieve
 */
void gsound_custom_ota_get_resume_info(GSoundOtaResumeInfo* info);

/**
 * @brief Initialize the gsound custom OTA info.
 * 
 */
void gsound_custom_ota_init_handler(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_OTA_H__ */