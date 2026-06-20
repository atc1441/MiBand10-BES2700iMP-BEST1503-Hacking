/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
 * DOC: GAMU is a graphical memory address map unit:

• Reduce memory usage to the circal displayer
• Physically store only visible pixel
• Up to 4 virtual buffers
• Each virtual buffer have 2048 bytes per line and 512 lines
• Each virtual buffer can be physically mapped to any system memory
*/

#include "hal_gamu.h"
#include "cmsis_nvic.h"
#include "reg_gamu.h"
#include "hal_trace.h"
#include "hal_cmu.h"
#include "stdio.h"
#include "string.h"


#ifdef GAMU_BASE


//#define GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define GAMU_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define GAMU_REG(...) TR_INFO(TR_MOD(TEST), __VA_ARGS__)

#else
#define GAMU_INFO(...)
#define GAMU_REG(...)

#endif

#define GAMU_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define GAMU_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)


#define GAMU_PF_BYPP(n) ((n) / 8)


#define getreg32(a)          (*(volatile uint32_t *)(a))

#define putreg32(v,a)        do { \
                                   GAMU_REG("putreg32(0x%x,0x%x);",(v), (a)) ; \
                                   *(volatile uint32_t *)(a) = (v); \
                              } while(0)

#define reg32_bitset(v,a)    do { \
                                   GAMU_REG("reg32_bitset(0x%x,0x%x);", v, a); \
                                   *(volatile uint32_t *)(a) |= v; \
                              } while(0)

#define reg32_bitclr(v,a)    do { \
                                   GAMU_REG("reg32_bitclr(0x%x,0x%x);", v, a); \
                                   *(volatile uint32_t *)(a) &= ~(v); \
                              } while(0)


#define square(n)   (n * n)

#define MAX_GAMU_BUFFER_NUM      4
#define BLKSIZE      16

typedef struct {
    uint16_t line;
    bool ena;
    uint8_t  first_vblock;
    uint8_t  last_vblock;
    int16_t  offset;
    uint32_t used_vblocks;
}gamu_lut_entry_t;

typedef struct {
    uint32_t  start_addr;
    uint32_t  used;
}gamu_virtual_mem_t;

static gamu_virtual_mem_t gamu_vmem ={
    .start_addr = GAMU_VIRTUAL_ADDRSPACE_BASE,
    .used = 0,
};


/*
 * gamu_write_lut_entry - setup buffer line configure.
 * @entry    : line index (0 - 511)
 * @ena      : line enable
 * @fblock   : first block number
 * @lblock   : last block number
 * @loffset  : line block offest
*/
static void gamu_write_lut_entry(gamu_lut_entry_t *entry)
{
    uint32_t addr = 0;
    uint32_t value = 0;
    addr = GAMU_LUT_ENTRY_X_ADDR(entry->line);
    value = GAMU_LUT_ENTRY_X_ENA(entry->ena) | GAMU_LUT_ENTRY_X_START_BLOCK(entry->first_vblock)
            | GAMU_LUT_ENTRY_X_END_BLOCK(entry->last_vblock)
            | GAMU_LUT_ENTRY_X_BLOCK_OFFSET(entry->offset);
    putreg32(value,addr);
}

/*
 * gamu_circal_linemem_length - calculate the visible bytes of current line
   @line  : line number
   @fb    : gamu buffer context reference
*/

static uint16_t gamu_circal_linemem_length(uint16_t line, gamu_fb_info_t* fb )
{
    uint32_t length = 0;
    uint32_t liner = 0;
    uint16_t loffest = fb->yoffset;
    if (line <  loffest || (line > (fb->radius * 2 + loffest))){
        return 0;
    }

    liner =line - loffest;
    length = square(fb->radius) - square((fb->radius - liner));
    length = (uint32_t)fb->sqrtf(length);

    length *= fb->bpp/4;
    length += 16*2;
    if (length > fb->stride){
        length = fb->stride;
    }
    return (uint16_t)length;
}

