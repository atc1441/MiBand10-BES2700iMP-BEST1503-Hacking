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

// Description: ARM Cortex M coprocessor interface for C/C++
//
// Clang's builtin_cdp(), builtin_mcr(), etc implementation is seriously
// broken by misplaced checks on constness of parameters, breaking templates
// and inline functions.
//
// Luckily we can re-implement the builtin using inline assembly.
//
#include <stdint.h>

// ARM Cortex M coprocessor interface
//
// Supports up to eight separate coprocessors, CP0-CP7

//
// MCR, MCR2
// ====================
//  Move to Coprocessor from ARM Register passes the value of an
//  ARM register to a coprocessor.
//
// Encoding
//  MCR[2]<c><q> <coproc>, #<opc1>, <Rt>, <CRn>, <CRm> {,#<opc2>}
// Range:
//    opc1, opc2: [0..7]
//    CRn, CRm:   [0..15]
//
//    Instruction CPOPC[8] CPOPC[7:4]   CPOPC[3:0]
//    -----------+--------+------------+-------------
//    MCR         'CP2'    {opc1, 1'bx} {opc2, 1'bx}
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 1 0|opc1 |0|  CRn  |     Rt    | coproc  |opc2 |1|  CRm  |
//   ---------------------------------------+--------------------------------------
//
//  (*) 0 for Encoding T1, 1 for Encoding T2

//
// MCRR, MCRR2
// ====================
//  Move to Coprocessor from two ARM Register passes the value from
//  ARM registers to a coprocessor.
//
// Encoding
//  MCRR[2]<c><q> <coproc>, #<opc1>, <Rt>, <Rt2>, <CRm>
//
// Range:
//    opc1: [0..15]
//    CRm:   [0..15]
//
//    Instruction CPOPC[8] CPOPC[7:4]   CPOPC[3:0]
//    -----------+--------+------------+-------------
//    MCRR        'CP2'    4'bx         opc1
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 0 0 0 1 0 0|  Rt2  |     Rt    | coproc  | opc1  | CRm   |
//   ---------------------------------------+--------------------------------------
//
//  (*) 0 for Encoding T1, 1 for Encoding T2

//
// MRC, MRC2
// ====================
//  Move to ARM Register from Coprocessor causes a coprocessor to transfer
//  a value to an ARM register or to the condition flags.
//
// Encoding
//  MRC[2]<c><q> <coproc>, #<opc1>, <Rt>, <CRn>, <CRm> {,#<opc2>}
//
// Range:
//    opc1, opc2: [0..7]
//    CRn, CRm:   [0..15]
//
//    Instruction CPOPC[8] CPOPC[7:4]   CPOPC[3:0]
//    -----------+--------+------------+-------------
//    MRC          'CP2'    {opc1, 1'bx} {opc2, 1'bx}
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 1 0|opc1 |1|  CRn  |     Rt    | coproc  |opc2 |1|  CRm  |
//   ---------------------------------------+--------------------------------------
//
//  (*) 0 for Encoding T1, 1 for Encoding T2

//
// MRRC, MRRC2
// ====================
//  Move to two ARM Registers from Coprocessor causes a coprocessor to
//  transfer values to two ARM registers.
//
// Encoding
//  MRRC[2]<c><q> <coproc>, #<opc1>, <Rt>, <Rt2>, <CRm>
//
// Range:
//    opc1: [0..15]
//    CRm:  [0..15]
//
//    Instruction CPOPC[8] CPOPC[7:4]   CPOPC[3:0]
//    -----------+--------+------------+-------------
//    MRRC         'CP2'    4'bx         opc1
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 0 0 0 1 0 1|  Rt2  |     Rt    | coproc  |  opc1 |  CRm  |
//   ---------------------------------------+--------------------------------------
//
//  (*) 0 for Encoding T1, 1 for Encoding T2
//

//
// CDP
// ===
//  Coprocessor Data Processing tells a coprocessor to perform
//  an operation that is independent of ARM registers and memory.
//
// Encoding
//  CDP[2]<c> <coproc>, #<opc1>, <CRd>, <CRn>, <CRm> {,#<opc2>}
//
//    Instruction CPOPC[8] CPOPC[7:4]   CPOPC[3:0]
//    -----------+--------+------------+-------------
//    CDP         'CP2'    opc1         {opc2, 1'bx}
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 1 0| opc1  |  CRn  |    CRd    | coproc  |opc2 |0| CRm   |
//   ---------------------------------------+--------------------------------------
//
//  (*) 0 for Encoding T1, 1 for Encoding T2


