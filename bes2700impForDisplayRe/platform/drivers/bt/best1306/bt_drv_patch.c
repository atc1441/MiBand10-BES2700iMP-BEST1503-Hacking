/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#include "plat_types.h"
#include "string.h"
#include "hal_chipid.h"
#include "hal_trace.h"
#include "bt_drv.h"
#include "bt_drv_interface.h"
#include "bt_drv_1306_internal.h"
#include "bt_1306_reg_map.h"
#include "bt_patch_1306_t0.h"
#ifdef BT_STEREO_PATCH
#include "bt_patch_1306_t1_stereo.h"
#else
#include "bt_patch_1306_t1.h"
#endif
#include "bt_patch_1306_t2.h"

//MAX patch number
#define BT_PATCH_ENTRY_NUM  (64)

#define BTDRV_PATCH_ACT     (0x1)
#define BTDRV_PATCH_INACT   (0x0)

#define BT_FUNC_PATCH_TYPE     (0x1)
#define BT_INS_PATCH_TYPE     (0x2)

//address remapping
#define BTDRV_PATCH_ROM_ADDRESS_OFFSET      (0xA0000000 - 0x00000000)          //0x00000000 --> 0xA0000000
#define BTDRV_PATCH_RAM_ADDRESS_OFFSET      (0xC0000000 - 0xA0200000)          //0xC0000000 --> 0xA0200000

//instruction requirement
#define BTDRV_PATCH_ARM_ADDR(a)          (a & 0xFFFFFFFE)
#define BTDRV_PATCH_THUMB_INS(a)         ((a & 0xFFFFFFFE) | 0x1)

#define CO_BIT(pos) (1UL<<(pos))

typedef struct{
    uint8_t patch_index;            //patch position
    uint8_t patch_state;            //is patch active
    uint16_t patch_length;          //patch length 0:one instrution replace  other:jump to ram to run more instruction
    uint32_t patch_remap_address;   //patch occured address
    uint32_t patch_remap_value;     //patch replaced instuction
    uint32_t patch_start_address;   //ram patch address for lenth>0
    uint8_t *patch_data;            //ram patch date for length >0
} BTDRV_PATCH_STRUCT;

typedef struct{
    uint32_t  text_start;
    uint32_t  bss_start;
    uint32_t  bss_end;
} patch_info_t;


typedef struct{
    uint32_t  active;
    uint32_t  source_addr;
    uint32_t  patch_addr;
} patch_entry_t;

typedef struct{
    uint32_t  copy_section_num;
    uint32_t  memset_section_num;
} patch_sec_num_t;

typedef struct{
    uint32_t  ram_text_start;
    uint32_t  flash_text_start;
    uint32_t  size;
} patch_text_t;

typedef struct{
    uint32_t  bss_start;
    uint32_t  size;
} patch_bss_t;

/*****************************************************************************
 Prototype    : func_b_jump_ins
 Description  : Generate B instrcution:jump from ROM origin func to RAM patch func
*****************************************************************************/
uint32_t func_b_jump_ins(uint32_t old_func, uint32_t new_func)
{
    union{
        uint32_t d32;
        uint16_t d16[2];
    } ins;
    uint32_t immd;
    uint8_t j1, j2, s;

    ins.d32 = 0x9000F000;
    immd = (new_func & ~1) - ((old_func + 4) & ~1);
    s = immd >> 31;
    j1 = s ^ !((immd >> 23) & 1);
    j2 = s ^ !((immd >> 22) & 1);
    ins.d16[0] |= (s << 10) | ((immd >> 12) & 0x3FF);
    ins.d16[1] |= (j1 << 13) | (j2 << 11) | ((immd >> 1) & 0x7FF);

    return ins.d32;
}

void btdrv_ins_patch_write(BTDRV_PATCH_STRUCT *ins_patch_p)
{
    BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + ins_patch_p->patch_index*sizeof(uint32_t),
        ins_patch_p->patch_remap_value);
    //have ram patch data
    if (ins_patch_p->patch_length != 0) {
        memcpy((uint8_t *)ins_patch_p->patch_start_address,ins_patch_p->patch_data,ins_patch_p->patch_length);
    }

    uint32_t remap_addr =  ins_patch_p->patch_remap_address | 1;
    BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START + ins_patch_p->patch_index*sizeof(uint32_t), remap_addr);
}

void btdrv_patch_en(uint8_t en)
{
    if (en) {
        *(volatile uint32_t *)BTDRV_PATCH_EN_REG |= 0x80000000;
    } else {
        *(volatile uint32_t *)BTDRV_PATCH_EN_REG &= ~0x80000000;
    }
}

void btdrv_ins_patch_disable(uint8_t index)
{
    //disable patch, clear remap address and jump instruction
    BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START+index*sizeof(uint32_t), 0);
    BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + index*sizeof(uint32_t), 0);
}

void btdrv_ins_patch_close(void)
{
    btdrv_patch_en(BTDRV_PATCH_INACT);

    for(uint8_t i=0; i< BT_PATCH_ENTRY_NUM; i++) {
        btdrv_ins_patch_disable(i);
    }
}

