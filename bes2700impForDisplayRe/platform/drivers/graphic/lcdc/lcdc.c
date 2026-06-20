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
 * DOC:The Display Controller operates in Slave mode and DMA mode.
 *
 * Slave mode is for the processor to read/write Smart Panel, and DMA mode is for
 *  the Display Controller to move pixels to either the Smart Panel or the Dumb Panels.
 * There are total of two layers that can be overlaid onto the display screen,
 *
 * The following operations will be done during DMA transfer:

 * image enhancement including contrast/saturation/brightness/hue adjust on the
 * two fore/back ground layes
 * palette color lookup on the two graphic paths, supporting both 4-bit/pixel and
 *  8-bit/pixel.
 * alpha blending in overlaid area for all paths, with configured alpha, alpha
 * combined with pixel and color key alpha.
 * partial display DMA when part of the picture is covered by another picture,
 * reducing bandwidth (refer to register bit descriptions for more information).

 * RGB gamma correction after alpha blending, up to two interfaces.
 * horizontal and vertical scaling and filtering on all of the graphic paths.
 * selec vertical mirror on all of the graphic objects.
 * dither function enable.
 *
 * The base addresses of the Display Controller registers is: 0x501c_0000.

 * LCDC hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,YUV422packed,
 *   YUV422 planar, YUV420 planar, Palette color 4-bit per pixel, Palette color
 *   8-bit per pixel, RGBA )
 * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,YUV422
 *  packed,Palette color 4-bit per pixel,Palette color 8-bit per pixel,RGB8A )
 *  (OL color fmt:RGB565 RGB888 )
 *
 *  |FG Layer(panel video dma layer) |-->|
 *                                       | Alpha_FG (pix/c)
 *                                       V
 *                                      |BLENDING|-->|Smart Panel|-|dither|->->|dsi|
 *                                       ^     Alpha_O (pix/c)               |
 *                                       | Alpha_BG (pix/c)                  |
 *  |BG Layer(panel graphic dma layer)|->|             ^                     |
 *                                                     |                     |
 *  |TV Path(not used currently by lcdc driver)|-------|                     |
 *                                                                           |
 *                                                                           |
 *  |write back layer)|<------------------------------------------------------
 */

#include <stdio.h>
#include "string.h"
#include "cmsis.h"
#if defined(__NuttX__)
#include <nuttx/config.h>
#include <nuttx/video/fb.h>
#include <arch/board/board.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/irq.h>
#include <debug.h>
#include <nuttx/power/pm.h>
#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)
#else
#include "fb.h"
#ifdef CONFIG_PM
#include "pm.h"
#endif
#endif
#include "lcdc.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_dma2d.h"
#include "dma2d.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"

#if defined(CONFIG_FBMEM_CACHED)
#define LCDC_DRV_WITH_CACHEOPS
#endif

#include "hal_trace.h"
#include "local_sem.h"
#include "lpcmd.h"

#if defined(CONFIG_VIDEO_FB)
#if !(defined(CONFIG_LCDC_L1) || defined(CONFIG_LCDC_L2))
#define CONFIG_LCDC_L1_ARGB8888
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE) && defined(CHIP_BEST2003) && !defined(CHIP_BEST2003_DSP)
#define CONFIG_DSI_UNIFIED_IRQ
#endif

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE) && (defined(CONFIG_DSI_SLEEP) || defined(CONFIG_LCDC_SLEEP))
#define CONFIG_DSI_GPIO_TE_IRQ
#endif

#if defined(CONFIG_DSI_GPIO_TE_IRQ)
#undef CONFIG_DSI_UNIFIED_IRQ
#endif

/**
 * Board LCDC Configuration
 */

#define LCDC_HEIGHT                 BOARD_LCDC_HEIGHT
#define LCDC_WIDTH                  BOARD_LCDC_WIDTH

#define CONFIG_DMA2D_LAYER_PPLINE   BOARD_LCDC_WIDTH

#define CONFIG_DMA2D_FB_SIZE        (LCDC_HEIGHT*LCDC_WIDTH*4*2)

//#define CONFIG_DMA2D_L8
#define CONFIG_LCDC_DITHER_RED     BOARD_LCDC_GCR_DRW

#define CONFIG_LCDC_DITHER_GREEN   BOARD_LCDC_GCR_DGW

#define CONFIG_LCDC_DITHER_BLUE    BOARD_LCDC_GCR_DBW

#define CONFIG_LCDC_L1_WIDTH BOARD_LCDC_WIDTH

#define CONFIG_LCDC_L2_WIDTH BOARD_LCDC_WIDTH

#define CONFIG_LCDC_L1_HEIGHT BOARD_LCDC_HEIGHT

#define CONFIG_LCDC_L2_HEIGHT BOARD_LCDC_HEIGHT

#define LCDC_LxWHPCR_WHSTPOS  (BOARD_LCDC_HSYNC + BOARD_LCDC_HBP - 1)

#define LCDC_LxWHPCR_WHSPPOS  (BOARD_LCDC_HSYNC + BOARD_LCDC_HBP + \
                                    LCDC_WIDTH - 1)

#define LCDC_LxWVPCR_WVSTPOS  (BOARD_LCDC_VSYNC + BOARD_LCDC_VBP - 1)

#define LCDC_LxWVPCR_WVSPPOS  (BOARD_LCDC_VSYNC + BOARD_LCDC_VBP + \
                                    LCDC_HEIGHT - 1)

#define LCDC_BPCR_AVBP        (LCDC_LxWVPCR_WVSTPOS)

#define LCDC_BPCR_AHBP        (LCDC_LxWHPCR_WHSTPOS)

#define LCDC_TWCR_TOTALH      (BOARD_LCDC_VSYNC + \
                                    BOARD_LCDC_VBP + \
                                    LCDC_HEIGHT + BOARD_LCDC_VFP - 1)
#define LCDC_TWCR_TOTALW      (BOARD_LCDC_HSYNC + \
                                    BOARD_LCDC_HBP + \
                                    LCDC_WIDTH + BOARD_LCDC_HFP - 1)

/* Synchronisation and Polarity */

#define LCDC_GCR_PCPOL        (BOARD_LCDC_GCR_PCPOL)
#define LCDC_GCR_DEPOL        (BOARD_LCDC_GCR_DEPOL)

#define LCDC_GCR_VSPOL        (BOARD_LCDC_GCR_VSPOL)
#define LCDC_GCR_HSPOL        (BOARD_LCDC_GCR_HSPOL)

/* Dither */

#define LCDC_GCR_DEN          (BOARD_LCDC_GCR_DEN)
#define LCDC_GCR_DBW          (BOARD_LCDC_GCR_DBW)
#define LCDC_GCR_DGW          (BOARD_LCDC_GCR_DGW)
#define STN32_LCDC_GCR_DRW    (BOARD_LCDC_GCR_DRW)

#define LCDC_LIPCR_LIPOS      (LCDC_TWCR_TOTALW)

/**
 * LCDC Layer Configuration
 */

#ifndef CONFIG_LCDC_DEFBACKLIGHT
#define CONFIG_LCDC_DEFBACKLIGHT 0xf0
#endif
#define LCDC_BACKLIGHT_OFF 0x00

/* Color/video formats */

/* Layer 1 format */

#if defined(CONFIG_LCDC_L1_L8)
#define LCDC_L1_BPP         8
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB8
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#define LCDC_L1_DMA2D_PF    FB_FMT_RGB8
#define LCDC_L1CMAP
#elif defined(CONFIG_LCDC_L1_RGB565)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB16_565
#define LCDC_L1PFCR_PF      FB_FMT_RGB16_565
#define LCDC_L1_DMA2D_PF    FB_FMT_RGB16_565
#elif defined(CONFIG_LCDC_L1_RGB888)
#define LCDC_L1_BPP         24
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB24
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#define LCDC_L1_DMA2D_PF    FB_FMT_RGB24
#elif defined(CONFIG_LCDC_L1_ARGB8888)
#define LCDC_L1_BPP         32
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB32
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#define LCDC_L1_DMA2D_PF    FB_FMT_RGB32
#elif defined(CONFIG_LCDC_L1_YUV422)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_YUVP
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#define LCDC_L1_DMA2D_PF    FB_FMT_YUVP
#else
#error "LCDC pixel format not supported"
#endif

/* Layer 2 format */

#ifdef CONFIG_LCDC_L2
#if defined(CONFIG_LCDC_L2_L8)
#define LCDC_L2_BPP         8
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB8
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#define LCDC_L2_DMA2D_PF    FB_FMT_RGB8
#define LCDC_L2CMAP
#elif defined(CONFIG_LCDC_L2_RGB565)
#define LCDC_L2_BPP         16
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB16_565
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#define LCDC_L2_DMA2D_PF    FB_FMT_RGB16_565
#elif defined(CONFIG_LCDC_L2_RGB888)
#define LCDC_L2_BPP         24
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB24
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#define LCDC_L2_DMA2D_PF    FB_FMT_RGB24
#elif defined(CONFIG_LCDC_L2_ARGB8888)
#define LCDC_L2_BPP         32
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB32
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#define LCDC_L2_DMA2D_PF    FB_FMT_RGB32
#else
#error "LCDC pixel format not supported"
#endif
#endif /* CONFIG_LCDC_L2 */

/*Write Back /Output Layer format*/

#if defined(CONFIG_LCDC_WB_TEST)
#if defined(CONFIG_LCDC_LO_RGB888)
#define  LCDC_LOPFCR_PF  FB_FMT_RGB32
#define  LCDC_LO_STRIDE  ((LCDC_WIDTH * 32 + 7) / 8)
#define  LCDC_LO_FBSIZE  (LCDC_LO_STRIDE * LCDC_HEIGHT)
#elif defined(CONFIG_LCDC_LO_RGB565)
#define  LCDC_LO_STRIDE  ((LCDC_WIDTH * 16 + 7) / 8)
#define  LCDC_LO_FBSIZE  (LCDC_LO_STRIDE * LCDC_HEIGHT)
#define  LCDC_LOPFCR_PF  FB_FMT_RGB16_565
#endif
#endif

/* Framebuffer sizes in bytes */

#if LCDC_L1_BPP == 8
#define LCDC_L1_STRIDE      (LCDC_WIDTH)
#elif LCDC_L1_BPP == 16
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 16 + 7) / 8)
#elif LCDC_L1_BPP == 24
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 24 + 7) / 8)
#elif LCDC_L1_BPP == 32
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 32 + 7) / 8)
#else
#error Undefined or unrecognized base resolution
#endif

/* LCDC only supports 8 bit per pixel overal */

#define LCDC_Lx_BYPP(n)       ((n) / 8)

#if defined(CONFIG_FB_DOUBLE_BUFFER)
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT *2)
#else
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT)
#endif

#ifdef CONFIG_LCDC_L2
#ifndef CONFIG_LCDC_L2_WIDTH
#define CONFIG_LCDC_L2_WIDTH LCDC_WIDTH
#endif

#if CONFIG_LCDC_L2_WIDTH > LCDC_WIDTH
#error Width of Layer 2 exceeds the width of the display
#endif

#ifndef CONFIG_LCDC_L2_HEIGHT
#define CONFIG_LCDC_L2_HEIGHT LCDC_HEIGHT
#endif

#if CONFIG_LCDC_L2_HEIGHT > LCDC_HEIGHT
#error Height of Layer 2 exceeds the height of the display
#endif

#if LCDC_L2_BPP == 8
#define LCDC_L2_STRIDE    (CONFIG_LCDC_L2_WIDTH)
#elif LCDC_L2_BPP == 16
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 16 + 7) / 8)
#elif LCDC_L2_BPP == 24
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 24 + 7) / 8)
#elif LCDC_L2_BPP == 32
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 32 + 7) / 8)
#else
#error Undefined or unrecognized base resolution
#endif

#if defined(CONFIG_FB_DOUBLE_BUFFER)
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * CONFIG_LCDC_L2_HEIGHT * 2)
#else
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * CONFIG_LCDC_L2_HEIGHT)
#endif

#else
#define LCDC_L2_FBSIZE (0)
#endif

/* Total memory used for framebuffers */
#ifdef CONFIG_LCDC_L2
#define LCDC_TOTAL_FBSIZE     (LCDC_L1_FBSIZE + \
                                     LCDC_L2_FBSIZE)
#else
#define LCDC_TOTAL_FBSIZE     (LCDC_L1_FBSIZE)
#endif
/* Debug option */
//#define GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#define LCDC_REGINFO(...)  TR_INFO( TR_MOD(TEST), __VA_ARGS__)
#else
#define LCDC_INFO(...)
#define LCDC_REGINFO(...)
#endif

#define LCDC_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

/* Preallocated LCDC framebuffers */

/**
 * Position the framebuffer memory in the center of the memory set aside.  We
 * will use any skirts before or after the framebuffer memory as a guard against
 * wild framebuffer writes.
 */

#define LCDC_BUFFER_SIZE      CONFIG_LCDC_FB_SIZE
#define LCDC_BUFFER_FREE      (LCDC_BUFFER_SIZE - LCDC_TOTAL_FBSIZE)
#define LCDC_BUFFER_START     (CONFIG_LCDC_FB_BASE + LCDC_BUFFER_FREE/2)

#if LCDC_BUFFER_FREE < 0
#error "LCDC_BUFFER_SIZE not large enough for frame buffers"
#endif

/* Layer frame buffer */

#define LCDC_BUFFER_L1        LCDC_BUFFER_START
#define LCDC_ENDBUF_L1        (LCDC_BUFFER_L1 + LCDC_L1_FBSIZE)

#ifdef CONFIG_LCDC_L2
#define LCDC_BUFFER_L2      LCDC_ENDBUF_L1
#define LCDC_ENDBUF_L2      (LCDC_BUFFER_L2 + LCDC_L2_FBSIZE)
#else
#define LCDC_ENDBUF_L2      LCDC_ENDBUF_L1
#endif

/* LCDC layer */

#ifdef CONFIG_LCDC_L2
#define LCDC_NLAYERS 2
#else
#define LCDC_NLAYERS 1
#endif

/* DMA2D layer */

#if defined(CONFIG_DMA2D) && (CONFIG_DMA2D_NLAYERS > 0)
#define LCDC_DMA2D_NLAYERS             CONFIG_DMA2D_NLAYERS
#if LCDC_DMA2D_NLAYERS < 1
#error "DMA2D must at least support 1 overlay"
#endif

#define DMA2D_WIDTH           CONFIG_DMA2D_LAYER_PPLINE

#if defined(CONFIG_DMA2D_L8)
#define DMA2D_STRIDE      (DMA2D_WIDTH)
#define DMA2D_BPP         8
#define DMA2D_COLOR_FMT   FB_FMT_RGB8
#elif defined(CONFIG_DMA2D_RGB565)
#define DMA2D_STRIDE      ((DMA2D_WIDTH * 16 + 7) / 8)
#define DMA2D_BPP         16
#define DMA2D_COLOR_FMT   FB_FMT_RGB16_565
#elif defined(CONFIG_DMA2D_RGB888)
#define DMA2D_STRIDE      ((DMA2D_WIDTH * 24 + 7) / 8)
#define DMA2D_BPP         24
#define DMA2D_COLOR_FMT   FB_FMT_RGB24
#elif defined(CONFIG_DMA2D_ARGB8888)
#define DMA2D_STRIDE      ((DMA2D_WIDTH * 32 + 7) / 8)
#define DMA2D_BPP         32
#define DMA2D_COLOR_FMT   FB_FMT_RGB32
#else
#error "DMA2D pixel format not supported"
#endif

