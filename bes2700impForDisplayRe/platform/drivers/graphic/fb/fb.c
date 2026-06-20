
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
 * DOC: lcdc and dma2d driver test program with fb interface
 *
 * "/dev/fb0" is the device node path for lcdc framebuffer driver
 * "/dev/fb_dma2d" is the device node path for dma2d framebuffer driver
 *
 *  fb_graphic_test is the program entry of lcdc and dma2d driver test
 *
 *  features of lcdc and dma2d configured with target.mk
 *  lcdc_conf.h includes lcd panel related Configuration
 *
 * lcdc testcase: blit, blend, blend_scale, fill color, pan display,
 * rect update, video mirror, color key, color dither, color adjust
 * NOTE:lcdc testcases show the usage of lcdc driver interface on lcdc layers
 *
 * dma2d testcase: fill color, blend, blit , clut,  color fmt convertion
 * NOTE: dma2d testcase shows how to use dma2d functions on dynamicly allocated
 * user overlayers
 */

#include "fb.h"

#include "cmsis_nvic.h"
#include "dma2d.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "heap_api.h"
#include "lcdc.h"
#include "string.h"
#include "hal_gpio.h"
#include "hal_iomux.h"

/**
 *      DEFINES
 */
#ifndef FBDEV_PATH

#define FBDEV_FB0_PATH "/dev/fb0"
#define FBDEV_DMA2D_PATH "/dev/fb_dma2d"

#endif

//#define GPIO_TRIGGER_FOR_TEST

#ifdef GPIO_TRIGGER_FOR_TEST
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
    {HAL_IOMUX_PIN_P8_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};
#endif

static void lcd_gpio_setup(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(HAL_IOMUX_PIN_P8_7, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P8_7);
#endif
}

void lcd_set_start_flag(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_gpio_pin_set(HAL_IOMUX_PIN_P8_7);
#endif
}

void lcd_set_end_flag(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P8_7);
#endif
}

#define FB_MEM_RESULT_CHECK

//#define FB_SIM
#define WB_TEST

#ifdef  WITH_OS
#undef  WITH_OS
#endif

#ifdef FB_SIM
#define FB_INFO(...)
#define FB_REP(...)
#define FB_ERR(...)
#else
//#define FB_INFO(...)
#define FB_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define FB_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define FB_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)
#endif

#define FAR
#define DEBUGASSERT(x) ASSERT((x), "%s %d", __func__, __LINE__)

#define fb_overlay_alloc(olptr, saddr, llen, lstride, lidx, pixbpp, lcolor, ltransp, ltransp_mode, laccl) \
    do {                                      \
        olptr->fbmem = (void *)saddr;           \
        olptr->fblen = llen;                     \
        olptr->stride = lstride;                   \
        olptr->overlay = lidx;                     \
        olptr->bpp = pixbpp;                       \
        olptr->color = lcolor;                     \
        olptr->transp.transp = ltransp;            \
        olptr->transp.transp_mode = ltransp_mode;  \
        olptr->accl = laccl;                       \
    } while (0)

/**
 * typedef struct  lv_2d_drv_t  - dma2d driver struct
 * @fb       : fb driver interface
 * @fbp      : fb memory base address
 */
typedef struct {
    struct fb_chardev_s *fb;
    char *fbp;
} lv_2d_drv_t;

/**
 *  Private variables
 */

/* lcdc/dma test program allocates layer buffers from below share memory blocks   */

#if defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST) || defined(CONFIG_DMA2D)
//static volatile uint32_t *fb_base = (uint32_t *) M55_SYS_RAM_BASE;//PSRAM_NC_BASE;
static volatile uint32_t *fb_base = (uint32_t *)PSRAM_BASE;
#endif

static heap_handle_t fb_heap;

POSSIBLY_UNUSED
static heap_handle_t fb_mem_setup(uint32_t * base_addr, uint32_t size)
{
#ifndef FB_SIM
    memset(base_addr, 0, size);
#endif
    return heap_register(base_addr, size);
}

static void *fb_malloc(uint32_t size)
{
    return heap_malloc(fb_heap, size + 8);
}

static void fb_mfree(void *mem)
{
    heap_free(fb_heap, mem);
}

#define FB_MALLOC fb_malloc
#define FB_FREE fb_mfree

#if defined(WITH_OS)
static long int screensize = 0;
#endif

static uint32_t fb0fd = 0;    /*handle for fb0 */

POSSIBLY_UNUSED
static uint32_t fbfd_dma2d = 0;    /*handle for dma2d device driver */

static lv_2d_drv_t lv_2d;

static struct fb_overlayinfo_s oinfo[2];
static uint8_t cfmt[3];

static void fb_overlay_cfmt_set(uint8_t lid, uint8_t fmt)
{
    cfmt[lid] = fmt;
}

#if defined(CONFIG_DMA2D)||defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST)
static uint8_t get_color_fmt(int lid)
{
    uint32_t fmt = 0;
    fmt = cfmt[lid];
    return (uint8_t) fmt;
}

static struct fb_overlayinfo_s *get_oinfo(int lid)
{
    return &oinfo[lid];
}
#endif

/**
 * fb_ioctl  - lcdc framebuffer driver ioctrl interface
 *
 */

int fb_ioctl(uint32_t fd, int cmd, unsigned long arg)
{
    FAR struct fb_chardev_s *fb;
    int ret;

    //FB_INFO("%s fd:%x cmd: %d arg: %lx\n", __func__, fd, cmd, arg);

    /* Get the framebuffer instance */

    fb = (FAR struct fb_chardev_s *)fd;

    /* Process the IOCTL command */

    switch (cmd) {
    case FBIOGET_VIDEOINFO:    /* Get color plane info */
        {
            FAR struct fb_videoinfo_s *vinfo = (FAR struct fb_videoinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(vinfo != 0 && fb->vtable != NULL && fb->vtable->getvideoinfo != NULL);
            ret = fb->vtable->getvideoinfo(fb->vtable, vinfo);
        } break;

    case FBIOGET_PLANEINFO:    /* Get video plane info */
        {
            FAR struct fb_planeinfo_s *pinfo = (FAR struct fb_planeinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(pinfo != 0 && fb->vtable != NULL && fb->vtable->getplaneinfo != NULL);
            ret = fb->vtable->getplaneinfo(fb->vtable, fb->plane, pinfo);
        } break;

#ifdef CONFIG_FB_CMAP
    case FBIOGET_CMAP:    /* Get RGB color mapping */
        {
            FAR struct fb_cmap_s *cmap = (FAR struct fb_cmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->getcmap != NULL);
            ret = fb->vtable->getcmap(fb->vtable, cmap);
        } break;

    case FBIOPUT_CMAP:    /* Put RGB color mapping */
        {
            FAR const struct fb_cmap_s *cmap = (FAR struct fb_cmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->putcmap != NULL);
            ret = fb->vtable->putcmap(fb->vtable, cmap);
        } break;
#endif

#ifdef CONFIG_FB_COLORADJCURVE
    case FBIOGET_CADJMAP:    /* Get RGB color mapping */
        {
            FAR struct fb_cadjmap_s *cmap = (FAR struct fb_cadjmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->getcadjmap != NULL);
            ret = fb->vtable->getcadjmap(fb->vtable, cmap);
        } break;

    case FBIOPUT_CADJMAP:    /* Put RGB color mapping */
        {
            FAR const struct fb_cadjmap_s *cmap = (FAR struct fb_cadjmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->putcadjmap != NULL);
            ret = fb->vtable->putcadjmap(fb->vtable, cmap);
        } break;
#endif

#ifdef CONFIG_FB_COLORDITHER

    case FBIOPUT_CDITHER:    /* Put RGB color dither table */
        {
            FAR const struct fb_cdithermap_s *cdithermap = (FAR struct fb_cdithermap_s *)((uintptr_t) arg);

            DEBUGASSERT(cdithermap != 0 && fb->vtable != NULL && fb->vtable->putcdithermap != NULL);
            ret = fb->vtable->putcdithermap(fb->vtable, cdithermap);
        } break;
#endif

#ifdef CONFIG_FB_HWCURSOR
    case FBIOGET_CURSOR:    /* Get cursor attributes */
        {
            FAR struct fb_cursorattrib_s *attrib = (FAR struct fb_cursorattrib_s *)((uintptr_t) arg);

            DEBUGASSERT(attrib != 0 && fb->vtable != NULL && fb->vtable->getcursor != NULL);
            ret = fb->vtable->getcursor(fb->vtable, attrib);
        } break;

    case FBIOPUT_CURSOR:    /* Set cursor attibutes */
        {
            FAR struct fb_setcursor_s *cursor = (FAR struct fb_setcursor_s *)((uintptr_t) arg);

            DEBUGASSERT(cursor != 0 && fb->vtable != NULL && fb->vtable->setcursor != NULL);
            ret = fb->vtable->setcursor(fb->vtable, cursor);
        } break;
#endif

#ifdef CONFIG_FB_UPDATE
    case FBIO_UPDATE:    /* Update the LCD with the modified framebuffer data  */
        {
            struct fb_area_s *area = (FAR struct fb_area_s *)((uintptr_t) arg);

            DEBUGASSERT(fb->vtable != NULL && fb->vtable->updatearea != NULL);
            ret = fb->vtable->updatearea(fb->vtable, area);
        } break;
#endif

#ifdef CONFIG_FB_SYNC
    case FBIO_WAITFORVSYNC:    /* Wait upon vertical sync */
        {
            ret = fb->vtable->waitforvsync(fb->vtable);
        }
        break;
#endif

#ifdef CONFIG_FB_OVERLAY
    case FBIO_SELECT_OVERLAY:    /* Select video overlay */
        {
            struct fb_overlayinfo_s oinfo;

            DEBUGASSERT(fb->vtable != NULL && fb->vtable->getoverlayinfo != NULL);
            ret = fb->vtable->getoverlayinfo(fb->vtable, arg, &oinfo);
            if (ret == OK) {
                fb->fbmem = oinfo.fbmem;
                fb->fblen = oinfo.fblen;
                fb->bpp = oinfo.bpp;
            }
        }
        break;

    case FBIOGET_OVERLAYINFO:    /* Get video overlay info */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);
            //FB_INFO("oinfo:%p fb:%p fb->vtable:%p fb->vtable->getoverlayinfo:%p ", oinfo, fb, fb->vtable, fb->vtable->getoverlayinfo);
            ASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->getoverlayinfo != NULL, "%s", __func__);

            ret = fb->vtable->getoverlayinfo(fb->vtable, oinfo->overlay, oinfo);
        }
        break;

    case FBIOSET_TRANSP:    /* Set video overlay transparency */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->settransp != NULL);
            ret = fb->vtable->settransp(fb->vtable, oinfo);
        } break;

