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
#ifndef __BES_GAP_API_H__
#define __BES_GAP_API_H__
#include "ble_core_common.h"
#include "nvrecord_extension.h"
#include "stdbool.h"
#ifdef BLE_HOST_SUPPORT
#ifdef __cplusplus
extern "C" {
#endif

#define BES_BLE_INVALID_CONNECTION_INDEX    0xFF

/// Length of AD Type flags
#define BES_GAPM_ADV_AD_TYPE_FLAGS_LENGTH           (3)
/// Length of AD Type flags information
#define BES_GAPM_ADV_AD_TYPE_FLAGS_INFO_LENGTH      (2)

/// error code, sync see@co_error
enum bes_error
{
/*****************************************************
 ***              ERROR CODES                      ***
 *****************************************************/

    BES_ERROR_NO_ERROR                        = 0x00,
    BES_ERROR_UNKNOWN_HCI_COMMAND             = 0x01,
    BES_ERROR_UNKNOWN_CONNECTION_ID           = 0x02,
    BES_ERROR_HARDWARE_FAILURE                = 0x03,
    BES_ERROR_PAGE_TIMEOUT                    = 0x04,
    BES_ERROR_AUTH_FAILURE                    = 0x05,
    BES_ERROR_PIN_MISSING                     = 0x06,
    BES_ERROR_MEMORY_CAPA_EXCEED              = 0x07,
    BES_ERROR_CON_TIMEOUT                     = 0x08,
    BES_ERROR_CON_LIMIT_EXCEED                = 0x09,
    BES_ERROR_SYNC_CON_LIMIT_DEV_EXCEED       = 0x0A,
    BES_ERROR_CON_ALREADY_EXISTS              = 0x0B,
    BES_ERROR_COMMAND_DISALLOWED              = 0x0C,
    BES_ERROR_CONN_REJ_LIMITED_RESOURCES      = 0x0D,
    BES_ERROR_CONN_REJ_SECURITY_REASONS       = 0x0E,
    BES_ERROR_CONN_REJ_UNACCEPTABLE_BDADDR    = 0x0F,
    BES_ERROR_CONN_ACCEPT_TIMEOUT_EXCEED      = 0x10,
    BES_ERROR_UNSUPPORTED                     = 0x11,
    BES_ERROR_INVALID_HCI_PARAM               = 0x12,
    BES_ERROR_REMOTE_USER_TERM_CON            = 0x13,
    BES_ERROR_REMOTE_DEV_TERM_LOW_RESOURCES   = 0x14,
    BES_ERROR_REMOTE_DEV_POWER_OFF            = 0x15,
    BES_ERROR_CON_TERM_BY_LOCAL_HOST          = 0x16,
    BES_ERROR_REPEATED_ATTEMPTS               = 0x17,
    BES_ERROR_PAIRING_NOT_ALLOWED             = 0x18,
    BES_ERROR_UNKNOWN_LMP_PDU                 = 0x19,
    BES_ERROR_UNSUPPORTED_REMOTE_FEATURE      = 0x1A,
    BES_ERROR_SCO_OFFSET_REJECTED             = 0x1B,
    BES_ERROR_SCO_INTERVAL_REJECTED           = 0x1C,
    BES_ERROR_SCO_AIR_MODE_REJECTED           = 0x1D,
    BES_ERROR_INVALID_LMP_PARAM               = 0x1E,
    BES_ERROR_UNSPECIFIED_ERROR               = 0x1F,
    BES_ERROR_UNSUPPORTED_LMP_PARAM_VALUE     = 0x20,
    BES_ERROR_ROLE_CHANGE_NOT_ALLOWED         = 0x21,
    BES_ERROR_LMP_RSP_TIMEOUT                 = 0x22,
    BES_ERROR_LMP_COLLISION                   = 0x23,
    BES_ERROR_LMP_PDU_NOT_ALLOWED             = 0x24,
    BES_ERROR_ENC_MODE_NOT_ACCEPT             = 0x25,
    BES_ERROR_LINK_KEY_CANT_CHANGE            = 0x26,
    BES_ERROR_QOS_NOT_SUPPORTED               = 0x27,
    BES_ERROR_INSTANT_PASSED                  = 0x28,
    BES_ERROR_PAIRING_WITH_UNIT_KEY_NOT_SUP   = 0x29,
    BES_ERROR_DIFF_TRANSACTION_COLLISION      = 0x2A,
    BES_ERROR_QOS_UNACCEPTABLE_PARAM          = 0x2C,
    BES_ERROR_QOS_REJECTED                    = 0x2D,
    BES_ERROR_CHANNEL_CLASS_NOT_SUP           = 0x2E,
    BES_ERROR_INSUFFICIENT_SECURITY           = 0x2F,
    BES_ERROR_PARAM_OUT_OF_MAND_RANGE         = 0x30,
    BES_ERROR_ROLE_SWITCH_PEND                = 0x32, /* LM_ROLE_SWITCH_PENDING               */
    BES_ERROR_RESERVED_SLOT_VIOLATION         = 0x34, /* LM_RESERVED_SLOT_VIOLATION           */
    BES_ERROR_ROLE_SWITCH_FAIL                = 0x35, /* LM_ROLE_SWITCH_FAILED                */
    BES_ERROR_EIR_TOO_LARGE                   = 0x36, /* LM_EXTENDED_INQUIRY_RESPONSE_TOO_LARGE */
    BES_ERROR_SP_NOT_SUPPORTED_HOST           = 0x37,
    BES_ERROR_HOST_BUSY_PAIRING               = 0x38,
    BES_ERROR_CONTROLLER_BUSY                 = 0x3A,
    BES_ERROR_UNACCEPTABLE_CONN_PARAM         = 0x3B,
    BES_ERROR_ADV_TO                          = 0x3C,
    BES_ERROR_TERMINATED_MIC_FAILURE          = 0x3D,
    BES_ERROR_CONN_FAILED_TO_BE_EST           = 0x3E,
    BES_ERROR_CCA_REJ_USE_CLOCK_DRAG          = 0x40,
    BES_ERROR_TYPE0_SUBMAP_NOT_DEFINED        = 0x41,
    BES_ERROR_UNKNOWN_ADVERTISING_ID          = 0x42,
    BES_ERROR_LIMIT_REACHED                   = 0x43,
    BES_ERROR_OPERATION_CANCELED_BY_HOST      = 0x44,
    BES_ERROR_PKT_TOO_LONG                    = 0x45,

    BES_ERROR_UNDEFINED                       = 0xFF,
};

/// GAP Advertising Flags
enum bes_gap_ad_type
{
    /// Flag
    BES_GAP_AD_TYPE_FLAGS                      = 0x01,
    /// Use of more than 16 bits UUID
    BES_GAP_AD_TYPE_MORE_16_BIT_UUID           = 0x02,
    /// Complete list of 16 bit UUID
    BES_GAP_AD_TYPE_COMPLETE_LIST_16_BIT_UUID  = 0x03,
    /// Use of more than 32 bit UUD
    BES_GAP_AD_TYPE_MORE_32_BIT_UUID           = 0x04,
    /// Complete list of 32 bit UUID
    BES_GAP_AD_TYPE_COMPLETE_LIST_32_BIT_UUID  = 0x05,
    /// Use of more than 128 bit UUID
    BES_GAP_AD_TYPE_MORE_128_BIT_UUID          = 0x06,
    /// Complete list of 128 bit UUID
    BES_GAP_AD_TYPE_COMPLETE_LIST_128_BIT_UUID = 0x07,
    /// Shortened device name
    BES_GAP_AD_TYPE_SHORTENED_NAME             = 0x08,
    /// Complete device name
    BES_GAP_AD_TYPE_COMPLETE_NAME              = 0x09,
    /// Transmit power
    BES_GAP_AD_TYPE_TRANSMIT_POWER             = 0x0A,
    /// Class of device
    BES_GAP_AD_TYPE_CLASS_OF_DEVICE            = 0x0D,
    /// Simple Pairing Hash C
    BES_GAP_AD_TYPE_SP_HASH_C                  = 0x0E,
    /// Simple Pairing Randomizer
    BES_GAP_AD_TYPE_SP_RANDOMIZER_R            = 0x0F,
    /// Temporary key value
    BES_GAP_AD_TYPE_TK_VALUE                   = 0x10,
    /// Out of Band Flag
    BES_GAP_AD_TYPE_OOB_FLAGS                  = 0x11,
    /// Slave connection interval range
    BES_GAP_AD_TYPE_SLAVE_CONN_INT_RANGE       = 0x12,
    /// Require 16 bit service UUID
    BES_GAP_AD_TYPE_RQRD_16_BIT_SVC_UUID       = 0x14,
    /// Require 32 bit service UUID
    BES_GAP_AD_TYPE_RQRD_32_BIT_SVC_UUID       = 0x1F,
    /// Require 128 bit service UUID
    BES_GAP_AD_TYPE_RQRD_128_BIT_SVC_UUID      = 0x15,
    /// Service data 16-bit UUID
    BES_GAP_AD_TYPE_SERVICE_16_BIT_DATA        = 0x16,
    /// Service data 32-bit UUID
    BES_GAP_AD_TYPE_SERVICE_32_BIT_DATA        = 0x20,
    /// Service data 128-bit UUID
    BES_GAP_AD_TYPE_SERVICE_128_BIT_DATA       = 0x21,
    /// Public Target Address
    BES_GAP_AD_TYPE_PUB_TGT_ADDR               = 0x17,
    /// Random Target Address
    BES_GAP_AD_TYPE_RAND_TGT_ADDR              = 0x18,
    /// Appearance
    BES_GAP_AD_TYPE_APPEARANCE                 = 0x19,
    /// Advertising Interval
    BES_GAP_AD_TYPE_ADV_INTV                   = 0x1A,
    /// LE Bluetooth Device Address
    BES_GAP_AD_TYPE_LE_BT_ADDR                 = 0x1B,
    /// LE Role
    BES_GAP_AD_TYPE_LE_ROLE                    = 0x1C,
    /// Simple Pairing Hash C-256
    BES_GAP_AD_TYPE_SPAIR_HASH                 = 0x1D,
    /// Simple Pairing Randomizer R-256
    BES_GAP_AD_TYPE_SPAIR_RAND                 = 0x1E,
    /// URI
    BES_GAP_AD_TYPE_URI                        = 0x24,
    /// Resolvable Set Identifier (Coordinated Set Identification Service)
    BES_GAP_AD_TYPE_RSI                        = 0x2E,
    /// Advertising Interval long
    BES_GAP_AD_TYPE_ADV_INTV_LONG              = 0x2F,
    // bcast name ad type public brodcast profile
    BES_GAP_AD_TYPE_ADV_BCAST_NAME             = 0x30,
    /// 3D Information Data
    BES_GAP_AD_TYPE_3D_INFO                    = 0x3D,
    /// Digital walkie-talkie data types(BES private type)
    BES_GAP_AD_TYPE_WALKIE                     = 0xFD,
    /// Manufacturer specific data
    BES_GAP_AD_TYPE_MANU_SPECIFIC_DATA         = 0xFF,
};

/// AD Type Flag - Bit field
enum
{
    /// Limited discovery flag - AD Flag - bit mask
    BES_GAP_LE_LIM_DISCOVERABLE_FLG_BIT     = 0x01,
    /// Limited discovery flag - AD Flag - bit position
    BES_GAP_LE_LIM_DISCOVERABLE_FLG_POS     = 0,
    /// General discovery flag - AD Flag - bit mask
    BES_GAP_LE_GEN_DISCOVERABLE_FLG_BIT     = 0x02,
    /// General discovery flag - AD Flag - bit position
    BES_GAP_LE_GEN_DISCOVERABLE_FLG_POS     = 1,
    /// Legacy BT not supported - AD Flag - bit mask
    BES_GAP_BR_EDR_NOT_SUPPORTED_BIT        = 0x04,
    /// Legacy BT not supported - AD Flag - bit position
    BES_GAP_BR_EDR_NOT_SUPPORTED_POS        =  2,
    /// Dual mode for controller supported (BR/EDR/LE) - AD Flag - bit mask
    BES_GAP_SIMUL_BR_EDR_LE_CONTROLLER_BIT  = 0x08,
    /// Dual mode for controller supported (BR/EDR/LE) - AD Flag - bit position
    BES_GAP_SIMUL_BR_EDR_LE_CONTROLLER_POS  = 3,
};

///BD address type
/// sync see@addr_type
enum bes_addr_type
{
    ///Public BD address
    BES_ADDR_PUBLIC                   = 0x00,
    ///Random BD Address
    BES_ADDR_RAND,
    /// Controller generates Resolvable Private Address based on the
    /// local IRK from resolving list. If resolving list contains no matching
    /// entry, use public address.
    BES_ADDR_RPA_OR_PUBLIC,
    /// Controller generates Resolvable Private Address based on the
    /// local IRK from resolving list. If resolving list contains no matching
    /// entry, use random address.
    BES_ADDR_RPA_OR_RAND,
    /// mask used to determine Address type in the air
    BES_ADDR_MASK                     = 0x01,
    /// mask used to determine if an address is an RPA
    BES_ADDR_RPA_MASK                 = 0x02,
    /// Random device address (controller unable to resolve)
    BES_ADDR_RAND_UNRESOLVED          = 0xFE,
    /// No address provided (anonymous advertisement)
    BES_ADDR_NONE                     = 0xFF,
};

///ADV channel map
/// sync see@adv_channel_map
enum bes_adv_channel_map
{
    ///Byte value for advertising channel map for channel 37 enable
    BES_ADV_CHNL_37_EN                = 0x01,
    ///Byte value for advertising channel map for channel 38 enable
    BES_ADV_CHNL_38_EN                = 0x02,
    ///Byte value for advertising channel map for channel 39 enable
    BES_ADV_CHNL_39_EN                = 0x04,
    ///Byte value for advertising channel map for channel 37, 38 and 39 enable
    BES_ADV_ALL_CHNLS_EN              = 0x07,
};

///Advertising filter policy
/// sync see@adv_filter_policy
enum bes_adv_filter_policy
{
    ///Allow both scan and connection requests from anyone
    BES_ADV_ALLOW_SCAN_ANY_CON_ANY    = 0x00,
    ///Allow both scan req from White List devices only and connection req from anyone
    BES_ADV_ALLOW_SCAN_WLST_CON_ANY,
    ///Allow both scan req from anyone and connection req from White List devices only
    BES_ADV_ALLOW_SCAN_ANY_CON_WLST,
    ///Allow scan and connection requests from White List devices only
    BES_ADV_ALLOW_SCAN_WLST_CON_WLST,
};

///Advertising HCI Type
enum
{
    ///Connectable Undirected advertising
    BES_ADV_CONN_UNDIR                = 0x00,
    ///Connectable high duty cycle directed advertising
    BES_ADV_CONN_DIR,
    ///Discoverable undirected advertising
    BES_ADV_DISC_UNDIR,
    ///Non-connectable undirected advertising
    BES_ADV_NONCONN_UNDIR,
    ///Connectable low duty cycle directed advertising
    BES_ADV_CONN_DIR_LDC,
};

/// Own BD address source of the device
/// sync see@APP_GAPM_OWN_ADDR_E
typedef enum
{
   /// Public or Private Static Address according to device address configuration
   BES_GAP_STATIC_ADDR,
   /// Generated resolvable private random address
   BES_GAP_GEN_RSLV_ADDR,
   /// Generated non-resolvable private random address
   BES_GAP_GEN_NON_RSLV_ADDR,
} BES_GAP_OWN_ADDR_E;

/**
 * @brief The event type of the ble
 *
 */
typedef enum
{
    BES_BLE_LINK_CONNECTED_EVENT               = 0,
    BES_BLE_CONNECT_BOND_EVENT                 = 1,    //pairing success
    BES_BLE_CONNECT_BOND_FAIL_EVENT            = 2,    //pairing failed
    BES_BLE_CONNECT_NC_EXCH_EVENT              = 3,    //Numeric Comparison - Exchange of Numeric Value
    BES_BLE_CONNECT_ENCRYPT_EVENT              = 4,    //encrypt complete
    BES_BLE_CONNECTING_STOPPED_EVENT           = 5,
    BES_BLE_CONNECTING_FAILED_EVENT            = 6,
    BES_BLE_DISCONNECT_EVENT                   = 7,
    BES_BLE_CONN_PARAM_UPDATE_REQ_EVENT        = 8,
    BES_BLE_CONN_PARAM_UPDATE_FAILED_EVENT     = 9,
    BES_BLE_CONN_PARAM_UPDATE_SUCCESSFUL_EVENT = 10,
    BES_BLE_SET_RANDOM_BD_ADDR_EVENT           = 11,
    BES_BLE_ADV_STARTED_EVENT                  = 12,
    BES_BLE_ADV_STARTING_FAILED_EVENT          = 13,
    BES_BLE_ADV_STOPPED_EVENT                  = 14,
    BES_BLE_SCAN_STARTED_EVENT                 = 15,
    BES_BLE_SCAN_DATA_REPORT_EVENT             = 16,
    BES_BLE_SCAN_STARTING_FAILED_EVENT         = 17,
    BES_BLE_SCAN_STOPPED_EVENT                 = 18,
    BES_BLE_CREDIT_BASED_CONN_REQ_EVENT        = 19,
    BES_BLE_RPA_ADDR_PARSED_EVENT              = 20,
    BES_BLE_GET_TX_PWR_LEVEL                   = 21,
    BES_BLE_TX_PWR_REPORT_EVENT                = 22,
    BES_BLE_PATH_LOSS_REPORT_EVENT             = 23,
    BES_BLE_SET_RAL_CMP_EVENT                  = 24,
    BES_BLE_SUBRATE_CHANGE_EVENT               = 25,
    BES_BLE_ENCRYPT_LTK_REPORT_EVENT           = 26,

    BES_BLE_EVENT_NUM_MAX,
} bes_ble_evnet_type_e;

typedef enum
{
    BES_BLE_TX_PWR_LEVEL_1M          = 0,
    BES_BLE_TX_PWR_LEVEL_2M          = 1,
    BES_BLE_TX_PWR_LEVEL_LE_CODED_S8 = 2,
    BES_BLE_TX_PWR_LEVEL_LE_CODED_S2 = 3,
} bes_ble_phy_pwr_value_e;

typedef enum
{
    BES_BLE_TX_LOCAL     = 0,
    BES_BLE_TX_REMOTE    = 1,
} bes_ble_tx_object_e;

typedef enum le_gap_phy_value
{
    BES_BLE_PHY_UNDEF_VALUE    = 0,
    BES_BLE_PHY_1MBPS_VALUE    = 1,
    BES_BLE_PHY_2MBPS_VALUE    = 2,
    BES_BLE_PHY_CODED_VALUE    = 3,
} bes_le_phy_val_e;

/// Specify what rate Host prefers to use in transmission on coded PHY. HCI:7.8.49
typedef enum le_gap_phy_opt
{
    /// The Host has no preferred coding when transmitting on the LE Coded PHY
    BES_BLE_PHY_OPT_NO_LE_CODED_TX_PREF,
    /// The Host prefers that S=2 coding be used when transmitting on the LE Coded PHY
    BES_BLE_PHY_OPT_S2_LE_CODED_TX_PREF,
    /// The Host prefers that S=8 coding be used when transmitting on the LE Coded PHY
    BES_BLE_PHY_OPT_S8_LE_CODED_TX_PREF,
} bes_le_phy_opt_e;

///for adv scan call back user
typedef enum BES_BLE_SCAN_RESULT_USER_ID
{
    BES_BLE_SCAN_RESULT_USER_ID_0,
    BES_BLE_SCAN_RESULT_USER_ID_1,

    BES_BLE_SCAN_RESULT_USER_MAX,
}bes_ble_scan_result_user_id_e;

/// Connection parameters
typedef struct bes_ble_scan_param
{
    // parameter specifies the type of scan to perform,
    // 0:Passive, 1:Active
    uint8_t scanType;
    // Scanning Filter Policy, see@BLE_SCAN_FILTER_POLICY
    uint8_t scanFolicyType;
    // Scan window
    uint16_t scanWindowMs;
    // Scan Interval
    uint16_t scanIntervalMs;
    // Scan Duration
    uint16_t scanDurationMs;
} bes_ble_scan_param_t;

/// Connection parameters
typedef struct bes_ble_conn_param
{
    /// Minimum value for the connection interval (in unit of 1.25ms). Shall be less than or equal to
    /// conn_intv_max value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_min;
    /// Maximum value for the connection interval (in unit of 1.25ms). Shall be greater than or equal to
    /// conn_intv_min value. Allowed range is 7.5ms to 4s.
    uint16_t conn_intv_max;
    /// Slave latency. Number of events that can be missed by a connected slave device
    uint16_t conn_latency;
    /// Link supervision timeout (in unit of 10ms). Allowed range is 100ms to 32s
    uint16_t supervision_to;
    /// Recommended minimum duration of connection events (in unit of 625us)
    uint16_t ce_len_min;
    /// Recommended maximum duration of connection events (in unit of 625us)
    uint16_t ce_len_max;
} bes_ble_conn_param_t;

/// Scan Window operation parameters
typedef struct bes_ble_scan_wd_op_param
{
    /// Scan interval
    uint16_t scan_intv_ms;
    /// Scan window
    uint16_t scan_wd_ms;
} bes_ble_scan_wd_t;

typedef struct bes_ble_bdaddr
{
    /// BD Address of device
    uint8_t addr[6];
    /// Address type of the device 0=public/1=private random
    uint8_t addr_type;
} bes_ble_bdaddr_t;

typedef struct
{
    BLE_ADV_ACTIVITY_USER_E actv_user;
    bool is_custom_adv_flags;
    BLE_ADV_ADDR_TYPE_E type;
    uint8_t *local_addr;
    bes_ble_bdaddr_t *peer_addr;
    uint32_t adv_interval;
    BLE_ADV_TYPE_E adv_type;
    ADV_MODE_E adv_mode;
    int8_t tx_power_dbm;
    uint8_t *adv_data;
    uint8_t adv_data_size;
    uint8_t *scan_rsp_data;
    uint8_t scan_rsp_data_size;
} bes_ble_gap_cus_adv_param_t;

typedef struct
{
    uint8_t io_cap;
    uint8_t has_oob_data;
    uint8_t auth_req;
    uint8_t max_enc_key_size;
    uint8_t init_key_dist;
    uint8_t resp_key_dist;
} bes_ble_smp_require_t;

typedef struct {
    uint16_t conn_interval_1_25ms;
    uint16_t peripheral_latency;
    uint16_t superv_timeout_ms;
    uint16_t central_clock_accuracy;
    uint16_t subrate_factor;
    uint16_t conn_continuation_number;
} bes_ble_conn_timing_t;

/**
 *@brief     Register gloable handler MAX handler num@BLE_MAX_CORE_EVT_CB
 *
 *@param[in] handler
 */
void bes_ble_gap_core_register_global_handler(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler);

/**
 *@brief     Unregister gloable handler
 *
 *@param[in] handler
 */
void bes_ble_gap_core_unregister_global_handler(APP_BLE_CORE_GLOBAL_HANDLER_FUNC handler);

/**
 *@brief  Stub adv init and enable if BLE_AUDIO_ENABLED == 0
 *
 */
void bes_ble_gap_stub_user_init(void);

/**
 *@brief     Init gap
 *
 *@param[in] mode @nvrec_appmode_e
 */
void bes_ble_gap_ready_and_init_done(nvrec_appmode_e mode);

typedef void (*bes_ble_adv_data_report_cb_t)(bes_ble_bdaddr_t *bleAddr, int8_t rssi, uint8_t evt_type, uint8_t *adv_buf, uint8_t len);

typedef void (*bes_ble_link_event_report_cb_t)(ble_event_handled_t param, ble_evnet_type_e type);

typedef void (*bes_ble_link_connect_cb_t)(uint8_t con_lid, bes_ble_bdaddr_t *bleAddr);

typedef void (*bes_ble_link_mtu_exch_cb_t)(uint8_t con_lid, uint32_t mtu_size);

/**
 *@brief Register adv scan report callback
 *
 *@param[in] id   User id
 *@param[in] cb   @bes_ble_adv_data_report_cb_t
 */
void bes_ble_gap_adv_report_callback_register(bes_ble_scan_result_user_id_e id, bes_ble_adv_data_report_cb_t cb);

/**
 *@brief Dregister adv scan report callback
 *
 *@param[in] id   User id
 */
void bes_ble_gap_adv_report_callback_deregister(bes_ble_scan_result_user_id_e id);

/**
 *@brief Register ble acl link event callback
 *
 *@param[in] cb   @bes_ble_link_event_report_cb_t
 */
void bes_ble_customif_link_event_callback_register(bes_ble_link_event_report_cb_t cb);

/**
 *@brief Dregister ble acl link event callback
 *
 */
void bes_ble_customif_link_event_callback_deregister(void);

/**
 *@brief Set tx rx defult pref phy
 *
 *@param[in] tx_pref_phy  Tx pref phy
 *@param[in] rx_pref_phy  Rx pref phy
 */
void bes_ble_set_tx_rx_pref_phy(uint32_t tx_pref_phy, uint32_t rx_pref_phy);

/**
 *@brief Register ble acl connect req callback
 *
 *@param[in] req_cb  @bes_ble_link_connect_cb_t
 *@param[in] done_cb @bes_ble_link_connect_cb_t
 */
void bes_ble_connect_req_callback_register(bes_ble_link_connect_cb_t req_cb, bes_ble_link_connect_cb_t done_cb);

/**
 *@brief Dregister ble acl connect req callback
 *
 */
void bes_ble_connect_req_callback_deregister(void);

/**
 *@brief Register mtu exchange ind callback
 *
 *@param[in] mtu_exec_cb @bes_ble_link_mtu_exch_cb_t
 */
void bes_ble_mtu_exec_ind_callback_register(bes_ble_link_mtu_exch_cb_t mtu_exec_cb);

/**
 *@brief Dregister mtu exchange ind callback
 *
 */
void bes_ble_mtu_exec_ind_callback_deregister(void);

/**
 *@brief Set scan coded phy and param
 *
 *@param[in] enable                    If enable
 *@param[in] start_scan_coded_scan_wd  Scan param @bes_ble_scan_wd_t
 */
void bes_ble_set_scan_coded_phy_en_and_param_before_start_scan(bool enable, bes_ble_scan_wd_t *start_scan_coded_scan_wd);

/**
 *@brief Set init conn param
 *
 *@param[in] init_param_universal  Init param@bes_ble_conn_param_t
 *@param[in] init_coded_scan_wd    Scan param@bes_ble_scan_wd_t
 */
void bes_ble_set_init_conn_all_phy_param_before_start_connect(bes_ble_conn_param_t *init_param_universal,
                                                        bes_ble_scan_wd_t *init_coded_scan_wd);

/**
 *@brief Set force adv enable or not
 *
 *@param[in] user           Set user@BLE_ADV_SWITCH_USER_E
 *@param[in] isToEnableAdv  Enable or not
 */
void bes_ble_gap_force_switch_adv(enum BLE_ADV_SWITCH_USER_E user, bool isToEnableAdv);

/**
 *@brief Stop all adv
 *
 */
void bes_ble_gap_stop_adv_all(void);

/**
 *@brief Start scan with param
 *
 *@param[in] param  Scan param@bes_ble_scan_param_t
 */
void bes_ble_gap_start_scan(bes_ble_scan_param_t *param);

/**
 *@brief Stop scan
 *
 */
void bes_ble_gap_stop_scan(void);

/**
 *@brief Start connectable adv with adv interval
 *
 *@param[in] advInterval   Adv interval
 */
void bes_ble_gap_start_connectable_adv(uint16_t advInterval);

/**
 *@brief Refresh start all it's flag is enabled adv
 *
 */
void bes_ble_gap_start_adv(void);

/**
 *@brief Set white list
 *
 *@param[in] user   List user@BLE_WHITE_LIST_USER_E
 *@param[in] bdaddr Addr list@ble_bdaddr_t
 *@param[in] size   Addr list size
 */
void bes_ble_gap_set_white_list(BLE_WHITE_LIST_USER_E user, const ble_bdaddr_t *bdaddr, uint8_t size);

/**
 *@brief Remove white list by user
 *
 *@param[in] user   White list user@BLE_WHITE_LIST_USER_E
 */
void bes_ble_gap_remove_white_list_user_item(BLE_WHITE_LIST_USER_E user);

/**
 *@brief Set rpa list
 *
 *@param[in] ble_addr  BLE addr@ble_bdaddr_t
 *@param[in] irk       IRK
 */
void bes_ble_gap_set_rpa_list(const ble_bdaddr_t *ble_addr, const uint8_t *irk);

/**
 *@brief Set bonded devices of nv record to rpa list
 *
 */
void bes_ble_gap_set_bonded_devs_rpa_list(void);

/**
 *@brief Set the length of time the Controller uses a 
 *       Resolvable Private Address before a new resolvable private
 *       address is generated and starts being used
 *
 *@param[in] rpa_timeout  Range: 0x0001 to 0x0E10, in unit of seconds
 */
void bes_ble_gap_set_rpa_timeout(uint16_t rpa_timeout);

/**
 *@brief Set adv param with user
 *
 *@param[in] param  Adv param@BLE_ADV_PARAM_T
 */
void bes_ble_gap_set_adv_param(BLE_ADV_PARAM_T *param, uint8_t user);

/**
 *@brief Test api:start three adv
 *
 *@param[in] BufPtr May ignore
 *@param[in] BufLen May ignore
 */
void bes_ble_gap_start_three_adv(uint32_t BufPtr, uint32_t BufLen);

/**
 *@brief Start custom adv with activity user
 *
 *@param[in] actv_user  @BLE_ADV_ACTIVITY_USER_E
 */
void bes_ble_gap_custom_adv_start(BLE_ADV_ACTIVITY_USER_E actv_user);

/**
 *@brief Get actv user from adv user
 *
 *@param[in] user    @BLE_ADV_USER_E
 *@return BLE_ADV_ACTIVITY_USER_E
 */
BLE_ADV_ACTIVITY_USER_E bes_ble_param_get_actv_user_from_adv_user(BLE_ADV_USER_E user);

/**
 *@brief Write custom adv param
 *
 *@param[in] param  @bes_ble_gap_cus_adv_param_t
 */
void bes_ble_gap_custom_adv_write_data(bes_ble_gap_cus_adv_param_t *param);

/**
 *@brief Stop custom adv
 *
 *@param[in] actv_user Actv user@BLE_ADV_ACTIVITY_USER_E
 */
void bes_ble_gap_custom_adv_stop(BLE_ADV_ACTIVITY_USER_E actv_user);

/**
 *@brief Test cmd stop all adv
 *
 *@param[in] BufPtr May ignore
 *@param[in] BufLen May ignore
 */
void bes_ble_gap_stop_all_adv(uint32_t BufPtr, uint32_t BufLen);

/**
 *@brief Refresh irk
 *
 */
void bes_ble_gap_refresh_irk(void);

/**
 *@brief APP adv event callback
 *
 *@param[in] connhdl  May adv handle
 *@param[in] event    @gap_adv_event_t
 *@param[in] param    @gap_adv_callback_param_t
 *@return int
 */
int bes_ble_gap_app_server_callback(uintptr_t connhdl, gap_adv_event_t event, gap_adv_callback_param_t param);

/**
 *@brief Get peer solved addr
 * 1. original address is stored in conn->peer_addr when conn establish, may be ia or rpa
 * 2. if rpa and host resolved success, related ia is stored to conn->sec.peer_addr
 * 3. if ia then host do nothing, conn->peer_addr is ia and conn->sec.peer_addr is empty
 * 4. after encrypted success, if conn->sec.peer_addr is empty, copy conn->peer_addr to conn->sec.peer_addr store into nv
 * 5. if ia is received at key dist phase, the ia is stored to conn->sec.peer_addr, and refresh nv
 *@param[in] conidx  Conn idx
 *@param[out] p_addr @ble_bdaddr_t
 *@return true/false
 */
bool bes_ble_gap_get_peer_solved_addr(uint8_t conidx, ble_bdaddr_t* p_addr);

/**
 *@brief Get curr acl conn timing
 *
 *@param[in] conidx   Conn idx
 *@return bes_ble_conn_timing_t
 */
bes_ble_conn_timing_t bes_ble_get_connection_curr_timing(uint8_t conidx);

/**
 *@brief Get curr conn att mtu
 *
 *@param[in] conidx  Conn idx
 *@return uint16_t
 */
uint16_t bes_ble_get_connection_current_att_mtu(uint8_t conidx);

/**
 *@brief Get peer device name
 *
 *@param[in] conidx   Conn idx
 *@return const char* Peer device name
 */
const char *bes_ble_gap_get_peer_device_name(uint8_t conidx);

/**
*@brief 
*
*@param[in]  addr      Peer ble device addr
*@param[in]  own_type  Owner type
*@return int @bt_status_t
 */
int bes_ble_gap_start_connect(bes_ble_bdaddr_t *addr, BES_GAP_OWN_ADDR_E own_type);

/**
*@brief 
*
*@param[in]  addr      Peer ble device addr
*@param[in]  own_type  Owner type
*@return int @bt_status_t
 */
int bes_ble_gap_connect_ble_audio_device(bes_ble_bdaddr_t *addr, BES_GAP_OWN_ADDR_E own_type);

/**
 *@brief Cancel connecting proc
 *
 */
void bes_ble_gap_cancel_connecting(void);

/**
 *@brief Set adv interval
 *
 *@param[in] adv_intv_user  @BLE_ADV_INTERVALREQ_USER_E
 *@param[in] adv_user       @BLE_ADV_USER_E
 *@param[in] interval       Adv interval
 */
void bes_ble_gap_param_set_adv_interval(BLE_ADV_INTERVALREQ_USER_E adv_intv_user, BLE_ADV_USER_E adv_user, uint32_t interval);

/**
 *@brief Set all adv tx pwr
 *
 *@param[in] txpwr_dbm  Tx pwr in dbm
 */
void bes_ble_set_all_adv_txpwr(int8_t txpwr_dbm);

/**
 *@brief If any adv started
 *
 *@return true/false
 */
bool bes_ble_gap_is_in_advertising_state(void);

/**
 *@brief Refresh all enabled adv
 */
void bes_ble_gap_refresh_adv_state(uint16_t advInterval);

/**
 *@brief Disconn acl
 *
 *@param[in] conIdx  Conn idx
 */
void bes_ble_gap_start_disconnect(uint8_t conIdx);

/**
 *@brief If idx with acl conn
 *
 *@param[in] index  Idx
 *@return true/false
 */
bool bes_ble_gap_is_connection_on(uint8_t index);

/**
 *@brief Register adv data fill handle callback
 *
 *@param[in] user   @BLE_ADV_USER_E
 *@param[in] func   @BLE_DATA_FILL_FUNC_T
 */
void bes_ble_gap_register_data_fill_handle(BLE_ADV_USER_E user, BLE_DATA_FILL_FUNC_T func, bool enable);

/**
 *@brief Enable fill
 *
 *@param[in] user   @BLE_ADV_USER_E
 *@param[in] enable True/Flase
 */
void bes_ble_gap_data_fill_enable(BLE_ADV_USER_E user, bool enable);

/**
 *@brief Disconnect all acl
 *
 */
void bes_ble_gap_disconnect_all(void);

/**
 *@brief Get conidx by peer addr
 *
 *@param[in] peer_addr  @ble_bdaddr_t
 *@return uint8_t
 */
uint8_t bes_ble_get_con_id_by_addr(const ble_bdaddr_t *peer_addr);

/**
 *@brief Disconnect acl by peer addr
 *
 *@param[in] peer_addr @ble_bdaddr_t
 *@return bt_status_t  @bt_status_t
 */
bt_status_t bes_ble_gap_disconnect_by_addr(const ble_bdaddr_t *peer_addr);

/**
 *@brief Get conn rssi
 *
 *@param[in] conidx  Conn idx
 *@return int8_t     Rssi
 */
int8_t bes_ble_gap_get_rssi(uint8_t conidx);

/**
 *@brief Clear white list for user BLE_WHITE_LIST_USER_MOBILE
 *
 */
void bes_ble_gap_clear_white_list_for_mobile(void);

/**
 *@brief Set acl phy mode
 *
 *@param[in] conidx      Conn idx
 *@param[in] tx_phy_bits Tx phy@gap_phy_bit_t
 *@param[in] rx_phy_bits Rx phy@gap_phy_bit_t
 *@param[in] phy_opt     Phy opt@gap_coded_phy_prefer_t
 */
void bes_ble_gap_set_phy_mode(uint8_t conidx, uint8_t tx_phy_bits, uint8_t rx_phy_bits, gap_coded_phy_prefer_t phy_opt);

/**
 *@brief Get phy mode
 *
 *@param[in] conidx Conn idx
 */
void bes_ble_gap_get_phy_mode(uint8_t conidx);

/**
 *@brief Get local tx pwr see event report @GAP_LE_CONN_LOCAL_TX_POWER
 *
 *@param[in] conidx Conn idx
 *@param[in] obj    @bes_ble_tx_object_e
 *@param[in] phy    @bes_ble_phy_pwr_value_e
 */
void bes_ble_gap_get_tx_pwr_value(uint8_t conidx, bes_ble_tx_object_e obj, bes_ble_phy_pwr_value_e phy);

/**
 *@brief Get dev tx pwr range see event report @GAP_LE_LOCAL_TX_POWER_RANGE
 *
 */
void bes_ble_gap_get_dev_tx_pwr_range(void);

/**
 *@brief Get adv tx pwr value see event report@GAP_LE_ADV_TX_POWER_LEVEL
 *
 */
void bes_ble_gap_get_adv_txpower_value(void);

/**
 *@brief Update conn param
 *
 *@param[in] conidx                     Conn idx
 *@param[in] min_interval_in_ms         Min interval 0x06 to 0x0C80 unit in 1.25ms
 *@param[in] max_interval_in_ms         Max interval 0x06 to 0x0C80 unit in 1.25ms
 *@param[in] supervision_timeout_in_ms  Supervison timeout
 *@param[in] slaveLatency               Slavelatency
 */
void bes_ble_gap_conn_update_param(uint8_t conidx, uint32_t min_interval_in_ms, uint32_t max_interval_in_ms,
        uint32_t supervision_timeout_in_ms, uint8_t  slaveLatency);

/**
 *@brief Update conn param by mode
 *
 *@param[in] mode     Param mode@BLE_CONN_PARAM_MODE_E
 *@param[in] isEnable Enable
 */
void bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_E mode, bool isEnable);

