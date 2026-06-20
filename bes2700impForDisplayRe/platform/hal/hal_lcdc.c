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
 * DOC: The Display Controller operates in Slave mode and DMA mode.
 *
 * Slave mode is for the processor to read/write Smart Panel, and DMA mode is for
 * the Display Controller to move pixels to either the Smart Panel or the Dumb Panels.
 * The <var Product Number> support one panel path and one TV path, for each path,
 * besides background color, there are total of three objects that can be overlaid
 * onto the display screen, video,graphicc, and hardware cursor. When one panel
 * path or TV path is disabled,the three objects can be overlaid on the remaining
 * panel or TV. When all dumb panels are working in DMA mode, smart panel can be
 * accessed through an AHB slave path concurrently. So, a total of three panel
 * interfacesare supported, with two operating in DMA mode and one operating in
 * slave mode. In the following context, the panel path is called panel path, and
 * those signals are mostly called pn_*. Smart panel signals are mostly called
 * *isa* or *smpn*.
 *
 * The following operations will be done during DMA transfer:
 * Color Space Conversion (CSC) on two video paths, supporting SDTV and HDTV, studio
 * and computer, and total of four CSC matrixes.
 * Video enhancement including contrast/saturation/brightness/hue adjust on the
 * two video paths.
 * palette color lookup on the two graphic paths, supporting both 4-bit/pixel and
 *  8-bit/pixel.
 * alpha blending in overlaid area for all paths, with configured alpha, alpha
 * combined with pixel and color key alpha.
 * partial display DMA when part of the picture is covered by another picture,
 * reducing bandwidth (refer to register bit descriptions for more information).
 * multiple DMA requests concatenated together to increase DDR page hit rate.
 * line buffer mode DMA to increase DDR page hit rate and reduce internal SRAM
 * occupation.
 * RGB gamma correction after alpha blending, up to two interfaces.
 * horizontal and vertical scaling and filtering on all of the four video and
 * graphic paths.
 * hardware cursor overlay, each cursor size up to 8192 pixels for 1 bit per pixel.
 * color modification for either Panel path or TV path to enhance the quality
 * of picture colors.
 * selec vertical mirror on all of the four video and graphic objects.
 * dither function enable for either Panel path or TV path.
 * The base addresses of the Display Controller registers is: 0xD420_B000.

 * LCDC hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,YUV422packed,
 *   YUV422 planar, YUV420 planar, Palette color 4-bit per pixel, Palette color
 *   8-bit per pixel, RGBA )
 * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,YUV422
 *  packed,Palette color 4-bit per pixel,Palette color 8-bit per pixel,RGB8A )
 *  (OL color fmt:RGB565 RGB888 ARGB888 RGBA888)
 *
 *  |FG Layer(panel video dma layer) |-->|
 *                                       | Alpha_FG (pix/c)
 *                                       V
 *                                      |BLENDING|-->|Smart Panel|-->->|dsi|
 *                                       ^     Alpha_O (pix/c)       |
 *                                       | Alpha_BG (pix/c)          |
 *  |BG Layer(panel graphic dma layer)|->|             |             |
 *                                                     |             |
 *  |TV Path(not used currently by lcdc driver)|-------V             |
 *                                                                   |
 *                                                                   |
 *  |write back layer)|<----------------------------------------------
 */

#include "string.h"
#include "cmsis.h"
#include "reg_lcdc_v1.h"
#include "hal_cmu.h"
#include "hal_timer.h"
#include "hal_lcdc.h"
#include "hal_trace.h"
#include "reg_lcdc.h"
#include "hal_psc.h"
#include "hal_dsi.h"
#include "reg_dsi.h"
#if !defined(__NuttX__)
#include "graphic_conf.h"
#endif

#ifdef LCDC_BASE

static struct LCDC_REG_T * const lcdc = (struct LCDC_REG_T *)LCDC_BASE;

#ifdef NULL
#undef NULL
#define NULL  (0)
#endif

//#define GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define LCDC_REGINFO(...)  TR_INFO( TR_MOD(TEST), __VA_ARGS__)
#else
#define LCDC_REGINFO(...)
#define LCDC_INFO(...)
#endif

#define LCDC_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define LCDC_NLAYERS            2
#define LAYER0_USE_GRA_FIFO     1
/**
 * Private Data
 */

/* LCDC_Layer Start Mem Addr register */

static const uintptr_t hal_lcdc_mar_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_BGMAR,
    LCDC_FGMAR,
#else
    LCDC_FGMAR,
    LCDC_BGMAR,
#endif
    LCDC_OMAR,
    NULL,
};

/*F/B Layer dma zoom Register Address*/

static const uintptr_t hal_lcdc_zoomr_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_GRAZOOMCR,
    LCDC_DMAZOOMCR,
#else
    LCDC_DMAZOOMCR,
    LCDC_GRAZOOMCR,
#endif
    NULL,
    NULL,
};

/*F/B Layer start point on screen Register Address*/

static const uintptr_t hal_lcdc_sponscreen_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_CFG_PGRA_SPONSCREENCR,
    LCDC_CFG_PDMA_SPONSCREENCR,
#else
    LCDC_CFG_PDMA_SPONSCREENCR,
    LCDC_CFG_PGRA_SPONSCREENCR,
#endif
    NULL,
    NULL,
};

/* LCDC Layer pfc control register */

static const uintptr_t hal_lcdc_pfccr_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_BGPFCCR,
    LCDC_FGPFCCR,
#else
    LCDC_FGPFCCR,
    LCDC_BGPFCCR,
#endif
    LCDC_OPFCCR,
    LCDC_SMPNCR
};

/* LCDC Layer pitch register */
static const uintptr_t hal_dma2d_lpitch_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_PGRAPCHCR,
    LCDC_PDMAPCHCR,
#else
    LCDC_PDMAPCHCR,
    LCDC_PGRAPCHCR,
#endif
    LCDC_WBPCHCR,
    NULL
};

/* LCDC Layer size register */

static const uintptr_t hal_lcdc_lszr_layer_t[LCDC_NLAYERS + 2] = {
#if LAYER0_USE_GRA_FIFO
    LCDC_PGRAAWCR,
    LCDC_PDMAAWCR,
#else
    LCDC_PDMAAWCR,
    LCDC_PGRAAWCR,
#endif
    LCDC_WBAWCR,
    LCDC_SPUAWCR,
};

/* LCDC color register */

static const uintptr_t hal_lcdc_color_layer_t[LCDC_NLAYERS + 2] = {
    NULL,
    NULL,
    NULL,
    LCDC_SPUCOLR
};

/**
 * Private Functions
 */

#define getreg32(a)          (*(volatile uint32_t *)(a))

#define putreg32(v,a)        do { \
                                   LCDC_REGINFO("putreg32(0x%x,0x%x);",(unsigned int)(v), (unsigned int)(a)) ; \
                                   *(volatile uint32_t *)(a) = (v); \
                              } while(0)

#define reg32_bitset(v,a)    do { \
                                   LCDC_REGINFO("reg32_bitset(0x%x,0x%x);", v, a); \
                                   *(volatile uint32_t *)(a) |= v; \
                              } while(0)

/**
 * hal_lcdc_irqn - get lcdc irq  number
 */
int hal_lcdc_irqn(void)
{
    return DISPLAY_IRQn;
}

/**
 * hal_lcdc_write_clut - write clut data into lcdc memory
 * @citem  : clut item
 * @n      : clut item address index
 */
void hal_lcdc_write_clut(uint32_t citem, int n)
{
    uint32_t crreg;
    putreg32(citem, LCDC_FBLCLUTWRDR);

    crreg = getreg32(LCDC_FBLCLUTCR);
    crreg &= ~LCDC_FBLCLUTTABLE_SETSRAM_ADDR_MASK;
    crreg &= ~LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_MASK;
    crreg &= ~LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_MASK;
    crreg |= LCDC_FBLCLUTTABLE_SETSRAM_ADDR(n);
    crreg |= LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID(3);
    crreg |= LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD(2);
    putreg32(crreg, LCDC_FBLCLUTCR);
}

/**
 * hal_lcdc_write_gamma - write gamma data into lcdc memory
 * @gitem   : gamma item
 * @n      :  item address index
 */
