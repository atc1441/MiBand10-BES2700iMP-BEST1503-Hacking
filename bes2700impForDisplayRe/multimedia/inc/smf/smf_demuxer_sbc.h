#pragma once
#ifndef __SMF_DEMUXER_SBC_H__
#define __SMF_DEMUXER_SBC_H__
#include "smf_demuxer.h"


/* register lc3 demuxer,support file "*.sbc".
 * @see libsmfpluginsbc.a
 */
EXTERNC void smf_sbc_demuxer_register();

/* register lc3 demuxer,support file "*.msbc".
 * @see libsmfpluginsbc.a
 */
EXTERNC void smf_msbc_demuxer_register();

/* register lc3 demuxer,support file "*.gsbc".
 * @see libsmfpluginsbc.a
 */
EXTERNC void smf_gsbc_demuxer_register();

#endif
