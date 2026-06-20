#pragma once
#ifndef __SMF_CODEC_SSC_H__
#define __SMF_CODEC_SSC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
}smf_media_ssc_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_ssc_def_t ssc;
}smf_media_ssc_t;
#if 0
enum smf_ssc_set_param_e{
	SMF_AAC_SET_LOW_SPEED = 0x0f00,
	SMF_AAC_SET_HIGH_SPEED = 0x0f01,
};

enum smf_ssc_get_param_e{
	SMF_AAC_SET_LOW_SPEED = 0x1f00,
	SMF_AAC_SET_HIGH_SPEED = 0x1f01,
};
#endif

enum smf_ssc_decode_channel_e{
	SMF_SSC_CHANNEL_LEFT = 0,
	SMF_SSC_CHANNEL_RIGHT = 1,
	SMF_SSC_CHANNEL_BOTH = 2,
};

typedef struct {
	smf_media_ssc_t* media;
	//int sbm_on_off;
	//smf_ssc_decode_channel_e output_channels;
#if 0
	int low_speed;
	int high_speed;
	int sbm_num_normal;
#endif
}smf_ssc_dec_open_param_t;

/**
 * register ssc decoder
 */
EXTERNC void smf_ssc_decoder_register(void);

#endif
