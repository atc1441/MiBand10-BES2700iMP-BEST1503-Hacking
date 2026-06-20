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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

/*****************************header include********************************/

#ifndef __AOB_SERVICE_SYNC_H__
#define __AOB_SERVICE_SYNC_H__

#ifdef __cplusplus
extern "C" {
#endif

bool app_ble_audio_support_sync_service(void);

bool app_ble_audio_send_service_data(const bt_bdaddr_t *remote_addr, uint32_t svc_uuid, void *data);

void app_ble_audio_recv_service_data(uint8_t *p_buff, uint16_t len);


//void app_ble_audio_share_send_svc_unit_test();

//void app_ble_audio_share_send_svc_rsp_unit_test(uint8_t *p_buff, uint16_t len);


#ifdef __cplusplus
}
#endif


#endif /* __AOB_SERVICE_SYNC_H__ */
