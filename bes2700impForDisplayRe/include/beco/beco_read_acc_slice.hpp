/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

static void beco_mmacgr_read_8_8bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i8[0] = beco_read_acc(BECO_ACC0, 0).i8[0];
    p_out[0].i8[1] = beco_read_acc(BECO_ACC0, 2).i8[0];
    p_out[0].i8[2] = beco_read_acc(BECO_ACC0, 8).i8[0];
    p_out[0].i8[3] = beco_read_acc(BECO_ACC0, 10).i8[0];

    p_out[1].i8[0] = beco_read_acc(BECO_ACC0, 4).i8[0];
    p_out[1].i8[1] = beco_read_acc(BECO_ACC0, 6).i8[0];
    p_out[1].i8[2] = beco_read_acc(BECO_ACC0, 12).i8[0];
    p_out[1].i8[3] = beco_read_acc(BECO_ACC0, 14).i8[0];
}

static void beco_mmacgr_read_8_16bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    p_out[0].i16[1] = beco_read_acc(BECO_ACC0, 2).i16[0];
    p_out[1].i16[0] = beco_read_acc(BECO_ACC0, 8).i16[0];
    p_out[1].i16[1] = beco_read_acc(BECO_ACC0, 10).i16[0];
    p_out[2].i16[0] = beco_read_acc(BECO_ACC0, 4).i16[0];
    p_out[2].i16[1] = beco_read_acc(BECO_ACC0, 6).i16[0];
    p_out[3].i16[0] = beco_read_acc(BECO_ACC0, 12).i16[0];
    p_out[3].i16[1] = beco_read_acc(BECO_ACC0, 14).i16[0];
}

static void beco_mmacgr_read_16_16bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    p_out[0].i16[1] = beco_read_acc(BECO_ACC0, 8).i16[0];
    p_out[1].i16[0] = beco_read_acc(BECO_ACC0, 4).i16[0];
    p_out[1].i16[1] = beco_read_acc(BECO_ACC0, 12).i16[0];
}

static void beco_mmacgr_read_16_32bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0] = beco_read_acc(BECO_ACC0, 0);
    p_out[1] = beco_read_acc(BECO_ACC0, 8);
    p_out[2] = beco_read_acc(BECO_ACC0, 4);
    p_out[3] = beco_read_acc(BECO_ACC0, 12);
    q31_t *out = (q31_t *)p_out;
    for (int i = 0; i < 4; i++) {
        if (out[i] > 67108864) {
            out[i] += 4160749568;
        }
        out[i] = out[i] >> scale;
    }
}

