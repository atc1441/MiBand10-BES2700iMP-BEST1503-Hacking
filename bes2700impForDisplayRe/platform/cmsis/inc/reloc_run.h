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
#ifndef __RELOC_RUN_H__
#define __RELOC_RUN_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

struct RELOC_RUN_CFG_T {
    uint32_t reloc_start;
    uint32_t reloc_end;
    int32_t offset;
    uint32_t new_sp;
    uint32_t new_sp_lim;
    uint32_t new_entry;
};

NORETURN
void reloc_run(const struct RELOC_RUN_CFG_T *cfg);

#ifdef __cplusplus
}
#endif

#endif
