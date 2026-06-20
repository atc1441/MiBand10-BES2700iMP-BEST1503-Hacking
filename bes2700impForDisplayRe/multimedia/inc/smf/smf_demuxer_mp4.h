#pragma once
#ifndef __SMF_DEMUXER_MP4_H__
#define __SMF_DEMUXER_MP4_H__

#include "smf_demuxer.h"

/* register mp4 demuxer,support file "*.mp4".
 * @see libsmfpluginmp4.a
 */
EXTERNC void smf_mp4_demuxer_register();

/* register m4a demuxer,support file "*.m4a".
 * @see libsmfpluginmp4.a
 */
EXTERNC void smf_m4a_demuxer_register();
EXTERNC void smf_m4v_demuxer_register();
#endif
