
/***************************************************************************
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

#ifndef __HAL_LCDC_H
#define __HAL_LCDC_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Included Files
 */
#include "plat_types.h"

/**
 * Pre-processor Definitions
 */
#define FAR

#define LCDC_NCLUT           256    /* Number of entries in the CLUT */

#define IRQ_LCDCINT

/**
 * Public Types
 */

/**
 * enum hal_lcdc_layer_e - lcdc hardware pixes color process layers
 * @LCDC_LAYER_LFORE  : spu/pn dma Foreground Layer
 * @LCDC_LAYER_LBACK  : gra Background Layer
 * @LCDC_LAYER_LWB    : Write Back Output Layer
 * @LCDC_LAYER_SPU    : smart panel layer
 */

enum hal_lcdc_layer_e {
    LCDC_LAYER_LFORE = 0,
    LCDC_LAYER_LBACK,
    LCDC_LAYER_LWB,
    LCDC_LAYER_SPU,
};

/**
 * enum hal_lcdc_dsi_cfmt_e - lcdc display out pixes color format
 * @DSI_RGB888    : 28
 * @DSI_RGB666    : 29
 * @DSI_RGB565    : 30
 * @SI_RGB101010  : 31
 */

enum hal_lcdc_dsi_cfmt_e {
    DSI_RGB888 = 28,
    DSI_RGB666,
    DSI_RGB565,
    SI_RGB101010,
};

/**
 * enum hal_lcdc_dsi_dswap_e - dsi output data byte-order.
 * @DSI_BD_BGR : 0
 * @DSI_BD_BRG : 1
 * @DSI_BD_GBR : 2
 * @DSI_BD_RBG : 3
 * @DSI_BD_GRB : 4
 * @DSI_BD_RGB : 5
 */

enum hal_lcdc_dsi_dswap_e {
    DSI_BD_BGR = 0,
    DSI_BD_BRG,
    DSI_BD_GBR,
    DSI_BD_RBG,
    DSI_BD_GRB,
    DSI_BD_RGB
};


/**
 * lcdc hal api
 */

/**
 * hal_lcdc_irqn - get lcdc irq  number
 */

int hal_lcdc_irqn(void);

/**
 * hal_lcdc_write_clut - write clut data into lcdc memory
 * @citem  : clut item
 * @n      : clut item address index
 */
void hal_lcdc_write_clut(uint32_t citem, int n);

/**
 * hal_lcdc_write_gamma - write gamma data into lcdc memory
 * @gitem   : gamma item
 * @n      :  item address index
 */
void hal_lcdc_write_gamma(uint32_t citem, int n);

/**
 * hal_lcdc_lcadjcurves_enable - enable/disalbe  gamma curves function
 */
void hal_lcdc_lcadjcurves_enable(int lid, bool enable);

/**
 * hal_lcdc_axifastmode_enable - enable/disalbe  axi bus fast mode function
 */
void hal_lcdc_axifastmode_enable(bool enable);

/**
 * hal_lcdc_lstartaddr - set lcdc layer dma memory start address
 * @lid    : Layer id
 * @addr   : memory address
 */
int hal_lcdc_lstartaddr(int lid, uint32_t addr);

/**
 * hal_lcdc_getcuraddr - get lcdc layer dma memory start address
 * @lid    : Layer id
 */
uint32_t hal_lcdc_getcuraddr(int lid);

/**
 * hal_lcdc_lpitch - set lcdc vsync start/end pixes count
 * @lid    : Layer id
 * @pitch  : pitch in bytes
 */
void hal_lcdc_lpitch(int lid, uint16_t pitch);

/**
 * hal_lcdc_lvsepxcount - set lcdc vsync start/end pix count
 * @start   : stby length
 * @end     : start line
 */
void hal_lcdc_lvsepxcount(int start, int end);

/**
 * hal_lcdc_stby_vsync_set - set stby vsync
 * @len   : stby length
 * @slen  : start line
 * @elen  : end line
 */
void hal_lcdc_stby_vsync_set(int len, int slen, int elen);

/**
 * hal_lcdc_porch_vsync_set - set vsync porch in pixes
 * @hf      : hFP
 * @hb      : hBP
 * @vlines  : NLINE
 */
void hal_lcdc_porch_vsync_set(int hf, int hb, int vlines);

