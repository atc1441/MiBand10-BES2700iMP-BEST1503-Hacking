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
#ifndef __A2DP_CODEC_SBC_H__
#define __A2DP_CODEC_SBC_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include "a2dp_api.h"

extern btif_avdtp_codec_t a2dp_avdtpcodec;
extern const unsigned char a2dp_codec_elements[];
bt_status_t a2dp_codec_sbc_init(int index);
void a2dp_codec_sbc_common_init(void);

#if defined(CUSTOM_BITRATE) || defined(BTIF_DIP_DEVICE)
uint8_t a2dp_avdtpcodec_sbc_user_bitpool_get();
void a2dp_avdtpcodec_sbc_user_configure_set(uint32_t bitpool,uint8_t user_configure);
#endif
#if defined(__cplusplus)
}
#endif

#endif /* __A2DP_CODEC_SBC_H__ */