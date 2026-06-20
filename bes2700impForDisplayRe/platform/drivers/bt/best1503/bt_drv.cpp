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
#include "string.h"
#ifndef NOSTD
#include "math.h"
#endif
#include "plat_types.h"
#include "plat_addr_map.h"
#include "hal_i2c.h"
#include "hal_uart.h"
#include "bt_drv.h"
#include "bt_drv_internal.h"
#include "bt_drv_1306_internal.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_sleep.h"
#include "hal_intersys.h"
#include "hal_intersys.h"
#include "hal_trace.h"
#include "hal_psc.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_chipid.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "pmu.h"
#ifdef RX_IQ_CAL
#include "iqcorrect.h"
#endif
#include "tgt_hardware.h"
#include "hal_btdump.h"
#include "bt_drv_internal.h"
#include "bt_drv_1306_config.h"
#include "nvrecord_dev.h"
#ifdef __BT_RAMRUN__
#include "bt_drv_ramrun_symbol_1306_t0.h"

extern uint32_t __bt_ramrun_code_start_flash[];
extern uint32_t __bt_ramrun_code_end_flash[];

#endif
#include "bt_drv_dpd_mem.h"
#ifdef BTC_CPUDUMP_BASE
#include "bt_drv_cpudump.h"
#endif

extern "C" void hal_iomux_set_controller_log(void);
bool btdrv_dut_mode_enable = false;

static volatile uint32_t btdrv_tx_flag = 1;
void btdrv_tx(const unsigned char *data, unsigned int len)
{
    BT_DRV_TRACE(0,"tx");
    btdrv_tx_flag = 1;
}

#ifdef NORMAL_TEST_MODE_SWITCH
typedef int (*btdrv_testmode_nonsig_rx_process_func)(const unsigned char *data, unsigned int len);
btdrv_testmode_nonsig_rx_process_func btdrv_nonsig_test_result_callback = NULL;
void btdrv_testmode_register_nonsig_rx_process(btdrv_testmode_nonsig_rx_process_func cb)
{
    btdrv_nonsig_test_result_callback = cb;
}

static void btdrv_testmode_nonsig_rx_handler(const unsigned char *data, unsigned int len)
{
    if(btdrv_nonsig_test_result_callback !=NULL)
    {
        btdrv_nonsig_test_result_callback(data, len);
    }
}
#endif

#ifdef REDUCE_EDGE_CHL_TXPWR
// Reduce the edge channel tx power(chl0 & chl78), only single hop, optimize DEVM
static bool btdrv_reduce_edge_chl_txpwr_flag = false;
#define REDUCE_EDGE_CHL_TXPWR_1P5_DBM       3
#define BT_TX_TEST_CHL_0                    0x00
#define BT_TX_TEST_CHL_39                   0x27
#define BT_TX_TEST_CHL_78                   0x4e

void btdrv_reduce_edge_chl_txpwr_set(bool btdrv_reduce_edge_chl_txpwr_flag)
{
    uint16_t gsg_nom_q;
    uint16_t gsg_nom_i;
    uint16_t dsg_nom;
    uint16_t gsg_edr_num;
    if (btdrv_reduce_edge_chl_txpwr_flag == false) {
        BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x3FF, 0, gsg_nom_q);
        BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x3FF, 10, gsg_nom_i);
        BTDIGITAL_REG_GET_FIELD(0xD0350344, 0x3FF, 0, dsg_nom);
        BTDIGITAL_REG_GET_FIELD(0xd0350370, 0x3ff, 0, gsg_edr_num);
        gsg_nom_q -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_nom_i -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        dsg_nom -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_edr_num -= REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        BTDIGITAL_REG_SET_FIELD(0xD0350344, 0x3FF, 0, dsg_nom);
        BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x3FF, 0, gsg_nom_q);
        BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x3FF, 10, gsg_nom_i);
        BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff, 0, gsg_edr_num);
        BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff, 10, gsg_edr_num);
    } else if (btdrv_reduce_edge_chl_txpwr_flag == true) {
        BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x3FF, 0, gsg_nom_q);
        BTDIGITAL_REG_GET_FIELD(0xD0350308, 0x3FF, 10, gsg_nom_i);
        BTDIGITAL_REG_GET_FIELD(0xD0350344, 0x3FF, 0, dsg_nom);
        BTDIGITAL_REG_GET_FIELD(0xd0350370, 0x3ff, 0, gsg_edr_num);
        gsg_nom_q += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_nom_i += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        dsg_nom += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        gsg_edr_num += REDUCE_EDGE_CHL_TXPWR_1P5_DBM;
        BTDIGITAL_REG_SET_FIELD(0xD0350344, 0x3FF, 0, dsg_nom);
        BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x3FF, 0, gsg_nom_q);
        BTDIGITAL_REG_SET_FIELD(0xD0350308, 0x3FF, 10, gsg_nom_i);
        BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff, 0, gsg_edr_num);
        BTDIGITAL_REG_SET_FIELD(0xd0350370, 0x3ff, 10, gsg_edr_num);
    }
}

void btdrv_reduce_edge_chl_txpwr_signal_mode(const unsigned char *data)
{
    if(data[0] == 0x04 && data[1] == 0xff && data[3] == 0x18 && data[6] == 0x00 && (data[4] == BT_TX_TEST_CHL_0 || data[4] == BT_TX_TEST_CHL_78)
         && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if ((data[0] == 0x04 && data[1] == 0xff && data[3] == 0x18 && btdrv_reduce_edge_chl_txpwr_flag == true)
        && (data[6] == 0x01 || (data[4] != BT_TX_TEST_CHL_0 && data[4] != BT_TX_TEST_CHL_78))) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = false;
    }
}

void btdrv_reduce_edge_chl_txpwr_nosignal_bt_mode(const unsigned char *data)
{
    if(data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 0x00 && (data[10] == BT_TX_TEST_CHL_0 || data[10] == BT_TX_TEST_CHL_78)
        && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if(data[0] == 0x01 && data[1] == 0x87 && data[2] == 0xfc && data[3] == 0x1c && data[4] == 0x00 && data[10] != BT_TX_TEST_CHL_0 && data[10] != BT_TX_TEST_CHL_78
        && btdrv_reduce_edge_chl_txpwr_flag == true) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
         btdrv_reduce_edge_chl_txpwr_flag = false;
    }

}

void btdrv_reduce_edge_chl_txpwr_nosignal_ble_mode(const unsigned char *data)
{
    if(data[0] == 0x01 && data[1] == 0x1e && data[3] == 0x03 && data[6] == 0x02 && (data[4] == BT_TX_TEST_CHL_0 || data[4] == BT_TX_TEST_CHL_39)
         && btdrv_reduce_edge_chl_txpwr_flag == false) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = true;
    } else if (data[0] == 0x01 && data[1] == 0x1e && data[3] == 0x03 && data[6] == 0x02 && btdrv_reduce_edge_chl_txpwr_flag == true && data[4] != BT_TX_TEST_CHL_0 && data[4] != BT_TX_TEST_CHL_39) {
        btdrv_reduce_edge_chl_txpwr_set(btdrv_reduce_edge_chl_txpwr_flag);
        btdrv_reduce_edge_chl_txpwr_flag = false;
    }
}
#endif

void btdrv_dut_accessible_mode_manager(const unsigned char *data);

static unsigned int btdrv_rx(const unsigned char *data, unsigned int len)
{
    hal_intersys_stop_recv(HAL_INTERSYS_ID_0);
    if(len > 5 && data[0] == 0x04 && data[1] == 0xff &&
        data[3] == 0x01)
    {
        BT_DRV_TRACE(2,"%s", &data[5]);
    }
    else
    {
        BT_DRV_TRACE(2,"%s len:%d", __func__, len);
        BT_DRV_DUMP("%02x ", data, len>7?7:len);
    }
#ifdef NORMAL_TEST_MODE_SWITCH
    btdrv_testmode_nonsig_rx_handler(data, len);
#endif
    btdrv_dut_accessible_mode_manager(data);
    hal_intersys_start_recv(HAL_INTERSYS_ID_0);

#ifdef REDUCE_EDGE_CHL_TXPWR
    btdrv_reduce_edge_chl_txpwr_signal_mode(data);
    btdrv_reduce_edge_chl_txpwr_nosignal_ble_mode(data);
#endif

    return len;
}