/**
 *@brief Send security req
 *
 *@param[in] conidx  Conn idx
 */
void bes_ble_gap_sec_send_security_req(uint8_t conidx, uint8_t sec_level);

/**
 *@brief Start authentication
 *
 *@param[in] conidx  Conn idx
 *@param[in] p_req   @bes_ble_smp_require_t
 *@return int @bt_status_t
 */
int bes_ble_gap_start_authentication(uint8_t conidx, const bes_ble_smp_require_t *p_req);

/**
 *@brief Start acl link encryption
 *
 *@param[in] conidx Conn idx
 *@param[in] ediv   Ediv
 *@param[in] rand   Rand
 *@param[in] ltk    LTK
 *@return int @bt_status_t
 */
int bes_ble_gap_enable_link_encryption(uint8_t conidx, const uint8_t* ediv, const uint8_t *rand, const uint8_t *ltk);

/**
 *@brief Set local irk
 *
 *@param[in] p_irk IRK
 */
void bes_ble_gap_set_local_irk(const uint8_t *p_irk);

/**
 *@brief Register dist LTK bit set callback
 *
 *@param[in] callback @set_rsp_dist_lk_bit_field_func
 */
void bes_ble_gap_sec_reg_dist_lk_bit_set_callback(set_rsp_dist_lk_bit_field_func callback);

