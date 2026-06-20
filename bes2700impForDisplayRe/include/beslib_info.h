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
#ifndef __BESLIB_INFO_H__
#define __BESLIB_INFO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

#ifndef BESLIB_INFO
#error "Please add 'CFLAGS_<fileBaseName>.o += -DBESLIB_INFO=$(BESLIB_INFO)' in Makefile"
#endif

#define BESLIB_INFO_STR "LIB_INFO=" TO_STRING(BESLIB_INFO)

#ifdef __cplusplus
}
#endif

#endif