////open intersys interface for hci data transfer
static bool hci_has_opened = false;

void btdrv_SendData(const uint8_t *buff,uint8_t len)
{
    if(hci_has_opened)
    {
        btdrv_tx_flag = 0;

        BT_DRV_TRACE(1,"%s", __func__);
        BT_DRV_DUMP("%02x ", buff, len);
        hal_intersys_send(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, buff, len);

        while( (btdrv_dut_mode_enable==0) && btdrv_tx_flag == 0);
    }
    else
    {
        //only for bridge mode
        btdrv_bridge_send_data(buff, len);
        btdrv_delay(50);
    }
}


void btdrv_hciopen(void)
{
    int ret = 0;

    if (hci_has_opened)
    {
        return;
    }

    hci_has_opened = true;

    ret = hal_intersys_open(HAL_INTERSYS_ID_0, HAL_INTERSYS_MSG_HCI, btdrv_rx, btdrv_tx, false);

    if (ret)
    {
        BT_DRV_TRACE(0,"Failed to open intersys");
        return;
    }

    hal_intersys_start_recv(HAL_INTERSYS_ID_0);
}

////open intersys interface for hci data transfer
void btdrv_hcioff(void)
{
    if (!hci_has_opened)
    {
        return;
    }
    hci_has_opened = false;

    hal_intersys_close(HAL_INTERSYS_ID_0,HAL_INTERSYS_MSG_HCI);
}

/*  btdrv power on or off the bt controller*/
void btdrv_poweron(uint8_t en)
{
    pmu_ldo_vpa_en(en);

    if (en) {
        //power on BTC
        hal_psc_bt_enable();
        hal_cmu_bt_clock_enable();
        hal_cmu_bt_reset_clear();
        hal_cmu_bt_module_init();
        btdrv_delay(10);
    } else {
        //power off BTC
        hal_cmu_bt_reset_set();
        hal_cmu_bt_clock_disable();
        hal_psc_bt_disable();
    }
}

void bt_drv_extra_config_after_init(void)
{
}

