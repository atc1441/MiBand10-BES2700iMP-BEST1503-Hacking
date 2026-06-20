#pragma once
#include <stdint.h>

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC bool smf_load_dll(const char* name, bool entry);
EXTERNC bool smf_load_dll_id(uint32_t id, bool entry);
EXTERNC bool smf_unload_dll(const char* name, bool exit);
EXTERNC bool smf_unload_dll_id(uint32_t id, bool exit);
EXTERNC bool smf_entry_dll(const char* name, void* para);
EXTERNC bool smf_entry_dll_id(uint32_t id, void* para);
EXTERNC bool smf_exit_dll(const char* name, void* para);
EXTERNC bool smf_exit_dll_id(uint32_t id, void* para);