/*
 * gamu_buffer_enable
   @id    : gamu buffer number
   @bbp    : 00 -  32bits
             01 -  24
             02 -  16
*/
static void gamu_buffer_enable(uint8_t id, uint8_t bpp)
{
    uint32_t addr = GAMU_CR_ADDR;
    uint32_t value = 0;
    uint8_t cfmt = 0;

    if (bpp == 32) cfmt = 0;
    if (bpp == 24) cfmt = 1;
    if (bpp == 16) cfmt = 2;
    value |= GAMU_PHYADDR_SELECT(id);
    value &= ~GAMU_VVM_ENA_MASK;
    value |= GAMU_VVM_ENA(1);
    value &= ~GAMU_PHYADDR_SELECT_ENA_MASK;
    value |= GAMU_PHYADDR_SELECT_ENA(0);
    //value |= GAMU_PHYADDR_SELECT_ENA(1);
    value &= ~GAMU_CROSS_1KENA_MASK;
    value |= GAMU_CROSS_1K_ENA(1);
    value &= ~GAMU_RGB_INPUT_FMT_MASK;
    value |= GAMU_RGB_INPUT_FMT(cfmt);
    putreg32(value,addr);
}


static void gamu_set_haddr_unvalid(uint32_t unvalidaddr)
{
    uint32_t addr = GAMU_HADDR_UNVALID_CR_ADDR;
    uint32_t value = unvalidaddr;
    putreg32(value,addr);
}

/*
 * gamu_set_buffer_phyaddress
   @id  : gamu buffer number
   @phyaddr    : gamu buffer start physical mem address
*/
static void gamu_set_buffer_phyaddress(uint8_t id, uint32_t phyaddr)
{
    uint32_t addr = GAMU_BUFFERx_PHYADDR_CR_ADDR(id);
    uint32_t value = GAMU_BUFFER_X_PHYADDR(phyaddr);
    putreg32(value,addr);

}


/*
 * gamu_fb layout
            width = 512
    |---------------------|
    |(xoffset, yoffset)   |
    | -|----------------| |
    |  |                | |
    |  |                | |
    |  |     2r         | |
    |  |                | |
    |  |                | |
    |  |----------------| |
    |                     |
    |---------------------|
*/

/*
 * gamu_get_line_conf
 * @line  : line number
 * @entry : line lut entry
 * @fb    : gamu buffer context reference
 *
 * NOTE:
 *   block_size = 16 bytes
*/
//#include "hal_timer.h"
static void gamu_get_line_conf(uint16_t line, gamu_lut_entry_t *entry, gamu_fb_info_t *fb)
{
    uint16_t line_vssize = gamu_circal_linemem_length(line,fb);
    uint8_t  vblks = line_vssize/BLKSIZE;
    uint8_t  tblks = (fb->stride - fb->yoffset )/BLKSIZE;

    entry->first_vblock = fb->yoffset/BLKSIZE + (tblks - vblks)/2;
    entry->last_vblock =  entry->first_vblock + vblks -1;

    entry->offset =  (int16_t)(fb->used_vblocks - entry->first_vblock -
                               fb->bufferoffset/BLKSIZE + fb->stride/BLKSIZE);
    entry->line = line;
    if (vblks){
     entry->ena = 1;
    }else{
     entry->ena = 0;
    }
    entry->used_vblocks = vblks;
    fb->used_vblocks += entry->used_vblocks;

    //hal_sys_timer_delay_us(10000);
    //TRACE(3,"l:%d fbs:%d lbs:%d offset:%d ubs:%d",line, entry->first_vblock,
    //      entry->last_vblock, entry->offset,fb->used_vblocks);
}

/*
 *   alloc_gamu_mem
 *  @size  : mmu mem size request
 *  @bufid : buffer id
 *  @bpp   : color deepth
 *
 *  return: mem address
 *
*/
static uint32_t alloc_gamu_mem(uint32_t size,uint8_t bufid ,uint8_t bpp)
{
    uint32_t addr = 0;

    if ((gamu_vmem.used + size) > GAMU_VIRTUAL_ADDRSPACE_TOP){
        return 0;
    }

    if (bpp == 32)
        addr = gamu_vmem.start_addr | bufid << 20 ;

    if (bpp == 24)
        addr = gamu_vmem.start_addr | bufid << 20 ;

    if (bpp == 16)
        addr = gamu_vmem.start_addr | bufid << 19 ;

    gamu_vmem.used += size;
    return addr;
}

/*
 *  gamu_clock_enable
 *  @fb : gamu buffer context reference
*/
static void gamu_clock_enable(void)
{
    hal_cmu_gamu_clock_enable();
}

