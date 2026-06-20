#pragma once
#include "smf_api.h"

typedef struct smf_vad_open_param_s {
	int rate;
	int channels;
	int frame_ms;
	int vad_mode;
	int sample_bits;
	void* other;
}smf_vad_open_param_t;

/**
 * register vad processer
 */
EXTERNC void smf_vad_register();
