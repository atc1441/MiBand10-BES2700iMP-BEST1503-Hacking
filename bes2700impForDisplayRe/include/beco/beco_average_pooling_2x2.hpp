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

#include "beco.h"
#include "beco_l1.h"
#include "beco_types.h"

#ifndef __BECO_AVERAGE_POOLING_2X2_H__
#define __BECO_AVERAGE_POOLING_2X2_H__


static inline void beco_average_pooling_2x2_4pixels(beco_vec32_in_t average_weight,
                                                    const beco_vec64_in_t *p,
                                                    int stepXstride)
{
    beco_write_reg(BECO_REG2, p[2*stepXstride]);
    beco_write_reg(BECO_REG3, p[3*stepXstride]);
    beco_mmacgr4(average_weight, BECO_REG0);
}

void beco_read_4banks1(q7_t *out, int channel)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i16[0] + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC0, 2).i16[0] + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC0, 8).i16[0] + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC0, 10).i16[0] + 2) >> 2;
    out[4] = (beco_read_acc(BECO_ACC0, 4).i16[0] + 2) >> 2;
    out[5] = (beco_read_acc(BECO_ACC0, 6).i16[0] + 2) >> 2;
    out[6] = (beco_read_acc(BECO_ACC0, 12).i16[0] + 2) >> 2;
    out[7] = (beco_read_acc(BECO_ACC0, 14).i16[0] + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC1, 0).i16[0] + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC1, 2).i16[0] + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC1, 8).i16[0] + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC1, 10).i16[0] + 2) >> 2;
    out[4] = (beco_read_acc(BECO_ACC1, 4).i16[0] + 2) >> 2;
    out[5] = (beco_read_acc(BECO_ACC1, 6).i16[0] + 2) >> 2;
    out[6] = (beco_read_acc(BECO_ACC1, 12).i16[0] + 2) >> 2;
    out[7] = (beco_read_acc(BECO_ACC1, 14).i16[0] + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC2, 0).i16[0] + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC2, 2).i16[0] + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC2, 8).i16[0] + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC2, 10).i16[0] + 2) >> 2;
    out[4] = (beco_read_acc(BECO_ACC2, 4).i16[0] + 2) >> 2;
    out[5] = (beco_read_acc(BECO_ACC2, 6).i16[0] + 2) >> 2;
    out[6] = (beco_read_acc(BECO_ACC2, 12).i16[0] + 2) >> 2;
    out[7] = (beco_read_acc(BECO_ACC2, 14).i16[0] + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC3, 0).i16[0] + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC3, 2).i16[0] + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC3, 8).i16[0] + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC3, 10).i16[0] + 2) >> 2;
    out[4] = (beco_read_acc(BECO_ACC3, 4).i16[0] + 2) >> 2;
    out[5] = (beco_read_acc(BECO_ACC3, 6).i16[0] + 2) >> 2;
    out[6] = (beco_read_acc(BECO_ACC3, 12).i16[0] + 2) >> 2;
    out[7] = (beco_read_acc(BECO_ACC3, 14).i16[0] + 2) >> 2;
}
void beco_read_4banks2(q15_t *out, int channel)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i32 + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC0, 8).i32 + 2) >> 2;
    out[2]= (beco_read_acc(BECO_ACC0, 4).i32 + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC0, 12).i32 + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC1, 0).i32 + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC1, 8).i32 + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC1, 4).i32 + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC1, 12).i32 + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC2, 0).i32 + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC2, 8).i32 + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC2, 4).i32 + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC2, 12).i32 + 2) >> 2;

    out += channel;
    out[0] = (beco_read_acc(BECO_ACC3, 0).i32 + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC3, 8).i32 + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC3, 4).i32 + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC3, 12).i32 + 2) >> 2;
}
template <typename Inttype> void beco_read_4banks(Inttype *out, int channel) {};
template <> void beco_read_4banks<q7_t>(q7_t *out, int channel)
{
    beco_read_4banks1(out, channel);
}
template <> void beco_read_4banks<q15_t>(q15_t *out, int channel)
{
    beco_read_4banks2(out, channel);
}

void beco_read_1bank1(q7_t *out)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i16[0] + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC0, 2).i16[0] + 2) >> 2;
    out[2] = (beco_read_acc(BECO_ACC0, 8).i16[0] + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC0, 10).i16[0] + 2) >> 2;
    out[4] = (beco_read_acc(BECO_ACC0, 4).i16[0] + 2) >> 2;
    out[5] = (beco_read_acc(BECO_ACC0, 6).i16[0] + 2) >> 2;
    out[6] = (beco_read_acc(BECO_ACC0, 12).i16[0] + 2) >> 2;
    out[7] = (beco_read_acc(BECO_ACC0, 14).i16[0] + 2) >> 2;

}
void beco_read_1bank2(q15_t *out)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i32 + 2) >> 2;
    out[1] = (beco_read_acc(BECO_ACC0, 8).i32 + 2) >> 2;
    out[2]= (beco_read_acc(BECO_ACC0, 4).i32 + 2) >> 2;
    out[3] = (beco_read_acc(BECO_ACC0, 12).i32 + 2) >> 2;
}
template <typename Inttype> void beco_read_1bank(Inttype *out) {};
template <> void beco_read_1bank<q7_t>(q7_t *out) {beco_read_1bank1(out);}
template <> void beco_read_1bank<q15_t>(q15_t *out) {beco_read_1bank2(out);}

