#pragma once
#ifndef __SMF_COMMON_H__
#define __SMF_COMMON_H__
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif

#ifdef WIN32
#define WEAK
#else
#define WEAK __attribute__((weak))
#endif

//#ifdef __cplusplus
//#if __cplusplus < 201103L
//#define override
//#define final
//#endif
//#endif
typedef enum smf_direction_e {
	smf_direction_upward=0,
	smf_direction_downward,
	smf_direction_forward,
	smf_direction_backward,
}smf_direction_e;

typedef struct smf_message_t {
	uint64_t id;
	void* creater;
	void* laster;
	uint32_t data[4];
	//
	uint16_t sub;
	bool is_processed : 1;
	bool is_notify : 1;
	uint8_t direction : 2;//smf_direction_e
	uint32_t flags : 12;
	void* priv;
}smf_message_t;
typedef bool(*cb_message_t)(smf_message_t* msg);

typedef struct smf_progress_t {
	uint32_t start;
	uint32_t current;
	uint32_t end;
}smf_progress_t;
//
typedef struct {
	const char* url;
	const char* title;
	const char* artist;
	const char* album;
	uint32_t duration;
}smf_meta_info_t;
//
typedef struct {
	uint32_t playtime;
	uint32_t frameindex;
	uint32_t position;
}smf_stream_status_t;

typedef struct smf_buffer_t {
	uint16_t max;
	uint16_t offset;
	uint16_t size;
	uint8_t falgs;
	uint8_t refs;
	void* buff;
}smf_buffer_t;
//
typedef struct {
	void* data;
	unsigned size;
}smf_pair_t;

typedef struct {
	uint16_t size;
	void* data;
}smf_pair16_t;
//
typedef struct {
	void* func;
	void* priv;
}smf_callback_t;
//
typedef struct {
	unsigned keys;
	void* data;
	unsigned size;
	void* priv;
	void (*uninit)(void*priv);
	void* other;
}smf_param_t;
//
typedef struct {
	unsigned keys;
	unsigned vals;
}smf_keys_value_t;
//
typedef union {
	int8_t i8[4];
	int16_t i16[2];
	int32_t i32;
	uint8_t u8[4];
	uint16_t u16[2];
	uint32_t u32;
}smf_int8x4_t;
//
typedef union {
	int8_t  i8[8];
	int16_t i16[4];
	int32_t i32[2];
	int64_t i64;
	uint8_t u8[8];
	uint16_t u16[4];
	uint32_t u32[2];
	uint64_t u64;
}smf_int8x8_t;

typedef struct smf_fifo_t {
	void* data;
	uint32_t max;
	uint64_t wi;
	uint64_t ri;
}smf_fifo_t, smf_ring_t;

typedef struct {
	uint8_t* begin;
	uint8_t* end;
	uint8_t* ptr;
	uint8_t* ptr_end;
}smf_shared_pool_t;

typedef struct {
	void* ptr;
	uint32_t size;
	int oft;
	uint32_t where;
}smf_io_read_para_t, smf_io_write_para_t;
#if 0
typedef struct {
	int* frameMax;
	int* frameMin;
	void** frame;
	void** media;
	smf_stream_status_t* status;
}smf_elememt_data_t;
#endif
typedef void (*smf_hook_t)(void* data, uint32_t size, void* priv);

typedef struct {
	int num;
	int den;
}smf_fraction_t;

typedef struct {
	uint32_t keys;
	void* data;
	uint32_t size;
	union {
		uint32_t flags;
		struct {
			bool enable : 1;
			bool is_static : 1;
			bool no_cache : 1;
			bool pass_val : 1;
			bool is_save : 1;
		};
	};
}smf_params_t;

typedef struct {
	uint32_t freq_mhz;
	int avg_mips;
	int max_mips;
	int min_mips;
}smf_mips_t;

typedef struct {
	void* data;
	uint32_t size;
}smf_wakeup_t;

typedef struct {
	union {
		uint64_t halName;
		uint8_t halId;
	};
	uint16_t blksize;
	uint8_t blknum;
	uint8_t blknumStart;
	uint32_t chmap;
	uint16_t fsCycles;
	uint8_t slotCycles;
	struct {
		bool master : 1;
		bool setId : 1;
		uint8_t rev0 : 6;
	};
	uint32_t rev[3];
}smf_dma_param_t; // 32byte

#endif
