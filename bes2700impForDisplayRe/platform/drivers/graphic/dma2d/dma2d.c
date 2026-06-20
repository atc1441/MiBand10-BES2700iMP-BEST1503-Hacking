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
 * DOC: DMA2D is a graphic accelerator which offers advanced bit blitting, row
 data copy
 * and pixel format conversion.
 *
 * It supports the following functions:

 * Filling a part or the whole of a destination image with a specific color
 * Copying a part or the whole of a source image into a part or the whole of a
 * destination image.
 * Copying a part or the whole of a source image into a part or the whole of a
 * destination image with a pixel format conversion.
 * Blending a part and/or three complete source images with different pixel
 * format and copy the result into a part or the whole of a destination image
 *  with a different color format.
 * Various image format coding are supported, from indirect 4bpp color mode up
 to
 * 32bpp direct color, include RGB565, RGB1555, RGB888 packed, RGB888 unpacked,
 * RGBA888, YUV422, YUV422 planar, YUV420 planar, Smart Panel command,
 * Palette color 4-bit per pixel, Palette color 8-bit per pixel, RGB888A.
 * It embeds dedicated memory to store color lookup tables.
 * An interrupt can be generated when an operation is complete or at a
 programmed
 * watermark.
 *
 * DMA2D hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,
 *   YUV422packed,YUV422 planar, YUV420 planar, Palette color 4-bit per pixel,
 *   Palette color 8-bit per pixel, RGBA )
  * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,
  *  YUV422 packed,Palette color 4-bit per pixel,Palette color 8-bit per
 pixel,RGB8A )
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


#include "dma2d.h"

#include "cmsis_nvic.h"
#if defined(__NuttX__)
#include "nuttx/video/fb.h"
#ifdef CONFIG_PM
#include "nuttx/power/pm.h"
#endif
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

#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_dma2d.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "stdio.h"
#include "string.h"
#include "hal_dma.h"
#include "dma2d_sem.h"

//#define DMA2D_DRV_WITH_CACHEOPS
#define  GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define DMA2D_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define DMA2D_REG(...) TR_INFO(TR_MOD(TEST), __VA_ARGS__)

#else
#define DMA2D_INFO(...)
#define DMA2D_REG(...)

#endif

#define DMA2D_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define DMA2D_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)

static uint32_t temp_buffer[2048];

#define DMA2D_ALLOC_MEM(...) (temp_buffer)

#define DMA2D_FREE(...)

/**
 * Pre-processor Definitions
 */
#define FAR

/* Only 8 bit per pixel overal supported */

#define DMA2D_PF_BYPP(n) ((n) / 8)

/**
 * Private Types
 */

static volatile bool dma2d_rbswap = 0;

/**
 * struct dma2d_s  -  DMA2D driver interface
 * @dma2d     :   Public dma2d interface
 * @clut      :   Color lookup table
 * @dither    :   Color dither table
 */

struct dma2d_s {
    struct dma2d_layer_s dma2d;

#ifdef CONFIG_DMA2D_FB_CMAP
    uint32_t *clut;
#endif

#ifdef CONFIG_DMA2D_FB_COLORDITHER
    uint32_t *dither;
#endif
/*private: internal use only */
/** @lock : Ensure mutually exclusive access */
    sem_t *lock;
};

/**
 * struct dma2d_interrupt_s   - Interrupt handling
 * @irq           : irq number
 * @error         : error status
 * @handler       : handler entry
 */

struct dma2d_interrupt_s {
    int irq;        /* irq number */
    int error;        /* Interrupt error */
    int (*handler) (int irq, void *context, FAR void *arg);
    /** @sem : Semaphore for waiting for irq */
    sem_t *sem;
};

/**
 * Private Function Prototypes
 */
#ifdef CONFIG_FB_CMAP
static int dma2d_setclut(FAR const struct fb_cmap_s *cmap);

static int dma2d_getclut(FAR const struct fb_cmap_s *cmap);
#endif

#ifdef CONFIG_FB_COLORDITHER
static int dma2d_setdither(FAR const struct fb_cdithermap_s *cmap);
#endif

static int dma2d_fillcolor(FAR struct dma2d_overlay_s *oinfo,
               FAR const struct fb_area_s *area, uint32_t argb);

static int dma2d_blit(FAR struct dma2d_overlay_s *doverlay,
              const struct fb_area_s *darea,
              FAR struct dma2d_overlay_s *soverlay,
              FAR const struct fb_area_s *sarea);

static int dma2d_blend(FAR struct dma2d_overlay_s *doverlay,
               FAR const struct fb_area_s *darea,
               FAR struct dma2d_overlay_s *foverlay,
               FAR const struct fb_area_s *bfarea,
               FAR struct dma2d_overlay_s *boverlay,
               FAR const struct fb_area_s *barea);

/**
 * Private Data
 */

static bool g_initialized = false;
struct dma2d_interrupt_s g_interrupt;

#ifdef CONFIG_DMA2D_FB_CMAP
static uint32_t g_clut[DMA2D_NCLUT *
#ifdef CONFIG_LCDC_FB_TRANSPARENCY
               4
#else
               3
#endif
               / 4];
#endif /* CONFIG_DMA2D_FB_CMAP */

#ifdef CONFIG_DMA2D_FB_COLORDITHER
static uint32_t g_dither[4];
#endif /* CONFIG_DMA2D_FB_CMAP */

/* semaphore for general 2d-dma image buffer covert operation */

static sem_t covert_done;

/* The DMA2D semaphore that enforces mutually exclusive access */

static sem_t g_lock;

/* Semaphore for interrupt handling */

static sem_t g_semirq;

/**
 * struct dma2d_s g_dma2ddev  - dma2d driver instance
 */

static struct dma2d_s g_dma2ddev = {
    .dma2d = {
#ifdef CONFIG_DMA2D_FB_CMAP
          .setclut = dma2d_setclut,
          .getclut = dma2d_getclut,
#endif
#ifdef CONFIG_DMA2D_FB_COLORDITHER
          .setdither = dma2d_setdither,
#endif

          .fillcolor = dma2d_fillcolor,
          .blit = dma2d_blit,
          .blend = dma2d_blend},

#ifdef CONFIG_DMA2D_FB_CMAP
    .clut = g_clut,
#endif

#ifdef CONFIG_DMA2D_FB_COLORDITHER
    .dither = g_dither,
#endif

    .lock = &g_lock
};

/**
 * Private Power Functions
 */

#ifdef CONFIG_PM
#ifndef __unused
#define __unused __attribute__((unused))
#endif
__unused static enum pm_state_e sys_pmstate = PM_NORMAL;
__unused static bool in_work_stat = false;

__unused static void pm_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);
__unused static int pm_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);

#define pm_register(cb) (0)

__unused static struct pm_callback_s module_pmcb = {
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
    sys_pmstate = pmstate;
}

/**
 * pm_prepare  - Request the driver to prepare for a new power state
 * @cb : Returned to the driver. The driver version of the callback
 *       structure may include additional, driver-specific state data at
 *       the end of the structure.
 * @pmstate : Identifies the new PM state
 *
 *   This is a warning that the system is about to enter into a new power state.
 *   The driver should begin whatever operations that may be required to enter
 * power state. The driver may abort the state change mode by returning a
 * non-zero value from the callback function.
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
    if (pmstate > sys_pmstate) {
        if (allow_lower_pwr()) {
            return OK;
        } else {
            return -1;
        }
    } else {
        return OK;
    }
}

__unused static void module_pm_activity(void)
{
    // request sys clk
    // pm_activity(RPMSG_IO_PDOMAIN , RPMSG_IO_ACTIVITY_PRIORITY);
}
#endif // CONFIG_PM

/**
 * Private DMA2D Functions
 */

