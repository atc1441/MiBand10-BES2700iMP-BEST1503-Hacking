/* disp_hwquad.c — RM690B0/C0 AMOLED bring-up over the BES2700 LCDC HARDWARE QUAD-SPI.
 *
 * Pipeline: GPIO panel power/reset -> LCDC display clock (96 MHz, OSC×4 tap) -> SMPN
 * command-engine sends the full RM690 init + per-frame DISPON/brightness/CASET/RASET
 * (single-line) -> GEN_FRAME generator streams the GRA-layer framebuffer as 4-lane quad
 * with the 0x32 quad-write head, framed by a manual GPIO CS that is held low until the
 * TXC (transmit-complete) gate so the panel commits the RAMWR.
 *
 * Everything reversed from vela_ota.bin / vela_ap.bin (best1503, NOT the best1306 SDK).
 * Raw MMIO, SDK-free. Panel 212x520 RGB565.
 */
#include <stdint.h>
#include "rm690_init.h"

#define REG(a)   (*(volatile uint32_t *)(uintptr_t)(a))
#define CR(o)    REG(0x40000000u + (o))     /* CMU     */
#define LR(o)    REG(0x40200000u + (o))     /* LCDC    */
#define AON(o)   REG(0x40080000u + (o))     /* AON_CMU */

/* CS = GPIO P5_1 (pin 41) = bank1 0x40089000 bit9, driven manually around each transfer. */
#define CSLO()   (REG(0x4008900Cu) = (1u << 9))
#define CSHI()   (REG(0x40089000u) = (1u << 9))

#define TXHOLD_US   20   /* hold CS low this long after TXC so the shift-register drains the
                          * last ~2 bytes (TXC fires when the FIFO is empty, not the shifter). */
#define PANEL_W     212
#define PANEL_H     520

static void hudelay(uint32_t us) { volatile uint32_t n = us * 200u + 50u; while (n--) __asm volatile("nop"); }

/* ======================================================================== *
 *  GPIO panel power + reset (stock GPIOs_LCD_inits_reset, vela_ap sub_2C10C138).
 *  Pins 73/74/76 = power-enable HIGH; pin 24 (P3_0) = RESX low->high; pin 23 (P2_7)
 *  = IOVCC enable HIGH. Banks: 0=0x40081000, 1=0x40089000, 2=0x4008A000.
 * ======================================================================== */
static const uint32_t g_gpio_bank[3] = { 0x40081000u, 0x40089000u, 0x4008A000u };
static void io_func(int pin, int func)   /* IOMUX func-select, 4 bits/pad */
{
    volatile uint32_t *r = (pin < 32)
        ? (volatile uint32_t *)(uintptr_t)(0x40086004u + (uint32_t)(pin >> 3) * 4)
        : (volatile uint32_t *)(uintptr_t)(0x4008607Cu + (uint32_t)((pin - 32) >> 3) * 4);
    int sh = (pin & 7) * 4;
    *r = (*r & ~(0xFu << sh)) | (((uint32_t)func & 0xF) << sh);
}
static void gp_out(int pin) { REG(g_gpio_bank[pin >> 5] + 0x04) = 1u << (pin & 31); }
static void gp_hi (int pin) { REG(g_gpio_bank[pin >> 5] + 0x00) = 1u << (pin & 31); }
static void gp_lo (int pin) { REG(g_gpio_bank[pin >> 5] + 0x0C) = 1u << (pin & 31); }

void disp_panel_power_reset(void)
{
    static const int pwr[3] = { 73, 74, 76 };
    for (int i = 0; i < 3; i++) { io_func(pwr[i], 0); gp_out(pwr[i]); gp_hi(pwr[i]); }
    io_func(24, 0); gp_out(24); gp_lo(24);   /* RESX (P3_0) LOW    */
    hudelay(10000);
    io_func(23, 0); gp_out(23); gp_hi(23);   /* enable (P2_7) HIGH */
    hudelay(15000);
    gp_hi(24);                               /* RESX release HIGH  */
    hudelay(20000);
}

