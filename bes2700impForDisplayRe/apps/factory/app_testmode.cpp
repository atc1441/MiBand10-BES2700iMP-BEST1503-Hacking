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
#ifdef NORMAL_TEST_MODE_SWITCH
#include <stdlib.h>
#include "string.h"
#include "bt_drv.h"
#include "app_testmode.h"
#include "bt_drv_reg_op.h"
#include "bt_drv_interface.h"
#include "hal_intersys.h"
#include "tgt_hardware.h"
#include "nvrecord_dev.h"
#include "besbt_string.h"
#include "intersyshci.h"
#include "hal_bootmode.h"
#include "apps.h"
#include "me_api.h"

#define ID_NUL_TYPE     0x0
#define POLL_TYPE       0x1
#define FHS_TYPE        0x2
#define DM1_TYPE        0x3
#define DH1_TYPE        0x4
#define DH1_2_TYPE      0x4
#define DH1_3_TYPE      0x8
#define HV1_TYPE        0x5
#define HV2_TYPE        0x6
#define EV3_2_TYPE      0x6
#define HV3_TYPE        0x7
#define EV3_TYPE        0x7
#define EV3_3_TYPE      0x7
#define DV_TYPE         0x8
#define AUX1_TYPE       0x9
#define DM3_TYPE        0xA
#define DH3_TYPE        0xB
#define DH3_2_TYPE      0xA
#define DH3_3_TYPE      0xB
#define EV4_TYPE        0xC
#define EV5_2_TYPE      0xC
#define EV5_TYPE        0xD
#define EV5_3_TYPE      0xD
#define DM5_TYPE        0xE
#define DH5_TYPE        0xF
#define DH5_2_TYPE      0xE
#define DH5_3_TYPE      0xF

#define FHS_PACKET_SIZE         18
#define DM1_PACKET_SIZE         17
#define DH1_PACKET_SIZE         27
#define DV_ACL_PACKET_SIZE      9
#define DM3_PACKET_SIZE         121
#define DH3_PACKET_SIZE         183
#define DM5_PACKET_SIZE         224
#define DH5_PACKET_SIZE         339
#define AUX1_PACKET_SIZE        29
#define HV1_PACKET_SIZE         10
#define HV2_PACKET_SIZE         20
#define HV3_PACKET_SIZE         30
#define EV3_PACKET_SIZE         30
#define EV4_PACKET_SIZE         120
#define EV5_PACKET_SIZE         180
#define DH1_2_PACKET_SIZE        54
#define DH1_3_PACKET_SIZE        83
#define DH3_2_PACKET_SIZE        367
#define DH3_3_PACKET_SIZE        552
#define DH5_2_PACKET_SIZE        679
#define DH5_3_PACKET_SIZE        1021
#define EV3_2_PACKET_SIZE       60
#define EV3_3_PACKET_SIZE       90
#define EV5_2_PACKET_SIZE       360
#define EV5_3_PACKET_SIZE       540

#define EDR_DISABLED    0 /* Erroneous Data Reporting disabled */
#define EDR_ENABLED     1 /* Erroneous Data Reporting enabled */

#define BT_BD_ADDR_SIZE 6

#define BTDRV_STORELE16(buff,num) (((buff)[1] = (uint8_t) ((num)>>8)), ((buff)[0] = (uint8_t) (num)))

