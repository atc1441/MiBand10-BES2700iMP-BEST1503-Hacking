#pragma once
#ifndef __SMF_CODEC_GIF_H__
#define __SMF_CODEC_GIF_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_gif_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_gif_def_t gif;
}smf_media_gif_t;

typedef struct smf_gif_enc_open_param_t {
	smf_media_gif_t media;
}smf_gif_enc_open_param_t;

typedef struct smf_gif_dec_open_param_t {
	smf_media_gif_t media;
}smf_gif_dec_open_param_t;

/**
 * register gif encoder
 */
EXTERNC void smf_gif_encoder_register(void);

/**
 * register gif decoder
 */
EXTERNC void smf_gif_decoder_register(void);
#endif
