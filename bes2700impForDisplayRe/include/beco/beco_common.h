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

// Description: Beco coprocessor types and constants

#ifndef _BECO_COMMON_H
#define _BECO_COMMON_H

#include <stdint.h>
#include "beco.h"

#if defined(CHIP_BEST1501)
#define BECO_PRE_V1
#endif

BECO_C_DECLARATIONS_START

/*************** OPCODES ***************/
/*
**Instruction**  |**TYPE**| **OPC1/2**  |**Format**|**Cycles**
-----------------|--------|-------------|----------|----------
READ_CPID        | MRC    | 6'b000/000  | fmt8     | 1
READ_STATUS      | MRC    | 6'b000/001  | fmt8     | 1
READ_ACC         | MRC    | 6'b000/010  | fmt9     | 1
READ_NEXT_ACC    | MRC    | 6'b000/100  | fmt9     | 1
READ_CONFIG      | MRC    | 6'b000/011  | fmt8     | 1
READ_REG         | MRRC   | 4'b0000/    | fmt10    | 1
WRITE_CONFIG     | MCR    | 6'b000/000  | fmt7     | 1
EXEC_MMACGR      | MCR    | 6'b000/001  | fmt11    | 1
MACRO_MMACGR4    | MCR    | 6'b000/010  | fmt11    | 4 *
EXEC_MMAC        | MCRR   | 4'b0000/    | fmt6     | 1
WRITE_REG        | MCRR   | 4'b0001/    | fmt5     | 1
WRITE_OUTCONFIG  | CDP    | 7'b0000/000 | fmt3     | 1
WRITE_ALUCONFIG  | CDP    | 7'b0000/001 | fmt3     | 1
WRITE_RFWINOFF   | CDP    | 7'b0000/101 | fmt3     | 1
CLEAR_ACC        | CDP    | 7'b0000/010 | fmt2     | 1
SET_ACC_BIAS     | CDP    | 7'b0010/010 | fmt2     | 1
PRELOAD_ACC      | CDP    | 7'b0010/011 | fmt2     | 1
MOVE_ACC_TO_REG  | CDP    | 7'b0000/011 | fmt4     | 1
EXEC_MMACR       | CDP    | 7'b0000/110 | fmt2     | 1
MOVE32           | CDP    | 7'b0000/111 | fmt1     | 1
BSHIFT           | CDP    | 7'b0001/??? | fmt1     | 1
MACRO_MMACRR     | CDP    | 7'b0010/000 | fmt2     | 4
MACRO_MOVEACC    | CDP    | 7'b0010/001 | fmt4     | 2
MACRO_BSHIFT5    | CDP    | 7'b0011/??? | fmt1     | 5

Beco v1.3:
EXEC_MMACGR_S    | MCR    | 6'b001/???  | fmt11s   | 1 (1.3)
EXEC_MMACGR_SWB  | MCR    | 6'b010/???  | fmt11s   | 1 (1.3)
MACRO_MMACGR4_S  | MCR    | 6'b011//??? | fmt11s   | 4 *(1.3)
MACRO_MMACGR4_SWB| MCR    | 6'b100/???  | fmt11s   | 4 *(1.3)
EXEC_MMACGRR     | MCRR   | 4'b0010     | fmt12    | 1 (1.3)

MOVEACC_IB       | CDP    | 7'b0000/100 | fmt4     | 1 (1.3)
MACRO_MOVEACC_IB | CDP    | 7'b0000/101 | fmt4     | 2 (1.3)
MACRO_BSHIFT3    | CDP    | 7'b0100/??? | fmt5     | 3 (1.3)
BSPLIT           | CDP    | 7'b0101/??? | fmt5     | 1 (1.3)

Beco FFT:
WRITE_FFTCONFIG  | CDP    | 7'b0010/111 | fmt3     | 1 (1.3)
EXEC_FFT         | MCRR   | 4'b0011/    | fmt6     | 1 (1.3)
READ_FFT         | MRRC   | 4'b0001/    | fmt10    | 1 (1.3)

*/

#define OPC_RDCPID  0,0   // MRC
#define OPC_RDSTAT  0,1   // MRC
#define OPC_RDACC   0,2   // MRC
#define OPC_RDCONF  0,3   // MRC
#define OPC_RDNXACC 0,4   // MRC
#define OPC_WRCONF  0,0   // MCR
#define OPC_MMACGR  0,1   // MCR
#define OPC_MMACGR4 0,2   // MCR
#define OPC_MMACGR_S(N)   1,N  // MCR V3
#define OPC_MMACGR_SWB(N) 2,N  // MCR V3
#define OPC_MMACGR4_S(N)  3,N  // MCR V3
#define OPC_MMACGR4_SWB(N) 4,N // MCR V3

#define OPC_MMAC    0     // MCRR
#define OPC_WRREG   1     // MCRR
#define OPC_MMACGRR 2     // MCRR
#define OPC_WRFFT   3     // MCRR v3 fft

#define OPC_RDREG   0     // MRRC
#define OPC_RDFFT   1     // MRRC v3 fft

#define OPC_SETOCONF 0,0   // CDP
#define OPC_SETACONF 0,1   // CDP
#define OPC_RFWINDOW 0,5   // CDP v3
#define OPC_CLRACC   0,2   // CDP
#define OPC_MATOR32  0,3   // CDP
#define OPC_MVACC32_IB 0,4 // CDP V3
#define OPC_MMACR    0,6   // CDP
#define OPC_MRTOR32  0,7   // CDP
#define OPC_MMACRR   2,0   // CDP
#define OPC_MATOR64  2,1   // CDP
#define OPC_SETBIAS  2,2   // CDP
#define OPC_PSETACC  2,3   // CDP
#define OPC_MMACRRNSQ  2,4 // CDP V3
#define OPC_MVACC64_IB 2,5 // CDP V3
#define OPC_SETACCMASK 2,6 // CDP v3
#define OPC_SETFFTCONF 2,7 // CDP v3 fft
#define OPC_BSHIFT(N) 1,N  // CDP
#define OPC_BSHIFT5(N) 3,N // CDP
#define OPC_BSHIFT3(N) 4,N // CDP V3
#define OPC_BSPLIT(M)  5,M // CDP V3