#ifdef CONFIG_DMA2D_LAYER_SHARED
#define DMA2D_FBSIZE      CONFIG_DMA2D_FB_SIZE
#define DMA2D_LAYER_SIZE  0
#else
#define DMA2D_FBSIZE      CONFIG_DMA2D_FB_SIZE / LCDC_DMA2D_NLAYERS
#define DMA2D_LAYER_SIZE  DMA2D_FBSIZE
#if DMA2D_FBSIZE * LCDC_DMA2D_NLAYERS > CONFIG_DMA2D_FB_SIZE
#error "DMA2D framebuffer size to small for configured number of overlays"
#endif
#endif /* CONFIG_DMA2D_LAYER_SHARED */

#define DMA2D_HEIGHT         DMA2D_FBSIZE / DMA2D_STRIDE

#define DMA2D_BUFFER_START   CONFIG_DMA2D_FB_BASE
#else
#define LCDC_DMA2D_NLAYERS              0
#endif /* CONFIG_DMA2D */

#define LCDC_NOVERLAYS              LCDC_NLAYERS + LCDC_DMA2D_NLAYERS

/* Dithering */

#ifndef CONFIG_LCDC_DITHER_LEVEL
#define LCDC_DITHER_LEVEL     1
#else
#define LCDC_DITHER_LEVEL     CONFIG_LCDC_DITHER_LEVEL
#endif

#ifndef CONFIG_LCDC_DITHER_RED
#define LCDC_DITHER_RED     0
#else
#define LCDC_DITHER_RED     CONFIG_LCDC_DITHER_RED
#endif
#ifndef CONFIG_LCDC_DITHER_GREEN
#define LCDC_DITHER_GREEN   0
#else
#define LCDC_DITHER_GREEN   CONFIG_LCDC_DITHER_GREEN
#endif
#ifndef CONFIG_LCDC_DITHER_BLUE
#define LCDC_DITHER_BLUE    0
#else
#define LCDC_DITHER_BLUE    CONFIG_LCDC_DITHER_BLUE
#endif

/* Background color */

#ifndef CONFIG_LCDC_BACKCOLOR
#define LCDC_BACKCOLOR      0
#else
#define LCDC_BACKCOLOR      CONFIG_LCDC_BACKCOLOR
#endif

/* Layer default color */

#ifdef CONFIG_LCDC_L1_COLOR
#define LCDC_L1_COLOR        CONFIG_LCDC_L1_COLOR
#else
#define LCDC_L1_COLOR        0x000000
#endif

#ifdef CONFIG_LCDC_L2
#ifdef CONFIG_LCDC_L2_COLOR
#define LCDC_L2_COLOR        CONFIG_LCDC_L2_COLOR
#else
#define LCDC_L2_COLOR        0x000000
#endif
#endif

/* Check pixel format support by DMA2D driver */

#ifdef CONFIG_DMA2D
#if defined(CONFIG_LCDC_L1_L8) || \
      defined(CONFIG_LCDC_L2_L8)
#if !defined(CONFIG_DMA2D_L8)
#error "DMA2D must support FB_FMT_RGB8 pixel format"
#endif
#endif
#if defined(CONFIG_LCDC_L1_RGB565) || \
      defined(CONFIG_LCDC_L2_RGB565)
#if !defined(CONFIG_DMA2D_RGB565)
#error "DMA2D must support FB_FMT_RGB16_565 pixel format"
#endif
#endif
#if defined(CONFIG_LCDC_L1_RGB888) || \
      defined(CONFIG_LCDC_L2_RGB888)
#if !defined(CONFIG_DMA2D_RGB888)
#error "DMA2D must support FB_FMT_RGB24 pixel format"
#endif
#endif
#if defined(CONFIG_LCDC_L1_ARGB8888) || \
      defined(CONFIG_LCDC_L2_ARGB8888)
#if !defined(CONFIG_DMA2D_ARGB8888)
#error "DMA2D must support FB_FMT_RGB32 pixel format"
#endif
#endif
#endif

/* Calculate the size of the layers clut table */

#ifdef CONFIG_FB_CMAP
#if defined(CONFIG_DMA2D) && !defined(CONFIG_DMA2D_L8)
#error "DMA2D must also support L8 CLUT pixel format if supported by LCDC"
#endif
#ifdef LCDC_L1CMAP
#ifdef CONFIG_FB_TRANSPARENCY
#define LAYER_CLUT_SIZE LCDC_NCLUT * sizeof(uint32_t)
#else
#define LAYER_CLUT_SIZE LCDC_NCLUT * 3 * sizeof(uint8_t)
#endif
#endif
#ifdef LCDC_L2CMAP
#undef  LAYER_CLUT_SIZE
#ifdef CONFIG_FB_TRANSPARENCY
#define LAYER_CLUT_SIZE LCDC_NCLUT * sizeof(uint32_t) * 2
#else
#define LAYER_CLUT_SIZE LCDC_NCLUT * 3 * sizeof(uint8_t) * 2
#endif
#endif
#endif

#ifndef CONFIG_FB_CMAP
#if defined(LCDC_L1CMAP) || defined(LCDC_L2CMAP)
#undef LCDC_L1CMAP
#undef LCDC_L2CMAP
#error "Enable cmap to support the configured layer format!"
#endif
#endif

/* Layer argb clut register position */

#define LCDC_CLUT_ADD(n)            ((uint32_t)(n) << 24)
#define LCDC_CLUT_ALPHA(n)          LCDC_CLUT_ADD(n)
#define LCDC_CLUT_RED(n)            ((uint32_t)(n) << 16)
#define LCDC_CLUT_GREEN(n)          ((uint32_t)(n) << 8)
#define LCDC_CLUT_BLUE(n)           ((uint32_t)(n) << 0)
#define LCDC_CLUT_RGB888_MASK       0xffffff

/* Layer argb cmap conversion */

#define LCDC_CMAP_ALPHA(n)          ((uint32_t)(n) >> 24)
#define LCDC_CMAP_RED(n)            ((uint32_t)(n) >> 16)
#define LCDC_CMAP_GREEN(n)          ((uint32_t)(n) >> 8)
#define LCDC_CMAP_BLUE(n)           ((uint32_t)(n) >> 0)

/* Hardware acceleration support */

/* Acceleration support for LCDC overlays */

#ifdef CONFIG_LCDC_L1_CHROMAKEYEN
#define LCDC_L1_CHROMAEN    true
#define LCDC_L1_CHROMAKEY   CONFIG_LCDC_CHROMAKEY
#define LCDC_LCDC_ACCL_L1         FB_ACCL_TRANSP | FB_ACCL_CHROMA
#else
#define LCDC_L1_CHROMAEN    false
#define LCDC_L1_CHROMAKEY   0
#define LCDC_LCDC_ACCL_L1         FB_ACCL_TRANSP
#endif

#ifdef CONFIG_LCDC_L2_CHROMAKEYEN
#define LCDC_L2_CHROMAEN    true
#define LCDC_L2_CHROMAKEY   CONFIG_LCDC_CHROMAKEY
#define LCDC_LCDC_ACCL_L2         FB_ACCL_TRANSP | FB_ACCL_CHROMA
#else
#define LCDC_L2_CHROMAEN    false
#define LCDC_L2_CHROMAKEY   0
#define LCDC_LCDC_ACCL_L2         FB_ACCL_TRANSP
#endif

#define LCDC_BLIT_ACCL          0

#define LCDC_DMA2D_ACCL           0

#define LCDC_L1_ACCL                LCDC_LCDC_ACCL_L1 | LCDC_DMA2D_ACCL
#ifdef CONFIG_LCDC_L2
#define LCDC_L2_ACCL              LCDC_LCDC_ACCL_L2 | LCDC_DMA2D_ACCL
#endif

/* Acceleration support for DMA2D overlays */

#define DMA2D_ACCL              FB_ACCL_AREA  | \
                                    FB_ACCL_TRANSP | \
                                    FB_ACCL_COLOR | \
                                    FB_ACCL_BLIT | \
                                    FB_ACCL_BLEND

/* Color normalization */

#if defined(CONFIG_LCDC_L1_RGB565)
#define RGB888_R(x)               (((((x) >> 11) & 0x1f) * 527 + 23) >> 6)
#define RGB888_G(x)               (((((x) >> 5) & 0x3f) * 259 + 33) >> 6)
#define RGB888_B(x)               ((((x) & 0x1f) * 527 + 23) >> 6)
#define ARGB8888(x)               ((RGB888_R(x) << 16) | \
                                     (RGB888_G(x) << 8)  | \
                                      RGB888_B(x))
#else
#define ARGB8888(x)               (x)
#endif

#ifdef CONFIG_PM
#define NPMCALL   2
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/**
 * enum layer_e   - This enumeration names each layer supported by the hardware
 * @LCDC_LAYER_L1  : LCDC  Layer 1
 * @LCDC_LAYER_L2  : LCDC  Layer 2
 * @LCDC_LAYER_O   : LCDC  WB Layer
 */

enum layer_e {
    LCDC_LAYER_L1 = 0,    /* LCDC  Layer 1 */
    LCDC_LAYER_L2,        /* LCDC  Layer 2 */
    LCDC_LAYER_O,        /* LCDC  WB Layer */
};

/**
 * struct lcdc_s  - LCDC General layer information
 * @layerno        :  layer number
 * @oinfo          :  Overlay info
 * @dma2dinfo      :  Overlay info for DMA2D
 */

struct lcdc_s {
    int layerno;

#ifdef CONFIG_FB_OVERLAY
    struct fb_overlayinfo_s oinfo;
#endif

#ifdef CONFIG_DMA2D
    struct dma2d_overlay_s dma2dinfo;
#endif
/**
 * @lock  : Ensure mutually exclusive access
 */
    sem_t *lock;
};

struct paninfo_s {
    sem_t sem_sync;;
    uint32_t yoffset;
    uint8_t volatile* panaddr;
};

/**
 * struct lcdcdev_s    - LCDC device driver interface
 * @vtable     :   Framebuffer operations interface
 * @vinfo      :   Framebuffer video information
 * @pinfo      :   Framebuffer plane information
 * @cmap       :   Cmap information
 * @cadjmap    :   Color adjust map information
 * @cdithermap :   Color dither map information
 * @layer[]    :   Layers information
 * @dma2d      :   DMA2D Driver interface
 *
 */

struct lcdcdev_s {
    struct fb_vtable_s vtable;
    struct fb_videoinfo_s vinfo;
    struct fb_planeinfo_s pinfo;
#ifdef CONFIG_FB_CMAP
    struct fb_cmap_s cmap;
#endif
#ifdef CONFIG_FB_COLORADJCURVE
    struct fb_cadjmap_s cadjmap;
#endif
#ifdef CONFIG_FB_COLORDITHER
    struct fb_cdithermap_s cdithermap;
#endif
    struct lcdc_s layer[LCDC_NOVERLAYS + 1];
#ifdef CONFIG_DMA2D
    FAR struct dma2d_layer_s *dma2d;
#endif

#if defined(CONFIG_FB_PANDISPLAY)
    struct paninfo_s panfo;
#endif

    FAR struct lcd_dev_s *lcddev;
#ifdef CONFIG_PM
    sem_t pmlock;
    enum pm_state_e sys_pmstate;
    FAR struct pm_callback_s *pmcb[NPMCALL];
#endif
};

/**
 * struct interrupt_s  - Interrupt handling
 * @irq      : irq number
 * @error    : Interrupt error status
 * @sem      : Semaphore for waiting for irq
*/

struct interrupt_s {
    int irq;
    int error;
    sem_t *sem;
    LCDC_IRQ_CALLBACK cb;
};

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
struct teinterrupt_s {
    int irq;
    int error;
    sem_t *sem;
    LCDC_IRQ_CALLBACK cb;
};
#endif

/**
 * Private Function Prototypes
 */
static uint8_t lcdc_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len);
static uint8_t lcdc_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len);

static void lcdc_periphconfig(void);
static void lcdc_bgcolor(uint32_t rgb);
static void lcdc_dither(bool enable, uint8_t level, uint8_t red,
            uint8_t green, uint8_t blue);
static int lcd_irq(int irq, void *context, FAR void *arg);
static int lcdc_waitforirq(void);

static void lcdc_lpixelformat(FAR struct lcdc_s *layer);
static void lcdc_lframebuffer(FAR struct lcdc_s *layer);
static void lcdc_lenable(FAR struct lcdc_s *layer, bool enable);
static void lcdc_ldefaultcolor(FAR struct lcdc_s *layer, uint32_t rgb);
static void lcdc_ltransp(FAR struct lcdc_s *layer, uint8_t transp,
             uint32_t mode);

#ifdef CONFIG_LCDC_CHROMAKEY
static void lcdc_lchromakey(FAR struct lcdc_s *layer,
                uint32_t chroma, uint32_t rchroma);
static void lcdc_lchromakeyenable(FAR struct lcdc_s *layer, bool enable);
#endif

static void lcdc_linit(uint8_t lid);

#ifdef CONFIG_DMA2D
static void lcdc_dma2dlinit(void);

#ifdef CONFIG_FB_OVERLAY_BLIT
static bool lcdc_lvalidate(FAR const struct lcdc_s *layer,
               FAR const struct fb_area_s *area);
#endif
#endif

#ifdef CONFIG_FB_CMAP
static void lcdc_lputclut(FAR struct lcdc_s *layer,
              FAR const struct fb_cmap_s *cmap);
static void lcdc_lgetclut(FAR struct lcdc_s *layer, FAR struct fb_cmap_s *cmap);
static void lcdc_lclutenable(FAR struct lcdc_s *layer, bool enable);
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap);
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap);
static void lcdc_lcadjcurves_enable(FAR struct lcdc_s *layer, bool enable);
#endif

static void lcdc_lclear(uint8_t overlayno);

static int getvideoinfo(FAR struct fb_vtable_s *vtable,
            FAR struct fb_videoinfo_s *vinfo);
static int getplaneinfo(FAR struct fb_vtable_s *vtable,
            int planeno, FAR struct fb_planeinfo_s *pinfo);
#ifdef CONFIG_PM
static int setpower(FAR struct fb_vtable_s *vtable, int power);
static int getpower(FAR struct fb_vtable_s *vtable);
#endif
#ifdef CONFIG_FB_CMAP
static int getcmap(FAR struct fb_vtable_s *vtable, FAR struct fb_cmap_s *cmap);
static int putcmap(FAR struct fb_vtable_s *vtable,
           FAR const struct fb_cmap_s *cmap);
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static int getcadjmap(struct fb_vtable_s *vtable, struct fb_cadjmap_s *cadjmap);
static int putcadjmap(struct fb_vtable_s *vtable,
              const struct fb_cadjmap_s *cadjmap);
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
static int putcdithermap(struct fb_vtable_s *vtable,
             const struct fb_cdithermap_s *cdithermap);
#endif /* CONFIG_FB_COLORADJCURVE */

uint32_t lcdc_memaddress(struct fb_overlayinfo_s *poverlay,
             uint32_t xpos, uint32_t ypos);
/**
 * The following is provided only if the video hardware signals vertical
 * synchronisation
 */

#ifdef CONFIG_FB_SYNC
static int waitforvsync(FAR struct fb_vtable_s *vtable);
#endif
#if defined(CONFIG_FB_PANDISPLAY)
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo);
#endif

#ifdef CONFIG_FB_VMIRRO

/**
 * video_mirror - lcdc mirro display layer
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 */
static int video_mirror(FAR struct fb_vtable_s *vtable,
            FAR struct fb_var_screeninfo *var);

#endif

#ifdef CONFIG_FB_UPDATE
static int updatearea(FAR struct fb_vtable_s *vtable, const struct fb_area_s *area);
#endif

/**
 *  The following is provided only if the video hardware supports overlays
 */

#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo);
static int settransp(FAR struct fb_vtable_s *vtable,
             FAR const struct fb_overlayinfo_s *oinfo);