static APP_DBG_NONSIG_TESTER_PKT_TYPE_T app_dbg_nonsig_pkt_type_table[APP_DBG_NONSIG_TYPE_END] =
{
    {APP_DBG_NONSIG_TYPE_ID_NUL,  ID_NUL_TYPE,    EDR_DISABLED,   0},
    {APP_DBG_NONSIG_TYPE_POLL,    POLL_TYPE,      EDR_DISABLED,   0},
    {APP_DBG_NONSIG_TYPE_FHS,     FHS_TYPE,       EDR_DISABLED,   FHS_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DM1,     DM1_TYPE,       EDR_DISABLED,   DM1_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH1,     DH1_TYPE,       EDR_DISABLED,   DH1_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH1_2,   DH1_2_TYPE,     EDR_ENABLED,    DH1_2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH1_3,   DH1_3_TYPE,     EDR_ENABLED,    DH1_3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_HV1,     HV1_TYPE,       EDR_DISABLED,   HV1_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_HV2,     HV2_TYPE,       EDR_DISABLED,   HV2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV3_2,   EV3_2_TYPE,     EDR_ENABLED,    EV3_2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_HV3,     HV3_TYPE,       EDR_DISABLED,   HV3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV3,     EV3_TYPE,       EDR_DISABLED,   EV3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV3_3,   EV3_3_TYPE,     EDR_ENABLED,    EV3_3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DV,      DV_TYPE,        EDR_DISABLED,   DV_ACL_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_AUX1,    AUX1_TYPE,      EDR_DISABLED,   AUX1_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DM3,     DM3_TYPE,       EDR_DISABLED,   DM3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH3,     DH3_TYPE,       EDR_DISABLED,   DH3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH3_2,   DH3_2_TYPE,     EDR_ENABLED,    DH3_2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH3_3,   DH3_3_TYPE,     EDR_ENABLED,    DH3_3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV4,     EV4_TYPE,       EDR_DISABLED,   EV4_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV5_2,   EV5_2_TYPE,     EDR_ENABLED,    EV5_2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV5,     EV5_TYPE,       EDR_DISABLED,   EV5_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_EV5_3,   EV5_3_TYPE,     EDR_ENABLED,    EV5_3_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DM5,     DM5_TYPE,       EDR_DISABLED,   DM5_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH5,     DH5_TYPE,       EDR_DISABLED,   DH5_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH5_2,   DH5_2_TYPE,     EDR_ENABLED,    DH5_2_PACKET_SIZE},
    {APP_DBG_NONSIG_TYPE_DH5_3,   DH5_3_TYPE,     EDR_ENABLED,    DH5_3_PACKET_SIZE},
};

typedef struct
{
    uint16_t    pkt_counters;
    uint16_t    head_errors;
    uint16_t    payload_errors;
    int16_t    avg_estsw;
    int16_t    avg_esttpl;
    uint32_t    payload_bit_errors;
}DBG_NONSIG_TEST_EVENT_IND_T;

typedef struct
{
    uint16_t pkt_counters;
}DBG_NONSIG_TEST_EVENT_IND_BLE_T;

bool btdrv_testmode_dut_mode_enable = false;
static dev_addr_name app_testmode_devinfo;
APP_DBG_NONSIG_TESTER_RESULT_BT_T app_testmode_test_result_bt;
APP_DBG_NONSIG_TESTER_RESULT_BLE_T app_testmode_test_result_ble;
extern "C" void pmu_sleep_en(unsigned char sleep_en);
extern "C" void pmu_reboot(void);
extern "C" int app_wdt_close(void);
extern "C" int app_wdt_open(int seconds);


void app_testmode_save_dev_addr_name(void)
{
    app_testmode_devinfo.btd_addr = bt_global_addr;
    app_testmode_devinfo.ble_addr = ble_global_addr;
    app_testmode_devinfo.localname = BT_LOCAL_NAME;
    app_testmode_devinfo.ble_name= BT_LOCAL_NAME;
    nvrec_dev_localname_addr_init(&app_testmode_devinfo);
    btdrv_write_localinfo((char *)app_testmode_devinfo.localname, strlen(app_testmode_devinfo.localname) + 1, app_testmode_devinfo.btd_addr);
}

static void testmode_error_check_timer_handler(void const *param);
osTimerDef(testmode_error_check_timer, testmode_error_check_timer_handler);
static osTimerId testmode_error_check_timer_id = NULL;

static void testmode_error_check_timer_handler(void const *param)
{
    //dump rssi
    bt_drv_rssi_dump_handler();
    //adjust parameter for testmode
    bt_drv_testmode_adaptive_adjust_param();

    //check BT core status
    if(bt_drv_error_check_handler())
    {
        bt_drv_reg_op_crash_dump();
        pmu_reboot();
    }
}

int app_testmode_enter(void)
{
    uint8_t enable = btif_me_get_testmode_enable();
    if (!enable)
    {
        app_wdt_close();
        app_stop_10_second_timer(APP_PAIR_TIMER_ID);
        app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
        if (!testmode_error_check_timer_id){
            testmode_error_check_timer_id = osTimerCreate(osTimer(testmode_error_check_timer), osTimerPeriodic, NULL);
        }
        if (testmode_error_check_timer_id != NULL) {
            osTimerStart(testmode_error_check_timer_id, 1000);
        }
        btif_me_set_testmode_enable(1);

        pmu_sleep_en(0);
        BESHCI_Close();
        btdrv_hciopen();
        btdrv_testmode_register_nonsig_rx_process(app_testmode_nonsig_test_result_save);
        btdrv_ins_patch_test_init();
        bt_drv_reg_op_key_gen_after_reset(false);
        btdrv_hci_reset();

        btdrv_sleep_config(0);
        btdrv_testmode_start();
        btdrv_feature_default();
        app_testmode_save_dev_addr_name();
        bt_drv_config_after_hci_reset();
    }
     return 0;
}

