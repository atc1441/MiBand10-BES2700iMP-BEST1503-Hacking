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


#ifndef __DMA2D_REG_DEFINE_H
#define __DMA2D_REG_DEFINE_H

#include "plat_types.h"
#include "plat_addr_map.h"

#define DMA2D_REG_BASE             IMGDMA_BASE

#define dma2d_tvd_start_addr_y0      (DMA2D_REG_BASE + (0x000))
#define dma2d_tvd_start_addr_u0      (DMA2D_REG_BASE + (0x004))
#define dma2d_tvd_start_addr_v0      (DMA2D_REG_BASE + (0x008))
#define cfg_tvd_y_squln_sa          (DMA2D_REG_BASE + (0x010))
#define cfg_tvd_u_squln_sa          (DMA2D_REG_BASE + (0x014))
#define cfg_tvd_v_squln_sa          (DMA2D_REG_BASE + (0x018))
#define dma2d_tvd_pitch_y          (DMA2D_REG_BASE + (0x020))
#define dma2d_tvd_pitch_uv          (DMA2D_REG_BASE + (0x024))
#define dma2d_tvd_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x028))
#define dma2d_tvd_hpxl_vln          (DMA2D_REG_BASE + (0x02C))
#define dma2d_tvdzm_hpxl_vln      (DMA2D_REG_BASE + (0x030))
#define dma2d_tvg_start_addr0      (DMA2D_REG_BASE + (0x034))
#define cfg_tvg_squln_sa          (DMA2D_REG_BASE + (0x038))
#define dma2d_tvg_pitch              (DMA2D_REG_BASE + (0x03C))
#define dma2d_tvg_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x040))
#define dma2d_tvg_hpxl_vln          (DMA2D_REG_BASE + (0x044))
#define dma2d_tvgzm_hpxl_vln      (DMA2D_REG_BASE + (0x048))
#define dma2d_tvc_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x04C))
#define dma2d_tvc_hpxl_vln          (DMA2D_REG_BASE + (0x050))
#define dma2d_tv_v_h_total          (DMA2D_REG_BASE + (0x054))
#define dma2d_tv_v_h_active          (DMA2D_REG_BASE + (0x058))
#define dma2d_tv_h_porch          (DMA2D_REG_BASE + (0x05C))
#define dma2d_tv_v_porch          (DMA2D_REG_BASE + (0x060))
#define dma2d_tv_blankcolor          (DMA2D_REG_BASE + (0x064))
#define dma2d_tv_alpha_color1      (DMA2D_REG_BASE + (0x068))
#define dma2d_tv_alpha_color2      (DMA2D_REG_BASE + (0x06C))
#define dma2d_tv_colorkey_y          (DMA2D_REG_BASE + (0x070))
#define dma2d_tv_colorkey_u          (DMA2D_REG_BASE + (0x074))
#define dma2d_tv_colorkey_v          (DMA2D_REG_BASE + (0x078))
#define dma2d_tv_sepxlcnt          (DMA2D_REG_BASE + (0x07C))
#define dma2d_tv_ctrl0              (DMA2D_REG_BASE + (0x080))
#define dma2d_tv_ctrl1              (DMA2D_REG_BASE + (0x084))
#define dma2d_tv_contrast          (DMA2D_REG_BASE + (0x088))
#define dma2d_tv_saturation          (DMA2D_REG_BASE + (0x08C))
#define dma2d_tv_cbsh_hue          (DMA2D_REG_BASE + (0x090))
#define dma2d_tvif_ctrl              (DMA2D_REG_BASE + (0x094))
#define dma2d_tviopad_ctrl          (DMA2D_REG_BASE + (0x098))
#define dma2d_tclk_div              (DMA2D_REG_BASE + (0x09C))

/*dma graphic layer start address register */
#define dma2d_dma_start_addr_y0      (DMA2D_REG_BASE + (0x0C0))
#define dma2d_dma_start_addr_u0      (DMA2D_REG_BASE + (0x0C4))
#define dma2d_dma_start_addr_v0      (DMA2D_REG_BASE + (0x0C8))
#define dma2d_dma_start_addr_c0      (DMA2D_REG_BASE + (0x0CC))

#define cfg_dma_y_squln_sa          (DMA2D_REG_BASE + (0x0D0))
#define cfg_dma_u_squln_sa          (DMA2D_REG_BASE + (0x0D4))
#define cfg_dma_v_squln_sa          (DMA2D_REG_BASE + (0x0D8))
#define cfg_dma_c_squln_sa          (DMA2D_REG_BASE + (0x0DC))

#define dma2d_dma_pitch_y          (DMA2D_REG_BASE + (0x0E0))
#define dma2d_dma_pitch_uv          (DMA2D_REG_BASE + (0x0E4))
#define dma2d_dma_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x0E8))
#define dma2d_dma_hpxl_vln          (DMA2D_REG_BASE + (0x0EC))
#define dma2d_dmazm_hpxl_vln      (DMA2D_REG_BASE + (0x0F0))

