/* rm690b0.c — RM690B0 AMOLED driver. The panel DCS sequence is ready; it only does real
 * DSI/LCDC work when the best1503 display HAL is wired (CONFIG_DISPLAY_DSI_BEST1503 +
 * DSI_BASE/LCDC_BASE + CMU display clocks — TODO(probe)). Without that flag the entry
 * points are safe stubs so board_init() links and runs today. */
#include "rm690b0.h"
#include "board_config.h"

#if defined(CONFIG_DISPLAY_DSI_BEST1503)
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_gpio.h"
#include "hal_timer.h"

#define FB_BASE   (BRD_PANEL_FB_PSRAM ? BRD_PSRAM_BASE : (BRD_SRAM_BASE + BRD_SRAM_FW_RESERVED))

static void rm690b0_panel_seq(void)
{
    uint16_t hs=0, he=BRD_PANEL_W-1, vs=0, ve=BRD_PANEL_H-1;
    hal_dsi_send_cmd_data(0xFE,1,0x00,0,0,0);          /* mfr cmd page 0   */
    hal_dsi_send_cmd_data(0x35,1,0x00,0,0,0);          /* TE on            */
    hal_dsi_send_cmd_data(0x36,1,0x00,0,0,0);          /* MADCTL           */
    hal_dsi_send_cmd_data(0x3A,1,0x55,0,0,0);          /* RGB565           */
    hal_dsi_send_cmd_data(0x51,1,0xFF,0,0,0);          /* brightness       */
    hal_dsi_send_cmd_data(0x2A,4,hs>>8,hs&0xFF,he>>8,he&0xFF);
    hal_dsi_send_cmd_data(0x2B,4,vs>>8,vs&0xFF,ve>>8,ve&0xFF);
    hal_dsi_send_cmd(0x11); hal_sys_timer_delay(MS_TO_TICKS(120));   /* sleep out  */
    hal_dsi_send_cmd(0x29); hal_sys_timer_delay(MS_TO_TICKS(20));    /* display on */
}
int rm690b0_init(void)
{
#if (BRD_DISPLAY_RST_PIN >= 0)
    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)BRD_DISPLAY_RST_PIN, HAL_GPIO_DIR_OUT, 1);
    hal_sys_timer_delay(MS_TO_TICKS(20)); hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)BRD_DISPLAY_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20)); hal_gpio_pin_set((enum HAL_GPIO_PIN_T)BRD_DISPLAY_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(120));
#endif
    hal_dsi_init(BRD_PANEL_W);
    rm690b0_panel_seq();
    hal_sys_timer_delay(MS_TO_TICKS(50));
    hal_dsi_start();
    return 0;
}
void rm690b0_on(bool on){ hal_dsi_send_cmd(on?0x29:0x28); }
void rm690b0_set_brightness(uint8_t b){ hal_dsi_send_cmd_data(0x51,1,b,0,0,0); }
void rm690b0_blit(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t *px)
{
    uint16_t xe=x+w-1, ye=y+h-1;
    hal_dsi_send_cmd_data(0x2A,4,x>>8,x&0xFF,xe>>8,xe&0xFF);
    hal_dsi_send_cmd_data(0x2B,4,y>>8,y&0xFF,ye>>8,ye&0xFF);
    uint16_t *fb=(uint16_t*)(uintptr_t)FB_BASE;
    for (uint32_t i=0;i<(uint32_t)w*h;i++) fb[i]=px[i];
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE,(uint32_t)(uintptr_t)fb);
    hal_lcdc_lsize(LCDC_LAYER_LFORE,w,h);
}
void rm690b0_fill(uint16_t color)
{
    uint16_t *fb=(uint16_t*)(uintptr_t)FB_BASE;
    for (uint32_t i=0;i<(uint32_t)BRD_PANEL_W*BRD_PANEL_H;i++) fb[i]=color;
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE,(uint32_t)(uintptr_t)fb);
    hal_lcdc_lsize(LCDC_LAYER_LFORE,BRD_PANEL_W,BRD_PANEL_H);
}
#else  /* display HAL not wired for best1503 yet */
int  rm690b0_init(void){ return -1; }                         /* todo(probe): DSI base/clocks */
void rm690b0_on(bool on){ (void)on; }
void rm690b0_set_brightness(uint8_t b){ (void)b; }
void rm690b0_blit(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t*p){(void)x;(void)y;(void)w;(void)h;(void)p;}
void rm690b0_fill(uint16_t c){ (void)c; }
#endif
