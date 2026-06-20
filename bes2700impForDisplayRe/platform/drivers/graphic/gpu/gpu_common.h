
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

#ifndef _GPU_COMMON_H
#define _GPU_COMMON_H

#include "gpu_conf.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

typedef struct {
    uint32_t width;    /*! Width of the buffer in pixels. */
    uint32_t height;    /*! Height of the buffer in pixels. */
    uint32_t stride;    /*! The number of bytes to move from one line in the buffer to the next line. */
    uint8_t *memory;
} screen_fb_buffer_t;



/**
 * gpu_save_cmdbuffer - save gpu cmd buffer to file
 *
 */
void gpu_save_cmdbuffer(char * name);

int gpu_save_raw(const char *name, vg_lite_buffer_t *buffer);

int gpu_lcdc_display(vg_lite_buffer_t * buffer);

void gpu_set_soft_break_point();


#ifdef __cplusplus
}
#endif

#endif //_GPU_COMMON_H