void btdrv_rxbb_rccal(void)
{
    uint16_t value = 0;
    uint16_t rccal_count = 0;
    const uint16_t base_count = 0x2041;
    float quotient = 0.0f;
    float factor;
    uint16_t filter_cap;
    uint16_t i2v_cap;
    uint16_t adc_cap;

    uint16_t rf_store_B8;
    uint16_t rf_store_172;
    uint32_t dig_store;

    dig_store = BTDIGITAL_REG(0xD0220C00);
    btdrv_read_rf_reg(0xB8, &rf_store_B8);
    btdrv_read_rf_reg(0x172, &rf_store_172);

    BTDIGITAL_REG_WR(0xD0220C00, 0x000A0080);       //rx on
    btdrv_delay(10);

    btdrv_write_rf_reg(0xB8, 0x7C00);
    BTRF_REG_SET_FIELD(0x172, 0x1, 5, 1);           //reg_rcosc_pu
    btdrv_delay(1);
    BTRF_REG_SET_FIELD(0x172, 0x1, 4, 1);           //reg_rccal_en
    btdrv_delay(10);

    btdrv_read_rf_reg(0x144, &value);
    BT_DRV_TRACE(2, "%s rf_103=0x%x", __func__, value);
    //bit12: dig_rccal_finish
    if (getbit(value, 15)) {
        rccal_count = value & 0x3FFF;                //dig_rccal_count[14:0]
        BT_DRV_TRACE(2, "%s rccal_count=0x%x", __func__, rccal_count);
    } else {
        BT_DRV_TRACE(1, "%s dig rccal not finish!!!", __func__);
        goto exit;
    }

    quotient = (float)((float)rccal_count / (float)base_count);
    TRACE(1, "quotient=%f", (double)quotient);
    if ((quotient > 0.7f) && (quotient < 1.3f)) {
        factor = quotient;
    } else {
        BT_DRV_TRACE(0, "quotient not in range [0.7, 1.3]");
        factor = 1.0f;
    }

    // flt cap
    BTRF_REG_GET_FIELD(0x27, 0xFF, 8, filter_cap);
    filter_cap = (uint16_t)((filter_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0x27, 0xFF, 8, filter_cap);  //1m
    BTRF_REG_GET_FIELD(0x13B, 0xFF, 0, filter_cap);
    filter_cap = (uint16_t)((filter_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0x13B, 0xFF, 0, filter_cap); //2m
    BTRF_REG_GET_FIELD(0x15D, 0xFF, 0, filter_cap);
    filter_cap = (uint16_t)((filter_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0x15D, 0xFF, 0, filter_cap); //8psk

    // i2v cap
    // bt_2m_table
    BTRF_REG_GET_FIELD(0x132, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x132, 0x7F, 5, i2v_cap);  //agc gain 0
    BTRF_REG_GET_FIELD(0x133, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x133, 0x7F, 5, i2v_cap);  //agc gain 1
    BTRF_REG_GET_FIELD(0x134, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x134, 0x7F, 5, i2v_cap);  //agc gain 2
    BTRF_REG_GET_FIELD(0x135, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x135, 0x7F, 5, i2v_cap);  //agc gain 3
    BTRF_REG_GET_FIELD(0x136, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x136, 0x7F, 5, i2v_cap);  //agc gain 4
    BTRF_REG_GET_FIELD(0x137, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x137, 0x7F, 5, i2v_cap);  //agc gain 5
    BTRF_REG_GET_FIELD(0x138, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x138, 0x7F, 5, i2v_cap);  //agc gain 6
    BTRF_REG_GET_FIELD(0x139, 0x7F, 5, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x139, 0x7F, 5, i2v_cap);  //agc gain 7

    //bt_3m_table
    BTRF_REG_GET_FIELD(0x162, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x162, 0x7F, 0, i2v_cap);  //agc gain 0
    BTRF_REG_GET_FIELD(0x162, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x162, 0x7F, 7, i2v_cap);  //agc gain 1
    BTRF_REG_GET_FIELD(0x163, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x163, 0x7F, 0, i2v_cap);  //agc gain 2
    BTRF_REG_GET_FIELD(0x163, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x163, 0x7F, 7, i2v_cap);  //agc gain 3
    BTRF_REG_GET_FIELD(0x164, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x164, 0x7F, 0, i2v_cap);  //agc gain 4
    BTRF_REG_GET_FIELD(0x164, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x164, 0x7F, 7, i2v_cap);  //agc gain 5
    BTRF_REG_GET_FIELD(0x165, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x165, 0x7F, 0, i2v_cap);  //agc gain 6
    BTRF_REG_GET_FIELD(0x165, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x165, 0x7F, 7, i2v_cap);  //agc gain 7

    // ble 2M table
    BTRF_REG_GET_FIELD(0x15E, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x15E, 0x7F, 0, i2v_cap);  //agc gain 0
    BTRF_REG_GET_FIELD(0x15E, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x15E, 0x7F, 7, i2v_cap);  //agc gain 1
    BTRF_REG_GET_FIELD(0x15F, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x15F, 0x7F, 0, i2v_cap);  //agc gain 2
    BTRF_REG_GET_FIELD(0x15F, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x15F, 0x7F, 7, i2v_cap);  //agc gain 3
    BTRF_REG_GET_FIELD(0x160, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x160, 0x7F, 0, i2v_cap);  //agc gain 4
    BTRF_REG_GET_FIELD(0x160, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x160, 0x7F, 7, i2v_cap);  //agc gain 5
    BTRF_REG_GET_FIELD(0x161, 0x7F, 0, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x161, 0x7F, 0, i2v_cap);  //agc gain 6
    BTRF_REG_GET_FIELD(0x161, 0x7F, 7, i2v_cap);
    i2v_cap = (uint16_t)((i2v_cap/factor)+0.5);
    i2v_cap = i2v_cap & 0x7E;
    BTRF_REG_SET_FIELD(0x161, 0x7F, 7, i2v_cap);  //agc gain 7

    // adc cap
    BTRF_REG_GET_FIELD(0xC2, 0xFF, 0, adc_cap);
    adc_cap = (uint16_t)((adc_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0xC2, 0xFF, 0, adc_cap);  //1M_1st
    BTRF_REG_GET_FIELD(0xC2, 0x3F, 8, adc_cap);
    adc_cap = (uint16_t)((adc_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0xC2, 0x3F, 8, adc_cap);  //1M_2nd
    BTRF_REG_GET_FIELD(0x13C, 0xFF, 0, adc_cap);
    adc_cap = (uint16_t)((adc_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0x13C, 0xFF, 0, adc_cap);  //2M_1st
    BTRF_REG_GET_FIELD(0x13C, 0x3F, 8, adc_cap);
    adc_cap = (uint16_t)((adc_cap/factor)+0.5);
    BTRF_REG_SET_FIELD(0x13C, 0x3F, 8, adc_cap);  //2M_2nd
 exit:
    //reset
    BTDIGITAL_REG_WR(0xD0220C00, dig_store);
    btdrv_write_rf_reg(0xB8, rf_store_B8);
    btdrv_write_rf_reg(0x172, rf_store_172);
}

#ifdef __PWR_FLATNESS__
#define PWR_FLATNESS_CONST_VAL                         0x11
void btdrv_channel_pwr_flatness(void)
{
    uint8_t rf_8c_val = 0;
    uint16_t rf_ed_val = 0;
    uint8_t RF_ED_9_6[3] = {0};
    uint8_t RF_2A1_9_6 = 0;
    uint32_t Channel[3] = {0xA0000, 0xA0027, 0xA004E};
    uint8_t average_value = 0;

    BTRF_REG_GET_FIELD(0x8c, 0xffff,0,rf_8c_val);
    TRACE(2, "original 0x8C val=0x%x",rf_8c_val);
    BTRF_REG_SET_FIELD(0x8c, 0x1,0,1);          //reg_bt_pa_ldo_pu_dr

    for(int i=0; i<3; i++)
    {
        BTDIGITAL_REG(0xD0220C00)  = Channel[i];
        btdrv_delay(1);

        BTRF_REG_GET_FIELD(0xED, 0xffff, 0, rf_ed_val);
        if (0x0400 & rf_ed_val) //bit10 txpll_capbank_done
        {
            BTRF_REG_GET_FIELD(0xED, 0xf, 6, RF_ED_9_6[i]);
            TRACE(2, "RF_ED_9_6[%d]=0x%x",i,RF_ED_9_6[i]);
        }
        else
        {
            TRACE(2, "[%d]txpll capbank not done,and 0xED=0x%x", i, rf_ed_val);
            return;
        }
    }

    average_value = (RF_ED_9_6[0] + RF_ED_9_6[1] + RF_ED_9_6[2]) / 3 ;
    RF_2A1_9_6 = PWR_FLATNESS_CONST_VAL - average_value;

    if(average_value > PWR_FLATNESS_CONST_VAL)
        RF_2A1_9_6 = 8;

    TRACE(1, "average_value=0x%x and RF_2A1_9_6=0x%x.",average_value,RF_2A1_9_6);
    BTRF_REG_SET_FIELD(0x2A1, 0xf, 0,RF_2A1_9_6);

    BTRF_REG_SET_FIELD(0x8c, 0xffff,0,rf_8c_val);
    BTDIGITAL_REG(0xD0220C00) = 0;
}
#endif

void btdrv_i2v_dccal(void)
{
    uint16_t read_value;
    uint8_t rf_f8_11_6, rf_f8_5_0 = 0;
    uint8_t reg_dc = 0;

    BTRF_REG_SET_FIELD(0xC4,0x3f,2,0x32);
    btdrv_read_rf_reg(0xC4, &read_value);
    BT_DRV_TRACE(1, "write reg 0xC4 val=%x", read_value);

    BTRF_REG_SET_FIELD(0xBD,0x3,4,0x2);
    btdrv_read_rf_reg(0xBD,&read_value);
    BT_DRV_TRACE(1,"write reg 0xBD val=%x",read_value);

    BTRF_REG_SET_FIELD(0x8C,0xf,6,0xD);
    btdrv_read_rf_reg(0x8C,&read_value);
    BT_DRV_TRACE(1,"write reg 0x8C val=%x",read_value);

    BTDIGITAL_REG(0xd0220c00) = 0x000A0080;
    btdrv_delay(1);

    //reg_bt_i2v_dccal_ready_dr
    BTRF_REG_SET_FIELD(0xBD,0x1,0,0);
    btdrv_read_rf_reg(0xBD,&read_value);
    BT_DRV_TRACE(1,"write reg 0xBD val=%x",read_value);

    //reg_rxgain_dr
    BTRF_REG_SET_FIELD(0xB8,0x1,0,1);
    btdrv_read_rf_reg(0xB8,&read_value);
    BT_DRV_TRACE(1,"write reg 0xB8 val=%x",read_value);

    //reg_bt_trx_cal_en
    BTRF_REG_SET_FIELD(0xBE,0x1,0,1);
    btdrv_read_rf_reg(0xBE,&read_value);
    BT_DRV_TRACE(1,"write reg 0xBE val=%x",read_value);

    for(int i=0; i<8; i++)
    {
        //reg_i2v_dc_cal_en
        BTRF_REG_SET_FIELD(0xC4,0x1,0,0);
        btdrv_read_rf_reg(0xC4, &read_value);
        BT_DRV_TRACE(1, "write reg 0xC4 val=%x", read_value);

        btdrv_delay(1);
        BTRF_REG_SET_FIELD(0xB8,0x7,1,i);
        btdrv_read_rf_reg(0xB8, &read_value);
        BT_DRV_TRACE(1, "write reg 0xB8 val=%x", read_value);

        //reg_bt_rxlowifdc_cal_resetn
        BTRF_REG_SET_FIELD(0xBE,0x1,8,1);
        btdrv_read_rf_reg(0xBE, &read_value);
        BT_DRV_TRACE(1, "write reg 0xBE val=%x", read_value);

        BTRF_REG_SET_FIELD(0xBE,0x1,8,0);
        btdrv_read_rf_reg(0xBE, &read_value);
        BT_DRV_TRACE(1, "write reg 0xBE val=%x", read_value);

        //reg_i2v_dc_cal_en
        BTRF_REG_SET_FIELD(0xC4,0x1,0,1);
        btdrv_read_rf_reg(0xC4, &read_value);
        BT_DRV_TRACE(1, "write reg 0xC4 val=%x", read_value);

        BTRF_REG_SET_FIELD(0xBE,0x1,8,1);
        btdrv_read_rf_reg(0xBE, &read_value);
        BT_DRV_TRACE(1, "write reg 0xBE val=%x", read_value);

        btdrv_delay(1);
        //i2v_dccal_value[12:0]
        btdrv_read_rf_reg(0xF8, &read_value);
        BT_DRV_TRACE(1, "read reg 0xF8 val=%x", read_value);
        if(read_value & 0x1000)
        {
            rf_f8_5_0 = (read_value & 0x3f);
            rf_f8_11_6 = ((read_value & 0xfc0) >> 6);
            BT_DRV_TRACE(2, "rf_f8_5_0=0x%x, rf_f8_11_6=0x%x", rf_f8_5_0,rf_f8_11_6);

            reg_dc = 0xc5 + i;
            BTRF_REG_SET_FIELD(reg_dc,0x3f,0,rf_f8_11_6);
            btdrv_read_rf_reg(reg_dc, &read_value);
            BT_DRV_TRACE(2, "write reg 0x%x[5:0] val=%x",reg_dc ,read_value);

            BTRF_REG_SET_FIELD(reg_dc,0x3f,6,rf_f8_5_0);
            btdrv_read_rf_reg(reg_dc, &read_value);
            BT_DRV_TRACE(2, "write reg 0x%x[11:6] val=%x",reg_dc ,read_value);
        }
    }

    //reg_bt_i2v_dccal_ready_dr
    BTRF_REG_SET_FIELD(0xBD,0x1,0,1);
    btdrv_read_rf_reg(0xBD,&read_value);
    BT_DRV_TRACE(1,"write reg 0xBD val=%x",read_value);

    BTRF_REG_SET_FIELD(0xB8,0xf,0,0);
    btdrv_read_rf_reg(0xB8,&read_value);
    BT_DRV_TRACE(1,"write reg 0xB8 val=%x",read_value);

    BTRF_REG_SET_FIELD(0xBE,0x1,8,0);
    btdrv_read_rf_reg(0xBE, &read_value);
    BT_DRV_TRACE(1, "write reg 0xBE val=%x", read_value);

    BTRF_REG_SET_FIELD(0xBE,0x1,0,0);
    btdrv_read_rf_reg(0xBE,&read_value);
    BT_DRV_TRACE(1,"write reg 0xBE val=%x",read_value);

    BTRF_REG_SET_FIELD(0x8C,0xf,6,0);
    btdrv_read_rf_reg(0x8C,&read_value);
    BT_DRV_TRACE(1,"write reg 0x8C val=%x",read_value);

    BTDIGITAL_REG(0xd0220c00) = 0;
}

void bt_drv_tx_dpd_write_memory(void)
{
    uint32_t i = 0;
    const int32_t * dpd_mem_p = bt_drv_dpd_mem;
    uint32_t dpd_mem_size = ARRAY_SIZE(bt_drv_dpd_mem);

    for (i=0; i<dpd_mem_size; i++) {
        BTDIGITAL_REG(0xC0520000+i*4) = dpd_mem_p[i];
    }
}

#ifndef NOSTD
const uint16_t tx_iq_rf_set[][2] =
{
    //[iq cal loop]
    {0x0F, 0x0030},  //pa on &amp;&amp; adc pu
    {0xC1, 0x1B83},  //padrv gain dr max
    {0x27, 0x8a04},
    {0x2B, 0x0f30},  //adc rst release
    {0x2C, 0x02F7},  //txflt gain dr max adc gain dr 0db
    {0x93, 0x0103},  //iqcal pu
    {0x94, 0x31b0},  //iqcal gain
    {0x9A, 0x0550},  //adcclk divn
    {0xBF, 0x3032},  //i2v pu
    {0x95, 0x0036},  //filter sel iqcal &amp; test en
    {0xBB, 0xFFFF},
    //{0xA6, 0x1E6A},  //tstbuf sel flt &amp; pu(for debug)
};

const uint16_t tx_iq_rf_store[][1] =
{
    {0x0F},  //pa on &amp;&amp; adc pu
    {0xC1},  //padrv gain dr max
    {0x27},
    {0x2B},  //adc rst release
    {0x2C},  //txflt gain dr max adc gain dr 0db
    {0x93},  //iqcal pu
    {0x94},  //iqcal gain
    {0x9A},  //adcclk divn
    {0xBF},  //i2v pu
    {0x95},
    {0xBB},
    //{0xA6},
};


void btdrv_tx_iq_manual_cal(void)
{
    uint32_t tx_iq_dig_store[10];
    uint32_t i;
    uint16_t value = 0;
    //int32_t i_data, q_data = 0;
    const uint16_t (*tx_iq_rf_set_p)[2];
    const uint16_t (*tx_iq_rf_store_p)[1];
    uint32_t rf_set_tbl_size = 0;
    uint16_t rf_local[ARRAY_SIZE(tx_iq_rf_store)];
    uint32_t rf_store_tbl_size = ARRAY_SIZE(tx_iq_rf_store);

    tx_iq_dig_store[0] = BTDIGITAL_REG(0xD0220C00);
    tx_iq_dig_store[1] = BTDIGITAL_REG(0xD0350028);
    tx_iq_dig_store[2] = BTDIGITAL_REG(0xD035002C);
    tx_iq_dig_store[3] = BTDIGITAL_REG(0xD0340020);
    tx_iq_dig_store[4] = BTDIGITAL_REG(0xD0330058);
    tx_iq_dig_store[5] = BTDIGITAL_REG(0xD0330060);
    tx_iq_dig_store[6] = BTDIGITAL_REG(0xD0310000);
    tx_iq_dig_store[7] = BTDIGITAL_REG(0xD0330064);
    tx_iq_dig_store[8] = BTDIGITAL_REG(0xD0350334);
    tx_iq_dig_store[9] = BTDIGITAL_REG(0xD0350248);
    tx_iq_rf_store_p = &tx_iq_rf_store[0];
    BT_DRV_TRACE(0,"rf reg_store:\n");
    for (i=0; i<rf_store_tbl_size; i++) {
        btdrv_read_rf_reg(tx_iq_rf_store_p[i][0],&value);
        rf_local[i] = value;
        //BT_DRV_TRACE(2,"rf reg=%x,v=%x",tx_iq_rf_store_p[i][0],value);
    }

    tx_iq_rf_set_p = &tx_iq_rf_set[0];
    rf_set_tbl_size = ARRAY_SIZE(tx_iq_rf_set);
    BT_DRV_TRACE(0,"rf reg_set:\n");
    for (i=0; i<rf_set_tbl_size; i++) {
        btdrv_write_rf_reg(tx_iq_rf_set_p[i][0],tx_iq_rf_set_p[i][1]);
        //BTRF_REG_DUMP(tx_iq_rf_set_p[i][0]);
    }

    //dig config
    //[loop back]
    BTDIGITAL_REG_WR(0xD0340020, 0x030E01C0);       //one tone set gain
    //[tone gen]
    BTDIGITAL_REG_WR(0xD0350028, 0x013F0012);
    BTDIGITAL_REG_WR(0xD035002C, 0x3);              //187.5k tone

    //DC
    BTDIGITAL_REG_WR(0xD0220C00, 0x800A0027);       //rx modem on

    BTDIGITAL_REG_SET_FIELD(0xD0350334, 0x1, 23, 1);
    BTDIGITAL_REG_SET_FIELD(0xD0350250, 0x1, 31, 0);
#ifdef RX_IQ_CAL
    //dc_iq_calib
    dc_iq_calib_1306();
#endif
    BT_DRV_TRACE(0,"reg_reset:\n");

    for (i=0; i<rf_store_tbl_size; i++) {
        btdrv_write_rf_reg(tx_iq_rf_store_p[i][0],rf_local[i]);
        //BT_DRV_TRACE(2,"rf reg=%x,v=%x",tx_iq_rf_store_p[i][0],value);
    }

    BTDIGITAL_REG_WR(0xD0220C00, tx_iq_dig_store[0]);
    BTDIGITAL_REG_WR(0xD0350028, tx_iq_dig_store[1]);
    BTDIGITAL_REG_WR(0xD035002C, tx_iq_dig_store[2]);
    BTDIGITAL_REG_WR(0xD0340020, tx_iq_dig_store[3]);
    BTDIGITAL_REG_WR(0xD0330058, tx_iq_dig_store[4]);
    BTDIGITAL_REG_WR(0xD0330060, tx_iq_dig_store[5]);
//    BTDIGITAL_REG_WR(0xD0310000, tx_iq_dig_store[6]);
    BTDIGITAL_REG_WR(0xD0330064, tx_iq_dig_store[7]);
    BTDIGITAL_REG_WR(0xD0350334, tx_iq_dig_store[8]);
    BTDIGITAL_REG_WR(0xD0350248, tx_iq_dig_store[9]);
}
#endif

struct TX_PWR_CALIB_ITEM
{
    uint32_t txpwr_dig_reg;
    uint16_t shift;
};

static const struct TX_PWR_CALIB_ITEM tx_pwr_calib[] =
{
    {0xD0350308,  0},
    {0xD0350308, 10},
    {0xD0350370,  0},
    {0xD0350370, 10},
    {0xD0350344,  0},
};

void btdrv_txpower_calib(void)
{
    uint16_t read_val;
    int16_t calib_val = 0;
    uint16_t reg_val;
    int16_t bt_pwr_freql, bt_pwr_freqm, bt_pwr_freqh;
    float bt_pwr_avg = 0.0f;
    uint8_t i;
    uint8_t calib_flag = 0;

    pmu_get_efuse(PMU_EFUSE_PAGE_RESERVED_7, &read_val);
    calib_flag = getbit(read_val, 12);

    bt_pwr_freql = read_val & 0x7;
    bt_pwr_freqm = (read_val >> 4) & 0x7;
    bt_pwr_freqh = (read_val >> 8) & 0x7;

    bt_pwr_freql = getbit(read_val, 3)  ? bt_pwr_freql : (-bt_pwr_freql);
    bt_pwr_freqm = getbit(read_val, 7)  ? bt_pwr_freqm : (-bt_pwr_freqm);
    bt_pwr_freqh = getbit(read_val, 11) ? bt_pwr_freqh : (-bt_pwr_freqh);

    if (calib_flag == 0) {
        bt_pwr_avg = bt_pwr_freql;
    } else if (calib_flag == 1) {
        bt_pwr_avg = ((float)(bt_pwr_freql + bt_pwr_freqm + bt_pwr_freqh) / 3.0f);
    }

    if (bt_pwr_avg > 0) {
        calib_val = (int16_t) (bt_pwr_avg + 0.5f);
    } else if (bt_pwr_avg < 0) {
        calib_val = (int16_t) (bt_pwr_avg - 0.5f);
    }
    TRACE(3, "%s efuse D val=0x%x, calib_val=%d",__func__, read_val, calib_val);

    if (calib_val > 0) {
        for (i = 0; i < ARRAY_SIZE(tx_pwr_calib); i++) {
            BTDIGITAL_REG_GET_FIELD(tx_pwr_calib[i].txpwr_dig_reg, 0x3F, tx_pwr_calib[i].shift, reg_val);
            reg_val = reg_val - calib_val;
            BTDIGITAL_REG_SET_FIELD(tx_pwr_calib[i].txpwr_dig_reg, 0x3F, tx_pwr_calib[i].shift, reg_val);
        }
    } else if (calib_val < 0){
        calib_val = (2 * calib_val - 1);
        BTRF_REG_GET_FIELD(0xcc, 0x1F, 4, reg_val);
        reg_val = reg_val - calib_val;
        BTRF_REG_SET_FIELD(0xcc, 0x1F, 4, reg_val);
    } else {
        //do nothing
    }
}

void bt_drv_calibration_init(void)
{
#ifndef NOSTD
    //TODO
    btdrv_txpower_calib();
    btdrv_tx_iq_manual_cal();
    btdrv_rxbb_rccal();
    //btdrv_i2v_dccal();

#ifdef RX_IQ_CAL
    //hal_btdump_clk_enable();
    //bt_iq_calibration_setup();
    //hal_btdump_clk_disable();
#endif

    //TO DO: RF calibration
#ifdef __PWR_FLATNESS__
    //btdrv_channel_pwr_flatness();
#endif
#endif
}

#ifdef SIMULTANEOUS_AWAKEN
#define HAL_SYS_WAKE_LOCK_USER1306_WORKAROUND HAL_SYS_WAKE_LOCK_USER_31
static HWTIMER_ID hw_timer_btc_sleep = NULL;
static HWTIMER_ID hw_timer_prevent_sleep = NULL;
static void btc_sleep_timer_periodic_irq_handler(void *param)
{
    hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER1306_WORKAROUND);
    hwtimer_stop(hw_timer_prevent_sleep);
    hwtimer_start(hw_timer_prevent_sleep, US_TO_TICKS(2*hal_cmu_get_26m_ready_timeout_us() + 1000));
}
static void prevent_sleep_clear_irq_handler(void *param)
{
    hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER1306_WORKAROUND);
}
void btc_sleep_rx()
{
    int32_t btc_sleep_dur = bt_drv_reg_op_get_btc_sleep_duration();
    if(btc_sleep_dur > 0)
    {
        int32_t curr_sleep_dur = btc_sleep_dur/2 - US_TO_TICKS(2*hal_cmu_get_26m_ready_timeout_us());
        if(curr_sleep_dur > 0)
        {
            hwtimer_stop(hw_timer_btc_sleep);
            hwtimer_start(hw_timer_btc_sleep, curr_sleep_dur);
            hal_sys_wake_unlock(HAL_SYS_WAKE_LOCK_USER1306_WORKAROUND);
        }
        else
        {
            hwtimer_stop(hw_timer_prevent_sleep);
            hwtimer_start(hw_timer_prevent_sleep, btc_sleep_dur/2 + MS_TO_TICKS(1));
            hal_sys_wake_lock(HAL_SYS_WAKE_LOCK_USER1306_WORKAROUND);
        }
    }
}
#endif
void btdrv_start_bt(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_52M);

    btdrv_common_init();

    //mqlsimu  disabled
   // hal_iomux_ispi_access_enable(HAL_IOMUX_ISPI_MOD_RF, HAL_IOMUX_ISPI_OWNER_BTH);
    //Symbol init should be done before powering on BTC
    bt_drv_reg_op_global_symbols_init();
    //power on BTC
#ifdef __BT_RAMRUN_NEW__
    bt_controller_open(NULL, NULL);
    btdrv_delay(100);
#else
    btdrv_poweron(BT_POWERON);
    //ROM patch init
    btdrv_ins_patch_init();
#endif
#ifdef BTC_CPUDUMP_BASE
    btdrv_cpudump_clk_enable();
    btdrv_cpudump_enable();
#endif
    //RF module
    btdrv_rf_init();
    //Digital module
    btdrv_config_init();

    //calibration module init
    bt_drv_calibration_init();

    // turn off bias current (only for version B)
    btdrv_turn_off_bias_current();

    /*reg controller crash dump*/
    hal_trace_crash_dump_register(HAL_TRACE_CRASH_DUMP_MODULE_BT, btdrv_btc_fault_dump);

    btdrv_config_end();

    btdrv_hciopen();

    btdrv_hciprocess();

    bt_drv_reg_op_init();

#ifdef SIMULTANEOUS_AWAKEN
    if(hal_get_chip_metal_id()<=HAL_CHIP_METAL_ID_1)
    {
        bt_drv_reg_op_set_simultaneous_awaken_flag(1);
        hw_timer_prevent_sleep = hwtimer_alloc(prevent_sleep_clear_irq_handler, 0);
        hw_timer_btc_sleep = hwtimer_alloc(btc_sleep_timer_periodic_irq_handler, 0);
        hal_intersys_wakeup_open(HAL_INTERSYS_ID_1, btc_sleep_rx);
    }
#else
    bt_drv_reg_op_set_simultaneous_awaken_flag(0);
#endif
    btdrv_hcioff();

    hal_sysfreq_req(HAL_SYSFREQ_USER_BT, HAL_CMU_FREQ_32K);
}

const uint8_t hci_cmd_enable_dut[] =
{
    0x01,0x03, 0x18, 0x00
};
const uint8_t hci_cmd_enable_allscan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x03
};
const uint8_t hci_cmd_disable_scan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x00
};
const uint8_t hci_cmd_enable_pagescan[] =
{
    0x01, 0x1a, 0x0c, 0x01, 0x02
};
const uint8_t hci_cmd_autoaccept_connect[] =
{
    0x01,0x05, 0x0c, 0x03, 0x02, 0x00, 0x02
};
const uint8_t hci_cmd_hci_reset[] =
{
    0x01,0x03,0x0c,0x00
};

const uint8_t hci_cmd_inquiry_scan_activity[] =
{
    0x01,0x1e,0x0c,0x04,0x40,0x00,0x12,0x00
};

const uint8_t hci_cmd_page_scan_activity[] =
{
    0x01,0x1c,0x0c,0x04,0x40,0x00,0x12,0x00
};


const uint8_t hci_cmd_nonsig_tx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x04, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x04, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x04, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_tx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x04, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

const uint8_t hci_cmd_nonsig_rx_dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x00, 0x04, 0x00, 0x1b, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x04, 0x00, 0x36, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh1_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x08, 0x00, 0x53, 0x00,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_2dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0a, 0x00, 0x6f, 0x01,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};
const uint8_t hci_cmd_nonsig_rx_3dh3_pn9_t0[] =
{
    0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x06, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x01, 0x01, 0x0b, 0x00, 0x28, 0x02,
    0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
};

