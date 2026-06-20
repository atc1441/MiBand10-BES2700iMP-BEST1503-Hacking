
/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
 * **************************************************************************/


#ifndef __HAL_GAMU_H
#define __HAL_GAMU_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Included Files
 */
#include "plat_types.h"

typedef void* (*fbmalloc)(uint32_t size, uint8_t aligns);
typedef float (*sqrtf_f)(float x);

typedef struct {
    uint16_t id;           //buffer id
    uint16_t width;        //screen width
    uint16_t height;       //screen heigth
    uint16_t stride;       //line stride
    uint8_t  bpp;          //buffer color deepth
    fbmalloc align_malloc; //provided by user, used to alloc memory for gfx buffers
    sqrtf_f sqrtf;           //provided by user, used to calculate line visible length
    uint32_t vmem_size;    //
    uint32_t *vmem;        //buffer start mmu virtual memory address, used by cpu,gpu,lcdc
    uint32_t phymem_size;  //for debug only
    uint32_t *phymem;      //for debug only
    uint32_t bufferoffset; //virtual buffer middle physical address offset
    uint32_t xoffset;
    uint32_t yoffset;
    uint16_t radius;       //internal use only
    uint32_t used_vblocks; //internal use only
}gamu_fb_info_t;

void hal_gamu_setup(gamu_fb_info_t *fb);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_GAMU_H */
