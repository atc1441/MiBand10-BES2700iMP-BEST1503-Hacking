/***************************************************************************
 *
 * Copyright 2015-2025 BES.
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
#ifndef __BT_GAP_SERVICE_H__
#define __BT_GAP_SERVICE_H__

/**
 * INCLUDE FILES
 *
 */

#include "adapter_service.h"

/**
 * MACRO DEFINES
 *
 *
 */
#define GAP_UNKNOWN_TX_POWER                        (0x7F)
#define GAP_UNKNOWN_RSSI                            (0x7F)
#define GAP_UNKNOWN_CONN_INTERVAL                   (0xFFFF)
#define GAP_INVALID_CONIDX                          (0xFF)
#define GAP_INVALID_ADV_HANDLE                      (0xFF)
#define GAP_INVALID_SCAN_HANDLE                     (0xFF)
#define GAP_INVALID_INIT_HANDLE                     (0xFF)
#define GAP_INVALID_ADV_SET_ID                      (0xFF)
#define GAP_PA_NO_SUBEVENT                          (0xFF)
#define GAP_INVALID_SYNC_HANDLE                     (0xFFFF)
#define GAP_INVALID_CONN_HANDLE                     (0xFFFF)
#define GAP_KEY_LEN                                 (16)
#define GAP_IV_LEN                                  (8)
#define GAP_RAND_LEN                                (8)
#define GAP_EDIV_LEN                                (2)
#define GAP_PUB_KEY_LEN                             (32)
#define GAP_ADDR_LEN                                (6)
#define GAP_COD_LEN                                 (3)
#define GAP_ADV_SID_MAX                             (0xF)
#define GAP_DEFAULT_RPA_TIMEOUT                     (900) // 15min
#define GAP_MAX_DEVICE_NAME_LEN                     (248)
#define GAP_ALL_CONNS                               (0xFFFFFFFF)
#define GAP_RESOLVABLE_TAG_LEN                      (6)
#define GAP_DECISION_DATA_MAX_LEN                   (8)
#define GAP_FILTER_INVALID_USER                     (0x7F)
#define GAP_DYN_ADV_HANDLE_START                    (0x10)
#define GAP_MAX_ADV_HANDLE_VALUE                    (0xEF)
#define GAP_MAX_ADV_ACTIVITY                        (5)
#define GAP_MAX_LEGACY_ADV_DATA_LEN                 (31)
#define GAP_MAX_EXTEND_ADV_DATA_LEN                 (251)
#define GAP_LE_LIMIT_ADV_TIMEOUT                    (30*1000) // max 180s, max advertising time in limited discoverable mode, required value
#define GAP_MAX_ATT_PREP_Q_SIZE                     (16)
#define GAP_MAX_FILTER_LIST_SIZE                    (10)
#define GAP_MAX_RESOLV_LIST_SIZE                    (10)
#define GAP_MAX_ADV_MONITOR_LIST_SIZE               (10)
#define GAP_MAX_NUM_PA_SYNC_LIST                    (8)
#define GAP_MAX_PEND_BLE_INIT                       (5)
#define GAP_MAX_ADV_PARAM_FOR_CONN                  (GAP_MAX_ADV_ACTIVITY + 2)

#define GAP_FEAT_PAGE_0_NUM_U32                     (2)
#define GAP_FEAT_PAGE_1_NUM_U32                     (1)
#define GAP_FEAT_LE_ENCRYPTION                      (0x00000001) /* send to peer (Y) host controlled (N) */
#define GAP_FEAT_CONN_PARAM_REQ_PROC                (0x00000002) /* (Y) (N) */
#define GAP_FEAT_EXT_REJ_INDICATION                 (0x00000004) /* (Y) (N) */
#define GAP_FEAT_PERIPH_INIT_FEAT_EXCHG             (0x00000008) /* (Y) (N) */
#define GAP_FEAT_LE_PING                            (0x00000010) /* (O) (N) */
#define GAP_FEAT_LE_DATA_LENGTH_EXT                 (0x00000020) /* (Y) (N) */
#define GAP_FEAT_LL_PRIVACY                         (0x00000040) /* (O) (N) */
#define GAP_FEAT_EXT_SCAN_FILTER_POLICIES           (0x00000080) /* (O) (N) */
#define GAP_FEAT_LE_2M_PHY                          (0x00000100) /* (Y) (N) */
#define GAP_FEAT_TX_STABLE_MOD_INDEX                (0x00000200) /* (Y) (N) */
#define GAP_FEAT_RX_STABLE_MOD_INDEX                (0x00000400) /* (Y) (N) */
#define GAP_FEAT_LE_CODED_PHY                       (0x00000800) /* (Y) (N) */
#define GAP_FEAT_LE_EXT_ADVERTISING                 (0x00001000) /* (O) (N) */
#define GAP_FEAT_LE_PERIODIC_ADVERTISING            (0x00002000) /* (O) (N) */
#define GAP_FEAT_CHANNEL_SEL_ALGO_2                 (0x00004000) /* (Y) (N) */
#define GAP_FEAT_LE_POWER_CLASS_1                   (0x00008000) /* (Y) (N) */
#define GAP_FEAT_MIN_NUM_USED_CHANS_PROC            (0x00010000) /* (Y) (N) */
#define GAP_FEAT_CONN_CTE_REQUEST                   (0x00020000) /* (Y) (N) */
#define GAP_FEAT_CONN_CTE_RESPONSE                  (0x00040000) /* (Y) (N) */
#define GAP_FEAT_CONNLESS_CTE_TX                    (0x00080000) /* (O) (N) */
#define GAP_FEAT_CONNLESS_CTE_RX                    (0x00100000) /* (O) (N) */
#define GAP_FEAT_CTE_TX_ANTENNA_SWITCH_AOD          (0x00200000) /* (O) (N) */
#define GAP_FEAT_CTE_RX_ANTENNA_SWITCH_AOA          (0x00400000) /* (O) (N) */
#define GAP_FEAT_RX_CONSTANT_TONE_EXTS              (0x00800000) /* (Y) (N) */
#define GAP_FEAT_PAST_SENDER                        (0x01000000) /* (Y) (N) */
#define GAP_FEAT_PAST_RECIPIENT                     (0x02000000) /* (Y) (N) */
#define GAP_FEAT_SLEEP_CLOCK_ACC_UPDATES            (0x04000000) /* (Y) (N) */
#define GAP_FEAT_REMOTE_PUB_KEY_VALIDATION          (0x08000000) /* (N) (N) */
#define GAP_FEAT_CIS_CENTRAL                        (0x10000000) /* (Y) (N) */
#define GAP_FEAT_CIS_PERIPHERAL                     (0x20000000) /* (Y) (N) */
#define GAP_FEAT_ISO_BROADCASTER                    (0x40000000) /* (Y) (N) */
#define GAP_FEAT_SYNCED_RECEIVER                    (0x80000000) /* (Y) (N) */
#define GAP_HIGH_FEAT_CIS_HOST_SUPPORT              (0x00000001) /* (Y) (Y) */
#define GAP_HIGH_FEAT_LE_POWER_CTRL_REQ             (0x00000002) /* (Y) (N) */ // shall always have the same value as next bit
#define GAP_HIGH_FEAT_LE_POWER_CTRL_RQS             (0x00000004) /* (Y) (N) */ // shall always have the same value as prev bit
#define GAP_HIGH_FEAT_LE_PATH_LOSS_MONITOR          (0x00000008) /* (Y) (N) */
#define GAP_HIGH_FEAT_PA_ADI_SUPPORT                (0x00000010) /* (O) (N) */
#define GAP_HIGH_FEAT_CONN_SUBRATING                (0x00000020) /* (Y) (N) */
#define GAP_HIGH_FEAT_CONN_SUBRAT_HOST_SUPP         (0x00000040) /* (Y) (Y) */
#define GAP_HIGH_FEAT_CHANNEL_CLASSIFY              (0x00000080) /* (Y) (N) */
#define GAP_HIGH_FEAT_ADV_CODING_SEL                (0x00000100) /* (Y) (N) */
#define GAP_HIGH_FEAT_ADV_CODSEL_HOST_SUPP          (0x00000200) /* (Y) (Y) */
#define GAP_HIGH_FEAT_DECISION_ADV_FILTER           (0x00000400) /* (N) (N) */
#define GAP_HIGH_FEAT_PA_WITH_RSP_ADVTISER          (0x00000800) /* (Y) (N) */
#define GAP_HIGH_FEAT_PA_WITH_RSP_SCANNER           (0x00001000) /* (Y) (N) */
#define GAP_HIGH_FEAT_UNSEG_FRAMED_MODE             (0x00002000) /* (Y) (N) */
#define GAP_HIGH_FEAT_CHANNEL_SOUNDING              (0x00004000) /* (Y) (N) */
#define GAP_HIGH_FEAT_CS_HOST_SUPPORT               (0x00008000) /* (Y) (Y) */
#define GAP_HIGH_FEAT_CS_TONE_QUAL_IND              (0x00010000) /* (N) (N) */
#define GAP_HIGH_FEAT_LL_EXT_FEAT_SET               (0x80000000) /* (Y) (N) */
#define GAP_LL_EXT_FEAT_MONITORING_ADVTISER         (0x00000001) /* (N) (N) */
#define GAP_LL_EXT_FEAT_FRAME_SPACE_UPDATE          (0x00000002) /* (Y) (N) */
#define GAP_HOST_FEAT_BIT_CIS_HOST_SUPPORT          (32)
#define GAP_HOST_FEAT_BIT_CONN_SUBRAT_HOST_SUPP     (38)
#define GAP_HOST_FEAT_BIT_ADV_CODSEL_HOST_SUPP      (41)

#define GAP_DT_FLAGS                                (0x01) // all numerical multi-byte entities and values shall use little-endian byte order
#define GAP_DT_SRVC_UUID_16_INCP_LIST               (0x02)
#define GAP_DT_SRVC_UUID_16_CMPL_LIST               (0x03)
#define GAP_DT_SRVC_UUID_32_INCP_LIST               (0x04)
#define GAP_DT_SRVC_UUID_32_CMPL_LIST               (0x05)
#define GAP_DT_SRVC_UUID_128_INCP_LIST              (0x06)
#define GAP_DT_SRVC_UUID_128_CMPL_LIST              (0x07)
#define GAP_DT_SHORT_LOCAL_NAME                     (0x08)
#define GAP_DT_COMPLETE_LOCAL_NAME                  (0x09)
#define GAP_DT_TX_POWER_LEVEL                       (0x0A)
#define GAP_DT_CLASS_OF_DEVICE                      (0x0D)
#define GAP_DT_SSP_HASH_C                           (0x0E)
#define GAP_DT_SSP_HASH_C_192                       (0x0E)
#define GAP_DT_SSP_RANDOMIZER_R                     (0x0F)
#define GAP_DT_SSP_RANDOMIZER_R_192                 (0x0F)
#define GAP_DT_DEVICE_ID                            (0x10)
#define GAP_DT_SM_TK_VALUE                          (0x10)
#define GAP_DT_SM_OOB_FLAGS                         (0x11)
#define GAP_DT_PERIPERAL_CONN_INTERVAL              (0x12)
#define GAP_DT_SRVC_SOLI_16_UUID_LIST               (0x14)
#define GAP_DT_SRVC_SOLI_128_UUID_LIST              (0x15)
#define GAP_DT_SERVICE_DATA                         (0x16)
#define GAP_DT_SERVICE_DATA_16BIT_UUID              (0x16)
#define GAP_DT_PUBLIC_TARGET_ADDRESS                (0x17)
#define GAP_DT_RANDOM_TARGET_ADDRESS                (0x18)
#define GAP_DT_APPEARANCE                           (0x19)
#define GAP_DT_ADV_INTERVAL                         (0x1A)
#define GAP_DT_LE_DEVICE_ADDRESS                    (0x1B)
#define GAP_DT_LE_ROLE                              (0x1C)
#define GAP_DT_SSP_HASH_C_256                       (0x1D)
#define GAP_DT_SSP_RANDOMIZER_R_256                 (0x1E)
#define GAP_DT_SRVC_SOLI_32_UUID_LIST               (0x1F)
#define GAP_DT_SERVICE_DATA_32BIT_UUID              (0x20)
#define GAP_DT_SERVICE_DATA_128BIT_UUID             (0x21)
#define GAP_DT_LE_SC_CONFIRM_VALUE                  (0x22)
#define GAP_DT_LE_SC_RANDOM_VALUE                   (0x23)
#define GAP_DT_URI                                  (0x24)
#define GAP_DT_INDOOR_POSITIONING                   (0x25)
#define GAP_DT_TRANSPORT_DISCOVERY_DATA             (0x26)
#define GAP_DT_LE_SUPPORTED_FEATURES                (0x27)
#define GAP_DT_CHAN_MAP_UPDATE_IND                  (0x28)
#define GAP_DT_MESH_PB_ADV                          (0x29)
#define GAP_DT_MESH_MESSAGE                         (0x2A)
#define GAP_DT_MESH_BEACON                          (0x2B)
#define GAP_DT_BIG_INFO                             (0x2C)
#define GAP_DT_BROADCAST_CODE                       (0x2D)
#define GAP_DT_RESOLVABLE_SET_ID                    (0x2E)
#define GAP_DT_ADV_INTERVAL_LONG                    (0x2F)
#define GAP_DT_BROADCAST_NAME                       (0x30)
#define GAP_DT_ENCRYPTED_ADV_DATA                   (0x31)
#define GAP_DT_PA_RSP_TIMING_INFO                   (0x32)
#define GAP_DT_3D_INFO_DATA                         (0x3D)
#define GAP_DT_MANUFACTURER_DATA                    (0xFF)

/**
 * The Encrypted Data data type consists of an encrypted payload secured with a pre-shared
 * session key, a pre-shared initializaiton vector, and randomizer. It is authenticated using
 * a message integerity check (MIC). The session key and iv can be shared by GATT Encrypted
 * Data Key Meterial Characteristic. The Encrypted Data data type shall contain Randomizer,
 * Payload, and MIC fields. The Payload field shall contain a sequence of one or more AD
 * structures that are encrypted.
 *
 * The Payload and MIC fields shall be encrypted using the CCM algorithm with following changes:
 *      The packetCounter and directionBit of the CCM nonce shall be set to the Randomizer field.
 *      In the B1 block, octet 2 shall equal 0xEA.
 *
 * LL CCM 13-byte nonce: From MSB to LSB
 * IV[7][6][5][4][3][2][1][0]|[directionBit|packetCounter[4]&7]|packetCounter[3][2][1][0]
 *
 * AD Type 13-byte nonce: From MSB to LSB
 * IV[7][6][5][4][3][2][1][0]|Randomizer[4]Randomizer[3]Randomizer[2]Randomizer[1]Randomizer[0]
 *
 * <<Encrypted Data>> AD Type:
 * [Len|ED Tag|---------------------Encrypted Data------------------------]
 *            |Randomizer 5B|-----------Payload--------------------|MIC 4B]
 *                          [Len|ESL TAG|ESL][Len|LN Tag|Local Name]
 */

#define GAP_ED_TYPE_ADD_AUTH_DATA                   (0xEA)

/**
 * Simultaneous LE and BR/EDR to Same Device Capable (Controller) indicates
 * that the Controller supports simultaneous LE and BR/EDR links to the
 * same remote device. The local Host uses this feature bit to determine
 * whether the Controller has capable of or not for some reason. A remote device does not
 * use this feature bit. This bit requires LE Supported (Controller) feature
 * bit. The Host shall ignore the 'Simultaneous LE and BR/EDR to Same Device
 * Capable (Controller)' bit in the Flags AD type.
 *
 */
#define GAP_FLAGS_LE_NON_DISCOVERABLE_MODE          (0x00)
#define GAP_FLAGS_LE_LIMITED_DISCOVERABLE_MODE      (0x01)
#define GAP_FLAGS_LE_GENERAL_DISCOVERABLE_MODE      (0x02)
#define GAP_FLAGS_BR_EDR_NOT_SUPPORT                (0x04)
#define GAP_FLAGS_SIMU_LE_BREDR_TO_SAME_DEVICE      (0x08)

#define gap_continue_loop                           (true)
#define gap_end_loop                                (false)

#define gap_zero_based_conidx(con_idx)              ((con_idx) & 0xF)
#define gap_zero_based_conidx_to_ble_conidx(conidx) (BLE_DEVICE_ID_BASE + ((conidx) & 0xf))
#define gap_zero_based_ble_conidx_as_hdl(conidx)    (gap_conn_idx_as_hdl(gap_zero_based_conidx_to_ble_conidx(conidx)))

#define SMP_MIN_ENC_KEY_SIZE                        (7)
#define SMP_MAX_ENC_KEY_SIZE                        (16)

#define SMP_AUTH_BONDING_MASK                       (0x03)
#define SMP_AUTH_NON_BONDING                        (0x00)
#define SMP_AUTH_BONDING                            (0x01)
#define SMP_AUTH_MITM_PROTECT                       (0x04)
#define SMP_AUTH_SC_SUPPORT                         (0x08) // LE Secure Connection Pairing support flag
#define SMP_AUTH_KEYPRESS_NOTIFY                    (0x10) // PairingKeypressNotification shall be generated if both side set support
#define SMP_AUTH_CT2_SUPPORT                        (0x20) // support for the h7 function or not for some reason

#define GATT_UUID_LEN_16BITS                        (2)
#define GATT_UUID_LEN_128BITS                       (16)

#define GATT_MAX_SERVER_CCCD                        (36)
#define GATT_MAX_CACHE_SERVICES                     (8)
#define GATT_MAX_CACHE_CHARS                        (64)
#define GATT_MAX_CACHE_128_SERVICES                 (3)
#define GATT_MAX_CACHE_128_CHARS                    (8)

/**
 * ENUMERATIONS
 *
 *
 */
typedef enum
{
    GAP_PHY_LE_UNKNOWN = 0x00,
    GAP_PHY_LE_1M = 0x01,
    GAP_PHY_LE_2M = 0x02,
    GAP_PHY_LE_CODED = 0x03,
} gap_le_phy_t;

typedef enum
{
    GAP_LE_PHY_UNKNOWN = 0x00,
    GAP_LE_PHY_1M = 0x01,
    GAP_LE_PHY_2M = 0x02,
    GAP_LE_PHY_CODED_S8_CODING = 0x03,
    GAP_LE_PHY_CODED_S2_CODING = 0x04,
} gap_le_detail_phy_t;

typedef enum
{
    GAP_PHY_BIT_LE_1M = 0x01,      // bit 0
    GAP_PHY_BIT_LE_2M = 0x02,      // bit 1
    GAP_PHY_BIT_LE_CODED = 0x04,   // bit 2
#if mHDT_LE_SUPPORT
    GAP_PHY_BIT_LE_MHDT = 0x80,   // bit 7
    GAP_PHY_BIT_ALL_MASK = 0x87,
#else
    GAP_PHY_BIT_ALL_MASK = 0x07,
#endif
} gap_phy_bit_t;

typedef enum
{
    GAP_CODED_PHY_NO_PREFER_CODING = 0x00,
    GAP_CODED_PHY_PREFER_S2_CODING = 0x01,
    GAP_CODED_PHY_PREFER_S8_CODING = 0x02,
    GAP_CODED_PHY_REQUIR_S2_CODING = 0x03,
    GAP_CODED_PHY_REQUIR_S8_CODING = 0x04,
} gap_coded_phy_prefer_t;