static void beco_mmacgr4_read_8_8bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i8[0] = beco_read_acc(BECO_ACC0, 0).i8[0];
    p_out[0].i8[1] = beco_read_acc(BECO_ACC0, 2).i8[0];
    p_out[0].i8[2] = beco_read_acc(BECO_ACC0, 8).i8[0];
    p_out[0].i8[3] = beco_read_acc(BECO_ACC0, 10).i8[0];

    p_out[1].i8[0] = beco_read_acc(BECO_ACC0, 4).i8[0];
    p_out[1].i8[1] = beco_read_acc(BECO_ACC0, 6).i8[0];
    p_out[1].i8[2] = beco_read_acc(BECO_ACC0, 12).i8[0];
    p_out[1].i8[3] = beco_read_acc(BECO_ACC0, 14).i8[0];

    p_out[2].i8[0] = beco_read_acc(BECO_ACC1, 0).i8[0];
    p_out[2].i8[1] = beco_read_acc(BECO_ACC1, 2).i8[0];
    p_out[2].i8[2] = beco_read_acc(BECO_ACC1, 8).i8[0];
    p_out[2].i8[3] = beco_read_acc(BECO_ACC1, 10).i8[0];

    p_out[3].i8[0] = beco_read_acc(BECO_ACC1, 4).i8[0];
    p_out[3].i8[1] = beco_read_acc(BECO_ACC1, 6).i8[0];
    p_out[3].i8[2] = beco_read_acc(BECO_ACC1, 12).i8[0];
    p_out[3].i8[3] = beco_read_acc(BECO_ACC1, 14).i8[0];

    p_out[4].i8[0] = beco_read_acc(BECO_ACC2, 0).i8[0];
    p_out[4].i8[1] = beco_read_acc(BECO_ACC2, 2).i8[0];
    p_out[4].i8[2] = beco_read_acc(BECO_ACC2, 8).i8[0];
    p_out[4].i8[3] = beco_read_acc(BECO_ACC2, 10).i8[0];

    p_out[5].i8[0] = beco_read_acc(BECO_ACC2, 4).i8[0];
    p_out[5].i8[1] = beco_read_acc(BECO_ACC2, 6).i8[0];
    p_out[5].i8[2] = beco_read_acc(BECO_ACC2, 12).i8[0];
    p_out[5].i8[3] = beco_read_acc(BECO_ACC2, 14).i8[0];

    p_out[6].i8[0] = beco_read_acc(BECO_ACC3, 0).i8[0];
    p_out[6].i8[1] = beco_read_acc(BECO_ACC3, 2).i8[0];
    p_out[6].i8[2] = beco_read_acc(BECO_ACC3, 8).i8[0];
    p_out[6].i8[3] = beco_read_acc(BECO_ACC3, 10).i8[0];

    p_out[7].i8[0] = beco_read_acc(BECO_ACC3, 4).i8[0];
    p_out[7].i8[1] = beco_read_acc(BECO_ACC3, 6).i8[0];
    p_out[7].i8[2] = beco_read_acc(BECO_ACC3, 12).i8[0];
    p_out[7].i8[3] = beco_read_acc(BECO_ACC3, 14).i8[0];
}

static void beco_mmacgr4_read_8_16bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    p_out[0].i16[1] = beco_read_acc(BECO_ACC0, 2).i16[0];
    p_out[1].i16[0] = beco_read_acc(BECO_ACC0, 8).i16[0];
    p_out[1].i16[1] = beco_read_acc(BECO_ACC0, 10).i16[0];
    p_out[2].i16[0] = beco_read_acc(BECO_ACC0, 4).i16[0];
    p_out[2].i16[1] = beco_read_acc(BECO_ACC0, 6).i16[0];
    p_out[3].i16[0] = beco_read_acc(BECO_ACC0, 12).i16[0];
    p_out[3].i16[1] = beco_read_acc(BECO_ACC0, 14).i16[0];

    p_out[4].i16[0] = beco_read_acc(BECO_ACC1, 0).i16[0];
    p_out[4].i16[1] = beco_read_acc(BECO_ACC1, 2).i16[0];
    p_out[5].i16[0] = beco_read_acc(BECO_ACC1, 8).i16[0];
    p_out[5].i16[1] = beco_read_acc(BECO_ACC1, 10).i16[0];
    p_out[6].i16[0] = beco_read_acc(BECO_ACC1, 4).i16[0];
    p_out[6].i16[1] = beco_read_acc(BECO_ACC1, 6).i16[0];
    p_out[7].i16[0] = beco_read_acc(BECO_ACC1, 12).i16[0];
    p_out[7].i16[1] = beco_read_acc(BECO_ACC1, 14).i16[0];

    p_out[8].i16[0] = beco_read_acc(BECO_ACC2, 0).i16[0];
    p_out[8].i16[1] = beco_read_acc(BECO_ACC2, 2).i16[0];
    p_out[9].i16[0] = beco_read_acc(BECO_ACC2, 8).i16[0];
    p_out[9].i16[1] = beco_read_acc(BECO_ACC2, 10).i16[0];
    p_out[10].i16[0] = beco_read_acc(BECO_ACC2, 4).i16[0];
    p_out[10].i16[1] = beco_read_acc(BECO_ACC2, 6).i16[0];
    p_out[11].i16[0] = beco_read_acc(BECO_ACC2, 12).i16[0];
    p_out[11].i16[1] = beco_read_acc(BECO_ACC2, 14).i16[0];

    p_out[12].i16[0] = beco_read_acc(BECO_ACC3, 0).i16[0];
    p_out[12].i16[1] = beco_read_acc(BECO_ACC3, 2).i16[0];
    p_out[13].i16[0] = beco_read_acc(BECO_ACC3, 8).i16[0];
    p_out[13].i16[1] = beco_read_acc(BECO_ACC3, 10).i16[0];
    p_out[14].i16[0] = beco_read_acc(BECO_ACC3, 4).i16[0];
    p_out[14].i16[1] = beco_read_acc(BECO_ACC3, 6).i16[0];
    p_out[15].i16[0] = beco_read_acc(BECO_ACC3, 12).i16[0];
    p_out[15].i16[1] = beco_read_acc(BECO_ACC3, 14).i16[0];
}

