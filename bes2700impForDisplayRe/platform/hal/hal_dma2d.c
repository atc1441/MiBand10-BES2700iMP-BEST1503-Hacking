/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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

/**
 * DOC: DMA2D is a graphic accelerator which offers advanced bit blitting, row data copy
 * and pixel format conversion.
 *
 *It supports the following functions:

 * Filling a part or the whole of a destination image with a specific color
 * Copying a part or the whole of a source image into a part or the whole of a
 * destination image.
 * Copying a part or the whole of a source image into a part or the whole of a
 * destination image with a pixel format conversion.
 * Blending a part and/or three complete source images with different pixel
 * format and copy the result into a part or the whole of a destination image
 *  with a different color format.
 * Various image format coding are supported, from indirect 4bpp color mode up to
 * 32bpp direct color, include RGB565, RGB1555, RGB888 packed, RGB888 unpacked,
 * RGBA888, YUV422, YUV422 planar, YUV420 planar, Smart Panel command,
 * Palette color 4-bit per pixel, Palette color 8-bit per pixel, RGB888A.
 * It embeds dedicated memory to store color lookup tables.
 * An interrupt can be generated when an operation is complete or at a programmed
 * watermark.
 *
 * DMA2D hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,
 *   YUV422packed,YUV422 planar, YUV420 planar, Palette color 4-bit per pixel,
 *   Palette color 8-bit per pixel, RGBA )
  * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,
  *  YUV422 packed,Palette color 4-bit per pixel,Palette color 8-bit per pixel,RGB8A )
 *  (OL color fmt:RGB565 RGB888 ARGB888 RGBA888)
 *
 *  |FG Layer(panel video dma layer) |--->|
 *                                        | Alpha_FG (pix/c)
 *                                        V
 *                                      |BLENDING|-->|Smart Panel| -|Dither|->|
 *                                        ^     Alpha_O (pix/c)               |
 *                                        | Alpha_BG (pix/c)                  |
 *  |BG Layer(panel graphic dma layer)|-->|                                   |
 *                                                                            |
 *  |Output Layer(write back layer)|<------------------------------------------
 *  BLENDING:
 *  alpha const mode:
 *     Alpha_FG + Alpha_BG = 255
 *     O_color = (FG_c * Alpha_FG + BG_c * Alpha_BG )/255
 *
*/


#include "string.h"
#include "stdio.h"
#include "hal_dma2d.h"
#include "hal_trace.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "reg_dma2d.h"

#ifdef IMGDMA_BASE

#ifdef NULL
#undef NULL
#define NULL  (0)
#endif

#ifdef GRAPHIC_DRV_DEBUG
#define DMA2D_INFO(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define DMA2D_REGINFO(...)   TR_INFO( TR_MOD(TEST),__VA_ARGS__)
#else
#define DMA2D_INFO(...)
#define DMA2D_REGINFO(...)
#endif


#define DMA2D_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define DMA2D_ERR(...)  TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

/**
 * Private Types
 */
static volatile bool dma2d_rbswap = 0;

/* DMA2D memory start address register */

static const uintptr_t hal_dma2d_mar_layer_t[DMA2D_NLAYERS] = {
    DMA2D_FGMAR,
    DMA2D_BGMAR,
    DMA2D_OMAR,
    NULL,
};

/* DMA2D pfc control register */

static const uintptr_t hal_dma2d_pfccr_layer_t[DMA2D_NLAYERS] = {
    DMA2D_FGPFCCR,
    DMA2D_BGPFCCR,
    DMA2D_OPFCCR,
    DMA2D_SMPNCR
};

/* DMA2D Layer pitch register */
static const uintptr_t hal_dma2d_lpitch_layer_t[DMA2D_NLAYERS] = {
    DMA2D_PDMAPCHCR,
    DMA2D_PGRAPCHCR,
    DMA2D_WBPCHCR,
    NULL
};

/* DMA2D Layer size register */

static const uintptr_t hal_dma2d_lszr_layer_t[DMA2D_NLAYERS] = {
    DMA2D_PDMAAWCR,
    DMA2D_PGRAAWCR,
    DMA2D_WBAWCR,
    DMA2D_SPUAWCR,
};

/*F/B Layer dma zoom Register Address*/

static const uintptr_t hal_dma2d_zoomr_layer_t[DMA2D_NLAYERS] = {
    DMA2D_DMAZOOMCR,
    DMA2D_GRAZOOMCR,
    NULL,
    NULL,
};

/*F/B Layer start point on screen Register Address*/

static const uintptr_t hal_dma2d_sponscreen_layer_t[DMA2D_NLAYERS] = {
    DMA2D_CFG_PDMA_SPONSCREENCR,
    DMA2D_CFG_PGRA_SPONSCREENCR,
    NULL,
    NULL,
};

/* DMA2D color register */

static const uintptr_t hal_dma2d_color_layer_t[DMA2D_NLAYERS] = {
    NULL,
    NULL,
    NULL,
    DMA2D_SPUCOLR
};

/**
 * Private Function Prototypes
 */

/**
 * Private Functions
 */

#define getreg32(a)          (*(volatile uint32_t *)(a))

#define putreg32(v,a)        do { \
                                   DMA2D_REGINFO("putreg32(0x%x,0x%x);",(v), (a)) ; \
                                   *(volatile uint32_t *)(a) = (v); \
                              } while(0)

#define reg32_bitset(v,a)    do { \
                                   DMA2D_REGINFO("reg32_bitset(0x%x,0x%x);", v, a); \
                                   *(volatile uint32_t *)(a) |= v; \
                              } while(0)

#define reg32_bitclr(v,a)    do { \
                                   DMA2D_REGINFO("reg32_bitclr(0x%x,0x%x);", v, a); \
                                   *(volatile uint32_t *)(a) &= ~(v); \
                              } while(0)

/**
 * dam2d hal Public API
 */

/**
 * hal_dma2d_irqn - get  dma2d irq  number
 */

int hal_dma2d_irqn(void)
{
    return IMGDMA_IRQn;
}

/**
 * hal_dma2d_get_irqstat - get dma2d irq status.
 */
uint32_t hal_dma2d_get_irqstat(void)
{
    uint32_t sreg = getreg32(DMA2D_ISR);;
    return sreg;
}

/**
 * hal_dma2d_spu_framedone_irq_enable - enable/disable spu frame done irq.
 */
int hal_dma2d_spu_framedone_irq_enable(bool ena)
{
    int ret = 0;
    uint32_t crreg;
    DMA2D_INFO("%s enable:%d", __func__, ena);
    crreg = getreg32(DMA2D_IER);
    crreg &= ~SPU_FRAMEDONE_IRQ_ENA_MASK;
    if (ena)
        crreg |= SPU_FRAMEDONE_IRQ_ENA;
    putreg32(crreg, DMA2D_IER);

    return ret;
}

/**
 * hal_dma2d_spu_framedone_irqstat_clr - clear spu frame done irq.
 */
int hal_dma2d_spu_framedone_irqstat_clr(void)
{
    int ret = 0;
    uint32_t crreg;
    DMA2D_INFO("%s ", __func__);
    crreg = getreg32(DMA2D_ISR);
    crreg &= ~(SPU_FRAMEDONE_CLEAN);
    putreg32(crreg, DMA2D_ISR);

    return ret;
}