/*gra graphic layer start address register */
#define dma2d_gra_start_addr0      (DMA2D_REG_BASE + (0x0F4))
#define cfg_gra_squln_sa          (DMA2D_REG_BASE + (0x0F8))
#define dma2d_gra_pitch              (DMA2D_REG_BASE + (0x0FC))
#define dma2d_gra_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x100))
#define dma2d_gra_hpxl_vln          (DMA2D_REG_BASE + (0x104))
#define dma2d_grazm_hpxl_vln      (DMA2D_REG_BASE + (0x108))
#define dma2d_hwc_ovsa_hpxl_vln      (DMA2D_REG_BASE + (0x10C))
#define dma2d_hwc_hpxl_vln          (DMA2D_REG_BASE + (0x110))
#define dma2d_pn_v_h_total          (DMA2D_REG_BASE + (0x114))
#define dma2d_pn_v_h_active          (DMA2D_REG_BASE + (0x118))
#define dma2d_pn_h_porch          (DMA2D_REG_BASE + (0x11C))
#define dma2d_pn_v_porch          (DMA2D_REG_BASE + (0x120))
#define dma2d_pn_blankcolor          (DMA2D_REG_BASE + (0x124))
#define dma2d_pn_alpha_color1      (DMA2D_REG_BASE + (0x128))
#define dma2d_pn_alpha_color2      (DMA2D_REG_BASE + (0x12C))
#define dma2d_pn_colorkey_y          (DMA2D_REG_BASE + (0x130))
#define dma2d_pn_colorkey_u          (DMA2D_REG_BASE + (0x134))
#define dma2d_pn_colorkey_v          (DMA2D_REG_BASE + (0x138))
#define dma2d_pn_sepxlcnt          (DMA2D_REG_BASE + (0x13C))
#define dma2d_dmavld_y              (DMA2D_REG_BASE + (0x14C))
#define dma2d_dmavld_uv              (DMA2D_REG_BASE + (0x150))
#define dma2d_tvggravld_hlen      (DMA2D_REG_BASE + (0x154))
#define dma2d_hwc_rddat              (DMA2D_REG_BASE + (0x158))
#define dma2d_pn_gamma_rddat      (DMA2D_REG_BASE + (0x15C))
#define dma2d_pn_clut_rddat          (DMA2D_REG_BASE + (0x160))
#define dma2d_tvdvld_y              (DMA2D_REG_BASE + (0x168))
#define dma2d_tvdvld_uv              (DMA2D_REG_BASE + (0x16C))
#define dma2d_tvc_rddat              (DMA2D_REG_BASE + (0x170))
#define dma2d_tv_gamma_rddat      (DMA2D_REG_BASE + (0x174))
#define dma2d_tv_clut_rddat          (DMA2D_REG_BASE + (0x178))
#define dma2d_frame_cnt              (DMA2D_REG_BASE + (0x17C))
#define dma2d_smpn_ctrl              (DMA2D_REG_BASE + (0x188))
#define dma2d_pn_ctrl0              (DMA2D_REG_BASE + (0x190))
#define dma2d_pn_ctrl1              (DMA2D_REG_BASE + (0x194))
#define dma2d_sram_ctrl              (DMA2D_REG_BASE + (0x198))
#define dma2d_sram_wrdat          (DMA2D_REG_BASE + (0x19C))
#define dma2d_sram_para0          (DMA2D_REG_BASE + (0x1A0))
#define dma2d_sram_para1          (DMA2D_REG_BASE + (0x1A4))
#define dma2d_sclk_div              (DMA2D_REG_BASE + (0x1A8))
#define dma2d_pn_contrast          (DMA2D_REG_BASE + (0x1AC))
#define dma2d_pn_saturation          (DMA2D_REG_BASE + (0x1B0))
#define dma2d_pn_cbsh_hue          (DMA2D_REG_BASE + (0x1B4))
#define dma2d_dumb_ctrl              (DMA2D_REG_BASE + (0x1B8))
#define spu_irq_ena_tb              (DMA2D_REG_BASE + (0x1C0))
#define spu_irq_isr                  (DMA2D_REG_BASE + (0x1C4))
#define spu_irq_rsr                  (DMA2D_REG_BASE + (0x1C8))
#define dma2d_gra_cuthpxl          (DMA2D_REG_BASE + (0x1CC))
#define dma2d_gra_cutvln          (DMA2D_REG_BASE + (0x1D0))
#define dma2d_tvg_cuthpxl          (DMA2D_REG_BASE + (0x1D4))
#define dma2d_tvg_cutvln          (DMA2D_REG_BASE + (0x1D8))
#define dma2d_top_ctrl              (DMA2D_REG_BASE + (0x1DC))
#define dma2d_pn_squln_ctrl          (DMA2D_REG_BASE + (0x1E0))
#define dma2d_tv_squln_ctrl          (DMA2D_REG_BASE + (0x1E4))
#define dma2d_afa_all2one_ctrl      (DMA2D_REG_BASE + (0x1E8))
#define dma2d_dither_ctrl          (DMA2D_REG_BASE + (0x1EC))
#define dma2d_dither_tbl_data      (DMA2D_REG_BASE + (0x1F0))
#define dma2d_misc_ctrl              (DMA2D_REG_BASE + (0x1F8))
#define dma2d_wdma_ctrl              (DMA2D_REG_BASE + (0x200))
#define dma2d_wdma_v_h              (DMA2D_REG_BASE + (0x204))
#define dma2d_wdma_start_addr     (DMA2D_REG_BASE + (0x208))
#define dma2d_tv_v_blank          (DMA2D_REG_BASE + (0x20C))
#define dma2d_tv_v_h_total_fld      (DMA2D_REG_BASE + (0x210))
#define dma2d_tv_v_porch_fld      (DMA2D_REG_BASE + (0x214))
#define dma2d_tv_sepxlcnt_fld      (DMA2D_REG_BASE + (0x218))
#define cfg_h_front_porch_smpn      (DMA2D_REG_BASE + (0x21C))
#define cfg_vsync_start_pix          (DMA2D_REG_BASE + (0x220))
#define dma2d_gen_frame_start      (DMA2D_REG_BASE + (0x224))
#define vdma_squ_w_burst_cnt      (DMA2D_REG_BASE + (0x22C))
#define vdma_squ_line_rdy          (DMA2D_REG_BASE + (0x230))
#define dma2d_version              (DMA2D_REG_BASE + (0x240))
#define dma2d_pn_cmap_cfgcr       (DMA2D_REG_BASE + (0x24c))

#define dma2d_tvd_ctrl            (DMA2D_REG_BASE + (0x260))
#define dma2d_pn_gra_ctrl         (DMA2D_REG_BASE + (0x264))

#define dma2d_vdma_sel_ctrl          (DMA2D_REG_BASE + (0x2F0))
#define dma2d_vdma_burst_cnt_rdy  (DMA2D_REG_BASE + (0x2F4))
#define timing_master_control      (DMA2D_REG_BASE + (0x2F8))
#define dsi_in_mux_ctl              (DMA2D_REG_BASE + (0x2FC))
#define dma2d_crc1_ctrl              (DMA2D_REG_BASE + (0x380))
#define dma2d_crc1_blk_size          (DMA2D_REG_BASE + (0x384))
#define dma2d_crc1_intc_treshld      (DMA2D_REG_BASE + (0x388))
#define dma2d_crc1_sram_power_ctrl  (DMA2D_REG_BASE + (0x38C))
#define dma2d_crc2_ctrl              (DMA2D_REG_BASE + (0x3C0))
#define dma2d_crc2_blk_size          (DMA2D_REG_BASE + (0x3C4))
#define dma2d_crc2_intc_treshld      (DMA2D_REG_BASE + (0x3C8))
#define dma2d_crc2_sram_power_ctrl  (DMA2D_REG_BASE + (0x3CC))

#define FAR

#define DMA2D_NCLUT           256    /* Number of entries in the CLUT */

/* DMA2D Register Addresses */

/*DMA2D Start Register Address*/
#define DMA2D_STARTCR            (dma2d_gen_frame_start)
#define DMA2D_PANEL_SCLKCR       (dma2d_sclk_div)