static void beco_mmacgr4_read_16_16bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    p_out[0].i16[1] = beco_read_acc(BECO_ACC0, 8).i16[0];
    p_out[1].i16[0] = beco_read_acc(BECO_ACC0, 4).i16[0];
    p_out[1].i16[1] = beco_read_acc(BECO_ACC0, 12).i16[0];

    p_out[2].i16[0] = beco_read_acc(BECO_ACC1, 0).i16[0];
    p_out[2].i16[1] = beco_read_acc(BECO_ACC1, 8).i16[0];
    p_out[3].i16[0] = beco_read_acc(BECO_ACC1, 4).i16[0];
    p_out[3].i16[1] = beco_read_acc(BECO_ACC1, 12).i16[0];

    p_out[4].i16[0] = beco_read_acc(BECO_ACC2, 0).i16[0];
    p_out[4].i16[1] = beco_read_acc(BECO_ACC2, 8).i16[0];
    p_out[5].i16[0] = beco_read_acc(BECO_ACC2, 4).i16[0];
    p_out[5].i16[1] = beco_read_acc(BECO_ACC2, 12).i16[0];

    p_out[6].i16[0] = beco_read_acc(BECO_ACC3, 0).i16[0];
    p_out[6].i16[1] = beco_read_acc(BECO_ACC3, 8).i16[0];
    p_out[7].i16[0] = beco_read_acc(BECO_ACC3, 4).i16[0];
    p_out[7].i16[1] = beco_read_acc(BECO_ACC3, 12).i16[0];
}

static void beco_mmacgr4_read_16_32bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0] = beco_read_acc(BECO_ACC0, 0);
    p_out[1] = beco_read_acc(BECO_ACC0, 8);
    p_out[2] = beco_read_acc(BECO_ACC0, 4);
    p_out[3] = beco_read_acc(BECO_ACC0, 12);

    p_out[4] = beco_read_acc(BECO_ACC1, 0);
    p_out[5] = beco_read_acc(BECO_ACC1, 8);
    p_out[6] = beco_read_acc(BECO_ACC1, 4);
    p_out[7] = beco_read_acc(BECO_ACC1, 12);

    p_out[8] = beco_read_acc(BECO_ACC2, 0);
    p_out[9] = beco_read_acc(BECO_ACC2, 8);
    p_out[10] = beco_read_acc(BECO_ACC2, 4);
    p_out[11] = beco_read_acc(BECO_ACC2, 12);

    p_out[12] = beco_read_acc(BECO_ACC3, 0);
    p_out[13] = beco_read_acc(BECO_ACC3, 8);
    p_out[14] = beco_read_acc(BECO_ACC3, 4);
    p_out[15] = beco_read_acc(BECO_ACC3, 12);
    q31_t *out = (q31_t *)p_out;
    for (int i = 0; i < 16; i++) {
        if (out[i] > 67108864) {
            out[i] += 4160749568;
        }
        out[i] = out[i] >> scale;
    }
}