// beco ALU
typedef enum {
    BECO_BSPLIT_BYTE_EVEN,
    BECO_BSPLIT_BYTE_ODD,
    BECO_BSPLIT_WORD_EVEN,
    BECO_BSPLIT_WORD_ODD,
} beco_bsplit_t;

// SALU Instructions

// Beco 64 bit SIMD unit (SALU), element size {8,16,32)

typedef enum {
    VECSZ32  = 0,
    VECSZ16  = 1,
    VECSZ8   = 2
} beco_vecsz;

// Constants for Salu instructions with small int {7, 15, 23}
//
// Functions: beco_salu_bclip_{16,32}
// Parameter n: pass [1,2,3] for constants [7,15,23]
typedef enum {
    SALU_CONST_7  = 1,
    SALU_CONST_15 = 2,
    SALU_CONST_23 = 3
} salu_const2s_t;

// Constants for Salu instructions with small int {8, 16, 24}
//
// Functions: beco_salu_bclipu_{16,32}
// Parameter n: pass [1,2,3] for constants [8,16,24]
typedef enum {
    SALU_CONST_8  = 1,
    SALU_CONST_16 = 2,
    SALU_CONST_24 = 3
} salu_const2u_t;


// Logic operations
#define OPC_SALU_LZERO      0b1100,0b000
#define OPC_SALU_LAND       0b1100,0b001
#define OPC_SALU_LNAND      0b1110,0b110
#define OPC_SALU_LANDN      0b1100,0b100
#define OPC_SALU_LXOR       0b1100,0b110
#define OPC_SALU_LXNOR      0b1110,0b001
#define OPC_SALU_LOR        0b1100,0b111
#define OPC_SALU_LNOR       0b1110,0b000
#define OPC_SALU_LNOT       0b1110,0b010
#define OPC_SALU_LMASK      0b1110,0b010
#define OPC_SALU_LONES      0b1110,0b111

// Arith operations
#define OPC_SALU_ADD(vs)   ((vs << 2) | 0b00),0b000
#define OPC_SALU_SUB(vs)   ((vs << 2) | 0b10),0b000
#define OPC_SALU_RSUB(vs)  ((vs << 2) | 0b00),0b001
#define OPC_SALU_ABS(vs)   ((vs << 2) | 0b10),0b001

// Misc Operarations
#define OPC_SALU_MASK(vs)  ((vs << 2) | 0b00),0b010

// Shifts
#define OPC_SALU_ASHL(vs)  ((vs << 2) | 0b10),0b011 // ASH: Arith shift: sign == 1
#define OPC_SALU_SHL(vs)   ((vs << 2) | 0b00),0b011 // LSH: Logic shift: sign == 0

// Min/Max operations
#define OPC_SALU_MIN(vs)   ((vs << 2) | 0b10),0b100
#define OPC_SALU_MINU(vs)  ((vs << 2) | 0b00),0b100
#define OPC_SALU_MAX(vs)   ((vs << 2) | 0b10),0b101
#define OPC_SALU_MAXU(vs)  ((vs << 2) | 0b00),0b101

// Set compare
#define OPC_SALU_LT(vs)    ((vs << 2) | 0b10),0b111 // signed compare   lt     (<)
#define OPC_SALU_LTU(vs)   ((vs << 2) | 0b00),0b111 // unsigned compare lt     (<)
#define OPC_SALU_GE(vs)    ((vs << 2) | 0b11),0b000 // signed compare   ge=~lt (>=)
#define OPC_SALU_GEU(vs)   ((vs << 2) | 0b01),0b000 // unsigned compare ge=~lt (>=)
#define OPC_SALU_EQ(vs)    ((vs << 2) | 0b01),0b001 // compare set equal
#define OPC_SALU_NE(vs)    ((vs << 2) | 0b11),0b001 // compare set not-equal

// Short immidiate major opcode
#define OPC_SALU_SHIMM(vs)   ((vs << 2) | 0b11),0b100
#define OPC_SALU_SHIMMU(vs)  ((vs << 2) | 0b01),0b100

#  define OPC_SALU_SHIMM_FN(op, n) (((op) << 2) | (n))
#  define FNC_SALU_SHIMM_CLIP(n)   OPC_SALU_SHIMM_FN(0,  ((n) & 3))   // SHIMMU/SHIMM
#  define FNC_SALU_SHIMM_RSUB(n)   OPC_SALU_SHIMM_FN(0, 0)            // SHIMMU
#  define FNC_SALU_SHIMM_ABS(n)    OPC_SALU_SHIMM_FN(0, 0)            // SHIMM
#  define FNC_SALU_SHIMM_MAX(n)    OPC_SALU_SHIMM_FN(1, 0)            // SHIMM
#  define FNC_SALU_SHIMM_EQ(n)     OPC_SALU_SHIMM_FN(1, 0)            // SHIMMU
#  define FNC_SALU_SHIMM_GE(n)     OPC_SALU_SHIMM_FN(2, 0)            // SHIMM

// Shifts - Rotate
#define OPC_SALU_ROL(vs)   ((vs << 2) | 0b01),0b101

#define OPC_SALU_CLIPZ(vs) ((vs << 2) | 0b11),0b101
#define OPC_SALU_RELU(vs)  OPC_SALU_CLIPZ(vs)
#define OPC_SALU_CLIP(vs)  ((vs << 2) | 0b11),0b110
#define OPC_SALU_CLIPU(vs) ((vs << 2) | 0b01),0b110

