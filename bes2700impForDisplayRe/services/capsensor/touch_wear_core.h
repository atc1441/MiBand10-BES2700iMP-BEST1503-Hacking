/***************************************************************************
 * Copyright 2022-2023 BES.
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
 ***************************************************************************/
#ifndef __TOUCH_WEAR_CORE_H__
#define __TOUCH_WEAR_CORE_H__
#include "stdint.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*capsensor_click_event_func_type)(uint8_t key_event);

/* register_capsensor_click_event_callback */
int register_capsensor_click_event_callback(capsensor_click_event_func_type p);

/* sensor hub send data to mcu */
void app_sensor_hub_core_send_touch_req_no_rsp(uint8_t *ptr, uint16_t len);

/* cap_sensor_core_thread_init */
void cap_sensor_core_thread_init(void);

#ifdef __cplusplus
}
#endif

#endif /* __TOUCH_WEAR_CORE_H__ */