/**dma2d_cache_sync - Flush cache to external memory
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
static uint8_t dma2d_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef DMA2D_DRV_WITH_CACHEOPS
    return 0;
#endif
    uint32_t buffer[32];
    uint32_t start_address_cache = start_address;
    uint32_t len_cache = len;
    if ((start_address%32)){
        start_address_cache =  start_address - (32 - start_address%32 );
        len_cache += (32 - start_address%32);
    }
    if ((len_cache%32)){
        len_cache += (32 - len_cache%32);
    }

   //TRACE(3, "%s s:0x%x l:0x%x", __func__, start_address_cache, len_cache );
   return hal_cache_sync(id, start_address_cache, len_cache);
//   return hal_cache_sync_all(id);

}

/**dma2d_cache_invalidate - sync external memory to cpu
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
static uint8_t dma2d_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len)
{
#ifndef DMA2D_DRV_WITH_CACHEOPS
    return 0;
#endif
    uint32_t buffer[32];
    uint32_t start_address_cache = start_address;
    uint32_t len_cache = len;
    if ((start_address%32)){
        start_address_cache =  start_address - (32 - start_address%32 );
        len_cache += (32 - start_address%32);
    }
    if ((len_cache%32)){
        len_cache += (32 - len_cache%32);
    }
   //TRACE(3, "%s s:0x%x l:0x%x", __func__, start_address_cache, len_cache );
   return hal_cache_invalidate(id, start_address_cache, len_cache);
}

static void set_dma2d_done_interrupt_handler(struct dma2d_interrupt_s *irq)
{
    NVIC_SetVector(irq->irq, (uint32_t) irq->handler);
    NVIC_SetPriority(irq->irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ(irq->irq);
    NVIC_EnableIRQ(irq->irq);
}


/**
 * dma2dirq -DMA2D interrupt handler
 *
 * Returns: OK On success, -errno on failure.
 */

static int dma2d_irq(int irq, void *context, FAR void *arg)
{
    int ret;
    FAR struct dma2d_interrupt_s *priv = &g_interrupt;
    uint32_t regval = hal_dma2d_get_irqstat();
    DMA2D_INFO("dma2d irq = %d, regval = %08x\n", irq, regval);

    if (hal_dma2d_irq_is_framedone(regval)) {
        /* Transfer complete interrupt */
        DMA2D_REP("DMA transfer complete\n");
        hal_dma2d_spu_framedone_irqstat_clr();

        priv->error = OK;
    } else if (hal_dma2d_irq_is_error(regval)) {
        /* Configuration error interrupt */
        DMA2D_REP("DMA2D ERROR: configuration\n");
        hal_dma2d_error_irqstat_clr();
        priv->error = -ECANCELED;
    } else {
        /* Unknown irq, should not occur */
        ASSERT(0, "Unknown interrupt error\n");
    }

    /* Unlock the semaphore if locked */

    ret = dma2d_sem_post(priv->sem);
    if (ret < 0) {
        DMA2D_ERR("ERROR: dma2d_sem_post() failed\n");
    }

    return priv->error;
}

/**
 * dma2d_waitforirq -Helper waits until the dma2d irq occurs.
 *
 * hat means that an ongoing clut.
 * loading or dma transfer was completed.
 * Note! The caller must use this function within a critical section.
 *
 * Returns: OK On success, -errno on failure.
 */

static int dma2d_waitforirq(void)
{
    int ret;
    FAR struct dma2d_interrupt_s *priv = &g_interrupt;
    TC_INIT
    DMA2D_REG("dma2d_waitforirq();");
    TC_START
    ret = dma2d_sem_wait(priv->sem);
    if (ret < 0) {
        DMA2D_ERR("ERROR: dma2d_sem_wait() failed\n");
        TC_END
        TC_REP(2dcost);
        return ret;
    }

    ret = priv->error;

    TC_END
    TC_REP(2dcost);
    return ret;
}

static int dma2d_start(void)
{
    int ret = 0;

#ifdef CONFIG_PM
    module_pm_activity();
#endif
    //exit low power mode
    hal_dma2d_pnlayer_clock_enable();

    hal_dma2d_axibus_arbiter_fastmode_enable();
    hal_dma2d_start();
  #ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
  #else
    ret = dma2d_waitforirq();
  #endif
    //enter low power mode
    hal_dma2d_pnlayer_clock_disable();

#ifdef CONFIG_PM
    sys_pmstate = PM_NORMAL;
#endif

    return ret;
}

/**
 * dma2d_memaddress - Helper to calculate the layer memory address
 * @oinfo : Reference to overlay information
 * @xpos  : x-Offset
 * @ypos  : y-Offset
 *
 * Returs : start memory address
 *
 */
static uint32_t dma2d_memaddress(FAR struct dma2d_overlay_s *oinfo,
                 uint32_t xpos, uint32_t ypos)
{
    uint32_t offset;
    FAR struct fb_overlayinfo_s *poverlay = oinfo->oinfo;

    offset = xpos * DMA2D_PF_BYPP(poverlay->bpp) + poverlay->stride * ypos;

    // DMA2D_INFO("%p, offset=%d\n", (void *)(((uint32_t) poverlay->fbmem) +
    // offset), offset);
    return ((uint32_t) poverlay->fbmem) + offset;
}

static bool is_dma2d_fast_mem(uint32_t address)
{
   bool ret = true;
   if ( ((address & 0xFF000000) == SYS_PSRAM_BASE) || ((address & 0xFF000000) == SYS_PSRAM_NC_BASE) ){
       ret = false;
   }
   return ret;
}


#ifdef CONFIG_DMA2D_FB_CMAP
/**
 * dma2d_setclut -Configure layer clut (color lookup table).
 * @cmap   - Color lookup table with up the 256 entries
 *
 * Returns: OK On success, -errno on failure.
 */
static int dma2d_setclut(FAR const struct fb_cmap_s *cmap)
{
    int n;

    uint32_t citem;
    FAR struct dma2d_s *priv = &g_dma2ddev;

    // DMA2D_INFO("cmap=%p\n", cmap);
    dma2d_sem_wait(priv->lock);

    for (n = cmap->first; n < cmap->len - 1 && n < DMA2D_NCLUT; n++) {
        /* Update the layer clut entry, will be automatically loaded before
         * blit operation becomes active
         */

        uint8_t *clut = (uint8_t *) g_dma2ddev.clut;
        uint16_t offset = 3 * n;

        clut[offset] = cmap->blue[n];
        clut[offset + 1] = cmap->green[n];
        clut[offset + 2] = cmap->red[n];
        citem =
            ((cmap->red[n] << 16) | (cmap->green[n] << 8) | cmap->
             blue[n]) & 0xffffff;
        // DMA2D_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n,
        // clut[offset],
        //        clut[offset + 1], clut[offset + 2]);
        hal_dma2d_write_clut(citem, n);
    }

    dma2d_sem_post(priv->lock);

    return OK;
}

