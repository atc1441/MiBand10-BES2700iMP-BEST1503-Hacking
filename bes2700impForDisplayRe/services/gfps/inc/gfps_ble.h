/***************************************************************************
 *
 * Copyright 2020-2025 BES.
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
#ifndef __GFPS_BLE_H__
#define __GFPS_BLE_H__

/**
 ****************************************************************************************
 * @addtogroup APP
 *
 * @brief Device Information Application Module Entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */
#define BLE_FASTPAIR_NORMAL_ADVERTISING_INTERVAL (160)
#define BLE_FASTPAIR_FAST_ADVERTISING_INTERVAL (48)
#define BLE_FASTPAIR_SPOT_ADVERTISING_INTERVAL (1000)
#define APP_GFPS_USE_KEY_COUNT                 (4)

// enable IS_USE_CUSTOM_FP_INFO if wanna use custom fastpair tx power, model id and anti-proof key
#define IS_USE_CUSTOM_FP_INFOx

#define APP_GFPS_SERVICE_UUID            0xFE2C
#define APP_SPOT_SERVICE_UUID            0xFEAA
#define APP_CAS_SERVICE_UUID             0x1853

#ifndef BLE_INVALID_CONNECTION_INDEX
#define BLE_INVALID_CONNECTION_INDEX 0xFF
#endif

/// Manufacturer Name Value
#define APP_GFPS_MANUFACTURER_NAME       ("RivieraWaves SAS")
#define APP_GFPS_MANUFACTURER_NAME_LEN   (16)

/// Model Number String Value
#define APP_GFPS_MODEL_NB_STR            ("RW-BLE-1.0")
#define APP_GFPS_MODEL_NB_STR_LEN        (10)

/// Serial Number
#define APP_GFPS_SERIAL_NB_STR           ("1.0.0.0-LE")
#define APP_GFPS_SERIAL_NB_STR_LEN       (10)

/// Firmware Revision
#define APP_GFPS_FIRM_REV_STR            ("6.1.2")
#define APP_GFPS_FIRM_REV_STR_LEN        (5)

/// System ID Value - LSB -> MSB
#define APP_GFPS_SYSTEM_ID               ("\x12\x34\x56\xFF\xFE\x9A\xBC\xDE")
#define APP_GFPS_SYSTEM_ID_LEN           (8)

/// Hardware Revision String
#define APP_GFPS_HARD_REV_STR           ("1.0.0")
#define APP_GFPS_HARD_REV_STR_LEN       (5)

/// Software Revision String
#define APP_GFPS_SW_REV_STR              ("6.3.0")
#define APP_GFPS_SW_REV_STR_LEN          (5)

/// IEEE
#define APP_GFPS_IEEE                    ("\xFF\xEE\xDD\xCC\xBB\xAA")
#define APP_GFPS_IEEE_LEN                (6)

/**
 * PNP ID Value - LSB -> MSB
 *      Vendor ID Source : 0x02 (USB Implementer’s Forum assigned Vendor ID value)
 *      Vendor ID : 0x045E      (Microsoft Corp)
 *      Product ID : 0x0040
 *      Product Version : 0x0300
 */
#define APP_GFPS_PNP_ID                    ("\x02\x5E\x04\x40\x00\x00\x03")
#define APP_GFPS_PNP_ID_LEN                (7)

#define APP_GFPS_ADV_POWER_UUID             "\x02\x0a\xf5"
#define APP_GFPS_ADV_POWER_UUID_LEN         (3)
#define APP_GFPS_ADV_APPEARANCE_UUID        "\x03\x19\xda\x96"
#define APP_GFPS_ADV_APPEARANCE_UUID_LEN    (4)
#define APP_GFPS_ADV_MANU_SPE_UUID_TEST     "\x07\xFF\xe0\x00\x01\x5B\x32\x01"
#define APP_GFPS_ADV_MANU_SPE_UUID_LEN      (8)

#define KEY_BASE_RSP_LEN                    (16)
#define KEY_BASE_RSP_SALT_LEN               (9)
#define KEY_BASE_EXT_RSP_SALT_LEN           (7)
#define GFPSP_ENCRYPTED_RSP_LEN             (16)

#define RAW_REQ_FLAGS_DISCOVERABILITY_BIT0_EN               (1)
#define RAW_REQ_FLAGS_DISCOVERABILITY_BIT0_DIS              (0)
#define RAW_REQ_FLAGS_INTBONDING_SEEKERADDR_BIT1_EN         (1)
#define RAW_REQ_FLAGS_INTBONDING_SEEKERADDR_BIT1_DIS        (0)

