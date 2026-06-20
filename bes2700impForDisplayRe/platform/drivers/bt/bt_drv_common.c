/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
#include "hal_i2c.h"
#include "hal_uart.h"
#include "hal_chipid.h"
#include "bt_drv.h"
#include "bt_drv_internal.h"
#include "bt_drv_interface.h"
#include "bt_drv_reg_op.h"
#include "watchdog.h"
#include "string.h"
#include "besbt_string.h"
#ifdef BTC_CPUDUMP_BASE
#include "bt_drv_cpudump.h"
#endif
#include "hal_intersys.h"
/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long unsigned int) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long unsigned int))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < (BIGBLOCKSIZE))

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

#define BT_DUMP_LEN_LINE    100
#define SLOT_SIZE           625
#if defined(BT_CLK_UNIT_IN_HSLOT)
//bt max slot clock
#define MAX_SLOT_CLOCK      ((1L<<28) - 1)
#else
//bt max slot clock
#define MAX_SLOT_CLOCK      ((1L<<27) - 1)
#endif
#define CLK_SUB(clock_a, clock_b)     ((uint32_t)(((clock_a) - (clock_b)) & MAX_SLOT_CLOCK))
#define CLK_ADD_2(clock_a, clock_b)     ((uint32_t)(((clock_a) + (clock_b)) & MAX_SLOT_CLOCK))

struct bt_cb_tag bt_drv_func_cb = {NULL};

struct bt_cb_tag* bt_drv_get_func_cb_ptr(void)
{
    return &bt_drv_func_cb;
}

static btstack_send_hci_cmd_t btstack_send_hci_cmd_hander = NULL;
int bt_drv_register_send_hci_cmd_hander(btstack_send_hci_cmd_t hander)
{
    btstack_send_hci_cmd_hander = hander;
    return 0;
}

int btdrv_send_hci_cmd_bystack(uint16_t opcode, uint8_t *param, uint8_t len)
{
    int ret = -1;
    if(btstack_send_hci_cmd_hander)
        ret = btstack_send_hci_cmd_hander(opcode, param, len);

    if(ret != 0)
    {
        BT_DRV_TRACE(0, "BT_DRV:hci cmd fail\n");
    }
    return ret;
}

void btdrv_stack_send_hci_cmd_data_bystack(uint8_t *hci_ptr)
{
    uint16_t opcode;

    if(*hci_ptr != 1)
    {
        BT_DRV_TRACE(0, "BT_DRV:hci cmd data error\n");
        return;
    }

    memcpy(&opcode, hci_ptr + 1, 2);

    btdrv_send_hci_cmd_bystack(opcode, hci_ptr + 4,*(hci_ptr + 3));
}

///only used for bt chip write patch data for speed up
void btdrv_memory_copy(uint32_t *dest,const uint32_t *src,uint16_t length)
{
    // memcpy(dest,src,length);
    uint16_t i;
    for(i=0; i<length/4; i++)
    {
        *dest++ = *src++;
    }
}

void *btdrv_memcpy_word(void * d, const void * s, size_t n)
{
    /* attempt word-sized copying only if buffers have identical alignment */
    unsigned char *d_byte = (unsigned char *)d;
    const unsigned char *s_byte = (const unsigned char *)s;
    const long unsigned int mask = sizeof(long unsigned int) - 1;

    if (unlikely(!TOO_SMALL(n))) {

       //let .txt code 4byte aligned in the loop
        __asm volatile("nop");

       if((((long unsigned int)d ^ (long unsigned int)s_byte) & mask) == 0)
        {
            /* do byte-sized copying until word-aligned or finished */
            while (((long unsigned int)d_byte) & mask) {
                if (n == 0) {
                    return d;
                }
                *(d_byte++) = *(s_byte++);
                n--;
            };

            /* do word-sized copying as long as possible */

            long unsigned int *d_word = (long unsigned int *)d_byte;
            const long unsigned int *s_word = (const long unsigned int *)s_byte;

            /* Copy 4X long words at a time if possible.  */
            while (n >= BIGBLOCKSIZE) {
                *(d_word++) = *(s_word++);
                *(d_word++) = *(s_word++);
                *(d_word++) = *(s_word++);
                *(d_word++) = *(s_word++);
                n -= BIGBLOCKSIZE;
            }

            /* Copy one long word at a time if possible.  */
            while (n >= LITTLEBLOCKSIZE) {
                *d_word++ = *s_word++;
                n -= LITTLEBLOCKSIZE;
            }

            d_byte = (unsigned char *)d_word;
            s_byte = (unsigned char *)s_word;
        }
    }

    //let .txt code 4byte aligned in the loop
    __asm volatile(".balign    4");
    __asm volatile("nop");

    /* do byte-sized copying until finished */
    while (n > 0) {
        *(d_byte++) = *(s_byte++);
        n--;
    }

    return d;
}


int btdrv_slave2master_clkcnt_convert(uint32_t local_clk, uint16_t local_cnt,
                                      int32_t clk_offset, uint16_t bit_offset,
                                      uint32_t *master_clk, uint16_t *master_cnt)
{
    // Adjust bit offset and clock offset if needed
    uint32_t new_clk;
    int16_t new_cnt;

    new_clk = CLK_ADD_2(local_clk, clk_offset);
    new_cnt = (int16_t)local_cnt + bit_offset;

    if (new_cnt > SLOT_SIZE)
    {
        new_cnt -= SLOT_SIZE;
        new_clk = CLK_SUB(new_clk, 1);
    }

    *master_clk = new_clk;
    *master_cnt = new_cnt;
    return 0;
}

extern uint8_t sleep_param[];

void btdrv_load_sleep_config(uint8_t* config)
{
    memcpy_s(config, 8, sleep_param, 8);
}

void btdrv_trigger_coredump(void)
{
    uint8_t *null_pointer = 0;
    *null_pointer = 0;//MPU trigger coredump
}

// This api should only be used to calculate dma capture clk diff, for chip after 1501, unit is hs and hus.
int btdrv_clkcnt_diff(int32_t clk1, int16_t cnt1,
                      int32_t clk2, int16_t cnt2,
                      int32_t *diff_clk, uint16_t *diff_bit)
{
    int32_t new_clk;
    int16_t new_cnt;
    int diff_us;

    new_clk = (int32_t)clk1 - (int32_t)clk2;
    new_cnt = cnt1 - cnt2;
    if (new_cnt < 0)
    {
        new_cnt += SLOT_SIZE;
    }else{
        new_clk -= 1;
    }

    *diff_clk = new_clk;
    *diff_bit = new_cnt;

    diff_us = new_clk * SLOT_SIZE + (SLOT_SIZE - new_cnt);

#if defined(BT_CLK_UNIT_IN_HSLOT)
    return diff_us/2;
#else
    return diff_us;
#endif
}

