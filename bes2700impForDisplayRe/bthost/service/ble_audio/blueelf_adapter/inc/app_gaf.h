/**
 ****************************************************************************************
 *
 * @file app_gaf.h
 *
 * @brief BLE Audio Generic Audio Framework
 *
 * Copyright 2015-2019 BES.
 *
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
 ****************************************************************************************
 */

/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef APP_GAF_H_
#define APP_GAF_H_

#if BLE_AUDIO_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

#include "app_gaf_define.h"
#include "app_bap_capa_srv_msg.h"

void app_gaf_earbuds_init(app_bap_capa_srv_dir_t *sink_capa_info, app_bap_capa_srv_dir_t *src_capa_info, uint32_t role_bf);

void app_gaf_earbuds_deinit(void);

void app_gaf_mobile_init(void);

void app_gaf_mobile_deinit(void);

/**
 * @brief Report event to up-layer
 *
 * @param evt           Event ID, @see app_gaf_evt_e to get more info
 * @param param         param_description
 * @param length        param_description
 */
void app_gaf_evt_report(uint16_t evt, uint8_t *param, uint32_t length);

#ifdef AOB_MOBILE_ENABLED
/**
 * @brief Report event of mobile side to up-layer
 *
 * @param evt           Event ID, @see app_gaf_mobile_evt_e to get more detailed info
 * @param param         Pointer of the event info
 * @param length        Length of the event info
 */
void app_gaf_mobile_evt_report(uint16_t evt, uint8_t *param, uint32_t length);
#endif

void *app_gaf_malloc_buff(uint16_t size);

void app_gaf_free_buff(void *mem_ptr);

#ifdef __cplusplus
}
#endif

#endif
#endif // APP_GAF_H_

/// @} APP_GAF