void hal_lcdc_write_gamma(uint32_t citem, int n)
{
    uint32_t crreg;
    putreg32(citem, LCDC_FBLGAMMATWRDR);
    crreg = getreg32(LCDC_FBLGAMMATWRCR);
    crreg &= ~LCDC_FBLGAMMAT_SETSRAM_ADDR_MASK;
    crreg &= ~LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_MASK;
    crreg &= ~LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD_MASK;
    crreg |= LCDC_FBLGAMMAT_SETSRAM_ADDR(n);
    crreg |= LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID(0);
    crreg |= LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD(2);
    putreg32(crreg, LCDC_FBLGAMMATWRCR);
    crreg &= ~LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_MASK;
    crreg |= LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID(1);
    putreg32(crreg, LCDC_FBLGAMMATWRCR);
    crreg &= ~LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_MASK;
    crreg |= LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID(2);
    putreg32(crreg, LCDC_FBLGAMMATWRCR);

}

/**
 * hal_lcdc_lcadjcurves_enable - enable/disalbe  gamma curves function
 */
void hal_lcdc_lcadjcurves_enable(int lid, bool enable)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_PNGAMMAENACR);

    if (enable) {
        crreg |= (1 << 30);
        //crreg |= (1 << 29);
    } else {
        crreg &= ~(1 << 30);
    }
    putreg32(crreg, LCDC_PNGAMMAENACR);
}

/**
 * hal_lcdc_axifastmode_enable - enable/disalbe  axi bus fast mode function
 */
void hal_lcdc_axifastmode_enable(bool enable)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_PNGAMMAENACR);

    if (enable) {
        crreg |= (1 << 27);
    } else {
        crreg &= ~(1 << 27);
    }
    putreg32(crreg, LCDC_PNGAMMAENACR);
}

/**
 * hal_lcdc_lstartaddr - set lcdc layer dma memory start address
 * @lid    : Layer id
 * @addr   : memory address
 */
int hal_lcdc_lstartaddr(int lid, uint32_t addr)
{
    LCDC_INFO("%s lid:%d addr:%x", __func__, lid, (unsigned int)addr);
    putreg32(addr, hal_lcdc_mar_layer_t[lid]);
    return 0;
}

/**
 * hal_lcdc_getcuraddr - get lcdc layer dma memory start address
 * @lid    : Layer id
 */
uint32_t hal_lcdc_getcuraddr(int lid)
{
  return getreg32(hal_lcdc_mar_layer_t[lid]);
}

/**
 * hal_lcdc_lpitch - set lcdc vsync start/end pixes count
 * @lid    : Layer id
 * @pitch  : pitch in bytes
 */
void hal_lcdc_lpitch(int lid, uint16_t pitch)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s lid:%d pitch:%d", __func__, lid, pitch);
    pfccrreg = getreg32(hal_dma2d_lpitch_layer_t[lid]);
    if (lid == LCDC_LAYER_LWB) {
        pfccrreg &= ~LCDC_OLPITCH_MASK;
        pfccrreg |= LCDC_OLPITCH(pitch);
    } else {
        pfccrreg &= ~LCDC_LPITCH_MASK;
        pfccrreg |= LCDC_LPITCH(pitch);
    }
    putreg32(pfccrreg, hal_dma2d_lpitch_layer_t[lid]);
}

/**
 * hal_lcdc_lvsepxcount - set lcdc vsync start/end pix count
 * @start   : stby length
 * @end     : start line
 */
void hal_lcdc_lvsepxcount(int start, int end)
{
    uint32_t reg = getreg32(LCDC_SEPXCNTCR);
    reg &= ~(LCD_CFG_PN_V_SPXLCNT_MASK);
    reg &= ~(LCD_CFG_PN_V_EPXLCNT_MASK);
    reg |= LCD_CFG_PN_V_SPXLCNT(start) | LCD_CFG_PN_V_EPXLCNT(end);
    putreg32(reg, LCDC_SEPXCNTCR);
}

/**
 * hal_lcdc_stby_vsync_set - set stby vsync
 * @len   : stby length
 * @slen  : start line
 * @elen  : end line
 */
void hal_lcdc_stby_vsync_set(int len, int slen, int elen)
{

    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_STBY_VSYNCCR);
    reg &= ~(LCDC_CFG_STBY_LENGTH_MASK);
    reg &= ~(LCDC_CFG_VSYNC_START_LINE_MASK);
    reg &= ~(LCDC_CFG_VSYNC_END_LINE_MASK);

    reg |= LCDC_CFG_STBY_LENGTH(len) | LCDC_CFG_VSYNC_START_LINE(slen)
        | LCDC_CFG_VSYNC_END_LINE(elen);
    putreg32(reg, LCDC_STBY_VSYNCCR);
}

/**
 * hal_lcdc_porch_vsync_set - set vsync porch in pixes
 * @hf      : hFP
 * @hb      : hBP
 * @vlines  : NLINE
 */
void hal_lcdc_porch_vsync_set(int hf, int hb, int vlines)
{

    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_PORCH_VSYNCCR);
    reg &= ~(LCDC_CFG_H_FRONT_PORCH_SMPN_MASK);
    reg &= ~(LCDC_CFG_H_BACK_PORCH_SMPN_MASK);
    reg &= ~(LCDC_CFG_VSYNC_NLINE_MASK);

    reg |= LCDC_CFG_H_FRONT_PORCH_SMPN(hf) | LCDC_CFG_H_BACK_PORCH_SMPN(hb)
        | LCDC_CFG_VSYNC_NLINE(vlines);

    putreg32(reg, LCDC_PORCH_VSYNCCR);
}

/**
 * hal_lcdc_pix_vsync_set - set vsync position in pixes
 * @s  : start
 * @e  : end
 */
void hal_lcdc_pix_vsync_set(int s, int e)
{

    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_VSYNC_PIXCR);
    reg &= ~(LCDC_CFG_VSYNC_START_PIX_MASK);
    reg &= ~(LCDC_CFG_VSYNC_END_PIX_MASK);

    reg |= LCDC_CFG_VSYNC_START_PIX(s) | LCDC_CFG_VSYNC_END_PIX(e);
    putreg32(reg, LCDC_VSYNC_PIXCR);
}

/**
 * hal_lcdc_sponscreen_set - set  F/B Layer screen start position in pixes
 * @lid    : Layer id
 * @vln    : y in line
 * @hpixl  : x in pix
 */
void hal_lcdc_sponscreen_set(int lid, uint16_t vln, uint16_t hpixl)
{
    LCDC_INFO("%s lid:%d vln:%d hpixl:%d", __func__, lid, vln, hpixl);
    uint32_t pfccrreg = getreg32(hal_lcdc_sponscreen_layer_t[lid]);
    pfccrreg &= ~(LCDC_CFG_DMA_OVSA_HPXL_MASK);
    pfccrreg &= ~(LCDC_CFG_DMA_OVSA_VLN_MASK);
    pfccrreg |= LCDC_CFG_DMA_OVSA_HPXL(vln) | LCDC_CFG_DMA_OVSA_VLN(hpixl);
    pfccrreg |= LCDC_CFG_DMAFRM_FIX;
    putreg32(pfccrreg, hal_lcdc_sponscreen_layer_t[lid]);
}

/**
 * hal_lcdc_bgl_vhtotal_set - set  F/B Layer PN_V/H_TOTAL in pix
 * @h    : height total
 * @v    : width total
 */
void hal_lcdc_bgl_vhtotal_set(int h, int v)
{
    LCDC_INFO("%s w:%d h:%d", __func__, h, v);
    uint32_t pfccrreg = getreg32(LCDC_PVH_TOTAL_CR);
    pfccrreg &= ~(LCDC_CFG_PN_H_TOTAL_MASK);
    pfccrreg &= ~(LCDC_CFG_PN_V_TOTAL_MASK);
    pfccrreg |= LCDC_CFG_PN_H_TOTAL(h) | LCDC_CFG_PN_V_TOTAL(v);
    putreg32(pfccrreg, LCDC_PVH_TOTAL_CR);
}

/**
 * hal_lcdc_bgl_vtotal_set - set  F/B Layer PN_V_TOTAL in pixes
 * @v  : width total
 */
