#pragma once
#ifndef __SMF_CODEC_APE_H__
#define __SMF_CODEC_APE_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint8_t _sampleBits;
}smf_media_ape_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_ape_def_t ape;
}smf_media_ape_t;

typedef struct ape_enc_open_param_t {
	smf_media_ape_t media;
}smf_ape_enc_open_param_t;

typedef struct ape_dec_open_param_t {
	smf_media_ape_t* media;
}smf_ape_dec_open_param_t;

/**
 * register ape encoder
 */
EXTERNC void smf_ape_encoder_register(void);

/**
 * register ape decoder
 */
EXTERNC void smf_ape_decoder_register(void);
#endif
