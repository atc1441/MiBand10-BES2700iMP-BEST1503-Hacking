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
#ifndef __A2DP_ENCODER_SBC_CP_H__
#define __A2DP_ENCODER_SBC_CP_H__

#include <stdio.h>
#include <stdbool.h>
#include "app_source_codec.h"
//#include "a2dp_codec_sbc.h"

#ifdef __cplusplus
extern "C" {
#endif

int a2dp_encode_lhdc_cp_init(void);
bool a2dp_source_cp_encode_lhdc_packet(a2dp_source_packet_t *source_packet);
void a2dp_encode_lhdc_cp_deinit(void);
#ifdef __cplusplus
}
#endif

#endif /* __A2DP_ENCODER_SBC_CP_H__ */