/**
 * hal_lcdc_pix_vsync_set - set vsync position in pixes
 * @s  : start
 * @e  : end
 */
void hal_lcdc_pix_vsync_set(int s, int e);

/**
 * hal_lcdc_sponscreen_set - set  F/B Layer screen start position in pixes
 * @lid    : Layer id
 * @vln    : y in line
 * @hpixl  : x in pix
 */
void hal_lcdc_sponscreen_set(int lid, uint16_t vln, uint16_t hpixl);

/**
 * hal_lcdc_bgl_vhtotal_set - set  F/B Layer PN_V/H_TOTAL in pix
 * @h    : height total
 * @v    : width total
 */
void hal_lcdc_bgl_vhtotal_set(int h, int v);

/**
 * hal_lcdc_bgl_vtotal_set - set  F/B Layer PN_V_TOTAL in pixes
 * @v  : width total
 */
void hal_lcdc_bgl_vtotal_set(int v);

/**
 * hal_lcdc_bgl_htotal_set - set  F/B Layer PN_H_TOTAL in pixes
 * @h    : height total
 */
void hal_lcdc_bgl_htotal_set(int h);

/**
 * hal_lcdc_bgl_hporch_set - set  F/B Layer HPORCH in pixes
 * @f     : fore porch
 * @b     : back porch
 */
void hal_lcdc_bgl_hporch_set(int f, int b);

/**
 * hal_lcdc_bgl_vporch_set - set  F/B Layer VPORCH in pixes
 *
 * Input Parameters:
 * @f  : fore porch
 * @b  : back porch
 */
void hal_lcdc_bgl_vporch_set(int f, int b);

/**
 * hal_lcdc_lsize - set  Layer image size in pixes
 * @lid   : LCDC_LAYER_SPU only
 * @w     : Layer width
 * @h     : Layer height
 */
void hal_lcdc_lsize(int lid, uint16_t w, uint16_t h);

/**
 * hal_lcdc_lzoom_set - set  F/B Layer zoom in pixes
 * @lid  : Layer id
 * @w    : window active area width
 * @h    : window active area height
 */
void hal_lcdc_lzoom_set(int lid, uint16_t w, uint16_t h);

/**
 * hal_lcdcl_enable - enable/disable lcdc Layer
 * @lid     : Layer index
 * @enable  : enable/disable
 */
void hal_lcdcl_enable(int lid, bool enable);

/**
 * hal_lcdc_lcolor - set  background Layer color
 * @lid    : LCDC_LAYER_SPU only
 * @argb   : color in ARGB888
 */
void hal_lcdc_lcolor(int lid, uint32_t argb);

/**
 * hal_lcdc_dither_s4x8 - enable/disable lcdc  DITHER_4X8_PN
 * @enable   : enable/disable
 */
void hal_lcdc_dither_s4x8(bool enable);

/**
 * hal_lcdc_dither - lcdc  dither configuration
 * @enable   : enable/disable
 * @level    : 0: low  1: high
 * @red      : color width(4,5,6,5)
 * @green    : color width(4,6,6,5)
 * @blue     : color width(4,5,6,5)
 */
void hal_lcdc_dither(bool enable, uint8_t level, uint8_t red,
             uint8_t green, uint8_t blue);

/**
 * hal_lcdc_dither_read - read  lcdc  dither table memory
 * @index   : table memory index
 *
 * Returns: mem value of dither table
 */
uint32_t hal_lcdc_dither_read(uint8_t index);

/**
 * hal_lcdc_dither_write - write  lcdc  dither table memory
 * @index   : table memory index
 * @value   : write value to dither table
 */
void hal_lcdc_dither_write(uint8_t index, uint32_t value);

uint8_t hal_fbcfmt(uint8_t fbcfmt);

/**
 * hal_lcdc_lpfc_fmt - Configure foreground and background color format
 * @lid  : Layer id (output, foreground, background)
 * @fmt  : FB_FMT_XXX
 */
void hal_lcdc_lpfc_fmt(int lid, uint8_t fmt);

void hal_lcdc_lchromakeyenable(int lid, bool enable);

/**
 * hal_lcdc_lpfc_ltransp - Configure fore and back ground blend function
 * @lid     : Layer id (output, foreground, background)
 * @mode    : FB blend mode: FB_XXX_ALPHA
 * @alpha   : Layer alpha
 *
 */
