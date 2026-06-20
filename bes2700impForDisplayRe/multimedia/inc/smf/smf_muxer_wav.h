#pragma once
#ifndef __SMF_MUXER_WAV_H__
#define __SMF_MUXER_WAV_H__

#include "smf_muxer.h"

/* register wav demuxer,support file "*.wav".
 * @see libsmfpluginwav.a
 */
EXTERNC void smf_wav_muxer_register();

#endif