// Sign/Zero extend
#define OPC_SALU_SEXT(vs)  ((vs << 2) | 0b11),0b111
#define OPC_SALU_ZEXT(vs)  ((vs << 2) | 0b01),0b111


#define cr0 0


/*************** BECO ACC REG ***************/

// ACC's
typedef enum {
    BECO_ACC0,
    BECO_ACC1,
    BECO_ACC2,
    BECO_ACC3
} beco_acc_t;

// ACC+LANE's
typedef enum {
    BECO_ACC0_0,
    BECO_ACC1_0,
    BECO_ACC2_0,
    BECO_ACC3_0,
    BECO_ACC0_4,
    BECO_ACC1_4,
    BECO_ACC2_4,
    BECO_ACC3_4,
    BECO_ACC0_8,
    BECO_ACC1_8,
    BECO_ACC2_8,
    BECO_ACC3_8,
    BECO_ACC0_12,
    BECO_ACC1_12,
    BECO_ACC2_12,
    BECO_ACC3_12,
} beco_acclane_t;

// REG's
typedef enum {
    BECO_REG0 = 0,
    BECO_REG1 = 2,
    BECO_REG2 = 4,
    BECO_REG3 = 6,
    BECO_REG4 = 8,
    BECO_REG5 = 10,
    BECO_REG6 = 12,
    BECO_REG7 = 14
} beco_reg_t;

// HALF REG's
typedef enum {
    BECO_REG0L,
    BECO_REG0H,
    BECO_REG1L,
    BECO_REG1H,
    BECO_REG2L,
    BECO_REG2H,
    BECO_REG3L,
    BECO_REG3H,
    BECO_REG4L,
    BECO_REG4H,
    BECO_REG5L,
    BECO_REG5H,
    BECO_REG6L,
    BECO_REG6H,
    BECO_REG7L,
    BECO_REG7H
} beco_hreg_t;


/*************** BECO CONF ***************/

// ALUCONF

#define BECO_ALUCNF_BMODE_REP32   (0x000)   // B-MODE (v3 - backward compatible)
#define BECO_ALUCNF_BMODE_REP16   (0x100)
#define BECO_ALUCNF_BMODE_REP64   (0x001)
#define BECO_ALUCNF_BMODE_REPR64  (0x101)
#define BECO_ALUCNF_BMODE_FIR     BECO_ALUCNF_BMODE_REP64
#define BECO_ALUCNF_BMODE_MAT     BECO_ALUCNF_BMODE_REP32
#define BECO_ALUCNF_AMODE_REP32   (0 << 1)   // A-MODE
#define BECO_ALUCNF_AMODE_REP16   (2 << 1)
#define BECO_ALUCNF_AMODE_REP64   (3 << 1)
#define BECO_ALUCNF_BTYPE_UINT8   (0 << 3)   // B-TYPE
#define BECO_ALUCNF_BTYPE_UINT16  (1 << 3)
#define BECO_ALUCNF_BTYPE_INT8    (2 << 3)
#define BECO_ALUCNF_BTYPE_INT16   (3 << 3)
#define BECO_ALUCNF_ATYPE_UINT8   (0 << 5)   // A-TYPE
#define BECO_ALUCNF_ATYPE_UINT16  (1 << 5)
#define BECO_ALUCNF_ATYPE_INT8    (2 << 5)
#define BECO_ALUCNF_ATYPE_INT16   (3 << 5)
#define BECO_ALUCNF_RES1          (1 << 7)
#define BECO_ALUCNF_N_SHIFT       10

// OUTCONF

#define BECO_OUTCNF_PACK_INT8     (0 << 0)   // Pack 8x8 ALU OP into v4q or v8q output
#define BECO_OUTCNF_PACK_INT16    (1 << 0)   // Pack 8x8 or 8x16 ALU OP into v2h or v4h output
#define BECO_OUTCNF_PACK_INT32    (2 << 0)   // All ALU OP into 32 bit int or v2s
#define BECO_OUTCNF_PACK_FLOAT32  (3 << 0)   // All ALU OP into 32 float or v2f
#define BECO_OUTCNF_RSHIFT(N)     (((N) & 0x1f) << 2) // Right shift
#define BECO_OUTCNF_GLOBALROTATE  (1 << 7)   // Rotate 2x2 blocks
#define BECO_OUTCNF_LOCALROTATE   (1 << 8)   // Rotate accumulators
#define BECO_OUTCNF_UNSIGNED      (1 << 9)   // Convert to unsigned
#define BECO_OUTCNF_RELU          (1 << 9)
#define BECO_OUTCNF_SRELU         (1 << 10)
#define BECO_OUTCNF_SMARTORDER    (1 << 11)
#define BECO_OUTCNF_N_SHIFT       0

// Short hand for normal read-out order

#define BECO_OUTCNF_RD_8x8         BECO_OUTCNF_LOCALROTATE
#define BECO_OUTCNF_RD_8x8_ROT90   BECO_OUTCNF_GLOBALROTATE
#define BECO_OUTCNF_RD_8x16        0
#define BECO_OUTCNF_RD_8x16_ROT90  BECO_OUTCNF_GLOBALROTATE
#define BECO_OUTCNF_RD_16x8        BECO_OUTCNF_LOCALROTATE
#define BECO_OUTCNF_RD_16x8_ROT90  (BECO_OUTCNF_LOCALROTATE | BECO_OUTCNF_GLOBALROTATE)
#define BECO_OUTCNF_RD_16x16       0
#define BECO_OUTCNF_RD_16x16_ROT90 BECO_OUTCNF_GLOBALROTATE


// COMBINED CONF