#ifdef CONFIG_LCDC_CHROMAKEY

    case FBIOSET_CHROMAKEY:    /* Set video overlay chroma key */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setchromakey != NULL);
            ret = fb->vtable->setchromakey(fb->vtable, oinfo);
        } break;
#endif
    case FBIOSET_COLOR:    /* Set video overlay color */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setcolor != NULL);
            ret = fb->vtable->setcolor(fb->vtable, oinfo);
        } break;

    case FBIOSET_BLANK:    /* Blank or unblank video overlay */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setblank != NULL);
            ret = fb->vtable->setblank(fb->vtable, oinfo);
        } break;

    case FBIOSET_AREA:    /* Set active video overlay area */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setarea != NULL);
            ret = fb->vtable->setarea(fb->vtable, oinfo);
        } break;

#ifdef CONFIG_FB_PANDISPLAY

    case FBIOPAN_DISPLAY:{
            FAR struct fb_planeinfo_s *var = (FAR struct fb_planeinfo_s *)((uintptr_t) arg);
            ret = fb->vtable->pandisplay(fb->vtable, var);
        } break;
#endif

#ifdef CONFIG_FB_VMIRRO

    case FBIOSET_VMIRRO:{
            FAR struct fb_var_screeninfo *var = (FAR struct fb_var_screeninfo *)((uintptr_t) arg);
            ret = fb->vtable->video_mirror(fb->vtable, var);
        } break;
#endif

#ifdef CONFIG_FB_OVERLAY_BLIT
    case FBIOSET_BLIT:    /* Blit operation between video overlays */
        {
            FAR struct fb_overlayblit_s *blit = (FAR struct fb_overlayblit_s *)((uintptr_t) arg);

            DEBUGASSERT(blit != 0 && fb->vtable != NULL && fb->vtable->blit != NULL);
            ret = fb->vtable->blit(fb->vtable, blit);
        } break;

    case FBIOSET_BLEND:    /* Blend operation between video overlays */
        {
            FAR struct fb_overlayblend_s *blend = (FAR struct fb_overlayblend_s *)((uintptr_t) arg);

            DEBUGASSERT(blend != 0 && fb->vtable != NULL && fb->vtable->blend != NULL);
            ret = fb->vtable->blend(fb->vtable, blend);
        } break;
#endif
#endif /* CONFIG_FB_OVERLAY */

    default:
        FB_ERR("ERROR: Unsupported IOCTL command: %d\n", cmd);
        ret = -ENOTTY;
        break;
    }

    return ret;
}

#if !defined(WITH_OS)
static int no_os_ioctl(uint32_t fd, int cmd, void *cmd_data)
{
    int ret = 0;

    ret = fb_ioctl(fd, cmd, (unsigned long)cmd_data);

    return ret;
}

POSSIBLY_UNUSED static char *no_os_mmap(void *addr, size_t length, int prot, int flags, int fd, int32_t offset)
{
    char *mapaddr = 0;

    struct fb_chardev_s *fb = (struct fb_chardev_s *)fd;
    mapaddr = (char *)fb->fbmem;
    return mapaddr;
}
#endif


#if defined(CONFIG_DMA2D)||defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST)

/**
 *  private dma2d fb ioctrl functions.
 */

static int dma2d_fillcolor(FAR struct fb_vtable_s *vtable, FAR const struct fb_overlayinfo_s *oinfo)
{
    struct dma2d_overlay_s dol;
    struct dma2d_layer_s *dma2d = dma2ddev();
    dol.oinfo = get_oinfo(oinfo->overlay);
    dol.fmt = get_color_fmt(oinfo->overlay);
    dol.xres = oinfo->stride * 8 / oinfo->bpp;
    dol.yres = oinfo->fblen * 8 / oinfo->bpp / oinfo->stride;
    // dol.oinfo = (struct fb_overlayinfo_s *)oinfo;

    return dma2d->fillcolor(&dol, &oinfo->sarea, oinfo->color);
}

static int dma2d_blit(FAR struct fb_vtable_s *vtable, FAR const struct fb_overlayblit_s *blit)
{
    struct dma2d_layer_s *dma2d = dma2ddev();
    struct dma2d_overlay_s dol;
    struct dma2d_overlay_s sol;

    dol.oinfo = get_oinfo(blit->dest.overlay);
    dol.fmt = get_color_fmt(blit->dest.overlay);
    dol.xres = dol.oinfo->stride * 8 / dol.oinfo->bpp;
    dol.yres = dol.oinfo->fblen / dol.oinfo->stride;

    sol.oinfo = get_oinfo(blit->src.overlay);
    sol.fmt = get_color_fmt(blit->src.overlay);
    sol.xres = sol.oinfo->stride * 8 / sol.oinfo->bpp;
    sol.yres = sol.oinfo->fblen / sol.oinfo->stride;

    return dma2d->blit(&dol, &blit->dest.area, &sol, &blit->src.area);
}

static int dma2d_blend(FAR struct fb_vtable_s *vtable, FAR const struct fb_overlayblend_s *blend)
{
    struct dma2d_layer_s *dma2d = dma2ddev();
    struct dma2d_overlay_s dol;
    struct dma2d_overlay_s fol;
    struct dma2d_overlay_s bol;

    dol.oinfo = get_oinfo(blend->dest.overlay);
    dol.fmt = get_color_fmt(blend->dest.overlay);
    dol.transp_mode = dol.oinfo->transp.transp_mode;
    dol.xres = dol.oinfo->stride * 8 / dol.oinfo->bpp;
    dol.yres = dol.oinfo->fblen * 8 / dol.oinfo->bpp / dol.oinfo->stride;

    fol.oinfo = get_oinfo(blend->foreground.overlay);
    fol.fmt = get_color_fmt(blend->foreground.overlay);
    fol.transp_mode = fol.oinfo->transp.transp_mode;
    fol.xres = fol.oinfo->stride * 8 / fol.oinfo->bpp;
    fol.yres = fol.oinfo->fblen / fol.oinfo->stride;

    bol.oinfo = get_oinfo(blend->background.overlay);
    bol.fmt = get_color_fmt(blend->background.overlay);
    bol.transp_mode = bol.oinfo->transp.transp_mode;
    bol.xres = bol.oinfo->stride * 8 / bol.oinfo->bpp;
    bol.yres = bol.oinfo->fblen / bol.oinfo->stride;

    return dma2d->blend(&dol, &blend->dest.area, &fol, &blend->foreground.area, &bol, &blend->dest.area);
}

#ifdef CONFIG_FB_CMAP
/* The following are provided only if the video hardware supports RGB
 * color mapping
 */

static int dma2d_getcmap(FAR struct fb_vtable_s *vtable, FAR struct fb_cmap_s *cmap)
{
    struct dma2d_layer_s *dma2d = dma2ddev();
    return dma2d->getclut(cmap);
}

static int dma2d_putcmap(FAR struct fb_vtable_s *vtable, FAR const struct fb_cmap_s *cmap)
{
    struct dma2d_layer_s *dma2d = dma2ddev();
    return dma2d->setclut(cmap);
}

#endif

#ifdef CONFIG_FB_COLORDITHER
/* The following are provided only if the video hardware supports RGB
 * color dither mapping
 */
static int dma2d_putcmap(FAR struct fb_vtable_s *vtable, FAR const struct fb_cdithermap_s *cmap)
{
    struct dma2d_layer_s *dma2d = dma2ddev();
    return dma2d->setdither(cmap);
}

#endif

#endif

#define ioctl no_os_ioctl
//#define mmap    no_os_mmap

/**
 * struct fb_chardev_s lcdc_fb0fd - lcdc driver instance handle
 */
static struct fb_chardev_s lcdc_fb0fd;

/**
 *  struct fb_chardev_s fd_dma2d - dma2d driver instance handle
 */
static struct fb_chardev_s fd_dma2d;
//#endif

/**
 * struct fb_vtable_s vtable  -  dam2d driver operation interface
 *
 */
struct fb_vtable_s vtable = {
#if defined(CONFIG_DMA2D)||defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST)
    //.getoverlayinfo = dma2d_getoverlayinfo,
    .setcolor = dma2d_fillcolor,.blit = dma2d_blit,.blend = dma2d_blend,
#ifdef CONFIG_FB_CMAP
    .getcmap = dma2d_getcmap,.putcmap = dma2d_putcmap,
#endif
#endif

};

static bool is_fb0(char *dev_path)
{
    return !memcmp(dev_path, FBDEV_FB0_PATH, strlen(FBDEV_FB0_PATH));
}

static bool is_fbdma2d(char *dev_path)
{
    return !memcmp(dev_path, FBDEV_DMA2D_PATH, strlen(FBDEV_DMA2D_PATH));
}

static void fbdev_init(uint32_t * fd, char *dev_path)
{
    uint32_t fbfd = 0;

    if (is_fb0(dev_path)) {
        fbfd = (uint32_t) & lcdc_fb0fd;
    } else if (is_fbdma2d(dev_path)) {
        fd_dma2d.vtable = &vtable;
        fbfd = (uint32_t) & fd_dma2d;
        #ifndef FB_SIM
        memset(&lv_2d, 0, sizeof(lv_2d_drv_t));
        #endif
        lv_2d.fb = (struct fb_chardev_s *)fbfd;
        lv_2d.fb->vtable = &vtable;
    } else {
        fbfd = -1;
    }

    *fd = fbfd;

    if (fbfd == -1) {
        FB_ERR("Error: cannot open framebuffer device %s", dev_path);
        return;
    }
    FB_REP("The framebuffer device %s was opened successfully.\n", dev_path);
}

void fbdev_exit(int *fd)
{
#if defined(NUTTX_OS)
    close(fbfd);
#else
#endif
}

static int fb_init(uint32_t * fd, char *dev_path)
{
    struct fb_chardev_s *fb = 0;
    int ret = 0;
    fbdev_init(fd, dev_path);

    fb = (struct fb_chardev_s *)(*fd);
    FB_INFO("%s fb:%p", __func__, fb);

    if (!is_fb0(dev_path)) {
        return ret;
    }

    if (fb) {
        // Get variable screen information
        oinfo[0].overlay = 0;
        ret = ioctl((uint32_t) fb, FBIOGET_OVERLAYINFO, &oinfo[0]);
        if (ret) {
            FB_ERR("Error reading overlay information");
            return ret;
        }

        oinfo[1].overlay = 1;
        ret = ioctl((uint32_t) fb, FBIOGET_OVERLAYINFO, &oinfo[1]);
        if (ret) {
            FB_ERR("Error reading overlay information");
            return ret;
        }
        FB_INFO("The framebuffer device was mapped to memory successfully.\n");
    }

    return ret;
}

