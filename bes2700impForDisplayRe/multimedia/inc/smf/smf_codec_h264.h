#pragma once

#include "smf_api.h"
#include "smf_media_def.h"

enum h264_profile_e {
	H264_Baseline = 66,	
	H264_Main = 77,	
	H264_Extended = 88,	
	H264_High = 100,	
	H264_High10 = 110,
	H264_High422 = 122,
	H264_High444 = 244,

	H264_BP = 66,
	H264_MP = 77,
	H264_XP = 88,
	H264_HiP = 100,
	H264_Hi10P = 110,
	H264_Hi422P = 122,
	H264_Hi444P = 244,
};

enum h264_format_e {
	IIII = 1,
	IPPP,
	IBBP,
};

enum h264_stream_e {
	H264StreamNull,
	H264StreamAnnexb,
	H264StreamAvcc,
	H264StreamRaw
};
typedef struct {
	uint32_t _bitrate;
	uint8_t _vbr;
	uint8_t _profile;//h264_profile_e
	uint8_t _level;
	uint8_t _format; //h264_format_e
	uint8_t _gop;//group of picture
	uint8_t _streamType;
	uint8_t _sps_len;
	uint8_t _pps_len;
	uint8_t* _sps;
	uint8_t* _pps;
}smf_media_h264_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_video_def_t video;
	smf_media_h264_def_t h264;
}smf_media_h264_t;

typedef struct smf_h264_dec_open_param_t {
	smf_media_h264_t media;
}smf_h264_dec_open_param_t;

typedef struct smf_h264_enc_open_param_t {
	smf_media_h264_t* media;
}smf_h264_enc_open_param_t;
/**
 * register h264 decoder
 */
EXTERNC void smf_h264_decoder_register(void);
EXTERNC void smf_h264_encoder_register(void);
EXTERNC void smf_h264_demuxer_register(void);


//typedef struct {
//}smf_media_h264_def_t;