typedef enum
{
    GAP_SPACING_TYPE_BIT_T_IFS_ACL_CP = 0x01, // bit 0
    GAP_SPACING_TYPE_BIT_T_IFS_ACL_PC = 0x02, // bit 1
    GAP_SPACING_TYPE_BIT_T_MCES = 0x04, // bit 2
    GAP_SPACING_TYPE_BIT_T_IFS_CIS = 0x08, // bit 3
    GAP_SPACING_TYPE_BIT_T_MSS_CIS = 0x10, // bit 4

    GAP_SPACING_TYPE_BIT_MASK = 0x1F,
} gap_spacing_types_bit_e;

typedef enum
{
    SMP_IO_DISPLAY_ONLY        = 0x00,
    SMP_IO_DISPLAY_YES_NO      = 0x01, // 'yes'could be indicated by pressing a button within a certim time limit otherwise 'no' would be assumed
    SMP_IO_KEYBOARD_ONLY       = 0x02,
    SMP_IO_NO_INPUT_NO_OUTPUT  = 0x03,
    SMP_IO_KEYBOARD_DISPLAY    = 0x04,
    SMP_IO_MAX_CPAS,
} smp_io_cap_t;

typedef enum
{
    SMP_METHOD_NONE             = 0x00,
    SMP_JUST_WORKS              = 0x01,
    SMP_PASSKEY_ENTRY           = 0x02,
    SMP_OOB_METHOD              = 0x03,
    SMP_NUMERIC_COMPARE         = 0x04,
    SMP_SECURE_JUST_WORKS       = 0x11,
    SMP_SECURE_PASSKEY_ENTRY    = 0x12,
    SMP_SECURE_OOB_METHOD       = 0x13,
    SMP_SECURE_NUMERIC_COMPARE  = 0x14,
} smp_pairing_method_t;

typedef enum
{
    SMP_KDIST_ENC_KEY  = 0x01, // LTK EDIV Rand (legacy pairing), or br gen LTK from Link Key
    SMP_KDIST_ID_KEY   = 0x02, // IRK IA
    SMP_KDIST_SIGN_KEY = 0x04, // CSRK
    SMP_KDIST_LINK_KEY = 0x08, // le gen Link Key from LTK
} smp_key_dist_t;

/**
 * LE security mode 1 levels:
 *      Level 1 - No security (no authentication and no encryption)
 *      Level 2 - Unauthenticated pairing with encryption
 *      Level 3 - Authenticated pairing with encryption
 *      Level 4 - Authenticated LE Secure Connections pairing with encryption using a 128-bit enc key
 * LE security mode 2 levels:
 *      shall not be used when a connection is operating in mode 1 level 2/3/4
 *      Level 1 - Unauthenticated pairing with data signing
 *      Level 2 - Authenticated pairing with data signing
 * LE security mode 3 levels:
 *      shall be used to broadcast a BIG in an ISO Broadcaster or receive a BIS in a Synced Receiver
 *      Level 1 - No security (no authentication and no encryption)
 *      Level 2 - Use of unauthenticated Broadcast_Code
 *      Level 3 - Use of authenticated Broadcast_Code
 * LE Secure Connecitons Only mode (LE security mode 1 level 4)
 *
 */

typedef enum
{
    GAP_SEC_NO_SECURITY         = 0x01,
    GAP_SEC_UNAUTHENTICATED     = 0x02,
    GAP_SEC_AUTHENTICATED       = 0x03,
    GAP_SEC_SC_AUTHENTICATED    = 0x04,
} gap_link_sec_level_t;

typedef enum
{
    GAP_DATA_SIGN_NO_SECURITY         = 0x00,
    GAP_DATA_SIGN_UNAUTHENTICATED     = 0x01,
    GAP_DATA_SIGN_AUTHENTICATED       = 0x02,
} gap_data_sign_level_t;

typedef enum
{
    GAP_BIG_SEC_NO_SECURITY     = 0x01,
    GAP_BIG_SEC_UNAUTHENTICATED = 0x02,
    GAP_BIG_SEC_AUTHENTICATED   = 0x03,
} gap_big_sec_level_t;

typedef enum
{
    GAP_SAM_JUST_WORKS              = 0x01,
    GAP_SAM_PASSKEY_ENTRY           = 0x02,
    GAP_SAM_OOB_METHOD              = 0x04,
    GAP_SAM_NUMERIC_COMPARE         = 0x08,
    GAP_SAM_SECURE_JUST_WORKS       = 0x10,
    GAP_SAM_SECURE_PASSKEY_ENTRY    = 0x20,
    GAP_SAM_SECURE_OOB_METHOD       = 0x40,
    GAP_SAM_SECURE_NUMERIC_COMPARE  = 0x80,
} gap_sec_accept_method_t;

typedef enum
{
    GAP_USER_NUMERIC_CONFIRM = 1,
    GAP_USER_LTK_REQUEST_CONFIRM,
    GAP_USER_INPUT_6_DIGIT_PASSKEY,
    GAP_USER_DISPLAY_6_DIGIT_PASSKEY,
    GAP_USER_GET_OOB_LEGACY_TK_VALUE,
    GAP_USER_GET_LOCAL_OOB_AUTH_DATA,
    GAP_USER_GET_PEER_OOB_AUTH_DATA,
} gap_user_confirm_type_t;

typedef enum
{
    GAP_LE_LOCAL_TX_POWER_RANGE = 1,
    GAP_LE_ADV_TX_POWER_LEVEL,
    GAP_LE_CONN_LOCAL_TX_POWER,
    GAP_LE_CONN_REMOTE_TX_POWER,
} gap_le_tx_power_type_t;

typedef enum
{
    GAP_RECV_PEER_IRK_IDENTITY = 1,
    GAP_RECV_PEER_CSRK,
    GAP_RECV_DERIVED_BT_LINK_KEY,
} gap_key_dist_type_t;

typedef enum
{
    GAP_CONN_EVENT_OPENED = BT_EVENT_GAP_CONN_EVENT_START,
    GAP_CONN_EVENT_CLOSED,
    GAP_CONN_EVENT_FAILED,
    GAP_CONN_EVENT_CACHE,
    GAP_CONN_EVENT_MTU_EXCHANGED,
    GAP_CONN_EVENT_USER_CONFIRM,
    GAP_CONN_EVENT_ENCRYPTED,
    GAP_CONN_EVENT_RECV_KEY_DIST,
    GAP_CONN_EVENT_UPDATE_REQ,
    GAP_CONN_EVENT_PARAMS_UPDATE,
    GAP_CONN_EVENT_SUBRATE_CHANGE,
    GAP_CONN_EVENT_PHY_UPDATE,
    GAP_CONN_EVENT_TX_POWER_REPORT,
    GAP_CONN_EVENT_RECV_KEY_METERIAL,
    GAP_CONN_EVENT_RECV_SEC_REQUEST,
    GAP_CONN_EVENT_RECV_SMP_REQUIRE,
    GAP_CONN_EVENT_PAIRING_COMPLETE,
    GAP_CONN_EVENT_FRAME_SPACE_UPDATE,
} gap_conn_event_t;

typedef enum
{
    GAP_EVENT_RECV_STACK_READY = BT_EVENT_GAP_GLOBAL_EVENT_START,
    GAP_EVENT_REFRESH_ADVERTISING,
    GAP_EVENT_RECV_TX_POWER_LEVEL,
    GAP_EVENT_RECV_DERIVED_BLE_LTK,
    GAP_EVENT_MONITORED_ADV_LIST_OP,
    GAP_EVENT_DECISION_INSTUCTIONS,
    GAP_EVENT_HOST_RPA_GENERATED,
} gap_global_event_t;

typedef enum
{
    SMP_ERROR_NO_ERROR                  = 0x00,
    SMP_ERROR_PASSKEY_ENTRY_FAILED      = 0x01,
    SMP_ERROR_OOB_NOT_AVAILABLE         = 0x02,
    SMP_ERROR_AUTH_REQUIREMENTS         = 0x03,
    SMP_ERROR_CONFIRM_VALUE_FAILED      = 0x04,
    SMP_ERROR_PAIRING_NOT_SUPPORT       = 0x05,
    SMP_ERROR_ENC_KEY_SIZE              = 0x06,
    SMP_ERROR_CMD_NOT_SUPPORT           = 0x07,
    SMP_ERROR_UNSPECIFIED_REASON        = 0x08,
    SMP_ERROR_REPEATED_ATTEMPTS         = 0x09,
    SMP_ERROR_INVALID_PARAMS            = 0x0a,
    SMP_ERROR_DHKEY_CHECK_FAILED        = 0x0b,
    SMP_ERROR_NUM_COMPARISON_FAILED     = 0x0c,
    SMP_ERROR_BEEDR_PAIRING_IN_PROGRESS = 0x0d,
    SMP_ERROR_CTKD_NOT_ALLOWED          = 0x0e,
    SMP_ERROR_KEY_REJECTED              = 0x0f,
    SMP_ERROR_STATUS_CONTINUE           = 0x80, // local use error code
    SMP_ERROR_STATUS_FINISHED           = 0x81,
    SMP_ERROR_STATUS_FAILED             = 0x82,
    SMP_ERROR_INVALID_SMP_CONN          = 0x83,
    SMP_ERROR_INVALID_RESULT            = 0x84,
    SMP_ERROR_HCI_STATUS_ERROR          = 0x85,
    SMP_ERROR_AES_RESULT_FAILED         = 0x86,
    SMP_ERROR_AES_CMAC_FAILED           = 0x87,
    SMP_ERROR_LE_ENC_CMD_FAILED         = 0x88,
    SMP_ERROR_GEN_TK_RAND_FAILED        = 0x89,
    SMP_ERROR_GEN_RANDOM_FAILED         = 0x8A,
    SMP_ERROR_GEN_CFM_VALUE_FAILED      = 0x8B,
    SMP_ERROR_GEN_STK_FAILED            = 0x8C,
    SMP_ERROR_GEN_PUB_KEY_FAILED        = 0x8D,
    SMP_ERROR_GEN_USER_VALUE_FAILED     = 0x8E,
    SMP_ERROR_GEN_DHKEY_FAILED          = 0x8F,
    SMP_ERROR_GEN_F5_KEY_T_FAILED       = 0x90,
    SMP_ERROR_GEN_MACKEY_FAILED         = 0x91,
    SMP_ERROR_GEN_LTK_FAILED            = 0x92,
    SMP_ERROR_GEN_CHECK_VALUE_FAILED    = 0x93,
    SMP_ERROR_ENABLE_ENC_FAILED         = 0x94,
    SMP_ERROR_GEN_ILK_FAILED            = 0x95,
    SMP_ERROR_GEN_ILTK_FAILED           = 0x96,
    SMP_ERROR_LK_FROM_ILK_FAILED        = 0x97,
    SMP_ERROR_LTK_FROM_ILTK_FAILED      = 0x98,
    SMP_ERROR_SMP_TIMER_TIMEOUT         = 0x99,
    SMP_ERROR_ENCRYPTION_FAILED         = 0x9A,
    SMP_ERROR_GET_OOB_DATA_FAILED       = 0x9B,
    SMP_ERROR_RX_PEER_PAIRING_FAIL      = 0x9C,
    SMP_ERROR_MIC_ERROR                 = 0x9D
} smp_error_code_t;

typedef enum
{
    GAP_AUTH_STARTED_BY_NONE                = 0x00,
    GAP_AUTH_STARTED_BY_BLE_TEST            = 0x01,
    GAP_AUTH_STARTED_BY_GATT_READY          = 0x02,
    GAP_AUTH_STARTED_BY_UPPER_APP           = 0x03,
    GAP_AUTH_STARTED_BY_ADDRESS             = 0x04,
    GAP_AUTH_STARTED_BY_RECV_SEC_LEVELS     = 0x05,
    GAP_AUTH_STARTED_BY_INIT_COMPLETE       = 0x06,
    GAP_AUTH_STARTED_BY_NOTIFY_SEC_CHECK    = 0x07,
    GAP_ATUH_STARTED_BY_RX_RSP_SEC_ERROR    = 0x08,
    GAP_ATUH_STARTED_BY_RX_REQ_SEC_ERROR    = 0x09,
    GAP_AUTH_STARTED_BY_EATT_SEC_FAIL       = 0x0A,
    GAP_AUTH_STARTED_BY_BTHOST_RPC          = 0x0B,
} gap_who_started_auth_t;

typedef enum
{
    GAP_CTE_TYPE_AOA = 0x00,
    GAP_CTE_TYPE_AOA_1US_SLOTS = 0x01,
    GAP_CTE_TYPE_AOA_2US_SLOTS = 0x02,
    GAP_CTE_TYPE_NO_CTE = 0xFF,
} gap_le_cte_type_t;

enum gap_monitored_advertisers_condition
{
    GAP_MONITORED_ADV_COND_BELOW_RSSI = 0,
    GAP_MONITORED_ADV_COND_GREATER_RSSI = 1,
};

typedef enum
{
    GAP_ADV_DATA      = 0x00,
    GAP_ADV_RSP_DATA,
    GAP_ADV_PER_DATA,
    GAP_DECISION_DATA,
    GAP_ADV_UNKNOW_DATA,
} gap_adv_data_type_t;

typedef enum
{
    GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS = 0x00,
    GAP_ADV_ACCEPT_ALL_CONN_REQS_BUT_ONLY_SCAN_REQS_IN_LIST = 0x01,
    GAP_ADV_ACCEPT_ALL_SCAN_REQS_BUT_ONLY_CONN_REQS_IN_LIST = 0x02,
    GAP_ADV_ACCEPT_ALL_CONN_SCAN_REQS_IN_LIST = 0x03,
} gap_adv_policy_t;

typedef enum
{
    GAP_ADV_CHANNEL_UNKNOWN = 0x00,
    GAP_ADV_CHANNEL_37 = 0x01,
    GAP_ADV_CHANNEL_38 = 0x02,
    GAP_ADV_CHANNEL_39 = 0x04,
} gap_adv_channel_t;

typedef enum
{
    GAP_ADV_DATA_INTER_FRAGMENT = 0x00,
    GAP_ADV_DATA_FIRST_FRAGMENT = 0x01,
    GAP_ADV_DATA_LAST_FRAGMENT = 0x02,
    GAP_ADV_DATA_COMPLETE = 0x03,
    GAP_ADV_DATA_UNCHANGED = 0x04, // just update Advertising DID of the adv
} gap_adv_data_op_t;

typedef enum
{
    GAP_ADV_DISABLE_BY_NONE                     = 0x00,
    GAP_ADV_DISABLE_BY_APP_BLE                  = 0x01,
    GAP_ADV_DISABLE_BY_SEND_CMD_FAIL            = 0x02,
    GAP_ADV_DISABLE_BY_SET_PARAM_FAIL           = 0x03,
    GAP_ADV_DISABLE_BY_SEND_ENABLE_FAIL         = 0x04,
    GAP_ADV_DISABLE_BY_ADV_TIMEOUT              = 0x05,
    GAP_ADV_DISABLE_BY_BIG_CLEANUP              = 0x06,
    GAP_ADV_DISABLE_BY_BIG_DISABLE              = 0x07,
    GAP_ADV_DISABLE_ALL_BY_BLE_TEST             = 0x08,
    GAP_ADV_DISABLE_ALL_BY_UPPER_APP            = 0x09,
    GAP_ADV_DISABLE_ALL_BY_RESOLVE              = 0x0A,
    GAP_ADV_DISABLE_ALL_BY_FILTER               = 0x0B,
    GAP_ADV_DISABLE_LOW_PRIORITY_BUT_NOT_FREE   = 0x0C,
    GAP_ADV_DISABLE_OLD_PARAM_BUT_NOT_FREE      = 0x0D,
    GAP_ADV_TIMEOUT_AND_RESTART_NOT_FREE        = 0x0E,
    GAP_ADV_DISABLE_BY_LEGACY_CONN_COMPLETE     = 0x0F,
    GAP_ADV_DISABLE_BY_ADV_SET_TERMINATE        = 0x10,
    GAP_ADV_DISABLE_BY_DISABLE_COMPLETE         = 0x11,
} gap_who_disable_adv_t;

enum gap_adv_event_types_bit
{
    // AdvMode 0b00
    GAP_ADV_EVENT_TYPES_NOCONN_NOSCAN_BIT = 0x01,
    // AdvMode 0b01
    GAP_ADV_EVENT_TYPES_CONN_UNDIRECT_BIT = 0x02,
    // AdvMode 0b10
    GAP_ADV_EVENT_TYPES_SCAN_UNDIRECT_BIT = 0x04,
};

typedef enum
{
    GAP_ADV_EVENT_STARTED = BT_EVENT_GAP_ADV_EVENT_START,
    GAP_ADV_EVENT_STOPPED,
    GAP_ADV_EVENT_SET_DATA,
    GAP_ADV_EVENT_SCAN_REQ,
    GAP_ADV_EVENT_PA_ENABLED,
    GAP_ADV_EVENT_PA_DISABLED,
    GAP_ADV_EVENT_PA_SUBEVENT_DATA_REQ,
    GAP_ADV_EVENT_PA_RESPONSE_REPORT,
    GAP_ADV_EVENT_CONN_OPENED = BT_EVENT_GAP_CONN_EVENT_START,
    GAP_ADV_EVENT_CONN_CLOSED,
    GAP_ADV_EVENT_CONN_FAILED,
    GAP_ADV_EVENT_CONN_CACHE,
    GAP_ADV_EVENT_CONN_MTU_EXCHANGED,
    GAP_ADV_EVENT_USER_CONFIRM,
    GAP_ADV_EVENT_CONN_ENCRYPTED,
    GAP_ADV_EVENT_RECV_KEY_DIST,
    GAP_ADV_EVENT_CONN_UPDATE_REQ,
    GAP_ADV_EVENT_CONN_PARAMS_UPDATE,
    GAP_ADV_EVENT_CONN_SUBRATE_CHANGE,
    GAP_ADV_EVENT_CONN_PHY_UPDATE,
    GAP_ADV_EVENT_TX_POWER_REPORT,
    GAP_ADV_EVENT_RECV_KEY_METERIAL,
    GAP_ADV_EVENT_RECV_SEC_REQUEST,
    GAP_ADV_EVENT_RECV_SMP_REQUIRE,
    GAP_ADV_EVENT_PAIRING_COMPLETE,
    GAP_ADV_EVENT_FRAME_SPACE_UPDATE
} gap_adv_event_t;

enum gap_scanning_filter_policy
{
    GAP_SCAN_FILTER_POLICY_BASIC_UNFILTERED = 0b0000,
    GAP_SCAN_FILTER_POLICY_BASIC_FILTERED   = 0b0001,
    GAP_SCAN_FILTER_POLICY_EXT_UNFILTERED   = 0b0010,
    GAP_SCAN_FILTER_POLICY_EXT_FILTERED     = 0b0011,

    GAP_SCAN_FILTER_POLICY_MASK             = 0b0011,
    // Below are Core spec 6.0 feature
    GAP_SCAN_FILTER_MODE_NO_DECISIONS       = 0b0000,
    GAP_SCAN_FILTER_MODE_ALL_PDU            = 0b0100,
    GAP_SCAN_FILTER_MODE_DECISIONS_ONLY     = 0b1100,

    GAP_SCAN_FILTER_MODE_MASK               = 0b1100,
};

typedef enum
{
    GAP_DUP_FILTER_DISABLE = 0,
    GAP_DUP_FILTER_ENABLED = 1,
    GAP_DUP_FILTER_ENABLED_AND_RESET_ON_EACH_PERIOD = 2,
} gap_dup_filter_op_t;

typedef enum
{
    GAP_SCAN_EVENT_STARTED = BT_EVENT_GAP_SCAN_EVENT_START,
    GAP_SCAN_EVENT_STOPPED,
    GAP_SCAN_EVENT_ADV_REPORT,
    GAP_SCAN_EVENT_MONITORED_ADV_REPORT,
    GAP_SCAN_EVENT_PA_SYNC_ESTABLISHED,
    GAP_SCAN_EVENT_PA_SYNC_TERMINATED,
    GAP_SCAN_EVENT_PA_REPORT,
    GAP_SCAN_EVENT_BIG_INFO_REPORT,
} gap_scan_event_t;