void hal_lcdc_lpfc_ltransp(int lid, uint32_t mode, uint8_t alpha);

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
void hal_lcdc_lchromakey(int lid, uint32_t rgb, uint32_t rrgb, uint8_t ra);

/**
 * hal_lcdc_lchromakey_newalpha - foreground and background color key Configure
 * @lid    : Layer id
 * @alpha  : new alpha for replaced pixes
 *
 * set foreground and background color key Configure, real color alpha will be
 * replaced , if it's color value is equal to the chroma key
 *
 */
void hal_lcdc_lchromakey_newalpha(int lid, uint8_t newalpha);

/**
 * hal_lcdc_panelpath_dmatrans_trigger_enable - enable/disable lcdc panel dma trigger
 * @lid    : Layer id
 * @ena    : enable/disable
 */
void hal_lcdc_panelpath_dmatrans_trigger_enable(int lid, bool ena);

/**
 * hal_lcdc_disp_avsync_trigger_disable - enable/disable lcdc disp avsync trigger
 * @dis     : enable/disable
 */
void hal_lcdc_disp_avsync_trigger_disable(bool dis);

/**
 * hal_lcdc_disp_svsync_trigger_disable - enable/disable lcdc disp svsync trigger
 * @dis   : enable/disable
 */
void hal_lcdc_disp_svsync_trigger_disable(bool dis);

/**
 * hal_lcdc_disp_mem_enable - enable/disable lcdc disp memory
 * @dis  : enable/disable
 */
void hal_lcdc_disp_mem_enable(bool ena);

void hal_lcdc_disp_extra_delay(uint16_t delay);

/**
 * hal_lcdc_ol_dmaburst_length_set - set lcdc wb path dma burst length
 * @len     :  burst length
 */
void hal_lcdc_ol_dmaburst_length_set(uint8_t len);

/**
 * hal_lcdc_smpn_mode_set - set lcdc smpn mode
 * @mode : smpn mode (0x0 = 3 cycles per pixel;
 *                    0x1 = RGB666, 3 cycles per pixel
 *                    0x2 = RGB565, 2 cycles per pixel;
 *                    0x3 = 1 cycle per pixel;
 *                    0x4 = RGB666, 1 cycle per pixel;
 *                    0x5 = RGB565 1 cycle per pixel )
 */
void hal_lcdc_smpn_mode_set(int mode);

/**
 * hal_lcdc_irq_disable - mask all lcdc_irqs
 */

int hal_lcdc_irq_disable(void);

uint32_t hal_lcdc_get_irqstat(void);
/**
 * hal_lcd_framedone_irq_enable - enable/disable spu frame done irq.
 * @ena    : ture/false
 *
 */
int hal_lcdc_framedone_irq_enable(bool ena);

/**
 * hal_lcdc_framedone_irqstat_clr  - clear spu frame done irq status.
 * @ena   : ture/false
 *
 */
int hal_lcdc_framedone_irqstat_clr(void);
int hal_lcdc_vsync_irqstat_clr(void);

/**
 * hal_lcdc_irq_is_framedone - lcdc framedone irq status.
 * @regval   : lcdc ISR value
 */
bool hal_lcdc_irq_is_framedone(uint32_t regval);
bool hal_lcdc_irq_is_framedone_irq0(uint32_t regval);
bool hal_lcdc_irq_is_framedone_irq1(uint32_t regval);
bool hal_lcdc_irq_is_vsync(uint32_t regval);

/**
 * hal_lcdc_irq_is_error - Get whether error irq status is ture or not
 * @regval  : lcdc ISR value
 *
 * Returns:  TRUE on erro irq, FALSE on without erro irq.
 */
bool hal_lcdc_irq_is_error(uint32_t regval);
/**
 * hal_lcdc_error_irq_enable - enable  all lcdc_error irqs
 */
int hal_lcdc_error_irq_enable(void);

/**
 * hal_lcdc_error_irqstat_clr - enable  all lcdc_error irqs
 */
int hal_lcdc_error_irqstat_clr(void);
/**
 * hal_cmu_disp_pn_reset   - reset display pannel
 */
void hal_lcdc_disp_pn_reset(void);
/**
 * hal_lcdc_reset - reset lcd configuration
 */
void hal_lcdc_reset(void);

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable.
 */
