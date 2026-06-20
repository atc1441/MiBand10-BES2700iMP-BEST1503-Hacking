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
 * DOC: code for DMA2D simulation test
 *
 */

#ifdef DMA2D_SIMU_TEST_CODE

#define TESTw 20        // 180//360//16
#define TESTh 20        // 180//360//2
static int sim_dma2d_blit(void);

/*
   fill color format:
     DMA2D_LAYER_SPU:bgr888(Bits [7:0] are used for Red
     Bits [15:8] are used for Green
     Bits [23:16] are used for Blue)
*/

int sim_dma2d_fillcolor_o(void)
{
    int ret;
    uint16_t pitch = 0;

    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    *(volatile uint32_t *)0x20400000 = 0xddaabb;
    *(volatile uint32_t *)0x20400004 = 0;

    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, true);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);
    *(volatile uint32_t *)(0x501d0260) |= 1 << 24;

#define FB_CONST_ALPHA 0x00    /* Transparency by alpha value */
#define FB_PIXEL_ALPHA 0x01    /* Transparency by pixel alpha value */

    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, DMA2D_PFCCR_AM_CONST, 0xaa, 13);    // color fmt :ARGB888 32

    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0xaa, 13);    // color fmt :ARGB888 32
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0, 13);    // color fmt :ARGB888 32

    //  hal_dma2d_lpfc(DMA2D_LAYER_LFORE, 0, 0, 11);  //color fmt :ARGB888 32

    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 13);

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
//  hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 11);  //out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif
    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 11);  //out lay color rgb565
    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Setup smart panel with filled color  */

#if 1                // test_in_argb888
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0xffddccbb);    // color fmt : ARGB888 32
#else // rgba888
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, (0xddaaeeff >> 8) & 0xffffff);    // color fmt : ARGB888 32
#endif

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, TESTw, TESTh);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LBACK, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x20400000);

    /*set layer image pitch */

    // pitch = TESTw * 2;
    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, 4, 4);

    hal_dma2d_lsize(DMA2D_LAYER_LBACK, 4, 4);

    hal_dma2d_lsize(DMA2D_LAYER_LOUT, TESTw, TESTh);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    // hal_dma2d_lsquln_start_set();
    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(TESTw + 2, TESTh + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();

    DMA2D_REP("%s end", __func__);

    return ret;
}

int sim_dma2d_fillcolor(void)
{
    int ret;
    uint16_t pitch = 0;

    // *((uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    *(uint32_t *) 0x20400000 = 0xddaabb;
    *(uint32_t *) 0x20400004 = 0;

    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

#if 0
    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, DMA2D_PFCCR_AM_CONST, 0xaa, 13);    //color fmt :ARGB888 32

    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0xaa, 13);    //color fmt :ARGB888 32
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0, 13);    //c
#else
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0, 13);    // color fmt :ARGB888 32

#endif
    //  hal_dma2d_lpfc(DMA2D_LAYER_LFORE, 0, 0, 11);  //color fmt :ARGB888 32

    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 13);

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
//  hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 11);  //out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif
    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 11);  //out lay color rgb565
    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Setup smart panel with filled color  */
    //  hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0xffff0000);
    //  hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0xffdd0000);

#if 1                // test_in_argb888
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0xffddccbb);    // color fmt : ARGB888 32
#else // rgba888
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, (0xddaaeeff >> 8) & 0xffffff);    // color fmt : ARGB888 32
#endif

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, TESTw, TESTh);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x20400000);
#if 0                // jjjj
    hal_dma2d_lstartaddr(DMA2D_LAYER_LBACK, 0x20400000);
#endif
    /*set layer image pitch */

    // pitch = TESTw * 2;
    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, 4, 4);

    hal_dma2d_lsize(DMA2D_LAYER_LBACK, 4, 4);

    hal_dma2d_lsize(DMA2D_LAYER_LOUT, TESTw, TESTh);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    // hal_dma2d_lsquln_start_set();
    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(TESTw + 2, TESTh + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();

    DMA2D_REP("%s end", __func__);

    return ret;
}