#define GFPSP_KEY_BASED_PAIRING_REQ_LEN_WITH_PUBLIC_KEY     (80)
#define GFPSP_KEY_BASED_PAIRING_REQ_LEN_WITHOUT_PUBLIC_KEY  (16)

#define IN_USE_ACCOUNT_KEY_HEADER           (0x06)
#define NONE_IN_USET_ACCOUNT_KEY_HEADER     (0x04)
#define MOST_RECENT_USED_ACCOUNT_KEY_HEADER (0x05)

#define APP_GFPS_RANDOM_RESOLVABLE_DATA_TYPE    (0x06)

#ifdef SASS_ENABLED
#define SASS_ADV_LEN_MAX                         (4)
#define APP_GFPS_ADV_VERSION                     (0x01)
#define APP_GFPS_ADV_LEN_SALT                    (2)
#else
#define APP_GFPS_ADV_VERSION                     (0x00)
#define APP_GFPS_ADV_LEN_SALT                    (1)
#endif

#define APP_GFPS_ADV_FLAG                        (0x0)
#define APP_GFPS_ADV_TYPE_ACCKEY_FILETER_SHOW_UI (0x0)
#define APP_GFPS_ADV_TYPE_ACCKEY_FILETER_HIDE_UI (0x10)
#define APP_GFPS_ADV_TYPE_SALT                   (0x01)

#define GFPS_BOND_OVER_BLE                       (1)
#define GFPS_BOND_OVER_BT                        (2)

#define GFPS_BATTERY_VALUE_MAX_COUNT             (3)

#define GFPS_SEEKER_PASSKEY_KEY                  0x00
#define GFPS_PROVIDER_PASSKEY_KEY                0x01

#define GFPS_PASSKEY_LEN                         0x03

enum {
    GFPS_STATUS_SUCCESS,
    GFPS_STATUS_PEINDING,
    GFPS_STATUS_FAIL,
};


typedef enum
{
    KEY_BASED_PAIRING_REQ = 0x00,
    KEY_BASED_PAIRING_RSP = 0x01,
    KEY_BASED_PAIRING_EXT_RSP = 0x02,
    ACTION_REQUEST        = 0x10,
} GFPS_BLE_MESSAGE_TYPE_E;

typedef struct {
    uint8_t message_type;
    uint8_t reserved[15];
} gfps_ble_raw_req_t;

typedef struct {
    uint8_t uint128_array[16];
} gfps_ble_encrypted_req_uint128;

typedef struct {
    uint8_t public_key_64B[64];
} gfps_ble_64B_public_key;

typedef struct {
    uint8_t message_type; // KEY_BASED_PAIRING_RSP
    uint8_t provider_addr[6];
    uint8_t salt[KEY_BASE_RSP_SALT_LEN];
} gfps_ble_raw_resp;

typedef struct {
    uint8_t message_type; // KEY_BASED_PAIRING_EXT_RSP
    uint8_t flags_reserved        :  5;
    uint8_t addr_type             :  1;
    uint8_t flags_ble_bonding     :  1;
    uint8_t flags_is_le_only      :  1;

    uint8_t addrNum;
    uint8_t local_addr[6];
    uint8_t peerAddrandSalt[KEY_BASE_EXT_RSP_SALT_LEN];
} gfps_ble_raw_ext_resp;

typedef struct {
    uint8_t message_type;
    uint8_t passkey[3];
    uint8_t reserved[12];
} gfps_ble_raw_pass_key_resp;

typedef struct {
    uint8_t uint128_array[GFPSP_ENCRYPTED_RSP_LEN];
} gfps_ble_encrypted_resp;

typedef struct {
    uint8_t message_type;   // KEY_BASED_PAIRING_REQ
    uint8_t flags_reserved          :   2;
    uint8_t flags_support_le_audio  :   1;
    uint8_t flags_support_le        :   1;
    uint8_t flags_retroactively_write_account_key   :   1;
    uint8_t flags_get_existing_name :   1;
    uint8_t flags_bonding_addr      :   1;
    uint8_t flags_discoverability   :   1;

    uint8_t provider_addr[6];
    uint8_t seeker_addr[6];
    uint8_t reserved[2];
} gfps_ble_key_based_pairing_req_t;

typedef struct {
    uint8_t message_type;   // ACTION_REQUEST
    uint8_t flags_reserved          :   6;
    uint8_t isFollowedByAdditionalDataCh   :   1;
    uint8_t isDeviceAction          :   1;

    uint8_t provider_addr[6];
    uint8_t messageGroup;           // Mandatory if Flags Bit 1 is set.
    uint8_t messageCode;            // Mandatory if Flags Bit 1 is set.
    uint8_t additionalDataLen;      // Mandatory if Flags Bit 1 is set. Less than 6
    uint8_t additionalData[6];      // Mandatory if Flags Bit 1 is set.
} gfps_ble_action_req_t;