static void hw_blit(lv_2d_drv_t * disp_drv, struct fb_overlayblit_s *blit)
{
    disp_drv->fb->vtable->blit(disp_drv->fb->vtable, blit);
}

static void hw_blend(lv_2d_drv_t * disp_drv, struct fb_overlayblend_s *blend)
{
    disp_drv->fb->vtable->blend(disp_drv->fb->vtable, blend);
}

#ifdef CONFIG_DMA2D_FB_CMAP
static void hw_setclut(lv_2d_drv_t * disp_drv, struct fb_cmap_s *cmap)
{
    disp_drv->fb->vtable->putcmap(disp_drv->fb->vtable, cmap);
}
#endif

static void hw_fill_cb(lv_2d_drv_t * disp_drv, struct fb_overlayinfo_s *ol)
{
    disp_drv->fb->vtable->setcolor(disp_drv->fb->vtable, ol);
}

// argb888
#define RED 0xFFFF0000
#define YELLOW 0xFF00FF00
#define BLUE 0xFF0000FF
#define WHITE 0xFFFFFFFF
#define BLACK 0xFF000000

static uint32_t test_colors[] = { RED, YELLOW, BLUE, WHITE, BLACK };

#ifdef CONFIG_DMA2D_FB_CMAP
static void fb2d_setclut(lv_2d_drv_t * fd, struct fb_cmap_s *cmap)
{
    hw_setclut(fd, cmap);
}
#endif

/**
 * fb2d_fill_rects - dma2d fill overlay color test
 * @fd :   dma2d driver interface
 *
 */
POSSIBLY_UNUSED
static int fb2d_fill_rects(lv_2d_drv_t * fd)
{
    int ret = 0;
    int loop = sizeof(test_colors) / sizeof(uint32_t);
    loop = 1;

    uint32_t x, y;
    lv_coord_t lw = 454;
    lv_coord_t lh = 454;
    uint8_t bpp = 32;

for(int j =4; j < 454; j+=4){
    lw = j;//240;
    lh = j;//240;

    lv_coord_t lstride = lw * bpp / 8;

    uint8_t ltransp = 0xff;
    uint8_t ltransp_mode = 0;    // const alpha
    uint32_t memsize = lw * lh * bpp / 8;
    char *fbp;
    uint32_t fb;
    uint32_t *dest_buf;
    lv_coord_t dest_width = lw;//454;
    lv_area_t fill_area;
    uint32_t color;

    fbp = (char *)FB_MALLOC(memsize + 64);
    fb = (uint32_t) (fbp);
    fb = fb + (32 - fb % 32);
    dest_buf = (uint32_t *) fb;
    // struct fb_overlayinfo_s oinfo;
    fill_area.x1 = 0;
    fill_area.y1 = 0;
    fill_area.x2 = lw;//454;
    fill_area.y2 = lh;//454;

#if defined(FB_MEM_RESULT_CHECK)
    uint32_t *dest_buf_org = dest_buf + fill_area.x1;
#endif
    for (int i = 0; i < loop; i++) {
        color = test_colors[i];
        struct fb_overlayinfo_s *ol = &oinfo[0];
        struct fb_area_s area;
        area.x = fill_area.x1;
        area.y = fill_area.y1;
        area.w = fill_area.x2 - fill_area.x1;
        area.h = fill_area.y2 - fill_area.y1;
        ol->sarea = area;

        fb_overlay_alloc(ol, fb, memsize, lstride, 0, bpp, color, ltransp, ltransp_mode, 0);

        fb_overlay_cfmt_set(0, FB_FMT_RGB32);
        fb_overlay_cfmt_set(1, FB_FMT_RGB32);
        hw_fill_cb(fd, ol);

#if defined(FB_MEM_RESULT_CHECK)
        // check result
        dest_buf = dest_buf_org;
        dest_buf += dest_width * fill_area.y1;    /*Go to the first line */

        for (y = fill_area.y1; y < fill_area.y2; y++) {
            for (x = fill_area.x1; x < fill_area.x2; x++) {
                if (dest_buf[x] != (0xffffff & color)) {
                    FB_ERR("%s test fail l:%d dest_buf[%d] %p ", __func__, y, x, dest_buf);
                    FB_FREE(fbp);
                    return -1;
                }
            }
            dest_buf += dest_width;    /*Go to the next line */
        }
#endif
    }
    TRACE(3,"%s o fbp:%p", __func__, fbp);

    FB_FREE(fbp);
    TRACE(3,"%s o1 fbp:%p", __func__, fbp);
 }//loop test
    FB_REP("%s test OK", __func__);
    return ret;
}

/**
 * Opacity percentages.
 */
enum {
    LV_OPA_TRANSP = 0,
    LV_OPA_0 = 0,
    LV_OPA_10 = 25,
    LV_OPA_20 = 51,
    LV_OPA_30 = 76,
    LV_OPA_40 = 102,
    LV_OPA_50 = 127,
    LV_OPA_60 = 153,
    LV_OPA_70 = 178,
    LV_OPA_80 = 204,
    LV_OPA_90 = 229,
    LV_OPA_100 = 255,
    LV_OPA_COVER = 255,
};

static lv_opa_t test_opa[] = {
    LV_OPA_TRANSP, LV_OPA_0, LV_OPA_10, LV_OPA_20, LV_OPA_30, LV_OPA_40,
    LV_OPA_50,
    LV_OPA_60, LV_OPA_70, LV_OPA_80, LV_OPA_90, LV_OPA_100, LV_OPA_COVER,
};

static void fb_overlay_clear(char *saddr, uint32_t argb888, uint32_t pixes)
{

    uint32_t *mem = (uint32_t *) saddr;
    for (int i = 0; i < pixes; i++) {
        *(mem + i) = argb888;
    }
  //  FB_REP("%s ", __func__);
}

static void fb_overlay_clear_cmap(char *saddr, uint8_t cidx, uint32_t pixes)
{
    uint8_t *mem = (uint8_t *) saddr;
    for (int i = 0; i < pixes; i++) {
        *(mem + i) = cidx;
    }
}

static uint32_t dma2d_memaddress(struct fb_overlayinfo_s *ol, uint32_t xpos, uint32_t ypos)
{
    uint32_t offset;

    offset = xpos * (ol->bpp / 8) + ol->stride * ypos;

    return ((uint32_t) ol->fbmem) + offset;
}

#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST

static lv_2d_drv_t * fd_psram_test;
static struct fb_overlayblit_s *blit_psram_test;
void dma2d_psram_loop_test(void)
{
    hw_blit(fd_psram_test, blit_psram_test);
}
extern int hal_lcdc_spu_framedone_irq_enable(bool ena);
extern int lcdc_update(void);
static volatile uint32_t * psram_test_addr = NULL;
static uint32_t  psram_test_lw = 454;
static uint32_t  psram_test_lh = 454;
void gra_psram_test(void)
{
    uint32_t count = psram_test_lw * psram_test_lh;

    while(psram_test_addr == NULL){
     ;
    }

    hal_sys_timer_delay_us(200000);

    while(1)
    {
     TRACE(3, "%s", __func__);

     dma2d_psram_loop_test();
      hal_lcdc_spu_framedone_irq_enable(false);
      lcdc_update();

     memset(psram_test_addr, 0x5a,count*4);
    for (int i  = 0; i < count; i++){
        if(*(psram_test_addr + i) != 0x5a5a5a5a){

            while(1){
                TRACE(3, "%s fail addr: %p ", __func__, (psram_test_addr + i));
                TRACE(3, "%s fail 0x5a5a5a5a: 0x%8x ", __func__, *(psram_test_addr + i));
                while(1);
            }
        }
    }
    hal_sys_timer_delay_us(200000);

    }

}
#endif

/**
 * fb2d_blit
 * @fd   :    dma2d driver interface
 * @cmap :    cmap enable/disable
 *
 *   Copy image from a source overlay (defined by sarea) to destination
 *   overlay position (defined by destxpos and destypos).
 */
