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

#ifndef __APP_BLE_USB_LATENCY_TEST_H__
#define __APP_BLE_USB_LATENCY_TEST_H__

void app_ble_usb_latency_test_reset();
uint8_t* app_ble_usb_latency_get_lc3_payload(float dur_frame);
void app_ble_usb_latency_signal_init();
void app_ble_usb_latency_send_data_signal();
void app_ble_usb_latency_playback_signal(void);

#endif /* __APP_BLE_USB_LATENCY_TEST_H__ */

