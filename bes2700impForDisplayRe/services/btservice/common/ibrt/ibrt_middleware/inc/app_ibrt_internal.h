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
#ifndef __APP_IBRT_INTERNAL__
#define __APP_IBRT_INTERNAL__

#include "cmsis_os2.h"
#include "app_tws_ibrt.h"

#include "cmsis_os.h"
#include "app_tws_ibrt.h"
#include "app_key.h"
#include "app_ibrt_middleware.h"

#ifdef IBRT_UI
#include "app_tws_ibrt_conn_api.h"
#include "app_ibrt_conn_evt.h"
#include "app_ui_api.h"
#include "app_ui_evt.h"
#endif

#ifdef IS_TWS_IBRT_DEBUG_SYSTEM_ENABLED
#include "app_tws_ibrt_analysis_system.h"
#endif

#define APP_IBRT_UI_MOBILE_PAIR_CANCELED(addr)                 app_tws_ibrt_mobile_pair_canceled(addr)
#define APP_IBRT_UI_GET_MOBILE_CONNHANDLE(addr)                app_tws_ibrt_get_mobile_handle(addr)
#define APP_IBAT_UI_GET_CURRENT_ROLE(addr)                     app_tws_get_ibrt_role(addr)
#define APP_IBRT_UI_GET_IBRT_HANDLE(addr)                      app_tws_ibrt_get_ibrt_handle(addr)
#define APP_IBRT_MOBILE_LINK_CONNECTED(addr)                   app_tws_ibrt_mobile_link_connected(addr)
#define APP_IBRT_SLAVE_IBRT_LINK_CONNECTED(addr)               app_tws_ibrt_slave_ibrt_link_connected(addr)
#define APP_IBRT_IS_PROFILE_EXCHANGED(addr)                    app_ibrt_conn_is_profile_exchanged(addr)
#define APP_IBRT_UI_GET_MOBILE_CONNSTATE(addr)                 app_ibrt_conn_get_mobile_constate(addr)
#define APP_IBRT_UI_GET_IBRT_CONNSTATE(addr)                   app_ibrt_conn_get_ibrt_constate(addr)
#define APP_IBRT_UI_GET_MOBILE_MODE(addr)                      app_ibrt_conn_get_mobile_mode(addr)
#define APP_IBRT_IS_A2DP_PROFILE_EXCHNAGED(addr)               app_ibrt_a2dp_profile_is_exchanged(addr)

#define IBRT_BOX_CONNECT_IDLE       (0)
#define IBRT_BOX_CONNECT_MASTER     (1<<0)
#define IBRT_BOX_CONNECT_SLAVE      (1<<1)