enum gap_initiator_filter_policy
{
    /// No filter no decisions, only peer type and address
    GAP_INIT_FILTER_POLICY_NO_FILTER_NO_DECISIONS   = 0x00,
    /// Use filter but no not process decisions
    GAP_INIT_FILTER_POLICY_USE_FILTER_NO_DECISIONS  = 0x01,
    /// Only process all decisions no filter
    GAP_INIT_FILTER_POLICY_NO_FILTER_ALL_DECISIONS  = 0x02,
    /// Use filter to all pdus
    GAP_INIT_FILTER_POLICY_USE_FILTER_TO_ALL_PDUS   = 0x03,
    /// Process all decisions and other PDUs use filter
    GAP_INIT_FILTER_POLICY_OTHER_FILTER_DECISIONS   = 0x04,

    GAP_INIT_FILTER_POLICY_MASK                     = 0x07,
};

typedef enum
{
    GAP_INIT_EVENT_SCAN_STARTED = BT_EVENT_GAP_SCAN_EVENT_START,
    GAP_INIT_EVENT_SCAN_STOPPED,
    GAP_INIT_EVENT_SCAN_ADV_REPORT,
    GAP_INIT_EVENT_SCAN_MONITORED_ADV_REPORT,
    GAP_INIT_EVENT_INIT_STARTED, // initiating started to start creating connection
    GAP_INIT_EVENT_INIT_STOPPED, // initiating stopped due to conn established or failed
    GAP_INIT_EVENT_CONN_OPENED = BT_EVENT_GAP_CONN_EVENT_START,
    GAP_INIT_EVENT_CONN_CLOSED,
    GAP_INIT_EVENT_CONN_FAILED,
    GAP_INIT_EVENT_CONN_CACHE,
    GAP_INIT_EVENT_CONN_MTU_EXCHANGED,
    GAP_INIT_EVENT_USER_CONFIRM,
    GAP_INIT_EVENT_CONN_ENCRYPTED,
    GAP_INIT_EVENT_RECV_KEY_DIST,
    GAP_INIT_EVENT_CONN_UPDATE_REQ,
    GAP_INIT_EVENT_CONN_PARMS_UPDATE,
    GAP_INIT_EVENT_CONN_SUBRATE_CHANGE,
    GAP_INIT_EVENT_CONN_PHY_UPDATE,
    GAP_INIT_EVENT_TX_POWER_REPORT,
    GAP_INIT_EVENT_RECV_KEY_METERIAL,
    GAP_INIT_EVENT_RECV_SEC_REQUEST,
    GAP_INIT_EVENT_RECV_SMP_REQUIRE,
    GAP_INIT_EVENT_PAIRING_COMPLETE,
    GAP_INIT_EVENT_FRAME_SPACE_UPDATE,
} gap_init_event_t;

enum gap_decision_instuct_flags
{
    GAP_DECISION_I_FLAG_NEVER_PASSES_TEST    = 0b0000,
    GAP_DECISION_I_FLAG_NO_RELEVANT_NO_CHECK = 0b1000,
    GAP_DECISION_I_FLAG_RELEVANT_CHECK_FAIL  = 0b0100,
    GAP_DECISION_I_FLAG_RELEVANT_CHECK_PASS  = 0b0010,

    GAP_DECISION_I_FLAG_MASK  = 0b1110,
};

enum gap_decision_instuct_fields
{
    // Resolvable Tag subfield
    GAP_DECISION_FIELDS_RESOLVABLE_TAG       = 0x00,
    // AdvMode field (which is always present), just check
    GAP_DECISION_FIELDS_ADV_MODE             = 0x06,
    // The relevant field is the RSSI field
    GAP_DECISION_FIELDS_RSSI                 = 0x07,
    // Using TxPower field and the RSSI to calculated this
    GAP_DECISION_FIELDS_PATH_LOSS            = 0x08,
    // AdvA field, which matches addresses
    GAP_DECISION_FIELDS_ADVA                 = 0x09,
    // N = the relevant field is the Arbitrary Data
    // but only if it contains exactly N - 16 octets
    GAP_DECISION_FIELDS_EXACTLY_N_BYTES_MIN  = 0x11,
    GAP_DECISION_FIELDS_EXACTLY_N_BYTES_MAX  = 0x18,
    // but only if it contains at least N - 32 octets
    GAP_DECISION_FIELDS_LEAST_N_BYTES_MIN    = 0x21,
    GAP_DECISION_FIELDS_LEAST_N_BYTES_MAX    = 0x28,
    // but only if it contains [1, N - 48] octets;
    GAP_DECISION_FIELDS_MOST_N_BYTES_MIN     = 0x31,
    GAP_DECISION_FIELDS_MOST_N_BYTES_MAX     = 0x38,
    // 240 to 255 is Vendor-specific
    GAP_DECISION_FIELDS_VENDOR_SPECIFIC_MIN = 0xF0,
    GAP_DECISION_FIELDS_VENDOR_SPECIFIC_MAX = 0xFF,
};

/**
 * TYPEDEFINES
 *
 *
 */
typedef struct
{
    smp_io_cap_t io_cap;
    uint8_t has_oob_data;
    uint8_t auth_req;
    uint8_t max_enc_key_size;
    uint8_t init_key_dist;
    uint8_t resp_key_dist;
} smp_requirements_t;

typedef struct
{
    uint16_t conn_interval_min_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 0xFFFF no specific value
    uint16_t conn_interval_max_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 0xFFFF no specific value
} gap_conn_interval_t;

typedef struct gap_conn_prefer_params_t
{
    uint16_t conn_interval_min_1_25ms; // connection interal = interval * 1.25ms
    uint16_t conn_interval_max_1_25ms; // connection interal = interval * 1.25ms
    uint16_t max_peripheral_latency; // 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
    uint16_t superv_timeout_ms; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
} gap_conn_prefer_params_t;

typedef struct
{
    uint16_t conn_interval_min_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t conn_interval_max_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t max_peripheral_latency; // 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
    uint16_t superv_timeout_ms; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length_slots; // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length_slots; // max len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint8_t l2cap_trans_id;
    bool update_using_l2cap_req;
} gap_update_params_t;

typedef struct
{
    uint16_t subrate_factor_min; // min subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint16_t subrate_factor_max; // max subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint16_t max_peripheral_latency; // 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
    uint16_t conn_continuation_number; // 0x00 to 0x01F3, default 0x00, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
    uint16_t superv_timeout_ms; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
} gap_subrate_params_t;

typedef struct
{
    uint16_t frame_space_min_us; // min subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint16_t frame_space_max_us; // max subrate factor to be applied to the underlying conn interval, 0x01 to 0x01F4
    uint8_t phy_bits; // @see gap_phy_bit_t at least one bit
    uint16_t spacing_types_bits; // @see gap_spacing_types_bit_e at least one bit
} gap_frame_space_param_t;

typedef struct
{
    gap_link_sec_level_t link_sec;
    gap_data_sign_level_t data_sign_sec;
    uint8_t enc_key_size;
    uint8_t accept_method;
} gap_sec_level_t;

typedef struct
{
    uint8_t link_sec_level: 3; // 0 not exist, 1 no security, 2 unauth, 3 auth, 4 sc auth
    uint8_t big_sec_level: 2; // 0 not exist, 1 no security, 2 unauth, 3 auth
    uint8_t data_sign_sec_level: 2; // 0 no security, 1 unauth, 2 auth
    uint8_t data_sign_sec_exist: 1;
} gap_security_levels_t;

typedef struct
{
    uint8_t session_key_le[GAP_KEY_LEN]; // the shared session key
    uint8_t iv_le[GAP_IV_LEN]; // initialization vector, used as the IV for encrypting and authenticating the Encrypted Data
} __attribute__((packed)) gap_key_material_t;

typedef struct
{
    gap_key_material_t data;
    bool is_exist;
} gap_encrypted_data_key_material_t;

typedef struct
{
    uint16_t char_end_handle;
    uint16_t char_value_handle;
    uint16_t char_cccd_handle;
    uint8_t service_seqn: 5;
    uint8_t in_128_service: 1;
    uint8_t reliable_write: 1;
    uint8_t writable_aux: 1;
    uint8_t char_prop;
    uint16_t char_uuid;
} __attribute__((packed)) gatt_char_cache_t;

typedef struct
{
    uint16_t char_end_handle;
    uint16_t char_value_handle;
    uint16_t char_cccd_handle;
    uint8_t service_seqn: 5;
    uint8_t in_128_service: 1;
    uint8_t reliable_write: 1;
    uint8_t writable_aux: 1;
    uint8_t char_prop;
    uint8_t uuid_128_le[GATT_UUID_LEN_128BITS];
} __attribute__((packed)) gatt_128_char_cache_t;

typedef struct
{
    uint16_t start_handle;
    uint16_t end_handle;
    uint16_t service_uuid;
} __attribute__((packed)) gatt_service_cache_t;

typedef struct
{
    uint16_t start_handle;
    uint16_t end_handle;
    uint8_t uuid_128_le[GATT_UUID_LEN_128BITS];
} __attribute__((packed)) gatt_128_service_cache_t;

typedef struct
{
    uint8_t server_cache_is_exist: 1;
    uint8_t service_change_unaware: 1;
    uint8_t gatt_client_supp_robust_caching: 1;
    uint8_t gatt_client_supp_eatt_bearer: 1;
    uint8_t gatt_client_supp_recv_multi_notify: 1;
    uint8_t gatt_server_supp_eatt_bearer: 1;
    uint8_t gatt_server_has_service_changed_char: 1;
    uint8_t gatt_server_has_database_hash_char: 1;
} __attribute__((packed)) gatt_server_cache_t;

typedef struct
{
    uint8_t client_cache_seqn;
    uint8_t peer_database_hash[GAP_KEY_LEN];
    gatt_service_cache_t peer_service[GATT_MAX_CACHE_SERVICES];
    gatt_128_service_cache_t peer_128_service[GATT_MAX_CACHE_128_SERVICES];
    gatt_char_cache_t peer_character[GATT_MAX_CACHE_CHARS];
    gatt_128_char_cache_t peer_128_character[GATT_MAX_CACHE_128_CHARS];
} __attribute__((packed)) gatt_client_cache_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bool trusted;
    bool for_bt_source;
    uint8_t link_key[GAP_KEY_LEN];
    uint8_t key_type;
    uint8_t cod[GAP_COD_LEN];
} gap_bredr_sec_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
    /// Pairing done successfully or prev bonded
    uint8_t device_paired: 1;
    /// Device paired but may be not bonded due to bonding_type is non bonding or LE legacy pairing LTK not distributed
    uint8_t device_bonded: 1;
    uint8_t secure_pairing: 1;
    uint8_t bonded_with_num_compare: 1;
    uint8_t bonded_with_passkey_entry: 1;
    uint8_t bonded_with_oob_method: 1;
    uint8_t peer_irk_distributed: 1;
    uint8_t peer_csrk_distributed: 1;
    uint8_t peer_irk[GAP_KEY_LEN];
    uint8_t peer_csrk[GAP_KEY_LEN];
    uint8_t ltk[GAP_KEY_LEN];
    uint8_t local_ltk[GAP_KEY_LEN];
    uint8_t rand[GAP_RAND_LEN];
    uint8_t local_rand[GAP_RAND_LEN];
    uint8_t ediv[GAP_EDIV_LEN];
    uint8_t local_ediv[GAP_EDIV_LEN];
    uint8_t enc_key_size;
} gap_bond_sec_t;

typedef struct
{
    uint32_t is_opened: 1;
    uint32_t is_central: 1;
    uint32_t conn_ready: 1;
    uint32_t authencated: 1;
    uint32_t encrypted: 1;
    uint32_t directed: 1;
    uint32_t conn_from_rs: 1;
    uint32_t new_link_key_notified: 1;
    uint32_t smp_pairing_ongoing: 1;
    uint32_t smp_encrypt_ongoing: 1;
    uint32_t auth_mitm_protection: 1;
    uint32_t reconn_eatt_after_enc: 1;
    uint32_t att_tx_sec_block: 1;
    uint32_t att_rx_sec_block: 1;
    uint32_t peer_pairing_is_bonding: 1;
    uint32_t local_pairing_is_bonding: 1;
    uint32_t peer_pairing_support_ct2: 1;
    uint32_t local_pairing_support_ct2: 1;
    uint32_t set_use_passkey_entry: 1;
    uint32_t set_use_oob_method: 1;
    uint32_t peer_legacy_ltk_received: 1;
    uint32_t peer_legacy_ediv_rand_received: 1;
    uint32_t peer_irk_received: 1;
    uint32_t peer_ia_received: 1;
    uint32_t peer_bt_supp_eatt_fc_mode: 1;
    uint32_t peer_bt_supp_fixed_smp_cid: 1;
    uint32_t ble_gen_link_key_wait_report: 1;
    uint32_t bredr_gen_ltk_wait_report: 1;
    uint32_t bredr_smp_wait_read_key_size: 1;
    uint32_t peer_rpa_is_resolving: 1;
    uint32_t encrypt_wait_report: 1;
    uint32_t update_conn_param_wait_cfm: 1;
} __attribute__((packed)) gap_conn_flag_t;

typedef struct
{
    uint16_t central_addr_reso_support: 1;
    uint16_t only_use_rpa_after_bonding: 1;
    uint16_t gatt_client_supp_robust_caching: 1;
    uint16_t gatt_client_supp_eatt_bearer: 1;
    uint16_t gatt_client_supp_recv_multi_notify: 1;
    uint16_t gatt_server_supp_eatt_bearer: 1;
    uint16_t gatt_server_has_service_changed_char: 1;
    uint16_t gatt_server_has_database_hash_char: 1;
    uint16_t gatt_service_change_unaware: 1;
    uint32_t le_feat_page_0[GAP_FEAT_PAGE_0_NUM_U32];
    uint32_t le_feat_page_1[GAP_FEAT_PAGE_1_NUM_U32];
    uint8_t service_database_hash[GAP_KEY_LEN];
    struct list_node srvc_list;
} gap_peer_info_t;

/**
 * IRK can be assigned, or randomly generated by the device during manufacturing, or some other method could
 * be used. CSRK is used to sign data in a connection. A device that has received CSRK can verify signatures
 * generated by the distributing device. CSRK can be assigned or randomly generated by the device during
 * manufacturing, or some other method could be used.
 *
 * LE legacy pairing generation of LTK, EDIV and Rand, new values of LTK, EDIV, and Rand shall be generated
 * each time they are distributed. The generated LTK size shall not be longer then the negotiated encryption
 * key size, so its size may need to be shortened.
 *
 */
typedef struct
{
    uint16_t conn_interval_1_25ms; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // num of subrated conn events, 0x00 to 0x1F3 (499)
    uint16_t superv_timeout_ms; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t central_clock_accuracy; // only valid on Peripheral, on Central shall be set to 0x00
    uint16_t subrate_factor; // 0x01 to 0x01F4, subrate factor applied to the specified underlying conn interval
    uint16_t conn_continuation_number; // 0x00 to 0x01F3, num of underlying conn events to remain active after a packet contain a LL PDU with non-zero length is sent or received
} gap_conn_timing_t;

typedef struct
{
    uint16_t conn_interval_min_1_25ms;  // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t conn_interval_max_1_25ms;  // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t max_peripheral_latency;    // 0x00 to 0x01F3, max peripheral latency in units of subrated conn intervals
    uint16_t superv_timeout_ms;         // default 5s, 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint16_t min_ce_length_slots;       // default 0, min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length_slots;       // default 8, max len of connection event, 0x00 to 0xFFFF * 0.625ms
} gap_conn_timing_config_t;

struct gap_conn_item_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
} gap_conn_param_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    uint16_t mtu;
} gap_conn_mtu_exchanged_t;

typedef struct
{
    uint8_t error_code;
    uint8_t role; // 0 central, 1 perpherial
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
} gap_conn_failed_t;

typedef struct
{
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    const gatt_server_cache_t *srv_cache;
    const gatt_client_cache_t *cli_cache;
} gap_conn_cache_ind_t;

typedef struct
{
    /// From hci_ev_le_ltk_request last two params
    uint8_t rand[GAP_RAND_LEN];
    uint8_t ediv[GAP_EDIV_LEN];
} gap_ltk_enc_info_t;

typedef struct
{
    gap_user_confirm_type_t type;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    union
    {
        uint32_t numeric_confirm_value; // 6 digits
        uint32_t display_passkey; // 6 digits
        gap_ltk_enc_info_t ltk_req;
    } data;
} gap_user_confirm_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    gap_update_params_t params_req;
} gap_conn_update_req_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    bool phy_update; // false indicate host read
    gap_le_phy_t tx_phy;
    gap_le_phy_t rx_phy;
} gap_conn_phy_update_t;

typedef struct
{
    gap_le_tx_power_type_t type;
    union
    {
        struct
        {
            int8_t min_tx_power; // -127dBm to 20dBm
            int8_t max_tx_power; // -127dBm to 20dBm
        } range;
        struct
        {
            uint8_t adv_handle;
            int8_t curr_tx_power;
        } adv;
        struct
        {
            uint16_t connhdl;
            gap_le_detail_phy_t phy;
            int8_t curr_tx_power; // -127dBm to 20dBm
            bool is_at_min_level;
            bool is_at_max_level;
            int8_t delta; // 0x7F change is not available or is out of range, 0xXX zero unchanged, positive increased, negative decreased
            int8_t max_tx_power; // -127dBm to 20dBm, or 0x7F GAP_UNKNOWN_TX_POWER
        } local;
        struct
        {
            uint16_t connhdl;
            gap_le_detail_phy_t phy;
            int8_t curr_tx_power; // -127dBm to 20dBm
            bool is_at_min_level;
            bool is_at_max_level;
            int8_t delta; // 0x7F change is not available or is out of range, 0xXX zero unchanged, positive increased, negative decreased
        } remote;
    } u;
} gap_le_tx_power_param_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    gap_le_tx_power_param_t param;
} gap_le_tx_power_report_t;

typedef struct
{
    gap_key_dist_type_t key_type;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    const uint8_t *recv_key;
    bt_addr_type_t recv_ia_type;
    bt_bdaddr_t recv_ia_addr;
    uint8_t enc_key_size;
    bool link_key_generated_but_still_wait_peer_ia;
    struct gap_conn_item_t *conn;
} gap_recv_key_dist_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    const gap_key_material_t *key_material;
} gap_recv_key_material_t;

typedef struct
{
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    uint8_t auth_req;
} gap_recv_sec_request_t;

typedef struct
{
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    bool pairing_req_or_rsp;
    smp_requirements_t requirements;
} gap_recv_smp_requirements_t;

typedef struct
{
    uint8_t err_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
} gap_smp_pairing_cmp_t;

typedef struct
{
    uint8_t error_code;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    struct gap_conn_item_t *conn;
    uint8_t initiator; // @see HCI_LE_FS_UPD_INITIATOR_LOCAL_HOST
    uint16_t frame_space_us; // Range: 0x0000 to 0x2710
    uint8_t phy_bit; // @see gap_phy_bit_t
    uint16_t spacing_types_bit; // @see gap_spacing_types_bit_e
} gap_frame_space_update_t;

