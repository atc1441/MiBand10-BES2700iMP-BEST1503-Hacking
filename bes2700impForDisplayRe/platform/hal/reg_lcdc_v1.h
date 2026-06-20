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

#ifndef __LCDC_REGV1_DEFINE_H
#define __LCDC_REGV1_DEFINE_H

#include "plat_types.h"
#include "plat_addr_map.h"

#define LCDC_REG_BASE             LCDC_BASE

#define lcdc_tvd_start_addr_y0      (LCDC_REG_BASE + (0x000))
#define lcdc_tvd_start_addr_u0      (LCDC_REG_BASE + (0x004))
#define lcdc_tvd_start_addr_v0      (LCDC_REG_BASE + (0x008))
#define cfg_tvd_y_squln_sa          (LCDC_REG_BASE + (0x010))
#define cfg_tvd_u_squln_sa          (LCDC_REG_BASE + (0x014))
#define cfg_tvd_v_squln_sa          (LCDC_REG_BASE + (0x018))
#define lcdc_tvd_pitch_y          (LCDC_REG_BASE + (0x020))
#define lcdc_tvd_pitch_uv          (LCDC_REG_BASE + (0x024))
#define lcdc_tvd_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x028))
#define lcdc_tvd_hpxl_vln          (LCDC_REG_BASE + (0x02C))
#define lcdc_tvdzm_hpxl_vln          (LCDC_REG_BASE + (0x030))
/* tvg graphic layer start address register */
#define lcdc_tvg_start_addr0      (LCDC_REG_BASE + (0x034))
#define cfg_tvg_squln_sa          (LCDC_REG_BASE + (0x038))
#define lcdc_tvg_pitch              (LCDC_REG_BASE + (0x03C))
#define lcdc_tvg_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x040))
#define lcdc_tvg_hpxl_vln          (LCDC_REG_BASE + (0x044))
#define lcdc_tvgzm_hpxl_vln          (LCDC_REG_BASE + (0x048))
#define lcdc_tvc_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x04C))
#define lcdc_tvc_hpxl_vln          (LCDC_REG_BASE + (0x050))
#define lcdc_tv_v_h_total          (LCDC_REG_BASE + (0x054))
#define lcdc_tv_v_h_active          (LCDC_REG_BASE + (0x058))
#define lcdc_tv_h_porch              (LCDC_REG_BASE + (0x05C))
#define lcdc_tv_v_porch              (LCDC_REG_BASE + (0x060))
#define lcdc_tv_blankcolor          (LCDC_REG_BASE + (0x064))
#define lcdc_tv_alpha_color1      (LCDC_REG_BASE + (0x068))
#define lcdc_tv_alpha_color2      (LCDC_REG_BASE + (0x06C))
#define lcdc_tv_colorkey_y          (LCDC_REG_BASE + (0x070))
#define lcdc_tv_colorkey_u          (LCDC_REG_BASE + (0x074))
#define lcdc_tv_colorkey_v          (LCDC_REG_BASE + (0x078))
#define lcdc_tv_sepxlcnt          (LCDC_REG_BASE + (0x07C))
#define lcdc_tv_ctrl0              (LCDC_REG_BASE + (0x080))
#define lcdc_tv_ctrl1              (LCDC_REG_BASE + (0x084))
#define lcdc_tv_contrast          (LCDC_REG_BASE + (0x088))
#define lcdc_tv_saturation          (LCDC_REG_BASE + (0x08C))
#define lcdc_tv_cbsh_hue          (LCDC_REG_BASE + (0x090))
#define lcdc_tvif_ctrl              (LCDC_REG_BASE + (0x094))
#define lcdc_tviopad_ctrl          (LCDC_REG_BASE + (0x098))
#define lcdc_tclk_div              (LCDC_REG_BASE + (0x09C))

/* dma graphic layer start address register */
#define lcdc_dma_start_addr_y0      (LCDC_REG_BASE + (0x0C0))
#define lcdc_dma_start_addr_u0      (LCDC_REG_BASE + (0x0C4))
#define lcdc_dma_start_addr_v0      (LCDC_REG_BASE + (0x0C8))
#define lcdc_dma_start_addr_c0      (LCDC_REG_BASE + (0x0CC))

#define cfg_dma_y_squln_sa          (LCDC_REG_BASE + (0x0D0))
#define cfg_dma_u_squln_sa          (LCDC_REG_BASE + (0x0D4))
#define cfg_dma_v_squln_sa          (LCDC_REG_BASE + (0x0D8))
#define cfg_dma_c_squln_sa          (LCDC_REG_BASE + (0x0DC))

#define lcdc_dma_pitch_y          (LCDC_REG_BASE + (0x0E0))
#define lcdc_dma_pitch_uv          (LCDC_REG_BASE + (0x0E4))
#define lcdc_dma_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x0E8))
#define lcdc_dma_hpxl_vln          (LCDC_REG_BASE + (0x0EC))
#define lcdc_dmazm_hpxl_vln          (LCDC_REG_BASE + (0x0F0))

/* gra graphic layer start address register */
#define lcdc_gra_start_addr0      (LCDC_REG_BASE + (0x0F4))
#define cfg_gra_squln_sa          (LCDC_REG_BASE + (0x0F8))
#define lcdc_gra_pitch              (LCDC_REG_BASE + (0x0FC))
#define lcdc_gra_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x100))
#define lcdc_gra_hpxl_vln          (LCDC_REG_BASE + (0x104))
#define lcdc_grazm_hpxl_vln       (LCDC_REG_BASE + (0x108))
#define lcdc_hwc_ovsa_hpxl_vln      (LCDC_REG_BASE + (0x10C))
#define lcdc_hwc_hpxl_vln          (LCDC_REG_BASE + (0x110))
#define lcdc_pn_v_h_total          (LCDC_REG_BASE + (0x114))
#define lcdc_pn_v_h_active          (LCDC_REG_BASE + (0x118))
#define lcdc_pn_h_porch              (LCDC_REG_BASE + (0x11C))
#define lcdc_pn_v_porch              (LCDC_REG_BASE + (0x120))
#define lcdc_pn_blankcolor          (LCDC_REG_BASE + (0x124))
#define lcdc_pn_alpha_color1      (LCDC_REG_BASE + (0x128))
#define lcdc_pn_alpha_color2      (LCDC_REG_BASE + (0x12C))
#define lcdc_pn_colorkey_y          (LCDC_REG_BASE + (0x130))
#define lcdc_pn_colorkey_u          (LCDC_REG_BASE + (0x134))
#define lcdc_pn_colorkey_v          (LCDC_REG_BASE + (0x138))
#define lcdc_pn_sepxlcnt          (LCDC_REG_BASE + (0x13C))
#define lcdc_dmavld_y              (LCDC_REG_BASE + (0x14C))
#define lcdc_dmavld_uv              (LCDC_REG_BASE + (0x150))
#define lcdc_tvggravld_hlen          (LCDC_REG_BASE + (0x154))
#define lcdc_hwc_rddat              (LCDC_REG_BASE + (0x158))
#define lcdc_pn_gamma_rddat          (LCDC_REG_BASE + (0x15C))
#define lcdc_pn_clut_rddat          (LCDC_REG_BASE + (0x160))
#define lcdc_tvdvld_y              (LCDC_REG_BASE + (0x168))
#define lcdc_tvdvld_uv              (LCDC_REG_BASE + (0x16C))
#define lcdc_tvc_rddat              (LCDC_REG_BASE + (0x170))
#define lcdc_tv_gamma_rddat          (LCDC_REG_BASE + (0x174))
#define lcdc_tv_clut_rddat          (LCDC_REG_BASE + (0x178))
#define lcdc_frame_cnt              (LCDC_REG_BASE + (0x17C))
#define lcdc_smpn_ctrl              (LCDC_REG_BASE + (0x188))
#define lcdc_pn_ctrl0              (LCDC_REG_BASE + (0x190))
#define lcdc_pn_ctrl1              (LCDC_REG_BASE + (0x194))
#define lcdc_sram_ctrl              (LCDC_REG_BASE + (0x198))
#define lcdc_sram_wrdat              (LCDC_REG_BASE + (0x19C))
#define lcdc_sram_para0              (LCDC_REG_BASE + (0x1A0))
#define lcdc_sram_para1              (LCDC_REG_BASE + (0x1A4))
#define lcdc_sclk_div              (LCDC_REG_BASE + (0x1A8))
#define lcdc_pn_contrast          (LCDC_REG_BASE + (0x1AC))
#define lcdc_pn_saturation          (LCDC_REG_BASE + (0x1B0))
#define lcdc_pn_cbsh_hue          (LCDC_REG_BASE + (0x1B4))
#define lcdc_dumb_ctrl              (LCDC_REG_BASE + (0x1B8))
#define lcdc_pnio_ctrl              (LCDC_REG_BASE + (0x1BC))
#define spu_irq_ena_tb              (LCDC_REG_BASE + (0x1C0))
#define spu_irq_isr                  (LCDC_REG_BASE + (0x1C4))
#define spu_irq_rsr                  (LCDC_REG_BASE + (0x1C8))
#define lcdc_gra_cuthpxl          (LCDC_REG_BASE + (0x1CC))
#define lcdc_gra_cutvln              (LCDC_REG_BASE + (0x1D0))
#define lcdc_tvg_cuthpxl          (LCDC_REG_BASE + (0x1D4))
#define lcdc_tvg_cutvln              (LCDC_REG_BASE + (0x1D8))
#define lcdc_top_ctrl              (LCDC_REG_BASE + (0x1DC))
#define lcdc_pn_squln_ctrl          (LCDC_REG_BASE + (0x1E0))
#define lcdc_tv_squln_ctrl          (LCDC_REG_BASE + (0x1E4))
#define lcdc_afa_all2one_ctrl      (LCDC_REG_BASE + (0x1E8))
#define lcdc_dither_ctrl          (LCDC_REG_BASE + (0x1EC))
#define lcdc_dither_tbl_data      (LCDC_REG_BASE + (0x1F0))
#define lcdc_misc_ctrl              (LCDC_REG_BASE + (0x1F8))
#define lcdc_wdma_ctrl              (LCDC_REG_BASE + (0x200))
#define lcdc_wdma_v_h              (LCDC_REG_BASE + (0x204))
#define lcdc_wdma_start_addr      (LCDC_REG_BASE + (0x208))
#define lcdc_tv_v_blank              (LCDC_REG_BASE + (0x20C))
#define lcdc_tv_v_h_total_fld      (LCDC_REG_BASE + (0x210))
#define lcdc_tv_v_porch_fld          (LCDC_REG_BASE + (0x214))
#define lcdc_tv_sepxlcnt_fld      (LCDC_REG_BASE + (0x218))
#define cfg_h_front_porch_smpn      (LCDC_REG_BASE + (0x21C))
#define cfg_vsync_start_pix          (LCDC_REG_BASE + (0x220))
#define lcdc_gen_frame_start      (LCDC_REG_BASE + (0x224))
#define lcdc_dsi_color_fmt         (LCDC_REG_BASE + (0x228))
#define vdma_squ_w_burst_cnt      (LCDC_REG_BASE + (0x22C))
#define vdma_squ_line_rdy          (LCDC_REG_BASE + (0x230))
#define lcdc_version              (LCDC_REG_BASE + (0x240))
#define lcdc_pn_cmap_cfgcr        (LCDC_REG_BASE + (0x24c))

