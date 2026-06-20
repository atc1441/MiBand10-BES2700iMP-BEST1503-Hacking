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
#ifndef __APP_IBRT_A2DP__
#define __APP_IBRT_A2DP__

#ifdef __cplusplus
extern "C" {
#endif

a2dp_stream_t *app_bt_get_mobile_a2dp_stream(uint32_t deviceId);
int a2dp_ibrt_sync_get_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);
int a2dp_ibrt_sync_set_status(uint8_t device_id, ibrt_a2dp_status_t *a2dp_status);
int a2dp_ibrt_sync_get_status_v2(uint8_t device_id, ibrt_mobile_info_t *p_mobile_info, ibrt_a2dp_status_t *a2dp_status);
int a2dp_ibrt_session_reset(uint8_t devId);
int a2dp_ibrt_session_new(uint8_t devId);
uint32_t a2dp_ibrt_session_get(uint8_t devId);
int a2dp_ibrt_session_set(uint8_t session,uint8_t devId);
int a2dp_ibrt_stream_need_autotrigger_set_flag(void);

#ifdef __cplusplus
}
#endif
#endif