typedef union
{
    void *param_ptr;
    gap_conn_param_t *opened;
    gap_conn_param_t *closed;
    gap_conn_failed_t *conn_failed;
    gap_conn_cache_ind_t *conn_cache;
    gap_conn_mtu_exchanged_t *mtu_exchanged;
    gap_user_confirm_t *user_confirm;
    gap_conn_param_t *encrypted;
    gap_recv_key_dist_t *recv_key_dist;
    gap_conn_update_req_t *update_req;
    gap_conn_param_t *params_update;
    gap_conn_param_t *subrate_change;
    gap_conn_phy_update_t *phy_update;
    gap_le_tx_power_report_t *tx_power;
    gap_recv_key_material_t *recv_key_material;
    gap_recv_sec_request_t *recv_security_req;
    gap_recv_smp_requirements_t *smp_requirements;
    gap_smp_pairing_cmp_t *smp_pairing_cmp;
    gap_frame_space_update_t *frame_space_update;
} gap_conn_callback_param_t;

typedef int (*gap_conn_callback_t)(uintptr_t connhdl, gap_conn_event_t event, gap_conn_callback_param_t param);

typedef struct gap_conn_item_t
{
    hci_conn_type_t conn_type;
    uint8_t con_idx;
    uint16_t connhdl;
    bt_bdaddr_t own_addr;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    uint16_t sync_handle; // PAwR central role: sync_handle
    bt_bdaddr_t peer_rpa;
    uint8_t ctkd_enc_key_size;
    uint8_t error_code;
    bt_bdaddr_t own_rpa;
    uint8_t adv_handle; // peripheral role: conn from adv_handle
    gap_conn_flag_t conn_flag;
    gap_sec_level_t sec_level;
    gap_conn_timing_t timing;
    gap_conn_callback_t conn_callback;
    gap_peer_info_t peer;
    gap_bond_sec_t sec;
    void *smp_conn;
    void *bap_callback;
    uint8_t *link_key;
} gap_conn_item_t;

typedef struct
{
    const uint8_t *ltk;
    uint8_t enc_key_size;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    bool ltk_generated_but_still_wait_peer_kdist;
    struct gap_conn_item_t *bredr_conn;
} gap_recv_derived_ltk_t;

typedef struct
{
    uint8_t err_code;
    uint8_t add_item: 1;
    uint8_t remove_item: 1;
    uint8_t clear_item: 1;
    uint8_t enable_monitor: 1;
    uint8_t disable_monitor: 1;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
} gap_adv_monitored_op_t;

typedef struct
{
    uint8_t err_code;
} gap_decision_instruct_t;

typedef struct
{
    const bt_bdaddr_t *p_host_rpa;
    void *priv;
} gap_host_rpa_generated_t;

typedef union
{
    void *param_ptr;
    gap_le_tx_power_param_t *tx_power;
    gap_recv_derived_ltk_t *recv_derived_ltk;
    gap_adv_monitored_op_t *adv_monitored_op;
    gap_decision_instruct_t *decision_instrut;
    gap_host_rpa_generated_t *rpa_generated;
} gap_global_event_param_t;

typedef struct gap_external_function_callbacks
{
    void (*nv_get_local_csrk)(uint8_t *csrk);
    bool (*nv_get_ble_device_by_index)(uint32_t i, gap_bond_sec_t *out);
    bool (*nv_get_ble_device_by_addr)(const gap_conn_item_t *conn, bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr,
                                      gap_bond_sec_t *out);
    void (*nv_add_ble_device)(const gap_conn_item_t *conn, const gap_bond_sec_t *bond);
    void (*smp_get_upper_requirements)(const gap_conn_item_t *conn, smp_requirements_t *p_requirements);
    void (*gap_get_upper_specific_irk_ia)(const gap_conn_item_t *conn, uint8_t *irk, ble_bdaddr_t *ia);
    bool (*nv_get_bt_device_by_addr)(const bt_bdaddr_t *bd_addr, gap_bredr_sec_t *out);
    bool (*gatt_need_start_mtu_exchange)(const gap_conn_item_t *conn);
    bool (*gatt_allow_data_send)(const gap_conn_item_t *conn);
} gap_ext_func_cbs_t;

typedef int (*gap_global_callback_t)(uintptr_t priv, gap_global_event_t event, gap_global_event_param_t param);

typedef bool (*gap_conn_foreach_func_t)(gap_conn_item_t *conn, void *priv);

typedef struct
{
    bt_l2cap_config_t cfg;
    uint8_t address_reso_support: 1;
    uint8_t use_random_identity_address: 1;
} gap_config_t;

typedef struct
{
    uint8_t pkx[GAP_PUB_KEY_LEN];
    uint8_t pky[GAP_PUB_KEY_LEN];
    uint8_t rand[GAP_KEY_LEN];
    uint8_t confirm[GAP_KEY_LEN];
} gap_smp_oob_auth_data_t;

typedef struct
{
    uint8_t length;
    uint8_t ad_type;
} __attribute__((packed)) gap_dt_head_t;

typedef struct
{
    struct pp_buff *ppb;
} gap_dt_buf_t;

typedef struct
{
    // Resolvable Tag present
    bool resolvable_tag_present;
    // Decision Data length
    uint8_t decision_data_len;
    // Decision Data
    uint8_t decision_data[GAP_DECISION_DATA_MAX_LEN];
} gap_decision_t;

typedef struct
{
    uint8_t uuid_le[GATT_UUID_LEN_128BITS];
} __attribute__((packed)) gap_uuid_128_t;

typedef struct
{
    gap_dt_head_t head;
    uint8_t randomizer_le[5];
} __attribute__((packed)) gap_enc_data_t;

typedef void (*gap_key_callback_t)(void *priv, int error_code, const uint8_t *key);
typedef void (*gap_dt_encrypt_callback_t)(void *priv, int error_code, const gap_enc_data_t *encrypted_data);

typedef struct
{
    uint16_t le_limited_discoverable_mode: 1;
    uint16_t le_general_discoverable_mode: 1;
    uint16_t simu_bredr_support: 1;
    uint16_t srvc_16_uuid_cmpl: 1;
    uint16_t srvc_128_uuid_cmpl: 1;
    uint16_t device_name_cmpl: 1;
    uint16_t has_le_features: 1;
    uint16_t has_manufacturer_data: 1;
    uint16_t has_tx_power_level: 1;
    uint16_t has_periph_prefer_conn_interval: 1;
    uint16_t has_le_role: 1;
    uint16_t has_le_supp_features: 1;
} gap_dt_flags_t;

typedef struct
{
    uint8_t has_oob_flags: 1;
    uint8_t oob_data_present: 1;
    uint8_t le_host_support: 1;
    uint8_t is_random_address: 1;
} gap_dt_oob_flags_t;

typedef struct
{
    gap_dt_flags_t flags;
    uint16_t device_apperance; // valid when non-zero
    uint32_t adv_interval_slots; // >= 40.96s <<advertising interval - long>> data type shall be used
    uint8_t le_role; // 0x00 periph, 0x01 central, 0x02 both and periph preferred for conn, 0x03 both and central preferred for conn
    uint8_t num_srvc_16_uuids;
    uint8_t num_srvc_128_uuids;
    uint8_t num_public_target_addr;
    uint8_t num_random_target_addr;
    uint8_t device_name_len;
    uint8_t big_info_data_len;
    int8_t tx_power_level;
    bt_addr_type_t le_addr_type;
    gap_dt_oob_flags_t smp_oob_flags;
    uint16_t manufacturer_company_id;
    const uint8_t *manufacturer_data;
    const uint16_t *srvc_16_uuid_le;
    const gap_uuid_128_t *srvc_128_uuid;
    const uint8_t *device_name;
    uint32_t le_feat_page_0[GAP_FEAT_PAGE_0_NUM_U32];
    gap_conn_interval_t periph_prefer_conn_interval; // valid when interval_min is not 0xffff
    bt_bdaddr_t *le_bdaddr;
    bt_bdaddr_t *public_target_addr;
    bt_bdaddr_t *random_target_addr;
    const uint8_t *chm_indication;
    const uint8_t *ascs_srvc_data;
    // Broadcast Audio Announcement: Broadcast ID
    const uint8_t *broadcast_id;
    // Basic Audio Announcement: BASE
    struct basic_audio_annoucement
    {
        uint8_t length;
        const uint8_t *base;
    } base_info;
    // Public Broadcast Announcement: PBA
    struct public_broadcast_annoucement
    {
        uint8_t length;
        const uint8_t *pba;
    } pba_info;
    // Broadcast Name
    struct broadcast_name
    {
        uint8_t length;
        const uint8_t *bc_name;
    } broadcast_name;
    const uint8_t *smp_tk;
    const uint8_t *big_info;
    const uint8_t *broadcast_code;
    const uint8_t *rsi; // resolvable set identifier - 6 bytes
} gap_dt_parsed_t;

typedef struct
{
    uint16_t extended_adv: 1;
    uint16_t legacy_pdu: 1;
    uint16_t connectable: 1;
    uint16_t scannable: 1;
    uint16_t directed: 1;
    uint16_t scan_rsp: 1;
    uint16_t cmpl_adv_data: 1;
    uint16_t incmpl_adv_data: 1;
    uint16_t has_more_data: 1;
    uint16_t peer_anonymous: 1;
    uint16_t direct_unresolv_addr: 1;
    uint16_t primary_phy: 2;
    uint16_t secondary_phy: 2;
} gap_adv_flags_t;

typedef struct
{
    gap_adv_flags_t adv;
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
    bt_addr_type_t direct_addr_type;
    bt_bdaddr_t direct_addr; // TargetA or Public IA or Random IA
    const uint8_t *data;
    uint8_t data_length;
    uint8_t adv_set_id;
    uint16_t sync_handle;
    uint16_t pa_interval;
    int8_t tx_power;
    int8_t rssi;
    uint16_t pa_event_counter;
    uint8_t subevent;
    uint8_t response_slot; // 0x00 to 0xFF, the response slot the data was received in
    bool aux_sync_subevent_ind_not_transmitted;
    bool fail_rx_aux_sync_subevent_rsp;
    bool fail_rx_aux_sync_subevent_ind;
    gap_le_cte_type_t cte_type; // 0x00 to 0x02, 0xFF no cte
    gap_coded_phy_prefer_t primary_coding;
    gap_coded_phy_prefer_t secondary_coding;
    gap_dt_parsed_t parsed;
} gap_adv_report_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
    uint8_t user: 7;
    uint8_t is_inuse: 1;
} gap_filter_item_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
    uint8_t device_privacy_mode: 1;
    uint8_t is_inuse: 1;
    uint8_t peer_irk[GAP_KEY_LEN];
} gap_resolv_item_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
    /// @see enum gap_monitored_advertisers_condition
    uint8_t condition;
} gap_monitored_report_t;

typedef struct
{
    bt_bdaddr_t peer_addr;
    bt_addr_type_t peer_type;
} gap_monitored_adv_t;

typedef struct
{
    uint8_t adv_handle;
    bt_status_t error_code;
} gap_adv_started_t;

typedef struct
{
    uint8_t adv_handle;
    bt_status_t error_code;
} gap_adv_stopped_t;

typedef struct
{
    uint8_t adv_handle;
    uint8_t data_type;
    bt_status_t error_code;
} gap_adv_set_data_t;

typedef struct
{
    uint8_t adv_handle;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
} gap_adv_scan_req_t;

typedef struct
{
    uint8_t adv_handle;
    bt_status_t error_code;
} gap_pa_enabled_t;

typedef struct
{
    uint8_t adv_handle;
    bt_status_t error_code;
} gap_pa_disabled_t;

typedef struct
{
    uint8_t adv_handle;
    uint8_t subevent_start; // 0x00 to 0x7F, the first subevent that data is requested for
    uint8_t subevent_data_count; // 0x01 to 0x80, num of subevents that data is requested for
} gap_pa_subevent_data_req_t;

typedef gap_adv_report_t gap_pa_response_report_t;

typedef union
{
    void *param_ptr;
    gap_adv_started_t *adv_started;
    gap_adv_stopped_t *adv_stopped;
    gap_adv_set_data_t *adv_set_data;
    gap_adv_scan_req_t *scan_req;
    gap_pa_enabled_t *pa_enabled;
    gap_pa_disabled_t *pa_disable;
    gap_pa_subevent_data_req_t *pa_subevent_data_req;
    gap_pa_response_report_t *pa_response_report;
    gap_conn_param_t *conn_opened;
    gap_conn_param_t *conn_closed;
    gap_conn_failed_t *conn_failed;
    gap_conn_cache_ind_t *conn_cache;
    gap_conn_mtu_exchanged_t *mtu_exchanged;
    gap_user_confirm_t *user_confirm;
    gap_conn_param_t *conn_encrypted;
    gap_recv_key_dist_t *recv_key_dist;
    gap_conn_update_req_t *conn_update_req;
    gap_conn_param_t *conn_params_update;
    gap_conn_param_t *conn_subrate_change;
    gap_conn_phy_update_t *conn_phy_update;
    gap_le_tx_power_report_t *tx_power;
    gap_recv_key_material_t *recv_key_material;
    gap_recv_sec_request_t *recv_security_req;
    gap_recv_smp_requirements_t *smp_requirements;
    gap_smp_pairing_cmp_t *smp_pairing_cmp;
    gap_frame_space_update_t *frame_space_update;
} gap_adv_callback_param_t;

typedef int (*gap_adv_callback_t)(uintptr_t adv, gap_adv_event_t event, gap_adv_callback_param_t param);

typedef struct
{
    /**
     * Recommended advertising interval for user initiated connectable modes:
     *      connectable undirected mode
     *      limited discoverable and connectable undirected mode
     *      general discoverable and connectable undirected mode
     *      connectable directed low duty cycle mode
     */
    uint32_t min_adv_fast_interval_ms; // LE 1M PHY
    uint32_t max_adv_fast_interval_ms; // LE 1M PHY
    uint32_t min_adv_fast_interval_coded_ms; // LE Coded PHY
    uint32_t max_adv_fast_interval_coded_ms; // LE Coded PHY
    /**
     * Recommended advertising interval for user initiated non-connectable modes:
     *      non-discoverable mode
     *      non-connectable mode
     *      limited discoverable mode
     *      general discoverable mode
     * Note: When advertising interval values of less than 100ms are used for
     * non-connectable or scannable undirected advertising in environments where
     * the advertiser can interfere with other devices, it is recommended that
     * steps be taken to minimize the interference. For example, the advertising
     * might be alternately enabled for only a few seconds and disabled for
     * several minutes.
     */
    uint32_t min_adv_slow_interval_ms; // LE 1M PHY
    uint32_t max_adv_slow_interval_ms; // LE 1M PHY
    uint32_t min_adv_slow_interval_coded_ms; // LE Coded PHY
    uint32_t max_adv_slow_interval_coded_ms; // LE Coded PHY
    /**
     * Recommended advertising interval for background advertising other then Directed Connectable High Duty Mode:
     */
    uint32_t min_adv_bg_interval_ms; // LE 1M PHY
    uint32_t max_adv_bg_interval_ms; // LE 1M PHY
    uint32_t min_adv_bg_interval_coded_ms; // LE Coded PHY
    uint32_t max_adv_bg_interval_coded_ms; // LE Coded PHY
} gap_adv_timing_t;

typedef struct
{
    bool force_start;
    bool connectable;
    bool scannable;
    bool directed_adv;
    bool anonymous_adv;
    bool high_duty_directed_adv; // <= 3.75ms adv interval
    bool use_legacy_pdu;
    bool include_tx_power_data; // include TxPower in the ext header of at least one adv pdu
    bool use_decisions_pdu; // Use decision PDUs when advertising
    bool include_adva_in_decisions; // Include AdvA in the extended header of all decision PDUs
    bool include_adi_in_decisions; // Include ADI in the extended header of all decision PDUs
    bool own_addr_use_rpa;
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    gap_adv_policy_t policy;
    bool use_custom_local_addr;
    bool use_fake_btc_rpa_when_no_irk_exist;
    bt_bdaddr_t custom_local_addr;
    bool limited_discoverable_mode;
    bool fast_advertising;
    bool continue_advertising;
    bool start_bg_advertising;
    bool has_prefer_adv_tx_power;
    bool has_custom_adv_timing;
    bool enable_scan_req_notify;
    int8_t adv_tx_power;
    uint8_t activity_priority;
    uint8_t adv_channel_map;
    uint8_t secondary_adv_max_skip;
    gap_le_phy_t primary_adv_phy;
    gap_coded_phy_prefer_t primary_coded_phy_prefer;
    gap_le_phy_t secondary_adv_phy;
    gap_coded_phy_prefer_t secondary_coded_phy_prefer;
    gap_adv_timing_t adv_timing;
    gap_dt_buf_t adv_data;
    gap_dt_buf_t scan_rsp_data;
    gap_decision_t decision_data;
    uint32_t duration_ms;
    uint8_t max_ext_adv_evts;
} gap_adv_param_t;

typedef struct
{
    uint32_t adv_interval_min;
    uint32_t adv_interval_max;
    uint16_t adv_event_props;
    uint8_t own_addr_type;
    uint8_t peer_type;
    uint8_t adv_channel_map;
    uint8_t adv_filter_policy;
    bt_bdaddr_t local_addr;
    bt_bdaddr_t peer_addr;
    int8_t adv_tx_power;
    uint8_t primary_adv_phy;
    uint8_t primary_coded_phy_prefer;
    uint8_t secondary_adv_phy;
    uint8_t secondary_coded_phy_prefer;
    uint8_t secondary_adv_max_skip;
    uint8_t scan_req_notify_enable;
} gap_adv_set_param_t;

typedef struct
{
    bool adv_params_set;
    bool adv_enabled;
    uint8_t adv_set_id;
    uint8_t adv_handle;
    bool legacy_advertising;
    int8_t selected_tx_power;
    bool fast_advertising;
    bool continue_advertising;
    bool start_bg_advertising;
    bool directed_advertising;
    uint8_t activity_priority;
    uint8_t adv_stop_timer;
    bool pa_is_enabled;
    uint32_t duration_ms;
    gap_dt_buf_t adv_data;
    gap_dt_buf_t scan_rsp_data;
    // Keep it until remove adv set
    gap_decision_t decision_data;
    gap_adv_callback_t adv_callback;
    gap_adv_set_param_t set_param;
    gap_adv_timing_t timing_cfg;
} gap_advertising_t;

typedef struct
{
    bt_status_t error_code;
} gap_scan_started_t;

typedef struct
{
    bt_status_t error_code;
} gap_scan_stopped_t;

typedef struct
{
    uint16_t sync_handle; // 0x0000 to 0x0EFE, sync_handle identifying the pa train
    uint8_t num_bis; // 0x01 to 0x1F, value of the Num_BIS subfield of the BIGInfo field
    uint8_t nse; // 0x01 to 0x1F, value of the NSE subfield of the BIGInfo field
    uint16_t iso_interval; // iso_interval subfield of the BIGInfo field
    uint8_t bn; // 0x01 to 0x07, BN subfield of the BIGInfo field
    uint8_t pto; // 0x00 to 0x0F, pre-transmission offset, PTO subfield of the BIGInfo field
    uint8_t irc; // 0x01 to 0x0F, IRC subfield of the BIGInfo field
    uint16_t max_pdu; // 0x01 to 0xFB, max_pdu subfiled of the BIGInfo
    uint32_t sdu_interval; // sdu_interval subfield of the BIGInfo field
    uint16_t max_sdu; // 0x01 to 0x0FFF, max_sdu subfield of the BIGInfo
    gap_le_phy_t big_phy; // 0x01 the BIG is transmitted on the LE 1M PHY, 0x02 LE 2M, 0x03 LE Coded
    uint8_t framed; // 0x00 unframed 0x01 framed
    uint8_t encrypted; // 0x00 BIG carries unencrypted data, 0x01 BIG carries encrypted data
} gap_big_info_adv_report_t;