#define AUTO_ENTER_NORMAL_MODE
void app_enter_normal_mode(void)
{
    uint8_t enable = btif_me_get_testmode_enable();
    if (!enable)
    {
        btif_me_chip_init_noraml_test_mode_switch();
        bt_drv_config_after_hci_reset();
    }
    else
    {
        BT_DRV_TRACE(1,"WARNING!!!, %s must called after app_testmode_exit\n", __func__);
    }
}

extern bool btdrv_dut_mode_enable;
int app_testmode_exit(void)
{
    uint8_t enable = btif_me_get_testmode_enable();

    if (enable)
    {
        app_wdt_open(30);
        app_start_10_second_timer(APP_PAIR_TIMER_ID);
        app_start_10_second_timer(APP_POWEROFF_TIMER_ID);
        if (testmode_error_check_timer_id) {
         osTimerStop(testmode_error_check_timer_id);
        }

        btdrv_hci_reset();
        app_testmode_save_dev_addr_name();

        btdrv_delay(1000);
        btdrv_hcioff();
        btdrv_testmode_end();
        btdrv_ins_patch_close();
        btdrv_testmode_register_nonsig_rx_process(NULL);
        btdrv_stop_bt();
        btdrv_start_bt();
        bt_drv_config_after_hci_reset();
        BESHCI_Open();

        uint32_t flags = int_lock();
        btif_me_set_testmode_enable(0);
#ifdef AUTO_ENTER_NORMAL_MODE
        app_enter_normal_mode();
#endif
        int_unlock(flags);
    }
    return 0;
}