#define BECO_CONF_BMODE_REP32   (BECO_ALUCNF_BMODE_REP32    << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BMODE_REP64   (BECO_ALUCNF_BMODE_REP64    << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BMODE_REP16   (BECO_ALUCNF_BMODE_REP16    << BECO_ALUCNF_N_SHIFT) // v3
#define BECO_CONF_BMODE_REPR64  (BECO_ALUCNF_BMODE_REPR64   << BECO_ALUCNF_N_SHIFT) // v3
#define BECO_CONF_BMODE_FIR     BECO_CONF_BMODE_REP64
#define BECO_CONF_BMODE_MAT     BECO_CONF_BMODE_REP32
#define BECO_CONF_AMODE_REP32   (BECO_ALUCNF_AMODE_REP32  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_AMODE_REP16   (BECO_ALUCNF_AMODE_REP16  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_AMODE_REP64   (BECO_ALUCNF_AMODE_REP64  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BTYPE_UINT8   (BECO_ALUCNF_BTYPE_UINT8  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BTYPE_UINT16  (BECO_ALUCNF_BTYPE_UINT16 << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BTYPE_INT8    (BECO_ALUCNF_BTYPE_INT8   << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_BTYPE_INT16   (BECO_ALUCNF_BTYPE_INT16  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_ATYPE_UINT8   (BECO_ALUCNF_ATYPE_UINT8  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_ATYPE_UINT16  (BECO_ALUCNF_ATYPE_UINT16 << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_ATYPE_INT8    (BECO_ALUCNF_ATYPE_INT8   << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_ATYPE_INT16   (BECO_ALUCNF_ATYPE_INT16  << BECO_ALUCNF_N_SHIFT)
#define BECO_CONF_PACK_INT8     (BECO_OUTCNF_PACK_INT8    << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_PACK_INT16    (BECO_OUTCNF_PACK_INT16   << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_PACK_INT32    (BECO_OUTCNF_PACK_INT32   << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_PACK_FLOAT32  (BECO_OUTCNF_PACK_FLOAT32 << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RSHIFT(N)     (BECO_OUTCNF_RSHIFT(N)    << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_GLOBALROTATE  (BECO_OUTCNF_GLOBALROTATE << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_LOCALROTATE   (BECO_OUTCNF_LOCALROTATE  << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_UNSIGNED      (BECO_OUTCNF_UNSIGNED     << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RELU          (BECO_OUTCNF_RELU         << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_SRELU         (1 << 19) // v1.3: Note: These are not in main group
#define BECO_CONF_SMARTORDER    (1 << 20) // -- " --

// Short hand for normal read-out order

#define BECO_CONF_RD_8x8         (BECO_OUTCNF_RD_8x8         << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_8x8_ROT90   (BECO_OUTCNF_RD_8x8_ROT90   << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_8x16        (BECO_OUTCNF_RD_8x16        << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_8x16_ROT90  (BECO_OUTCNF_RD_8x16_ROT90  << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_16x8        (BECO_OUTCNF_RD_16x8        << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_16x8_ROT90  (BECO_OUTCNF_RD_16x8_ROT90  << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_16x16       (BECO_OUTCNF_RD_16x16       << BECO_OUTCNF_N_SHIFT)
#define BECO_CONF_RD_16x16_ROT90 (BECO_OUTCNF_RD_16x16_ROT90 << BECO_OUTCNF_N_SHIFT)

// Masks for type and sign

#define BECO_CONF_ATYPE_SIGNED    BECO_CONF_ATYPE_INT8  // 0b10 << ats
#define BECO_CONF_BTYPE_SIGNED    BECO_CONF_BTYPE_INT8  // 0b10 << bts
#define BECO_CONF_ATYPE_MASK      BECO_CONF_ATYPE_INT16 // 0b11 << ats
#define BECO_CONF_BTYPE_MASK      BECO_CONF_BTYPE_INT16 // 0b11 << bts

#define BECO_CONF_RSHIFT_GET(v)      (((v) & BECO_CONF_RSHIFT(0xff)) >> (BECO_OUTCNF_N_SHIFT + 2))
#define BECO_CONF_PACK_MASK(v)       ((v) & (BECO_CONF_PACK_INT8 | BECO_CONF_PACK_INT16 | BECO_CONF_PACK_INT32))
#define BECO_CONF_ROTATE_MASK(v)     ((v) & (BECO_CONF_GLOBALROTATE | BECO_CONF_LOCALROTATE))
#define BECO_CONF_CONV_MASK(v)       ((v) & (BECO_CONF_SRELU | BECO_CONF_UNSIGNED))
#define BECO_CONF_SMARTORDER_MASK(v) ((v) & (BECO_CONF_SRELU | BECO_CONF_SMARTORDER))

#define BECO_CONF_GET_ATYPE_MASK(v) ((v) & (BECO_CONF_ATYPE_INT16 | BECO_CONF_ATYPE_UINT8))
#define BECO_CONF_GET_BTYPE_MASK(v) ((v) & (BECO_CONF_BTYPE_INT16 | BECO_CONF_BTYPE_UINT8))
#define BECO_CONF_AMODE_MASK(v) ((v) & (BECO_CONF_AMODE_REP16 | BECO_CONF_AMODE_REP32 | BECO_CONF_AMODE_REP64))
#define BECO_CONF_BMODE_MASK(v) ((v) & (BECO_CONF_BMODE_REP16 | BECO_CONF_BMODE_REP32 | BECO_CONF_BMODE_REP64 | BECO_CONF_BMODE_REPR64))

#define BECO_ACC_IS_SIGNED(conf) ((conf & BECO_CONF_ATYPE_SIGNED) || (conf & BECO_CONF_BTYPE_SIGNED))


// Registerfile Window Offset
#define BECO_CONF_REG_COUNT           32
#define BECO_CONF_REG_WRITE_OFFSET(N) (((N) & (BECO_CONF_REG_COUNT-1) ) <<  6)
#define BECO_CONF_REG_READ_OFFSET(N)  (((N) & (BECO_CONF_REG_COUNT-1) ) <<  0)


// Feature bits
//
#define BECO_FEATURE_
#define BECO_FEATURE_MATRIX_MULT             (1 << 0)

