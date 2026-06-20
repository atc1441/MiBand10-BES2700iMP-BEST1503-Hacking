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

#ifndef __AUDIO_CODEC_API_H__
#define __AUDIO_CODEC_API_H__
#include "plat_types.h"

// Encoder APIs Declaration
void audio_bt_sbc_init_encoder(void *Encoder, void *info);

int audio_bt_sbc_encode_frames(void *Encoder, void *PcmData, void *SbcData);

int audio_bt_sbc_encoder_get_stream_info(void *Encoder, void *info);

int audio_bt_sbc_encoder_set_stream_info(void *Encoder, void *info);

// Decoder APIs Declaration


// System APIs Declaration
void audio_bt_update_latency_buff_size(uint16_t targetMs);

void audio_bt_update_system_frequency(void);

#endif /*__AUDIO_CODEC_IPC_API_H__ */