/* ======================================================================== *
 *  Clock tree.
 * ======================================================================== */

/* AON_PSC display power island (0x40085028): OFF at boot -> all LCDC registers read 0.
 * Stock sub_2C0A0FA2: 0x7F assert -> 0x77 clear PSW -> 0x70 clock on / un-isolate / un-reset. */
static void psc_display_powerup(void)
{
    volatile uint32_t *DIS = (volatile uint32_t *)0x40085028u;
    *DIS = 0xCAFE02FFu; hudelay(1000);
    *DIS = 0xCAFE02F7u; hudelay(1000);
    *DIS = 0xCAFE02F0u; hudelay(1000);
}

/* AON_PSC clock-out enable key 0xCAF5 (cmu_pll_enable_module, stock sub_274628). */
static void psc_clkout_key(void)
{
    volatile uint32_t *K = (volatile uint32_t *)0x40085028u;
    *K = 0xCAF5083Fu; hudelay(20); *K = 0xCAF50837u; hudelay(20); *K = 0xCAF50830u;
}

/* Boot clock-tree module HCLK/reset enables (stock sub_C0A12B8). The stripped best1503 boot
 * OMITS these — they enable the module clocks the GEN_FRAME DMA-master needs (without them the
 * b29 DMA fetch never engages). MUST run ONCE at boot BEFORE the UART init (0x03C/0x00C touch
 * module 42 = UART0, which the SDK re-inits). */
void disp_boot_clocks(void)
{
    /* 0x03C = PRESET_SET, 0x00C = PCLK_DISABLE. The stock mask 0xFFFFFDF2 also resets + PCLK-gates
     * UART0/1/2 (SYS_PCLK_UART0/1/2 = bits 10/11/12) -> in the full DOOM build that kills the trace
     * UART (UART1=COM24) and the SDK no longer re-inits it. Clear bits 10-12 so the UARTs are left
     * running; the display modules are unaffected. (mask & ~0x1C00 = 0xFFFFE1F2) */
    CR(0x150) = 0x3CFFu;   CR(0x174) = 0x7FFu;   CR(0x03C) = 0xFFFFE1F2u;
    CR(0x16C) = 0x7FFu;    CR(0x00C) = 0xFFFFE1F2u; CR(0x144) = 0x3CFFu;
    CR(0x044) = 0x100000u; CR(0x014) = 0x100000u;
    CR(0x034) = 0x80000000u; CR(0x004) = 0x80000000u;
}

/* cmu_disp_clk_set_freq(96) tap-select (vela_ota 0x26f578): 96 MHz = OSC×4 tap. Re-writing the
 * CR 0x104/0x100 (DSI clock disable/enable) pair also power-cycles the SMPN clock domain. */
static void disp_clk_tap(void)
{
    CR(0x104) = (CR(0x104) & 0xFFFC1FFFu) | 0x3A000u;   /* OSC×4 tap -> SCLK ~50 MHz */
    CR(0x100) = (CR(0x100) & 0xFFFC1FFFu) | 0x4000u;
}

/* hal_cmu_lcdc_enable (sub_26F1E8) + cmu_disp_clk_set_freq(96) (sub_26F29A). Brings up the OSC×2
 * (48 MHz) + OSC×4 (96 MHz) multipliers and selects the 96 MHz display tap (the stripped boot
 * never enables OSC×4, so the tap had no source -> 12 MHz fallback). */
