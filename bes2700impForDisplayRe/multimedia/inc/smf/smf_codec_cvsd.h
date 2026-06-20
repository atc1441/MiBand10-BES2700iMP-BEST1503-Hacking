#pragma once
#ifndef __SMF_CODEC_CVSD_H__
#define __SMF_CODEC_CVSD_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {

}smf_media_cvsd_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_cvsd_def_t cvsd;
}smf_media_cvsd_t;

typedef struct smf_cvsd_enc_open_param_s {
	smf_media_cvsd_t media;
}smf_cvsd_enc_open_param_t;

typedef struct smf_cvsd_dec_open_param_s {
	smf_media_cvsd_t* media;
}smf_cvsd_dec_open_param_t;

///
/**
 * register cvsd decoder
 */
EXTERNC void smf_cvsd_decoder_register(void);
/**
 * register cvsd encoder
 */
EXTERNC void smf_cvsd_encoder_register(void);
//
#endif
