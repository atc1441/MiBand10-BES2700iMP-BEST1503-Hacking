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

// Description: beco coprocessor instructions bindings for verilator sim

#ifndef _BECO_SIM_IF_H
#define _BECO_SIM_IF_H

BECO_C_DECLARATIONS_START

#include <stdint.h>

// From simulation lib.

void     CDP(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn);
void     MCR(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint32_t rt,  uint8_t cpm, uint8_t cpn);
uint32_t MRC(uint8_t cpnum,    uint8_t opc1, uint8_t opc2, uint8_t cpm,  uint8_t cpn);
uint64_t MRRC_R(uint8_t cpnum, uint8_t opc1, uint8_t cpm);
void     MCRR(uint8_t cpnum,   uint8_t opc1, uint32_t rtl, uint32_t rth, uint8_t cpm);

inline void MCRR_Q(uint8_t ncp, uint8_t opc, uint64_t rt, uint8_t cpm) {
       union {uint64_t v64; uint32_t v32[2];} vp = {.v64 = rt};
       MCRR(ncp, opc, vp.v32[0], vp.v32[1], cpm);
}
inline void CDP2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn) {
    CDP(cpnum, (opc1 | 0x10), opc2, cpd, cpm, cpn);
}
inline void MCR2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint32_t cpd, uint8_t cpm, uint8_t cpn) {
    MCR(cpnum, (opc1 | 0x08), opc2, cpd, cpm, cpn);
}
inline uint32_t MRC2(uint8_t cpnum, uint8_t opc1, uint8_t opc2, uint8_t cpm, uint8_t cpn) {
    return MRC(cpnum, (opc1 | 0x08), opc2, cpm, cpn);
}

#define BECO_BARRIER()

// OS / SIMULATOR HOOK

int beco_init(int argc, char** argv, char** env);
int beco_exit(int ret);

#ifndef BECO_INIT
#  define BECO_INIT()    beco_init(argc, argv, env)
#  define BECO_EXIT(ret) beco_exit(ret)
#endif

BECO_C_DECLARATIONS_END

#endif