/*DMA2D Swap Panel Path Processing Register Address*/
#define DMA2D_SWAPCR             (dma2d_top_ctrl)

/*DMA2D Blend Register Address*/
#define DMA2D_BLDCR              (dma2d_afa_all2one_ctrl)

/*Layer Size Register Address*/
#define DMA2D_PDMAAWCR           (dma2d_dma_hpxl_vln)
#define DMA2D_PGRAAWCR           (dma2d_gra_hpxl_vln)
#define DMA2D_SPUAWCR            (dma2d_pn_v_h_active)
#define DMA2D_WBAWCR             (dma2d_wdma_v_h)

/*F/B Layer VH Register Address*/
#define DMA2D_PVH_TOTAL_CR             (dma2d_pn_v_h_total)

/*F/B Layer HPorchRegister Address*/
#define DMA2D_PHPORCH_CR             (dma2d_pn_h_porch)

/*F/B Layer VPorchRegister Address*/
#define DMA2D_PVPORCH_CR               (dma2d_pn_v_porch)

/*Layer pitch Register Address*/
#define DMA2D_PDMAPCHCR           (dma2d_dma_pitch_y)
#define DMA2D_PGRAPCHCR           (dma2d_gra_pitch)
#define DMA2D_SPUPCHCR            (NULL)
#define DMA2D_WBPCHCR             (dma2d_wdma_ctrl)

/*OUT Layer dma burst Register Address*/
#define DMA2D_WBBURSTCR             (dma2d_wdma_ctrl)

/*Layer pfc Register Address*/
#define DMA2D_FGPFCCR         (dma2d_pn_ctrl0)
#define DMA2D_FGPFCCR1        (dma2d_pn_ctrl1)
#define DMA2D_BGPFCCR         (dma2d_pn_gra_ctrl)
#define DMA2D_OPFCCR          (dma2d_wdma_ctrl)

/*F/B Layer dma zoom Register Address*/
#define DMA2D_DMAZOOMCR       (dma2d_dmazm_hpxl_vln)
#define DMA2D_GRAZOOMCR       (dma2d_grazm_hpxl_vln)

/*Layer Start mem adress Register Address*/
#define DMA2D_FGMAR           (dma2d_dma_start_addr_y0)
#define DMA2D_BGMAR           (dma2d_gra_start_addr0)
#define DMA2D_OMAR            (dma2d_wdma_start_addr)

/*Layer Color Register Address*/
#define DMA2D_FGCOLR          (NULL)
#define DMA2D_BGCOLR          (NULL)
#define DMA2D_OCOLR           (NULL)
#define DMA2D_SPUCOLR         (dma2d_pn_blankcolor)

/*DMA2d irq control Register Address*/
#define DMA2D_IER              (spu_irq_ena_tb)
#define DMA2D_ISR              (spu_irq_isr)
#define DMA2D_ICR              (spu_irq_rsr)

#define DMA2D_DISPCNFCR        (dma2d_tv_v_h_total_fld)

#define DMA2D_SMPNCR           (dma2d_smpn_ctrl)

/* DMA2D WBTG Control Register Address*/

#define DMA2D_WBFPTCR             (dma2d_wdma_v_h)    //frame per toggle

#define DMA2D_STBY_VSYNCCR        (dma2d_tv_sepxlcnt_fld)
#define DMA2D_PORCH_VSYNCCR       (cfg_h_front_porch_smpn)
#define DMA2D_VSYNC_PIXCR         (cfg_vsync_start_pix)

#define DMA2D_CFG_PDMA_Y_SQULN_START     (dma2d_dma_ovsa_hpxl_vln)
#define DMA2D_CFG_PDMA_U_SQULN_START     (dma2d_hwc_ovsa_hpxl_vln)

#define DMA2D_CFG_PDMA_SPONSCREENCR     (dma2d_dma_ovsa_hpxl_vln)
#define DMA2D_CFG_PGRA_SPONSCREENCR     (dma2d_gra_ovsa_hpxl_vln)

/* DMA2D F/B CLUT Table Set Register Address*/
#define DMA2D_FBLCLUTCR     (dma2d_sram_ctrl)
#define DMA2D_FBLCLUTWRDR    (dma2d_sram_wrdat)

/* DMA2D Dither Register Address*/
#define DMA2D_DITHERCR        (dma2d_dither_ctrl)
#define DMA2D_DITHERDR        (dma2d_dither_tbl_data)

/* DMA2D Clock Disable Register Address*/
#define DMA2D_DCLKCR        (dma2d_pn_clut_rddat)

/* DMA2D Clock Enable Register Address*/
#define DMA2D_ECLKCR        (dma2d_pn_clut_rddat + 4)

/* DMA2D Register Bit Definitions */

/* DMA2D Start Register  */
#define DMA2D_STARTCR_SHIFT         (0)    /* Bits 0: Start dma2d process  */
#define DMA2D_STARTCR_MASK          (0x1 << DMA2D_STARTCR_SHIFT)
#define DMA2D_STARTCR_ENA(n)        ((uint32_t)(n) << DMA2D_STARTCR_SHIFT)

/* DMA2D Swap Panel Path Processing Register */
#define DMA2D_SWAPPPPCR_ASEL_SHIFT       (17)    /*Bits 17: Swap Panel Path Processing */
/* 0 = Panel path video processing is for video DMA, palette table is for graphic DMA */
/* 1 = Panel path video processing is for graphic DMA, palette table is for video DMA */

#define DMA2D_SWAPPPPCR_ASEL_MASK        (0x1 << DMA2D_SWAPPPPCR_ASEL_SHIFT)
#define DMA2D_SWAPPPPCR_ASEL(n)          ((uint32_t)(n) << DMA2D_SWAPPPPCR_ASEL_SHIFT)

#define DMA2D_SWAPPPPCR_ASEL_ENA_SHIFT   (16)    /*Bits 16: Swap Panel Path Enable
                           0 = Auto detect Panel path video processing path (<Swap Panel Path Processing> field is ignored)
                           1 = Panel path video processing is selected by the <Swap Panel Path Processing> field
                         */
#define DMA2D_SWAPPPPCR_ASEL_ENA_MASK        (0x1 << DMA2D_SWAPPPPCR_ASEL_ENA_SHIFT)
#define DMA2D_SWAPPPPCR_ASEL_ENA(n)          ((uint32_t)(n) << DMA2D_SWAPPPPCR_ASEL_ENA_SHIFT)

