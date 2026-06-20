#pragma once
#ifndef __SMF_FRAME_H__
#define __SMF_FRAME_H__
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "smf_media_def.h"

typedef struct smf_frame_t {
	void* buff;
	int max;
	int offset;
	int size;
	unsigned flags;
	smf_media_def_t* media;
	unsigned index;
	unsigned timestamp;
	struct smf_frame_t* frame;
	void* port;
	void* priv;
	//void (*free)(struct smf_frame_t*);
	void* pool;
}smf_frame_t;
typedef bool (*CbFrame)(smf_frame_t**frm,void*priv);
enum {
	SMF_FRAME_PACKET_MASK = 0xff,
	SMF_FRAME_IS_FAIL = 1u << 0,//0x01 error packet
	SMF_FRAME_IS_EOS = 1u << 1,//0x02 end of scan
	SMF_FRAME_IS_FIRST = 1u << 2,//0x04 first of scan
	SMF_FRAME_IS_EMPTY = 1u << 3,
	SMF_FRAME_IS_EXTRA = 1u << 4,//0x10
	SMF_FRAME_IS_MEDIA = 1u << 5,//0x20	
	SMF_FRAME_INCOMPLETE = 1u << 6,//0x40
	SMF_FRAME_IS_MUTE = 1u << 7,//0x80
	//
	SMF_FRAME_CACHED = 1u << 8,
	SMF_FRAME_SHARED = 1u << 9,
	SMF_FRAME_PACKED = 1u << 10,
	SMF_FRAME_AUTO_FREE = 1u << 11,
	SMF_FRAME_PROCESS_MULTIFRAMES = 1u << 12,
	//
	SMF_FRAME_IS_PLC = 1u << 16,
	SMF_FRAME_IS_RESERVE = 1u << 17,
	//
	SMF_FRAME_CUST_MASK = 0xff000000,
	SMF_FRAME_CUST0 = 1u << 24,
	SMF_FRAME_CUST1 = 1u << 25,
	SMF_FRAME_CUST2 = 1u << 26,
	SMF_FRAME_CUST3 = 1u << 27,
	SMF_FRAME_CUST4 = 1u << 28,
	SMF_FRAME_CUST5 = 1u << 29,
	SMF_FRAME_CUST6 = 1u << 30,
	SMF_FRAME_CUST7 = 1u << 31,
	//
	SMF_FRAMR_RESULT = SMF_FRAME_IS_EOS | SMF_FRAME_IS_FAIL,
};

enum {
	SMF_PACKET_FLAGS_FAIL = SMF_FRAME_IS_FAIL,// 1u << 0,//error packet
	SMF_PACKET_FLAGS_EOS = SMF_FRAME_IS_EOS,//1u << 1,//end of scan
	SMF_PACKET_FLAGS_FIRST = SMF_FRAME_IS_FIRST,//1u << 2,//first of scan
	SMF_PACKET_FLAGS_EXTRA = SMF_FRAME_IS_EXTRA,//1u << 4,//extra data package
	SMF_PACKET_FLAGS_MEDIA = SMF_FRAME_IS_MEDIA,//1u << 5,//shm_media_t
	SMF_PACKET_INCOMPLETE = SMF_FRAME_INCOMPLETE,//1u << 6,
	//SMF_PACKET_INFOMATION = SMF_FRAME_INFOMATION,//1u << 7,
};
//
typedef struct {
	uint8_t seqNO : 4;
	uint8_t crc : 4;
	uint8_t flags;
	uint16_t payload_size;
}smf_packet_t;
#endif
