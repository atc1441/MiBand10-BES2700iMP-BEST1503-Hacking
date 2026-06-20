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
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "bt_drv_interface.h"
#include "intersyshci.h"
#include "apps.h"
#include "bluetooth_bt_api.h"
#include "app_bt.h"
#include "app_factory.h"
#include "app_factory_bt.h"
#include "app_utils.h"
#include "nvrecord_bt.h"
#include "nvrecord_dev.h"
#include "nvrecord_env.h"
#include "factory_section.h"
#include "pmu.h"
#include "tgt_hardware.h"
#include "app_battery.h"
#include "bt_drv_reg_op.h"
#include "conmgr_api.h"
#include "me_api.h"
#include "hal_bootmode.h"
#include "hal_chipid.h"
#include "watchdog/watchdog.h"
#include "audio_codec_api.h"
#include "app_trace_rx.h"
#include "besbt.h"

#define APP_FACT_CPU_WAKE_LOCK              HAL_CPU_WAKE_LOCK_USER_3

typedef struct
{
    const char* string;
    void (*cmd_function)(const char* param, uint32_t param_len);
} app_ibrt_test_cmd_table_t;

#ifdef __FACTORY_MODE_SUPPORT__
static uint8_t inquiry_buff[] = {0x01, 0x72, 0x77, 0xb0, 0x18, 0x57, 0x60,\
                        0x01, 0x00, 0x00, 0x00, 0x1f, 0x00, 0x00, 0x00};

static void app_bt_nonsignalingtest_end_timer_handler(void const *param);
osTimerDef(bt_nonsignalingtest_end_timer, app_bt_nonsignalingtest_end_timer_handler);
static osTimerId bt_nonsignalingtest_end_timer_id = NULL;


static void bt_error_check_timer_handler(void const *param);
osTimerDef(bt_error_check_timer, bt_error_check_timer_handler);
static osTimerId bt_error_check_timer_id = NULL;
uint8_t test_mode_type=0;

#ifdef SLIM_BTC_ONLY
extern "C" void uart_send_data_teset(void);
extern "C" int bt_notify_bridge_data(uint32_t msg);

static void bt_error_check_timer_handler(void const *param)
{
    if(bt_drv_error_check_handler())
    {
        osTimerStop(bt_error_check_timer_id);
        bt_drv_reg_op_crash_dump_2_uart();
        bt_drv_reg_op_crash_dump();

    }
}
#else
static void bt_error_check_timer_handler(void const *param)
{
    //dump rssi
    bt_drv_rssi_dump_handler();
    //adjust parameter for testmode
    bt_drv_testmode_adaptive_adjust_param();

    //check BT core status
    if(bt_drv_error_check_handler())
    {
        if(test_mode_type==1)
        {
            hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_SIGNALINGMODE);
        }
        else if(test_mode_type==2)
        {
            hal_sw_bootmode_set(HAL_SW_BOOTMODE_TEST_MODE|HAL_SW_BOOTMODE_TEST_NOSIGNALINGMODE);
        }
        pmu_reboot();
    }
}
#endif

static void app_factorymode_bt_inquiry_buff_update(void)
{
    bt_bdaddr_t flsh_dongle_addr;
    int ret = -1;

    ret = nvrec_dev_get_dongleaddr(&flsh_dongle_addr);
    if(0 == ret) {
        memcpy((void *)&inquiry_buff[1],(void *)flsh_dongle_addr.address,BTIF_BD_ADDR_SIZE);
        DUMP8("0x%02x ", &inquiry_buff[2], BTIF_BD_ADDR_SIZE);
    }
}

#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
void app_factorymode_CmgrCallback(btif_cmgr_handler_t *cHandler,
                              cmgr_event_t    Event,
                              bt_status_t     Status)
{
    APP_FACTORY_TRACE(4,"%s cHandler:%p Event:%d status:%d", __func__, cHandler, Event, Status);
    if (Event == BTIF_CMEVENT_DATA_LINK_CON_CNF){
        if (Status == BT_STS_SUCCESS){
            APP_FACTORY_TRACE(0,"connect ok");
            app_factorymode_result_set(true);
            btif_cmgr_remove_data_link((bt_bdaddr_t *)(inquiry_buff+1));
        }else{
            APP_FACTORY_TRACE(0,"connect failed");
            app_factorymode_result_set(false);
        }
    }

    if (Event == BTIF_CMEVENT_DATA_LINK_DIS){
        if (Status == BT_STS_SUCCESS){
            APP_FACTORY_TRACE(0,"disconnect ok");
        }else{
            APP_FACTORY_TRACE(0,"disconnect failed");
        }
    }
}
#endif

