/* lcd.c - BES2700/best1503 display glue for GBADoom.
 *
 * Replaces the Apollo MSPI/Raydium LCD path. The best1503 firmware already owns
 * the RM690B0 AMOLED: a shared 8bpp RGB332 framebuffer at FB_PTR (0x20080000,
 * DISPLAY_WIDTH x DISPLAY_HEIGHT, layout fb[y*W + x]) pushed to the panel by
 * fb_refresh() (HW QSPI, platform/main/hw_selftest.c).
 *
 * DOOM renders an 8bpp palette-indexed buffer of SCREENWIDTH x SCREENHEIGHT
 * (120x160). lcd_set_pal() converts the active DOOM palette to RGB332 once;
 * lcd_render_fb() maps indices -> RGB332 into a window centred on the panel.
 */
#include <stdint.h>
#include "doomdef.h"
#include "lcd.h"

/* best1503 panel + shared framebuffer (constants mirror hw_selftest.c). */
#define BES_W       212
#define BES_H       520
#define BES_FB      ((volatile uint8_t *)0x20080000u)

/* The doom image is 120x160, drawn 1:1 centred on the 212x520 panel. */
#define IMG_X0  ((BES_W - SCREENWIDTH)  / 2)        /* 46  */
#define IMG_Y0  ((BES_H - SCREENHEIGHT) / 2)        /* 180 */
#define IMG_X1  (IMG_X0 + SCREENWIDTH  - 1)         /* 165 */
#define IMG_Y1  (IMG_Y0 + SCREENHEIGHT - 1)         /* 339 */

extern void fb_refresh(void);                                  /* full-panel push (static UI) */
extern void disp_send_window(const uint8_t *buf,
                             int x0, int y0, int x1, int y1);   /* partial (per-frame) push */

/* DOOM palette kept as full RGB; dithered to RGB332 at blit time so the 8bpp
 * panel shows smoother gradients instead of flat grey bands. */
static uint8_t palR[256], palG[256], palB[256];

void lcd_set_pal(uint8_t *pal)
{
    for (int i = 0; i < 256; i++)
    {
        palR[i] = *pal++; palG[i] = *pal++; palB[i] = *pal++;
    }
}

/* 4x4 ordered (Bayer) dither matrix, 0..15. */
static const uint8_t bayer4[4][4] = {
    {  0,  8,  2, 10 },
    { 12,  4, 14,  6 },
    {  3, 11,  1,  9 },
    { 15,  7, 13,  5 },
};
static inline int dith(int v, int t, int step)
{
    v += ((t * 2 - 15) * step) / 32;   /* ~ (t/16 - 0.5) * step */
    return v < 0 ? 0 : (v > 255 ? 255 : v);
}
/* palette index + screen pos -> dithered RGB332 (RRRGGGBB). */
static inline uint8_t pal_dither(int idx, int x, int y)
{
    int t = bayer4[y & 3][x & 3];
    int r = dith(palR[idx], t, 32);    /* 3-bit R, step 32 */
    int g = dith(palG[idx], t, 32);    /* 3-bit G, step 32 */
    int b = dith(palB[idx], t, 64);    /* 2-bit B, step 64 */
    return (uint8_t)((r & 0xE0) | ((g & 0xE0) >> 3) | (b >> 6));
}

extern void doom_draw_ui(void);   /* title + touch-zone legend in the panel border */

void lcd_init(void)
{
    /* Panel is brought up by the firmware (disp_init) before doom starts.
     * Paint the static control overlay once; doom only repaints the centre. */
    doom_draw_ui();
}

void lcd_render_finish(void)
{
}

/* Blit the 120x160 doom image into the centre of the panel FB and push the FULL
 * panel. The static title/legend lives in the FB border (drawn once by
 * doom_draw_ui) and is preserved because lcd_render_fb only writes the centre.
 * A partial-window push left the border flickering (this panel doesn't retain
 * GRAM outside the written window), so the whole frame is sent each time.
 * DOOM's buffer is 16-bit slots; the palette index is in the low byte. */
void lcd_render_fb(uint8_t *fb)
{
    const uint16_t *src16 = (const uint16_t *)fb;

    for (int y = 0; y < SCREENHEIGHT; y++)
    {
        volatile uint8_t *dst  = BES_FB + (IMG_Y0 + y) * BES_W + IMG_X0;
        const uint16_t   *srow = src16 + y * SCREENWIDTH;   /* SCREENPITCH == SCREENWIDTH shorts */
        for (int x = 0; x < SCREENWIDTH; x++)
            dst[x] = pal_dither(srow[x] & 0xFF, x, y);
    }

    fb_refresh();
}
