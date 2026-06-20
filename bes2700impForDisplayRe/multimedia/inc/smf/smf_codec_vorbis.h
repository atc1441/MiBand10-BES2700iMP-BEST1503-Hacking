#pragma once
#ifndef __SMF_CODEC_OGG_H__
#define __SMF_CODEC_OGG_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint32_t _bandWidth;
	uint32_t _bitrate;
	uint8_t _complexity;
	uint8_t _encmode;
	uint8_t _vbr;
}smf_media_vorbis_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_vorbis_def_t vorbis;
}smf_media_vorbis_t;


enum encmode_s{
	SMF_VBR = 0,
	SMF_ABR = 1,
};

typedef struct smf_vorbis_enc_open_param_s {
	smf_media_vorbis_t media;
}smf_vorbis_enc_open_param_t;

typedef struct vorbis_dec_open_param_s {
	smf_media_vorbis_t media;
}smf_vorbis_dec_open_param_t;


/**
 * register vorbis encoder
 */
EXTERNC void smf_vorbis_encoder_register(void);

/**
 * register vorbis decoder
 */
EXTERNC void smf_vorbis_decoder_register(void);
#endif