/* DMA2D Blend Register */
#define DMA2D_BLDCR_FIRSTLSEL_SHIFT       (0)    /*Bits 0-1: Alpha Blending First Layer Selection */
/*  This field is used when more than two layers are overlaid. It selects the first layer */
/*  0x0 = Top layer is Panel path video DMA */
/*  0x1 = Top layer is Panel path graphic DMA */
/*  0x2 = Top layer is TV path video DMA */
/*  0x3 = Top layer is TV path graphic DMA */

#define DMA2D_BLDCR_FIRSTLSEL_MASK        (0x3 << DMA2D_BLDCR_FIRSTLSEL_SHIFT)
#define DMA2D_BLDCR_FIRSTLSEL(n)          ((uint32_t)(n) << DMA2D_BLDCR_FIRSTLSEL_SHIFT )

#define DMA2D_BLDCR_SECONDLSEL_SHIFT (2)    /*Bits 2-3:Alpha Blending Second Layer Selection */
/* This field is used when more than two layers are overlaid. It selects the second layer. */
/* 0x0 = Second layer is Panel path video DMA */
/* 0x1 = Second layer is Panel path graphic DMA */
/* x2 = Second layer is TV path video DMA */
/* 0x3 = second layer is TV graphic DMA */
#define DMA2D_BLDCR_SECONDLSEL_MASK        (0x3 << DMA2D_BLDCR_SECONDLSEL_SHIFT)
#define DMA2D_BLDCR_SECONDLSEL(n)          ((uint32_t)(n) << DMA2D_BLDCR_SECONDLSEL_SHIFT )

/* DMA2D Active Width Configuration Register */
#define DMA2D_AWCR_AAW_SHIFT (0)    /* Bits 0-11: Accumulated Active width (scan lines) */
                            /* Panel Path Screen Horizontal  Pixels<p>This field sets */
                            /* the  horizontal screen display width for both Dumb Panel and Smart Panel. */
#define DMA2D_AWCR_AAW_MASK          (0x7ff << DMA2D_AWCR_AAW_SHIFT)
#define DMA2D_AWCR_AAW(n)          ((uint32_t)(n) << DMA2D_AWCR_AAW_SHIFT)
#define DMA2D_AWCR_AAH_SHIFT         (16)    /* Bits 16-27: Accumulated Active height (pixel clocks) */
                                /* Panel Path Screen  Vertical Lines<p>This field sets the */
                                /* vertical screen display size for both Dumb Panel and Smart Panel. */
#define DMA2D_AWCR_AAH_MASK        (0xfff << DMA2D_AWCR_AAH_SHIFT)
#define DMA2D_AWCR_AAH(n)          ((uint32_t)(n) << DMA2D_AWCR_AAH_SHIFT)

/* DMA2D Interrupt Enable Register */
#define FGL_PDMA_FRAME_IRQ0_ENA_SHIFT      (31)    /* Bits 31: Panel Path DMA Frame 0 Done IRQ Enable */
#define FGL_PDMA_FRAME_IRQ0_ENA_MASK       (0x01<< DMA_FRAME_IRQ0_ENA_SHIFT)
#define FGL_PDMA_FRAME_IRQ0_ENA            (1<<DMA_FRAME_IRQ0_ENA_SHIFT)

#define FGL_PDMA_FRAME_IRQ1_ENA   (1 <<30)    /* Panel Path DMA Frame 1 Done IRQ Enable */
#define FGL_PDMA_FF_UNDERFLOW_ENA (1 << 29)    /* Panel Path DMA FIFO Underflow IRQ Enable */
#define DMA2D_M2A_IRQ_ENA         (1 << 28)    /*  AXI Bus Error IRQ Enable */
#define BGL_GRA_FRAME_IRQ0_ENA    (1 << 27)    /* Panel Path Graphic Frame 0 IRQ Enable */
#define BGL_GRA_FRAME_IRQ1_ENA    (1 << 26)    /* Panel Path Graphic Frame 1 IRQ Enable */
#define BGL_GRA_FF_UNDERFLOW_ENA  (1 << 25)    /* Panel Path Graphic FIFO Underflow IRQ Enable */

#define SPU_FRAMEDONE_IRQ_ENA_SHIFT      (21)    /* Bits 21: Panel Path Smart Panel Frame Done IRQ Enable */
#define SPU_FRAMEDONE_IRQ_ENA_MASK       (0x01<< SPU_FRAMEDONE_IRQ_ENA_SHIFT)
#define SPU_FRAMEDONE_IRQ_ENA  (1<<SPU_FRAMEDONE_IRQ_ENA_SHIFT)
#define SPU_FRAMEDONE_ENA      (1 << SPU_FRAMEDONE_IRQ_ENA_SHIFT)    /*Panel Path Smart Panel Frame Done IRQ Enable */

#define DMA2D_PWRDN_IRQ_ENA     ( 1 << 17)    /* Power Down Request IRQ Enable */
#define DMA2D_ERR_IRQ_ENA       (1 << 16)    /* AXI Bus Latency Too Long IRQ Enable */

/* DMA2D Interrupt Status Register */
#define FGL_PDMA_FRAME_IRQ0     (1 << 31)    /* Panel Path Video Frame 0 Done Rising Edge IRQ */
#define FGL_DMA_FRAME_IRQ1      (1 << 30)    /* Panel Path Video Frame 1 Done Rising Edge IRQ */
#define FGL_DMA_FF_UNDERFLOW    (1 << 29)    /* Panel Path Video FIFO Underflow IRQ */
#define DMA2D_M2A_IRQ           (1 << 28)    /* AXI Bus Error IRQ */
#define BGL_GRA_FRAME_IRQ0      (1 << 27)    /* Panel Path Graphic Frame 0 Done Rising Edge IRQ */
#define BGL_GRA_FRAME_IRQ1      (1 << 26)    /* Panel Path Graphic Frame 1 Done Rising Edge IRQ */
#define BGL_GRA_FF_UNDERFLOW    (1 << 25)    /* Panel Path Graphic FIFO Underflow IRQ */
#define SPU_TXC_FRAMEDONE       (1 << 21)    /* Panel Path Smart Panel Display Area DMA Done IRQ */
#define DMA2D_PWRDN_IRQ         (1 << 17)    /* Power Down Request IRQ */
#define DMA2D_ERR_IRQ           (1 << 16)    /* AXI Bus Latency Too Long IRQ */
#define FGL_DMA_FF_EMPY         (1 << 3)    /* Level of DMA_FF_EMPY */
#define BGL_GRA_FF_EMPTY        (1 << 2)    /* Level of GRA_FF_EMPTY */

