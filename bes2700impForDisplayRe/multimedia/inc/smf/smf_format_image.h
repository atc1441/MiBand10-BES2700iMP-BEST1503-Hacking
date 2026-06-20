#pragma once
#ifndef __SMF_FORMAT_IMAGE_H__
#define __SMF_FORMAT_IMAGE_H__

#include "smf_api.h"
#include "smf_media.h"

typedef struct smf_image_format_open_param_t {
	smf_media_t media;	
	const char* target_codec;
}smf_image_format_open_param_t;

/**
 * register image convert
 */
EXTERNC void smf_image_format_register();

#endif
