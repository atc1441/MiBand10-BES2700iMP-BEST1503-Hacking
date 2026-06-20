/**
 * @file app_ble_audio_audio.h
 * @author BES AI team
 * @version 0.1
 * @date 2021-12-23
 *
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

#ifndef __GAF_MOBILE_USB_AUDIO_H__
#define __GAF_MOBILE_USB_AUDIO_H__
#include "bluetooth_ble_api.h"
#include "cmsis.h"
#include "cmsis_os.h"

#if defined(__cplusplus)
extern "C" {
#endif

void gap_mobile_start_usb_audio_receiving_dma(void);
void gap_mobile_start_usb_audio_transmission_dma(void);
void gaf_mobile_usb_dma_playback_stop(void);
void gaf_mobile_usb_dma_capture_stop(void);

void gaf_mobile_usb_feed_data(uint8_t* ptrBuf, uint32_t length);
void gap_mobile_start_usb_audio_receiving_dma(void);
uint32_t gaf_mobile_usb_processing_received_data(uint8_t* ptrBuf, uint32_t length);

int gaf_mobile_usb_audio_media_stream_start_handler(void* _pStreamEnv);
int gaf_mobile_usb_audio_media_stream_stop_handler(void* _pStreamEnv);
int gaf_mobile_usb_audio_capture_start_handler(void* _pStreamEnv);
int gaf_mobile_usb_audio_capture_stop_handler(void* _pStreamEnv);


#if defined(__cplusplus)
}
#endif
#endif /* __GAF_MOBILE_USB_AUDIO_H__ */

