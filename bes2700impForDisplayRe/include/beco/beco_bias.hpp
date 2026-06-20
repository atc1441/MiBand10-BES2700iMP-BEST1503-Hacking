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


#ifndef _BECO_BIAS_HPP
#define _BECO_BIAS_HPP

#include "beco_common.h"
#include "beco_types.h"

#include <stddef.h>
#include <type_traits>
#include <typeinfo>


typedef struct {
    uint32_t bias[4];
} beco_bias_t;


template <typename T>
inline void beco_calc_acc_preload_8x8(T v0, T v1, T v2, T v3,
                                      int32_t offset,
                                      int l_shift, beco_bias_t *load)
{
    load->bias[0] = ((uint32_t)v0 << l_shift) + offset;
    load->bias[1] = ((uint32_t)v1 << l_shift) + offset;
    load->bias[2] = ((uint32_t)v2 << l_shift) + offset;
    load->bias[3] = ((uint32_t)v3 << l_shift) + offset;
}

template <typename T>
inline void beco_calc_acc_preload_8x16(T _v0, T _v1,
                                       int32_t offset,
                                       int l_shift, beco_bias_t *load)
{
    // Create a 32 bit type of same sign as input parameter
    typedef typename std::conditional<std::is_signed<T>::value, int32_t, uint32_t>::type value_t;

    value_t v0;
    value_t v1;

    v0 = ((value_t)_v0 << l_shift) + offset;
    v1 = ((value_t)_v1 << l_shift) + offset;

    load->bias[1] = (v0 >> 8);
    load->bias[0] = (uint8_t) (v0);
    load->bias[3] = (v1 >> 8);
    load->bias[2] = (uint8_t) (v1);
}


template <typename T>
inline void beco_calc_acc_preload_16x8(T _v0, T _v1,
                                       int32_t offset,
                                       int l_shift, beco_bias_t *load)
{
    // Create a 32 bit type of same sign as input parameter
    typedef typename std::conditional<std::is_signed<T>::value, int32_t, uint32_t>::type value_t;

    value_t v0 = ((value_t)_v0 << l_shift) + offset;
    value_t v1 = ((value_t)_v1 << l_shift) + offset;

    load->bias[2] = (v0 >> 8);
    load->bias[0] = (uint8_t) (v0);
    load->bias[3] = (v1 >> 8);
    load->bias[1] = (uint8_t) (v1);
}


template <typename T>
inline void beco_calc_acc_preload_16x16(T _v,
                                        int32_t offset,
                                        int l_shift, beco_bias_t *load)
{
    // Create a 32 bit type of same sign as input parameter
    typedef typename std::conditional<std::is_signed<T>::value, int32_t, uint32_t>::type value_t;

    value_t v = ((value_t)_v << l_shift) + offset;

    load->bias[3] = (v >> 16);
    load->bias[2] = (uint8_t) (v >> 8);
    load->bias[1] = 0;
    load->bias[0] = (uint8_t) (v);
}



