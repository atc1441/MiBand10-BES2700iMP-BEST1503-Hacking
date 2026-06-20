#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Support functions to readout micro-tile results

#ifndef _BECO_READOUT_HPP
#define _BECO_READOUT_HPP

#include "beco_common.h"
#include "beco_types.h"

#include <stddef.h>

#include "beco_l1.h"

extern "C" {
BECO_GEN_READ_ALL_2x8_rot90(beco_conv_read_2x8_32bit);
BECO_GEN_READ_ALL_4x4_rot90(beco_conv_read_4x4_32bit);
BECO_GEN_READ_ALL_4x8_rot90(beco_conv_read_4x8_32bit);
BECO_GEN_READ_ALL_8x8_rot90(beco_conv_read_8x8_32bit);
/*
BECO_GEN_READ_ALL_1x16_rot90(beco_conv_read_1x16_32bit);
BECO_GEN_READ_ALL_2x16_rot90(beco_conv_read_2x16_32bit);
BECO_GEN_READ_ALL_4x16_rot90(beco_conv_read_4x16_32bit);
BECO_GEN_READ_ALL_1x32_rot90(beco_conv_read_1x32_32bit);
BECO_GEN_READ_ALL_2x32_rot90(beco_conv_read_2x32_32bit);
*/
};

/*
   void read_accs() {
   BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(2,2,stride), BECO_SEQUENCE_OFFSET_2x2(stride));
   }
   void read_1x16() {
   BECO_GEN_READ_CODE(BECO_SEQUENCE_ALL_ACC_ROT90(1,4,stride), BECO_SEQUENCE_OFFSET_1x4(stride))
   }
*/

namespace Beco {

    class ReadAcc {
        template <int chanWidth, int SIMDWidth> struct ReadSel {
            enum { w = chanWidth, h = SIMDWidth };
    };


public:

    static void beco_conv_read_acc_impl(beco_vec32_out_t *tile, ReadSel<16,4> s)
    {
        typedef typeof(s) sel;
        //printf("beco_conv_read_4x4_32bit: %d %d\n", sel::w, sel::h);

        beco_conv_read_4x4_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(beco_vec32_out_t *tile, ReadSel<8,8> s)
    {
        typedef typeof(s) sel;
        //printf("beco_conv_read_2x8_32bit: %d %d\n", sel::w, sel::h);

        beco_conv_read_2x8_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(beco_vec32_out_t *tile, ReadSel<16,8> s)
    {
        typedef typeof(s) sel;
        //printf("beco_conv_read_4x8_32bit: %d %d\n", sel::w, sel::h);

        beco_conv_read_4x8_32bit(tile, sel::w/sizeof(*tile));
    }

    static inline void beco_conv_read_acc_impl(beco_vec32_out_t *tile, ReadSel<32,8> s)
    {
        typedef typeof(s) sel;
        //printf("beco_conv_read_8x8_32bit: %d %d\n", sel::w, sel::h);

        beco_conv_read_8x8_32bit(tile, sel::w/sizeof(*tile));
    }

#if 0
    // 8bit - 4 channels 16 wide
    static void beco_conv_read_acc_impl(beco_vec32_out_t *tile, ReadSel<4,16> s)
    {
        typedef typeof(s) sel;
        beco_conv_read_1x16_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(int16_t *tile, ReadSel<8,16> s)
    {
        typedef typeof(s) sel;
        beco_conv_read_2x16_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(int32_t *tile, ReadSel<16,16> s)
    {
        typedef typeof(s) sel;
        beco_conv_read_4x16_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(int16_t *tile, ReadSel<4,32> s)
    {
        typedef typeof(s) sel;
        beco_conv_read_1x32_32bit(tile, sel::w/sizeof(*tile));
    }

    static void beco_conv_read_acc_impl(int32_t *tile, ReadSel<8,32> s)
    {
        typedef typeof(s) sel;
        beco_conv_read_2x32_32bit(tile, sel::w/sizeof(*tile));
    }
#endif


public:

    template <int numBundledAccs, int chanPerBatch, int SIMDWidth, typename T>
        static void beco_conv_read_acc(T *tile)
    {
        beco_conv_read_acc_impl((beco_vec32_out_t *)tile,
            ReadSel<chanPerBatch*sizeof(T), SIMDWidth>());
    }

};

};

#endif