POSSIBLY_UNUSED
static int fb2d_blit(lv_2d_drv_t * fd, bool cmap)
{
    int ret = 0;
    struct fb_overlayblit_s blit;
    struct fb_area_s sarea;
    struct fb_overlayinfo_s *ol = &oinfo[0];
    lv_coord_t lw = 454;//480;//360;//480;
    lv_coord_t lh = 454;//480;//360;//480;
    uint8_t bpp = 32;

for(int j =4; j < 454; j+=4){

#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
#else
    lw = j;//240;
    lh = j;//240;
#endif
    lv_coord_t lstride = lw * bpp / 8;
    uint8_t ltransp = 0xff;
    uint8_t ltransp_mode = 0;    // const alpha
    uint32_t memsize = lw * lh * bpp / 8;

    char *fb0p = (char *)FB_MALLOC(memsize + 64 );
 //   char *fb0p = (char *)SYS_RAM_BASE;
#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
    if(psram_test_addr == NULL){
          psram_test_addr = FB_MALLOC(memsize + 64);
          //psram_test_addr = (uint32_t *)SYS_RAM_BASE;
    }
    char *fb1p = fb0p;
    TRACE(3,"%s psram_test_addr:%p", __func__, psram_test_addr);
    TRACE(3,"%s psram_fb:%p", __func__, fb0p);
    TRACE(3,"%s psram_wb:%p", __func__, fb1p);
#else
    char *fb1p = (char *)FB_MALLOC(memsize + 64 );
  //  char *fb1p = (char *)SYS_RAM_BASE;

#endif
    uint32_t fb0 = (uint32_t) fb0p;
    uint32_t fb1 = (uint32_t) fb1p;
    fb0 = fb0 + (32 - fb0 % 32);
    fb1 = fb1 + (32 - fb1 % 32);

    if (fb0p == 0 || fb1p == 0) {
        ASSERT(0, "%s fb0: %p fb1: %p", __func__, fb0p, fb1p);
    }

    sarea.w = lw;//454;//320;
    sarea.h = lh;//454;//320;
    sarea.x = 0;        // 0;
    sarea.y = 0;        // 0 ;

    // setup source layer
    ol = &oinfo[0];
    ol->sarea = sarea;
    if (cmap) {        // cmap test
        uint8_t cidx = 15;
        fb_overlay_alloc(ol, fb0, memsize, lstride, 0, 8, cidx, ltransp, ltransp_mode, 0);

        fb_overlay_cfmt_set(0, 6);

        // clear source with color index
        fb_overlay_clear_cmap((char *)fb0, cidx, lw * lh);
    } else {
        fb_overlay_alloc(ol, fb0, memsize, lstride, 0, 32, 0, ltransp, ltransp_mode, 0);

        fb_overlay_cfmt_set(0, FB_FMT_RGB32);
        #ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
        #else
        // clear source with RED
        fb_overlay_clear((char *)fb0, 0xFFDDCCBB, lw * lh);
        #endif
    }
    // setup dest layer
    ol = &oinfo[1];
    ol->sarea = sarea;
    fb_overlay_alloc(ol, fb1, memsize, lstride, 1, 32, 0, ltransp, ltransp_mode, 0);

    fb_overlay_cfmt_set(1, FB_FMT_RGB32);
    #ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
    #else
    fb_overlay_clear((char *)fb1, 0x0, lw * lh);
    #endif
    blit.dest.overlay = 1;
    blit.dest.area = sarea;

    blit.src.overlay = 0;
    blit.src.area = sarea;

#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
{
   fd_psram_test = fd;
   blit_psram_test = &blit;
   gra_psram_test();
}
#else
        TRACE(3,"%s sfb:%p l:0x%x", __func__,oinfo[0].fbmem, oinfo[0].fblen );

    hw_blit(fd, &blit);
#endif
#if defined(FB_MEM_RESULT_CHECK)

    char *src = (char *)dma2d_memaddress(&oinfo[0], blit.src.area.x,
                         blit.src.area.y);
    char *dest = (char *)dma2d_memaddress(&oinfo[1], blit.dest.area.x,
                          blit.dest.area.y);
    uint8_t sbpp = oinfo[0].bpp;
    // uint8_t dbpp = oinfo[1].bpp;
    //TRACE(3,"s xxxx - d xxxx \r " );
    uint16_t copy_w = sarea.w;
    //TRACE(3,"s xxxx - d xxxx \r " );

    for (int i = 0; i < sarea.h; i++) {
        if (cfmt[0] == cfmt[1]) {
//            if (memcmp(src, dest, sarea.w * sbpp / 8)) {
                //FB_ERR("s xxxx - d xxxx \r " );
                for (int c =0; c < sarea.w * sbpp / 8; c++){
                    if ((0xff & src[c]) != (0xff & dest[c])){
                        TRACE(3,"%s test fail line:%d", __func__,i);
                        TRACE(3,"s 0x%x - d 0x%x \r ",0xff & src[c], 0xff & dest[c] );
                        {
                           volatile uint32_t v = 1;
                           //volatile uint8_t  pixbs = 0xff & src[c];
                           //volatile uint8_t  pixbd = 0xff & dest[c];
                           while(v);
                        }
                        ret = -1;
                        goto __exit;

                    }
                 }

            src += oinfo[0].stride;
            dest += oinfo[1].stride;
        } else if (cfmt[0] == 6 && cfmt[1] == FB_FMT_RGB32) {
            uint32_t *dest_color = (uint32_t *) dest;
            for (int c = 0; c < copy_w; c++) {
                if (*(dest_color + c) != 0x000f0f0f) {
                    FB_ERR("%s test fail c:%d dest:%p v: 0x%x", __func__, c, dest, *(dest_color + c));
                    ret = -1;
                    goto __exit;
                }
            }
            src += oinfo[0].stride;
            dest += oinfo[1].stride;
        }
    }

#endif

__exit:

    FB_FREE(fb0p);
    FB_FREE(fb1p);
}
    if (ret){
       FB_REP("%s with cmap:%d test fail", __func__, cmap);
    }else
        FB_REP("%s with cmap:%d test OK", __func__, cmap);

    return ret;
}

/**
 * fb2d_blend   -  dma2d blend test
 * @fd     :       dma2d driver interface
 * @const_alpha   : true/false
 *
 * FB_CONST_ALPHA mode:
 * dest(opa) = dest(opa) + src(0xff-opa)
 * FB_PIXEL_ALPHA mode:
 * dest(pix_opa) = dest(pix_opa) + src(0xff-pix_opa)
 */
POSSIBLY_UNUSED
static int fb2d_blend(lv_2d_drv_t * fd, bool const_alpha)
{
    int ret = 0;
    int loop = sizeof(test_opa) / sizeof(lv_opa_t);
    loop = 1;
    lv_opa_t opa;
    uint8_t alpha_mode = 0;

    struct fb_overlayinfo_s *ol;
    lv_coord_t lw = 454;//240;
    lv_coord_t lh = 454;//240;

    for(int j =4; j < 454; j+=4){
    lw = j;//240;
    lh = j;//240;
  //  lv_coord_t lh = 100;//240;
    uint8_t bpp = 32;
    lv_coord_t lstride = lw * bpp / 8;
    uint32_t memsize = lw * lh * bpp / 8;

    char *fb0p = (char *)FB_MALLOC(memsize + 64);
   // char *fb0p = (char *)0x20400000;
    char *fb1p = (char *)FB_MALLOC(memsize + 64);
    uint32_t fb0 = (uint32_t) fb0p;
    uint32_t fb1 = (uint32_t) fb1p;
    struct fb_overlayblend_s blend;
    struct fb_area_s sarea;

    fb0 = fb0 + (32 - fb0 % 32);
    fb1 = fb1 + (32 - fb1 % 32);

#if BLEND_AREA_TEST

    sarea.w = 10;
    sarea.h = 2;
    sarea.x = 2;
    sarea.y = 2;
#else
    sarea.w = lw;//454;//240;
    sarea.h = lh;//454;//240;
//    sarea.h = 100;//240;
    sarea.x = 0;
    sarea.y = 0;

#endif

    if (const_alpha) {
        alpha_mode = FB_CONST_ALPHA;
    } else {
        alpha_mode = FB_PIXEL_ALPHA;
    }

//while(1){

    for (int i = 0; i < loop; i++) {
        opa = test_opa[i];

        // setup source layer
        ol = &oinfo[0];
        ol->sarea = sarea;
        fb_overlay_alloc(ol, fb0, memsize, lstride, 0, bpp, 0, 0xff - opa, alpha_mode, 0);

        fb_overlay_cfmt_set(0, FB_FMT_RGB32);
        fb_overlay_clear((char *)fb0, 0xffDD00BB, lw * lh);

        // setup dest layer
        ol = &oinfo[1];
        ol->sarea = sarea;
        fb_overlay_alloc(ol, fb1, memsize, lstride, 1, bpp, 0, opa, alpha_mode, 0);
        fb_overlay_cfmt_set(1, FB_FMT_RGB32);
        fb_overlay_clear((char *)fb1, 0xFFDDCCBB, lw * lh);

        blend.dest.area = sarea;
        blend.dest.overlay = 1;

        blend.foreground.area = sarea;
        blend.foreground.overlay = 0;

        blend.background.area = sarea;
        blend.background.overlay = 1;
        hw_blend(fd, &blend);
       // hal_sys_timer_delay_us(5000000);
#if defined(FB_MEM_RESULT_CHECK)
        char *src = (char *)dma2d_memaddress(&oinfo[0], blend.foreground.area.x,
                             blend.foreground.area.y);
        char *dest = (char *)dma2d_memaddress(&oinfo[1], blend.dest.area.x,
                              blend.dest.area.y);

        uint16_t copy_w = sarea.w;
        int cal_gcolor = (LV_MATH_UDIV255(0xcc * opa));
        int rel_gcolor = 0;
        uint8_t rel_alpha = 0;
        uint32_t cal_color = opa << 24 | 0x00dd00bb | cal_gcolor << 8;
        for (int i = 0; i < sarea.h; i++) {
            if (cfmt[0] == cfmt[1]) {
                uint32_t *dest_color = (uint32_t *) dest;
                for (int c = 0; c < copy_w; c++) {
                    rel_gcolor = ((*(dest_color + c)) & 0x0000ff00)
                        >> 8;
                    rel_alpha = ((*(dest_color + c)) & 0xff000000)
                        >> 24;
                    if (alpha_mode == FB_CONST_ALPHA) {
                        if ((opa != rel_alpha)
                            || (LV_MATH_ABS((rel_gcolor - cal_gcolor)) > 2)) {
                            FB_ERR("%s test fail color:0x%x != 0x%x ", __func__, *(dest_color + c), cal_color);
                            ret = -1;
                            goto __exit;
                        }
                    } else {
                        cal_gcolor = (LV_MATH_UDIV255(0xcc * 0xff));
                        if ((0xff != rel_alpha)
                            || (LV_MATH_ABS((rel_gcolor - cal_gcolor)) > 2)) {
                            FB_ERR("%s test fail color:0x%x != 0x%x ", __func__, *(dest_color + c), cal_color);
                            ret = -1;
                            goto __exit;
                        }
                    }
                }

                src += oinfo[0].stride;
                dest += oinfo[1].stride;
            }
        }
#endif
    }
//}
__exit:
    FB_FREE(fb0p);
    FB_FREE(fb1p);
    } //loop test
    FB_REP("%s alpha mode:%d test OK", __func__, alpha_mode);
    return ret;
}

static void fb_wait_log_out(void)
{
  //  hal_sys_timer_delay_us(5000);
}

static int check_blit_result(struct fb_overlayblit_s *blit, struct fb_overlayinfo_s *soinfo, struct fb_overlayinfo_s *doinfo)
{
    char *src = (char *)dma2d_memaddress(soinfo, blit->src.area.x,
                         blit->src.area.y);
    char *dest = (char *)dma2d_memaddress(doinfo, blit->dest.area.x,
                          blit->dest.area.y);

    uint8_t sbpp = soinfo->bpp;
    uint16_t copy_w = blit->dest.area.w;
    int lines = blit->dest.area.h;
    FB_INFO("%s src:%p dest:%p", __func__, src, dest);
    FB_INFO("lines:%d stride:%d copy_w:%d sbpp:%d", lines, soinfo->stride, copy_w, sbpp);

    for (int i = 0; i < lines; i++) {
        if (soinfo->bpp == doinfo->bpp) {    //
            if (memcmp(src, dest, copy_w * sbpp / 8)) {
                FB_ERR("%s test fail ", __func__);
                uint32_t *cv = (uint32_t *) src;
                FB_ERR("%s %d src color:0x%x ", __func__, i, *cv);
                cv = (uint32_t *) dest;
                FB_ERR("%s %d dest color:0x%x ", __func__, i, *cv);
                return -1;
            }
        } else if (soinfo->bpp == 16 && doinfo->bpp == 32) {
            FB_ERR("%s blit test dest bpp:%d src bpp: 0%d", __func__, doinfo->bpp, soinfo->bpp);
#if 1
            uint32_t *dest_color = (uint32_t *) dest;
            for (int c = 0; c < copy_w; c++) {
                if (*(dest_color + c) != 0x000f0f0f) {
                    FB_ERR("%s test fail c:%d dest:%p v: 0x%x", __func__, c, dest, *(dest_color + c));
                     return -1;
                }

            }

#endif
        } else {
            FB_ERR("%s blit test dest bpp:%d src bpp: 0%d", __func__, doinfo->bpp, soinfo->bpp);
            return -1;
        }
        src += soinfo->stride;
        dest += doinfo->stride;

    }
        FB_REP("%s pass", __func__);
        return 0;

}

