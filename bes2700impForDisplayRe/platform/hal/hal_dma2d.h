
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
 * **************************************************************************/

/**
 * DOC:DMA2D is a graphic accelerator which offers advanced bit blitting, row data copy
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
#ifndef __HAL_DMA2D_H
#define __HAL_DMA2D_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Included Files
 */
#include "plat_types.h"

#define DMA2D_NCLUT           256    /* Number of entries in the CLUT */

/* DMA2D PFC alpha mode */
#define DMA2D_PFCCR_AM_FLPIXEL   0
#define DMA2D_PFCCR_AM_BLPIXEL   1
#define DMA2D_PFCCR_AM_CONST     2
#define DMA2D_PFCCR_AM_NO        3



#define DMA2D_ACTION_TYPE_FILL 0
#define DMA2D_ACTION_TYPE_BLIT 1
#define DMA2D_ACTION_TYPE_BLEND 2


/**
 * Pre-processor Definitions
 */

#define DMA2D_NLAYERS            4

/**
 * Public Types
 */

/**
 * enum hal_dma2d_layer_e - DMA2D color process layers.
 * @DMA2D_LAYER_LFORE : dma2d panel layer 0 with YUV color fmt support
 * @DMA2D_LAYER_LBACK : dma2d panel layer 1 without YUV color fmt support
 * @DMA2D_LAYER_LOUT  : dma2d write back layer
 * @DMA2D_LAYER_SPU   : internel used only
 *
 * This enumeration define 4 layers supported by the dma2d controller
 */

enum hal_dma2d_layer_e {
    DMA2D_LAYER_LFORE = 0,    /* spu/pn dma Foreground Layer */
    DMA2D_LAYER_LBACK,    /* gra Background Layer */
    DMA2D_LAYER_LOUT,    /* Write Back Output Layer */
    DMA2D_LAYER_SPU,    /* Smart panel Layer */
};

/*
 * dma2d hal api
 */

/**
 * hal_dma2d_irqn - get  dma2d irq  number
 */

int hal_dma2d_irqn(void);

/**
 * hal_dma2d_get_irqstat - get dma2d irq status.
 */
uint32_t hal_dma2d_get_irqstat(void);

/**
 * hal_dma2d_spu_framedone_irq_enable - enable/disable spu frame done irq.
 */
int hal_dma2d_spu_framedone_irq_enable(bool ena);

/**
 * hal_dma2d_spu_framedone_irqstat_clr - clear spu frame done irq.
 */
int hal_dma2d_spu_framedone_irqstat_clr(void);

/**
 * hal_dma2d_irq_is_framedone - Reture whether framedone irq has come or not
 *
 * Returns:   TRUE on irq has come, FALSE on irq not come.
 */
bool hal_dma2d_irq_is_framedone(uint32_t regval);

/**
 * hal_dma2d_irq_disable - mask all dma2d_irqs
 *
 */

int hal_dma2d_irq_disable(void);

/**
 * hal_dma2d_irq_is_error - Get whether dma2d error irq status is ture or not.
 * @regval : dma2d ISR value
 *
 * Returns:  TRUE on erro irq, FALSE on without erro irq.
 */
bool hal_dma2d_irq_is_error(uint32_t regval);

/**
 * hal_dma2d_error_irq_enable - enable  all dma2d_error irqs
 */
int hal_dma2d_error_irq_enable(void);

/**
 * hal_dma2d_underflow_irq_enable - enable  dma2d_underflow irqs
 */
int hal_dma2d_underflow_irq_enable(void);

/**
 * hal_dma2d_error_irq_clr - clear all dma2d_error irqs
 */
int hal_dma2d_error_irqstat_clr(void);

/**
 * hal_dma2d_start - Starts the dma transfer process.
 */

int hal_dma2d_start(void);

/**
 * hal_dma2d_reset - reset dma2d configuration
 */
void hal_dma2d_reset(void);

/**
 * hal_dma2d_panelpath_dmatrans_trigger_enable - enable/disable dma2d panel dma trigger
 *@lid  : Layer id
 *@ena  : enable/disable
 */
void hal_dma2d_panelpath_dmatrans_trigger_enable(int lid, bool ena);

/**
 * hal_dma2d_disp_avsync_trigger_disable - enable/disable dma2d disp avsync trigger
 * @dis  : disable/enable
 */
void hal_dma2d_disp_avsync_trigger_disable(bool dis);

/**
 * hal_dma2d_disp_svsync_trigger_disable - enable/disable dma2d disp svsync trigger
 * @dis  disable/enable
 */
void hal_dma2d_disp_svsync_trigger_disable(bool dis);

/**
 * hal_dma2d_disp_mem_enable - enable/disable dma2d disp memory
 * @dis     - enable/disable
 */
void hal_dma2d_disp_mem_enable(bool ena);

/**
 * hal_dma2d_lcolor - fill the color for the layer
 * @lid  - Layer index (output, foreground, background, spu )
 * @argb - argb8888 color
 *
 */

void hal_dma2d_lcolor(int lid, uint32_t argb);

/**
 * hal_dma2d_lpfc_swaprb - enable RB byte order swap
 * @lid  - Layer index (output, foreground, background, spu )
 * @ena  - swap or not
 */
void hal_dma2d_lpfc_swaprb(int lid, bool ena);

void hal_dma2d_lpfc_reset(void);

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

void hal_dma2d_lpfc(int lid, uint32_t blendmode, uint8_t alpha, uint8_t fmt);

/**
 * hal_dma2d_write_clut - write clut data into dma2d memory
 * @citem  : clut item
 * @n      : clut item address index
 */
void hal_dma2d_write_clut(uint32_t citem, int n);

/**
 * hal_dma2d_lstartaddr - set dma2d layer dma memory start address
 * @lid   : Layer id
 * @addr  : memory address
 */