static void app_factorymode_bt_InquiryResult_add(void)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    U8 len = 15;
    bool rssi = false, extended  = false;
    U8* parm = (U8*)inquiry_buff;

    /* Found one or more devices. Report to clients */
    APP_FACTORY_TRACE(4,"%s len:%d rssi:%d extended:%d", __func__, len, rssi, extended);
    DUMP8("0x%02x ", parm, len);
    btif_me_inquiry_result_setup(parm, rssi, extended);
#endif
}

void app_factorymode_bt_create_connect(void)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    bt_status_t status;
    bt_bdaddr_t *bdAddr = (bt_bdaddr_t *)(inquiry_buff+1);

    status = btif_cmgr_create_data_link(NULL, bdAddr);
    APP_FACTORY_TRACE(2,"%s:%d", __func__, status);
#endif
}

void app_factorymode_bt_init_connect(void)
{
#if defined(BT_BUILD_WITH_CUSTOMER_HOST) || defined(BLE_ONLY_ENABLED)
#else // bes classic bt is disabled
    void *app_factorymode_cmgrHandler = btif_cmgr_get_default_conn_handler();
    btif_cmgr_register_handler(app_factorymode_cmgrHandler, app_factorymode_CmgrCallback);
#endif
    app_factorymode_bt_inquiry_buff_update();
    app_factorymode_bt_InquiryResult_add();
}

#if defined(CHIP_BEST1400) || defined(CHIP_BEST1402)
#define XTAL_FCAP_RANGE (0x1FF)
#else
#define XTAL_FCAP_RANGE (0xFF)
#endif

void app_factorymode_bt_xtalrangetest(APP_KEY_STATUS *status, void *param)
{
    dev_addr_name devinfo;
    uint32_t fcap = 0;
    APP_FACTORY_TRACE(1,"%s",__func__);
    app_wdt_close();
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
        osTimerStart(bt_error_check_timer_id, 1000);
    }
    test_mode_type = 1;
    app_status_indication_set(APP_STATUS_INDICATION_TESTMODE);
    pmu_sleep_en(0);
    BESHCI_Close();
    btdrv_hciopen();
    btdrv_hci_reset();

    btdrv_sleep_config(0);
    osDelay(2000);
    btdrv_ins_patch_test_init();
    btdrv_feature_default();
    devinfo.btd_addr = bt_get_local_address();
    devinfo.ble_addr = bt_get_ble_local_address();
    devinfo.localname = bt_get_local_name();
    nvrec_dev_localname_addr_init(&devinfo);
    btdrv_write_localinfo((char *)devinfo.localname, strlen(devinfo.localname) + 1, devinfo.btd_addr);

    btdrv_vco_test_start(78);
    while(1){
        btdrv_rf_init_xtal_fcap(fcap%XTAL_FCAP_RANGE);
        osDelay(300);
        TRACE(2,"xtal tune:%d", fcap%XTAL_FCAP_RANGE);
        fcap++;
    }
}

void app_factorymode_bt_signalingtest(APP_KEY_STATUS *status, void *param)
{
    dev_addr_name devinfo;
    APP_FACTORY_TRACE(1,"%s",__func__);
    app_wdt_close();
#ifndef BT_SIGNALTEST_SLEEP_EN
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
#endif
    app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
#ifdef BT_SIGNALTEST_SLEEP_EN
        osTimerStart(bt_error_check_timer_id, 5000);
#else
        osTimerStart(bt_error_check_timer_id, 1000);
#endif
    }
    test_mode_type = 1;
    app_status_indication_set(APP_STATUS_INDICATION_TESTMODE);
    pmu_sleep_en(0);
    BESHCI_Close();
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    btdrv_hci_reset();

    btdrv_sleep_config(0);
    osDelay(2000);
    btdrv_testmode_start();
    btdrv_feature_default();
    devinfo.btd_addr = bt_get_local_address();
    devinfo.ble_addr = bt_get_ble_local_address();
    devinfo.localname = bt_get_local_name();
    devinfo.ble_name= bt_get_local_name();
    nvrec_dev_localname_addr_init(&devinfo);
