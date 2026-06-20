#pragma once
#ifndef __SMF_CODEC_LDAC_H__
#define __SMF_CODEC_LDAC_H__

#include "smf_api.h"
#include "smf_media_def.h"
typedef struct {
	uint32_t _bitrate;
	uint32_t _bandrate;
	uint8_t _frame_dms;
}smf_media_ldac_def_t;

typedef struct {

	smf_media_def_t head;
	smf_media_audio_def_t audio;
	smf_media_ldac_def_t ldac;
}smf_media_ldac_t;

enum ldac_eqmid_e {
	LDAC_EQMID_HQ = 0,
	LDAC_EQMID_SQ = 1,
	LDAC_EQMID_MQ = 2,
	LDAC_EQMID_NUM = 3,     /* terminator */
	LDAC_EQMID_ABR = 0x7F
};

enum ldac_channel_mode_e {
	LDAC_CHANNEL_MODE_MONO = 0x04,
	LDAC_CHANNEL_MODE_DUAL_CHANNEL = 0x02,
	LDAC_CHANNEL_MODE_STEREO = 0x01
};

typedef struct ldac_enc_open_param_t {
	smf_media_ldac_t media;
}smf_ldac_enc_open_param_t;

typedef struct ldac_dec_open_param_t {
	smf_media_ldac_t* media;
}smf_ldac_dec_open_param_t;

typedef struct {
	uint8_t vendor_id0;   /* octet 0 */
	uint8_t vendor_id1;   /* octet 1 */
	uint8_t vendor_id2;   /* octet 2 */
	uint8_t vendor_id3;   /* octet 3 */
	uint8_t codec_id0;    /* octet 4 */
	uint8_t codec_id1;    /* octet 5 */

	uint8_t fs_info:6;    /* octet 6 */
	uint8_t rfa0:2;       /* octet 6 */

	uint8_t cm:3;         /* octet 7 */
	uint8_t rfa1:5;       /* octet 7 */
}smf_ldac_param_header_t;

/**
 * register silk encoder
 */
EXTERNC void smf_ldac_encoder_register();

/**
 * register silk decoder
 */
EXTERNC void smf_ldac_decoder_register();
#endif