void beco_read_margin1(q7_t *out)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i16[0] + 1) >> 1;
    out[1] = (beco_read_acc(BECO_ACC0, 2).i16[0] + 1) >> 1;
    out[2] = (beco_read_acc(BECO_ACC0, 8).i16[0] + 1) >> 1;
    out[3] = (beco_read_acc(BECO_ACC0, 10).i16[0] + 1) >> 1;
    out[4] = (beco_read_acc(BECO_ACC0, 4).i16[0] + 1) >> 1;
    out[5] = (beco_read_acc(BECO_ACC0, 6).i16[0] + 1) >> 1;
    out[6] = (beco_read_acc(BECO_ACC0, 12).i16[0] + 1) >> 1;
    out[7] = (beco_read_acc(BECO_ACC0, 14).i16[0] + 1) >> 1;
}
void beco_read_margin2(q15_t *out)
{
    out[0] = (beco_read_acc(BECO_ACC0, 0).i32 + 1) >> 1;
    out[1] = (beco_read_acc(BECO_ACC0, 8).i32 + 1) >> 1;
    out[2] = (beco_read_acc(BECO_ACC0, 4).i32 + 1) >> 1;
    out[3] = (beco_read_acc(BECO_ACC0, 12).i32 + 1) >> 1;
}
template <typename Inttype> void beco_read_margin(Inttype *out) {};
template <> void beco_read_margin<q7_t>(q7_t *out) {beco_read_margin1(out);}
template <> void beco_read_margin<q15_t>(q15_t *out) {beco_read_margin2(out);}

template <typename Inttype>
static void beco_average_pooling_2x2_4results(beco_vec32_in_t average_weight,
                                              const beco_vec64_in_t *p,
                                              const int channel,
                                              int out_delta,
                                              int stride,
                                              int h_stride,
                                              int changerow_stride,
                                              Inttype *out)
{
    beco_clear_acc(BECO_ACC0);
    beco_clear_acc(BECO_ACC1);
    beco_clear_acc(BECO_ACC2);
    beco_clear_acc(BECO_ACC3);
    int stepXstride = 2 * stride;
    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 2; ++j) {
            beco_write_reg(BECO_REG0, p[0]);
            beco_write_reg(BECO_REG1, p[stepXstride]);
            beco_average_pooling_2x2_4pixels(average_weight, p, stepXstride);
            p += stride;
        }
        p += changerow_stride;
    }
    beco_read_4banks<Inttype>(out, channel);
}

template <typename Inttype> void read_from_src(const beco_vec64_in_t *p, Inttype *dest) {};
template <> void read_from_src<q7_t>(const beco_vec64_in_t *p, q7_t *dest)
{
    dest[0] = p->i8[0];
    dest[1] = p->i8[1];
    dest[2] = p->i8[2];
    dest[3] = p->i8[3];
    dest[4] = p->i8[4];
    dest[5] = p->i8[5];
    dest[6] = p->i8[6];
    dest[7] = p->i8[7];
}
template <> void read_from_src<q15_t>(const beco_vec64_in_t *p, q15_t *dest)
{
    dest[0] = p->i16[0];
    dest[1] = p->i16[1];
    dest[2] = p->i16[2];
    dest[3] = p->i16[3];
}

template <typename Inttype>
static void beco_average_pooling_2x2_processRow(beco_vec32_in_t average_weight,
                                                const beco_vec64_in_t *p,
                                                const int channel,
                                                int out_width,
                                                int stride,
                                                int h_stride,
                                                int changerow_stride,
                                                const bool paddingX,
                                                Inttype *out)
{
    int roll = out_width >> 2;
    int remainder = out_width & 3;

    int p_stride = stride << 3;
    int out_stride = channel << 2;
    int out_delta = sizeof(Inttype) == 1 ? channel - 7 : channel -3;

    if (paddingX) {
        beco_clear_acc(BECO_ACC0);
        beco_write_reg(BECO_REG0, p[0]);
        beco_mmacgr(BECO_ACC0, average_weight, BECO_REG0);
        p += h_stride;
        beco_write_reg(BECO_REG0, p[0]);
        beco_mmacgr(BECO_ACC0, average_weight, BECO_REG0);
        beco_read_margin<Inttype>(out);
        out += channel;
        p += stride - h_stride;
    }

    while ((--roll) >= 0) {
        beco_average_pooling_2x2_4results<Inttype>(average_weight, p, channel, out_delta, stride,
                                                                h_stride, changerow_stride, out);
        p += p_stride;
        out += out_stride;
    }

    while ((--remainder) >= 0) {
        beco_clear_acc(BECO_ACC0);
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                beco_write_reg(BECO_REG0, p[0]);
                beco_mmacgr(BECO_ACC0, average_weight, BECO_REG0);
                p += stride;
            }
            p += changerow_stride;
        }
        p += 2 * stride - 2 * h_stride;
        beco_read_1bank(out);
        out += channel;
    }
}