#ifdef __IBRT_IBRT_TESTMODE__
    uint8_t ibrt_address[6] = {0x11,0x22,0x33,0x44,0x55,0x66};
    bt_set_local_address(ibrt_address);
    memcpy(devinfo.btd_addr,ibrt_address,6);
#endif
    btdrv_write_localinfo((char *)devinfo.localname, strlen(devinfo.localname) + 1, devinfo.btd_addr);
    bt_drv_config_after_hci_reset();
    btdrv_enable_dut();
#ifdef __IBRT_IBRT_TESTMODE__
    btdrv_enable_ibrt_test();
#endif
#ifdef BT_SIGNALTEST_SLEEP_EN
    btdrv_sleep_config(1);
    pmu_sleep_en(1);
#endif

}

static uint8_t test_end_timer_argument = TEST_ERROR_TYPE;
static void app_bt_start_nonsignalingtest_end_timer(uint32_t timeout, uint8_t timer_type)
{
    APP_FACTORY_TRACE(1,"app factory: timeout =%d", timeout);

    do{
        if(timeout < 2000)
        {
            break;
        }

        test_end_timer_argument = timer_type;
        if (!bt_nonsignalingtest_end_timer_id){
            bt_nonsignalingtest_end_timer_id = osTimerCreate(osTimer(bt_nonsignalingtest_end_timer),
                osTimerOnce, (void *)(&test_end_timer_argument));
        }

        if (bt_nonsignalingtest_end_timer_id != NULL) {
            osTimerStart(bt_nonsignalingtest_end_timer_id, timeout);
        }
    }while(0);
}
WEAK void app_factory_run_next_nonsignalingtest(void)
{
    return;
}

static void app_bt_nonsignalingtest_end_timer_handler(void const *param)
{
    uint8_t timer_type_param = *((uint8_t *)param);
    if(timer_type_param == LE_TEST_RX_TYPE)
    {
        btdrv_le_test_end();
    }
    else if(timer_type_param == BT_NONSIGNALING_RX_TYPE)
    {
        btdrv_nosig_stop_rx_test_set();
        app_factory_run_next_nonsignalingtest();
    }
    else
    {
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
        nv_record_flash_flush();
        pmu_reboot();
    }

    test_end_timer_argument = TEST_ERROR_TYPE;
}

static bool app_le_test_end_hander(uint8_t status, uint16_t nb_packet_received)
{
    APP_FACTORY_TRACE(0, "APP factory:LE test end, status=%d, packet_received =%d", status, nb_packet_received);
    nv_record_update_factory_le_rx_test_result((status == 0), nb_packet_received);

    //if you want to reconnect mobile,|HAL_SW_BOOTMODE_TEST_NORMAL_MODE
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    nv_record_flash_flush();
    osDelay(500);
    pmu_reboot();

    return true;
}

WEAK bool app_factory_reboot(void)
{
    return true;
}

static bool app_bt_nonsignaling_test_end_hander(uint8_t status, uint16_t nb_packet_received, uint16_t hec_nb, uint16_t crc_nb)
{
    do{
        if(status == 0 && nb_packet_received == 0 && hec_nb == 0xffff && crc_nb == 0xffff)
        {
           break;
        }
        APP_FACTORY_TRACE(0, "APP factory:BT non-singnalint test end, status=0x%x, packet_received =%d,hec error =%d, crc error=%d",
            status, nb_packet_received, hec_nb, crc_nb);

        nv_record_update_factory_bt_nonsignaling_test_result((status == NONSIGNALING_TEST_OK), nb_packet_received, hec_nb, crc_nb);

        if(app_factory_reboot())
        {
          //if you want to reconnect mobile,|HAL_SW_BOOTMODE_TEST_NORMAL_MODE
          hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
          nv_record_flash_flush();
          osDelay(500);
          pmu_reboot();
        }
    }while(0);


    return true;
}

app_factory_test_ind_handler_func app_factory_test_ind_callback = NULL;
void app_factory_register_test_ind_callback(app_factory_test_ind_handler_func cb)
{
    app_factory_test_ind_callback = cb;
}

