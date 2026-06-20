#pragma once
#ifndef __SMF_CODEC_AAC_H__
#define __SMF_CODEC_AAC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint32_t _bandWidth;
	uint32_t _bitrate;
	uint8_t _package;
	uint8_t _aot;
	uint8_t _layout;
	uint8_t _vbr;
}smf_media_aac_def_t;

typedef struct {
	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_aac_def_t aac;
}smf_media_aac_t;

enum smf_aac_aot_e {
	SMF_AAC_AOT_LC = 2,
	SMF_AAC_AOT_HE_AAC = 5,
	SMF_AAC_AOT_HE_AAC_V2 = 29,
	SMF_AAC_AOT_LC_MPEG2 = 129,
};
enum smf_aac_package_e {
	SMF_AAC_PACKAGE_RAW=0,
	SMF_AAC_PACKAGE_ADIF=1,
	SMF_AAC_PACKAGE_ADTS=2,
	SMF_AAC_PACKAGE_MCP1=6,
	SMF_AAC_PACKAGE_MCP0=7,
	SMF_AAC_PACKAGE_LOAS=10,
};
enum smf_aac_brmode_e {
	SMF_AAC_BRMODE_CBR=0,
	SMF_AAC_BRMODE_VBR_1_VERY_LOW_BITRATE=1,
	SMF_AAC_BRMODE_VBR_2_LOW_BITRATE=2,
	SMF_AAC_BRMODE_VBR_3_MEDIUM_BITRATE=3,
	SMF_AAC_BRMODE_VBR_4_HIGH_BITRATE=4,
	SMF_AAC_BRMODE_VBR_5_VERY_HIGH_BITRATE=5,
};

typedef struct {
	smf_media_aac_t media;
	//smf_encode_param_t
	//smf_media_t media;
	//aac encode params
	//unsigned char aot;
	//unsigned char layer;
    //int vbr;
}smf_aac_enc_open_param_t;

typedef struct {
	smf_media_aac_t media;
	//smf_media_t media;
	//unsigned char aot;
	//bool is_support_sbr;
}smf_aac_dec_open_param_t;

enum smf_aac_dec_param_set_e {
	SMF_AAC_DEC_PARAM_SET_SUPPORT_SBR = 0x7000,
};
/**
 * register aac encoder
 */
EXTERNC void smf_aac_encoder_register(void);

/**
 * register aac decoder
 */
EXTERNC void smf_aac_decoder_register(void);
#endif
