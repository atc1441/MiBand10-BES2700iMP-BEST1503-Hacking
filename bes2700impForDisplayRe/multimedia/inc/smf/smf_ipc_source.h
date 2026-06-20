#pragma once
#ifndef __SMF_IPC_SOURCE_H__
#define __SMF_IPC_SOURCE_H__
#include "smf_common.h"

typedef struct smf_ipc_source_open_param_t {
	smf_source_param_t source;
}smf_ipc_source_open_param_t;

/**
 * register ipc source
 */

EXTERNC void smf_ipc_source_register(void);
#endif