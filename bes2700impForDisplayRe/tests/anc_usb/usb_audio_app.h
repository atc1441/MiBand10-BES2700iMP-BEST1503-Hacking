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
#ifndef __USB_AUDIO_APP_H__
#define __USB_AUDIO_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"
#include "hal_key.h"
#include "audio_process.h"
#ifdef DONGLE_SUPPORT
#include CHIP_SPECIFIC_HDR(dongle)
#endif

struct USB_AUDIO_BUF_CFG {
    uint8_t *play_buf;
    uint32_t play_size;
    uint8_t *cap_buf;
    uint32_t cap_size;
    uint8_t *recv_buf;
    uint32_t recv_size;
    uint8_t *send_buf;
    uint32_t send_size;
    uint8_t *eq_buf;
    uint32_t eq_size;
    uint8_t *resample_buf;
    uint32_t resample_size;
#ifdef USB_AUDIO_MULTIFUNC
    uint8_t *recv2_buf;
    uint32_t recv2_size;
#endif
};

#define CODEC_BUFF_FRAME_NUM            (20)
#define USB_BUFF_FRAME_NUM              (CODEC_BUFF_FRAME_NUM * 2)

typedef void (*USB_AUDIO_ENQUEUE_CMD_CALLBACK)(uint32_t data);

void usb_audio_app(bool on);
void usb_audio_keep_streams_running(bool enable);
void usb_audio_app_init(const struct USB_AUDIO_BUF_CFG *cfg);

void usb_audio_app_term(void);
int usb_audio_app_key(enum HAL_KEY_CODE_T code, enum HAL_KEY_EVENT_T event);
void usb_audio_app_loop(void);
uint32_t usb_audio_get_capture_sample_rate(void);

uint32_t usb_audio_set_eq(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t index);
uint8_t usb_audio_get_eq_index(AUDIO_EQ_TYPE_T audio_eq_type,uint8_t anc_status);

void usb_audio_set_enqueue_cmd_callback(USB_AUDIO_ENQUEUE_CMD_CALLBACK cb);

#ifdef DONGLE_SUPPORT
void usb_dongle_callback(DONGLE_NOTIFY_EVENT_E, DONGLE_INFORM_PARAM_T*);
#endif

typedef void (*USB_AUDIO_SOURCE_INIT_CALLBACK)(void);
typedef void (*USB_AUDIO_SOURCE_DEINIT_CALLBACK)(void);
typedef uint32_t (*USB_AUDIO_SOURCE_DATA_PLAYBACK_CALLBACK)(uint8_t * pcm_buf, uint32_t len);
typedef uint32_t (*USB_AUDIO_SOURCE_DATA_CAPTURE_CALLBACK)(uint8_t * pcm_buf, uint32_t len);
typedef void (*USB_AUDIO_SOURCE_PLAYBACK_STREAM_START_CALLBACK)(void);
typedef void (*USB_AUDIO_SOURCE_PLAYBACK_STREAM_STOP_CALLBACK)(void);
typedef void (*USB_AUDIO_SOURCE_CAPTURE_STREAM_START_CALLBACK)(void);
typedef void (*USB_AUDIO_SOURCE_CAPTURE_STREAM_STOP_CALLBACK)(void);
typedef void (*USB_AUDIO_SOURCE_DATA_PREPARATION_CALLBACK)(enum AUD_STREAM_T stream);
typedef void (*USB_ADUIO_SOURCE_PLAYBACK_VOL_CHANGE)(uint32_t level);

struct USB_AUDIO_SOURCE_EVENT_CALLBACK_T {
    USB_AUDIO_SOURCE_INIT_CALLBACK                     init_cb;
    USB_AUDIO_SOURCE_DEINIT_CALLBACK                   deinit_cb;
    USB_AUDIO_SOURCE_DATA_PLAYBACK_CALLBACK            data_playback_cb;
    USB_AUDIO_SOURCE_DATA_CAPTURE_CALLBACK             data_capture_cb;
    USB_AUDIO_SOURCE_DATA_PREPARATION_CALLBACK         data_prep_cb;
    USB_AUDIO_SOURCE_PLAYBACK_STREAM_START_CALLBACK    playback_start_cb;
    USB_AUDIO_SOURCE_PLAYBACK_STREAM_STOP_CALLBACK     playback_stop_cb;
    USB_AUDIO_SOURCE_CAPTURE_STREAM_START_CALLBACK     capture_start_cb;
    USB_AUDIO_SOURCE_CAPTURE_STREAM_STOP_CALLBACK      capture_stop_cb;

    USB_ADUIO_SOURCE_PLAYBACK_VOL_CHANGE               playback_vol_change_cb;
};

void usb_audio_source_config_init(const struct USB_AUDIO_SOURCE_EVENT_CALLBACK_T *cb_list);

#ifdef __cplusplus
}
#endif

#endif