#define lcdc_te_ctrl              (LCDC_REG_BASE + (0x254))
#define lcdc_te_trriger           (LCDC_REG_BASE + (0x258))

#define lcdc_tvd_ctrl             (LCDC_REG_BASE + (0x260))
#define lcdc_pn_gra_ctrl          (LCDC_REG_BASE + (0x264))

#define lcdc_vdma_sel_ctrl          (LCDC_REG_BASE + (0x2F0))
#define lcdc_vdma_burst_cnt_rdy      (LCDC_REG_BASE + (0x2F4))
#define timing_master_control      (LCDC_REG_BASE + (0x2F8))
#define dsi_in_mux_ctl              (LCDC_REG_BASE + (0x2FC))
#define lcdc_crc1_ctrl              (LCDC_REG_BASE + (0x380))
#define lcdc_crc1_blk_size          (LCDC_REG_BASE + (0x384))
#define lcdc_crc1_intc_treshld      (LCDC_REG_BASE + (0x388))
#define lcdc_crc1_sram_power_ctrl (LCDC_REG_BASE + (0x38C))
#define lcdc_crc2_ctrl              (LCDC_REG_BASE + (0x3C0))
#define lcdc_crc2_blk_size          (LCDC_REG_BASE + (0x3C4))
#define lcdc_crc2_intc_treshld      (LCDC_REG_BASE + (0x3C8))
#define lcdc_crc2_sram_power_ctrl (LCDC_REG_BASE + (0x3CC))

/* LCDC Register Addresses */

/*LCDC Start Register Address*/
#define LCDC_STARTCR            (lcdc_gen_frame_start)
#define LCDC_PANEL_SCLKCR       (lcdc_sclk_div)

/*LCDC Swap Panel Path Processing Register Address */
#define LCDC_SWAPCR             (lcdc_top_ctrl)

/*LCDC Blend Register Address*/
#define LCDC_BLDCR              (lcdc_afa_all2one_ctrl)

/*Layer Size Register Address*/
#define LCDC_PDMAAWCR           (lcdc_dma_hpxl_vln)
#define LCDC_PGRAAWCR           (lcdc_gra_hpxl_vln)
#define LCDC_SPUAWCR            (lcdc_pn_v_h_active)
#define LCDC_WBAWCR             (lcdc_wdma_v_h)

/*F/B Layer VH Register Address*/
#define LCDC_PVH_TOTAL_CR        (lcdc_pn_v_h_total)

/*F/B Layer HPorchRegister Address*/
#define LCDC_PHPORCH_CR             (lcdc_pn_h_porch)

/*F/B Layer VPorchRegister Address*/
#define LCDC_PVPORCH_CR             (lcdc_pn_v_porch)

/*Layer pitch Register Address*/
#define LCDC_PDMAPCHCR           (lcdc_dma_pitch_y)
#define LCDC_PGRAPCHCR           (lcdc_gra_pitch)
#define LCDC_SPUPCHCR            (NULL)
#define LCDC_WBPCHCR             (lcdc_wdma_ctrl)

/*OUT Layer dma burst Register Address*/
#define LCDC_WBBURSTCR             (lcdc_wdma_ctrl)

/*Layer pfc Register Address*/
#define LCDC_FGPFCCR         (lcdc_pn_ctrl0)
#define LCDC_FGPFCCR1        (lcdc_pn_ctrl1)
#define LCDC_BGPFCCR         (lcdc_pn_gra_ctrl)
#define LCDC_OPFCCR          (lcdc_wdma_ctrl)

/*F/B Layer dma zoom Register Address*/
#define LCDC_DMAZOOMCR       (lcdc_dmazm_hpxl_vln)
#define LCDC_GRAZOOMCR       (lcdc_grazm_hpxl_vln)

/*Layer Start mem adress Register Address*/
#define LCDC_FGMAR           (lcdc_dma_start_addr_y0)
#define LCDC_BGMAR           (lcdc_gra_start_addr0)
#define LCDC_OMAR            (lcdc_wdma_start_addr)

/*Layer Color Register Address*/
#define LCDC_FGCOLR          (NULL)
#define LCDC_BGCOLR          (NULL)
#define LCDC_OCOLR           (NULL)
#define LCDC_SPUCOLR         (lcdc_pn_blankcolor)

/*Layer F/B  start/end  Pix count Register Address*/
#define LCDC_SEPXCNTCR         (lcdc_pn_sepxlcnt)

/*LCDC irq control Register Address*/
#define LCDC_IER              (spu_irq_ena_tb)
#define LCDC_ISR              (spu_irq_isr)
#define LCDC_ICR              (spu_irq_rsr)

#define LCDC_DISPCNFCR        (lcdc_tv_v_h_total_fld)

#define LCDC_SMPNCR           (lcdc_smpn_ctrl)

/*frame per toggle*/
#define LCDC_WBFPTCR             (lcdc_wdma_v_h)

#define LCDC_HSYNCLENCR        (lcdc_tv_v_porch_fld)
#define LCDC_STBY_VSYNCCR      (lcdc_tv_sepxlcnt_fld)
#define LCDC_PORCH_VSYNCCR     (cfg_h_front_porch_smpn)
#define LCDC_VSYNC_PIXCR       (cfg_vsync_start_pix)

#define LCDC_TECR              (lcdc_te_ctrl)
#define LCDC_TETRRIGERCR              (lcdc_te_trriger)

#define LCDC_CFG_PDMA_Y_SQULN_START     (lcdc_dma_ovsa_hpxl_vln)
#define LCDC_CFG_PDMA_U_SQULN_START     (lcdc_hwc_ovsa_hpxl_vln)

#define LCDC_CFG_PDMA_SPONSCREENCR     (lcdc_dma_ovsa_hpxl_vln)
#define LCDC_CFG_PGRA_SPONSCREENCR     (lcdc_gra_ovsa_hpxl_vln)

/* LCDC F/B CLUT Table Set Register Address*/
#define LCDC_FBLCLUTCR     (lcdc_sram_ctrl)
#define LCDC_FBLCLUTWRDR        (lcdc_sram_wrdat)

/* LCDC F/B GAMMA Table Set Register Address*/
#define LCDC_FBLGAMMATWRCR     (lcdc_sram_ctrl)
#define LCDC_FBLGAMMATWRDR       (lcdc_sram_wrdat)
#define LCDC_FBLGAMMADEBUGR       (lcdc_sram_para0)
#define LCDC_FBLGAMMATWRENAR   (lcdc_sram_para1)

/* LCDC F/B Chormkey contol Register Address*/
#define LCDC_BFGCHOKCR        (lcdc_pn_ctrl1)

/* LCDC F/B Chormkey  Set Register Address*/
#define LCDC_FGCHOKYSR        ( lcdc_pn_colorkey_y)
#define LCDC_FGCHOKUSR        ( lcdc_pn_colorkey_u)
#define LCDC_FGCHOKVSR        ( lcdc_pn_colorkey_v)

/* LCDC Dumb Register Address*/
#define LCDC_DUMBCR        (lcdc_dumb_ctrl)

/* LCDC Dither Register Address*/
#define LCDC_DITHERCR        (lcdc_dither_ctrl)
#define LCDC_DITHERDR        (lcdc_dither_tbl_data)

/* LCDC WBTG Control Register Address*/
#define LCDC_WBTGCR        (lcdc_wdma_v_h)

/* LCDC F/B Chormkey Enable Register Address*/
#define LCDC_CHOKYENACR      (lcdc_tv_ctrl1)

/* LCDC DSI Color FMT Register Address*/
#define LCDC_DSICFMTCR      (lcdc_dsi_color_fmt)

/*LCDC GAMMA Read Data Register Address*/
#define LCDC_PNGAMMARDDR      (lcdc_pn_gamma_rddat)

/*LCDC Panel GAMMA Correction Register Address*/
#define LCDC_PNGAMMAENACR      (lcdc_tvd_ctrl)

/*LCDC Panel IO Register Address*/
#define LCDC_PNIOCR      (lcdc_pnio_ctrl)

/* LCDC Register Bit Definitions */

/* LCDC Start Register  */
#define LCDC_STARTCR_SHIFT         (0)    /* Bits 0: Start dma2d process  */
#define LCDC_STARTCR_MASK          (0x1 << LCDC_STARTCR_SHIFT)
#define LCDC_STARTCR_ENA(n)        ((uint32_t)(n) << LCDC_STARTCR_SHIFT)

/* LCDC Swap Panel Path Processing Register */
#define LCDC_SWAPPPPCR_ASEL_SHIFT       (17)    /*Bits 17: Swap Panel Path Processing */
                          /* 0 = Panel path video processing is for video DMA, palette table is for graphic DMA */
                          /* 1 = Panel path video processing is for graphic DMA, palette table is for video DMA */

#define LCDC_SWAPPPPCR_ASEL_MASK        (0x1 << LCDC_SWAPPPPCR_ASEL_SHIFT)
#define LCDC_SWAPPPPCR_ASEL(n)          ((uint32_t)(n) << LCDC_SWAPPPPCR_ASEL_SHIFT)

#define LCDC_SWAPPPPCR_ASEL_ENA_SHIFT       (16)    /*Bits 16: Swap Panel Path Enable */
/* 0 = Auto detect Panel path video processing path */
/* 1 = Panel path video processing is selected by the <Swap Panel Path Processing> field */

#define LCDC_SWAPPPPCR_ASEL_ENA_MASK        (0x1 << LCDC_SWAPPPPCR_ASEL_ENA_SHIFT)
#define LCDC_SWAPPPPCR_ASEL_ENA(n)          ((uint32_t)(n) << LCDC_SWAPPPPCR_ASEL_ENA_SHIFT)

#define LCDC_SWAPPPPCR_BURST_LEN_SHIFT       (8)    /*Bits 8: burst length */
#define LCDC_SWAPPPPCR_BURST_LEN_MASK        (0x3 << LCDC_SWAPPPPCR_BURST_LEN_SHIFT)
#define LCDC_SWAPPPPCR_BURST_LEN(n)          ((uint32_t)(n) << LCDC_SWAPPPPCR_BURST_LEN_SHIFT)

/* LCDC Blend Register */
#define LCDC_BLDCR_FIRSTLSEL_SHIFT       (0) /*Bits 0-1: Alpha Blending First Layer Selection */
 /* This field is used when more than two layers are overlaid. It selects the first layer */
 /* 0x0 = Top layer is Panel path video DMA */
 /* 0x1 = Top layer is Panel path graphic DMA */
 /* 0x2 = Top layer is TV path video DMA */
 /* 0x3 = Top layer is TV path graphic DMA */