// This api should only be used to calculate dma capture clk total us, for chip after 1501, unit is hs and hus.
int64_t btdrv_clkcnt_to_us(uint32_t clk, uint16_t cnt)
{
    int64_t total_cnt;
#if defined(BT_CLK_UNIT_IN_HSLOT)
    total_cnt = ((int64_t)clk * (int64_t)SLOT_SIZE + (int64_t)cnt)/2;
#else
    total_cnt = (int64_t)clk * (int64_t)SLOT_SIZE + (int64_t)cnt;
#endif
    return total_cnt;
}


// This api should only be used to calculate dma capture clk total us, for chip after 1501, unit is hs and hus.
int64_t btdrv_iso_clkcnt_to_us(uint32_t clk, uint16_t cnt)
{
    int64_t total_cnt;
#if defined(BT_CLK_UNIT_IN_HSLOT)
    total_cnt = ((int64_t)clk * (int64_t)SLOT_SIZE + (int64_t)(SLOT_SIZE - cnt - 1))/2;
#else
    total_cnt = (int64_t)clk * (int64_t)SLOT_SIZE + (int64_t)(SLOT_SIZE - cnt - 1);
#endif
    return total_cnt;
}

void WEAK bt_drv_reg_op_enable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
}

void WEAK bt_drv_reg_op_disable_dma_tc(uint8_t adma_ch, uint32_t dma_base)
{
}

void WEAK bt_drv_reg_op_dma_tc_clkcnt_get_by_ch(uint32_t *btclk, uint16_t *btcnt, uint8_t adma_ch, uint32_t dma_base)
{
}

void WEAK bt_drv_reg_op_multi_ibrt_music_config(uint8_t* link_id, uint8_t* active, uint8_t num)
{
}

void WEAK bt_drv_digital_config_for_ble_adv(bool en)
{
}

void WEAK bt_drv_reg_op_clean_flags_of_ble_and_sco(void)
{
}

void WEAK bt_drv_reg_op_set_ibrt_reject_sniff_req(bool en)
{
}

uint8_t WEAK bt_drv_reg_op_get_sync_id_by_conhdl(uint16_t conhdl)
{
    return MAX_NUM_SCO_LINKS;
}

void WEAK bt_drv_reg_op_clear_skip_flag(void)
{
}

void WEAK btdrv_reconn(bool en)
{
}

void WEAK bt_drv_reg_op_reset_agc_thd(void)
{
}

void WEAK bt_drv_reg_op_set_agc_thd(bool isMaster, bool isSco)
{

}

void WEAK btdrv_regist_le_teset_end_callback(le_test_end_handler_func cb)
{
}

void WEAK btdrv_regist_bt_nonsignalint_test_end_callback(bt_nonsig_test_end_handler_func cb)
{
}

int8_t WEAK btdrv_reg_op_txpwr_idx_to_rssidbm(uint8_t txpwr_idx)
{
    return -127;
}

void WEAK bt_drv_reg_op_afh_assess_en(bool en)
{
}

void WEAK bt_drv_i2v_disable_sleep_for_bt_access(void)
{
}

void WEAK bt_drv_i2v_enable_sleep_for_bt_access(void)
{
}

void WEAK bt_drv_ble_adv_txpwr_via_advhdl(uint8_t adv_hdl, uint8_t idx, int8_t txpwr_dbm)
{

}

void WEAK btdrv_regop_get_ble_event_info(uint16_t ble_conhdl, uint16_t *evt_count, uint32_t *interval, uint32_t *anchor_ts, uint16_t *bit_off)
{

}

void WEAK bt_drv_reg_op_adjust_rf_param_for_testmode(void)
{

}

void WEAK bt_drv_reg_op_i2v_checker_for_testmode(void)
{
}

void WEAK bt_drv_reg_op_low_txpwr_set(uint8_t enable, uint8_t factor, uint8_t bt_or_ble, uint8_t link_id)
{
}

void WEAK bt_drv_rf_high_efficency_tx_pwr_ctrl(bool en, bool reset)
{
}

void WEAK bt_drv_reg_op_setting_wesco(uint8_t wesco)
{
}

void WEAK bt_drv_reg_op_ecc_softbit_process(uint16_t* p_conn_handle1,uint16_t* p_conn_handle2, uint16_t length, uint8_t *data)
{
}

bool WEAK bt_drv_error_check_handler(void)
{
    bool ret = false;
    return ret;
}

void WEAK bt_drv_reg_op_dump_rx_record(void)
{
}

void WEAK bt_drv_reg_op_multi_ibrt_sche_dump(void)
{
}

uint8_t WEAK bt_drv_reg_op_read_bt_3m_rxgain(void)
{
    return 0xff;
}

void WEAK bt_drv_reg_op_set_btpcm_trig_flag(bool flag)
{
}

void WEAK bt_drv_reg_op_ble_sup_timeout_set(uint16_t ble_conhdl, uint16_t sup_to)
{
}

void WEAK bt_drv_reg_op_key_gen_after_reset(bool enable)
{
}

void WEAK bt_drv_reg_op_write_private_public_192_256_key(uint8_t* private_key,uint8_t* public_key,bool p192_or_p256)
{

}