/**
 * hal_dma2d_irq_is_framedone - Reture whether framedone irq has come or not
 *
 * Returns:   TRUE on irq has come, FALSE on irq not come.
 */
bool hal_dma2d_irq_is_framedone(uint32_t regval)
{
    return ! !(regval & SPU_TXC_FRAMEDONE);
}

/**
 * hal_dma2d_irq_disable - mask all dma2d_irqs
 *
 */

int hal_dma2d_irq_disable(void)
{
    int ret = 0;
    uint32_t crreg = 0;
    putreg32(crreg, DMA2D_IER);
    return ret;
}

/**
 * hal_dma2d_irq_is_error - Get whether dma2d error irq status is ture or not.
 * @regval : dma2d ISR value
 *
 * Returns:  TRUE on erro irq, FALSE on without erro irq.
 */
bool hal_dma2d_irq_is_error(uint32_t regval)
{
    return ! !(regval &
           (DMA2D_ERR_IRQ || BGL_GRA_FF_UNDERFLOW
            || FGL_DMA_FF_UNDERFLOW));
}

/**
 * hal_dma2d_error_irq_enable - enable  all dma2d_error irqs
 */
int hal_dma2d_error_irq_enable(void)
{
    int ret = 0;
    uint32_t crreg = getreg32(DMA2D_IER);
    crreg |= DMA2D_ERR_IRQ_ENA;
    //crreg |= FGL_PDMA_FF_UNDERFLOW_ENA;
    //crreg |= BGL_GRA_FF_UNDERFLOW_ENA;
    putreg32(crreg, DMA2D_IER);
    return ret;
}

int hal_dma2d_underflow_irq_enable(void)
{
    int ret = 0;
    uint32_t crreg = getreg32(DMA2D_IER);
    crreg |= FGL_PDMA_FF_UNDERFLOW_ENA;
    crreg |= BGL_GRA_FF_UNDERFLOW_ENA;
    putreg32(crreg, DMA2D_IER);
    return ret;
}

/**
 * hal_dma2d_error_irq_clr - clear all dma2d_error irqs
 */
int hal_dma2d_error_irqstat_clr(void)
{
    int ret = 0;
    //reg32_bitset(DMA2D_ERR_IRQ_CLEAN, DMA2D_ICR);
    getreg32(DMA2D_ICR);
    return ret;
}

/**
 * hal_dma2d_start - Starts the dma transfer process.
 */

int hal_dma2d_start(void)
{
    int ret = 0;
    uint32_t crreg;
    getreg32(DMA2D_ICR);
    crreg = getreg32(DMA2D_STARTCR);
    crreg &= ~DMA2D_STARTCR_MASK;
    //putreg32(crreg, DMA2D_STARTCR);
    crreg |= DMA2D_STARTCR_ENA(1);
    putreg32(crreg, DMA2D_STARTCR);
    hal_dma2d_underflow_irq_enable();
    DMA2D_REP("%s\n", __func__);
    return ret;
}

/**
 * hal_dma2d_reset - reset dma2d configuration
 *
 * registers reset
 */
void hal_dma2d_reset(void)
{
    DMA2D_REGINFO("hal_dma2d_reset();");
    hal_cmu_dma2d_reset_set();
    hal_cmu_dma2d_reset_clear();
}

/**
 * hal_dma2d_panelpath_dmatrans_trigger_enable - enable/disable dma2d panel dma trigger
 *@lid  : Layer id
 *@ena  : enable/disable
 */
void hal_dma2d_panelpath_dmatrans_trigger_enable(int lid, bool ena)
{
    uint32_t crreg;

    DMA2D_INFO("lid=%d, ena=%08x\n", lid, ena);
    crreg = getreg32(hal_dma2d_pfccr_layer_t[lid] + 4);
    if (ena && (crreg & DMA2D_xGPFCCR1_TRANS_TRIGGER_MASK)) {
        return;
    }
    DMA2D_INFO("lid=%d, set  ena=%08x\n", lid, ena);
    crreg &= ~DMA2D_xGPFCCR1_TRANS_TRIGGER_MASK;
    crreg |= DMA2D_xGPFCCR1_TRANS_TRIGGER_ENA(ena);
    putreg32(crreg, hal_dma2d_pfccr_layer_t[lid] + 4);
}

/**
 * hal_dma2d_disp_avsync_trigger_disable - enable/disable dma2d disp avsync trigger
 * @dis  : disable/enable
 */
void hal_dma2d_disp_avsync_trigger_disable(bool dis)
{
    uint32_t crreg;
    crreg = getreg32(DMA2D_DISPCNFCR);
    crreg &= ~DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE_MASK;
    crreg |= DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE(dis);
    putreg32(crreg, DMA2D_DISPCNFCR);
}

/**
 * hal_dma2d_disp_svsync_trigger_disable - enable/disable dma2d disp svsync trigger
 * @dis  disable/enable
 */
void hal_dma2d_disp_svsync_trigger_disable(bool dis)
{
    uint32_t crreg;
    crreg = getreg32(DMA2D_DISPCNFCR);
    crreg &= ~DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE_MASK;
    crreg |= DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE(dis);
    putreg32(crreg, DMA2D_DISPCNFCR);
}

/**
 * hal_dma2d_disp_mem_enable - enable/disable dma2d disp memory
 * @dis     - enable/disable
 */
void hal_dma2d_disp_mem_enable(bool ena)
{
    uint32_t crreg;
    crreg = getreg32(DMA2D_DISPCNFCR);
    crreg &= ~DMA2D_DISPCNFCR_MEM_ENABLE_MASK;
    crreg |= DMA2D_DISPCNFCR_MEM_ENABLE(ena);
    putreg32(crreg, DMA2D_DISPCNFCR);
}

/**
 * hal_dma2d_lcolor - fill the color for the layer
 * @lid  - Layer index (output, foreground, background, spu )
 * @argb - argb8888 color
 *
 */

void hal_dma2d_lcolor(int lid, uint32_t argb)
{
    DMA2D_INFO("lid=%d, argb=%08x\n", lid, argb);
    ASSERT((lid == DMA2D_LAYER_SPU), "%s invalid layer ", __func__);
//    putreg32(dma2d_lcolor(argb), hal_dma2d_color_layer_t[lid]);
    putreg32(argb, hal_dma2d_color_layer_t[lid]);
}

/**
 * hal_dma2d_lpfc_swaprb - enable RB byte order swap
 * @lid  - Layer index (output, foreground, background, spu )
 * @ena  - swap or not
 */
void hal_dma2d_lpfc_swaprb(int lid, bool ena)
{
    uint32_t crreg;
    DMA2D_INFO("%s lid=%d, ena=%08x\n", __func__, lid, ena);
    ASSERT((lid != DMA2D_LAYER_SPU), "%s invalid layer ", __func__);
    crreg = getreg32(hal_dma2d_pfccr_layer_t[lid]);
    crreg &= ~DMA2D_xGPFCCR_SWAPRB_ENA_MASK;
    crreg |= DMA2D_xGPFCCR_SWAPRB_ENA(ena);
    putreg32(crreg, hal_dma2d_pfccr_layer_t[lid]);
}

/* vendor Color format definitions:
* 0x0 = RGB565
* 0x1 = RGB1555
* 0x2 = RGB888 packed
* 0x3 = RGB888 unpacked
* 0x4 = ARGB
* 0x5 = YUV422 packed
* 0x6 = YUV422 planar
* 0x7 = YUV420 planar
* 0x8 = Smart Panel command
* 0x9 =  Palette color 4-bit per pixel
* 0xA = Palette color 8-bit per pixel
* 0xB = RGBA
* All other values are reserved. */

