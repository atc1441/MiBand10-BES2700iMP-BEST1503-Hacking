/**
 * @copyright Copyright (c) 2015-2021 BES Technic.
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


#ifndef __GAF_MOBILE_MEDIA_STREAM_H__
#define __GAF_MOBILE_MEDIA_STREAM_H__

#ifdef AOB_MOBILE_ENABLED
/*****************************header include********************************/
#include "gaf_media_common.h"
#include "gaf_media_pid.h"

/******************************macro defination*****************************/
#define GAF_MOB_MAXIMUM_CONNECTION_COUNT            (2)

#define GAF_MOBILE_AUDIO_STREAM_CAPTURE_CHANNEL_NUM      AUD_CHANNEL_NUM_2

#if defined (BLE_AUDIO_USE_TWO_MIC_SRC_FOR_DONGLE) || defined (BLE_AUDIO_STEREO_CHAN_OVER_CIS_CNT)
#define GAF_MOBILE_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM     AUD_CHANNEL_NUM_2
#else
#define GAF_MOBILE_AUDIO_STREAM_PLAYBACK_CHANNEL_NUM     AUD_CHANNEL_NUM_1
#endif

#ifdef __BLE_AUDIO_24BIT__
#define GAF_MOBILE_AUDIO_STREAM_BIT_NUM            AUD_BITS_24
#else
#define GAF_MOBILE_AUDIO_STREAM_BIT_NUM            AUD_BITS_16
#endif

/*
* Note: Temporary solution on mobile phone
* 1. The timestamp received by the phone is always less than the current bt time.
* 2. Lost packet
*/
#define GAF_MOBILE_TEMP_SOLUTION_ENABLE

#define GAF_MOBILE_AUDIO_MAX_DIFF_BT_TIME   (4000)

#ifdef BLE_USB_AUDIO_SUPPORT
extern const GAF_MEDIA_STREAM_TYPE_OPERATION_RULE_T gaf_cis_mobile_stream_type_op_rule;
#endif

/******************************type defination******************************/

/****************************function declaration***************************/
#ifdef __cplusplus
extern "C"{
#endif

void gaf_mobile_audio_stream_update_and_start_handler(uint8_t ase_lid, uint8_t con_lid);
void gaf_mobile_audio_stream_update_and_stop_handler(uint8_t ase_lid, uint8_t con_lid);

void gaf_mobile_audio_clear_playback_buf_list(GAF_AUDIO_STREAM_ENV_T* _pStreamEnv);

gaf_media_data_t *gaf_mobile_audio_get_packet(GAF_AUDIO_STREAM_ENV_T* pStreamEnv,
    uint32_t dmaIrqHappeningTimeUs, uint8_t cisChannel);

void gaf_mobile_audio_stream_init(void);
void gaf_mobile_audio_receive_data(uint16_t conhdl, GAF_ISO_PKT_STATUS_E pkt_status);

#ifdef __cplusplus
}
#endif

#endif
#endif /* #ifndef __GAF_MOBILE_MEDIA_STREAM_H__ */