#ifdef CONFIG_LCDC_CHROMAKEY
static int setchromakey(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);
#endif
static int setcolor(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);
static int setblank(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);
static int setarea(FAR struct fb_vtable_s *vtable,
           FAR const struct fb_overlayinfo_s *oinfo);

/**
 * The following is provided only if the video hardware supports blit and blend
 * operation
 */

#ifdef CONFIG_FB_OVERLAY_BLIT
static int blit(FAR struct fb_vtable_s *vtable,
        FAR const struct fb_overlayblit_s *blit);
static int blend(FAR struct fb_vtable_s *vtable,
         FAR const struct fb_overlayblend_s *blend);
#endif /* CONFIG_FB_OVERLAY_BLIT */
#endif /* CONFIG_FB_OVERLAY */

/**
 * Private Data
 */

#ifdef CONFIG_FB_CMAP
/* The layers clut table entries */

static uint8_t g_redclut[LCDC_NCLUT];
static uint8_t g_greenclut[LCDC_NCLUT];
static uint8_t g_blueclut[LCDC_NCLUT];
#ifdef CONFIG_FB_TRANSPARENCY
static uint8_t g_transpclut[LCDC_NCLUT];
#endif
#endif /* CONFIG_FB_CMAP */

#ifdef CONFIG_FB_COLORADJCURVE
/* The color adjust curves table entries */
static uint8_t g_redcurve[LCDC_NCLUT];
static uint8_t g_greencurve[LCDC_NCLUT];
static uint8_t g_bluecurve[LCDC_NCLUT];
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
/* The layers dither table entries */
static uint32_t g_cdither[4];
#endif /* CONFIG_FB_COLORDITHER */

/* The LCDC semaphore that enforces mutually exclusive access */

static sem_t g_lock;

/* The semaphore for interrupt handling */

static sem_t g_semirq;

/* The semaphore for te interrupt handling */

static sem_t g_semirq_te;

/* This structure provides irq handling */

static struct interrupt_s g_interrupt = {
    .irq = 0,
    .error = OK,
    .sem = &g_semirq,
    .cb = NULL,
};

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
/* This structure provides dsi irq handling */

static struct teinterrupt_s g_teinterrupt = {
    .irq = 0,
    .error = OK,
    .sem = &g_semirq_te,
    .cb = NULL,
};
#endif

/**
 *  struct lcdcdev_s g_vtable - lcdc driver instance
*/

static struct lcdcdev_s g_vtable = {
    .vtable = {
           .getvideoinfo = getvideoinfo,
           .getplaneinfo = getplaneinfo
#ifdef CONFIG_PM
            ,
           .setpower = setpower,
           .getpower = getpower
#endif
#ifdef CONFIG_FB_SYNC
           ,
           .waitforvsync = waitforvsync
#endif
#if defined(CONFIG_FB_PANDISPLAY)
           ,
           .pandisplay = pan_display
#endif
#ifdef CONFIG_FB_VMIRRO
           ,
           .video_mirror = video_mirror
#endif
#ifdef CONFIG_FB_UPDATE
           ,
           .updatearea = updatearea
#endif
#ifdef CONFIG_FB_CMAP
           ,
           .getcmap = getcmap,
           .putcmap = putcmap
#endif
#ifdef CONFIG_FB_COLORADJCURVE
           ,
           .getcadjmap = getcadjmap,
           .putcadjmap = putcadjmap
#endif
#ifdef CONFIG_FB_COLORDITHER
           ,
           .putcdithermap = putcdithermap
#endif
#ifdef CONFIG_FB_OVERLAY
           ,
           .getoverlayinfo = getoverlayinfo,
           .settransp = settransp,

#ifdef CONFIG_LCDC_CHROMAKEY
           .setchromakey = setchromakey,
#endif
           .setcolor = setcolor,
           .setblank = setblank,
           .setarea = setarea
#ifdef CONFIG_FB_OVERLAY_BLIT
           ,
           .blit = blit,
           .blend = blend
#endif
#endif /* CONFIG_FB_OVERLAY */
           },
#ifdef CONFIG_LCDC_L2
    .pinfo = {
          .fblen = LCDC_L2_FBSIZE,
          .stride = LCDC_L2_STRIDE,
          .display = 0,
          .bpp = LCDC_L2_BPP},
    .vinfo = {
          .fmt = LCDC_L2_COLOR_FMT,
          .xres = LCDC_WIDTH,
          .yres = LCDC_HEIGHT,
          .nplanes = 1,
#ifdef CONFIG_FB_OVERLAY
          .noverlays = LCDC_NOVERLAYS
#endif
          }
#else
    .pinfo = {
          // .fbmem           = (void *)(LCDC_BUFFER_L1),
          .fblen = LCDC_L1_FBSIZE,
          .stride = LCDC_L1_STRIDE,
          .display = 0,
          .bpp = LCDC_L1_BPP,
          .xres_virtual = LCDC_WIDTH,
#if defined(CONFIG_FB_DOUBLE_BUFFER)
          .yres_virtual = LCDC_HEIGHT * 2,
#else
          .yres_virtual = LCDC_HEIGHT,
#endif
          .xoffset = 0,
          .yoffset = 0
          },
    .vinfo = {
          .fmt = LCDC_L1_COLOR_FMT,
          .xres = LCDC_WIDTH,
          .yres = LCDC_HEIGHT,
          .nplanes = 1,
#ifdef CONFIG_FB_OVERLAY
          .noverlays = LCDC_NOVERLAYS
#endif
          }
#endif /* CONFIG_LCDC_L2 */
    ,
#ifdef CONFIG_FB_CMAP
    .cmap = {
         .first = 0,
         .len = LCDC_NCLUT,
         .red = g_redclut,
         .green = g_greenclut,
         .blue = g_blueclut,
#ifdef CONFIG_FB_TRANSPARENCY
         .transp = g_transpclut
#endif
         }
    ,
#endif

#ifdef CONFIG_FB_COLORADJCURVE
    .cadjmap = {
            .first = 0,
            .len = LCDC_NCLUT,
            .red = g_redcurve,
            .green = g_greencurve,
            .blue = g_bluecurve,
            }
    ,
#endif

#ifdef CONFIG_FB_COLORDITHER
    .cdithermap = {
               .first = 0,
               .len = 4,
               .data = g_cdither,
               }
    ,
#endif
    .layer[LCDC_LAYER_L1] = {
                 .layerno = LCDC_LAYER_L1,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       // .fbmem            = (uint8_t *)LCDC_BUFFER_L1,
                       .fblen = LCDC_L1_FBSIZE,
                       .stride = LCDC_L1_STRIDE,
                       .overlay = LCDC_LAYER_L1,
                       .bpp = LCDC_L1_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                       .accl = LCDC_L1_ACCL},
#endif

#ifdef CONFIG_DMA2D
                 .dma2dinfo = {
                           .fmt = LCDC_L1_DMA2D_PF,
                           .transp_mode = FB_CONST_ALPHA,
                           .xres = LCDC_WIDTH,
                           .yres = LCDC_HEIGHT,
                           .oinfo =
                           &g_vtable.layer[LCDC_LAYER_L1].
                           oinfo},
#endif
                 .lock = &g_lock}
#ifdef CONFIG_LCDC_L2
    ,
    .layer[LCDC_LAYER_L2] = {
                 .layerno = LCDC_LAYER_L2,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       .overlay = LCDC_LAYER_L2,
                       // .fbmem            = (uint8_t *)LCDC_BUFFER_L2,
                       .fblen = LCDC_L2_FBSIZE,
                       .stride = LCDC_L2_STRIDE,
                       .bpp = LCDC_L2_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                       .accl = LCDC_L2_ACCL},
#endif

#ifdef CONFIG_DMA2D
                 .dma2dinfo = {
                           .fmt = LCDC_L2_DMA2D_PF,
                           .transp_mode = FB_CONST_ALPHA,
                           .xres = LCDC_WIDTH,
                           .yres = LCDC_HEIGHT,
                           .oinfo =
                           &g_vtable.layer[LCDC_LAYER_L2].
                           oinfo},
#endif
                 .lock = &g_lock}
#endif
#ifdef CONFIG_LCDC_LWB
    ,
    .layer[LCDC_LAYER_O] = {
                .layerno = LCDC_LAYER_O,
#ifdef CONFIG_FB_OVERLAY
                .oinfo = {
                      .overlay = LCDC_LAYER_O,
                      .fbmem =
                      (uint8_t *) CONFIG_LCDC_FBWB_BASE,
                      .fblen = LCDC_LO_FBSIZE,
                      .stride = LCDC_LO_STRIDE,
                      .bpp = CONFIG_LCDC_FBWB_BPP,
                      .blank = 0,
                      .chromakey = 0,
                      .color = 0,
                      .transp = {
                             .transp = 0,
                             .transp_mode =
                             FB_CONST_ALPHA},
                      .sarea = {
                            .x = 0,
                            .y = 0,
                            .w = LCDC_WIDTH,
                            .h = LCDC_HEIGHT},
                      .accl = LCDC_L2_ACCL},
#endif

#ifdef CONFIG_DMA2D
                .dma2dinfo = {
                          .fmt = LCDC_LOPFCR_PF,
                          .transp_mode = FB_CONST_ALPHA,
                          .xres = LCDC_WIDTH,
                          .yres = LCDC_HEIGHT,
                          .oinfo =
                          &g_vtable.layer[LCDC_LAYER_O].
                          oinfo},
#endif
                .lock = &g_lock}
#endif
#if defined(CONFIG_FB_PANDISPLAY)
    ,
    .panfo = {
        .yoffset = 0,
        .panaddr = NULL,
    },
#endif
};

/* LCDC width */

static const uint32_t width_layer_t[LCDC_NLAYERS] = {
    LCDC_WIDTH
#ifdef CONFIG_LCDC_L2
        , LCDC_WIDTH
#endif
};

/* LCDC height */

static const uint32_t height_layer_t[LCDC_NLAYERS] = {
    LCDC_HEIGHT
#ifdef CONFIG_LCDC_L2
        , LCDC_HEIGHT
#endif
};

/* LCDC stride */

static const uint32_t stride_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_STRIDE
#ifdef CONFIG_LCDC_L2
        , LCDC_L2_STRIDE
#endif
};

/* LCDC framebuffer len */

static const uint32_t fblen_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_FBSIZE
#ifdef CONFIG_LCDC_L2
        , LCDC_L2_FBSIZE
#endif
};

/* LCDC framebuffer */

static uint32_t fbmem_layer_t[LCDC_NLAYERS + 1] = {
    0,            //LCDC_BUFFER_L1
#ifdef CONFIG_LCDC_L2
    0,            // LCDC_BUFFER_L2
#endif
    0,
};

/* LCDC default color lookup table */

static const uint32_t defaultcolor_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_COLOR
#ifdef CONFIG_LCDC_L2
        , LCDC_L2_COLOR
#endif
};

#ifdef CONFIG_LCDC_CHROMAKEY

/* LCDC default chromakey */

static const uint32_t chromakey_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_CHROMAKEY
#ifdef CONFIG_LCDC_L2
        , LCDC_L2_CHROMAKEY
#endif
};

/* LCDC chromakey enabled state */

static const bool chromakeyen_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_CHROMAEN
#ifdef CONFIG_LCDC_L2
        , LCDC_L2_CHROMAEN
#endif
};

#endif

/* LCDC pixel format lookup table */

static const uint32_t fmt_layer_t[LCDC_NLAYERS + 1] = {
    LCDC_L1PFCR_PF
#ifdef CONFIG_LCDC_L2
        , LCDC_L2PFCR_PF
#endif
#ifdef CONFIG_LCDC_WB_TEST
    , LCDC_LOPFCR_PF
#endif
};

/* The initialized state of the driver */

static bool g_initialized;

/**
 * Private Power Functions
 */

#ifdef CONFIG_PM
#ifndef __unused
#define __unused __attribute__((unused))
#endif
__unused static bool in_work_stat = false;

static void pm_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);
static int pm_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);

static struct pm_callback_s module_pmcb = {
    .notify = pm_notify,
    .prepare = pm_prepare,
};

__unused static bool allow_lower_pwr(void)
{
    return (in_work_stat) ? false : true;
}

/**
 * pm_notify -  Notify the driver of new power state.
 * @cb : Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 * @pmstate : the new PM state
 *
 * This callback is called after all drivers have had the opportunity to prepare
 * for the new power state.
 *
 */

static void pm_notify(struct pm_callback_s *cb, int domain,
              enum pm_state_e pmstate)
{
    int i;

    if (PM_NORMAL == pmstate) {
        for (i = ARRAY_SIZE(g_vtable.pmcb) - 1; i >= 0; i--) {
            if (g_vtable.pmcb[i] != NULL) {
                LCDC_INFO("%s, call pm notify function %p\n", __func__,
                          g_vtable.pmcb[i]->notify);
                g_vtable.pmcb[i]->notify(g_vtable.pmcb[i], domain, pmstate);
            }
        }
    }
    else {
        for (i = 0; i < ARRAY_SIZE(g_vtable.pmcb); i++) {
            if (g_vtable.pmcb[i] != NULL) {
                LCDC_INFO("%s, call pm notify function %p\n", __func__,
                          g_vtable.pmcb[i]->notify);
                g_vtable.pmcb[i]->notify(g_vtable.pmcb[i], domain, pmstate);
            }
        }
    }

    g_vtable.sys_pmstate = pmstate;
}

/**
 * pm_prepare  - Request the driver to prepare for a new power state
 * @cb : Returned to the driver. The driver version of the callback
 *       structure may include additional, driver-specific state data at
 *       the end of the structure.
 * @pmstate : Identifies the new PM state
 *
 *   This is a warning that the system is about to enter into a new power state.
 *   The driver should begin whatever operations that may be required to enter power
 *   state. The driver may abort the state change mode by returning a non-zero
 *   value from the callback function.
 *
 * Returns :
 *   Zero - (OK) means the event was successfully processed and that the
 *          driver is prepared for the PM state change.
 *
 *   Non-zero - means that the driver is not prepared to perform the tasks
 *              needed achieve this power setting and will cause the state
 *              change to be aborted. NOTE: The prepare() method will also
 *              be called when reverting from lower back to higher power
 *              consumption modes (say because another driver refused a
 *              lower power state change). Drivers are not permitted to
 *              return non-zero values when reverting back to higher power
 *              consumption modes!
 *
 */
static int pm_prepare(struct pm_callback_s *cb, int domain,
              enum pm_state_e pmstate)
{
    /* Logic to prepare for a reduced power state goes here. */
    int i;
    int ret = OK;

    if (PM_STANDBY == g_vtable.sys_pmstate && PM_NORMAL == pmstate) {
        for (i = ARRAY_SIZE(g_vtable.pmcb) - 1; i >= 0; i--) {
            if (g_vtable.pmcb[i] != NULL) {
                LCDC_INFO("%s, call pm prepare function %p\n", __func__,
                      g_vtable.pmcb[i]->prepare);
                ret |= g_vtable.pmcb[i]->prepare(g_vtable.pmcb[i], domain, pmstate);
            }
        }
    }
    else {
        for (i = 0; i < ARRAY_SIZE(g_vtable.pmcb); i++) {
            if (g_vtable.pmcb[i] != NULL) {
                LCDC_INFO("%s, call pm prepare function %p\n", __func__,
                      g_vtable.pmcb[i]->prepare);
                ret |= g_vtable.pmcb[i]->prepare(g_vtable.pmcb[i], domain, pmstate);
            }
        }
    }

    return ret;
    // if (pmstate > sys_pmstate) {
    //     if (allow_lower_pwr()) {
    //         return OK;
    //     } else {
    //         return -1;
    //     }
    // } else {
    //     return OK;
    // }
}