/**
 *@brief Get dist LTK bit set callback
 *
 *@return set_rsp_dist_lk_bit_field_func
 */
set_rsp_dist_lk_bit_field_func bes_ble_gap_sec_reg_dist_lk_bit_get_callback();

/**
 *@brief Register smp identity info cmp callback
 *
 *@param[in] callback @smp_identify_addr_exch_complete
 */
void bes_ble_gap_sec_reg_smp_identify_info_cmp_callback(smp_identify_addr_exch_complete callback);

/**
 *@brief Register resolving list fill callback
 *
 *@param[in] user  @custom_resol_fill_user
 *@param[in] cb    @ble_resolving_list_fill_cb_t
 */
void bes_ble_gap_resolving_list_fill_cb_resgiter(uint8_t user,
                        void (*cb)(ble_bdaddr_t *bleAddr, uint8_t *peer_irk, uint8_t *local_irk));

/**
 *@brief Enable TX pwr report
 *
 *@param[in] conidx        Conn idx
 *@param[in] local_enable  Local enable
 *@param[in] remote_enable Remote enable
 */
void bes_ble_gap_tx_power_report_enable(uint8_t conidx, bool local_enable, bool remote_enable);

/**
 *@brief Subrate req
 *
 *@param[in] conidx      Conn idx
 *@param[in] subrate_min Range: 0x0001 to 0x01F4
 *@param[in] subrate_max Range: 0x0001 to 0x01F4
 *@param[in] latency_max Range: 0x0000 to 0x01F3
 *@param[in] cont_num    Range: 0x0000 to 0x01F3
 *@param[in] timeout     0x000A to 0x0C80 in unit 10ms
 */