#ifdef __cplusplus
extern "C" {
#endif

typedef ibrt_ctrl_t app_ibrt_if_ctrl_t;

enum APP_IBRT_IF_SNIFF_CHECKER_USER_T
{
    APP_IBRT_IF_SNIFF_CHECKER_USER_HFP,
    APP_IBRT_IF_SNIFF_CHECKER_USER_A2DP,
    APP_IBRT_IF_SNIFF_CHECKER_USER_SPP,

    APP_IBRT_IF_SNIFF_CHECKER_USER_QTY
};

#define app_ibrt_if_request_modify_tws_bandwidth  app_tws_ibrt_request_modify_tws_bandwidth

#define app_ibrt_if_exit_sniff_with_mobile(mobileAddr) app_ibrt_middleware_exit_sniff_with_mobile(mobileAddr)



typedef enum {
    IBRT_BAM_NOT_ACCESSIBLE_MODE = 0x00,
    IBRT_BAM_DISCOVERABLE_ONLY = 0x01,
    IBRT_BAM_CONNECTABLE_ONLY = 0x02,
    IBRT_BAM_GENERAL_ACCESSIBLE = 0x03,
} ibrt_if_access_mode_enum;

typedef struct
{
    uint16_t spec_id;
    uint16_t vend_id;
    uint16_t prod_id;
    uint16_t prod_ver;
    uint8_t  prim_rec;
    uint16_t vend_id_source;
} ibrt_if_pnp_info;

typedef struct {
    bt_bdaddr_t btAddr;
    uint8_t linkKey[16];
} ibrt_if_link_key_info;

typedef struct {
    ble_bdaddr_t bleAddr;
    uint8_t bleLTK[16];
} ibrt_if_ble_ltk_info;

typedef struct {
    int pairedDevNum;
    ibrt_if_link_key_info linkKey[MAX_BT_PAIRED_DEVICE_COUNT];
} ibrt_if_paired_bt_link_key_info;

typedef struct {
    int blePairedDevNum;
    ibrt_if_ble_ltk_info bleLtk[BLE_RECORD_NUM];
} ibrt_if_paired_ble_ltk_info;

int app_ibrt_if_voice_report_trig_retrigger(void);

int app_ibrt_if_keyboard_notify_v2(bt_bdaddr_t *remote, APP_KEY_STATUS *status, void *param);
void app_ibrt_if_a2dp_lowlatency_scan(uint16_t interval, uint16_t window, uint8_t interlanced);
void app_ibrt_if_a2dp_restore_scan(void);
void app_ibrt_if_sco_lowlatency_scan(uint16_t interval, uint16_t window, uint8_t interlanced);
void app_ibrt_if_sco_restore_scan(void);
#ifdef IBRT_SEARCH_UI
void app_start_tws_serching_direactly();
void app_bt_manager_ibrt_role_process(const btif_event_t *Event);
#ifdef SEARCH_UI_COMPATIBLE_UI_V2
void app_ibrt_search_ui_init(bool boxOperation,app_ui_evt_t box_event);
#else
void app_ibrt_search_ui_init(bool boxOperation,ibrt_event_type evt_type);
#endif
void app_ibrt_enter_limited_mode(void);
void app_ibrt_reconfig_btAddr_from_nv();
#endif

int app_ibrt_if_sniff_checker_start(enum APP_IBRT_IF_SNIFF_CHECKER_USER_T user);
int app_ibrt_if_sniff_checker_stop(enum APP_IBRT_IF_SNIFF_CHECKER_USER_T user);
int app_ibrt_if_sniff_checker_init(uint32_t delay_ms);
int app_ibrt_if_sniff_checker_reset(void);

void app_ibrt_if_pairing_mode_refresh(void);

void app_ibrt_if_post_role_switch_handler(ibrt_mobile_info_t *p_mobile_info);

// void app_ibrt_if_sync_volume_info_v2(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Set earbud side.
 *
 * @param[in] side               Only can be set to left or right side
 ****************************************************************************************
 */
#define app_ibrt_if_set_side    app_ibrt_middleware_set_side

/**
 ****************************************************************************************
 * @brief Initialize and make preparation for TWS synchronization.
 ****************************************************************************************
 */
void app_ibrt_if_prepare_sync_info(void);


/**
 ****************************************************************************************
 * @brief Flush pending tws sync information to peer device.
 ****************************************************************************************
 */
void app_ibrt_if_flush_sync_info();


/**
 ****************************************************************************************
 * @brief Fill tws information into pending sync list.
 *
 * @param[in] id                User id for tws information filling
 *
 * @return none
 ****************************************************************************************
 */
void app_ibrt_if_sync_info(TWS_SYNC_USER_E id);

/**
 ****************************************************************************************
 * @brief Freeman mode test
 ****************************************************************************************
 */
void app_ibrt_if_test_enter_freeman(void);





/**
 ****************************************************************************************
 * @brief Get the latest newly paired mobile bt address.
 *
 * @return uint8_t* pointer of the bt address
 ****************************************************************************************
 */
uint8_t* app_ibrt_if_get_latest_paired_mobile_bt_addr(void);

/**
 ****************************************************************************************
 * @brief Clear the latest newly paired mobile bt address
 *
 * @return
 ****************************************************************************************
 */
void app_ibrt_if_clear_newly_paired_mobile(void);

/**
 ****************************************************************************************
 * @brief Initialize the newly paired mobile callback function
 *
 * @return
 ****************************************************************************************
 */
void app_ibrt_if_init_newly_paired_mobile_callback(void);

/**
 ****************************************************************************************
 * @brief Initialize the newly paired tws callback function
 *
 * @return
 ****************************************************************************************
 */
void app_ibrt_if_init_newly_paired_tws_callback(void);


#define RX_DM1  12
#define RX_2DH1 18
#define RX_2DH3 20
#define RX_2DH5 22
#define HEC_ERR 24

typedef struct
{
    uint8_t hec_err;
    uint8_t crc_err;
    uint8_t fec_err;
    uint8_t grad_err;
    uint8_t ecc_cnt;
    uint8_t rx_dm1;
    uint8_t rx_2dh1;
    uint8_t rx_2dh3;
    uint8_t rx_2dh5;
    uint8_t rx_seq_err_cnt;
    uint8_t rev_fa_cnt;
    uint32_t last_ticks;
    uint32_t curr_ticks;
} __attribute__ ((__packed__)) ll_monitor_info_t;

typedef struct
{
    int8_t lastReceivedRssi;
    int8_t currentReceivedRssi;
    uint16_t AclPacketInterval;
    uint32_t AclPacketBtclock;
} __attribute__ ((__packed__)) acl_packet_interval_t;

#define ACL_PACKET_INTERVAL_THRESHOLD_MS            60
#define TOP_ACL_PACKET_INTERVAL_CNT 1

typedef struct
{
    uint8_t retriggerType;
    //uint8_t mobileChlMap[BT_DEVICE_NUM][10];
    int8_t mobile_rssi[BT_DEVICE_NUM];
    int8_t tws_rssi;
    uint32_t clock;
    acl_packet_interval_t acl_packet_interval[TOP_ACL_PACKET_INTERVAL_CNT];
    ll_monitor_info_t ll_monitor_info;

} __attribute__ ((__packed__)) connectivity_log_t;

#define RECORD_RX_NUM 5

typedef struct
{
    uint32_t clkn;
    int8_t rssi;
} __attribute__ ((__packed__)) rx_clkn_rssi_t;

typedef struct
{
    uint8_t disconnectReson;
    uint8_t disconnectObject;
    uint8_t lcState;
    uint8_t addr[6];
    uint8_t activeConnection;
    uint8_t mobileCurrMode[BT_DEVICE_NUM];
    uint8_t twsCurrMode;
    uint32_t mobileSniffInterval[BT_DEVICE_NUM];
    uint32_t twsSniffInterval;
    rx_clkn_rssi_t rxClknRssi[RECORD_RX_NUM];
} __attribute__ ((__packed__)) disconnect_reason_t;

typedef void (*reportConnectivityLogCallback_t)(connectivity_log_t* connectivity_log);
typedef void (*reportDisconnectReasonCallback_t)(disconnect_reason_t *disconnectReason);
typedef void (*connectivity_log_report_intersys_api)(uint8_t* data);
typedef void (*remoteDevice_name_Callback_t)(const bt_bdaddr_t *current_addr, const uint8_t *devName);
extern connectivity_log_report_intersys_api ibrt_if_report_intersys_callback;

void app_ibrt_if_report_connectivity_log_init(void);
void app_ibrt_if_register_report_connectivity_log_callback(reportConnectivityLogCallback_t callback);
void app_ibrt_if_register_report_disonnect_reason_callback(reportDisconnectReasonCallback_t callback);
void app_ibrt_if_save_bt_clkoffset(uint32_t clkoffset, uint8_t device_id);
void app_ibrt_if_disconnect_event(btif_remote_device_t *rem_dev, bt_bdaddr_t *addr, uint8_t disconnectReason, uint8_t activeConnection);
void app_ibrt_if_save_curr_mode_to_disconnect_info(uint8_t currMode, uint32_t interval, bt_bdaddr_t *addr);
void app_ibrt_if_update_rssi_info(const char* tag, rx_agc_t link_agc_info, uint8_t device_id);
void app_ibrt_if_update_chlMap_info(const char* tag, uint8_t *chlMap, uint8_t device_id);
void app_ibrt_if_report_audio_retrigger(uint8_t retriggerType);
void app_ibrt_if_report_connectivity_log(void);
void app_ibrt_if_update_link_monitor_info(uint8_t *ptr);
void app_ibrt_if_reset_acl_data_packet_check(void);
void app_ibrt_if_reset_tws_acl_data_packet_check(void);
void app_ibrt_if_check_acl_data_packet_during_a2dp_streaming(void);
void app_ibrt_if_check_tws_acl_data_packet(void);
void app_ibrt_if_acl_data_packet_check_handler(uint8_t *data);
#ifdef __IAG_BLE_INCLUDE__
void app_ibrt_if_set_nonConn_adv_data(uint8_t len, uint8_t *adv_data, uint16_t interval, bool enable);
void app_ibrt_if_stop_ble_adv(void);
#endif /*__IAG_BLE_INCLUDE__*/
typedef void (*hfp_vol_sync_done_cb)(void);
void app_ibrt_if_register_hfp_vol_sync_done_callback(hfp_vol_sync_done_cb callback);
void hfp_ibrt_sync_status_sent_callback(void);
void app_ibrt_if_get_sbm_status_callback(void);
bool app_ibrt_if_is_hfp_status_sync_from_master(uint8_t deviceId);
#ifdef CUSTOM_BITRATE
void app_ibrt_if_set_codec_param(uint32_t aac_bitrate,uint32_t sbc_boitpool,uint32_t audio_latency);
void app_ibrt_user_a2dp_codec_info_action(void);
#endif

void app_ibrt_internal_hold_background_switch(void);
void app_ibrt_internal_register_ibrt_cbs(void);
void app_ibrt_internal_profile_connect(uint8_t device_id, int profile_id, uint32_t extra_data);
void app_ibrt_internal_profile_disconnect(uint8_t device_id, int profile_id);
void app_ibrt_internal_disonnect_rfcomm(bt_spp_channel_t *dev,uint8_t reason);
void app_ibrt_set_profile_connect_protect(uint8_t device_id, int profile_id);
void app_ibrt_clear_profile_connect_protect(uint8_t device_id, int profile_id);
void app_ibrt_set_profile_disconnect_protect(uint8_t device_id, int profile_id);
void app_ibrt_clear_profile_disconnect_protect(uint8_t device_id, int profile_id);
bool app_ibrt_internal_is_tws_role_switch_on(void);
bool app_ibrt_internal_role_unified(void);
void app_ibrt_set_audio_trigger_protect(void);
void app_ibrt_clear_audio_trigger_protect(void);
/**************************************APIs For Customer**********************************************/

/**
 ****************************************************************************************
 * @brief Connect hfp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_connect_hfp_profile(uint8_t device_id);

/**
 ****************************************************************************************
 * @brief Connect a2dp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_connect_a2dp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Connect avrcp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_connect_avrcp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Disconnect hfp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_disconnect_hfp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Disconnect a2dp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_disconnect_a2dp_profile(uint8_t device_id);


/**
 ****************************************************************************************
 * @brief Disconnect avrcp profile, only for ibrt master
 *
 * @param[in] device_id            device index
 ****************************************************************************************
 */
void app_ibrt_if_master_disconnect_avrcp_profile(uint8_t device_id);
void app_ibrt_profile_protect_timer_init(void);

void app_ibrt_stop_profile_protect_timer(uint8_t device_id);

void app_ibrt_start_profile_connect_delay_timer(uint8_t device_id, int profile_id);

void app_ibrt_start_profile_disconnect_delay_timer(uint8_t device_id, int profile_id);

void app_ibrt_if_tws_switch_prepare_done_in_bt_thread(IBRT_ROLE_SWITCH_USER_E user, uint32_t role);

app_ibrt_if_ctrl_t *app_ibrt_if_get_bt_ctrl_ctx(void);

void app_ibrt_internal_ctx_checker(void);

void app_ibrt_internal_get_tws_conn_state_test(void);

int app_ibrt_if_force_audio_retrigger(uint8_t retriggerType);

#ifdef PRODUCTION_LINE_PROJECT_ENABLED
/**
 ****************************************************************************************
 * @brief Open box test API for production line
 ****************************************************************************************
 */
void app_ibrt_if_test_open_box(void);


/**
 ****************************************************************************************
 * @brief Close box test API for production line
 ****************************************************************************************
 */
void app_ibrt_if_test_close_box(void);
#endif

bool app_ibrt_internal_post_custom_reboot_handler(void);

bool app_ibrt_internal_is_tws_addr(const uint8_t* pBdAddr);

void app_ibrt_internal_stack_is_ready(void);

void app_ibrt_internal_enable_bluetooth(void);

void app_ibrt_internal_link_disconnected(void);

void app_ibrt_internal_disable_bluetooth(void);

void app_ibrt_internal_enter_freeman_pairing(void);

void app_ibrt_internal_enter_pairing_after_power_on(void);

void app_ibrt_internal_update_tws_pairing_info(ibrt_role_e role, uint8_t* peerAddr);

void app_ibrt_internal_start_tws_pairing(ibrt_role_e role, uint8_t* peerAddr);

typedef void (*app_ibrt_if_ui_role_update_callback)(uint8_t newRole);
void app_ibrt_internal_register_tws_ui_role_update_handle(app_ibrt_if_ui_role_update_callback func);

#ifdef __cplusplus
}
#endif

#endif /*__APP_IBRT_INTERNAL__*/
