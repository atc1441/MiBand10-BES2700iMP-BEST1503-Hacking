#pragma once
#ifndef __SMF_CODEC_FLAC_H__
#define __SMF_CODEC_FLAC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint8_t _sampleBits;
}smf_media_flac_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_flac_def_t flac;
}smf_media_flac_t;

typedef struct smf_flac_enc_open_param_t {
	smf_media_flac_t media;
}smf_flac_enc_open_param_t;

typedef struct smf_flac_dec_open_param_t {
	smf_media_flac_t* media;
}smf_flac_dec_open_param_t;

/**
 * register aac encoder
 */
EXTERNC void smf_flac_encoder_register(void);

/**
 * register aac decoder
 */
EXTERNC void smf_flac_decoder_register(void);

#endif