//vco test
const uint8_t hci_cmd_start_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00,0x02
};
const uint8_t hci_cmd_stop_bt_vco_test[] =
{
    0x01, 0xaa, 0xfc, 0x02, 0x00,0x04
};

const uint8_t hci_cmd_enable_ibrt_test[] =
{
    0x01, 0xb4, 0xfc, 0x01, 0x01
};

const uint8_t hci_cmd_set_ibrt_mode[] =
{
    0x01, 0xa2, 0xfc, 0x02, 0x01,0x00
};

void btdrv_testmode_start(void)
{
#ifdef __HW_AGC__
    bt_drv_rf_hw_agc_mode_set(RX_GAIN_IDX_0, HW_AGC_HIGH_PWEF_MODE);
#else
#ifdef __BLE_NEW_SWAGC_MODE__
    //BLE sync agc can only be used in BLE test mode
    bt_drv_reg_op_ble_sync_agc_mode_set(true);
#endif
#endif
}

void btdrv_testmode_end(void)
{
#ifdef __BLE_NEW_SWAGC_MODE__
    bt_drv_reg_op_ble_sync_agc_mode_set(false);
#endif
}

void btdrv_write_localinfo(const char *name, uint8_t len, const uint8_t *addr)
{
    uint8_t hci_cmd_write_addr[5+6] =
    {
        0x01, 0x72, 0xfc, 0x07, 0x00
    };

    uint8_t hci_cmd_write_name[248+4] =
    {
        0x01, 0x13, 0x0c, 0xF8
    };
    memset(&hci_cmd_write_name[4], 0, sizeof(hci_cmd_write_name)-4);
    memcpy(&hci_cmd_write_name[4], name, len);
    btdrv_SendData(hci_cmd_write_name, sizeof(hci_cmd_write_name));
    btdrv_delay(50);
    memcpy(&hci_cmd_write_addr[5], addr, 6);
    btdrv_SendData(hci_cmd_write_addr, sizeof(hci_cmd_write_addr));
    btdrv_delay(20);
}