/* DMA2D Interrupt Flag Clear Register */
#define FGL_DMA_FRAME_IRQ0_CLEAN     (1<<31)    /* Clean Panel Path DMA Frame 0 Done IRQ FLAG */
#define FGL_DMA_FRAME_IRQ1_CLEAN     (1 <<30)    /* Clean Panel Path DMA Frame 1 Done IRQ FLAG */
#define FGL_DMA_FF_UNDERFLOW_CLEAN   (1 << 29)    /* Clean Panel Path DMA FIFO Underflow IRQ FLAG */
#define DMA2D_M2A_IRQ_CLEAN          (1 << 28)    /* Clean  AXI Bus Error IRQ FLAG */
#define BGL_GRA_FRAME_IRQ0_CLEAN     (1 << 27)    /* Clean Panel Path Graphic Frame 0 IRQ FLAG */
#define BGL_GRA_FRAME_IRQ1_CLEAN     (1 << 26)    /* Clean Panel Path Graphic Frame 1 IRQ FLAG */
#define BGL_GRA_FF_UNDERFLOW_CLEAN   (1 << 25)    /* Clean Panel Path Graphic FIFO Underflow IRQ FLAG */
#define SPU_FRAMEDONE_CLEAN          (1 << 21)    /* Clean Panel Path Smart Panel Frame Done IRQ FLAG */
#define DMA2D_PWRDN_IRQ_CLEAN        ( 1 << 17)    /* Clean Power Down Request IRQ FLAG */
#define DMA2D_ERR_IRQ_CLEAN          (1 << 16)    /* Clean AXI Bus Latency Too Long IRQ FLAG */

/* DMA2D Foreground/Background PFC Control Register */
#define DMA2D_xGPFCCR_LENA_SHIFT      (0)    /* Bits 0  layer enable  */
#define DMA2D_xGPFCCR_LENA_MASK       (0x01 << DMA2D_xGPFCCR_LENA_SHIFT)
#define DMA2D_xGPFCCR_LENA(n)         ((uint32_t)(n) << DMA2D_xGPFCCR_LENA_SHIFT)

#define DMA2D_CFG_YUV2RGB_ENA_SHIFT   (1)
#define DMA2D_CFG_YUV2RGB_ENA_MASK    ( 0x01 << DMA2D_CFG_YUV2RGB_ENA_SHIFT)
#define DMA2D_CFG_YUV2RGB_ENA(n)      ( (0x01 &(n)) << DMA2D_CFG_YUV2RGB_ENA_SHIFT)

#define DMA2D_xGPFCCR_LENA_SHIFT      (0)    /* Bits 0  layer enable  */
#define DMA2D_xGPFCCR_LENA_MASK       (0x01 << DMA2D_xGPFCCR_LENA_SHIFT)
#define DMA2D_xGPFCCR_LENA(n)         ((uint32_t)(n) << DMA2D_xGPFCCR_LENA_SHIFT)

#define DMA2D_xGPFCCR_CM_SHIFT      (8)    /* Bits 8-11 Color Mode */
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

                            /* Panel Path Graphic Memory Color Format   */
                            /* 0x0 = RGB565                          */
                            /* 0x1 = RGB1555  */
                            /* 0x2 = RGB888 packed  */
                            /* 0x3 = RGB888 unpacked  */
                            /* 0x4 = ARGB  */
                            /* 0x5 = YUV422 packed  */
                            /* 0x6 to 0x8 = Reserved  */
                            /* 0x9 = Palette color 4-bit per pixel  */
                            /* 0xA = Palette color 8-bit per pixel  */
                            /* 0xb = RGB8A   */
                            /* All other values are reserved. */

#define DMA2D_xGPFCCR1_TRANS_TRIGGER_SHIFT      (31)    /* Bits 31: Panel Path DMA Transfer Trigger
                               1 = DMA transfer is triggered, this is equivalent to VSYNC pulse
                               This field is useful when Smart Panel VSYNC input is not available.
                             */
#define DMA2D_xGPFCCR1_TRANS_TRIGGER_MASK      (1 << DMA2D_xGPFCCR1_TRANS_TRIGGER_SHIFT)
#define DMA2D_xGPFCCR1_TRANS_TRIGGER_ENA(n)    ((uint32_t)(n) << DMA2D_xGPFCCR1_TRANS_TRIGGER_SHIFT)

#define DMA2D_xGPFCCR_CM_MASK        (0xF << DMA2D_xGPFCCR_CM_SHIFT)
#define DMA2D_xGPFCCR_CM(n)          ((uint32_t)(n) << DMA2D_xGPFCCR_CM_SHIFT)
#define DMA2D_xGPFCCR1_CCM           (0x07 << 24)    /* CLUT Color Mode */
#define DMA2D_xGPFCCR_START          (1 << 5)    /* Start */
#define DMA2D_xGPFCCR1_AM_SHIFT      (16)    /* Bits 16-17 Alpha Mode */
#define DMA2D_xGPFCCR1_AM_MASK       (3 << DMA2D_xGPFCCR1_AM_SHIFT)
#define DMA2D_xGPFCCR1_AM(n)         ((uint32_t)(n) << DMA2D_xGPFCCR1_AM_SHIFT)
#define DMA2D_xGPFCCR1_ALPHA_SHIFT   (8)    /* Bits 8-15 Alpha Value */
#define DMA2D_xGPFCCR1_ALPHA_MASK    (0xFF << DMA2D_xGPFCCR1_ALPHA_SHIFT)
#define DMA2D_xGPFCCR1_ALPHA(n)      ((uint32_t)(n) << DMA2D_xGPFCCR1_ALPHA_SHIFT)

#define DMA2D_xGPFCCR_SWAPRB_ENA_SHIFT   (4)    /* Bits 4: Panel Path Graphic DMA Swap R and B
                           This field swaps R and B. For example, RGB is swapped to BGR.
                           1 = Swap enabled
                           0 = Swap disabled
                         */
#define DMA2D_xGPFCCR_SWAPRB_ENA_MASK    (0x01 << DMA2D_xGPFCCR_SWAPRB_ENA_SHIFT)
#define DMA2D_xGPFCCR_SWAPRB_ENA(n)      ((uint32_t)(n) << DMA2D_xGPFCCR_SWAPRB_ENA_SHIFT)

/*F/B Layer VH Register */
#define DMA2D_CFG_PN_H_TOTAL_SHIFT        (0)
#define DMA2D_CFG_PN_H_TOTAL(n)           (((n) & 0xFFF) << DMA2D_CFG_PN_H_TOTAL_SHIFT)
#define DMA2D_CFG_PN_H_TOTAL_MASK         (0xFFF << DMA2D_CFG_PN_H_TOTAL_SHIFT)