void bes_ble_gap_subrate_request(uint8_t conidx, uint16_t subrate_min, uint16_t subrate_max,
        uint16_t latency_max, uint16_t cont_num, uint16_t timeout);

/// IBRT CALL FUNC

/**
 *@brief Start ble roleswitch
 *
 *@param[in] curr_ui_role @TWS_UI_ROLE_E
 */
void bes_ble_roleswitch_start(uint8_t curr_ui_role);

/**
 *@brief BLE roleswitch complete
 *
 *@param[in] newRole @TWS_UI_ROLE_E
 */
void bes_ble_roleswitch_complete(uint8_t newRole);

/**
 *@brief Update ble role
 *
 *@param[in] newRole @TWS_UI_ROLE_E
 */
void bes_ble_role_update(uint8_t newRole);

/**
 *@brief Entry ibrt event
 *
 *@param[in] ibrt_evt_type @app_ui_evt_t
 */
void bes_ble_ibrt_event_entry(uint8_t ibrt_evt_type);

#ifdef TWS_SYSTEM_ENABLED

/**
 *@brief Sync ble info
 *
 */
void bes_ble_sync_ble_info(void);

/**
 *@brief TWS sync init
 *
 */
void bes_ble_gap_mode_tws_sync_init(void);

#endif

/**
 *@brief Demo app recv tws sync info handler
 *
 *@param[in] p_info  Info data
 *@param[in] len     Len of data
 */