static int sim_dma2d_blit_fb(void)
{
    int ret;
    uint32_t mode;

    uint32_t *saddr0 = (uint32_t *) 0x20400b48;
    uint32_t *saddr1 = (uint32_t *) 0x2047f448;

    for (int i = 0; i < 10; i++) {
        *(saddr0 + i) = 0xaabbccdd;
        *(saddr1 + i) = 0x11223344;
    }
    saddr0 += 360;
    saddr1 += 360;

    for (int i = 0; i < 10; i++) {
        *(saddr0 + i) = 0xaabbccdd;
        *(saddr1 + i) = 0x11223344;
    }

    hal_dma2d_reset();
    hal_dma2d_lpfc_reset();
    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0, 13);
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 13);

    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, 10, 2);

    /*set layer dma start mem address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400b48);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x2047f448);

    /*set layer image pitch */
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, 1440);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, 1440);

    hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, 10, 2);
    hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, 10, 2);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

    /*set layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, 10, 2);
    hal_dma2d_lsize(DMA2D_LAYER_LBACK, 10, 2);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, 10, 2);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    /*setup smpn and write back path work mode */
    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_wdma_1fpt_set();
    hal_dma2d_ol_dmaburst_length_set(8);
    hal_dma2d_panelpath_dmatrans_trigger_enable(DMA2D_LAYER_LFORE, true);

    /*config  dispaly module */
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_vsync_porch_set();

    hal_dma2d_bgl_vhtotal_set(10 + 2, 2 + 2);
    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();

    saddr1 = (uint32_t *) 0x2047f448;

    for (int i = 0; i < 10; i++) {
        if (*(saddr1 + i) != 0xaabbccdd) {
            while (1) ;
        }
    }
    saddr1 += 360;

    for (int i = 0; i < 10; i++) {
        if (*(saddr1 + i) != 0xaabbccdd) {
            while (1) ;
        }
    }

    if (ret != OK) {
        ret = -ECANCELED;
        DMA2D_ERR("ERROR: Returning ECANCELED\n");
    }

    DMA2D_REP("%s end", __func__);
    return ret;
}

static int sim_dma2d_blit(void)
{
    int ret;
    uint16_t pitch = 0;

    //  *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
    hal_dma2d_reset();
    *(uint32_t *) 0x20400000 = 0xffddccbb;
    *(uint32_t *) 0x20400004 = 0x99ddaabb;
    *(uint32_t *) (0x20400000 + 15 * 4) = 0x11223344;
    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);
#define FB_CONST_ALPHA 0x00    /* Transparency by alpha value */
#define FB_PIXEL_ALPHA 0x01    /* Transparency by pixel alpha value */

    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0, 13);    // color fmt :ARGB888 32
    // hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0xff, 13);
    // //color fmt :ARGB888 32

    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 13);

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif

    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, TESTw, TESTh);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x2047f000);

    /*set layer image pitch */

    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, TESTw, TESTh);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, TESTw, TESTh);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(TESTw + 2, TESTh + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();
    DMA2D_REP("%s end", __func__);
    return ret;
}

/*
  16bit RGB565 -> 24bit RGB888
  16bit RGB656 R4 R3 R2 R1 R0 G5 G4 G3 G2 G1 G0 B4 B3 B2 B1 B0
  24ibt RGB888 R4 R3 R2 R1 R0 0 0 0 G5 G4 G3 G2 G1 G0 0 0 B4 B3 B2 B1 B0 0 0 0
*/
static int sim_dma2d_blit_rgb565_to_argb888(void)
{
    int ret;
    uint16_t pitch = 0;

    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    *(uint16_t *) 0x20400000 = 0xffff;
    *(uint16_t *) 0x20400002 = 0xeefe;
    *(uint16_t *) (0x20400000 + 15 * 2) = 0xdfed;

    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);
#define FB_CONST_ALPHA 0x00    /* Transparency by alpha value */
#define FB_PIXEL_ALPHA 0x01    /* Transparency by pixel alpha value */

    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0, 11);    // color fmt :RGB565

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif

    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, TESTw, TESTh);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x20401000);

    /*set layer image pitch */

    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, TESTw, TESTh);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, TESTw, TESTh);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(TESTw + 2, TESTh + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();
    while (1) ;
    return ret;
}

static int sim_dma2d_blend(void)
{
    int ret;
    uint16_t pitch = 0;

    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    *(uint32_t *) 0x20400000 = 0x08ddccbb;
    *(uint32_t *) 0x20400200 = 0x02112233;

    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, true);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

#if 1
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_BLPIXEL, 0, 13);
    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, DMA2D_PFCCR_AM_BLPIXEL, 0, 13);

#else
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0xff - 0x7f,
               13);
    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, 0xff, 0, 13);
    // (FG_c * (0xff - alpha) +  BG_c * alpha)/ alpha
#endif
    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 13);

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif
    /*Setup SWAP config */
    // hal_dma2d_swap_path(1);
    //  hal_dma2d_swap_path(0);
    // hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    // hal_dma2d_lblend_select(DMA2D_LAYER_LFORE, DMA2D_LAYER_LBACK);
    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, TESTw, TESTh);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400000);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LBACK, 0x20400200);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x20401000);

    /*set layer image pitch */

    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LBACK, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, TESTw, TESTh);

    hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, TESTw, TESTh);
    hal_dma2d_lsize(DMA2D_LAYER_LBACK, TESTw, TESTh);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, TESTw, TESTh);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    // hal_dma2d_lsquln_start_set();
    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(TESTw + 2, TESTh + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();
    while (1) ;
    return ret;
}

