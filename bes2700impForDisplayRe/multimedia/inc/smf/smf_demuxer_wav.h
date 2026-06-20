#pragma once
#ifndef __SMF_DEMUXER_WAV_H__
#define __SMF_DEMUXER_WAV_H__

#include "smf_demuxer.h"

/* register wav demuxer,support file "*.wav".
 * @see libsmfpluginwav.a
 */
EXTERNC void smf_wav_demuxer_register();

#endif
