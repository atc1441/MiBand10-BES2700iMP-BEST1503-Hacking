/* rm690b0.h — Raydium RM690B0 AMOLED panel (MiBand9). [SCAFFOLD]
 * MIPI-DSI command mode ("MIPI SPI"), 1 data lane, ~192x490 RGB565. Panel identified
 * from the stock Vela FW (init rm690b0_lcd / rm690b0_check_id, VXN/TM variants). The DSI
 * host + LCDC + framebuffer use the SDK's hal_dsi/hal_lcdc — those need DSI_BASE/LCDC_BASE
 * + CMU display clocks for best1503 (TODO(probe), see DISPLAY_BRINGUP.md in 2700-src). */
#ifndef BES2700IMP_RM690B0_H
#define BES2700IMP_RM690B0_H
#include <stdint.h>
#include <stdbool.h>

int  rm690b0_init(void);                  /* power+reset, hal_dsi_init, panel DCS, dsi_start */
void rm690b0_on(bool on);                 /* display on/off (0x29/0x28)                       */
void rm690b0_set_brightness(uint8_t b);   /* 0x51                                              */
/* blit an RGB565 rectangle (framebuffer in PSRAM/extended SRAM -> LCDC layer) */
void rm690b0_blit(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t *px);
void rm690b0_fill(uint16_t color);        /* clear whole panel                                */

#endif /* BES2700IMP_RM690B0_H */