void hal_lcdc_smpn_enable(void);

/**
 * hal_lcdc_smpn_vsync_delaymode_set - set lcdc smpn module vsync delay.
 */
void hal_lcdc_smpn_vsync_delaymode_set(uint8_t delay);

/**
 * hal_lcdc_smpn_vsync_delaymode_get - get lcdc smpn module vsync delay.
 */
uint32_t hal_lcdc_smpn_vsync_delaymode_get(void);

/**
 * hal_lcdc_smpn_enable - lcdc smpn module enable/disable.
 */
void hal_lcdc_gen_frame_enable(bool ena);

/**
 * hal_lcdc_set_sync_line_pixes - set lcdc sync pixs.
 */
void hal_lcdc_set_sync_line_pixs(uint32_t pixs);

/**
 * hal_lcdc_set_sync_lines  - set lcdc sync lines.
 */
void hal_lcdc_set_sync_lines(uint32_t lines);

/**
 * hal_lcdc_get_sync_delay_pixs - set lcdc delay pixes.
 */
uint32_t hal_lcdc_get_sync_delay_pixs(void);

/**
 * hal_lcdc_get_sync_delay_lines - get lcdc delay lines.
 */
uint32_t hal_lcdc_get_sync_delay_lines(void);
/**
 * hal_lcdc_start_wb  - Starts the wblayer dma2d transfer process
 */

int hal_lcdc_start_wb(void);

/**
 * hal_lcdc_gen_start  - Starts the  dma2d transfer process by write register
 */
int hal_lcdc_gen_start(void);

/**
 * hal_lcdc_start_dsi - Starts the lcdc transfer process.
 */

int hal_lcdc_start_dsi(void);
void hal_lcdc_te_trigger();

/**
 * hal_lcdc_vsync_porch_set  - set vsync porch with default value
 */
void hal_lcdc_vsync_porch_set(void);
/**
 * hal_lcdc_wdma_1fpt_set - set write back dma in one frame per trigger mode
 */
void hal_lcdc_wdma_1fpt_set(void);

/**
 * hal_lcdc_wdma_toggle_frame - toggle signal to start a write back dma frame
 */
void hal_lcdc_wdma_toggle_frame(void);

/**
 * hal_lcdc_select_top - select f/b dma path  as top layer to blender
 */
void hal_lcdc_select_top(uint8_t lid);

/**
 * hal_lcdc_swap_path - select f/b dma path
 */
void hal_lcdc_swap_path(uint8_t lid);

/**
 * hal_lcdc_swap_path_enable  - enable selected f/b dma path
 */
void hal_lcdc_swap_path_enable(bool enable);

/**
 * hal_lcdc_swap_burst_length  - send burst length
 */
void hal_lcdc_swap_burst_length(uint8_t mode);

/**
 * hal_lcdc_set_vmirro_enable - enable/disable selected layer vertical mirro function
 * @lid : 0 - lcdc layer1, 1 - lcdc layer2
 */
void hal_lcdc_set_vmirro_enable(uint8_t lid, bool enable);

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
void hal_lcdc_disp_color_fmt(enum hal_lcdc_dsi_cfmt_e fmt);

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
void hal_lcdc_disp_dsi_data_swap(enum hal_lcdc_dsi_dswap_e border);

/**
 *  hal_lcdc_axibus_arbiter_fastmode_enable - AXI Bus Arbiter Fast Mode Enable
 */
void hal_lcdc_axibus_arbiter_fastmode_enable(void);

/**
 * hal_lcdc_sl_dmaburst_length_set - set lcdc source path dma read burst length
 */
void hal_lcdc_sl_dmaburst_length_set(uint8_t len);

void hal_lcdc_sleep(void);

void hal_lcdc_wakeup(void);

void hal_lcdc_qspi_init(uint32_t width, uint32_t higth);

int hal_lcdc_qspi_send_cmd(uint8_t *buf, uint32_t len);

int hal_lcdc_qspi_send_data(const uint8_t *layer);
//video mode
void hal_lcdcl_dumb_enable( bool enable);
void hal_lcdc_dumb_vsync_mode(uint8_t mode);
void hal_lcdc_source_switch(bool isdumb);
//test mode
void hal_lcdc_test_mode(bool enable);

#ifdef __cplusplus
}
#endif
#endif /* __HAL_LCDC_H */
