/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "string.h"
#include "stdlib.h"
#include "stdio.h"

#include "export_fn_rom.h"
#include "plat_addr_map.h"

void *
memset(dst0, c0, length)
    void *dst0;
    int c0;
    size_t length;
{
#ifdef CHIP_BEST1306
    return export_fn_rom->memset(dst0, c0, length);
#else
    return __export_fn_rom.memset(dst0, c0, length);
#endif
}

void * memcpy(void *dst,const void *src, size_t length)
{
#ifdef CHIP_BEST1306
    return export_fn_rom->memcpy(dst, src, length);
#else
    return __export_fn_rom.memcpy(dst, src, length);
#endif
}

#ifdef NO_MEMMOVE
#ifdef NOSTD
#include "memmove.c"
#else
// Use the memmove() in toolchain lib
#endif
#else
void * memmove(void *dst,const void *src, size_t length)
{
#ifdef CHIP_BEST1306
    return export_fn_rom->memmove(dst, src, length);
#else
    return __export_fn_rom.memmove(dst, src, length);
#endif
}
#endif

