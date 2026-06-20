/****************************************************************************
 * apps/examples/fb/fb_main.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

//#include <unistd.h>
//#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

#include "fb.h"
/****************************************************************************
 * Included Files
 ****************************************************************************/

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Color Creation and Conversion Macros *************************************/

/* This macro creates RGB24 from 8:8:8 RGB */

#define RGBTO24(r,g,b) \
  ((uint32_t)((r) & 0xff) << 16 | (uint32_t)((g) & 0xff) << 8 | (uint32_t)((b) & 0xff))

/* And these macros perform the inverse transformation */

#define RGB24RED(rgb)   (((rgb) >> 16) & 0xff)
#define RGB24GREEN(rgb) (((rgb) >> 8)  & 0xff)
#define RGB24BLUE(rgb)  ( (rgb)        & 0xff)

/* This macro creates RGB16 (5:6:5) from 8:8:8 RGB:
 *
 *   R[7:3] -> RGB[15:11]
 *   G[7:2] -> RGB[10:5]
 *   B[7:3] -> RGB[4:0]
 */

#define RGBTO16(r,g,b) \
  ((((uint16_t)(r) << 8) & 0xf800) | (((uint16_t)(g) << 3) & 0x07e0) | (((uint16_t)(b) >> 3) & 0x001f))

/* And these macros perform the inverse transformation */

#define RGB16RED(rgb)   (((rgb) >> 8) & 0xf8)
#define RGB16GREEN(rgb) (((rgb) >> 3) & 0xfc)
#define RGB16BLUE(rgb)  (((rgb) << 3) & 0xf8)

/* This macro creates RGB8 (3:3:2) from 8:8:8 RGB */

#define RGBTO8(r,g,b) \
  ((((uint8_t)(r) << 5) & 0xe0) | (((uint8_t)(g) << 2) & 0x1c) | ((uint8_t)(b) & 0x03))

/* And these macros perform the inverse transformation */

#define RGB8RED(rgb)    ( (rgb)       & 0xe0)
#define RGB8GREEN(rgb)  (((rgb) << 3) & 0xe0)
#define RGB8BLUE(rgb)   (((rgb) << 6) & 0xc0)

/* This macro converts RGB24 (8:8:8) to RGB16 (5:6:5):
 *
 *   00000000 RRRRRRRR BBBBBBBB GGGGGGGG -> RRRRRBBB BBBGGGGG
 */

#define RGB24TO16(rgb24) \
  (((rgb24 >> 8) & 0xf800) | ((rgb24 >> 5) & 0x07e0) | ((rgb24 >> 3) & 0x001f))

/* This macro converts RGB16 (5:6:5) to RGB24 (8:8:8):
 *
 *   RRRRRBBB BBBGGGGG -> 00000000 RRRRRRRR BBBBBBBB GGGGGGGG
 */

#define RGB16TO24(rgb16) \
  (((rgb16 & 0xf800) << 8) | ((rgb16 & 0x07e0) << 5)  | ((rgb16 & 0x001f) << 3))

/* Standard Color Definitions ***********************************************/

/* RGB24-888: 00000000 RRRRRRRR GGGGGGGG BBBBBBBB */

#define RGB24_BLACK          0x00000000
#define RGB24_WHITE          0x00ffffff

#define RGB24_BLUE           0x000000ff
#define RGB24_GREEN          0x0000ff00
#define RGB24_RED            0x00ff0000