static const uint8_t dma2d_fbl_cfmt[] = {
    0xff,            //#define FB_FMT_Y1             0   /* BPP=1, monochrome */
    0xff,            //#define FB_FMT_Y2             1   /* BPP=2, 2-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y4             2   /* BPP=4, 4-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y8             3   /* BPP=8, 8-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y16            4   /* BPP=16, 16-bit uncompressed greyscale */
    /* Standard RGB */
    0x09,            //#define FB_FMT_RGB4           5   /* BPP=4 */
    0x0a,            //#define FB_FMT_RGB8           6   /* BPP=8 RGB palette index */
    0xff,            //#define FB_FMT_RGB8_222       7   /* BPP=8  R=2, G=2, B=2 */
    0xff,            //#define FB_FMT_RGB8_332       8   /* BPP=8  R=3, G=3, B=2 */
    0xff,            //#define FB_FMT_RGB12_444      9   /* BPP=12 R=4, G=4, B=4 */
    0x01,            //#define FB_FMT_RGB16_555      10  /* BPP=16 R=5, G=5, B=5 (1 unused bit) */
    0x00,            //#define FB_FMT_RGB16_565      11  /* BPP=16 R=6, G=6, B=5 */
    0x03,            //#define FB_FMT_RGB24          12  /* BPP=24 */
    0x04,            //#define FB_FMT_RGB32          13  /* BPP=32 */
    //
    ///* Run length encoded RGB */
    //
    0xff,            //#define FB_FMT_RGBRLE4        14  /* BPP=4 */
    0xff,            //#define FB_FMT_RGBRLE8        15  /* BPP=8 */
    ///* Raw RGB */
    0xff,            //#define FB_FMT_RGBRAW         16  /* BPP=? */
    //
    ///* Raw RGB with arbitrary sample packing within a pixel. Packing and precision
    // * of R, G and B components is determined by bit masks for each.
    // */
    //
    0xff,            //#define FB_FMT_RGBBTFLD16     17  /* BPP=16 */
    0xff,            //#define FB_FMT_RGBBTFLD24     18  /* BPP=24 */
    0xff,            //#define FB_FMT_RGBBTFLD32     19  /* BPP=32 */
    0xff,            //#define FB_FMT_RGBA16         20  /* BPP=16 Raw RGB with alpha */
    0x0b,            //#define FB_FMT_RGBA32         21  /* BPP=32 Raw RGB with alpha */
    //
    ///* Raw RGB with a transparency field. Layout is as for standard RGB at 16 and
    // * 32 bits per pixel but the msb in each pixel indicates whether the pixel is
    // * transparent or not.
    // */
    //
    0xff,            //#define FB_FMT_RGBT16         22  /* BPP=16 */
    0xff,            //#define FB_FMT_RGBT32         23  /* BPP=32 */

    ///* Packed YUV Formats *******************************************************/

    0xff,            //#define FB_FMT_AYUV           24  /* BPP=32  Combined YUV and alpha */
    0xff,            //#define FB_FMT_CLJR           25  /* BPP=8   4 pixels packed into a uint32_t.
    //                                           *         YUV 4:1:1 with l< 8 bits
    //                                           *         per YUV sample */
    0xff,            //#define FB_FMT_CYUV           26  /* BPP=16  UYVY except that height is
    //                                           *         reversed */
    0xff,            //#define FB_FMT_IRAW           27  /* BPP=?   Intel uncompressed YUV */
    0xff,            //#define FB_FMT_IUYV           28  /* BPP=16  Interlaced UYVY (line order
    //                                           *         0,2,4,.., 1,3,5...) */
    0xff,            //#define FB_FMT_IY41           29  /* BPP=12  Interlaced Y41P (line order
    //                                           *         0,2,4,.., 1,3,5...) */
    0xff,            //#define FB_FMT_IYU2           30  /* BPP=24 */
    0xff,            //#define FB_FMT_HDYC           31  /* BPP=16  UYVY except uses the BT709
    0xff,            //                                           *         color space  */
    //#define FB_FMT_UYVP           32          /* BPP=24? YCbCr 4:2:2, 10-bits per
    //                                           *         component in U0Y0V0Y1 order */
    0xff,            //#define FB_FMT_UYVY           33  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_V210           34  /* BPP=32  10-bit 4:2:2 YCrCb */
    0xff,            //#define FB_FMT_V422           35  /* BPP=16  Upside down version of UYVY */
    0xff,            //#define FB_FMT_V655           36  /* BPP=16? 16-bit YUV 4:2:2 */
    0xff,            //#define FB_FMT_VYUY           37  /* BPP=?   ATI Packed YUV Data */
    0xff,            //#define FB_FMT_YUYV           38  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_YVYU           39  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_Y41P           40  /* BPP=12  YUV 4:1:1 */
    0xff,            //#define FB_FMT_Y411           41  /* BPP=12  YUV 4:1:1 */
    0xff,            //#define FB_FMT_Y211           42  /* BPP=8  */
    0xff,            //#define FB_FMT_Y41T           43  /* BPP=12  Y41P LSB for transparency */
    0xff,            //#define FB_FMT_Y42T           44  /* BPP=16  UYVY LSB for transparency */
    0x05,            //#define FB_FMT_YUVP           45  /* BPP=24? YCbCr 4:2:2 Y0U0Y1V0 order */

    //
    ///* Packed Planar YUV Formats ************************************************/
    //
    0xff,            //#define FB_FMT_YVU9           46  /* BPP=9   8-bit Y followed by 8-bit
    //                                           *         4x4 VU */
    0xff,            //#define FB_FMT_YUV9           47  /* BPP=9? */
    0xff,            //#define FB_FMT_IF09           48  /* BPP=9.5 YVU9 + 4x4 plane of delta
    //                                           *         relative to tframe. */
    0xff,            //#define FB_FMT_YV16           49  /* BPP=16  8-bit Y followed by 8-bit
    //                                           *         2x1 VU */
    0xff,            //#define FB_FMT_YV12           50  /* BPP=12  8-bit Y followed by 8-bit
    //                                           *         2x2 VU */
    0x07,            //#define FB_FMT_I420           51  /* BPP=12  8-bit Y followed by 8-bit
    //                                           *         2x2 UV */
    0xff,            //#define FB_FMT_NV12           52  /* BPP=12  8-bit Y followed by an
    //                                           *         interleaved 2x2 UV */
    0xff,            //#define FB_FMT_NV21           53  /* BPP=12  NV12 with UV reversed */
    0xff,            //#define FB_FMT_IMC1           54  /* BPP=12  YV12 except UV planes same
    //                                           *         stride as Y */
    0xff,            //#define FB_FMT_IMC2           55  /* BPP=12  IMC1 except UV lines
    //                                           *         interleaved at half stride
    //                                           *         boundaries */
    0xff,            //#define FB_FMT_IMC3           56  /* BPP=12  As IMC1 except that UV
    //                                           *         swapped */
    0xff,            //#define FB_FMT_IMC4           57  /* BPP=12  As IMC2  except that UV
    //                                           *         swapped */
    0xff,            //#define FB_FMT_CLPL           58  /* BPP=12  YV12 but including a level
    //                                           *         of indirection. */
    0xff,            //#define FB_FMT_Y41B           59  /* BPP=12?  4:1:1 planar. */
    0x06,            //#define FB_FMT_Y42B           60  /* BPP=16?  YUV 4:2:2 planar. */
    0xff,            //#define FB_FMT_CXY1           61  /* BPP=12 */
    0xff,            //#define FB_FMT_CXY2           62  /* BPP=16 */

    0x80,            //#define FB_FMT_BGR565         63  /* BPP=16 BGR565 */
    0x83,            //#define FB_FMT_BGR888         64  /* BPP=32 BGR888 */
    0x84,            //#define FB_FMT_BGRA32         65  /* BPP=32 RBGA888 */
    0x8b,            //#define FB_FMT_ABGR32         66  /* BPP=32 ABGR888 */
};

