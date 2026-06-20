/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "stdio.h"
#include "hal_trace.h"
#include "string.h"
#include "heap_api.h"
#define LZMA_SDK 1
#if LZMA_SDK
#include "LzmaDec.h"
#endif

static heap_handle_t g_lzma_heap;
int bes_lzma_dec_init(uint8_t *lzma_heap, uint32_t heap_size)
{
    TRACE(0,"bes_lzma_dec_init lzma_heap=%p,heap_size=%d",lzma_heap,heap_size);
    g_lzma_heap = heap_register(lzma_heap, heap_size);
    return 0;
}

void *bes_lzma_malloc(uint32_t size)
{
    return heap_malloc(g_lzma_heap,size);
}

void bes_lzma_free(void *p)
{
    heap_free(g_lzma_heap,p);
}

#if LZMA_SDK
#define UNUSED_VAR(x) (void)x;
static void *SzAlloc(ISzAllocPtr p, size_t size) { UNUSED_VAR(p); return bes_lzma_malloc(size); }
static void SzFree(ISzAllocPtr p, void *address) { UNUSED_VAR(p); bes_lzma_free(address); }
static const ISzAlloc g_Alloc = { SzAlloc, SzFree };
#define LZMA_DATA_OFFSET       LZMA_PROPS_SIZE + 8
#endif
int bes_code_decom (unsigned char *outStream, unsigned int *uncompressedSize,
			      unsigned char *inStream,  unsigned int  length);

int bes_lzma_dec_buf(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t *output_size)
{
#if LZMA_SDK
    ELzmaStatus status = LZMA_STATUS_NOT_SPECIFIED;
#else
    int status = 0;
#endif
    TRACE(0,"lzma_dec_buf input=%p,input_size=%d,output=%p,output_size=%d",input,input_size,output,*output_size);
#if LZMA_SDK
    //DUMP8("0x%02x ", (uint8_t *)input, LZMA_DATA_OFFSET);
    input_size -= LZMA_DATA_OFFSET;    
    int ret = LzmaDecode(output, output_size, input+LZMA_DATA_OFFSET, &input_size, input, LZMA_DATA_OFFSET, LZMA_FINISH_END, &status, &g_Alloc);
#else
    int ret = bes_code_decom(output,output_size,input,input_size);
#endif
    TRACE(0,"lzma_dec_buf ret=%d,output_size=0x%x,input_size=0x%x,status=%d",ret,*output_size,input_size,status);
    return ret;
}

#if !LZMA_SDK
#include "lzma_decode.h"

#define LZMA_PROPERTIES_OFFSET 0
#define LZMA_SIZE_OFFSET       LZMA_PROPERTIES_SIZE
//#define LZMA_DATA_OFFSET       LZMA_SIZE_OFFSET + sizeof(uint64_t)
#define LZMA_DATA_OFFSET       LZMA_SIZE_OFFSET + 8

int bes_code_decom (unsigned char *outStream, unsigned int *uncompressedSize,
			      unsigned char *inStream,  unsigned int  length)
{
	int res = LZMA_RESULT_DATA_ERROR;
	int i;
	unsigned int outSizeFull = 0xFFFFFFFF; /* 4GBytes limit */
	unsigned int inProcessed;
	unsigned int outProcessed;
	unsigned int outSize;
	unsigned int outSizeHigh;
	CLzmaDecoderState state;  /* it's about 24-80 bytes structure, if int is 32-bit */
	unsigned char properties[LZMA_PROPERTIES_SIZE];
	unsigned int compressedSize = (unsigned int)(length - LZMA_DATA_OFFSET);

#if 0
	TRACE ("LZMA: Image address............... 0x%lx", inStream);
	TRACE ("LZMA: Properties address.......... 0x%lx", inStream + LZMA_PROPERTIES_OFFSET);
	TRACE ("LZMA: Uncompressed size address... 0x%lx", inStream + LZMA_SIZE_OFFSET);
	TRACE ("LZMA: Compressed data address..... 0x%lx", inStream + LZMA_DATA_OFFSET);
	TRACE ("LZMA: Destination address......... 0x%lx", outStream);
#endif
	if (inStream == NULL)
		return LZMA_RESULT_DATA_ERROR;

	memcpy(properties, inStream + LZMA_PROPERTIES_OFFSET, LZMA_PROPERTIES_SIZE);
	memset(&state, 0, sizeof(state));
	res = LzmaDecodeProperties(&state.Properties, properties, LZMA_PROPERTIES_SIZE);
	if (res != LZMA_RESULT_OK) {
		TRACE(0, "lzma decode properties fail");
		return res;
	}

	outSize = 0;
	outSizeHigh = 0;
	/* Read the uncompressed size */
	for (i = 0; i < 8; i++) {
		unsigned char b = inStream[LZMA_SIZE_OFFSET + i];
	    if (i < 4) {
		    outSize += (UInt32)(b) << (i * 8);
		} else {
			outSizeHigh += (UInt32)(b) << ((i - 4) * 8);
		}
	}

	outSizeFull = (unsigned int)outSize;
#if 0
	if (sizeof(unsigned int) >= 8) {
		/*unsigned int is a 64 bit uint => We can manage files larger than 4GB! */
		outSizeFull |= (((unsigned int)outSizeHigh << 16) << 16);
	} else if (outSizeHigh != 0 || (UInt32)(unsigned int)outSize != outSize) {
	    /* unsigned int is a 32 bit uint => We cannot manage files larger than 4GB! */
		TRACE ("LZMA: 64bit support not enabled.\n");
	     return LZMA_RESULT_DATA_ERROR;
	}
#endif

	//TRACE ("LZMA: Uncompresed size............ 0x%lx", outSizeFull);
	//TRACE ("LZMA: Compresed size.............. 0x%lx", compressedSize);
	TRACE (1, "LZMA: Dynamic memory needed....... 0x%x", LzmaGetNumProbs(&state.Properties) * sizeof(CProb));

	state.Probs = (CProb *)bes_lzma_malloc(LzmaGetNumProbs(&state.Properties) * sizeof(CProb));
#if 0
	hal_cmu_codec_clock_enable();
    hal_cmu_codec_reset_clear();
	state.Probs = (CProb *)0x40308000;
#endif
	if (state.Probs == 0
	    || (outStream == 0 && outSizeFull != 0)
	    || (inStream == 0 && compressedSize != 0)) {
        if (state.Probs != NULL)
        {
            bes_lzma_free(state.Probs);
        }

		TRACE (0, "a7 decom malloc fail");
		return LZMA_RESULT_DATA_ERROR;
	}

	TRACE (0, " allocated.");

	/* Decompress */
	res = LzmaDecode(&state, inStream + LZMA_DATA_OFFSET, compressedSize,
		&inProcessed, outStream, outSizeFull,  &outProcessed);
	if (res != LZMA_RESULT_OK)  {
		TRACE(0, "lzma decompress fail");
		return res;
	}

	*uncompressedSize = outProcessed;
	bes_lzma_free(state.Probs);
	TRACE(0,"LZMA: Uncompresed end outProcessed=%d",outProcessed);
	return res;
}
#endif
