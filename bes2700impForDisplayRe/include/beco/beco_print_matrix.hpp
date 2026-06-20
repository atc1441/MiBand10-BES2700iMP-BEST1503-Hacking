#pragma once
//
// Copyright 2020 BES Technic
//
// C++ templates to print matrix and vectors

#ifndef PRINT_MATRIX_H
#define PRINT_MATRIX_H

#include <stdint.h>
#include <stdio.h>
#include <beco_types.h>

template <typename fmtType>  constexpr auto print_mx_fmt = "%08x ";
template <>  constexpr auto print_mx_fmt<int8_t>     = "%3d ";
template <>  constexpr auto print_mx_fmt<uint8_t>    = "%3u ";
template <>  constexpr auto print_mx_fmt<int16_t>    = "%4d ";
template <>  constexpr auto print_mx_fmt<uint16_t>   = "%4u ";
#ifdef __arm__
template <>  constexpr auto print_mx_fmt<int32_t>    = "%5d ";
template <>  constexpr auto print_mx_fmt<uint32_t>   = "%5u ";
#else
template <>  constexpr auto print_mx_fmt<int32_t>    = "%5d ";
template <>  constexpr auto print_mx_fmt<uint32_t>   = "%5u ";
#endif
template <>  constexpr auto print_mx_fmt<float>      = "%-5.2f ";

inline float print_mx_q2float(const q7_t v)  { return Q7_TO_FLOAT(v);  }
inline float print_mx_q2float(const q15_t v) { return Q15_TO_FLOAT(v); }
inline float print_mx_q2float(const q31_t v) { return Q15_TO_FLOAT(v); }


// Print a w x h matrix.
// Element order is 0     1 ... (w-1)
//                  w   w+1 ... (2*w-1)
//                  2*w ...
//
// Parameter 'stride': number of elements per row.
template <typename Tp> void print_matrix(const Tp *m, int w, int h, size_t stride)
{
    int i, j;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            printf(print_mx_fmt<Tp>, m[i*stride + j]);
        }
        printf("\n");
    }
}

template <typename Tq> 
void print_matrix_f(const Tq *m, int w, int h, size_t stride)
{
    int i, j;

    for (i = 0; i < h; i++) {
        for (j = 0; j < w; j++) {
            printf(print_mx_fmt<float>, print_mx_q2float(m[i*stride + j]));
        }
        printf("\n");
    }
}

// Print a w x h Beco accumulator.
// Element order is (w-1) ...  1  0
//                  ...
//
// Parameter 'stride': number of beco_vec32_out_t's per row.
template <typename Tp>
void print_vector32(const Tp *tm, int w, int h, size_t stride)
{
    int i, j;
    stride = stride*sizeof(beco_vec32_out_t) / sizeof(Tp);

    for (i = 0; i < h; i++) {
        for (j = w - 1; j >= 0; j--) {
            printf(print_mx_fmt<Tp>, tm[i*stride + j]);
        }
        printf("\n");
    }
}

#endif