static void stock_cmu_display_clock(void)
{
    CR(0x104) = (CR(0x104) & 0xFFFFE0FFu) | 0x1B00u;
    CR(0x100) = (CR(0x100) & 0xFFFFE0FFu) | 0x400u;
    psc_clkout_key();
    CR(0x000) = 0x200000u;
    CR(0x140) = 0x40u;
    CR(0x010) = 0x200000u;
    CR(0x044) = 0x20000000u; hudelay(1); CR(0x048) = 0x20000000u;
    CR(0x150) = 0x40u;  hudelay(1); CR(0x154) = 0x40u;
    CR(0x150) = 0x100u; hudelay(1); CR(0x154) = 0x100u;
    CR(0x034) = 0x200000u; hudelay(1);
    CR(0x038) = 0x200000u;
    CR(0x150) = 0x80u;  hudelay(1); CR(0x154) = 0x80u;
    CR(0x044) = 0x10000000u; hudelay(1); CR(0x048) = 0x10000000u;
    /* CMU display/peripheral clock-divider config (hal_cmu_module_init_state) the boot omits. */
    CR(0x0D4) = 67903552u;  CR(0x0D8) = 153121157u; CR(0x0DC) = 305447678u;
    CR(0x068) = (CR(0x068) & 0xFFFFFFC0u) | 0x13u;
    CR(0x0E0) = 409841429u; CR(0x0E4) = 494131673u; CR(0x0E8) = 585066462u;
    CR(0x0EC) = (CR(0x0EC) & 0xFFFFFFC0u) | 0x23u;
    CR(0x060) = 0x4000u; hudelay(30);    /* osc_x2 request          */
    CR(0x060) = 0x2000u;                 /* osc_x2 enable (48 MHz)  */
    CR(0x060) = 0x8000u;                 /* osc_x4 enable (96 MHz)  */
    AON(0x018) &= ~0x800u;               /* osc_x4 AON gate clear   */
    disp_clk_tap();                      /* select the 96 MHz tap   */
    psc_clkout_key();
    CR(0x168) = 0x400u; CR(0x048) = 0x200000u; CR(0x178) = 0x400u;
}

/* Pulse-reset the LCDC SMPN/panel-path core (modules 91/92) — stock sub_C083390. Without
 * resetting module 91 it stays held in reset and the GRA-DMA never fetches. */
static void lcdc_module_reset(void)
{
    CR(0x044) = 0x08000000u; hudelay(2); CR(0x048) = 0x08000000u; hudelay(2);   /* module 91 */
    CR(0x044) = 0x10000000u; hudelay(2); CR(0x048) = 0x10000000u; hudelay(2);   /* module 92 */
}

/* Per-transfer SMPN reset (the stock cmu_smpn_xfer_clock ritual): re-tap the clock + module
 * clock-enable/reset-clear. Without it the SMPN cmd-done (b18) only latches on the first
 * command of a back-to-back pair (CASET ok, RASET garbled). */
static void smpn_xfer_reset(void)
{
    disp_clk_tap();
    CR(0x168) = 0x400u; CR(0x048) = 0x200000u; CR(0x178) = 0x400u;
}

/* SMPN pixel/command pipeline config (stock sub_2C564E0C(2,4)). */
static void lcdc_smpn_config(void)
{
    LR(0x1A8) = 1u;
    LR(0x2A8) = 0x10009F01u;
    LR(0x1F8) = 0x43u;
    LR(0x180) = 0x02000F89u;
    LR(0x22C) = 0x86002C00u;
    LR(0x194) = 0x81u;
    LR(0x188) = 0x2501u;
    LR(0x264) = 1u;
}

/* P5_0/2..5 -> func2 (LCDC SCLK + D0..D3), P5_1 -> GPIO CS, drive strength 3 (sharp edges at
 * the quad clock rate). Pads 40-45 drive = 0x400860A0 bits[27:16], 2 bits/pad. */
static void lcdc_pads_hw(void)
{
    io_func(40, 2); io_func(42, 2); io_func(43, 2); io_func(44, 2); io_func(45, 2);
    io_func(41, 0); REG(0x40089004u) = (1u << 9); CSHI();
    REG(0x400860ACu) |= 0x200u;
    REG(0x400860A0u) = (REG(0x400860A0u) & ~0x0FFF0000u) | 0x0FFF0000u;   /* all 6 pads drive 3 */
}

/* ======================================================================== *
 *  SMPN single-line command engine (DCS commands + CASET/RASET window).
 * ======================================================================== */