void btdrv_enable_dut(void)
{
    btdrv_SendData(hci_cmd_enable_dut, sizeof(hci_cmd_enable_dut));
    btdrv_delay(100);
    btdrv_SendData(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(100);
    btdrv_SendData(hci_cmd_autoaccept_connect, sizeof(hci_cmd_autoaccept_connect));
    btdrv_delay(100);
    btdrv_dut_mode_enable = true;
}

void btdrv_enable_dut_again(void)
{
    btdrv_SendData(hci_cmd_enable_allscan, sizeof(hci_cmd_enable_allscan));
    btdrv_delay(20);
    btdrv_dut_mode_enable = true;
}

void btdrv_enable_ibrt_test(void)
{
    btdrv_SendData(hci_cmd_enable_ibrt_test, sizeof(hci_cmd_enable_ibrt_test));
    btdrv_delay(20);
    btdrv_SendData(hci_cmd_set_ibrt_mode, sizeof(hci_cmd_set_ibrt_mode));
    btdrv_delay(20);
}

void btdrv_connect_ibrt_device(uint8_t *addr)
{
    uint8_t hci_cmd_connect_device[17] =
    {
        0x01, 0x05, 0x04, 0x0D, 0xab, 0x89, 0x67, 0x45, 0x23, 0x01, 0x18, 0xCC,
        0x01, 0x00, 0x00, 0x00, 0x00
    };
    memcpy(&hci_cmd_connect_device[4],addr,6);
    btdrv_SendData(hci_cmd_connect_device, sizeof(hci_cmd_connect_device));
    btdrv_delay(50);

}

void btdrv_disable_scan(void)
{
    btdrv_SendData(hci_cmd_disable_scan, sizeof(hci_cmd_disable_scan));
    btdrv_delay(20);
}

static uint32_t dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;

uint32_t btdrv_dut_get_connect_status(void)
{
    return dut_connect_status;
}

void btdrv_enable_dut_again(void);
void btdrv_dut_accessible_mode_manager(const unsigned char *data)
{
    if(btdrv_dut_mode_enable)
    {
        if(data[0]==0x04&&data[1]==0x03&&data[2]==0x0b&&data[3]==0x00)
        {
#ifdef __IBRT_IBRT_TESTMODE__
            if(memcmp(&data[6],bt_get_local_address(),6))
            {
                btdrv_disable_scan();
            }
#else
            btdrv_disable_scan();
#endif
            dut_connect_status = DUT_CONNECT_STATUS_CONNECTED;
        }
        else if(data[0]==0x04&&data[1]==0x05&&data[2]==0x04&&data[3]==0x00)
        {
            btdrv_enable_dut_again();
            dut_connect_status = DUT_CONNECT_STATUS_DISCONNECTED;
        }
    }
}

void btdrv_hci_reset(void)
{
    btdrv_SendData(hci_cmd_hci_reset, sizeof(hci_cmd_hci_reset));
    btdrv_delay(350);
}

void btdrv_enable_nonsig_tx(uint8_t index)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_tx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_tx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_tx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_tx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_tx_dh1_pn9_t0));

    btdrv_delay(20);

}