#ifndef __NuttX__
#define pm_register(cb)              (0)
#endif

__unused static void module_pm_activity(void)
{
    //request sys clk
    //pm_activity(RPMSG_IO_PDOMAIN , RPMSG_IO_ACTIVITY_PRIORITY);
}

/**
 * @brief lcdc_pmcb_init - init lcdc pm callbacks.
 *
 */
static void lcdc_pmcb_init(void)
{
    static bool volatile has_init = false;

    if (false == has_init) {
        lcdc_sem_init(&g_vtable.pmlock, 0, 1);
        g_vtable.sys_pmstate = PM_NORMAL;

        for (int i = 0; i < ARRAY_SIZE(g_vtable.pmcb); i++) {
            g_vtable.pmcb[i] = NULL;
        }

        has_init = true;
    }
}

/**
 * @brief lcdc_pmcb_early_register - register pm callbacks by touchdev
 *
 * @param cb  : pm callbacks
 * @return int : (OK) success; (<0) failed
 */
int lcdc_pmcb_early_register(struct pm_callback_s *cb)
{
    int i;

    lcdc_pmcb_init();
    lcdc_sem_wait(&g_vtable.pmlock);
    for (i = 0; i < ARRAY_SIZE(g_vtable.pmcb) / 2; i++) {
        if (g_vtable.pmcb[i] == NULL) {
            g_vtable.pmcb[i] = cb;
            break;
        }
    }

    lcdc_sem_post(&g_vtable.pmlock);
    if (i == ARRAY_SIZE(g_vtable.pmcb) / 2) {
        return -EBUSY;
    }
    return OK;
}

/**
 * @brief lcdc_pmcb_register - register pm callbacks by lcddev
 *
 * @param cb : pm callbacks
 * @return int : (OK) success; (<0) failed
 */
int lcdc_pmcb_register(struct pm_callback_s *cb)
{
    int i;

    lcdc_pmcb_init();
    lcdc_sem_wait(&g_vtable.pmlock);
    for (i = ARRAY_SIZE(g_vtable.pmcb) / 2;
         i < ARRAY_SIZE(g_vtable.pmcb); i++) {
        if (g_vtable.pmcb[i] == NULL) {
            g_vtable.pmcb[i] = cb;
            break;
        }
    }

    lcdc_sem_post(&g_vtable.pmlock);
    if (i == ARRAY_SIZE(g_vtable.pmcb)) {
        return -EBUSY;
    }

    return OK;
}

/**
 * @brief lcdc_pmcb_unregister - unregister pm callbacks
 *
 * @param cb : callbacks
 * @return int : always success
 */
int lcdc_pmcb_unregister(struct pm_callback_s *cb)
{
    int i;

    lcdc_pmcb_init();
    lcdc_sem_wait(&g_vtable.pmlock);
    for (i = 0; i < ARRAY_SIZE(g_vtable.pmcb); i++) {
        if (g_vtable.pmcb[i] == cb) {
            g_vtable.pmcb[i] = NULL;
            break;
        }
    }

    lcdc_sem_post(&g_vtable.pmlock);


    return OK;
}


#endif //CONFIG_PM

/**
 * Private Lcdc Functions
 */

static void lcdc_setup_layer_fbmem(void *fbmem, uint32_t fbsize)
{
    uint32_t buffer_l1 = 0;
#ifdef CONFIG_LCDC_L2
    uint32_t buffer_l2 = 0;
#endif
    if (fbmem)
    {
        buffer_l1 = (uint32_t)fbmem;
#ifdef CONFIG_LCDC_L2
        buffer_l2 = (uint32_t)(fbmem + fbsize/2);
#endif
    }
    else
    {
        buffer_l1 = LCDC_BUFFER_L1;
#ifdef CONFIG_LCDC_L2
        buffer_l2 = LCDC_BUFFER_L2;
#endif
    }
    fbmem_layer_t[0] = (uint32_t) (buffer_l1);
#ifdef CONFIG_LCDC_L2
    g_vtable.pinfo.fbmem = (void *)(buffer_l2);
    fbmem_layer_t[1] = (uint32_t) g_vtable.pinfo.fbmem;
#else
    g_vtable.pinfo.fbmem = (void *)(buffer_l1);
#endif

#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[LCDC_LAYER_L1].oinfo.fbmem = (uint8_t *) buffer_l1;
#endif

#ifdef CONFIG_LCDC_L2
#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[LCDC_LAYER_L2].oinfo.fbmem = (uint8_t *) buffer_l2;
#endif
#endif
}

/**
 * lcdc_periphconfig - lcdc display panel related configuration
 *
 * Configures the synchronous timings
 * Configures the synchronous signals and clock polarity
 *
 */

static void lcdc_periphconfig(void)
{
#ifdef FPGA
    hal_lcdc_stby_vsync_set(0x400, 1, 5);
#else
    if (((CONFIG_LCDC_FB_BASE & 0xFF000000) != PSRAM_BASE ) &&
       ((CONFIG_LCDC_FB_BASE & 0xFF000000) != PSRAM_NC_BASE )) {//framebuffer use sram
        hal_lcdc_stby_vsync_set(0x1e0, 1, 5);
    }else{
         hal_lcdc_stby_vsync_set(0x200, 1, 5);
    }
#endif
    hal_lcdc_porch_vsync_set(32, 32, 10);

    hal_lcdc_pix_vsync_set(30, 31);

    hal_lcdc_sponscreen_set(LCDC_LAYER_LFORE, 1, 1);

    hal_lcdc_sponscreen_set(LCDC_LAYER_LBACK, 1, 1);

    /* Configure LCDC_BFPCR */
    hal_lcdc_bgl_hporch_set(BOARD_LCDC_HBP, BOARD_LCDC_HFP);
    hal_lcdc_bgl_vporch_set(BOARD_LCDC_VBP, BOARD_LCDC_VFP);
    /* Configure LCDC_TWCR */
#ifdef CONFIG_DSI_VIDEO_MODE
    hal_lcdc_bgl_vhtotal_set(LCDC_TWCR_TOTALW+4, LCDC_TWCR_TOTALH+8);
#else
    hal_lcdc_bgl_vhtotal_set(LCDC_TWCR_TOTALW, LCDC_TWCR_TOTALH);
#endif

#ifdef CONFIG_DSI_VIDEO_MODE
    hal_lcdc_lvsepxcount(0x17e,0x17e);
#else
    hal_lcdcl_enable(LCDC_LAYER_SPU, true);
#endif

#ifdef TEST_WITH_GPU
    hal_lcdc_lsize(LCDC_LAYER_SPU, SCREEN_WIDTH, LCDC_HEIGHT);
#else
    hal_lcdc_lsize(LCDC_LAYER_SPU, LCDC_WIDTH, LCDC_HEIGHT);
#endif
    hal_lcdc_select_top(LCDC_LAYER_LFORE);

#ifdef CONFIG_LCDC_LWB
    hal_lcdcl_enable(LCDC_LAYER_LWB, true);
    hal_lcdc_lsize(LCDC_LAYER_LWB, LCDC_WIDTH, LCDC_HEIGHT);
    hal_lcdc_lpfc_fmt(LCDC_LAYER_LWB, fmt_layer_t[LCDC_LAYER_LWB]);
    hal_lcdc_lstartaddr(LCDC_LAYER_LWB, CONFIG_LCDC_FBWB_BASE);

    uint16_t pitch = LCDC_WIDTH * CONFIG_LCDC_FBWB_BPP / 8;
    hal_lcdc_lpitch(LCDC_LAYER_LWB, pitch);
#endif
    hal_lcdc_framedone_irq_enable(true);
    hal_lcdc_disp_avsync_trigger_disable(true);
    hal_lcdc_disp_svsync_trigger_disable(true);

    if (g_vtable.pinfo.bpp == 32) {
        hal_lcdc_smpn_mode_set(3);
    } else if (g_vtable.pinfo.bpp == 16) {
        hal_lcdc_smpn_mode_set(5);
    } else
        hal_lcdc_smpn_mode_set(3);

    hal_lcdc_disp_mem_enable(true);

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_LFORE, true);

#if 0 //defined(CONFIG_ULPS_MODE_SUPPORT)
    hal_lcdc_set_sync_line_pixs(0x55);
    hal_lcdc_set_sync_lines(0x23d);
#else
    hal_lcdc_set_sync_line_pixs(5);
    hal_lcdc_set_sync_lines(0);
#endif
    hal_lcdc_smpn_vsync_delaymode_set(1);
    hal_lcdc_disp_extra_delay(0);
    hal_lcdc_sl_dmaburst_length_set(0x0f);
    hal_lcdc_ol_dmaburst_length_set(8);

}

/**
 * lcdc_ldefaultcolor - Configures layer default color.
 * @layer : Reference to the layer control structure
 * @rgb : RGB888 background color
 *
 */

static void lcdc_ldefaultcolor(FAR struct lcdc_s *layer, uint32_t rgb)
{
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    LCDC_INFO("set LCDC_L%dDCCR=%08x\n", layer->layerno + 1, (unsigned int)rgb);
    hal_lcdc_lcolor(LCDC_LAYER_SPU, rgb);

}

/**
 * lcdc_bgcolor - Configures background color.
 * @rgb : RGB888 background color
 */

static void lcdc_bgcolor(uint32_t rgb)
{
    hal_lcdc_lcolor(LCDC_LAYER_SPU, rgb);
}

/**
 * lcdc_byteorder - Configures dsi byte order of rgb.
 * @border : Reference enum hal_lcdc_dsi_dswap_e
 */
static void lcdc_byteorder(enum hal_lcdc_dsi_dswap_e border)
{
    hal_lcdc_disp_dsi_data_swap(border);
}

/**
 * lcdc_dither - Configures dither settings of the LCD controller.
 * @enable : Enable/disable dithering
 * @red    : Red dither width
 * @green  : Green dither width
 * @blue   : Blue dither width
 *
 */

static void lcdc_dither(bool enable, uint8_t level, uint8_t red,
            uint8_t green, uint8_t blue)
{
    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(enable, level, red, green, blue);
}

/**
 * lcdc_axifastmode - Configures axi axi bus arbiter fast mode.
 * @enable : Enable/disable axi bus fast mode
 */
static void lcdc_axifastmode(bool enable)
{
    hal_lcdc_axifastmode_enable(enable);
}

void lcdc_enter_lowpower()
{
    //LCDC_REP("%s\n", __func__);
#ifndef CONFIG_DSI_VIDEO_MODE
#if defined(CONFIG_ULPS_MODE_SUPPORT)
    hal_dsi_enter_ulps_mode();
#endif
#ifdef CONFIG_DSI_SLEEP
    hal_dsi_sleep();
#endif
#ifdef CONFIG_LCDC_SLEEP
    hal_lcdc_sleep();
#endif
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_32K);
}

void lcdc_exit_lowpower()
{
    //LCDC_REP("%s\n", __func__);
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_104M);
#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef CONFIG_LCDC_SLEEP
    hal_lcdc_wakeup();
#endif
#ifdef CONFIG_DSI_SLEEP
    hal_dsi_wakeup();
#endif
#if defined(CONFIG_ULPS_MODE_SUPPORT)
    hal_dsi_exit_ulps_mode();
#endif
#endif
}

void lcdc_exit_lowpower_with_cb(LCDC_IRQ_CALLBACK cb, void *param)
{
    //LCDC_REP("%s\n", __func__);
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_104M);
#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef CONFIG_LCDC_SLEEP
    hal_lcdc_wakeup();
#endif
#ifdef CONFIG_DSI_SLEEP
    hal_dsi_wakeup();
#endif
#if defined(CONFIG_ULPS_MODE_SUPPORT)
    hal_dsi_exit_ulps_with_cb((HAL_DSI_ULPS_CB_HANDLER)cb, param);
#endif
#endif
}

#if defined(CONFIG_FB_PANDISPLAY)

static void lcdc_pandisp_init(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;

    lcdc_sem_init(&priv->panfo.sem_sync, 0, 0);
}

static void lcdc_pandisp_exec(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;
    if (priv->panfo.panaddr) {
        lcdc_exit_lowpower();
        LCDC_INFO("%s panaddr = %08x,yoffset = %d\n", __func__, priv->panfo.panaddr,priv->panfo.yoffset);
        hal_lcdc_lstartaddr(0, (uint32_t)priv->panfo.panaddr);
        priv->pinfo.yoffset = priv->panfo.yoffset;
#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef CONFIG_TRIGGER_FRAME_BY_SW
        hal_lcdc_gen_start();
#else
        hal_lcdc_te_trigger();
#endif
#endif
        priv->panfo.panaddr = NULL;
    }
    else {
        lpcmds_flush();
    }
#ifdef CONFIG_LCDC_L2

#endif
}

__attribute__((unused)) static void lcdc_pandisp_postsync(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;
    lcdc_sem_post(&priv->panfo.sem_sync);
}

#ifdef CONFIG_FB_SYNC
static int lcdc_pandisp_trywait(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;
    int ret;

    ret = lcdc_sem_trywait(&priv->panfo.sem_sync);

    return ret;
}
#endif
#endif

/**
 * lcd_irq -LCDC interrupt handler
 *
 */

#ifdef CONFIG_DSI_UNIFIED_IRQ
static int dsi_irq(int irq, void *context, FAR void *arg);
#endif

static int lcd_irq(int irq, void *context, FAR void *arg)
{
    FAR struct interrupt_s *priv = &g_interrupt;
    uint32_t regval = hal_lcdc_get_irqstat();
    LCDC_INFO("%s lcd irq = %d, regval = %08x\n", __func__, irq,regval);
    if (regval & 0xffffff00) {
        if (hal_lcdc_irq_is_framedone(regval)) {
            hal_lcdc_framedone_irqstat_clr(); //clear irq
            /* Register reload interrupt */
            if ((hal_lcdc_irq_is_framedone_irq0(regval)
                 || hal_lcdc_irq_is_framedone_irq1(regval))) {
#if defined(CONFIG_DSI_VIDEO_MODE)
                lcdc_pandisp_exec();
                lcdc_pandisp_postsync();
#else
#if defined(CONFIG_LCDC_AUTOREFRESH_MODE) && !defined(CONFIG_TRIGGER_FRAME_BY_SW)
                lcdc_pandisp_exec();
#endif
#endif
                if (priv->cb) {
                    priv->cb(priv->error);
                }

                /* Unlock the semaphore if locked */
                int ret = 0;
                lcdc_sem_get_value(priv->sem, &ret);
                if (ret < 1)
                    lcdc_sem_post(priv->sem);
                priv->error = OK;
            }  else if (hal_lcdc_irq_is_error(regval)) {
                /* Configuration error interrupt */
                LCDC_ERR("%s irq = %d, regval = %08x\n", __func__, irq, regval);
                LCDC_ERR("ERROR: configuration\n");
                //hal_lcdc_error_irqstat_clr();
                priv->error = -ECANCELED;
            } else {
                LCDC_REP("%s irq = %d, regval = %08x\n", __func__, irq,regval);
                priv->error = -EINVAL;
            }

        } else {
            /* Unknown irq, should not occur */
            //ASSERT(0, "Unknown interrupt error\n");
            //LCDC_ERR("%s Unknown status irq = %d, regval = %08x\n", __func__, irq,regval);
        }
    }
#ifdef CONFIG_DSI_UNIFIED_IRQ
    dsi_irq(irq, context, arg);
#endif
    return priv->error;
}

/**
 * lcdc_waitforirq - Helper waits until the lcdc irq occurs.
 */
__attribute__((unused)) static int lcdc_waitforirq(void)
{
    int ret = OK;
    FAR struct interrupt_s *priv = &g_interrupt;
    TC_INIT
    LCDC_REGINFO("lcdc_waitforirq();");
    TC_START
    ret = lcdc_sem_wait(priv->sem);

    if (ret < 0) {
        LCDC_ERR("ERROR: lcdc_sem_wait() failed\n");
    }

    ret = priv->error;
    TC_END
    TC_REP(lcdccost);
    return ret;
}

