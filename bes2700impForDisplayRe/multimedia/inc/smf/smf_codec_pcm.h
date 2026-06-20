#pragma once
#ifndef __SMF_CODEC_PCM_H__
#define __SMF_CODEC_PCM_H__

#include "smf_media_def.h"
typedef struct {
	uint8_t _sampleBits;
	uint8_t _sampleWidth;
	bool _isNonInterlace;
	bool _isUnSigned;
	bool _isBigEndian;
	bool _isFloat;
}smf_media_pcm_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_pcm_def_t pcm;
}smf_media_pcm_t;

typedef struct smf_pcm_enc_open_param_t {
	smf_media_pcm_t media;
}smf_pcm_enc_open_param_t;

typedef struct smf_pcm_dec_open_param_t {
	smf_media_pcm_t media;
}smf_pcm_dec_open_param_t;

/**
 * register pcm decoder
 */
EXTERNC void smf_pcm_decoder_register(void);

/**
 * register pcm encoder
 */
EXTERNC void smf_pcm_encoder_register(void);

/**
 * register adpcm decoder
 */
EXTERNC void smf_adpcm_decoder_register(void);

/**
 * register adpcm encoder
 */
EXTERNC void smf_adpcm_encoder_register(void);
#endif