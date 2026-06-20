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
#ifndef __BES_SOURCE_API_H__
#define __BES_SOURCE_API_H__
#include "source_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

#ifdef BT_SOURCE

void bes_bt_source_start_a2dp_stream(uint8_t device_id);

void bes_bt_source_suspend_a2dp_stream(uint8_t device_id);

void bes_bt_source_toggle_a2dp_stream(uint8_t device_id);

uint32_t bes_bt_source_write_a2dp_data(bool packet_type, uint8_t encoded_type, uint8_t * pcm_buf, uint32_t len, uint16_t frame_len);

uint32_t bes_bt_source_write_a2dp_pcm_data(uint8_t * pcm_buf, uint32_t len);

void bes_bt_source_reconnect_hfp_profile(const bt_bdaddr_t *remote);

void bes_bt_source_reconnect_a2dp_profile(const bt_bdaddr_t *remote);

void bes_bt_source_reconnect_avrcp_profile(const bt_bdaddr_t *remote);

#if defined(BT_HID_DEVICE)
void bes_bt_source_reconnect_hid_profile(const bt_bdaddr_t *remote);
#endif // BT_HID_DEVICE

void bes_bt_source_search_device(void);

void bes_bt_source_register_bt_callback(bt_source_callback_t cb);

void bes_bt_source_a2dp_stream_buffer_init();

void bes_bt_source_a2dp_stream_buffer_deinit();

#endif /* BT_SOURCE */

#ifdef __cplusplus
}
#endif
#endif /* __BES_SOURCE_API_H__ */
