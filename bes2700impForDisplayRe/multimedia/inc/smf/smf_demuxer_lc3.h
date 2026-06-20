#pragma once
#ifndef __SMF_DEMUXER_LC3_H__
#define __SMF_DEMUXER_LC3_H__

#include "smf_demuxer.h"

/* register lc3 demuxer,support file "*.lc3".
 * @see libsmfpluginlc3.a
 */
EXTERNC void smf_lc3_demuxer_register();
EXTERNC void smf_lc3plus_demuxer_register();

#endif