void bes_ble_demo_app_tws_sync_info_recv_handler(uint8_t *p_info, uint16_t len);

/**
 *@brief If curr acl support le role switch
 *
 *@param[in] connhdl  Conn handle
 *@return true/false
 */
bool bes_ble_is_connection_support_role_switch(uint16_t connhdl);

/**
 *@brief Save ctx
 *
 *@param[in] conidx   Conn idx
 *@param[in] buf      Data buf
 *@param[in] buf_len  Buf len
 *@return uint32_t @offset
 */
uint32_t bes_ble_app_save_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len);

/**
 *@brief Restore ctx
 *
 *@param[in] conidx  Conn idx
 *@param[in] buf     Data buf
 *@param[in] buf_len Buf len
 *@return uint32_t @offset
 */
uint32_t bes_ble_app_restore_ctx(uint8_t conidx, uint8_t *buf, uint16_t buf_len);

/**
 *@brief If any conn exist
 *
 *@return true/false
 */
bool bes_ble_gap_is_any_connection_exist(void);

/**
 *@brief If conidx is connection
 *
 *@param[in] conidx
 *@return true/false
 */
bool bes_ble_is_connection_on_by_index(uint8_t conidx);

/**
 *@brief Get curr acl cnt
 *
 *@return uint8_t @le_acl_count
 */