#define LCDC_BLDCR_FIRSTLSEL_MASK        (0x3 << LCDC_BLDCR_FIRSTLSEL_SHIFT)
#define LCDC_BLDCR_FIRSTLSEL(n)          ((uint32_t)(n) << LCDC_BLDCR_FIRSTLSEL_SHIFT )

#define LCDC_BLDCR_SECONDLSEL_SHIFT       (2) /*Bits 2-3:Alpha Blending Second Layer Selection */
 /* This field is used when more than two layers are overlaid. It selects the second layer. */
 /* 0x0 = Second layer is Panel path video DMA */
 /* 0x1 = Second layer is Panel path graphic DMA */
 /* x2 = Second layer is TV path video DMA */
 /* 0x3 = second layer is TV graphic DMA */

#define LCDC_BLDCR_SECONDLSEL_MASK        (0x3 << LCDC_BLDCR_SECONDLSEL_SHIFT)
#define LCDC_BLDCR_SECONDLSEL(n)          ((uint32_t)(n) << LCDC_BLDCR_SECONDLSEL_SHIFT )

/* LCDC Active Width Configuration Register */
#define LCDC_AWCR_AAW_SHIFT       (0)    /* Bits 0-11: Accumulated Active width (scan lines) */
                           /* Panel Path Screen Horizontal  Pixels<p>This field sets */
                           /* the  horizontal screen display width for both Dumb Panel and Smart Panel. */
#define LCDC_AWCR_AAW_MASK        (0x7ff << LCDC_AWCR_AAW_SHIFT)
#define LCDC_AWCR_AAW(n)          ((uint32_t)(n) << LCDC_AWCR_AAW_SHIFT)

#define LCDC_AWCR_AAH_SHIFT       (16)    /* Bits 16-27: Accumulated Active height (pixel clocks) */
                           /* Panel Path Screen  Vertical Lines<p>This field sets the */
                           /*   vertical screen display size for both Dumb Panel and Smart Panel. */
#define LCDC_AWCR_AAH_MASK       (0xfff << LCDC_AWCR_AAH_SHIFT)
#define LCDC_AWCR_AAH(n)         ((uint32_t)(n) << LCDC_AWCR_AAH_SHIFT)

/* LCDC Interrupt Enable Register */
#define FGL_PDMA_FRAME_IRQ0_ENA_SHIFT      (31)    /* Bits 31: Panel Path DMA Frame 0 Done IRQ Enable */
#define FGL_PDMA_FRAME_IRQ0_ENA_MASK       (0x01<< DMA_FRAME_IRQ0_ENA_SHIFT)
#define FGL_PDMA_FRAME_IRQ0_ENA            (1<<DMA_FRAME_IRQ0_ENA_SHIFT)

#define FGL_PDMA_FRAME_IRQ1_ENA   (1 <<30)    /* Panel Path DMA Frame 1 Done IRQ Enable  */
#define FGL_PDMA_FF_UNDERFLOW_ENA (1 << 29)    /* Panel Path DMA FIFO Underflow IRQ Enable */
#define LCDC_M2A_IRQ_ENA          (1 << 28)    /*  AXI Bus Error IRQ Enable */
#define BGL_GRA_FRAME_IRQ0_ENA    (1 << 27)    /* Panel Path Graphic Frame 0 IRQ Enable */
#define BGL_GRA_FRAME_IRQ1_ENA    (1 << 26)    /* Panel Path Graphic Frame 1 IRQ Enable */
#define BGL_GRA_FF_UNDERFLOW_ENA  (1 << 25)    /* Panel Path Graphic FIFO Underflow IRQ Enable */

#define PN_VSYNC_IRQ_ENA_SHIFT      (23)    /* Bits 23: Video mode VSYNC IRQ Enable */
#define PN_VSYNC_IRQ_ENA_MASK       (0x01<< PN_VSYNC_IRQ_ENA_SHIFT)
#define PN_VSYNC_IRQ_ENA  (1<<PN_VSYNC_IRQ_ENA_SHIFT)

#define DUMB_FRAMEDONE_IRQ_ENA_SHIFT      (22)    /* Bits 22: DUMB Panel Path Smart Panel Frame Done IRQ Enable */
#define DUMB_FRAMEDONE_IRQ_ENA_MASK       (0x01<< DUMB_FRAMEDONE_IRQ_ENA_SHIFT)
#define DUMB_FRAMEDONE_IRQ_ENA  (1<<DUMB_FRAMEDONE_IRQ_ENA_SHIFT)

#define SPU_FRAMEDONE_IRQ_ENA_SHIFT      (21)    /* Bits 21: Panel Path Smart Panel Frame Done IRQ Enable */
#define SPU_FRAMEDONE_IRQ_ENA_MASK       (0x01<< SPU_FRAMEDONE_IRQ_ENA_SHIFT)
#define SPU_FRAMEDONE_IRQ_ENA  (1<<SPU_FRAMEDONE_IRQ_ENA_SHIFT)

#define LCDC_PWRDN_IRQ_ENA  ( 1 << 17)                /*Power Down Request IRQ Enable */
#define LCDC_ERR_IRQ_ENA(n) ( ((n) & 0x1) << 16)    /*AXI Bus Latency Too Long IRQ Enable */

/* LCDC Interrupt Status Register */
#define FGL_PDMA_FRAME_IRQ0 (1 << 31)    /* Panel Path Video Frame 0 Done Rising Edge IRQ */
#define FGL_DMA_FRAME_IRQ1  (1 << 30)    /* Panel Path Video Frame 1 Done Rising Edge IRQ */
#define FGL_DMA_FF_UNDERFLOW (1 << 29)    /* Panel Path Video FIFO Underflow IRQ */
#define LCDC_M2A_IRQ (1 << 28)          /* AXI Bus Error IRQ */
#define BGL_GRA_FRAME_IRQ0 (1 << 27)    /* Panel Path Graphic Frame 0 Done Rising Edge IRQ */
#define BGL_GRA_FRAME_IRQ1 (1 << 26)    /* Panel Path Graphic Frame 1 Done Rising Edge IRQ */
#define BGL_GRA_FF_UNDERFLOW (1 << 25)    /* Panel Path Graphic FIFO Underflow IRQ */
#define PN_VSYNC_IRQ       (1 << 23)    /* Video mode VSYNC IRQ */
#define DUMB_FRAMEDONE     (1 << 22)    /* Video mode Dumb Panel Display Area DMA Done IRQ */
#define SPU_TXC_FRAMEDONE  (1 << 21)    /* Panel Path Smart Panel Display Area DMA Done IRQ */
#define LCDC_PWRDN_IRQ     (1 << 17)    /* Power Down Request IRQ */
#define LCDC_ERR_IRQ       (1 << 16)    /* AXI Bus Latency Too Long IRQ */
#define FGL_DMA_FF_EMPY    (1 << 3)        /* Level of DMA_FF_EMPY */
#define BGL_GRA_FF_EMPTY   (1 << 2)        /* Level of GRA_FF_EMPTY */

/* LCDC Interrupt Flag Clear Register */
#define FGL_DMA_FRAME_IRQ0_CLEAN   (1<<31)        /* Clean Panel Path DMA Frame 0 Done IRQ FLAG */
#define FGL_DMA_FRAME_IRQ1_CLEAN   (1 <<30)        /* Clean Panel Path DMA Frame 1 Done IRQ FLAG */
#define FGL_DMA_FF_UNDERFLOW_CLEAN (1 << 29)    /* Clean Panel Path DMA FIFO Underflow IRQ FLAG */
#define LCDC_M2A_IRQ_CLEAN         (1 << 28)    /* Clean  AXI Bus Error IRQ FLAG */
#define BGL_GRA_FRAME_IRQ0_CLEAN   (1 << 27)    /* Clean Panel Path Graphic Frame 0 IRQ FLAG */
#define BGL_GRA_FRAME_IRQ1_CLEAN   (1 << 26)    /* Clean Panel Path Graphic Frame 1 IRQ FLAG */
#define BGL_GRA_FF_UNDERFLOW_CLEAN (1 << 25)    /* Clean Panel Path Graphic FIFO Underflow IRQ FLAG */
#define PN_VSYNC_IRQ_CLEAN         (1 << 23)    /* Clean Video mode VSYNC IRQ FLAG */
#define DUMB_FRAMEDONE_CLEAN       (1 << 22)    /* Clean Dumb Panel Path Smart Panel Frame Done IRQ FLAG */
#define SPU_FRAMEDONE_CLEAN        (1 << 21)    /* Clean Panel Path Smart Panel Frame Done IRQ FLAG */
#define LCDC_PWRDN_IRQ_CLEAN       ( 1 << 17)    /* Clean Power Down Request IRQ FLAG */
#define LCDC_ERR_IRQ_CLEAN         (1 << 16)    /* Clean AXI Bus Latency Too Long IRQ FLAG */

/* LCDC Foreground/Background PFC Control Register */
#define LCDC_xGPFCCR_LENA_SHIFT      (0)    /* Bits 0  layer enable  */
#define LCDC_xGPFCCR_LENA_MASK       (0x01 << LCDC_xGPFCCR_LENA_SHIFT)
#define LCDC_xGPFCCR_LENA(n)         ((uint32_t)(n) << LCDC_xGPFCCR_LENA_SHIFT)

#define LCDC_CFG_YUV2RGB_ENA_SHIFT   (1)
#define LCDC_CFG_YUV2RGB_ENA_MASK    ( 0x01 << LCDC_CFG_YUV2RGB_ENA_SHIFT)
#define LCDC_CFG_YUV2RGB_ENA(n)      ( (0x01 &(n)) << LCDC_CFG_YUV2RGB_ENA_SHIFT)

#define LCDC_xGPFCCR_CM_SHIFT      (8)    /* Bits 8-11 Color Mode */
                     /* Panel Path Video memory Color Format */
                     /* 0x0 = RGB565 */
                     /* 0x1 = RGB1555 */
                     /* 0x2 = RGB888 packed */
                     /* 0x3 = RGB888 unpacked */
                     /* 0x4 = ARGB */
                     /* 0x5 = YUV422 packed */
                     /* 0x6 = YUV422 planar */
                     /* 0x7 = YUV420 planar */
                     /* 0x8 = Smart Panel command */
                     /* 0x9 =  Palette color 4-bit per pixel */
                     /* 0xA = Palette color 8-bit per pixel */
                     /* 0xB = RGBA */
                     /* All other values are reserved. */

                     /* Panel Path Graphic Memory Color Format */
                     /* 0x0 = RGB565  */
                     /* 0x1 = RGB1555 */
                     /* 0x2 = RGB888 packed */
                     /* 0x3 = RGB888 unpacked */
                     /* 0x4 = ARGB */
                     /* 0x5 = YUV422 packed */
                     /* 0x6 to 0x8 = Reserved */
                     /* 0x9 = Palette color 4-bit per pixel */
                     /* 0xA = Palette color 8-bit per pixel */
                     /* 0xb = RGB8A  */
                     /* All other values are reserved. */