typedef struct gap_pa_sync_establish_t gap_pa_sync_establish_t;
typedef struct gap_pa_sync_terminated_t gap_pa_sync_terminated_t;

typedef union
{
    void *param_ptr;
    gap_scan_started_t *scan_started;
    gap_scan_stopped_t *scan_stopped;
    const gap_adv_report_t *adv_report;
    const gap_pa_sync_establish_t *pa_sync_estb;
    const gap_pa_sync_terminated_t *pa_sync_term;
    const gap_adv_report_t *per_adv_report;
    const gap_big_info_adv_report_t *big_info_report;
    const gap_monitored_report_t *adv_monitored_report;
} gap_scan_callback_param_t;

typedef int (*gap_scan_callback_t)(uintptr_t scan, gap_scan_event_t event, gap_scan_callback_param_t param);

typedef struct
{
    // user initiated discovery or connection establishment
    uint16_t fg_scan_interval_ms;               // default 30ms~60ms    LE 1M PHY
    uint16_t fg_scan_window_ms;                 // default 30ms         LE 1M PHY
    uint16_t fg_scan_interval_coded_ms;         // default 90ms~180ms   LE Coded PHY
    uint16_t fg_scan_window_coded_ms;           // default 90ms         LE Coded PHY
    uint16_t fg_slow_scan_interval_ms;          // default 30ms~60ms    LE 1M PHY
    uint16_t fg_slow_scan_window_ms;            // default 30ms         LE 1M PHY
    uint16_t fg_slow_scan_interval_coded_ms;    // default 90ms~180ms   LE Coded PHY
    uint16_t fg_slow_scan_window_coded_ms;      // default 90ms         LE Coded PHY
    uint32_t fg_scan_time_ms;                   // default 30.72s
    // background discovery or connection establishment
    uint16_t bg_scan_interval_ms;               // default 1.28s        LE 1M PHY
    uint16_t bg_scan_window_ms;                 // default 11.25ms      LE 1M PHY
    uint16_t bg_scan_interval_coded_ms;         // default 3.84s        LE Coded PHY
    uint16_t bg_scan_window_coded_ms;           // default 33.75ms      LE Coded PHY
    // slow discovery or connection establishment
    uint16_t bg_slow_scan_interval_ms;          // default 2.56s        LE 1M PHY
    uint16_t bg_slow_scan_window_ms;            // default 22.5ms       LE 1M PHY
    uint16_t bg_slow_scan_interval_coded_ms;    // default 7.68s        LE Coded PHY
    uint16_t bg_slow_scan_window_coded_ms;      // default 67.5ms       LE Coded PHY
} gap_scan_timing_t;

typedef struct
{
    bt_addr_type_t own_addr_type;
    /// @see gap_scanning_filter_policy
    uint8_t filter_policy;
    bool active_scan;
    bool filter_duplicated;
    bool slow_scan;
    bool continue_scanning;
    bool has_custom_scan_timing;
    bool dont_auto_start;
    bool legacy;
    uint8_t phys;
    gap_scan_timing_t scan_timing;
    uint32_t duration_ms; // 0x01 to 0xFFFF * 10ms, 10ms to 655.35s (655350ms)
} gap_scan_param_t;

typedef struct
{
    bt_status_t error_code;
} gap_init_started_t;

typedef struct
{
    bt_status_t error_code;
} gap_init_stopped_t;

typedef union
{
    void *param_ptr;
    gap_scan_started_t *scan_started;
    gap_scan_stopped_t *scan_stopped;
    const gap_adv_report_t *adv_report;
    gap_init_started_t *init_started;
    gap_init_stopped_t *init_stopped;
    gap_conn_param_t *conn_opened;
    gap_conn_param_t *conn_closed;
    gap_conn_failed_t *conn_failed;
    gap_conn_cache_ind_t *conn_cache;
    gap_conn_mtu_exchanged_t *mtu_exchanged;
    gap_user_confirm_t *user_confirm;
    gap_conn_param_t *conn_encrypted;
    gap_recv_key_dist_t *recv_key_dist;
    gap_conn_update_req_t *conn_update_req;
    gap_conn_param_t *conn_params_update;
    gap_conn_param_t *conn_subrate_change;
    gap_conn_phy_update_t *conn_phy_update;
    gap_le_tx_power_report_t *tx_power;
    gap_recv_key_material_t *recv_key_material;
    gap_recv_sec_request_t *recv_security_req;
    gap_recv_smp_requirements_t *smp_requirements;
    gap_smp_pairing_cmp_t *smp_pairing_cmp;
    gap_frame_space_update_t *frame_space_update;
} gap_init_callback_param_t;

typedef int (*gap_init_callback_t)(uintptr_t init, gap_init_event_t event, gap_init_callback_param_t param);

typedef struct
{
    uint16_t min_initial_conn_interval_1_25ms;  // default 30~50ms  LE 1M PHY
    uint16_t max_initial_conn_interval_1_25ms;  // default 30~50ms  LE 1M PHY
    uint16_t min_initial_conn_interval_coded;   // default 90~150ms LE Coded PHY
    uint16_t max_initial_conn_interval_coded;   // default 90~150ms LE Coded PHY
    uint16_t initial_superv_timeout_ms;         // 100ms to 32s
    uint16_t initial_min_ce_length_slots;       // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t initial_max_ce_length_slots;       // max len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint32_t init_proc_timeout_ms;              // default 30.72s, bap 30s
} gap_init_timing_t;

typedef struct
{
    bt_addr_type_t own_addr_type;
    bt_addr_type_t peer_type;
    bt_bdaddr_t peer_addr;
    /// @see gap_initiator_filter_policy
    uint8_t filter_policy;
    bool continue_initiating;
    uint8_t initiating_phys;
    bool has_custom_scan_timing;
    bool has_custom_init_timing;
    gap_scan_timing_t scan_timing; // initiator use scan_timing to do initiating scan
    gap_init_timing_t init_timing; // init_timing is the initial ble connection parameers
} gap_init_param_t;

typedef struct
{
    // Use mask to filter arbitary data
    uint8_t arbitary_mask[GAP_DECISION_DATA_MAX_LEN];
    // Match target pass check
    uint8_t arbitary_target[GAP_DECISION_DATA_MAX_LEN];
} gap_decision_arbitary_check_t;

typedef struct
{
    // Min RSSI in dbm
    int8_t rssi_min_dbm;
    // Max RSSI in dbm
    int8_t rssi_max_dbm;
    // octets 2 - 15 RFU
} gap_decision_rssi_check_t;

typedef struct
{
    // Check mode
    uint8_t check;
    // 0 for Public 1 for random
    bt_addr_type_t addr_type_1;
    // Address 1
    bt_bdaddr_t address_1;
    // 0 for Public 1 for random
    bt_addr_type_t addr_type_2;
    // Address 2
    bt_bdaddr_t address_2;
    // octets 15 RFU
} gap_decision_adva_check_t;

typedef struct
{
    // Min Path Loss in dbm
    uint8_t loss_min_db;
    // Max Path Loss in dbm
    uint8_t loss_max_db;
    // octets 2 - 15 RFU
} gap_decision_passloss_check_t;

typedef struct
{
    // Event type of adv mode @see gap_adv_event_types_bit
    uint8_t event_type;
    // octets 1- 15 RFU
} gap_decision_advmode_check_t;

typedef struct
{
    // Start of group tests
    bool test_grp_start;
    // Test Flags @see gap_decision_instuct_flags
    uint8_t test_flags;
    // Test Field
    uint8_t test_fields;
    // Test_Parameters
    uint8_t test_params[GAP_KEY_LEN];
} gap_decision_test_t;

typedef struct
{
    uint8_t num_subevents; // 0x00 to 0x80
    uint8_t subevent_interval_1_25ms; // 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0xXX 0x01 to 0xFE * 1.25ms (1.25ms to 317.5ms) time between the adv packet in a subevent and the 1st response slot
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0xXX 0x02 to 0xFF * 0x125ms (0x25ms to 31.875ms) time between response slots
    uint8_t num_response_slots; // 0x00 no response slots, 0xXX 0x01 to 0xFF num of subevent response slots
} gap_pa_subevent_param_t;

typedef struct
{
    uint8_t subevent; // 0x00 to 0x7F subevent index
    uint8_t response_slot_start; // the first response slot to be used in the subevent
    uint8_t response_slot_count; // num of response slots to be used
    uint8_t subevent_data_len; // 0 to 251
    const uint8_t *subevent_data; // ad type formatted subevent data
} gap_pa_subevent_data_t;

typedef struct gap_pa_sync_establish_t
{
    bt_status_t error_code;
    uint16_t pa_sync_hdl;
    bool is_over_past;
    uint16_t acl_conn_hdl;
    uint16_t service_data;
    uint8_t adv_sid;
    uint8_t adv_addr_type; // 0x00 public 0x01 random 0x02 public identity 0x03 random static identity
    bt_bdaddr_t adv_addr;
    uint8_t adv_phy; // 0x01 le 1m, 0x02 le 2m, 0x03 le coded
    uint16_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
    uint8_t num_subevents; // 0x00 no subevents, 0x01 to 0x80 num of subevents
    uint8_t subevent_interval_1_25ms; // 0x00 no subevents, 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0x01 to 0xFE * 1.25ms (1.25ms to 317.6ms)
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0x02 to 0xFF * 0.125ms (0.25ms to 31.875ms)
} gap_pa_sync_establish_t;

typedef struct gap_pa_sync_terminated_t
{
    uint16_t pa_sync_hdl;
    uint8_t reason;
} gap_pa_sync_terminated_t;

typedef gap_scan_event_t gap_pa_sync_event_t;
typedef gap_scan_callback_param_t gap_pa_sync_callback_param_t;
typedef gap_scan_callback_t gap_pa_sync_callback_t;

typedef struct
{
    uint8_t options;
    uint8_t adv_set_id;
    bt_addr_type_t adv_addr_type;
    bt_bdaddr_t adv_addr;
    uint16_t skip;
    uint16_t sync_timeout_10ms;
    uint8_t sync_cte_type;
} gap_pa_sync_param_t;

typedef struct
{
    uint8_t response_subevent; // 0x00 to 0x7F identify the subevent of PAwR train to response
    uint8_t response_slot; // 0x00 to 0xFF identify the response slot of the PAwR train
    uint8_t response_data_len; // 0 to 251
    const uint8_t *response_data; // ad type formatted response data
} gap_pa_response_data_t;

typedef struct
{
    gap_dt_encrypt_callback_t func;
    void *priv;
} gap_enc_data_env_t;

typedef struct
{
    bt_bdaddr_t local_rpa;
    void (*cmpl)(void *priv, const bt_bdaddr_t *local_rpa);
    void *priv;
} gap_gen_rpa_t;

typedef struct
{
    gap_bond_sec_t record;
    bt_bdaddr_t peer_rpa;
    uint8_t record_idx;
    void (*cmpl)(void *priv, const gap_bond_sec_t *out);
    void *priv;
} gap_resolve_rpa_t;

typedef struct
{
    bool scan_enabled;
    bool legacy_scanning;
    bool background_scan;
    bool initiating_scan;
    bt_addr_type_t own_addr_type;
    uint8_t scan_handle;
    uint8_t scan_stop_timer;
    uint16_t period_1_28s;
    uint32_t duration_ms;
    uint16_t scan_interval_slots;
    uint16_t scan_interval_coded;
    uint16_t scan_window_slots;
    uint16_t scan_window_coded;
    gap_scan_param_t param;
    gap_scan_callback_t scan_callback;
} gap_scanning_t;

typedef struct
{
    bool legacy_initiating;
    bool is_conn_creating;
    bool is_direct_initiating;
    uint8_t peer_type;
    bt_addr_type_t own_addr_type;
    bt_bdaddr_t peer_addr;
    uint32_t init_proc_timeout_ms;
    bool timeout_and_restart_initiating;
    bool start_bg_initiating;
    bool peer_is_ble_audio;
    uint8_t init_handle;
    uint8_t init_stop_timer;
    uint8_t pa_adv_handle;
    uint8_t sd_subevent;
    uint16_t scan_interval_slots;
    uint16_t scan_interval_coded;
    uint16_t scan_window_slots;
    uint16_t scan_window_coded;
    uint16_t min_conn_interval_1_25ms;  // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t max_conn_interval_1_25ms;  // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t min_conn_interval_coded;   // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t max_conn_interval_coded;   // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4s
    uint16_t superv_timeout_ms;         // 100ms to 32s
    uint16_t min_ce_length_slots;       // min len of connection event, 0x00 to 0xFFFF * 0.625ms
    uint16_t max_ce_length_slots;       // max len of connection event, 0x00 to 0xFFFF * 0.625ms
    gap_init_callback_t init_callback;
    gap_init_param_t init_param;
} gap_initiating_t;

typedef struct
{
    gap_advertising_t *adv;
} gap_advertising_item_t;

typedef struct
{
    bool phy_update; // false indicate host read
    gap_le_phy_t tx_phy;
    gap_le_phy_t rx_phy;
} gap_conn_phy_param_t;

typedef struct
{
    /// PA sync handle
    uint16_t pa_sync_hdl;
    /// Periodic Adv Timing
    uint8_t adv_phy; // 0x01 le 1m, 0x02 le 2m, 0x03 le coded
    uint16_t pa_interval_1_25ms; // 0x06 to 0xFFFF * 1.25ms (7.5ms to 81.9s)
    uint8_t adv_clock_accuracy; // 0x00 to 0x07 500ppm to 20ppm
    uint8_t num_subevents; // 0x00 no subevents, 0x01 to 0x80 num of subevents
    uint8_t subevent_interval_1_25ms; // 0x00 no subevents, 0x06 to 0xFF * 1.25ms (7.5ms to 318.75ms)
    uint8_t response_slot_delay_1_25ms; // 0x00 no response slots, 0x01 to 0xFE * 1.25ms (1.25ms to 317.6ms)
    uint8_t response_slot_spacing_0_125ms; // 0x00 no response slots, 0x02 to 0xFF * 0.125ms (0.25ms to 31.875ms)
    /// Sync param from upper
    gap_pa_sync_param_t pa_sync_param;
    gap_pa_sync_callback_t pa_sync_callback;
} gap_pa_sync_t;

typedef struct
{
    gap_pa_sync_t *pa_sync;
} gap_pa_sync_item_t;

typedef struct
{
    uint32_t adv_seqn;
    uint8_t adv_handle;
    uint8_t own_addr_type: 2;
    uint8_t directed_advertising: 1;
    bt_bdaddr_t local_addr;
    gap_adv_callback_t adv_callback;
} __attribute__((packed)) gap_adv_param_for_conn_t;

typedef struct
{
    uint8_t status;
    uint16_t conn_handle;
    uint8_t role; // 0x00 Central, 0x01 Peripheral
    uint8_t peer_addr_type; // 0x00 public, 0x01 random, 0x02 public ia, 0x03 random ia
    bt_bdaddr_t peer_addr;
    uint8_t local_rpa[GAP_ADDR_LEN]; // only valid own_addr_type 0x02 0x03 and non-zero local IRK, otherwise shall return all zeros
    uint8_t peer_rpa[GAP_ADDR_LEN]; // only valid when peer_addr_type is 0x02 or 0x03, for other peer_addr_type shall set all zeros
    uint16_t conn_interval; // 0x06 to 0x0C80 * 1.25ms, 7.5ms to 4000ms
    uint16_t peripheral_latency; // peripheral latency in num of subrated conn events, 0x00 to 0x01F3 (499)
    uint16_t superv_timeout; // 0x0A to 0x0C80 * 10ms, 100ms to 32s
    uint8_t central_clock_accuracy; // 0x00 500, 0x01 250, 0x02 150, 0x03 100, 0x04 75, 0x05 50, 0x06 30, 0x07 20ppm, only valid on Peripheral, on Central shall be set to 0x00
} __attribute__((packed)) gap_le_switch_link_setup_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * FUNCTIONS DECLARATIONS
 *
 *
 */

/**
 * @brief Construct connection bitfield
 *
 * @param[in] con_idx  Connection local index
 *
 * @return uint32_t    Connection bitfield
 */
uint32_t gap_conn_bf(uint8_t con_idx);

/**
 * @brief Get real connection handle from conidx
 *
 * @param[in] con_idx  Connection local index
 *
 * @return uint16_t    Connection handle
 */
uint16_t gap_conn_hdl(uint8_t con_idx);

/**
 * @brief Construct unreal connection handle
 *
 * @param[in] con_idx  Connection local index
 *
 * @return uint16_t    Connection handle
 */
uint16_t gap_conn_idx_as_hdl(uint8_t con_idx);

/**
 * @brief gap module init api
 *
 * @param[in] cfg     gap init configurations
 * @param[in] global_callback
 *                    gap global event callbacks
 * @param[in] p_ext_func_cbs
 *                    gap used external callbacks to get info
 *
 */
void gap_init(const gap_config_t *cfg, gap_global_callback_t global_callback, const gap_ext_func_cbs_t *p_ext_func_cbs);

/**
 * @brief Get gap configurations
 *
 *
 * @return gap_config_t
 *                    gap configurations
 */
gap_config_t gap_get_config(void);

/**
 * @brief Check gap module stack ready or not for some reason
 *
 *
 * @return bool      Stack is read or not for some reason
 */
bool gap_stack_is_ready(void);

/**
 * @brief Get gap connection item by connhdl
 *
 * @param[in] connhdl  Connection handle
 *
 * @return gap_conn_item_t
 *                     Gap connection stored info
 */
gap_conn_item_t *gap_get_conn_item(uint16_t connhdl);

/**
 * @brief Get gap connection item by bt address
 *
 * @param[in] remote   BDAddress
 *
 * @return gap_conn_item_t
 *                     Gap connection stored info
 */
gap_conn_item_t *gap_get_conn_by_bt_address(const bt_bdaddr_t *remote);

/**
 * @brief Get gap connection item by ble address
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return gap_conn_item_t
 *                     Gap connection stored info
 */
gap_conn_item_t *gap_get_conn_by_le_address(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Get current gap connection item count
 *
 *
 * @return uint8_t     Gap connection item count
 */
uint8_t gap_count_ble_connections(void);

/**
 * @brief Dump gap module status about le part
 *
 *
 */
void gap_dump_ble_status(void);

/**
 * @brief Get gap procedure pending state
 *
 * @param[in] connhdl  Connection handle
 *
 * @return bool        Pending or not for some reason
 */
bool gap_is_any_procedure_pending(uint16_t connhdl);

/**
 * @brief Gap connection item iterator
 *
 * @param[in] func     Iterator callback for every connection item
 * @param[in] param    Context upper provided and return with func
 *
 * @return int         Iterator run success number on item
 */
int gap_conn_foreach(gap_conn_foreach_func_t func, void *param);

/**
 * @brief Get gap module used local le name pointer
 *
 * @param[in] len      Gap module used local le name length
 *
 * @return const char* Gap module used local le name pointer
 */
const char *gap_local_le_name(uint8_t *len);

/**
 * @brief Get gap module used bt address in hci
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_hci_bt_address(void);

/**
 * @brief Get gap module used ble address in hci
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_hci_le_address(void);

/**
 * @brief Get gap module used bt address in factory section
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_factory_bt_address(void);

/**
 * @brief Get gap module used ble address in factory section
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_factory_le_address(void);

/**
 * @brief Get gap module used public bt address
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_public_identity_address(void);

/**
 * @brief Get gap module used public ble address
 *
 *
 * @return const bt_bdaddr_t*
 *                     BDAddress pointer
 */
const bt_bdaddr_t *gap_random_identity_address(void);

/**
 * @brief Get local used indentity address according to own type
 *
 * @param[in] own_addr_type
 *                     Own address type used to get different type of address
 *
 * @return ble_bdaddr_t
 *                     ble address
 *
 */
ble_bdaddr_t gap_local_identity_address(bt_addr_type_t own_addr_type);

/**
 * @brief Get connection used indentity address by connection item
 *
 * @param[in] conn     Connetion item
 *
 * @return ble_bdaddr_t
 *                     ble address
 */
ble_bdaddr_t gap_conn_own_identity_address(const gap_conn_item_t *conn);

/**
 * @brief Set controller used public address
 *
 * @param[in] public_addr
 *                     public address
 * @param[in] is_ble   Set ble or bt public address
 *
 * @return bt_status_t Set successfully or failed due to some reason
 */
bt_status_t gap_set_public_address(const bt_bdaddr_t *public_addr, bool is_ble);

/**
 * @brief Set controller used bt public address
 *
 * @param[in] public_addr
 *                     public address
 *
 * @return bt_status_t Set successfully or failed due to some reason
 */
bt_status_t gap_set_bt_public_address(const bt_bdaddr_t *public_addr);

/**
 * @brief Set controller used ble public address
 *
 * @param[in] public_addr
 *                     public address
 *
 * @return bt_status_t Set successfully or failed due to some reason
 */
bt_status_t gap_set_le_public_address(const bt_bdaddr_t *public_addr);

/**
 * @brief Set le tx power according to connection handle
 *
 * @param[in] connhdl  Connection handle
 * @param[in] tx_pwr   TX power in dbm
 *
 * @return bt_status_t Set successfully or failed due to some reason
 */
bt_status_t gap_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr);