void app_factory_enter_le_rx_test_v2(int timeout, int remote_tx_nb, int rx_channel, int phy, int mod_idx)
{
    do{

        APP_FACTORY_TRACE(0, "APP Factory:LERX test to=%d, remote_tx_nb=%d, rx_channel=%d, phy=%d,mod id=%d",
                    timeout, remote_tx_nb, rx_channel, phy, mod_idx);
        nv_record_update_factory_le_rx_test_env(timeout, remote_tx_nb,
                    NV_LE_NONSIGNALING_RX_MODE, rx_channel, phy, mod_idx);

        app_enter_non_signalingtest_mode();
    }while(0);
}


void app_factory_enter_le_tx_test_v2(int timeout, int tx_channel, int data_len, int pkt_payload, int phy)
{
    do{

#if 0
        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_LE_NONSIGNALING_TX_MODE);
        }

        if(need_block == true)
        {
            break;
        }
#endif
        APP_FACTORY_TRACE(0, "APP Factory:LETX test to=%d, tx_channel=%d, data len=%d, payl_type=%d, phy=%d", timeout,
            tx_channel, data_len, pkt_payload, phy);

        nv_record_update_factory_le_tx_test_env(timeout, NV_LE_NONSIGNALING_TX_MODE, tx_channel, data_len, pkt_payload, phy);
        app_enter_non_signalingtest_mode();
    }while(0);
}


bool app_factory_read_le_test_result(uint16_t *test_result, uint16_t * remote_tx_nb)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    bool test_done = nvrecord_env->le_rx_env.test_done;
    uint16_t result_temp = nvrecord_env->le_rx_env.test_result;
    uint16_t tx_packet_nb_temp = nvrecord_env->factory_test_env.tx_packet_nb;

    if(test_result != NULL)
    {
        *test_result = result_temp;
    }

    if(remote_tx_nb)
    {
        * remote_tx_nb = tx_packet_nb_temp;
    }

    APP_FACTORY_TRACE(0, "APP Factory: read LE rx test done=%d, received nb=%d, remote tx nb=%d",
        test_done, result_temp, tx_packet_nb_temp);

    if(test_done)
    {
        //flush le rx test result
        nv_record_update_factory_le_rx_test_result(false, 0);
    }

    return test_done;
}

bool app_factory_read_bt_nonsign_test_result(void)
{
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    bool test_done = false;
    uint16_t result_temp = 0;
    uint16_t hec_error_nb = 0;
    uint16_t crc_error_nb = 0;
    nv_record_factory_get_bt_nonsignalint_rx_test_result(&test_done, &result_temp, &hec_error_nb, &crc_error_nb);

    APP_FACTORY_TRACE(0, "APP Factory: BT rx test done=%d, recv nb=%d, hec nb=%d, crc nb=%d",
        test_done, result_temp, hec_error_nb, crc_error_nb);

    if(test_done)
    {
        //flush BT non-signaling rx test result
        nv_record_update_factory_bt_nonsignaling_test_result(false, 0, 0 ,0);
    }

    return test_done;
}

void app_factory_enter_tx_single_tone_test(uint8_t channel)
{

    do{

        APP_FACTORY_TRACE(1, "APP Factory: BT tx single tone: %d \n", channel);
        nv_record_update_factory_tx_single_tone_env(channel);
        app_enter_non_signalingtest_mode();

    }while(0);

}

static const uint8_t test_instrument_addr[6] = {0x51, 0x33, 0x33, 0x22, 0x11, 0x11};

WEAK const uint8_t* app_bt_factorty_get_test_instrument_addr(void)
{
#ifdef FACTORY_REVERT_INSTRUMENT_ADDR
    static uint8_t revert_addr[6];

    for (int i = 0; i < 6; i++)
    {
        revert_addr[i] = test_instrument_addr[5 - i];
    }

    return revert_addr;
#else
    return test_instrument_addr;
#endif
}

void app_bt_factory_bt_nonsign_rx_test_ind(uint32_t test_end_timeout, struct nvrecord_env_t *nvrecord_env)
{
    app_bt_start_nonsignalingtest_end_timer(test_end_timeout, BT_NONSIGNALING_RX_TYPE);
    btdrv_regist_bt_nonsignalint_test_end_callback(app_bt_nonsignaling_test_end_hander);
    if(nvrecord_env != NULL)
    {

        bt_nonsignaling_test_t* param = &nvrecord_env->bt_nonsignaling_env;
        btdrv_nosig_rx_test_set(param->hopping_mode, param->whitening_mode, param->tx_freq,
            param->rx_freq, param->power_level, app_bt_factorty_get_test_instrument_addr(), param->lt_addr, param->edr_enabled,
            param->packet_type, param->payload_pattern, param->payload_length);
    }
}