void hal_lcdc_bgl_vtotal_set(int v)
{
    LCDC_INFO("%s v:%d ", __func__, v);
    uint32_t pfccrreg = getreg32(LCDC_PVH_TOTAL_CR);
    pfccrreg &= ~(LCDC_CFG_PN_V_TOTAL_MASK);
    pfccrreg |= LCDC_CFG_PN_V_TOTAL(v);
    putreg32(pfccrreg, LCDC_PVH_TOTAL_CR);
}

/**
 * hal_lcdc_bgl_htotal_set - set  F/B Layer PN_H_TOTAL in pixes
 * @h    : height total
 */
void hal_lcdc_bgl_htotal_set(int h)
{
    LCDC_INFO("%s h:%d", __func__, h);
    uint32_t pfccrreg = getreg32(LCDC_PVH_TOTAL_CR);
    pfccrreg &= ~(LCDC_CFG_PN_H_TOTAL_MASK);
    pfccrreg |= LCDC_CFG_PN_H_TOTAL(h);
    putreg32(pfccrreg, LCDC_PVH_TOTAL_CR);
}

/**
 * hal_lcdc_bgl_hporch_set - set  F/B Layer HPORCH in pixes
 * @f     : fore porch
 * @b     : back porch
 */
void hal_lcdc_bgl_hporch_set(int f, int b)
{
    LCDC_INFO("%s f:%d b:%d", __func__, f, b);
    uint32_t pfccrreg = getreg32(LCDC_PHPORCH_CR);
    pfccrreg &= ~(LCDC_CFG_PN_H_FPORCH_MASK);
    pfccrreg &= ~(LCDC_CFG_PN_H_BPORCH_MASK);
    pfccrreg |= LCDC_CFG_PN_H_FPORCH(f);
    pfccrreg |= LCDC_CFG_PN_H_BPORCH(b);
    putreg32(pfccrreg, LCDC_PHPORCH_CR);
}

/**
 * hal_lcdc_bgl_vporch_set - set  F/B Layer VPORCH in pixes
 *
 * Input Parameters:
 * @f  : fore porch
 * @b  : back porch
 */
void hal_lcdc_bgl_vporch_set(int f, int b)
{
    LCDC_INFO("%s f:%d b:%d", __func__, f, b);
    uint32_t pfccrreg = getreg32(LCDC_PVPORCH_CR);
    pfccrreg &= ~(LCDC_CFG_PN_V_FPORCH_MASK);
    pfccrreg &= ~(LCDC_CFG_PN_V_BPORCH_MASK);
    pfccrreg |= LCDC_CFG_PN_V_FPORCH(f);
    pfccrreg |= LCDC_CFG_PN_V_BPORCH(b);
    putreg32(pfccrreg, LCDC_PVPORCH_CR);
}

/**
 * hal_lcdc_lsize - set  Layer image size in pixes
 * @lid   : LCDC_LAYER_SPU only
 * @w     : Layer width
 * @h     : Layer height
 */
void hal_lcdc_lsize(int lid, uint16_t w, uint16_t h)
{
    LCDC_INFO("%s lid:%d w:%d h:%d", __func__, lid, w, h);
    uint32_t pfccrreg = getreg32(hal_lcdc_lszr_layer_t[lid]);
    pfccrreg &= ~(LCDC_AWCR_AAW_MASK);
    pfccrreg &= ~(LCDC_AWCR_AAH_MASK);
    if (lid == LCDC_LAYER_SPU) {
#ifdef CONFIG_DSI_VIDEO_MODE
        pfccrreg |= LCDC_AWCR_AAW(w ) | LCDC_AWCR_AAH(h);
#else
        pfccrreg |= LCDC_AWCR_AAW(w + 1) | LCDC_AWCR_AAH(h + 1);
#endif
    } else {
        pfccrreg |= LCDC_AWCR_AAW(w) | LCDC_AWCR_AAH(h);
    }
    putreg32(pfccrreg, hal_lcdc_lszr_layer_t[lid]);

}

/**
 * hal_lcdc_lzoom_set - set  F/B Layer zoom in pixes
 * @lid  : Layer id
 * @w    : window active area width
 * @h    : window active area height
 */
void hal_lcdc_lzoom_set(int lid, uint16_t w, uint16_t h)
{
    LCDC_INFO("%s lid:%d w:%d h:%d", __func__, lid, w, h);
    uint32_t pfccrreg = getreg32(hal_lcdc_zoomr_layer_t[lid]);
    pfccrreg &= ~(LCDC_CFG_DMAZM_H_PIXEL_MASK);
    pfccrreg &= ~(LCDC_CFG_DMAZM_V_LINE_MASK);
    pfccrreg |= LCDC_CFG_DMAZM_H_PIXEL(w) | LCDC_CFG_DMAZM_V_LINE(h);
    putreg32(pfccrreg, hal_lcdc_zoomr_layer_t[lid]);

}

/**
 * hal_lcdcl_enable - enable/disable lcdc Layer
 * @lid     : Layer index
 * @enable  : enable/disable
 */
void hal_lcdcl_enable(int lid, bool enable)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s lid:%d enable:%d", __func__, lid, enable);
    pfccrreg = getreg32(hal_lcdc_pfccr_layer_t[lid]);
    pfccrreg &= ~LCDC_xGPFCCR_LENA_MASK;
    pfccrreg |= LCDC_xGPFCCR_LENA(enable);
    putreg32(pfccrreg, hal_lcdc_pfccr_layer_t[lid]);
}

void hal_lcdc_test_mode(bool enable)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s enable:%d", __func__, enable);
    pfccrreg = getreg32(LCDC_BGPFCCR);
    pfccrreg &= ~(1<<5);
    pfccrreg |= (enable<<5);
    putreg32(pfccrreg, LCDC_BGPFCCR);
}

//for video mode dumb panel enable
void hal_lcdcl_dumb_enable( bool enable)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s enable:%d", __func__, enable);
    pfccrreg = getreg32(LCDC_DUMBCR);
    pfccrreg &= ~LCDC_xGPFCCR_LENA_MASK;
    pfccrreg |= LCDC_xGPFCCR_LENA(enable);
    putreg32(pfccrreg, LCDC_DUMBCR);
}

//for video mode: enable VSYNC sigal on dumb panel
void hal_lcdc_dumb_vsync_mode(uint8_t mode)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s mode:%d", __func__,  mode);
#if LAYER0_USE_GRA_FIFO
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LBACK] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#else
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LFORE] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#endif
    pfccrreg = getreg32(reg);
    pfccrreg &= ~LCDC_CFG_DUMBMODE_MASK;
    pfccrreg |= LCDC_CFG_DUMBMODE(mode);
    putreg32(pfccrreg, reg);
}

//switch the dumb panel as source
void hal_lcdc_source_switch(bool isdumb)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    if(!isdumb)
        crreg &= ~LCDC_DISPCNFCR_DSI_SOURCE_SEL;
    else
        crreg |= LCDC_DISPCNFCR_DSI_SOURCE_SEL;
    putreg32(crreg, LCDC_DISPCNFCR);
}

/**
 * hal_lcdc_lcolor - set  background Layer color
 * @lid    : LCDC_LAYER_SPU only
 * @argb   : color in ARGB888
 */
void hal_lcdc_lcolor(int lid, uint32_t argb)
{
    LCDC_INFO("lid=%d, argb=%08x\n", lid, (unsigned int)argb);
    ASSERT((lid == LCDC_LAYER_SPU), "%s invalid layer ", __func__);
    putreg32(argb, hal_lcdc_color_layer_t[lid]);
}

/**
 * hal_lcdc_dither_s4x8 - enable/disable lcdc  DITHER_4X8_PN
 * @enable   : enable/disable
 */
void hal_lcdc_dither_s4x8(bool enable)
{
    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_DITHERCR);

    reg &= ~(LCDC_DITHER_4X8_PN_MASK);
    reg |= LCDC_DITHER_4X8_PN(enable);

    putreg32(reg, LCDC_DITHERCR);
}

/**
 * hal_lcdc_dither - lcdc  dither configuration
 * @enable   : enable/disable
 * @level    : 0: low  1: high
 * @red      : color width(4,5,6,5)
 * @green    : color width(4,6,6,5)
 * @blue     : color width(4,5,6,5)
 */
