#pragma once
#ifndef __SMF_DEMUXER_OPUS_H__
#define __SMF_DEMUXER_OPUS_H__

#include "smf_demuxer.h"

/* register opus demuxer,support file "*.opus".
 * @see libsmfpluginopus.a
 */
EXTERNC void smf_opus_demuxer_register();

#endif