static const uint8_t dma2d_ol_cfmt[] = {
    0xff,            //#define FB_FMT_Y1             0  /* BPP=1, monochrome */
    0xff,            //#define FB_FMT_Y2             1  /* BPP=2, 2-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y4             2  /* BPP=4, 4-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y8             3  /* BPP=8, 8-bit uncompressed greyscale */
    0xff,            //#define FB_FMT_Y16            4  /* BPP=16, 16-bit uncompressed greyscale */
    /* Standard RGB */
    0xff,            //#define FB_FMT_RGB4           5  /* BPP=4 */
    0xff,            //#define FB_FMT_RGB8           6  /* BPP=8 RGB palette index */
    0xff,            //#define FB_FMT_RGB8_222       7  /* BPP=8  R=2, G=2, B=2 */
    0xff,            //#define FB_FMT_RGB8_332       8  /* BPP=8  R=3, G=3, B=2 */
    0xff,            //#define FB_FMT_RGB12_444      9  /* BPP=12 R=4, G=4, B=4 */
    0xff,            //#define FB_FMT_RGB16_555      10 /* BPP=16 R=5, G=5, B=5 (1 unused bit) */
    0x00,            //#define FB_FMT_RGB16_565      11 /* BPP=16 R=6, G=6, B=5 */
    0x01,            //#define FB_FMT_RGB24          12 /* BPP=24 */
    0x02,            //#define FB_FMT_RGB32          13 /* BPP=32  ARGB888*/
    //
    ///* Run length encoded RGB */
    //
    0xff,            //#define FB_FMT_RGBRLE4        14  /* BPP=4 */
    0xff,            //#define FB_FMT_RGBRLE8        15  /* BPP=8 */
    ///* Raw RGB */
    0xff,            //#define FB_FMT_RGBRAW         16  /* BPP=? */
    //
    ///* Raw RGB with arbitrary sample packing within a pixel. Packing and precision
    // * of R, G and B components is determined by bit masks for each.
    // */
    //
    0xff,            //#define FB_FMT_RGBBTFLD16     17  /* BPP=16 */
    0xff,            //#define FB_FMT_RGBBTFLD24     18  /* BPP=24 */
    0xff,            //#define FB_FMT_RGBBTFLD32     19  /* BPP=32 */
    0xff,            //#define FB_FMT_RGBA16         20  /* BPP=16 Raw RGB with alpha */
    0x03,            //#define FB_FMT_RGBA32         21  /* BPP=32 Raw RGB with alpha */
    //
    ///* Raw RGB with a transparency field. Layout is as for standard RGB at 16 and
    // * 32 bits per pixel but the msb in each pixel indicates whether the pixel is
    // * transparent or not.
    // */
    //
    0xff,            //#define FB_FMT_RGBT16         22  /* BPP=16 */
    0xff,            //#define FB_FMT_RGBT32         23  /* BPP=32 */

    ///* Packed YUV Formats *******************************************************/

    0xff,            //#define FB_FMT_AYUV           24  /* BPP=32  Combined YUV and alpha */
    0xff,            //#define FB_FMT_CLJR           25  /* BPP=8   4 pixels packed into a uint32_t.
    //                                           *         YUV 4:1:1 with l< 8 bits
    //                                           *         per YUV sample */
    0xff,            //#define FB_FMT_CYUV           26  /* BPP=16  UYVY except that height is
    //                                           *         reversed */
    0xff,            //#define FB_FMT_IRAW           27  /* BPP=?   Intel uncompressed YUV */
    0xff,            //#define FB_FMT_IUYV           28  /* BPP=16  Interlaced UYVY (line order
    //                                           *         0,2,4,.., 1,3,5...) */
    0xff,            //#define FB_FMT_IY41           29  /* BPP=12  Interlaced Y41P (line order
    //                                           *         0,2,4,.., 1,3,5...) */
    0xff,            //#define FB_FMT_IYU2           30  /* BPP=24 */
    0xff,            //#define FB_FMT_HDYC           31  /* BPP=16  UYVY except uses the BT709
    0xff,            //                                  /*         color space  */
    0xff,            //#define FB_FMT_UYVY           33  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_V210           34  /* BPP=32  10-bit 4:2:2 YCrCb */
    0xff,            //#define FB_FMT_V422           35  /* BPP=16  Upside down version of UYVY */
    0xff,            //#define FB_FMT_V655           36  /* BPP=16? 16-bit YUV 4:2:2 */
    0xff,            //#define FB_FMT_VYUY           37  /* BPP=?   ATI Packed YUV Data */
    0xff,            //#define FB_FMT_YUYV           38  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_YVYU           39  /* BPP=16  YUV 4:2:2 */
    0xff,            //#define FB_FMT_Y41P           40  /* BPP=12  YUV 4:1:1 */
    0xff,            //#define FB_FMT_Y411           41  /* BPP=12  YUV 4:1:1 */
    0xff,            //#define FB_FMT_Y211           42  /* BPP=8  */
    0xff,            //#define FB_FMT_Y41T           43  /* BPP=12  Y41P LSB for transparency */
    0xff,            //#define FB_FMT_Y42T           44  /* BPP=16  UYVY LSB for transparency */
    0xff,            //#define FB_FMT_YUVP           45             /* BPP=24? YCbCr 4:2:2 Y0U0Y1V0 order */

    //
    ///* Packed Planar YUV Formats ************************************************/
    //
    0xff,            //#define FB_FMT_YVU9           46  /* BPP=9   8-bit Y followed by 8-bit
    //                                           *         4x4 VU */
    0xff,            //#define FB_FMT_YUV9           47  /* BPP=9? */
    0xff,            //#define FB_FMT_IF09           48  /* BPP=9.5 YVU9 + 4x4 plane of delta
    //                                           *         relative to tframe. */
    0xff,            //#define FB_FMT_YV16           49  /* BPP=16  8-bit Y followed by 8-bit
    //                                           *         2x1 VU */
    0xff,            //#define FB_FMT_YV12           50  /* BPP=12  8-bit Y followed by 8-bit
    //                                           *         2x2 VU */
    0xff,            //#define FB_FMT_I420           51  /* BPP=12  8-bit Y followed by 8-bit
    //                                           *         2x2 UV */
    0xff,            //#define FB_FMT_NV12           52  /* BPP=12  8-bit Y followed by an
    //                                           *         interleaved 2x2 UV */
    0xff,            //#define FB_FMT_NV21           53  /* BPP=12  NV12 with UV reversed */
    0xff,            //#define FB_FMT_IMC1           54  /* BPP=12  YV12 except UV planes same
    //                                           *         stride as Y */
    0xff,            //#define FB_FMT_IMC2           55  /* BPP=12  IMC1 except UV lines
    //                                           *         interleaved at half stride
    //                                           *         boundaries */
    0xff,            //#define FB_FMT_IMC3           56  /* BPP=12  As IMC1 except that UV
    //                                           *         swapped */
    0xff,            //#define FB_FMT_IMC4           57  /* BPP=12  As IMC2  except that UV
    //                                           *         swapped */
    0xff,            //#define FB_FMT_CLPL           58  /* BPP=12  YV12 but including a level
    //                                           *         of indirection. */
    0xff,            //#define FB_FMT_Y41B           59  /* BPP=12?  4:1:1 planar. */
    0xff,            //#define FB_FMT_Y42B           60  /* BPP=16?  YUV 4:2:2 planar. */
    0xff,            //#define FB_FMT_CXY1           61  /* BPP=12 */
    0xff,            //#define FB_FMT_CXY2           62  /* BPP=16 */
    0x80,            //#define FB_FMT_BGR565         63  /* BPP=16 BGR565 */
    0x81,            //#define FB_FMT_BGR888         64  /* BPP=32 BGR888 */
    0x82,            //#define FB_FMT_BGRA32         65  /* BPP=32 RBGA888 */
    0x83,            //#define FB_FMT_ABGR32         66  /* BPP=32 ABGR888 */
};

