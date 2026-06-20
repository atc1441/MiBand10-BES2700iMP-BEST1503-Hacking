#pragma once

#include "smf_api.h"
#include "smf_media_def.h"

enum h265_profile_e {
	H265_Baseline = 1,
	H265_Main,
	H265_High,
};

enum h265_format_e {
	H265_IIII = 1,
	H265_IPPP,
	H265_IBBP,
};

enum h265_stream_e {
	H265_Annexb = 0,
	H265_Avcc,
	H265_Raw
};
typedef struct {
	uint32_t _bitrate;
	uint8_t _vbr;
	uint8_t _profile;//h265_profile_e
	uint8_t _format; //h265_format_e
	uint8_t _gop;//group of picture
	uint8_t _streamType;
}smf_media_h265_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_h265_def_t h265;
}smf_media_h265_t;

typedef struct smf_h265_dec_open_param_t {
	smf_media_h265_t media;
}smf_h265_dec_open_param_t;

typedef struct smf_h265_enc_open_param_t {
	smf_media_h265_t* media;
}smf_h265_enc_open_param_t;
/**
 * register h265 decoder
 */
EXTERNC void smf_h265_decoder_register(void);
//EXTERNC void smf_h265_encoder_register(void);
EXTERNC void smf_h265_demuxer_register(void);

