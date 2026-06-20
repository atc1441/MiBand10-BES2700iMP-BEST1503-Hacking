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
 ****************************************************************************/
#ifndef __ASM_MACROS_H__
#define __ASM_MACROS_H__

#ifdef __ASSEMBLER__

    .macro    def_FuncExt   suffix name addr
    \name\()_\suffix = (\addr | 1)
    @.set \name\()_\suffix, (\addr | 1)
    .endm

    .macro    def_Func      name addr
    \name = (\addr | 1)
    @.set \name, (\addr | 1)
    .endm

    .macro    def_Symbol    name addr
    .global \name
    \name = \addr
    @.set \name, \addr
    .endm

#endif

#endif