void hal_lcdc_dither(bool enable, uint8_t level, uint8_t red,
             uint8_t green, uint8_t blue)
{
    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_DITHERCR);
    uint8_t *dither_mode_table =
        (level > 0) ? dither_mode_high : dither_mode;
    uint8_t mode_index = 0;
    if (red == 5 && green == 5 && blue == 5) {
        mode_index = 3;
    } else {
        mode_index = red - 4;
    }

    reg &= ~(LCDC_DITHER_EN_PN_MASK);
    reg &= ~(LCDC_DITHER_MODE_PN_MASK);

    reg |=
        LCDC_DITHER_EN_PN(enable) |
        LCDC_DITHER_MODE_PN(dither_mode_table[mode_index]);

    putreg32(reg, LCDC_DITHERCR);
}

/**
 * hal_lcdc_dither_read - read  lcdc  dither table memory
 * @index   : table memory index
 *
 * Returns: mem value of dither table
 */
uint32_t hal_lcdc_dither_read(uint8_t index)
{
    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(LCDC_DITHERCR);

    reg &= ~(LCDC_DITHER_TABLE_INDEX_SEL_MASK);
    reg |= LCDC_DITHER_TABLE_INDEX_SEL(index);

    putreg32(reg, LCDC_DITHERCR);

    reg = getreg32(LCDC_DITHERDR);
    return reg;
}

/**
 * hal_lcdc_dither_write - write  lcdc  dither table memory
 * @index   : table memory index
 * @value   : write value to dither table
 */
void hal_lcdc_dither_write(uint8_t index, uint32_t value)
{
    // LCDC_INFO("%s ", __func__ );
    uint32_t reg = getreg32(LCDC_DITHERCR);

    reg &= ~(LCDC_DITHER_TABLE_INDEX_SEL_MASK);
    reg |= LCDC_DITHER_TABLE_INDEX_SEL(index);

    putreg32(reg, LCDC_DITHERCR);

    reg = getreg32(LCDC_DITHERDR);
    reg &= ~(LCDC_DITHER_TBL_DATA_MASK);
    reg |= LCDC_DITHER_TBL_DATA(value);
    putreg32(reg, LCDC_DITHERDR);
}

static const uint8_t lcdc_fbl_cfmt[] = {
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
    0xff,            //#define FB_FMT_RGB16_555      10  /* BPP=16 R=5, G=5, B=5 (1 unused bit) */
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

uint8_t hal_fbcfmt(uint8_t fbcfmt)
{
    uint8_t fmt = lcdc_fbl_cfmt[fbcfmt];
    ASSERT(fmt < 0xff, "%s invalid fb color format:%d", __func__, fbcfmt);
    return fmt;
}

static const uint8_t lcdc_ol_cfmt[] = {
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
    0xff,            //#define FB_FMT_YUVP           45  /* BPP=24? YCbCr 4:2:2 Y0U0Y1V0 order */

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

static uint8_t lcdc_blend_mode[] = {
    LCDC_PFCCR_AM_CONST,    //#  define FB_CONST_ALPHA      0x00         /* Transparency by alpha value */
    LCDC_PFCCR_AM_BLPIXEL,    //#  define FB_PIXEL_ALPHA    0x01         /* Transparency by pixel alpha value */
};

static bool is_yuv(uint8_t fmt)
{
    return (lcdc_fbl_cfmt[fmt] == 05);
}

/**
 * hal_lcdc_lpfc_fmt - Configure foreground and background color format
 * @lid  : Layer id (output, foreground, background)
 * @fmt  : FB_FMT_XXX
 */
void hal_lcdc_lpfc_fmt(int lid, uint8_t fmt)
{
    uint32_t pfccrreg0;

    LCDC_INFO("%s lid:%d  fmt:%d", __func__, lid, fmt);
    ASSERT((lid != LCDC_LAYER_SPU), " %s invalid layer ", __func__);
    pfccrreg0 = getreg32(hal_lcdc_pfccr_layer_t[lid]);

    if (lid == LCDC_LAYER_LWB) {
        ASSERT((0xFF != lcdc_ol_cfmt[fmt]), "%s invalid color fmt:%d",
               __func__, fmt);
        pfccrreg0 &= ~LCDC_OPFCCR_CM_MASK;
        pfccrreg0 |= LCDC_OPFCCR_CM(0x0F & (lcdc_ol_cfmt[fmt]));
    } else {
        ASSERT((0xFF != lcdc_fbl_cfmt[fmt]), "%s invalid color fmt:%d",
               __func__, fmt);
        pfccrreg0 &= ~LCDC_xGPFCCR_CM_MASK;
        pfccrreg0 |= LCDC_xGPFCCR_CM(0x0F & (lcdc_fbl_cfmt[fmt]));
    }
    //enable yuv2rgb
    if (is_yuv(fmt)) {
        pfccrreg0 |= LCDC_CFG_YUV2RGB_ENA(0x1);
    } else {
        pfccrreg0 &= ~LCDC_CFG_YUV2RGB_ENA_MASK;

    }
    putreg32(pfccrreg0, hal_lcdc_pfccr_layer_t[lid]);

}

void hal_lcdc_lchromakeyenable(int lid, bool enable)
{
    uint32_t crreg;
    LCDC_INFO("%s lid:%d  ena:%d", __func__, lid, enable);
    if (lid > 0) {
        LCDC_INFO("%s invalid  lid:%d", __func__, lid);
        return;
    }
    crreg = getreg32(LCDC_BFGCHOKCR);
    crreg &= ~(LCDC_xGPFCCR1_CCM_MASK);
    if (enable) {
        crreg |= LCDC_xGPFCCR1_CCM(CKEY_RGB_ALL);
    }
    putreg32(crreg, LCDC_BFGCHOKCR);

    crreg = getreg32(LCDC_CHOKYENACR);
    crreg &= ~(LCDC_CFG_PNV_CKEY_DMA);
    crreg &= ~(LCDC_CFG_PNG_CKEY_GRA);
    if (enable) {
        crreg |= LCDC_CFG_PNV_CKEY_DMA | LCDC_CFG_PNG_CKEY_GRA;
    }
    putreg32(crreg, LCDC_CHOKYENACR);

}

/**
 * hal_lcdc_lpfc_ltransp - Configure fore and back ground blend function
 * @lid     : Layer id (output, foreground, background)
 * @mode    : FB blend mode: FB_XXX_ALPHA
 * @alpha   : Layer alpha
 *
 */
void hal_lcdc_lpfc_ltransp(int lid, uint32_t mode, uint8_t alpha)
{
    uint32_t pfccrreg1;
    //bool       swaprb = false;
    uint32_t blend_mode;
    LCDC_INFO("%s lid:%d bmode:%d alpha:%d ", __func__, lid, (unsigned int)mode, alpha);
    ASSERT((lid != LCDC_LAYER_SPU), " %s invalid layer ", __func__);
    blend_mode = lcdc_blend_mode[mode];

    if (lid != LCDC_LAYER_LFORE) {
        return;
    }
    /* Set alpha blend mode */

#if LAYER0_USE_GRA_FIFO
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LBACK] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#else
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LFORE] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#endif
    pfccrreg1 =
        getreg32(reg);
    pfccrreg1 &= ~LCDC_xGPFCCR1_AM_MASK;
    pfccrreg1 |= LCDC_xGPFCCR1_AM(blend_mode);

    if (blend_mode == LCDC_PFCCR_AM_CONST) {
        /* Set alpha value */
        pfccrreg1 &= ~LCDC_xGPFCCR1_ALPHA_MASK;

        pfccrreg1 |= LCDC_xGPFCCR1_ALPHA(alpha);
    }
    putreg32(pfccrreg1, reg);
}

/**
 * hal_lcdc_lchromakey - foreground and background color key Configure
 * @lid   : Layer id
 * @rgb   : chroma key
 * @rrgb  : color to replace chroma key
 * @ra    :  new alpha for replaced pixes
 *
 * Set foreground and background color key Configure, real color will be
 * replaced , if it's value is equal to the chroma key
 *
 */