extern uint32_t *lcdc_get_wb(void);

static int check_outlayer_result(char *wbptr, struct fb_overlayinfo_s *oinfo, uint32_t color)
{
    #ifdef WB_TEST
    uint32_t *wbp = 0;
    uint32_t rcolor = 0;
    uint32_t ecolor = color & 0xffffff;
    int dr, dg, db;
    int count = oinfo->fblen * 8 / oinfo->bpp;
    if (wbptr == 0) {
        wbp = lcdc_get_wb();
    }
    for (int i = 0; i < count; i++) {
        rcolor = (((uint32_t *) wbp)[i] & 0xffffff);
        dr = LV_MATH_ABS(((int)(rcolor & 0xff) - (int)(ecolor & 0xff)));
        dg = LV_MATH_ABS(((int)((rcolor & 0xff00) >> 8) - (int)((ecolor & 0xff00) >> 8)));
        db = LV_MATH_ABS(((int)((rcolor & 0xff0000) >> 16) - (int)((ecolor & 0xff0000) >> 16)));

        if ((dr > 2) || (dg > 2) || (db > 2)) {
            FB_REP("%s fail wbp[%d] color: %x  != %x", __func__, i, ((uint32_t *) wbp)[i], color);

            return -1;
        }
    }

    FB_REP("%s pass  wbp:%p", __func__, wbp);
    return 0;
    #else
    return 0;
    #endif
}

POSSIBLY_UNUSED static int check_outlayer_update_result(char *wbptr, struct fb_overlayinfo_s *oinfo, struct fb_area_s *area, uint32_t color)
{
    #ifdef WB_TEST
    char *src = (char *)dma2d_memaddress(oinfo, area->x, area->y);

    char *dest;
    char *wbp;

    uint8_t sbpp = oinfo->bpp;
    uint16_t copy_w = area->w;
    int lines = area->h;

    if (wbptr == 0) {
        wbp = (char *)lcdc_get_wb();
    }

    dest = wbp + area->x * (sbpp / 8) + oinfo->stride * area->y;

    FB_INFO("%s src:%p dest:%p", __func__, src, dest);

    for (int i = 0; i < lines; i++) {
        if (memcmp(src, dest, copy_w * sbpp / 8)) {
            FB_ERR("%s test fail ", __func__);
            uint32_t *cv = (uint32_t *) src;
            FB_ERR("%s %d src color:0x%x ", __func__, i, *cv);
            cv = (uint32_t *) dest;
            FB_ERR("%s %d dest color:0x%x ", __func__, i, *cv);
            //while(1);
            return -1;
        }

        src += oinfo->stride;
        dest += oinfo->stride;
    }
    FB_REP("%s pass", __func__);
    return 0;
    #else
    return 0;
    #endif
}

#ifndef CONFIG_FB_PANDISPLAY
static int check_outlayer_result_blend(char *wbptr, struct fb_overlayinfo_s *oinfo, uint32_t color)
{
    #ifdef WB_TEST

    int ret = 0;
    uint32_t *wbp = 0;
    uint32_t rcolor = 0;
    uint32_t ecolor = color & 0xffffff;
    uint8_t dr, dg, db;
    int count = oinfo->fblen * 8 / oinfo->bpp;
    if (wbptr == 0) {
        wbp = lcdc_get_wb();
    }
    for (int i = 0; i < count; i++) {
        rcolor = (((uint32_t *) wbp)[i] & 0xffffff);
        dr = LV_MATH_ABS(((int)(rcolor & 0xff) - (ecolor & 0xff)));
        dg = LV_MATH_ABS(((int)((rcolor & 0xff00) >> 8) - (int)((ecolor & 0xff00) >> 8)));
        db = LV_MATH_ABS(((int)((rcolor & 0xff0000) >> 16) - (int)((ecolor & 0xff0000) >> 16)));

        if ((dr > 2) || (dg > 2) || (db > 2)) {
            FB_REP("%s fail wbp[%d] color: %x  != %x", __func__, i, ((uint32_t *) wbp)[i], color);
            ret = -1;
            break;
        }
    }

    //for (int i = 0; i < count; i++) {
        //   ((uint32_t *)wbp)[i] = 0;
    //}

    FB_REP("%s pass  wbp:%p", __func__, wbp);
    return ret;
    #else
    return 0;
    #endif
}
#endif

extern struct fb_overlayinfo_s *lcdc_get_oinfo(int lid);

static int check_outlayer_result_blend_area(char *wbptr, struct fb_overlayblend_s *blend, uint32_t color)
{                // wb color fmt bpp32
#ifndef WB_TEST
    return 0;
#endif

    int ret = 0;
    uint32_t *wbp = 0;
    uint32_t rcolor = 0;
    uint32_t ecolor = color & 0xffffff;
    uint8_t dr, dg, db;
    struct fb_overlayinfo_s *oinfo;
    struct fb_area_s *area;

    oinfo = lcdc_get_oinfo(blend->dest.overlay);
    area = &blend->dest.area;

    int count = area->w;
    int lines = area->h;
    if (wbptr == 0) {
        wbp = lcdc_get_wb();
    }

    wbp = (uint32_t *) ((uint32_t) wbp + area->x * (oinfo->bpp / 8) + oinfo->stride * area->y);

    for (int l = 0; l < lines; l++) {
        for (int i = 0; i < count; i++) {
            rcolor = (((uint32_t *) wbp)[i] & 0xffffff);
            dr = LV_MATH_ABS(((int)(rcolor & 0xff) - (int)(ecolor & 0xff)));
            dg = LV_MATH_ABS(((int)((rcolor & 0xff00) >> 8)
                      - (int)((ecolor & 0xff00) >> 8)));
            db = LV_MATH_ABS(((int)
                      ((rcolor & 0xff0000) >> 16) - (int)((ecolor & 0xff0000) >> 16)));

            if ((dr > 2) || (dg > 2) || (db > 2)) {
                FB_REP("%s fail wbp[%d] color: %x  != %x", __func__, i, ((uint32_t *) wbp)[i], color);
                return -1;
            }
        }

        wbp += oinfo->stride / ((oinfo->bpp / 8));
    }

    for (int i = 0; i < count; i++) {
        //   ((uint32_t *)wbp)[i] = 0;
    }

    FB_REP("%s pass  wbp:%p", __func__, wbp);
    return ret;
}

POSSIBLY_UNUSED
static int check_overlay_color(struct fb_overlayinfo_s *oinfo, uint32_t color)
{
    #ifdef WB_TEST

    int ret = 0;
    int count = oinfo->fblen * 8 / oinfo->bpp;
    volatile uint32_t val;

    for (int i = 0; i < count; i++) {
         val = (((uint32_t *) oinfo->fbmem)[i] & 0xffffff);
        if (val != color) {
            FB_REP("%s fail pcount:%d fbp:%p oinfo->fbmem[%d] color: %x  != %x", __func__, count,oinfo->fbmem,i,((uint32_t *) oinfo->fbmem)[i], color);
            //TRACE(3, "%s",__func__);
            //while(1);
            ret = -1;
             break;
        }
    }

    FB_REP("%s pass  fbmem:%p", __func__, oinfo->fbmem);
    return ret;
    #else
    return 0;
    #endif
}

#ifdef CONFIG_FB_COLORDITHER
static int check_overlay_color_565(struct fb_overlayinfo_s *oinfo, uint16_t color)
{
    #ifdef WB_TEST
    int ret = 0;
    int count = oinfo->fblen * 8 / oinfo->bpp;

    for (int i = 0; i < count; i++) {
        if ((((uint16_t *) oinfo->fbmem)[i] & 0xffff) != color) {
            FB_REP("fail [%d] 0x%x  != 0x%x", i, ((uint16_t *) oinfo->fbmem)[i], color);
            ret = -1;
            break;
        }
    }

    FB_REP("%s pass  fbmem:%p", __func__, oinfo->fbmem);
    return ret;
    #else
    return 0;
    #endif
}
#endif

#ifdef CONFIG_DMA2D_FB_CMAP
static uint8_t clut_table[256 * 3];
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static uint8_t cadjcurves_table[256 * 3];
#endif

#ifdef CONFIG_FB_COLORDITHER
static uint32_t cdither_table[4];
#endif

#ifdef CONFIG_FB_COLORDITHER

#define RAND_MAX 2147483647
static uint32_t seed = 1;
static uint32_t rand()
{
    // Based on Knuth "The Art of Computer Programming"
    seed = seed * 1103515245 + 12345;
    return ((uint32_t) (seed / 65536) % (RAND_MAX + 1));
}
#endif

int fb_register(int display, int plane);
extern int lcdc_update(void);

uint8_t yuv_layer[] = {
    0x80,
    0x0,
    0x80,
    0xFF,
};

#ifdef CONFIG_FB_COLORADJCURVE
static int lcdc_set_cadjmap(void)
{
    int ret = 0;
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

    ret = ioctl(fb0fd, FBIOPUT_CADJMAP, &cadjmap);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    return ret;
}
#endif

#ifdef CONFIG_FB_COLORDITHER
static int lcdc_set_cdithermap(void)
{
    int ret = 0;

    struct fb_cdithermap_s cdithermap;
    cdithermap.first = 0;
    cdithermap.len = 4;
    cdithermap.data = &cdither_table[0];
    for (int n = cdithermap.first; n < cdithermap.len; n++) {
        cdithermap.data[n] = rand();
        cdithermap.data[n] = 0xffffffff;
    }
    ret = ioctl(fb0fd, FBIOPUT_CDITHER, &cdithermap);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    return ret;
}
#endif // CONFIG_FB_COLORDITHER

static int lcdc_setarea_test(struct fb_overlayinfo_s *oinfo)
{
    int ret = 0;
#ifdef LCDC_SETAREA_TEST
    FB_REP("lcdc set layer area ");
    oinfo->overlay = 1;
    oinfo->sarea.x = 0;
    oinfo->sarea.y = 0;
    oinfo->sarea.w = oinfo->stride * 8 / oinfo->bpp;
    oinfo->sarea.h = oinfo->fblen / oinfo->stride;
    ret = ioctl(fb0fd, FBIOSET_AREA, &oinfo);

    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    return ret;
}