int app_testmode_bt_nonsig_tx(APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T *tx_cfg)
{
    int sRet = 0;

     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     APP_DBG_NONSIG_TESTER_PKT_TYPE_T *nonsig_pkt_type_p = NULL;

     uint8_t hci_cmd_nonsig_tx_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x00, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     hci_cmd_nonsig_tx_buf[9] = tx_cfg->tx_freq;
     hci_cmd_nonsig_tx_buf[11] = tx_cfg->power_level;

     sRet = memcpy_s(&hci_cmd_nonsig_tx_buf[12], BT_BD_ADDR_SIZE, tx_cfg->addr, BT_BD_ADDR_SIZE);
     if(sRet != 0)
     {
         TRACE(1,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
     }
     for(uint8_t i = 0; i < APP_DBG_NONSIG_TYPE_END; i++)
     {
         if(app_dbg_nonsig_pkt_type_table[i].enum_pkt_type == tx_cfg->btdrv_pkt_type)
         {
             nonsig_pkt_type_p = &app_dbg_nonsig_pkt_type_table[i];
             break;
         }
     }

     if (!nonsig_pkt_type_p){
         return -1;
     }

     hci_cmd_nonsig_tx_buf[19] = nonsig_pkt_type_p->edr_enabled;
     hci_cmd_nonsig_tx_buf[20] = nonsig_pkt_type_p->packet_type;
     BTDRV_STORELE16(&hci_cmd_nonsig_tx_buf[22], ((nonsig_pkt_type_p->payload_length)));
     hci_cmd_nonsig_tx_buf[21] = tx_cfg->payload_pattern;

     btdrv_SendData(hci_cmd_nonsig_tx_buf, sizeof(hci_cmd_nonsig_tx_buf));

     return 0;
}

int app_testmode_bt_nonsig_rx(APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T *rx_cfg)
{
    int sRet = 0;

     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     APP_DBG_NONSIG_TESTER_PKT_TYPE_T *nonsig_pkt_type_p = NULL;

     uint8_t hci_cmd_nonsig_rx_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x01, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x00, 0x04, 0x04, 0x1b, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     hci_cmd_nonsig_rx_buf[10] = rx_cfg->rx_freq;

     sRet = memcpy_s(&hci_cmd_nonsig_rx_buf[12], BT_BD_ADDR_SIZE, rx_cfg->addr, BT_BD_ADDR_SIZE);
     if(sRet != 0)
     {
         TRACE(1,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
     }
     for(uint8_t i = 0; i < APP_DBG_NONSIG_TYPE_END; i++)
     {
         if(app_dbg_nonsig_pkt_type_table[i].enum_pkt_type == rx_cfg->btdrv_pkt_type)
         {
             nonsig_pkt_type_p = &app_dbg_nonsig_pkt_type_table[i];
             break;
         }
     }

     if (!nonsig_pkt_type_p){
         return -1;
     }

     hci_cmd_nonsig_rx_buf[19] = nonsig_pkt_type_p->edr_enabled;
     hci_cmd_nonsig_rx_buf[20] = nonsig_pkt_type_p->packet_type;
     BTDRV_STORELE16(&hci_cmd_nonsig_rx_buf[22], ((nonsig_pkt_type_p->payload_length)));

     btdrv_SendData(hci_cmd_nonsig_rx_buf, sizeof(hci_cmd_nonsig_rx_buf));

     return 0;
}

int app_testmode_bt_nonsig_endtest()
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     uint8_t hci_cmd_nonsig_end_test_buf[] =
     {
         0x01, 0x87, 0xfc, 0x1c, 0x02, 0xe8, 0x03, 0x00, 0x00, 0x00, 0x00, 0x02, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x01, 0x01, 0x04, 0x04, 0x36, 0x00,
         0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff
     };

     btdrv_SendData(hci_cmd_nonsig_end_test_buf, sizeof(hci_cmd_nonsig_end_test_buf));

     return 0;
}

int app_testmode_nonsig_test_result_save(const unsigned char *data, unsigned int len)
{
    int sRet = 0;

    BT_DRV_TRACE(1,"%s", __func__);

    const unsigned char nonsig_test_report_bt[] = {0x04, 0x0e, 0x12};

    DBG_NONSIG_TEST_EVENT_IND_T base_test_result_bt;
    DBG_NONSIG_TEST_EVENT_IND_T *pBaseResultBt = NULL;

    const unsigned char nonsig_test_report_ble[] = {0x04, 0x0e, 0x06};

    DBG_NONSIG_TEST_EVENT_IND_BLE_T base_test_result_ble;
    DBG_NONSIG_TEST_EVENT_IND_BLE_T *pBaseResultBle = NULL;

    if (0 == memcmp(data, nonsig_test_report_bt, sizeof(nonsig_test_report_bt))) {
        pBaseResultBt = (DBG_NONSIG_TEST_EVENT_IND_T *)(data + 7);
        if (pBaseResultBt->pkt_counters != 0) {
            sRet = memcpy_s(&base_test_result_bt, sizeof(base_test_result_bt), pBaseResultBt, sizeof(base_test_result_bt));
            if(sRet != 0)
            {
                TRACE(1,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
            }
            app_testmode_test_result_bt.pkt_counters = base_test_result_bt.pkt_counters;
            app_testmode_test_result_bt.head_errors = base_test_result_bt.head_errors;
            app_testmode_test_result_bt.payload_errors = base_test_result_bt.payload_errors;
            TRACE(3, "bt result save cnt:%d head:%d payload:%d", app_testmode_test_result_bt.pkt_counters,
                                                        app_testmode_test_result_bt.head_errors, app_testmode_test_result_bt.payload_errors);
        }
    }
    else if (0 == memcmp(data, nonsig_test_report_ble, sizeof(nonsig_test_report_ble))) {
        pBaseResultBle = (DBG_NONSIG_TEST_EVENT_IND_BLE_T *)(data + 7);
        if (pBaseResultBle->pkt_counters != 0) {
            sRet = memcpy_s(&base_test_result_ble, sizeof(base_test_result_ble), pBaseResultBle, sizeof(base_test_result_ble));
            if(sRet != 0)
            {
                TRACE(1,"func-s line:%d sRet:%d %s ", __LINE__, sRet, __func__);
            }
            app_testmode_test_result_ble.pkt_counters = base_test_result_ble.pkt_counters;
            TRACE(1, "ble result save cnt:%d", app_testmode_test_result_ble.pkt_counters);
        }
    }
    return 0;
}

int app_testmode_bt_nonsig_result_clear(void)
{
    if(app_testmode_test_result_bt.pkt_counters == 0) {
        return 0;
    }

    app_testmode_test_result_bt.pkt_counters = 0;
    app_testmode_test_result_bt.payload_errors = 0;
    app_testmode_test_result_bt.head_errors = 0;

    TRACE(0, "clear bt test reult");

    return 0;
}

int app_testmode_ble_nonsig_result_clear(void)
{
    if(app_testmode_test_result_ble.pkt_counters == 0) {
        return 0;
    }

    app_testmode_test_result_ble.pkt_counters = 0;

    TRACE(0, "clear ble test reult");

    return 0;
}

int app_testmode_bt_nonsig_result_get(APP_DBG_NONSIG_TESTER_RESULT_BT_T *bt_result)
{
    if(!bt_result) {
        return -1;
    }

    bt_result->pkt_counters = app_testmode_test_result_bt.pkt_counters;
    bt_result->payload_errors = app_testmode_test_result_bt.payload_errors;
    bt_result->head_errors = app_testmode_test_result_bt.head_errors;

    TRACE(3, "final bt test result cnt:%d head:%d payload:%d", bt_result->pkt_counters, bt_result->head_errors, bt_result->payload_errors);

    app_testmode_bt_nonsig_result_clear();

    return 0;
}

int app_testmode_ble_tx_v1(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V1_T *ble_tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     uint8_t hci_cmd_nonsig_ble_tx_buf[] =
     {
         0x01, 0x1e, 0x20, 0x03, 0x00, 0x25, 0x00
     };

     hci_cmd_nonsig_ble_tx_buf[4] = ble_tx_cfg->tx_freq;
     hci_cmd_nonsig_ble_tx_buf[5] = ble_tx_cfg->test_data_len;
     hci_cmd_nonsig_ble_tx_buf[6] = ble_tx_cfg->pkt_payload;

     btdrv_SendData(hci_cmd_nonsig_ble_tx_buf, sizeof(hci_cmd_nonsig_ble_tx_buf));

     return 0;
}

int app_testmode_ble_rx_v1(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V1_T *ble_rx_cfg)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    btdrv_hci_reset();

    uint8_t hci_cmd_nonsig_ble_rx_buf[] =
    {
        0x01, 0x1d, 0x20, 0x01, 0x00
    };

    hci_cmd_nonsig_ble_rx_buf[4] = ble_rx_cfg->rx_freq;

    btdrv_SendData(hci_cmd_nonsig_ble_rx_buf, sizeof(hci_cmd_nonsig_ble_rx_buf));

    return 0;
}

int app_testmode_ble_tx_v2(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V2_T *ble_tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     uint8_t hci_cmd_nonsig_ble_tx_buf[] =
     {
         0x01, 0x34, 0x20, 0x04, 0x00, 0x25, 0x00, 0x01
     };

     hci_cmd_nonsig_ble_tx_buf[4] = ble_tx_cfg->tx_freq;
     hci_cmd_nonsig_ble_tx_buf[5] = ble_tx_cfg->test_data_len;
     hci_cmd_nonsig_ble_tx_buf[6] = ble_tx_cfg->pkt_payload;
     hci_cmd_nonsig_ble_tx_buf[7] = ble_tx_cfg->phy;

     btdrv_SendData(hci_cmd_nonsig_ble_tx_buf, sizeof(hci_cmd_nonsig_ble_tx_buf));

     return 0;
}

int app_testmode_ble_rx_v2(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V2_T *ble_rx_cfg)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    btdrv_hci_reset();

    uint8_t hci_cmd_nonsig_ble_rx_buf[] =
    {
        0x01, 0x33, 0x20, 0x03, 0x00, 0x01, 0x00
    };

    hci_cmd_nonsig_ble_rx_buf[4] = ble_rx_cfg->rx_freq;
    hci_cmd_nonsig_ble_rx_buf[5] = ble_rx_cfg->phy;
    hci_cmd_nonsig_ble_rx_buf[6] = ble_rx_cfg->modulation_idx;

    btdrv_SendData(hci_cmd_nonsig_ble_rx_buf, sizeof(hci_cmd_nonsig_ble_rx_buf));

    return 0;
}