#define RGB24_NAVY           0x00000080
#define RGB24_DARKBLUE       0x0000008b
#define RGB24_DARKGREEN      0x00006400
#define RGB24_DARKCYAN       0x00008b8b
#define RGB24_CYAN           0x0000ffff
#define RGB24_TURQUOISE      0x0040e0d0
#define RGB24_INDIGO         0x004b0082
#define RGB24_DARKRED        0x00800000
#define RGB24_OLIVE          0x00808000
#define RGB24_GRAY           0x00808080
#define RGB24_SKYBLUE        0x0087ceeb
#define RGB24_BLUEVIOLET     0x008a2be2
#define RGB24_LIGHTGREEN     0x0090ee90
#define RGB24_DARKVIOLET     0x009400d3
#define RGB24_YELLOWGREEN    0x009acd32
#define RGB24_BROWN          0x00a52a2a
#define RGB24_DARKGRAY       0x00a9a9a9
#define RGB24_SIENNA         0x00a0522d
#define RGB24_LIGHTBLUE      0x00add8e6
#define RGB24_GREENYELLOW    0x00adff2f
#define RGB24_SILVER         0x00c0c0c0
#define RGB24_LIGHTGREY      0x00d3d3d3
#define RGB24_LIGHTCYAN      0x00e0ffff
#define RGB24_VIOLET         0x00ee82ee
#define RGB24_AZUR           0x00f0ffff
#define RGB24_BEIGE          0x00f5f5dc
#define RGB24_MAGENTA        0x00ff00ff
#define RGB24_TOMATO         0x00ff6347
#define RGB24_GOLD           0x00ffd700
#define RGB24_ORANGE         0x00ffa500
#define RGB24_SNOW           0x00fffafa
#define RGB24_YELLOW         0x00ffff00

/* RGB16-565: RRRRRGGG GGGBBBBB */

#define RGB16_BLACK          0x0000
#define RGB16_WHITE          0xffff

#define RGB16_BLUE           0x001f
#define RGB16_GREEN          0x07e0
#define RGB16_RED            0xf800

#define RGB16_NAVY           0x0010
#define RGB16_DARKBLUE       0x0011
#define RGB16_DARKGREEN      0x0320
#define RGB16_DARKCYAN       0x0451
#define RGB16_CYAN           0x07ff
#define RGB16_TURQUOISE      0x471a
#define RGB16_INDIGO         0x4810
#define RGB16_DARKRED        0x8000
#define RGB16_OLIVE          0x8400
#define RGB16_GRAY           0x8410
#define RGB16_SKYBLUE        0x867d
#define RGB16_BLUEVIOLET     0x895c
#define RGB16_LIGHTGREEN     0x9772
#define RGB16_DARKVIOLET     0x901a
#define RGB16_YELLOWGREEN    0x9e66
#define RGB16_BROWN          0xa145
#define RGB16_DARKGRAY       0xad55
#define RGB16_SIENNA         0xa285
#define RGB16_LIGHTBLUE      0xaedc
#define RGB16_GREENYELLOW    0xafe5
#define RGB16_SILVER         0xc618
#define RGB16_LIGHTGREY      0xd69a
#define RGB16_LIGHTCYAN      0xe7ff
#define RGB16_VIOLET         0xec1d
#define RGB16_AZUR           0xf7ff
#define RGB16_BEIGE          0xf7bb
#define RGB16_MAGENTA        0xf81f
#define RGB16_TOMATO         0xfb08
#define RGB16_GOLD           0xfea0
#define RGB16_ORANGE         0xfd20
#define RGB16_SNOW           0xffdf
#define RGB16_YELLOW         0xffe0

/* RGB12-444: RRRR GGGGBBBB */

#define RGB12_BLACK          0x0000
#define RGB12_WHITE          0x0fff

#define RGB12_BLUE           0x000f
#define RGB12_GREEN          0x00f0
#define RGB12_RED            0x0f00

#define RGB12_NAVY           0x0008
#define RGB12_DARKBLUE       0x0009
#define RGB12_DARKGREEN      0x0060
#define RGB12_DARKCYAN       0x0099
#define RGB12_CYAN           0x00ff
#define RGB12_TURQUOISE      0x04ed
#define RGB12_INDIGO         0x0508
#define RGB12_DARKRED        0x0800
#define RGB12_OLIVE          0x0880
#define RGB12_GRAY           0x0888
#define RGB12_SKYBLUE        0x08df
#define RGB12_BLUEVIOLET     0x093e
#define RGB12_LIGHTGREEN     0x09f9
#define RGB12_DARKVIOLET     0x090d
#define RGB12_YELLOWGREEN    0x0ad3
#define RGB12_BROWN          0x0a33
#define RGB12_DARKGRAY       0x0bbb
#define RGB12_SIENNA         0x0a53
#define RGB12_LIGHTBLUE      0x0bee
#define RGB12_GREENYELLOW    0x0bf3
#define RGB12_SILVER         0x0ccc
#define RGB12_LIGHTGREY      0x0ddd
#define RGB12_LIGHTCYAN      0x0eff
#define RGB12_VIOLET         0x0f8f
#define RGB12_AZUR           0x0fff
#define RGB12_BEIGE          0x0ffe
#define RGB12_MAGENTA        0x0f0f
#define RGB12_TOMATO         0x0f64
#define RGB12_GOLD           0x0fd0
#define RGB12_ORANGE         0x0fa0
#define RGB12_SNOW           0x0fff
#define RGB12_YELLOW         0x0ff0

