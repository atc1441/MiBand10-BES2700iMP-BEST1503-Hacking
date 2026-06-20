#pragma once
#ifndef __SMF_MUXER_H__
#define __SMF_MUXER_H__

#include "smf_api.h"
#include "smf_media.h"
typedef struct {
	smf_media_t* media;
	void* io;
	const char* url;
	smf_meta_info_t* meta;
	///
	void* other;
}smf_iosink_param_t,smf_muxer_param_t;

#endif