/**
 * lcdc_irqconfig - Configure interrupts
 */

static void lcdc_irqconfig(void)
{
    g_interrupt.irq = hal_lcdc_irqn();
    hal_lcdc_irq_disable();
    //hal_lcdc_error_irq_enable();

    /* Initialize the LCDC semaphore that enforces mutually exclusive access */
    lcdc_sem_init(&g_lock, 0, 1);
    /* Initialize the semaphore for interrupt handling.  This waitsem
     * semaphore is used for signaling and, hence, should not have priority
     * inheritance enabled.
     */
    lcdc_sem_init(g_interrupt.sem, 0, 1);
    //lcdc_sem_setprotocol(g_interrupt.sem, SEM_PRIO_NONE);
    /* Attach LCDC interrupt vector */
    LCDC_REGINFO("g_interrupt.irq = hal_lcdc_irqn();");

    LCDC_INFO("%s irq:%d irq_handler:%p", __func__, g_interrupt.irq,
          lcd_irq);
    NVIC_SetVector(g_interrupt.irq, (uint32_t) lcd_irq);
    LCDC_REGINFO("NVIC_SetVector(g_interrupt.irq, (uint32_t)lcd_irq);");
    NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);
    LCDC_REGINFO("NVIC_SetPriority(g_interrupt.irq, IRQ_PRIORITY_NORMAL);");
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    LCDC_REGINFO("NVIC_ClearPendingIRQ(g_interrupt.irq);");
    NVIC_EnableIRQ(g_interrupt.irq);
    LCDC_REGINFO("NVIC_EnableIRQ(g_interrupt.irq);");
}


#ifdef CONFIG_LCDC_TE_IRQ_ENABLE

/**
 * dsi_irq -dsi interrupt handler
 *
 */
__attribute__((unused)) static int dsi_irq(int irq, void *context, FAR void *arg)
{
    FAR struct teinterrupt_s *priv = &g_teinterrupt;
    uint32_t regval = hal_dsi_get_irqstate();
    LCDC_INFO("%s dsi irq = %d, regval = %08x\n", __func__, irq,regval);
    if (hal_dsi_irq_is_te(regval)) {
        hal_dsi_clear_teirq_flag();
#if defined(CONFIG_LCDC_AUTOREFRESH_MODE) && defined(CONFIG_TRIGGER_FRAME_BY_SW)
        lcdc_pandisp_exec();
        lcdc_pandisp_postsync();
#endif
        priv->error = OK;
        if (priv->cb) {
            priv->cb(priv->error);
        }

        /* Unlock the semaphore if locked */

        int ret = 0;
        lcdc_sem_get_value(priv->sem, &ret);
        if (ret < 1)
            lcdc_sem_post(priv->sem);
        if (ret < 0) {
            LCDC_ERR("ERROR: lcdc_sem_post() failed at dsi interrupt\n");
        }
    }

    if (hal_dsi_irq_is_err(regval)) {
        hal_dsi_irq_clear_err(regval);
    }

    return priv->error;
}

#ifdef CONFIG_DSI_GPIO_TE_IRQ
static void dsi_te_gpio_irq(void)
{
    LCDC_INFO("%s\n", __func__);
    FAR struct teinterrupt_s *priv = &g_teinterrupt;
#if defined(CONFIG_LCDC_AUTOREFRESH_MODE) && defined(CONFIG_TRIGGER_FRAME_BY_SW)
    lcdc_pandisp_exec();
    lcdc_pandisp_postsync();
#endif
    priv->error = OK;
    if (priv->cb) {
        priv->cb(priv->error);
    }
    /* Unlock the semaphore if locked */

    int ret = 0;
    lcdc_sem_get_value(priv->sem, &ret);
    if (ret < 1)
        lcdc_sem_post(priv->sem);
    if (ret < 0) {
        LCDC_ERR("ERROR: lcdc_sem_post() failed at dsi interrupt\n");
    }
}
#endif

/**
 * lcdc_teirqconfig - config te interrupt
 */
static void lcdc_teirqconfig(void)
{
    //g_teinterrupt
    g_teinterrupt.irq = hal_dsi_irqn();
    hal_dsi_irq_disable();
    /* Initialize the semaphore for interrupt handling.  This waitsem
     * semaphore is used for signaling and, hence, should not have priority
     * inheritance enabled.
     */
    lcdc_sem_init(g_teinterrupt.sem, 0, 0);
#ifdef CONFIG_DSI_GPIO_TE_IRQ
#if !defined(SIMU)
    hal_dsi_te_gpio_irq_register(dsi_te_gpio_irq);
    hal_dsi_te_gpio_irq_enable();
#endif
#else
#ifndef CONFIG_DSI_UNIFIED_IRQ
    NVIC_SetVector(g_teinterrupt.irq, (uint32_t)dsi_irq);
    NVIC_SetPriority(g_teinterrupt.irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(g_teinterrupt.irq);
    NVIC_EnableIRQ(g_teinterrupt.irq);
#endif
    hal_dsi_teirq_enable();
#endif
}

#endif

/**
 * lcdc_globalconfig - Configure background color and dithering
 *
 */

static void lcdc_globalconfig(void)
{
    /* Configure dither */
    lcdc_dither(
#ifdef CONFIG_LCDC_DITHER
               true,
#else
               false,
#endif
               LCDC_DITHER_LEVEL,
               LCDC_DITHER_RED,
               LCDC_DITHER_GREEN, LCDC_DITHER_BLUE);

    /* Configure background color */

    lcdc_bgcolor(LCDC_BACKCOLOR);

#ifdef CONFIG_DSIPHY_OUTFMT_RGB565
    hal_lcdc_disp_color_fmt(DSI_RGB565);
#elif defined(CONFIG_DSIPHY_OUTFMT_RGB888)
    hal_lcdc_disp_color_fmt(DSI_RGB888);
#else
    hal_lcdc_disp_color_fmt(DSI_RGB888);
#endif
    /* Configure dsi byte order as rgb */

    lcdc_byteorder(DSI_BD_RGB);

    /* Configure enable axi bus arbiter fast mode */

    lcdc_axifastmode(true);
}

/**
 * lcdc_lpixelformat - Set the layer pixel format.
 * @layer   Reference to the layer control structure
 *
 */
static void lcdc_lpixelformat(FAR struct lcdc_s *layer)
{
    uint8_t overlay = layer->layerno;
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);

    /* Configure PFCR register */

    LCDC_INFO("set LCDC_L%dPFCR=%08x\n", overlay + 1, (unsigned int)fmt_layer_t[overlay]);
    hal_lcdc_lpfc_fmt(layer->layerno, fmt_layer_t[overlay]);

}

/**
 * lcdc_lframebuffer - Configure layer framebuffer of the entire window.
 * @layer   Reference to the layer control structure
 */

static void lcdc_lframebuffer(FAR struct lcdc_s *layer)
{

    uint8_t layerno = layer->layerno;

    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    LCDC_INFO("xpos = %d, ypos = %d, xres = %d, yres = %d\n", 0, 0,
          (int)width_layer_t[layerno], (int)height_layer_t[layerno]);

    /* Configure Layer start addr register */
    hal_lcdc_lstartaddr(layerno, fbmem_layer_t[layerno]);

    /* Configure Layer size register */

    hal_lcdc_lsize(layerno, width_layer_t[layerno],
               height_layer_t[layerno]);

    /* Configure Layer pitch register */

    hal_lcdc_lpitch(layerno, stride_layer_t[layerno]);

    hal_lcdc_lzoom_set(layerno, width_layer_t[layerno],
               height_layer_t[layerno]);
#ifdef CONFIG_DSI_VIDEO_MODE
    hal_lcdc_sponscreen_set(layerno, 0, 0);
#else
    hal_lcdc_sponscreen_set(layerno, 1, 1);
#endif
}

/**
 * lcdc_lenable - Enable or disable layer.
 * @layer  : Reference to the layer control structure
 * @enable : Enable or disable layer
 */
static void lcdc_lenable(FAR struct lcdc_s *layer, bool enable)
{
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    /* Enable/Disable layer */
    hal_lcdcl_enable(layer->layerno, enable);
}

/**
 * lcdc_ltransp - Change layer transparency.
 * @layer  : Reference to the layer control structure
 * @transp : Transparency
 * @mode   : Transparency mode
 *
 */
static void lcdc_ltransp(FAR struct lcdc_s *layer, uint8_t transp,
             uint32_t mode)
{
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    FAR struct lcdcdev_s *dev = &g_vtable;

    //for dma2d blend setting
    dev->layer[layer->layerno].oinfo.transp.transp = transp;
    dev->layer[layer->layerno].oinfo.transp.transp_mode = mode;

    hal_lcdc_lpfc_ltransp(layer->layerno, mode, transp);

}

#ifdef CONFIG_LCDC_CHROMAKEY

/**
 * lcdc_lchromakey - Change layer chromakey.
 * @layer    : Reference to the layer control structure
 * @chroma   : chromakey
 * @rchroma  : color to replace chromakey
 *
 */
static void lcdc_lchromakey(FAR struct lcdc_s *layer,
                uint32_t chroma, uint32_t rchroma)
{
#if 1
    uint32_t rgb, rrgb;
    uint8_t r, g, b;
    uint8_t rr, rg, rb;
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);

    /* Set chromakey */

#ifdef CONFIG_FB_CMAP
    r = g_vtable.cmap.red[chroma];
    g = g_vtable.cmap.green[chroma];
    b = g_vtable.cmap.blue[chroma];
    rgb = ((r << 16) | (g << 8) | b);
    rr = g_vtable.cmap.red[rchroma];
    rg = g_vtable.cmap.green[rchroma];
    rb = g_vtable.cmap.blue[rchroma];
    rrgb = ((rr << 16) | (gr << 8) | rb);
#else
//  rgb = ARGB8888(chroma);
    rgb = chroma;

    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0xff);

#if 1
    rgb = ((b << 16) | (g << 8) | r);
#endif

    rrgb = rchroma;
    rr = (rrgb & 0xff0000) >> 16;
    rg = (rrgb & 0x00ff00) >> 8;
    rb = (rrgb & 0xff);

#if 1
    rrgb = ((rb << 16) | (rg << 8) | rr);
#endif

#endif

    LCDC_INFO("set LCDC_L%dCKCR=%08x\n", layer->layerno + 1, rgb);

    hal_lcdc_lchromakey(layer->layerno, rgb, rrgb, 0);

#endif
}

/**
 * lcdc_lchromakeyenable - layers chromkey enable
 * @layer  : Reference to the layer control structure
 * @enable : Enable or disable chromakey
 *
 * Enable or disable layer chromakey support.
 * Note!  F/B layer share same chromakey.
 */
static void lcdc_lchromakeyenable(FAR struct lcdc_s *layer, bool enable)
{
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    hal_lcdc_lchromakeyenable(layer->layerno, enable);
}

#endif //CONFIG_LCDC_CHROMAKEY

/**
 * lcdc_lclutenable - Disable or enable the layer clut support
 * @layer  : Reference to the layer control structure
 * @enable : Enable or disable
 */
#ifdef CONFIG_FB_CMAP
static void lcdc_lclutenable(FAR struct lcdc_s *layer, bool enable)
{

}

/**
 * lcdc_setclut - Configure foreground and background clut table
 * @cmap  : fb cmap
 */
int lcdc_setclut(FAR const struct fb_cmap_s *cmap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cmap->first; n < cmap->len - 1 && n < LCDC_NCLUT; n++) {
        /* Update the layer clut entry, will be automatically loaded before
         * blit operation becomes active
         */

        priv->cmap.blue[n] = cmap->blue[n];
        priv->cmap.green[n] = cmap->green[n];
        priv->cmap.red[n] = cmap->red[n];

        citem =
            ((cmap->red[n] << 16) | (cmap->green[n] << 8) | cmap->
             blue[n]) & 0xffffff;
        //LCDC_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n, cmap->red[n],
        //        cmap->green[n], cmap->blue[n]);

        hal_lcdc_write_clut(citem, n);

    }

    return OK;
}

/**
 * lcdc_lputclut - Update the clut layer register during blank period.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lputclut(FAR struct lcdc_s *layer,
              FAR const struct fb_cmap_s *cmap)
{
    int n;
    uint32_t flags;
    /* Disable clut during register update */

    lcdc_lclutenable(layer, false);

    /* Update the clut registers. Ensure operation is atomic or in interrupt
     * protected context.
     * */
    flags = int_lock();
    lcdc_setclut(cmap);
    int_unlock(flags);
    /* Enable clut after register update */

    lcdc_lclutenable(layer, true);
}

/**
 * lcdc_lgetclut - Copy the layers color lookup table.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lgetclut(FAR struct lcdc_s *layer, FAR struct fb_cmap_s *cmap)
{
    int n;
    FAR struct fb_cmap_s *priv_cmap = &g_vtable.cmap;

    /* Copy from internal cmap */

    for (n = cmap->first; n < cmap->len && n < LCDC_NCLUT; n++) {

#ifdef CONFIG_FB_TRANSPARENCY
        cmap->transp[n] = priv_cmap->transp[n];
#endif
        cmap->red[n] = priv_cmap->red[n];
        cmap->green[n] = priv_cmap->green[n];
        cmap->blue[n] = priv_cmap->blue[n];

        LCDC_INFO
            ("color = %d, transp=%02x, red=%02x, green=%02x, blue=%02x\n",
             n,
#ifdef CONFIG_FB_TRANSPARENCY
             cmap->transp[n],
#endif
             cmap->red[n], cmap->green[n], cmap->blue[n]);
    }
}
#endif /* CONFIG_FB_CMAP */

#ifdef CONFIG_FB_COLORADJCURVE

static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap);
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap);

static void lcdc_lcadjcurves_enable(FAR struct lcdc_s *layer, bool enable)
{
    hal_lcdc_lcadjcurves_enable(layer->layerno, enable);
}

/**
 * lcdc_cadjcurves_enable - enable/disable foreground and background color adjust curves
 */
void lcdc_cadjcurves_enable(bool enable)
{
    FAR struct lcdc_s *layer = &g_vtable.layer[0];
    lcdc_lcadjcurves_enable(layer, enable);
}

/**
 * lcdc_setcadjcurves - Configure foreground and background color adjust curves table
 * @cadjmap  : fb color adjust curves
 *
 */
static int lcdc_setcadjcurves(FAR const struct fb_cadjmap_s *cadjmap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cadjmap->first; n < cadjmap->len && n <= LCDC_NCLUT; n++) {
        priv->cadjmap.blue[n] = cadjmap->blue[n];
        priv->cadjmap.green[n] = cadjmap->green[n];
        priv->cadjmap.red[n] = cadjmap->red[n];
        citem =
            ((cadjmap->red[n] << 16) | (cadjmap->green[n] << 8) |
             cadjmap->blue[n]) & 0xffffff;

        LCDC_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n,
              cadjmap->red[n], cadjmap->green[n], cadjmap->blue[n]);

        hal_lcdc_write_gamma(citem, n);
        // hal_sys_timer_delay_us(1000);
    }

    return OK;
}

