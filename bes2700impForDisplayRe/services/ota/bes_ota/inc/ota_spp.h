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
#ifndef __APP_SPP_OTA_H__
#define __APP_SPP_OTA_H__

#include "stdint.h"
#include "stddef.h"
#include "bluetooth_bt_api.h"

#define OTA_SPP_MAX_PACKET_SIZE     BT_SPP_MAX_TX_MTU
#define OTA_SPP_MAX_PACKET_NUM      3

#define OTA_SPP_RECV_BUFFER_SIZE   (OTA_SPP_MAX_PACKET_SIZE*6)

#define OTA_SPP_TX_BUF_SIZE         (OTA_SPP_MAX_PACKET_SIZE*OTA_SPP_MAX_PACKET_NUM)

#define APP_OTA_DATA_CMD_TIME_OUT_IN_MS	5000

typedef struct {
    bt_spp_channel_t  *ota_spp_dev;
    bool permissionToApply;
    bool isConnected;
} OtaContext;

void app_ota_spp_send_data(uint8_t *ptrData, uint16_t length);
void app_ota_spp_init(void);
void ota_disconnect(void);

#endif

