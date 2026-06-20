#pragma once
#ifndef __SMF_DEMUXER_MP3_H__
#define __SMF_DEMUXER_MP3_H__

#include "smf_demuxer.h"

/* register mp3 demuxer,support file "*.mp3".
 * @see libsmfpluginmp3.a
 */
EXTERNC void smf_mp3_demuxer_register();

#endif
