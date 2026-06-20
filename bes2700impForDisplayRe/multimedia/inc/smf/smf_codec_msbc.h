#pragma once
#ifndef __SMF_CODEC_mSBC_H__
#define __SMF_CODEC_mSBC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_msbc_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_msbc_def_t msbc;
}smf_media_msbc_t;

typedef struct smf_msbc_enc_open_param_t {
	smf_media_msbc_t media;
} smf_msbc_enc_open_param_t;

typedef struct smf_msbc_dec_open_param_t {
	smf_media_msbc_t* media;
	//
	bool plc;
}smf_msbc_dec_open_param_t;
///
/**
 * register msbc decoder
 */
EXTERNC void smf_msbc_decoder_register(void);

/**
 * register msbc encoder
 */
EXTERNC void smf_msbc_encoder_register(void);
///
#endif