/**
 * dma2d_getclut - get layer clut (color lookup table).
 * @cmap   : Color lookup table with up the 256 entries
 *
 * Returns: OK On success, -errno on failure.
 */
static int dma2d_getclut(FAR const struct fb_cmap_s *cmap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    FAR struct dma2d_s *priv = &g_dma2ddev;
    uint8_t *clut = (uint8_t *) g_dma2ddev.clut;
    uint16_t offset = 0;

    // DMA2D_INFO("%s cmap=%p\n",__func__ ,cmap);

    for (n = cmap->first; n < cmap->len - 1 && n < DMA2D_NCLUT; n++) {
        offset = 3 * n;
        cmap->blue[n] = clut[offset];
        cmap->green[n] = clut[offset + 1];
        cmap->red[n] = clut[offset + 2];

        // DMA2D_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n,
        // clut[offset],
        //          clut[offset + 1], clut[offset + 2]);
    }
    return OK;
}

#endif /* CONFIG_DMA2D_FB_CMAP */

#ifdef CONFIG_DMA2D_FB_COLORDITHER

/**
 * dma2d_setdither - Configure color dither table.
 * @cmap   : Color dither table with up the 256 entries
 *
 * Returns : OK On success, -errno on failure.
 */
static int dma2d_setdither(FAR const struct fb_cdithermap_s *cmap)
{
    int n;

    uint32_t citem;
    FAR struct dma2d_s *priv = &g_dma2ddev;

    // DMA2D_INFO("cmap=%p\n", cmap);
    dma2d_sem_wait(priv->lock);

    for (n = cmap->first; n < cmap->len && n < 4; n++) {
        /* Update the layer clut entry, will be automatically loaded before
         * blit operation becomes active
         */

        uint32_t *dither = (uint8_t *) g_dma2ddev.dither;
        dither[n] = cmap->data[n];
        citem = dither[n];
        hal_dma2d_dither_write(n, citem);
    }

    dma2d_sem_post(priv->lock);

    return OK;
}
#endif /* CONFIG_DMA2D_FB_COLORDITHER */

static uint8_t dma2d_blend_mode[] = {
    DMA2D_PFCCR_AM_CONST,    //#  define FB_CONST_ALPHA      0x00         /*
    // Transparency by alpha value */
    DMA2D_PFCCR_AM_BLPIXEL,    //#  define FB_PIXEL_ALPHA    0x01         /*
    // Transparency by pixel alpha value */
};

static int sdma2d_fillcolor(FAR struct dma2d_overlay_s *oinfo,
                FAR struct fb_area_s *area, uint32_t argb)
{
    int ret = 0;
    uint8_t *daddr;
    uint8_t *saddr;
    uint8_t *ds;
    uint16_t *nda;
    struct fb_area_s narea = *area;
    // copy src area  to new dest area (width = 4 height = darea->h )
    nda = (uint16_t *) DMA2D_ALLOC_MEM(area->h * 4 * 2);

    struct fb_overlayinfo_s noinfo = *(oinfo->oinfo);
    struct dma2d_overlay_s ndoverlay = *oinfo;
    ndoverlay.oinfo = &noinfo;
    noinfo.fbmem = nda;
    noinfo.fblen = 4 * area->h;
    noinfo.stride = 8;
    ndoverlay.xres = 4;
    narea.w = 4;
    narea.x = 0;
    narea.y = 0;

    ret = dma2d_fillcolor(&ndoverlay, &narea, argb);
    // copy data from new dest area to dest area
    ds = (uint8_t *) dma2d_memaddress(oinfo, area->x, area->y);
    for (int i = 0; i < area->h; i++) {
        daddr = ds + oinfo->oinfo->stride * i;
        saddr = noinfo.fbmem + noinfo.stride * i;
        for (int c = 0; c < area->w * 2; c++) {
            *daddr++ = *saddr++;
        }
    }
    DMA2D_FREE(nda);
    return ret;
}

/**
 * dma2d_fillcolor - Fill the selected area of the whole overlay with a specific
 color
 * @oinfo : Overlay to fill
 * @farea  : Reference to the valid area structure select the area
 * @argb  : Color to fill the selected area. Color must be argb8888 formated.
 *
 * Fill the selected area of the whole overlay with a specific color.
   The caller must ensure that the area is within the entire overlay.
 *
 * Data path: FG layer(oinfo->fb area) --> spu(bg color:rgb)-> write
 back(oinfo->fb area )
 *
 * Returns: OK on success ,-errno on failure. Error values:
 *   - ``-EINVAL``: If one of the parameter invalid or if the size of the
 selected
 *                area outside the visible area of the layer
 *   - ``-ECANCELED``: Operation cancelled, something goes wrong.
 */
static int dma2d_fillcolor(FAR struct dma2d_overlay_s *oinfo,
               FAR const struct fb_area_s *farea, uint32_t argb)
{
    int ret;
    uint16_t pitch = 0;
    struct fb_area_s *area = (struct fb_area_s *)farea;
    uint16_t remain_width = area->w % 4;
    FAR struct dma2d_s *priv = &g_dma2ddev;
    uint32_t flayers = 0;
    if (is_dma2d_fast_mem((uint32_t)oinfo->oinfo->fbmem)){
        flayers = 1;
    }
    ASSERT(oinfo != NULL && oinfo->oinfo != NULL && area != NULL, "%s",
           __func__);

    DMA2D_INFO("%s oinfo=%p, argb=%08x\n", __func__, oinfo, argb);
#if 0
    ASSERT((oinfo->fmt == FB_FMT_BGRA32 || oinfo->fmt == FB_FMT_ABGR32
        || oinfo->fmt == FB_FMT_RGB32), "%s invalid color fmt:%d",
           __func__, oinfo->fmt);
#endif

#ifdef CONFIG_DMA2D_FB_CMAP
    if (oinfo->fmt == FB_FMT_RGB8) {
        /* CLUT output not supported */

        DMA2D_ERR("ERROR: Returning ENOSYS, "
              "output to layer with CLUT format not supported.\n");
        return -ENOSYS;
    }
#endif

    dma2d_sem_wait(priv->lock);

    if ((remain_width != 0) && (oinfo->fmt == FB_FMT_RGB16_565)) {
        area->w -= remain_width;
    }

    hal_dma2d_reset();
    hal_dma2d_irq_disable();
    hal_dma2d_error_irq_enable();
    /* Set layer enable */
    hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
    hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
    hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
    hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

    // reg32_bitset((1 << 24),0x501d0260 );
    hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_CONST, 0, oinfo->fmt);
    hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, oinfo->fmt);

    /*Setup SWAP config */
    hal_dma2d_swap_path(0);
    hal_dma2d_swap_path_enable(true);

    /*Set Blending layers */
    hal_dma2d_lblend(DMA2D_LAYER_LFORE);

    /* Setup smart panel with filled color  */
    hal_dma2d_lcolor(DMA2D_LAYER_SPU, argb);

    /* Set smart panel screen size  */
    hal_dma2d_lsize(DMA2D_LAYER_SPU, area->w, area->h);

    /*set layer dma start mem address */
    hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE,
                 dma2d_memaddress(oinfo, area->x, area->y));
    hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT,
                 dma2d_memaddress(oinfo, area->x, area->y));

    /*set layer image pitch */
    //  hal_dma2d_lpitch(DMA2D_LAYER_LFORE, 0);
    DMA2D_INFO("%s xres:%d bpp:%d", __func__, oinfo->xres,
           oinfo->oinfo->bpp);
    pitch = oinfo->xres * oinfo->oinfo->bpp / 8;
    hal_dma2d_lpitch(DMA2D_LAYER_LFORE, pitch);
    hal_dma2d_lpitch(DMA2D_LAYER_LOUT, pitch);

    /*set  layer image size */
    hal_dma2d_lsize(DMA2D_LAYER_LFORE, 4, 4);
    hal_dma2d_lsize(DMA2D_LAYER_LBACK, 4, 4);
    hal_dma2d_lsize(DMA2D_LAYER_LOUT, area->w, area->h);

    /*config interrupt */
    hal_dma2d_spu_framedone_irq_enable(true);
    hal_dma2d_disp_avsync_trigger_disable(true);
    hal_dma2d_disp_svsync_trigger_disable(true);

    /*setup smpn and write back path work mode */
    hal_dma2d_smpn_mode_set(3);
    hal_dma2d_wdma_1fpt_set();
    hal_dma2d_ol_dmaburst_length_set(8);
    hal_dma2d_sl_dmaburst_maxlength_set();
    hal_dma2d_panelpath_dmatrans_trigger_enable(DMA2D_LAYER_LFORE, true);

    /*config  dispaly module */
    hal_dma2d_disp_mem_enable(true);
    hal_dma2d_vsync_porch_set(DMA2D_ACTION_TYPE_FILL, area->w, flayers, (1 - flayers));

    hal_dma2d_bgl_vhtotal_set(area->w + 2, area->h + 2);
    hal_dma2d_bgl_hporch_set(12, 10);
    hal_dma2d_bgl_vporch_set(10, 10);

    dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,oinfo->oinfo->fbmem, oinfo->oinfo->fblen );
    /* Start DMA2D and wait until completed */
    ret = dma2d_start();
    dma2d_cache_invalidate(HAL_CACHE_ID_D_CACHE,oinfo->oinfo->fbmem, oinfo->oinfo->fblen );

    if (ret != OK) {
        ret = -ECANCELED;
        DMA2D_ERR("ERROR: Returning ECANCELED\n");
    }

    dma2d_sem_post(priv->lock);

    if ((remain_width != 0) && (oinfo->fmt == FB_FMT_RGB16_565)) {
        area->x += area->w;
        area->w = remain_width;

        ret = sdma2d_fillcolor(oinfo, area, argb);
    }

    return ret;
}