int app_testmode_ble_tx_v3(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V3_T *ble_tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     uint8_t *hci_cmd_nonsig_ble_tx_buf = NULL;
     hci_cmd_nonsig_ble_tx_buf = (uint8_t *) malloc(BLE_TX_V3_CMD_MAX_LEN);
     uint8_t cmd_len = 0;

     hci_cmd_nonsig_ble_tx_buf[0] = 0x01;
     hci_cmd_nonsig_ble_tx_buf[1] = 0x50;
     hci_cmd_nonsig_ble_tx_buf[2] = 0x20;
     hci_cmd_nonsig_ble_tx_buf[3] = 7 + ble_tx_cfg->switching_pattern_len;
     hci_cmd_nonsig_ble_tx_buf[4] = ble_tx_cfg->tx_freq;
     hci_cmd_nonsig_ble_tx_buf[5] = ble_tx_cfg->test_data_len;
     hci_cmd_nonsig_ble_tx_buf[6] = ble_tx_cfg->pkt_payload;
     hci_cmd_nonsig_ble_tx_buf[7] = ble_tx_cfg->phy;
     hci_cmd_nonsig_ble_tx_buf[8] = ble_tx_cfg->cte_len;
     hci_cmd_nonsig_ble_tx_buf[9] = ble_tx_cfg->cte_type;
     hci_cmd_nonsig_ble_tx_buf[10] = ble_tx_cfg->switching_pattern_len;
     memcpy(&hci_cmd_nonsig_ble_tx_buf[11], &ble_tx_cfg->antenna_id[0], ble_tx_cfg->switching_pattern_len);
     cmd_len = 11 + ble_tx_cfg->switching_pattern_len;

     btdrv_SendData(hci_cmd_nonsig_ble_tx_buf, cmd_len);
     free(hci_cmd_nonsig_ble_tx_buf);
     return 0;
}