static void beco_mmacgr4_read_8_32bit(beco_vec32_out_t *p_out, int scale)
{
    p_out[0] = beco_read_acc(BECO_ACC0, 0);
    p_out[1] = beco_read_acc(BECO_ACC0, 2);
    p_out[2] = beco_read_acc(BECO_ACC0, 8);
    p_out[3] = beco_read_acc(BECO_ACC0, 10);
    p_out[4] = beco_read_acc(BECO_ACC0, 4);
    p_out[5] = beco_read_acc(BECO_ACC0, 6);
    p_out[6] = beco_read_acc(BECO_ACC0, 12);
    p_out[7] = beco_read_acc(BECO_ACC0, 14);

    p_out[8] = beco_read_acc(BECO_ACC1, 0);
    p_out[9] = beco_read_acc(BECO_ACC1, 2);
    p_out[10] = beco_read_acc(BECO_ACC1, 8);
    p_out[11] = beco_read_acc(BECO_ACC1, 10);
    p_out[12] = beco_read_acc(BECO_ACC1, 4);
    p_out[13] = beco_read_acc(BECO_ACC1, 6);
    p_out[14] = beco_read_acc(BECO_ACC1, 12);
    p_out[15] = beco_read_acc(BECO_ACC1, 14);

    p_out[16] = beco_read_acc(BECO_ACC2, 0);
    p_out[17] = beco_read_acc(BECO_ACC2, 2);
    p_out[18] = beco_read_acc(BECO_ACC2, 8);
    p_out[19] = beco_read_acc(BECO_ACC2, 10);
    p_out[20] = beco_read_acc(BECO_ACC2, 4);
    p_out[21] = beco_read_acc(BECO_ACC2, 6);
    p_out[22] = beco_read_acc(BECO_ACC2, 12);
    p_out[23] = beco_read_acc(BECO_ACC2, 14);

    p_out[24] = beco_read_acc(BECO_ACC3, 0);
    p_out[25] = beco_read_acc(BECO_ACC3, 2);
    p_out[26] = beco_read_acc(BECO_ACC3, 8);
    p_out[27] = beco_read_acc(BECO_ACC3, 10);
    p_out[28] = beco_read_acc(BECO_ACC3, 4);
    p_out[29] = beco_read_acc(BECO_ACC3, 6);
    p_out[30] = beco_read_acc(BECO_ACC3, 12);
    p_out[31] = beco_read_acc(BECO_ACC3, 14);

    q31_t *out = (q31_t *)p_out;
    for (int i = 0; i < 32; i++) {
        if (out[i] > 67108864) {
            out[i] += 4160749568;
        }
        out[i] = out[i] >> scale;
    }
}


static void beco_mmacrr_read_8x8_8(beco_vec32_out_t *p_out, int stride, int scale)
{
    beco_vec32_out_t *out2 = p_out + 4*stride;
    beco_read_acc(BECO_ACC0, -4);
    for (int i = 0; i < 4; i++) {
        uint32_t a0 = beco_read_next_acc(BECO_ACC0, 4).i32;
        uint32_t a1 = beco_read_next_acc(BECO_ACC1, 0).i32;
        uint32_t a2 = beco_read_next_acc(BECO_ACC2, 0).i32;
        uint32_t a3 = beco_read_next_acc(BECO_ACC3, 0).i32;
        p_out[0].u32 = a0;
        p_out[1].u32 = a2;
        p_out += stride;
        out2[0].u32 = a1;
        out2[1].u32 = a3;
        out2 += stride;
    }
}

static void beco_mmacrr_read_8x8_16(beco_vec32_out_t *p_out, int stride, int scale)
{
    beco_vec32_out_t *out2 = p_out + 4*stride;
    uint32_t a0, a1, a2, a3, a4, a5, a6, a7;
    beco_read_acc(BECO_ACC0, -2);
    for (int i = 0; i < 4; i++) {
        a0 = beco_read_next_acc(BECO_ACC0, 2).i32;
        a2 = beco_read_next_acc(BECO_ACC1, 0).i32;
        a4 = beco_read_next_acc(BECO_ACC2, 0).i32;
        a6 = beco_read_next_acc(BECO_ACC3, 0).i32;
        a1 = beco_read_next_acc(BECO_ACC0, 2).i32;
        a3 = beco_read_next_acc(BECO_ACC1, 0).i32;
        a5 = beco_read_next_acc(BECO_ACC2, 0).i32;
        a7 = beco_read_next_acc(BECO_ACC3, 0).i32;
        p_out[0].u32 = a0;
        p_out[1].u32 = a1;
        p_out[2].u32 = a4;
        p_out[3].u32 = a5;
        p_out += stride;
        out2[0].u32 = a2;
        out2[1].u32 = a3;
        out2[2].u32 = a6;
        out2[3].u32 = a7;
        out2 += stride;
    }
}