void app_bt_factory_bt_nonsign_tx_test_ind(uint32_t test_end_timeout, struct nvrecord_env_t *nvrecord_env)
{
    app_bt_start_nonsignalingtest_end_timer(test_end_timeout, BT_NONSIGNALING_TX_TYPE);
    if(nvrecord_env != NULL)
    {
        uint8_t local_addr[6] = {0x55,0x55,0x55,0x55,0x55,0x55};
        const uint8_t* factory_addr = app_bt_factorty_get_test_instrument_addr();
        if(factory_addr != NULL)
        {
            memcpy(local_addr, factory_addr, 6);
        }

        bt_nonsignaling_test_t* param = &nvrecord_env->bt_nonsignaling_env;
        btdrv_nosig_tx_test_set(param->hopping_mode, param->whitening_mode, param->tx_freq,
            param->rx_freq, param->power_level, local_addr, param->lt_addr, param->edr_enabled,
            param->packet_type, param->payload_pattern, param->payload_length, param->tx_packet_num);
    }
}

void app_factory_tota_nonsignalingtest_handler(void)
{
    uint8_t test_type = NV_BT_ERROR_TEST_MODE;
    uint32_t test_end_timeout = 0;
    //fetch test mode in NV
    struct nvrecord_env_t *nvrecord_env;
    nv_record_env_get(&nvrecord_env);

    if(nvrecord_env->factory_test_env.test_type != NV_BT_ERROR_TEST_MODE)
    {
        test_type = nvrecord_env->factory_test_env.test_type;
        test_end_timeout = nvrecord_env->factory_test_env.test_end_timeout;
    }

    switch (test_type)
    {
        case NV_BT_NONSIGNALING_MODE:
            break;

        case NV_BT_NONSIGNALING_RX_MODE:
            app_bt_factory_bt_nonsign_rx_test_ind(test_end_timeout, nvrecord_env);
            break;

        case NV_BT_NONSIGNALING_TX_MODE:
            app_bt_factory_bt_nonsign_tx_test_ind(test_end_timeout, nvrecord_env);
            break;

        case NV_LE_NONSIGNALING_TX_MODE:
            app_bt_start_nonsignalingtest_end_timer(test_end_timeout, LE_TEST_TX_TYPE);
        //fall through
        case NV_LE_NONSIGNALING_CONT_TX_MODE:
            btdrv_le_tx_test_v2(nvrecord_env->le_tx_env.tx_channel, nvrecord_env->le_tx_env.data_len,
                nvrecord_env->le_tx_env.pkt_payload, nvrecord_env->le_tx_env.phy);
            break;

        case NV_LE_NONSIGNALING_RX_MODE:
            app_bt_start_nonsignalingtest_end_timer(test_end_timeout, LE_TEST_RX_TYPE);
            btdrv_regist_le_teset_end_callback(app_le_test_end_hander);
        //fall through
        case NV_LE_NONSIGNALING_CONT_RX_MODE:
            btdrv_le_rx_test_v2(nvrecord_env->le_rx_env.rx_channel, nvrecord_env->le_rx_env.phy,
                nvrecord_env->le_rx_env.mod_idx);
            break;

        default:
            APP_FACTORY_TRACE(1,"app fatorty:error type, test type=0x%x", test_type);
            break;
    }

    //clear the test mode in nv
    nv_record_reset_factory_test_env();
}

int app_battery_stop(void);
void app_factorymode_bt_nosignalingtest(APP_KEY_STATUS *status, void *param)
{
    dev_addr_name devinfo;
    APP_FACTORY_TRACE(1,"%s",__func__);
    app_wdt_close();
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
    app_status_indication_set(APP_STATUS_INDICATION_TESTMODE1);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
        osTimerStart(bt_error_check_timer_id, 1000);
    }
    test_mode_type = 2;
    app_battery_stop();
    pmu_sleep_en(0);
    BESHCI_Close();
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    bt_drv_reg_op_key_gen_after_reset(false);
    btdrv_hci_reset();
    btdrv_sleep_config(0);
    btdrv_delay_for_wakeup();
    btdrv_testmode_start();
    btdrv_feature_default();
    devinfo.btd_addr = bt_get_local_address();
    devinfo.ble_addr = bt_get_ble_local_address();
    devinfo.localname = bt_get_local_name();
    devinfo.ble_name= bt_get_local_name();
    nvrec_dev_localname_addr_init(&devinfo);
    btdrv_write_localinfo((char *)devinfo.localname, strlen(devinfo.localname) + 1, devinfo.btd_addr);
    bt_drv_config_after_hci_reset();
    app_factory_tota_nonsignalingtest_handler();

    btdrv_hcioff();
    btdrv_uart_bridge_loop();
}


