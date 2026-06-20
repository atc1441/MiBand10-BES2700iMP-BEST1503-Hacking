#pragma once
#ifndef __SMF_CODEC_RGB_H__
#define __SMF_CODEC_RGB_H__

#include "smf_api.h"
#include "smf_media_def.h"
enum rgb_format_e: uint8_t {
	null = 128,
	gray8,
	rgb8,
	rgb565,
	rgb888,
	rgba8888,
	argb8888,
	pal8,
};

typedef struct {
	uint8_t b:5;
	uint8_t g:6;
	uint8_t r:5;
}rgb565_t;

typedef struct {
	uint8_t b;
	uint8_t g;
	uint8_t r;
}rgb888_t;

typedef struct {
	uint8_t a;
	uint8_t b;
	uint8_t g;
	uint8_t r;
}rgba8888_t;

typedef struct {
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;
}argb8888_t;

typedef struct {
	uint16_t _paletteCount;
	rgb888_t* _palette;
}smf_media_rgb_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_video_raw_def_t raw;
	smf_media_rgb_def_t rgb;
}smf_media_rgb_t;

typedef struct smf_rgb_enc_open_param_t {
	smf_media_rgb_t media;
}smf_rgb_enc_open_param_t;

typedef struct smf_rgb_dec_open_param_t {
	smf_media_rgb_t* media;
}smf_rgb_dec_open_param_t;

/**
 * register rgb encoder
 */
EXTERNC void smf_rgb_encoder_register(void);

/**
 * register rgb decoder
 */
EXTERNC void smf_rgb_decoder_register(void);
#endif
