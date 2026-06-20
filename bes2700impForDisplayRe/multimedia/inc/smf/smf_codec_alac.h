#pragma once
#ifndef __SMF_CODEC_ALAC_H__
#define __SMF_CODEC_ALAC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint8_t _sampleBits;
	uint32_t _max_sample_per_frame;
	uint32_t _max_coded_frame_size;
}smf_media_alac_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_alac_def_t alac;
}smf_media_alac_t;

typedef struct {
	smf_media_alac_t media;
}smf_alac_enc_open_param_t;

typedef struct {
	smf_media_alac_t* media;
	//uint32_t bitrate;
	//uint32_t max_sample_per_frame;
	//uint8_t sample_size;
	//uint8_t rice_history_mult;
	//uint8_t rice_initial_history;
	//uint8_t rice_kmodifier;
	//uint32_t max_coded_frame_size;
}smf_alac_dec_open_param_t;

/**
 * register alac encoder
 */
EXTERNC void smf_alac_encoder_register(void);

/**
 * register alac decoder
 */
EXTERNC void smf_alac_decoder_register(void);
#endif