/* RGB8-332: RRRGGGBB
 * (really not enough color resolution for the following)
 */

#define RGB8_BLACK           0x00
#define RGB8_WHITE           0xff

#define RGB8_BLUE            0x03
#define RGB8_GREEN           0x1c
#define RGB8_RED             0xe0
#define RGB8_NAVY            0x02
#define RGB8_DARKBLUE        0x02
#define RGB8_DARKGREEN       0x0c
#define RGB8_DARKCYAN        0x16
#define RGB8_CYAN            0x1f
#define RGB8_TURQUOISE       0x5f
#define RGB8_INDIGO          0x62
#define RGB8_DARKRED         0x80
#define RGB8_OLIVE           0x90
#define RGB8_GRAY            0x92
#define RGB8_SKYBLUE         0x9f
#define RGB8_BLUEVIOLET      0xab
#define RGB8_LIGHTGREEN      0xbe
#define RGB8_DARKVIOLET      0x93
#define RGB8_YELLOWGREEN     0x9d
#define RGB8_BROWN           0xa9
#define RGB8_DARKGRAY        0xdb
#define RGB8_SIENNA          0xa9
#define RGB8_LIGHTBLUE       0xdf
#define RGB8_GREENYELLOW     0xdd
#define RGB8_SILVER          0xd9
#define RGB8_LIGHTGREY       0xd9
#define RGB8_LIGHTCYAN       0xff
#define RGB8_VIOLET          0xf3
#define RGB8_AZUR            0xff
#define RGB8_BEIGE           0xff
#define RGB8_MAGENTA         0xed
#define RGB8_TOMATO          0xfc
#define RGB8_GOLD            0xfc
#define RGB8_ORANGE          0xf8
#define RGB8_SNOW            0xff
#define RGB8_YELLOW          0xfc

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
/****************************************************************************
 * Public Types
 ****************************************************************************/

#ifndef __ASSEMBLY__

/* This is a generic representation of an RGB color */

struct rgbcolor_s
{
  uint8_t r;  /* Red value */
  uint8_t b;  /* Blue value */
  uint8_t g;  /* Green value */
};

/* This is a generic representation of an RGB color with ALPHA */

struct argbcolor_s
{
  uint8_t a;  /* Alpha value (transparency) */
  uint8_t r;  /* Red value */
  uint8_t b;  /* Blue value */
  uint8_t g;  /* Green value */
};

/****************************************************************************
 * Public Data
 ****************************************************************************/

#ifdef __cplusplus
#define EXTERN extern "C"
extern "C"
{
#else
#define EXTERN extern
#endif

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

#undef EXTERN
#ifdef __cplusplus
}
#endif

#endif /* __ASSEMBLY__ */

/****************************************************************************
 * Preprocessor Definitions
 ****************************************************************************/

#define NCOLORS 6

/****************************************************************************
 * Private Types
 ****************************************************************************/

struct fb_state_s
{
  int fd;
  struct fb_videoinfo_s vinfo;
  struct fb_planeinfo_s pinfo;
#ifdef CONFIG_FB_OVERLAY
  struct fb_overlayinfo_s oinfo;
#endif
  FAR void *fbmem;
};

/****************************************************************************
 * Private Data
 ****************************************************************************/

/* Violet-Blue-Green-Yellow-Orange-Red */

static const uint32_t g_rgb24[NCOLORS] =
{
  RGB24_VIOLET, RGB24_BLUE, RGB24_GREEN,
  RGB24_YELLOW, RGB24_ORANGE, RGB24_RED
};

static const uint16_t g_rgb16[NCOLORS] =
{
  RGB16_VIOLET, RGB16_BLUE, RGB16_GREEN,
  RGB16_YELLOW, RGB16_ORANGE, RGB16_RED
};

