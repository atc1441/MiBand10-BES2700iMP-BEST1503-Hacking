#pragma once

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC void smf_statistics_reset();
EXTERNC void smf_statistics_clear();
EXTERNC int smf_statistics_get_index(const char* func, const char* file, unsigned line, const char* other);
EXTERNC unsigned smf_statistics_get_time();
EXTERNC void smf_statistics_tmp(void* p);
EXTERNC void smf_statistics_start(int idx);
EXTERNC void smf_statistics_stop(int idx);
EXTERNC void smf_statistics_print();