#define LCDC_xGPFCCR1_TRANS_TRIGGER_SHIFT      (31)    /* Bits 31: Panel Path DMA Transfer Trigger */
                             /* 1 = DMA transfer is triggered, this is equivalent to VSYNC pulse */
                             /* This field is useful when Smart Panel VSYNC input is not available */
#define LCDC_xGPFCCR1_TRANS_TRIGGER_MASK      (1 << LCDC_xGPFCCR1_TRANS_TRIGGER_SHIFT)
#define LCDC_xGPFCCR1_TRANS_TRIGGER_ENA(n)    ((uint32_t)(n) << LCDC_xGPFCCR1_TRANS_TRIGGER_SHIFT)

#define LCDC_xGPFCCR_CM_MASK        (0xF << LCDC_xGPFCCR_CM_SHIFT)
#define LCDC_xGPFCCR_CM(n)          ((uint32_t)(n) << LCDC_xGPFCCR_CM_SHIFT)

#define LCDC_xGPFCCR1_CCM_SHIFT        (24)    /*  Color key Mode */
#define LCDC_xGPFCCR1_CCM_MASK         (0x07 << LCDC_xGPFCCR1_CCM_SHIFT)    /*  Color key Mode */
#define LCDC_xGPFCCR1_CCM(n)           ((uint32_t)(n) << LCDC_xGPFCCR1_CCM_SHIFT)    /*  Color key Mode */

#define LCDC_xGPFCCR_START_SHIFT       (5)    /* Start */
#define LCDC_xGPFCCR_START_MASK        (1 << LCDC_xGPFCCR_START_SHIFT)    /* Start */
#define LCDC_xGPFCCR_START(n)          (1 << LCDC_xGPFCCR_START_SHIFT)    /* Start */

#define LCDC_xGPFCCR1_AM_SHIFT      (16)    /* Bits 16-17 Alpha Mode */
#define LCDC_xGPFCCR1_AM_MASK       (3 << LCDC_xGPFCCR1_AM_SHIFT)
#define LCDC_xGPFCCR1_AM(n)         ((uint32_t)(n) << LCDC_xGPFCCR1_AM_SHIFT)

#define LCDC_xGPFCCR1_ALPHA_SHIFT   (8)    /* Bits 8-15 Alpha Value */
#define LCDC_xGPFCCR1_ALPHA_MASK    (0xFF << LCDC_xGPFCCR1_ALPHA_SHIFT)
#define LCDC_xGPFCCR1_ALPHA(n)      ((uint32_t)(n) << LCDC_xGPFCCR1_ALPHA_SHIFT)

#define LCDC_xGPFCCR_SWAPRB_ENA_SHIFT   (4)    /* Bits 4: Panel Path Graphic DMA Swap R and B  */
                         /* This field swaps R and B. For example, RGB is swapped to BGR. */
                         /* 1 = Swap enabled */
                         /* 0 = Swap disabled */
#define LCDC_xGPFCCR_SWAPRB_ENA_MASK    (0x01 << LCDC_xGPFCCR_SWAPRB_ENA_SHIFT)
#define LCDC_xGPFCCR_SWAPRB_ENA(n)      ((uint32_t)(n) << LCDC_xGPFCCR_SWAPRB_ENA_SHIFT)

/*F/B Layer VH Register */
#define LCDC_CFG_PN_H_TOTAL_SHIFT        (0)
#define LCDC_CFG_PN_H_TOTAL(n)           (((n) & 0xFFF) << LCDC_CFG_PN_H_TOTAL_SHIFT)
#define LCDC_CFG_PN_H_TOTAL_MASK         (0xFFF << LCDC_CFG_PN_H_TOTAL_SHIFT)

#define LCDC_CFG_PN_V_TOTAL_SHIFT        (16)
#define LCDC_CFG_PN_V_TOTAL(n)           (((n) & 0xFFF) << LCDC_CFG_PN_V_TOTAL_SHIFT)
#define LCDC_CFG_PN_V_TOTAL_MASK         (0xFFF << LCDC_CFG_PN_V_TOTAL_SHIFT)

/*F/B Layer HPorchRegister */
#define LCDC_CFG_PN_H_FPORCH_SHIFT       (0)
#define LCDC_CFG_PN_H_FPORCH(n)          (((n) & 0xFFF) << LCDC_CFG_PN_H_FPORCH_SHIFT)
#define LCDC_CFG_PN_H_FPORCH_MASK        (0xFFF << LCDC_CFG_PN_H_FPORCH_SHIFT)

#define LCDC_CFG_PN_H_BPORCH_SHIFT       (16)
#define LCDC_CFG_PN_H_BPORCH(n)          (((n) & 0xFFF) << LCDC_CFG_PN_H_BPORCH_SHIFT)
#define LCDC_CFG_PN_H_BPORCH_MASK        (0xFFF << LCDC_CFG_PN_H_BPORCH_SHIFT)

/*F/B Layer VPorchRegister */
#define LCDC_CFG_PN_V_FPORCH_SHIFT       (0)
#define LCDC_CFG_PN_V_FPORCH(n)          (((n) & 0xFFF) << LCDC_CFG_PN_V_FPORCH_SHIFT)
#define LCDC_CFG_PN_V_FPORCH_MASK        (0xFFF << LCDC_CFG_PN_V_FPORCH_SHIFT)

#define LCDC_CFG_PN_V_BPORCH_SHIFT       (16)
#define LCDC_CFG_PN_V_BPORCH(n)          (((n) & 0xFFF) << LCDC_CFG_PN_V_BPORCH_SHIFT)
#define LCDC_CFG_PN_V_BPORCH_MASK        (0xFFF << LCDC_CFG_PN_V_BPORCH_SHIFT)

/*F/B Layer Start/End Pix count  Register */
#define LCD_CFG_PN_V_SPXLCNT_SHIFT        (0)
#define LCD_CFG_PN_V_SPXLCNT(n)           (((n) & 0xFFF) << LCD_CFG_PN_V_SPXLCNT_SHIFT)
#define LCD_CFG_PN_V_SPXLCNT_MASK         (0xFFF << LCD_CFG_PN_V_SPXLCNT_SHIFT)

#define LCD_RESERVED_SHIFT                (12)
#define LCD_RESERVED(n)                   (((n) & 0xF) << LCD_RESERVED_SHIFT)
#define LCD_RESERVED_MASK                 (0xF << LCD_RESERVED_SHIFT)

#define LCD_CFG_PN_V_EPXLCNT_SHIFT        (16)
#define LCD_CFG_PN_V_EPXLCNT(n)           (((n) & 0xFFF) << LCD_CFG_PN_V_EPXLCNT_SHIFT)
#define LCD_CFG_PN_V_EPXLCNT_MASK         (0xFFF << LCD_CFG_PN_V_EPXLCNT_SHIFT)

#define LCD_CFG_DBG_RD_INDEX_SHIFT        (28)
#define LCD_CFG_DBG_RD_INDEX(n)           (((n) & 0xF) << LCD_CFG_DBG_RD_INDEX_SHIFT)
#define LCD_CFG_DBG_RD_INDEX_MASK         (0xF << LCD_CFG_DBG_RD_INDEX_SHIFT)

/*Display module config Register  210 */
#define LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT   (0)    /* Bits 0: cfg_vsync_trig_disable_s */
#define LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE_MASK    (0x01 << LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT)
#define LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE(n)      ((uint32_t)(n) << LCDC_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT)

#define LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT   (1)    /* Bits 1: cfg_vsync_trig_disable_s */
#define LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE_MASK    (0x01 << LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT)
#define LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE(n)      ((uint32_t)(n & 0x01) << LCDC_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT)

#define LCDC_DISPCNFCR_MEM_ENABLE_SHIFT   (30)    /* Bits 30: disp memory enable */
#define LCDC_DISPCNFCR_MEM_ENABLE_MASK    (0x01 << LCDC_DISPCNFCR_MEM_ENABLE_SHIFT)
#define LCDC_DISPCNFCR_MEM_ENABLE(n)      ((uint32_t)(n) << LCDC_DISPCNFCR_MEM_ENABLE_SHIFT)

#define LCDC_DISPCNFCR_TE_BYPASS            (1 << 2)
#define LCDC_DISPCNFCR_TE_SEL               (1 << 3)
#define LCDC_DISPCNFCR_EDGE_SEL             (1 << 4)
#define LCDC_DISPCNFCR_TE_MODE              (1 << 5)

#define LCDC_DISPCNFCR_NFRAME_ACTIVE_SHIFT  (6)
#define LCDC_DISPCNFCR_NFRAME_ACTIVE(n)     (((n) & 0xFF) << LCDC_DISPCNFCR_NFRAME_ACTIVE_SHIFT)
#define LCDC_DISPCNFCR_NFRAME_ACTIVE_MASK   (0xFF << LCDC_DISPCNFCR_NFRAME_ACTIVE_SHIFT)

#define LCDC_DISPCNFCR_EXTRA_DELAY_SHIFT    (14)
#define LCDC_DISPCNFCR_EXTRA_DELAY(n)       (((n) & 0x3FF) << LCDC_DISPCNFCR_EXTRA_DELAY_SHIFT)
#define LCDC_DISPCNFCR_EXTRA_DELAY_MASK     (0x3FF << LCDC_DISPCNFCR_EXTRA_DELAY_SHIFT)

#define LCDC_DISPCNFCR_TE_CLR               (1 << 24)
#define LCDC_DISPCNFCR_TE_MASK              (1 << 25)
#define LCDC_DISPCNFCR_DSI_DR               (1 << 26)
#define LCDC_DISPCNFCR_BACKLIGHT_EN         (1 << 27)
#define LCDC_DISPCNFCR_BACKLIGHT_DUMB_MASK  (1 << 28)

#define LCDC_DISPCNFCR_REG_GEN_FRAME_SHIFT  (29)
#define LCDC_DISPCNFCR_REG_GEN_FRAME_MASK   (1 << LCDC_DISPCNFCR_REG_GEN_FRAME_SHIFT)
#define LCDC_DISPCNFCR_REG_GEN_FRAME(n)     (((n) & 0x01) << LCDC_DISPCNFCR_REG_GEN_FRAME_SHIFT)

#define LCDC_DISPCNFCR_DSI_SOURCE_SEL       (1 << 31)

/*Display module config Register  214 */

#define LCDC_DISPCNFCR_HSYNC_LENGTH_SHIFT   (0)
#define LCDC_DISPCNFCR_HSYNC_LENGTH(n)      (((n) & 0xFFFF) << LCDC_DISPCNFCR_HSYNC_LENGTH_SHIFT)
#define LCDC_DISPCNFCR_HSYNC_LENGTH_MASK    (0xFFFF << LCDC_DISPCNFCR_HSYNC_LENGTH_SHIFT)