/* Bounded ~8ms poll for cmd-done (b18). The command is already on the wire; b18 is flaky, so a
 * miss just stops waiting (no commit impact) instead of the old 2M-spin 170ms stall. */
static int lcdc_wait_done(void)
{
    /* ~160us cap. A single-line command clocks out in <6us at the 12MHz command rate, so the
     * command is on the wire well within this; the cmd-done (b18) bit is just flaky. Capping low
     * means a missed b18 costs ~160us, not the old 8ms (the "~7ms SPI ready" stall). */
    for (volatile uint32_t g = 0; g < 2000u; g++)
        if (LR(0x1C4) & 0x40000u) { LR(0x1C4) &= ~0x40000u; return 0; }
    return -1;
}

/* Single-line DCS command (0x02 instruction) + up to 32 param bytes, CS-framed. The trigger is
 * written ONCE with the param length folded in — the stock two-step write double-fires on a slow
 * clock (header alone, then header+params again). */
static void smpn_write(uint8_t cmd, const uint8_t *p, int n)
{
    smpn_xfer_reset();
    CSLO();
    LR(0x1F8) = 0x08u;                              /* single-line for command + params */
    LR(0x1C4) &= ~0x40000u;
    LR(0x284) = ((uint32_t)cmd << 8) | 0x02000000u;
    if (n > 0 && p) {
        if (n > 0x20) n = 0x20;
        volatile uint32_t *fifo = (volatile uint32_t *)0x40200288u;
        int v7 = 0, words = (n + 3) >> 2;
        for (int w = 0; w < words; w++) {
            uint32_t v = 0;
            for (int k = 0; k < 4; k++) { v <<= 8; if (n > v7) { v |= p[v7]; v7++; } }
            *fifo++ = v;
        }
    }
    {
        uint32_t trig = 1u;
        if (n > 0 && p) trig |= ((((uint32_t)n << 7) - 16) & 0x1FF0u) | 2u;
        LR(0x280) = trig;
    }
    lcdc_wait_done();
    CSHI();
}

/* CASET (0x2A) + RASET (0x2B): the GEN_FRAME auto-head only emits RAMWR (0x2C), so the write
 * window must be set on the command engine before the quad data or the panel drops it. */
static void smpn_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    uint8_t c[4];
    c[0] = x0 >> 8; c[1] = x0 & 0xFF; c[2] = x1 >> 8; c[3] = x1 & 0xFF; smpn_write(0x2A, c, 4);
    c[0] = y0 >> 8; c[1] = y0 & 0xFF; c[2] = y1 >> 8; c[3] = y1 & 0xFF; smpn_write(0x2B, c, 4);
}

/* Send a panel init table over the HW command engine (manufacturer page, COLMOD, MADCTL,
 * CASET/RASET, TE, sleep-out, display-on, ...). */
static void smpn_send_config(const struct rm690_cmd *init, int count)
{
    for (int i = 0; i < count; i++) {
        const struct rm690_cmd *c = &init[i];
        if (c->reg == 0xFFu && c->len == RM690_DELAY) { hudelay(60000); continue; }   /* 60 ms */
        smpn_write(c->reg, c->d, c->len < 0 ? 0 : c->len);
    }
}

/* ---- Panel RDID read + variant selection ---------------------------------
 * Faithful port of the stock LCD_read_regbuff(reg, &b, 1) (vela_ap @0xc564fd0). One DCS register
 * byte: the 0x03 read-instruction goes out, the panel's reply clocks into the RX FIFO (0x140).
 *
 * Two things the stock does that are essential for a REPEATABLE read (and that my earlier version
 * got wrong, giving 0xFF after the first byte):
 *   1) it re-taps the display clock at the start of EVERY read — that rewrite of CMU 0x104/0x100
 *      power-cycles the SMPN clock domain, which is the real per-read state-reset (NOT a 0x188
 *      SMPN_ENA toggle, which left the serializer holding the stale 0xFF);
 *   2) the length register 0x27C is 0 for a 1-byte read ((rxLen+3>>2)-1 = 0). I had it at 3, which
 *      mis-set the transfer length and returned idle 0xFF.
 * The read also stays in the configured 4-lane state (0x1F8 untouched) and only swaps CFG_180 to
 * the read value 0x02071F8B (RXBITS=7 -> 8 bits in), restoring 0x02000F89 afterwards. */