static bool is_same_area_size(struct fb_area_s *sarea, struct fb_area_s *darea)
{
    return ((sarea->w == darea->w) && (sarea->h == darea->h));
}

static int sdma2d_blit(FAR struct dma2d_overlay_s *doverlay,
               struct fb_area_s *darea,
               FAR struct dma2d_overlay_s *soverlay,
               FAR struct fb_area_s *sarea)
{
    int ret = 0;
    uint16_t dwidth;
    uint8_t *daddr;
    uint8_t *saddr;
    uint8_t *ds;
    uint16_t *nda;
    // uint16_t res = doverlay->xres;
    // copy src area  to new dest area (width = 4 height = darea->h )
    nda = (uint16_t *) DMA2D_ALLOC_MEM(darea->h * 4 * 2);
    struct fb_area_s ndarea;
    struct fb_overlayinfo_s oinfo = *(doverlay->oinfo);
    struct dma2d_overlay_s ndoverlay = *doverlay;
    ndoverlay.oinfo = &oinfo;
    ndoverlay.xres = 4;

    oinfo.fbmem = nda;
    oinfo.fblen = oinfo.bpp / 2 * darea->h;
    oinfo.stride = oinfo.bpp / 2;

    ndarea = *darea;
    ndarea.w = 4;
    ndarea.x = 0;
    ndarea.y = 0;

    dma2d_blit(&ndoverlay, &ndarea, soverlay, sarea);
    ds = (uint8_t *) dma2d_memaddress(doverlay, darea->x, darea->y);
    dwidth = darea->w * oinfo.bpp / 8;
    //    DMA2D_REP("%s ds:%p dw: %d", __func__,ds,dwidth);

    for (int i = 0; i < darea->h; i++) {
        daddr = ds + doverlay->oinfo->stride * i;
        saddr = (uint8_t *) oinfo.fbmem + oinfo.stride * i;
        memcpy(daddr, saddr, dwidth);
    }
    DMA2D_FREE(nda);

    return ret;
}

/**
 * dma2d_blit - Copy memory from a source overlay to destination
 * @darea    : Valid reference to the selected area of the dest overlay
 * @soverlay : Valid reference to the source overlay
 * @sarea    : Valid reference to the selected area of the source overlay
 *
 * Copy memory from a source overlay (defined by sarea) to destination
 * overlay position (defined by destxpos and destypos).
 *
 * data path: FG layer(oinfo->fb area) --> spu-> write back(oinfo->fb area )
 *
 * Returns: OK on success ,-errno on failure. Error values:
 *   - ``-EINVAL``: If one of the parameter invalid or if the size of the
 * selected source area outside the visible area of the destination layer. (The
 * visible area usually represents the display size)
 *   - ``-ECANCELED``: Operation cancelled, something goes wrong.
 *
 */