void hal_dma2d_lpfc_reset(void)
{
    putreg32(0, hal_dma2d_pfccr_layer_t[DMA2D_LAYER_SPU]);
    putreg32(0, hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LOUT]);
    putreg32(0, hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LFORE]);
    putreg32(0, hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LBACK]);
    putreg32(0,
         hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LFORE] + (DMA2D_FGPFCCR1 -
                                   DMA2D_FGPFCCR));
}

#ifndef CONFIG_DMA2D_FB_NO_COLORDITHER

static uint8_t redw(uint8_t fmt)
{
    uint8_t w = 0;
    switch (fmt) {
    case 9:
        w = 4;
        break;
    case 10:
        w = 5;
        break;
    case 11:
        w = 5;
        break;
    case 63:
        w = 5;
        break;
    default:
        w = 8;

    }
    return w;
}

static uint8_t greenw(uint8_t fmt)
{
    uint8_t w = 0;
    switch (fmt) {
    case 9:
        w = 4;
        break;
    case 10:
        w = 6;
        break;
    case 11:
        w = 6;
        break;
    case 63:
        w = 6;
        break;
    default:
        w = 8;

    }
    return w;
}

static uint8_t bluew(uint8_t fmt)
{
    uint8_t w = 0;
    switch (fmt) {
    case 9:
        w = 4;
        break;
    case 10:
        w = 5;
        break;
    case 11:
        w = 5;
        break;
    case 63:
        w = 5;
        break;
    default:
        w = 8;

    }
    return w;
}

#endif //CONFIG_DMA2D_FB_NO_COLORDITHER

static bool is_yuv(uint8_t fmt)
{
    return (dma2d_fbl_cfmt[fmt] == 05);
}

/**
 * hal_dma2d_lpfc - Configure foreground and background layer pixes function
 * @lid        : Layer id (output, foreground, background)
 * @blendmode  : Layer blendmode (dma2d register values),
 *               Panel Path Alpha Selection
 *               This field selects the Panel Path video/graphic alpha for overlaid area.
 *               0x0 = Video Path alpha, alpha is generated from RGB1555 or RGBA888/A
 *               of the video data or video color key
 *               0x1 = Graphic Path alpha, alpha is generated from RGB1555 or
 *               RGBA888/A of the graphic data or graphic color key
 *               0x2 = Configurable alpha, alpha is defined in the <Panel Path
 *               Configure Alpha> field
 *               0x3 = Panel Path alpha is not used
 *
 *  @alpha     : Transparency
 *  @fmt       : FB color format
 */

void hal_dma2d_lpfc(int lid, uint32_t blendmode, uint8_t alpha, uint8_t fmt)
{
    uint32_t pfccrreg0;
    uint32_t pfccrreg1;
    bool swaprb = false;

    DMA2D_INFO("%s lid:%d bmode:%d alpha:%d fmt:%d", __func__, lid,
           blendmode, alpha, fmt);
    ASSERT((lid != DMA2D_LAYER_SPU), " %s invalid layer ", __func__);
    pfccrreg0 = getreg32(hal_dma2d_pfccr_layer_t[lid]);

    if (lid == DMA2D_LAYER_LOUT) {
        ASSERT((0xFF != dma2d_ol_cfmt[fmt]),
               "%s lid:%d invalid color fmt:%d", __func__, lid, fmt);
        pfccrreg0 &= ~DMA2D_OPFCCR_CM_MASK;
        pfccrreg0 |= DMA2D_OPFCCR_CM(0x0F & (dma2d_ol_cfmt[fmt]));
        if ((0x80 & dma2d_ol_cfmt[fmt])) {    //swap rb
            swaprb = true;
        }
        hal_dma2d_lpfc_swaprb(lid, swaprb);

        putreg32(pfccrreg0, hal_dma2d_pfccr_layer_t[lid]);
        return;
    } else {
        ASSERT((0xFF != dma2d_fbl_cfmt[fmt]),
               "%s lid:%d invalid color fmt:%d", __func__, lid, fmt);
        pfccrreg0 &= ~DMA2D_xGPFCCR_CM_MASK;
        pfccrreg0 |= DMA2D_xGPFCCR_CM(0x0F & (dma2d_fbl_cfmt[fmt]));
        if ((0x80 & dma2d_fbl_cfmt[fmt])) {    //swap rb
            // swaprb = true;
        }
        hal_dma2d_lpfc_swaprb(lid, swaprb);

#ifndef CONFIG_DMA2D_FB_NO_COLORDITHER
        uint8_t red, green, blue;
        red = redw(fmt);
        green = greenw(fmt);
        blue = bluew(fmt);
        if (red < 8 || green < 8 || blue < 8) {
            hal_dma2d_dither_s4x8(true);
            hal_dma2d_dither(true, 1, red, green, blue);
        } else {
            hal_dma2d_dither(false, 1, red, green, blue);
        }

#endif

    }
    //enable yuv2rgb
    if (is_yuv(fmt)) {
        pfccrreg0 |= DMA2D_CFG_YUV2RGB_ENA(0x1);
    } else {
        pfccrreg0 &= ~DMA2D_CFG_YUV2RGB_ENA_MASK;

    }
    putreg32(pfccrreg0, hal_dma2d_pfccr_layer_t[lid]);

    if (lid != DMA2D_LAYER_LFORE) {
        return;
    }

    /* Set alpha blend mode */
    pfccrreg1 =
        getreg32(hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LFORE] +
             (DMA2D_FGPFCCR1 - DMA2D_FGPFCCR));
    pfccrreg1 &= ~DMA2D_xGPFCCR1_AM_MASK;
    pfccrreg1 |= DMA2D_xGPFCCR1_AM(blendmode);

    if (blendmode == DMA2D_PFCCR_AM_CONST    /* ||
                           blendmode == DMA2D_PFCCR_AM_VPIXEL */ ) {
        /* Set alpha value */
        pfccrreg1 &= ~DMA2D_xGPFCCR1_ALPHA_MASK;

        pfccrreg1 |= DMA2D_xGPFCCR1_ALPHA(alpha);
    }
    putreg32(pfccrreg1,
         hal_dma2d_pfccr_layer_t[DMA2D_LAYER_LFORE] + (DMA2D_FGPFCCR1 -
                                   DMA2D_FGPFCCR));
}

