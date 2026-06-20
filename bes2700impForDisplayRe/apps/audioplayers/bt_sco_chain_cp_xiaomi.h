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
#ifndef __BT_SCO_CHAIN_CP_H__
#define __BT_SCO_CHAIN_CP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"
#include "stdbool.h"

int sco_cp_init(int frame_len, int channel_num);

int sco_cp_deinit(void);

int sco_cp_process(short *pcm_buf, short *ref_buf, int *_pcm_len, sco_status_t *status);

#ifdef __cplusplus
}
#endif

#endif