/**
 * @brief Get default connection parameters example
 *
 *
 * @return gap_conn_timing_config_t
 *                     Connection parameters structure
 */
gap_conn_timing_config_t gap_default_conn_timing(void);

/**
 * @brief Get connection peer solved address by resolving list
 *
 * @param[in] connhdl  Connection handle
 *
 * @return ble_bdaddr_t
 *                     ble address resolved by resolving list
 */
ble_bdaddr_t gap_get_peer_resolved_address(uint16_t connhdl);

/**
 * @brief Control directly send error response to peer when gatt perm error occurs
 *
 * @param[in] send_sec_error_rsp
 *                     Directly send err rsp or trigger local auth start
 *
 */
void gap_set_send_sec_error_rsp_directly(uint8_t send_sec_error_rsp);

/**
 * @brief Control directly trigger local auth start or just report to upper when gatt perm error received
 *
 * @param[in] report_sec_error
 *                     Report sec err to upper or trigger local auth start
 *
 */
void gap_set_report_sec_error_directly(uint8_t report_sec_error);

/**
 * @brief Set peer security levels value to gap module
 *
 * @param[in] conn     Connection item
 * @param[in] sec_levels
 *                     Security levels value
 *
 */
void gap_peer_security_levels_received(gap_conn_item_t *conn, gap_security_levels_t sec_levels);

/**
 * @brief Get local le indentity address that will used for CTKD (Random or public)
 *
 * @param[in] conn     Connection item run CTKD (SMP IA Exchange)
 *
 * @return ble_bdaddr_t
 *                     ble address
 */
ble_bdaddr_t gap_ctkd_get_le_identity_address(const gap_conn_item_t *conn);

/**
 * @brief Set local used secret and public key pair for SMP pairing (Replace controller key pair read)
 *
 * @param[in] p_sec_key_256
 *                     Secret Key P256
 * @param[in] p_pub_key_256
 *                     Public Key P256
 *
 * @return bt_status_t Set successfully or failed due to some reason
 */
bt_status_t gap_set_local_secret_public_key_pair(const uint8_t *p_sec_key_256, const uint8_t *p_pub_key_256);

/**
 * @brief Generate local rpa
 *
 * @param[in] cmpl     Complete event callback
 * @param[in] priv     Context upper provided and return with cmpl
 *
 * @return bt_status_t Generate start successfully or not for some reason
 */
bt_status_t gap_gen_local_rpa(void (*cmpl)(void *priv, const bt_bdaddr_t *local_rpa), void *priv);

/**
 * @brief Resolve a rpa address
 *
 * @param[in] rpa      RPA that need to be resolved
 * @param[in] cmpl     RPA resolved or failed with some record related complete event
 * @param[in] priv     Context upper provided and return with cmpl
 *
 * @return bt_status_t Resolve start successfully or not for some reason
 */
bt_status_t gap_resolve_rpa(const bt_bdaddr_t *rpa, void (*cmpl)(void *priv, const gap_bond_sec_t *out), void *priv);

/**
 * @brief Generate local host used rpa and report with GAP_EVENT_HOST_RPA_GENERATED
 * 
 * @param[in] priv     Context upper provided and return with cmpl in GAP_EVENT_HOST_RPA_GENERATED
 * 
 */
bt_status_t gap_generate_local_host_rpa(void *priv);

/**
 * @brief Terminate a connection with some reason
 *
 * @param[in] connhdl  Connection handle
 * @param[in] reason   Terminate reason to peer
 *
 * @return bt_status_t Start termination successfully or not for some reason
 */
bt_status_t gap_terminate_connection(uint16_t connhdl, uint8_t reason);

/**
 * @brief Terminate all ble connections
 *
 *
 * @return bt_status_t Start termination successfully or not for some reason
 */
bt_status_t gap_terminate_all_ble_connection(void);

/**
 * @brief Update connection paramters
 *
 * @param[in] connhdl  Connection handle
 * @param[in] params   Connection parameters want to update to in request
 *
 * @return bt_status_t Start connection update successfully or not for some reason
 */
bt_status_t gap_update_le_conn_parameters(uint16_t connhdl, const gap_update_params_t *params);

/**
 * @brief Response to a connection update paramters request
 *
 * @param[in] connhdl  Connection handle
 * @param[in] params   Connection parameters want to update to in rsp
 * @param[in] accept   Accept or not
 *
 * @return bt_status_t Send response successfully or not for some reason
 */
bt_status_t gap_accept_le_conn_parameters(uint16_t connhdl, const gap_update_params_t *params, bool accept);

/**
 * @brief Set connection subrate default paramters
 *
 * @param[in] params   Subrate default paramters
 *
 * @return bt_status_t Set default parameters successfully or not for some reason
 */
bt_status_t gap_set_default_subrate(const gap_subrate_params_t *params);

/**
 * @brief Update connection subrate paramters
 *
 * @param[in] connhdl  Connection handle
 * @param[in] params   Subrate paramters want to update to
 *
 * @return bt_status_t Start subrate parameters update successfully or not for some reason
 */
bt_status_t gap_update_subrate_parameters(uint16_t connhdl, const gap_subrate_params_t *params);

/**
 * @brief Update frame space parameters
 *
 * @param[in] connhdl  Connection handle
 * @param[in] params   frame space parameters want to update to
 *
 * @return bt_status_t Start frame space parameters update successfully or not for some reason
 */
bt_status_t gap_update_frame_space_parameters(uint16_t connhdl, const gap_frame_space_param_t *params);

/**
 * @brief Write le suggested default data length
 *
 * @param[in] max_tx_octets
 *                     Max tx data length in octets host suggested when controller start transmission
 * @param[in] max_tx_time_us
 *                     Max tx data time in us host suggested when controller start transmission
 *
 * @return bt_status_t Write le suggested default tx data length successfully or not for some reason
 */
bt_status_t gap_write_le_suggested_default_data_length(uint16_t max_tx_octets, uint16_t max_tx_time_us);

/**
 * @brief Write le tx data length
 *
 * @param[in] connhdl  Connection handle
 * @param[in] max_tx_octets
 *                     Max tx data length in octets host set when controller start transmission
 * @param[in] max_tx_time_us
 *                     Max tx data time in us host set when controller start transmission
 *
 * @return bt_status_t Write le tx data length successfully or not for some reason
 */
bt_status_t gap_set_le_data_length(uint16_t connhdl, uint16_t max_tx_octets, uint16_t max_tx_time_us);

/**
 * @brief Read current connection used phy value
 *
 * @param[in] connhdl  Connection handle
 *
 * @return bt_status_t Read current used connection phy successfully or not for some reason
 */
bt_status_t gap_read_le_conn_phy(uint16_t connhdl);

/**
 * @brief Set default LE connection phy when connection is established
 *
 * @param[in] tx_phy_bits
 *                     TX phy bits
 * @param[in] rx_phy_bits
 *                     RX phy bits
 *
 * @return bt_status_t Set le default phy successfully or not for some reason
 */
bt_status_t gap_set_le_default_phy(uint8_t tx_phy_bits, uint8_t rx_phy_bits);

/**
 * @brief Set LE connection phy when connection exist
 *
 * @param[in] connhdl  Connection handle
 * @param[in] tx_phy_bits
 *                     TX phy bits
 * @param[in] rx_phy_bits
 *                     RX phy bits
 * @param[in] coded_prefer
 *                     Coded phy prefer
 *
 * @return bt_status_t Set le connection phy successfully or not for some reason
 */
bt_status_t gap_set_le_conn_phy(uint16_t connhdl, uint8_t tx_phy_bits, uint8_t rx_phy_bits,
                                gap_coded_phy_prefer_t coded_prefer);

/**
 * @brief Send raw value hci cmd
 *
 * @param[in] cmd_opcode
 *                     HCI cmd code with OCF OGF concat
 * @param[in] cmd_len  HCI cmd length only count cmd data payload
 * @param[in] data_little_endian
 *                     HCI cmd data payload in little endian
 * @param[in] cmd_cb   HCI cmd complete or
 * @param[in] priv     Context upper provided and return with cmpl
 *
 * @return bt_status_t Send successfully or failed
 */
bt_status_t gap_send_raw_hci_cmd(uint16_t cmd_opcode, uint16_t cmd_len, const uint8_t *data_little_endian,
                                 hci_cmd_evt_func_t cmd_cb, void *priv);

/**
 * @brief Send raw value hci cmd with extra data
 *
 * @param[in] cmd_opcode
 *                     HCI cmd code with OCF OGF concat
 * @param[in] cmd_len  HCI cmd length only count cmd data payload
 * @param[in] data_le  HCI cmd data payload in little endian
 * @param[in] cmd_cb   HCI cmd complete or
 * @param[in] priv     Context upper provided and return with cmpl
 * @param[in] cont     Continue data that will return with cmp evt
 * @param[in] cmpl     Complete data that will return with cmp evt
 *
 * @return bt_status_t Send raw hci cmd status
 */
bt_status_t gap_send_raw_hci_cmd_extra(uint16_t cmd_opcode, uint16_t cmd_len, const uint8_t *data_le,
                                       hci_cmd_evt_func_t cmd_cb, void *priv, void *cont, void *cmpl);

/**
 * @brief Read le tx power range
 *
 *
 * @return bt_status_t Read le tx power range successfully or not for some reason
 */
bt_status_t gap_read_le_tx_power_range(void);

/**
 * @brief Read legacy adv tx power
 *
 *
 * @return bt_status_t Read legacy adv tx power successfully or not for some reason
 */
bt_status_t gap_read_legacy_adv_tx_power(void);

/**
 * @brief Read adv tx power according to adv handle
 *
 * @param[in] adv_handle
 *                     Adv handle that need to be read tx power
 *
 * @return bt_status_t Read adv tx power successfully or not for some reason
 */
bt_status_t gap_read_le_adv_tx_power(uint8_t adv_handle);

/**
 * @brief Read connection local tx power with specified phy
 *
 * @param[in] connhdl  Connection handle
 * @param[in] phy      Phy value
 *
 * @return bt_status_t Read connection local tx power successfully or not for some reason
 */
bt_status_t gap_read_conn_local_tx_power(uint16_t connhdl, gap_le_detail_phy_t phy);

/**
 * @brief Read connection remote tx power with specified phy
 *
 * @param[in] connhdl  Connection handle
 * @param[in] phy      Phy value
 *
 * @return bt_status_t Read connection remote tx power successfully or not for some reason
 */
bt_status_t gap_read_conn_remote_tx_power(uint16_t connhdl, gap_le_detail_phy_t phy);

/**
 * @brief Control connection tx power report
 *
 * @param[in] connhdl  Connection handle
 * @param[in] local_enable
 *                     Local tx power report enable or disable
 * @param[in] remote_enable
 *                     Reamote tx power report enable or disable
 *
 * @return bt_status_t Control connection tx power report successfully or not for some reason
 */
bt_status_t gap_set_conn_tx_power_report(uint16_t connhdl, bool local_enable, bool remote_enable);

/*
 * Features
 *
*/

/**
 * @brief Gap get local le features that has been read after gap init
 *
 *
 * @return uint32_t    GAP features
 */
uint32_t gap_local_le_features(void);

/**
 * @brief Gap get local le high features that has been read after gap init
 *
 *
 * @return uint32_t    GAP high features
 */
uint32_t gap_local_le_high_features(void);

/**
 * @brief Control le host supported features
 *
 * @param[in] bit_number
 *                     Feature bit number
 * @param[in] enable   Enable or disable
 *
 * @return bt_status_t Control le host feature successfully or not for some reason
 */
bt_status_t gap_set_le_host_feature_support(uint16_t bit_number, uint8_t enable);

/**
 * Security
 *
 */

/**
 * @brief Get gap module used local irk
 *
 *
 * @return const uint8_t*
 *                     IRK value pointer
 */
const uint8_t *gap_get_local_irk(void);

/**
 * @brief Set gap module used local irk
 *
 * @param[in] p_irk    IRK value pointer
 *
 */
void gap_set_local_irk(const uint8_t *p_irk);

/**
 * @brief Get gap module used local csrk
 *
 *
 * @return const uint8_t*
 *                     CSRK pointer
 */
const uint8_t *gap_get_local_csrk(void);

/**
 * @brief Start bredr ltk derivation from linkkey
 *
 * @param[in] connhdl  Connection handle
 *
 * @return bt_status_t Start CTKD from BREDR status
 */
bt_status_t gap_bredr_start_ltk_derivation(uint16_t connhdl);

/**
 * @brief Set connection security level
 *
 * @param[in] connhdl  Connection handle
 * @param[in] sec_level
 *                     Security level requirements for this link
 *
 * @return bt_status_t Set Security level status
 */
bt_status_t gap_set_security_level(uint16_t connhdl, gap_link_sec_level_t sec_level);

/**
 * @brief Set connection Data sign level
 *
 * @param[in] connhdl  Connection handle
 * @param[in] data_sign_level
 *                     Data sign level requirements for this link
 *
 * @return bt_status_t Set Data sign level status
 */
bt_status_t gap_set_data_sign_security(uint16_t connhdl, gap_data_sign_level_t data_sign_level);

/**
 * @brief Set connection key strength
 *
 * @param[in] connhdl  Connection handle
 * @param[in] key_size Key size requirements for this connection
 *
 * @return bt_status_t Set connection key strength status
 */
bt_status_t gap_set_key_strength(uint16_t connhdl, uint8_t key_size);

/**
 * @brief Set connection accepted smp pairing method
 *
 * @param[in] connhdl  Connection handle
 * @param[in] accept_method
 *                     @see gap_sec_accept_method_t
 *
 * @return bt_status_t Set connection accepted smp pairing method status
 */
bt_status_t gap_set_accepted_pairing_method(uint16_t connhdl, uint8_t accept_method);

/**
 * @brief Start block att tx rx by secure reason
 *
 * @param[in] connhdl  Connection handle
 * @param[in] att_tx   Block att tx data
 * @param[in] att_rx   Block att rx data
 *
 * @return bt_status_t Block att tx rx status
 */
bt_status_t gap_block_att_tx_rx_by_secure(uint16_t connhdl, bool att_tx, bool att_rx);

/**
 * @brief Start unblock att tx rx by secure reason
 *
 * @param[in] connhdl  Connection handle
 * @param[in] att_tx   Unblock att tx
 * @param[in] att_rx   Unblock att rx
 *
 * @return bt_status_t Unblock att tx rx status
 */
bt_status_t gap_unblock_att_tx_rx_by_secure(uint16_t connhdl, bool att_tx, bool att_rx);

/**
 * @brief Start smp authentication according to connection set method and sec level
 *
 * @param[in] connhdl  Connection handle
 * @param[in] who      Who started this auth
 *
 * @return bt_status_t Start auth status
 */
bt_status_t gap_start_authentication(uint16_t connhdl, gap_who_started_auth_t who);

/**
 * @brief Start smp authentication according to connection set method and sec level
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Start auth status
 */