void hal_lcdc_lchromakey(int lid, uint32_t rgb, uint32_t rrgb, uint8_t ra)
{
    uint32_t reg;

    uint8_t r, g, b;
    uint8_t rr, rg, rb;

    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0xff);
    rr = (rrgb & 0xff0000) >> 16;
    rg = (rrgb & 0x00ff00) >> 8;
    rb = (rrgb & 0xff);

    reg = getreg32(LCDC_FGCHOKYSR);
    reg &= ~LCDC_CFG_PN_CKEY_Y_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_Y1_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_Y2_MASK;
    reg &= ~LCDC_CFG_PN_ALPHA_Y_MASK;

    reg |= LCDC_CFG_PN_CKEY_Y(rr);
    reg |= LCDC_CFG_PN_CKEY_Y1(r);
    reg |= LCDC_CFG_PN_CKEY_Y2(r);
    reg |= LCDC_CFG_PN_ALPHA_Y(ra);
    putreg32(reg, LCDC_FGCHOKYSR);

    reg = getreg32(LCDC_FGCHOKUSR);
    reg &= ~LCDC_CFG_PN_CKEY_U_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_U_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_U_MASK;
    reg &= ~LCDC_CFG_PN_ALPHA_U_MASK;
    reg |= LCDC_CFG_PN_CKEY_U(rg);
    reg |= LCDC_CFG_PN_CKEY_U1(g);
    reg |= LCDC_CFG_PN_CKEY_U2(g);
    reg |= LCDC_CFG_PN_ALPHA_U(ra);
    putreg32(reg, LCDC_FGCHOKUSR);

    reg = getreg32(LCDC_FGCHOKVSR);
    reg &= ~LCDC_CFG_PN_CKEY_V_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_V1_MASK;
    reg &= ~LCDC_CFG_PN_CKEY_V2_MASK;
    reg &= ~LCDC_CFG_PN_ALPHA_V_MASK;
    reg |= LCDC_CFG_PN_CKEY_V(rb);
    reg |= LCDC_CFG_PN_CKEY_V1(b);
    reg |= LCDC_CFG_PN_CKEY_V2(b);
    reg |= LCDC_CFG_PN_ALPHA_V(ra);
    putreg32(reg, LCDC_FGCHOKVSR);

}

/**
 * hal_lcdc_lchromakey_newalpha - foreground and background color key Configure
 * @lid    : Layer id
 * @alpha  : new alpha for replaced pixes
 *
 * set foreground and background color key Configure, real color alpha will be
 * replaced , if it's color value is equal to the chroma key
 *
 */
void hal_lcdc_lchromakey_newalpha(int lid, uint8_t newalpha)
{
    uint32_t reg;

    reg = getreg32(LCDC_FGCHOKYSR);
    reg &= ~LCDC_CFG_PN_ALPHA_Y_MASK;
    reg |= LCDC_CFG_PN_ALPHA_Y(newalpha);
    putreg32(reg, LCDC_FGCHOKYSR);

    reg = getreg32(LCDC_FGCHOKUSR);
    reg &= ~LCDC_CFG_PN_ALPHA_U_MASK;
    reg |= LCDC_CFG_PN_ALPHA_U(newalpha);
    putreg32(reg, LCDC_FGCHOKUSR);

    reg = getreg32(LCDC_FGCHOKVSR);
    reg &= ~LCDC_CFG_PN_ALPHA_V_MASK;
    reg |= LCDC_CFG_PN_ALPHA_V(newalpha);
    putreg32(reg, LCDC_FGCHOKVSR);
}

/**
 * hal_lcdc_panelpath_dmatrans_trigger_enable - enable/disable lcdc panel dma trigger
 * @lid    : Layer id
 * @ena    : enable/disable
 */
void hal_lcdc_panelpath_dmatrans_trigger_enable(int lid, bool ena)
{
    uint32_t crreg;
    LCDC_INFO("lid=%d, ena=%08x\n", lid, ena);

#if LAYER0_USE_GRA_FIFO
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LBACK] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#else
    uint32_t reg = hal_lcdc_pfccr_layer_t[LCDC_LAYER_LFORE] +
             (LCDC_FGPFCCR1 - LCDC_FGPFCCR);
#endif
    crreg = getreg32(reg);
    if (ena && (crreg & LCDC_xGPFCCR1_TRANS_TRIGGER_MASK)) {
        return;
    }
    LCDC_INFO("lid=%d, set  ena=%08x\n", lid, ena);
    crreg &= ~LCDC_xGPFCCR1_TRANS_TRIGGER_MASK;
    crreg |= LCDC_xGPFCCR1_TRANS_TRIGGER_ENA(ena);
    putreg32(crreg, reg);
}

/**
 * hal_lcdc_disp_avsync_trigger_disable - enable/disable lcdc disp avsync trigger
 * @dis     : enable/disable
 */
void hal_lcdc_disp_avsync_trigger_disable(bool dis)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    crreg &= ~LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE_MASK;
    crreg |= LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE(dis);
    putreg32(crreg, LCDC_DISPCNFCR);
}

/**
 * hal_lcdc_disp_svsync_trigger_disable - enable/disable lcdc disp svsync trigger
 * @dis   : enable/disable
 */
void hal_lcdc_disp_svsync_trigger_disable(bool dis)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    crreg &= ~LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE_MASK;
    crreg |= LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE(dis);
    putreg32(crreg, LCDC_DISPCNFCR);
}

/**
 * hal_lcdc_disp_mem_enable - enable/disable lcdc disp memory
 * @dis  : enable/disable
 */
void hal_lcdc_disp_mem_enable(bool ena)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    crreg &= ~LCDC_DISPCNFCR_MEM_ENABLE_MASK;
    crreg |= LCDC_DISPCNFCR_MEM_ENABLE(ena);
    putreg32(crreg, LCDC_DISPCNFCR);
}

void hal_lcdc_disp_extra_delay(uint16_t delay)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    crreg &= ~LCDC_DISPCNFCR_EXTRA_DELAY_MASK;
    crreg |= LCDC_DISPCNFCR_EXTRA_DELAY(delay);
    putreg32(crreg, LCDC_DISPCNFCR);
}

/**
 * hal_lcdc_ol_dmaburst_length_set - set lcdc wb path dma burst length
 * @len     :  burst length
 */
void hal_lcdc_ol_dmaburst_length_set(uint8_t len)
{
    LCDC_INFO("%s ", __func__);
    uint32_t pfccrreg = getreg32(LCDC_WBBURSTCR);
    pfccrreg &= ~(LCDC_OL_BURST_LEN_MASK);
    pfccrreg |= LCDC_OL_BURST_LEN(len);
    putreg32(pfccrreg, LCDC_WBBURSTCR);
}

/**
 * hal_lcdc_smpn_mode_set - set lcdc smpn mode
 * @mode : smpn mode (0x0 = 3 cycles per pixel;
 *                    0x1 = RGB666, 3 cycles per pixel
 *                    0x2 = RGB565, 2 cycles per pixel;
 *                    0x3 = 1 cycle per pixel;
 *                    0x4 = RGB666, 1 cycle per pixel;
 *                    0x5 = RGB565 1 cycle per pixel )
 */
void hal_lcdc_smpn_mode_set(int mode)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s  mode:%d", __func__, mode);
    pfccrreg = getreg32(LCDC_SMPNCR);
    pfccrreg &= ~LCDC_SMPNCR_MODE_MASK;
#ifdef CONFIG_DSI_VIDEO_MODE
    pfccrreg &= ~LCDC_CFG_SLV_ENA;
    pfccrreg |= LCDC_CFG_SLV_ENA;
#endif
    pfccrreg |= LCDC_SMPNCR_MODE_SET(mode);
    putreg32(pfccrreg, LCDC_SMPNCR);
}

/**
 * hal_lcdc_irq_disable - mask all lcdc_irqs
 */
int hal_lcdc_irq_disable(void)
{
    int ret = 0;
    uint32_t crreg = 0;
    putreg32(crreg, LCDC_IER);
    return ret;
}

uint32_t hal_lcdc_get_irqstat(void)
{
    uint32_t sreg = getreg32(LCDC_ISR);
    return sreg;
}

/**
 * hal_lcdc_spu_framedone_irq_enable - enable/disable spu frame done irq.
 * @ena    : ture/false
 *
 */
int hal_lcdc_framedone_irq_enable(bool ena)
{
    int ret = 0;
    uint32_t crreg;
    LCDC_INFO("%s enable:%d", __func__, ena);
    crreg = getreg32(LCDC_IER);
#ifdef CONFIG_DSI_VIDEO_MODE
    crreg &= ~DUMB_FRAMEDONE_IRQ_ENA_MASK;
    if (ena)
        crreg |= DUMB_FRAMEDONE_IRQ_ENA|PN_VSYNC_IRQ_ENA;
#else
    crreg &= ~SPU_FRAMEDONE_IRQ_ENA_MASK;
    if (ena)
        crreg |= SPU_FRAMEDONE_IRQ_ENA;
#endif
    putreg32(crreg, LCDC_IER);

    return ret;
}

