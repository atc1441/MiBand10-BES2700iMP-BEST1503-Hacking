// Copyright 2020 BES Technic
//
// Description: Test beco instruction patterns.
//
//#include <stdint.h>
//#include <stdio.h>
//#include <math.h>

#include "beco.h"
#include "beco_l1.h"
#include "beco_read.h"

// Use beco_l1 to Generate the beco_read_nxm functions
//
// All have prototype:
//
//     void beco_read_xxxx(beco_vec32_out_t rd[], size_t stride);

//
// Functions to read-out results from individual ACC's
// (for 32bit times 32bit input vectors)
//
#include "read_intrinsics.c"

#define BECO_ALIAS(fn, wafn) void wafn(beco_vec32_out_t rd[], size_t stride) \
                                        __attribute__ ((weak, alias (#fn)));

// 8x8
//
// In 8x8 bit mode, each acc hold one result.  A total 4x4 results.
// The output element size is either 8, 16 or 32 bit. Each read thus pack
// 1, 2 or 4 elements.

// Generate function for 4 packed 8 bit results with prototype:
BECO_ALIAS(beco_read_acc0_1x4, beco_read_acc0_8x8_8bit)
BECO_ALIAS(beco_read_acc1_1x4, beco_read_acc1_8x8_8bit)
BECO_ALIAS(beco_read_acc2_1x4, beco_read_acc2_8x8_8bit)
BECO_ALIAS(beco_read_acc3_1x4, beco_read_acc3_8x8_8bit)

// Generate function for 2 packed 16 bit results with prototype:
BECO_ALIAS(beco_read_acc0_2x4, beco_read_acc0_8x8_16bit)
BECO_ALIAS(beco_read_acc1_2x4, beco_read_acc1_8x8_16bit)
BECO_ALIAS(beco_read_acc2_2x4, beco_read_acc2_8x8_16bit)
BECO_ALIAS(beco_read_acc3_2x4, beco_read_acc3_8x8_16bit)

// Generate function for 1 32 bit results with prototype:
BECO_ALIAS(beco_read_acc0_4x4, beco_read_acc0_8x8_32bit)
BECO_ALIAS(beco_read_acc1_4x4, beco_read_acc1_8x8_32bit)
BECO_ALIAS(beco_read_acc2_4x4, beco_read_acc2_8x8_32bit)
BECO_ALIAS(beco_read_acc3_4x4, beco_read_acc3_8x8_32bit)



// 16x16
//
// In 16x16 bit mode, each result require all four accs in the MU. A total
// of 2x2 results.
// The output is element size can only be 32 bit, each read thus return 1 element.

// Generate function for 32 bit result readout with prototype:
BECO_ALIAS(beco_read_acc0_2x2, beco_read_acc0_16x16_32bit)
BECO_ALIAS(beco_read_acc1_2x2, beco_read_acc1_16x16_32bit)
BECO_ALIAS(beco_read_acc2_2x2, beco_read_acc2_16x16_32bit)
BECO_ALIAS(beco_read_acc3_2x2, beco_read_acc3_16x16_32bit)



// 16x8
//
// In 16x8 bit mode, each result require two accs from each MU. A total of
// 2x4 results.
// The output element size is either 16 or 32 bit. Each read thus pack 1
// or 2 elements.

// The rotation affect the output to 4x2 results 
//
// NOTE: 16x8 always need LOCAL_ROTATE mode to align partial sum accumulators.
// NOTE: 16x8 always need ROT90 is readout is rotated with GLOBAL_ROTATE mode.


// Generate function for 32 bit readout with prototype:
BECO_ALIAS(beco_read_acc0_2x4, beco_read_acc0_16x8_32bit)
BECO_ALIAS(beco_read_acc1_2x4, beco_read_acc1_16x8_32bit)
BECO_ALIAS(beco_read_acc2_2x4, beco_read_acc2_16x8_32bit)
BECO_ALIAS(beco_read_acc3_2x4, beco_read_acc3_16x8_32bit)


// Generate function for 32 bit rotated readout with prototype:
BECO_ALIAS(beco_read_acc0_4x2, beco_read_acc0_16x8_32bit_rot90)
BECO_ALIAS(beco_read_acc1_4x2, beco_read_acc1_16x8_32bit_rot90)
BECO_ALIAS(beco_read_acc2_4x2, beco_read_acc2_16x8_32bit_rot90)
BECO_ALIAS(beco_read_acc3_4x2, beco_read_acc3_16x8_32bit_rot90)


// Generate function for 2 packed 16 bit readout with prototype :
BECO_ALIAS(beco_read_acc0_1x4, beco_read_acc0_16x8_16bit)
BECO_ALIAS(beco_read_acc1_1x4, beco_read_acc1_16x8_16bit)
BECO_ALIAS(beco_read_acc2_1x4, beco_read_acc2_16x8_16bit)
BECO_ALIAS(beco_read_acc3_1x4, beco_read_acc3_16x8_16bit)

// Generate function for 2 packed 16 bit rotated readout with prototype :
BECO_ALIAS(beco_read_acc0_2x2, beco_read_acc0_16x8_16bit_rot90)
BECO_ALIAS(beco_read_acc1_2x2, beco_read_acc1_16x8_16bit_rot90)
BECO_ALIAS(beco_read_acc2_2x2, beco_read_acc2_16x8_16bit_rot90)
BECO_ALIAS(beco_read_acc3_2x2, beco_read_acc3_16x8_16bit_rot90)



// 8x16
//
// In 8x16 bit mode, each result require two accs from each MU. A total of
// 4x2 results.
// The output element size is either 16 or 32 bit. Each read thus pack 1
// or 2 elements.

// The rotation affect the output. After rotation size is 2x4 results 
//
// NOTE: 8x16 can not use LOCAL_ROTATE mode.
// NOTE: 8x16 always need ROT90 is readout is rotated with GLOBAL_ROTATE mode.

// Generate function for 32 bit non-rotated readout with prototype :
BECO_ALIAS(beco_read_acc0_4x2, beco_read_acc0_8x16_32bit)
BECO_ALIAS(beco_read_acc1_4x2, beco_read_acc1_8x16_32bit)
BECO_ALIAS(beco_read_acc2_4x2, beco_read_acc2_8x16_32bit)
BECO_ALIAS(beco_read_acc3_4x2, beco_read_acc3_8x16_32bit)

// Generate function for 32 bit rotated readout with prototype :
BECO_ALIAS(beco_read_acc0_2x4, beco_read_acc0_8x16_32bit_rot90)
BECO_ALIAS(beco_read_acc1_2x4, beco_read_acc1_8x16_32bit_rot90)
BECO_ALIAS(beco_read_acc2_2x4, beco_read_acc2_8x16_32bit_rot90)
BECO_ALIAS(beco_read_acc3_2x4, beco_read_acc3_8x16_32bit_rot90)

// Generate function for 2 packed 16 bit non-rotated readout with prototype :
BECO_ALIAS(beco_read_acc0_2x2, beco_read_acc0_8x16_16bit)
BECO_ALIAS(beco_read_acc1_2x2, beco_read_acc1_8x16_16bit)
BECO_ALIAS(beco_read_acc2_2x2, beco_read_acc2_8x16_16bit)
BECO_ALIAS(beco_read_acc3_2x2, beco_read_acc3_8x16_16bit)

// Generate function for 2 packed 16 bit rotated readout with prototype :
BECO_ALIAS(beco_read_acc0_1x4, beco_read_acc0_8x16_16bit_rot90)
BECO_ALIAS(beco_read_acc1_1x4, beco_read_acc1_8x16_16bit_rot90)
BECO_ALIAS(beco_read_acc2_1x4, beco_read_acc2_8x16_16bit_rot90)
BECO_ALIAS(beco_read_acc3_1x4, beco_read_acc3_8x16_16bit_rot90)

// Create alias
BECO_ALIAS(beco_read_all_8x8, beco_read_all_8x8_32bit)
BECO_ALIAS(beco_read_all_4x8, beco_read_all_8x8_16bit)
BECO_ALIAS(beco_read_all_2x8, beco_read_all_8x8_8bit)

BECO_ALIAS(beco_read_all_8x4, beco_read_all_16x8_32bit)
BECO_ALIAS(beco_read_all_4x4, beco_read_all_16x8_16bit)

BECO_ALIAS(beco_read_all_4x8, beco_read_all_8x16_32bit)
BECO_ALIAS(beco_read_all_2x8, beco_read_all_8x16_16bit)

BECO_ALIAS(beco_read_all_4x4, beco_read_all_16x16_32bit)