extern "C" void btdrv_uart_bridge_btc(void);

void app_factorymode_btc_only_mode(APP_KEY_STATUS *status, void *param)
{
    APP_FACTORY_TRACE(1,"%s",__func__);
    app_stop_10_second_timer(APP_PAIR_TIMER_ID);
    app_stop_10_second_timer(APP_POWEROFF_TIMER_ID);
    app_status_indication_set(APP_STATUS_INDICATION_TESTMODE1);
    if (!bt_error_check_timer_id){
        bt_error_check_timer_id = osTimerCreate(osTimer(bt_error_check_timer), osTimerPeriodic, NULL);
    }
    if (bt_error_check_timer_id != NULL) {
        osTimerStart(bt_error_check_timer_id, 1000);
    }

    BESHCI_Close();
    btdrv_hciopen();
    bt_drv_reg_op_key_gen_after_reset(false);
    btdrv_hcioff();
    btdrv_uart_bridge_btc();
}

int app_factorymode_bt_xtalcalib_proc(void)
{
    uint32_t capval = 0x80;
    int nRet;

    APP_FACTORY_TRACE(1,"%s",__func__);
    hal_cpu_wake_lock(APP_FACT_CPU_WAKE_LOCK);
    APP_FACTORY_TRACE(1,"calib default, capval:%d", capval);
    btdrv_hciopen();
    btdrv_ins_patch_test_init();
    btdrv_hci_reset();
    btdrv_hcioff();
    capval = 0x80;
    bt_drv_calib_open();
    nRet = bt_drv_calib_result_porc(&capval);
    bt_drv_calib_close();
    TRACE(2,"!!!!!!!!!!!!!!!!!!!!!!!!!!!calib ret:%d, capval:%d", nRet, capval);
    if (!nRet)
        nvrec_dev_set_xtal_fcap((unsigned int)capval);

    return nRet;
}

void app_factorymode_bt_xtalcalib(APP_KEY_STATUS *status, void *param)
{
    APP_FACTORY_TRACE(1,"%s",__func__);
    app_factorymode_bt_xtalcalib_proc();
}