void btdrv_enable_nonsig_rx(uint8_t index)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    if (index == 0)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 1)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else if (index == 2)
        btdrv_SendData(hci_cmd_nonsig_rx_2dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_2dh1_pn9_t0));
    else if (index == 3)
        btdrv_SendData(hci_cmd_nonsig_rx_3dh3_pn9_t0, sizeof(hci_cmd_nonsig_rx_3dh1_pn9_t0));
    else
        btdrv_SendData(hci_cmd_nonsig_rx_dh1_pn9_t0, sizeof(hci_cmd_nonsig_rx_dh1_pn9_t0));

    btdrv_delay(20);
}

static bool btdrv_vco_test_running = false;
static uint16_t vco_test_reg_val_C1 = 0;
static uint16_t vco_test_reg_val_24 = 0;
#ifdef VCO_TEST_TOOL
static unsigned short vco_test_hack_flag = 0;
static unsigned short vco_test_channel = 0xff;
static unsigned short set_xatl_facp_hack_flag = 0;
static unsigned short vco_test_xtal_fcap = 0xffff;

unsigned short btdrv_get_vco_test_process_flag(void)
{
    return vco_test_hack_flag;
}

bool btdrv_vco_test_bridge_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if(data[0]==0x01 &&data[1]==0xaa&&data[2]==0xfc &&data[3]==0x02)
    {
        status = true;
        vco_test_hack_flag = data[5];
        vco_test_channel = data[4];
    }

    return status;
}

void btdrv_vco_test_process(uint8_t op)
{
    if(op == 0x02)//vco test start
    {
        if(vco_test_channel != 0xff)
            btdrv_vco_test_start(vco_test_channel);
    }
    else if(op ==0x04)//vco test stop
    {
        btdrv_vco_test_stop();
    }
    vco_test_channel =0xff;
    vco_test_hack_flag = 0;
}

unsigned short btdrv_get_vco_test_set_xtal_fcap_process_flag(void)
{
    return set_xatl_facp_hack_flag;
}

bool btdrv_vco_test_set_xtal_fcap_bridge_intsys_callback(const unsigned char *data)
{
    bool status = false;
    if(data[0]==0x01 && data[1]==0xab && data[2]==0xfc && data[3]==0x03)
    {
        status = true;
        vco_test_xtal_fcap = (data[5] << 8) | data[4];
        set_xatl_facp_hack_flag = data[6];
    }

    return status;
}

void btdrv_vco_test_set_xtal_fcap_process(uint8_t op)
{
    if(op == 0x02)// set_xtal_fcap
    {
        btdrv_rf_init_xtal_fcap(vco_test_xtal_fcap);
    }
    else if (op == 0x04)// save_xtal_fcap
    {
        nvrec_dev_set_xtal_fcap(vco_test_xtal_fcap);
    }
    vco_test_xtal_fcap = 0xffff;
    set_xatl_facp_hack_flag = 0;
}
#endif

void btdrv_vco_test_start(uint8_t chnl)
{
    if (!btdrv_vco_test_running)
    {
        btdrv_vco_test_running = true;
        btdrv_read_rf_reg(0xC1, &vco_test_reg_val_C1);
        btdrv_read_rf_reg(0x24, &vco_test_reg_val_24);

        btdrv_reg_op_vco_test_start(chnl);

        BTRF_REG_SET_FIELD(0xC1, 0x1, 11, 0);   //Transmit signal without modulation
        BTRF_REG_SET_FIELD(0x24, 0xF, 4, 9);     //tx gain dr idx5
    }
}

