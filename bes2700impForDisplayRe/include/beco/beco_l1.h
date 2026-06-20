#pragma once
//
// Copyright 2020 BES Technic
//
// C-preprocessor iterator and list abstraction.

#ifndef _BECO_L1_H
#define _BECO_L1_H

#include "beco_common.h"

BECO_C_DECLARATIONS_START

#include <stddef.h>
#include "macro_iterator.h"

#if 0
// 8x8
//
// In 8x8 bit mode, each acc hold one result.  A total 4x4 results.
// The output element size is either 8, 16 or 32 bit. Each read thus pack
// 1, 2 or 4 elements.
//
// Generate function for 4 packed 8 bit results with prototype:
//FIXME    void beco_read_acc0_8x8_8bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x8_8bit(a,b)         beco_read_acc0_1x4(a,b)

// Generate function for 2 packed 16 bit results with prototype:
//FIXME    void beco_read_acc0_8x8_16bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x8_16bit(a,b)         beco_read_acc0_2x4(a,b)
#define beco_read_acc0_8x8_32bit(a,b)         beco_read_acc0_4x4(a,b)


// 16x16
//
// In 16x16 bit mode, each result require all four accs in the MU. A total
// of 2x2 results.
// The output is element size is only 32 bit. Each read thus return 1 element.
//
// Generate function for 32 bit result readout with prototype :
//FIXME    void beco_read_acc0_16x16_32bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_16x16_32bit(a,b)       beco_read_acc0_2x2(a,b)


// 16x8
//
// In 16x8 bit mode, each result require two accs from each MU. A total of
// 2x4 results.
// The output element size is either 16 or 32 bit. Each read thus pack 1
// or 2 elements.

// The rotation affect the output to 4x2 results 

// NOTE: 16x8 always need LOCAL_ROTATE mode to align partial sum accumulators.
// NOTE: 16x8 always need ROT90 is readout is rotated with GLOBAL_ROTATE mode.

//
// Generate function for 32 bit readout with prototype:
//FIXME    void beco_read_acc0_16x8_32bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_16x8_32bit(a,b)        beco_read_acc0_2x4(a,b)

// Generate function for 32 bit rotated readout with prototype:
//FIXME    void beco_read_acc0_16x8_32bit_rot90(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_16x8_32bit_rot90(a,b)  beco_read_acc0_4x2(a,b)

// Generate function for 2 packed 16 bit readout with prototype :
//FIXME    void beco_read_acc0_16x8_16bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_16x8_16bit(a,b)        beco_read_acc0_1x4(a,b)

// Generate function for 2 packed 16 bit rotated readout with prototype :
//FIXME    void beco_read_acc0_16x8_16bit_rot90(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_16x8_16bit_rot90(a,b)  beco_read_acc0_2x2(a,b)



// 8x16
//
// In 8x16 bit mode, each result require two accs from each MU. A total of
// 4x2 results.
// The output element size is either 16 or 32 bit. Each read thus pack 1
// or 2 elements.

// The rotation affect the output. After rotation size is 2x4 results 

// NOTE: 8x16 can not use LOCAL_ROTATE mode.
// NOTE: 8x16 always need ROT90 is readout is rotated with GLOBAL_ROTATE mode.

// Generate function for 32 bit non-rotated readout with prototype :
//FIXME    void beco_read_acc0_8x16_32bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x16_32bit(a,b)        beco_read_acc0_4x2(a,b)

// Generate function for 32 bit rotated readout with prototype :
//FIXME    void beco_read_acc0_8x16_32bit_rot90(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x16_32bit_rot90(a,b)  beco_read_acc0_2x4(a,b)

// Generate function for 2 packed 16 bit non-rotated readout with prototype :
//FIXME    void beco_read_acc0_8x16_16bit(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x16_16bit(a,b)        beco_read_acc0_2x2(a,b)

// Generate function for 2 packed 16 bit rotated readout with prototype :
//FIXME    void beco_read_acc0_8x16_16bit_rot90(beco_vec32_out_t rd[], size_t stride);
//
#define beco_read_acc0_8x16_16bit_rot90(a,b)  beco_read_acc0_1x4(a,b)
#endif

