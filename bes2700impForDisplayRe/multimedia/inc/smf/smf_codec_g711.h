#pragma once
#ifndef __SMF_CODEC_G711_H__
#define __SMF_CODEC_G711_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_g711_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_g711_def_t g711;
}smf_media_g711_t;

typedef struct {
	smf_media_g711_t media;
}smf_g711_enc_open_param_t;

typedef struct {
	smf_media_g711_t* media;
}smf_g711_dec_open_param_t;

/**
 * register g711 encoder
 */
EXTERNC void smf_g711_encoder_register(void);

/**
 * register g711 decoder
 */
EXTERNC void smf_g711_decoder_register(void);
#endif
