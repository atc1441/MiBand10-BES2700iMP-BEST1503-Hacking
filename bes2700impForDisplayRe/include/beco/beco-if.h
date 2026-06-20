/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#pragma once

// Description: beco coprocessor instructions for C

#ifndef _BECO_IF_H
#define _BECO_IF_H

BECO_C_DECLARATIONS_START

#include <arm_acle.h>

// Convert to ARM coprocessor instructions

#define CDP(ncp, opc1, opc2, crd, crm, crn)     \
        __arm_cdp(ncp, opc1, crd, crn, crm, opc2)
#define CDP2(ncp, opc1, opc2, crd, crm, crn)    \
        __arm_cdp2(ncp, opc1, crd, crn, crm, opc2)
#define MCR(ncp, opc1, opc2, regin, crm, crn)   \
        __arm_mcr(ncp, opc1, (uint32_t)regin, crn, crm, opc2)
#define MCR2(ncp, opc1, opc2, regin, crm, crn)  \
        __arm_mcr2(ncp, opc1, (uint32_t)regin, crn, crm, opc2)
#define MRC(ncp, opc1, opc2, crm, crn)          \
        __arm_mrc(ncp, opc1, crn, crm, opc2)
#define MRRC_R(ncp, opc, cpm)                   \
        __arm_mrrc(ncp, opc, cpm)
#define MCRR_Q(ncp, opc, rt, cpm)                              \
       __arm_mcrr(ncp, opc, rt, cpm)

#define BECO_BARRIER() __asm volatile ("" :::"memory")

BECO_C_DECLARATIONS_END

#endif /*_BECO_IF_H*/
