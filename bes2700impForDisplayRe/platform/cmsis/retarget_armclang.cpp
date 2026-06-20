/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifdef __ARMCC_VERSION

extern "C"
{

#include "hal_trace.h"

#ifdef NOSTD

void __rt_raise(int sig, intptr_t type)
{
    ASSERT(false, "__rt_raise: sig=%d type=0x%x", sig, type);
}

#endif

}

#endif // __ARMCC_VERSION