#define DMA2D_CFG_PN_V_TOTAL_SHIFT        (16)
#define DMA2D_CFG_PN_V_TOTAL(n)           (((n) & 0xFFF) << DMA2D_CFG_PN_V_TOTAL_SHIFT)
#define DMA2D_CFG_PN_V_TOTAL_MASK         (0xFFF << DMA2D_CFG_PN_V_TOTAL_SHIFT)

/*F/B Layer HPorchRegister */
#define DMA2D_CFG_PN_H_FPORCH_SHIFT       (0)
#define DMA2D_CFG_PN_H_FPORCH(n)          (((n) & 0xFFF) << DMA2D_CFG_PN_H_FPORCH_SHIFT)
#define DMA2D_CFG_PN_H_FPORCH_MASK        (0xFFF << DMA2D_CFG_PN_H_FPORCH_SHIFT)

#define DMA2D_CFG_PN_H_BPORCH_SHIFT       (16)
#define DMA2D_CFG_PN_H_BPORCH(n)          (((n) & 0xFFF) << DMA2D_CFG_PN_H_BPORCH_SHIFT)
#define DMA2D_CFG_PN_H_BPORCH_MASK        (0xFFF << DMA2D_CFG_PN_H_BPORCH_SHIFT)

/*F/B Layer VPorchRegister */
#define DMA2D_CFG_PN_V_FPORCH_SHIFT       (0)
#define DMA2D_CFG_PN_V_FPORCH(n)          (((n) & 0xFFF) << DMA2D_CFG_PN_V_FPORCH_SHIFT)
#define DMA2D_CFG_PN_V_FPORCH_MASK        (0xFFF << DMA2D_CFG_PN_V_FPORCH_SHIFT)

#define DMA2D_CFG_PN_V_BPORCH_SHIFT       (16)
#define DMA2D_CFG_PN_V_BPORCH(n)          (((n) & 0xFFF) << DMA2D_CFG_PN_V_BPORCH_SHIFT)
#define DMA2D_CFG_PN_V_BPORCH_MASK        (0xFFF << DMA2D_CFG_PN_V_BPORCH_SHIFT)

/*Display module config Register */
#define DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT   (0)    /* Bits 0: cfg_vsync_trig_disable_s */
#define DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE_MASK    (0x01 << DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT)
#define DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE(n)      ((uint32_t)(n) << DMA2D_DISPCNFCR_SVSYNCTRIG_DISABLE_SHIFT)

#define DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT   (1)    /* Bits 1: cfg_vsync_trig_disable_s */
#define DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE_MASK    (0x01 << DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT)
#define DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE(n)      ((uint32_t)(n) << DMA2D_DISPCNFCR_AVSYNCTRIG_DISABLE_SHIFT)

#define DMA2D_DISPCNFCR_MEM_ENABLE_SHIFT   (30)    /* Bits 30: disp memory enable */
#define DMA2D_DISPCNFCR_MEM_ENABLE_MASK    (0x01 << DMA2D_DISPCNFCR_MEM_ENABLE_SHIFT)
#define DMA2D_DISPCNFCR_MEM_ENABLE(n)      ((uint32_t)(n) << DMA2D_DISPCNFCR_MEM_ENABLE_SHIFT)

//SMPU Module Config_Register
#define DMA2D_SMPNCR_MODE_SHIFT   (8)    /* Bits 8-11 Alpha Value */
#define DMA2D_SMPNCR_MODE_MASK    (0xF << DMA2D_SMPNCR_MODE_SHIFT)
#define DMA2D_SMPNCR_MODE_SET(n)      ((uint32_t)(n) << DMA2D_SMPNCR_MODE_SHIFT)

/* DMA2D F/B CLUT Table Set Register */

#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_SHIFT     (0)
#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR(n)        (((n) & 0xFF) << 0)
#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_MASK      (0xFF << 0)

#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_SHIFT      (8)
#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_LCDID(n)         (((n) & 0xF) << 8)
#define DMA2D_FBLCLUTTABLE_SETSRAM_ADDR_LCDID_MASK       (0xF << 8)

#define DMA2D_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_SHIFT      (14)
#define DMA2D_FBLCLUTTABLE_SETSRAM_INIT_WR_RD(n)         (((n) & 0x3) << 14)
#define DMA2D_FBLCLUTTABLE_SETSRAM_INIT_WR_RD_MASK       (0x3 << 14)

#define DMA2D_FBLCLUTTABLE_WRDAT_SHIFT                   (0)
#define DMA2D_FBLCLUTTABLE_WRDAT(n)                      (((n) & 0xFF) << 0)
#define DMA2D_FBLCLUTTABLE_WRDAT_MASK                    (0xFF << 0)

/* DMA2D Output layer PFC Control Register */
#define DMA2D_OPFCCR_LENA_SHIFT      (0)    /* Bits 0  layer enable  */
#define DMA2D_OPFCCR_LENA_MASK       (0x01 << DMA2D_OPFCCR_LENA_SHIFT)
#define DMA2D_OPFCCR_LENA(n)         ((uint32_t)(n) << DMA2D_OPFCCR_LENA_SHIFT)

#define DMA2D_OPFCCR_CM_SHIFT      (4)    /* Bits 4-5 Color Mode */
                                /* 0x0 = RGB565 */
                                /* 0x1 = RGB888 */
                                /* 0x2 = ARGB888 32 */
                                /* 0x3 = RGBA888 32 */
#define DMA2D_OPFCCR_CM_MASK       (0x3 << DMA2D_OPFCCR_CM_SHIFT)
#define DMA2D_OPFCCR_CM(n)         ((uint32_t)(n) << DMA2D_OPFCCR_CM_SHIFT)

/*OUT Layer dma burst Register */
#define DMA2D_OL_BURST_LEN_SHIFT       (8)
#define DMA2D_OL_BURST_LEN_MASK        (0x1F << 8)
#define DMA2D_OL_BURST_LEN(n)          (((n) & 0x1F) << 8)

#define DMA2D_LPITCH_SHIFT             (0)
#define DMA2D_LPITCH_MASK              (0xFFFF << DMA2D_LPITCH_SHIFT)
#define DMA2D_LPITCH(n)                (((n) & 0xFFFF) << DMA2D_LPITCH_SHIFT)

#define DMA2D_OLPITCH_SHIFT            (16)
#define DMA2D_OLPITCH_MASK             (0xFFFF << DMA2D_OLPITCH_SHIFT)
#define DMA2D_OLPITCH(n)               (((n) & 0xFFFF) << DMA2D_OLPITCH_SHIFT)

