#pragma once
#ifndef __SMF_MEDIA_H__
#define __SMF_MEDIA_H__
#include "smf_media_def.h"
#include "smf_codec_aac.h"
#include "smf_codec_alac.h"
#include "smf_codec_amr.h"
#include "smf_codec_ape.h"
#include "smf_codec_cvsd.h"
#include "smf_codec_codec2.h"
#include "smf_codec_flac.h"
#include "smf_codec_g711.h"
#include "smf_codec_g722.h"
#include "smf_codec_gif.h"
#include "smf_codec_h264.h"
#include "smf_codec_h265.h"
#include "smf_codec_jpeg.h"
#include "smf_codec_lc3.h"
#include "smf_codec_ldac.h"
#include "smf_codec_mp3.h"
#include "smf_codec_opus.h"
#include "smf_codec_pcm.h"
#include "smf_codec_png.h"
#include "smf_codec_rgb.h"
#include "smf_codec_sbc.h"
#include "smf_codec_silk.h"
#include "smf_codec_ssc.h"
#include "smf_codec_vorbis.h"
#include "smf_codec_yuv.h"
//
typedef struct smf_media_t {
	smf_media_def_t head;
	union {
		struct {
			smf_media_audio_def_t audio;
			union {			
				smf_media_pcm_def_t pcm;
				smf_media_g711_def_t g711;
				smf_media_g722_def_t g722;
				
				smf_media_flac_def_t flac;
				smf_media_ape_def_t ape;
				smf_media_alac_def_t alac;
				
				smf_media_aac_def_t aac;
				smf_media_mp3_def_t mp3;
				smf_media_opus_def_t opus;
				smf_media_vorbis_def_t vorbis;
				
				smf_media_lc3_def_t lc3;
				smf_media_sbc_def_t sbc;
				//smf_media_msbc_def_t msbc;
				//smf_media_gsbc_def_t gsbc;
				smf_media_cvsd_def_t cvsd;
				smf_media_ldac_def_t ldac;
				smf_media_ssc_def_t ssc;
				
				smf_media_silk_def_t silk;
				smf_media_amr_def_t amr;
				smf_media_codec2_def_t codec2;
			};
		};
		struct {
			smf_media_video_def_t video;
			union {
				smf_media_rgb_def_t rgb;
				smf_media_yuv_def_t yuv;
				smf_media_jpeg_def_t jpeg;
				smf_media_gif_def_t gif;
				smf_media_png_def_t png;
				smf_media_h264_def_t h264;
				smf_media_h264_def_t h265;
			};
		};
	};
}smf_media_t;

#endif