#ifdef APP_TRACE_RX_ENABLE
static void app_factory_enter_le_rx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        if (cmd == NULL || cmd_len == 0)
        {
            APP_FACTORY_TRACE(3, "%s invalid param %p %d", __func__, cmd, cmd_len);
            break;
        }

        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_LE_NONSIGNALING_RX_MODE);
        }

        if(need_block == true)
        {
            break;
        }

        int timeout = 0;
        int remote_tx_nb = 0;
        int rx_channel = 0;
        int phy = 0;
        int mod_idx = 0;
        sscanf(cmd, "%d %d %d %d %d",&timeout, &remote_tx_nb, &rx_channel, &phy, &mod_idx);

        APP_FACTORY_TRACE(0, "APP Factory:LERX test to=%d, remote_tx_nb=%d, rx_channel=%d, phy=%d,mod id=%d",
                    timeout, remote_tx_nb, rx_channel, phy, mod_idx);
        nv_record_update_factory_le_rx_test_env(timeout, remote_tx_nb,
                    NV_LE_NONSIGNALING_RX_MODE, rx_channel, phy, mod_idx);

        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_enter_le_tx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        if (cmd == NULL || cmd_len == 0)
        {
            APP_FACTORY_TRACE(3, "%s invalid param %p %d", __func__, cmd, cmd_len);
            break;
        }

        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_LE_NONSIGNALING_TX_MODE);
        }

        if(need_block == true)
        {
            break;
        }

        int timeout = 0;
        int tx_channel = 0;
        int data_len = 0;
        int pkt_payload = 0;
        int phy = 0;
        sscanf(cmd, "%d %d %d %d %d", &timeout, &tx_channel, &data_len, &pkt_payload, &phy);

        APP_FACTORY_TRACE(0, "APP Factory:LETX test to=%d, tx_channel=%d, data len=%d, payl_type=%d, phy=%d", timeout,
            tx_channel, data_len, pkt_payload, phy);

        nv_record_update_factory_le_tx_test_env(timeout, NV_LE_NONSIGNALING_TX_MODE, tx_channel, data_len, pkt_payload, phy);
        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_enter_le_continueous_tx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        if (cmd == NULL || cmd_len == 0)
        {
            APP_FACTORY_TRACE(3, "%s invalid param %p %d", __func__, cmd, cmd_len);
            break;
        }

        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_LE_NONSIGNALING_CONT_TX_MODE);
        }

        if(need_block == true)
        {
            break;
        }

        int tx_channel = 0;
        int data_len = 0;
        int pkt_payload = 0;
        int phy = 0;
        sscanf(cmd, "%d %d %d %d", &tx_channel, &data_len, &pkt_payload, &phy);

        APP_FACTORY_TRACE(0, "APP Factory:LETX continueous test, tx_channel=%d, data len=%d, payl_type=%d, phy=%d",
            tx_channel, data_len, pkt_payload, phy);

        nv_record_update_factory_le_tx_test_env(0, NV_LE_NONSIGNALING_CONT_TX_MODE, tx_channel, data_len, pkt_payload, phy);
        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_enter_le_continueous_rx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        if (cmd == NULL || cmd_len == 0)
        {
            APP_FACTORY_TRACE(3, "%s invalid param %p %d", __func__, cmd, cmd_len);
            break;
        }

        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_LE_NONSIGNALING_CONT_RX_MODE);
        }

        if(need_block == true)
        {
            break;
        }

        int rx_channel = 0;
        int phy = 0;
        int mod_idx = 0;
        sscanf(cmd, "%d %d %d", &rx_channel,&phy,&mod_idx);

        TRACE(0, "APP Factory:LERX continueous test,  rx_channel=%d, phy=%d,mod id=%d", rx_channel, phy, mod_idx);
        nv_record_update_factory_le_rx_test_env(0, 0,  NV_LE_NONSIGNALING_CONT_RX_MODE, rx_channel, phy, mod_idx);

        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_change_dst_mtu_test(const char* cmd, uint32_t cmd_len)
{
    do {
        int targetms = 0;
        sscanf(cmd, "%d", &targetms);

        TRACE(0, "[%s] targetms %d", __func__, targetms);

        audio_bt_update_latency_buff_size(targetms);
    } while(0);
}

static void app_factory_remote_fetch_le_teset_result_test(const char* cmd, uint32_t cmd_len)
{
    do{
        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_READ_LE_TEST_RESULT);
        }

        if(need_block == true)
        {
            break;
        }

        uint16_t result_report = 0;
        uint16_t remote_tx_nb  = 1000;
        bool done = app_factory_read_le_test_result(&result_report, &remote_tx_nb);
        if(remote_tx_nb > 0 && remote_tx_nb > result_report && done)
        {
            APP_FACTORY_TRACE(0, "APP Factory: rx test success, PER=%d/%d", remote_tx_nb - result_report, remote_tx_nb);
        }
        else
        {
            APP_FACTORY_TRACE(0, "APP Factory: rx test fail, rxnb =%d, remote_tx_nb=%d", result_report, remote_tx_nb);
        }
    }while(0);
}

static void app_factory_terminate_le_test(const char* cmd, uint32_t cmd_len)
{
}

static void app_factory_enter_signalingtest_mode_test(const char* cmd, uint32_t cmd_len)
{
    app_enter_signalingtest_mode();
}

static void app_factory_enter_non_signalingtest_mode_test(const char* cmd, uint32_t cmd_len)
{
    app_enter_non_signalingtest_mode();
}

