#pragma once
#ifndef __SMF_CODEC_YUV_H__
#define __SMF_CODEC_YUV_H__

#include "smf_api.h"
#include "smf_media_def.h"
enum yuv_format_e: uint8_t {
	yuv = 0,

	yuvy,
	vyuy,
	uyvy,
	yuyv,
	I420,//y8-uuvv

	i420,//y8-uuvv
	u420,//y8-uuvv
	yv12,//y8-vvuu
	nv12,//y8-uvuv
	nv21,//y8-vuvu
	I422,//y4-uuvv
	yv16,//y4-vvuu
	nv61,//y4-uvuv
	nv16,//y4-vuvu
	i444,//yy-uuvv
	u444,//yy-uuvv
	yv24,//yy-vvuu
	nv24,//yy-uvuv
	nv42,//yy-vuvu	

	i420p8,//y8-uuvv
	u420p8,//y8-uuvv
	yv12p8,//y8-vvuu
	nv12p8,//y8-uvuv
	nv21p8,//y8-vuvu
	i422p8,//y4-uuvv
	yv16p8,//y4-vvuu
	nv61p8,//y4-uvuv
	nv16p8,//y4-vuvu
	i444p8,//yy-uuvv
	u444p8,//yy-uuvv
	yv24p8,//yy-vvuu
	nv24p8,//yy-uvuv
	nv42p8,//yy-vuvu	

	i420p16,//y8-uuvv
	u420p16,//y8-uuvv
	yv12p16,//y8-vvuu
	nv12p16,//y8-uvuv
	nv21p16,//y8-vuvu
	i422p16,//y4-uuvv
	yv16p16,//y4-vvuu
	nv61p16,//y4-uvuv
	nv16p16,//y4-vuvu
	i444p16,//yy-uuvv
	yv24p16,//yy-vvuu
	nv24p16,//yy-uvuv
	nv42p16,//yy-vuvu	
};

typedef struct {
	uint8_t rev;
}smf_media_yuv_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_video_raw_def_t raw;
	smf_media_yuv_def_t yuv;
}smf_media_yuv_t;

typedef struct smf_yuv_enc_open_param_t {
	smf_media_yuv_t media;
}smf_yuv_enc_open_param_t;

typedef struct smf_yuv_dec_open_param_t {
	smf_media_yuv_t* media;
}smf_yuv_dec_open_param_t;

/**
 * register yuv encoder
 */
EXTERNC void smf_yuv_encoder_register(void);

/**
 * register yuv decoder
 */
EXTERNC void smf_yuv_decoder_register(void);
#endif