/**
 * hal_lcdc_spu_framedone_irqstat_clr  - clear spu frame done irq status.
 * @ena   : ture/false
 *
 */
int hal_lcdc_framedone_irqstat_clr(void)
{
    int ret = 0;
    uint32_t crreg;
    LCDC_INFO("%s ", __func__);
    crreg = getreg32(LCDC_ISR);
#ifdef CONFIG_DSI_VIDEO_MODE
    crreg &= ~(DUMB_FRAMEDONE_CLEAN);
#else
    crreg &= ~(SPU_FRAMEDONE_CLEAN);
#endif
    putreg32(0, LCDC_ISR);

    return ret;
}

int hal_lcdc_vsync_irqstat_clr(void)
{
    int ret = 0;
    LCDC_INFO("%s ", __func__);
#ifdef CONFIG_DSI_VIDEO_MODE
    uint32_t crreg;
    crreg = getreg32(LCDC_ISR);
    crreg &= ~(PN_VSYNC_IRQ_CLEAN);
    putreg32(0, LCDC_ISR);
#else
    hal_dsi_clear_teirq_flag();
#endif
    return ret;
}

/**
 * hal_lcdc_irq_is_framedone - lcdc framedone irq status.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_framedone(uint32_t regval)
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return ! !(regval & DUMB_FRAMEDONE);
#else
    return ! !(regval & SPU_TXC_FRAMEDONE);
#endif
}

bool hal_lcdc_irq_is_vsync(uint32_t regval)
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return ! !(regval & PN_VSYNC_IRQ);
#else
    return ! !(regval & DSI_TE_INTR);
#endif
}

bool hal_lcdc_irq_is_framedone_irq0(uint32_t regval)
{
    return ! !(regval & FGL_PDMA_FRAME_IRQ0) || ! !(regval & BGL_GRA_FRAME_IRQ0);
}

bool hal_lcdc_irq_is_framedone_irq1(uint32_t regval)
{
    return ! !(regval & FGL_DMA_FRAME_IRQ1) || ! !(regval & BGL_GRA_FRAME_IRQ1);
}

/**
 * hal_lcdc_irq_is_error - Get whether error irq status is ture or not
 * @regval  : lcdc ISR value
 *
 * Returns:  TRUE on erro irq, FALSE on without erro irq.
 */
bool hal_lcdc_irq_is_error(uint32_t regval)
{
    return ! !(regval &
           (LCDC_ERR_IRQ | BGL_GRA_FF_UNDERFLOW
            | FGL_DMA_FF_UNDERFLOW));
}

/**
 * hal_lcdc_error_irq_enable - enable  all lcdc_error irqs
 */
int hal_lcdc_error_irq_enable(void)
{
    int ret = 0;
    uint32_t crreg = getreg32(LCDC_IER);
    crreg |= LCDC_ERR_IRQ_ENA(1);
    crreg |= FGL_PDMA_FF_UNDERFLOW_ENA;
    crreg |= BGL_GRA_FF_UNDERFLOW_ENA;
    putreg32(crreg, LCDC_IER);
    return ret;
}

/**
 * hal_lcdc_error_irqstat_clr - enable  all lcdc_error irqs
 */
int hal_lcdc_error_irqstat_clr(void)
{
    int ret = 0;
    reg32_bitset(LCDC_ERR_IRQ_CLEAN, LCDC_ICR);
    return ret;
}

/**
 * hal_lcdc_disp_pn_reset   - reset display pannel
 *
 * reset state context of lcdc pannel
 */
void hal_lcdc_disp_pn_reset(void)
{
    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PN);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PN);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_TV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_TV);
}

/**
 * hal_lcdc_reset - reset lcd configuration
 *
 * registers reset
 */
void hal_lcdc_reset(void)
{
    LCDC_REGINFO("hal_lcdc_reset();");

    hal_cmu_reset_set(HAL_CMU_MOD_H_DISPPRE);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_DISPPRE);

    hal_cmu_reset_set(HAL_CMU_MOD_H_LCDC);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_LCDC);

    hal_cmu_reset_set(HAL_CMU_MOD_H_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_H_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISP);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISP);

    hal_cmu_reset_set(HAL_CMU_MOD_X_DISPB);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_X_DISPB);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PN);
    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_TV);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PN);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_TV);

#if 0
    // dsi reset
    hal_cmu_reset_set(HAL_CMU_MOD_Q_LCDC_P);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_LCDC_P);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_PIX);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_PIX);

    hal_cmu_reset_set(HAL_CMU_MOD_Q_DIS_DSI);
    hal_sys_timer_delay(1);
    hal_cmu_reset_clear(HAL_CMU_MOD_Q_DIS_DSI);
#endif
}

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable.
 */
void hal_lcdc_smpn_enable(void)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_SMPNCR);
    crreg |= LCDC_CFG_SMPN_ENA;
    putreg32(crreg, LCDC_SMPNCR);
}

/**
 * hal_lcdc_smpn_vsync_delaymode_set - set lcdc smpn module vsync delay.
 */
void hal_lcdc_smpn_vsync_delaymode_set(uint8_t delay)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_SMPNCR);
    crreg &= ~LCDC_CFG_SMPNVSYNC_MASK;
    crreg |= LCDC_CFG_SMPNVSYNC(delay);
    putreg32(crreg, LCDC_SMPNCR);
}

/**
 * hal_lcdc_smpn_vsync_delaymode_get - get lcdc smpn module vsync delay.
 */
uint32_t hal_lcdc_smpn_vsync_delaymode_get(void)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_SMPNCR);
    crreg &= LCDC_CFG_SMPNVSYNC_MASK;
    return (crreg >> LCDC_CFG_SMPNVSYNC_SHIFT);
}

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable/disable.
 */
void hal_lcdc_gen_frame_enable(bool ena)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DISPCNFCR);
    crreg &= ~LCDC_DISPCNFCR_REG_GEN_FRAME_MASK;
    crreg |= LCDC_DISPCNFCR_REG_GEN_FRAME(ena);
    putreg32(crreg, LCDC_DISPCNFCR);
}

/**
 * hal_lcdc_set_sync_line_pixes - set lcdc sync pixs.
 */
void hal_lcdc_set_sync_line_pixs(uint32_t pixs)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_HSYNCLENCR);
    crreg &= ~LCDC_DISPCNFCR_HSYNC_LENGTH_MASK;
    crreg |= LCDC_DISPCNFCR_HSYNC_LENGTH(pixs);
    putreg32(crreg, LCDC_HSYNCLENCR);
}

/**
 * hal_lcdc_set_sync_lines  - set lcdc sync lines.
 */
void hal_lcdc_set_sync_lines(uint32_t lines)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_HSYNCLENCR);
    crreg &= ~LCDC_DISPCNFCR_DEL_NLINE_MASK;
    crreg |= LCDC_DISPCNFCR_DEL_NLINE(lines);
    putreg32(crreg, LCDC_HSYNCLENCR);
}

/**
 * hal_lcdc_get_sync_delay_pixs - set lcdc delay pixes.
 */
uint32_t hal_lcdc_get_sync_delay_pixs(void)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_HSYNCLENCR);
    crreg &= LCDC_DISPCNFCR_HSYNC_LENGTH_MASK;
    return (crreg >> LCDC_DISPCNFCR_HSYNC_LENGTH_SHIFT);
}

/**
 * hal_lcdc_get_sync_delay_lines - get lcdc delay lines.
 */
uint32_t hal_lcdc_get_sync_delay_lines(void)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_HSYNCLENCR);
    crreg &= LCDC_DISPCNFCR_DEL_NLINE_MASK;
    return (crreg >> LCDC_DISPCNFCR_DEL_NLINE_SHIFT);
}

/**
 * hal_lcdc_start_wb  - Starts the wblayer dma2d transfer process
 */
