#pragma once
#ifndef __SMF_SERVICECMD_H__
#define __SMF_SERVICECMD_H__

typedef struct smf_servicecmd_open_param_t {
	smf_source_param_t media;
}smf_servicecmd_open_param_t;
/**
 * register speaker
 */
EXTERNC void smf_servicecmd_register(void);
#endif