static int sim_dma2d_blend_fbb(void)
{
    int ret;
    uint16_t pitch = 0;

    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    // FG
    *(uint32_t *) 0x20400b48 = 0x08dd88bb;
    *(uint32_t *) 0x20400b94 = 0x08dd88bb;
    // BG
    *(uint32_t *) 0x2047f448 = 0xffdd99bb;
    *(uint32_t *) 0x2047f494 = 0xffdd99bb;

    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, true);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

#if 1
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0x0f, 13);
    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, DMA2D_PFCCR_AM_CONST, 0, 13);

#else
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0xff - 0x7f,
               13);
    hal_dma2d_lpfc(DMA2D_LAYER_LBACK, 0xff, 0, 13);
    // (FG_c * (0xff - alpha) +  BG_c * alpha)/ alpha
#endif
    // hal_dma2d_lpfc(DMA2D_LAYER_LOUT,0, 0, 13);

#if 1                // test_in_argb888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 65);    // out lay color ARGB888 32
#else // rgba888
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, 21);    // out lay color RGBA888 32
#endif
    /*Setup SWAP config */
    // hal_dma2d_swap_path(1);
    //  hal_dma2d_swap_path(0);
    // hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    // hal_dma2d_lblend_select(DMA2D_LAYER_LFORE, DMA2D_LAYER_LBACK);
    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, 10, 2);

    /*set layer start address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE, 0x20400b48);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LBACK, 0x2047f448);
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT, 0x2047f448);

    /*set layer image pitch */

    pitch = TESTw * 4;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LBACK, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, 10, 2);
    hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, 10, 2);

    hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
    hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, 10, 2);
    hal_dma2d_lsize(DMA2D_LAYER_LBACK, 10, 2);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, 10, 2);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

    // hal_dma2d_lsquln_start_set();
    hal_dma2d_vsync_porch_set();

    hal_dma2d_wdma_1fpt_set();

    hal_dma2d_bgl_vhtotal_set(10 + 2, 2 + 2);

    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);
    hal_dma2d_ol_dmaburst_length_set(8);

    /* Start DMA2D and wait until completed */
    ret = dma2d_start();
    while (1) ;
    return ret;
}

#define getreg32(a) (*(volatile uint32_t *)(a))

#define putreg32(v, a)                               \
    do {                                             \
        DMA2D_REG("putreg32(0x%x,0x%x);", (v), (a)); \
        *(volatile uint32_t *)(a) = (v);             \
    } while (0)

#define reg32_bitset(v, a)              \
    do {                                \
        *(volatile uint32_t *)(a) |= v; \
    } while (0)

static void sim_dma2d_fill_err()
{
    // The framebuffer device /dev/fb_dma2d was opened successfully.

    // fb_init fb:0x2400229c
    // Initialize DMA2D driver

    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    // fb2d_fill
    // dma2d_fillcolor oinfo=0x2417ffb8, argb=ffff0000

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    // hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
    // hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
    // hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
    // hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
    // hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
    // hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
    // hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
    // hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
    // hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
    // hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
    // hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
    // lid=3, argb=ffff0000

    putreg32(0xffff0000, 0x501d0124);
    // hal_dma2d_lsize lid:3 w:240 h:240
    putreg32(0xf100f1, 0x501d0118);
    // hal_dma2d_lstartaddr lid:0 addr:20400024
    putreg32(0x20400024, 0x501d00c0);
    // hal_dma2d_lstartaddr lid:2 addr:20400024
    putreg32(0x20400024, 0x501d0208);
    // dma2d_fillcolor xres:240 bpp:32
    // hal_dma2d_lpitch lid:0 pitch:960
    putreg32(0x3c0, 0x501d00e0);
    // hal_dma2d_lpitch lid:2 pitch:960
    putreg32(0x3c00021, 0x501d0200);
    // hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
    // hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
    // hal_dma2d_lsize lid:2 w:240 h:240
    putreg32(0xf000f0, 0x501d0204);
    // hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    // hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
    // hal_dma2d_wdma_1fpt_set
    putreg32(0x10f000f0, 0x501d0204);
    // hal_dma2d_ol_dmaburst_length_set
    putreg32(0x3c00821, 0x501d0200);
    // lid=0, ena=00000001

    // lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    // hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    // hal_dma2d_bgl_vhtotal_set w:242 h:242
    putreg32(0xf200f2, 0x501d0114);
    // hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    // hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
    // hal_dma2d_start

    dma2d_waitforirq();
    // dma2d irq = 0, regval = 00a22005

    // DMA transfer complete

    // hal_dma2d_spu_framedone_irqstat_clr
    putreg32(0x822005, 0x501d01c4);
    while (1) ;
    // fb2d_fill_rects test OK
}

