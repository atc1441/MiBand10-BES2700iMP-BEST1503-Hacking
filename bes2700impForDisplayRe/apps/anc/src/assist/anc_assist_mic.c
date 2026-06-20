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
#include "anc_assist_mic.h"
#include "hal_codec.h"
#include "hal_trace.h"
#include "tgt_hardware.h"

#ifndef ANC_FF_MIC_CH_L
#define ANC_FF_MIC_CH_L			(0xFF)
#endif

#ifndef ANC_FF_MIC_CH_R
#define ANC_FF_MIC_CH_R			(0xFF)
#endif

#ifndef ANC_FB_MIC_CH_L
#define ANC_FB_MIC_CH_L			(0xFF)
#endif

#ifndef ANC_FB_MIC_CH_R
#define ANC_FB_MIC_CH_R			(0xFF)
#endif

#ifndef ANC_TALK_MIC_CH_L
#define ANC_TALK_MIC_CH_L		(0xFF)
#endif

#ifndef ANC_TALK_MIC_CH_R
#define ANC_TALK_MIC_CH_R		(0xFF)
#endif

#ifndef ANC_REF_MIC_CH_L
#define ANC_REF_MIC_CH_L		(0xFF)
#endif

#ifndef ANC_REF_MIC_CH_R
#define ANC_REF_MIC_CH_R		(0xFF)
#endif

#ifndef ANC_SPK_CH
#define ANC_SPK_CH				(AUD_CHANNEL_MAP_CH0)
#endif

static uint32_t anc_mic_cfg		= 0;
static uint32_t app_mic_cfg 	= 0;
static uint32_t all_mic_cfg 	= 0;
static uint32_t anc_mic_ch_num 	= 0;
static uint32_t app_mic_ch_num	= 0;
static uint32_t all_mic_ch_num 	= 0;

static uint32_t ff_l_mic_index	= MIC_INDEX_QTY;
static uint32_t ff_r_mic_index  = MIC_INDEX_QTY;
static uint32_t fb_l_mic_index	= MIC_INDEX_QTY;
static uint32_t fb_r_mic_index  = MIC_INDEX_QTY;
static uint32_t talk_l_mic_index = MIC_INDEX_QTY;
static uint32_t talk_r_mic_index = MIC_INDEX_QTY;
static uint32_t ref_l_mic_index  = MIC_INDEX_QTY;
static uint32_t ref_r_mic_index  = MIC_INDEX_QTY;

static uint32_t codec_dev_get_mic_cfg(enum AUD_IO_PATH_T path)
{
    if (path == AUD_IO_PATH_NULL) {
        return 0;
    } else {
        return hal_codec_get_input_path_cfg(path) & AUD_CHANNEL_MAP_ALL;
    }
}

static uint32_t codec_dev_get_mic_path_ch_num(enum AUD_IO_PATH_T path)
{
    return hal_codec_get_input_path_chan_num(path);
}

static uint32_t codec_dev_get_mic_map_ch_num(uint32_t ch_map)
{
    return hal_codec_get_input_map_chan_num(ch_map);
}

int32_t anc_assist_mic_reset(void)
{
	STATIC_ASSERT(ANC_FF_MIC_CH_L 	!= 0xFF, "Need define ANC_FF_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FF_MIC_CH_R 	!= 0xFF, "Need define ANC_FF_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FB_MIC_CH_L 	!= 0xFF, "Need define ANC_FB_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_FB_MIC_CH_R 	!= 0xFF, "Need define ANC_FB_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_TALK_MIC_CH_L != 0xFF, "Need define ANC_TALK_MIC_CH_L in tgt_hardware.h");
	STATIC_ASSERT(ANC_TALK_MIC_CH_R != 0xFF, "Need define ANC_TALK_MIC_CH_R in tgt_hardware.h");
	STATIC_ASSERT(ANC_REF_MIC_CH_L  != 0xFF, "Need define ANC_REF_MIC_CH_L  in tgt_hardware.h");
	STATIC_ASSERT(ANC_REF_MIC_CH_R  != 0xFF, "Need define ANC_REF_MIC_CH_R  in tgt_hardware.h");

	anc_mic_cfg		= codec_dev_get_mic_cfg(AUD_INPUT_PATH_ANC_ASSIST);
	app_mic_cfg 	= 0;

	anc_mic_ch_num 	= codec_dev_get_mic_path_ch_num(AUD_INPUT_PATH_ANC_ASSIST);
	app_mic_ch_num	= 0;

	all_mic_cfg 	= anc_mic_cfg;
	all_mic_ch_num 	= anc_mic_ch_num;

	TRACE(4, "[%s] CH MAP: 0x%x", __func__, anc_mic_cfg);
	TRACE(4, "[%s] CH NUM: %d", __func__, anc_mic_ch_num);

	// TODO: Check ANC VMIC includes other VMIC

	return 0;
}