//
// Define Codegen macros for Beco accumulator readout.
//
#define BECO_SEQUENCE_SINGLE_ACC(acc) ((0, acc))
#define BECO_SEQUENCE_ALL_ACC(n,m, stride)        \
                      ((0,          BECO_ACC0),   \
                       (n,          BECO_ACC1),   \
                       (  m*stride, BECO_ACC2),   \
                       (n+m*stride, BECO_ACC3))
#define BECO_SEQUENCE_ALL_ACC_ROT90(n,m, stride)  \
                      ((0,          BECO_ACC0),   \
                       (n,          BECO_ACC2),   \
                       (  m*stride, BECO_ACC1),   \
                       (n+m*stride, BECO_ACC3))

#define OUT_READ(lane,acc) \
    out[P1 lane + P1 acc] = beco_read_acc(P2 acc, P2 lane);

#define READ_INNER(nn, seq) \
    FORALL(OUT_READ, nn, EXPAND seq)

#define BECO_GEN_READ_CODE(accs, seq)                  \
        EVAL(FORALL(READ_INNER, seq, EXPAND accs))     \


#define BECO_SEQUENCE_OFFSET_2x2(stride)             ( \
		(0 + 0*stride, 0), (1 + 0*stride,  4), \
		(0 + 1*stride, 8), (1 + 1*stride, 12) )
#define BECO_SEQUENCE_OFFSET_1x4(stride)             (\
		(0 + 0*stride, 0),                    \
		(0 + 1*stride, 4),                    \
		(0 + 2*stride, 8),                    \
		(0 + 3*stride,12) )
#define BECO_SEQUENCE_OFFSET_2x4(stride)              (\
		(0 + 0*stride, 0), (1 + 0*stride, 2), \
		(0 + 1*stride, 4), (1 + 1*stride, 6), \
		(0 + 2*stride, 8), (1 + 2*stride,10), \
		(0 + 3*stride,12), (1 + 3*stride,14) )
#define BECO_SEQUENCE_OFFSET_4x2(stride)             (\
		(0+0*stride,0), (1+0*stride,2),  (2+0*stride,4), (3+0*stride,6),\
		(0+1*stride,8), (1+1*stride,10), (2+1*stride,12), (3+1*stride,14) )
#define BECO_SEQUENCE_OFFSET_4x4(stride)             (\
		(0 + 0*stride, 0), (1 + 0*stride, 1), (2 + 0*stride, 2), (3 + 0*stride, 3),\
		(0 + 1*stride, 4), (1 + 1*stride, 5), (2 + 1*stride, 6), (3 + 1*stride, 7),\
		(0 + 2*stride, 8), (1 + 2*stride, 9), (2 + 2*stride,10), (3 + 2*stride,11),\
		(0 + 3*stride,12), (1 + 3*stride,13), (2 + 3*stride,14), (3 + 3*stride,15) )

//
// Define factories to generate Beco Readout functions
//
#define BECO_GEN_READ_2x2_ACC(name, acc)                   \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_2x2(stride)) \
	}

#define BECO_GEN_READ_1x4_ACC(name, acc) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_1x4(stride)) \
	}

#define BECO_GEN_READ_2x4_ACC(name, acc) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_2x4(stride)) \
	}

#define BECO_GEN_READ_4x2_ACC(name, acc) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_4x2(stride)) \
	}

#define BECO_GEN_READ_4x4_ACC(name, acc) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_4x4(stride)) \
	}

//
// Fixed stride offset versions
//
#define BECO_GEN_READ_2x2_CONST_STRIDE_ACC(name, acc, stride)             \
	void name(beco_vec32_out_t out[]) {                               \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_2x2(stride)) \
	}

#define BECO_GEN_READ_1x4_CONST_STRIDE_ACC(name, acc, stride)             \
	void name(beco_vec32_out_t out[]) {                               \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_1x4(stride)) \
	}

#define BECO_GEN_READ_2x4_CONST_STRIDE_ACC(name, acc, stride)             \
	void name(beco_vec32_out_t out[]) {                               \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_2x4(stride)) \
	}

#define BECO_GEN_READ_4x2_CONST_STRIDE_ACC(name, acc, stride)             \
	void name(beco_vec32_out_t out[]) {                               \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_4x2(stride)) \
	}

