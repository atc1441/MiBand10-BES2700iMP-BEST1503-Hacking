#pragma once
#ifndef __SMF_PROCESS_WALKIE_TALKIE_H__
#define __SMF_PROCESS_WALKIE_TALKIE_H__

#include "smf_api.h"

typedef struct smf_walkie_talkie_open_param_s {
	int rate;
	int channels;
    int frame_sample;
	void* other;
}smf_walkie_talkie_open_param_t;

/**
 * register walkie_talkie processer
 */
EXTERNC void smf_walkietalkie_upload_register();
EXTERNC void smf_walkietalkie_download_register();

#endif
