#pragma once
#ifndef __SMF_CODEC_G722_H__
#define __SMF_CODEC_G722_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_g722_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_g722_def_t g722;
}smf_media_g722_t;

typedef struct {
	smf_media_g722_t media;
}smf_g722_enc_open_param_t;

typedef struct {
	smf_media_g722_t* media;
}smf_g722_dec_open_param_t;

/**
 * register g722 encoder
 */
EXTERNC void smf_g722_encoder_register(void);

/**
 * register g722 decoder
 */
EXTERNC void smf_g722_decoder_register(void);
#endif
