#pragma once
//
// Copyright 2020 BES Technic
//
// Description: Beco convolve with CMSIS data-layout
//

#ifndef _BECO_CONVOLVE_H
#define _BECO_CONVOLVE_H

#include "beco_common.h"

#include <assert.h>

#include "beco.h"
#include "beco_types.h"
#include "beco_bias.hpp"

//#include "beco_print_matrix.hpp"

#include "beco_conv_readout.hpp"
#include "beco_conv_matmul.hpp"



#define DO7BIT 0



namespace Beco {

#define BECO_TFCONVNET(id) \
            id ##_IN_CH,    id ##_IN_DIM_X,  id ##_IN_DIM_Y,  \
            id ##_OUT_CH,   id ##_OUT_DIM_X, id ##_OUT_DIM_Y, \
            id ##_PAD_X,    id ##_PAD_Y,                      \
            id ##_STRIDE_X, id ##_STRIDE_Y,                   \
            id ##_KER_DIM_X,id ##_KER_DIM_Y

#ifndef BECO_DEBUG
#define BECO_DEBUG_PRIVATE private
#else
#define BECO_DEBUG_PRIVATE public
#endif

//
// CopyPolicy
//
// must implement a copy_out(int16_t *op, const int32_t *ip, int n) method
class CopyNormal
{
public:
    static void copy_out(int16_t *out, const int32_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_SSAT(ip[0], 16);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int8_t *out, const int16_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_SSAT(ip[0], 8);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int8_t *out, const int32_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_SSAT(ip[0], 8);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int8_t *out, const int8_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            memcpy(out, in, w*sizeof(*out));
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int16_t *out, const int16_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            memcpy(out, in, w*sizeof(*out));
            out += out_stride;
            in += in_stride;
        }
    }

};

