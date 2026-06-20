/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#ifndef __APP_BLE_USB_STREAM_STM_H__
#define __APP_BLE_USB_STREAM_STM_H__
#ifdef BLE_USB_AUDIO_SUPPORT

#include "ble_audio_define.h"
#ifdef __cplusplus
extern "C"{
#endif

typedef enum
{
    USB_REQ_STREAM_PLAYBACK_START,
    USB_REQ_STREAM_CAPTURE_START,
    USB_REQ_STREAM_PLAYBACK_STOP,
    USB_REQ_STREAM_CAPTURE_STOP,

    EVT_PLAYBACK_STREAM_STARTED,
    EVT_CAPTURE_STREAM_STARTED,

    STREAM_EVENT_MAX,
} BLE_USB_STREAM_EVENT_E;

typedef enum
{
    BLE_USB_STREAM_STATE_IDLE,
    BLE_USB_STREAM_STATE_PLAYBACK_STARTED,
    BLE_USB_STREAM_STATE_CAPTURE_STARTED,
    BLE_USB_STREAM_STATE_BOTH_STARTED,

    BLE_USB_STREAM_STATE_UNKNOW,
} BLE_USB_STREAM_STATE_E;

typedef struct
{
    /* ase hsm define */
    Hsm super;

    State idle;
    State playback_start;
    State capture_start;
    State both_start;

    bool used;
} usb_stream_stm_t;

typedef struct
{
    void *p_pend_stm;
    BLE_USB_STREAM_EVENT_E op;
} USB_STREAM_PENDING_INFO_T;

void ble_usb_stream_resume_ble_audio(uint8_t con_lid);

void ble_usb_stream_send_message(usb_stream_stm_t *p_stream_sm, BLE_USB_STREAM_EVENT_E event, uint32_t param0, uint32_t param1);
usb_stream_stm_t* ble_usb_get_stream_sm(void);
BLE_USB_STREAM_STATE_E ble_usb_stream_stm_get_cur_state(void);

void ble_usb_stream_stm_startup(usb_stream_stm_t *ble_usb_stream_sm);
void ble_usb_stream_stm_init(void);

#ifdef __cplusplus
}
#endif
#endif

#endif