#define BECO_GEN_READ_4x4_CONST_STRIDE_ACC(name, acc, stride)             \
	void name(beco_vec32_out_t out[]) {                               \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_SINGLE_ACC(acc), BECO_SEQUENCE_OFFSET_4x4(stride)) \
	}

//
// Macro to generate default readout functions for single accumulators 
//
#define BECO_GEN_READ_DEF(acc, shape) \
	BECO_GEN_READ_##shape ##_ACC(beco_read_acc ## acc ##_## shape, BECO_ACC ## acc)
//
// Use
//   BECO_GEN_READ_DEF(0, 2x2) - to generate the `beco_read_acc0_2x2()` function
//   BECO_GEN_READ_DEF(1, 1x4) - to generate the `beco_read_acc1_1x4()` function
//   BECO_GEN_READ_DEF(2, 2x4) - to generate the `beco_read_acc2_2x4()` function
//   BECO_GEN_READ_DEF(3, 4x2) - to generate the `beco_read_acc3_4x2()` function
//   BECO_GEN_READ_DEF(0, 4x4) - to generate the `beco_read_acc0_4x4()` function

//
// Macro to generate default readout functions for single accumulators, const stride
//
#define BECO_GEN_READ_CONST_STRIDE_DEF(acc, shape, stride)      \
	BECO_GEN_READ_## shape ## _CONST_STRIDE ##_ACC(         \
		beco_read_s ## stride ##_acc ## acc ##_## shape,\
		BECO_ACC ## acc,                                \
		stride)
//
// Use
//   BECO_GEN_READ_CONST_STRIDE_DEF(0, 2x2, 32) - to generate the `beco_read_s32_acc0_2x2()` function
//   etc...




//
// Macro to generate default readout functions for combined accumulators, variable stride
//
#define BECO_GEN_READ_ALL_2x8(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC(1,4,stride), BECO_SEQUENCE_OFFSET_1x4(stride)) \
	}

#define BECO_GEN_READ_ALL_4x4(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC(2,2,stride), BECO_SEQUENCE_OFFSET_2x2(stride)) \
	}

#define BECO_GEN_READ_ALL_8x4(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC(4,2,stride), BECO_SEQUENCE_OFFSET_4x2(stride)) \
	}

#define BECO_GEN_READ_ALL_4x8(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC(2,4,stride), BECO_SEQUENCE_OFFSET_2x4(stride)) \
	}

#define BECO_GEN_READ_ALL_8x8(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC(4,4,stride), BECO_SEQUENCE_OFFSET_4x4(stride)) \
	}




//
// Macro to generate default readout functions for combined accumulators, variable stride, rot90
// Only difference between this and non-rotate is the order of acc packing. 
//
#define BECO_GEN_READ_ALL_2x8_rot90(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(1,4,stride), BECO_SEQUENCE_OFFSET_1x4(stride)) \
	}

#define BECO_GEN_READ_ALL_4x4_rot90(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(2,2,stride), BECO_SEQUENCE_OFFSET_2x2(stride)) \
	}

#define BECO_GEN_READ_ALL_8x4_rot90(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(4,2,stride), BECO_SEQUENCE_OFFSET_4x2(stride)) \
	}

#define BECO_GEN_READ_ALL_4x8_rot90(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(2,4,stride), BECO_SEQUENCE_OFFSET_2x4(stride)) \
	}

#define BECO_GEN_READ_ALL_8x8_rot90(name) \
	void name(beco_vec32_out_t out[], size_t stride) {                \
	    BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(4,4,stride), BECO_SEQUENCE_OFFSET_4x4(stride)) \
	}


//
// Macro to generate default readout functions for single accumulators 
//
#define BECO_GEN_READ_ALL_DEF(shape) \
	BECO_GEN_READ_ALL_##shape(beco_read_all_ ## shape)

//
// Use
//   BECO_GEN_READ_ALL_DEF(2x2) - to generate the `beco_read_all_2x2()` function
//   BECO_GEN_READ_ALL_DEF(1x4) - to generate the `beco_read_all_1x4()` function
//   BECO_GEN_READ_ALL_DEF(2x4_rot90) - to generate the `beco_read_all_2x4_rot90()` function

BECO_C_DECLARATIONS_END

#endif /*_BECO_L1_H*/