void btdrv_vco_test_stop(void)
{
    if (btdrv_vco_test_running)
    {
        btdrv_vco_test_running = false;
        btdrv_reg_op_vco_test_stop();

        btdrv_write_rf_reg(0xC1, vco_test_reg_val_C1);

        btdrv_write_rf_reg(0x24, vco_test_reg_val_24);
    }
}

void btdrv_stop_bt(void)
{
    btdrv_poweron(BT_POWEROFF);
}

void btdrv_write_memory(uint8_t wr_type,uint32_t address,const uint8_t *value,uint8_t length)
{
    uint8_t buff[256];
    if(length ==0 || length >128)
        return;
    buff[0] = 0x01;
    buff[1] = 0x02;
    buff[2] = 0xfc;
    buff[3] = length + 6;
    buff[4] = address & 0xff;
    buff[5] = (address &0xff00)>>8;
    buff[6] = (address &0xff0000)>>16;
    buff[7] = address>>24;
    buff[8] = wr_type;
    buff[9] = length;
    memcpy(&buff[10],value,length);
    btdrv_SendData(buff,length+10);
    btdrv_delay(2);


}

void btdrv_send_cmd(uint16_t opcode,uint8_t cmdlen,const uint8_t *param)
{
    uint8_t buff[256];
    buff[0] = 0x01;
    buff[1] = opcode & 0xff;
    buff[2] = (opcode &0xff00)>>8;
    buff[3] = cmdlen;
    if(cmdlen>0)
        memcpy(&buff[4],param,cmdlen);
    btdrv_SendData(buff,cmdlen+4);
}

//[26:0] 0x07ffffff
//[27:0] 0x0fffffff

uint32_t btdrv_syn_get_curr_ticks(void)
{
    uint32_t value;

    value = btdrv_reg_op_syn_get_curr_ticks();
    return value;
}

uint32_t btdrv_syn_get_cis_curr_time(void)
{
    uint32_t value;

    value = btdrv_reg_op_syn_get_cis_curr_time();
    return value;
}

#ifdef __SW_TRIG__

uint16_t Tbit_M_ori_flag = 200;
#define RF_DELAY  18

#define ROUNDDOWN(x)   ((int)(x))
#define ROUNDUP(x)     ((int)(x) + ((x-(int)(x)) > 0 ? 1 : 0))
#define ROUND(x)       ((int)((x) + ((x) > 0 ? 0.5 : -0.5)))

uint16_t btdrv_Tbit_M_h_get(uint32_t Tclk_M, int16_t Tbit_M_h_ori)
{
    uint16_t Tbit_M_h;
    Tbit_M_h = Tbit_M_h_ori;
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tbit_M_h);
    return Tbit_M_h;
}


uint16_t btdrv_Tbit_M_get(uint32_t Tclk_M, uint16_t Tbit_M_h)
{
    uint16_t Tbit_M;
    if(Tbit_M_h % 2)///if Tbit_M_h is odd
    {
        Tbit_M = (uint16_t)ROUNDUP(Tbit_M_h/2) + 1;
    }
    else
    {
        Tbit_M = (uint16_t)ROUNDUP(Tbit_M_h/2);
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tbit_M);
    return Tbit_M;
}


extern "C" int32_t bt_syn_get_clkoffset(uint16_t conhdl)
{
    int32_t offset;

    if(conhdl>=0x80)
        offset = bt_drv_reg_op_get_clkoffset(conhdl-0x80);
    else
        offset = 0;

    return offset;
}

int32_t btdrv_slotoff_get(uint16_t conhdl,int32_t clkoff)
{
    int32_t slotoff;

    if(clkoff < 0)
    {
        slotoff = -(int32_t)ROUNDDOWN((-clkoff)/2);
    }
    else
    {
        if(clkoff % 2)
        {
            slotoff = (int32_t)ROUNDUP(clkoff/2) + 1;
        }
        else
        {
            slotoff = (int32_t)ROUNDUP(clkoff/2);
        }
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,slotoff);
    return slotoff;
}

int16_t btdrv_bitoff_get(uint16_t rxbit_1us,int32_t clkoff)
{
    int16_t bitoff;

    bitoff = rxbit_1us - 68 - RF_DELAY;
    BT_DRV_TRACE(1,"[%s] bitoff=%d\n",__func__,bitoff);
    if((bitoff < 0) && (clkoff % 2))
    {
        bitoff += 624;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,bitoff);
    return bitoff;
}


uint8_t btdrv_clk_adj_M_get(uint16_t Tbit_M,int16_t bitoff)
{
    uint8_t clk_adj_M;

    if(bitoff >= Tbit_M)
    {
        clk_adj_M = 1;
    }
    else
    {
        clk_adj_M = 0;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,clk_adj_M);
    return clk_adj_M;
}

uint8_t btdrv_clk_adj_S_get(uint8_t clk_adj_M,uint16_t Tbit_M,int16_t bitoff)
{
    uint8_t clk_adj_S;

    if(clk_adj_M > 0)
    {
        int16_t temp = (bitoff - Tbit_M)*2;
        if(temp > 624)
        {
            clk_adj_S = 1;
        }
        else
        {
            clk_adj_S = 0;
        }
    }
    else
    {
        clk_adj_S = 0;
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,clk_adj_S);
    return clk_adj_S;
}

uint32_t btdrv_Tclk_S_get(uint32_t Tclk_M, uint8_t clk_adj_M,
                                uint8_t clk_adj_S, int32_t slotoff)
{
    uint32_t Tclk_S;
    Tclk_S = Tclk_M - slotoff*2 + clk_adj_M + clk_adj_S;
    Tclk_S &= 0x0fffffff;
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tclk_S);
    return Tclk_S;
}


uint16_t btdrv_Tclk_S_h_get(uint8_t clk_adj_M,uint8_t clk_adj_S,
                                    uint16_t Tbit_M,int16_t bitoff)
{
    uint16_t Tclk_S_h;
    if(clk_adj_M == 0)
    {
        Tclk_S_h = (Tbit_M - bitoff)*2;
    }
    else
    {
        if(clk_adj_S == 1)
        {
            Tclk_S_h = (1248-(bitoff-Tbit_M)*2);
        }
        else
        {
            Tclk_S_h = (624-(bitoff-Tbit_M)*2);
        }
    }
    BT_DRV_TRACE(1,"[%s]:%d\n",__func__,Tclk_S_h);
    return Tclk_S_h;
}

void btdrv_sw_trig_tg_finecnt_set(uint16_t tg_bitcnt, uint8_t trig_route)
{
    btdrv_reg_op_sw_trig_tg_finecnt_set(tg_bitcnt,trig_route);
}

uint16_t btdrv_sw_trig_tg_finecnt_get(uint8_t trig_route)
{
    uint16_t finecnt = 0;
    finecnt = btdrv_reg_op_sw_trig_tg_finecnt_get(trig_route);
    return finecnt;
}

void btdrv_sw_trig_tg_clkncnt_set(uint32_t num, uint8_t trig_route)
{
    btdrv_reg_op_sw_trig_tg_clkncnt_set(num,trig_route);
}

void btdrv_sw_trig_slave_calculate_and_set(uint16_t conhdl, uint32_t Tclk_M, uint16_t Tbit_M_h_ori, uint8_t trig_route)
{
    uint16_t Tbit_M_h;
    uint16_t Tbit_M;
    uint16_t rxbit_1us;
    int16_t bitoff;
    int32_t clkoff;
    int32_t slotoff;
    uint8_t clk_adj_M;
    uint8_t clk_adj_S;
    uint32_t Tclk_S;
    uint32_t Tclk_S_h;

    Tbit_M_h = Tbit_M_ori_flag;
    Tbit_M = btdrv_Tbit_M_get(Tclk_M,Tbit_M_h);
    rxbit_1us = bt_drv_reg_op_rxbit_1us_get(conhdl);
    clkoff = bt_syn_get_clkoffset(conhdl);
    slotoff = btdrv_slotoff_get(conhdl,clkoff);
    bitoff = btdrv_bitoff_get(rxbit_1us,clkoff);
    clk_adj_M = btdrv_clk_adj_M_get(Tbit_M,bitoff);
    clk_adj_S = btdrv_clk_adj_S_get(clk_adj_M,Tbit_M,bitoff);
    Tclk_S = btdrv_Tclk_S_get(Tclk_M,clk_adj_M,clk_adj_S,slotoff);
    Tclk_S_h = btdrv_Tclk_S_h_get(clk_adj_M,clk_adj_S,Tbit_M,bitoff);

    btdrv_sw_trig_tg_clkncnt_set(Tclk_S,trig_route);
    btdrv_sw_trig_tg_finecnt_set(Tclk_S_h,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);
}