static void app_factory_enter_bt_38chanl_dh5_prbs9_tx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_BT_NONSIGNALING_TX_MODE);
        }

        if(need_block == true)
        {
            break;
        }
        APP_FACTORY_TRACE(0, "APP Factory: BT non-singnaling test 0091");

        nv_record_update_factory_bt_nonsignaling_env(5000, NV_BT_NONSIGNALING_TX_MODE,
            HOPSINGLE, WHITENING_MODE_ON, 38, 38,  1, 1, BT_EDR_DISABLE, DH5_TYPE,
            BT_NONSIGNALING_PRBS9_PAYLOAD, DH5_PACKET_SIZE, TX_NUM_UNDEFINE);

        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_enter_bt_38chanl_dh1_prbs9_rx_test(const char* cmd, uint32_t cmd_len)
{
    do{
        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_BT_NONSIGNALING_RX_MODE);
        }

        if(need_block == true)
        {
            break;
        }
        APP_FACTORY_TRACE(0, "APP Factory: BT non-singnaling test 0094");
        nv_record_update_factory_bt_nonsignaling_env(5000, NV_BT_NONSIGNALING_RX_MODE,
            HOPSINGLE, WHITENING_MODE_ON, 38, 38,  1, 1, BT_EDR_DISABLE, DH1_TYPE,
            BT_NONSIGNALING_PRBS9_PAYLOAD, DH1_PACKET_SIZE, TX_NUM_UNDEFINE);

        app_enter_non_signalingtest_mode();
    }while(0);
}

static void app_factory_remote_fetch_bt_nonsig_test_result_test(const char* cmd, uint32_t cmd_len)
{
    do{
        bool need_block = false;
        if(app_factory_test_ind_callback)
        {
            need_block = app_factory_test_ind_callback(NV_READ_BT_RX_TEST_RESULT);
        }

        if(need_block == true)
        {
            break;
        }

        app_factory_read_bt_nonsign_test_result();
    }while(0);
}

const static app_ibrt_test_cmd_table_t app_fact_bt_test_cmd_table[]=
{
    {"LE_TX_TEST",                      app_factory_enter_le_tx_test},
    {"LE_RX_TEST",                      app_factory_enter_le_rx_test},
    {"LE_CONT_TX_TEST",                 app_factory_enter_le_continueous_tx_test},
    {"LE_CONT_RX_TEST",                 app_factory_enter_le_continueous_rx_test},
    {"CHANGE_MTU",                      app_factory_change_dst_mtu_test},
    {"LE_TEST_RESULT",                  app_factory_remote_fetch_le_teset_result_test},
    {"LE_TEST_END",                     app_factory_terminate_le_test},
    {"BT_DUT_MODE",                     app_factory_enter_signalingtest_mode_test},
    {"NON_SIGNALING_TEST",              app_factory_enter_non_signalingtest_mode_test},
    {"0091",                            app_factory_enter_bt_38chanl_dh5_prbs9_tx_test},
    {"0094",                            app_factory_enter_bt_38chanl_dh1_prbs9_rx_test},
    {"BT_RX_TEST_RESULT",               app_factory_remote_fetch_bt_nonsig_test_result_test},
};

static unsigned int app_factory_bt_cmd_callback(unsigned char *buf, unsigned int length)
{
    // Check len
    APP_FACTORY_TRACE(1, "[%s] cmd: %s", __func__, buf);
    int cmd_count;
    uint8_t cmd_number;
    char *param_addr=NULL;
    unsigned int param_len=0;

    cmd_number = ARRAY_SIZE(app_fact_bt_test_cmd_table);
    for (cmd_count = 0; cmd_count < cmd_number; cmd_count++)
    {
        if ((strncmp((char*)buf, app_fact_bt_test_cmd_table[cmd_count].string,
                strlen(app_fact_bt_test_cmd_table[cmd_count].string)) == 0) ||
                strstr(app_fact_bt_test_cmd_table[cmd_count].string, (char*)buf))
        {
            param_addr = strstr((char*)buf, "|");
            if(param_addr != NULL)
            {
                param_addr++;
                param_len = length - (param_addr - (char *)buf);
            }

            app_fact_bt_test_cmd_table[cmd_count].cmd_function(param_addr, param_len);
            break;
        }
    }

    if (cmd_count >= cmd_number)
    {
        TRACE(0,"bt host not found,cmd=%s", buf);
    }

    return 0;
}
#endif

void app_factory_bt_cmd_init(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0, "factory_bt_cmd init");
    app_trace_rx_register("FACTORY_BT", app_factory_bt_cmd_callback);
#else
    TRACE(0, "factory_bt_cmd init fail!, not open APP_TRACE_RX_ENABLE");
#endif
    return;
}

void app_factory_bt_cmd_deinit(void)
{
#ifdef APP_TRACE_RX_ENABLE
    TRACE(0, "factory_bt_cmd deinit");
    app_trace_rx_deregister("FACTORY_BT");
#else
    TRACE(0, "factory_bt_cmd deinit fail!, not open APP_TRACE_RX_ENABLE");
#endif
}

#endif