#define BECO_FEATURE_MACROINSTRUCTIONS       (1 << 1)
// Add u-coded instructions that operate on more registers and accumulators

// Scalar ALU

#define BECO_FEATURE_NUM_REGS_32             (1 << 2)
// Increase the number of available 64bit scalar registers to 32.
//   See beco_set_reg_window()

#define BECO_FEATURE_SALU                    (1 << 3)
// Add 64bit SIMD instructions, like ABS, MIN, CLAMP, SHIFT etc.
//   See beco_salu_and(), beco_salu_max_16(), etc

#define BECO_FEATURE_SHIFT_MULTIPLY          (1 << 4)

// Output Scale and Pack

#define BECO_FEATURE_OUT_PACK_V2             (3 << 19)

#define BECO_FEATURE_OUT_SRELU               (1 << 19)
//  Add Signed ReLU. Result clamped to largest positive signed
#define BECO_FEATURE_OUT_SMARTORDER          (1 << 20)
//  Smart order. Intelligently choose the correct accumulator ordering.
#define BECO_FEATURE_OUT_PACK_NARROW         (1 << 20)
//  Extended Pack. Pack result of 8x16, 16x8, 16x16 bit multiplies into 8, 16 and 32 bit vectors


// FFT

#define BECO_FEATURE_RADIX8_FFT             (1 << 6)
#define BECO_FEATURE_FFT_INPUTSCALE         (1 << 8)



// Error codes

#define BECO_SUCCESS         0
#define BECO_SIZE_MISMATCH   1


// Declare vector types
//     v2sf b;
//     float e, f;
//     b = (v2sf) {e, f};


typedef int8_t   v4i8   __attribute__ ((vector_size(4)));
typedef uint8_t  v4u8   __attribute__ ((vector_size(4)));
typedef int16_t  v2i16  __attribute__ ((vector_size(4)));
typedef uint16_t v2u16  __attribute__ ((vector_size(4)));

typedef int8_t   v8i8   __attribute__ ((vector_size(8)));
typedef uint8_t  v8u8   __attribute__ ((vector_size(8)));
typedef int16_t  v4i16  __attribute__ ((vector_size(8)));
typedef uint16_t v4u16  __attribute__ ((vector_size(8)));
typedef int32_t  v2i32  __attribute__ ((vector_size(8)));
typedef uint32_t v2u32  __attribute__ ((vector_size(8)));
typedef float    v2sf   __attribute__ ((vector_size(8)));

typedef union {
    v4i8     i8;
    v4u8     u8;
    v2i16    i16;
    v2u16    u16;
    float    f32;
    int32_t  i32;
    uint32_t u32;
} beco_vec32_out_t;

typedef union {
    v8i8     i8;
    v8u8     u8;
    v4i16    i16;
    v4u16    u16;
    v2i32    i32;
    v2u32    u32;
    v2sf     f32;
    uint64_t u64;
    beco_vec32_out_t v32[2];
} beco_vec64_out_t;

typedef union {
    v4i8     i8;
    v4u8     u8;
    v2i16    i16;
    v2u16    u16;
    uint32_t u32;
} beco_vec32_in_t;

typedef union {
    v8i8     i8;
    v8u8     u8;
    v4i16    i16;
    v4u16    u16;
    v2i32    i32;
    v2u32    u32;
    uint64_t u64;
    beco_vec32_in_t v32[2];
} beco_vec64_in_t;



/*************** API ***************/


#define BECO_I12_TO_CP_DMN(a) \
    ((a) >> 4) & 0xf,         \
    (a) & 0xf,                \
    ((a) >> 8) & 0xf

#ifndef MCRR
#define MCRR(ncp, opc, rt, rt2, cpm)    do {                   \
       union pack64 {uint64_t v64; uint32_t v32[2];};          \
       MCRR_Q(ncp, opc, ((union pack64){.v32={(uint32_t)rt, (uint32_t)rt2}}).v64, cpm); \
    } while(0)
#endif

#define BECO_CDP(opc, cpd, cpm, cpn)            \
        CDP(_CPNUM, opc, cpd, cpm, cpn)
#define BECO_CDP2(opc, cpd, cpm, cpn)           \
        CDP2(_CPNUM, opc, cpd, cpm, cpn)

#define BECO_CDP_IMM(opc, imm12)                \
        CDP(_CPNUM, opc, imm12)

#define BECO_MCR(opc, rt, cpm, cpn)             \
        MCR(_CPNUM, opc, rt, cpm, cpn);
#define BECO_MCR2(opc1, opc0, rt, cpm, cpn)     \
        MCR2(_CPNUM, opc1, opc0, rt, cpm, cpn)

#define SALU_MCR2_EXPAND(opc1, opc0, rt, cpm, cpn)     \
        BECO_MCR2((opc1)>>1, opc0, rt, cpm, cpn) // Discard extra OPC[4] bit
#define SALU_MCR2(opc, rt, cpm, cpn)            \
        SALU_MCR2_EXPAND(opc, rt, cpm, cpn)

#define BECO_MRC(opc, cpm, cpn)                 \
        MRC(_CPNUM, opc, cpm, cpn);

#define BECO_MRRC_R(opc, cpm)                   \
        MRRC_R(_CPNUM, opc, cpm)

#define BECO_MCRR_Q(opc, rt64, cpm)             \
        MCRR_Q(_CPNUM, opc, rt64, cpm)

#define BECO_MCRR_RR(opc, rt, rt2, cpm)         \
        MCRR(_CPNUM, opc, rt, rt2, cpm)


// Specialized instructions for FFT related instructions
#define BECO_FFT_CDP_IMM(opc, imm12)            \
        CDP(_CPNUM_FFT, opc, imm12)

#define BECO_FFT_MRRC_R(opc, cpm)               \
        MRRC_R(_CPNUM_FFT, opc, cpm)