/*DMA2D OUTPUT Layer FPT Register*/
#define DMA2D_OL_FPT_SHIFT             (28)
#define DMA2D_OL_FPT_MASK              (1 << DMA2D_OL_FPT_SHIFT)
#define DMA2D_OL_FPT_SET(n)                (((n) & 0xFFFF) << DMA2D_OL_FPT_SHIFT)

/* DMA2D_STBY_VSYNCCR  */
#define DMA2D_CFG_STBY_LENGTH_SHIFT           (0)
#define DMA2D_CFG_STBY_LENGTH(n)              (((n) & 0xFFF) << DMA2D_CFG_STBY_LENGTH_SHIFT)
#define DMA2D_CFG_STBY_LENGTH_MASK            (0xFFF << DMA2D_CFG_STBY_LENGTH_SHIFT)

#define DMA2D_CFG_VSYNC_START_LINE_SHIFT      (12)
#define DMA2D_CFG_VSYNC_START_LINE(n)         (((n) & 0xFF) << DMA2D_CFG_VSYNC_START_LINE_SHIFT)
#define DMA2D_CFG_VSYNC_START_LINE_MASK       (0xFF << DMA2D_CFG_VSYNC_START_LINE_SHIFT)

#define DMA2D_CFG_VSYNC_END_LINE_SHIFT        (20)
#define DMA2D_CFG_VSYNC_END_LINE(n)           (((n) & 0xFF) << DMA2D_CFG_VSYNC_END_LINE_SHIFT)
#define DMA2D_CFG_VSYNC_END_LINE_MASK         (0xFF << DMA2D_CFG_VSYNC_END_LINE_SHIFT)

/* DMA2D_PORCH_VSYNCCR     */
#define DMA2D_CFG_H_FRONT_PORCH_SMPN_SHIFT    (0)
#define DMA2D_CFG_H_FRONT_PORCH_SMPN(n)       (((n) & 0xFFF) << DMA2D_CFG_H_FRONT_PORCH_SMPN_SHIFT)
#define DMA2D_CFG_H_FRONT_PORCH_SMPN_MASK     (0xFFF << DMA2D_CFG_H_FRONT_PORCH_SMPN_SHIFT)

#define DMA2D_CFG_H_BACK_PORCH_SMPN_SHIFT     (12)
#define DMA2D_CFG_H_BACK_PORCH_SMPN(n)        (((n) & 0xFFF) << DMA2D_CFG_H_BACK_PORCH_SMPN_SHIFT)
#define DMA2D_CFG_H_BACK_PORCH_SMPN_MASK      (0xFFF << DMA2D_CFG_H_BACK_PORCH_SMPN_SHIFT)

#define DMA2D_CFG_VSYNC_NLINE_SHIFT           (24)
#define DMA2D_CFG_VSYNC_NLINE(n)              (((n) & 0xFF) << DMA2D_CFG_VSYNC_NLINE_SHIFT)
#define DMA2D_CFG_VSYNC_NLINE_MASK            (0xFF << DMA2D_CFG_VSYNC_NLINE_SHIFT)

/* DMA2D_VSYNC_PIXCR     */
#define DMA2D_CFG_VSYNC_START_PIX_SHIFT       (0)
#define DMA2D_CFG_VSYNC_START_PIX(n)          (((n) & 0xFFFF) << DMA2D_CFG_VSYNC_START_PIX_SHIFT)
#define DMA2D_CFG_VSYNC_START_PIX_MASK        (0xFFFF << DMA2D_CFG_VSYNC_START_PIX_SHIFT)

#define DMA2D_CFG_VSYNC_END_PIX_SHIFT         (16)
#define DMA2D_CFG_VSYNC_END_PIX(n)            (((n) & 0xFFFF) << DMA2D_CFG_VSYNC_END_PIX_SHIFT)
#define DMA2D_CFG_VSYNC_END_PIX_MASK          (0xFFFF << DMA2D_CFG_VSYNC_END_PIX_SHIFT)

/*F layer dma zoom register*/
#define DMA2D_CFG_DMAZM_H_PIXEL_SHIFT           (0)
#define DMA2D_CFG_DMAZM_H_PIXEL(n)              (((n) & 0xFFF) << DMA2D_CFG_DMAZM_H_PIXEL_SHIFT)
#define DMA2D_CFG_DMAZM_H_PIXEL_MASK            (0xFFF << DMA2D_CFG_DMAZM_H_PIXEL_SHIFT)

#define DMA2D_CFG_DMAZM_V_LINE_SHIFT            (16)
#define DMA2D_CFG_DMAZM_V_LINE(n)               (((n) & 0xFFF) << DMA2D_CFG_DMAZM_V_LINE_SHIFT)
#define DMA2D_CFG_DMAZM_V_LINE_MASK             (0xFFF << DMA2D_CFG_DMAZM_V_LINE_SHIFT)

/*B layer dma zoom register*/
#define DMA2D_CFG_GRAZM_H_PIXEL_SHIFT           (0)
#define DMA2D_CFG_GRAZM_H_PIXEL(n)              (((n) & 0xFFF) << DMA2D_CFG_GRAZM_H_PIXEL_SHIFT)
#define DMA2D_CFG_GRAZM_H_PIXEL_MASK            (0xFFF << DMA2D_CFG_GRAZM_H_PIXEL_SHIFT)

#define DMA2D_CFG_GRAZM_V_LINE_SHIFT            (16)
#define DMA2D_CFG_GRAZM_V_LINE(n)               (((n) & 0xFFF) << DMA2D_CFG_GRAZM_V_LINE_SHIFT)
#define DMA2D_CFG_GRAZM_V_LINE_MASK             (0xFFF << DMA2D_CFG_GRAZM_V_LINE_SHIFT)

/*F Layer start point on screen config register   */
#define DMA2D_CFG_DMA_OVSA_HPXL_SHIFT     (0)
#define DMA2D_CFG_DMA_OVSA_HPXL(n)        (((n) & 0xFFF) << DMA2D_CFG_DMA_OVSA_HPXL_SHIFT)
#define DMA2D_CFG_DMA_OVSA_HPXL_MASK      (0xFFF << DMA2D_CFG_DMA_OVSA_HPXL_SHIFT)

#define DMA2D_CFG_DMAFRM_Y                (1 << 12)
#define DMA2D_CFG_DMAFRM_U                (1 << 13)
#define DMA2D_CFG_DMAFRM_V                (1 << 14)
#define DMA2D_CFG_DMAFRM_C                (1 << 15)