int app_testmode_ble_rx_v3(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V3_T *ble_rx_cfg)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    btdrv_hci_reset();

    uint8_t *hci_cmd_nonsig_ble_rx_buf = NULL;
    hci_cmd_nonsig_ble_rx_buf = (uint8_t *) malloc(BLE_RX_V3_CMD_MAX_LEN);
    uint8_t cmd_len = 0;

    hci_cmd_nonsig_ble_rx_buf[0] = 0x01;
    hci_cmd_nonsig_ble_rx_buf[1] = 0x4f;
    hci_cmd_nonsig_ble_rx_buf[2] = 0x20;
    hci_cmd_nonsig_ble_rx_buf[3] = 7 + ble_rx_cfg->switching_pattern_len;
    hci_cmd_nonsig_ble_rx_buf[4] = ble_rx_cfg->rx_freq;
    hci_cmd_nonsig_ble_rx_buf[5] = ble_rx_cfg->phy;
    hci_cmd_nonsig_ble_rx_buf[6] = ble_rx_cfg->modulation_idx;
    hci_cmd_nonsig_ble_rx_buf[7] = ble_rx_cfg->exp_cte_len;
    hci_cmd_nonsig_ble_rx_buf[8] = ble_rx_cfg->exp_cte_type;
    hci_cmd_nonsig_ble_rx_buf[9] = ble_rx_cfg->slot_durations;
    hci_cmd_nonsig_ble_rx_buf[10] = ble_rx_cfg->switching_pattern_len;
    memcpy(&hci_cmd_nonsig_ble_rx_buf[11], &ble_rx_cfg->antenna_id[0], ble_rx_cfg->switching_pattern_len);
    cmd_len = 11 + ble_rx_cfg->switching_pattern_len;

    btdrv_SendData(hci_cmd_nonsig_ble_rx_buf, cmd_len);
    free(hci_cmd_nonsig_ble_rx_buf);
    return 0;
}