void btdrv_sw_trig_master_set(uint32_t Tclk_M, uint16_t Tbit_M_h_ori, uint8_t trig_route)
{
    uint16_t Tbit_M_h;

    btdrv_sw_trig_tg_clkncnt_set(Tclk_M,trig_route);
    Tbit_M_h = btdrv_Tbit_M_h_get(Tclk_M,Tbit_M_h_ori);
    btdrv_sw_trig_tg_finecnt_set(Tbit_M_h,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);
}

#endif


void bt_syn_cancel_tg_ticks(uint8_t trig_route)
{
#ifdef __SW_TRIG__
    btdrv_sw_trig_disable_set(trig_route);
#endif
}

static void btdrv_syn_set_tg_ticks(uint32_t num, uint8_t mode, uint16_t conhdl, uint8_t trig_route)
{
    if (mode == BT_TRIG_MASTER_ROLE)
    {
#ifdef __SW_TRIG__
        BT_DRV_TRACE(1,"[%s] __SW_TRIG__ conhdl=0x%x num=%d trig_route=%d\n",__func__,conhdl,num,trig_route);
        btdrv_sw_trig_master_set(num, Tbit_M_ori_flag, trig_route);
#else
        btdrv_reg_op_syn_set_tg_ticks_master_role();
#endif
    }
    else
    {
#ifdef __SW_TRIG__
        //uint16_t conhdl = btdrv_sync_sw_trig_get_conhdl();
        BT_DRV_TRACE(1,"[%s] __SW_TRIG__ conhdl=0x%x num=%d trig_route=%d\n",__func__,conhdl,num,trig_route);
        btdrv_sw_trig_slave_calculate_and_set(conhdl, num, Tbit_M_ori_flag,trig_route);
#else
        btdrv_reg_op_syn_set_tg_ticks_slave_role(num);
#endif
    }
}

// Can be used by master or slave
// Ref: Master bt clk
uint32_t bt_syn_get_curr_ticks(uint16_t conhdl)
{
    int32_t curr,offset;

    curr = btdrv_syn_get_curr_ticks();

    if (btdrv_is_link_index_valid(btdrv_conhdl_to_linkid(conhdl)))
        offset = bt_drv_reg_op_get_clkoffset(btdrv_conhdl_to_linkid(conhdl));
    else
        offset = 0;
//    BT_DRV_TRACE(4,"[%s] curr(%d) + offset(%d) = %d", __func__, curr , offset,curr + offset);
    return (curr + offset) & 0x0fffffff;
}

void bt_syn_trig_checker(uint16_t conhdl)
{
}

// Can be used by master or slave
// Ref: Master bt clk
void bt_syn_set_tg_ticks(uint32_t val,uint16_t conhdl, uint8_t mode, uint8_t trig_route, bool no_link_trig)
{
    int32_t offset;
    uint8_t link_id = btdrv_conhdl_to_linkid(conhdl);
    if(no_link_trig == false)
    {
        if(HCI_LINK_INDEX_INVALID == link_id)
        {
            BT_DRV_TRACE(3,"%s,ERR INVALID CONHDL 0x%x!! ca=%p",__func__,conhdl, __builtin_return_address(0));
            return;
        }
    }
    if (btdrv_is_link_index_valid(link_id))
    {
        offset = bt_drv_reg_op_get_clkoffset(link_id);
    }
    else
    {
        offset = 0;
    }

#if !defined(__SW_TRIG__)
    btdrv_reg_op_syn_set_tg_ticks_linkid(link_id);
#endif

    if ((mode == BT_TRIG_MASTER_ROLE) && (offset !=0))
    {
        BT_DRV_TRACE(0,"ERROR OFFSET !!");
    }

#if !defined(__SW_TRIG__)
    val = val>>1;
    val = val<<1;
    val += 1;
#endif

    bt_syn_cancel_tg_ticks(trig_route);
    BT_DRV_TRACE(4,"bt_syn_set_tg_ticks val:%d num:%d mode:%d conhdl:%02x", val, val - offset, mode, conhdl);
    btdrv_syn_set_tg_ticks(val, mode, conhdl, trig_route);
    bt_syn_trig_checker(conhdl);
}

void bt_syn_ble_set_tg_ticks(uint32_t val,uint8_t trig_route)
{
#ifdef __SW_TRIG__
    uint32_t clkncnt = 0;
    uint16_t finecnt = 0;

    btdrv_syn_clr_trigger(trig_route);
    btdrv_reg_op_bts_to_bt_time(val, &clkncnt, &finecnt);

    btdrv_sw_trig_tg_clkncnt_set(clkncnt,trig_route);
    btdrv_sw_trig_tg_finecnt_set(finecnt,trig_route);
    btdrv_sw_trig_en_set(trig_route);
    btdrv_reg_op_sw_trig_route_extra_set(trig_route);
#endif
}

uint32_t bt_syn_ble_bt_time_to_bts(uint32_t hs, uint16_t hus)
{
    return btdrv_reg_op_bt_time_to_bts(hs, HALF_SLOT_INV(hus));
}

void btdrv_enable_playback_triggler(uint8_t triggle_mode)
{
    btdrv_reg_op_enable_playback_triggler(triggle_mode);
}


void btdrv_play_trig_mode_set(uint8_t mode)
{
    btdrv_reg_op_play_trig_mode_set(mode);
}

void btdrv_disable_playback_triggler(void)
{
    btdrv_reg_op_disable_playback_triggler();
}
//pealse use btdrv_is_link_index_valid() check link index whether valid
uint8_t btdrv_conhdl_to_linkid(uint16_t connect_hdl)
{
    //invalid hci handle,such as link disconnected
    if(connect_hdl < HCI_HANDLE_MIN || connect_hdl > HCI_HANDLE_MAX)
    {
        TRACE(2, "ERROR Connect Handle=0x%x ca=%p", connect_hdl, __builtin_return_address(0));
        return HCI_LINK_INDEX_INVALID;
    }
    else
    {
        return (connect_hdl - HCI_HANDLE_MIN);
    }
}

void btdrv_pcm_enable(void)
{
    btdrv_reg_op_pcm_enable();
}

void btdrv_pcm_disable(void)
{
    btdrv_reg_op_pcm_disable();
}

// Trace tport
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_tport[] =
{
    {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE},
};

int btdrv_host_gpio_tport_open(void)
{
    uint32_t i;

    for (i=0; i<sizeof(pinmux_tport)/sizeof(struct HAL_IOMUX_PIN_FUNCTION_MAP); i++)
    {
        hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&pinmux_tport[i], 1);
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)pinmux_tport[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
    return 0;
}

int btdrv_gpio_port_set(int port)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

int btdrv_gpio_tport_clr(int port)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)pinmux_tport[port].pin);
    return 0;
}

#if defined(TX_RX_PCM_MASK)
uint8_t  btdrv_is_pcm_mask_enable(void)
{
    return 1;
}
#endif

#ifdef PCM_FAST_MODE
void btdrv_open_pcm_fast_mode_enable(void)
{
    btdrv_reg_op_open_pcm_fast_mode_enable();
}
void btdrv_open_pcm_fast_mode_disable(void)
{
    btdrv_reg_op_open_pcm_fast_mode_disable();
}
#endif

#if defined(CVSD_BYPASS)
void btdrv_cvsd_bypass_enable(void)
{
    btdrv_reg_op_cvsd_bypass_enable();
}
#endif

void btdrv_enable_rf_sw(int rx_on, int tx_on)
{
    hal_iomux_set_bt_rf_sw(rx_on, tx_on);
}