static int dma2d_blit(FAR struct dma2d_overlay_s *doverlay,
              const struct fb_area_s *dareac,
              FAR struct dma2d_overlay_s *soverlay,
              FAR const struct fb_area_s *sareac)
{
    int ret = 0;
    struct fb_area_s *darea = (struct fb_area_s *)dareac;
    struct fb_area_s *sarea = (struct fb_area_s *)sareac;
    uint16_t remain_width = sarea->w % 4;
    uint16_t remain_width_d = darea->w % 4;
    FAR struct dma2d_s *priv = &g_dma2ddev;
    struct fb_area_s sarea_o, darea_o;
    bool split = 0;
    bool dss = 0;
    uint32_t flayers = 0;

    TC_INIT
    TC_START
    if (is_dma2d_fast_mem((uint32_t)soverlay->oinfo->fbmem)){
        flayers = 1;
    }
    if (is_dma2d_fast_mem((uint32_t)doverlay->oinfo->fbmem)){
        flayers += 1;
    }
    DMA2D_INFO("%s doverlay=%p, destxpos=%d, destypos=%d, soverlay=%p, sarea=%p\n",
         __func__, doverlay, darea->x, darea->y, soverlay, sarea);
    DMA2D_INFO("%s  destw=%d, desth=%d, sw=%d, sh=%d\n", __func__, darea->w,
           darea->h, sarea->w, sarea->h);

    dma2d_sem_wait(priv->lock);

    if ((remain_width_d != 0) &&
        (soverlay->fmt == FB_FMT_RGB16_565
         || soverlay->fmt == FB_FMT_YUVP)) {
        darea_o = *darea;
        sarea_o = *sarea;
        darea->w -= remain_width_d;

        if (is_same_area_size(sarea, darea)) {
            sarea->w -= remain_width;
            dss = true;
            split = 1;
        } else {
            if (0 == darea->w) {
                split = 1;
            } else {
                *sarea = sarea_o;
                *darea = darea_o;
            }
            dss = false;
        }
    }

    if (darea->w) {
        hal_dma2d_reset();
        hal_dma2d_lpfc_reset();
        hal_dma2d_irq_disable();
        hal_dma2d_error_irq_enable();
        /* Set layer enable */
        hal_dma2dl_enable(DMA2D_LAYER_LBACK, false);
        hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
        hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
        hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

        hal_dma2d_lpfc(DMA2D_LAYER_LFORE, DMA2D_PFCCR_AM_FLPIXEL, 0,
                   soverlay->fmt);
        hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, doverlay->fmt);

        /* Setup smart panel with filled color  */
        hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

        /*Setup SWAP config */
        hal_dma2d_swap_path(0);
        hal_dma2d_swap_path_enable(true);

        /*Set Blending layers */
        hal_dma2d_lblend(DMA2D_LAYER_LFORE);

        /* Set smart panel screen size  */
        hal_dma2d_lsize(DMA2D_LAYER_SPU, darea->w, darea->h);

        /*set layer dma start mem address */
        hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE,
                     dma2d_memaddress(soverlay, sarea->x,
                              sarea->y));
        hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT,
                     dma2d_memaddress(doverlay, darea->x,
                              darea->y));

        /*set layer image pitch */
        hal_dma2d_lpitch(DMA2D_LAYER_LFORE,
                 soverlay->xres * soverlay->oinfo->bpp / 8);
        hal_dma2d_lpitch(DMA2D_LAYER_LOUT,
                 doverlay->xres * doverlay->oinfo->bpp / 8);

        hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, darea->w, darea->h);
        hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, darea->w, darea->h);

        hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
        hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

        /*set layer image size */
        hal_dma2d_lsize(DMA2D_LAYER_LFORE, sarea->w, sarea->h);
        hal_dma2d_lsize(DMA2D_LAYER_LBACK, sarea->w, sarea->h);
        hal_dma2d_lsize(DMA2D_LAYER_LOUT, darea->w, darea->h);

        /*config interrupt */
        hal_dma2d_spu_framedone_irq_enable(true);
        hal_dma2d_disp_avsync_trigger_disable(true);
        hal_dma2d_disp_svsync_trigger_disable(true);

        /*setup smpn and write back path work mode */
        if (doverlay->fmt == FB_FMT_RGB16_565) {
            hal_dma2d_smpn_mode_set(5);
        } else
            hal_dma2d_smpn_mode_set(3);

        hal_dma2d_wdma_1fpt_set();
        hal_dma2d_ol_dmaburst_length_set(8);
        hal_dma2d_sl_dmaburst_maxlength_set();
        hal_dma2d_panelpath_dmatrans_trigger_enable(DMA2D_LAYER_LFORE,
                                true);

        /*config  dispaly module */
        hal_dma2d_disp_mem_enable(true);
        hal_dma2d_vsync_porch_set(DMA2D_ACTION_TYPE_BLIT, sarea->w, flayers, (2 - flayers));

        hal_dma2d_bgl_vhtotal_set(sarea->w + 2, sarea->h + 2);
        hal_dma2d_bgl_hporch_set(12, 10);
        hal_dma2d_bgl_vporch_set(10, 10);

        TRACE(3,"%s fb:%p l:0x%x", __func__,soverlay->oinfo->fbmem, soverlay->oinfo->fblen );
        /* Start DMA2D and wait until completed */
        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,soverlay->oinfo->fbmem, soverlay->oinfo->fblen );
        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,doverlay->oinfo->fbmem, doverlay->oinfo->fblen );

        ret = dma2d_start();
        dma2d_cache_invalidate(HAL_CACHE_ID_D_CACHE,doverlay->oinfo->fbmem, doverlay->oinfo->fblen );

        if (ret != OK) {
            ret = -ECANCELED;
            DMA2D_ERR("ERROR: Returning ECANCELED\n");
        }
    }

    dma2d_sem_post(priv->lock);

    if (split) {
        if (dss) {
            sarea->x += sarea->w;
            darea->x += darea->w;
            sarea->w = remain_width;
            darea->w = remain_width_d;

        } else {
            darea->x += darea->w;
            darea->w = remain_width_d;
        }
        ret = sdma2d_blit(doverlay, darea, soverlay, sarea);
        // restore areas params
        *sarea = sarea_o;
        *darea = darea_o;
    }

    return ret;
}

static int sdma2d_blend(FAR struct dma2d_overlay_s *doverlay,
            struct fb_area_s *darea,
            FAR struct dma2d_overlay_s *foverlay,
            struct fb_area_s *farea,
            FAR struct dma2d_overlay_s *boverlay,
            FAR struct fb_area_s *barea)
{
    int ret = 0;
    uint16_t bwidth = barea->w;
    uint16_t fwidth = farea->w;
    uint16_t dwidth = darea->w;

    uint8_t *daddr;
    uint8_t *saddr;
    uint8_t *ds;
    uint16_t *nda;

    // copy src area  to new dest area (width = 4 height = darea->h )
    nda = (uint16_t *) DMA2D_ALLOC_MEM(darea->h * 4 * 2);
    struct fb_area_s ndarea = *darea;
    struct fb_overlayinfo_s oinfo = *(doverlay->oinfo);
    struct dma2d_overlay_s ndoverlay = *doverlay;
    ndoverlay.oinfo = &oinfo;
    ndoverlay.xres = 4;
    oinfo.fbmem = nda;
    oinfo.fblen = oinfo.bpp / 2 * darea->h;
    oinfo.stride = oinfo.bpp / 2;

    if (is_same_area_size(barea, darea) && is_same_area_size(barea, farea)) {
        farea->w = 4;
        barea->w = 4;
    }
    ndarea.x = 0;
    ndarea.y = 0;
    ndarea.w = 4;

    dma2d_blend(&ndoverlay, &ndarea, foverlay, farea, boverlay, barea);

    if (is_same_area_size(barea, darea) && is_same_area_size(barea, farea)) {
        barea->w = bwidth;
        farea->w = fwidth;
    }
    // copy data from new dest area to dest area

    ds = (uint8_t *) dma2d_memaddress(doverlay, darea->x, darea->y);
    dwidth = darea->w * oinfo.bpp / 8;
    for (int i = 0; i < darea->h; i++) {
        daddr = ds + doverlay->oinfo->stride * i;
        saddr = (uint8_t *) oinfo.fbmem + oinfo.stride * i;
        memcpy(daddr, saddr, dwidth);
    }
    DMA2D_FREE(nda);

    return ret;
}

/**
 * dma2d_blend - Blends the selected areas
 * @doverlay : Destination overlay
 * @darea    : x-Offset, y-Offset, x-resolution and y-resolution of dest layer
 *            overlay
 * @foverlay : Foreground overlay
 * @farea    : x-Offset, y-Offset, x-resolution and y-resolution of foreground
 *            overlay
 * @boverlay : Background overlay
 * @barea    : x-Offset, y-Offset, x-resolution and y-resolution of background
 *            overlay
 *
 * Blends the selected area from a background layer with selected position
 * of the foreground layer. Copies the result to the selected position of
 * the destination layer. Note! The content of the foreground and background
 * layer keeps unchanged as long destination layer is unequal to the
 * foreground and background layer.
 *
 * Returns:  OK - On success, -errno on failure. Error values:
 *   - ``-EINVAL``: If one of the parameter invalid or if the size of the
 * selected source area outside the visible area of the destination layer. (The
 * visible area usually represents the display size)
 *   - ``-ECANCELED``: Operation cancelled, something goes wrong.
 *
 */

