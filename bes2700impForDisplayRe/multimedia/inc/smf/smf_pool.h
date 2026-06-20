#pragma once
#include <stdint.h>
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC void* smf_dbg_alloc(void* pool, uint32_t size, const char* file, int line);
EXTERNC void* smf_dbg_realloc(void* pool, void* ptr, uint32_t size, uint32_t lastsize, const char* file, int line);
EXTERNC void* smf_dbg_calloc(void* pool, uint32_t num, uint32_t size, const char* file, int line);
EXTERNC void  smf_dbg_free(void* pool, void* ptr, const char* file, int line);

EXTERNC void* smf_mem_alloc(void* pool, uint32_t size);
EXTERNC void* smf_mem_realloc(void* pool, void* ptr, uint32_t size, uint32_t lastsize);
EXTERNC void* smf_mem_calloc(void* pool, uint32_t num, uint32_t size);
EXTERNC void  smf_mem_free(void* pool, void* ptr);

EXTERNC void* smf_gbl_alloc(uint32_t size);
EXTERNC void smf_gbl_free(void* ptr);