static const uint8_t g_rgb8[NCOLORS] =
{
  RGB8_VIOLET, RGB8_BLUE, RGB8_GREEN,
  RGB8_YELLOW, RGB8_ORANGE, RGB8_RED
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * draw_rect
 ****************************************************************************/

static void draw_rect32(FAR struct fb_state_s *state,
                        FAR struct fb_area_s *area, int color)
{
  FAR uint32_t *dest;
  FAR uint8_t *row;
  int x;
  int y;

  row = (FAR uint8_t *)state->fbmem + state->pinfo.stride*state->pinfo.yoffset + state->pinfo.stride * area->y;
  for (y = 0; y < area->h; y++)
    {
      dest = ((FAR uint32_t *)row) + area->x;
      for (x = 0; x < area->w; x++)
        {
          *dest++ = g_rgb24[color];
        }

      row += state->pinfo.stride;
    }
}

static void draw_rect16(FAR struct fb_state_s *state,
                        FAR struct fb_area_s *area, int color)
{
  FAR uint16_t *dest;
  FAR uint8_t *row;
  int x;
  int y;

  row = (FAR uint8_t *)state->fbmem + state->pinfo.stride * area->y;
  for (y = 0; y < area->h; y++)
    {
      dest = ((FAR uint16_t *)row) + area->x;
      for (x = 0; x < area->w; x++)
        {
          *dest++ = g_rgb16[color];
        }

      row += state->pinfo.stride;
    }
}

static void draw_rect8(FAR struct fb_state_s *state,
                       FAR struct fb_area_s *area, int color)
{
  FAR uint8_t *dest;
  FAR uint8_t *row;
  int x;
  int y;

  row = (FAR uint8_t *)state->fbmem + state->pinfo.stride * area->y;
  for (y = 0; y < area->h; y++)
    {
      dest = row + area->x;
      for (x = 0; x < area->w; x++)
        {
          *dest++ = g_rgb8[color];
        }

      row += state->pinfo.stride;
    }
}

static void draw_rect1(FAR struct fb_state_s *state,
                       FAR struct fb_area_s *area, int color)
{
  FAR uint8_t *pixel;
  FAR uint8_t *row;
  uint8_t color8 = (color & 1) == 0 ? 0 : 0xff;
  uint8_t lmask;
  uint8_t rmask;
  int startx;
  int endx;
  int x;
  int y;

  /* Calculate the framebuffer address of the first row to draw on */

  row    = (FAR uint8_t *)state->fbmem + state->pinfo.stride * area->y;

  /* Calculate the start byte position rounding down so that we get the
   * first byte containing any part of the pixel sequence.  Then calculate
   * the last byte position with a ceil() operation so it includes any final
   * final pixels of the sequence.
   */

  startx = (area->x >> 3);
  endx   = ((area->x + area->w + 6) >> 3);

  /* Calculate a mask on the first and last bytes of the sequence that may
   * not be completely filled with pixel.
   */

  lmask  = 0xff << (8 - (area->x & 7));
  rmask  = 0xff >> ((area->x + area->w - 1) & 7);

  /* Now draw each row, one-at-a-time */

  for (y = 0; y < area->h; y++)
    {
      /* 'pixel' points to the 1st pixel the next row */

      pixel = row + startx;

      /* Special case: The row is less no more than one byte wide */

      if (startx == endx)
        {
          uint8_t mask = lmask | rmask;

          *pixel = (*pixel & mask) | (color8 & ~mask);
        }
      else
        {
          /* Special case the first byte of the row */

          *pixel = (*pixel & lmask) | (color8 & ~lmask);
          pixel++;

          /* Handle all middle bytes in the row */

          for (x = startx + 1; x < endx; x++)
            {
              *pixel++ = color8;
            }

          /* Handle the final byte of the row */

          *pixel = (*pixel & rmask) | (color8 & ~rmask);
        }

      row += state->pinfo.stride;
    }
}
int fb_ioctl(uint32_t fd, int cmd, unsigned long arg);

static void draw_rect(FAR struct fb_state_s *state,
                      FAR struct fb_area_s *area, int color)
{
#ifdef CONFIG_FB_UPDATE
  int ret;
#endif

  switch (state->pinfo.bpp)
    {
      case 32:
        draw_rect32(state, area, color);
        break;

      case 16:
        draw_rect16(state, area, color);
        break;

      case 8:
      default:
        draw_rect8(state, area, color);
        break;

      case 1:
        draw_rect1(state, area, color);
        break;
    }

#ifdef CONFIG_FB_UPDATE
  ret = fb_ioctl(state->fd, FBIO_UPDATE,
              (unsigned long)((uintptr_t)area));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIO_UPDATE) failed: %d\n",
              errcode);
    }
