#pragma once
#ifndef __SMF_CODEC_RLE_H__
#define __SMF_CODEC_RLE_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_rle_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_rle_def_t rle;
}smf_media_rle_t;

typedef struct smf_rle_enc_open_param_t {
	smf_media_rle_t media;
}smf_rle_enc_open_param_t;

typedef struct smf_rle_dec_open_param_t {
	smf_media_rle_t* media;
}smf_rle_dec_open_param_t;

/**
 * register rle encoder
 */
EXTERNC void smf_rle_encoder_register(void);

/**
 * register rle decoder
 */
EXTERNC void smf_rle_decoder_register(void);
#endif