#define DMA2D_CFG_DMA_OVSA_VLN_SHIFT      (16)
#define DMA2D_CFG_DMA_OVSA_VLN(n)         (((n) & 0xFFF) << DMA2D_CFG_DMA_OVSA_VLN_SHIFT)
#define DMA2D_CFG_DMA_OVSA_VLN_MASK       (0xFFF << DMA2D_CFG_DMA_OVSA_VLN_SHIFT)
#define DMA2D_CFG_DMAFRM_FIX              (1 << 31)

/*B Layer start point on screen  config  register   */
#define DMA2D_CFG_GRA_OVSA_HPXL_SHIFT     (0)
#define DMA2D_CFG_GRA_OVSA_HPXL(n)        (((n) & 0xFFF) << DMA2D_CFG_GRA_OVSA_HPXL_SHIFT)
#define DMA2D_CFG_GRA_OVSA_HPXL_MASK      (0xFFF << DMA2D_CFG_GRA_OVSA_HPXL_SHIFT)

#define DMA2D_CFG_GRA_OVSA_VLN_SHIFT      (16)
#define DMA2D_CFG_GRAFRM_Y                (1 << 12)
#define DMA2D_CFG_GRA_OVSA_VLN(n)         (((n) & 0xFFF) << DMA2D_CFG_GRA_OVSA_VLN_SHIFT)
#define DMA2D_CFG_GRA_OVSA_VLN_MASK       (0xFFF << DMA2D_CFG_GRA_OVSA_VLN_SHIFT)
#define DMA2D_CFG_GRAFRM_FIX              (1 << 31)

/* DMA2D Dither Contrl Register Address*/

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

#define DMA2D_DITHER_EN_PN(n)               (((n) & 0x1) << 0)
#define DMA2D_DITHER_EN_PN_MASK             (1 << 0)

#define DMA2D_DITHER_4X8_PN(n)              (((n) & 0x1) << 1)
#define DMA2D_DITHER_4X8_PN_MASK            (1 << 1)

#define DMA2D_RESERVED_1ECBIT2_SHIFT        (2)
#define DMA2D_RESERVED_1ECBIT2(n)           (((n) & 0x3) << DMA2D_RESERVED_1ECBIT2_SHIFT)
#define DMA2D_RESERVED_1ECBIT2_MASK         (0x3 << DMA2D_RESERVED_1ECBIT2_SHIFT)
#define DMA2D_DITHER_MODE_PN_SHIFT          (4)
#define DMA2D_DITHER_MODE_PN(n)             (((n) & 0x7) << DMA2D_DITHER_MODE_PN_SHIFT)
#define DMA2D_DITHER_MODE_PN_MASK           (0x7 << DMA2D_DITHER_MODE_PN_SHIFT)
#define DMA2D_RESERVED_1ECBIT7              (1 << 7)
#define DMA2D_DITHER_EN_TV                  (1 << 8)
#define DMA2D_DITHER_4X8_TV                 (1 << 9)
#define DMA2D_RESERVED_1ECBIT10_SHIFT       (10)
#define DMA2D_RESERVED_1ECBIT10(n)          (((n) & 0x3) << DMA2D_RESERVED_1ECBIT10_SHIFT)
#define DMA2D_RESERVED_1ECBIT10_MASK        (0x3 << DMA2D_RESERVED_1ECBIT10_SHIFT)
#define DMA2D_DITHER_MODE_TV_SHIFT          (12)
#define DMA2D_DITHER_MODE_TV(n)             (((n) & 0x7) << DMA2D_DITHER_MODE_TV_SHIFT)
#define DMA2D_DITHER_MODE_TV_MASK           (0x7 << DMA2D_DITHER_MODE_TV_SHIFT)
#define DMA2D_RESERVED_1ECBIT15             (1 << 15)
#define DMA2D_DITHER_TABLE_INDEX_SEL_SHIFT  (16)
#define DMA2D_DITHER_TABLE_INDEX_SEL(n)     (((n) & 0x3) << DMA2D_DITHER_TABLE_INDEX_SEL_SHIFT)
#define DMA2D_DITHER_TABLE_INDEX_SEL_MASK   (0x3 << DMA2D_DITHER_TABLE_INDEX_SEL_SHIFT)
#define DMA2D_RESERVED_1ECBIT18_SHIFT       (18)
#define DMA2D_RESERVED_1ECBIT18(n)          (((n) & 0x3FFF) << DMA2D_RESERVED_1ECBIT18_SHIFT)
#define DMA2D_RESERVED_1ECBIT18_MASK        (0x3FFF << DMA2D_RESERVED_1ECBIT18_SHIFT)

/* DMA2D Dither TBL Register Address*/
#define DMA2D_DITHER_TBL_DATA_SHIFT         (0)
#define DMA2D_DITHER_TBL_DATA(n)            (((n) & 0xFFFFFFFF) << DMA2D_DITHER_TBL_DATA_SHIFT)
#define DMA2D_DITHER_TBL_DATA_MASK          (0xFFFFFFFF << DMA2D_DITHER_TBL_DATA_SHIFT)

/* DMA2D Clock Disable Register Address*/
#define DMA2D_DCLKCR_PNDIS_SHIFT            (3)
#define DMA2D_DCLKCR_TVDIS_SHIFT            (4)
#define DMA2D_DCLKCR_PNDIS                  ( 0x1  << DMA2D_DCLKCR_PNDIS_SHIFT)
#define DMA2D_DCLKCR_TVDIS                  ( 0x1  << DMA2D_DCLKCR_TVDIS_SHIFT)
#define DMA2D_DCLKCR_PNDIS_MASK             (1 << DMA2D_DCLKCR_PNDIS_SHIFT)
#define DMA2D_DCLKCR_TVDIS_MASK             (1 << DMA2D_DCLKCR_TVDIS_SHIFT)

/* DMA2D Clock Enable Register Address*/
#define DMA2D_ECLKCR_PNENA_SHIFT            (3)
#define DMA2D_ECLKCR_TVENA_SHIFT            (4)
#define DMA2D_ECLKCR_PNENA                  ( 0x1  << DMA2D_ECLKCR_PNENA_SHIFT)
#define DMA2D_ECLKCR_TVENA                  ( 0x1  << DMA2D_ECLKCR_TVENA_SHIFT)
#define DMA2D_ECLKCR_PNENA_MASK             (1 << DMA2D_ECLKCR_PNENA_SHIFT)
#define DMA2D_ECLKCR_TVENA_MASK             (1 << DMA2D_ECLKCR_TVENA_SHIFT)

#endif /* __DMA2D_REG_DEFINE_H */