#define BECO_FFT_MCRR_RR(opc, rt, rt2, cpm)     \
        MCRR(_CPNUM_FFT, opc, rt, rt2, cpm)


// beco_cpid
//
BECO_INLINE uint32_t beco_cpid(void)
{
    return (uint32_t)BECO_MRC(OPC_RDCPID, cr0, cr0);
}

// beco_status
//
// Read status
BECO_INLINE uint32_t beco_status(void)
{
    return (uint32_t)BECO_MRC(OPC_RDSTAT, cr0, cr0);
}

// beco_config
//
// Read config register
BECO_INLINE uint32_t beco_config(void)
{
    return (uint32_t)BECO_MRC(OPC_RDCONF, cr0, cr0);
}

// beco_read_acc
//
// Return : 4x8, 2x16, 32 or float32 accumulator
//
BECO_INLINE beco_vec32_out_t beco_read_acc(const beco_acc_t acc, const uint8_t lane)
{
    beco_vec32_out_t r;
    r.u32 = (uint32_t)BECO_MRC(OPC_RDACC, acc, lane & 0xf);
    return r;
}

// beco_next_read_acc
//
// Return : 4x8, 2x16, 32 or float32 accumulator
//
BECO_INLINE beco_vec32_out_t beco_read_next_acc(const beco_acc_t acc, const uint8_t lane)
{
    beco_vec32_out_t r;
    r.u32 = (uint32_t)BECO_MRC(OPC_RDNXACC, acc, lane & 0xf);
    return r;
}

// beco_read_reg
//
// Return : cp register contents.
//
BECO_INLINE beco_vec64_out_t  beco_read_reg(const beco_reg_t cp_src)
{
    beco_vec64_out_t r;
    r.u64 = BECO_MRRC_R(OPC_RDREG, cp_src);
    return r;
}

// beco_write_config
//
// Write configuration register
BECO_INLINE void beco_write_config(int32_t config)
{
    BECO_MCR(OPC_WRCONF, config, cr0, cr0);
}

// beco_write_reg
//
BECO_INLINE void beco_write_reg(const beco_reg_t cp_dst, beco_vec64_in_t v64)
{
    BECO_MCRR_Q(OPC_WRREG, v64.u64, cp_dst);
}

// beco_set_outconfig
//
// Set output format and packing
BECO_INLINE void beco_set_outconfig(const uint16_t outconfig)
{
    BECO_CDP_IMM(OPC_SETOCONF, BECO_I12_TO_CP_DMN(outconfig) );
}

// beco_set_accmask
//
// Set ACCMASK, 1: disable respective ACC Unit or B-input lane.
//
// Input:
//    [11:4]:  mask B-input (1: disable update)
//    [3:0]:   MACC unit mask-input (1: disable update)
//
BECO_INLINE void beco_set_accmask(const uint16_t acc_mask)
{
    BECO_CDP_IMM(OPC_SETACCMASK, BECO_I12_TO_CP_DMN(acc_mask) );
}

// beco_set_aluconfig
//
// Set input type and matrix/broadcast mode
BECO_INLINE void beco_set_aluconfig(const uint16_t aluconfig)
{
    BECO_CDP_IMM(OPC_SETACONF, BECO_I12_TO_CP_DMN(aluconfig) );
}

// beco_set_reg_window
//
// Set the scalar register window offset for read and write
BECO_INLINE void beco_set_reg_window(const uint8_t rd_offset, const uint8_t wr_offset)
{
    BECO_CDP_IMM(OPC_RFWINDOW,
        BECO_I12_TO_CP_DMN( BECO_CONF_REG_WRITE_OFFSET(wr_offset) | BECO_CONF_REG_READ_OFFSET(rd_offset) )
      );
}


BECO_INLINE uint32_t beco_feature_list_get(void)
{
    uint32_t cpid = beco_cpid();
    uint32_t features = 0;
    uint32_t cpid_minor = cpid & 0xf;

    if ((cpid & ~0xf) != 0x424d3130) {
        /* unknown coproc */;
        return 0;
    }

    switch (cpid_minor) {
    case 0:
        features = BECO_FEATURE_MATRIX_MULT;
        break;
    case 1:
        features = BECO_FEATURE_MATRIX_MULT | BECO_FEATURE_MACROINSTRUCTIONS;
        break;
    case 2:
        break;
    case 4:
        features |= beco_status() & ~0xff;
        /* fallthrough */
    case 3:
        beco_set_outconfig(BECO_OUTCNF_SMARTORDER  | BECO_OUTCNF_SRELU);
        features |= beco_config() & (BECO_CONF_SMARTORDER | BECO_CONF_SRELU);
        features |= BECO_FEATURE_MATRIX_MULT
            | BECO_FEATURE_MACROINSTRUCTIONS
            | BECO_FEATURE_NUM_REGS_32
            | BECO_FEATURE_SALU
            | BECO_FEATURE_SHIFT_MULTIPLY
            | BECO_FEATURE_RADIX8_FFT;
        break;
    default: /*error*/ ;
    }
    return features;
}

// beco_move32
//
BECO_INLINE void beco_move32(const beco_hreg_t cp_dst, const beco_hreg_t cp_src)
{
    BECO_CDP(OPC_MRTOR32, cp_dst, cp_src, 0);
}

// beco_bshift
//
BECO_INLINE void beco_bshift(
    const beco_reg_t cp_dst,
    const beco_reg_t cp_hi, const beco_reg_t cp_lo, const uint8_t N)
{
    BECO_CDP(OPC_BSHIFT(N), cp_dst, cp_lo, cp_hi);
}

#define REGNEXT(x, n) (const beco_reg_t)((int)x + 2*n)
#define HREGNEXT(x, n) (const beco_hreg_t)((int)x + n)

