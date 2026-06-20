#pragma once
#include "smf_api.h"
#include "smf_common.h"
#include "smf_media.h"

typedef struct{
	smf_media_t imedia;
	smf_media_t omedia;
	uint16_t width;
	uint16_t height;
	void* other;
}smf_yun2rgb_open_param_t;

EXTERNC void smf_yuv2rgb_register();