#ifdef CONFIG_FB_UPDATE
POSSIBLY_UNUSED
static int lcdc_update_rect_test(struct fb_overlayinfo_s *oinfo, uint32_t ecolor)
{
    int ret = 0;
    struct fb_area_s uarea;
    uarea.x = 0;
    uarea.y = 0;
    uarea.w = 10;
    uarea.h = 1;

    ret = ioctl(fb0fd, FBIO_UPDATE, &uarea);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#ifdef CONFIG_FG_YUV_FMT
    // check_outlayer_result(0,&foinfo, 0xffba00 );
#else
    ret = check_outlayer_update_result(0, oinfo, &uarea, ecolor);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    uarea.x = 0;
    uarea.y = 0;
    uarea.w = 120;
    uarea.h = 120;

    ret = ioctl(fb0fd, FBIO_UPDATE, &uarea);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#ifdef CONFIG_FG_YUV_FMT
// check_outlayer_result(0,&foinfo, 0xffba00 );
#else
    ret = check_outlayer_update_result(0, oinfo, &uarea, ecolor);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    return ret;
}
#endif

static int lcdc_video_mirror_test(struct fb_overlayinfo_s *foinfo, struct fb_overlayinfo_s *boinfo, struct fb_var_screeninfo *var)
{
    int ret = 0;

#ifndef WB_TEST
    return 0;
#endif

#ifdef LCDC_MIRROR_TEST
    // vertical mirro test
    uint32_t *pixs;
    uint16_t lines, count;
    pixs = (uint32_t *) foinfo->fbmem;
    lines = foinfo->fblen / foinfo->stride;
    count = foinfo->stride * 8 / foinfo->bpp;
    FB_REP("lines:%d count:%d", lines, count);
    for (int i = 0; i < lines; i++) {
        for (int c = 0; c < count; c++) {
            pixs[c] = i;
        }
        pixs += count;
    }
    var->yoffset = 0;
    var->xoffset = 0;
    var->rotate = 180;
    ret = ioctl(fb0fd, FBIOSET_VMIRRO, var);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    fb_wait_log_out();

    ret = ioctl(fb0fd, FBIOPAN_DISPLAY, var);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    // check mirro result
    pixs = lcdc_get_wb();
    lines = foinfo->fblen / foinfo->stride;
    count = foinfo->stride * 8 / foinfo->bpp;
    for (int i = 0; i < lines; i++) {
        for (int c = 0; c < count; c++) {
            if (pixs[c] != (lines - i - 1)) {
                FB_REP("check mirro test fail c:%d i:%d", c, i);
                return ret;
            }
        }
        pixs += count;
    }

    FB_REP("mirro test pass");

    // disable vmirro
    var->yoffset = 0;
    var->xoffset = 0;
    var->rotate = 0;
    ret = ioctl(fb0fd, FBIOSET_VMIRRO, var);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    return ret;
}

static int lcdc_blend_test(struct fb_overlayinfo_s *foinfo, struct fb_overlayinfo_s *boinfo, uint32_t ecolor)
{
    int ret = 0;
#ifndef CONFIG_FB_PANDISPLAY
    FB_REP("fb0 fb1 blending display");
    fb_wait_log_out();

    ret = ioctl(fb0fd, FBIOSET_TRANSP, foinfo);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    lcdc_update();
    fb_wait_log_out();

    ret = check_outlayer_result_blend(0, boinfo, ecolor);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif // ndef CONFIG_FB_PANDISPLAY
    return ret;
}

static int lcdc_blend_scale_test(uint32_t fb0fd, struct fb_overlayinfo_s *foinfo, struct fb_overlayinfo_s *boinfo, uint32_t ecolor)
{
    FB_REP("dma2d blend  fb0 to fb1 with scale to wb ");
    int ret = 0;

    foinfo->transp.transp = 0x7f;
    foinfo->transp.transp_mode = FB_CONST_ALPHA;
    ret = ioctl(fb0fd, FBIOSET_TRANSP, foinfo);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    boinfo->transp.transp = 0xff - 0x7f;
    boinfo->transp.transp_mode = FB_CONST_ALPHA;
    ret = ioctl(fb0fd, FBIOSET_TRANSP, boinfo);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    struct fb_overlayblend_s blend;
    struct fb_area_s area;
#if 1
    area.x = 0;
    area.y = 0;
    area.w = 120;
    area.h = 120;
#else
    area.w = 10;
    area.h = 2;
    area.x = 2;
    area.y = 2;
#endif
    blend.dest.area = area;
    blend.dest.overlay = 2;
    // blend.dest.overlay = 1;
    blend.foreground.area = area;
    blend.foreground.overlay = 0;
    blend.background.area = area;
    blend.background.overlay = 1;

    ret = ioctl(fb0fd, FBIOSET_BLEND, &blend);

    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#ifdef CONFIG_FB_COLORDITHER
#else
    ret = check_outlayer_result_blend_area(0, &blend, ecolor);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    return ret;
}

static int lcdc_blit_test(struct fb_overlayinfo_s *foinfo, struct fb_overlayinfo_s *boinfo)
{
    FB_REP("lcdc dma2d copy fb0 to fb1 with scale ");
    int ret = 0;
    struct fb_overlayblit_s blit;
    blit.dest.overlay = 1;
    blit.dest.area.x = 0;
    blit.dest.area.y = 0;
    blit.dest.area.w = 3;
    blit.dest.area.h = 2;
    blit.src.overlay = 0;
    blit.src.area.x = 0;
    blit.src.area.y = 0;
    blit.src.area.w = 11;
    blit.src.area.h = 2;

    ret = ioctl(fb0fd, FBIOSET_BLIT, &blit);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    FB_REP("dma2d blit test ");

    fb_wait_log_out();

    ret = check_blit_result(&blit, foinfo, boinfo);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    return ret;
}

#if 0
static void fb_test_spram(void)
{
#ifdef PSRAM_TEST
    //enable psram 0x38000000 -
    *(volatile uint32_t *)0x400850d0 = 0xcafe00fb;

    {
        uint32_t *st = (volatile uint32_t *)0x38100000;
        for (int i = 0; i < 20; i++) {
            st[i] = 0x0;
        }

        *(volatile uint32_t *)0x38100000 = 0xa;

        for (int i = 0; i < 20; i++) {
            FB_ERR("x%x", st[i]);
        }
    }
#endif
}
#endif

#include "hal_dsi.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "../../utils/hwtimer_list/hwtimer_list.h"

extern void sleep_test_register_loop_callback(void (*cb)(void));
extern int fb_single_rect_draw_test(uint32_t fb_fd);

#define DSI_1S_FRAME_RATE                   60
#if 1
#define DSI_1S_SYS_FREQ                     HAL_CMU_FREQ_104M
#define DSI_1S_CPU_WORKING_TIME_MS          16
#else
#define DSI_1S_SYS_FREQ                     HAL_CMU_FREQ_208M
#define DSI_1S_CPU_WORKING_TIME_MS          12
#endif

enum FB_DSI_1S_TIMER_EVENT_T {
    FB_DSI_1S_DRAW_START,
};

enum FB_DSI_1S_STATE_T {
    FB_DSI_1S_STATE_LCDC_DONE   = (1 << 0),
    FB_DSI_1S_STATE_GOT_TE      = (1 << 1),
};

enum FB_DSI_1S_STATE_T dsi_1s_state;
static bool dsi_1s_lcdc_done;
static bool dsi_1s_draw_start;
static bool dsi_1s_got_te;
static uint8_t dsi_1s_te_cnt;
static uint32_t dsi_1s_te_time;
static uint32_t dsi_1s_draw_time;
static HWTIMER_ID dsi_1s_timer;

static void fb_dsi_1s_timer_cb(void *param)
{
    enum FB_DSI_1S_TIMER_EVENT_T evt = (enum FB_DSI_1S_TIMER_EVENT_T)param;
    TRACE(0, "%s: evt=%d", __func__, evt);

    if (evt == FB_DSI_1S_DRAW_START) {
        dsi_1s_draw_start = true;
    } else {
        ASSERT(false, "%s: Bad evt=%d", __func__, evt);
    }
    hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);
}

static void fb_dsi_1s_lcdc_irq_cb(int error)
{
    TRACE(0, "%s: error=%d", __func__, error);
    dsi_1s_lcdc_done = true;
    hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);
}

static void fb_dsi_1s_te_gpio_irq(int error)
{
    dsi_1s_te_cnt++;
    if (dsi_1s_te_cnt >= DSI_1S_FRAME_RATE) {
        dsi_1s_te_time = hal_sys_timer_get();
        dsi_1s_got_te = true;
        hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);

        TRACE(0, "Got %u te irq", dsi_1s_te_cnt);
        dsi_1s_te_cnt = 0;
    }
}

static void fb_dsi_1s_loop(void)
{
    uint32_t lock;
    bool lcdc_done;
    bool got_te;
    bool draw_start;
    uint32_t time;
    uint32_t interval;

    lock = int_lock();
    lcdc_done = dsi_1s_lcdc_done;
    dsi_1s_lcdc_done = false;
    got_te = dsi_1s_got_te;
    dsi_1s_got_te = false;
    draw_start = dsi_1s_draw_start;
    dsi_1s_draw_start = false;
    hal_cpu_wake_unlock(HAL_CPU_WAKE_LOCK_USER_12);
    int_unlock(lock);

    if (lcdc_done) {
        dsi_1s_state |= FB_DSI_1S_STATE_LCDC_DONE;

        time = hal_sys_timer_get();
        interval = time - dsi_1s_draw_time;

        TRACE(0, "Draw+LCDC+DSI: %u ms", TICKS_TO_MS(interval));
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }
    if (got_te) {
        dsi_1s_state |= FB_DSI_1S_STATE_GOT_TE;
    }
    if ((dsi_1s_state & (FB_DSI_1S_STATE_LCDC_DONE | FB_DSI_1S_STATE_GOT_TE)) ==
            (FB_DSI_1S_STATE_LCDC_DONE | FB_DSI_1S_STATE_GOT_TE)) {
        uint32_t frame_ms = 1000 / DSI_1S_FRAME_RATE;
        uint32_t draw_time_ms = frame_ms - (DSI_1S_CPU_WORKING_TIME_MS % frame_ms);
        uint32_t draw_time = MS_TO_TICKS(draw_time_ms);

        dsi_1s_state = 0;
        time = hal_sys_timer_get();
        interval = time - dsi_1s_te_time;
        if (interval < draw_time) {
            interval = draw_time - interval;
            hwtimer_update_then_start(dsi_1s_timer, fb_dsi_1s_timer_cb, (void *)FB_DSI_1S_DRAW_START, interval);
        } else {
            draw_start = true;
        }
    }
    if (draw_start) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, DSI_1S_SYS_FREQ);

        time = hal_sys_timer_get();
        interval = time - dsi_1s_draw_time;
        dsi_1s_draw_time = time;
        TRACE(0, "Draw interval: %u ms", TICKS_TO_MS(interval));

        lcd_set_start_flag();
        fb_single_rect_draw_test(fb0fd);
        lcd_set_end_flag();
    }
}

