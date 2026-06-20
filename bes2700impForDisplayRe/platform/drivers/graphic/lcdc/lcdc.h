
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
 * multiple DMA requests concatenated together to increase DDR page hit rate.
 * line buffer mode DMA to increase DDR page hit rate and reduce internal SRAM
 * occupation.
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

#ifndef _LCDC_H
#define _LCDC_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Included Files
 */
#include "plat_types.h"
#ifdef __NuttX__
#include <nuttx/power/pm.h>
#else
#include "pm.h"
#endif
/*
 * Pre-processor Definitions
 */
#define FAR

#define LCDC_NCLUT           256    /* Number of entries in the CLUT */

#define IRQ_LCDCINT

/*
 * Public Types
 */

typedef void (*LCDC_IRQ_CALLBACK)(int error);

/*
 * lcdc driver api
 */

/**
 * lcdc_cadjcurves_enable - enable/disable foreground and background color adjust curves
 */
void lcdc_cadjcurves_enable(bool enable);

/**
 * lcdc_initialize - Initialize the lcdc controller
 *
 * Returns:   OK on success, -errno on failure.
 */
int lcdc_initialize(void *fbmem, uint32_t fbsize);

int lcdc_irq_callback_register(LCDC_IRQ_CALLBACK cb);

int lcdc_teirq_callback_register(LCDC_IRQ_CALLBACK cb);

/**
 * lcdcgetvplane - Get a reference to the lcdc driver interface for the specified video
 * @vplane : video plane number
 *
 * Returns reference to the lcdc fb driver interface for the specified video
 * plane.
 *
 */
struct fb_vtable_s *lcdcgetvplane(int vplane);

/**
 * lcdcuninitialize - Uninitialize the framebuffer lcdc driver
 *
 * Uninitialize the framebuffer driver.  Bad things will happen if you
 * call this without first calling fb_initialize()!
 */
void lcdcuninitialize(void);

struct fb_vtable_s *lcdcgetvplane(int vplane);

/**
 * lcdc_update - trigger lcdc to generate one frame for display
 */
int lcdc_update(void);

#ifdef CONFIG_PM

/**
 * @brief lcdc_pmcb_early_register - register pm callbacks by touchdev
 *
 * @param cb  : pm callbacks
 * @return int : (OK) success; (<0) failed
 */
int lcdc_pmcb_early_register(struct pm_callback_s *cb);

/**
 * @brief lcdc_pmcb_register - register pm callbacks by lcddev
 *
 * @param cb : pm callbacks
 * @return int : (OK) success; (<0) failed
 */
int lcdc_pmcb_register(struct pm_callback_s *cb);

/**
 * @brief lcdc_pmcb_unregister - unregister pm callbacks
 *
 * @param cb : callbacks
 * @return int : always success
 */
int lcdc_pmcb_unregister(struct pm_callback_s *cb);
#endif

void lcdc_enter_lowpower(void);

void lcdc_exit_lowpower(void);

void lcdc_exit_lowpower_with_cb(LCDC_IRQ_CALLBACK cb, void *param);

#ifdef __cplusplus
}
#endif

#endif //_LCDC_H
