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
#include "hal_trace.h"
#include "anc_assist_resample.h"
#include "anc_assist_mic.h"
#include "integer_resampling.h"

static IntegerResamplingState *resample_st[MIC_INDEX_QTY] = {NULL, NULL, NULL, NULL};

int32_t anc_assist_resample_init(uint32_t sample_rate, uint32_t frame_len, custom_allocator *allocator)
{
	TRACE(0, "[%s] sample_rate: %d, frame_len: %d", __func__, sample_rate, frame_len);
	INTEGER_RESAMPLING_ITEM_T item = INTEGER_RESAMPLING_ITEM_32K_TO_16K;

	if (sample_rate == 32000) {
		item = INTEGER_RESAMPLING_ITEM_32K_TO_16K;
	} else if (sample_rate == 48000) {
		item = INTEGER_RESAMPLING_ITEM_48K_TO_16K;
	} else if (sample_rate == 96000) {
		item = INTEGER_RESAMPLING_ITEM_96K_TO_16K;
	} else {
		TRACE(0, "[%s] TODO: Don't need to do resample", __func__);
	}

	for (uint32_t ch=0; ch<MIC_INDEX_QTY; ch++) {
		if (anc_assist_mic_anc_mic_is_enabled(ch)) {
			TRACE(0, "Resample for ch %d", ch);
			resample_st[ch] = integer_resampling_create_with_custom_allocator(frame_len, 1, item, allocator);
		} else {
			resample_st[ch] = NULL;
		}
	}

	return 0;
}

int32_t anc_assist_resample_deinit(void)
{
	for (uint32_t ch=0; ch<MIC_INDEX_QTY; ch++) {
		if (resample_st[ch]) {
			integer_resampling_destroy(resample_st[ch]);
			resample_st[ch] = NULL;
		}
	}

	return 0;
}

int32_t anc_assist_resample_process(float *pcm_buf, uint32_t buf_frame_len, uint32_t valid_frame_len)
{
	for (uint32_t ch=0; ch<MIC_INDEX_QTY; ch++) {
		if (resample_st[ch]) {
			integer_resampling_process_f32(resample_st[ch], pcm_buf + ch * buf_frame_len, valid_frame_len, pcm_buf + ch * buf_frame_len);
		}
	}

	return 0;
}