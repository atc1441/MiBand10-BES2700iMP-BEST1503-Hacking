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
#ifndef __NANDFLASH_CFG_H__
#define __NANDFLASH_CFG_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "stdint.h"

enum NANDFLASH_SEC_REG_BASE_T {
    NANDFLASH_SEC_REG_BASE_PAGE2,
    NANDFLASH_SEC_REG_BASE_PAGE0,
};

enum NANDFLASH_ECC_STATUS_T {
    NANDFLASH_ECC_ST_2BITS_11_RES           = 0,
    NANDFLASH_ECC_ST_2BITS_11_COR           = 1,
    NANDFLASH_ECC_ST_3BITS_111_BAD          = 2,
    NANDFLASH_ECC_ST_3BITS_010_BAD          = 3,
};

#ifdef __cplusplus
}
#endif

#endif