int hal_lcdc_start_wb(void)
{
    int ret = 0;
    uint32_t crreg;

    hal_lcdc_gen_frame_enable(true);
    crreg = getreg32(LCDC_STARTCR);
    crreg &= ~LCDC_STARTCR_MASK;
    crreg |= LCDC_STARTCR_ENA(1);
    putreg32(crreg, LCDC_STARTCR);

    LCDC_INFO("%s\n", __func__);
    return ret;
}

/**
 * hal_lcdc_gen_start  - Starts the  dma2d transfer process by write register
 */
int hal_lcdc_gen_start(void)
{
    int ret = 0;
    uint32_t crreg;

    crreg = getreg32(LCDC_STARTCR);
    crreg &= ~LCDC_STARTCR_MASK;
    crreg |= LCDC_STARTCR_ENA(1);
    putreg32(crreg, LCDC_STARTCR);

    return ret;
}

/**
 * hal_lcdc_start_dsi - Starts the lcdc transfer process.
 */
int hal_lcdc_start_dsi(void)
{
    int ret = 0;
    uint32_t crreg;
#if 0
    bool sready = false;
    uint32_t pixs, lines;

    uint32_t delaymode = hal_lcdc_smpn_vsync_delaymode_get();

    if (delaymode == 1) {
        while ((sready == false)) {
            pixs = hal_lcdc_get_sync_delay_pixs();
            lines = hal_lcdc_get_sync_delay_lines();
            sready = (pixs == 0) && (lines == 0);
        }
    }
#endif
    hal_lcdc_gen_frame_enable(false);
#ifndef CONFIG_DSI_VIDEO_MODE
    hal_lcdc_smpn_enable();
#endif
	crreg = getreg32(LCDC_WBPCHCR);
	crreg = 0x0;
	putreg32(crreg, LCDC_WBPCHCR);

    crreg = getreg32(LCDC_TECR);
    crreg |= LCDC_TECNFCR_USE_SW;
    putreg32(crreg, LCDC_TECR);

    crreg = getreg32(LCDC_TETRRIGERCR);//reg258
    crreg |= 0x0;
    putreg32(crreg, LCDC_TETRRIGERCR);

    LCDC_INFO("%s\n", __func__);

    return ret;
}

void hal_lcdc_te_trigger()
{
    uint32_t crreg;
    crreg = getreg32(LCDC_TETRRIGERCR);//reg258
    crreg |= 0x0;
    putreg32(crreg, LCDC_TETRRIGERCR);
}

/**
 * hal_lcdc_vsync_porch_set  - set vsync porch with default value
 */
void hal_lcdc_vsync_porch_set(void)
{

    LCDC_INFO("%s ", __func__);
#ifdef FPGA
    putreg32(0x501200, LCDC_STBY_VSYNCCR);
#else
    putreg32(0x501080, LCDC_STBY_VSYNCCR);
#endif
    putreg32(0xa020020, LCDC_PORCH_VSYNCCR);
    putreg32(0x1f001e, LCDC_VSYNC_PIXCR);
}

/**
 *  hal_lcdc_axibus_arbiter_fastmode_enable - AXI Bus Arbiter Fast Mode Enable
 */
void hal_lcdc_axibus_arbiter_fastmode_enable(void)
{
    LCDC_INFO("%s ", __func__);
    uint32_t reg = getreg32(lcdc_tvd_ctrl);
    reg |=(1 << 27);
    putreg32(reg, lcdc_tvd_ctrl);
}

/**
 * hal_lcdc_wdma_1fpt_set - set write back dma in one frame per trigger mode
 */
void hal_lcdc_wdma_1fpt_set(void)
{
    LCDC_INFO("%s ", __func__);
    uint32_t pfccrreg = getreg32(LCDC_WBFPTCR);
    pfccrreg &= ~(LCDC_OL_FPT_MASK);
    pfccrreg |= LCDC_OL_FPT_SET(1);

    putreg32(pfccrreg, LCDC_WBFPTCR);
}

/**
 * hal_lcdc_wdma_toggle_frame - toggle signal to start a write back dma frame
 */
void hal_lcdc_wdma_toggle_frame(void)
{
    LCDC_INFO("%s ", __func__);
    uint32_t pfccrreg = getreg32(LCDC_WBTGCR);
    if (pfccrreg & LCDC_WBTG_MASK) {
        pfccrreg &= ~(LCDC_WBTG_MASK);
    } else {
        pfccrreg |= LCDC_WBTG(1);
    }
    putreg32(pfccrreg, LCDC_WBFPTCR);

}

/**
 * hal_lcdc_select_top - select f/b dma path  as top layer to blender
 */
void hal_lcdc_select_top(uint8_t lid)
{
    LCDC_INFO("%s lid:%d ", __func__, lid);
    uint32_t pfccrreg = getreg32(LCDC_BLDCR);

    pfccrreg &= ~(LCDC_BLDCR_FIRSTLSEL_MASK);
    pfccrreg |= LCDC_BLDCR_FIRSTLSEL(lid);
    putreg32(pfccrreg, LCDC_BLDCR);
}

/**
 * hal_lcdc_swap_path - select f/b dma path
 */
void hal_lcdc_swap_path(uint8_t lid)
{
    LCDC_INFO("%s lid:%d ", __func__, lid);
    uint32_t pfccrreg = getreg32(LCDC_SWAPCR);

    pfccrreg &= ~(LCDC_SWAPPPPCR_ASEL_MASK);
    pfccrreg |= LCDC_SWAPPPPCR_ASEL(lid);
    putreg32(pfccrreg, LCDC_SWAPCR);
}

/**
 * hal_lcdc_swap_path_enable  - enable selected f/b dma path
 */
void hal_lcdc_swap_path_enable(bool enable)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s enable:%d", __func__, enable);
    pfccrreg = getreg32(LCDC_SWAPCR);
    pfccrreg &= ~LCDC_SWAPPPPCR_ASEL_ENA_MASK;
    pfccrreg |= LCDC_SWAPPPPCR_ASEL_ENA(enable);
    putreg32(pfccrreg, LCDC_SWAPCR);
}

/**
 * hal_lcdc_swap_burst_length  - send burst length
 */
void hal_lcdc_swap_burst_length(uint8_t mode)
{
    uint32_t pfccrreg;
    LCDC_INFO("%s mode:%d", __func__, mode);
    pfccrreg = getreg32(LCDC_SWAPCR);
    pfccrreg &= ~LCDC_SWAPPPPCR_BURST_LEN_SHIFT;
    pfccrreg |= LCDC_SWAPPPPCR_BURST_LEN(mode);
    putreg32(pfccrreg, LCDC_SWAPCR);
}


/**
 * hal_lcdc_set_vmirro_enable - enable/disable selected layer vertical mirro function
 * @lid : 0 - lcdc layer1, 1 - lcdc layer2
 */
void hal_lcdc_set_vmirro_enable(uint8_t lid, bool enable)
{
    uint32_t crreg;
    LCDC_INFO("%s lid:%d enable:%d", __func__, lid, enable);
    crreg = getreg32(LCDC_PNIOCR);
    if (lid == 0) {
        crreg &= ~LCD_CFG_DMA_VM_ENA;
    }
    if (lid == 1) {
        crreg &= ~LCD_CFG_GRA_VM_ENA;
    }
    if (enable) {

        if (lid == 0) {
            crreg |= LCD_CFG_DMA_VM_ENA;
        }
        if (lid == 1) {
            crreg |= LCD_CFG_GRA_VM_ENA;
        }
    }
    putreg32(crreg, LCDC_PNIOCR);
}

/**
 * hal_lcdc_disp_color_fmt - set color ouput format for display interface
 * @fmt  : ctrl bit:
 *         DSI_RGB888                (28)
 *         DSI_RGB666                (29)
 *         DSI_RGB565                (30)
 *         DSI_RGB101010             (31)
 *
 *
 */
void hal_lcdc_disp_color_fmt(enum hal_lcdc_dsi_cfmt_e fmt)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DSICFMTCR);
    crreg &= ~LCDC_CFG_DSI_CFMT_MASK;
    crreg |= (1 << fmt);
    putreg32(crreg, LCDC_DSICFMTCR);
}

/**
 * hal_lcdc_disp_dsi_data_swap - set dsi output data byte-order
 * @border  : ctrl fmt
 *            DSI_BD_BGR : 0
 *            DSI_BD_BRG : 1
 *            DSI_BD_GBR : 2
 *            DSI_BD_RBG : 3
 *            DSI_BD_GRB : 4
 *            DSI_BD_RGB : 5
 */
