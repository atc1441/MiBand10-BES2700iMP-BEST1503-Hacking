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
#ifndef GAF_STREAM_PROCESS_H_
#define GAF_STREAM_PROCESS_H_

#include "plat_types.h"
#include "audioflinger.h"
#include "gaf_media_common.h"

#ifdef __cplusplus
extern "C" {
#endif

bool gaf_stream_process_context_is_call(uint16_t context);
bool gaf_stream_process_context_is_binaural_record(uint16_t context);

int32_t gaf_stream_process_init(void);
int32_t gaf_stream_process_deinit(void);

uint32_t gaf_stream_process_need_capture_buf_size(uint16_t stream);
int32_t gaf_stream_process_set_capture_buf(uint8_t *buf, uint32_t size);
int32_t gaf_stream_process_capture_open(uint16_t stream, struct AF_STREAM_CONFIG_T *stream_cfg);
int32_t gaf_stream_process_capture_close(void);
uint32_t gaf_stream_process_capture_run(uint8_t *buf, uint32_t len);

uint32_t gaf_stream_process_need_playback_buf_size(uint16_t stream);
int32_t gaf_stream_process_set_playback_buf(uint8_t *buf, uint32_t size);
int32_t gaf_stream_process_playback_open(uint16_t stream, struct AF_STREAM_CONFIG_T *stream_cfg);
int32_t gaf_stream_process_playback_close(void);
uint32_t gaf_stream_process_playback_run(uint8_t *buf, uint32_t len);

uint32_t gaf_stream_process_need_upsampling_buf_size(uint16_t stream);
int32_t gaf_stream_process_set_upsampling_buf(uint8_t *buf, uint32_t size);
uint8_t *gaf_stream_process_get_upsampling_buf(void);
int32_t gaf_stream_process_capture_upsampling_init(GAF_AUDIO_STREAM_INFO_T *stream_info, struct AF_STREAM_CONFIG_T *stream_cfg);
int32_t gaf_stream_process_capture_upsampling_deinit(void);
uint32_t gaf_stream_process_capture_upsampling_run(uint8_t *buf, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif // GAF_STREAM_PROCESS_H_