static uint8_t smpn_read_byte(uint8_t reg)
{
    disp_clk_tap();                          /* re-tap = SMPN clock-domain reset (stock per-read)   */
    CSLO();
    LR(0x1A8) = 8u;                          /* SCLK ~12 MHz: slow enough for the panel's read turnaround */
    LR(0x1C4) &= ~0x40000u;                  /* clear cmd-done                                      */
    LR(0x27C) = 0u;                          /* length field = 0 for a single byte                  */
    LR(0x284) = ((uint32_t)reg << 8) | 0x03000000u;               /* 0x03 read instruction + reg    */
    /* RXBITS=8 -> read 9 bits. At 1A8=8 the RX window opens one SCLK early, so the panel reply
     * comes back as [dummy, V7..V0] accumulated MSB-first; reading 9 bits lands V in bits 0..7
     * (dummy in bit 8) so (rx & 0xFF) == V. (RXBITS=7 dropped the LSB -> value read as V>>1.) */
    LR(0x180) = (((uint32_t)8 << 16) & 0xFF0000u) | 0x02001F8Bu;  /* read cfg, RXBITS=8 (9 bits in) */
    LR(0x230) |= 0x40u;                       /* enable RX capture                                  */
    LR(0x280) = 1u;                           /* trigger                                            */
    lcdc_wait_done();
    LR(0x284) = 0u;
    LR(0x230) &= ~0x40u;
    LR(0x27C) = 0u;
    hudelay(5);                               /* stock delay_aon_timer_ticks(3) settle              */
    uint32_t rx = LR(0x140);                  /* RX FIFO, low byte = the reply                      */
    LR(0x180) = 0x02000F89u;                  /* restore cmd cfg (IsSavedInitLCDC_to2 == 2)         */
    LR(0x1A8) = 1u;                           /* restore SCLK divider                               */
    CSHI();
    return (uint8_t)(rx & 0xFF);
}

static uint32_t lcd_read_id(void)   /* RDID1/2/3 (0xDA/0xDB/0xDC) -> 24-bit ID */
{
    uint32_t a = smpn_read_byte(0xDA), b = smpn_read_byte(0xDB), c = smpn_read_byte(0xDC);
    return (a << 16) | (b << 8) | c;
}

uint32_t g_lcd_id = 0;                                  /* the RDID actually read                */
const struct rm690_variant *g_lcd_dev = &g_rm690_variants[RM690_DEFAULT_VARIANT];  /* selected variant */
uint32_t disp_hwquad_get_id(void) { return g_lcd_id; }
const char *disp_hwquad_get_variant(void) { return g_lcd_dev->name; }

/* ======================================================================== *
 *  Per-frame LCDC re-engagement: pads + display clock (re-taps the SMPN clock domain) + SMPN
 *  core module-reset + SMPN config + the full GEN_FRAME generator/layer setup. This RE-ARMS the
 *  SMPN and MUST run every frame — without it only the first frame transmits; the second sends a
 *  0xFF header with CS too early (the SMPN is left hung). It is all fast MMIO (no command-engine
 *  cmd-done waits), so it does not add the ~7ms stalls (those came from the smpn_write commands,
 *  which now run only once in disp_hwquad_init).
 * ======================================================================== */