int app_testmode_ble_tx_v4(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V4_T *ble_tx_cfg)
{
     BT_DRV_TRACE(1,"%s\n", __func__);

     btdrv_hci_reset();

     uint8_t *hci_cmd_nonsig_ble_tx_buf = NULL;
     hci_cmd_nonsig_ble_tx_buf = (uint8_t *) malloc(BLE_TX_V4_CMD_MAX_LEN);
     uint8_t cmd_len = 0;
     uint8_t tx_pwr_lvl_idx = 0;

     hci_cmd_nonsig_ble_tx_buf[0] = 0x01;
     hci_cmd_nonsig_ble_tx_buf[1] = 0x7b;
     hci_cmd_nonsig_ble_tx_buf[2] = 0x20;
     hci_cmd_nonsig_ble_tx_buf[3] = ble_tx_cfg->switching_pattern_len + 8;
     hci_cmd_nonsig_ble_tx_buf[4] = ble_tx_cfg->tx_freq;
     hci_cmd_nonsig_ble_tx_buf[5] = ble_tx_cfg->test_data_len;
     hci_cmd_nonsig_ble_tx_buf[6] = ble_tx_cfg->pkt_payload;
     hci_cmd_nonsig_ble_tx_buf[7] = ble_tx_cfg->phy;
     hci_cmd_nonsig_ble_tx_buf[8] = ble_tx_cfg->cte_len;
     hci_cmd_nonsig_ble_tx_buf[9] = ble_tx_cfg->cte_type;
     hci_cmd_nonsig_ble_tx_buf[10] = ble_tx_cfg->switching_pattern_len;
     memcpy(&hci_cmd_nonsig_ble_tx_buf[11], &ble_tx_cfg->antenna_id[0], ble_tx_cfg->switching_pattern_len);
     tx_pwr_lvl_idx = 11 + ble_tx_cfg->switching_pattern_len;
     hci_cmd_nonsig_ble_tx_buf[tx_pwr_lvl_idx] = ble_tx_cfg->tx_pwr_lvl;
     cmd_len = 12 + ble_tx_cfg->switching_pattern_len;

     btdrv_SendData(hci_cmd_nonsig_ble_tx_buf, cmd_len);
     free(hci_cmd_nonsig_ble_tx_buf);
     return 0;
}

int app_testmode_ble_endtest(void)
{
    BT_DRV_TRACE(1,"%s\n", __func__);

    uint8_t hci_cmd_nonsig_ble_end_test_buf[] =
    {
        0x01, 0x1f, 0x20, 0x00
    };

    btdrv_SendData(hci_cmd_nonsig_ble_end_test_buf, sizeof(hci_cmd_nonsig_ble_end_test_buf));

    return 0;
}

int app_testmode_ble_result_get(APP_DBG_NONSIG_TESTER_RESULT_BLE_T *ble_result)
{
    if(!ble_result) {
        return -1;
    }

    ble_result->pkt_counters = app_testmode_test_result_ble.pkt_counters;

    TRACE(1, "final ble test cnt:%d", ble_result->pkt_counters);

    app_testmode_ble_nonsig_result_clear();

    return 0;
}

APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_TX_T tx_cfg_DH1={0x02,0x01,0x11,0x22,0x33,0x44,0x55,0x66,APP_DBG_NONSIG_TYPE_DH1,0x00};
APP_DBG_NONSIG_TESTER_SETUP_SLIM_CMD_RX_T rx_cfg_DH1 ={0x02,0x11,0x22,0x33,0x44,0x55,0x66,APP_DBG_NONSIG_TYPE_DH1};

APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V1_T ble_rx_v1_cfg_00 = {0x00};
APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V1_T ble_tx_v1_cfg_00 = {0x00,0x25,PAYL_PSEUDO_RAND_9};

APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V2_T ble_rx_v2_cfg_00 = {0x00,0x01,0x00};
APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V2_T ble_tx_v2_cfg_00 = {0x00,0x25,PAYL_PSEUDO_RAND_9,0x01};

void app_testmode_ble_rx_v3_cfg_00_init(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V3_T *ble_rx_v3_cfg_00)
{
    uint8_t antenna_id[2] = {0x00,0x00};
    ble_rx_v3_cfg_00->rx_freq = 0;
    ble_rx_v3_cfg_00->phy = 0x01;
    ble_rx_v3_cfg_00->modulation_idx = 0x00;
    ble_rx_v3_cfg_00->exp_cte_len = 0;
    ble_rx_v3_cfg_00->exp_cte_type = 0;
    ble_rx_v3_cfg_00->slot_durations = 0x01;
    ble_rx_v3_cfg_00->switching_pattern_len = 2;
    memcpy(&ble_rx_v3_cfg_00->antenna_id[0], &antenna_id[0], ble_rx_v3_cfg_00->switching_pattern_len);
}

