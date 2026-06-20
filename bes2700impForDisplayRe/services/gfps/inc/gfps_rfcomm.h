/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifndef __GFPS_RFCOMM_H__
#define __GFPS_RFCOMM_H__
#if GFPS_ENABLED

typedef struct
{
    uint8_t    devId;
    uint8_t    isConnected;
    uint8_t    isRfcommInitialized;
    bt_spp_channel_t *spp_chan;
} fpRfcommEnv_t;

typedef struct
{
    fpRfcommEnv_t fp_rfcomm_service[BT_DEVICE_NUM];
} fpRfcommSrvEnv_t;


#ifdef __cplusplus
extern "C" {
#endif

bool app_fp_rfcomm_send(uint8_t device_id, uint8_t *ptrData, uint32_t length);

bt_status_t app_fp_rfcomm_init(void);

void app_fp_disconnect_rfcomm(uint8_t device_id);

uint16_t app_fp_rfcomm_get_data_len(uint8_t devId);

void app_fp_rfcomm_data_done(uint8_t devId, uint16_t consumeLen, uint8_t *buf, uint16_t *len);


#ifdef __cplusplus
}
#endif
#endif

#endif  // __GFPS_RFCOMM_H__