template <typename Inttype>
void beco_average_pooling_prcoss1stRow(const beco_vec64_in_t *p,
                                       beco_vec32_in_t average_weight,
                                       const int channel,
                                       int out_width,
                                       int stride,
                                       Inttype *dest)
{
    for (int i = 0; i < out_width; i++) {
        beco_clear_acc(BECO_ACC0);
        beco_write_reg(BECO_REG0, p[0]);
        beco_mmacgr(BECO_ACC0, average_weight, BECO_REG0);
        p += stride;
        beco_write_reg(BECO_REG0, p[0]);
        beco_mmacgr(BECO_ACC0, average_weight, BECO_REG0);
        p += stride;
        beco_read_margin<Inttype>(dest);
        dest += channel;
    }
}

template <typename Inttype>
void beco_average_pooling_2x2_padding(const Inttype *src,
                                      const int height,
                                      const int width,
                                      const int channel,
                                      const bool paddingX,
                                      const bool paddingY,
                                      Inttype *dest)
{
    const beco_vec64_in_t *p = (const beco_vec64_in_t *) src;
    int stride = sizeof(Inttype) == 1 ? channel >> 3 : channel >> 2;
    int h_stride = width * stride;
    beco_vec32_in_t average_weight = (beco_vec32_in_t){.i16 = {1, 0}};

    int count_h = height < 2 ? 0 : ((height - 2) / 2 + 1);
    int count_c = stride;
    int out_width =  width < 2 ? 0 : ((width - 2) / 2 + 1);

    int p_stride = 2 * h_stride - stride;
    int out_stride = channel * (out_width - 1);
    int changrow_stride = h_stride - 2 * stride;
    int delta_dest = sizeof(Inttype) == 1 ? 8 : 4;

    if (paddingX)
        out_stride += channel;

    if (paddingY) {
        if (paddingX) {
            while ((--count_c) >= 0) {
                read_from_src<Inttype>(p, dest);
                beco_average_pooling_prcoss1stRow<Inttype>(p + stride, average_weight, channel,
                                                             out_width, stride, dest + channel);
                ++p;
                dest += delta_dest;
            }
        } else {
            while ((--count_c) >= 0) {
                beco_average_pooling_prcoss1stRow<Inttype>(p, average_weight, channel,
                                                             out_width, stride, dest);
                ++p;
                dest += delta_dest;
            }
        }
        p += h_stride - stride;
        dest += out_stride;
        count_c = stride;
    }

    while ((--count_h) >= 0) {
        while ((--count_c) >= 0) {
            beco_average_pooling_2x2_processRow<Inttype>(average_weight, p, channel, out_width,
                                            stride, h_stride, changrow_stride, paddingX, dest);
            ++p;
            dest += delta_dest;
        }
        count_c = stride;
        p += p_stride;
        dest += out_stride;
    }
}

void beco_set_average_pooling_config()
{
    uint32_t config;
    config =
            BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 |
            BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT8  |
            BECO_CONF_RSHIFT(0) |
            BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x8_ROT90;
    beco_write_config(config);
}

template <typename Inttype>
void beco_average_pooling_2x2(const Inttype *src,
                              const int dim_in_h,
                              const int dim_in_w,
                              const int dim_in_c,
                              Inttype *dest)
{
    bool paddingX = dim_in_w & 1;
    bool paddingY = dim_in_h & 1;
    beco_set_average_pooling_config();
    beco_average_pooling_2x2_padding<Inttype>(src, dim_in_h, dim_in_w, dim_in_c,
                                                      paddingX, paddingY, dest);
}


template <typename Inttype>
void beco_imgCHW2HWC(const Inttype *src,
                     const int height,
                     const int width,
                     const int channel,
                     Inttype *src_hwc)
{
    int count = 0;
    int num = height * width * channel;
    int wc = width * channel;
    int hw = height * width;
    int which_h;
    int which_w;
    int which_c;
    while (count < num) {
        which_c = count / hw;
        which_h = count / width - height * which_c;
        which_w = count % width;
        src_hwc[which_h * wc + which_w * channel + which_c] = src[count];
        ++count;
    }
}
#endif