/**
 * lcdc_lputcadjcurves - Update the clut layer register during blank period.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap)
{
    int n;
    uint32_t flags;

    if (layer->layerno != 0) {
        return;
    }
    flags = int_lock();
    if (layer->layerno == 0) {
        lcdc_setcadjcurves(cadjmap);
    }
    int_unlock(flags);

    lcdc_lcadjcurves_enable(layer, true);

}

/**
 * lcdc_lgetcadjcurves - Copy the layers color lookup table.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 */
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap)
{
    int n;
    FAR struct fb_cadjmap_s *priv_cmap = &g_vtable.cadjmap;

    /* Copy from internal cmap */

    for (n = cadjmap->first; n < cadjmap->len && n < LCDC_NCLUT; n++) {

        cadjmap->red[n] = priv_cmap->red[n];
        cadjmap->green[n] = priv_cmap->green[n];
        cadjmap->blue[n] = priv_cmap->blue[n];

        LCDC_INFO("color = %d, red=%02x, green=%02x, blue=%02x\n", n,
              cadjmap->red[n], cadjmap->green[n], cadjmap->blue[n]);
    }
}
#endif /* CONFIG_FB_COLORADJCURVE */

/**
 * lcdc_lclear - Clear the whole layer
 * @overlayno : Number overlay
 *
 */
static void lcdc_lclear(uint8_t overlayno)
{
    memset((uint8_t *) fbmem_layer_t[overlayno], 0,
           fblen_layer_t[overlayno]);
}

#if defined(CONFIG_FB_PANDISPLAY)

/**
 * pan_display - lcdc switch display layer
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 * Used for fb double buffer application,change display layer by xoffset and yoffset
 */
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo)
{
    FAR struct lcdcdev_s *priv = &g_vtable;
    FAR struct fb_planeinfo_s *pri_pinfo = &priv->pinfo;

#if defined(CONFIG_FB_DOUBLE_BUFFER)
    if (pinfo->yoffset > pri_pinfo->yres_virtual - priv->vinfo.yres ||
        pinfo->xoffset > pri_pinfo->xres_virtual - priv->vinfo.xres) {
        return -EINVAL;
    }
#endif
    uint32_t flags = int_lock();
    lcdc_sem_reset(&priv->panfo.sem_sync, 0);
    uint32_t dispbuff = (uint32_t)priv->pinfo.fbmem;
#if defined(CONFIG_FB_DOUBLE_BUFFER)
    dispbuff = (uint32_t)priv->pinfo.fbmem + priv->pinfo.stride * pinfo->yoffset;
#else
    if (pinfo->yoffset != 0)
        dispbuff = pinfo->yoffset;
#endif
    LCDC_INFO("%s panaddr =%08x, %08x,yoffset = %d\n", __func__, priv->panfo.panaddr, dispbuff ,priv->panfo.yoffset);
    lcdc_cache_sync(HAL_CACHE_ID_D_CACHE, dispbuff, LCDC_WIDTH*4*LCDC_HEIGHT);
    priv->panfo.panaddr = (uint8_t *)dispbuff;
    priv->panfo.yoffset = pinfo->yoffset;
    int_unlock(flags);
    return OK;
}
#endif

#ifdef CONFIG_FB_VMIRRO

/**
 * video_mirror - lcdc set mirror display layer enable
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 */
static int video_mirror(FAR struct fb_vtable_s *vtable,
            FAR struct fb_var_screeninfo *var)
{
    int ret = 0;
    uint8_t lid = 0;
    bool mirro_enable = false;
    struct fb_overlayinfo_s *poverlay;

    if (var->xoffset == 0 && var->yoffset == 0) {
        lid = 0;
    } else {
        lid = 1;
    }
    poverlay = &g_vtable.layer[lid].oinfo;

    mirro_enable = (var->rotate == 180) ? true : false;

    LCDC_INFO("%s enable lid:%d", __func__, lid);
    if (mirro_enable) {
        hal_lcdc_lstartaddr(lid,
                    lcdc_memaddress(poverlay, poverlay->sarea.x,
                            poverlay->sarea.h - 1));
    } else {
        hal_lcdc_lstartaddr(lid,
                    lcdc_memaddress(poverlay, poverlay->sarea.x,
                            poverlay->sarea.y));
    }
    hal_lcdc_set_vmirro_enable(lid, mirro_enable);
    // ret = lcdc_update();
    return ret;
}
#endif

/**
 * lcdc_memaddress - Helper to calculate the layer memory address
 * @oinfo - Reference to overlay information
 * @xpos  - x-Offset
 * @ypos  - y-Offset
 *
 * Returns: start memory address
 *
 */
uint32_t lcdc_memaddress(struct fb_overlayinfo_s * poverlay,
             uint32_t xpos, uint32_t ypos)
{
    uint32_t offset;

    offset = xpos * (poverlay->bpp / 8) + poverlay->stride * ypos;

    //LCDC_INFO("0x%x, offset=%d\n", ((uint32_t) poverlay->fbmem) + offset,
    //      offset);
    return ((uint32_t) poverlay->fbmem) + offset;
}

#ifdef CONFIG_FB_UPDATE
#define FULL_SCREEN_REDRAWN
/**
 * updatearea - lcdc update display with area
 * @vtable : Reference to lcdc driver interface
 * @area   : Reference to area to be displayed
 */
static int updatearea(FAR struct fb_vtable_s *vtable, const struct fb_area_s *area)
{

    struct fb_overlayinfo_s *poverlay;
    int ret = 0;
    FAR struct lcdcdev_s *dev = &g_vtable;
    FAR struct lcdc_s *layer = &dev->layer[LCDC_LAYER_L1];
    poverlay = &layer->oinfo;

#if defined(CONFIG_LCDC_AUTOREFRESH_MODE) || defined(CONFIG_DSI_VIDEO_MODE)
#if defined(CONFIG_FB_PANDISPLAY)
    if (dev->panfo.panaddr != 0)
        return 0;
#endif
#endif

#if defined(CONFIG_FB_DOUBLE_BUFFER)
    uint8_t *display_buffer = (uint8_t *)lcdc_memaddress(poverlay, area->x,area->y+
        dev->pinfo.yoffset);
#else
    uint8_t *display_buffer = (uint8_t *)lcdc_memaddress(poverlay, area->x,area->y);
#if defined(CONFIG_FB_PANDISPLAY)
    if (dev->panfo.yoffset != 0)
        display_buffer = dev->panfo.yoffset;
#endif
#endif

    uint32_t flags;
    flags = int_lock();
    if (0 == area->x && area->w == LCDC_WIDTH)
        lcdc_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)display_buffer, LCDC_WIDTH*4*area->h);
    else
    {
        for(int i = 0; i < area->h; i ++)
          {
              lcdc_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)display_buffer + i * poverlay->stride, area->w * poverlay->bpp / 8);
          }
    }
    int_unlock(flags);

#if defined(CONFIG_DSI_VIDEO_MODE)
    return 0;
#endif

    lcdc_waitforirq();

#if !defined(FULL_SCREEN_REDRAWN) // !whole screen redrawn
    lcdc_exit_lowpower();
    struct lcd_planeinfo_s pinfo = {0};
    if (dev->lcddev != NULL && dev->lcddev->getplaneinfo != NULL)
        dev->lcddev->getplaneinfo(dev->lcddev,0,&pinfo);

    if (pinfo.putarea != NULL)
    {
        pinfo.putarea(area->y,area->y+area->h-1,
                area->x,area->x+area->w-1,poverlay->fbmem + poverlay->stride * dev->pinfo.yoffset);
    }
    else
    {
        uint16_t tmphend,tmpvend;
        tmphend = area->x+area->w-1;
        tmpvend = area->y+area->h-1;
        area->x &= 0xfffe;
        area->y &= 0xfffe;
        area->w = (tmphend - area->x +2)&0xfffe;
        area->h = (tmpvend - area->y +2)&0xfffe;

        uint16_t hstart = 14 + area->x;
        uint16_t hend = hstart+area->w -1;
        uint16_t vstart = area->y;
        uint16_t vend = vstart+area->h-1;

        hal_dsi_reset(area->w);
        hal_dsi_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
        hal_dsi_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);

        display_buffer = lcdc_memaddress(poverlay, area->x,area->y+dev->pinfo.yoffset);
        /* Configure Layer start addr register */
        hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, display_buffer);
        /* Configure Layer size register */
        hal_lcdc_lsize(LCDC_LAYER_LFORE, area->w,area->h);
        hal_lcdc_lsize(LCDC_LAYER_SPU, area->w,area->h);
        hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, area->w,area->h);
    }
#endif
    /* update Layer start addr register */
    //lcdc_lenable(layer, true);
    LCDC_INFO("updatearea:x:%d,y:%d  w: %d h:%d", area->x,area->y, area->w,area->h);
    ret = lcdc_update();
    return ret;
}
#endif

/**
 * lcdc_lvalidate - Validates if the given area is within the overlay framebuffer memory
 * @layer  : Reference to the layer control structure
 * @area   : Reference to the overlay area
 *
 */
#if defined(CONFIG_DMA2D) && defined(CONFIG_FB_OVERLAY_BLIT)
static bool lcdc_lvalidate(FAR const struct lcdc_s *layer,
               FAR const struct fb_area_s *area)
{
    uint32_t offset;

    offset =
        (area->y + area->h - 1) * layer->oinfo.stride + (area->x +
                                 area->w) *
        layer->oinfo.bpp / 8;
    LCDC_INFO("layer->oinfo.fblen: %d", layer->oinfo.fblen);
    LCDC_INFO("offset: %ld", offset);
    LCDC_INFO(" stride:%d  w: %d h:%d", layer->oinfo.stride, area->w,
          area->h);
    return (offset <= layer->oinfo.fblen && area->w > 0 && area->h > 0);
}
#endif /* defined(CONFIG_DMA2D) && defined(CONFIG_FB_OVERLAY_BLIT) */

/**
 * lcdc_linit - Initialize layer to their default states.
 * @layer : Reference to the layer control structure
 *
 * Initialize:
 * - ``-layer framebuffer
 * - ``-layer pixelformat
 * - ``-layer defaultcolor
 * - ``-layer chromakey
 * - ``-layer transparency
 * - ``-layer clut
 *
 */
static void lcdc_linit(uint8_t overlay)
{
    ASSERT(overlay < LCDC_NLAYERS, "%s", __func__);

    FAR struct lcdcdev_s *dev = &g_vtable;
    FAR struct lcdc_s *layer = &dev->layer[overlay];

    /* Disable layer */

    lcdc_lenable(layer, false);

    /* Clear the layer framebuffer */

    lcdc_lclear(overlay);

    /* Set layers framebuffer */

    lcdc_lframebuffer(layer);

    /* Set layers pixel input format */

    lcdc_lpixelformat(layer);

    /* Configure layer default color */

    lcdc_ldefaultcolor(layer, defaultcolor_layer_t[overlay]);

    /* Layers default transparency */

    lcdc_ltransp(layer, layer->oinfo.transp.transp,
             layer->oinfo.transp.transp_mode);

    /* Layers chromakey */
#ifdef CONFIG_LCDC_CHROMAKEY

    LCDC_INFO("%s chromkey:0x%x", __func__, chromakey_layer_t[overlay]);
    lcdc_lchromakey(layer, chromakey_layer_t[overlay], 0);

    /* Enable chromakey */

    lcdc_lchromakeyenable(layer, chromakeyen_layer_t[overlay]);
#endif
#ifdef CONFIG_FB_CMAP
    /* Disable clut by default */

    if (dev->vinfo.fmt == FB_FMT_RGB8) {
        /* Initialize LCDC clut register */

        lcdc_lputclut(layer, &g_vtable.cmap);

        /* Configure the clut register */

        lcdc_lclutenable(layer, true);
    }
#endif

#ifdef CONFIG_FB_COLORADJCURVE
    //    lcdc_lputcadjcurves(layer, &g_vtable.cadjmap);
    //   lcdc_lcadjcurves_enable(layer, true);
#endif

    /* Finally enable the layer */
    lcdc_lenable(layer, true);
}

/**
 * lcdc_dma2dlinit - Initialize dma2d layer to their default states.
 * @layer : Reference to the layer control structure
 *
 * Initialize:
 * - ``-layer framebuffer
 * - ``-layer pixelformat
 * - ``-layer size
 * - ``-layer color
 * - ``-layer chromakey
 * - ``-layer transparency
 * - ``-layer clut
 */
#if defined(CONFIG_DMA2D)
static void lcdc_dma2dlinit(void)
{

#if (LCDC_DMA2D_NLAYERS > 0)
    int n;
    FAR struct lcdcdev_s *dev = &g_vtable;

    uint8_t *fbmem = (uint8_t *) DMA2D_BUFFER_START;

    for (n = 0; n < LCDC_DMA2D_NLAYERS; n++) {
        uint32_t overlay = n + LCDC_NLAYERS;
        FAR struct lcdc_s *layer = &dev->layer[overlay];

        layer->layerno = overlay;
        layer->oinfo.fbmem = fbmem + DMA2D_LAYER_SIZE * n;
        layer->oinfo.fblen = DMA2D_FBSIZE;
        layer->oinfo.stride = DMA2D_STRIDE;
        layer->oinfo.overlay = overlay;
        layer->oinfo.bpp = DMA2D_BPP;
        layer->oinfo.blank = 0;
        layer->oinfo.chromakey = 0;
        layer->oinfo.color = 0;
        layer->oinfo.transp.transp = 0xff;
        layer->oinfo.transp.transp_mode = 0;
        layer->oinfo.sarea.x = 0;
        layer->oinfo.sarea.y = 0;
        layer->oinfo.sarea.w = DMA2D_WIDTH;
        layer->oinfo.sarea.h = DMA2D_HEIGHT;
        layer->oinfo.accl = DMA2D_ACCL;
        layer->lock = &g_lock;
        layer->dma2dinfo.fmt = DMA2D_COLOR_FMT;
        layer->dma2dinfo.transp_mode = 0;
        layer->dma2dinfo.xres = layer->oinfo.sarea.w;
        layer->dma2dinfo.yres = layer->oinfo.sarea.h;
        layer->dma2dinfo.oinfo = &layer->oinfo;
    }
#endif
}
#endif /* CONFIG_DMA2D */

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * getvideoinfo - Get the videoinfo for the framebuffer
 * @vtable : Reference to lcdc driver interface
 * @vinfo  : Reference to fb video info structure
 *
 * Entrypoint ioctl FBIOGET_VIDEOINFO
 * Get the videoinfo for the framebuffer
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int getvideoinfo(struct fb_vtable_s *vtable,
            struct fb_videoinfo_s *vinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    LCDC_INFO("vtable=%p vinfo=%p\n", vtable, vinfo);
    ASSERT(vtable != NULL && priv == &g_vtable
           && vinfo != NULL, "%s", __func__);

    memcpy(vinfo, &priv->vinfo, sizeof(struct fb_videoinfo_s));

#ifdef CONFIG_FB_MODULEINFO
    struct fb_videoinfo_s devvinfo;

    ASSERT(priv->lcddev != NULL && priv->lcddev->getvideoinfo != NULL,
           "%s", __func__);

    priv->lcddev->getvideoinfo(priv->lcddev, &devvinfo);
    memcpy(vinfo->moduleinfo, &devvinfo.moduleinfo,
           sizeof(devvinfo.moduleinfo));
#endif
    return OK;
}

/**
 * getplaneinfo - Get the planeinfo for the framebuffer
 * @vtable : Reference to lcdc driver interface
 * @pinfo  :  Reference to fb plane info structure
 *
 * Entrypoint ioctl FBIOGET_PLANEINFO
 * Get the planeinfo for the framebuffer
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int getplaneinfo(struct fb_vtable_s *vtable, int planeno,
            struct fb_planeinfo_s *pinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    //LCDC_INFO("vtable=%p planeno=%d pinfo=%p\n", vtable, planeno, pinfo);

    if (planeno == 0) {
        memcpy(pinfo, &priv->pinfo, sizeof(struct fb_planeinfo_s));
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#ifdef CONFIG_PM
/**
 * setpower - set panel power.
 * @vtable : Reference to lcdc driver interface
 * @power  : Power to set.
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int setpower(FAR struct fb_vtable_s *vtable, int power)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    ASSERT(priv->lcddev != NULL && priv->lcddev->setpower != NULL,
           "%s", __func__);

    return priv->lcddev->setpower(priv->lcddev, power);
}

/**
 * setpower - get panel power.
 * @vtable : Reference to lcdc driver interface
 *
 * Returns:   The current power for panel.
 */