static void lcdc_frame_setup(void)
{
    lcdc_pads_hw();
    stock_cmu_display_clock();
    lcdc_module_reset();
    lcdc_smpn_config();

    LR(0x21C) = 0x20020u;                                       /* porch_cfg(32,32,0)        */
    LR(0x220) = 0x001F001Eu;                                    /* sync(30,31)               */
    LR(0x11C) = (LR(0x11C) & 0xF000F000u) | 0x000C000Au;        /* hporch(10,12)             */
    LR(0x120) = (LR(0x120) & 0xF000F000u) | 0x000A000Au;        /* vporch(10,10)             */
    LR(0x114) = (LR(0x114) & 0xF000F000u) | (uint32_t)(PANEL_W + 31) | ((uint32_t)(PANEL_H + 21) << 16);

    LR(0x108) = (uint32_t)PANEL_W | ((uint32_t)PANEL_H << 16);  /* layer0 (GRA) zoom         */
    LR(0x100) = 0x80000000u;                                    /* layer0 src_size | valid   */
    LR(0x0F0) = (uint32_t)PANEL_W | ((uint32_t)PANEL_H << 16);  /* layer1 (DMA) zoom         */
    LR(0x0E8) = 0x80000000u;                                    /* layer1 src_size           */
    LR(0x118) = (uint32_t)PANEL_W | ((uint32_t)PANEL_H << 16);  /* SPU dst_size              */
    LR(0x188) |= 1u;                                            /* layer_enable(5)           */
    LR(0x124) = 0u;                                             /* SPU background = 0        */

    LR(0x1E8) &= ~3u;                                           /* FIRSTLSEL=0               */
    LR(0x1DC) = (LR(0x1DC) & 0xFFFFF0FFu) | (15u << 8);         /* sl_dmaburst(15)           */
    LR(0x200) = (LR(0x200) & 0xFFFFE0FFu) | (8u << 8);          /* ol_dmaburst(8)            */
    LR(0x1EC) = (LR(0x1EC) & 0xFFFFFF8Cu) | 0x12u;              /* dma_enable + burst        */
    LR(0x214) = 5u;
    LR(0x228) = 0x4A000000u;

    LR(0x104) = (uint32_t)PANEL_W | ((uint32_t)PANEL_H << 16);  /* GRA dst_size              */
    LR(0x0FC) = (uint32_t)(PANEL_W * 2) | (4u << 16);           /* GRA pitch                 */
    LR(0x264) = 1u;                                             /* GRA_ENA                   */

    LR(0x1C8) = 0x40000u;                                       /* SPI_IRQ                   */
    LR(0x1C4) = 0u;
    LR(0x1C0) = 0x200000u;                                      /* IER: SPU_FRAMEDONE/TXC b21 */
    LR(0x254) = 65u;                                            /* TECR DB_TARGET = TE GPIO 65 */
    LR(0x260) |= (1u << 27);                                    /* ARBFAST_ENA: AXI arbiter fast */
    LR(0x1F8) = 0x43u;                                          /* 4LN_MODE                  */
    LR(0x22C) = 0x86002C00u;                                    /* auto-head -> wire 0x32 0x00 0x2C 0x00 */
    LR(0x188) = 0x6501u;                                        /* SMPN_ENA | SMPNVSYNC      */
    LR(0x180) = 0x02000F89u;                                    /* SPI_ENA|TXBITS16|SCLKCNT2|START */
    LR(0x194) = 0x80027F81u;                                    /* PXLMD|PN_ALPHA|ALPHA_MODE2|TRIG */
    LR(0x210) = 0x00000003u;                                    /* VSYNC_TRIG_DIS            */
    LR(0x264) &= ~1u; LR(0x264) |= 1u;                          /* GRA enable -> latch + arm DMA */
    LR(0x230) |= 0x40u;                                         /* LINE_RDY                  */
    LR(0x210) |= 0x20000000u;                                   /* GEN_FRAME (b29)           */
}

/* ======================================================================== *
 *  One-time bring-up: power island + the LCDC setup + the panel init (RM690 config + DISPON +
 *  brightness + CASET/RASET window) over the single-line command engine. Call once after
 *  disp_panel_power_reset(). The command-engine transfers (the ~7ms cmd-done waits) run ONLY here;
 *  the panel keeps the window + brightness, so active sending never re-sends them.
 * ======================================================================== */
