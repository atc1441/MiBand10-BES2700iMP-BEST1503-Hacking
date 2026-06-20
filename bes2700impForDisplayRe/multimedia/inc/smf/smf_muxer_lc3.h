#pragma once
#ifndef __SMF_MUXER_LC3_H__
#define __SMF_MUXER_LC3_H__

#include "smf_muxer.h"

/* register lc3 demuxer,support file "*.lc3".
 * @see libsmfpluginlc3.a
 */
EXTERNC void smf_lc3_muxer_register();
EXTERNC void smf_lc3plus_muxer_register();

#endif