/**
 * hal_dma2d_write_clut - write clut data into dma2d memory
 * @citem  : clut item
 * @n      : clut item address index
 */
void hal_dma2d_write_clut(uint32_t citem, int n)
{
    uint32_t crreg;
    putreg32(citem, DMA2D_FBLCLUTWRDR);

    crreg = getreg32(DMA2D_FBLCLUTCR);
    crreg &= ~DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_MASK;
    crreg &= ~DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_MASK;
    crreg &= ~DMA2D_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_MASK;
    crreg |= DMA2D_FBLCLUTTABLE_SETSRAM_ADDR(n);
    crreg |= DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_LCDID(3);
    crreg |= DMA2D_FBLCLUTTABLE_SETSRAM_INIT_WR_RD(2);
    putreg32(crreg, DMA2D_FBLCLUTCR);
}

/**
 * hal_dma2d_lstartaddr - set dma2d layer dma memory start address
 * @lid   : Layer id
 * @addr  : memory address
 */
int hal_dma2d_lstartaddr(int lid, uint32_t addr)
{
    DMA2D_INFO("%s lid:%d addr:%x", __func__, lid, addr);
    putreg32(addr, hal_dma2d_mar_layer_t[lid]);
    return 0;
}

/**
 * hal_dma2d_lpitch - set dma2d vsync start/end pixes count
 * @lid     : Layer id
 * @pitch   : pitch in bytes
 */
void hal_dma2d_lpitch(int lid, uint16_t pitch)
{
    uint32_t pfccrreg;
    DMA2D_INFO("%s lid:%d pitch:%d", __func__, lid, pitch);
    pfccrreg = getreg32(hal_dma2d_lpitch_layer_t[lid]);
    if (lid == DMA2D_LAYER_LOUT) {
        pfccrreg &= ~DMA2D_OLPITCH_MASK;
        pfccrreg |= DMA2D_OLPITCH(pitch);
    } else {
        pfccrreg &= ~DMA2D_LPITCH_MASK;
        pfccrreg |= DMA2D_LPITCH(pitch);
    }
    putreg32(pfccrreg, hal_dma2d_lpitch_layer_t[lid]);
}

/**
 * hal_dma2dl_enable  - enable/disable dma2d Layer
 * @lid     : Layer index
 * @enable  : enable/disable
 */
void hal_dma2dl_enable(int lid, bool enable)
{
    uint32_t pfccrreg;
    DMA2D_INFO("%s lid:%d enable:%d", __func__, lid, enable);
    pfccrreg = getreg32(hal_dma2d_pfccr_layer_t[lid]);
    pfccrreg &= ~DMA2D_xGPFCCR_LENA_MASK;
    pfccrreg |= DMA2D_xGPFCCR_LENA(enable);
    putreg32(pfccrreg, hal_dma2d_pfccr_layer_t[lid]);
}

/**
 * hal_dma2d_smpn_mode_set - set dma2d smpn mode
 * @mode : smpn mode (0x0 = 3 cycles per pixel;
 *                    0x1 = RGB666, 3 cycles per pixel
 *                    0x2 = RGB565, 2 cycles per pixel;
 *                    0x3 = 1 cycle per pixel;
 *                    0x4 = RGB666, 1 cycle per pixel;
 *                    0x5 = RGB565 1 cycle per pixel )
 *
 */
void hal_dma2d_smpn_mode_set(int mode)
{
    uint32_t pfccrreg;
    DMA2D_INFO("%s  mode:%d", __func__, mode);
    pfccrreg = getreg32(DMA2D_SMPNCR);
    pfccrreg &= ~DMA2D_SMPNCR_MODE_MASK;
    pfccrreg |= DMA2D_SMPNCR_MODE_SET(mode);
    putreg32(pfccrreg, DMA2D_SMPNCR);
}

/**
 * hal_dma2d_lsize - set  Layer image size in pixes
 * @lid  : Layer id
 * @w    : Layer width
 * @h    : Layer height
 */
void hal_dma2d_lsize(int lid, uint16_t w, uint16_t h)
{
    DMA2D_INFO("%s lid:%d w:%d h:%d", __func__, lid, w, h);
    uint32_t pfccrreg = getreg32(hal_dma2d_lszr_layer_t[lid]);
    pfccrreg &= ~(DMA2D_AWCR_AAW_MASK);
    pfccrreg &= ~(DMA2D_AWCR_AAH_MASK);
    if (lid == DMA2D_LAYER_SPU) {
        pfccrreg |= DMA2D_AWCR_AAW(w + 1) | DMA2D_AWCR_AAH(h + 1);
    } else {
        pfccrreg |= DMA2D_AWCR_AAW(w) | DMA2D_AWCR_AAH(h);
    }
    putreg32(pfccrreg, hal_dma2d_lszr_layer_t[lid]);

}

/**
 * hal_dma2d_sponscreen_set - set F/B Layer screen start position in pix
 * @lid   : Layer id
 * @vln   : y in line
 * @hpixl : x in pix
 */
void hal_dma2d_sponscreen_set(int lid, uint16_t vln, uint16_t hpixl)
{
    DMA2D_INFO("%s lid:%d vln:%d hpixl:%d", __func__, lid, vln, hpixl);
    uint32_t pfccrreg = getreg32(hal_dma2d_sponscreen_layer_t[lid]);
    pfccrreg &= ~(DMA2D_CFG_DMA_OVSA_HPXL_MASK);
    pfccrreg &= ~(DMA2D_CFG_DMA_OVSA_VLN_MASK);
    pfccrreg |=
        DMA2D_CFG_DMA_OVSA_HPXL(vln) | DMA2D_CFG_DMA_OVSA_VLN(hpixl);
    putreg32(pfccrreg, hal_dma2d_sponscreen_layer_t[lid]);

}

/**
 * Name: hal_dma2d_lzoom_set - set  F/B Layer zoom size in pix
 * @lid    : Layer id
 * @w      : window active area width
 * @h      : window active area height
 */
void hal_dma2d_lzoom_set(int lid, uint16_t w, uint16_t h)
{
    DMA2D_INFO("%s lid:%d w:%d h:%d", __func__, lid, w, h);
    uint32_t pfccrreg = getreg32(hal_dma2d_zoomr_layer_t[lid]);
    pfccrreg &= ~(DMA2D_CFG_DMAZM_H_PIXEL_MASK);
    pfccrreg &= ~(DMA2D_CFG_DMAZM_V_LINE_MASK);
    pfccrreg |= DMA2D_CFG_DMAZM_H_PIXEL(w) | DMA2D_CFG_DMAZM_V_LINE(h);
    putreg32(pfccrreg, hal_dma2d_zoomr_layer_t[lid]);

}

void hal_dma2d_swap_path(uint8_t lid)
{
    DMA2D_INFO("%s lid:%d ", __func__, lid);
    uint32_t pfccrreg = getreg32(DMA2D_SWAPCR);

    pfccrreg &= ~(DMA2D_SWAPPPPCR_ASEL_MASK);
    pfccrreg |= DMA2D_SWAPPPPCR_ASEL(lid);
    putreg32(pfccrreg, DMA2D_SWAPCR);
}

