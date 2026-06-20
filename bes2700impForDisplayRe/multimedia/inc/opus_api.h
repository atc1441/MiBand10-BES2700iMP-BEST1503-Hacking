/***************************************************************************
 *
 * Copyright 2015-2024 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#ifndef __OPUS_API_H__
#define __OPUS_API_H__
#include <stddef.h>

#define OPUS_API_RET_SUCCESS 0
#define OPUS_API_RET_FAIL -1

typedef int OpusApiRet_t;

typedef struct {
	void* (*malloc_cb)(int size);
	void* (*realloc_cb)(void* rmem, int newsize);
	void(*free_cb)(void* buf);
	void* (*memset_cb) (void* dst0, int c0, size_t length);
	void* (*memcpy_cb) (void* dst, const void* src, size_t length);
	void* (*memmove_cb) (void* dst, const void* src, size_t length);
}OpusApi_BasePort_t;

typedef enum {
	OPUS_API_ENC_CHOOSE_NORMAL = 0,
	OPUS_API_ENC_CHOOSE_INROM,
	OPUS_API_ENC_CHOOSE_MAX,
}OpusApi_EncChoose_c;

typedef enum {
	OPUS_API_DEC_CHOOSE_NORMAL = 0,
	OPUS_API_DEC_CHOOSE_MAX,
}OpusApi_DecChoose_c;

typedef enum {
	OPUS_API_ENC_SET_BIT_RATE = 0,
	OPUS_API_ENC_SET_FRAME_DURATION_0P1MS,
	OPUS_API_ENC_SET_USE_VBR,
	OPUS_API_ENC_SET_COMPLEXITY,
	OPUS_API_ENC_SET_MAX,
}OpusApi_EncSetChhoose_e;

typedef enum {
	OPUS_API_DEC_SET_SEEK_SAMPLES = 0,
	OPUS_API_DEC_SET_MAX,
}OpusApi_DecSetChhoose_e;


typedef enum {
	//OPUS_API_ENC_GET_FINAL_RANGE = 0,
	OPUS_API_ENC_GET_MAX,
}OpusApi_EncGetChhoose_e;


typedef struct {
	int usr;
}OpusApi_DecReserve_t;


#ifdef __cplusplus
extern "C" {
#endif

	//base
	OpusApiRet_t opus_api_memory_register(OpusApi_BasePort_t* memory);

	//enc
	OpusApiRet_t opus_api_create_encoder(void** pHd, int fs, int channels, bool isWithHead, OpusApi_EncChoose_c choose);
	OpusApiRet_t opus_api_destory_encoder(void* hd);
	OpusApiRet_t opus_api_encoder_set(void* hd, OpusApi_EncSetChhoose_e choose, void* val);
	OpusApiRet_t opus_api_encoder_get(void* hd, OpusApi_EncGetChhoose_e choose, void* val);
	OpusApiRet_t opus_api_encoder_run(void* hd, short* in, int inSample, unsigned char* out, int* outByte);

	//dec
	OpusApiRet_t opus_api_create_decoder(void** pHd, int fs, int channels, OpusApi_DecChoose_c choose);
	OpusApiRet_t opus_api_destory_decoder(void* hd);
	OpusApiRet_t opus_api_decoder_set(void* hd, OpusApi_DecSetChhoose_e choose, void* val);
	OpusApiRet_t opus_api_decoder_run(void* hd, unsigned char* in, int inByte, short* out, int* outSample, bool isPlc);


#ifdef __cplusplus
}
#endif

#endif