#define LCDC_DISPCNFCR_DEL_NLINE_SHIFT      (16)
#define LCDC_DISPCNFCR_DEL_NLINE(n)         (((n) & 0x3FF) << LCDC_DISPCNFCR_DEL_NLINE_SHIFT)
#define LCDC_DISPCNFCR_DEL_NLINE_MASK       (0x3FF << LCDC_DISPCNFCR_DEL_NLINE_SHIFT)

#define LCDC_DISPCNFCR_HWC_ENA              (1 << 26)
#define LCDC_DISPCNFCR_HWC_1BITENA          (1 << 27)
#define LCDC_DISPCNFCR_HWC_1BITMOD          (1 << 28)
#define LCDC_DISPCNFCR_SPI_CMD_START        (1 << 29)
#define LCDC_DISPCNFCR_DSI_RD_MEM_CMD       (1 << 30)
#define LCDC_DISPCNFCR_TE_INV

//LCDC_TECR   254
#define LCDC_TECNFCR_DB_TARGET_SHIFT         (0)
#define LCDC_TECNFCR_DB_TARGET(n)            (((n) & 0xFF) << LCDC_TECNFCR_DB_TARGET_SHIFT)
#define LCDC_TECNFCR_DB_TARGET_MASK          (0xFF << LCDC_TECNFCR_DB_TARGET_SHIFT)

#define LCDC_TECNFCR_DB_BYPASS               (1 << 8)

#define LCDC_TECNFCR_USE_SW                  (1 << 9)

#define LCDC_TECNFCR_RESERVED_SHIFT          (10)
#define LCDC_TECNFCR_RESERVED(n)             (((n) & 0x3FFFFF) << LCDC_TECNFCR_RESERVED_SHIFT)
#define LCDC_TECNFCR_RESERVED_MASK           (0x3FFFFF << LCDC_TECNFCR_RESERVED_SHIFT)

//LCDC_FRAME CONFIG 258
#define LCD_CFG_FRAME_VALID_SHIFT      (0)
#define LCD_CFG_FRAME_VALID(n)         (((n) & 0xFFFFFFFF) << LCD_CFG_FRAME_VALID_SHIFT)
#define LCD_CFG_FRAME_VALID_MASK       (0xFFFFFFFF << LCD_CFG_FRAME_VALID_SHIFT)

//SMPU Module Config_Register  188

#define LCDC_CFG_SMPN_ENA          (1 << 0)
#define LCDC_CFG_KEEPXFER          (1 << 1)
#define LCDC_CFG_68S80SB           (1 << 2)
#define LCDC_SMPN_RSTB             (1 << 3)
#define LCDC_CFG_SLV_ENA           (1 << 4)
#define LCDC_SMPN_SEL              (1 << 5)
#define LCDC_CFG_SWAPBYTES         (1 << 6)
#define LCDC_CFG_CMD32OR16B        (1 << 7)
#define LCDC_SMPNCR_MODE_SHIFT     (8)
#define LCDC_SMPNCR_MODE_SET(n)    (((n) & 0xF) << LCDC_SMPNCR_MODE_SHIFT)
#define LCDC_SMPNCR_MODE_MASK      (0xF << LCDC_SMPNCR_MODE_SHIFT)
#define LCDC_CFG_SLV_ONLY          (1 << 12)
#define LCDC_CFG_IORDY_MSK         (1 << 13)
#define LCDC_CFG_SMPNVSYNC_SHIFT   (14)
#define LCDC_CFG_SMPNVSYNC(n)      (((n) & 0x3) << LCDC_CFG_SMPNVSYNC_SHIFT)
#define LCDC_CFG_SMPNVSYNC_MASK    (0x3 << LCDC_CFG_SMPNVSYNC_SHIFT)
#define LCDC_CFG_ISA_TXHIGH_SHIFT  (16)
#define LCDC_CFG_ISA_TXHIGH(n)     (((n) & 0xF) << LCDC_CFG_ISA_TXHIGH_SHIFT)
#define LCDC_CFG_ISA_TXHIGH_MASK   (0xF << LCDC_CFG_ISA_TXHIGH_SHIFT)
#define LCDC_CFG_ISA_TXLOW_SHIFT   (LCDC_CFG_ISA_TXLOW_SHIFT)
#define LCDC_CFG_ISA_TXLOW(n)      (((n) & 0xF) << LCDC_CFG_ISA_TXLOW_SHIFT)
#define LCDC_CFG_ISA_TXLOW_MASK    (0xF << LCDC_CFG_ISA_TXLOW_SHIFT)
#define LCDC_CFG_ISA_RXHIGH_SHIFT  (24)
#define LCDC_CFG_ISA_RXHIGH(n)     (((n) & 0xF) << LCDC_CFG_ISA_RXHIGH_SHIFT)
#define LCDC_CFG_ISA_RXHIGH_MASK   (0xF << LCDC_CFG_ISA_RXHIGH_SHIFT)
#define LCDC_CFG_ISA_RXLOW_SHIFT   (28)
#define LCDC_CFG_ISA_RXLOW(n)      (((n) & 0xF) << LCDC_CFG_ISA_RXLOW_SHIFT)
#define LCDC_CFG_ISA_RXLOW_MASK    (0xF << LCDC_CFG_ISA_RXLOW_SHIFT)

/* LCDC F/B CLUT Table Set Register */

#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR_SHIFT  (0)
#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR(n)     (((n) & 0xFF) << LCDC_FBLCLUTTABLE_SETSRAM_ADDR_SHIFT)
#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR_MASK   (0xFF << LCDC_FBLCLUTTABLE_SETSRAM_ADDR_SHIFT)

#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_SHIFT (8)
#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID(n)    (((n) & 0xF) << LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_SHIFT)
#define LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_MASK  (0xF << LCDC_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_SHIFT)

#define LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_SHIFT (14)
#define LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD(n)    (((n) & 0x3) << LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_SHIFT)
#define LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_MASK  (0x3 << LCDC_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_SHIFT)

#define LCDC_FBLCLUTTABLE_WRDAT_SHIFT      (0)
#define LCDC_FBLCLUTTABLE_WRDAT(n)         (((n) & 0xFF) << LCDC_FBLCLUTTABLE_WRDAT_SHIFT)
#define LCDC_FBLCLUTTABLE_WRDAT_MASK       (0xFF << LCDC_FBLCLUTTABLE_WRDAT_SHIFT)

/* LCDC F/B GAMMA Table Set Register Address*/

//#define LCDC_FBLGAMMATWRCR     (lcdc_sram_ctrl)
#define LCDC_FBLGAMMAT_SETSRAM_ADDR_SHIFT  (0)
#define LCDC_FBLGAMMAT_SETSRAM_ADDR(n)     (((n) & 0xFF) << LCDC_FBLGAMMAT_SETSRAM_ADDR_SHIFT)
#define LCDC_FBLGAMMAT_SETSRAM_ADDR_MASK   (0xFF << LCDC_FBLGAMMAT_SETSRAM_ADDR_SHIFT)

#define LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_SHIFT (8)
#define LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID(n)    (((n) & 0xF) << LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_SHIFT)
#define LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_MASK  (0xF << LCDC_FBLGAMMAT_SETSRAM_ADDR_LCDID_SHIFT)

#define LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD_SHIFT (14)
#define LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD(n)    (((n) & 0x3) << LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD_SHIFT)
#define LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD_MASK  (0x3 << LCDC_FBLGAMMAT_SETSRAM_INIT_WR_RD_SHIFT)

//#define LCDC_FBLGAMMATWRDR       (lcdc_sram_wrdat)
#define LCDC_FBLGAMMAT_WRDAT_SHIFT      (0)
#define LCDC_FBLGAMMAT_WRDAT(n)         (((n) & 0xFF) << LCDC_FBLGAMMAT_WRDAT_SHIFT)
#define LCDC_FBLGAMMAT_WRDAT_MASK       (0xFF << LCDC_FBLGAMMAT_WRDAT_SHIFT)

//#define LCDC_FBLGAMMADEBUGR      (lcdc_sram_para0)
#define LCDC_CFG_RTC128X66(n)           (((n) & 0x3) << 0)
#define LCDC_CFG_RTC128X66_MASK         (0x3 << 0)
#define LCDC_CFG_RTC128X66_SHIFT        (0)

#define LCDC_CFG_WTC128X66(n)           (((n) & 0x3) << 2)
#define LCDC_CFG_WTC128X66_MASK         (0x3 << 2)
#define LCDC_CFG_WTC128X66_SHIFT        (2)
#define LCDC_CFG_RTC64X66(n)            (((n) & 0x3) << 4)
#define LCDC_CFG_RTC64X66_MASK          (0x3 << 4)
#define LCDC_CFG_RTC64X66_SHIFT         (4)
#define LCDC_CFG_WTC64X66(n)            (((n) & 0x3) << 6)
#define LCDC_CFG_WTC64X66_MASK          (0x3 << 6)
#define LCDC_CFG_WTC64X66_SHIFT         (6)
#define LCDC_CFG_RTC32X66(n)            (((n) & 0x3) << 8)
#define LCDC_CFG_RTC32X66_MASK          (0x3 << 8)
#define LCDC_CFG_RTC32X66_SHIFT         (8)
#define LCDC_CFG_WTC32X66(n)            (((n) & 0x3) << 10)
#define LCDC_CFG_WTC32X66_MASK          (0x3 << 10)
#define LCDC_CFG_WTC32X66_SHIFT         (10)
#define LCDC_CFG_RTC32X32(n)            (((n) & 0x3) << 12)
#define LCDC_CFG_RTC32X32_MASK          (0x3 << 12)
#define LCDC_CFG_RTC32X32_SHIFT         (12)
#define LCDC_CFG_WTC32X32(n)            (((n) & 0x3) << 14)
#define LCDC_CFG_WTC32X32_MASK          (0x3 << 14)
#define LCDC_CFG_WTC32X32_SHIFT         (14)
#define LCDC_RESERVED(n)                (((n) & 0xF) << 16)
#define LCDC_RESERVED_MASK              (0xF << 16)
#define LCDC_RESERVED_SHIFT             (16)
#define LCDC_CFG_RTC256X8(n)            (((n) & 0x3) << 20)
#define LCDC_CFG_RTC256X8_MASK          (0x3 << 20)
#define LCDC_CFG_RTC256X8_SHIFT         (20)
#define LCDC_CFG_WTC256X8(n)            (((n) & 0x3) << 22)
#define LCDC_CFG_WTC256X8_MASK          (0x3 << 22)
#define LCDC_CFG_WTC256X8_SHIFT         (22)
#define LCDC_CFG_RTC256X24(n)           (((n) & 0x3) << 24)
#define LCDC_CFG_RTC256X24_MASK         (0x3 << 24)
#define LCDC_CFG_RTC256X24_SHIFT        (24)
#define LCDC_CFG_WTC256X24(n)           (((n) & 0x3) << 26)
#define LCDC_CFG_WTC256X24_MASK         (0x3 << 26)
#define LCDC_CFG_WTC256X24_SHIFT        (26)
#define LCDC_CFG_RTC256X32(n)           (((n) & 0x3) << 28)
#define LCDC_CFG_RTC256X32_MASK         (0x3 << 28)
#define LCDC_CFG_RTC256X32_SHIFT        (28)
#define LCDC_CFG_WTC256X32(n)           (((n) & 0x3) << 30)
#define LCDC_CFG_WTC256X32_MASK         (0x3 << 30)
#define LCDC_CFG_WTC256X32_SHIFT        (30)