int32_t anc_assist_mic_parser_app_buf(void *buf, uint32_t *len)
{
    ASSERT(*len % sizeof(_PCM_T) == 0, "[%s]buf_len err", __FUNCTION__);
    _PCM_T *pcm_buf = (_PCM_T *)buf;
    uint32_t pcm_len = *len / sizeof(_PCM_T);

    uint32_t in_offset = 0;
    uint32_t out_offset = 0;

    for(in_offset = 0, out_offset = 0; in_offset < pcm_len; in_offset += all_mic_ch_num, out_offset += app_mic_ch_num) {
        uint32_t all_ch_cnt = 0;
        uint32_t app_ch_cnt = 0;

        uint32_t app_cfg = app_mic_cfg;
        uint32_t all_cfg = all_mic_cfg;
        while (app_cfg) {
            if (all_cfg & 0x1) {
                if (app_cfg & 0x1) {
                    pcm_buf[out_offset + app_ch_cnt] = pcm_buf[in_offset + all_ch_cnt];
                    app_ch_cnt++;
                }
                all_ch_cnt++;
            }

        all_cfg >>= 1;
        app_cfg >>= 1;
        }
    }
    *len = (*len) / all_mic_ch_num * app_mic_ch_num;

    return 0;
}

int32_t anc_assist_mic_parser_anc_buf(bool pcm_interval, enum AUD_IO_PATH_T path, float *anc_buf, uint32_t anc_frame_len, _PCM_T *pcm_buf, uint32_t pcm_len)
{
	uint32_t frame_len = 0;
	uint32_t ch_num = 0;
	float *anc_buf_ptr = NULL;

	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		ch_num = anc_mic_ch_num;
	} else {
		ch_num = all_mic_ch_num;
	}

	frame_len = pcm_len / ch_num;

#if defined(ANC_ASSIST_UNUSED_ON_PHONE_CALL)
	float **anc_buf_tmp = (float **)anc_buf;
	for (int32_t ch = 0; ch < ch_num; ch++) {
		if (ch == ff_l_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_FF_L];
		} else if (ch == ff_r_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_FF_R];
		} else if (ch == fb_l_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_FB_L];
		} else if (ch == fb_r_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_FB_R];
		} else if (ch == talk_l_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_TALK_L];
		}else if (ch == talk_r_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_TALK_R];
		} else if (ch == ref_l_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_REF_L];
		} else if (ch == ref_r_mic_index) {
			anc_buf_ptr = anc_buf_tmp[MIC_INDEX_REF_R];
		} else {
			anc_buf_tmp = NULL;
		}
#else
	for (int32_t ch=0; ch<ch_num; ch++) {
		if (ch == ff_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FF_L;
		} else if (ch == ff_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FF_R;
		} else if (ch == fb_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FB_L;
		} else if (ch == fb_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_FB_R;
		} else if (ch == talk_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_TALK_L;
		} else if (ch == talk_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_TALK_R;
		} else if (ch == ref_l_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_REF_L;
		} else if (ch == ref_r_mic_index) {
			anc_buf_ptr = anc_buf + anc_frame_len * MIC_INDEX_REF_R;
		} else {
			anc_buf = NULL;
		}
#endif

		if (anc_buf) {
            if (pcm_interval == false) {
                for (int32_t i=0; i<frame_len; i++) {
                    anc_buf_ptr[i] = (float)(((_PCM_T *)pcm_buf)[i* ch_num + ch]);
                }
            } else {
                for (int32_t i=0; i<frame_len; i++) {
                    anc_buf_ptr[i] = ((float *)pcm_buf)[ch* frame_len + i];
                }
            }
		}
	}

	return 0;
}

