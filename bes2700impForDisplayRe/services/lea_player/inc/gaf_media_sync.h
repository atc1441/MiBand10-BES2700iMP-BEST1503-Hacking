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
/**
 ****************************************************************************************
 * @addtogroup APP_BAP
 * @{
 ****************************************************************************************
 */

#ifndef GAF_MEDIA_SYNC_H_
#define GAF_MEDIA_SYNC_H_
#if BLE_AUDIO_ENABLED


#ifdef __cplusplus
//extern "C" {
#endif

void gaf_media_prepare_playback_trigger(uint8_t trigger_channel);
void gaf_media_prepare_capture_trigger(uint8_t trigger_channel);
uint32_t gaf_media_sync_get_curr_time(void);   //uinit -- us
uint32_t gaf_media_sync_get_curr_bt_clk_in_us(void);

#ifdef __cplusplus
//}
#endif

#endif
#endif // GAF_MEDIA_SYNC_H_

/// @} APP_BAP
