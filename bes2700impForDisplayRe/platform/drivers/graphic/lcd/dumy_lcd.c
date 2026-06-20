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
 * DOC: a dumy board lcd driver
 *  setup lcd display pannel
 *  Initialize the framebuffer video hardware associated with the display
 *
 */

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
#include "hal_psc.h"

#define LCD_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)

#if defined(DISPLAY_GC9503NP)
#define WIDTH       480
#define HEIGHT      800
#else
#define WIDTH       454
#endif

static uint32_t fb_buf[CONFIG_LCDC_FB_SIZE/4];

//#define SOCKET_BOARD

#ifdef SOCKET_BOARD
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#else
#if defined(CHIP_BEST2003)
#define GPIO_RST_PIN HAL_GPIO_PIN_P0_3
#elif defined(CHIP_BEST1502X)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_1
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_4
#endif
#endif

/**
 * Pre-processor Definitions
 */


/**
 * Private Data
 */
POSSIBLY_UNUSED
static int putarea(fb_coord_t row_start, fb_coord_t row_end,
               fb_coord_t col_start, fb_coord_t col_end,
               FAR const uint8_t *buffer)
{
    uint16_t width,heigh;
    uint32_t fbmem = (uint32_t)buffer;
    col_start &= 0xfffe;
    row_start &= 0xfffe;
    width = (col_end - col_start +2)&0xfffe;
    heigh = (row_end - row_start +2)&0xfffe;

    uint16_t hstart = 14 + col_start;
    uint16_t hend = hstart+width -1;
    uint16_t vstart = row_start;
    uint16_t vend = vstart+heigh-1;

    hal_dsi_reset(width);
    hal_dsi_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    hal_dsi_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);

    fbmem += (row_start*WIDTH*4)+col_start*4;
    /* Configure Layer start addr register */
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, fbmem);
    /* Configure Layer size register */
    hal_lcdc_lsize(LCDC_LAYER_LFORE, width,heigh);
    hal_lcdc_lsize(LCDC_LAYER_SPU, width,heigh);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, width,heigh);

    return 0;
}

static int lcdc_getplaneinfo(FAR struct lcd_dev_s *dev,
                               unsigned int planeno,
                               FAR struct lcd_planeinfo_s *pinfo)
{

  pinfo->putarea = putarea;
  return OK;
}


struct lcd_dev_s g_lcd;


/**
 * Private Functions
 */

/**
 * lcd_initialize - Initialize the  LCD controller to the RGB interface mode.
 *
 */
#define RM69330_XRES       454
#define RM69330_YRES       454
POSSIBLY_UNUSED
static void rm69330_init()
{
    uint16_t hstart = 14;
    uint16_t hend = RM69330_XRES-1+hstart;
    uint16_t vstart = 0;
    uint16_t vend = RM69330_YRES-1+vstart;

    hal_dsi_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    hal_dsi_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    hal_dsi_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    hal_dsi_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));
}

POSSIBLY_UNUSED
static void rm69090_init(void)
{
	 //DSI init-2
    hal_dsi_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    hal_dsi_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
	hal_dsi_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    //hal_dsi_send_cmd_data(0x2a, 4, 0x0, 0x10, 0x01, 0x7f);
    //hal_dsi_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x01, 0xbf);
    hal_dsi_send_cmd_data(0x2a, 4, 0x0, 0xe, 0x1, 0xd3);
	hal_dsi_send_cmd_data(0x2a, 4, 0x0, 0xe, 0x1, 0xd3);
    hal_dsi_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x1, 0xc5);
	hal_dsi_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x1, 0xc5);
    //DSI sleep out
    hal_dsi_send_cmd(0x11);
	hal_dsi_send_cmd(0x11);

    hal_sys_timer_delay(MS_TO_TICKS(240));

    //DSI disp on
    hal_dsi_send_cmd(0x29);
	hal_dsi_send_cmd(0x29);

    hal_sys_timer_delay(MS_TO_TICKS(150));
}

#define SCAN_LINE               0
#define REGFLAG_DELAY           0xFC
#define REGFLAG_END_OF_TABLE    0xFD	/* END OF REGISTERS MARKER */

struct LCM_setting_table {
	unsigned char cmd;
	unsigned char count;
	unsigned char params[64];
};

POSSIBLY_UNUSED
static struct LCM_setting_table lcm_initialization_setting[] = {
    {0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}},
    {0xF6, 2, {0x5A, 0x87}},
    {0xC1, 1, {0x3F}},
    {0xC2, 1, {0x0E}},
    {0xC6, 1, {0xF8}},
    {0xC9, 1, {0x10}},
    {0xCD, 1, {0x25}},
    {0x86, 4, {0x88, 0xA3, 0xA3, 0x31}},
    {0x87, 3, {0x04, 0x03, 0x66}},
    {0xAC, 1, {0x65}},
    {0xF8, 1, {0x8A}},
    {0xA7, 1, {0x47}},
    {0xA0, 1, {0xFF}},
    {0xB1, 1, {0x04}},
    {0xFA, 4, {0x08, 0x08, 0x08, 0x04}},
    {0x71, 1, {0x48}},
    {0x72, 1, {0x48}},
    {0x73, 2, {0x00, 0x44}},
    {0xA3, 1, {0xEE}},
    {0xFD, 3, {0x3C, 0x3C, 0x00}},
    {0x97, 1, {0xEE}},
    {0x83, 1, {0x93}},
    {0x77, 2, {0x00, 0x08}},
    {0x9A, 1, {0x98}},
    {0x9B, 1, {0x98}},
    {0x82, 2, {0x6F, 0x6F}},
    {0x80, 1, {0x0D}},
    {0x7A, 2, {0x13, 0x1A}},
    {0x7B, 2, {0x13, 0x1A}},
    {0x6D, 32, {0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E}},
    {0x60, 8, {0x18, 0x08, 0x10, 0x70, 0x18, 0x07, 0x10, 0x70}},
    {0x64, 16, {0x18, 0x06, 0x03, 0x22, 0x70, 0x03, 0x18, 0x05, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x65, 16, {0x18, 0x04, 0x03, 0x23, 0x70, 0x03, 0x18, 0x03, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x66, 16, {0x18, 0x02, 0x03, 0x23, 0x70, 0x03, 0x18, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x67, 16, {0x18, 0x00, 0x03, 0x23, 0x70, 0x03, 0x10, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}},
    {0x68, 13, {0x00, 0x08, 0x1D, 0x08, 0x1C, 0x00, 0x00, 0x08, 0x11, 0x08, 0x10, 0x00, 0x00}},
    {0x69, 7, {0x14, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08}},
    {0x6A, 8, {0x08, 0x20, 0x48, 0x16, 0x01, 0x23, 0x45, 0x67}},
    {0x6B, 1, {0xA7}},
    {0xD1, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}},
    {0xD2, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}},
    {0xD3, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}},
    {0xD4, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}},
    {0xD5, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}},
    {0xD6, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}},
    {0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {0x6D, 32, {0x1E, 0x10, 0x0E, 0x0C, 0x0A, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x09, 0x0B, 0x0D, 0x0F, 0x1E}},
    {0x77, 2, {0x00, 0x00}},
    {0xA6, 1, {0x56}},
    {0x29, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},
};