int fb_dsi_1s_test(void)
{
    int ret = 0;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, DSI_1S_SYS_FREQ);
    dsi_1s_timer = hwtimer_alloc(fb_dsi_1s_timer_cb, NULL);
    ASSERT(dsi_1s_timer, "Failed to alloc hwtimer");
    dsi_1s_draw_time = hal_sys_timer_get();
    lcdc_irq_callback_register(fb_dsi_1s_lcdc_irq_cb);
    sleep_test_register_loop_callback(fb_dsi_1s_loop);
    lcdc_teirq_callback_register(fb_dsi_1s_te_gpio_irq);

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
    lcd_gpio_setup();
    lcd_set_start_flag();
    fb_single_rect_draw_test(fb0fd);
    lcd_set_end_flag();
    return ret;
}

/**
 * fb_lcdc_test -  lcdc driver test entry
 */

extern int fb_rect_draw_test(uint32_t fb_fd);

int fb_lcdc_test(void)
{
    int ret = 0;
    uint32_t ecolor;

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
#if 1
    while (1) {        //lcdc test case
        fb_rect_draw_test(fb0fd);
        hal_sys_timer_delay_us(2000 * 1000);
    }
    return ret;
#endif
        struct fb_var_screeninfo var;
        struct fb_overlayinfo_s foinfo;
        struct fb_overlayinfo_s boinfo;

        ret = ioctl(fb0fd, FBIOGET_VIDEOINFO, &var);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        FB_REP("lcdc set foreground fb0 color with green color");

        foinfo.overlay = 0;
        ret = ioctl(fb0fd, FBIOGET_OVERLAYINFO, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
        fb_wait_log_out();
#ifdef CONFIG_DMA2D

#ifdef CONFIG_FB_COLORADJCURVE
        ret = lcdc_set_cadjmap();
#endif
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#ifdef CONFIG_FB_COLORDITHER
        lcdc_set_cdithermap();
        foinfo.color = 0x1861;
#else
        foinfo.color = 0xff00ff00;
//        foinfo.color = 0xffff00ff;
#endif // CONFIG_FB_COLORDITHER

/*set layer color*/
#ifdef CONFIG_FG_YUV_FMT
        // fill fgl with "0x80, 0x0, 0x80, 0xFF"
        {
            int count = 120 * 120 / 2;

            for (int i = 0; i < count; i++) {
                ((uint32_t *) foinfo.fbmem)[i] = 0x800080ff;
            }
        }

#else

        ret = ioctl(fb0fd, FBIOSET_COLOR, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
        #ifdef CONFIG_DMA2D
            return ret;
        #endif
        }
#endif

       fb_wait_log_out();

#ifdef CONFIG_FB_COLORDITHER
        ecolor = 0x1861;
#else
        ecolor = 0x00ff00;

#endif // CONFIG_FB_COLORDITHER

#ifdef CONFIG_FB_COLORDITHER
        check_overlay_color_565(&foinfo, ecolor);
#else
        check_overlay_color(&foinfo, ecolor);
#endif


        FB_REP("lcdc set background fb1 color with red color");

        boinfo.overlay = 1;
        ret = ioctl(fb0fd, FBIOGET_OVERLAYINFO, &boinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#ifdef CONFIG_FB_COLORDITHER
        boinfo.color = 0xffff;
#else
        boinfo.color = 0xffff0000;
#endif
        ret = ioctl(fb0fd, FBIOSET_COLOR, &boinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#ifdef CONFIG_FB_COLORDITHER
        check_overlay_color_565(&boinfo, boinfo.color);
#else
        check_overlay_color(&boinfo, 0xffffff & boinfo.color);
#endif

#endif

        fb_wait_log_out();

        FB_REP("display foreground layer ");

        foinfo.transp.transp = 0x00;    // background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

/*chromakey test, fill flayer data with chromakey*/
#ifdef CONFIG_LCDC_L1_CHROMAKEYEN
        uint32_t *pixs = (uint32_t *) foinfo.fbmem;
        int count = 240;    // foinfo.fblen/foinfo.stride;
        for (int i = 0; i < count; i++) {
            pixs[i] = CONFIG_LCDC_CHROMAKEY;
        }
#endif

#ifdef CONFIG_FB_PANDISPLAY
        /*display flayer */
        var.yoffset = 0;
        var.xoffset = 0;

        ret = ioctl(fb0fd, FBIOPAN_DISPLAY, &var);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }
#ifdef CONFIG_FG_YUV_FMT
        check_outlayer_result(0, &foinfo, 0xffba00);
#else
        check_outlayer_result(0, &foinfo, 0x00ff00);
#endif

#ifdef CONFIG_FG_YUV_FMT
        ecolor = 0xff0000;
#else
        ecolor = 0xff0000;
#endif
#ifdef CONFIG_FB_UPDATE

        ret = lcdc_update_rect_test(&foinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
          //  return ret;
        }
#endif
        /*display blayer */
        var.yoffset = var.yres;
        ret = ioctl(fb0fd, FBIOPAN_DISPLAY, &var);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }


#ifdef CONFIG_FG_YUV_FMT
        check_outlayer_result(0, &foinfo, 0xff0000);
#else
        check_outlayer_result(0, &foinfo, 0xff0000);
#endif

       // fb_overlay_clear((char *)lcdc_get_wb(), 0x0, 120 * 120);

#ifdef CONFIG_FG_YUV_FMT
        ecolor = 0xff00;
#else
        ecolor = 0xff00;
#endif
#ifdef CONFIG_FB_UPDATE
        ret = lcdc_update_rect_test(&boinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
          //  return ret;
        }
#endif
#else

        /*display flayer only */
        foinfo.transp.transp = 0x00;    // background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;    // FB_PIXEL_ALPHA;//FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }

       // fb_overlay_clear((char *)lcdc_get_wb(), 0x0, 120 * 120);
        lcdc_update();

#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
return 0;
#endif
        fb_wait_log_out();

#ifdef CONFIG_FB_COLORDITHER
        check_outlayer_result(0, &foinfo, 0x1f0f0f);
#else
#ifdef CONFIG_FG_YUV_FMT
        check_outlayer_result(0, &foinfo, 0xffba00);
#else
        check_outlayer_result(0, &foinfo, 0x00ff00);
#endif

#endif

#endif // CONFIG_FB_PANDISPLAY

#ifdef CONFIG_LCDC_L1_CHROMAKEYEN
    #if WB_TEST
        pixs = (uint32_t *) lcdc_get_wb();
        for (int i = 0; i < count; i++) {
            if (((pixs[i]) & 0xffffff) != 0) {
                FB_ERR(" chromakey test fail %s pixs[%d]:0x%x  ", __func__, i, pixs[i]);
                return ret;
            }
        }
        FB_REP("chromakey test pass %s ", __func__);

        for (int i = 0; i < count; i++) {
            pixs[i] = 0xff00;
        }
    #endif
#else
#ifdef CONFIG_FB_COLORADJCURVE
        check_outlayer_result(0, &foinfo, 0x007f00);
        lcdc_cadjcurves_enable(false);
#else
// check_outlayer_result(0,&foinfo, 0x00ff00 );
#endif
#endif
        fb_wait_log_out();

        ret = lcdc_setarea_test(&boinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }
        FB_REP("display  background layer ");

        fb_wait_log_out();

#ifdef LCDC_BLANK_TEST
        /*display blayer only */
        foinfo.overlay = 0;
        foinfo.blank = 1;
        ret = ioctl(fb0fd, FBIOSET_BLANK, &foinfo);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        boinfo.overlay = 1;
        boinfo.blank = 0;

        ret = ioctl(fb0fd, FBIOSET_BLANK, &boinfo);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        foinfo.transp.transp = 0xff;    // 0xff;//background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#endif

        /*blend display blayer only */
        foinfo.transp.transp = 0xff;    // background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;

#ifdef CONFIG_FB_COLORDITHER
        ecolor = 0xffffff;
#else
        ecolor = 0xff0000;
#endif
        fb_wait_log_out();
        //fb_overlay_clear((char *)lcdc_get_wb(), 0x0, 120 * 120);

        ret = lcdc_blend_test(&foinfo, &boinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
        /*blend display b(a =0x7f) + f(0xff - 0x7f) layers */
        foinfo.transp.transp = 0x7f;
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
#ifdef CONFIG_FB_COLORDITHER
        ecolor = 0x8e8686;
#else
        ecolor = 0x7f7f00;
#endif

        fb_wait_log_out();
        ret = lcdc_blend_test(&foinfo, &boinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
        fb_wait_log_out();
        ret = lcdc_blend_scale_test(fb0fd, &foinfo, &boinfo, 0x7f7f00);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
        fb_wait_log_out();
        ret = lcdc_blit_test(&foinfo, &boinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
        ret = lcdc_video_mirror_test(&foinfo, &boinfo, &var);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#if defined(WITH_OS)
        munmap(fbp, screensize);
        close(fp);
#endif
        FB_REP("lcdc test pass ");
        return ret;
#if 0
    }
#endif
}


int fb_buffer_draw_test(uint32_t fb_fd,uint8_t *buffer);

int fb_lcdc_display_buffer_test(uint8_t *buffer)
{
    fb_buffer_draw_test(fb0fd,buffer);
    return 0;
}


/**
 * fb_dma2d_test -  dma2d driver test entry
 */

int fb_dma2d_test(void)
{
#if defined(LCDC_DMA2D_CPU_PSRAM_PARALL_TEST) || defined(CONFIG_DMA2D)

    int ret = 0;
    fb_heap = fb_mem_setup(fb_base, 480 * 480 * 12);

    /*create  fb_2d */
    fb_init(&fbfd_dma2d, FBDEV_DMA2D_PATH);

#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
#else
    dma2dinitialize();
#endif

#ifdef CONFIG_DMA2D_FB_CMAP
    struct fb_cmap_s cmap;
    cmap.first = 0;
    cmap.len = 256;
    cmap.blue = &clut_table[0];
    cmap.red = &clut_table[256];
    cmap.green = &clut_table[512];
    for (int n = cmap.first; n < cmap.len - 1; n++) {
        cmap.blue[n] = n;
        cmap.green[n] = n;
        cmap.red[n] = n;
    }
    fb2d_setclut(&lv_2d, &cmap);
#endif
  while(1){
  //  while(0){

#ifndef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST

    FB_REP("fb2d_fill");
    fb_wait_log_out();
    ret = fb2d_fill_rects(&lv_2d);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
   // }

    fb_wait_log_out();


    FB_ERR("blit loop test %s %d", __func__, __LINE__);
    //while(0){
    #if 1
    ret = fb2d_blit(&lv_2d, 0);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    #ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
    return 0;
    #endif

    #endif
  //  };
#if 0
    fb_wait_log_out();
    ret = fb2d_blit(&lv_2d, 1);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    fb_wait_log_out();
#endif
#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
while(0){
#else
//while(1){
#endif
    ret = fb2d_blend(&lv_2d, 1);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#ifdef LCDC_DMA2D_CPU_PSRAM_PARALL_TEST
  }
#else
//}
#endif
     #ifndef FB_LOOP_TEST
         break;
     #endif
}
    fb_wait_log_out();
    ret = fb2d_blend(&lv_2d, 0);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

#if defined(WITH_OS)
    munmap(fbp, screensize);
    close(fp);
#endif
    FB_REP("dma2d test pass ");
    return ret;

#else
    return 0;
#endif
}

int fb_gpu_test_init(void)
{
    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
    return 0;
}

POSSIBLY_UNUSED
static void psram_pll_init(void)
{
    enum HAL_CMU_PLL_T pll;
    enum HAL_CMU_PLL_USER_T user;

#if 0
#elif defined(CHIP_BEST1501) || defined(CHIP_BEST1600)
    pll = HAL_CMU_PLL_BB;
    user = HAL_CMU_PLL_USER_PSRAM;
#elif defined(CHIP_BEST2001)
    pll = HAL_CMU_PLL_BB_PSRAM;
    user = HAL_CMU_PLL_USER_PSRAM;
#else
    pll = HAL_CMU_PLL_USB;
    user = HAL_CMU_PLL_USER_SYS;
#endif
    hal_cmu_pll_enable(pll, user);
    hal_cmu_mem_select_pll(pll);
}

//#define  LCDC_TEST

int fb_graphic_test(void)
{
    int ret = 0;
    FB_REP("lcdc test ");
//    hal_cache_disable(HAL_CACHE_ID_D_CACHE);

#ifdef TEST_WITH_LCD

    ret = fb_lcdc_test();
    if (ret) {
        FB_REP("lcdc test fail !!!!!!");
        ret = -1;
    }
#endif
    ret = fb_dma2d_test();

    if (ret) {
        ret = -1;
        FB_REP("dma2d test fail !!!!!!");
    }
    return ret;
}

/**
 * fb_register - Register the framebuffer character device
 *
 * Register the framebuffer character device at /dev/fbN where N is the
 *   display number if the devices supports only a single plane.  If the
 *   hardware supports multiple color planes, then the device will be
 *   registered at /dev/fbN.M where N is the again display number but M
 *   is the display plane.
 */

int fb_register(int display, int plane)
{
    FAR struct fb_chardev_s *fb;
    struct fb_videoinfo_s vinfo;
    struct fb_planeinfo_s pinfo;
#ifdef CONFIG_FB_OVERLAY
    struct fb_overlayinfo_s oinfo;
#endif
    // char devname[16];
    // uint8_t nplanes;

    int ret;

    /* Allocate a framebuffer state instance */

#if 0
    fb = (FAR struct fb_chardev_s *)
        kmm_zalloc(sizeof(struct fb_chardev_s));
#else
    if (display == 0) {
        fb = &lcdc_fb0fd;
    } else {
        FB_ERR("%s only one framebuffer !!!!!!", __func__);
    }

#endif

    if (fb == NULL) {
        return -1;
    }

    /* Initialize the frame buffer device. */
    /*dsi interface setup*/

    ret = up_fbinitialize(display);
    if (ret < 0) {
        FB_ERR("ERROR: up_fbinitialize() failed for display %d: %d\n", display, ret);
        goto errout_with_fb;
    }

    DEBUGASSERT((unsigned)plane <= UINT8_MAX);
    fb->plane = plane;

    fb->vtable = up_fbgetvplane(display, plane);
    if (fb->vtable == NULL) {
        FB_ERR("ERROR: up_fbgetvplane() failed, vplane=%d\n", plane);
        goto errout_with_fb;
    }

    /* Initialize the frame buffer instance. */

    DEBUGASSERT(fb->vtable->getvideoinfo != NULL);
    ret = fb->vtable->getvideoinfo(fb->vtable, &vinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getvideoinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }
#ifdef WITH_OS
    uint8_t nplanes;
    nplanes = vinfo.nplanes;
    DEBUGASSERT(vinfo.nplanes > 0 && (unsigned)plane < vinfo.nplanes);
#endif

    DEBUGASSERT(fb->vtable->getplaneinfo != NULL);
    ret = fb->vtable->getplaneinfo(fb->vtable, plane, &pinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getplaneinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }

    fb->fbmem = pinfo.fbmem;
    fb->fblen = pinfo.fblen;
    fb->bpp = pinfo.bpp;

    /* Clear the framebuffer memory */

#ifndef FB_SIM
    memset(pinfo.fbmem, 0, pinfo.fblen);
#endif

#ifdef CONFIG_FB_OVERLAY
    /* Initialize first overlay but do not select */

    DEBUGASSERT(fb->vtable->getoverlayinfo != NULL);
    ret = fb->vtable->getoverlayinfo(fb->vtable, 0, &oinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getoverlayinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }

    /* Clear the overlay memory. Necessary when plane 0 and overlay 0
     * different.
     */

#ifndef FB_SIM
    memset(oinfo.fbmem, 0, oinfo.fblen);
#endif
#endif

#ifdef WITH_OS
    /* Register the framebuffer device */

    if (nplanes < 2) {
        snprintf(devname, 16, "/dev/fb%d", display);
    } else {
        snprintf(devname, 16, "/dev/fb%d.%d", display, plane);
    }

    ret = register_driver(devname, &fb_fops, 0666, (FAR void *)fb);
#endif

    if (ret < 0) {
        FB_ERR("ERROR: register_driver() failed: %d\n", ret);
        goto errout_with_fb;
    }

    return OK;

errout_with_fb:
#if 0
    kmm_free(fb);
#endif
    return ret;
}

#include "hal_lcdc.h"

#define PICTURE_BUFFER_ATT POSSIBLY_UNUSED static const

#if !defined(FPGA) && !defined(SIMU)
PICTURE_BUFFER_ATT uint8_t background_368_448[] = {
#include "display_resource/RGB888_unpacked_368x448.txt"
};
#endif

struct fb_planeinfo_s g_pinfo;
struct fb_videoinfo_s g_vinfo;

static bool lcd_frame_done = 0;
static uint32_t lcd_frame_count = 0;
static uint32_t lcd_te_count = 0;

static void fb_dsi_60hz_lcdc_irq_cb(int error)
{
    lcd_frame_done = 1;
    lcd_frame_count ++;
#if defined(CONFIG_TRIGGER_FRAME_BY_SW)
    lcdc_enter_lowpower();
#else
    static uint32_t yoffset = 1;
    if (g_pinfo.yres_virtual == g_vinfo.yres)
        yoffset = 0;
    uint8_t *row = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * yoffset;
    if (yoffset < g_vinfo.yres)
        yoffset += 1;
    else
        yoffset = 0;
    hal_lcdc_lstartaddr(0, (uint32_t)row);
    hal_lcdc_start_dsi();
#endif
}

static void ulps_exit_hander(int fb)
{
    hal_lcdc_lstartaddr(0, (uint32_t)fb);
    hal_lcdc_gen_start();
}

static void fb_dsi_60hz_te_gpio_irq(int error)
{
    if (lcd_frame_done > 0)
    {
        lcd_te_count++;
        if (lcd_te_count == 61)
        {
            FB_REP("frame rate:%d\n",lcd_frame_count);
            lcd_te_count = 0;
            lcd_frame_count = 0;
        }
#if defined(CONFIG_TRIGGER_FRAME_BY_SW)
        static uint32_t yoffset = 1;
        if (g_pinfo.yres_virtual == g_vinfo.yres)
            yoffset = 0;
        uint8_t *row = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * yoffset;
        if (yoffset < g_vinfo.yres)
            yoffset += 1;
        else
            yoffset = 0;
        lcdc_exit_lowpower_with_cb(ulps_exit_hander,row);
#endif
    }
}

int  fb_dsi_60hz_test()
{
    lcdc_irq_callback_register(fb_dsi_60hz_lcdc_irq_cb);
    lcdc_teirq_callback_register(fb_dsi_60hz_te_gpio_irq);

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);

    int ret = fb_ioctl(fb0fd, FBIOGET_VIDEOINFO,
                (unsigned long)((uintptr_t)&g_vinfo));

    FB_REP("VideoInfo:%d\n",ret);
    FB_REP("      fmt: %u\n", g_vinfo.fmt);
    FB_REP("     xres: %u\n", g_vinfo.xres);
    FB_REP("     yres: %u\n", g_vinfo.yres);
    FB_REP("  nplanes: %u\n", g_vinfo.nplanes);

    ret = fb_ioctl(fb0fd, FBIOGET_PLANEINFO,
                (unsigned long)((uintptr_t)&g_pinfo));
    FB_REP("PlaneInfo (plane 0):%d\n",ret);
    FB_REP("    fbmem: %p\n", g_pinfo.fbmem);
    FB_REP("    fblen: %lu\n", (unsigned long)g_pinfo.fblen);
    FB_REP("   stride: %u\n", g_pinfo.stride);
    FB_REP("  display: %u\n", g_pinfo.display);
    FB_REP("      bpp: %u\n", g_pinfo.bpp);

    int xoffset = (g_vinfo.xres - 368) /2;
    int yoffset = (g_vinfo.yres - 448) /2;

    memset(g_pinfo.fbmem,0,g_pinfo.fblen);
    if (xoffset >= 0 && yoffset >= 0)
    {
        for (int y = 0; y < 448; y++)
          {
            uint8_t *dist_addr = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * (yoffset+y) + (g_pinfo.bpp/8)*xoffset;
            uint8_t *src_addr = (uint8_t *)background_368_448 + 368*4*y;
            memcpy(dist_addr,src_addr,368*4);
          }
        if (g_pinfo.yres_virtual == (g_vinfo.yres * 2))
            memcpy(g_pinfo.fbmem + g_pinfo.fblen/2, g_pinfo.fbmem, g_pinfo.fblen/2);
    }
    else
    {
        memcpy(g_pinfo.fbmem, background_368_448, g_pinfo.fblen);
    }
#if defined(CONFIG_TRIGGER_FRAME_BY_SW)
    lcd_frame_done = 1;
#else
    struct fb_area_s area;
    area.x = 0;
    area.y = 0;
    area.w = g_vinfo.xres;
    area.h = g_vinfo.yres;
    ret = fb_ioctl(fb0fd, FBIO_UPDATE,
                (unsigned long)((uintptr_t)&area));
#endif
    FB_REP("fb_ioctl(FBIO_UPDATE) ret: %d\n",ret);
    return ret;
}