// STC (not supported in CM33, STAR)
// =================
//  Store Coprocessor stores data from a coprocessor to a sequence
//  of consecutive memory addresses.
//
// Encoding
//  SDC[2]{L}<c> <coproc>,<CRd>,[<Rn>{,#+/-<imm>}]
//  SDC[2]{L}<c> <coproc>,<CRd>,[<Rn>,#+/-<imm>]!
//  SDC[2]{L}<c> <coproc>,<CRd>,[<Rn>],#+/-<imm>
//  SDC[2]{L}<c> <coproc>,<CRd>,[<Rn>],<option>
//
// Range:
//    imm8: [-128..127]
//    CRd : [0..15]
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 0|P U D W 0|   Rn  |    CRd    | coproc  |    imm8       |
//   ---------------------------------------+--------------------------------------
//
//   (*) 0 for Encoding T1, 1 for Encoding T2
//

// LDC IMMEDIATE (not supported in CM33, STAR)
// =============
//  Load Coprocessor loads memory data from a sequence of consecutive
//  memory addresses to a coprocessor.
//
// Encoding
//  LDC[2]{L}<c> <coproc>,<CRd>,[<Rn>{,#+/-<imm>}]
//  LDC[2]{L}<c> <coproc>,<CRd>,[<Rn>,#+/-<imm>]!
//  LDC[2]{L}<c> <coproc>,<CRd>,[<Rn>],#+/-<imm>
//  LDC[2]{L}<c> <coproc>,<CRd>,[<Rn>],<option>
//
// Range:
//    imm8: [-128..127]
//    CRd : [0..15]
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 0|P U D W 1|   Rn  |    CRd    | coproc  |    imm8       |
//   ---------------------------------------+--------------------------------------
//
//   (*) 0 for Encoding T1, 1 for Encoding T2
//

//
// LDC PCREL (not supported in CM33, STAR)
// =========
//  Load Coprocessor loads memory data from a sequence of consecutive
//  memory addresses to a coprocessor.
//
// Encoding
//  LDC[2]{L}<c> <coproc>,<CRd>,label
//
//   ---------------------------------------+--------------------------------------
//   | 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0|
//   |  1  1  1  *| 1  1 0|1 U D 0|1 1 1 1 1|    CRd    | coproc  |    imm8       |
//   ---------------------------------------+--------------------------------------
//
//   (*) 0 for Encoding T1, 1 for Encoding T2
//

#define BECO_BARRIER() __asm volatile ("" :::"memory")

// Extended assembly:
//    %0    Constant or register. Constant format is "#100" (note '#' prefix).
//    %R0   High part of register or Constant.
//    %c0   Constant without '#' prefix

#define CPINST(inst, coproc, opc) inst " " #coproc ", #" #opc


//
// MRC
// move to register from coprocessor
//
// MRC[2]<c><q> <coproc>, #<opc1>, <Rt>, <CRn>, <CRm> {,#<opc2>}
//   opc1, opc2: [0..7]
//   CRn, CRm:   [0..15]
//
#define MRC_(mnemonic, type, __num, __opc1, __opc2, __crm, __crn) ({                  \
  type __res;                                                                         \
  BECO_VASM(mnemonic "\tp%c[num], %[opc], %[res], cr%c[crn], cr%c[crm], %[opc2]"      \
        : [res]"=r"(__res)                                                            \
        : [num]"n"(__num), [opc]"n"(__opc1), [opc2]"n"(__opc2),                       \
          [crn]"n"(__crn), [crm]"n"(__crm));                                          \
  __res;                                                                              \
  })


#define MRC(ARGS...)           MRC_("mrc",  uint32_t, ARGS)
#define MRC2(ARGS...)          MRC_("mrc2", uint32_t, ARGS)



