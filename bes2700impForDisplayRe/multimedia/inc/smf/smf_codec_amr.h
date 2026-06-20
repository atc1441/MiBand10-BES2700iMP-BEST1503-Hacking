#pragma once
#ifndef __SMF_CODEC_AMR_H__
#define __SMF_CODEC_AMR_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_amr_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_amr_def_t amr;
}smf_media_amr_t;

typedef struct {
	smf_media_amr_t media;
}smf_amr_enc_open_param_t;

typedef struct {
	smf_media_amr_t media;
}smf_amr_dec_open_param_t;

/**
 * register amr_nb encoder
 */
EXTERNC void smf_amr_nb_encoder_register(void);

/**
 * register amr_nb decoder
 */
EXTERNC void smf_amr_nb_decoder_register(void);

/**
 * register amr_wb encoder
 */
EXTERNC void smf_amr_wb_encoder_register(void);

/**
 * register amr_wb decoder
 */
EXTERNC void smf_amr_wb_decoder_register(void);
#endif