uint8_t bes_ble_gap_connection_count(void);

/**
 *@brief Get local static ble IA addr
 *
 *@return ble_bdaddr_t
 */
ble_bdaddr_t bes_ble_gap_get_current_ble_addr(void);

/**
 *@brief Get local hci ble IA addr
 *
 *@param[in] conidx   Conn idx
 *@return ble_bdaddr_t
 */
ble_bdaddr_t bes_ble_gap_get_local_identity_addr(uint8_t conidx);

/**
 *@brief Get local rpa addr
 *
 *@param[in] conidx      Conn idx
 *@return const uint8_t*
 */
const uint8_t *bes_ble_gap_get_local_rpa_addr(uint8_t conidx);

/**
 *@brief Read local rpa addr by adv hdl--HCI cmd
 *
 *@param[in] adv_hdl  Adv hdl
 */
void bes_ble_gap_read_local_rpa_by_adv_hdl(uint8_t adv_hdl);

/**
 *@brief Get local rpa by adv hdl saved in ble activity@ble_adv_activity_t
 *
 *@param[in] adv_hdl     Adv hdl
 *@return const uint8_t* RPA addr
 */
const uint8_t *bes_ble_gap_get_local_rpa_by_adv_hdl(uint8_t adv_hdl);

/**
 *@brief Get conn handle from idx
 *
 *@param[in] conidx Conn idx
 *@return uint16_t  Conn handle
 */
