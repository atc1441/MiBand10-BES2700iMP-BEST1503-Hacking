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
#include "plat_addr_map.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_memsc.h"

#ifdef CHIP_SUBSYS_SENS
#include CHIP_SPECIFIC_HDR(reg_senscmu)

static struct SENSCMU_T * const cmu = (struct SENSCMU_T *)SENS_CMU_BASE;
#else
#include CHIP_SPECIFIC_HDR(reg_cmu)

static struct CMU_T * const cmu = (struct CMU_T *)CMU_BASE;
#endif

int hal_memsc_lock(enum HAL_MEMSC_ID_T id)
{
    if (id >= ARRAY_SIZE(cmu->MEMSC)) {
        return 0;
    }

    return cmu->MEMSC[id];
}

void hal_memsc_unlock(enum HAL_MEMSC_ID_T id)
{
    if (id >= ARRAY_SIZE(cmu->MEMSC)) {
        return;
    }

    cmu->MEMSC[id] = 1;
}

bool hal_memsc_avail(enum HAL_MEMSC_ID_T id)
{
    if (id >= ARRAY_SIZE(cmu->MEMSC)) {
        return false;
    }

    return !!(cmu->MEMSC_STATUS & (1 << id));
}

#ifdef AON_CMU_BASE
#include CHIP_SPECIFIC_HDR(reg_aoncmu)

static struct AONCMU_T * const aoncmu = (struct AONCMU_T *)AON_CMU_BASE;

int BOOT_TEXT_SRAM_LOC hal_memsc_aon_lock(enum HAL_MEMSC_AON_ID_T id)
{
    if (id >= ARRAY_SIZE(aoncmu->MEMSC)) {
        return 0;
    }

    return aoncmu->MEMSC[id];
}

void BOOT_TEXT_SRAM_LOC hal_memsc_aon_unlock(enum HAL_MEMSC_AON_ID_T id)
{
    if (id >= ARRAY_SIZE(aoncmu->MEMSC)) {
        return;
    }

    aoncmu->MEMSC[id] = 1;
}

bool hal_memsc_aon_avail(enum HAL_MEMSC_AON_ID_T id)
{
    if (id >= ARRAY_SIZE(aoncmu->MEMSC)) {
        return false;
    }

    return !!(aoncmu->MEMSC_STATUS & (1 << id));
}
#endif

