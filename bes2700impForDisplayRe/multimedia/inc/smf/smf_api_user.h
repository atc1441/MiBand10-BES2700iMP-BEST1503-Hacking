#pragma once
#include <stdint.h>

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC void dbgPrintfs(const char* name, unsigned n, ...);
#define smf_dbg_printf(fd,...) dbgPrintfs("test", 7, 0, 0, __FILE__, __LINE__, __FUNCTION__, 0x1200, ##__VA_ARGS__)
//
EXTERNC void* smf_pool_malloc(void* pool, int size);
EXTERNC void* smf_pool_realloc(void* pool, void* ptr, int size);
EXTERNC void* smf_pool_calloc(void* pool, int count, int size);
EXTERNC void smf_pool_free(void* pool, void* ptr);
//
EXTERNC void* smf_io_open(const char* fname, const char* para);
EXTERNC int smf_io_read(void* io, void* buff, int size);
EXTERNC int smf_io_write(void* io, void* buff, int size);
EXTERNC int smf_io_seek(void* io, int offset, int origin);
EXTERNC void smf_io_close(void* io);
EXTERNC int smf_io_tell(void* io);

