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
#ifndef __APP_VOICE_ASSIST_CUSTOM_LEAK_DETECT_H__
#define __APP_VOICE_ASSIST_CUSTOM_LEAK_DETECT_H__

#include "plat_types.h"

#ifdef __cplusplus
extern "C" {
#endif

enum LEAK_DETECT_STATUS
{
    APP_LEAK_DETECT_DETECT_STATUS = 0,
    APP_LEAK_DETECT_CALIB_STATUS,
    APP_LEAK_DETECT_UNKNOWN_STATUS,
};

int32_t app_voice_assist_custom_leak_detect_init(void);
int32_t app_voice_assist_custom_leak_detect_open(void);
int32_t app_voice_assist_custom_leak_detect_close(void);
void app_voice_assist_custom_leak_detect_set_working_status(int32_t status);

#ifdef CUSTOMER_APP_BOAT
uint8_t app_voice_assitant_get_status(void);
#endif


#ifdef __cplusplus
}
#endif

#endif