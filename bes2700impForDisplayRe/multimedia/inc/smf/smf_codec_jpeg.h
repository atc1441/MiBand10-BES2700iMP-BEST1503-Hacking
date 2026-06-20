#pragma once
#ifndef __SMF_CODEC_JPEG_H__
#define __SMF_CODEC_JPEG_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint32_t _bitrate;
}smf_media_jpeg_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_jpeg_def_t jpeg;
}smf_media_jpeg_t;

typedef struct smf_jpeg_enc_open_param_t {
	smf_media_jpeg_t media;
}smf_jpeg_enc_open_param_t;

typedef struct smf_jpeg_dec_open_param_t {
	smf_media_jpeg_t* media;
}smf_jpeg_dec_open_param_t;

/**
 * register jpeg encoder
 */
EXTERNC void smf_jpeg_encoder_register(void);

/**
 * register jpeg decoder
 */
EXTERNC void smf_jpeg_decoder_register(void);
#endif