#ifdef BTC_CPUDUMP_BASE
#define BTC_DUMPPC_TOTAL_NUM    0x2000/4
#if CHIP_BEST1306
#undef BTC_DUMPPC_TOTAL_NUM
#define BTC_DUMPPC_TOTAL_NUM    0x400/4
#endif
#define BTC_DUMPPC_NUM    32
void btdrv_trace_pc_dump(void)
{
    uint32_t i, j = 0;
    uint32_t print_start_pos = 0;
    volatile uint32_t *maddr = (uint32_t*)BTC_CPUDUMP_BASE;

    btdrv_cpudump_disable();
#if CHIP_BEST1306
    for(i = 0; i < BTC_DUMPPC_TOTAL_NUM; i++)
#else
    for(i = 0; i < BTC_DUMPPC_TOTAL_NUM/4; i++)
#endif
    {
        if((maddr[i] & 0x1) == 1){
            //TRACE(0, "%d, 0x%x, 0x%x", i, i*4, maddr[i]);
            break;
        }
    }

    if(i > BTC_DUMPPC_NUM)
    {
        print_start_pos = i + 1 - BTC_DUMPPC_NUM;
        for(j = 0; j < BTC_DUMPPC_NUM/4; j++)
        TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4], maddr[print_start_pos + j*4 + 1],
            maddr[print_start_pos + j*4 + 2], maddr[print_start_pos + j*4 + 3]);
    }
    else
    {
        print_start_pos = BTC_DUMPPC_TOTAL_NUM - (BTC_DUMPPC_NUM - (i+1));
        for(j = 0; j < BTC_DUMPPC_NUM/4; j++)
        {
            if((print_start_pos + j*4) < (BTC_DUMPPC_TOTAL_NUM - 3))
                TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4], maddr[print_start_pos + j*4 + 1],
                    maddr[print_start_pos + j*4 + 2], maddr[print_start_pos + j*4 + 3]);
            else if((print_start_pos + j*4) == (BTC_DUMPPC_TOTAL_NUM - 3))
                TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4], maddr[print_start_pos + j*4 + 1],
                    maddr[print_start_pos + j*4 + 2], maddr[print_start_pos + j*4 + 3 - BTC_DUMPPC_TOTAL_NUM]);
            else if((print_start_pos + j*4) == (BTC_DUMPPC_TOTAL_NUM - 2))
                TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4], maddr[print_start_pos + j*4 + 1],
                    maddr[print_start_pos + j*4 + 2 - BTC_DUMPPC_TOTAL_NUM], maddr[print_start_pos + j*4 + 3 - BTC_DUMPPC_TOTAL_NUM+1]);
            else if((print_start_pos + j*4) == (BTC_DUMPPC_TOTAL_NUM - 1))
                TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4], maddr[print_start_pos + j*4 + 1 - BTC_DUMPPC_TOTAL_NUM],
                    maddr[print_start_pos + j*4 + 2 - BTC_DUMPPC_TOTAL_NUM+1], maddr[print_start_pos + j*4 + 3 - BTC_DUMPPC_TOTAL_NUM+2]);
            else
                TRACE(0, "0x%08x, 0x%08x, 0x%08x, 0x%08x", maddr[print_start_pos + j*4 - BTC_DUMPPC_TOTAL_NUM], maddr[print_start_pos + j*4 + 1 - BTC_DUMPPC_TOTAL_NUM],
                    maddr[print_start_pos + j*4 + 2 - BTC_DUMPPC_TOTAL_NUM], maddr[print_start_pos + j*4 + 3 - BTC_DUMPPC_TOTAL_NUM]);
       }
    }
}
#endif

void btdrv_btc_fault_dump(void)
{
#ifdef BTC_CPUDUMP_BASE
    btdrv_trace_pc_dump();
#endif //BTC_CPUDUMP_BASE

#if CONTROLLER_DUMP_ENABLE
#ifdef __WATCHER_DOG_RESET__
    watchdog_hw_stop();
#endif
    hal_intersys_dump_register();
    bt_drv_reg_op_crash_dump();
#endif //CONTROLLER_DUMP_ENABLE
}

void btdrv_dump_mem(uint8_t *dump_mem_start, uint32_t dump_length, uint8_t dump_type)
{
#if CONTROLLER_DUMP_ENABLE
    uint32_t dump_counter=0;
    uint32_t dump_line=0;

    hal_sys_timer_delay(MS_TO_TICKS(100));
    switch (dump_type)
    {
        case BT_SUB_SYS_TYPE:
            BT_DRV_TRACE(1,"controller dump start, dump len=0x%x",dump_length);
            break;

        case MCU_SYS_TYPE:
            BT_DRV_TRACE(1,"mcu dump start, dump len=0x%x",dump_length);
            break;

        case BT_EM_AREA_1_TYPE:
            BT_DRV_TRACE(1,"em_area1 dump start, dump len=0x%x",dump_length);
            break;

        case BT_EM_AREA_2_TYPE:
            BT_DRV_TRACE(1,"em_area2 dump start, dump len=0x%x",dump_length);
            break;
    }
    while (dump_counter < dump_length)
    {
        uint32_t data_left = dump_length - dump_counter;
        dump_line += 1;
        if (data_left >= BT_DUMP_LEN_LINE)
        {
            BT_DRV_TRACE(1,"dump_line=%d", dump_line);
            DUMP8("%02x", dump_mem_start, BT_DUMP_LEN_LINE);
            hal_trace_flush_buffer();
            dump_mem_start += BT_DUMP_LEN_LINE;
            dump_counter += BT_DUMP_LEN_LINE;
            hal_sys_timer_delay(MS_TO_TICKS(50));
        }
        else
        {
            BT_DRV_TRACE(1,"dump_line=%d", dump_line);
            DUMP8("%02x", dump_mem_start, data_left);
            hal_trace_flush_buffer();
            hal_sys_timer_delay(MS_TO_TICKS(50));
            break;
        }
    }
    switch (dump_type)
    {
        case BT_SUB_SYS_TYPE:
            BT_DRV_TRACE(0,"controller dump end");
            break;

        case MCU_SYS_TYPE:
            BT_DRV_TRACE(0,"mcu dump end");
            break;

        case BT_EM_AREA_1_TYPE:
            BT_DRV_TRACE(0,"em_area1 dump end");
            break;

        case BT_EM_AREA_2_TYPE:
            BT_DRV_TRACE(0,"em_area2 dump end");
            break;
    }
#endif
}


#ifdef  BES_ANDROID_SUPPORT
 char uart_dump_buf[255];

int hal_uart_dump(const char *fmt, unsigned int size,  unsigned int count, const void *buffer)
{
    int len=0,  i=0;

    switch( size )
    {
        case sizeof(uint32_t):
            while(i<count && len<sizeof(uart_dump_buf))
            {
                len += format_string(&uart_dump_buf[len], sizeof(uart_dump_buf) - len, fmt, *(uint32_t *)((uint32_t *)buffer+i));
                i++;
            }
            break;
        case sizeof(uint16_t):
                while(i<count && len<sizeof(uart_dump_buf))
                {
                    len += format_string(&uart_dump_buf[len], sizeof(uart_dump_buf) - len, fmt, *(uint16_t *)((uint16_t *)buffer+i));
                    i++;
                }
                break;
        case sizeof(uint8_t):
        default:
            while(i<count && len<sizeof(uart_dump_buf))
            {
                len += format_string(&uart_dump_buf[len], sizeof(uart_dump_buf) - len, fmt, *(uint8_t *)((uint8_t *)buffer+i));
                i++;
            }
            break;
    }

#if 1//def TRACE_CRLF
    if (sizeof(uart_dump_buf) < 2) {
        len = 0;
    } else if (len + 2 > sizeof(uart_dump_buf) && sizeof(uart_dump_buf) >= 2) {
        len = sizeof(uart_dump_buf) - 2;
    }
    if (len + 2 <= sizeof(uart_dump_buf)) {
        uart_dump_buf[len++] = '\r';
        uart_dump_buf[len++] = '\n';
    }
#else
    if (len + 1 > sizeof(uart_dump_buf) && sizeof(uart_dump_buf) >= 1) {
        len = sizeof(uart_dump_buf) - 1;
    }
    if (len + 1 <= sizeof(uart_dump_buf)) {
        uart_dump_buf[len++] = '\n';
    }
#endif

    bt_uart_debug_send((uint8_t *)&uart_dump_buf, len);

    return 0;
}