int hal_dma2d_lstartaddr(int lid, uint32_t addr);

/**
 * hal_dma2d_lpitch - set dma2d vsync start/end pixes count
 * @lid     : Layer id
 * @pitch   : pitch in bytes
 */
void hal_dma2d_lpitch(int lid, uint16_t pitch);

/**
 * hal_dma2dl_enable  - enable/disable dma2d Layer
 * @lid     : Layer index
 * @enable  : enable/disable
 */
void hal_dma2dl_enable(int lid, bool enable);

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
void hal_dma2d_smpn_mode_set(int mode);

/**
 * hal_dma2d_lsize - set  Layer image size in pixes
 * @lid  : Layer id
 * @w    : Layer width
 * @h    : Layer height
 */
void hal_dma2d_lsize(int lid, uint16_t w, uint16_t h);

/**
 * hal_dma2d_sponscreen_set - set F/B Layer screen start position in pix
 * @lid   : Layer id
 * @vln   : y in line
 * @hpixl : x in pix
 */
void hal_dma2d_sponscreen_set(int lid, uint16_t vln, uint16_t hpixl);

/**
 * Name: hal_dma2d_lzoom_set - set  F/B Layer zoom size in pix
 * @lid    : Layer id
 * @w      : window active area width
 * @h      : window active area height
 */
void hal_dma2d_lzoom_set(int lid, uint16_t w, uint16_t h);

void hal_dma2d_swap_path(uint8_t lid);

void hal_dma2d_swap_path_enable(bool enable);

/**
 * hal_dma2d_lblend - select  F/B Layer blend first layer
 * @first  : Layer id
 */
void hal_dma2d_lblend(uint8_t first);

/**
 * hal_dma2d_lblend_select - select  F/B Layer blend first and second layer
 * @first   : Layer id
 * @second  : Layer id
 */
void hal_dma2d_lblend_select(uint8_t first, uint8_t second);

/**
 * hal_dma2d_vsync_porch_set - set vsync porch with act,image width and mem types
 * @action    : 0/fill, 1/blit, 2/blend
 * @lwidth    : layer width
 * @flayers   : fast layer number
 * @slayers   : slow layer number
 */
void hal_dma2d_vsync_porch_set(uint8_t action, uint32_t lwidth, uint8_t flayers2d, uint8_t slayers2d);

/**
 * hal_dma2d_wdma_1fpt_set - set write back dma in one frame per trigger mode
 */
void hal_dma2d_wdma_1fpt_set(void);

/**
 * hal_dma2d_bgl_vhtotal_set - set  F/B Layer PN_V/H_TOTAL in pix
 * @h : height total
 * @v : width total
 */
void hal_dma2d_bgl_vhtotal_set(int h, int v);

/**
 * hal_dma2d_bgl_hporch_set - set  F/B Layer HPORCH in pixes
 * @f : fore porch
 * @b : back porch
 */
void hal_dma2d_bgl_hporch_set(int f, int b);

/**
 * hal_dma2d_bgl_vporch_set - set  F/B Layer VPORCH in pixes
 * @f  : fore porch
 * @b  : back porch
 */
void hal_dma2d_bgl_vporch_set(int f, int b);

/**
 * hal_dma2d_sl_dmaburst_maxlength_set - set dma2d source path dma read burst length
 */
void hal_dma2d_sl_dmaburst_maxlength_set(void);

/**
 * hal_dma2d_sl_dmaburst_length_set - set dma2d source path dma read burst length
 */
void hal_dma2d_sl_dmaburst_length_set(uint8_t len);

/**
 * hal_dma2d_ol_dmaburst_length_set - set dma2d wb path dma burst length
 * @len   :  burst length
 */
void hal_dma2d_ol_dmaburst_length_set(uint8_t len);

/**
 * hal_dma2d_dither_s4x8 - enable/disable dma2d DITHER_4X8_PN
 * @enable     - enable/disable
 */
void hal_dma2d_dither_s4x8(bool enable);

/**
 * hal_dma2d_dither - dma2d  dither configuration
 * @enable  : enable/disable
 * @lelvel  : 0:low 1:high
 * @red     : color width(4,5,6)
 * @green   : color width(4,6,6)
 * @blue    : color width(4,5,6)
 */
void hal_dma2d_dither(bool enable, uint8_t level, uint8_t red,
              uint8_t green, uint8_t blue);

/**
 * hal_dma2d_dither_read - read  dma2d  dither table memory
 * @index : table memory index
 *
 * Returns:  mem value of dither table
 */
uint32_t hal_dma2d_dither_read(uint8_t index);

/**
 * hal_dma2d_dither_write - write  dma2d  dither table memory
 * @index  : table memory index
 * @value  : write value to dither table
 */
void hal_dma2d_dither_write(uint8_t index, uint32_t value);

/**
 *  hal_dma2d_axibus_arbiter_fastmode_enable - AXI Bus Arbiter Fast Mode Enable
 */
void hal_dma2d_axibus_arbiter_fastmode_enable(void);

/**
 * hal_dma2d_pnlayer_clock_enable  - enable clock for dma2d PNLayer
 */
void hal_dma2d_pnlayer_clock_enable(void);

/**
 * hal_dma2d_tvlayer_clock_enable  - enable clock for dma2d TVLayer
 */
void hal_dma2d_tvlayer_clock_enable(void);

/**
 * hal_dma2d_pnlayer_clock_disable  - disable clock for dma2d PNLayer
 */
void hal_dma2d_pnlayer_clock_disable(void);

/**
 * hal_dma2d_tvlayer_clock_disable  - disable clock for dma2d TVLayer
 */
void hal_dma2d_tvlayer_clock_disable(void);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_DMA2D_H */
