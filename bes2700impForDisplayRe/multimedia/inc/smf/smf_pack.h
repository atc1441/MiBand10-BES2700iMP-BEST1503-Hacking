#pragma once
#include <stdint.h>
#include <stdbool.h>
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC unsigned smf_pack_crc(void* pack0);

EXTERNC bool smf_pack_crc_check(void* pack0);

EXTERNC void smf_pack_crc_fill(void* pack0);