uint16_t bes_ble_gap_get_conhdl_from_conidx(uint8_t conidx);

/**
 *@brief Get conidx from handle
 *
 *@param[in] connhdl Conn handle
 *@return uint8_t    Conn idx
 */
uint8_t bes_ble_gap_get_conidx_from_conhdl(uint16_t connhdl);

/**
 *@brief GATT service send service changed
 *
 *@param[in] conidx  Conn idx
 */
void bes_ble_gatt_server_send_service_changed(uint8_t conidx);

/**
 *@brief Set ecdh pair key
 *
 *@param[in] p_sec_key_256 Sec key 256
 *@param[in] p_pub_key_256 Pub key 256
 *@return uint8_t @bt_status_t
 */
uint8_t bes_ble_gap_set_ecdh_key_pair(const uint8_t *p_sec_key_256, const uint8_t *p_pub_key_256);

/**
 *@brief Set local le public addr by HCI cmd
 *
 *@param[in] public_addr @bt_bdaddr_t
 *@return uint8_t @bt_status_t
 */
uint8_t bes_ble_gap_set_public_address(const bt_bdaddr_t *public_addr);

/**
 *@brief Set le tx pwr by HCI cmd
 *
 *@param[in] connhdl Conn handle
 *@param[in] tx_pwr  Tx pwr in dbm
 *@return uint8_t @bt_status_t
 */
uint8_t bes_ble_gap_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr);

/**
 *@brief DUMP conn state
 *
 */
void bes_ble_gap_dump_conn_state(void);

/// END IBRT CALL FUNC

#ifdef __cplusplus
}
#endif
#endif
#endif /* __BES_GAP_API_H__ */