void sim_blend_err(void)
{
    // The framebuffer device /dev/fb_dma2d was opened successfully.

    // fb_init fb:0x2400229c
    // Initialize DMA2D driver

    //*((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
    // doverlay=0x2417ff58, destxpos=0, destypos=0, foverlay=0x2417ff64,
    // forexpos=0, foreypos=0, boverlay=0x2417ff70,
    // barea=\0x00\0xef\0xbe\0xad\0xde\0x00\0x00\0x00\0x00\0xb6\0xff\0x17$\0xb6\0xff\0x17$\0xf0\0x00\0x00\0x00\0x03\0x02\0x02\0x00\0x80c\0x02\0x00X\0xff\0x17$\0x00\0x00\0x00\0x00\0x00\0x00\0x00\0x00d\0xff\0x17$\0x00\0x00\0x00\0x00\0x00\0x00\0x00\0x00p\0xff\0x17$hal_dma2d_reset();
    putreg32(0x0, 0x501d0188);
    putreg32(0x0, 0x501d0200);
    putreg32(0x0, 0x501d0190);
    putreg32(0x0, 0x501d0264);
    putreg32(0x0, 0x501d0194);
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
    // hal_dma2dl_enable lid:1 enable:1
    putreg32(0x1, 0x501d0264);
    // hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
    // hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
    // hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
    // hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
    // hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
    // hal_dma2d_lpfc lid:1 bmode:0 alpha:0 fmt:13
    // hal_dma2d_lpfc_swaprb lid=1, ena=00000000

    putreg32(0x1, 0x501d0264);
    putreg32(0x401, 0x501d0264);
    // hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
    // hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
    // lid=3, argb=00000000

    putreg32(0x0, 0x501d0124);
    // hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
    // hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
    // hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
    // hal_dma2d_lsize lid:3 w:240 h:7
    putreg32(0x800f1, 0x501d0118);
    // hal_dma2d_lstartaddr lid:0 addr:20400028
    putreg32(0x20400028, 0x501d00c0);
    // hal_dma2d_lstartaddr lid:1 addr:20438438
    putreg32(0x20438438, 0x501d00f4);
    // hal_dma2d_lstartaddr lid:2 addr:20438438
    putreg32(0x20438438, 0x501d0208);
    // hal_dma2d_lpitch lid:0 pitch:960
    putreg32(0x3c0, 0x501d00e0);
    // hal_dma2d_lpitch lid:1 pitch:960
    putreg32(0x3c0, 0x501d00fc);
    // hal_dma2d_lpitch lid:2 pitch:960
    putreg32(0x3c00021, 0x501d0200);
    // hal_dma2d_lzoom_set lid:0 w:240 h:7
    putreg32(0x700f0, 0x501d00f0);
    // hal_dma2d_lzoom_set lid:1 w:240 h:7
    putreg32(0x700f0, 0x501d0108);
    // hal_dma2d_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501d00e8);
    // hal_dma2d_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501d0100);
    // hal_dma2d_lsize lid:0 w:240 h:7
    putreg32(0x700f0, 0x501d00ec);
    // hal_dma2d_lsize lid:1 w:240 h:7
    putreg32(0x700f0, 0x501d0104);
    // hal_dma2d_lsize lid:2 w:240 h:7
    putreg32(0x700f0, 0x501d0204);
    // hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    // hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
    // hal_dma2d_wdma_1fpt_set
    putreg32(0x100700f0, 0x501d0204);
    // hal_dma2d_ol_dmaburst_length_set
    putreg32(0x3c00821, 0x501d0200);
    // lid=0, ena=00000001

    // lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    // hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    // hal_dma2d_bgl_vhtotal_set w:242 h:9
    putreg32(0x900f2, 0x501d0114);
    // hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    // hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
    putreg32(0x22210000, 0x501d01c0);
    // hal_dma2d_start

    dma2d_waitforirq();
    // dma2d irq = 0, regval = 02820005

    // ERROR: configuration

    // dma2d irq = 0, regval = 8aa240c9

    // DMA transfer complete

    // hal_dma2d_spu_framedone_irqstat_clr
    // putreg32(0x8a8240c9,0x501d01c4);
    // fb2d_blend alpha mode:0 test OK
    // fb2d_blend_consta ret:0
}

int sim_dma2d_test(void)
{
    dma2dinitialize();
    // sim_dma2d_fill_err();
    sim_blend_err();
    // sim_dma2d_fillcolor();
    // sim_dma2d_fillcolor();
    // while(1);
    // sim_dma2d_blit();
    // sim_dma2d_blit_fb();
    while (1) ;
    // sim_dma2d_blend();
    // sim_dma2d_blend_fbb();
    // sim_dma2d_blit_rgb565_to_argb888();
}

#endif // DMA2D_SIMU_TEST_CODE
