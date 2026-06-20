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
#ifndef __NVRECORD_APPMODE_H__
#define __NVRECORD_APPMODE_H__

#include "nvrecord_extension.h"

#ifdef __cplusplus
extern "C" {
#endif

nvrec_appmode_e nv_record_appmode_get();
void nv_record_appmode_set(nvrec_appmode_e mode);

#ifdef __cplusplus
}
#endif

#endif /* __NVRECORD_APPMODE_H__ */