/*
 *  hal_gamu_setup
 *  @fb : gamu buffer context reference
*/
void hal_gamu_setup(gamu_fb_info_t *fb)
{
    gamu_lut_entry_t entry;
    gamu_clock_enable();
    gamu_buffer_enable(fb->id, fb->bpp);

    //calculate physical mem size
    fb->stride = fb->width * fb->bpp /8;
    for(int i = 0; i < fb->height; i++){
         gamu_get_line_conf(i,&entry,fb);
    }

    fb->phymem_size = fb->used_vblocks * BLKSIZE + 64;
    fb->phymem = fb->align_malloc(fb->phymem_size,64);

    fb->bufferoffset = fb->phymem_size / 2;
    gamu_set_buffer_phyaddress(fb->id, (uint32_t)fb->phymem  + fb->bufferoffset);
    gamu_set_haddr_unvalid((uint32_t)fb->phymem);

    //line config
    fb->used_vblocks = 0;
    for(int i = 0; i < fb->height; i++){
       gamu_get_line_conf(i,&entry,fb);
       gamu_write_lut_entry(&entry);
    }

    fb->vmem_size = fb->stride * fb->height;
    fb->vmem = (uint32_t *)alloc_gamu_mem(fb->vmem_size,fb->id ,fb->bpp);
    TRACE(3, "%s pmemsize:%d", __func__,fb->phymem_size );
}


#if 0 //test


#include "math.h"

static void *fb_malloc(uint32_t size, uint8_t aligns)
{
   return 0x38000000;
}

/*
 * fb layout
            width = 512
    |---------------------|
    |(xoffset, yoffset)   |
    | -|----------------| |
    |  |                | |
    |  |                | |
    |  |     2r         | |
    |  |                | |
    |  |                | |
    |  |----------------| |
    |                     |
    |---------------------|
*/

static gamu_fb_info_t  gamu_fb0 = {
    .id = 0,
    .width = 512,  // must be 512 pixels for 1502x
    .height = 512,
    .bpp = 32,
    .align_malloc = fb_malloc,
    .sqrtf = sqrtf,
    // width = xoffset + 2* radius + x_remain
    // height = yoffset + 2* radius + y_remain
    .radius = 240,//256,//,128,//48,
    .xoffset = 32, // colu offset
    .yoffset = 32, // line offset

    .used_vblocks = 0,
};


void setup_gamu_buffer( void )
{
    volatile uint32_t * p = 0;
    volatile uint32_t wait = 1;
    hal_gamu_setup(&gamu_fb0);
    p = gamu_fb0.vmem;
    p += 256*gamu_fb0.width;
    for(int i = 0; i < gamu_fb0.width; i++){
      *(p + i) = 256*gamu_fb0.width + i;
    }
    p += gamu_fb0.width;
    for(int i = 0; i < gamu_fb0.width; i++){
      *(p + i) = 257*gamu_fb0.width + i;
    }
    TRACE(3, "%s gamufb0  vmem:%p vmsize:%d  pmem:%p pmemsize:%d"
          , __func__, gamu_fb0.vmem, gamu_fb0.vmem_size
          , gamu_fb0.phymem, gamu_fb0.phymem_size );
#if 0
    gamu_fb.id = 1;
    hal_gamu_setup(&gamu_fb);
    TRACE(3, "%s gamufb1  vmem:%p vmsize:%d  pmem:%p pmemsize:%d"
          , __func__, gamu_fb.vmem, gamu_fb.vmem_size
          , gamu_fb.phymem, gamu_fb.phymem_size );
    gamu_fb.id = 2;
    hal_gamu_setup(&gamu_fb);
    TRACE(3, "%s gamufb2  vmem:%p vmsize:%d  pmem:%p pmemsize:%d"
          , __func__, gamu_fb.vmem, gamu_fb.vmem_size
          , gamu_fb.phymem, gamu_fb.phymem_size );
    gamu_fb.id = 3;
    hal_gamu_setup(&gamu_fb);
    TRACE(3, "%s gamufb3  vmem:%p vmsize:%d  pmem:%p pmemsize:%d"
          , __func__, gamu_fb.vmem, gamu_fb.vmem_size
          , gamu_fb.phymem, gamu_fb.phymem_size );
#endif
}
#endif

#endif
