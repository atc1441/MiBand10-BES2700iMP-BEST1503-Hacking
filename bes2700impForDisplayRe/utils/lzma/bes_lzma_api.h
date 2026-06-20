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
#ifndef __BES_LZMA_API_H__
#define __BES_LZMA_API_H__

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int bes_lzma_dec_init(uint8_t *lzma_heap, uint32_t heap_size);
void *bes_lzma_malloc(uint32_t size);
void bes_lzma_free(void *p);
int bes_lzma_dec_buf(uint8_t *input, uint32_t input_size, uint8_t *output, uint32_t *output_size);
#ifdef OTA_BIN_COMPRESSED
bool ota_copy_compressed_image(uint32_t srcFlashOffset, uint32_t dstFlashOffset,
                                uint32_t imageCrc, uint32_t imageSize);
#endif
#ifdef __BT_RAMRUN__
uint32_t ramrun_copy_compressed_image(void *dstFlashOffset,const void *srcFlashOffset);
#endif
#ifdef __cplusplus
    }
#endif

#endif//__FMDEC_H__