typedef struct {
    gfps_ble_encrypted_req_uint128 * en_req;
    gfps_ble_64B_public_key        * pub_key;
} gfps_ble_key_based_pairing_req;

typedef struct {
    union{
        gfps_ble_raw_req_t                 raw_req;
        gfps_ble_encrypted_req_uint128     en_req;
        gfps_ble_key_based_pairing_req_t   key_based_pairing_req;
        gfps_ble_action_req_t              action_req;
        gfps_ble_raw_resp                  key_based_pairing_rsp;
        gfps_ble_encrypted_resp            en_rsp;
    }rx_tx;
} gfps_ble_req_resp;

typedef struct {
    uint8_t message_type;
    uint8_t passkey[3];
    uint8_t bonding_addr[6];
    uint8_t status;
    uint8_t reserved[5];
}gfpsp_ble_additional_pass_key_resp;


#ifdef SPOT_ENABLED
#define GFPS_NONCE_SIZE                 8
#define GFPS_AUTH_KEY_SIZE              8
#define GFPS_RECOVERY_KEY_SIZE          8
#define GFPS_RING_KEY_SIZE              8

#define GFPS_BEACON_READ_BEACON_PARAM                 0x00
#define GFPS_BEACON_READ_PROVISION_STATE              0x01
#define GFPS_BEACON_SET_EPHEMERAL_IDENTITY_KEY        0x02
#define GFPS_BEACON_CLEAR_EPHEMERAL_IDENTITY_KEY      0x03

#define GFPS_BEACON_READ_EPHEMERAL_IDENTITY_KEY       0x04
#define GFPS_BEACON_RING                              0x05
#define GFPS_BEACON_READ_RING_STATE                   0x06
#define GFPS_BEACON_ACTIVATE_UNWANTED_TRACK_MODE      0x07
#define GFPS_BEACON_DEACTIVATE_UNWANTED_TRACK_MODE    0x08
#define GFPS_BEACON_MULITI_OPERATION                  0x09

#define GFPS_BEACON_RINGING_STATE_STATED              0x00
#define GFPS_BEACON_RINGING_STATE_FAILED              0x01
#define GFPS_BEACON_RINGING_STATE_STOPPED_TIMEOUT     0x02
#define GFPS_BEACON_RINGING_STATE_STOPPED_PRESS       0x03
#define GFPS_BEACON_RINGING_STATE_STOPPED_REQUEST     0x04

#define GFPS_BEACON_RINGING_NONE                      0x00
#define GFPS_BEACON_RINGING_RIGHT                     0x01
#define GFPS_BEACON_RINGING_LEFT                      0x02
#define GFPS_BEACON_RINGING_RIGHT_AND_LEFT            0x03
#define GFPS_BEACON_RINGING_BOX                       0x04
#define GFPS_BEACON_RINGING_ALL                       0xFF

#define GFPS_BEACON_INCAPABLE_OF_RING                 0x00
#define GFPS_BEACON_ONE_CAPABLE_OF_RING               0x01
#define GFPS_BEACON_TWO_CAPABLE_OF_RING               0x02
#define GFPS_BEACON_THREE_CAPABLE_OF_RING             0x03
#define GFPS_BEACON_SECP_160R1_METHOD                 0x00
#define GFPS_BEACON_SECP_256R1_METHOD                 0x01

#define GFPS_BEACON_RINGING_VOLUME_NOT_AVAILABLE      0x00
#define GFPS_BEACON_RINGING_VOLUME_AVAILABLE          0x01

#define GFPS_BEACON_PROTOCOL_VERSION                  0x01

#define GFPS_BEACON_CONTROL_FLAG_SKIP_RING_AUT        0x01

#define READ_COMPONENTS_STATE                         0x01
#define ACTIVATE_MULTI_TRACKING_MODE                  0x02

#define GFPS_BEACON_NOT_SUPPORT_BATTERY               0x00
#define GFPS_BEACON_NORMAL_BATTERY                    0x02
#define GFPS_BEACON_LOW_BATTERY                       0x04
#define GFPS_BEACON_CRITICALLY_LOW_BATTERY            0x06

#define GFPS_BEACON_IN_TRACKING_MODE                  0x01
#define GFPS_BEACON_IN_UNTRACKING_MODE                0x00

#define GFPS_BEACON_UNKOWN_PLACEMENT                  0x00
#define GFPS_BEACON_LEFT_BUD_IN_CASE                  0x10
#define GFPS_BEACON_LEFT_BUD_OUT_OF_CASE              0x20
#define GFPS_BEACON_RIGHT_BUD_IN_CASE                 0x01
#define GFPS_BEACON_RIGHT_BUD_OUT_OF_CASE             0x02


