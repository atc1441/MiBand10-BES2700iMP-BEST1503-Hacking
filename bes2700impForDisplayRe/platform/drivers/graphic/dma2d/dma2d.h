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
#ifndef __DMA2D_H
#define __DMA2D_H

#ifdef __cplusplus
extern "C" {
#endif


/****************************************************************************
 * Included Files
 ****************************************************************************/
#if defined(__NuttX__)
#include <nuttx/config.h>
#include <nuttx/video/fb.h>
#else
#include "fb.h"
#endif

/*
 * Pre-processor Definitions
 */

#ifdef CONFIG_FB_OVERLAY

/*
 * Public Types
 */

/* This structure describes DMA2D overlay information */

struct dma2d_overlay_s {
    uint8_t fmt;        /* DMA2D pixel format */
    uint8_t transp_mode;    /* DMA2D transparency mode */
    fb_coord_t xres;    /* X-resolution overlay */
    fb_coord_t yres;    /* Y-resolution overlay */
    FAR struct fb_overlayinfo_s *oinfo;    /* Framebuffer overlay information */
};

/* DMA2D overlay driver operations */

struct dma2d_layer_s {
    /*   Set the cmap table for both foreground and background layer.
     *   Up to 256 colors supported.
     */

#ifdef CONFIG_FB_CMAP
    int (*setclut) (FAR const struct fb_cmap_s * cmap);
    int (*getclut) (FAR const struct fb_cmap_s * cmap);
#endif

#ifdef CONFIG_DMA2D_FB_COLORDITHER
    int (*setdither) (FAR const struct fb_cdithermap_s * cmap);
#endif

    /*   Fill a specific memory region with a color.
     *   The caller must ensure that the memory region (area) is within the entire
     *   overlay.
     */

    int (*fillcolor) (FAR struct dma2d_overlay_s * oinfo,
              FAR const struct fb_area_s * area, uint32_t argb);

    /* Copies memory from a source overlay (defined by sarea) to destination
     *   overlay position (defined by destxpos and destypos) without pixelformat
     *   conversion. The caller must ensure that the memory region (area) is
     *   within the entire overlay.
     */

    int (*blit) (FAR struct dma2d_overlay_s * doverlay,
             FAR const struct fb_area_s * darea,
             FAR struct dma2d_overlay_s * soverlay,
             FAR const struct fb_area_s * sarea);
    /* Blends two source memory areas to a destination memory area with
     *   pixelformat conversion if necessary. The caller must ensure that the
     *   memory region (area) is within the entire overlays.
     */
    int (*blend) (FAR struct dma2d_overlay_s * doverlay,
              FAR const struct fb_area_s * darea,
              FAR struct dma2d_overlay_s * foverlay,
              FAR const struct fb_area_s * farea,
              FAR struct dma2d_overlay_s * boverlay,
              FAR const struct fb_area_s * barea);
};

/*
 * dam2d driver api
 */

/**
 * dma2ddev -Get a reference to the dma2d controller
 *
 */

FAR struct dma2d_layer_s *dma2ddev(void);

/**
 * dma2dinitialize - Initialize the dma2d controller
 *
 * Initialize the dma2d controller
 *
 * Returns: OK - On success, an error if initializing failed.
 */

int dma2dinitialize(void);

/**
 * dma2duninitialize - Uninitialize the dma2d controller
 *
 * Uninitialize the dma2d controller
 *
 */

void dma2duninitialize(void);

#endif /* CONFIG_FB_OVERLAY */

#ifdef __cplusplus
}
#endif

#endif /* __ARCH_ARM_SRC_DMA2D_H */