static int dma2d_blend(FAR struct dma2d_overlay_s *doverlay,
               const struct fb_area_s *dareac,
               FAR struct dma2d_overlay_s *foverlay,
               const struct fb_area_s *fareac,
               FAR struct dma2d_overlay_s *boverlay,
               FAR const struct fb_area_s *bareac)
{
    int ret = 0;
    struct fb_area_s *darea = (struct fb_area_s *)dareac;
    struct fb_area_s *farea = (struct fb_area_s *)fareac;
    struct fb_area_s *barea = (struct fb_area_s *)bareac;

    FAR struct dma2d_s *priv = &g_dma2ddev;
    // uint16_t pitch = 0;
    uint8_t blend_mode = 0;
    uint8_t alpha = 0;
    uint16_t remain_width = barea->w % 4;
    uint16_t remain_width_f = farea->w % 4;
    uint16_t remain_width_d = darea->w % 4;
    struct fb_area_s barea_o, darea_o, farea_o;
    bool split = 0;
    bool dss = 0;
    uint32_t flayers = 0;
    if (is_dma2d_fast_mem((uint32_t)foverlay->oinfo->fbmem)){
        flayers = 1;
    }
    if (is_dma2d_fast_mem((uint32_t)boverlay->oinfo->fbmem)){
        flayers += 1;
    }
    if (is_dma2d_fast_mem((uint32_t)doverlay->oinfo->fbmem)){
        flayers += 1;
    }
    DMA2D_INFO("%s doverlay=%p, destxpos=%d, destypos=%d \n",__func__,
           doverlay, darea->x, darea->y);

    DMA2D_INFO("foverlay=%p, forexpos=%d, foreypos=%d \n",
           foverlay, farea->x, farea->y);

    DMA2D_INFO("boverlay=%p, barea=%p, barea.x=%d, barea.y=%d, barea.w=%d, "
           "barea.h=%d\n",
           boverlay, barea, barea->x, barea->y, barea->w, barea->h);


#ifdef CONFIG_DMA2D_FB_CMAP
    if (doverlay->fmt == FB_FMT_RGB8) {
        /* CLUT output not supported */

        DMA2D_ERR("ERROR: Returning ENOSYS, "
              "output to layer with CLUT format not supported.\n");
        return -ENOSYS;
    }
#endif

    dma2d_sem_wait(priv->lock);

    if (((remain_width != 0) || (remain_width_d != 0) ||
         (remain_width_f != 0)) &&
        (foverlay->fmt == FB_FMT_RGB16_565 ||
         boverlay->fmt == FB_FMT_RGB16_565 || foverlay->fmt == FB_FMT_YUVP
         || boverlay->fmt == FB_FMT_YUVP)) {
        barea_o = *barea;
        darea_o = *darea;
        farea_o = *farea;
        darea->w -= remain_width_d;

        if (is_same_area_size(barea, darea) &&
            is_same_area_size(barea, farea)) {
            barea->w -= remain_width;
            farea->w -= remain_width_f;
            dss = 1;
            split = 1;
        } else {
            if (0 == darea->w) {
                dss = 0;
                split = 1;
            } else {
                *barea = barea_o;
                *darea = darea_o;
                *farea = farea_o;
            }
        }
    }

    if (darea->w) {
        hal_dma2d_reset();
        hal_dma2d_lpfc_reset();
        hal_dma2d_irq_disable();
        hal_dma2d_error_irq_enable();
        /* Set layer enable */
        hal_dma2dl_enable(DMA2D_LAYER_LBACK, true);
        hal_dma2dl_enable(DMA2D_LAYER_LFORE, true);
        hal_dma2dl_enable(DMA2D_LAYER_LOUT, true);
        hal_dma2dl_enable(DMA2D_LAYER_SPU, true);

        blend_mode = dma2d_blend_mode[boverlay->transp_mode];

        if (blend_mode == DMA2D_PFCCR_AM_CONST) {
            alpha = boverlay->oinfo->transp.transp;
        }

        hal_dma2d_lpfc(DMA2D_LAYER_LFORE, blend_mode, alpha,
                   foverlay->fmt);
        hal_dma2d_lpfc(DMA2D_LAYER_LBACK, 0, 0, boverlay->fmt);

        hal_dma2d_lpfc(DMA2D_LAYER_LOUT, 0, 0, doverlay->fmt);

        /* Setup smart panel with filled color  */
        hal_dma2d_lcolor(DMA2D_LAYER_SPU, 0x000000);

        /*Setup SWAP config */
        hal_dma2d_swap_path(0);
        hal_dma2d_swap_path_enable(true);

        /*Set Blending layers */
        hal_dma2d_lblend(DMA2D_LAYER_LFORE);

        /* Set smart panel screen size  */
        hal_dma2d_lsize(DMA2D_LAYER_SPU, darea->w, darea->h);

        /*set layer dma start mem address */
        hal_dma2d_lstartaddr(DMA2D_LAYER_LFORE,
                     dma2d_memaddress(foverlay, farea->x,
                              farea->y));
        hal_dma2d_lstartaddr(DMA2D_LAYER_LBACK,
                     dma2d_memaddress(boverlay, barea->x,
                              barea->y));
        hal_dma2d_lstartaddr(DMA2D_LAYER_LOUT,
                     dma2d_memaddress(doverlay, darea->x,
                              darea->y));

        /*set layer image pitch */
        hal_dma2d_lpitch(DMA2D_LAYER_LFORE,
                 foverlay->xres * foverlay->oinfo->bpp / 8);
        hal_dma2d_lpitch(DMA2D_LAYER_LBACK,
                 boverlay->xres * boverlay->oinfo->bpp / 8);
        hal_dma2d_lpitch(DMA2D_LAYER_LOUT,
                 doverlay->xres * doverlay->oinfo->bpp / 8);

        hal_dma2d_lzoom_set(DMA2D_LAYER_LFORE, darea->w, darea->h);
        hal_dma2d_lzoom_set(DMA2D_LAYER_LBACK, darea->w, darea->h);

        hal_dma2d_sponscreen_set(DMA2D_LAYER_LFORE, 1, 1);
        hal_dma2d_sponscreen_set(DMA2D_LAYER_LBACK, 1, 1);

        /*set  layer image size */
        hal_dma2d_lsize(DMA2D_LAYER_LFORE, farea->w, farea->h);
        hal_dma2d_lsize(DMA2D_LAYER_LBACK, barea->w, barea->h);
        hal_dma2d_lsize(DMA2D_LAYER_LOUT, darea->w, darea->h);

        /*config interrupt */
        hal_dma2d_spu_framedone_irq_enable(true);
        hal_dma2d_disp_avsync_trigger_disable(true);
        hal_dma2d_disp_svsync_trigger_disable(true);

        if (doverlay->fmt == FB_FMT_RGB16_565) {
            hal_dma2d_smpn_mode_set(5);
        } else
            hal_dma2d_smpn_mode_set(3);

        hal_dma2d_wdma_1fpt_set();
        hal_dma2d_ol_dmaburst_length_set(8);
        hal_dma2d_sl_dmaburst_maxlength_set();

        hal_dma2d_panelpath_dmatrans_trigger_enable(0, true);

        hal_dma2d_disp_mem_enable(true);
        hal_dma2d_vsync_porch_set(DMA2D_ACTION_TYPE_BLEND, farea->w, flayers, (3 - flayers));

        hal_dma2d_bgl_vhtotal_set(barea->w + 2, barea->h + 2);
        hal_dma2d_bgl_hporch_set(12, 10);
        hal_dma2d_bgl_vporch_set(10, 10);

        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,foverlay->oinfo->fbmem, foverlay->oinfo->fblen );
        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,boverlay->oinfo->fbmem, boverlay->oinfo->fblen );
        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE,doverlay->oinfo->fbmem, doverlay->oinfo->fblen );
        /* Start DMA2D and wait until completed */
        ret = dma2d_start();
        dma2d_cache_invalidate(HAL_CACHE_ID_D_CACHE,doverlay->oinfo->fbmem, doverlay->oinfo->fblen );

        if (ret != OK) {
            ret = -ECANCELED;
            DMA2D_ERR("ERROR: Returning ECANCELED\n");
        }
    }

    dma2d_sem_post(priv->lock);

    if (split) {
        if (dss) {
            barea->x += barea->w;
            darea->x += darea->w;
            farea->x += farea->w;
            barea->w = remain_width;
            darea->w = remain_width_d;
            farea->w = remain_width_f;
        } else {
            darea->x += darea->w;
            darea->w = remain_width_d;
        }
        ret =
            sdma2d_blend(doverlay, darea, foverlay, farea, boverlay,
                 barea);
        *barea = barea_o;
        *darea = darea_o;
        *farea = farea_o;
    }

    return ret;
}