void hal_dma2d_swap_path_enable(bool enable)
{
    uint32_t pfccrreg;
    DMA2D_INFO("%s enable:%d", __func__, enable);
    pfccrreg = getreg32(DMA2D_SWAPCR);
    pfccrreg &= ~DMA2D_SWAPPPPCR_ASEL_ENA_MASK;
    pfccrreg |= DMA2D_SWAPPPPCR_ASEL_ENA(enable);
    putreg32(pfccrreg, DMA2D_SWAPCR);
}

/**
 * hal_dma2d_lblend - select  F/B Layer blend first layer
 * @first  : Layer id
 */
void hal_dma2d_lblend(uint8_t first)
{
    DMA2D_INFO("%s first:%d", __func__, first);
    uint32_t pfccrreg = getreg32(DMA2D_BLDCR);
    pfccrreg &= ~(DMA2D_BLDCR_FIRSTLSEL_MASK);
    pfccrreg |= DMA2D_BLDCR_FIRSTLSEL(first);
    putreg32(pfccrreg, hal_dma2d_lszr_layer_t[first]);
}

/**
 * hal_dma2d_lblend_select - select  F/B Layer blend first and second layer
 * @first   : Layer id
 * @second  : Layer id
 */
void hal_dma2d_lblend_select(uint8_t first, uint8_t second)
{
    DMA2D_INFO("%s first:%d", __func__, first);
    uint32_t pfccrreg = getreg32(DMA2D_BLDCR);
    pfccrreg &= ~(DMA2D_BLDCR_FIRSTLSEL_MASK);
    pfccrreg &= ~(DMA2D_BLDCR_SECONDLSEL_MASK);
    pfccrreg |=
        DMA2D_BLDCR_FIRSTLSEL(first) | DMA2D_BLDCR_SECONDLSEL(second);
    putreg32(pfccrreg, hal_dma2d_lszr_layer_t[first]);
}

/**
 * hal_dma2d_vsync_porch_set - set vsync porch with act,image width and mem types
 * @action    : 0/fill, 1/blit, 2/blend
 * @lwidth    : layer width
 * @flayers   : fast layer number
 * @slayers   : slow layer number
 */

#define LCDC2D_USE_PLL_BB

#ifndef LCDC2D_USE_PLL_BB
#define LCDC2D_USE_OSCX2
#endif

static uint8_t lcdc_framebuffer_slayers(void)
{
    return 0;
}

static  uint8_t lcdc_framebuffer_flayers(void)
{
    return 0;
}

void hal_dma2d_vsync_porch_set(uint8_t action, uint32_t lwidth, uint8_t flayers2d, uint8_t slayers2d)
{
    uint32_t width = 0;
    uint8_t  flayers = 0;
    uint8_t  slayers = 0;
    uint32_t fr_fill = 0x1;
    uint32_t fr_blit = 0x1;
    uint32_t fr_blend = 0x1;
    uint32_t sr_fill = 0x1;
    uint32_t sr_blit = 0x1;
    uint32_t sr_blend = 0x1;
#ifdef LCDC2D_USE_OSCX2
    width += 0x10;
#elif defined( LCDC2D_USE_OSCX4 )
    width += 0x20;
#elif defined( LCDC2D_USE_PLL_BB )
    width += 0x20;
#else
   #error "NO PLL SELECTED FOR LCDC and DMA2D MODULES"
#endif
    slayers = slayers2d + lcdc_framebuffer_slayers();
    flayers = flayers2d + lcdc_framebuffer_flayers();

    switch(action){
        case DMA2D_ACTION_TYPE_FILL:
             width += ( fr_fill * flayers + sr_fill * slayers )*(lwidth/64 + 1);
             break;
        case DMA2D_ACTION_TYPE_BLIT:
             width += ( fr_blit * flayers + sr_blit * slayers )*(lwidth/64 + 1);
             break;
        case DMA2D_ACTION_TYPE_BLEND:
             width += ( fr_blend * flayers + sr_blend * slayers )*(lwidth/64 + 1);
             break;
        default:
             width += ( fr_blend * flayers + sr_blend * slayers )*(lwidth/64 + 1);
             break;
    }

#ifdef FPGA
    width = 0x400;
#endif
    width += 0x100;

    TRACE(3, "%s lwidth:%d delayw:0x%x", __func__, lwidth ,width);
    putreg32((0x501000 | (width & 0xfff)), DMA2D_STBY_VSYNCCR);

}

/**
 *  hal_dma2d_axibus_arbiter_fastmode_enable - AXI Bus Arbiter Fast Mode Enable
 */
void hal_dma2d_axibus_arbiter_fastmode_enable(void)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t reg = getreg32(dma2d_tvd_ctrl);
    reg |=(1 << 27);
    putreg32(reg, dma2d_tvd_ctrl);
}

/**
 * hal_dma2d_wdma_1fpt_set - set write back dma in one frame per trigger mode
 */
void hal_dma2d_wdma_1fpt_set(void)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t pfccrreg = getreg32(DMA2D_WBFPTCR);

    if (pfccrreg & DMA2D_OL_FPT_MASK) {
        pfccrreg &= ~(DMA2D_OL_FPT_MASK);
    } else {
        pfccrreg |= DMA2D_OL_FPT_SET(1);
    }

    putreg32(pfccrreg, DMA2D_WBFPTCR);
}

/**
 * hal_dma2d_bgl_vhtotal_set - set  F/B Layer PN_V/H_TOTAL in pix
 * @h : height total
 * @v : width total
 */
void hal_dma2d_bgl_vhtotal_set(int h, int v)
{
    DMA2D_INFO("%s w:%d h:%d", __func__, h, v);
    uint32_t pfccrreg = getreg32(DMA2D_PVH_TOTAL_CR);
    pfccrreg &= ~(DMA2D_CFG_PN_H_TOTAL_MASK);
    pfccrreg &= ~(DMA2D_CFG_PN_V_TOTAL_MASK);
    pfccrreg |= DMA2D_CFG_PN_H_TOTAL(h) | DMA2D_CFG_PN_V_TOTAL(v);
    putreg32(pfccrreg, DMA2D_PVH_TOTAL_CR);
}

/**
 * hal_dma2d_bgl_hporch_set - set  F/B Layer HPORCH in pixes
 * @f : fore porch
 * @b : back porch
 */
void hal_dma2d_bgl_hporch_set(int f, int b)
{
    DMA2D_INFO("%s f:%d b:%d", __func__, f, b);
    uint32_t pfccrreg = getreg32(DMA2D_PHPORCH_CR);
    pfccrreg &= ~(DMA2D_CFG_PN_H_FPORCH_MASK);
    pfccrreg |= DMA2D_CFG_PN_H_FPORCH(f);
    putreg32(pfccrreg, DMA2D_PHPORCH_CR);

    pfccrreg = getreg32(DMA2D_PHPORCH_CR);
    pfccrreg &= ~(DMA2D_CFG_PN_H_BPORCH_MASK);
    pfccrreg |= DMA2D_CFG_PN_H_BPORCH(b);
    putreg32(pfccrreg, DMA2D_PHPORCH_CR);
}

/**
 * hal_dma2d_bgl_vporch_set - set  F/B Layer VPORCH in pixes
 * @f  : fore porch
 * @b  : back porch
 */
