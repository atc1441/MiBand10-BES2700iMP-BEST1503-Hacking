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
#ifndef __APP_AUDIO_CALL_MEDIATOR_H__
#define __APP_AUDIO_CALL_MEDIATOR_H__

void app_audio_bt_switch_to_le_call_check(uint8_t device_id);

void app_audio_le_switch_to_bt_call_check(uint8_t device_id);

void app_audio_bt_ringtone_request(uint8_t device_id);

void app_audio_ble_audio_ringtone_request(uint8_t device_id,uint8_t ase_lid);

void app_audio_clear_le_call_state_by_device_id(uint8_t device_id);

#endif /* __APP_AUDIO_CALL_MEDIATOR_H__ */