void disp_hwquad_init(void)
{
    psc_display_powerup();
    lcdc_frame_setup();

    /* Read the panel RDID and select the matching variant (init table + CASET X-offset), like the
     * stock AppBringUp. Falls back to variant 3 (O66 TM, our panel) if the read matches nothing. */
    g_lcd_id = lcd_read_id();
    g_lcd_dev = &g_rm690_variants[RM690_DEFAULT_VARIANT];
    for (unsigned i = 0; i < RM690_VARIANT_COUNT; i++)
        if (g_rm690_variants[i].id == g_lcd_id) { g_lcd_dev = &g_rm690_variants[i]; break; }

    LR(0x1A8) = 8u;                                             /* slow command clock        */
    smpn_send_config(g_lcd_dev->init, g_lcd_dev->count);        /* variant-specific init     */
    smpn_write(0x29, 0, 0);                                    /* DISPON                    */
    { uint8_t br = 0xFFu; smpn_write(0x51, &br, 1); }           /* brightness = full         */
    {   /* write window with the per-variant X-offset (0x26xxxx panels start at X=24) */
        uint16_t x0 = g_lcd_dev->x_offset;
        smpn_set_window(x0, 0, (uint16_t)(x0 + PANEL_W - 1), PANEL_H - 1);
    }
    LR(0x1F8) = 0x43u;                                          /* back to 4-lane for the data */
}

/* ======================================================================== *
 *  Active sending. disp_hwquad_start() re-arms the SMPN (lcdc_frame_setup) and kicks STARTCR, then
 *  RETURNS — the data transfer runs in HARDWARE (GRA-DMA + serializer), so the CPU is free to
 *  render the OTHER buffer while this one is in flight (double-buffer ping-pong). disp_hwquad_wait()
 *  blocks until the serializer has drained (TXC b21) and releases CS.
 * ======================================================================== */
void disp_hwquad_start(const uint16_t *fb)
{
    lcdc_frame_setup();                                 /* full per-frame re-engage (working)   */
    /* Flush the stale 0xFF from the TX serializer (it idles HIGH; from the 2nd frame on the
     * auto-head would emit that stale byte first). Toggle SMPN_ENA (b0) off->on + RSTB (b3). */
    LR(0x188) &= ~((1u << 0) | (1u << 3));              /* SMPN disable + RSTB assert */
    hudelay(2);
    LR(0x188) |= (1u << 0);                             /* SMPN enable (RSTB released, b3=0) */
    LR(0x1A8) = 1u;                                      /* full-rate generator clock            */
    CSLO();
    LR(0x0F4) = (uint32_t)(uintptr_t)fb;                 /* GRA layer0 addr = framebuffer        */
    if (LR(0x194) >= 0) LR(0x194) |= 0x80000000u;        /* re-arm panel-path trans-trigger      */
    LR(0x210) |= 0x20000000u;                            /* (re-)enable GEN_FRAME for THIS frame */
    LR(0x1C4) = 0x00E00000u;                             /* W1C-clear TXC(b21)+b22+VSYNC(b23)    */
    LR(0x224) |= 1u;                                     /* STARTCR — kick one generator frame   */
}

void disp_hwquad_wait(void)
{
    /* Wait for TXC (b21 = serializer drained), NOT GRA_FRAME (b27 = DMA-done, which fires while
     * the serializer is still clocking out -> CS would go high mid-data). */
    for (volatile uint32_t g = 0; g < 6000000u; g++)
        if (LR(0x1C4) & 0x00200000u) break;

    LR(0x210) &= ~0x20000000u;   /* stop GEN_FRAME so it does not free-run -> stale TXC next frame */
    hudelay(TXHOLD_US);          /* let the shift-register drain the last ~2 bytes after TXC */
    CSHI();
}

/* Convenience blocking send (start + wait). */
void disp_hwquad_frame(const uint16_t *fb) { disp_hwquad_start(fb); disp_hwquad_wait(); }