/*
 * dam2d public api
 */

/**
 * dma2dinitialize - Initialize the dma2d controller
 *
 * Initialize the dma2d controller
 *
 * Returns: OK - On success, an error if initializing failed.
 */
int dma2dinitialize(void)
{
    DMA2D_INFO("Initialize DMA2D driver\n");
    if (g_initialized == false) {
        hal_psc_display_enable(HAL_PSC_DISPLAY_MOD_IMGDMA);
        hal_cmu_dma2d_clock_enable();
        hal_dma2d_tvlayer_clock_disable();
        // hal_dma2duninitialize();
        g_interrupt.irq = hal_dma2d_irqn();

        dma2d_sem_init(&covert_done, 0, 0);

        dma2d_sem_init(&g_lock, 0, 1);

        g_interrupt.sem = &g_semirq;
        dma2d_sem_init(g_interrupt.sem, 0, 0);

        g_interrupt.handler = dma2d_irq;

        /*cmu open clock gate   todo for chip */

        hal_dma2d_irq_disable();
        hal_dma2d_error_irq_enable();
        /* Attach DMA2D interrupt vector */
        set_dma2d_done_interrupt_handler(&g_interrupt);

#ifdef CONFIG_PM
        int ret = pm_register(&module_pmcb);
        if (ret) {
            DMA2D_ERR("fail to register module pmcb !!!!!!");
        }
#endif
        /* Enable the IRQ at the NVIC */
        g_initialized = true;
    }

    return OK;
}

/**
 * dma2duninitialize - Uninitialize the dma2d controller
 *
 * Uninitialize the dma2d controller
 *
 */

void dma2duninitialize(void)
{
    /* Disable DMA2D clk */
    hal_cmu_dma2d_clock_disable();
    hal_psc_display_disable(HAL_PSC_DISPLAY_MOD_IMGDMA);
    g_initialized = false;
}

/**
 * dma2ddev -Get a reference to the dma2d controller driver instance
 *
 */

FAR struct dma2d_layer_s *dma2ddev(void)
{
    return &g_dma2ddev.dma2d;
}

#ifdef DMA2D_SIMU_TEST_CODE
#include "dma2d_sim_test.h"
#endif // DMA2D_SIMU_TEST_CODE


static struct HAL_DMA_CH_CFG_T dma_cfg;
static struct HAL_DMA_DESC_T desc_bank[80];



static void hw_wait_complete(void)
{
    dma2d_sem_wait(&covert_done);
}

static void dma_irq_handler_covert(void)
{
    dma2d_sem_post(&covert_done);
    TRACE(3, "%s", __func__);
}

static int hwcovert_dma_init(uint32_t *dest)
{
    enum HAL_DMA_PERIPH_T periph;
    uint8_t ch;
    periph = HAL_GPDMA_MEM;
    ch = hal_gpdma_get_chan(periph, HAL_DMA_HIGH_PRIO);
    if (ch == HAL_DMA_CHAN_NONE) {
        TRACE(3, "%s %d", __func__, __LINE__);
        return -1;
    }
    memset(&dma_cfg, 0, sizeof(dma_cfg));
    dma_cfg.handler = dma_irq_handler_covert;
    dma_cfg.dst = (uint32_t)dest;
    dma_cfg.dst_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.dst_periph = periph;
    dma_cfg.dst_width = HAL_DMA_WIDTH_BYTE;
    //dma_cfg.dst_width = HAL_DMA_WIDTH_WORD;
    dma_cfg.src_bsize = HAL_DMA_BSIZE_1;
    dma_cfg.src_periph = 0;
    dma_cfg.src_width = HAL_DMA_WIDTH_BYTE;
    dma_cfg.type = HAL_DMA_FLOW_M2M_DMA;
    dma_cfg.try_burst = 0;
    dma_cfg.ch = ch;
}

/**
 *   use dma channel's 2d dma mode to trans a8 buffer to argb888 buffer.
 *   example: buf{0x1,0x2,0x3,0x4} -> dest_port{0x01xx xxxx,0x02xx xxxx,0x03xx xxxx,0x04xx xxxx}
 *   description of 2d dma:
 *       for(int i = 0 ; i < ycount; i++ )
 *       {
 *           y = y + ymodify * i;
 *           for (int j = 0; j < xcount; j++){
 *               x = xmodify * j  + j;
 *               dest_addr = x + y;
 *           }
 *       }
 *
 *   2d dma setting:
 *      dma_cfg.src = buf;
 *      dst_2d.xmodify = 1;
 *      dst_2d.xcount = 1;
 *      dst_2d.ymodify = 3;
 *      dst_2d.ycount = dma_cfg.src_tsize;
 *
 *      y0 = 3;
*/

/**
 * dma2d_a8_to_argb8888 - blit a8 array to argb8888 buffer
 * @a8 : A8 buffer start address
 * @len  A8 buffer length
 * @argb8888 : Argb8888 buffer start address
 *
 * Returns:  0 - On success, -errno on failure.
 */

