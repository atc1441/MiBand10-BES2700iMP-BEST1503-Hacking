/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#ifndef __APP_KARAOKE_H__
#define __APP_KARAOKE_H__

#ifdef __cplusplus
extern "C" {
#endif

uint32_t app_karaoke_need_buf_size(void);

int32_t app_karaoke_set_buf(uint8_t *buf, uint32_t size);

int32_t app_karaoke_start(bool on);

#ifdef __cplusplus
}
#endif
#endif // __APP_KARAOKE_H__
