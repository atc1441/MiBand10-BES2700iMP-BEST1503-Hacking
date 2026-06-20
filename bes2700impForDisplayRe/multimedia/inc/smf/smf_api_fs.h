#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

EXTERNC void smf_fs_register(void);

EXTERNC void smf_file_buffer_register(const char* filename, void* data, int size);

typedef void* (*CbFOpen)(const char* __filename, const char* __modes);
typedef int (*CbFClose)(void* __stream);
typedef size_t(*CbFRead)(void* __ptr, size_t __size, size_t __n, void* __stream);
typedef size_t(*CbFWrite)(const void* ptr, size_t size, size_t nmemb, void* __stream);
typedef int (*CbFSeek)(void* __stream, long int __off, int __whence);
typedef long (*CbFTell)(void* __stream);
typedef long (*CbFLength)(void* __stream);
EXTERNC void smf_fs_callback_register(
	CbFOpen fopen_
	, CbFClose fclose_
	, CbFRead fread_
	, CbFWrite fwrite_
	, CbFSeek fseek_
	, CbFTell ftell_
	, CbFLength flength
);

