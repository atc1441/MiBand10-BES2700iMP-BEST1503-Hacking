#pragma once
#ifndef __SMF_AF_H__
#define __SMF_AF_H__
#include "smf_common.h"
/**
 * register audioflinger sink
 */
EXTERNC void smf_af_sink_register(void);

/**
 * register audioflinger source
 */

EXTERNC void smf_af_source_register(void);

#endif