static int getpower(FAR struct fb_vtable_s *vtable)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    ASSERT(priv->lcddev != NULL && priv->lcddev->getpower != NULL,
           "%s", __func__);
    LCDC_REP("lcdc get power %d\n");
    return priv->lcddev->getpower(priv->lcddev);
}
#endif

/**
 * getcmap - Get a range of CLUT values for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cmap  :  Reference to dest color table structure
 *
 * Entrypoint ioctl FBIOGET_CMAP
 * Get a range of CLUT values for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 *
 */
#ifdef CONFIG_FB_CMAP
static int getcmap(struct fb_vtable_s *vtable, struct fb_cmap_s *cmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cmap=%p\n", vtable, cmap);

    if (priv->vinfo.fmt != FB_FMT_RGB8) {
        LCDC_ERR
            ("ERROR: CLUT is not supported for the pixel format: %d\n",
             priv->vinfo.fmt);
        ret = -EINVAL;
    } else if (cmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps. Get the cmap
         * from the main overlay.
         */

        FAR struct lcdc_s *layer;
#ifdef CONFIG_LCDC_L2
        layer = &priv->layer[LCDC_LAYER_L2];
#else
        layer = &priv->layer[LCDC_LAYER_L1];
#endif

        lcdc_sem_wait(layer->lock);

        lcdc_lgetclut(layer, cmap);

        lcdc_sem_post(layer->lock);
        ret = OK;
    }

    return ret;
}

/**
 * putcmap  - Set a range of the CLUT values for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cmap  :  Reference to source color table structure
 *
 * Entrypoint ioctl FBIOPUT_CMAP
 * Set a range of the CLUT values for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcmap(struct fb_vtable_s *vtable, const struct fb_cmap_s *cmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cmap=%p\n", vtable, cmap);

    if (priv->vinfo.fmt != FB_FMT_RGB8) {
        LCDC_ERR
            ("ERROR: CLUT is not supported for the pixel format: %d\n",
             priv->vinfo.fmt);
        ret = -EINVAL;
    } else if (cmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cmap_s *priv_cmap = &g_vtable.cmap;

        /* First copy to internal cmap */

        for (n = cmap->first; n < cmap->len && n < LCDC_NCLUT; n++) {
            priv_cmap->red[n] = cmap->red[n];
            priv_cmap->green[n] = cmap->green[n];
            priv_cmap->blue[n] = cmap->blue[n];
#ifdef CONFIG_FB_TRANSPARENCY
            /* Not supported by LCDC */

            priv_cmap->transp[n] = cmap->transp[n];
#endif
        }

        priv_cmap->first = cmap->first;
        priv_cmap->len = cmap->len;

        /* Update the layer clut register */
        lcdc_sem_wait(&g_lock);
        for (n = 0; n < LCDC_NLAYERS; n++) {
            FAR struct lcdc_s *layer = &priv->layer[n];
            lcdc_lputclut(layer, priv_cmap);
        }
#ifdef CONFIG_DMA2D
        /* Update dma2d cmap */

        priv->dma2d->setclut(cmap);
#endif
        lcdc_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_CMAP */

/**
 * getcadjmap - Get color adjuct curves  for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to source color adjust table structure
 *
 * Entrypoint ioctl FBIOGET_CMAP
 * Get color adjuct curves  for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
#ifdef CONFIG_FB_COLORADJCURVE
static int getcadjmap(struct fb_vtable_s *vtable, struct fb_cadjmap_s *cadjmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cadjmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cadjmap=%p\n", vtable, cadjmap);

    if (cadjmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps. Get the cmap
         * from the main overlay.
         */

        FAR struct lcdc_s *layer;
#ifdef CONFIG_LCDC_L2
        layer = &priv->layer[LCDC_LAYER_L2];
#else
        layer = &priv->layer[LCDC_LAYER_L1];
#endif

        lcdc_sem_wait(layer->lock);

        lcdc_lgetcadjcurves(layer, cadjmap);

        lcdc_sem_post(layer->lock);
        ret = OK;
    }

    return ret;
}

/**
 * putcadjmap - Set color adjust curves for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to dest color adjust table structure
 *
 *  Entrypoint ioctl FBIOPUT_CADJMAP
 * Set color adjust curves for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcadjmap(struct fb_vtable_s *vtable,
              const struct fb_cadjmap_s *cadjmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cadjmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cadjmap=%p\n", vtable, cadjmap);

    if (cadjmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cadjmap_s *priv_cmap = &g_vtable.cadjmap;

        /* First copy to internal cmap */

        for (n = cadjmap->first; n < cadjmap->len && n < LCDC_NCLUT;
             n++) {
            priv_cmap->red[n] = cadjmap->red[n];
            priv_cmap->green[n] = cadjmap->green[n];
            priv_cmap->blue[n] = cadjmap->blue[n];
        }

        priv_cmap->first = cadjmap->first;
        priv_cmap->len = cadjmap->len;

        /* Update the layer clut register */
        lcdc_sem_wait(&g_lock);
        for (n = 0; n < LCDC_NLAYERS; n++) {
            FAR struct lcdc_s *layer = &priv->layer[n];
            lcdc_lputcadjcurves(layer, priv_cmap);
        }
#ifdef CONFIG_DMA2D
        /* Update dma2d cmap */
//      priv->dma2d->setclut(cmap);
#endif
        lcdc_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
/**
 *lcdc_setcadjcurves - Configure color dither table
 *@cadjmap : fb color adjust curves
 */
static int lcdc_setcdither(FAR const struct fb_cdithermap_s *cdithermap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cdithermap->first; n < cdithermap->len && n < 4; n++) {
        priv->cdithermap.data[n] = cdithermap->data[n];
        citem = priv->cdithermap.data[n];

        hal_lcdc_dither_write(n, citem);
        // hal_sys_timer_delay_us(1000);
    }

    return OK;
}

/**
 * putcadjmap - Set color adjust curves for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to source color adjust table structure
 *
 * Entrypoint ioctl FBIOPUT_CADJMAP
 * Set color adjust curves for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcdithermap(struct fb_vtable_s *vtable,
             const struct fb_cdithermap_s *cdithermap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cdithermap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cdithermap=%p\n", vtable, cdithermap);

    if (cdithermap->first >= 4) {
        LCDC_ERR
            ("ERROR: only %d color dither  table entries supported\n",
             4);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cdithermap_s *priv_cmap = &g_vtable.cdithermap;

        /* First copy to internal cmap */

        for (n = cdithermap->first; n < cdithermap->len && n < 4; n++) {
            priv_cmap->data[n] = cdithermap->data[n];
            //LCDC_REP("cdithermap->data[%d]: 0x%x", n, cdithermap->data[n]);
        }

        priv_cmap->first = cdithermap->first;
        priv_cmap->len = cdithermap->len;

        /* Update the layer clut register */
        lcdc_sem_wait(&g_lock);
        lcdc_setcdither(priv_cmap);
#ifdef CONFIG_DMA2D
        /* Update dma2d cmap */
        priv->dma2d->setdither(priv_cmap);
#endif
        lcdc_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_COLORDITHER */

/**
 * waitforvsync - wait for vsync signal
 * @vtable : Reference to lcdc driver interface
 *
 * Entrypoint ioctl FBIO_WAITFORSYNC
 */
#ifdef CONFIG_FB_SYNC
extern int lcdc_waitforirq(void);

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
/**
 * dsi_waitforirq - Helper waits until the dsi te irq occurs.
 */
static int dsi_waitforirq(void)
{
    int ret = OK;
    FAR struct teinterrupt_s *priv = &g_teinterrupt;
    //TC_INIT
    LCDC_REGINFO("dsi_waitforirq();");
    //TC_START
    ret = lcdc_sem_wait(priv->sem);

    if (ret < 0) {
        LCDC_ERR("ERROR: lcdc_sem_wait() failed\n");
    }

    ret = priv->error;

    return ret;
}
#endif

static int waitforvsync(FAR struct fb_vtable_s *vtable)
{
    int ret;

    ASSERT(vtable != NULL && vtable == &g_vtable.vtable, "%s", __func__);
    /* Wait upon vertical synchronization. */
#if defined(CONFIG_FB_PANDISPLAY)
    ret = lcdc_pandisp_trywait();
#elif defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    ret = dsi_waitforirq();
#else
    ret = lcdc_waitforirq();
#endif
    return ret;
}
#endif /* CONFIG_FB_SYNC */

/**
 * getoverlayinfo - get lcdc overlay info
 * @vtable : Reference to lcdc driver interface
 * @overlayno : fb overlay index
 * @oinfo : Reference to dest fb overlay info structure
 *
 * Entrypoint ioctl FBIOGET_OVERLAYINFO
 */