bt_status_t gap_start_authentication_by_address(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Send pairing request and fill smp pairing request
 *
 * @param[in] connhdl  Connection handle
 * @param[in] p_req    @see smp_requirements_t
 *
 * @return bt_status_t Send smp pairing request successfully or not for some reason
 */
bt_status_t gap_send_pairing_request(uint16_t connhdl, const smp_requirements_t *p_req);

/**
 * @brief Send scurity request and fill auth request value
 *
 * @param[in] connhdl  Connection handle
 * @param[in] auth_req @see SMP_AUTH_BONDING_MASK around
 *
 * @return bt_status_t Send smp pairing request successfully or not for some reason
 */
bt_status_t gap_send_security_request(uint16_t connhdl, uint8_t auth_req);

/**
 * @brief Enable connection encryption using enc data and ltk in central role
 *
 * @param[in] connhdl  Connection handle
 * @param[in] p_enc_data
 *                     @see gap_ltk_enc_info_t
 * @param[in] p_ltk    LTK used for connection encryption
 *
 * @return bt_status_t Enable connection encryption successfully or not for some reason
 */
bt_status_t gap_start_enable_encryption(uint16_t connhdl, const gap_ltk_enc_info_t *p_enc_data, const uint8_t *p_ltk);

/**
 * @brief Set passkey entry method when start smp
 *
 * @param[in] connhdl  Connection handle
 * @param[in] passkey_entry
 *                     Use passkey entry smp method or not for some reason
 *
 * @return bt_status_t Set passkey entry method status
 */
bt_status_t gap_set_use_passkey_entry_method(uint16_t connhdl, bool passkey_entry);

/**
 * @brief Set passkey entry method when start smp according to address
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] passkey_entry
 *                     Use passkey entry smp method or not for some reason
 *
 * @return bt_status_t Set passkey entry method status
 */
bt_status_t gap_set_use_passkey_entry_method_by_address(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr,
                                                        bool passkey_entry);

/**
 * @brief Control aes encrypt using controller or host
 *
 * @param[in] enable   Enable using host or disable using controller
 *
 * @return bt_status_t Control aes encrypt usage status
 */
bt_status_t gap_set_use_aes_encrypt_by_host_enable(bool enable);

/**
 * @brief Upper confirm numeric compare or not for some reason confirm
 *
 * @param[in] peer_type_or_connhdl
 *                     Peer address type or connection handle
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] user_confirmed
 *                     Upper confirmed or not for some reason
 *
 * @return bt_status_t Input upper nc confirm status
 */
bt_status_t gap_input_numeric_confirm(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, bool user_confirmed);

/**
 * @brief Upper reply to ltk request
 *
 * @param[in] connhdl  Connection handle
 * @param[in] negative_reply
 *                     Negative or positive reply to ltk request
 * @param[in] p_ltk_reply
 *                     The ltk used for positive reply
 *
 * @return bt_status_t Reply to ltk request status
 */
bt_status_t gap_reply_peer_ltk_request(uint16_t connhdl, bool negative_reply, const uint8_t *p_ltk_reply);

/**
 * @brief Upper reply to pairing request
 *
 * @param[in] connhdl  Connection handle
 * @param[in] pairing_failed
 *                     Do not accept pairing request with pairing failed reason or accept it
 * @param[in] err_code Pairing failed reason if not accept pairing request
 *
 * @return bt_status_t Reply to pairing request status
 */
bt_status_t gap_reply_peer_pairing_request(uint16_t connhdl, bool pairing_failed, smp_error_code_t err_code);

/**
 * @brief Upper input 6 digit passkey value
 *
 * @param[in] peer_type_or_connhdl
 *                     Peer address type or connection handle
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] passkey  Passkey value only 6 digit value valid
 *
 * @return bt_status_t Reply to 6 digit passkey request status
 */
bt_status_t gap_input_6_digit_passkey(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, uint32_t passkey);

/**
 * @brief Upper input Legacy TermKey value from oob
 *
 * @param[in] peer_type_or_connhdl
 *                     Peer address type or connection handle
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] tk       TermKey
 *
 * @return bt_status_t Reply to TK request status
 */
bt_status_t gap_input_oob_legacy_tk(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr, const uint8_t *tk);

/**
 * @brief Upper input local used auth data from oob
 *
 * @param[in] peer_type_or_connhdl
 *                     Peer address type or connection handle
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] data     OOB auth data local used
 *
 * @return bt_status_t Reply to local used oob auth data request status
 */
bt_status_t gap_input_local_oob_auth_data(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr,
                                          const gap_smp_oob_auth_data_t *data);

/**
 * @brief Upper input peer used auth data from oob
 *
 * @param[in] peer_type_or_connhdl
 *                     Peer address type or connection handle
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] data     OOB auth data peer used
 *
 * @return bt_status_t Reply to peer used oob auth data request status
 */
bt_status_t gap_input_peer_oob_auth_data(uint16_t peer_type_or_connhdl, const bt_bdaddr_t *peer_addr,
                                         const gap_smp_oob_auth_data_t *data);

/**
 * Data types for AdvData, ScanRsp, ACAD, OOB, EIR
 *
 */

/**
 * @brief Start using key material value to encrypt some data and callback with func
 *
 * @param[in] key      Key used to start encryption
 * @param[in] data     Data used to be encrypted
 * @param[in] len      Data length used to be encrypted
 * @param[in] func     A func callback when encryption is completed
 * @param[in] priv     Context upper provided and return with func
 *
 * @return bool        Start encryption successfully or not for some reason
 */
bool gap_dt_start_encrypt_data(const gap_key_material_t *key, const uint8_t *data, uint16_t len,
                               gap_dt_encrypt_callback_t func, void *priv);

/**
 * @brief Start using key material value to decrypt some data and callback with func
 *
 * @param[in] key      Key used to start decryption
 * @param[in] p        Data used to be encrypted
 * @param[in] func     A func callback when decryption is completed
 * @param[in] priv     Context upper provided and return with func
 *
 * @return bool        Start decryption successfully or not for some reason
 */
bool gap_dt_start_decrypt_data(const gap_key_material_t *key, const gap_enc_data_t *p, gap_key_callback_t func,
                               void *priv);

/**
 * @brief Add specified data type and data and formated in ltv structure to buf
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] type     Source data type @see GAP_DT_FLAGS
 * @param[in] data     Source data
 * @param[in] len      Length of source data
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_data_type(gap_dt_buf_t *buf, uint8_t type, const uint8_t *data, uint16_t len);

/**
 * @brief Add raw data in ltv structure format to buf with no extra operation
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] data     Source data in ltv format
 * @param[in] len      Length of source data in ltv format
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_raw_data(gap_dt_buf_t *buf, const uint8_t *data, uint16_t len);

/**
 * @brief A demonstration code to add the ad flag field of adv @see GAP_DT_FLAGS
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] discoverable
 *                     Discoverable bit @see GAP_FLAGS_LE_GENERAL_DISCOVERABLE_MODE
 * @param[in] simu_bredr_support
 *                     @see GAP_FLAGS_SIMU_LE_BREDR_TO_SAME_DEVICE
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_flags(gap_dt_buf_t *buf, uint8_t discoverable, bool simu_bredr_support);

/**
 * @brief A demonstration code to add the local name field of adv @see GAP_DT_SHORT_LOCAL_NAME
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] use_legacy_pdu
 *                     Check whether adv data is much longer to add this name string
 * @param[in] cust_le_name
 *                     Custom specified name string
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_local_le_name(gap_dt_buf_t *buf, bool use_legacy_pdu, const char *cust_le_name);

/**
 * @brief A demonstration code to add the tx power field of adv @see GAP_DT_TX_POWER_LEVEL
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] tx_power_level
 *                     TX power level
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_tx_power(gap_dt_buf_t *buf, uint8_t tx_power_level);

/**
 * @brief A demonstration code to add the 16 bits service data field of adv @see GAP_DT_SERVICE_DATA_16BIT_UUID
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] service_uuid
 *                     The service uuid that service data belong to
 * @param[in] data     Service data
 * @param[in] len      Length of service data
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_dt_add_service_data(gap_dt_buf_t *buf, uint16_t service_uuid, const uint8_t *data, uint8_t len);

/**
 * @brief A demonstration code to add the service 16 bits uuid list field of adv @see GAP_DT_SRVC_UUID_16_CMPL_LIST
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] uuid     16 bits uuid list
 * @param[in] count    List size of 16 bits uuid list
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_set_le_service_16_uuid(gap_dt_buf_t *buf, const uint16_t *uuid, uint16_t count);

/**
 * @brief A demonstration code to add the service 128 bits uuid list field of adv @see GAP_DT_SRVC_UUID_128_CMPL_LIST
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] uuid_le  128 bits uuid in little endian list
 * @param[in] count    List size of 128 bits uuid in little endian list
 *
 * @return bool        Add to buf successfully or not for some reason, failed with invalid param or OOM
 */
bool gap_set_le_service_128_uuid(gap_dt_buf_t *buf, const gap_uuid_128_t *uuid_le, uint16_t count);

/**
 * @brief Check whether the buf is empty
 *
 * @param[in] buf      Buf to check
 *
 * @return bool        Empty or not for some reason
 */
bool gap_dt_buf_is_empty(const gap_dt_buf_t *buf);

/**
 * @brief Get the complete data of buf at the beginning
 *
 * @param[in] buf      Buf to get total buf pointer
 *
 * @return const uint8_t*
 *                     The complete data pointer of buf at the beginning
 */
const uint8_t *gap_dt_buf_data(const gap_dt_buf_t *buf);

/**
 * @brief Get the length of complete data of buf
 *
 * @param[in] buf      Buf to get total buf length
 *
 * @return uint16_t    The length of complete data of buf
 */
uint16_t gap_dt_buf_len(const gap_dt_buf_t *buf);

/**
 * @brief Clear all data that hasbeen added to the buf
 *
 * @param[in] buf      Buf all data has been added into
 *
 */
void gap_dt_buf_clear(gap_dt_buf_t *buf);

/**
 * @brief Find the data start of specified data type in buf
 *
 * @param[in] buf      Destination buf to be added
 * @param[in] ad_type  Data type first priority matched
 * @param[in] ad_type_2
 *                     Data type second priority matched
 *
 * @return const gap_dt_head_t*
 *                     Data start head that has been matched
 */
const gap_dt_head_t *gap_dt_buf_find_type(const gap_dt_buf_t *buf, uint8_t ad_type, uint8_t ad_type_2);

/**
 * Filter Accept List
 *
 */

/**
 * @brief Add filter accept list item
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Add item status
 */
bt_status_t gap_filter_list_add(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Add filter accept list items for which user
 *
 * @param[in] user     Filter accept list used for which user
 * @param[in] addr_list
 *                     Filter accept list used address list
 * @param[in] count    The list size of filter accept list used address list
 *
 * @return bt_status_t Add user item status
 */
bt_status_t gap_filter_list_add_user_item(uint8_t user, const ble_bdaddr_t *addr_list, uint16_t count);

/**
 * @brief Add 0xFF type filter accept list item, means that devices sending anonymous advertisements can be accept
 *
 *
 * @return bt_status_t Add anonymous item status
 */
bt_status_t gap_filter_list_add_anonymous(void);

/**
 * @brief Remove filter accept list item
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Remove item status
 */
bt_status_t gap_filter_list_remove(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Remove all items belonged to which user
 *
 * @param[in] user     Filter accept list used for which user
 *
 * @return bt_status_t Remove all items belonged to user status
 */
bt_status_t gap_filter_list_remove_user_item(uint8_t user);

/**
 * @brief Remove anonymous filter accept list item
 *
 *
 * @return bt_status_t Remove anonymous filter accept list item status
 */
bt_status_t gap_filter_list_remove_anonymous(void);

/**
 * @brief Remove filter accept list item by index
 *
 * @param[in] index    Filter accept list item index
 *
 * @return bt_status_t Remove filter accept list item by index status
 */
bt_status_t gap_filter_list_remove_by_index(int index);

/**
 * @brief Clear all filter accept list items
 *
 *
 * @return bt_status_t Clear all filter accept list items successfully or not for some reason
 */
bt_status_t gap_filter_list_clear(void);

/**
 * @brief Set items in raw device structures format
 *
 * @param[in] items    Items in raw device structures format
 * @param[in] count    Number of items
 *
 * @return bt_status_t Set items status
 */
bt_status_t gap_filter_list_set_devices(const gap_filter_item_t *items, uint16_t count);

/**
 * @brief Get items specified by index
 *
 * @param[in] index    Item index
 *
 * @return const gap_filter_item_t*
 *                     Return item in raw device structures format
 */
const gap_filter_item_t *gap_filter_list_get_item(int index);

/**
 * @brief Check is there any items belonged to which user
 *
 * @param[in] user     Filter accept list used for which user
 *
 * @return bool        Exist or not for some reason
 */
bool gap_filter_list_user_item_exist(uint8_t user);

/**
 * @brief Get current filter list items number
 *
 *
 * @return uint8_t     The number of items added
 */
uint8_t gap_filter_list_curr_size(void);

/**
 * @brief Get current filter accept list max size
 *
 *
 * @return uint8_t     Size of max filter accept list size
 */
uint8_t gap_filter_list_max_size(void);

/**
 * Resolving List
 *
 */

/**
 * @brief Add device to resolving list, set device mode if it is device privacy mode
 *        and disable address resolution if enabled and restore after add list complete
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] peer_irk Peer device ITK
 * @param[in] device_privacy_mode
 *                     Use device privacy mode
 * @param[in] check_addr_reso_switch
 *                     Check address resolution state and auto disable and enable control
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 *
 * @return bt_status_t Add device to resolving list status
 */
bt_status_t gap_resolving_list_add(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, const uint8_t *peer_irk,
                                   bool device_privacy_mode, bool check_addr_reso_switch);

/**
 * @brief Add device to resolving list, set device mode if it is device privacy mode
 *        and disable address resolution if enabled and restore if needed after add list complete
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] peer_irk Peer device ITK
 * @param[in] local_irk
 *                     Local used IRK
 * @param[in] device_privacy_mode
 *                     Use device privacy mode
 * @param[in] check_addr_reso_switch
 *                     Check address resolution state and auto disable and enable control
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 *
 * @return bt_status_t Add device to resolving list status
 */
bt_status_t gap_resolving_list_add_item(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, const uint8_t *peer_irk,
                                        const uint8_t *local_irk, bool device_privacy_mode, bool check_addr_reso_switch);

/**
 * @brief Remove device from resolving list and disable address resolution if enabled
 *        and restore if needed after add list complete
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] check_addr_reso_switch
 *                     Check address resolution state and auto disable and enable control
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 *
 * @return bt_status_t Remove device from resolving list status
 */
bt_status_t gap_resolving_list_remove(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr,
                                      bool check_addr_reso_switch);

/**
 * @brief Remove device from resolving list status by index
 *
 * @param[in] index    Device index in resolving list
 * @param[in] check_addr_reso_switch
 *                     Check address resolution state and auto disable and enable control
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 *
 * @return bt_status_t Remove device from resolving list by index status
 */
bt_status_t gap_resolving_list_remove_by_index(int index, bool check_addr_reso_switch);

/**
 * @brief Control address resolution in controller (Do not turn it on/off unless necessary after off/on)
 *
 * @param[in] enable   Enable or disable
 *
 * @return bt_status_t Enable or disable address resolution in controller status
 */
bt_status_t gap_enable_address_resolution(bool enable);

/**
 * @brief Clear all resolving list items
 *
 * @param[in] check_addr_reso_switch
 *                     Check address resolution state and auto disable and enable control
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 *
 * @return bt_status_t Clear all resolving list items status
 */
bt_status_t gap_resolving_list_clear(bool check_addr_reso_switch);

/**
 * @brief Set resolving list item device to network privacy mode
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Set device network privacy mode status
 */
bt_status_t gap_set_network_privacy_mode(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Set resolving list item device to device privacy mode
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] auto_disable_reso_first
 *                     Disable address resolution first if needed
 *                     Now leave it as false, we can stop all related adv no matter addr reso
 * @param[in] set_addr_reso_enable
 *                     Set address resolution enable after all things done
 *
 * @return bt_status_t Set device privacy mode status
 */
bt_status_t gap_set_device_privacy_mode(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr,
                                        bool auto_disable_reso_first, bool set_addr_reso_enable);

/**
 * @brief Set all devices network privacy mode in resolving list
 *
 *
 * @return bt_status_t Set all devices network privacy mode status
 */
bt_status_t gap_set_all_network_privacy_mode(void);

/**
 * @brief Set all devices device privacy mode in resolving list
 *
 *
 * @return bt_status_t Set all devices device privacy mode status
 */
bt_status_t gap_set_all_device_privacy_mode(void);

/**
 * @brief Read connection peer used rpa
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] cmd_cb   Read complete and callback using cmd_cb
 * @param[in] priv     Context upper provided and return with cmd_cb
 *
 * @return bt_status_t Read peer rpa cmd send successfully or not for some reason
 */
bt_status_t gap_read_peer_rpa(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, hci_cmd_evt_func_t cmd_cb,
                              void *priv);

/**
 * @brief Read local adv used rpa when using resolving list item to generate with locak IRK
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] cmd_cb   Read complete and callback using cmd_cb
 * @param[in] priv     Context upper provided and return with cmd_cb
 *
 * @return bt_status_t Read local rpa cmd send successfully or not for some reason
 */
bt_status_t gap_read_local_rpa(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr, hci_cmd_evt_func_t cmd_cb,
                               void *priv);

/**
 * @brief Set local used RPA renew timeout
 *
 * @param[in] timeout_sec
 *                     Local used RPA renew timer timeout in seconds
 *
 * @return bt_status_t Set RPA renew timeout status
 */
bt_status_t gap_set_rpa_timeout(uint16_t timeout_sec);

/**
 * @brief Find resolving list item by address info
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return const gap_resolv_item_t*
 *                     Return found resolving list item
 */
const gap_resolv_item_t *gap_resolving_list_find_item(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Get resolving list item by index
 *
 * @param[in] index    Resolving list item index in list
 *
 * @return const gap_resolv_item_t*
 *                     Return index's resolving list item
 */
const gap_resolv_item_t *gap_resolving_list_get_item(int index);

/**
 * @brief Get address resolution enable state
 *
 *
 * @return bool        Enabled or disabled
 */
bool gap_address_reso_is_enabled(void);

/**
 * @brief Check whether to refresh adv after address reso is enabled
 *        Sometime we use this to check and avoid extra adv enable operation from upper
 *
 *
 * @return bool        Address resolution control is under process and need to refresh after all thing done
 */
bool gap_is_refresh_adv_after_addr_reso_enable(void);

/**
 * @brief Get current resolving list items number
 *
 *
 * @return uint8_t     Current resolving list items number
 */
uint8_t gap_resolving_list_curr_size(void);

/**
 * @brief Get max resolving list items size
 *
 *
 * @return uint8_t     Max resolving list items size
 */
uint8_t gap_resolving_list_max_size(void);

/**
 * Monitored Advertisers
 *
 *
 */
/**
 * @brief Add a monitored advertiser to list
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 * @param[in] low_rssi_timeout_s
 *                     Timeout report when advertiser's rssi is under rssi_threshold_low_dbm
 * @param[in] rssi_threshold_low_dbm
 *                     Low threshold RSSI in dbm, device's rssi is lower than it monitored
 * @param[in] rssi_threshold_high_dbm
 *                     High threshold RSSI in dbm, device's rssi is higher than it monitored
 *
 * @return bt_status_t Add monitored advertiser into list status
 */
bt_status_t gap_adv_monitored_list_add(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr,
                                       uint8_t low_rssi_timeout_s,
                                       int8_t rssi_threshold_low_dbm, int8_t rssi_threshold_high_dbm);

/**
 * @brief Remove a monitored advertiser from list
 *
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Remove a monitored advertiser from list successfully or not for some reason
 */
bt_status_t gap_adv_monitored_list_remove(bt_addr_type_t peer_type, const bt_bdaddr_t *peer_addr);

/**
 * @brief Clear all monitored advertisers in list
 *
 *
 * @return bt_status_t Clear all successfully or not for some reason
 */
bt_status_t gap_adv_monitored_list_clear_all(void);

/**
 * @brief Control monitoring advertisers in list
 *
 * @param[in] enable   Enable or disable monitoring
 *
 * @return bt_status_t Control status
 */
bt_status_t gap_enable_monitoring_advertisers(bool enable);

/**
 * @brief Get current monitored advertisers list size
 *
 *
 * @return uint8_t     Current monitored advertisers list size
 */
uint8_t gap_adv_monitored_list_curr_size(void);

/**
 * @brief Get max monitored advertisers list size
 *
 *
 * @return uint8_t     Max monitored advertisers list size
 */
uint8_t gap_adv_monitored_list_max_size(void);

/**
 * BLE Advertising
 *
 */
/**
 * @brief Set adv parameters and return with allocated adv hdl
 *
 * @param[in] param    Adv parameters
 * @param[in] cb       Adv callabck when adv and connection event is generated and report with
 *
 * @return uint8_t     Adv hdl allocated
 */
uint8_t gap_set_adv_parameters(const gap_adv_param_t *param, gap_adv_callback_t cb);

/**
 * @brief Set specified adv hdl adv's parameters
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] param    Adv parameters
 * @param[in] cb       Adv callabck when adv and connection event is generated and report with
 *
 */
void gap_set_advertising(uint8_t adv_handle, const gap_adv_param_t *param, gap_adv_callback_t cb);

/**
 * @brief Update specified adv hdl adv's parameters
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] param    Adv parameters
 * @param[in] cb       Adv callabck when adv and connection event is generated and report with
 *
 */
void gap_refresh_advertising(uint8_t adv_handle, const gap_adv_param_t *param, gap_adv_callback_t cb);

/**
 * @brief Get specified adv's adv_sid value
 *
 * @param[in] adv_hdl  Adv hdl specified which adv set to operate
 *
 * @return uint8_t     adv sid if adv is exist or @see GAP_INVALID_ADV_SET_ID
 */
uint8_t gap_adv_get_adv_set_id(uint8_t adv_hdl);

/**
 * @brief Set specified adv's random address if we use host specified random address
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] random   Random BDAddress
 *
 * @return bt_status_t Set adv ramdon address successfully or not for some reason
 */
bt_status_t gap_set_adv_random_address(uint8_t adv_handle, const bt_bdaddr_t *random);

/**
 * @brief Set adv data unchanged but adv DID is changed to update air pdu data
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 *
 * @return bt_status_t Set adv data unchanged status
 */
bt_status_t gap_set_adv_data_unchanged(uint8_t adv_handle); // unchanged data, just update the Advertising DID

/**
 * @brief Set specified adv hdl adv's complete advertising data
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] data     Advertising data in complete provided
 * @param[in] len      Length of advertising data
 *
 * @return bt_status_t Set adv data status
 */
bt_status_t gap_set_adv_data(uint8_t adv_handle, const uint8_t *data, uint16_t len);

/**
 * @brief Set specified adv hdl adv's complete scan response data
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] data     Scan response data in complete provided
 * @param[in] len      Length of scan response data
 *
 * @return bt_status_t Set scan rsp data status
 */
bt_status_t gap_set_scan_rsp_data(uint8_t adv_handle, const uint8_t *data, uint16_t len);

/**
 * @brief Set specified adv hdl adv's advertising data using data operation
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] op       Data operation
 * @param[in] data     Advertising data
 * @param[in] len      Length of advertising data
 *
 * @return bt_status_t Set adv data with op status
 */
bt_status_t gap_set_ext_adv_data(uint8_t adv_handle, gap_adv_data_op_t op, const uint8_t *data, uint16_t len);

/**
 * @brief Set specified adv hdl adv's scan response data using data operation
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] op       Data operation
 * @param[in] data     Scan response data
 * @param[in] len      Length of scan response data
 *
 * @return bt_status_t Set scan rsp data with op status
 */
bt_status_t gap_set_ext_scan_rsp_data(uint8_t adv_handle, gap_adv_data_op_t op, const uint8_t *data, uint16_t len);

/**
 * @brief Enable advertising
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] duration_ms
 *                     Adv last duration in ms
 * @param[in] max_ext_adv_events
 *                     Max advertising event number to sent out before duration is expired
 *
 * @return bt_status_t Enable adv successfully or not for some reason
 */
bt_status_t gap_set_ext_adv_enable(uint8_t adv_handle, uint32_t duration_ms, uint8_t max_ext_adv_events);

/**
 * @brief Set ext adv decision data in decision PDU
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] resolvable_tag_present
 *                     Resolvable tag present in decision data or other data present
 * @param[in] arbitary_data
 *                     Resolvable tag or other data
 * @param[in] arbitary_data_len
 *                     Resolvable tag in 6 octets or other data length
 *
 * @return bt_status_t Set adv decision data successfully or not for some reason
 */
bt_status_t gap_set_ext_adv_decision_data(uint8_t adv_handle, bool resolvable_tag_present,
                                          const uint8_t *arbitary_data, uint16_t arbitary_data_len);

/**
 * @brief Enable advertising @see gap_set_ext_adv_enable
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] duration_ms
 *                     Adv last duration in ms
 * @param[in] max_ext_adv_evts
 *                     Max advertising event number to sent out before duration is expired
 *
 * @return bt_status_t Enable adv successfully or not for some reason
 */
bt_status_t gap_enable_advertising(uint8_t adv_handle, uint32_t duration_ms, uint8_t max_ext_adv_evts);

/**
 * @brief Disable advertising and remove adv set is needed
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] remove_adv_set
 *                     whether it is necessary to remove adv set
 * @param[in] who      Disable by who
 *
 * @return bt_status_t Disable adv successfully or not for some reason
 */
bt_status_t gap_disable_advertising(uint8_t adv_handle, bool remove_adv_set, gap_who_disable_adv_t who);

/**
 * @brief Set flag to update adv's rpa when adv data or scan rsp data changed
 *
 * @param[in] adv_handle
 *                     Adv hdl specified which adv set to operate
 * @param[in] adv_data_change
 *                     Advertising data when changed update rpa
 * @param[in] scan_rsp_change
 *                     Scan response data when changed update rpa
 *
 * @return bt_status_t Set rpa update flag successfully or not for some reason
 */
bt_status_t gap_update_advertising_rpa_when(uint8_t adv_handle, bool adv_data_change, bool scan_rsp_change);

/**
 * @brief Get default advertising timing example
 *
 *
 * @return gap_adv_timing_t
 *                     Advertising timing
 */
gap_adv_timing_t gap_default_adv_timing(void);

/**
 * BLE Scanning
 *
 */

/**
 * @brief Start or set scanning with specified parameters and event callback
 *
 * @param[in] param    Scanning parameters with auto start option
 * @param[in] cb       Event callbacks
 *
 * @return bt_status_t Start scanning successfully or not for some reason
 */
bt_status_t gap_start_scanning(const gap_scan_param_t *param, gap_scan_callback_t cb);

/**
 * @brief Start or set 'bg' scanning with specified parameters and event callback
 *
 * @param[in] param    Scanning parameters with auto start option
 * @param[in] cb       Event callbacks
 *
 * @return bt_status_t Start scanning successfully or not for some reason
 */
bt_status_t gap_start_background_scanning(const gap_scan_param_t *param, gap_scan_callback_t cb);

/**
 * @brief Enable scanning with filter policy, duration in ms and period in ms
 *
 * @param[in] filter_duplicates
 *                     Filter duplicated advertising pdu
 * @param[in] duration_ms
 *                     Scanning last duration in every period in ms
 *                     If duration_ms == 0, adv is last until upper disable it
 * @param[in] period_ms
 *                     Scanning period in ms, should be period_ms >= duration_ms
 *                     Ignored when duration_ms == 0
 *
 * @return bt_status_t Set scanning enable successfully or not for some reason
 */
bt_status_t gap_set_ext_scan_enable(gap_dup_filter_op_t filter_duplicates, uint32_t duration_ms, uint32_t period_ms);

/**
 * @brief Enable scanning with filter policy, duration in ms
 *
 * @param[in] filter_duplicates
 *                       Filter duplicated advertising pdu
 * @param[in] duration_ms
 *                       Scanning is expired when duration in ms is arrived
 *
 * @return bt_status_t Enable an durations_ms scanning last successfully or not for some reason
 */
bt_status_t gap_enable_scanning(bool filter_duplicates, uint32_t duration_ms);

/**
 * @brief Disable scanning
 *
 *
 * @return bt_status_t Disable scanning successfully or not for some reason
 */
bt_status_t gap_disable_scanning(void);

/**
 * @brief Get default scannning timing example
 *
 *
 * @return gap_scan_timing_t
 *                       Default scanning timing
 */
gap_scan_timing_t gap_default_scan_timing(void);

/**
 * BLE Initiating
 *
 */
/**
 * @brief Start initiating using specified parameters and event callback
 *
 * @param[in] param    Initiating parameters
 * @param[in] cb       Event callback
 *
 * @return bt_status_t Start initiating successfully or not for some reason
 */
bt_status_t gap_start_initiating(const gap_init_param_t *param, gap_init_callback_t cb);

/**
 * @brief Start extended initiating using specified parameters and event callback
 * 
 * @param[in] param    Initiating parameters
 * @param[in] peer_is_lea
 *                     If peer_is_lea == true, will trigger SMP pairing after connection established
 * @param[in] cb       Event callback
 * 
 * @return bt_status_t Start initiating successfully or not for some reason
 */
bt_status_t gap_start_ext_initiating(const gap_init_param_t *param, bool peer_is_lea, gap_init_callback_t cb);

/**
 * @brief Cancel an initiating
 *
 *
 * @return bt_status_t Cancel an initiating successfully or not for some reason
 */
bt_status_t gap_cancel_initiating(void);

/**
 * @brief Cancel all pending initiating and cancel initiating pending in controller
 *
 * @param[in] also_cancel_curr_initiating
 *                     Cancel initiating pending in controller
 *
 * @return bt_status_t Cancel all pending initiating successfully or not for some reason
 */
bt_status_t gap_cancel_all_pend_initiating(bool also_cancel_curr_initiating);

/**
 * @brief Get default initiating timing example
 *
 *
 * @return gap_init_timing_t
 *                     Default initiating timing
 */
gap_init_timing_t gap_default_init_timing(void);

/**
 * Set Decision Instructions
 *
 *
 */
/**
 * @brief Set decision instructions tests to start decision test and report event
 *
 * @param[in] num_tests
 *                     Number of decision instructions tests
 * @param[in] tests    Decision instructions tests
 *
 * @return bt_status_t Set decision instructions tests successfully or not for some reason
 */
bt_status_t gap_set_decision_instructions(uint8_t num_tests, const gap_decision_test_t *tests);

/**
 * PA Source - Advertiser
 *
 */

/**
 * @brief Set periodic advertising parameters
 *
 * @param[in] adv_handle
 *                     PA related advertising handle
 * @param[in] pa_interval_1_25ms
 *                     PA interval in 1.25ms
 * @param[in] include_tx_power
 *                     Include TX power in pdu
 * @param[in] param    PA parameters
 *
 * @return bt_status_t Set PA parameters successfully or not for some reason
 */
bt_status_t gap_set_pa_parameters(uint8_t adv_handle, uint16_t pa_interval_1_25ms, bool include_tx_power,
                                  const gap_pa_subevent_param_t *param);

/**
 * @brief Set periodic advertising data
 *
 * @param[in] adv_handle
 *                     PA related advertising handle
 * @param[in] op       PA data with operation
 * @param[in] pa_data  PA data
 * @param[in] data_len Length of pa data
 *
 * @return bt_status_t Set PA data with op successfully or not for some reason
 */
bt_status_t gap_set_pa_data(uint8_t adv_handle, gap_adv_data_op_t op, const uint8_t *pa_data, uint16_t data_len);

/**
 * @brief Control periodic advertising enable state
 *
 * @param[in] adv_handle
 *                     PA related advertising handle
 * @param[in] enable   Enable or disable
 * @param[in] include_adi_field
 *                     Enable PA with ADI included
 *
 * @return bt_status_t Control periodic advertising successfully or not for some reason
 */
bt_status_t gap_set_pa_enable(uint8_t adv_handle, bool enable, bool include_adi_field);

/**
 * @brief Set periodic advertising subevent data
 *
 * @param[in] adv_handle
 *                     PA related advertising handle
 * @param[in] num_subevents_with_data
 *                     Number of PA subevent with data
 * @param[in] data     Every subevent data for num_subevents_with_data subevent
 *
 * @return bt_status_t Set periodic advertising subevent data successfully or not for some reason
 */
bt_status_t gap_set_pa_subevent_data(uint8_t adv_handle, uint8_t num_subevents_with_data,
                                     const gap_pa_subevent_data_t *data);

/**
 * @brief Start periodic advertising with response initiating
 *
 * @param[in] adv_handle
 *                     PA related advertising handle
 * @param[in] subevent Specified subevent
 * @param[in] peer_type
 *                     Peer address type
 * @param[in] peer_addr
 *                     Peer BDAddress
 *
 * @return bt_status_t Start PAwR initiating success
 */
bt_status_t gap_advertiser_start_pawr_initiating(uint8_t adv_handle, uint8_t subevent, bt_addr_type_t peer_type,
                                                 const bt_bdaddr_t *peer_addr);

/**
 * PA Sink - Self Create Sync
 *
 */
/**
 * @brief Add PA train to pa list
 *
 * @param[in] adv_addr_type
 *                     PA related address type
 * @param[in] adv_addr PA related BDAddress
 * @param[in] adv_set_id
 *                     PA related adv set ID
 *
 * @return bt_status_t Add PA train to pa list successfully or not
 */
bt_status_t gap_pa_list_add(bt_addr_type_t adv_addr_type, const bt_bdaddr_t *adv_addr, uint8_t adv_set_id);

/**
 * @brief Remove PA train from pa list
 *
 * @param[in] adv_addr_type
 *                     PA related address type
 * @param[in] adv_addr PA related BDAddress
 * @param[in] adv_set_id
 *                     PA related adv set ID
 *
 * @return bt_status_t Remove PA train from pa list successfully or not for some reason
 */
bt_status_t gap_pa_list_remove(bt_addr_type_t adv_addr_type, const bt_bdaddr_t *adv_addr, uint8_t adv_set_id);

/**
 * @brief Clear all PA train from list
 *
 *
 * @return bt_status_t Clear all successfully or not for some reason
 */
bt_status_t gap_pa_list_clear(void);

/**
 * @brief Get PA train list max size
 *
 *
 * @return uint8_t     Max pa list size
 */
uint8_t gap_pa_list_max_size(void);

/**
 * @brief Create PA sync with parameters and event callback
 *
 * @param[in] p_param  PA create sync parameters
 * @param[in] p_cb     Event callback
 *
 * @return bt_status_t PA create sync status
 */
bt_status_t gap_pa_create_sync(const gap_pa_sync_param_t *p_param, const gap_pa_sync_callback_t p_cb);

/**
 * @brief Cancel PA create sync
 *
 *
 * @return bt_status_t Cancel PA create sync send successfully or not for some reason
 */
bt_status_t gap_pa_create_sync_cancel(void);

/**
 * @brief Terminate PA sync
 *
 * @param[in] pa_sync_hdl
 *                     PA sync handle from PA sync established event or PAST
 *
 * @return bt_status_t Terminate PA sync send successfully or not for some reason
 */
bt_status_t gap_pa_terminate_sync(uint16_t pa_sync_hdl);

/**
 * @brief Control PA related event report policy
 *
 * @param[in] pa_sync_hdl
 *                     PA sync handle from PA sync established event or PAST
 * @param[in] enable   Enable or disable data report
 * @param[in] filter_duplicated
 *                     Filter Duplicated enable or disable
 *
 * @return bt_status_t Control PA receive status
 */
bt_status_t gap_set_pa_receive_enable(uint16_t pa_sync_hdl, bool enable, bool filter_duplicated);

/**
 * @brief Set PAwR train subevent to sync with
 *
 * @param[in] pa_sync_hdl
 *                     PA sync handle from PA sync established event or PAST
 * @param[in] include_tx_power
 *                     TX power included
 * @param[in] num_subevents_to_sync
 *                     Number of subevents to sync with
 * @param[in] subevent Subevents to sync with
 *
 * @return bt_status_t Set PAwR train subevent to sync with status
 */
bt_status_t gap_set_pa_sync_subevent(uint16_t pa_sync_hdl, bool include_tx_power, uint8_t num_subevents_to_sync,
                                     const uint8_t *subevent);

/**
 * @brief Set PA response data for subevent specified
 *
 * @param[in] pa_sync_hdl
 *                     PA sync handle from PA sync established event or PAST
 * @param[in] pa_event_counter
 *                     PA event counter
 * @param[in] subevent Subevent specified
 * @param[in] data     PA response data
 *
 * @return bt_status_t Set PA response data successfully or not for some reason
 */
bt_status_t gap_set_pa_response_data(uint16_t pa_sync_hdl, uint16_t pa_event_counter, uint8_t subevent,
                                     const gap_pa_response_data_t *data);

/**
 * PA Sink - Sync from PAST
 *
 */

/**
 * @brief Start PA sync transfer to specified connection local index
 *
 * @param[in] con_idx  Connection local index
 * @param[in] service_data
 *                     Service data from local to peer with PAST event report
 * @param[in] pa_sync_hdl
 *                     PAST operate on PA sync handle
 *
 * @return bt_status_t Start PAST successfully or not for some reason
 */
bt_status_t gap_pa_sync_transfer(uint8_t con_idx, uint16_t service_data, uint16_t pa_sync_hdl);

/**
 * @brief Start PA sync transfer to specified connection local index with local PA related adv set handle
 *
 * @param[in] con_idx  Connection local index
 * @param[in] service_data
 *                     Service data from local to peer with PAST event report
 * @param[in] adv_hdl  Local PA related adv set handle
 *
 * @return bt_status_t Start PAST successfully or not for some reason
 */
bt_status_t gap_pa_set_info_transfer(uint8_t con_idx, uint16_t service_data, uint8_t adv_hdl);

/**
 * @brief Register PAST event received event and PA event callback
 *
 * @param[in] p_cb     PA event callback
 *
 * @return bt_status_t Register successfully or not for some reason
 */
bt_status_t gap_register_past_received_callback(const gap_pa_sync_callback_t p_cb);

/**
 * @brief Set PAST paramterser when received PAST event for specified connection
 *
 * @param[in] con_idx  Connection local index
 * @param[in] mode     PAST mode
 * @param[in] skip     Number of consecutive periodic advertising packets
 *                     that the receiver may skip
 * @param[in] sync_to_10ms
 *                     Maximum permitted time between successful receives in 10ms
 * @param[in] cte_type Whether to only synchronize to periodic advertising
 *                     with certain types of Constant Tone Extension
 *
 * @return bt_status_t Set PAST receive parameters status
 */
bt_status_t gap_set_past_parameters(uint8_t con_idx, uint8_t mode, uint16_t skip, uint16_t sync_to_10ms,
                                    uint8_t cte_type);

/**
 * @brief Set default PAST paramterser when received PAST event
 *
 * @param[in] mode     PAST mode
 * @param[in] skip     Number of consecutive periodic advertising packets
 *                     that the receiver may skip
 * @param[in] sync_to_10ms
 *                     Maximum permitted time between successful receives in 10ms
 * @param[in] cte_type Whether to only synchronize to periodic advertising
 *                     with certain types of Constant Tone Extension
 *
 * @return bt_status_t Set default PAST receive parameters status
 */
bt_status_t gap_set_default_past_parameters(uint8_t mode, uint16_t skip, uint16_t sync_to_10ms, uint8_t cte_type);

/**
 * bes tws fast pair host support apis
 * usage: set enable before le adv/scan
*/

/**
 * @brief TODO:BES FP
 *
 * @param[in] tws_bt_addr...
 * @param[in] peer_le_addr...
 *
 */
void gap_vendor_enable_besfp(uint8_t *tws_bt_addr, uint8_t *peer_le_addr);

/**
 * @brief TODO:BES FP
 *
 *
 */
void gap_vendor_disable_besfp();

/**
 * @brief TODO:BES FP
 *
 *
 * @return true        ...
 * @return false       ...
 */
bool gap_vendor_is_besfp_enabled();

/**
 * @brief TODO:BES FP
 *
 *
 * @return uint8_t*    ...
 */
uint8_t *gap_vender_besfp_get_tws_bt_addr();

/**
 * @brief TODO:BES FP
 *
 *
 * @return uint8_t*    ...
 */
uint8_t *gap_vender_besfp_get_peer_le_addr();

/**
 * LE seemless
 *
 *
 */
/**
 * @brief LE seemless switch link setup complete api to allocated hci connection
 *
 * @param[in] p        Connection setup paramters @see gap_le_switch_link_setup_t
 * @param[in] own_addr_type
 *                     Own address type
 * @param[in] own_addr Own address
 * @param[in] cb       Connection event callback
 *
 * @return bt_status_t Setup connection successfully or not for some reason
 */
bt_status_t gap_le_switch_link_setup_complete(const gap_le_switch_link_setup_t *p,
                                              bt_addr_type_t own_addr_type, const bt_bdaddr_t *own_addr,
                                              gap_adv_callback_t cb);

/**
 * @brief LE seemless switch context save for gap module
 *
 * @param[in] connhdl  Connection handle
 * @param[in] buf      Destination buf for context save
 * @param[in] buf_len  Destination buf left length for context save
 *
 * @return uint32      Used length for context save done
 */
uint32 gap_save_ctx(uint16_t connhdl, uint8_t *buf, uint32_t buf_len);

/**
 * @brief LE seemless switch context restore for gap module
 *
 * @param[in] connhdl  Connection handle
 * @param[in] buf      Source buf start for context restore
 * @param[in] buf_len  Source buf left length for context restore
 *
 * @return uint32      Used length for context restore done
 */
uint32 gap_restore_ctx(uint16_t connhdl, uint8_t *buf, uint32_t buf_len);

#if (mHDT_LE_SUPPORT)
/**
 * @brief Send read mhdt local proprietary features cmd
 *
 *
 */
void gap_mhdt_hci_le_rd_local_proprietary_feat_cmd(void);

/**
 * @brief Send read mhdt remote proprietary features cmd
 *
 * @param[in] conidx   Connection local index
 *
 */
void gap_mhdt_hci_le_rd_remote_proprietary_feat_cmd(uint8_t conidx);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __BT_GAP_SERVICE_H__ */