static void beco_mmacrr_read_16x16_16(beco_vec32_out_t *p_out, int stride, int scale)
{
    p_out[0].i16[0] = beco_read_acc(BECO_ACC0, 0).i16[0];
    p_out[0].i16[1] = beco_read_acc(BECO_ACC0, 8).i16[0];
    p_out[1].i16[0] = beco_read_acc(BECO_ACC2, 0).i16[0];
    p_out[1].i16[1] = beco_read_acc(BECO_ACC2, 8).i16[0];

    p_out[stride].i16[0] = beco_read_acc(BECO_ACC0, 4).i16[0];
    p_out[stride].i16[1] = beco_read_acc(BECO_ACC0, 12).i16[0];
    p_out[stride+1].i16[0] = beco_read_acc(BECO_ACC2, 4).i16[0];
    p_out[stride+1].i16[1] = beco_read_acc(BECO_ACC2, 12).i16[0];

    p_out[2*stride].i16[0] = beco_read_acc(BECO_ACC1, 0).i16[0];
    p_out[2*stride].i16[1] = beco_read_acc(BECO_ACC1, 8).i16[0];
    p_out[2*stride+1].i16[0] = beco_read_acc(BECO_ACC3, 0).i16[0];
    p_out[2*stride+1].i16[1] = beco_read_acc(BECO_ACC3, 8).i16[0];

    p_out[3*stride].i16[0] = beco_read_acc(BECO_ACC1, 4).i16[0];
    p_out[3*stride].i16[1] = beco_read_acc(BECO_ACC1, 12).i16[0];
    p_out[3*stride+1].i16[0] = beco_read_acc(BECO_ACC3, 4).i16[0];
    p_out[3*stride+1].i16[1] = beco_read_acc(BECO_ACC3, 12).i16[0];
}

static void beco_mmacrr_read_16x16_32(beco_vec32_out_t *p_out, int stride, int scale)
{
    p_out[0] = beco_read_acc(BECO_ACC0, 0);
    p_out[1] = beco_read_acc(BECO_ACC0, 8);
    p_out[2] = beco_read_acc(BECO_ACC2, 0);
    p_out[3] = beco_read_acc(BECO_ACC2, 8);
    p_out[stride] = beco_read_acc(BECO_ACC0, 4);
    p_out[stride+1] = beco_read_acc(BECO_ACC0, 12);
    p_out[stride+2] = beco_read_acc(BECO_ACC2, 4);
    p_out[stride+3] = beco_read_acc(BECO_ACC2, 12);
    p_out[2*stride] = beco_read_acc(BECO_ACC1, 0);
    p_out[2*stride+1] = beco_read_acc(BECO_ACC1, 8);
    p_out[2*stride+2] = beco_read_acc(BECO_ACC3, 0);
    p_out[2*stride+3] = beco_read_acc(BECO_ACC3, 8);
    p_out[3*stride] = beco_read_acc(BECO_ACC1, 4);
    p_out[3*stride+1] = beco_read_acc(BECO_ACC1, 12);
    p_out[3*stride+2] = beco_read_acc(BECO_ACC3, 4);
    p_out[3*stride+3] = beco_read_acc(BECO_ACC3, 12);
    q31_t *out = (q31_t *)p_out;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (out[i*stride + j] > 67108864) {
                out[i*stride + j] += 4160749568;
            }
            out[i*stride + j] = out[i*stride + j] >> scale;
        }
    }
}