// beco_bshift3
//
// For normal case cp_hi = cp_lo+1, cp_dst=cp_lo
BECO_INLINE void beco_bshift3(
    const beco_reg_t cp_dst,
    const beco_reg_t cp_hi, const beco_reg_t cp_lo, const uint8_t N)
{
    BECO_CDP(OPC_BSHIFT3(N), cp_dst, cp_lo, cp_hi);
}

// beco_bshift5
//
// For normal case cp_hi = cp_lo+1, cp_dst=cp_lo
BECO_INLINE void beco_bshift5(
    const beco_reg_t cp_dst,
    const beco_reg_t cp_hi, const beco_reg_t cp_lo, const uint8_t N)
{
#ifdef BECO_PRE_V1
    beco_bshift(REGNEXT(cp_dst, 0), REGNEXT(cp_hi, 0), REGNEXT(cp_lo, 0), N);
    beco_bshift(REGNEXT(cp_dst, 1), REGNEXT(cp_hi, 1), REGNEXT(cp_lo, 1), N);
    beco_bshift(REGNEXT(cp_dst, 2), REGNEXT(cp_hi, 2), REGNEXT(cp_lo, 2), N);
    beco_bshift(REGNEXT(cp_dst, 3), REGNEXT(cp_hi, 3), REGNEXT(cp_lo, 3), N);
    beco_bshift(REGNEXT(cp_dst, 4), REGNEXT(cp_hi, 4), REGNEXT(cp_lo, 4), N);
#else
    BECO_CDP(OPC_BSHIFT5(N), cp_dst, cp_lo, cp_hi);
#endif
}

// beco_move (using bshift with shift=0)
//
BECO_INLINE void beco_move(
    const beco_reg_t cp_dst, const beco_reg_t cp_lo)
{
    beco_bshift(cp_dst, cp_lo, cp_lo, 0);
}

// beco_shift_block3
//
// alias for bshift3 normal case cp_hi = cp_lo+1, cp_dst=cp_lo
BECO_INLINE void beco_shift_block3(const beco_reg_t cp_dst, const uint8_t N)
{
    const beco_reg_t cp_p1 = (beco_reg_t)((int)cp_dst+2);
    beco_bshift3(cp_dst, cp_p1, cp_dst, N);
}

BECO_INLINE void beco_shift_block5(
    const beco_reg_t cp_dst, const uint8_t N)
{
    const beco_reg_t cp_p1 = (beco_reg_t)((int)cp_dst+2);
    beco_bshift5(cp_dst, cp_p1, cp_dst, N);
}


// FFT OPERATIONS

// beco_set_fftconfig
//
// Set the fft config register
BECO_INLINE void beco_set_fftconfig(const uint16_t fc)
{
    BECO_FFT_CDP_IMM(OPC_SETFFTCONF, BECO_I12_TO_CP_DMN(fc));
}

// beco_read_fft
//
// Read FFT result, results returned in sequence.
// Return : Two packed fft results.
//
BECO_INLINE beco_vec64_out_t  beco_read_fft(void)
{
    beco_vec64_out_t r;
    r.u64 = BECO_FFT_MRRC_R(OPC_RDFFT, 0);
    return r;
}

// beco_write_fft
//
// Write FFT block to two operands per call
//
BECO_INLINE void beco_write_fft(int32_t v32_0, int32_t v32_1)
{
    BECO_FFT_MCRR_RR(OPC_WRFFT, v32_0, v32_1, 0);
}


// ACCUMULATOR ACCESS

// beco_clear_acc
//
// Clear all accumulators in acc
BECO_INLINE void beco_clear_acc(const beco_acc_t acc)
{
    BECO_CDP(OPC_CLRACC, acc, 0, 0);
}

// beco_preload_acc
//
// Preload accumulators in acc (like clear_acc)
BECO_INLINE void beco_preload_acc(
    const beco_acc_t acc,
    const beco_reg_t cp_hi, const beco_reg_t cp_lo)
{
    BECO_CDP(OPC_PSETACC, acc, cp_hi, cp_lo);
}

// beco_set_acc_bias
//
// Set bias of single accumulator in acc
BECO_INLINE void beco_set_acc_bias(
    const beco_acclane_t acc,
    const beco_reg_t cp_hi, const beco_reg_t cp_lo)
{
    BECO_CDP(OPC_SETBIAS, acc, cp_hi, cp_lo);
}

// beco_move_acc32
//
// Calculate ACC result according to OUTCNF of one ACC
// Moving the result {<acc>.<lane>} into Beco half-register cp_dst.
BECO_INLINE void beco_move_acc32(
    const beco_hreg_t cp_dst, const beco_acc_t acc, const uint8_t lane)
{
    BECO_CDP(OPC_MATOR32, cp_dst, acc, lane);
}

// beco_move_acc64
//
// Calculate ACC result according to OUTCNF of two consecutive ACC's
// Moving the result {low: <acc>.<lane>, <acc+1>.<lane>} into Beco register cp_dst.
BECO_INLINE void beco_move_acc64(
    const beco_reg_t cp_dst, const beco_acc_t acc, const uint8_t lane)
{
    BECO_CDP(OPC_MATOR64, cp_dst, acc, lane);
}

// beco_move_next_acc32
//
// Calculate ACC result according to OUTCNF of a lane of ACC
BECO_INLINE void beco_move_next_acc32(
    const beco_hreg_t cp_dst,
    const beco_acc_t acc, const uint8_t lane)
{
    BECO_CDP(OPC_MVACC32_IB, cp_dst, acc, lane);
}

// beco_move_next_acc64
//
// Calculate ACC result according to OUTCNF of two consecutive lanes of one ACC
BECO_INLINE void beco_move_next_acc64(
    const beco_reg_t cp_dst,
    const beco_acc_t acc, const uint8_t lane)
{
    BECO_CDP(OPC_MVACC64_IB, cp_dst, acc, lane);
}


// MULTIPLY ACCUMULATE