void btdrv_dump_mem_2_uart(uint8_t *dump_mem_start, uint32_t dump_length, uint8_t dump_type)
{
#if CONTROLLER_DUMP_ENABLE
    uint32_t dump_counter=0;
    uint32_t dump_line=0;

    hal_sys_timer_delay(MS_TO_TICKS(100));
    switch (dump_type)
    {
        case BT_SUB_SYS_TYPE:
            trace_2_uart(1,"controller dump start, dump len=0x%x",dump_length);
            break;

        case MCU_SYS_TYPE:
            trace_2_uart(1,"mcu dump start, dump len=0x%x",dump_length);
            break;

        case BT_EM_AREA_1_TYPE:
            trace_2_uart(1,"em_area1 dump start, dump len=0x%x",dump_length);
            break;

        case BT_EM_AREA_2_TYPE:
            trace_2_uart(1,"em_area2 dump start, dump len=0x%x",dump_length);
            break;
    }
    while (dump_counter < dump_length)
    {
        uint32_t data_left = dump_length - dump_counter;
        dump_line += 1;
        if (data_left >= BT_DUMP_LEN_LINE)
        {
            trace_2_uart(1,"dump_line=%d", dump_line);
            UART_DUMP8("%02x", dump_mem_start, BT_DUMP_LEN_LINE);
            dump_mem_start += BT_DUMP_LEN_LINE;
            dump_counter += BT_DUMP_LEN_LINE;
            hal_sys_timer_delay(MS_TO_TICKS(50));
        }
        else
        {
            trace_2_uart(1,"dump_line=%d", dump_line);
            UART_DUMP8("%02x", dump_mem_start, data_left);
            hal_sys_timer_delay(MS_TO_TICKS(50));
            break;
        }
    }
    switch (dump_type)
    {
        case BT_SUB_SYS_TYPE:
            trace_2_uart(0,"controller dump end");
            break;

        case MCU_SYS_TYPE:
            trace_2_uart(0,"mcu dump end");
            break;

        case BT_EM_AREA_1_TYPE:
            trace_2_uart(0,"em_area1 dump end");
            break;

        case BT_EM_AREA_2_TYPE:
            trace_2_uart(0,"em_area2 dump end");
            break;
    }
#endif
}

#endif

void btdrv_enable_tports(void)
{
#ifdef __BT_DEBUG_TPORTS__
#ifdef __BT_3WIRE_TPORTS__
    bt_drv_reg_op_enable_3wire_tports();
#elif __BT_2WIRE_TPORTS__
    bt_drv_reg_op_enable_2wire_tports();
#else
    hal_iomux_set_bt_tport();
#endif
    bt_drv_bt_tport_type_config();
#endif
}

bool bt_drv_is_enhanced_ibrt_rom(void)
{
    bool ret = false;

#ifdef CHIP_BEST2300P
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_1)
        ret = true;
#elif CHIP_BEST1400
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2)
        ret = true;
#elif defined(CHIP_BEST2000) || defined(CHIP_BEST1000)
    ret = false;
#else
    ret = true;
#endif

    return ret;
}

bool bt_drv_is_esco_auto_accept_support(void)
{
    bool ret = false;

#ifdef CHIP_BEST2300P
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2)
        ret = true;
#elif CHIP_BEST1400
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_4)
        ret = true;
#elif CHIP_BEST1402
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_1)
        ret = true;
#elif defined(CHIP_BEST2000) || defined(CHIP_BEST1000) || defined(CHIP_BEST2300)
    ret = false;
#else
    ret = true;
#endif

    return ret;
}

void btdrv_delay_for_wakeup(void)
{
#ifndef BT_CLK_UNIT_IN_HSLOT
    osDelay(2000);
#endif
}

bool btdrv_is_acl_ecc_softbit_support(void)
{
    bool ret = false;
#ifdef CHIP_BEST2300A
        ret = true;
#elif CHIP_BEST1501
        ret = true;
#elif CHIP_BEST2003
        ret = true;
#endif

    return ret;
}


void btdrv_softbit_enable(uint16_t connhdl, uint8_t type1,uint8_t type2,uint8_t type3, uint8_t num)
{
    if(btdrv_is_acl_ecc_softbit_support())
    {
#if (defined(SOFTBIT_EN))
        btdrv_softbit_config(connhdl,  type1, type2, type3,  num);
#endif
    }
}

void bt_drv_config_after_hci_reset(void)
{
    bt_drv_extra_config_after_init();
    btdrv_enable_tports();
}

bool bt_drv_is_bes_fa_mode_en(void)
{
    bool ret = false;
#ifdef __BES_FA_MODE__
    ret = true;
#endif
    return ret;
}

bool bt_drv_is_support_multipoint_ibrt(void)
{
    bool ret = false;
#if defined(BT_CLK_UNIT_IN_HSLOT)
    ret = true;
#endif
    return ret;
}

void bt_drv_enable_hw_spi(uint8_t elt_idx, bool hwspien)
{
#if defined(CHIP_BEST2300A)
    //only for 2300A fast lock
    bt_drv_reg_op_hw_spi_en_setf(elt_idx, hwspien);
#endif
}

void* bt_drv_get_btstack_chip_config(void)
{
    return &g_bt_drv_btstack_chip_config;
}

void bt_drv_hwspi_select(uint8_t link_id, uint8_t spi_idx)
{
#ifdef __FA_RX_GAIN_CTRL__
    // 1400 1402 1302 fix fast ack RX gain
    btdrv_spi_trig_select(link_id, spi_idx);
#endif
}

void bt_drv_rssi_dump_handler(void)
{
    bool ret = false;
    rx_agc_t bt_agc ={0,0xf};
    for(int i = 0; i< TEST_MODE_MAX_NB_ACTIVE_ACL; i++)
    {
        ret = bt_drv_reg_op_read_rssi_in_dbm(BT_ACL_CONHDL_BIT+i, &bt_agc);
        if(ret)
        {
            BT_DRV_TRACE(3,"BT=0x%x,rssi=%d,gain=%d,3m_gain=%d",(BT_ACL_CONHDL_BIT+i),
                bt_agc.rssi, bt_agc.rxgain,bt_drv_reg_op_read_bt_3m_rxgain());
        }
        bt_agc.rssi = 0;
        bt_agc.rxgain = 0xf;
    }

    rx_agc_t ble_agc ={0,0xf};
    bt_drv_reg_op_read_ble_rssi_in_dbm(0,&ble_agc);
    BT_DRV_TRACE(2,"BLE rssi=%d,gain=%d\n", ble_agc.rssi, ble_agc.rxgain);
}

