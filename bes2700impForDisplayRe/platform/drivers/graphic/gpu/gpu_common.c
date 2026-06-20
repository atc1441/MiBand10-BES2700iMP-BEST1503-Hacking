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
#include <stdio.h>
#include <string.h>
#include "cmsis_nvic.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "heap_api.h"
#include "gpu_port.h"
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hw.h"
#include "vg_lite_hal.h"
#include "vg_lite_platform.h"
#include "hal_location.h"
#include "gpu_port.h"
#include "gpu_common.h"



#define GPU_LCDC_FB_BUFFER   M55_SYS_RAM_BASE+454*464*2




extern void vg_lite_get_cmdbuf_info(uint32_t *start, uint32_t *bytes);
extern uint32_t vg_lite_get_tsbuffer_size(void);
extern int fb_lcdc_display_buffer_test(uint8_t *buffer);



int gpu_lcdc_display(vg_lite_buffer_t * buffer)
{
#ifdef TEST_WITH_LCD
    TC_INIT

    vg_lite_buffer_t sram_fb;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    
    filter = VG_LITE_FILTER_POINT;

    memcpy(&sram_fb,buffer, sizeof(vg_lite_buffer_t));

    sram_fb.memory = GPU_LCDC_FB_BUFFER;
    sram_fb.address = GPU_LCDC_FB_BUFFER;

    vg_lite_identity(&matrix);
    TC_START
    vg_lite_blit(&sram_fb, buffer, &matrix, VG_LITE_BLEND_NONE, 0, filter);

    vg_lite_finish();
    TC_END
    GPU_REP("CALCULATE BLIT COST TIME######");
    TC_REP(test_blit);

    GPU_REP("###Render size: stride %d  %d x %d\n", sram_fb.stride,sram_fb.width, sram_fb.height);
    cpu_gpu_data_cache_invalid(sram_fb.memory, sram_fb.height*sram_fb.stride);
    GPU_REP("****%s %d fb: %p", __func__, __LINE__, sram_fb.memory);
#if 1//dsi    
    fb_lcdc_display_buffer_test(NULL);//if use gpu mem copy,no need cpu copy
#else//qspi rgb 565
    uint16_t *lptr;
    uint16_t pix;

    lptr = (uint16_t*)sfb.memory;
    for(int i=0; i < sfb.height; i++){
        lptr = (uint16_t*)(i*sfb.stride + (uint32_t)sfb.memory);
        for(int j=0; j<sfb.stride/2; j++ ){
            pix = lptr[j];
            lptr[j] = ((pix & 0xff) << 8) | ((pix &0xff00) >> 8);
        }
    }
    qspi_display_frame_buf(&sfb);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(50));
#endif
    return 0;

}


int gpu_save_raw(const char *name, vg_lite_buffer_t *buffer)
{
#ifdef TEST_WITH_FS
    return vg_lite_save_raw(name, buffer);
#endif     
}


/**
 * gpu_save_cmdbuffer - save gpu cmd buffer to file
 * vg_lite_get_tsbuffer_size
 */
void gpu_save_cmdbuffer(char * name)
{
    char line[128];
    uint32_t start;
    uint32_t *data;
    uint32_t bytes;
    uint32_t itme;
    //FILE * fp;

    vg_lite_get_cmdbuf_info(&start, &bytes);

    data = (uint32_t *)start;
    itme = bytes/4 ;
#ifdef TEST_WITH_FS
    FILE * fp;
    fp = fopen(name, "wb");

    if (fp != NULL) {
         sprintf(line," \\\\ tsb size:%d \r\n ",vg_lite_get_tsbuffer_size());
         fwrite(line, 1, strlen(line), fp);
         sprintf(line,"\\\\ cbaddr:%p cbsize:%d \r\n ", data, bytes + 8);
         fwrite(line, 1, strlen(line), fp);
         sprintf(line,"%s", "__aligned(64) uint32_t cmd_buffer[]={ \r\n ");
         fwrite(line, 1, strlen(line), fp);

         for (int i = 0; i < itme ; i++){
         sprintf(line,"%x, \r\n ", *(data + i));
         fwrite(line, 1, strlen(line), fp);

         }

         /*append end cmd*/
         sprintf(line,"%x, \r\n ", 0);
         fwrite(line, 1, strlen(line), fp);

         sprintf(line, "%x \r\n", 0);
         fwrite(line, 1, strlen(line), fp);

         sprintf(line,"%s", "};\r\n");

         fwrite(line, 1, strlen(line), fp);

         fclose(fp);
    }
 #else
         TRACE(3," \\\\ tsb size:%d \r\n ",vg_lite_get_tsbuffer_size());
         TRACE(3,"\\\\ cbaddr:%p cbsize:%d \r\n ", data, bytes + 8);
         TRACE(3,"%s", "__aligned(64) uint32_t cmd_buffer[]={ \r\n ");

         for (int i = 0; i < itme ; i++){
         TRACE(3,"%x, \r\n ", *(data + i));
         }

         /*append end cmd*/
         TRACE(3,"%x, \r\n ", 0);

         TRACE(3, "%x \r\n", 0);

         TRACE(3,"%s", "};\r\n");
 #endif

}


void gpu_set_soft_break_point()
{
#ifdef TEST_WITH_SOFTBP
      extern void set_soft_break_point(void);
      set_soft_break_point();
#endif
}



