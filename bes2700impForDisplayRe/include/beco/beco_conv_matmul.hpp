#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Support functions to micro-tile matrix-multiply

#ifndef _BECO_CONV_MATMUL_HPP
#define _BECO_CONV_MATMUL_HPP

#include "beco_common.h"
#include "beco_types.h"

#include <stddef.h>


namespace Beco {

#if 1
#define DEBUG_TEXT(s,...)
#define DEBUG_TILE()
#else
#define DEBUG_TEXT(s,...) printf(s, __VA_ARGS__)
#define DEBUG_TILE() \
        printf("im:");   print_matrix(p, shape_h, 1, 1);  \
        printf("wt:");   print_matrix(w, shape_w, 1, 1)
#endif
#define AS_VEC32(p) ((beco_vec32_in_t){.u32 = { p} })

template <int shape_w, int shape_h, int wt_stride, typename wt_t, typename in_t> class MatmulT {

    //
    // Deduce a compatible accumulator output element size.
    //
    template <int n_acc_bundle, typename T> struct acc_output_t;
    template <typename T> struct acc_output_t< 1, T > {
        static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
        typedef T type;
    };
    template <typename T> class acc_output_t< 2, T > {
        static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
        typedef typename std::conditional<std::is_signed<T>::value, int16_t, uint16_t>::type xint16_t;
        static constexpr bool is_single { sizeof(T) == 1 };
    public:
        typedef typename std::conditional<is_single, xint16_t, T>::type type;
    };
    template <typename T> class acc_output_t< 4, T > {
        static_assert(sizeof(T) <= 4, "Output element size must be 8,16 or 32bit");
        typedef typename std::conditional<std::is_signed<T>::value, int32_t, uint32_t>::type xint32_t;
        static constexpr bool is_32bit { sizeof(T) == 4 };
    public:
        // handle float:
        typedef typename std::conditional<is_32bit, T, xint32_t>::type type;
    };

    template <typename a_t, typename b_t> struct BundledAccs {
        enum { value = sizeof(a_t) * sizeof(b_t) };
    };

    template <int w, int h> struct TileShape {};

    static inline void outer_product_impl(const void *pvw, const void *pvp, TileShape<8,8>)
    {
#ifdef BECO_PRE_V1
        const beco_vec32_in_t *w = (const beco_vec32_in_t *)pvw;
        const beco_vec32_in_t *p = (const beco_vec32_in_t *)pvp;

        beco_mmac(BECO_ACC0, w[0], p[0]);
        beco_mmac(BECO_ACC1, w[0], p[1]);
        beco_mmac(BECO_ACC2, w[1], p[0]);
        beco_mmac(BECO_ACC3, w[1], p[1]);
#else
        const uint32_t *w = (const uint32_t *)pvw;
        const uint32_t *p = (const uint32_t *)pvp;

        beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = { p[0], p[1] } }));
        beco_write_reg(BECO_REG3, ((beco_vec64_in_t){.u32 = { w[0], w[1] } }));
        beco_mmacrr(BECO_REG3, BECO_REG2);
#endif
    }

    static inline void outer_product_impl(const void *pvw, const void *pvp, TileShape<4,16>)
    {
        const beco_vec32_in_t *w = (const beco_vec32_in_t *)pvw;
        const beco_vec32_in_t *p = (const beco_vec32_in_t *)pvp;

        beco_mmac(BECO_ACC0, w[0], p[0]);
        beco_mmac(BECO_ACC2, w[0], p[1]);
        beco_mmac(BECO_ACC1, w[0], p[2]);
        beco_mmac(BECO_ACC3, w[0], p[3]);
    }

    static inline void outer_product_impl(const void *pvw, const void *pvp, TileShape<2,32>)
    {
        const uint16_t *w = (const uint16_t *)pvw;
        const beco_vec64_in_t *p = (const beco_vec64_in_t *)pvp;
        const beco_vec32_in_t w_vec {.u16 = { w[0] }};

        // only low 16bit of w is valid.
        beco_write_reg(BECO_REG2, p[0]);
        beco_write_reg(BECO_REG3, p[1]);
        beco_mmacgr(BECO_ACC0, w_vec, BECO_REG2);
        beco_mmacgr(BECO_ACC2, w_vec, BECO_REG3);
        beco_write_reg(BECO_REG2, p[2]);
        beco_write_reg(BECO_REG3, p[3]);
        beco_mmacgr(BECO_ACC0, w_vec, BECO_REG2);
        beco_mmacgr(BECO_ACC2, w_vec, BECO_REG3);
    }

public:

    template <typename out_t> struct AccType {
        typedef typename acc_output_t< BundledAccs<wt_t, in_t>::value, out_t >::type type;
    };

    static inline void outer_product(const wt_t *w, const in_t *p)
    {
        // Call generic methods of the right size
        outer_product_impl((const void*)w, (const void*)p,
            TileShape<shape_w*sizeof(wt_t), shape_h*sizeof(in_t)>());
    }


    static void calculate(const wt_t *w, const in_t *p, int n)
    {
        DEBUG_TEXT("-------(mults %d - WxH=%dx%d)\n", n, shape_w*sizeof(wt_t),shape_h*sizeof(in_t));

        while (n > 0) {
            DEBUG_TILE();

            outer_product(w, p);

            p += shape_h;
            w += wt_stride;
            n -= 1;
        }
    }
};

};

#endif
