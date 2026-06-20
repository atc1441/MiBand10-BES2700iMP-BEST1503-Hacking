#include "beco.h"
#include "beco_l1.h"
#include "beco_read.h"

BECO_GEN_READ_DEF(0, 1x4) // generate the `beco_read_acc0_1x4()` function
BECO_GEN_READ_DEF(0, 2x2) // generate the `beco_read_acc0_2x2()` function
BECO_GEN_READ_DEF(0, 2x4) // generate the `beco_read_acc0_2x4()` function
BECO_GEN_READ_DEF(0, 4x2) // generate the `beco_read_acc0_4x2()` function
BECO_GEN_READ_DEF(0, 4x4) // generate the `beco_read_acc0_4x4()` function

BECO_GEN_READ_DEF(1, 1x4) // generate the `beco_read_acc1_1x4()` function
BECO_GEN_READ_DEF(1, 2x2) // generate the `beco_read_acc1_2x2()` function
BECO_GEN_READ_DEF(1, 2x4) // generate the `beco_read_acc1_2x4()` function
BECO_GEN_READ_DEF(1, 4x2) // generate the `beco_read_acc1_4x2()` function
BECO_GEN_READ_DEF(1, 4x4) // generate the `beco_read_acc1_4x4()` function

BECO_GEN_READ_DEF(2, 1x4) // generate the `beco_read_acc2_1x4()` function
BECO_GEN_READ_DEF(2, 2x2) // generate the `beco_read_acc2_2x2()` function
BECO_GEN_READ_DEF(2, 2x4) // generate the `beco_read_acc2_2x4()` function
BECO_GEN_READ_DEF(2, 4x2) // generate the `beco_read_acc2_4x2()` function
BECO_GEN_READ_DEF(2, 4x4) // generate the `beco_read_acc2_4x4()` function

BECO_GEN_READ_DEF(3, 1x4) // generate the `beco_read_acc3_1x4()` function
BECO_GEN_READ_DEF(3, 2x2) // generate the `beco_read_acc3_2x2()` function
BECO_GEN_READ_DEF(3, 2x4) // generate the `beco_read_acc3_2x4()` function
BECO_GEN_READ_DEF(3, 4x2) // generate the `beco_read_acc3_4x2()` function
BECO_GEN_READ_DEF(3, 4x4) // generate the `beco_read_acc3_4x4()` function

//
// Functions to read-out results from combined ACC's
// (for 64bit times 64bit input vectors)
//
//   BECO_GEN_READ_ALL_DEF(2x2) - to generate the `beco_read_all_2x2()` function
//   BECO_GEN_READ_ALL_DEF(1x4) - to generate the `beco_read_all_1x4()` function
//   BECO_GEN_READ_ALL_DEF(2x4_rot90) - to generate the `beco_read_all_2x4_rot90()` function

BECO_GEN_READ_ALL_DEF(8x8) // generate the `beco_read_all_8x4()` function
BECO_GEN_READ_ALL_DEF(4x8) // generate the `beco_read_all_4x8()` function
BECO_GEN_READ_ALL_DEF(2x8) // generate the `beco_read_all_2x8()` function
BECO_GEN_READ_ALL_DEF(8x4) // generate the `beco_read_all_8x4()` function
BECO_GEN_READ_ALL_DEF(4x4) // generate the `beco_read_all_4x4()` function
/*
BECO_GEN_READ_ALL_8x8(beco_read_all_8x8_32bit)
BECO_GEN_READ_ALL_4x8(beco_read_all_8x8_16bit)
BECO_GEN_READ_ALL_2x8(beco_read_all_8x8_8bit)

BECO_GEN_READ_ALL_8x4(beco_read_all_16x8_32bit)
BECO_GEN_READ_ALL_4x4(beco_read_all_16x8_16bit)

BECO_GEN_READ_ALL_4x8(beco_read_all_8x16_32bit)
BECO_GEN_READ_ALL_2x8(beco_read_all_8x16_16bit)

BECO_GEN_READ_ALL_4x4(beco_read_all_16x16_32bit)
*/