#endif
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * fb_main
 ****************************************************************************/
#define PRIu8       "u"
#define PRIu16      "u"
#define PRIu32      "lu"
#define PRIu64      "llu"

#define PRIuPTR     "u"
    
#define PRIx8       "x"
#define PRIx16      "x"
#define PRIx32      "lx"
#define PRIx64      "llx"

int fb_single_rect_draw_test(uint32_t fb_fd)
{
  struct fb_state_s state;
  struct fb_area_s area;
  int ret;

  /* There is a single required argument:  The path to the framebuffer
   * driver.
   */

  /* Open the framebuffer driver */

  state.fd = fb_fd;
  if (state.fd < 0)
    {
      return EXIT_FAILURE;
    }

  /* Get the characteristics of the framebuffer */

  ret = fb_ioctl(state.fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&state.vinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_VIDEOINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("VideoInfo:\n");
  FB_REP("      fmt: %u\n", state.vinfo.fmt);
  FB_REP("     xres: %u\n", state.vinfo.xres);
  FB_REP("     yres: %u\n", state.vinfo.yres);
  FB_REP("  nplanes: %u\n", state.vinfo.nplanes);
#endif

#ifdef CONFIG_FB_OVERLAY
#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("noverlays: %u\n", state.vinfo.noverlays);
#endif

  /* Select the first overlay, which should be the composed framebuffer */
  state.oinfo.overlay = 0;
  ret = fb_ioctl(state.fd, FBIO_SELECT_OVERLAY, state.oinfo.overlay);
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIO_SELECT_OVERLAY) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  ret = fb_ioctl(state.fd, FBIOGET_OVERLAYINFO,
                        (unsigned long)((uintptr_t)&state.oinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_OVERLAYINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("OverlayInfo (overlay 0):\n");
  FB_REP("    fbmem: %p\n", state.oinfo.fbmem);
  FB_REP("    fblen: %lu\n", (unsigned long)state.oinfo.fblen);
  FB_REP("   stride: %u\n", state.oinfo.stride);
  FB_REP("  overlay: %u\n", state.oinfo.overlay);
  FB_REP("      bpp: %u\n", state.oinfo.bpp);
  FB_REP("    blank: %u\n", state.oinfo.blank);
  FB_REP("chromakey: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.chromakey);
  FB_REP("    color: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.color);
  FB_REP("   transp: 0x%02x\n", state.oinfo.transp.transp);
  FB_REP("     mode: %u\n", state.oinfo.transp.transp_mode);
  FB_REP("     area: (%u,%u) => (%u,%u)\n",
                      state.oinfo.sarea.x, state.oinfo.sarea.y,
                      state.oinfo.sarea.w, state.oinfo.sarea.h);
  FB_REP("     accl: %" PRIu32 "\n", (unsigned long)state.oinfo.accl);
#endif

#endif

  ret = fb_ioctl(state.fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&state.pinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("PlaneInfo (plane 0):\n");
  FB_REP("    fbmem: %p\n", state.pinfo.fbmem);
  FB_REP("    yoffset: %p\n", state.pinfo.yoffset);
  FB_REP("    fblen: %lu\n", (unsigned long)state.pinfo.fblen);
  FB_REP("   stride: %u\n", state.pinfo.stride);
  FB_REP("  display: %u\n", state.pinfo.display);
  FB_REP("      bpp: %u\n", state.pinfo.bpp);
#endif

  /* Only these pixel depths are supported.  viinfo.fmt is ignored, only
   * certain color formats are supported.
   */

  if (state.pinfo.bpp != 32 && state.pinfo.bpp != 16 &&
      state.pinfo.bpp != 8  && state.pinfo.bpp != 1)
    {
      FB_REP("ERROR: bpp=%u not supported\n", state.pinfo.bpp);
      return EXIT_FAILURE;
    }

  /* mmap() the framebuffer.
   *
   * NOTE: In the FLAT build the frame buffer address returned by the
   * FBIOGET_PLANEINFO fb_ioctl command will be the same as the framebuffer
   * address.  mmap(), however, is the preferred way to get the framebuffer
   * address because in the KERNEL build, it will perform the necessary
   * address mapping to make the memory accessible to the application.
   */

  //state.fbmem = mmap(NULL, state.pinfo.fblen, PROT_READ | PROT_WRITE,
  //                   MAP_SHARED | MAP_FILE, state.fd, 0);
  state.fbmem = state.pinfo.fbmem;
  if (0)//state.fbmem == MAP_FAILED)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("Mapped FB: %p\n", state.fbmem);
#endif

  /* Draw some rectangles */
  static int color = 0;

  area.x = 0;
  area.y = 0;
  area.w = state.vinfo.xres;
  area.h = state.vinfo.yres;

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("%2d: (%3d,%3d) (%3d,%3d)\n",
          color, area.x, area.y, area.w, area.h);
#endif

#ifdef CONFIG_FB_PANDISPLAY
  state.pinfo.yoffset = (state.pinfo.yoffset == 0)
                  ? state.vinfo.yres : 0;
  //draw_rect(&state, &area, color);
  draw_rect32(&state, &area, color);
  fb_ioctl(state.fd , FBIOPAN_DISPLAY,
        (unsigned long)((uintptr_t)&state.pinfo));
#else
  draw_rect(&state, &area, color);
#endif
  color++;
  if (color >= NCOLORS) {
    color = 0;
  }

#ifdef SINGLE_RECT_TEST_TRACE
  FB_REP("Test finished\n");
#endif
  return EXIT_SUCCESS;
}


int fb_rect_draw_test(uint32_t fb_fd)
{
  struct fb_state_s state;
  struct fb_area_s area;
  int nsteps;
  int xstep;
  int ystep;
  int width;
  int height;
  int color;
  int x;
  int y;
  int ret;

  /* There is a single required argument:  The path to the framebuffer
   * driver.
   */

  /* Open the framebuffer driver */

  state.fd = fb_fd;
  if (state.fd < 0)
    {
      return EXIT_FAILURE;
    }

  /* Get the characteristics of the framebuffer */

  ret = fb_ioctl(state.fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&state.vinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_VIDEOINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("VideoInfo:\n");
  FB_REP("      fmt: %u\n", state.vinfo.fmt);
  FB_REP("     xres: %u\n", state.vinfo.xres);
  FB_REP("     yres: %u\n", state.vinfo.yres);
  FB_REP("  nplanes: %u\n", state.vinfo.nplanes);

#ifdef CONFIG_FB_OVERLAY
  FB_REP("noverlays: %u\n", state.vinfo.noverlays);

  /* Select the first overlay, which should be the composed framebuffer */
  state.oinfo.overlay = 0;
  ret = fb_ioctl(state.fd, FBIO_SELECT_OVERLAY, state.oinfo.overlay);
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIO_SELECT_OVERLAY) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  ret = fb_ioctl(state.fd, FBIOGET_OVERLAYINFO,
                        (unsigned long)((uintptr_t)&state.oinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_OVERLAYINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("OverlayInfo (overlay 0):\n");
  FB_REP("    fbmem: %p\n", state.oinfo.fbmem);
  FB_REP("    fblen: %lu\n", (unsigned long)state.oinfo.fblen);
  FB_REP("   stride: %u\n", state.oinfo.stride);
  FB_REP("  overlay: %u\n", state.oinfo.overlay);
  FB_REP("      bpp: %u\n", state.oinfo.bpp);
  FB_REP("    blank: %u\n", state.oinfo.blank);
  FB_REP("chromakey: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.chromakey);
  FB_REP("    color: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.color);
  FB_REP("   transp: 0x%02x\n", state.oinfo.transp.transp);
  FB_REP("     mode: %u\n", state.oinfo.transp.transp_mode);
  FB_REP("     area: (%u,%u) => (%u,%u)\n",
                      state.oinfo.sarea.x, state.oinfo.sarea.y,
                      state.oinfo.sarea.w, state.oinfo.sarea.h);
  FB_REP("     accl: %" PRIu32 "\n", (unsigned long)state.oinfo.accl);

#endif

  ret = fb_ioctl(state.fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&state.pinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("PlaneInfo (plane 0):\n");
  FB_REP("    fbmem: %p\n", state.pinfo.fbmem);
  FB_REP("    fblen: %lu\n", (unsigned long)state.pinfo.fblen);
  FB_REP("   stride: %u\n", state.pinfo.stride);
  FB_REP("  display: %u\n", state.pinfo.display);
  FB_REP("      bpp: %u\n", state.pinfo.bpp);

  /* Only these pixel depths are supported.  viinfo.fmt is ignored, only
   * certain color formats are supported.
   */

  if (state.pinfo.bpp != 32 && state.pinfo.bpp != 16 &&
      state.pinfo.bpp != 8  && state.pinfo.bpp != 1)
    {
      FB_REP("ERROR: bpp=%u not supported\n", state.pinfo.bpp);
      return EXIT_FAILURE;
    }

  /* mmap() the framebuffer.
   *
   * NOTE: In the FLAT build the frame buffer address returned by the
   * FBIOGET_PLANEINFO fb_ioctl command will be the same as the framebuffer
   * address.  mmap(), however, is the preferred way to get the framebuffer
   * address because in the KERNEL build, it will perform the necessary
   * address mapping to make the memory accessible to the application.
   */

  //state.fbmem = mmap(NULL, state.pinfo.fblen, PROT_READ | PROT_WRITE,
  //                   MAP_SHARED | MAP_FILE, state.fd, 0);
  state.fbmem = state.pinfo.fbmem;
  if (0)//state.fbmem == MAP_FAILED)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("Mapped FB: %p\n", state.fbmem);

  /* Draw some rectangles */

  nsteps = 2 * (NCOLORS - 1) + 1;
  xstep  = state.vinfo.xres / nsteps;
  ystep  = state.vinfo.yres / nsteps;
  width  = state.vinfo.xres;
  height = state.vinfo.yres;

  for (x = 0, y = 0, color = 0;
       color < NCOLORS;
       x += xstep, y += ystep, color++)
    {
      area.x = x;
      area.y = y;
      area.w = width;
      area.h = height;

      FB_REP("%2d: (%3d,%3d) (%3d,%3d)\n",
             color, area.x, area.y, area.w, area.h);

      draw_rect(&state, &area, color);
      hal_sys_timer_delay_us(500 * 1000);

      width  -= (2 * xstep);
      height -= (2 * ystep);
    }

  FB_REP("Test finished\n");
  return EXIT_SUCCESS;
}


int fb_buffer_draw_test(uint32_t fb_fd,uint8_t *buffer)
{
  struct fb_state_s state;
  struct fb_area_s area;
  int width;
  int height;
  int ret;

  /* There is a single required argument:  The path to the framebuffer
   * driver.
   */

  /* Open the framebuffer driver */

  state.fd = fb_fd;
  if (state.fd < 0)
    {
      return EXIT_FAILURE;
    }

  /* Get the characteristics of the framebuffer */

  ret = fb_ioctl(state.fd, FBIOGET_VIDEOINFO,
              (unsigned long)((uintptr_t)&state.vinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_VIDEOINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("VideoInfo:\n");
  FB_REP("      fmt: %u\n", state.vinfo.fmt);
  FB_REP("     xres: %u\n", state.vinfo.xres);
  FB_REP("     yres: %u\n", state.vinfo.yres);
  FB_REP("  nplanes: %u\n", state.vinfo.nplanes);

#ifdef CONFIG_FB_OVERLAY
  FB_REP("noverlays: %u\n", state.vinfo.noverlays);

  /* Select the first overlay, which should be the composed framebuffer */
  state.oinfo.overlay = 0;
  ret = fb_ioctl(state.fd, FBIO_SELECT_OVERLAY, state.oinfo.overlay);
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIO_SELECT_OVERLAY) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  ret = fb_ioctl(state.fd, FBIOGET_OVERLAYINFO,
                        (unsigned long)((uintptr_t)&state.oinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_OVERLAYINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("OverlayInfo (overlay 0):\n");
  FB_REP("    fbmem: %p\n", state.oinfo.fbmem);
  FB_REP("    fblen: %lu\n", (unsigned long)state.oinfo.fblen);
  FB_REP("   stride: %u\n", state.oinfo.stride);
  FB_REP("  overlay: %u\n", state.oinfo.overlay);
  FB_REP("      bpp: %u\n", state.oinfo.bpp);
  FB_REP("    blank: %u\n", state.oinfo.blank);
  FB_REP("chromakey: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.chromakey);
  FB_REP("    color: 0x%08" PRIx32 "\n", (unsigned long)state.oinfo.color);
  FB_REP("   transp: 0x%02x\n", state.oinfo.transp.transp);
  FB_REP("     mode: %u\n", state.oinfo.transp.transp_mode);
  FB_REP("     area: (%u,%u) => (%u,%u)\n",
                      state.oinfo.sarea.x, state.oinfo.sarea.y,
                      state.oinfo.sarea.w, state.oinfo.sarea.h);
  FB_REP("     accl: %" PRIu32 "\n", (unsigned long)state.oinfo.accl);

#endif

  ret = fb_ioctl(state.fd, FBIOGET_PLANEINFO,
              (unsigned long)((uintptr_t)&state.pinfo));
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("PlaneInfo (plane 0):\n");
  FB_REP("    fbmem: %p\n", state.pinfo.fbmem);
  FB_REP("    fblen: %lu\n", (unsigned long)state.pinfo.fblen);
  FB_REP("   stride: %u\n", state.pinfo.stride);
  FB_REP("  display: %u\n", state.pinfo.display);
  FB_REP("      bpp: %u\n", state.pinfo.bpp);

  /* Only these pixel depths are supported.  viinfo.fmt is ignored, only
   * certain color formats are supported.
   */

  if (state.pinfo.bpp != 32 && state.pinfo.bpp != 16 &&
      state.pinfo.bpp != 8  && state.pinfo.bpp != 1)
    {
      FB_REP("ERROR: bpp=%u not supported\n", state.pinfo.bpp);
      return EXIT_FAILURE;
    }

  /* mmap() the framebuffer.
   *
   * NOTE: In the FLAT build the frame buffer address returned by the
   * FBIOGET_PLANEINFO fb_ioctl command will be the same as the framebuffer
   * address.  mmap(), however, is the preferred way to get the framebuffer
   * address because in the KERNEL build, it will perform the necessary
   * address mapping to make the memory accessible to the application.
   */

  //state.fbmem = mmap(NULL, state.pinfo.fblen, PROT_READ | PROT_WRITE,
  //                   MAP_SHARED | MAP_FILE, state.fd, 0);
  state.fbmem = state.pinfo.fbmem;
  if (0)//state.fbmem == MAP_FAILED)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIOGET_PLANEINFO) failed: %d\n",
              errcode);
      return EXIT_FAILURE;
    }

  FB_REP("Mapped FB: %p\n", state.fbmem);


//test display picture
  width  = state.vinfo.xres;
  height = state.vinfo.yres;

  area.x = 0;
  area.y = 0;
  area.w = width;
  area.h = height;
  
  FB_REP("++++++++++++++++++++++++++++===========display picture: (%3d,%3d) (%3d,%3d)\n",
         area.x, area.y, area.w, area.h);

  if( buffer!= NULL) {
  #if 0
      uint32_t offset_lcd = 0;
      uint32_t offset_gpu = 0;  
      for(int i=0; i < 454; i++){
        memcpy((FAR uint8_t *)(state->fbmem+offset_lcd),(uint8_t *)(buffer+offset_gpu),454*4);
        offset_lcd+=(454*4);
        offset_gpu+=(464*4);
      }
  #else
      memcpy((FAR uint8_t *)state.fbmem,buffer,area.w*area.h*4);
  #endif
  }
  #ifdef CONFIG_FB_UPDATE
  ret = fb_ioctl(state.fd, FBIO_UPDATE,
              (unsigned long)((uintptr_t)&area));
  
  if (ret < 0)
    {
      int errcode = -1;
      FB_REP("ERROR: fb_ioctl(FBIO_UPDATE) failed: %d\n",
              errcode);
    }
  #endif

  FB_REP("Test finished\n");
  return EXIT_SUCCESS;
}