void app_testmode_ble_tx_v3_cfg_00_init(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V3_T *ble_tx_v3_cfg_00)
{
    uint8_t antenna_id[2] = {0x00,0x00};
    ble_tx_v3_cfg_00->tx_freq = 0;
    ble_tx_v3_cfg_00->test_data_len = 0x25;
    ble_tx_v3_cfg_00->pkt_payload = PAYL_PSEUDO_RAND_9;
    ble_tx_v3_cfg_00->phy = 0x01;
    ble_tx_v3_cfg_00->cte_len = 0;
    ble_tx_v3_cfg_00->cte_type = 0;
    ble_tx_v3_cfg_00->switching_pattern_len = 2;
    memcpy(&ble_tx_v3_cfg_00->antenna_id[0], &antenna_id[0], ble_tx_v3_cfg_00->switching_pattern_len);
}

void app_testmode_ble_tx_v4_cfg_00_init(APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V4_T *ble_tx_v4_cfg_00)
{
    uint8_t antenna_id[2] = {0x00,0x00};
    ble_tx_v4_cfg_00->tx_freq = 0;
    ble_tx_v4_cfg_00->test_data_len = 0x25;
    ble_tx_v4_cfg_00->pkt_payload = PAYL_PSEUDO_RAND_9;
    ble_tx_v4_cfg_00->phy = 0x01;
    ble_tx_v4_cfg_00->cte_len = 0;
    ble_tx_v4_cfg_00->cte_type = 0;
    ble_tx_v4_cfg_00->switching_pattern_len = 2;
    memcpy(&ble_tx_v4_cfg_00->antenna_id[0], &antenna_id[0], ble_tx_v4_cfg_00->switching_pattern_len);
    ble_tx_v4_cfg_00->tx_pwr_lvl = 0x7f;
}


void app_enter_signal_testmode(void)
{
    app_testmode_enter();
    btdrv_enable_dut();
}

void app_exit_signal_testmode(void)
{
    app_testmode_exit();
    btdrv_dut_mode_enable = false;
}

void app_enter_nosignal_tx_testmode(void)
{
    app_testmode_enter();
    app_testmode_bt_nonsig_tx(&tx_cfg_DH1);
}

void app_enter_nosignal_rx_testmode(void)
{
    app_testmode_enter();
    app_testmode_bt_nonsig_rx(&rx_cfg_DH1);
}

void app_exit_nosignal_trx_testmode(void)
{
    app_testmode_bt_nonsig_endtest();
    osDelay(1000);
    app_testmode_exit();
}

void app_enter_ble_tx_v1_testmode(void)
{
    app_testmode_enter();
    app_testmode_ble_tx_v1(&ble_tx_v1_cfg_00);
}

void app_enter_ble_rx_v1_testmode(void)
{
    app_testmode_enter();
    app_testmode_ble_rx_v1(&ble_rx_v1_cfg_00);
}

void app_enter_ble_tx_v2_testmode(void)
{
    app_testmode_enter();
    app_testmode_ble_tx_v2(&ble_tx_v2_cfg_00);
}

void app_enter_ble_rx_v2_testmode(void)
{
    app_testmode_enter();
    app_testmode_ble_rx_v2(&ble_rx_v2_cfg_00);
}

void app_enter_ble_tx_v3_testmode(void)
{
    APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V3_T ble_tx_v3_cfg_00;
    app_testmode_ble_tx_v3_cfg_00_init(&ble_tx_v3_cfg_00);
    app_testmode_enter();
    app_testmode_ble_tx_v3(&ble_tx_v3_cfg_00);
}

void app_enter_ble_rx_v3_testmode(void)
{
    APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_RX_V3_T ble_rx_v3_cfg_00;
    app_testmode_ble_rx_v3_cfg_00_init(&ble_rx_v3_cfg_00);
    app_testmode_enter();
    app_testmode_ble_rx_v3(&ble_rx_v3_cfg_00);
}

void app_enter_ble_tx_v4_testmode(void)
{
    APP_DBG_NONSIG_TESTER_SETUP_CMD_BLE_TX_V4_T ble_tx_v4_cfg_00;
    app_testmode_ble_tx_v4_cfg_00_init(&ble_tx_v4_cfg_00);
    app_testmode_enter();
    app_testmode_ble_tx_v4(&ble_tx_v4_cfg_00);
}

void app_exit_ble_trx_testmode(void)
{
    app_testmode_ble_endtest();
    osDelay(1000);
    app_testmode_exit();
}
#endif // NORMAL_TEST_MODE_SWITCH
