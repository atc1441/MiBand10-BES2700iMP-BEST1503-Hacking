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
#ifndef __AUDIO_DUMP_H__
#define __AUDIO_DUMP_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void audio_dump_init(int frame_len, int sample_bytes, int channel_num);
void audio_dump_deinit(void);
void audio_dump_clear_up(void);
void audio_dump_add_channel_data_from_multi_channels(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index);
void audio_dump_add_channel_data_from_multi_channels_32bit_to_16bit(int channel_id, void *pcm_buf, int pcm_len, int channel_num, int channel_index, int right_shift);
void audio_dump_add_channel_data(int channel_id, void *pcm_buf, int pcm_len);
void audio_dump_run(void);
void audio_dump_get_buff_info(uint8_t **buf, uint32_t *len);

int audio_dump_buffer_mutex_init(void);
int audio_dump_buffer_mutex_lock(void);
int audio_dump_buffer_mutex_unlock(void);

void data_dump_init(void);
void data_dump_deinit(void);
void data_dump_run(uint32_t ch, const char *str, void *data_buf, uint32_t data_len);

#ifdef __cplusplus
}
#endif

#endif