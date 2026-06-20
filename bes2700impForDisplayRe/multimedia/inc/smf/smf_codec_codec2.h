#pragma once
#ifndef __SMF_CODEC_CODEC2_H__
#define __SMF_CODEC_CODEC2_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_codec2_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_codec2_def_t codec2;
}smf_media_codec2_t;

typedef struct {
	smf_media_codec2_t media;
}smf_codec2_enc_open_param_t;

typedef struct {
	smf_media_codec2_t* media;
}smf_codec2_dec_open_param_t;

/**
 * register codec2 encoder
 */
EXTERNC void smf_codec2_encoder_register(void);

/**
 * register codec2 decoder
 */
EXTERNC void smf_codec2_decoder_register(void);
#endif