void hal_lcdc_disp_dsi_data_swap(enum hal_lcdc_dsi_dswap_e border)
{
    uint32_t crreg;
    crreg = getreg32(LCDC_DSICFMTCR);
    crreg &= ~LCDC_CFG_DSI_DATA_SWAP_MASK;
    crreg |= LCDC_CFG_DSI_DATA_SWAP(border);
    putreg32(crreg, LCDC_DSICFMTCR);
}

/**
 * hal_lcdc_sl_dmaburst_length_set - set lcdc source path dma read burst length
 * @len: max value is 0xF
 */
void hal_lcdc_sl_dmaburst_length_set(uint8_t len)
{
    uint32_t v;
    v = getreg32(LCDC_SWAPCR);
    v &=~( 0x3 << 10);
    v &=~( 0x3 << 8);
    v |= (len&0xf) << 8;
    putreg32(v, LCDC_SWAPCR);
}

void hal_lcdc_sleep(void)
{
    hal_cmu_lcdc_sleep();
}

void hal_lcdc_wakeup(void)
{
    hal_cmu_lcdc_wakeup();
}

void hal_lcdc_qspi_cs_pin(bool dr_mode, uint32_t val)
{
    if (dr_mode) {
        if (val)
            lcdc->REG_230 |= LCD_CFG_SPI_XCS;
        else
            lcdc->REG_230 &= ~LCD_CFG_SPI_XCS;
        lcdc->REG_230 |= LCD_CFG_SPI_XCS_DR;
    } else {
        lcdc->REG_230 &= ~LCD_CFG_SPI_XCS_DR;
    }
}

static bool send_locked = false;

void hal_lcdc_qspi_irq_handler()
{
    uint32_t irq_stats;

    irq_stats = lcdc->REG_1C4;
    TRACE(0, "irq_stats:0x%x", irq_stats);
    lcdc->REG_1C4 = 0;
    if (irq_stats & LCD_TXC_FRAMEDONE) { //QSPI frame done
        hal_lcdc_qspi_cs_pin(true, 1);
        clear_bool_flag(&send_locked);
    }
}

int hal_lcdc_qspi_send_cmd(uint8_t *buf, uint32_t len)
{
    uint32_t send = 0;
    uint32_t xfer, xfer_bit;
    uint32_t val;

    if (set_bool_flag(&send_locked)) {
        return -1;
    }

    lcdc->REG_1F8 = LCD_SPI_3LINE_MODE;

    hal_lcdc_qspi_cs_pin(true, 0);

    while (send < len) {
        xfer = len - send > 4 ? 4 : len - send;
        xfer_bit = xfer * 8;
        val = 0;
        while (xfer --) {
            val = buf[send++] | (val << 8);
        }
#if 0
        lcdc->REG_1C8 = LCD_SPI_IRQ;
#endif
        lcdc->REG_184 = val;
        hal_sys_timer_delay_us(4);
        lcdc->REG_180 = LCD_CFG_SCLKCNT(2) | LCD_CFG_RXBITS(0) | LCD_CFG_TXBITS(xfer_bit - 1) |
            LCD_CFG_SPI_ENA | LCD_CFG_SPI_START;
#if 0
        while (0) {
            if (lcdc->REG_1C4 & LCD_SPI_IRQ)
                break;
            hal_sys_timer_delay_ns(200);
        }
#else
        hal_sys_timer_delay_us(6);
#endif
    }
    hal_lcdc_qspi_cs_pin(true, 1);
    clear_bool_flag(&send_locked);
    return 0;
}

#define QSPI_FRAME_HEAD_MANUALLY

#ifdef QSPI_FRAME_HEAD_MANUALLY
void hal_lcdc_qspi_send_frame_head()
{
    uint32_t xfer, xfer_bit;

    lcdc->REG_1F8 = LCD_SPI_3LINE_MODE;

    xfer = 4;
    xfer_bit = xfer * 8;
#if 0
    lcdc->REG_1C8 = LCD_SPI_IRQ;
#endif

    lcdc->REG_184 = 0x32002c00;
    hal_sys_timer_delay_us(4);
    lcdc->REG_180 = LCD_CFG_SCLKCNT(2) | LCD_CFG_RXBITS(0) | LCD_CFG_TXBITS(xfer_bit - 1) |
        LCD_CFG_SPI_ENA | LCD_CFG_SPI_START;
#if 0
    while (0) {
        if (lcdc->REG_1C4 & LCD_SPI_IRQ)
            break;
        hal_sys_timer_delay_ns(200);
    }
#else
    hal_sys_timer_delay_us(6);
#endif
}
#endif

void hal_lcdc_qspi_init(uint32_t width, uint32_t higth)
{
    hal_cmu_lcdc_clock_enable(0);

    lcdc->REG_1A8 = LCD_CLK_INT_DIV(1);
    lcdc->REG_118 = LCD_CFG_PN_H_ACTIVE(width) | LCD_CFG_PN_V_ACTIVE(higth);
    lcdc->REG_0FC = LCD_CFG_GRA_PITCH(width * 2) | LCD_CFG_PN_BKLIGHTDIV(0x4);  // RGB567
    lcdc->REG_100 = LCD_CFG_GRA_OVSA_HPXL(0) | LCD_CFG_GRA_OVSA_VLN(0);
    lcdc->REG_104 = LCD_CFG_GRA_H_PIXEL(width) | LCD_CFG_GRA_V_LINE(higth);
    lcdc->REG_108 = LCD_CFG_GRAZM_H_PIXEL(higth) | LCD_CFG_GRAZM_V_LINE(higth);
    lcdc->REG_180 = LCD_CFG_SPI_ENA;
    lcdc->REG_188 = LCD_CFG_SMPN_ENA | LCD_CFG_SMPNMODE(0x5) | LCD_CFG_IORDY_MSK;

    lcdc->REG_22C =
#ifndef QSPI_FRAME_HEAD_MANUALLY
        LCD_CFG_TXDATA_HEAD_24B(0x2c00) | LCD_CFG_SPI_HEAD_TYPE(1) |
#endif
        LCD_CFG_SPI_4LN_565;  //565
    lcdc->REG_210 = LCD_CFG_VSYNC_TRIG_DISABLE_S | LCD_CFG_VSYNC_TRIG_DISABLE_A |
        LCD_CFG_BACKLIGHT_EN | LCD_CFG_REG_GEN_FRAME;
    lcdc->REG_194 = LCD_CFG_PXLMD(0x81);
    lcdc->REG_264 = LCD_CFG_GRA_ENA | LCD_CFG_GRAFORMAT(0); //565

    //enable irq
    lcdc->REG_1C8 = LCD_SPI_IRQ;
    lcdc->REG_1C4 = 0;
    lcdc->REG_1C0 = LCD_TWC_FRAMEDONE_ENA;

    NVIC_SetVector(DISPLAY_IRQn, (uint32_t)hal_lcdc_qspi_irq_handler);
    NVIC_SetPriority(DISPLAY_IRQn, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(DISPLAY_IRQn);
    NVIC_EnableIRQ(DISPLAY_IRQn);
}

int hal_lcdc_qspi_send_data(const uint8_t *layer)
{
    if (set_bool_flag(&send_locked)) {
        return -1;
    }

    hal_lcdc_qspi_cs_pin(true, 0);

#ifdef QSPI_FRAME_HEAD_MANUALLY
    hal_lcdc_qspi_send_frame_head();
#else
    lcdc->REG_22C |= LCD_CFG_USE_FRAME_HEAD;
#endif
    lcdc->REG_1C8 = 0;
    lcdc->REG_1F8 = LCD_SPI_HW_CTRL | LCD_SMPN_SWAP_RB | LCD_SPI_4LN_MODE;
    lcdc->REG_0F4 = LCD_CFG_GRA_SA0((uint32_t)layer);
    lcdc->REG_180 = LCD_CFG_SPI_ENA | LCD_CFG_TXBITS(0xF) | LCD_CFG_SCLKCNT(0x2);
    lcdc->REG_224 = 0;

    //hal_sys_timer_delay(MS_TO_TICKS(100));
    //clear_bool_flag(&send_locked);
    return 0;
}
#endif
