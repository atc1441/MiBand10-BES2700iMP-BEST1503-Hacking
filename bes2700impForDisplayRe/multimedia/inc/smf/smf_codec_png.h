#pragma once
#ifndef __SMF_CODEC_PNG_H__
#define __SMF_CODEC_PNG_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint32_t _bitrate;
}smf_media_png_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_png_def_t png;
}smf_media_png_t;

typedef struct smf_png_enc_open_param_t {
	smf_media_png_t media;
}smf_png_enc_open_param_t;

typedef struct smf_png_dec_open_param_t {
	smf_media_png_t* media;
}smf_png_dec_open_param_t;

/**
 * register png encoder
 */
EXTERNC void smf_png_encoder_register(void);

/**
 * register png decoder
 */
EXTERNC void smf_png_decoder_register(void);
#endif