class CopyReLU {
public:
    static void copy_out(int16_t *out, const int32_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_USAT(ip[0], 15);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int8_t *out, const int16_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_USAT(ip[0], 7);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static void copy_out(int8_t *out, const int32_t *in, int w, int h,
                         int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_USAT(ip[0], 7);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static inline void copy_out(int8_t *out, const int8_t *in, int w, int h,
                                int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_USAT(ip[0], 7);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
    static inline void copy_out(int16_t *out, const int16_t *in, int w, int h,
                                int out_stride, int in_stride)
    {
        for (int j = 0; j < h; j++) {
            typeof(out) p = out;
            typeof(in ) ip = in;
            for (int i = 0; i < w; i++) {
                *p++ = BECO_USAT(ip[0], 15);
                ip += 1;
            }
            out += out_stride;
            in += in_stride;
        }
    }
};

//
// Read method
//
template <
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY, int SIMDWidth>
class ReadDummy {
public:
    static void getStrip(int y, const int16_t * restrict pim, int16_t * restrict pout) {
    }

    void flush() {
    }

    inline bool valid(int ky) {
        return true;
    }

    void update(int iy, const q15_t *im) {
    }

    inline bool get(int ky, int16_t *&col) {
        return true;
    }
};

#include "beco_conv_read_hwc.hpp"


template <class Parameters>
    class ReadHWC : public Parameters {

    using Parameters::SIMDWidth;
    using Parameters::inWidth;
    using Parameters::inHeight;
    using Parameters::outWidth;
    using Parameters::outHeight;
    using Parameters::outChannels;
    using Parameters::inChannels;
    using Parameters::kernelWidth;
    using Parameters::kernelHeight;
    using Parameters::padWidth;
    using Parameters::padHeight;
    using Parameters::strideXIncr;
    using Parameters::strideYIncr;

    using typename Parameters::in_t;
    using typename Parameters::out_t;
    using typename Parameters::wt_t;
    using typename Parameters::bias_t;

    static constexpr int kernelSize    { inChannels * kernelWidth*kernelHeight * outWidth };
    static constexpr int stripSize     { SIMDWidth * kernelSize };
    static constexpr int padStartX     { padWidth };
    static constexpr int padEndX       { outWidth*strideXIncr + kernelWidth - 1 - inWidth - padWidth };
    static_assert(padWidth < kernelWidth && padEndX < kernelWidth,
                    "Padding must be smaller than kernel.");

    template <typename T>
        void swap(T &a, T&b) {T c; c=a; a=b; b=c; }

    in_t     *imcache[kernelHeight];
    bool     cache_valid[kernelHeight];
    int      starty;
    uint32_t im_kernel[ stripSize ]; // FIXME: too big? maybe acc_t?

    static void deb(const in_t *pout, int y)
    {
#if 0
        int x, kx, i;
        printf("y=%d\n", y);
        for (x = 0; x < outWidth; x += SIMDWidth) {
            for (kx = 0; kx < kernelWidth; kx++) {
                for (i = 0; i < inChannels; i++) {
                    printf("ch%2d kx%2d x%2d: %2d %2d %2d %2d\n", i, kx, x, pout[0], pout[1], pout[2], pout[3]);
                    pout += 4;
                }
            }
        }
#endif
    }

public:

    ReadHWC() {
       // printf("read with method: %s\n", hwc_im2col<inChannels, strideXIncr, SIMDWidth>::id());
    }

    static void getStrip(int y, const in_t * restrict pim, in_t * restrict pout) {
        int kx;
        const in_t * restrict p;
        in_t * restrict po = pout;

        pim += inChannels*inWidth*y - inChannels*padStartX;

        for (int x = 0; x < outWidth; x+= SIMDWidth) {
            p = pim;
            for (kx = 0; kx < kernelWidth; kx++) {
                hwc_im2col<inChannels, strideXIncr, SIMDWidth>::copy_channels(pout, p);
                p += inChannels;
                pout += inChannels*SIMDWidth;
            }
            pim += SIMDWidth*strideXIncr*inChannels;
        }

        // Clear values outside input matrix
        for (int px = 0; px < padStartX; px++) {
            for (int c = 0; c < (padStartX - px)*inChannels; c++)
                po[c*SIMDWidth + px] = 0;
        }
//        printf("erase x = %d \n", padEndX);
        for (int px = 0; px < padEndX; px++) {
            for (int c = 0; c < (padEndX - px)*inChannels; c++)
                pout[-(c*SIMDWidth + px) - 1] = 0;
        }
    }

    void flush() {
        in_t *p = (in_t *)im_kernel;
        for (int i = 0; i < kernelHeight; i++) {
            imcache[i] = p;
            cache_valid[i] = false;
            p += kernelSize/kernelHeight;
        }
    }

    inline bool valid(int ky) {
        return (starty+ky >= 0 && cache_valid[ky]);
    }

    void update(int iy, const in_t *im) {
        starty = iy;

        if (kernelHeight == 1) {
            cache_valid[0] = false;
            if (iy >= 0 && iy < inHeight) {
//                printf("  GEN (k=1) strip slot:%d (y=%d)\n",0, iy);
                getStrip(iy, im, imcache[0]);
                cache_valid[0] = true;
            }
            return;
        }

        // Discard expired entries.
        if (iy != -padHeight) {
            for (int i = 0; i < kernelHeight-strideYIncr; i++) {
                swap(imcache[i], imcache[i+strideYIncr]);
                cache_valid[i] = cache_valid[i+strideYIncr];
                cache_valid[i+strideYIncr] = false;
            }
        }

        // Load new entries.
        for  (int i = 0; i < kernelHeight; i++) {
            if (iy+i >= inHeight)
                cache_valid[i] = false;
            else if (iy+i >= 0 && cache_valid[i] == false) {
//                assert (iy+i < inHeight);
//                printf("  GEN strip slot:%d (y=%d)\n",i, iy+i);
                getStrip(iy+i, im, imcache[i]);
                cache_valid[i] = true;
            }
//            deb(imcache[i], iy+i);
        }
    }

    //
    // Get im2col tile position for kernel pos ky
    //
    inline bool get(int ky, in_t *&col) {
        if (!valid(ky)) return false;
        col = imcache[ky];

        return true;
    }
};

//
// Setting Policy Parameter Argument Policy
//
template
  <
    typename IN_T,
    typename BIAS_T,
    typename WT_T,
    typename OUT_T,
    int ICH, int IW, int IH,
    int OCH, int OW, int OH,
    int PX,  int PY,
    int SX,  int SY,
    int KX,  int KY
  >
class CNParameters {

//    static_assert(OCH % 2 == 0, "Check input dimension meets the constraints.");
    static_assert(OCH*ICH % 4 ==0, "TODO: Check input channel meets the constraints.");
    static_assert(OW % 4 == 0, "Output stride must be multiple of 4");

public:
    static constexpr int numAccs = 64;

    // Return strip size in number of 16bit elements
    // Strip need 4*ICH*KX*KY*OH 16 bit elements
    //static constexpr int stripSize     { 4 * ICH * KX*KY * OH };

    static constexpr int imageSize     { ICH * IW*IH };
    static constexpr int outSize       { OCH * OW*OH };
    static constexpr int biasSize      { OCH };
    static constexpr int weightSize    { ICH * KX*KY * OCH };
    //static constexpr int matmulSize    { ICH * KX * OCH };

    static constexpr int inWidth       { IW };
    static constexpr int inHeight      { IH };
    static constexpr int outWidth      { OW };
    static constexpr int outHeight     { OH };
    static constexpr int inChannels    { ICH };
    static constexpr int outChannels   { OCH };
    static constexpr int kernelWidth   { KX };
    static constexpr int kernelHeight  { KY };
    static constexpr int padWidth      { PX };
    static constexpr int padHeight     { PY };
    static constexpr int strideXIncr   { SX };
    static constexpr int strideYIncr   { SY };

    typedef IN_T   in_t;
    typedef OUT_T  out_t;
    typedef WT_T   wt_t;
    typedef BIAS_T bias_t;


    #define BATCH(t, ch) ((ch)*sizeof(t) > 8? 8/sizeof(t): (ch))
    #define ALIGN_TO(type, N, intype) \
            (((N*sizeof(intype)+sizeof(type)-1) & (-sizeof(type))) / sizeof(intype))

    // Calculate 'channel' microbatch size
    static constexpr int chanPerBatch { BATCH(WT_T, OCH) };

    template <typename a_t, typename b_t> struct BundledAccs {
        enum { value = sizeof(a_t) * sizeof(b_t) };
    };

    // SIMDWidth is the number of elements managed in parallel.
    // outChannels wt_t(size) in_t  SIMDWidth
    //     c        n          m    64/c/sizeof(wt_t)/sizeof(in_t)
    //     4        2          2    64/4/2/2 = 4 (4x16bit = 64bits)
    //     4        1          1    64/4/1/1 = 16 (16x8bit = 128bits)
    //     4        1          2    64/4/1/2 = 8  (8*16bit = 128bits)
    //     8        1          1    64/8/1/1 = 8  (8*8bit = 64bits)
    static constexpr int numBundledAccs { BundledAccs<wt_t, in_t>::value };
    static constexpr int SIMDWidth     { numAccs/numBundledAccs/chanPerBatch };

    static constexpr int weightChannelStride = ALIGN_TO(uint32_t, outChannels, WT_T);
};



template
  <
    class Parameters,
    class Copy = CopyNormal,
    template <class> class ReadT = ReadHWC
  >
class Convolve : public Parameters {

public:

    using Parameters::SIMDWidth;
    using Parameters::outWidth;
    using Parameters::outHeight;
    using Parameters::outChannels;
    using Parameters::inChannels;
    using Parameters::kernelWidth;
    using Parameters::kernelHeight;
    using Parameters::padWidth;
    using Parameters::padHeight;
    using Parameters::strideXIncr;
    using Parameters::strideYIncr;
    using Parameters::weightSize;
    using Parameters::chanPerBatch;
    using Parameters::weightChannelStride;
    using Parameters::numBundledAccs;

    using typename Parameters::in_t;
    using typename Parameters::out_t;
    using typename Parameters::wt_t;
    using typename Parameters::bias_t;

    static constexpr int matmulSize    { inChannels * kernelWidth };

    typedef MatmulT<chanPerBatch, SIMDWidth, weightChannelStride, wt_t, in_t> Matmul;

    typedef typename Matmul::template AccType<out_t>::type acc_t;

    ReadT<Parameters> Read;

public:

    Convolve() { }
//    ~Convolve() { }


private:

    static void copy_microtile(out_t *out, int n_active)
    {
        acc_t tile[chanPerBatch*SIMDWidth];

        // (4x4x16bit): 4 channels, 32bit out, 4x16bit (in)SIMDWidth -> incr 4x32bit
        //              ---A-x16--  ^._out     ^,_N             out*N = -4words------
        // (4x8bit x 8x16bit): 4 channel, 16bit out, 8x16bit -> incr 8x16bit
        //                     ---Ax8---, ^,out      ^,N    out*N = -4words--

        ReadAcc::beco_conv_read_acc<numBundledAccs, SIMDWidth, chanPerBatch>(tile);

//        printf("---------\n");
//        print_matrix((out_t*)tile, n_active, SIMDWidth, chanPerBatch);

        Copy::copy_out(out, tile, n_active, SIMDWidth, outChannels, chanPerBatch);
    }

    static inline void matmul_kernel(const wt_t *w, const in_t *in, int n)
    {
        Matmul::calculate(w, in, n);
    }

    static void set_rounding()
    {
        beco_preload_acc(BECO_ACC0, BECO_REG0, BECO_REG1);
        beco_preload_acc(BECO_ACC1, BECO_REG0, BECO_REG1);
        beco_preload_acc(BECO_ACC2, BECO_REG0, BECO_REG1);
        beco_preload_acc(BECO_ACC3, BECO_REG0, BECO_REG1);
    }

    static inline void matmul_set_bias(const bias_t *bias, const in_t *factor)
    {
        uint32_t config = beco_config();

        // factor (B) is unsigned of same type, modify config:
        beco_write_config(config & ~BECO_CONF_BTYPE_SIGNED);

        set_rounding();
        Matmul::outer_product(bias, factor);

        beco_write_config(config);
    }

public:

    void convolve(const in_t *im, out_t *out)
    {
        int x, y;   // coord in out[]
        int ix, iy; // coord in im[]

        Read.flush();

        // Configure Beco
        beco_write_config(beco_acc_config);

        // Set rounding
        beco_set_preload_16x16( round );

        iy = - padHeight;
        for (y = 0; y < outHeight; y++, iy += strideYIncr) {
            Read.update(iy, im);

            ix = 0;
            for (x = 0; x < outWidth; x += SIMDWidth) {
                for (int ch = 0; ch < outChannels; ch += chanPerBatch) {
                    wt_t *w = weights + ch;
                    int n_active = (ch > outChannels-chanPerBatch? outChannels-ch: chanPerBatch);
                    matmul_set_bias(bias + ch, bias_factor);
                    for (int ky = 0; ky < kernelHeight; ky++) {
                        in_t *col;

                        DEBUG_TEXT("k%d (y=%d) ", ky, y+ky);

                        if (Read.get(ky, col)) {
                            matmul_kernel(w, (col + ix), matmulSize);
                        }

                        w += weightChannelStride*inChannels*kernelWidth;
                    }
                    copy_microtile(out + ch, n_active);
                }
                out += outChannels*SIMDWidth;
                ix += SIMDWidth*inChannels*kernelWidth;
            }
        }
    }

    void setTruncate(bool trunc) {
        round = (trunc)? 0: q_round(out_rshift);
    }


    void convertWeights(const wt_t *wt)
    {
        // Convert layout of weights from CMSIS to Beco
        // CMSIS weights:
        //
        //  ky\kx: 0      1        KX-1
        //   0   CCC..C CCC..C ... CCC.C    <- Channel 0
        //   1   CCC..C CCC..C ... CCC.C
        //       ...
        //  KY-1 CCC..C CCC..C ... CCC.C
        //   0   CCC..C CCC..C ... CCC.C    <- Channel 1
        //  ...

        wt_t *w_aligned = weights;
        for (int ky = 0; ky < kernelHeight; ky++)
        for (int kx = 0; kx < kernelWidth; kx++)
        for (int ich = 0; ich < inChannels; ich++) {
            wt_t *w = w_aligned;
            // 0*CHSTR | ch1 | ch2 | ... | chN | 0 | 0 | <- two channel alignments
            // 1*CHSTR | ...
            for (int och = 0; och < outChannels; och++) {
                *w++ = wt[ och*inChannels*kernelWidth*kernelHeight
                         + ky*kernelWidth*inChannels
                         + kx*inChannels
                         + ich];
            }
            w_aligned += weightChannelStride;
        }
    }

    void setPlan(
            const wt_t   *wt,
            const bias_t *_bias, // bias_t == wt_t
            const uint16_t _bias_lshift,
            const uint16_t _out_rshift)
    {
        assert(_bias_lshift < sizeof(in_t)*8);

        out_rshift = _out_rshift;
        bias = _bias;

        // replicate bias multiply factor
        for (int i = 0; i < SIMDWidth; i++) {
            bias_factor[i] = 1 << _bias_lshift;
        }

        setTruncate(false);
#if DO7BIT
        beco_acc_config = BECO_CONF_BMODE_REP64 | BECO_CONF_AMODE_REP16 |
                BECO_CONF_ATYPE_INT8 | BECO_CONF_BTYPE_INT8 |
                BECO_CONF_RSHIFT(out_rshift) |
                BECO_CONF_PACK_INT32 | BECO_CONF_RD_8x8; //_ROT90;
#else
        beco_acc_config = BECO_CONF_BMODE_MAT | BECO_CONF_AMODE_REP32 |
                BECO_CONF_ATYPE_INT16 | BECO_CONF_BTYPE_INT16 |
                BECO_CONF_RSHIFT(out_rshift) |
                BECO_CONF_PACK_INT32 | BECO_CONF_RD_16x16; //_ROT90;
#endif

        convertWeights(wt);
    }

private:

    uint32_t beco_acc_config;

    int round;
    uint16_t out_rshift;

    const wt_t  *bias;
    in_t        bias_factor[ SIMDWidth ];

BECO_DEBUG_PRIVATE:
    wt_t weights[ weightSize ];
};

}
#endif
