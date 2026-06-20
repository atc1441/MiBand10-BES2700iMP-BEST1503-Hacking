#pragma once
#ifndef __SMF_API_OS_H__
#define __SMF_API_OS_H__
#include "smf_common.h"

EXTERNC void smf_register_thread_operators(
	int (*get_id)(void)
	,void (*yield)(void)
	,void (*waitfor)(int ms)
);

#endif
