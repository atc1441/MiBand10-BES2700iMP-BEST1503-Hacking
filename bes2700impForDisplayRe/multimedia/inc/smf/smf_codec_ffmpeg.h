#pragma once

#include "smf_api.h"
#include "smf_media_def.h"

typedef struct {
	uint32_t _bitrate;
	uint8_t _vbr;
	uint8_t _profile;//ffmpeg_profile_e
	uint8_t _format; //ffmpeg_format_e	
	uint8_t _gop;//group of picture
}smf_media_ffmpeg_def_t;

typedef struct {
	uint8_t* y;
	uint8_t* u;
	uint8_t* v;
	uint32_t l;
	uint16_t sw;
	uint16_t sh;
	uint16_t dw;
	uint16_t dh;
}smf_media_yuvl_t;


typedef struct {
	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_ffmpeg_def_t ffmpeg;
}smf_media_ffmpeg_t;

typedef struct smf_ffmpeg_dec_open_param_t {
	smf_media_ffmpeg_t media;
}smf_ffmpeg_dec_open_param_t;

typedef struct smf_ffmpeg_enc_open_param_t {
	smf_media_ffmpeg_t* media;
}smf_ffmpeg_enc_open_param_t;
/**
 * register ffmpeg decoder
 */
EXTERNC void smf_ffmpeg_decoder_register(void);
EXTERNC void smf_ffmpeg_encoder_register(void);