void hal_dma2d_bgl_vporch_set(int f, int b)
{
    DMA2D_INFO("%s f:%d b:%d", __func__, f, b);
    uint32_t pfccrreg = getreg32(DMA2D_PVPORCH_CR);
    pfccrreg &= ~(DMA2D_CFG_PN_V_FPORCH_MASK);
    pfccrreg |= DMA2D_CFG_PN_V_FPORCH(f);
    putreg32(pfccrreg, DMA2D_PVPORCH_CR);

    pfccrreg = getreg32(DMA2D_PVPORCH_CR);
    pfccrreg &= ~(DMA2D_CFG_PN_V_BPORCH_MASK);
    pfccrreg |= DMA2D_CFG_PN_V_BPORCH(b);
    putreg32(pfccrreg, DMA2D_PVPORCH_CR);
}

/**
 * hal_dma2d_sl_dmaburst_maxlength_set - set dma2d source path dma read burst length
 */
void hal_dma2d_sl_dmaburst_maxlength_set(void)
{
    uint32_t v;
    v = getreg32(DMA2D_SWAPCR);
    v &=~( 0x3 << 10);
    v &=~( 0x3 << 8);
    v |= (0x3 << 10) | (0x3 <<8);
    putreg32(v, DMA2D_SWAPCR);
    putreg32(0xa020020, DMA2D_PORCH_VSYNCCR);
    putreg32(0x1f001e, DMA2D_VSYNC_PIXCR);
}

/**
 * hal_dma2d_sl_dmaburst_length_set - set dma2d source path dma read burst length
 */
void hal_dma2d_sl_dmaburst_length_set(uint8_t len)
{
    uint32_t v;
    v = getreg32(DMA2D_SWAPCR);
    v &=~( 0x3 << 10);
    v &=~( 0x3 << 8);
    v |= (len&0xf) << 8;
    putreg32(v, DMA2D_SWAPCR);
}

/**
 * hal_dma2d_ol_dmaburst_length_set - set dma2d wb path dma burst length
 * @len   :  burst length
 */
void hal_dma2d_ol_dmaburst_length_set(uint8_t len)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t pfccrreg = getreg32(DMA2D_WBBURSTCR);
    pfccrreg &= ~(DMA2D_OL_BURST_LEN_MASK);
    pfccrreg |= DMA2D_OL_BURST_LEN(len);
    putreg32(pfccrreg, DMA2D_WBBURSTCR);
}

/**
 * hal_dma2d_dither_s4x8 - enable/disable dma2d DITHER_4X8_PN
 * @enable     - enable/disable
 */
void hal_dma2d_dither_s4x8(bool enable)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t reg = getreg32(DMA2D_DITHERCR);

    reg &= ~(DMA2D_DITHER_4X8_PN_MASK);
    reg |= DMA2D_DITHER_4X8_PN(enable);

    putreg32(reg, DMA2D_DITHERCR);
}

/**
 * hal_dma2d_dither - dma2d  dither configuration
 * @enable  : enable/disable
 * @lelvel  : 0:low 1:high
 * @red     : color width(4,5,6)
 * @green   : color width(4,6,6)
 * @blue    : color width(4,5,6)
 */
void hal_dma2d_dither(bool enable, uint8_t level, uint8_t red,
              uint8_t green, uint8_t blue)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t reg = getreg32(DMA2D_DITHERCR);
    uint8_t *dither_mode_table =
        (level > 0) ? dither_mode_high : dither_mode;
    uint8_t mode_index = 0;
    if (red == 5 && green == 5 && blue == 5) {
        mode_index = 3;
    } else {
        mode_index = red - 4;
    }

    reg &= ~(DMA2D_DITHER_EN_PN_MASK);
    reg &= ~(DMA2D_DITHER_MODE_PN_MASK);
    if (enable) {
        reg |=
            DMA2D_DITHER_EN_PN(enable) |
            DMA2D_DITHER_MODE_PN(dither_mode_table[mode_index]);
    }

    putreg32(reg, DMA2D_DITHERCR);
}

/**
 * hal_dma2d_dither_read - read  dma2d  dither table memory
 * @index : table memory index
 *
 * Returns:  mem value of dither table
 */
uint32_t hal_dma2d_dither_read(uint8_t index)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t reg = getreg32(DMA2D_DITHERCR);

    reg &= ~(DMA2D_DITHER_TABLE_INDEX_SEL_MASK);
    reg |= DMA2D_DITHER_TABLE_INDEX_SEL(index);

    putreg32(reg, DMA2D_DITHERCR);

    reg = getreg32(DMA2D_DITHERDR);
    return reg;
}

/**
 * hal_dma2d_dither_write - write  dma2d  dither table memory
 * @index  : table memory index
 * @value  : write value to dither table
 */
void hal_dma2d_dither_write(uint8_t index, uint32_t value)
{
    DMA2D_INFO("%s ", __func__);
    uint32_t reg = getreg32(DMA2D_DITHERCR);

    reg &= ~(DMA2D_DITHER_TABLE_INDEX_SEL_MASK);
    reg |= DMA2D_DITHER_TABLE_INDEX_SEL(index);

    putreg32(reg, DMA2D_DITHERCR);

    reg = getreg32(DMA2D_DITHERDR);
    reg &= ~(DMA2D_DITHER_TBL_DATA_MASK);
    reg |= DMA2D_DITHER_TBL_DATA(value);
    putreg32(reg, DMA2D_DITHERDR);
}

/**
 * hal_dma2d_pnlayer_clock_enable  - enable clock for dma2d PNLayer
 */
void hal_dma2d_pnlayer_clock_enable(void)
{
    uint32_t reg;
    DMA2D_INFO("%s ", __func__);
    reg = getreg32(DMA2D_ECLKCR);
    reg &= ~DMA2D_ECLKCR_PNENA_MASK;
    reg |= DMA2D_ECLKCR_PNENA;
    putreg32(reg, DMA2D_ECLKCR);
}

/**
 * hal_dma2d_tvlayer_clock_enable  - enable clock for dma2d TVLayer
 */
void hal_dma2d_tvlayer_clock_enable(void)
{
    uint32_t reg;
    DMA2D_INFO("%s ", __func__);
    reg = getreg32(DMA2D_ECLKCR);
    reg &= ~DMA2D_ECLKCR_TVENA_MASK;
    reg |= DMA2D_ECLKCR_TVENA;
    putreg32(reg, DMA2D_ECLKCR);
}

/**
 * hal_dma2d_pnlayer_clock_disable  - disable clock for dma2d PNLayer
 */
void hal_dma2d_pnlayer_clock_disable(void)
{
    uint32_t reg;
    DMA2D_INFO("%s ", __func__);
    reg = getreg32(DMA2D_DCLKCR);
    reg &= ~DMA2D_DCLKCR_PNDIS_MASK;
    reg |= DMA2D_DCLKCR_PNDIS;
    putreg32(reg, DMA2D_DCLKCR);
}

/**
 * hal_dma2d_tvlayer_clock_disable  - disable clock for dma2d TVLayer
 */
void hal_dma2d_tvlayer_clock_disable(void)
{
    uint32_t reg;
    DMA2D_INFO("%s ", __func__);
    reg = getreg32(DMA2D_DCLKCR);
    reg &= ~DMA2D_DCLKCR_TVDIS_MASK;
    reg |= DMA2D_DCLKCR_TVDIS;
    putreg32(reg, DMA2D_DCLKCR);
}

#endif