typedef struct {
    int8_t power_value;
    uint8_t clock_value[4];
    uint8_t SECP_method;
    uint8_t numbesr_of_ringing;
    uint8_t ringing_capability;
    uint8_t padgding[8];
} gfps_ble_spot_read_beacon_additional_data;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t additional_data[16];
} gfps_ble_spot_read_beacon_state_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data;
} gfps_ble_spot_read_beacon_provision_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data;
    uint8_t EIK[20];
} gfps_ble_spot_read_EIK_beacon_provision_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
} gfps_ble_spot_read_set_beacon_provision_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
} gfps_ble_spot_clear_EIK_beacon_provision_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data[32];
} gfps_ble_spot_read_beacon_identity_key_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data[4];
} gfps_ble_spot_beacon_ring_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data[3];
} gfps_ble_spot_read_beacon_ring_state_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
} gfps_ble_spot_activate_unwanted_tracking_mode_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
} gfps_ble_spot_deactivate_unwanted_tracking_mode_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data[5];
} gfps_ble_spot_read_components_state_resp;

typedef struct {
    uint8_t data_id;
    uint8_t data_length;
    uint8_t auth_data[8];
    uint8_t data;
} gfps_ble_spot_activate_tracking_mode_resp;

#endif

struct gfps_ble_env_tag
{
    uint8_t connectionIndex;
    uint8_t isNotificationEnabled[BLE_CONNECTION_MAX];
    uint8_t isInitialPairing;
    bt_bdaddr_t seeker_bt_addr;
    bt_bdaddr_t local_le_addr;
    bt_bdaddr_t local_bt_addr;
    uint8_t keybase_pair_key[16];
    uint8_t aesKeyFromECDH[16];
    uint8_t isPendingForWritingNameReq;
    uint8_t advRandSalt;
    uint8_t isInFastPairing;
    uint8_t bondMode;
    uint8_t txPower[APP_GFPS_ADV_POWER_UUID_LEN];
#ifdef SPOT_ENABLED
    uint8_t protocol_version;
    uint8_t nonce[GFPS_NONCE_SIZE];
    uint8_t adv_identifer[20];
    uint8_t beacon_time[4];
    uint16_t remaining_ring_time;
    uint8_t control_flag;
    bool enable_unwanted_tracking_mode;
    uint8_t orignal_flag;
    uint8_t hashed_flag;
    uint8_t Ecc_private_key[20];
    bool ring_state;
    bool not_allowed_retry_flag;
#endif
    bool gfps_flag;
    uint8_t passkey[GFPS_PASSKEY_LEN];
    uint8_t addition_passkey[GFPS_PASSKEY_LEN];
};

/*
 * GLOBAL FUNCTIONS DECLARATION
 ****************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Initialize Device Information Service Application
 ****************************************************************************************
*/
void gfps_ble_init(void);

/**
 ****************************************************************************************
 * @brief Get Connection index of the Device
 ****************************************************************************************
*/
uint8_t gfps_ble_get_connect_dev(void);
void big_little_switch(const uint8_t *in, uint8_t *out, uint8_t len);
void gfps_set_flag(bool flag);
void gfps_set_if_send_passkey(bool flag);
bool gfps_get_if_send_passkey();
bool gfps_get_flag();
void gfps_send_flag_to_slave(bool flag);
void gfps_send_passkey_to_slave(uint8_t *p_buff, uint16_t length);
uint8_t* gfps_get_keybase_pair_key();
int gfps_ble_get_conidx(void);
void gfps_ble_tx_ccc_changed(uint8_t conidx, bool notify_enabled);
uint8_t gfps_ble_is_connected(uint8_t conidx);
uint8_t* gfps_get_passkey();
void gfps_set_pass_key(uint8_t *p_buff, uint16_t length);
uint8_t* gfps_get_additional_passkey();
void gfps_set_additional_pass_key(uint8_t *p_buff, uint16_t length);
void gfps_ble_get_addr(uint8_t *addr);
#ifdef SPOT_ENABLED
bool gfps_ble_get_spot_get_mode(void);
uint8_t* gfps_ble_get_EID(void);
uint8_t gfps_ble_get_hashed_value(void);
void app_gfps_spot_press_to_factory_handle(APP_KEY_STATUS *status, void *param);
void gfps_ble_spot_generate_EID_handler(uint8_t* orig_eph_identity_key);
void gfps_ble_spot_generate_hashed_value();
#endif

#ifdef __cplusplus
}
#endif


#endif //__GFPS_BLE_H__
