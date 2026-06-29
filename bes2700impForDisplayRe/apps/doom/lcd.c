/* lcd.c - BES2700/best1503 display glue for GBADoom.
 *
 * HARDWARE 4-lane QUAD-SPI path (platform/main/disp_hwquad.c): the panel is driven by the
 * LCDC GEN_FRAME generator streaming a full RGB565 framebuffer as quad data with the 0x32
 * quad-write head. DOUBLE-BUFFER ping-pong: two full-panel 212x520 RGB565 buffers (in the FB
 * hole above the system stack; DOOM's arena was moved past them in apps/doom/bes_glue.c).
 *
 * SIDEWAYS (landscape) layout: the watch is held rotated 90 deg, so on screen the panel's tall
 * axis (py, 0..519) is horizontal and the short axis (px, 0..211) is vertical. DOOM's 120x160
 * image is rotated 90 deg and scaled up keeping its aspect ratio to fill the full screen height
 * (212 px), giving a 282x212 image centred along the long axis (py 119..400). That leaves a
 * 119 px control strip at each end: LEFT (py 0..118) = arrow keys, RIGHT (py 401..519) =
 * function keys (FIRE/USE/MENU). The strips are painted once by doom_ui_render565(); only the
 * image band is repainted per frame.
 */
#include <stdint.h>
#include "doomdef.h"
#include "lcd.h"

/* best1503 panel geometry (memory: fb[py*BES_W + px], px=0..211, py=0..519). */
#define BES_W   212
#define BES_H   520
#define FB_BYTES (BES_W * BES_H * 2)

/* Rotated/scaled DOOM image band along the long (py) axis. DOOM height 160 -> IMG_PW at the
 * 212/120 fill scale; centred in 520 leaves a 119 px control strip each side. */
#define IMG_PW   282                 /* image extent along py (SCREENHEIGHT * BES_W / SCREENWIDTH) */
#define IMG_PY0  ((BES_H - IMG_PW) / 2)   /* 119 */

static uint16_t *const g_fb_a = (uint16_t *)0x20080000u;
static uint16_t *const g_fb_b = (uint16_t *)0x200B6000u;
static uint16_t       *g_draw = (uint16_t *)0x200B6000u;   /* back buffer (not in flight) */

/* the HW quad driver + firmware glue (platform/main/disp_hwquad.c, hw_selftest.c). */
extern void doom_quad_bringup(void);                  /* one-time LCDC quad bring-up        */
extern void doom_ui_render565(uint16_t *buf);         /* static sideways control overlay   */
extern void doom_fb_flush(const void *p, unsigned n); /* clean D-cache before a DMA send    */
extern void disp_hwquad_start(const uint16_t *fb);     /* kick a frame (non-blocking)        */
extern void disp_hwquad_wait(void);                    /* wait for the in-flight frame (TXC) */
extern void doom_draw_title(uint16_t *buf);            /* title text over the game image     */
extern void doom_frame_tick(void);                     /* per-frame FPS counter -> trace UART */

/* DOOM palette -> RGB565 lookup, rebuilt whenever the active palette changes. */
static uint16_t pal565[256];

/* rotation/scale maps. The DOOM image gets an extra 90 deg (axes swapped vs the first pass):
 * the long band axis (i, along py) now samples DOOM x, the short axis (px) samples DOOM y. The
 * whole panel is then flipped 180 deg at write time. Flip a map (e.g. SCREENWIDTH-1 - ...) to
 * mirror an axis if the image comes out the wrong way round. */
static uint8_t map_x[IMG_PW];        /* i  (0..281, along py) -> DOOM x (0..119) */
static uint8_t map_y[BES_W];         /* px (0..211)           -> DOOM y (0..159) */
static int     maps_ready = 0;

static void build_maps(void)
{
    for (int i  = 0; i  < IMG_PW; i++)  map_x[i]  = (uint8_t)(i  * SCREENWIDTH  / IMG_PW);
    for (int px = 0; px < BES_W; px++)  map_y[px] = (uint8_t)(px * SCREENHEIGHT / BES_W);
    maps_ready = 1;
}

void lcd_set_pal(uint8_t *pal)
{
    for (int i = 0; i < 256; i++) {
        uint8_t r = *pal++, g = *pal++, b = *pal++;
        pal565[i] = (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }
}

void lcd_init(void)
{
    build_maps();
    doom_quad_bringup();                          /* clocks + 4-lane LCDC + RM690 init     */
    doom_ui_render565(g_fb_a);                    /* sideways control overlay into BOTH bufs */
    doom_ui_render565(g_fb_b);
    doom_fb_flush(g_fb_a, FB_BYTES);
    doom_fb_flush(g_fb_b, FB_BYTES);
    disp_hwquad_start(g_fb_a);                     /* prime: kick buffer A (overlay only)   */
    g_draw = g_fb_b;
}

void lcd_render_finish(void) { }

/* DOOM 120x160 palette-index frame -> rotated + upscaled RGB565 into the image band of the back
 * buffer, then ping-pong. DOOM's buffer is 16-bit slots; the palette index is the low byte.
 * Destination drow[px] is contiguous (one panel row); source gathers within one DOOM row. */
void lcd_render_fb(uint8_t *fb)
{
    const uint16_t *src16 = (const uint16_t *)fb;
    if (!maps_ready) build_maps();

    for (int i = 0; i < IMG_PW; i++) {
        unsigned dx = map_x[i];                                            /* DOOM column (fixed)   */
        /* mirror the long (py) axis so the game + overlay + touch share one consistent frame */
        uint16_t *prow = g_draw + (BES_H - 1 - (IMG_PY0 + i)) * BES_W;     /* panel row 400-i       */
        for (int px = 0; px < BES_W; px++)
            prow[px] = pal565[ src16[ (unsigned)map_y[px] * SCREENWIDTH + dx ] & 0xFF ];
    }
    doom_draw_title(g_draw);                       /* title on top of the game band         */
    doom_fb_flush(g_draw, FB_BYTES);

    disp_hwquad_wait();                            /* wait for the previously-kicked frame  */
    disp_hwquad_start(g_draw);                     /* send this freshly-rendered buffer     */
    g_draw = (g_draw == g_fb_a) ? g_fb_b : g_fb_a; /* next frame renders into the other     */
    doom_frame_tick();                             /* FPS to the trace UART                 */
}