//#define LCDC_FBLGAMMATWRENAR   (lcdc_sram_para1)
#define LCDC_CFG_PDWN128X66             (1 << 0)
#define LCDC_CFG_PDWN64X66              (1 << 1)
#define LCDC_CFG_PDWN32X66              (1 << 2)
#define LCDC_CFG_PDWN32X32              (1 << 3)
#define LCDC_RESERVED_1A4BIT4           (1 << 4)
#define LCDC_CFG_PDWN256X8              (1 << 5)
#define LCDC_CFG_PDWN256X24             (1 << 6)
#define LCDC_CFG_PDWN256X32             (1 << 7)

#define LCDC_RESERVED_1A4BIT8_SHIFT     (8)
#define LCDC_RESERVED_1A4BIT8(n)        (((n) & 0x1F) << LCDC_RESERVED_1A4BIT8_SHIFT)
#define LCDC_RESERVED_1A4BIT8_MASK      (0x1F << LCDC_RESERVED_1A4BIT8_SHIFT)

#define LCDC_CFG_CSB_256X8_GAMMA_ENA_SHIFT   (13)
#define LCDC_CFG_CSB_256X8_GAMMA_ENA_MASK    (1 << LCDC_CFG_CSB_256X8_GAMMA_ENA_SHIFT)
#define LCDC_CFG_CSB_256X8_GAMMA_ENA(n)      ( (n & 0x01 ) << LCDC_CFG_CSB_256X8_GAMMA_ENA_SHIFT)

#define LCDC_CFG_CSB_256X24_clut        (1 << 14)
#define LCDC_CFG_CSB_256X32_cursor      (1 << 15)
#define LCDC_RESERVED_1A4BIT16_SHIFT    (16)
#define LCDC_RESERVED_1A4BIT16(n)       (((n) & 0xFFFF) << LCDC_RESERVED_1A4BIT16_SHIFT)
#define LCDC_RESERVED_1A4BIT16_MASK     (0xFFFF << LCDC_RESERVED_1A4BIT16_SHIFT)

/* LCDC Output layer PFC Control Register */
#define LCDC_OPFCCR_LENA_SHIFT      (0)    /* Bits 0  layer enable  */
#define LCDC_OPFCCR_LENA_MASK       (0x01 << LCDC_OPFCCR_LENA_SHIFT)
#define LCDC_OPFCCR_LENA(n)         ((uint32_t)(n) << LCDC_OPFCCR_LENA_SHIFT)

#define LCDC_OPFCCR_CM_SHIFT      (4)    /* Bits 4-5 Color Mode */
                     /* 0x0 = RGB565 */
                     /* 0x1 = RGB888 */
                     /* 0x2 = ARGB888 32 */
                     /* 0x3 = RGBA888 32 */
#define LCDC_OPFCCR_CM_MASK       (0x3 << LCDC_OPFCCR_CM_SHIFT)
#define LCDC_OPFCCR_CM(n)         ((uint32_t)(n) << LCDC_OPFCCR_CM_SHIFT)

/*OUT Layer dma burst Register */
#define LCDC_OL_BURST_LEN_SHIFT       (8)
#define LCDC_OL_BURST_LEN_MASK        (0x1F << 8)
#define LCDC_OL_BURST_LEN(n)          (((n) & 0x1F) << 8)

#define LCDC_LPITCH_SHIFT             (0)
#define LCDC_LPITCH_MASK              (0xFFFF << LCDC_LPITCH_SHIFT)
#define LCDC_LPITCH(n)                (((n) & 0xFFFF) << LCDC_LPITCH_SHIFT)

#define LCDC_OLPITCH_SHIFT            (16)
#define LCDC_OLPITCH_MASK             (0xFFFF << LCDC_OLPITCH_SHIFT)
#define LCDC_OLPITCH(n)               (((n) & 0xFFFF) << LCDC_OLPITCH_SHIFT)

/*LCDC OUTPUT Layer FPT Register*/
#define LCDC_OL_FPT_SHIFT             (28)
#define LCDC_OL_FPT_MASK              (1 << LCDC_OL_FPT_SHIFT)
#define LCDC_OL_FPT_SET(n)            (((n) & 0xFFFF) << LCDC_OL_FPT_SHIFT)

/* LCDC_STBY_VSYNCCR  */
#define LCDC_CFG_STBY_LENGTH_SHIFT           (0)
#define LCDC_CFG_STBY_LENGTH(n)              (((n) & 0xFFF) << LCDC_CFG_STBY_LENGTH_SHIFT)
#define LCDC_CFG_STBY_LENGTH_MASK            (0xFFF << LCDC_CFG_STBY_LENGTH_SHIFT)

#define LCDC_CFG_VSYNC_START_LINE_SHIFT      (12)
#define LCDC_CFG_VSYNC_START_LINE(n)         (((n) & 0xFF) << LCDC_CFG_VSYNC_START_LINE_SHIFT)
#define LCDC_CFG_VSYNC_START_LINE_MASK       (0xFF << LCDC_CFG_VSYNC_START_LINE_SHIFT)

#define LCDC_CFG_VSYNC_END_LINE_SHIFT        (20)
#define LCDC_CFG_VSYNC_END_LINE(n)           (((n) & 0xFF) << LCDC_CFG_VSYNC_END_LINE_SHIFT)
#define LCDC_CFG_VSYNC_END_LINE_MASK         (0xFF << LCDC_CFG_VSYNC_END_LINE_SHIFT)

/* LCDC_PORCH_VSYNCCR     */
#define LCDC_CFG_H_FRONT_PORCH_SMPN_SHIFT    (0)
#define LCDC_CFG_H_FRONT_PORCH_SMPN(n)       (((n) & 0xFFF) << LCDC_CFG_H_FRONT_PORCH_SMPN_SHIFT)
#define LCDC_CFG_H_FRONT_PORCH_SMPN_MASK     (0xFFF << LCDC_CFG_H_FRONT_PORCH_SMPN_SHIFT)

#define LCDC_CFG_H_BACK_PORCH_SMPN_SHIFT     (12)
#define LCDC_CFG_H_BACK_PORCH_SMPN(n)        (((n) & 0xFFF) << LCDC_CFG_H_BACK_PORCH_SMPN_SHIFT)
#define LCDC_CFG_H_BACK_PORCH_SMPN_MASK      (0xFFF << LCDC_CFG_H_BACK_PORCH_SMPN_SHIFT)

#define LCDC_CFG_VSYNC_NLINE_SHIFT           (24)
#define LCDC_CFG_VSYNC_NLINE(n)              (((n) & 0xFF) << LCDC_CFG_VSYNC_NLINE_SHIFT)
#define LCDC_CFG_VSYNC_NLINE_MASK            (0xFF << LCDC_CFG_VSYNC_NLINE_SHIFT)

/* LCDC_VSYNC_PIXCR     */
#define LCDC_CFG_VSYNC_START_PIX_SHIFT       (0)
#define LCDC_CFG_VSYNC_START_PIX(n)          (((n) & 0xFFFF) << LCDC_CFG_VSYNC_START_PIX_SHIFT)
#define LCDC_CFG_VSYNC_START_PIX_MASK        (0xFFFF << LCDC_CFG_VSYNC_START_PIX_SHIFT)

#define LCDC_CFG_VSYNC_END_PIX_SHIFT         (16)
#define LCDC_CFG_VSYNC_END_PIX(n)            (((n) & 0xFFFF) << LCDC_CFG_VSYNC_END_PIX_SHIFT)
#define LCDC_CFG_VSYNC_END_PIX_MASK          (0xFFFF << LCDC_CFG_VSYNC_END_PIX_SHIFT)

/*F layer dma zoom register*/
#define LCDC_CFG_DMAZM_H_PIXEL_SHIFT           (0)
#define LCDC_CFG_DMAZM_H_PIXEL(n)              (((n) & 0xFFF) << LCDC_CFG_DMAZM_H_PIXEL_SHIFT)
#define LCDC_CFG_DMAZM_H_PIXEL_MASK            (0xFFF << LCDC_CFG_DMAZM_H_PIXEL_SHIFT)

#define LCDC_CFG_DMAZM_V_LINE_SHIFT            (16)
#define LCDC_CFG_DMAZM_V_LINE(n)               (((n) & 0xFFF) << LCDC_CFG_DMAZM_V_LINE_SHIFT)
#define LCDC_CFG_DMAZM_V_LINE_MASK             (0xFFF << LCDC_CFG_DMAZM_V_LINE_SHIFT)

/*B layer dma zoom register*/
#define LCDC_CFG_GRAZM_H_PIXEL_SHIFT           (0)
#define LCDC_CFG_GRAZM_H_PIXEL(n)              (((n) & 0xFFF) << LCDC_CFG_GRAZM_H_PIXEL_SHIFT)
#define LCDC_CFG_GRAZM_H_PIXEL_MASK            (0xFFF << LCDC_CFG_GRAZM_H_PIXEL_SHIFT)

#define LCDC_CFG_GRAZM_V_LINE_SHIFT            (16)
#define LCDC_CFG_GRAZM_V_LINE(n)               (((n) & 0xFFF) << LCDC_CFG_GRAZM_V_LINE_SHIFT)
#define LCDC_CFG_GRAZM_V_LINE_MASK             (0xFFF << LCDC_CFG_GRAZM_V_LINE_SHIFT)

/*F Layer start point on screen config register   */
#define LCDC_CFG_DMA_OVSA_HPXL_SHIFT     (0)
#define LCDC_CFG_DMA_OVSA_HPXL(n)        (((n) & 0xFFF) << LCDC_CFG_DMA_OVSA_HPXL_SHIFT)
#define LCDC_CFG_DMA_OVSA_HPXL_MASK      (0xFFF << LCDC_CFG_DMA_OVSA_HPXL_SHIFT)

#define LCDC_CFG_DMAFRM_Y                (1 << 12)
#define LCDC_CFG_DMAFRM_U                (1 << 13)
#define LCDC_CFG_DMAFRM_V                (1 << 14)
#define LCDC_CFG_DMAFRM_C                (1 << 15)

#define LCDC_CFG_DMA_OVSA_VLN_SHIFT      (16)
#define LCDC_CFG_DMA_OVSA_VLN(n)         (((n) & 0xFFF) << LCDC_CFG_DMA_OVSA_VLN_SHIFT)
#define LCDC_CFG_DMA_OVSA_VLN_MASK       (0xFFF << LCDC_CFG_DMA_OVSA_VLN_SHIFT)
#define LCDC_CFG_DMAFRM_FIX              (1 << 31)