#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    LCDC_INFO("vtable=%p overlay=%d oinfo=%p\n", vtable, overlayno, oinfo);
    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (overlayno < LCDC_NOVERLAYS) {
        FAR struct lcdc_s *layer = &priv->layer[overlayno];
        memcpy(oinfo, &layer->oinfo, sizeof(struct fb_overlayinfo_s));
        LCDC_INFO("oinfo->fbmem=%p\n", oinfo->fbmem);
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * settransp - set fb overlay transparency
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_TRANSP
 */

static int settransp(FAR struct fb_vtable_s *vtable,
             FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    LCDC_INFO("vtable=%p, overlay=%d, transp=%02x, transp_mode=%02x\n",
          vtable, oinfo->overlay, oinfo->transp.transp,
          oinfo->transp.transp_mode);

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (oinfo->transp.transp_mode > 1) {
        LCDC_ERR
            ("ERROR: Returning ENOSYS, transparency mode not supported\n");
        return -ENOSYS;
    }

    if (oinfo->overlay < LCDC_NOVERLAYS) {
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];

        lcdc_sem_wait(layer->lock);
        layer->oinfo.transp.transp = oinfo->transp.transp;
        layer->oinfo.transp.transp_mode = oinfo->transp.transp_mode;

#ifdef CONFIG_DMA2D
        if (layer->oinfo.transp.transp_mode == 0) {
            layer->dma2dinfo.transp_mode = FB_CONST_ALPHA;
        } else if (layer->oinfo.transp.transp_mode == 1) {
            layer->dma2dinfo.transp_mode = FB_PIXEL_ALPHA;
        }

        if (oinfo->overlay < LCDC_NLAYERS)
#endif
        {
            /* Set LCDC blendmode and alpha value */

            lcdc_ltransp(layer, layer->oinfo.transp.transp,
                     layer->oinfo.transp.transp_mode);
        }
        lcdc_sem_post(layer->lock);
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#ifdef CONFIG_LCDC_CHROMAKEY

/**
 * setchromakey - set fb layers chromakey
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_CHROMAKEY
 * Returns:   OK on success, -errno on failure.
 */
static int setchromakey(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    ASSERT(vtable != NULL && priv == &g_vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d, chromakey=%08x\n", vtable,
          oinfo->overlay, oinfo->chromakey);

    if (oinfo->overlay < LCDC_NLAYERS) {
        int ret;
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];

#ifndef CONFIG_LCDC_L1_CHROMAKEY
        if (oinfo->overlay == LCDC_LAYER_L1) {
            return -ENOSYS;
        }
#endif
#ifndef CONFIG_LCDC_L2_CHROMAKEY
        if (oinfo->overlay == LCDC_LAYER_L2) {
            return -ENOSYS;
        }
#endif

        lcdc_sem_wait(layer->lock);
#ifdef CONFIG_FB_CMAP
        if (oinfo->chromakey >= g_vtable.cmap.len) {
            LCDC_ERR("ERROR: Clut index %d is out of range\n",
                 oinfo->chromakey);
            ret = -EINVAL;
        } else
#endif
        {
            layer->oinfo.chromakey = oinfo->chromakey;

            /* Set chromakey */

            lcdc_lchromakey(layer, layer->oinfo.chromakey, 0);
            ret = OK;
        }

        lcdc_sem_post(layer->lock);
        return ret;
    }
#ifdef CONFIG_DMA2D
    else if (oinfo->overlay < LCDC_NOVERLAYS) {
        /* Chromakey not supported by DMA2D */

        return -ENOSYS;
    }
#endif

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#endif

/**
 * setcolor  - set fb layer color by DMA2D
 *
 * Entrypoint ioctl FBIOSET_COLOR
 *
 * Returns:   OK on success, -errno on failure.
 */
static int setcolor(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo)
{
    ASSERT(vtable != NULL && vtable == &g_vtable.vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO(" %s vtable=%p,lid:%d color=%08x\n",__func__,vtable ,oinfo->overlay, (unsigned int)oinfo->color);

    if (oinfo->overlay < LCDC_NOVERLAYS) {
#ifdef CONFIG_DMA2D
        /* Set color within the active overlay is not supported by LCDC. So use
         * DMA2D controller instead when configured.
         */

        int ret;
        FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];
        FAR struct fb_overlayinfo_s *poverlay = layer->dma2dinfo.oinfo;

        ASSERT(&layer->oinfo == poverlay, "%s", __func__);

        lcdc_sem_wait(layer->lock);
        poverlay->color = oinfo->color;
        ret =
            priv->dma2d->fillcolor(&layer->dma2dinfo, &poverlay->sarea,
                       poverlay->color);
        lcdc_sem_post(layer->lock);
        return ret;
#else
        /* Coloring not supported by LCDC */

        return -ENOSYS;
#endif
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * setblank - blank fb layer
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_BLANK
 * Returns:   OK on success, -errno on failure.
 */
static int setblank(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d, blank=%02x\n", vtable ,oinfo->overlay ,oinfo->blank);

    if (oinfo->overlay < LCDC_NLAYERS) {
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];
        lcdc_sem_wait(layer->lock);
        layer->oinfo.blank = oinfo->blank;

        /* Enable or disable layer */

        lcdc_lenable(layer, (layer->oinfo.blank == 0));
        lcdc_sem_post(layer->lock);
        return OK;
    }
#ifdef CONFIG_DMA2D
    else if (oinfo->overlay < LCDC_NOVERLAYS) {
        /* DMA2D overlays are non visible */

        return OK;
    }
#endif

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * setarea - set dma2d layer select area
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_AREA
 * Returns:   OK on success, -errno on failure.
 */
static int setarea(FAR struct fb_vtable_s *vtable,
           FAR const struct fb_overlayinfo_s *oinfo)
{
    ASSERT(vtable != NULL && vtable == &g_vtable.vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d, x=%d, y=%d, w=%d, h=%d\n", vtable,
          oinfo->overlay, oinfo->sarea.x, oinfo->sarea.y,
          oinfo->sarea.w, oinfo->sarea.h);

    if (oinfo->overlay < LCDC_NLAYERS) {
        /* LCDC area is defined by the overlay size (display resolution) only */

        return -ENOSYS;
    }
#ifdef CONFIG_DMA2D
    if (oinfo->overlay < LCDC_NOVERLAYS) {
        FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];
        lcdc_sem_wait(layer->lock);
        memcpy(&layer->oinfo.sarea, &oinfo->sarea,
               sizeof(struct fb_area_s));

        lcdc_sem_post(layer->lock);
        return OK;
    }
#endif

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * blit - copy image form source to dest area
 * @vtable : Reference to lcdc driver interface
 * @blit : Reference to fb overlay blit info structure
 *
 * Entrypoint ioctl FBIOSET_BLIT
 * Returns:   OK on success, -errno on failure.
 */
#ifdef CONFIG_FB_OVERLAY_BLIT
static int blit(FAR struct fb_vtable_s *vtable,
        FAR const struct fb_overlayblit_s *blit)
{
    ASSERT(vtable != NULL && vtable == &g_vtable.vtable
           && blit != NULL, "%s", __func__);
    LCDC_INFO("vtable = %p, blit = %p\n", vtable, blit);
    if (blit->dest.overlay < LCDC_NOVERLAYS
        && blit->src.overlay < LCDC_NOVERLAYS) {
#ifdef CONFIG_DMA2D
        int ret;
        struct fb_area_s sarea;
        FAR const struct fb_area_s *darea = &blit->dest.area;
        FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
        FAR struct lcdc_s *dlayer = &priv->layer[blit->dest.overlay];
        FAR struct lcdc_s *slayer = &priv->layer[blit->src.overlay];

        ASSERT(&dlayer->oinfo == dlayer->dma2dinfo.oinfo &&
               &slayer->oinfo == slayer->dma2dinfo.oinfo, "%s",
               __func__);

        /* DMA2D support image scale, so set to the smallest area */

        memcpy(&sarea, &blit->src.area, sizeof(struct fb_area_s));
        LCDC_INFO("darea x = %d y = %d, sarea x = %d y = %d \n",
              darea->x, darea->y, sarea.x, sarea.y);

        /* Check if area is within the entire overlay */

        if (!lcdc_lvalidate(dlayer, darea) ||
            !lcdc_lvalidate(slayer, &sarea)) {
            LCDC_INFO("%s lcdc_linvalidate", __func__);
            return -EINVAL;
        }
        lcdc_sem_wait(dlayer->lock);

        ret = priv->dma2d->blit(&dlayer->dma2dinfo, darea,
                    &slayer->dma2dinfo, &sarea);

        lcdc_sem_post(dlayer->lock);

        return ret;
#else
        /* LCDC doesn't support blit transfer */

        return -ENOSYS;
#endif
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * blend - blend Back and fore Ground layers to dest layer
 * @vtable : Reference to lcdc driver interface
 * @blend : Reference to fb overlay blend info structure
 *
 * Entrypoint ioctl FBIOSET_BLEND
 * Returns:   OK on success, -errno on failure.
 */
static int blend(FAR struct fb_vtable_s *vtable,
         FAR const struct fb_overlayblend_s *blend)
{
    LCDC_INFO("vtable = %p, blend = %p\n", vtable, blend);
    ASSERT(vtable != NULL && vtable == &g_vtable.vtable
           && blend != NULL, "%s", __func__);

    if (blend->dest.overlay < LCDC_NOVERLAYS &&
        blend->foreground.overlay < LCDC_NOVERLAYS &&
        blend->background.overlay < LCDC_NOVERLAYS) {
#ifdef CONFIG_DMA2D
        int ret;
        struct fb_area_s barea;
        FAR const struct fb_area_s *darea = &blend->dest.area;
        FAR const struct fb_area_s *farea = &blend->foreground.area;
        FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
        FAR struct lcdc_s *dlayer = &priv->layer[blend->dest.overlay];
        FAR struct lcdc_s *flayer =
            &priv->layer[blend->foreground.overlay];
        FAR struct lcdc_s *blayer =
            &priv->layer[blend->background.overlay];

        ASSERT(&dlayer->oinfo == dlayer->dma2dinfo.oinfo &&
               &flayer->oinfo == flayer->dma2dinfo.oinfo &&
               &blayer->oinfo == blayer->dma2dinfo.oinfo, "%s",
               __func__);

        /* DMA2D support image scale */

        memcpy(&barea, &blend->background.area,
               sizeof(struct fb_area_s));

        /* Check if area is within the entire overlay */

        LCDC_REP("%s dlayer on:%d stride:%d  fmt:%d", __func__,
             blend->dest.overlay, dlayer->oinfo.stride,
             dlayer->dma2dinfo.fmt);

        if (!lcdc_lvalidate(dlayer, darea) ||
            !lcdc_lvalidate(flayer, farea) ||
            !lcdc_lvalidate(blayer, &barea)) {
            LCDC_ERR("ERROR: Returning EINVAL\n");
            return -EINVAL;
        }
        lcdc_sem_wait(dlayer->lock);

        ret = priv->dma2d->blend(&dlayer->dma2dinfo, darea,
                     &flayer->dma2dinfo, farea,
                     &blayer->dma2dinfo, &barea);

        lcdc_sem_post(dlayer->lock);
        return ret;
#else
        /* LCDC doesn't support blend transfer */

        return -ENOSYS;
#endif
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}
#endif /* CONFIG_FB_OVERLAY_BLIT */
#endif /* CONFIG_FB_OVERLAY */

/**
 * lcdcreset - Reset LCDC via APB2RSTR
 *
 * Returns:   OK on success, -errno on failure.
 *
 */
void lcdcreset(void)
{
    hal_lcdc_reset();
}

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev);

#if defined(SIMU) && !defined(CONFIG_DSI_VIDEO_MODE)
#include "hal_timer.h"
#include "hwtimer_list.h"
static HWTIMER_ID hw_timer = NULL;
static void timer_handler(void *param)
{
    dsi_te_gpio_irq();
    hwtimer_start(hw_timer, MS_TO_TICKS(17));
}
#endif

/**
 * lcdc_initialize - Initialize the lcdc controller
 *
 * Returns:   OK on success, -errno on failure.
 */
int lcdc_initialize(void *fbmem, uint32_t fbsize)
{
    int ret = OK;
    LCDC_INFO("Initialize LCDC driver fbmem:0x%x,fbsize:%d\n",fbmem,fbsize);
    if (g_initialized == true) {
        return ret;
    }

    /* reset clk */
    hal_lcdc_disp_pn_reset();

#ifdef CONFIG_LCDC_WB_TEST
    hal_cmu_lcdc_clock_enable(0);
#endif
    lcdc_setup_layer_fbmem(fbmem, fbsize);

    LCDC_INFO("Configuring the LCD controller\n");
    /* Configure LCD periphery */
    LCDC_INFO("Configure lcd periphery\n");
    lcdc_periphconfig();

    /* Configure interrupts */
    LCDC_INFO("Configure interrupts\n");

    lcdc_irqconfig();

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    LCDC_INFO("Configure dsi interrupts\n");
    lcdc_teirqconfig();
#endif

    /* Configure global lcdc register */
    LCDC_INFO("Configure global register\n");

    lcdc_globalconfig();

    hal_lcdc_axibus_arbiter_fastmode_enable();
    g_vtable.lcddev = (struct lcd_dev_s *)board_lcd_getdev(0);

#if defined(CONFIG_DMA2D) || defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST)
    /* Initialize the dma2d controller */

    ret = dma2dinitialize();
    LCDC_REGINFO("dma2dinitialize();");

    if (ret != OK) {
        return ret;
    }

    /* Bind the dma2d interface */

#ifndef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
    g_vtable.dma2d = dma2ddev();
    ASSERT(g_vtable.dma2d != NULL, "%s", __func__);
#endif
#endif

#ifdef CONFIG_FB_CMAP
    /* Cleanup clut */

    memset(&g_redclut, 0, LCDC_NCLUT);
    memset(&g_blueclut, 0, LCDC_NCLUT);
    memset(&g_greenclut, 0, LCDC_NCLUT);
#ifdef CONFIG_FB_TRANSPARENCY
    memset(&g_transpclut, 0, LCDC_NCLUT);
#endif
#endif /* CONFIG_FB_CMAP */

    /* Initialize lcdc layer */

    LCDC_INFO("Initialize lcdc layer\n");
    lcdc_linit(LCDC_LAYER_L1);
#ifdef CONFIG_LCDC_L2
    lcdc_linit(LCDC_LAYER_L2);
#endif

#ifdef CONFIG_DSI_VIDEO_MODE
    hal_lcdcl_dumb_enable(true);
    hal_lcdc_dumb_vsync_mode(2);
    hal_lcdc_source_switch(true);
#endif

#ifdef CONFIG_DMA2D
    /* Initialize lcdc dma2d layer */
    lcdc_dma2dlinit();
#endif
    /* Enable the backlight */

#ifdef CONFIG_LCD_BACKLIGHT
    backlight(true);
#endif

    LCDC_INFO("Enabling the display\n");

    /* Set initialized state */
#ifdef CONFIG_PM
    lcdc_pmcb_init();
    ret = pm_register(&module_pmcb);
    if (ret) {
        LCDC_ERR("fail to register module pmcb !!!!!!");
    }
#endif

#if defined(CONFIG_FB_PANDISPLAY)
    LCDC_INFO("Init mutible buffer support.\n");
    lcdc_pandisp_init();
#endif

#if defined(CONFIG_ZOOM_TO_SCREEN)
    struct fb_videoinfo_s devvinfo;
    if (g_vtable.lcddev != NULL && g_vtable.lcddev->getvideoinfo != NULL)
        g_vtable.lcddev->getvideoinfo(g_vtable.lcddev,&devvinfo);

    hal_lcdc_lsize(LCDC_LAYER_SPU, devvinfo.xres,devvinfo.yres);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, devvinfo.xres,devvinfo.yres);

#define LCD_TWCR_TOTALH      (BOARD_LCDC_VSYNC + \
                                        BOARD_LCDC_VBP + \
                                        devvinfo.yres + BOARD_LCDC_VFP - 1)
#define LCD_TWCR_TOTALW      (BOARD_LCDC_HSYNC + \
                                        BOARD_LCDC_HBP + \
                                        devvinfo.xres + BOARD_LCDC_HFP - 1)
#ifdef CONFIG_DSI_VIDEO_MODE
        hal_lcdc_bgl_vhtotal_set(LCD_TWCR_TOTALH+4, LCD_TWCR_TOTALH+8);
#else
        hal_lcdc_bgl_vhtotal_set(LCD_TWCR_TOTALW, LCD_TWCR_TOTALH);
#endif
#endif

    g_initialized = true;

#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef CONFIG_TRIGGER_FRAME_BY_SW
    /* enable software start transfer data, not use te trigger hardware. */
    hal_lcdc_gen_frame_enable(true);
#else
#ifdef CONFIG_LCDC_AUTOREFRESH_MODE
    /* use te trigger hardware to auto transfer */
    hal_lcdc_start_dsi();
#endif
#endif
#if defined(SIMU)
    hw_timer = hwtimer_alloc(timer_handler, 0);
    hwtimer_start(hw_timer, MS_TO_TICKS(17));
#endif
#endif

    hal_lcdc_framedone_irq_enable(true);
    lcdc_enter_lowpower();
    return ret;
}

int lcdc_irq_callback_register(LCDC_IRQ_CALLBACK cb)
{
    g_interrupt.cb = cb;
    return 0;
}

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
int lcdc_teirq_callback_register(LCDC_IRQ_CALLBACK cb)
{
    g_teinterrupt.cb = cb;
    return 0;
}
#endif

/**
 * lcdcgetvplane - Get a reference to the lcdc driver interface for the specified video
 * @vplane : video plane number
 *
 * Returns reference to the lcdc driver interface for the specified video
 * plane.
 *
 */
struct fb_vtable_s *lcdcgetvplane(int vplane)
{
    LCDC_INFO("vplane: %d\n", vplane);

    if (vplane == 0) {
        return &g_vtable.vtable;
    }

    return NULL;
}

/**
 * lcdcuninitialize - Uninitialize the framebuffer lcdc driver
 *
 * Uninitialize the framebuffer driver.  Bad things will happen if you
 * call this without first calling fb_initialize()!
 */
void lcdcuninitialize(void)
{
    /* Disable all lcdc interrupts */
    hal_lcdc_irq_disable();
    NVIC_ClearPendingIRQ(g_interrupt.irq);
    NVIC_DisableIRQ(g_interrupt.irq);

    hal_cmu_dsi_clock_disable();
    hal_cmu_lcdc_clock_disable();
    /* Set initialized state */

    g_initialized = false;
}

/**
 * lcd_backlight - turn the backlight on and off
 *
 * Provide this interface to turn the backlight on and off.
 *
 */
#ifdef CONFIG_LCD_BACKLIGHT
void backlight(bool blon)
{
    /* Set default backlight level CONFIG_STM32_LCDC_DEFBACKLIGHT */

    LCDC_ERR("ERROR: Not supported\n");
}
#endif

/**lcdc_cache_sync - Flush cache to external memory
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 * NOTE: make sure that the buffer must allocted in a safe flush mem range
 *
    |       len            |      used range
  | s  |    32*n        |  e  |   safe range
    32                     32
*/
POSSIBLY_UNUSED
static inline uint8_t lcdc_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef LCDC_DRV_WITH_CACHEOPS
    return 0;
#endif
#ifndef __ARM_ARCH_ISA_ARM
    return hal_cache_sync(id, start_address, len);
#else
#if defined(__NuttX__)
    //return up_flush_dcache(start_address,start_address+len);
    return up_clean_dcache(start_address,start_address+len);
    //return up_coherent_dcache(start_address,len);
#endif
#endif
}

/**lcdc_cache_invalidate - sync external memory to cpu
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 * NOTE: make sure that the buffer must allocted in a safe flush mem range
 *
    |       len            |      used range
  | s  |    32*n        |  e  |   safe range
    32                     32
*/
POSSIBLY_UNUSED
static inline uint8_t lcdc_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef LCDC_DRV_WITH_CACHEOPS
    return 0;
#endif
#ifndef __ARM_ARCH_ISA_ARM
    return hal_cache_invalidate(id, start_address, len);
#else
#if defined(__NuttX__)
    return up_invalidate_dcache(start_address, start_address+len);
#endif
#endif
}

/**
 * lcdc_update - trigger lcdc to generate one frame for display
 */
int lcdc_update(void)
{
    int ret = 0;
    LCDC_INFO("%s enter", __func__);

#ifdef CONFIG_PM
    module_pm_activity();
#endif

    lcdc_exit_lowpower();

#ifdef CONFIG_LCDC_WB_TEST
    lcdc_cache_sync(HAL_CACHE_ID_D_CACHE,CONFIG_LCDC_FBWB_BASE, LCDC_L1_STRIDE * LCDC_HEIGHT);
    hal_lcdc_wdma_toggle_frame();
    hal_lcdc_lstartaddr(LCDC_LAYER_LWB, CONFIG_LCDC_FBWB_BASE);
    hal_lcdc_start_wb();
#else
#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef CONFIG_TRIGGER_FRAME_BY_SW
    hal_lcdc_gen_start();
#else
    hal_lcdc_start_dsi();
#endif
#endif
#endif

#ifdef CONFIG_LCDC_WB_TEST
    lcdc_cache_invalidate(HAL_CACHE_ID_D_CACHE,CONFIG_LCDC_FBWB_BASE, LCDC_L1_STRIDE * LCDC_HEIGHT);
#endif

    LCDC_INFO("lcdcirqstatus:0x%x", (unsigned int)hal_lcdc_get_irqstat());
    LCDC_INFO("%s exit", __func__);
    return ret;
}

/**
 * lcdc_get_wb - get start memory address of lcdc write back layer
 */
uint32_t *lcdc_get_wb(void)
{
    uint32_t *addr = NULL;
#ifdef CONFIG_LCDC_WB_TEST

    addr = (uint32_t *) CONFIG_LCDC_FBWB_BASE;
#endif
    return addr;
}

struct fb_overlayinfo_s *lcdc_get_oinfo(int lid)
{
    FAR struct lcdcdev_s *dev = &g_vtable;
    return &dev->layer[lid].oinfo;

}

#ifdef LCDC_SIMU_TEST_CODE
#include "lcdc_sim_test.h"
#endif //LCDC_SIMU_TEST_CODE