void btdrv_function_patch_init_common(uint32_t * patch, uint32_t patch_size)
{
    uint8_t *bt_patch_data = NULL;

    patch_sec_num_t *patch_info_ptr = NULL;
    patch_entry_t *patch_table = NULL;
    patch_entry_t *patch_entry_ptr = NULL;
    uint32_t i = 0;
    uint32_t rom_addr;
    uint32_t jmp_ins;
    patch_text_t *patch_text_ptr;
    patch_bss_t *patch_bss_ptr;
    uint32_t size;

    bt_patch_data = (uint8_t *)patch;

    patch_table = (patch_entry_t *)bt_patch_data;

    patch_info_ptr = (patch_sec_num_t *)((uint32_t *)bt_patch_data + sizeof(patch_entry_t) / sizeof(uint32_t) * BT_PATCH_ENTRY_NUM);

    patch_text_ptr = (patch_text_t *)((uint32_t *)patch_info_ptr + sizeof(patch_sec_num_t) / sizeof(uint32_t));

    for (i = 0; i <  patch_info_ptr->copy_section_num; i ++)
    {
        if(CO_BIT(31) & patch_text_ptr->size)
        {
            size = patch_text_ptr->size & ~CO_BIT(31);
            if(size > sizeof(uint32_t) || size == 3)
            {
                 ASSERT(0, "%s: error size,0x%x,0x%x,0x%x",
                        __func__, patch_text_ptr->ram_text_start, patch_text_ptr->flash_text_start, patch_text_ptr->size);
            } else {
                memcpy((uint8_t *)patch_text_ptr->ram_text_start, &patch_text_ptr->flash_text_start, size);
            }
        } else {
            btdrv_memcpy_word((uint8_t *)patch_text_ptr->ram_text_start, bt_patch_data + patch_text_ptr->flash_text_start, patch_text_ptr->size);
        }
        patch_text_ptr += 1;
    }

    patch_bss_ptr = (patch_bss_t *)patch_text_ptr;

    for (i = 0; i <  patch_info_ptr->memset_section_num; i ++){
        memset((uint8_t *)patch_bss_ptr->bss_start, 0x00, patch_bss_ptr->size);
        patch_bss_ptr += 1;
    }

    //set patch entry
    for (i = 0; i < BT_PATCH_ENTRY_NUM; i++){
        patch_entry_ptr = (patch_table + i);

        if (patch_entry_ptr->active == BT_FUNC_PATCH_TYPE){
            rom_addr = BTDRV_PATCH_THUMB_INS(patch_entry_ptr->source_addr) - BTDRV_PATCH_ROM_ADDRESS_OFFSET;
            BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START + i*sizeof(uint32_t), rom_addr);
            jmp_ins = func_b_jump_ins(BTDRV_PATCH_ARM_ADDR(patch_entry_ptr->source_addr),BTDRV_PATCH_ARM_ADDR(patch_entry_ptr->patch_addr) - BTDRV_PATCH_RAM_ADDRESS_OFFSET);
            BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + i*sizeof(uint32_t), jmp_ins);
        }
        else if(patch_entry_ptr->active == BT_INS_PATCH_TYPE){
            BT_PATCH_WR(BTDRV_PATCH_INS_REMAP_ADDR_START + i*sizeof(uint32_t), patch_entry_ptr->patch_addr);
            rom_addr = BTDRV_PATCH_THUMB_INS(patch_entry_ptr->source_addr) - BTDRV_PATCH_ROM_ADDRESS_OFFSET;
            BT_PATCH_WR(BTDRV_PATCH_INS_COMP_ADDR_START + i*sizeof(uint32_t), rom_addr);
        }
    }
}

/*
 * function patch entry init
 */
void btdrv_ins_patch_init(void)
{
    enum HAL_CHIP_METAL_ID_T metal_id = hal_get_chip_metal_id();
    if (metal_id == HAL_CHIP_METAL_ID_0) {
        btdrv_function_patch_init_common((uint32_t *)bt_patch_1306_t0,sizeof(bt_patch_1306_t0));
        BT_DRV_TRACE(1,"BTC:1306 t0 work mode patch version:%08x", BTDIGITAL_REG(WORKMODE_PATCH_VERSION_T0_ADDR));
    } else if (metal_id == HAL_CHIP_METAL_ID_1) {
#ifdef BT_STEREO_PATCH
        btdrv_function_patch_init_common((uint32_t *)bt_patch_1306_t1_stereo,sizeof(bt_patch_1306_t1_stereo));
        BT_DRV_TRACE(1,"BTC:1306 t1 stereo work mode patch version:%08x", BTDIGITAL_REG(WORKMODE_PATCH_VERSION_T1_STEREO_ADDR));
#else
        btdrv_function_patch_init_common((uint32_t *)bt_patch_1306_t1,sizeof(bt_patch_1306_t1));
        BT_DRV_TRACE(1,"BTC:1306 t1 work mode patch version:%08x", BTDIGITAL_REG(WORKMODE_PATCH_VERSION_T1_ADDR));
#endif
    } else if (metal_id >= HAL_CHIP_METAL_ID_2)
    {
        btdrv_function_patch_init_common((uint32_t *)bt_patch_1306_t2,sizeof(bt_patch_1306_t2));
        BT_DRV_TRACE(1,"BTC:1306 t2 work mode patch version:%08x", BTDIGITAL_REG(WORKMODE_PATCH_VERSION_T2_ADDR));
    }
    else
    {
        ASSERT(0, "%s:error metal id=%d", __func__, metal_id);
    }
    //Enable patch module
    btdrv_patch_en(1);
}

void btdrv_ins_patch_test_init(void)
{
    //TODO
    bt_drv_reg_op_for_test_mode_disable();
}