// beco_mmacgr
//
BECO_INLINE void beco_mmacgr(
    const beco_acc_t acc, beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b)
{
    BECO_MCR(OPC_MMACGR, v32_a.u32, acc, cp_v64_b);
}

// beco_mmacgrr
//
BECO_INLINE void beco_mmacgrr(
    const beco_acc_t acc,
    beco_vec64_in_t v64_a, const beco_reg_t cp_v64_b)
{
    // Should verify that only the limited range of register is used:
    //    static_assert((int)cp_v64_b & 0b0110, "only beco-reg 0-3 allowed");

    // Pack acc and oprand register b in 4 bit {B[2:1], acc[1:0]}
    beco_reg_t packed_b_acc = (beco_reg_t)( ((int)acc) | ((int)cp_v64_b & 0b0110) << 1 );

    BECO_MCRR_Q(OPC_MMACGRR, v64_a.u64, packed_b_acc);
}

// beco_mmacgr4
//
BECO_INLINE void beco_mmacgr4( beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b)
{
#ifdef BECO_PRE_V1
    beco_mmacgr(BECO_ACC0, v32_a, REGNEXT(cp_v64_b, 0));
    beco_mmacgr(BECO_ACC1, v32_a, REGNEXT(cp_v64_b, 1));
    beco_mmacgr(BECO_ACC2, v32_a, REGNEXT(cp_v64_b, 2));
    beco_mmacgr(BECO_ACC3, v32_a, REGNEXT(cp_v64_b, 3));
#else
    BECO_MCR(OPC_MMACGR4, v32_a.u32,  BECO_ACC0, cp_v64_b);
#endif
}

// beco_mmac
//
BECO_INLINE void beco_mmac(
    const beco_acc_t acc, beco_vec32_in_t v32_a, beco_vec32_in_t v32_b)
{
    BECO_MCRR_RR(OPC_MMAC, v32_a.u32, v32_b.u32, acc);
}

// beco_mmacr
//
BECO_INLINE void beco_mmacr(
    const beco_acc_t acc, const beco_hreg_t cp_vec32_a, const beco_hreg_t cp_vec32_b)
{
    BECO_CDP(OPC_MMACR, acc, cp_vec32_a, cp_vec32_b);
}

// beco_mmacrr
//
BECO_INLINE void beco_mmacrr(const beco_reg_t cp_vec64_a, const beco_reg_t cp_vec64_b)
{
#ifdef BECO_PRE_V1
    beco_mmacr(BECO_ACC0, HREGNEXT(cp_vec64_a, 0), HREGNEXT(cp_vec64_b, 0));
    beco_mmacr(BECO_ACC1, HREGNEXT(cp_vec64_a, 0), HREGNEXT(cp_vec64_b, 1));
    beco_mmacr(BECO_ACC2, HREGNEXT(cp_vec64_a, 1), HREGNEXT(cp_vec64_b, 0));
    beco_mmacr(BECO_ACC3, HREGNEXT(cp_vec64_a, 1), HREGNEXT(cp_vec64_b, 1));
#undef ENUMNEXT
#else
    BECO_CDP(OPC_MMACRR, BECO_ACC0, cp_vec64_a, cp_vec64_b);
#endif
}


// Beco v3

// beco_mmacrr_nsq
//
// Calculate  ACC_n += REG_{a+i} * REG_{b+j}, for n = 0..3, i=n*ainc, j=n*binc
// All 4 accumulators are updated.
// (4 cycle macro instruction)
BECO_INLINE void beco_mmacrr_nsq(const beco_reg_t cp_vec64_a, const beco_reg_t cp_vec64_b)
{
    BECO_CDP(OPC_MMACRRNSQ, BECO_ACC0, cp_vec64_a, cp_vec64_b);
}

// beco_mmacgr_shift
//
// Broadcast multiply with shift
//
// * Read registers cp_b, cp_b+1, shift {cp_b+1, cp_b} right by 8*N,
//   then execute beco_mmacgr
BECO_INLINE void beco_mmacgr_shift(
    const beco_acc_t acc,
    beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b,
    const int8_t N)
{
    BECO_MCR(OPC_MMACGR_S(N), v32_a.u32, acc, cp_v64_b);
}

// beco_mmacgr_shift_wb
//
// Broadcast multiply with shift and writeback
//
// * Read registers cp_b, cp_b+1, shift {cp_b+1, cp_b} right by 8*N, write-back
//   result to cp_b, then execute beco_mmacgr
BECO_INLINE void beco_mmacgr_shift_wb(
    const beco_acc_t acc,
    beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b,
    const int8_t N)
{
    BECO_MCR(OPC_MMACGR_SWB(N), v32_a.u32, acc, cp_v64_b);
}

// beco_mmacgr4_shift
//
// Repeat 4 times: beco_mmacgr_shift
// * Read registers cp_b, cp_b+1, shift {cp_b+1, cp_b} right by 8*N
//   then execute beco_mmacgr
BECO_INLINE void beco_mmacgr4_shift(
    //const beco_acc_t acc,
    beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b,
    const int8_t N)
{
    BECO_MCR(OPC_MMACGR4_S(N), v32_a.u32, BECO_ACC0, cp_v64_b);
}

// beco_mmacgr4_shift_wb
//
// Repeat 4 times:
// * Read registers cp_b, cp_b+1, shift {cp_b+1, cp_b} right by 8*N, write-back
//   result to cp_b, then execute beco_mmacgr
BECO_INLINE void beco_mmacgr4_shift_wb(
    //const beco_acc_t acc,
    beco_vec32_in_t v32_a, const beco_reg_t cp_v64_b,
    const int8_t N)
{
    BECO_MCR(OPC_MMACGR4_SWB(N), v32_a.u32, BECO_ACC0, cp_v64_b);
}


// Include Salu instruction bindings
#include "beco_salu.h"


BECO_C_DECLARATIONS_END

#endif