void bt_drv_rx_status_dump(uint16_t conhdl)
{
    BT_DRV_TRACE(2,"%s, hdl=0x%x",__func__, conhdl);
    bt_drv_rssi_dump_handler();
    bt_drv_reg_op_dump_rx_record();
    bt_drv_reg_op_multi_ibrt_sche_dump();
}

void bt_drv_testmode_adaptive_adjust_param(void)
{
    //only for 1305
    bt_drv_reg_op_adjust_rf_param_for_testmode();
    //for 1501 2003
    bt_drv_reg_op_i2v_checker_for_testmode();
}

#ifndef KERNEL_LITEOS_M
size_t  memcpy_s(void *dst,size_t dstMax,const void *src, size_t srcMax)
{
    memcpy(dst, src, srcMax);
    return 0;
}

size_t memset_s(dst0,dstMax, c0, srcMax)
                void *dst0;
                size_t dstMax;
                int c0;
                size_t srcMax;
{
    memset(dst0, c0, srcMax);
    return 0;
}
#endif
void btdrv_le_rx_test_v2(uint8_t rx_channel, uint8_t phy, uint8_t mod_idx)
{
    BT_DRV_TRACE(0, "LE_RX_TEST: RX channel=%d, PHY=%d, mod idx=%d", rx_channel, phy, mod_idx);
    uint8_t hci_cmd[] =
    {
        0x01, 0x33, 0x20, 0x03, 0x00, 0x00, 0x00
    };

    hci_cmd[4] = rx_channel;
    hci_cmd[5] = phy;
    hci_cmd[6] = mod_idx;

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_le_tx_test_v2(uint8_t tx_channel, uint8_t data_len, uint8_t pkt_payload, uint8_t phy)
{
    BT_DRV_TRACE(0, "LE_TX_TEST: TX channel=%d,data len=%d, payload=%d, PHY=%d", tx_channel, data_len, pkt_payload, phy);
    uint8_t hci_cmd[] =
    {
        0x01, 0x34, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00
    };

    hci_cmd[4] = tx_channel;
    hci_cmd[5] = data_len;
    hci_cmd[6] = pkt_payload;
    hci_cmd[7] = phy;

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_le_test_end(void)
{
    BT_DRV_TRACE(0, "LE TEST END!!");
    uint8_t hci_cmd[] =
    {
        0x01, 0x1f, 0x20, 0x00
    };

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_le_rx_test_v2_bystack(uint8_t rx_channel, uint8_t phy, uint8_t mod_idx)
{
    BT_DRV_TRACE(0, "LE_RX_TEST: RX channel=%d, PHY=%d, mod idx=%d", rx_channel, phy, mod_idx);
    uint8_t hci_cmd[] =
    {
        0x01, 0x33, 0x20, 0x03, 0x00, 0x00, 0x00
    };

    hci_cmd[4] = rx_channel;
    hci_cmd[5] = phy;
    hci_cmd[6] = mod_idx;

    btdrv_stack_send_hci_cmd_data_bystack(hci_cmd);
}

void btdrv_le_tx_test_v2_bystack(uint8_t tx_channel, uint8_t data_len, uint8_t pkt_payload, uint8_t phy)
{
    BT_DRV_TRACE(0, "LE_TX_TEST: TX channel=%d,data len=%d, payload=%d, PHY=%d", tx_channel, data_len, pkt_payload, phy);
    uint8_t hci_cmd[] =
    {
        0x01, 0x34, 0x20, 0x04, 0x00, 0x00, 0x00, 0x00
    };

    hci_cmd[4] = tx_channel;
    hci_cmd[5] = data_len;
    hci_cmd[6] = pkt_payload;
    hci_cmd[7] = phy;

    btdrv_stack_send_hci_cmd_data_bystack(hci_cmd);
}

void btdrv_le_test_end_bystack(void)
{
    BT_DRV_TRACE(0, "LE TEST END!!");
    uint8_t hci_cmd[] =
    {
        0x01, 0x1f, 0x20, 0x00
    };

    btdrv_stack_send_hci_cmd_data_bystack(hci_cmd);
}


void btdrv_nosig_tx_test_set(uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length,
                                uint32_t tx_packet_num)
{
    BT_DRV_TRACE(0, "nosig_tx_test: hopping_mode=%d,whitening_mode=%d,tx_freq=%d,",hopping_mode, whitening_mode, tx_freq);
    BT_DRV_TRACE(0, "nosig_tx_test: rx_freq=%d,power_level=%d,lt_addr=%d,edr_enabled=%d",rx_freq, power_level,lt_addr, edr_enabled);
    BT_DRV_TRACE(0, "nosig_tx_test: packet_type=%d,payload_pattern=%d,payload_length=0x%02x",packet_type, payload_pattern, payload_length);
    BT_DRV_TRACE(0,"nosig_tx_test: bd_addr:");
    BT_DRV_DUMP("%02x ",bd_addr,6);
    uint8_t hci_cmd[] =
    {
        0x01, 0x87, 0xfc, 0x1c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
    };

    hci_cmd[7] = hopping_mode;
    hci_cmd[8] = whitening_mode;
    hci_cmd[9] = tx_freq;
    hci_cmd[10] = rx_freq;
    hci_cmd[11] = power_level;
    memcpy(&hci_cmd[12],bd_addr,6);
    hci_cmd[18] = lt_addr;
    hci_cmd[19] = edr_enabled;
    hci_cmd[20] = packet_type;
    hci_cmd[21] = payload_pattern;
    memcpy(&hci_cmd[22],&payload_length,2);
    memcpy(&hci_cmd[28],&tx_packet_num,4);

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}


void btdrv_nosig_rx_test_set(uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                const uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type,
                                uint8_t payload_pattern, uint16_t payload_length)
{
    BT_DRV_TRACE(0, "nosig_rx_test: hopping_mode=%d,whitening_mode=%d,tx_freq=%d,", hopping_mode, whitening_mode, tx_freq);
    BT_DRV_TRACE(0, "nosig_rx_test: rx_freq=%d,power_level=%d,lt_addr=%d,edr_enabled=%d",rx_freq, power_level,lt_addr, edr_enabled);
    BT_DRV_TRACE(0, "nosig_rx_test: packet_type=%d,payload_pattern=%d,payload_length=0x%02x",packet_type, payload_pattern, payload_length);
    BT_DRV_TRACE(0,"nosig_rx_test: bd_addr:");
    BT_DRV_DUMP("%02x ",bd_addr,6);
    uint8_t hci_cmd[] =
    {
        0x01, 0x87, 0xfc, 0x1c, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
    };

    hci_cmd[7] = hopping_mode;
    hci_cmd[8] = whitening_mode;
    hci_cmd[9] = tx_freq;
    hci_cmd[10] = rx_freq;
    hci_cmd[11] = power_level;
    memcpy(&hci_cmd[12],bd_addr,6);
    hci_cmd[18] = lt_addr;
    hci_cmd[19] = edr_enabled;
    hci_cmd[20] = packet_type;
    hci_cmd[21] = payload_pattern;
    memcpy(&hci_cmd[22],&payload_length,2);

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_nosig_stop_rx_test_set(void)
{
    BT_DRV_TRACE(0, "nosig_stop_rx_test");
    uint8_t hci_cmd[] =
    {
        0x01, 0x87, 0xfc, 0x1c, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
    };

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_nosig_tx_test_set_for_1300(uint16_t test_period, uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length)
{
    BT_DRV_TRACE(0, "nosig_tx_test: test_period=%02x,hopping_mode=%d,whitening_mode=%d,tx_freq=%d,",test_period, hopping_mode, whitening_mode, tx_freq);
    BT_DRV_TRACE(0, "nosig_tx_test: rx_freq=%d,power_level=%d,lt_addr=%d,edr_enabled=%d",rx_freq, power_level,lt_addr, edr_enabled);
    BT_DRV_TRACE(0, "nosig_tx_test: packet_type=%d,payload_pattern=%d,payload_length=0x%02x",packet_type, payload_pattern, payload_length);
    BT_DRV_TRACE(0,"nosig_tx_test: bd_addr:");
    BT_DRV_DUMP("%02x ",bd_addr,6);
    uint8_t hci_cmd[] =
    {
        0x01, 0x87, 0xfc, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    memcpy(&hci_cmd[5],&test_period,2);
    hci_cmd[7] = hopping_mode;
    hci_cmd[8] = whitening_mode;
    hci_cmd[9] = tx_freq;
    hci_cmd[10] = rx_freq;
    hci_cmd[11] = power_level;
    memcpy(&hci_cmd[12],bd_addr,6);
    hci_cmd[18] = lt_addr;
    hci_cmd[19] = edr_enabled;
    hci_cmd[20] = packet_type;
    hci_cmd[21] = payload_pattern;
    memcpy(&hci_cmd[22],&payload_length,2);

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

void btdrv_nosig_rx_test_set_for_1300(uint16_t test_period, uint8_t hopping_mode, uint8_t whitening_mode, uint8_t tx_freq, uint8_t rx_freq, uint8_t power_level,
                                uint8_t *bd_addr, uint8_t lt_addr, uint8_t edr_enabled, uint8_t packet_type, uint8_t payload_pattern, uint16_t payload_length)
{
    BT_DRV_TRACE(0, "nosig_rx_test: test_period=%02x,hopping_mode=%d,whitening_mode=%d,tx_freq=%d,",test_period, hopping_mode, whitening_mode, tx_freq);
    BT_DRV_TRACE(0, "nosig_rx_test: rx_freq=%d,power_level=%d,lt_addr=%d,edr_enabled=%d",rx_freq, power_level,lt_addr, edr_enabled);
    BT_DRV_TRACE(0, "nosig_rx_test: packet_type=%d,payload_pattern=%d,payload_length=0x%02x",packet_type, payload_pattern, payload_length);
    BT_DRV_TRACE(0,"nosig_rx_test: bd_addr:");
    BT_DRV_DUMP("%02x ",bd_addr,6);
    uint8_t hci_cmd[] =
    {
        0x01, 0x87, 0xfc, 0x14, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };

    memcpy(&hci_cmd[5],&test_period,2);
    hci_cmd[7] = hopping_mode;
    hci_cmd[8] = whitening_mode;
    hci_cmd[9] = tx_freq;
    hci_cmd[10] = rx_freq;
    hci_cmd[11] = power_level;
    memcpy(&hci_cmd[12],bd_addr,6);
    hci_cmd[18] = lt_addr;
    hci_cmd[19] = edr_enabled;
    hci_cmd[20] = packet_type;
    hci_cmd[21] = payload_pattern;
    memcpy(&hci_cmd[22],&payload_length,2);

    btdrv_SendData(hci_cmd, sizeof(hci_cmd));
}

freqcalib_mode_get_handler_func freqcalib_mode_get_callback = NULL;
void btdrv_register_freqcalib_mode_get_callback(freqcalib_mode_get_handler_func cb)
{
    if(cb != NULL) {
        freqcalib_mode_get_callback = cb;
    }
}

bool btdrv_freqcalib_mode_get(void)
{
    if(freqcalib_mode_get_callback != NULL) {
        return freqcalib_mode_get_callback();
    }
    return false;
}

btdrv_temp_compensation_handler btdrv_temp_compensation_callback = NULL;
void btdrv_register_temp_compensation_callback(btdrv_temp_compensation_handler cb)
{
    if(cb != NULL) {
        btdrv_temp_compensation_callback = cb;
    }
}

void btdrv_compensate_pwr_based_on_temp(void)
{
    if(btdrv_temp_compensation_callback != NULL) {
        btdrv_temp_compensation_callback();
    }
}


bool WEAK bt_drv_reg_op_check_link_exist(uint16_t con_hdl)
{
    return false;
}

uint8_t WEAK btdrv_reg_op_isohci_in_nb_buffer(uint8_t link_id)
{
    return 0;
}

void WEAK btdrv_regop_dump_btpcm_reg(void)
{
}

void WEAK btdrv_regop_force_preferred_rate_send(uint16_t conhdl)
{
}

void WEAK btdrv_set_btpcm_mute_pattern(uint16_t pattern)
{
}

void WEAK bt_drv_reg_op_write_ble_p256(uint8_t* private_key)
{
}

bool WEAK bt_drv_reg_op_read_fa_rssi_in_dbm(rx_agc_t* rx_val)
{
    return false;
}

void WEAK btdrv_set_bdr_ble_txpower(uint8_t txpwr_idx, uint16_t n)
{
}

int8_t WEAK btdrv_ultra_lowpwr_rssidbm(int8_t adv_tx_pwr, uint8_t den_step)
{
    return adv_tx_pwr;
}

uint8_t WEAK bt_drv_reg_op_get_max_acl_nb(void)
{
    return 3;
}
WEAK void iomux_restore_function_on_tport_disabled(void)
{
    hal_iomux_set_uart0();
}

WEAK void btdrv_bridge_send_data(const uint8_t *buff,uint8_t len)
{
}

WEAK void bt_drv_reg_op_set_page_device_support_le_audio(bool support)
{
}

void bt_drv_ctrl_tports(bool en)
{
    if(en)
    {
        bt_drv_extra_config_after_init();
        btdrv_enable_tports();
    }
    else
    {
        iomux_restore_function_on_tport_disabled();
    }
}

extern const char *BT_LOCAL_NAME;
extern const char *BLE_DEFAULT_NAME;
extern uint8_t ble_global_addr[6];
extern uint8_t bt_global_addr[6];

unsigned char *bt_get_local_address(void)
{
    return bt_global_addr;
}

void bt_set_local_address(unsigned char* btaddr)
{
    if (btaddr != NULL) {
        memcpy(bt_global_addr, btaddr, 6);
    }
}

void bt_set_ble_local_address(uint8_t* bleAddr)
{
    if (bleAddr)
    {
        memcpy(ble_global_addr, bleAddr, 6);
    }
}

unsigned char *bt_get_ble_local_address(void)
{
    return ble_global_addr;
}

const char *bt_get_local_name(void)
{
    return BT_LOCAL_NAME;
}

void bt_set_local_name(const char* name)
{
    if (name != NULL) {
        BT_LOCAL_NAME = name;
    }
}

const char *bt_get_ble_local_name(void)
{
    return BLE_DEFAULT_NAME;
}

void bt_set_ble_local_name(const char *name)
{
    if (name != NULL)
    {
        BLE_DEFAULT_NAME = name;
    }
}

#ifdef NO_MULTI_HEAP
#include <stdlib.h>
#else
#include "heap_api.h"
#include "hal_location.h"

#define BT_DRV_HEAP_MAX_SIZE                (78*1024)
static heap_handle_t POSSIBLY_UNUSED g_bt_drv_heap;

#endif /* NO_MULTI_HEAP */

void bt_drv_heap_init(void)
{
#if defined(PROGRAMMER)

#elif defined(NO_MULTI_HEAP)
#else
    uint8_t *heap_addr;

    g_bt_drv_heap = NULL;

    // heap_size = syspool_total_size();
    // if (heap_size < BT_DRV_HEAP_MAX_SIZE) {
    //     TR_WARN(0, " ");
    //     TR_WARN(0, "*** WARNING:%s: bt DRV pool too small: %u (< %u)", __func__, heap_size, BT_DRV_HEAP_MAX_SIZE);
    //     TR_WARN(0, " ");
    // } else {
    //     heap_size = BT_DRV_HEAP_MAX_SIZE;
    // }
    syspool_init_specific_size(BT_DRV_HEAP_MAX_SIZE);
    syspool_get_buff(&heap_addr, BT_DRV_HEAP_MAX_SIZE);
    ASSERT(heap_addr, "%s: Failed to get buffer from BT DRV pool: size=%u", __func__, BT_DRV_HEAP_MAX_SIZE);

    if (heap_addr) {
        g_bt_drv_heap = heap_register(heap_addr, BT_DRV_HEAP_MAX_SIZE);
        ASSERT(g_bt_drv_heap, "%s: Failed to register BT DRV heap: ptr=%p size=%u", __func__, heap_addr, BT_DRV_HEAP_MAX_SIZE);
    }
#endif
}

void btdrv_common_init(void)
{
    TRACE(2, "%s hw_metal_id:%d chip_metal_id:%d", __func__, read_hw_metal_id(), hal_get_chip_metal_id());
    bt_drv_heap_init();
}

bool bt_drv_work_in_ramrun_mode(void)
{
    bool status = false;
#if defined(__BT_RAMRUN__) || defined(__BT_RAMRUN_NEW__)
    status = true;
#endif
    return status;
}

bool WEAK bt_drv_is_support_hci_set_tws_link(void)
{
    bool ret = bt_drv_work_in_ramrun_mode();

    return ret;
}

bool WEAK bt_drv_is_support_hci_config_multi_ibrt_sche(void)
{
    return false;
}

bool WEAK bt_drv_is_support_set_bt_ble_active_link_switch(void)
{
    bool ret = false;

    if(bt_drv_work_in_ramrun_mode())
    {
        // if support rom version, add them
        ret = true;
    }
#if CHIP_BEST1306
    if (hal_get_chip_metal_id() >= HAL_CHIP_METAL_ID_2)
    {
        // if support rom version, add them
        ret = true;
    }
#endif
    return ret;
}

bool WEAK bt_drv_is_support_hci_enable_btpcm(void)
{
    return bt_drv_work_in_ramrun_mode();
}

uint32_t bt_drv_reg_op_get_multi_ibrt_slice(uint8_t active)
{
    uint32_t timeslice = MULTI_IBRT_BG_SLICE;
    if(active == 1)
    {
        timeslice = MULTI_IBRT_FG_SLICE;//active timeslice
    }
    else
    {
        timeslice = MULTI_IBRT_BG_SLICE;//inactive timeslice
    }

    return timeslice;
}

void *bt_drv_malloc(unsigned int size)
{
#if defined(PROGRAMMER)
    return NULL;
#elif defined(NO_MULTI_HEAP)
    return malloc(size);
#else
    return heap_malloc(g_bt_drv_heap, size);
#endif
}

void bt_drv_free(void *ptr)
{
#if defined(PROGRAMMER)
    return;
#elif defined(NO_MULTI_HEAP)
    return free(ptr);
#else
    return heap_free(g_bt_drv_heap, ptr);
#endif
}

#define Dec2Hex(A) ((A)>99 ? 0:(((A)/10)<<4)+(A) % 10)
static uint32_t btc_sw_verison = 0;
static void bt_drv_update_btc_sw_version(uint32_t sw_version)
{
    uint8_t year   = Dec2Hex((sw_version >> 9) & 0x7F);
    uint8_t month  = Dec2Hex((sw_version >> 5) & 0xF);
    uint8_t day    = Dec2Hex(sw_version & 0x1F);
    btc_sw_verison = (uint32_t)year << 16 | (uint32_t)month << 8 | day ;
    TRACE(3, "%s,0x%x to 0x%x", __func__, sw_version, btc_sw_verison);
}

uint32_t bt_drv_get_btc_sw_version(void)
{
    TRACE(2, "%s=0x%x", __func__, btc_sw_verison);
    return btc_sw_verison;
}

typedef struct
{
    uint16_t    pkt_counters;
    uint16_t    head_errors;
    uint16_t    payload_errors;
    int16_t    avg_estsw;
    int16_t    avg_esttpl;
    uint32_t    payload_bit_errors;
}bt_tester_nosig_test_evt_ind_t;

uint8_t nosig_rx_test_flag = 0;
void bt_tester_nonsig_test_result_save(const unsigned char *data, unsigned int len)
{
    int sRet = 0;
    uint32_t per = 0;
    uint32_t little = 0;

    bt_tester_nosig_test_evt_ind_t base_test_result_bt;
    bt_tester_nosig_test_evt_ind_t *pBaseResultBt = NULL;

    if(nosig_rx_test_flag == 2)
    {
        nosig_rx_test_flag = 0;
        pBaseResultBt = (bt_tester_nosig_test_evt_ind_t *)(data + 7);
        if (pBaseResultBt->pkt_counters != 0) {
            sRet = memcpy_s(&base_test_result_bt, sizeof(base_test_result_bt), pBaseResultBt, sizeof(base_test_result_bt));
            if(sRet != 0)
            {
                TRACE(0,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
            }

            TRACE(0,"nosig rx result:");
            TRACE(0,"Pkt Total Received [%d]", base_test_result_bt.pkt_counters);
            TRACE(0,"Pkt Head Error [%d]", base_test_result_bt.head_errors);
            TRACE(0,"Pkt Payload Error [%d]", base_test_result_bt.payload_errors);
            per = (1000 - base_test_result_bt.pkt_counters + base_test_result_bt.payload_errors + base_test_result_bt.head_errors) / 10;
            little = (1000 - base_test_result_bt.pkt_counters + base_test_result_bt.payload_errors + base_test_result_bt.head_errors) % 10;
            TRACE(0,"Per [%d.%d%%]", per, little);
        }
        else
        {
            TRACE(0,"nosig rx result: no pkt received");
        }
    }
}

void bt_tester_cmd_receive_evt_analyze(const unsigned char *data, unsigned int len)
{
    if(data[0] == 0x04 && data[4] == 0x09 && data[5] == 0x10)
    {
        TRACE(0,"CMD_RD_ADDR:");
        DUMP8("0x%02x ",&data[7],6);//04 0e 0a 05 09 10 00 12 34 56 78 90 ab
    }

    if(data[0] == 0x04 && data[4] == 0x03 && data[5] == 0x10)
    {
        TRACE(0,"CMD_RD_BT_LOCAL_FEAT:");
        DUMP8("0x%02x ",&data[7],8);//04 0e 0c 05 03 10 00 bf ee cd fa db bd 7b 87
    }

    if(data[0] == 0x04 && data[4] == 0x1b && data[5] == 0x0c)
    {
        uint16_t pscan_interval = (uint16_t)(data[8] << 8) | data[7];
        uint16_t pscan_window = (uint16_t)(data[10] << 8) | data[9];
        TRACE(0,"CMD_RD_PSCAN_ACTIVITY: interval=0x%02x,window=0x%02x",pscan_interval,pscan_window);
        //04 0e 08 05 1b 0c 00 40 00 12 00
    }

    if(data[0] == 0x04 && data[4] == 0x1d && data[5] == 0x0c)
    {
        uint16_t iscan_interval = (uint16_t)(data[8] << 8) | data[7];
        uint16_t iscan_window = (uint16_t)(data[10] << 8) | data[9];
        TRACE(0,"CMD_RD_ISCAN_ACTIVITY: interval=0x%02x,window=0x%02x",iscan_interval,iscan_window);
        //04 0e 08 05 1d 0c 00 40 00 12 00
    }

    if(data[0] == 0x04 && data[4] == 0x1f && data[5] == 0x20)
    {
        uint16_t per = 0;
        uint16_t num_pkts = (uint16_t)(data[8] << 8) | data[7];
        per = 1000 - data[8] * 0x100 + data[7];
        TRACE(0,"CMD_BLE_TEST_END: num_pkts=0x%x, per=%d",num_pkts,per);//04 0e 06 05 1f 20 00 00 00
    }

    if(data[0] == 0x04 && data[4] == 0x87 && data[5] == 0xfc)
    {
        bt_tester_nonsig_test_result_save(data,len);
    }

    if(data[0] == 0x04 && data[4] == 0x01 && data[5] == 0x10)
    {
        if(len == 0xf)
        {
            uint16_t hci_build_time = (uint16_t)(data[9] << 8) | data[8];
            bt_drv_update_btc_sw_version(hci_build_time);
        }
    }
}

#ifdef MCU_WAKEUP_BT_V2
static volatile uint32_t intersys1_tx_done_flag = INTERSYS_TX_DONE;

void bt_intersys_oper_wait_bt_response(void)
{
    uint32_t loop_cnt = 0;

    // Check if the intersys wakeup flag is set
    while(hal_intersys_get_wakeup_flag())
    {
        // If loop count exceeds the limit, trigger ASSERT
        if (++loop_cnt >= 30)
        {
            ASSERT(0, "BT: Wake up BT fail,tx done=%d", intersys1_tx_done_flag);
            break;
        }
        uint32_t wait_time = 10*loop_cnt;
        hal_sys_timer_delay_us(wait_time);
        //BT_DRV_TRACE(0, "cnt=%d, wait time=%d", loop_cnt,wait_time);
    }
    //BT_DRV_TRACE(0, "%s", __func__);
    intersys1_tx_done_flag = true;
}

static uint8_t bt_wakeup_send_msg[MCU_WAKEUP_BT_MSG_LEN];
void bt_intersys_oper_btcore(enum WAKEUP_BT_USER_T user, enum WAKEUP_BT_MSG_T msg)
{
    if (user >= WAKEUP_BT_USER_QTY || msg >= WAKEUP_BT_MSG_QTY)
    {
        ASSERT(0, "%s, user=%d, msg=%d, flag=%d", __func__, user, msg, intersys1_tx_done_flag)
    }

    if (intersys1_tx_done_flag == INTERSYS_WAIT_BTC_RESPONSE)
    {
        BT_DRV_TRACE(0, "%s fail", __func__);
        return;
    }

    uint32_t reg_op_user = user;
    uint32_t reg_op_msg = msg;

    // enum WAKEUP_BT_USER_T
    co_write32(&bt_wakeup_send_msg[0], reg_op_user);
    // bit 0 : wakeup BT and disable sleep
    // bit 1 : disable sleep (0) / resume sleep (1)
    // bit 2 : RFU
    co_write32(&bt_wakeup_send_msg[4], reg_op_msg);

    intersys1_tx_done_flag = INTERSYS_WAIT_BTC_RESPONSE;
    hal_intersys_send(HAL_INTERSYS_ID_1, HAL_INTERSYS_MSG_HCI, bt_wakeup_send_msg, MCU_WAKEUP_BT_MSG_LEN);
    //BT_DRV_TRACE(0, "%s user%d,msg=%d", __func__, user, msg);
}
#endif //MCU_WAKEUP_BT_V2