/*B Layer start point on screen  config  register   */
#define LCDC_CFG_GRA_OVSA_HPXL_SHIFT     (0)
#define LCDC_CFG_GRA_OVSA_HPXL(n)        (((n) & 0xFFF) << LCDC_CFG_GRA_OVSA_HPXL_SHIFT)
#define LCDC_CFG_GRA_OVSA_HPXL_MASK      (0xFFF << LCDC_CFG_GRA_OVSA_HPXL_SHIFT)

#define LCDC_CFG_GRA_OVSA_VLN_SHIFT      (16)
#define LCDC_CFG_GRAFRM_Y                (1 << 12)
#define LCDC_CFG_GRA_OVSA_VLN(n)         (((n) & 0xFFF) << LCDC_CFG_GRA_OVSA_VLN_SHIFT)
#define LCDC_CFG_GRA_OVSA_VLN_MASK       (0xFFF << LCDC_CFG_GRA_OVSA_VLN_SHIFT)
#define LCDC_CFG_GRAFRM_FIX              (1 << 31)

/*F/B Layer chromakey set Register*/
#define LCDC_CFG_PN_ALPHA_Y_SHIFT         (0)
#define LCDC_CFG_PN_ALPHA_Y(n)            (((n) & 0xFF) << LCDC_CFG_PN_ALPHA_Y_SHIFT)
#define LCDC_CFG_PN_ALPHA_Y_MASK          (0xFF << LCDC_CFG_PN_ALPHA_Y_SHIFT)

#define LCDC_CFG_PN_CKEY_Y_SHIFT          (8)
#define LCDC_CFG_PN_CKEY_Y(n)             (((n) & 0xFF) << LCDC_CFG_PN_CKEY_Y_SHIFT)
#define LCDC_CFG_PN_CKEY_Y_MASK           (0xFF << LCDC_CFG_PN_CKEY_Y_SHIFT)

#define LCDC_CFG_PN_CKEY_Y1_SHIFT         (16)
#define LCDC_CFG_PN_CKEY_Y1(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_Y1_SHIFT)
#define LCDC_CFG_PN_CKEY_Y1_MASK          (0xFF << LCDC_CFG_PN_CKEY_Y1_SHIFT)

#define LCDC_CFG_PN_CKEY_Y2_SHIFT         (24)
#define LCDC_CFG_PN_CKEY_Y2(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_Y2_SHIFT)
#define LCDC_CFG_PN_CKEY_Y2_MASK          (0xFF << LCDC_CFG_PN_CKEY_Y2_SHIFT)

#define LCDC_CFG_PN_ALPHA_U_SHIFT         (0)
#define LCDC_CFG_PN_ALPHA_U(n)            (((n) & 0xFF) << LCDC_CFG_PN_ALPHA_U_SHIFT)
#define LCDC_CFG_PN_ALPHA_U_MASK          (0xFF << LCDC_CFG_PN_ALPHA_U_SHIFT)

#define LCDC_CFG_PN_CKEY_U_SHIFT          (8)
#define LCDC_CFG_PN_CKEY_U(n)             (((n) & 0xFF) << LCDC_CFG_PN_CKEY_U_SHIFT)
#define LCDC_CFG_PN_CKEY_U_MASK           (0xFF << LCDC_CFG_PN_CKEY_U_SHIFT)

#define LCDC_CFG_PN_CKEY_U1_SHIFT         (16)
#define LCDC_CFG_PN_CKEY_U1(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_U1_SHIFT)
#define LCDC_CFG_PN_CKEY_U1_MASK          (0xFF << LCDC_CFG_PN_CKEY_U1_SHIFT)

#define LCDC_CFG_PN_CKEY_U2_SHIFT         (24)
#define LCDC_CFG_PN_CKEY_U2(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_U2_SHIFT)
#define LCDC_CFG_PN_CKEY_U2_MASK          (0xFF << LCDC_CFG_PN_CKEY_U2_SHIFT)

#define LCDC_CFG_PN_ALPHA_V_SHIFT         (0)
#define LCDC_CFG_PN_ALPHA_V(n)            (((n) & 0xFF) << LCDC_CFG_PN_ALPHA_V_SHIFT)
#define LCDC_CFG_PN_ALPHA_V_MASK          (0xFF << LCDC_CFG_PN_ALPHA_V_SHIFT)

#define LCDC_CFG_PN_CKEY_V_SHIFT          (8)
#define LCDC_CFG_PN_CKEY_V(n)             (((n) & 0xFF) << LCDC_CFG_PN_CKEY_V_SHIFT)
#define LCDC_CFG_PN_CKEY_V_MASK           (0xFF << LCDC_CFG_PN_CKEY_V_SHIFT)

#define LCDC_CFG_PN_CKEY_V1_SHIFT         (16)
#define LCDC_CFG_PN_CKEY_V1(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_V1_SHIFT)
#define LCDC_CFG_PN_CKEY_V1_MASK          (0xFF << LCDC_CFG_PN_CKEY_V1_SHIFT)

#define LCDC_CFG_PN_CKEY_V2_SHIFT         (24)
#define LCDC_CFG_PN_CKEY_V2(n)            (((n) & 0xFF) << LCDC_CFG_PN_CKEY_V2_SHIFT)
#define LCDC_CFG_PN_CKEY_V2_MASK          (0xFF << LCDC_CFG_PN_CKEY_V2_SHIFT)

/*Color key contrl*/
//Overlay color key mode
#define CKEY_Y_ENA           0x1
#define CKEY_U_ENA           0x2
#define CKEY_V_ENA           0x4
#define CKEY_YUV_ALL         0x5
#define CKEY_G_ENA           0x6
#define CKEY_B_ENA           0x7
#define CKEY_RGB_ALL         0x3
#define CKEY_DIS             0x0

/* LCDC PFC alpha mode */
#define LCDC_PFCCR_AM_FLPIXEL   0
#define LCDC_PFCCR_AM_BLPIXEL   1
#define LCDC_PFCCR_AM_CONST     2
#define LCDC_PFCCR_AM_NO        3

/* LCDC Dumb Register Address*/

#define LCDC_CFG_DUMB_ENA             (1 << 0)
#define LCDC_CFG_PN_INV_PCLK          (1 << 1)
#define LCDC_CFG_PN_INV_HSYNC         (1 << 2)
#define LCDC_CFG_PN_INV_VSYNC         (1 << 3)
#define LCDC_CFG_PN_INV_HENA          (1 << 4)
#define LCDC_CFG_PN_INV_COMPSYNC      (1 << 5)
#define LCDC_CFG_PN_INV_COMPBLANK     (1 << 6)
#define LCDC_CFG_PN_REVERSE_RGB       (1 << 7)
#define LCDC_CFG_PN_BIASOUT           (1 << 8)
#define LCDC_CFG_PN_NO_IOPAD          (1 << 9)
#define LCDC_RESERVED_1B8BIT10        (1 << 10)
#define LCDC_CFG_GRA_PMBURST          (1 << 11)
#define LCDC_CFG_LCDGPIO_ENA_SHIFT    (12)
#define LCDC_CFG_LCDGPIO_ENA(n)       (((n) & 0xFF) << LCDC_CFG_LCDGPIO_ENA_SHIFT)
#define LCDC_CFG_LCDGPIO_ENA_MASK     (0xFF << 12)
#define LCDC_CFG_LCDGPIO_O_SHIFT      (20)
#define LCDC_CFG_LCDGPIO_O(n)         (((n) & 0xFF) << LCDC_CFG_LCDGPIO_O_SHIFT)
#define LCDC_CFG_LCDGPIO_O_MASK       (0xFF << LCDC_CFG_LCDGPIO_O_SHIFT)
#define LCDC_CFG_DUMBMODE_SHIFT       (28)
#define LCDC_CFG_DUMBMODE(n)          (((n) & 0xF) << LCDC_CFG_DUMBMODE_SHIFT)
#define LCDC_CFG_DUMBMODE_MASK        (0xF << LCDC_CFG_DUMBMODE_SHIFT)

/* LCDC Dither Contrl Register Address*/

POSSIBLY_UNUSED static uint8_t dither_mode[] = {
    6,            //RGB 444 mode
    4,            //RGB 565 mode
    5,            //RGB 666 mode
    7            //RGB 555 mode
};

POSSIBLY_UNUSED static uint8_t dither_mode_high[] = {
    0,            //RGB 444 mode
    1,            //RGB 565 mode
    2,            //RGB 666 mode
    3,            //RGB 555 mode
};

#define LCDC_DITHER_EN_PN(n)               (((n) & 0x1) << 0)
#define LCDC_DITHER_EN_PN_MASK             (1 << 0)

#define LCDC_DITHER_4X8_PN(n)              (((n) & 0x1) << 1)
#define LCDC_DITHER_4X8_PN_MASK            (1 << 1)

#define LCDC_RESERVED_1ECBIT2_SHIFT        (2)
#define LCDC_RESERVED_1ECBIT2(n)           (((n) & 0x3) << LCDC_RESERVED_1ECBIT2_SHIFT)
#define LCDC_RESERVED_1ECBIT2_MASK         (0x3 << LCDC_RESERVED_1ECBIT2_SHIFT)

#define LCDC_DITHER_MODE_PN_SHIFT          (4)
#define LCDC_DITHER_MODE_PN(n)             (((n) & 0x7) << LCDC_DITHER_MODE_PN_SHIFT)
#define LCDC_DITHER_MODE_PN_MASK           (0x7 << LCDC_DITHER_MODE_PN_SHIFT)

#define LCDC_RESERVED_1ECBIT7              (1 << 7)
#define LCDC_DITHER_EN_TV                  (1 << 8)
#define LCDC_DITHER_4X8_TV                 (1 << 9)

#define LCDC_RESERVED_1ECBIT10_SHIFT       (10)
#define LCDC_RESERVED_1ECBIT10(n)          (((n) & 0x3) << LCDC_RESERVED_1ECBIT10_SHIFT)
#define LCDC_RESERVED_1ECBIT10_MASK        (0x3 << LCDC_RESERVED_1ECBIT10_SHIFT)

#define LCDC_DITHER_MODE_TV_SHIFT          (12)
#define LCDC_DITHER_MODE_TV(n)             (((n) & 0x7) << LCDC_DITHER_MODE_TV_SHIFT)
#define LCDC_DITHER_MODE_TV_MASK           (0x7 << LCDC_DITHER_MODE_TV_SHIFT)

#define LCDC_RESERVED_1ECBIT15             (1 << 15)

#define LCDC_DITHER_TABLE_INDEX_SEL_SHIFT  (16)
#define LCDC_DITHER_TABLE_INDEX_SEL(n)     (((n) & 0x3) << LCDC_DITHER_TABLE_INDEX_SEL_SHIFT)
#define LCDC_DITHER_TABLE_INDEX_SEL_MASK   (0x3 << LCDC_DITHER_TABLE_INDEX_SEL_SHIFT)