int dma2d_a8_to_argb8888(uint8_t * a8,
                     uint32_t len, uint32_t *argb8888)
{
    enum HAL_DMA_RET_T ret;
    uint32_t remain = 0;
    uint32_t dma_frame_sz = (HAL_DMA_MAX_DESC_XFER_SIZE/2) & ~0x03;

    struct HAL_DMA_DESC_T *desc = NULL;
    uint32_t desc_cnt = (len - 1) / dma_frame_sz + 1;
    //TRACE(3, "%s %d desc_cnt:%d len:%d dma_frame_sz:%d ", __func__, __LINE__, desc_cnt, len, dma_frame_sz);
    if (desc_cnt > 128) {
        ASSERT(false, "%s: alloc memeory(size:%d) fail", __func__,
               sizeof(struct HAL_DMA_DESC_T) * desc_cnt);
    }
    desc = desc_bank;
    if (len % dma_frame_sz) {
        remain = len % dma_frame_sz;
    }
    ASSERT(desc, "%s: alloc memeory(size:%d) fail", __func__,
           sizeof(struct HAL_DMA_DESC_T) * desc_cnt);

    hwcovert_dma_init((uint32_t)argb8888 + 3 );

    dma_cfg.src = (uint32_t)a8;

    if (desc_cnt == 1) {
        dma_cfg.src_tsize = len;
        ret = hal_dma_init_desc(&desc[0], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }
        struct HAL_DMA_2D_CFG_T dest_2d;
        dest_2d.xmodify = 0;
        dest_2d.xcount  = 1;
        dest_2d.ymodify = 3;
        dest_2d.ycount  = dma_cfg.src_tsize;

        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, NULL,&dest_2d);
        remain = 0;
    } else {
        int i = 0;
        for (i = 0; i < desc_cnt - 1; i++) {
            dma_cfg.src_tsize = dma_frame_sz;
            ret =
                hal_dma_init_desc(&desc[i], &dma_cfg, &desc[i + 1], 0);
            if (ret != HAL_DMA_OK) {
                goto _err_exit;
            }
            dma_cfg.src += dma_frame_sz;
            dma_cfg.dst += (dma_frame_sz*4);

        }

        dma_cfg.src_tsize = remain;
        ret = hal_dma_init_desc(&desc[i], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }

        struct HAL_DMA_2D_CFG_T dest_2d;
        dest_2d.xmodify = 0;
        dest_2d.xcount = 1;
        dest_2d.ymodify = 3;
        dest_2d.ycount = dma_frame_sz;
        dma_cfg.src_tsize = dma_frame_sz;
        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, NULL, &dest_2d);
    }
_err_exit:
    if (ret != HAL_DMA_OK) {
        hal_dma_free_chan(dma_cfg.ch);
        return len;
    }
    hw_wait_complete();
    return 0;
}

/**
 * dma2d_set_argb8888l_globle_alpha - set globle alpha to argb8888 layer
 * @a8 : A8 buffer start address
 * @len  argb8888 pix number
 * @argb8888 : Argb8888 buffer start address
 *
 * Returns:  0 - On success, -errno on failure.
 */

int dma2d_set_argb8888l_globle_alpha(uint8_t * a8,
                     uint32_t len, uint32_t *argb8888)
{
    enum HAL_DMA_RET_T ret;
    uint32_t remain = 0;
    uint32_t dma_frame_sz = (HAL_DMA_MAX_DESC_XFER_SIZE/2) & ~0x03;

    struct HAL_DMA_DESC_T *desc = NULL;
    uint32_t desc_cnt = (len - 1) / dma_frame_sz + 1;
    TRACE(3, "%s %d desc_cnt:%d len:%d dma_frame_sz:%d ", __func__, __LINE__, desc_cnt, len, dma_frame_sz);
    if (desc_cnt > 128) {
        ASSERT(false, "%s: alloc memeory(size:%d) fail", __func__,
               sizeof(struct HAL_DMA_DESC_T) * desc_cnt);
    }
    desc = desc_bank;
    if (len % dma_frame_sz) {
        remain = len % dma_frame_sz;
    }
    ASSERT(desc, "%s: alloc memeory(size:%d) fail", __func__,
           sizeof(struct HAL_DMA_DESC_T) * desc_cnt);

    hwcovert_dma_init((uint32_t)argb8888 + 3 );

    dma_cfg.src = (uint32_t)a8;

    if (desc_cnt == 1) {
        dma_cfg.src_tsize = len;
        ret = hal_dma_init_desc(&desc[0], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }

        struct HAL_DMA_2D_CFG_T src_2d;
        src_2d.xmodify = -1;
        src_2d.xcount  = 0;
        src_2d.ymodify = 0;
        src_2d.ycount  = 0;



        struct HAL_DMA_2D_CFG_T dest_2d;
        dest_2d.xmodify = 0;
        dest_2d.xcount  = 1;
        dest_2d.ymodify = 3;
        dest_2d.ycount  = dma_cfg.src_tsize;

        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, &src_2d,&dest_2d);
        remain = 0;
    } else {
        int i = 0;
        for (i = 0; i < desc_cnt - 1; i++) {
            dma_cfg.src_tsize = dma_frame_sz;
            ret =
                hal_dma_init_desc(&desc[i], &dma_cfg, &desc[i + 1], 0);
            if (ret != HAL_DMA_OK) {
                goto _err_exit;
            }
            dma_cfg.dst += (dma_frame_sz*4);

        }

        dma_cfg.src_tsize = remain;
        ret = hal_dma_init_desc(&desc[i], &dma_cfg, NULL, 1);
        if (ret != HAL_DMA_OK) {
            goto _err_exit;
        }
        struct HAL_DMA_2D_CFG_T src_2d;
        src_2d.xmodify = -1;
        src_2d.xcount  = 0;
        src_2d.ymodify = 0;
        src_2d.ycount  = 0;

        struct HAL_DMA_2D_CFG_T dest_2d;
        dest_2d.xmodify = 0;
        dest_2d.xcount = 1;
        dest_2d.ymodify = 3;
        dest_2d.ycount = dma_frame_sz;
        dma_cfg.src_tsize = dma_frame_sz;
        ret = hal_dma_sg_2d_start(&desc[0], &dma_cfg, &src_2d, &dest_2d);
    }
_err_exit:
    if (ret != HAL_DMA_OK) {
        hal_dma_free_chan(dma_cfg.ch);
        return len;
    }
    hw_wait_complete();
    return 0;
}

#if 0 //test program

static uint8_t a8[8000+40]={
};

static uint32_t argb8888[8000+40]={
};


void a8_to_argb888_test(void)
{

    memset(a8, 0x5a, sizeof(a8));

    memset(argb8888, 0x46, sizeof(argb8888));
    dma2d_a8_to_argb8888(a8,
                     sizeof(a8), argb8888);
    for(int i = 0; i < sizeof(a8); i++ ){
       if (argb8888[i] != 0x5a000000){
           TRACE(3,"%s fail!!!!!!!!!!! i:%d | 0x%x", __func__,i ,argb8888[i]);
           while(1);
       }
    }
    TRACE(3, "%2x %2x %2x %2x %2x %2x", argb8888[0], argb8888[1], argb8888[2], argb8888[3],argb8888[4], argb8888[5] );
}

void dma2d_set_argb8888l_globle_alpha_test(void)
{

    uint8_t a8 = 0x5a;

    memset(argb8888, 0x46, sizeof(argb8888));
    dma2d_set_argb8888l_globle_alpha(&a8,
                     sizeof(argb8888)/4, argb8888);
    for(int i = 0; i < sizeof(argb8888)/4; i++ ){
       if (argb8888[i] != 0x5a464646){
           TRACE(3,"%s fail!!!!!!!!!!! i:%d | 0x%x", __func__,i ,argb8888[i]);
           while(1);
       }
    }
    TRACE(3, "%2x %2x %2x %2x %2x %2x", argb8888[0], argb8888[1], argb8888[2], argb8888[3],argb8888[4], argb8888[5] );
}

#endif
