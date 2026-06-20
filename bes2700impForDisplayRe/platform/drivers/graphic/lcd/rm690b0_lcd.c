/***************************************************************************
 * RM690B0 AMOLED panel driver (MiBand 9)  —  bring-up
 *
 * The MiBand 9 uses a Raydium RM690B0 (variants VXN / TM) AMOLED on MIPI-DSI in
 * command ("SPI") mode. This driver is modelled on the stock dumy_lcd.c
 * rm69330_init() (RM693xx sibling, identical Raydium command set) and plugs into
 * the SDK framebuffer framework (fb.c / lcdc.c). Enable with DISPLAY_RM690B0.
 *
 * TODO(vela): confirm against the MiBand 9 Vela rm690b0_lcd driver —
 *   - exact resolution (RM690B0_XRES/YRES, placeholder 192x490),
 *   - the column offset (hstart, RM69330 used 14),
 *   - any panel-specific manufacturer commands (gamma / VXN-vs-TM tweaks),
 *   - RST / power-rail GPIOs for the MiBand 9 board.
 * It also depends on the best1503 DSI/LCDC HAL glue (hal_display_best1503.c) +
 * DSI_BASE/LCDC_BASE being correct. Until those are verified, do not expect light.
 ***************************************************************************/
#if defined(DISPLAY_RM690B0)

#include <stdio.h>
#include "string.h"
#include "fb.h"
#include "lcdc.h"
#include "hal_dsi.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_lcdc.h"

#define LCD_INFO(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)

/* TODO(vela): MiBand 9 panel size (public spec ~1.62" 192x490). Confirm from the
 * Vela hal_dsi_init(h_res) arg + the 0x2A/0x2B window in rm690b0_lcd. */
#define RM690B0_XRES        192
#define RM690B0_YRES        490
#define RM690B0_HOFFSET     0           /* TODO(vela): column start offset           */
#define WIDTH               RM690B0_XRES

/* TODO(vela): MiBand 9 reset + power-rail GPIOs. Placeholder uses a generic pin. */
#define GPIO_RST_PIN        HAL_GPIO_PIN_P3_4   /* TODO(vela): real MiBand9 RST pad */
/* #define GPIO_1V8_PIN     HAL_GPIO_PIN_Px_x */ /* TODO(vela): panel power rails    */

/* Framebuffer lives in PSRAM (CONFIG_LCDC_FB_BASE = PSRAM_NC_BASE), NOT in a .bss
 * array — 192*490*4 = ~368 KB would overflow the 296 KB on-chip RAM. */
#define FB_PSRAM_PTR    ((uint32_t *)(uintptr_t)CONFIG_LCDC_FB_BASE)
#define FB_PSRAM_BYTES  (CONFIG_LCDC_FB_SIZE)
struct lcd_dev_s g_lcd;

/* Push one rectangle to the panel (CASET/RASET + LCDC layer), as in dumy_lcd.c. */
static int putarea(fb_coord_t row_start, fb_coord_t row_end,
                   fb_coord_t col_start, fb_coord_t col_end,
                   FAR const uint8_t *buffer)
{
    uint16_t width, heigh;
    uint32_t fbmem = (uint32_t)buffer;
    col_start &= 0xfffe; row_start &= 0xfffe;
    width  = (col_end - col_start + 2) & 0xfffe;
    heigh  = (row_end - row_start + 2) & 0xfffe;

    uint16_t hstart = RM690B0_HOFFSET + col_start;
    uint16_t hend   = hstart + width - 1;
    uint16_t vstart = row_start;
    uint16_t vend   = vstart + heigh - 1;

    hal_dsi_reset(width);
    hal_dsi_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    hal_dsi_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);

    fbmem += (row_start*WIDTH*4) + col_start*4;
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, fbmem);
    hal_lcdc_lsize(LCDC_LAYER_LFORE, width, heigh);
    hal_lcdc_lsize(LCDC_LAYER_SPU,   width, heigh);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, width, heigh);
    return 0;
}

static int lcdc_getplaneinfo(FAR struct lcd_dev_s *dev, unsigned int planeno,
                             FAR struct lcd_planeinfo_s *pinfo)
{
    (void)dev; (void)planeno;
    pinfo->putarea = putarea;
    return OK;
}

/* RM690B0 init — Raydium command set, same shape as dumy_lcd.c rm69330_init().
 * TODO(vela): replace/augment with the exact MiBand 9 sequence once extracted. */
static void rm690b0_init(void)
{
    uint16_t hstart = RM690B0_HOFFSET;
    uint16_t hend   = RM690B0_XRES - 1 + hstart;
    uint16_t vstart = 0;
    uint16_t vend   = RM690B0_YRES - 1 + vstart;

    hal_dsi_send_cmd_data(0xfe, 0x1, 0x00, 0, 0, 0);  /* manufacturer cmd page 0   */
    hal_dsi_send_cmd_data(0x35, 0x1, 0x00, 0, 0, 0);  /* TE on (vsync)             */
    hal_dsi_send_cmd_data(0x36, 0x1, 0x00, 0, 0, 0);  /* MADCTL (orientation)      */
    hal_dsi_send_cmd_data(0x3a, 0x1, 0x55, 0, 0, 0);  /* pixel format = RGB565     */
    hal_dsi_send_cmd_data(0x51, 0x1, 0xff, 0, 0, 0);  /* brightness = max          */
    hal_dsi_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    hal_dsi_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    hal_dsi_send_cmd(0x11);                            /* sleep out                 */
    hal_sys_timer_delay(MS_TO_TICKS(120));
    hal_dsi_send_cmd(0x29);                            /* display on                */
    hal_sys_timer_delay(MS_TO_TICKS(20));
}

static int lcd_initialize(void)
{
    g_lcd.getplaneinfo = lcdc_getplaneinfo;

    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {GPIO_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
#ifdef GPIO_1V8_PIN
    pinmux[0].pin = GPIO_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_1V8_PIN);
#endif
    /* reset pulse */
    pinmux[0].pin = GPIO_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(120));

    hal_dsi_init(WIDTH);            /* best1503 DSI host + PHY (hal_display_best1503.c) */
    rm690b0_init();                 /* panel DCS init                                  */
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_dsi_start();                /* start DSI output                                */
    LCD_INFO("rm690b0 init done %dx%d", RM690B0_XRES, RM690B0_YRES);
    return OK;
}

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev) { (void)lcddev; return &g_lcd; }
int  board_lcd_initialize(void) { return lcd_initialize(); }
void board_lcd_uninitialize(void) { }

int up_fbinitialize(int display)
{
    static bool initialized = false;
    int ret = OK;
    (void)display;
    if (!initialized) {
        board_lcd_initialize();
        ret = lcdc_initialize(FB_PSRAM_PTR, FB_PSRAM_BYTES);
        initialized = (ret >= OK);
    }
    return ret;
}

FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{ (void)display; return lcdcgetvplane(vplane); }
void up_fbuninitialize(int display) { (void)display; lcdcuninitialize(); }

#endif /* DISPLAY_RM690B0 */