#define LCDC_RESERVED_1ECBIT18_SHIFT       (18)
#define LCDC_RESERVED_1ECBIT18(n)          (((n) & 0x3FFF) << LCDC_RESERVED_1ECBIT18_SHIFT)
#define LCDC_RESERVED_1ECBIT18_MASK        (0x3FFF << LCDC_RESERVED_1ECBIT18_SHIFT)

/* LCDC Dither TBL Register Address*/
#define LCDC_DITHER_TBL_DATA_SHIFT         (0)
#define LCDC_DITHER_TBL_DATA(n)            (((n) & 0xFFFFFFFF) << LCDC_DITHER_TBL_DATA_SHIFT)
#define LCDC_DITHER_TBL_DATA_MASK          (0xFFFFFFFF << LCDC_DITHER_TBL_DATA_SHIFT)

/* LCDC WBTG Control Register Address*/

#define LCDC_WDMA_IMG_WIDTH_SHIFT    (0)
#define LCDC_WDMA_IMG_WIDTH(n)       (((n) & 0x1FFF) << LCDC_WDMA_IMG_WIDTH_SHIFT)
#define LCDC_WDMA_IMG_WIDTH_MASK     (0x1FFF << LCDC_WDMA_IMG_WIDTH_SHIFT)

#define LCDC_RESERVED_204BIT13_SHIFT (13)
#define LCDC_RESERVED_204BIT13(n)    (((n) & 0x7) << LCDC_RESERVED_204BIT13_SHIFT)
#define LCDC_RESERVED_204BIT13_MASK  (0x7 << LCDC_RESERVED_204BIT13_SHIFT)

#define LCDC_WDMA_IMG_HEIGHT_SHIFT   (16)
#define LCDC_WDMA_IMG_HEIGHT(n)      (((n) & 0xFFF) << LCDC_WDMA_IMG_HEIGHT_SHIFT)
#define LCDC_WDMA_IMG_HEIGHT_MASK    (0xFFF << LCDC_WDMA_IMG_HEIGHT_SHIFT)

#define LCDC_WBTG_SHIFT              (28)
#define LCDC_WBTG(n)             (((n) & 0xF) << LCDC_WBTG_SHIFT)
#define LCDC_WBTG_MASK           (0xF << LCDC_WBTG_SHIFT)

/* LCDC F/B Chormkey Enable Register Address*/
//#define LCDC_CHOKYENACR      (lcdc_tv_ctrl1)
// reg_84
#define LCDC_CFG_TV_NI                (1 << 0)
#define LCDC_CFG_TV_RESETB            (1 << 1)
#define LCDC_CFG_TV_TVGOTOPN          (1 << 2)
#define LCDC_CFG_TV_PNGOTOTV          (1 << 3)
#define LCDC_CFG_IO_TVGOTOPN          (1 << 4)
#define LCDC_CFG_IO_PNGOTOTV          (1 << 5)
#define LCDC_CFG_TMSYNC_ENA           (1 << 6)
#define LCDC_RESERVED_84BIT7          (1 << 7)

#define LCDC_CFG_TV_ALPHA_SHIFT       (8)
#define LCDC_CFG_TV_ALPHA(n)          (((n) & 0xFF) << LCDC_CFG_TV_ALPHA_SHIFT)
#define LCDC_CFG_TV_ALPHA_MASK        (0xFF << LCDC_CFG_TV_ALPHA_SHIFT)

#define LCDC_CFG_TV_ALPHAMODE_SHIFT   (16)
#define LCDC_CFG_TV_ALPHAMODE(n)      (((n) & 0x3) << LCDC_CFG_TV_ALPHAMODE_SHIFT)
#define LCDC_CFG_TV_ALPHAMODE_MASK    (0x3 << LCDC_CFG_TV_ALPHAMODE_SHIFT)

#define LCDC_CFG_PNV_CKEY_DMA         (1 << 18)
#define LCDC_CFG_PNG_CKEY_GRA         (1 << 19)
#define LCDC_CFG_TV_CKEY_TVD          (1 << 20)
#define LCDC_CFG_TV_CKEY_TVG          (1 << 21)
#define LCDC_RESERVED_84BIT22         (1 << 22)
#define LCDC_CFG_TV_CARRY             (1 << 23)

#define LCDC_CFG_TV_CKEY_MODE_SHIFT   (24)
#define LCDC_CFG_TV_CKEY_MODE(n)      (((n) & 0x7) << LCDC_CFG_TV_CKEY_MODE_SHIFT)
#define LCDC_CFG_TV_CKEY_MODE_MASK    (0x7 << LCDC_CFG_TV_CKEY_MODE_SHIFT)

#define LCDC_CFG_TVSYN_INV            (1 << 27)

#define LCDC_RESERVED_84BIT28_SHIFT
#define LCDC_RESERVED_84BIT28(n)      (((n) & 0xF) << LCDC_RESERVED_84BIT28_SHIFT)
#define LCDC_RESERVED_84BIT28_MASK    (0xF << LCDC_RESERVED_84BIT28_SHIFT)

/*DSI config Register */

#define LCDC_CFG_FRAME_HALT_EN             (1 << 0)
#define LCDC_CFG_FRAME_HALT                (1 << 1)
#define LCDC_CFG_TEST_PATTERN_EN           (1 << 2)
#define LCDC_CFG_PATTERN_MODE(n)           (((n) & 0x7F) << 3)
#define LCDC_CFG_PATTERN_MODE_MASK         (0x7F << 3)
#define LCDC_CFG_PATTERN_MODE_SHIFT        (3)
#define LCDC_CFG_PATTERN_PIX_NUM(n)        (((n) & 0x7FF) << 10)
#define LCDC_CFG_PATTERN_PIX_NUM_MASK      (0x7FF << 10)
#define LCDC_CFG_PATTERN_PIX_NUM_SHIFT     (10)
#define LCDC_CFG_PATTERN_RGB_ORDER(n)      (((n) & 0x7) << 21)
#define LCDC_CFG_PATTERN_RGB_ORDER_MASK    (0x7 << 21)
#define LCDC_CFG_PATTERN_RGB_ORDER_SHIFT   (21)
#define LCDC_CFG_PATTERN_IS_CIR            (1 << 24)
#define LCDC_CFG_DSI_DATA_SWAP(n)          (((n) & 0x7) << 25)
#define LCDC_CFG_DSI_DATA_SWAP_MASK        (0x7 << 25)
#define LCDC_CFG_DSI_DATA_SWAP_SHIFT       (25)

#define LCDC_CFG_DSI_CFMT_SHIFT       (28)
#define LCDC_CFG_DSI_CFMT_MASK        ((0xF) << LCDC_CFG_DSI_CFMT_SHIFT )

#define LCDC_CFG_DSI_RGB888                (1 << 28)
#define LCDC_CFG_DSI_RGB666                (1 << 29)
#define LCDC_CFG_DSI_RGB565                (1 << 30)
#define LCDC_CFG_DSI_RGB101010             (1 << 31)

/*LCDC GAMMA Read Data Register Address*/
//#define LCDC_PNGAMMARDDR      (lcdc_pn_gamma_rddat)
#define LCDC_CFG_PN_GAMMA_RDDAT_SHIFT   (0)
#define LCDC_CFG_PN_GAMMA_RDDAT(n)      (((n) & 0xFF) << LCDC_CFG_PN_GAMMA_RDDAT_SHIFT)
#define LCDC_CFG_PN_GAMMA_RDDAT_MASK    (0xFF << LCDC_CFG_PN_GAMMA_RDDAT_SHIFT)
#define LCDC_RESERVED_15CBIT8_SHIFT     (8)
#define LCDC_RESERVED_15CBIT8(n)                (((n) & 0xFFFFFF) << LCDC_RESERVED_15CBIT8_SHIFT)
#define LCDC_RESERVED_15CBIT8_MASK              (0xFFFFFF << LCDC_RESERVED_15CBIT8_SHIFT)

/*LCDC Panel IO Register Address*/
//#define LCDC_PNIOCR      (cdc_pnio_ctrl)
#define LCD_CFG_IOPADMODE(n)                                (((n) & 0xF) << 0)
#define LCD_CFG_IOPADMODE_MASK                              (0xF << 0)
#define LCD_CFG_IOPADMODE_SHIFT                             (0)
#define LCD_CFG_CYC_BURST_LENGTH                            (1 << 4)
#define LCD_CFG_BOUNDARY                                    (1 << 5)
#define LCD_RESERVED_1BCBIT6_SHIFT                          (6)
#define LCD_RESERVED_1BCBIT6(n)                             (((n) & 0x3) << LCD_RESERVED_1BCBIT6_SHIFT)
#define LCD_RESERVED_1BCBIT6_MASK                           (0x3 << LCD_RESERVED_1BCBIT6_SHIFT)
#define LCD_CFG_PN_CSC(n)                                   (((n) & 0x3) << 8)
#define LCD_CFG_PN_CSC_MASK                                 (0x3 << 8)
#define LCD_CFG_PN_CSC_SHIFT                                (8)
#define LCD_CFG_TV_CSC(n)                                   (((n) & 0x3) << 10)
#define LCD_CFG_TV_CSC_MASK                                 (0x3 << 10)
#define LCD_CFG_TV_CSC_SHIFT                                (10)
#define LCD_CFG_CMD_VM_ENA                                  (1 << 12)
#define LCD_CFG_DMA_VM_ENA                                  (1 << 13)
#define LCD_RESERVED_1BCBIT14                               (1 << 14)
#define LCD_CFG_GRA_VM_ENA                                  (1 << 15)
#define LCD_RESERVED_1BCBIT16                               (1 << 16)
#define LCD_CFG_TVD_VM_ENA                                  (1 << 17)
#define LCD_RESERVED_1BCBIT18                               (1 << 18)
#define LCD_CFG_TVG_VM_ENA                                  (1 << 19)
#define LCD_CFG_SA_ENABLE                                   (1 << 20)
#define LCD_RESERVED_1BCBIT21                               (1 << 21)
#define LCD_CFG_TVD_SA_CMSK                                 (1 << 22)
#define LCD_CFG_DMA_SA_CMSK                                 (1 << 23)
#define LCD_CFG_TVG_SA_YMSK                                 (1 << 24)
#define LCD_CFG_TVD_SA_VMSK                                 (1 << 25)
#define LCD_CFG_TVD_SA_UMSK                                 (1 << 26)
#define LCD_CFG_TVD_SA_YMSK                                 (1 << 27)
#define LCD_CFG_GRA_SA_YMSK                                 (1 << 28)
#define LCD_CFG_DMA_SA_VMSK                                 (1 << 29)
#define LCD_CFG_DMA_SA_UMSK                                 (1 << 30)
#define LCD_CFG_DMA_SA_YMSK                                 (1 << 31)

#endif /* __LCDC_REGV1_DEFINE_H */
