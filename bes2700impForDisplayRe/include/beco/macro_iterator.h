#pragma once
//
// Copyright 2020 BES Technic
//
// C-preprocessor iterator and list abstraction.

/*
A basic example is:

    #define PRINT(v, ...) {P1 v, P2 v },
    #define LOG2(seq) \
       FORALL(PRINT, , EXPAND seq)
    int lookup_log2[][2] = {
	    EVAL(LOG2( ((2,1), (4,2), (8,3), (16,4)) ))
    };

Which generate:

    int lookup_log2[][2] = {
        { 2, 1 }, { 4, 2 }, { 8, 3 }, { 16, 4 },
    };
*/

/*
Below is a somewhat advanced example show how to have nested expansion
and and the use of lists

    #define OUT_READ(lane, acc) \
        rd[P1 lane + P1 acc] = beco_read_acc(P2 acc, P2 lane);

    #define INNER(nn, seq) \
       FORALL(OUT_READ, nn, EXPAND seq)
        
    #define BECO_GEN_READ(name, accs, seq) \
        void name(beco_vec32_out_t rd[], size_t stride) {  \
            EVAL(FORALL(INNER, seq, EXPAND accs))          \
        }

    BECO_GEN_READ(beco_read_acc0_8x8_8bit, \
           (                 \
             (0,BECO_ACC0),  \
             (4,BECO_ACC1)   \
           ),                \
           (                 \
             (0+0*stride,0), \
             (0+1*stride,4), \
             (0+2*stride,8), \
             (0+3*stride,12) \
           ) );

This example expands to:

    void beco_read_acc0_8x8_8bit(beco_vec32_out_t rd[], size_t stride)
    {
        rd[0 + 0 * stride + 0] = beco_read_acc(BECO_ACC0, 0);
        rd[0 + 1 * stride + 0] = beco_read_acc(BECO_ACC0, 4);
        rd[0 + 2 * stride + 0] = beco_read_acc(BECO_ACC0, 8);
        rd[0 + 3 * stride + 0] = beco_read_acc(BECO_ACC0, 12);
        rd[0 + 0 * stride + 4] = beco_read_acc(BECO_ACC1, 0);
        rd[0 + 1 * stride + 4] = beco_read_acc(BECO_ACC1, 4);
        rd[0 + 2 * stride + 4] = beco_read_acc(BECO_ACC1, 8);
        rd[0 + 3 * stride + 4] = beco_read_acc(BECO_ACC1, 12);
    }


*/


/* Implementation */

#define EVAL(...)  EVAL1(EVAL1(EVAL1(__VA_ARGS__)))
#define EVAL1(...) EVAL2(EVAL2(EVAL2(__VA_ARGS__)))
#define EVAL2(...) EVAL3(EVAL3(EVAL3(__VA_ARGS__)))
#define EVAL3(...) EVAL4(EVAL4(EVAL4(__VA_ARGS__)))
#define EVAL4(...) EVAL5(EVAL5(EVAL5(__VA_ARGS__)))
#define EVAL5(...) __VA_ARGS__

#define P1(p1,p2) p1
#define P2(p1,p2) p2

#define EMPTY(...)
#define DEFER(...) __VA_ARGS__ EMPTY()
#define CALL(...)  __VA_ARGS__ DEFER(EMPTY)()
	
#define TAIL(a, ...) __VA_ARGS__
#define HEAD(a, ...) a
#define EAT(...)
#define EXPAND(...) __VA_ARGS__

#define TEST_EMPTY(_0,_1,_2,_3,_4,_5,_11,_12,_13,_14,_21,_22,_23,_24,_31,_32,_33,_34, action, ...) action
#define IF_NEMPTY(...)    TEST_EMPTY(0, ##__VA_ARGS__, EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EXPAND,EAT)

#define FORALL(macro, S, ...) \
    IF_NEMPTY(__VA_ARGS__) ( \
        CALL(macro) ( \
            HEAD(__VA_ARGS__),S \
        ) \
        CALL(FORALL_INDIRECT) () ( \
            macro, S, TAIL(__VA_ARGS__) \
        ) \
    )
#define FORALL_INDIRECT() FORALL


