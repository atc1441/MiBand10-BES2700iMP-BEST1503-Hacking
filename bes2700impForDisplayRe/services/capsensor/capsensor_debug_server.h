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
#ifndef __CAPSENSOR_DEBUG_SERVER_H__
#define __CAPSENSOR_DEBUG_SERVER_H__
#include "stdint.h"

#include "capsensor_driver.h"

#ifdef __cplusplus
extern "C" {
#endif

/* app_spp capsensor_transport debug */
void app_spp_capsensor_server(void);

/* switch capsensor_transport  */
void capsensor_check_mem_data_flag_set(uint8_t* data);

/* data capsensor  */
void capsensor_check_mem_data(struct capsensor_sample_data * data, int len, uint8_t m_touch_event, int16_t* m_touch_pos);

#ifdef __cplusplus
}
#endif

#endif /* __CAPSENSOR_DEBUG_SERVER_H__ */