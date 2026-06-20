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

#ifndef __AUDIO_MANAGER_H__
#define __AUDIO_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

void amgr_init();
void amgr_set_bt_sco_ringtone_mode(bool en);
bool amgr_is_bt_sco_ringtone_mode(void);
//uint8_t amgr_is_bluetooth_sco_on();
void amgr_set_bluetooth_sco_on(uint8_t sco_on);
void amgr_set_bt_a2dp_is_on(bool a2dp_on);
bool amgr_is_bt_a2dp_on(void);
#ifdef __cplusplus
    }
#endif


#endif /* __AUDIO_MANAGER_H__  */