inline void beco_set_preload_8x8(int32_t q_bias)
{
    beco_bias_t load;

    beco_calc_acc_preload_8x8(0, 0, 0, 0, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_8x8_4ACC(int32_t q_bias)
{
    beco_bias_t load;

    beco_calc_acc_preload_8x8(0, 0, 0, 0, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC1, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC2, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC3, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_8x16(int32_t q_bias)
{
    beco_bias_t load;

    beco_calc_acc_preload_8x16(0, 0, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_16x8(int32_t q_bias)
{
    const int32_t zero = 0;
    beco_bias_t load;

    beco_calc_acc_preload_16x8(zero, zero, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_16x8_4ACC(int32_t q_bias)
{
    const int32_t zero = 0;
    beco_bias_t load;

    beco_calc_acc_preload_16x8(zero, zero, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC1, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC2, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC3, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_8x16_4ACC(int32_t q_bias)
{
    const int32_t zero = 0;
    beco_bias_t load;

    beco_calc_acc_preload_8x16(zero, zero, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC1, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC2, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC3, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_16x16(int32_t q_bias)
{
    const int32_t zero = 0;
    beco_bias_t load;

    beco_calc_acc_preload_16x16(zero, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
}

inline void beco_set_preload_16x16_4ACC(int32_t q_bias)
{
    const int32_t zero = 0;
    beco_bias_t load;

    beco_calc_acc_preload_16x16(zero, q_bias, 0, &load);

    beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }}));
    beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }}));
    beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC1, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC2, BECO_REG0, BECO_REG1);
    beco_preload_acc(BECO_ACC3, BECO_REG0, BECO_REG1);
}

template <typename bias_t> void beco_add_bias_8x8(const bias_t *bias, p7_t q);
extern template void beco_add_bias_8x8(const int8_t *bias, p7_t q);
extern template void beco_add_bias_8x8(const uint8_t *bias, p7_t q);

template <typename bias_t> void beco_add_bias_8x16(const bias_t *bias, p7_t q);
extern template void beco_add_bias_8x16(const int16_t *bias, p7_t q);
extern template void beco_add_bias_8x16(const uint16_t *bias, p7_t q);

template <typename bias_t> void beco_add_bias_16x8(const bias_t *bias, p7_t q);
extern template void beco_add_bias_16x8(const int16_t *bias, p7_t q);
extern template void beco_add_bias_16x8(const uint16_t *bias, p7_t q);

template <typename bias_t> void beco_add_bias_16x16(const bias_t *bias, p15_t q);
extern template void beco_add_bias_16x16(const int16_t *bias, p15_t q);
extern template void beco_add_bias_16x16(const uint16_t *bias, p15_t q);

template <typename bias_t> void beco_add_bias_4x4_16x16(const bias_t *bias, p15_t q);
extern template void beco_add_bias_4x4_16x16(const int16_t *bias, p15_t q);
extern template void beco_add_bias_4x4_16x16(const uint16_t *bias, p15_t q);



/* Obsolete API */

#define BECO_LOAD_BIAS_ACC_MU(acc, load) \
    do { \
        beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }})); \
        beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }})); \
        \
        beco_set_acc_bias(acc, BECO_REG0, BECO_REG1);  \
    } while(0)

#define BECO_SET_BIAS_ACC(acc, load) do { \
        beco_write_reg(BECO_REG0, ((beco_vec64_in_t){.u32={load.bias[0],  load.bias[1] }})); \
        beco_write_reg(BECO_REG1, ((beco_vec64_in_t){.u32={load.bias[2],  load.bias[3] }})); \
        \
        beco_preload_acc(acc, BECO_REG0, BECO_REG1);  \
    } while(0)


template <typename bias_t>
inline void beco_set_preload_8x8(const bias_t *bias, int bias_shift, int out_shift)
{
    beco_bias_t load;

    int32_t q_half = q_round(out_shift);

    beco_calc_acc_preload_8x8(bias[0],  bias[4],  bias[1],  bias[5],  q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_0, load);

    beco_calc_acc_preload_8x8(bias[2],  bias[6],  bias[3],  bias[7],  q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_4, load);

    beco_calc_acc_preload_8x8(bias[8],  bias[12],  bias[9], bias[13], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_8, load);

    beco_calc_acc_preload_8x8(bias[10], bias[14], bias[11], bias[15], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_12, load);
}

template <typename bias_t>
inline void beco_set_preload_16x8(const bias_t *bias, int bias_shift, int out_shift)
{
    beco_bias_t load;

    int32_t q_half = q_round(out_shift);

    beco_calc_acc_preload_16x8(bias[0], bias[2], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_0, load);

    beco_calc_acc_preload_16x8(bias[1], bias[3], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_4, load);

    beco_calc_acc_preload_16x8(bias[4], bias[6], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_8, load);

    beco_calc_acc_preload_16x8(bias[5], bias[7], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_12, load);
}


template <typename bias_t>
inline void beco_set_preload_8x16(const bias_t *bias, int bias_shift, int out_shift)
{
    beco_bias_t load;

    int32_t q_half = q_round(out_shift);

    beco_calc_acc_preload_8x16(bias[0], bias[1], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_0, load);

    beco_calc_acc_preload_8x16(bias[2], bias[3], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_4, load);

    beco_calc_acc_preload_8x16(bias[4], bias[5], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_8, load);

    beco_calc_acc_preload_8x16(bias[6], bias[7], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_12, load);
}


template <typename bias_t>
inline void beco_set_preload_16x16(const bias_t *bias, int bias_shift, int out_shift)
{
    beco_bias_t load;

    int32_t q_half = q_round(out_shift);

    beco_calc_acc_preload_16x16(bias[0], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_0, load);

    beco_calc_acc_preload_16x16(bias[1], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_4, load);

    beco_calc_acc_preload_16x16(bias[2], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_8, load);

    beco_calc_acc_preload_16x16(bias[3], q_half, bias_shift, &load);
    BECO_LOAD_BIAS_ACC_MU(BECO_ACC0_12, load);
}

#endif