int32_t anc_assist_mic_parser_index(enum AUD_IO_PATH_T path)
{
	uint32_t pos 	= 0;
	uint32_t index 	= 0;
	uint32_t mic_cfg = 0;

	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		mic_cfg = anc_mic_cfg;
	} else {
		mic_cfg = all_mic_cfg;
	}

	ff_l_mic_index	= MIC_INDEX_QTY;
	ff_r_mic_index  = MIC_INDEX_QTY;
	fb_l_mic_index	= MIC_INDEX_QTY;
	fb_r_mic_index  = MIC_INDEX_QTY;
	talk_l_mic_index = MIC_INDEX_QTY;
	talk_r_mic_index = MIC_INDEX_QTY;
	ref_l_mic_index  = MIC_INDEX_QTY;
	ref_r_mic_index  = MIC_INDEX_QTY;

	while (mic_cfg) {
		if (mic_cfg & 0x1) {
			if ((0x1 << pos) & ANC_FF_MIC_CH_L) {
				ff_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_FF_MIC_CH_R) {
				ff_r_mic_index = index;
			} else if ((0x1 << pos) & ANC_FB_MIC_CH_L) {
				fb_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_FB_MIC_CH_R) {
				fb_r_mic_index = index;
			} else if ((0x1 << pos) & ANC_TALK_MIC_CH_L) {
				talk_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_TALK_MIC_CH_R) {
				talk_r_mic_index = index;
			} else if ((0x1 << pos) & ANC_REF_MIC_CH_L) {
				ref_l_mic_index = index;
			} else if ((0x1 << pos) & ANC_REF_MIC_CH_R) {
				ref_r_mic_index = index;
			}

			index++;
		}

		pos++;
		mic_cfg >>= 1;
	}

	TRACE(7, "[%s] MIC INDEX: ff_l: %d, ff_r: %d, fb_l: %d, fb_r: %d",
		__func__, ff_l_mic_index, ff_r_mic_index, fb_l_mic_index, fb_r_mic_index );
	TRACE(7, "[%s] MIC INDEX: talk_l: %d, talk_r: %d, ref_l: %d, ref_r: %d",
		__func__, talk_l_mic_index, talk_r_mic_index, ref_l_mic_index, ref_r_mic_index);

	return 0;
}

int32_t anc_assist_mic_set_app_cfg(enum AUD_IO_PATH_T path)
{

    app_mic_cfg = codec_dev_get_mic_cfg(path);
    app_mic_ch_num = codec_dev_get_mic_map_ch_num(app_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: app: %d, all: %d", __func__, app_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: app: 0x%x, all: 0x%x", __func__, app_mic_cfg, all_mic_cfg);

	return 0;
}

int32_t anc_assist_mic_set_anc_cfg(enum AUD_IO_PATH_T path)
{
	ASSERT(path == AUD_INPUT_PATH_ANC_ASSIST, "[%s] path: %d", __func__, path);

	anc_mic_cfg = codec_dev_get_mic_cfg(path);
	anc_mic_ch_num = codec_dev_get_mic_map_ch_num(anc_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: anc: %d, all: %d", __func__, anc_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: anc: 0x%x, all: 0x%x", __func__, anc_mic_cfg, all_mic_cfg);

	return 0;
}

