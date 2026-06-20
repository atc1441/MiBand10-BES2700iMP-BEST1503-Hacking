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
 * DOC:code for lcdc simulation test.
 *
 */

#ifdef LCDC_SIMU_TEST_CODE

#define getreg32(a)          (*(volatile uint32_t *)(a))

#define putreg32(v,a)        do { \
                                   LCDC_REGINFO("putreg32(0x%x,0x%x);",(v), (a)) ; \
                                   *(volatile uint32_t *)(a) = (v); \
                              } while(0)

#define reg32_bitset(v,a)    do { \
                                   *(volatile uint32_t *)(a) |= v; \
                              } while(0)

extern int dma2d_waitforirq(void);
extern int dma2dinitialize(void);

#define hal_sys_timer_delay_us(...)
void lcdc_test_sim_00(void)
{

//Initialize LCDC driver

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//Configuring the LCD controller

//Configure lcd periphery

//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20410000
    putreg32(0x20410000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80027f00, 0x501c0194);
//hal_lcdc_vsync_porch_set
    putreg32(0x501080, 0x501c0218);
    putreg32(0xa020020, 0x501c021c);
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_wdma_1fpt_set
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501c0114);
//hal_lcdc_bgl_hporch_set f:12 b:10
    putreg32(0xa000c, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:10 b:10
    putreg32(0xa000a, 0x501c0120);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
//Configure interrupts

    putreg32(0x0, 0x501c01c0);
    putreg32(0x10000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
//lcdc_irqconfig irq:0 irq_handler:0x222b9
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//Configure global register

//Initialize DMA2D driver

    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
    dma2dinitialize();
//Initialize lcdc layer

//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x400, 0x501c0190);
//xpos = 0, ypos = 0, xres = 120, yres = 120

//hal_lcdc_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//set LCDC_L1PFCR=0000000d

//hal_lcdc_lpfc_fmt lid:0  fmt:13
    putreg32(0x400, 0x501c0190);
//set LCDC_L1DCCR=00000000

//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x401, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x400, 0x501c0264);
//xpos = 0, ypos = 0, xres = 120, yres = 120

//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:480
    putreg32(0x1e0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//set LCDC_L2PFCR=0000000d

//hal_lcdc_lpfc_fmt lid:1  fmt:13
    putreg32(0x400, 0x501c0264);
//set LCDC_L2DCCR=00000000

//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x401, 0x501c0264);
//Enabling the dis play

//vplane: 0

//vtable=0x240001d0 vinfo=0x2417ff3c

//vtable=0x240001d0 planeno=0 pinfo=0x2417ff44

//vtable=0x240001d0 overlay=0 oinfo=0x2417ff50

//vplane: 0

//vtable=0x240001d0 vinfo=0x2417ff3c

//vtable=0x240001d0 planeno=0 pinfo=0x2417ff44

//vtable=0x240001d0 overlay=0 oinfo=0x2417ff50

//fb_init fb:24000ba4
//fb_ioctl fd:24000ba4 cmd: 9 arg: 24000be4

//oinfo:24000be4 fb:24000ba4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=0 oinfo=0x24000be4

//fb_ioctl fd:24000ba4 cmd: 9 arg: 24000c0c

//oinfo:24000c0c fb:24000ba4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=1 oinfo=0x24000c0c

//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//fb_init fb:24000bb4
//fb_ioctl fd:24000bb4 cmd: 9 arg: 24000be4

//oinfo:24000be4 fb:24000bb4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=0 oinfo=0x24000be4

//fb_ioctl fd:24000bb4 cmd: 9 arg: 24000c0c

//oinfo:24000c0c fb:24000bb4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=1 oinfo=0x24000c0c

//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:24000ba4 cmd: 9 arg: 2417ff98

//oinfo:2417ff98 fb:24000ba4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=0 oinfo=0x2417ff98

//fb_ioctl fd:24000ba4 cmd: 13 arg: 2417ff98

//vtable=0x240001d0, overlay=-16711936, color=00021e4d

//dma2d_fillcolor oinfo=0x24000250, argb=ff00ff00

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ff00ff00

    putreg32(0xff00ff00, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501d00c0);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:2 addr:20400000
    putreg32(0x20400000, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x20400000
//lcdc set background fb1 color with red color
//fb_ioctl fd:24000ba4 cmd: 9 arg: 2417ffc0

//oinfo:2417ffc0 fb:24000ba4 fb->vtable:240001d0 fb->vtable->getoverlayinfo:2233d
//vtable=0x240001d0 overlay=1 oinfo=0x2417ffc0

//fb_ioctl fd:24000ba4 cmd: 13 arg: 2417ffc0

//vtable=0x240001d0, overlay=-65536, color=00021e4d

//dma2d_fillcolor oinfo=0x2400028c, argb=ffff0000

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ffff0000

    putreg32(0xffff0000, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:0 addr:2040e100
    putreg32(0x2040e100, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x2040e100
//display  foreground fb0
//fb_ioctl fd:24000ba4 cmd: 11 arg: 2417ff98

//vtable=0x240001d0, overlay=0, transp=00, transp_mode=00

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
//lcdc_update enter
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//lcd_irq irq = 0, regval = 88a040cf

//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xcf
//lcdc_update exit
//check_outlayer_result pass  wbp:0x20410000
//display  background fb1
//fb_ioctl fd:24000ba4 cmd: 11 arg: 2417ff98

//vtable=0x240001d0, overlay=0, transp=ff, transp_mode=00

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:255
    putreg32(0x2ff00, 0x501c0194);
//lcdc_update enter
//hal_lcdc_wdma_toggle_frame
    putreg32(0x10780078, 0x501c0204);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//lcd_irq irq = 0, regval = 44a0400f

//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xf
//lcdc_update exit
//check_outlayer_result fail wbp[0] color: 0  != ff0000
//lcdc dma2d  fill fb0 with green color
//lcdc dma2d  fill fb1 with red   color
//fb0 fb1 blending display
//fb_ioctl fd:24000ba4 cmd: 11 arg: 2417ff98

//vtable=0x240001d0, overlay=0, transp=7f, transp_mode=00

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//lcdc_update enter
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//lcd_irq irq = 0, regval = 88a040cf

//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xcf
//lcdc_update exit
//check_outlayer_result fail wbp[0] color: 0  != ffff00

    while (1) ;

}

void lcdc_swap_fbl(void)
{
    hal_lcdc_lstartaddr(0, fbmem_layer_t[1]);
    hal_lcdc_lstartaddr(1, fbmem_layer_t[0]);
}

void lcdc_test_sim_02(void)
{

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20410000
    putreg32(0x20410000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80027f00, 0x501c0194);
//hal_lcdc_vsync_porch_set
    putreg32(0x501080, 0x501c0218);
    putreg32(0xa020020, 0x501c021c);
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_wdma_1fpt_set
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501c0114);
//hal_lcdc_bgl_hporch_set f:12 b:10
    putreg32(0xa000c, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:10 b:10
    putreg32(0xa000a, 0x501c0120);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
    putreg32(0x0, 0x501c01c0);
    putreg32(0x10000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//Initialize DMA2D driver

    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
    dma2dinitialize();
//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x400, 0x501c0190);
//hal_lcdc_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_lpfc_fmt lid:0  fmt:13
    putreg32(0x400, 0x501c0190);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x401, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x400, 0x501c0264);
//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:480
    putreg32(0x1e0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_lpfc_fmt lid:1  fmt:13
    putreg32(0x400, 0x501c0264);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x401, 0x501c0264);
//lcdc set foreground fb0 color with green color
//dma2d_fillcolor oinfo=0x24000250, argb=ff00ff00

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ff00ff00

    putreg32(0xff00ff00, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501d00c0);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:2 addr:20400000
    putreg32(0x20400000, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x20400000
//lcdc set background fb1 color with red color
//dma2d_fillcolor oinfo=0x2400028c, argb=ffff0000

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ffff0000

    putreg32(0xffff0000, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:0 addr:2040e100
    putreg32(0x2040e100, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x2040e100
//display  foreground fb0
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc8, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20410000
    putreg32(0x20410000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result pass  wbp:0x20410000
//display  background fb1
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:255
    putreg32(0x2ff00, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc0, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20410000
    putreg32(0x20410000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xf
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//check_outlayer_result fail wbp[1984] color: ff000000  != ff0000
//lcdc dma2d  fill fb0 with green color
//lcdc dma2d  fill fb1 with red   color
//fb0 fb1 blending display
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc0, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20410000
    putreg32(0x20410000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result_blend pass  wbp:0x20410000
//lcdc dma2d copy fb0 to fb1
//dma2d_blit doverlay=0x2400028c, destxpos=0, destypos=0, soverlay=0x24000250, sarea=0x2417ff40

    hal_dma2d_reset();
    putreg32(0x0, 0x501d0188);
    putreg32(0x0, 0x501d0200);
    putreg32(0x0, 0x501d0190);
    putreg32(0x0, 0x501d0264);
    putreg32(0x0, 0x501d0194);
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x0, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//lid=3, argb=00000000

    putreg32(0x0, 0x501d0124);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//hal_dma2d_lsize lid:3 w:20 h:2
    putreg32(0x30015, 0x501d0118);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lzoom_set lid:0 w:20 h:2
    putreg32(0x20014, 0x501d00f0);
//hal_dma2d_lzoom_set lid:1 w:20 h:2
    putreg32(0x20014, 0x501d0108);
//hal_dma2d_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501d00e8);
//hal_dma2d_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501d0100);
//hal_dma2d_lsize lid:0 w:20 h:2
    putreg32(0x20014, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:20 h:2
    putreg32(0x20014, 0x501d0104);
//hal_dma2d_lsize lid:2 w:20 h:2
    putreg32(0x20014, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10020014, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80000000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:22 h:4
    putreg32(0x40016, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 80a2008d

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x8082408d,0x501d01c4);
//display fb1
//check_blit_result test fail
//check_blit_result 1 src color:0xff00
//check_blit_result 1 dest color:0xff0000

    while (1) ;
}

void lcdc_chromakey_test(void)
{

//Initialize LCDC driver

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//Configuring the LCD controller

//Configure lcd periphery

//hal_lcdc_stby_vsync_set
    putreg32(0x10501080, 0x501c0218);
//hal_lcdc_porch_vsync_set
    putreg32(0xa020020, 0x501c021c);
//hal_lcdc_pix_vsync_set
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_bgl_hporch_set f:19 b:12
    putreg32(0xc0013, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:11 b:10
    putreg32(0xa000b, 0x501c0120);
//hal_lcdc_bgl_vhtotal_set w:141 h:151
    putreg32(0x97008d, 0x501c0114);
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x87020000, 0x501c0194);
    putreg32(0x78, 0x501c0214);
    putreg32(0x780000, 0x501c0214);
    putreg32(0x301, 0x501c0188);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
//Configure interrupts

    putreg32(0x0, 0x501c01c0);
    putreg32(0x10000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
//lcdc_irqconfig irq:0 irq_handler:0x22465
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//Configure global register

//Initialize DMA2D driver

    dma2dinitialize();
//Initialize lcdc layer

//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x400, 0x501c0190);
//xpos = 0, ypos = 0, xres = 120, yres = 120

//hal_lcdc_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//set LCDC_L1PFCR=0000000d

//hal_lcdc_lpfc_fmt lid:0  fmt:13
    putreg32(0x400, 0x501c0190);
//set LCDC_L1DCCR=00000000

//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x7027f00, 0x501c0194);
//lcdc_linit chromkey:0xfe
//set LCDC_L1CKCR=000000fe

    putreg32(0x0, 0x501c0130);
    putreg32(0x0, 0x501c0134);
    putreg32(0xfefe0000, 0x501c0138);
//hal_lcdc_lchromakeyenable lid:0  ena:1
    putreg32(0x7027f00, 0x501c0194);
    putreg32(0xc0000, 0x501c0084);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x401, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x400, 0x501c0264);
//xpos = 0, ypos = 0, xres = 120, yres = 120

//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:480
    putreg32(0x1e0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//set LCDC_L2PFCR=0000000d

//hal_lcdc_lpfc_fmt lid:1  fmt:13
    putreg32(0x400, 0x501c0264);
//set LCDC_L2DCCR=00000000

//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//lcdc_linit chromkey:0xfe
//set LCDC_L2CKCR=000000fe

    putreg32(0x0, 0x501c0130);
    putreg32(0x0, 0x501c0134);
    putreg32(0xfefe0000, 0x501c0138);
//hal_lcdc_lchromakeyenable lid:1  ena:1
//hal_lcdc_lchromakeyenable invalid  lid:1
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x401, 0x501c0264);
//Enabling the dis play

//vplane: 0

//vtable=0x240001c4 vinfo=0x2417ff3c

//vtable=0x240001c4 planeno=0 pinfo=0x2417ff44

//vtable=0x240001c4 overlay=0 oinfo=0x2417ff50

//vplane: 0

//vtable=0x240001c4 vinfo=0x2417ff3c

//vtable=0x240001c4 planeno=0 pinfo=0x2417ff44

//vtable=0x240001c4 overlay=0 oinfo=0x2417ff50

//fb_init fb:24000574
//fb_ioctl fd:24000574 cmd: 9 arg: 240005b4

//oinfo:240005b4 fb:24000574 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=0 oinfo=0x240005b4

//fb_ioctl fd:24000574 cmd: 9 arg: 240005dc

//oinfo:240005dc fb:24000574 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=1 oinfo=0x240005dc

//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//fb_init fb:24000584
//fb_ioctl fd:24000584 cmd: 9 arg: 240005b4

//oinfo:240005b4 fb:24000584 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=0 oinfo=0x240005b4

//fb_ioctl fd:24000584 cmd: 9 arg: 240005dc

//oinfo:240005dc fb:24000584 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=1 oinfo=0x240005dc

//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:24000574 cmd: 9 arg: 2417ff98

//oinfo:2417ff98 fb:24000574 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=0 oinfo=0x2417ff98

//fb_ioctl fd:24000574 cmd: 13 arg: 2417ff98

//vtable=0x240001c4, overlay=-16711936, color=0002202d

//dma2d_fillcolor oinfo=0x2400022c, argb=ff00ff00

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ff00ff00

    putreg32(0xff00ff00, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501d00c0);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:2 addr:20400000
    putreg32(0x20400000, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x20400000
//lcdc set background fb1 color with red color
//fb_ioctl fd:24000574 cmd: 9 arg: 2417ffc0

//oinfo:2417ffc0 fb:24000574 fb->vtable:240001c4 fb->vtable->getoverlayinfo:224e9
//vtable=0x240001c4 overlay=1 oinfo=0x2417ffc0

//fb_ioctl fd:24000574 cmd: 13 arg: 2417ffc0

//vtable=0x240001c4, overlay=-65536, color=0002202d

//dma2d_fillcolor oinfo=0x24000268, argb=ffff0000

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ffff0000

    putreg32(0xffff0000, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:0 addr:2040e100
    putreg32(0x2040e100, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x2040e100
//display  foreground fb0
//fb_ioctl fd:24000574 cmd: 11 arg: 2417ff98

//vtable=0x240001c4, overlay=0, transp=00, transp_mode=00
//set chromakey color to fb

    {
        volatile uint32_t *fb = (uint32_t *) 0x20400000;
        for (int i = 0; i < 360; i++) {
            fb[i] |= 0xfe;
        }
    }

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x7020000, 0x501c0194);
//lcdc_update enter
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc8, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//lcd_irq irq = 0, regval = 88a040cf

//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
//lcdcirqstatus:0xcf
//lcdc_update exit
//check_outlayer_result fail wbp[0] color: fffe  != ff00

    while (1) ;
}

static uint8_t cadjcurves_table[256 * 3];
void lcdc_coloradjcurve_test()
{
#ifdef CONFIG_FB_COLORADJCURVE
    struct fb_cadjmap_s cadjmap;
    cadjmap.first = 0;
    cadjmap.len = 256;
    cadjmap.blue = &cadjcurves_table[0];
    cadjmap.red = &cadjcurves_table[256];
    cadjmap.green = &cadjcurves_table[512];

    for (int n = cadjmap.first; n < cadjmap.len; n++) {
        cadjmap.blue[n] = n / 2;
        cadjmap.green[n] = n / 2;
        cadjmap.red[n] = n / 2;
    }
#endif

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;

    FAR struct lcdc_s *layer = &g_vtable.layer[0];

#ifdef CONFIG_FB_COLORADJCURVE
    lcdc_lcadjcurves_enable(layer, true);
    lcdc_lputcadjcurves(layer, &cadjmap);
#endif

//hal_lcdc_stby_vsync_set
    putreg32(0x10501080, 0x501c0218);
//hal_lcdc_porch_vsync_set
    putreg32(0xa020020, 0x501c021c);
//hal_lcdc_pix_vsync_set
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_bgl_hporch_set f:19 b:12
    putreg32(0xc0013, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:11 b:10
    putreg32(0xa000b, 0x501c0120);
//hal_lcdc_bgl_vhtotal_set w:141 h:151
    putreg32(0x97008d, 0x501c0114);
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501c0194);
    putreg32(0x78, 0x501c0214);
    putreg32(0x780000, 0x501c0214);
    putreg32(0x301, 0x501c0188);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
    putreg32(0x0, 0x501c01c0);
    putreg32(0x10000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//Initialize DMA2D driver

    dma2dinitialize();
//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x400, 0x501c0190);
//hal_lcdc_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_lpfc_fmt lid:0  fmt:13
    putreg32(0x400, 0x501c0190);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
    putreg32(0x0, 0x501c0130);
    putreg32(0x0, 0x501c0134);
    putreg32(0x0, 0x501c0138);
//hal_lcdc_lchromakeyenable lid:0  ena:0
    putreg32(0x27f00, 0x501c0194);
    putreg32(0x0, 0x501c0084);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x401, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x400, 0x501c0264);
//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:480
    putreg32(0x1e0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_lpfc_fmt lid:1  fmt:13
    putreg32(0x400, 0x501c0264);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
    putreg32(0x0, 0x501c0130);
    putreg32(0x0, 0x501c0134);
    putreg32(0x0, 0x501c0138);
//hal_lcdc_lchromakeyenable lid:1  ena:0
//hal_lcdc_lchromakeyenable invalid  lid:1
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x401, 0x501c0264);
//fb_init fb:24000b94
//fb_ioctl fd:24000b94 cmd: 9 arg: 24000bd4

//oinfo:24000bd4 fb:24000b94 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//fb_ioctl fd:24000b94 cmd: 9 arg: 24000bfc

//oinfo:24000bfc fb:24000b94 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//fb_init fb:24000ba4
//fb_ioctl fd:24000ba4 cmd: 9 arg: 24000bd4

//oinfo:24000bd4 fb:24000ba4 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//fb_ioctl fd:24000ba4 cmd: 9 arg: 24000bfc

//oinfo:24000bfc fb:24000ba4 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//360x360, 32bpp

//The phy mem = 0x0, total size = 518400(byte)

//xres =  360, yres =  360, bits_per_pixel = 32

//So the screensize = 518400(byte), using 1 frame

//vinfo.xoffset = 0, vinfo.yoffset = 0

//vinfo.vmode is :0

//finfo.ypanstep is :0

//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:24000b94 cmd: 9 arg: 2417ff90

//oinfo:2417ff90 fb:24000b94 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//fb_ioctl fd:24000b94 cmd: 19 arg: 2417ff80

//fb_ioctl fd:24000b94 cmd: 13 arg: 2417ff90

//dma2d_fillcolor oinfo=0x24000244, argb=ff00ff00

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ff00ff00

    putreg32(0xff00ff00, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20400000
    putreg32(0x20400000, 0x501d00c0);
//0x20400000, offset=0

//hal_dma2d_lstartaddr lid:2 addr:20400000
    putreg32(0x20400000, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x20400000
//lcdc set background fb1 color with red color
//fb_ioctl fd:24000b94 cmd: 9 arg: 2417ffb8

//oinfo:2417ffb8 fb:24000b94 fb->vtable:240001c4 fb->vtable->getoverlayinfo:222c5
//fb_ioctl fd:24000b94 cmd: 13 arg: 2417ffb8

//dma2d_fillcolor oinfo=0x24000280, argb=ffff0000

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x10000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
    putreg32(0x401, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:13
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x21, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=ffff0000

    putreg32(0xffff0000, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:0 addr:2040e100
    putreg32(0x2040e100, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:32
//hal_dma2d_lpitch lid:0 pitch:480
    putreg32(0x1e0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:480
    putreg32(0x1e00021, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color pass  fbmem:0x2040e100
//display  foreground fb0
//fb_ioctl fd:24000b94 cmd: 11 arg: 2417ff90

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc8, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x10000, 0x501c01c0);
    while (1) ;
}

static void lcdc_dither_test(void)
{

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//hal_lcdc_stby_vsync_set
    putreg32(0x10501080, 0x501c0218);
//hal_lcdc_porch_vsync_set
    putreg32(0xa020020, 0x501c021c);
//hal_lcdc_pix_vsync_set
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_bgl_hporch_set f:19 b:12
    putreg32(0xc0013, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:11 b:10
    putreg32(0xa000b, 0x501c0120);
//hal_lcdc_bgl_vhtotal_set w:141 h:151
    putreg32(0x97008d, 0x501c0114);
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x1, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:11
    putreg32(0x1, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:240
    putreg32(0xf00001, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x200000, 0x501c01c0);
    putreg32(0xc004002, 0x501c0210);
    putreg32(0xc004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:5
    putreg32(0x501, 0x501c0188);
    putreg32(0x4c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80000000, 0x501c0194);
    putreg32(0x78, 0x501c0214);
    putreg32(0x780000, 0x501c0214);
    putreg32(0x501, 0x501c0188);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0xf00801, 0x501c0200);
    putreg32(0x0, 0x501c01c0);
    putreg32(0x22010000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//hal_lcdc_dither_s4x8
    putreg32(0x2, 0x501c01ec);
//hal_lcdc_dither
    putreg32(0x13, 0x501c01ec);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//Initialize DMA2D driver

    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    dma2dinitialize();
//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x0, 0x501c0190);
//hal_lcdc_lstartaddr lid:0 addr:20407080
    putreg32(0x20407080, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:240
    putreg32(0xf0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_lpfc_fmt lid:0  fmt:11
    putreg32(0x0, 0x501c0190);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x1, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x0, 0x501c0264);
//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:240
    putreg32(0xf0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_lpfc_fmt lid:1  fmt:11
    putreg32(0x0, 0x501c0264);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x1, 0x501c0264);
//fb_init fb:240025d0
//fb_ioctl fd:240025d0 cmd: 9 arg: 24002610

//oinfo:24002610 fb:240025d0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//fb_ioctl fd:240025d0 cmd: 9 arg: 24002638

//oinfo:24002638 fb:240025d0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//The framebuffer device was mapped to memory successfully.

//fb_init fb:240025e0
//fb_ioctl fd:240025e0 cmd: 9 arg: 24002610

//oinfo:24002610 fb:240025e0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//fb_ioctl fd:240025e0 cmd: 9 arg: 24002638

//oinfo:24002638 fb:240025e0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:240025d0 cmd: 9 arg: 2417ff88

//oinfo:2417ff88 fb:240025d0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//dither table[0]:0x41c6
//dither table[1]:0x967e
//dither table[2]:0x2781
//dither table[3]:0xc46b
//fb_ioctl fd:240025d0 cmd: 20 arg: 2417ff80

//hal_lcdc_dither_write
    putreg32(0x13, 0x501c01ec);
    putreg32(0x41c6, 0x501c01f0);
//hal_lcdc_dither_write
    putreg32(0x10013, 0x501c01ec);
    putreg32(0x967e, 0x501c01f0);
//hal_lcdc_dither_write
    putreg32(0x20013, 0x501c01ec);
    putreg32(0x2781, 0x501c01f0);
//hal_lcdc_dither_write
    putreg32(0x30013, 0x501c01ec);
    putreg32(0xc46b, 0x501c01f0);
//hal_dma2d_dither_write
    putreg32(0x0, 0x501d01ec);
    putreg32(0x41c6, 0x501d01f0);
//hal_dma2d_dither_write
    putreg32(0x10000, 0x501d01ec);
    putreg32(0x967e, 0x501d01f0);
//hal_dma2d_dither_write
    putreg32(0x20000, 0x501d01ec);
    putreg32(0x2781, 0x501d01f0);
//hal_dma2d_dither_write
    putreg32(0x30000, 0x501d01ec);
    putreg32(0xc46b, 0x501d01f0);
//fb_ioctl fd:240025d0 cmd: 13 arg: 2417ff88

//dma2d_fillcolor oinfo=0x24000250, argb=00001861

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:11
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
//hal_dma2d_dither_s4x8
    putreg32(0x2, 0x501d01ec);
//hal_dma2d_dither
    putreg32(0x13, 0x501d01ec);
    putreg32(0x1, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:11
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=00001861

    putreg32(0x1861, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x20407080, offset=0

//hal_dma2d_lstartaddr lid:0 addr:20407080
    putreg32(0x20407080, 0x501d00c0);
//0x20407080, offset=0

//hal_dma2d_lstartaddr lid:2 addr:20407080
    putreg32(0x20407080, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:16
//hal_dma2d_lpitch lid:0 pitch:240
    putreg32(0xf0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:240
    putreg32(0xf00001, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0xf00801, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color_565 pass  fbmem:0x20407080
//lcdc set background fb1 color with red color
//fb_ioctl fd:240025d0 cmd: 9 arg: 2417ffb0

//oinfo:2417ffb0 fb:240025d0 fb->vtable:240001dc fb->vtable->getoverlayinfo:227b5
//fb_ioctl fd:240025d0 cmd: 13 arg: 2417ffb0

//dma2d_fillcolor oinfo=0x2400028c, argb=0000ffff

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
//hal_dma2dl_enable lid:1 enable:0
    putreg32(0x0, 0x501d0264);
//hal_dma2dl_enable lid:0 enable:1
    putreg32(0x1, 0x501d0190);
//hal_dma2dl_enable lid:2 enable:1
    putreg32(0x1, 0x501d0200);
//hal_dma2dl_enable lid:3 enable:1
    putreg32(0x1, 0x501d0188);
//hal_dma2d_lpfc lid:0 bmode:2 alpha:0 fmt:11
//hal_dma2d_lpfc_swaprb lid=0, ena=00000000

    putreg32(0x1, 0x501d0190);
//hal_dma2d_dither_s4x8
    putreg32(0x2, 0x501d01ec);
//hal_dma2d_dither
    putreg32(0x13, 0x501d01ec);
    putreg32(0x1, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
//hal_dma2d_lpfc lid:2 bmode:0 alpha:0 fmt:11
//hal_dma2d_lpfc_swaprb lid=2, ena=00000000

    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
//hal_dma2d_swap_path lid:0
    putreg32(0x0, 0x501d01dc);
//hal_dma2d_swap_path_enable enable:1
    putreg32(0x10000, 0x501d01dc);
//hal_dma2d_lblend first:0
    putreg32(0x0, 0x501d00ec);
//lid=3, argb=0000ffff

    putreg32(0xffff, 0x501d0124);
//hal_dma2d_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501d0118);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:0 addr:2040e100
    putreg32(0x2040e100, 0x501d00c0);
//0x2040e100, offset=0

//hal_dma2d_lstartaddr lid:2 addr:2040e100
    putreg32(0x2040e100, 0x501d0208);
//dma2d_fillcolor xres:120 bpp:16
//hal_dma2d_lpitch lid:0 pitch:240
    putreg32(0xf0, 0x501d00e0);
//hal_dma2d_lpitch lid:2 pitch:240
    putreg32(0xf00001, 0x501d0200);
//hal_dma2d_lsize lid:0 w:4 h:4
    putreg32(0x40004, 0x501d00ec);
//hal_dma2d_lsize lid:1 w:4 h:4
    putreg32(0x40004, 0x501d0104);
//hal_dma2d_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501d0204);
//hal_dma2d_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
//hal_dma2d_smpn_mode_set  mode:3
    putreg32(0x301, 0x501d0188);
//hal_dma2d_wdma_1fpt_set
    putreg32(0x10780078, 0x501d0204);
//hal_dma2d_ol_dmaburst_length_set
    putreg32(0xf00801, 0x501d0200);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
//hal_dma2d_vsync_porch_set
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
//hal_dma2d_bgl_vhtotal_set w:122 h:122
    putreg32(0x7a007a, 0x501d0114);
//hal_dma2d_bgl_hporch_set f:12 b:10
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
//hal_dma2d_bgl_vporch_set f:10 b:10
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//dma2d irq = 0, regval = 00a20005

//DMA transfer complete

//hal_dma2d_spu_framedone_irqstat_clr
//putreg32(0x824005,0x501d01c4);
//check_overlay_color_565 pass  fbmem:0x2040e100
//display  foreground fb0
//fb_ioctl fd:240025d0 cmd: 11 arg: 2417ff88

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0x1ffff, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20420000
    putreg32(0x20420000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x22010000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result_565 pass  wbp:0x20420000

    while (1) ;
}

void lcdc_blit_sim_38100000(void)
{

    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//hal_lcdc_stby_vsync_set
    putreg32(0x10501080, 0x501c0218);
//hal_lcdc_porch_vsync_set
    putreg32(0xa020020, 0x501c021c);
//hal_lcdc_pix_vsync_set
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_bgl_hporch_set f:19 b:12
    putreg32(0xc0013, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:11 b:10
    putreg32(0xa000b, 0x501c0120);
//hal_lcdc_bgl_vhtotal_set w:141 h:151
    putreg32(0x97008d, 0x501c0114);
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80027f00, 0x501c0194);
    putreg32(0x78, 0x501c0214);
    putreg32(0x780000, 0x501c0214);
    putreg32(0x301, 0x501c0188);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
    putreg32(0x0, 0x501c01c0);
    putreg32(0x22010000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//hal_lcdc_dither_s4x8
    putreg32(0x30013, 0x501c01ec);
//hal_lcdc_dither
    putreg32(0x30013, 0x501c01ec);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    dma2dinitialize();
//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x0, 0x501c0190);
//hal_lcdc_lstartaddr lid:0 addr:20407080
    putreg32(0x20407080, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:240
    putreg32(0xf0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_lpfc_fmt lid:0  fmt:11
    putreg32(0x0, 0x501c0190);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x1, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x0, 0x501c0264);
//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:240
    putreg32(0xf0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_lpfc_fmt lid:1  fmt:11
    putreg32(0x0, 0x501c0264);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x1, 0x501c0264);
//fb_init fb:240145cc
//fb_ioctl fd:240145cc cmd: 9 arg: 2401460c

//oinfo:2401460c fb:240145cc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//fb_ioctl fd:240145cc cmd: 9 arg: 24014634

//oinfo:24014634 fb:240145cc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//The framebuffer device was mapped to memory successfully.

//fb_init fb:240145dc
//fb_ioctl fd:240145dc cmd: 9 arg: 2401460c

//oinfo:2401460c fb:240145dc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//fb_ioctl fd:240145dc cmd: 9 arg: 24014634

//oinfo:24014634 fb:240145dc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:240145cc cmd: 9 arg: 2417ff88

//oinfo:2417ff88 fb:240145cc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//fb_ioctl fd:240145cc cmd: 20 arg: 2417ff6c

    putreg32(0x13, 0x501c01ec);
    putreg32(0xffffffff, 0x501c01f0);
    putreg32(0x10013, 0x501c01ec);
    putreg32(0xffffffff, 0x501c01f0);
    putreg32(0x20013, 0x501c01ec);
    putreg32(0xffffffff, 0x501c01f0);
    putreg32(0x30013, 0x501c01ec);
    putreg32(0xffffffff, 0x501c01f0);
    putreg32(0x0, 0x501d01ec);
    putreg32(0xffffffff, 0x501d01f0);
    putreg32(0x10000, 0x501d01ec);
    putreg32(0xffffffff, 0x501d01f0);
    putreg32(0x20000, 0x501d01ec);
    putreg32(0xffffffff, 0x501d01f0);
    putreg32(0x30000, 0x501d01ec);
    putreg32(0xffffffff, 0x501d01f0);
//fb_ioctl fd:240145cc cmd: 13 arg: 2417ff88

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    putreg32(0x0, 0x501d0264);
    putreg32(0x1, 0x501d0190);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0188);
    putreg32(0x1, 0x501d0190);
    putreg32(0x2, 0x501d01ec);
    putreg32(0x13, 0x501d01ec);
    putreg32(0x1, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
    putreg32(0x0, 0x501d01dc);
    putreg32(0x10000, 0x501d01dc);
    putreg32(0x0, 0x501d00ec);
    putreg32(0x1861, 0x501d0124);
    putreg32(0x790079, 0x501d0118);
    putreg32(0x20407080, 0x501d00c0);
    putreg32(0x20407080, 0x501d0208);
    putreg32(0xf0, 0x501d00e0);
    putreg32(0xf00001, 0x501d0200);
    putreg32(0x40004, 0x501d00ec);
    putreg32(0x40004, 0x501d0104);
    putreg32(0x780078, 0x501d0204);
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    putreg32(0x301, 0x501d0188);
    putreg32(0x10780078, 0x501d0204);
    putreg32(0xf00801, 0x501d0200);
    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    putreg32(0x7a007a, 0x501d0114);
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//DMA transfer complete

//putreg32(0x824005,0x501d01c4);
//check_overlay_color_565 pass  fbmem:0x20407080
//lcdc set background fb1 color with red color
//fb_ioctl fd:240145cc cmd: 9 arg: 2417ffb0

//oinfo:2417ffb0 fb:240145cc fb->vtable:240001dc fb->vtable->getoverlayinfo:223bd
//fb_ioctl fd:240145cc cmd: 13 arg: 2417ffb0

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    putreg32(0x0, 0x501d0264);
    putreg32(0x1, 0x501d0190);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0188);
    putreg32(0x1, 0x501d0190);
    putreg32(0x2, 0x501d01ec);
    putreg32(0x13, 0x501d01ec);
    putreg32(0x1, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
    putreg32(0x0, 0x501d01dc);
    putreg32(0x10000, 0x501d01dc);
    putreg32(0x0, 0x501d00ec);
    putreg32(0xffff, 0x501d0124);
    putreg32(0x790079, 0x501d0118);
    putreg32(0x2040e100, 0x501d00c0);
    putreg32(0x2040e100, 0x501d0208);
    putreg32(0xf0, 0x501d00e0);
    putreg32(0xf00001, 0x501d0200);
    putreg32(0x40004, 0x501d00ec);
    putreg32(0x40004, 0x501d0104);
    putreg32(0x780078, 0x501d0204);
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    putreg32(0x301, 0x501d0188);
    putreg32(0x10780078, 0x501d0204);
    putreg32(0xf00801, 0x501d0200);
    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    putreg32(0x7a007a, 0x501d0114);
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//DMA transfer complete

//putreg32(0x824005,0x501d01c4);
//check_overlay_color_565 pass  fbmem:0x2040e100
//display  foreground fb0
//fb_ioctl fd:240145cc cmd: 11 arg: 2417ff88

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc8, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x22010000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result pass  wbp:0x20460000
//display  background fb1
//fb_ioctl fd:240145cc cmd: 11 arg: 2417ff88

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:255
    putreg32(0x2ff00, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc0, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x22010000, 0x501c01c0);
//lcdcirqstatus:0xf
//check_outlayer_result pass  wbp:0x20460000
//lcdc dma2d  fill fb0 with green color
//lcdc dma2d  fill fb1 with red   color
//fb0 fb1 blending display
//fb_ioctl fd:240145cc cmd: 11 arg: 2417ff88

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc0, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x22010000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result_blend pass  wbp:0x20460000
//lcdc dma2d copy fb0 to fb1 with scale
//fb_ioctl fd:240145cc cmd: 16 arg: 2417ff74

    hal_dma2d_reset();
    putreg32(0x0, 0x501d0188);
    putreg32(0x0, 0x501d0200);
    putreg32(0x0, 0x501d0190);
    putreg32(0x0, 0x501d0264);
    putreg32(0x0, 0x501d0194);
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    putreg32(0x0, 0x501d0264);
    putreg32(0x1, 0x501d0190);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0188);
    putreg32(0x1, 0x501d0190);
    putreg32(0x2, 0x501d01ec);
    putreg32(0x13, 0x501d01ec);
    putreg32(0x1, 0x501d0190);
    putreg32(0x0, 0x501d0194);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
    putreg32(0x0, 0x501d0124);
    putreg32(0x0, 0x501d01dc);
    putreg32(0x10000, 0x501d01dc);
    putreg32(0x0, 0x501d00ec);
    putreg32(0x30005, 0x501d0118);
    putreg32(0x20407080, 0x501d00c0);
    putreg32(0x38100000, 0x501d0208);
    putreg32(0xf0, 0x501d00e0);
    putreg32(0x80001, 0x501d0200);
    putreg32(0x20004, 0x501d00f0);
    putreg32(0x20004, 0x501d0108);
    putreg32(0x10001, 0x501d00e8);
    putreg32(0x10001, 0x501d0100);
    putreg32(0x2000b, 0x501d00ec);
    putreg32(0x2000b, 0x501d0104);
    putreg32(0x20004, 0x501d0204);
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    putreg32(0x501, 0x501d0188);
    putreg32(0x10020004, 0x501d0204);
    putreg32(0x80801, 0x501d0200);
    putreg32(0x80000000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    putreg32(0x4000d, 0x501d0114);
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//DMA transfer complete

    putreg32(0x8082408d, 0x501d01c4);
    while (1) ;
}

void lcdc_yuv_test_sim(void)
{
    hal_lcdc_reset();
    *((volatile uint32_t *)(AON_PSC_BASE + 0xbc)) = 0xcafe0bf0;
//hal_lcdc_stby_vsync_set
    putreg32(0x10501080, 0x501c0218);
//hal_lcdc_porch_vsync_set
    putreg32(0xa020020, 0x501c021c);
//hal_lcdc_pix_vsync_set
    putreg32(0x1f001e, 0x501c0220);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_bgl_hporch_set f:19 b:12
    putreg32(0xc0013, 0x501c011c);
//hal_lcdc_bgl_vporch_set f:11 b:10
    putreg32(0xa000b, 0x501c0120);
//hal_lcdc_bgl_vhtotal_set w:141 h:151
    putreg32(0x97008d, 0x501c0114);
//hal_lcdcl_enable lid:3 enable:1
    putreg32(0x301, 0x501c0188);
//hal_lcdc_lsize lid:3 w:120 h:120
    putreg32(0x790079, 0x501c0118);
//hal_lcdc_select_top lid:0
    putreg32(0x0, 0x501c01e8);
//hal_lcdcl_enable lid:2 enable:1
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lsize lid:2 w:120 h:120
    putreg32(0x10780078, 0x501c0204);
//hal_lcdc_lpfc_fmt lid:2  fmt:13
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
//hal_lcdc_lpitch lid:2 pitch:480
    putreg32(0x1e00821, 0x501c0200);
//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x6c004003, 0x501c0210);
//hal_lcdc_smpn_mode_set  mode:3
    putreg32(0x301, 0x501c0188);
    putreg32(0x6c004003, 0x501c0210);
//lid=0, ena=00000001

//lid=0, set  ena=00000001

    putreg32(0x80027f00, 0x501c0194);
    putreg32(0x78, 0x501c0214);
    putreg32(0x780000, 0x501c0214);
    putreg32(0x301, 0x501c0188);
//hal_lcdc_ol_dmaburst_length_set
    putreg32(0x1e00821, 0x501c0200);
    putreg32(0x0, 0x501c01c0);
    putreg32(0x22010000, 0x501c01c0);
    g_lock = 1;
    g_interrupt.sem = 0;
    g_interrupt.irq = hal_lcdc_irqn();
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_EnableIRQ(g_interrupt.irq);
//hal_lcdc_dither_s4x8
    putreg32(0x30013, 0x501c01ec);
//hal_lcdc_dither
    putreg32(0x30012, 0x501c01ec);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    dma2dinitialize();
//hal_lcdcl_enable lid:0 enable:0
    putreg32(0x0, 0x501c0190);
//hal_lcdc_lstartaddr lid:0 addr:20407080

    {
        int count = 60;
        volatile uint32_t *fb = (uint32_t *) 0x20407080;
        for (int i = 0; i < count; i++) {
            fb[i] = 0xff800080;
        }

    }

    putreg32(0x20407080, 0x501c00c0);
//hal_lcdc_lsize lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00ec);
//hal_lcdc_lpitch lid:0 pitch:240
    putreg32(0xf0, 0x501c00e0);
//hal_lcdc_lzoom_set lid:0 w:120 h:120
    putreg32(0x780078, 0x501c00f0);
//hal_lcdc_sponscreen_set lid:0 vln:1 hpixl:1
    putreg32(0x10001, 0x501c00e8);
//hal_lcdc_lpfc_fmt lid:0  fmt:45
    putreg32(0x502, 0x501c0190);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:127
    putreg32(0x27f00, 0x501c0194);
//hal_lcdcl_enable lid:0 enable:1
    putreg32(0x503, 0x501c0190);
//hal_lcdcl_enable lid:1 enable:0
    putreg32(0x0, 0x501c0264);
//hal_lcdc_lstartaddr lid:1 addr:2040e100
    putreg32(0x2040e100, 0x501c00f4);
//hal_lcdc_lsize lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0104);
//hal_lcdc_lpitch lid:1 pitch:240
    putreg32(0xf0, 0x501c00fc);
//hal_lcdc_lzoom_set lid:1 w:120 h:120
    putreg32(0x780078, 0x501c0108);
//hal_lcdc_sponscreen_set lid:1 vln:1 hpixl:1
    putreg32(0x10001, 0x501c0100);
//hal_lcdc_lpfc_fmt lid:1  fmt:11
    putreg32(0x0, 0x501c0264);
//lid=3, argb=00000000

    putreg32(0x0, 0x501c0124);
//hal_lcdc_lpfc_ltransp lid:1 bmode:0 alpha:127
//hal_lcdcl_enable lid:1 enable:1
    putreg32(0x1, 0x501c0264);
//fb_init fb:2401457c
//fb_ioctl fd:2401457c cmd: 9 arg: 240145bc

//oinfo:240145bc fb:2401457c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//fb_ioctl fd:2401457c cmd: 9 arg: 240145e4

//oinfo:240145e4 fb:2401457c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//The framebuffer device was mapped to memory successfully.

//fb_init fb:2401458c
//fb_ioctl fd:2401458c cmd: 9 arg: 240145bc

//oinfo:240145bc fb:2401458c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//fb_ioctl fd:2401458c cmd: 9 arg: 240145e4

//oinfo:240145e4 fb:2401458c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//The framebuffer device was mapped to memory successfully.

//lcdc set foreground fb0 color with green color
//fb_ioctl fd:2401457c cmd: 9 arg: 2417ff90

//oinfo:2417ff90 fb:2401457c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//check_overlay_color fail oinfo->fbmem[0] color: ff800080  != ff00
//lcdc set background fb1 color with red color
//fb_ioctl fd:2401457c cmd: 9 arg: 2417ffb8

//oinfo:2417ffb8 fb:2401457c fb->vtable:240001cc fb->vtable->getoverlayinfo:221a9
//fb_ioctl fd:2401457c cmd: 13 arg: 2417ffb8

    hal_dma2d_reset();
    putreg32(0x0, 0x501d01c0);
    putreg32(0x22010000, 0x501d01c0);
    putreg32(0x0, 0x501d0264);
    putreg32(0x1, 0x501d0190);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0188);
    putreg32(0x1, 0x501d0190);
    putreg32(0x1, 0x501d0190);
    putreg32(0x20000, 0x501d0194);
    putreg32(0x1, 0x501d0200);
    putreg32(0x1, 0x501d0200);
    putreg32(0x0, 0x501d01dc);
    putreg32(0x10000, 0x501d01dc);
    putreg32(0x0, 0x501d00ec);
    putreg32(0xffff0000, 0x501d0124);
    putreg32(0x790079, 0x501d0118);
    putreg32(0x2040e100, 0x501d00c0);
    putreg32(0x2040e100, 0x501d0208);
    putreg32(0xf0, 0x501d00e0);
    putreg32(0xf00001, 0x501d0200);
    putreg32(0x40004, 0x501d00ec);
    putreg32(0x40004, 0x501d0104);
    putreg32(0x780078, 0x501d0204);
    putreg32(0x22210000, 0x501d01c0);
    putreg32(0xc004002, 0x501d0210);
    putreg32(0xc004003, 0x501d0210);
    putreg32(0x301, 0x501d0188);
    putreg32(0x10780078, 0x501d0204);
    putreg32(0xf00801, 0x501d0200);
    putreg32(0x80020000, 0x501d0194);
    putreg32(0x4c004003, 0x501d0210);
    putreg32(0x501080, 0x501d0218);
    putreg32(0xa020020, 0x501d021c);
    putreg32(0x1f001e, 0x501d0220);
    putreg32(0x7a007a, 0x501d0114);
    putreg32(0xc, 0x501d011c);
    putreg32(0xa000c, 0x501d011c);
    putreg32(0xa, 0x501d0120);
    putreg32(0xa000a, 0x501d0120);
    putreg32(0x1, 0x501d0224);
//hal_dma2d_start

    dma2d_waitforirq();
//DMA transfer complete

//putreg32(0x824005,0x501d01c4);
//check_overlay_color fail oinfo->fbmem[0] color: 0  != ff0000
//display  foreground fb0
//fb_ioctl fd:2401457c cmd: 11 arg: 2417ff90

//hal_lcdc_lpfc_ltransp lid:0 bmode:0 alpha:0
    putreg32(0x20000, 0x501c0194);
    putreg32(0xcafe0001, 0x50000080);
    putreg32(0xc8, 0x50000170);
    putreg32(0xc0, 0x50000174);
    putreg32(0x0, 0x50000080);
//hal_lcdc_wdma_toggle_frame
    putreg32(0x780078, 0x501c0204);
//hal_lcdc_lstartaddr lid:2 addr:20460000
    putreg32(0x20460000, 0x501c0208);
    putreg32(0x6c004003, 0x501c0210);
    putreg32(0x1, 0x501c0224);
//hal_lcdc_start_wb

//hal_lcdc_spu_framedone_irq_enable enable:1
    putreg32(0x22210000, 0x501c01c0);
    lcdc_waitforirq();
//LCDC DMA transfer complete

//hal_lcdc_spu_framedone_irqstat_clr
//putreg32(0x0,0x501c01c4);
//hal_lcdc_spu_framedone_irq_enable enable:0
    putreg32(0x22010000, 0x501c01c0);
//lcdcirqstatus:0xcf
//check_outlayer_result fail wbp[0] color: 808000  != ff4d00
    while (1) ;
}

void lcdc_test_sim(void)
{

    //enable sram 0x38000000 -
    *(volatile uint32_t *)0x400850d0 = 0xcafe00fb;

    uint32_t *st = (volatile uint32_t *)0x38100000;
    volatile uint32_t v;
    for (int i = 0; i < 20; i++) {
        st[i] = 0x0;
    }

    *(volatile uint32_t *)0x38100000 = 0xa;

    for (int i = 0; i < 20; i++) {
        v = st[i];
        //LCDC_ERR("x%x",st[i]);
    }
    while (1) ;

    // lcdc_yuv_test_sim();
    //  lcdc_blit_sim_38100000();
    //lcdc_dither_test();
    //lcdc_chromakey_test();
    //lcdc_coloradjcurve_test();
}

#endif //LCDC_SIMU_TEST_CODE
