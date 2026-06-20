#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Support functions to read HWC layout (im2col)

#ifndef _BECO_CONV_READHWC_HPP
#define _BECO_CONV_READHWC_HPP

#include "beco_common.h"
#include "beco_types.h"

#include <stddef.h>



#if 0

//template <int inChannels, int strideXIncr, int SIMDWidth> struct hwc_im2col;

template <int inChannels, int strideXIncr, int SIMDWidth> struct hwc_im2col
 {
    // Inline methods
    static const char* id() {return "type: int16_t, general stride, inChannels % 2";}

    static inline void rotate_words_90(uint32_t *pout, int stride, uint32_t a, uint32_t b)
    {
            BECO_ROT(a, b, 0xffff, 16);
            pout[0] = a;
            pout[stride] = b;
    }

    static void copy_channels(int16_t *_pout, const int16_t * restrict _pin)
    {
        // Calculate X increment in 32bit words
        const int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(*_pin));

        for (int c = 0; c < inChannels; c += 2) {

            const uint32_t * restrict pin = (const uint32_t *)_pin;
            uint32_t * restrict pout = (uint32_t *)_pout;

            // Assume SIMDWidth is 4 or 8 elements
            for (int x = 0; x < SIMDWidth; x += 4) {
                rotate_words_90(pout,     SIMDWidth/2, pin[0],      pin[step]);
                rotate_words_90(pout + 1, SIMDWidth/2, pin[2*step], pin[3*step]);
                pin += 4*step;
                pout += 2;
            }
            _pin += 2; // handle next two channels
            _pout += SIMDWidth * 2; // loop handled 2 channels of SIMDWidth each.
        }
    }
};
#endif

#if 0
template <int inChannels, int strideXIncr, int SIMDWidth>
    struct hwc_im2col
 {
    typedef uint32_t * restrict uint32_tpr;
    
    // Inline methods
    static const char* id() {return "type: int8_t, general stride, inChannels % 4";}


    static inline void rot90_4x4_8bit(uint32_t *restrict pout, const uint32_t *restrict pin)
    {
        uint32_t v0,v1,v2,v3;

        static constexpr int stride = SIMDWidth/2;
        static constexpr int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(int8_t));

        for (int x = 0; x < SIMDWidth; x += 4) {

            // read 4 same channel#'s at x=x0, x0+1,...
            v0 = pin[0]; // FIXME: also support channel not %4 (use byte pointer :-s)
            v1 = pin[step];
            v2 = pin[2*step];
            v3 = pin[3*step];
            BECO_ROT(v0,v1, 0x00ff00ff,  8);
            BECO_ROT(v2,v3, 0x00ff00ff,  8);
            BECO_ROT(v0,v2, 0x0000ffff, 16);
            BECO_ROT(v1,v3, 0x0000ffff, 16);
            pout[0*stride] = v0;
            pout[1*stride] = v1;
            pout[2*stride] = v2;
            pout[3*stride] = v3;

            pin += 4*step;
            pout += 1;
        }
    }

    static inline void rot90_2x4_8bit(uint32_t *restrict pout, const uint32_t *restrict pin)
    {
        uint32_t v0,v1;

        static constexpr int stride = SIMDWidth/2;
        static constexpr int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(int8_t));

        for (int x = 0; x < SIMDWidth; x += 4) {

            v0 = pin[0];
            v1 = pin[step];
            BECO_ROT(v0,v1, 0x0000ffff, 16);    
            BECO_ROT(v0,v1, 0x00ff00ff, 8);
            pout[0*stride] = v0;
            pout[1*stride] = v1;

            pin += 2*step;
            pout += 1;
        }
    }

    // Local (h)WC -> (h)CW (deinterleave channels)
    // read 4 inputs of 2 channels each HC and rotate to
    // produce two lines of 4 inputs 

    static inline void rot90_2x4_16bit(uint32_t * restrict pout, const uint32_t * restrict pin)
    {
        uint32_t v0,v1,v2,v3;

        static constexpr int stride = SIMDWidth/2;
        static constexpr int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(int16_t));

        for (int x = 0; x < SIMDWidth; x += 4) {
            v0 = pin[0];
            v1 = pin[step];
            v2 = pin[2*step];
            v3 = pin[3*step];
            BECO_ROT(v0, v1, 0xffff, 16);
            BECO_ROT(v2, v3, 0xffff, 16);
            pout[0]        = v0;
            pout[1]        = v2;
            pout[stride]   = v1;
            pout[stride+1] = v3;

            pin += 4*step;
            pout += 2;
        }
    }

    static void copy_channels(int16_t *_pout, const int16_t * restrict _pin)
    {
        // Calculate X increment in 32bit words
        const int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(*_pin));

        for (int c = 0; c < inChannels; c += 2) {
            rot90_2x4_16bit((uint32_t *)_pout, (const uint32_t *)_pin);
            _pin += 2; // handle next two channels
            _pout += SIMDWidth * 2; // loop handled 2 channels of SIMDWidth each.
        }
    }

    static void copy_channels(int8_t *_pout, const int8_t * restrict _pin)
    {
        // Calculate X increment in 32bit words
        const int step = inChannels*strideXIncr / (sizeof(uint32_t)/sizeof(*_pin));

        for (int c = 0; c < inChannels; c += 4) {
            rot90_4x4_8bit((uint32_t *)_pout, (const uint32_t *)_pin);
            _pin += 4; // handle next four channels
            _pout += SIMDWidth * 2; // loop handled 2 channels of SIMDWidth each.
        }
/*        if (inChannels % 4 != 0) {
            rot90_2x4_8bit((uint16_t *)_pout, (const uint16_t *)_pin);
            _pin += 2; // handle next two channels
            _pout += SIMDWidth * 2; // loop handled 2 channels of SIMDWidth each.
        }*/
    }
};

#else

template <int inChannels, int strideXIncr, int SIMDWidth>//, typename T>
    struct hwc_im2col;

template <int inChannels, int strideXIncr, int SIMDWidth> 
    struct hwc_im2col//<inChannels, strideXIncr, SIMDWidth>
{
    static const char* id() {return "type: int16_t, inchannels=1, general stride";}
    static void copy_channels(int16_t *_pout, const int16_t * restrict _pin)
    {
        for (int c = 0; c < inChannels; c++) {
            typeof(_pin) pin = _pin + c;
            for (int x = 0; x < SIMDWidth; x++) {
                *_pout++ = *pin;
                pin += strideXIncr*inChannels;
            }
        }
    }
    static void copy_channels(int8_t *_pout, const int8_t *restrict _pin)
    {
        for (int c = 0; c < inChannels; c++) {
            typeof(_pin) pin = _pin + c;
            for (int x = 0; x < SIMDWidth; x++) {
                *_pout++ = *pin;
                pin += strideXIncr*inChannels;
            }
        }
    }
};
#endif

template <int strideXIncr, int SIMDWidth>
    struct hwc_im2col<1, strideXIncr, SIMDWidth>
{
    static const char* id() {return "type: int16_t, inchannels=1, general stride";}
    static void copy_channels(int16_t *_pout, const int16_t * restrict _pin)
    {
        for (int c = 0; c < SIMDWidth; _pin += strideXIncr, c++)
            *_pout++ = *_pin;
    }
};

template <int SIMDWidth>
    struct hwc_im2col<1, 1, SIMDWidth>
{
    static const char* id() {return "type: int16_t, inchannels=1, stride=1";}
    static void copy_channels(int16_t *_pout, const int16_t * restrict _pin)
    {
        // require unaligned access:
        memcpy(_pout, _pin, sizeof(*_pin)*SIMDWidth);
    }
};


#endif