uint32_t anc_assist_mic_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	uint32_t mic_cfg = 0;

	if (cfg->ff_howling_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
#if defined(FREEMAN_ENABLED_STERO)
		mic_cfg |= ANC_FF_MIC_CH_R;
#endif
	}
	if (thirdparty_cfg->en) {
        mic_cfg |= 0;
	}

	if (cfg->fb_howling_en) {
		mic_cfg |= ANC_FB_MIC_CH_L;
#if defined(FREEMAN_ENABLED_STERO)
		mic_cfg |= ANC_FB_MIC_CH_R;
#endif
	}

	if (cfg->noise_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	if (cfg->noise_classify_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	if (cfg->wind_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
#if !defined(VOICE_ASSIST_WIND_SINGLE_MIC)
#if defined(FREEMAN_ENABLED_STERO)
		mic_cfg |= ANC_FF_MIC_CH_R;
#else
		mic_cfg |= ANC_TALK_MIC_CH_L;
#endif
#endif
	}

	if (cfg->wind_single_mic_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	if (cfg->pilot_en || cfg->ultrasound_en) {
		mic_cfg |= ANC_FB_MIC_CH_L | ANC_REF_MIC_CH_L;
#if defined(FREEMAN_ENABLED_STERO)
		mic_cfg |= ANC_FB_MIC_CH_R;
#endif
	}

	if (cfg->wsd_en) {
		mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L;
	}

	if (cfg->extern_kws_en) {
		mic_cfg |= ANC_TALK_MIC_CH_L; //NOTE: Add customer mic cfg
	}

	if(cfg->prompt_adaptive_en) {
		mic_cfg |= ANC_FB_MIC_CH_L | ANC_FF_MIC_CH_L | ANC_REF_MIC_CH_L;
#if defined(FREEMAN_ENABLED_STERO)
		mic_cfg |= ANC_FB_MIC_CH_R | ANC_FF_MIC_CH_R;
#endif
	}

    if (cfg->fir_lms_en) {
        mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L | ANC_TALK_MIC_CH_L | ANC_REF_MIC_CH_L | ANC_REF_MIC_CH_R;
#if defined(FREEMAN_ENABLED_STERO)
        mic_cfg |= ANC_FF_MIC_CH_R | ANC_FB_MIC_CH_R | ANC_TALK_MIC_CH_R;
#endif
    }

	if (cfg->ada_iir_en) {
        mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L; // | ANC_TALK_MIC_CH_L | ANC_REF_MIC_CH_L | ANC_REF_MIC_CH_R;
	}

	if(cfg->optimal_tf_en) {
		mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L | ANC_REF_MIC_CH_L;
	}

	if(cfg->pnc_en) {
		mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L;
	}

	if(cfg->fir_anc_open_leak_en) {
		mic_cfg |= ANC_FF_MIC_CH_L;
	}

	// Workaround for phone call and record
	// mic_cfg |= ANC_FF_MIC_CH_L | ANC_FB_MIC_CH_L | ANC_TALK_MIC_CH_L | ANC_REF_MIC_CH_L | ANC_REF_MIC_CH_R;
	return mic_cfg;
}

uint32_t anc_assist_spk_parser_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	uint32_t spk_cfg = 0;

	if ((cfg->pilot_en && (cfg->pilot_cfg.wd_en || cfg->pilot_cfg.adaptive_anc_en)) || cfg->ultrasound_en) {
		spk_cfg |= ANC_SPK_CH;
	}

	if (thirdparty_cfg->en) {
		spk_cfg |= 0;
	}
	return spk_cfg;
}

int32_t anc_assist_mic_update_anc_cfg(AncAssistConfig *cfg, ThirdpartyAssistConfig *thirdparty_cfg)
{
	anc_mic_cfg = anc_assist_mic_parser_anc_cfg(cfg, thirdparty_cfg);
	anc_mic_ch_num = codec_dev_get_mic_map_ch_num(anc_mic_cfg);

	// Update all mic cfg and channel number
	all_mic_cfg = anc_mic_cfg | app_mic_cfg;
	all_mic_ch_num = codec_dev_get_mic_map_ch_num(all_mic_cfg);

	TRACE(3, "[%s] CH NUM: anc: %d, all: %d", __func__, anc_mic_ch_num, all_mic_ch_num);
	TRACE(3, "[%s] CH MAP: anc: 0x%x, all: 0x%x", __func__, anc_mic_cfg, all_mic_cfg);

	return 0;
}

uint32_t anc_assist_mic_get_cfg(enum AUD_IO_PATH_T path)
{
	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		return anc_mic_cfg;
	} else {
		return all_mic_cfg;
	}
}

uint32_t anc_assist_mic_get_ch_num(enum AUD_IO_PATH_T path)
{
	if (path == AUD_INPUT_PATH_ANC_ASSIST) {
		return anc_mic_ch_num;
	} else {
		return all_mic_ch_num;
	}
}

uint32_t anc_assist_mic_anc_mic_is_enabled(anc_assist_mic_index_t index)
{
	if ((index == MIC_INDEX_FF_L) && (anc_mic_cfg & ANC_FF_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_FF_R) && (anc_mic_cfg & ANC_FF_MIC_CH_R)) {
		return true;
	} else if ((index == MIC_INDEX_FB_L) && (anc_mic_cfg & ANC_FB_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_FB_R) && (anc_mic_cfg & ANC_FB_MIC_CH_R)) {
		return true;
	} else if ((index == MIC_INDEX_TALK_L) && (anc_mic_cfg & ANC_TALK_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_TALK_R) && (anc_mic_cfg & ANC_TALK_MIC_CH_R)) {
		return true;
	} else if ((index == MIC_INDEX_REF_L) && (anc_mic_cfg & ANC_REF_MIC_CH_L)) {
		return true;
	} else if ((index == MIC_INDEX_REF_R) && (anc_mic_cfg & ANC_REF_MIC_CH_R)) {
		return true;
	} else {
		return false;
	}
}