POSSIBLY_UNUSED
static int push_table(struct LCM_setting_table *table, int count)
{
  for(int i = 0; i < count; i++)
  {
    switch (table[i].cmd)
    {
    case REGFLAG_DELAY:
      //usleep(table[i].count * 1000);
      hal_sys_timer_delay(MS_TO_TICKS(table[i].count));
      break;
    /*case REG_READ_DELAY:
       {    uint8_t buf;
            hal_dsi_read_cmd(table[i-1].cmd,&buf,table[i].count);
            if(buf == table[i-1].params[0]){
              break;
            }else{
              LCD_ERR("INIT ERR!!!!!!\n");
              return -1;
              break;
            }
       }*/
    default:
      hal_dsi_send_cmd_list(table[i].cmd, table[i].count, table[i].params);
      break;
    }
  }
  return 0;
}

POSSIBLY_UNUSED
static void gc9503np_init(void)
{
    push_table(lcm_initialization_setting,
	      sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table));
}

static int lcd_initialize(void)
{
    g_lcd.getplaneinfo = lcdc_getplaneinfo;
    LCD_INFO("in lcd initialize1n");
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {GPIO_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
#ifdef GPIO_1V8_PIN
    pinmux[0].pin = GPIO_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_1V8_PIN);
#endif
#ifdef GPIO_2V8_PIN
    pinmux[0].pin = GPIO_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_2V8_PIN);
#endif
#ifdef GPIO_3V3_PIN
    pinmux[0].pin = GPIO_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_3V3_PIN);
#endif
    pinmux[0].pin = GPIO_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_RST_PIN);
#if defined(DISPLAY_RM69330)
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(1));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(100));
#elif defined(DISPLAY_GC9503NP)
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(20));
#endif
    LCD_INFO("in lcd initialize2n");

    hal_dsi_init(WIDTH);
    LCD_INFO("in lcd initialize!!!!!!!!!!!!!!!!\n");
#if defined(DISPLAY_RM69330)
    rm69330_init();
#elif defined(DISPLAY_GC9503NP)
    gc9503np_init();
#endif
    LCD_INFO("in lcd initialize!!!!!!!!!!!!!!!!\n");
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_dsi_start();
    LCD_INFO("in lcd initialize!!!!!!!!!!!!!!!!\n");
  return OK;
}

/**
 * Public Functions
 */

/**
 * board_lcd_uninitialize - Uninitialize the LCD Device.
 *
 */

void board_lcd_uninitialize(void)
{
  /* Set display off */
}


/**
 * board_lcd_getdev - Return a reference to the LCD object for the specified LCD Device.
 *   This allows support for multiple LCD devices.
 * @lcddev - Number of the LDC Device.
 *
 * Returns: Reference to the LCD object if exist otherwise NULL
 *
 */

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev)
{
  return &g_lcd;
}

/**
 * board_lcd_initialize
 *
 *   Initialize the LCD video hardware. The initial state of the LCD is
 *   fully initialized, display memory cleared, and the LCD ready to use, but
 *   with the power setting at 0 (full off).
 *
 */

int board_lcd_initialize(void)
{
  /* check if always initialized */

  /* Initialize the sub driver structure */
  lcd_initialize();

  return OK;
}

/**
 * up_fbinitialize - Initialize the framebuffer video hardware associated with the display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 * Returns: Zero is returned on success; a negated errno value is returned on any
 *   failure.
 */

int up_fbinitialize(int display)
{
  static bool initialized = false;
  int ret = OK;

  if (!initialized)
    {
      board_lcd_initialize();
      ret = lcdc_initialize(fb_buf,sizeof(fb_buf));
      initialized = (ret >= OK);
    }

  return ret;
}

/**
 * up_fbgetvplane -  Return a a reference to the framebuffer object for the specified video
 *   plane of the specified plane.  Many OSDs support multiple planes of video.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 * @vplane : Identifies the plane being queried.
 *
 * Returns: A non-NULL pointer to the frame buffer access structure is returned on
 *   success; NULL is returned on any failure.
 *
 */

FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{
  return lcdcgetvplane(vplane);
}

/**
 * up_fbuninitialize - Uninitialize the framebuffer support for the specified display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 */

void up_fbuninitialize(int display)
{
  lcdcuninitialize();
}