//
// MCR
// move to coprocessor from register
//
// MCR[2]<c><q> <coproc>, #<opc1>, <Rt>, <CRn>, <CRm> {,#<opc2>}
//   opc1, opc2: [0..7]
//   CRn, CRm:   [0..15]
//
// (Notice: wrapper below flip crm/crn operands compared to ARM opcode.)

#define MCR_(mnemonic, __num, __opc1, __opc2, __rt, __crm, __crn)                     \
  BECO_VASM(mnemonic "\tp%c[num], %[opc], %[res], cr%c[crn], cr%c[crm], %[opc2]"      \
        :                                                                             \
        : [num]"n"(__num), [opc]"n"(__opc1), [opc2]"n"(__opc2),                       \
          [res]"r"(__rt), [crn]"n"(__crn), [crm]"n"(__crm))


#define MCR(ARGS...)           MCR_("mcr",  ARGS)
#define MCR2(ARGS...)          MCR_("mcr2", ARGS)




//
// MRRC
// move to register from coprocessor
//
//  MRRC[2]<c><q> <coproc>, #<opc1>, <Rt>, <Rt2>, <CRm>
//   CRm:   [0..15]
//
#define MRRC_R_(mnemonic, type, __num, __opc1, __crm) ({                              \
  type __res;                                                                         \
  BECO_VASM(mnemonic "\tp%c[num], %[opc], %[res],%R[res], cr%c[crm]"                  \
        : [res]"=r"(__res)                                                            \
        : [num]"n"(__num), [opc]"n"(__opc1), [crm]"n"(__crm));                        \
  __res;                                                                              \
  })


#define MRRC_R(ARGS...)        MRRC_R_("mrrc", uint64_t, ARGS)
#define MRRC2_R(ARGS...)       MRRC_R_("mrrc2", uint64_t, ARGS)




//
// MCRR
// move to coprocessor from register
//
//  MCRR[2]<c><q> <coproc>, #<opc1>, <Rt>, <Rt2>, <CRm>
//   CRm:   [0..15]
//
#define MCRR_(mnemonic, ncp, opc, rt, rt2, crm)                                       \
        BECO_VASM(mnemonic "\tp%c0, %1, %2, %3, cr%c4"                                \
           :                                                                          \
           : "n"(ncp),"n"(opc), "r"(rt),"r"(rt2), "n"(crm)  )

#define MCRR_Q_(mnemonic, ncp, opc, rt, crm)                                          \
        BECO_VASM(mnemonic "\tp%c0, %1, %2, %R2, cr%c3"                               \
           :                                                                          \
           : "n"(ncp), "n"(opc), "r"(rt), "n"(crm)  )


#define MCRR( ARGS...)         MCRR_("mcrr",  ARGS)
#define MCRR2(ARGS...)         MCRR_("mcrr2", ARGS)

#define MCRR_Q( ARGS...)       MCRR_Q_("mcrr",  ARGS)
#define MCRR2_Q(ARGS...)       MCRR_Q_("mcrr2", ARGS)




//
// CDP
// Internal data processing
//
//  CDP[2]<c> <coproc>,<opc1>,<CRd>,<CRn>,<CRm>,<opc2>
//   CRd, CRn, CRm:   [0..15]
//
// (Notice: wrapper below flip crm/crn operands compared to ARM opcode.)

#define CDP_(mnemonic, __num, __opc1, __opc2, __crd, __crm, __crn)                    \
  BECO_VASM(mnemonic "\tp%c[num], %[opc], cr%c[crd], cr%c[crn], cr%c[crm], %[opc2]"   \
        :                                                                             \
        : [num]"n"(__num), [opc]"n"(__opc1), [opc2]"n"(__opc2),                       \
          [crd]"n"(__crd), [crn]"n"(__crn), [crm]"n"(__crm))


#define CDP(ARGS...)           CDP_("cdp", ARGS)
#define CDP2(ARGS...)          CDP_("cdp2", ARGS)



//
// Unsupported instructions
//
//  LDC[2]{L}<c> <coproc>,<CRd>,label
#define LDCPC() __not_implemented__
//  LDC[2]{L}<c> <coproc>,<CRd>,[<Rn>{,#+/-<imm>}]
#define LDC()   __not_implemented__
//  SDC[2]{L}<c> <coproc>,<CRd>,[<Rn>{,#+/-<imm>}]
#define STC()   __not_implemented__

