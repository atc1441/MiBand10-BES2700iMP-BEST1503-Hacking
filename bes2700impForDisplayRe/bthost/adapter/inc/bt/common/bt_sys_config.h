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
#ifndef __BLUETOOTH_SYS_CONFIG_H__
#define __BLUETOOTH_SYS_CONFIG_H__
#ifdef __cplusplus
extern "C" {
#endif

#ifndef XA_INTEGER_SIZE
#define XA_INTEGER_SIZE     4
#endif

#define DEBUG_BLE_HCI_DBG   0
#define DEBUG_BLE_GAP_DBG   0
#define DEBUG_BLE_GATT_DBG  0
#define DEBUG_BLE_ATT_DBG   0
#define DEBUG_BLE_L2C_DBG   0
#define DEBUG_BLE_SMP_DBG   0
#define DEBUG_BLE_APP_DBG   0
#define DEBUG_BLE_PRF_DBG   0
#define DEBUG_BLE_BLE_DBG   0

#define BTIF_DISABLED       0
#define BTIF_ENABLED        1
#define BTIF_SBC_ENCODER    BTIF_ENABLED
#define BTIF_SBC_DECODER    BTIF_ENABLED
#define BTIF_SPP_CLIENT     BTIF_ENABLED
#define BTIF_SPP_SERVER     BTIF_ENABLED

#define BTIF_BT_DEFAULT_PAGE_SCAN_WINDOW    0x12
#define BTIF_BT_DEFAULT_PAGE_SCAN_INTERVAL  0x800
#define BTIF_BT_DEFAULT_INQ_SCAN_WINDOW     0x12
#define BTIF_BT_DEFAULT_INQ_SCAN_INTERVAL   0x800
#define BTIF_BT_DEFAULT_PAGE_TIMEOUT_IN_MS  5000

#define BTIF_HF_CUSTOM_FEATURE_BATTERY_REPORT    (0x03 << 0)
#define BTIF_HF_CUSTOM_FEATURE_DOCK              (0x01 << 2)
#define BTIF_HF_CUSTOM_FEATURE_SIRI_REPORT       (0x01 << 3)
#define BTIF_HF_CUSTOM_FEATURE_NR_REPORT         (0x01 << 4)
#define BTIF_HF_CUSTOM_FEATURE_SUPPORT \
    (BTIF_HF_CUSTOM_FEATURE_BATTERY_REPORT | BTIF_HF_CUSTOM_FEATURE_SIRI_REPORT)

#define  __BTIF_EARPHONE__

/* for debug usage */
#if !defined(DEBUG)
#define DEBUG 0
#endif

#if DEBUG == 1
#define DBG_DEBUG_PRINT_ENABLE                  1
#define DBG_COBUF_NEED_STATISTIC                1
#else
#define DBG_DEBUG_PRINT_ENABLE                  0
#define DBG_COBUF_NEED_STATISTIC                0
#endif

#define HCI_TX_BUFF_SIZE_CHECK 128
#define HCI_TX_BUFF_SIZE_NO_LESS_COUNT 8

/* config memory block size and count */
#define COBUF_SIZE_N1    8
#define COBUF_SIZE_N2    16
#define COBUF_SIZE_N3    32
#define COBUF_SIZE_N4    48
#define COBUF_SIZE_N5    64
#define COBUF_SIZE_N6    160
#define COBUF_SIZE_N7    ((L2CAP_CFG_MTU+3)/4*4)

#if defined(CHIP_BEST1402) || defined(CHIP_BEST1400)
#define COBUF_NUMS_N1    32
#define COBUF_NUMS_N2    45
#define COBUF_NUMS_N3    39
#define COBUF_NUMS_N4    40
#define COBUF_NUMS_N5    20
#define COBUF_NUMS_N6    10
#define COBUF_NUMS_N7    6
#else
#define COBUF_NUMS_N1    90
#define COBUF_NUMS_N2    90
#define COBUF_NUMS_N3    80
#define COBUF_NUMS_N4    90
#define COBUF_NUMS_N5    60
#define COBUF_NUMS_N6    20
#define COBUF_NUMS_N7    12
#endif

#ifdef BT_USE_COHEAP_ALLOC
#define BT_DYNAMIC_ALLOC_HCI_RX_BUFF
#endif

#define BLE_USE_ENC_DATA_KEY_MATERIAL_CHAR   0
#define BLE_LOCAL_USE_RPA_ONLY_AFTER_BONDING 0

#ifdef BLE_ONLY_ENABLED
#undef __GATT_OVER_BR_EDR__
#undef CTKD_ENABLE
#undef IS_CTKD_OVER_BR_EDR_ENABLED
#endif

#ifdef BLE_PERIPHERAL_ONLY
#define BLE_GAP_CENTRAL_ROLE 0
#else
#define BLE_GAP_CENTRAL_ROLE 1
#endif

#ifdef BLE_GATT_CLIENT_DISABLE
#define BLE_GATT_CLIENT_SUPPORT 0
#else
#define BLE_GATT_CLIENT_SUPPORT 1
#endif

#if BLE_AUDIO_ENABLED
#define BLE_PA_SUPPORT 1
#else
#ifdef BLE_PERIODIC_ADV_DISABLE
#define BLE_PA_SUPPORT 0
#else
#define BLE_PA_SUPPORT 1
#endif
#endif

#if BLE_CHANNEL_SOUDING_ENABLED
#define BLE_GAP_CS_SUPPORT   (1)
#else
#define BLE_GAP_CS_SUPPORT   (0)
#endif

#ifdef CFG_APP_AHP_SERVER
#define BLE_AHP_SERVER_SUPPORT   (1)
#else
#define BLE_AHP_SERVER_SUPPORT   (0)
#endif

#ifdef CFG_APP_AHP_CLIENT
#define BLE_AHP_CLIENT_SUPPORT   (1)
#else
#define BLE_AHP_CLIENT_SUPPORT   (0)
#endif

#if defined(EP_1306_SMALL_CODE) && (!BLE_AUDIO_ENABLED) && (!BLE_ISO_ENABLED)
#undef BLE_GAP_CENTRAL_ROLE
#undef BLE_GATT_CLIENT_SUPPORT
#undef BLE_PA_SUPPORT
#undef BT_LOG_SIMPLIFY
#undef BT_REDUCE_CALLBACK_FUNC
#define BLE_GAP_CENTRAL_ROLE 0
#define BLE_GATT_CLIENT_SUPPORT 0
#define BLE_PA_SUPPORT 0
#define BT_LOG_SIMPLIFY
#ifndef BTH_IN_ROM
#define BT_REDUCE_CALLBACK_FUNC
#endif
#endif

#ifdef BT_OBEX_SUPPORT
#define BT_L2CAP_ENRE_MODE_SUPPORT
#else
#undef BT_L2CAP_ENRE_MODE_SUPPORT
#endif

#undef BT_HFP_TEST_SUPPORT
#ifdef BT_HFP_SUPPORT
#if 0
#define BT_HFP_TEST_SUPPORT
#endif
#endif

#undef BT_A2DP_TEST_SUPPORT
#ifdef BT_A2DP_SUPPORT
#if 0
#define BT_A2DP_TEST_SUPPORT
#endif
#endif

#undef BT_AVRCP_TEST_SUPPORT
#ifdef BT_AVRCP_SUPPORT
#if 0
#define BT_AVRCP_TEST_SUPPORT
#endif
#endif

#undef BT_PBAP_TEST_SUPPORT
#ifdef BT_PBAP_SUPPORT
#if 0
#define BT_PBAP_TEST_SUPPORT
#endif
#endif

#undef BT_MAP_TEST_SUPPORT
#ifdef BT_MAP_SUPPORT
#if 0
#define BT_MAP_TEST_SUPPORT
#endif
#endif

#undef LOCAL_AUDIO_TEST_ENABLE
#ifdef LOCAL_AUDIO_SUPPORT
#if 1
#define LOCAL_AUDIO_TEST_ENABLE
#endif
#endif // LOCAL_AUDIO_SUPPORT

#define ESCO_ENABLE 1

#ifdef CVSD_BYPASS
#define BTM_SYNC_CONN_AUDIO_SETTING_DEFAULT            0x0040
#else
#define BTM_SYNC_CONN_AUDIO_SETTING_DEFAULT            0x0060
#endif

#define BTM_SYNC_CONN_AUDIO_SETTING_IN_CODING_LINEAR   0x0000   /* Linear */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_CODING_ULAW     0x0100   /* u-law */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_CODING_ALAW     0x0200   /* a-law */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_DATA_ONES       0x0000   /* 1's complement */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_DATA_TWOS       0x0040   /* 2's complement */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_DATA_SM         0x0080   /* Sign-Magnitude */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_SAMPLE_8BIT     0x0000   /* 8 bit */
#define BTM_SYNC_CONN_AUDIO_SETTING_IN_SAMPLE_16BIT    0x0020   /* 16 bit */
#define BTM_SYNC_CONN_AUDIO_SETTING_CVSD               0x0000   /* CVSD */
#define BTM_SYNC_CONN_AUDIO_SETTING_ULAW               0x0001   /* u-LAW */
#define BTM_SYNC_CONN_AUDIO_SETTING_ALAW               0x0002   /* A-LAW */
#define BTM_SYNC_CONN_AUDIO_SETTING_TRANSPNT           0x0003   /* msbc */
#define BTM_SYNC_CONN_AUDIO_SETTING_MSBC               0x0060

#define BTM_SYNC_CONN_AUDIO_PARAM_SCO      1
#define BTM_SYNC_CONN_AUDIO_PARAM_S1       2
#define BTM_SYNC_CONN_AUDIO_PARAM_S2       3
#define BTM_SYNC_CONN_AUDIO_PARAM_S3       4
#define BTM_SYNC_CONN_AUDIO_PARAM_S4       5
#define BTM_SYNC_CONN_AUDIO_PARAM_CUSTOM   6
#define BTM_SYNC_CONN_AUDIO_PARAM_T1       7
#define BTM_SYNC_CONN_AUDIO_PARAM_T2       8

#define BTM_SYNC_CONN_AUDIO_DEFAULT_PARMS   BTM_SYNC_CONN_AUDIO_PARAM_S4

/* HFP */
#define HFP_CMD_FLOW_CONTROL_ENABLE        1
#define HFP_CMD_SYST_TX_TIMEOUT_VAL_MS     3000

#define HFP_HF_CHANNEL 7

#define HFP_HF_FEAT_ECNR        0x00000001
#define HFP_HF_FEAT_3WAY        0x00000002
#define HFP_HF_FEAT_CLI         0x00000004
#define HFP_HF_FEAT_VR          0x00000008
#define HFP_HF_FEAT_RVC         0x00000010
#define HFP_HF_FEAT_ECS         0x00000020
#define HFP_HF_FEAT_ECC         0x00000040
#define HFP_HF_FEAT_CODEC       0x00000080
#define HFP_HF_FEAT_HF_IND      0x00000100
#define HFP_HF_FEAT_ESCO_S4_T2  0x00000200

#define HFP_HF_SDP_FEAT_MASK    0x001f
#define HFP_HF_SDP_FEAT_WBS     0x0020
#if defined(HFP_SUPPORT_LC3_SWB)
#define HFP_HF_SDP_FEAT_LC3_SWB 0x0100
#endif

#define HFP_AG_FEAT_3WAY        0x00000001
#define HFP_AG_FEAT_ECNR        0x00000002
#define HFP_AG_FEAT_VR          0x00000004
#define HFP_AG_FEAT_INBAND      0x00000008
#define HFP_AG_FEAT_VTAG        0x00000010
#define HFP_AG_FEAT_REJ_CALL    0x00000020
#define HFP_AG_FEAT_ECS         0x00000040
#define HFP_AG_FEAT_ECC         0x00000080
#define HFP_AG_FEAT_EXT_ERR     0x00000100
#define HFP_AG_FEAT_CODEC       0x00000200
#define HFP_AG_FEAT_HF_IND      0x00000400
#define HFP_AG_FEAT_ESCO_S4_T2  0x00000800

#define HFP_AG_SDP_FEAT_MASK    0x001f
#define HFP_AG_SDP_FEAT_WBS     0x0020
#if defined(HFP_SUPPORT_LC3_SWB)
#define HFP_AG_SDP_FEAT_LC3_SWB 0x0100
#endif

#if defined(HFP_1_9_ENABLE)
#define HFP_HF_FEATURES ( \
                HFP_HF_FEAT_CLI   | \
                HFP_HF_FEAT_RVC   | \
                HFP_HF_FEAT_ECS   | \
                HFP_HF_FEAT_ECC   | \
                HFP_HF_FEAT_CODEC | \
                HFP_HF_FEAT_ECNR  | \
                HFP_HF_FEAT_3WAY  | \
                HFP_HF_FEAT_VR    | \
                HFP_HF_FEAT_ESCO_S4_T2 \
                )
#if defined(HFP_SUPPORT_LC3_SWB)
#define HFP_HF_SDP_FEATURES ((HFP_HF_FEATURES & HFP_HF_SDP_FEAT_MASK) | HFP_HF_SDP_FEAT_WBS | HFP_HF_SDP_FEAT_LC3_SWB)
#else
#define HFP_HF_SDP_FEATURES ((HFP_HF_FEATURES & HFP_HF_SDP_FEAT_MASK) | HFP_HF_SDP_FEAT_WBS)
#endif

#define HFP_AG_FEATURES ( \
                HFP_AG_FEAT_3WAY     | \
                HFP_AG_FEAT_ECNR     | \
                HFP_AG_FEAT_REJ_CALL | \
                HFP_AG_FEAT_ECS      | \
                HFP_AG_FEAT_CODEC    | \
                HFP_AG_FEAT_ESCO_S4_T2 \
                )
#if defined(HFP_SUPPORT_LC3_SWB)
#define HFP_AG_SDP_FEATURES ((HFP_AG_FEATURES & HFP_AG_SDP_FEAT_MASK) | HFP_AG_SDP_FEAT_WBS | HFP_AG_SDP_FEAT_LC3_SWB)
#else
#define HFP_AG_SDP_FEATURES ((HFP_AG_FEATURES & HFP_AG_SDP_FEAT_MASK) | HFP_AG_SDP_FEAT_WBS)
#endif
#else
#define HFP_HF_FEATURES ( \
                HFP_HF_FEAT_CLI   | \
                HFP_HF_FEAT_RVC   | \
                HFP_HF_FEAT_ECS   | \
                HFP_HF_FEAT_ECC     \
                )
#define HFP_HF_SDP_FEATURES (HFP_HF_FEATURES & HFP_HF_SDP_FEAT_MASK)

#define HFP_AG_FEATURES ( \
                HFP_AG_FEAT_REJ_CALL | \
                HFP_AG_FEAT_ECS      | \
                )
#define HFP_AG_SDP_FEATURES ((HFP_AG_FEATURES & HFP_AG_SDP_FEAT_MASK) | HFP_AG_SDP_FEAT_WBS)
#endif

#define MAX_SDP_SOCK_NUMBER                      2

#define NUM_SCO_CONNS                            2
#define NUM_BLE_DEVICES                          BLE_CONNECTION_MAX

#define CFG_VOICE_SETTING_DEFAULT                0x0043

#define CFG_HCI_EVT_DATA_SIZE                    260

#define BTM_DEVICEDB_SIZE                        6
#define BTM_PINCODE_MAX_LEN                      8    /*include '\0'*/


#define BTM_CFG_TIMEOUT_BEFORE_LOWPOWER               5
#define BTM_INQUIRY_RESULT_UNUSED_TIMEOUT             30   /*seconds, decide the time out to destory an inquiry result structure*/
#define BTM_DISCONN_WAITING_TIME                      1    /* changed by marvin.zhu. seconds, decide the timeout be disconnect an acl link after it is not used by l2cap*/

/* audio connection config */
#define HCI_CFG_SYNC_TX_BANDWIDTH            0x00001F40
#define HCI_CFG_SYNC_RX_BANDWIDTH            0x00001F40
#define HCI_CFG_SYNC_MAX_LATENCY             0xffff
#define HCI_CFG_SYNC_RETX_EFFORT             0x2
//#define HCI_CFG_SYNC_PKT_TYPE              HCI_PKT_TYPE_HV3
#if ESCO_ENABLE
#define HCI_CFG_SYNC_SCO_PKTS                (PACKET_TYPE_HV1 | PACKET_TYPE_HV2 | PACKET_TYPE_HV3)
#define HCI_CFG_SYNC_PKT_TYPE                (PACKET_TYPE_HV1 | PACKET_TYPE_HV2 | PACKET_TYPE_HV3 | PACKET_TYPE_EV3)
#else
#define HCI_CFG_SYNC_PKT_TYPE                (PACKET_TYPE_HV1_FLAG | PACKET_TYPE_HV2_FLAG | PACKET_TYPE_HV3_FLAG)
#endif
#define HCI_CFG_INPUT_CODING                 INPUT_CODING_LINEAR
#define HCI_CFG_INPUT_DATA_FORMAT            INPUT_DATA_FORMAT_1S
#define HCI_CFG_INPUT_SAMPLE_SIZE            INPUT_SAMPLE_SIZE_8BITS
#if defined(HFP_1_9_ENABLE)
#define HCI_CFG_AIR_CODING_FORMAT            AIR_CODING_FORMAT_MSBC
#else
#define HCI_CFG_AIR_CODING_FORMAT            AIR_CODING_FORMAT_CVSD
#endif
#define HCI_CFG_LINEAR_PCM_BITPOS            0x00

#define HCI_CFG_VOICE_SETTING               (HCI_CFG_INPUT_CODING&  \
                                             HCI_CFG_INPUT_DATA_FORMAT&  \
                                             HCI_CFG_INPUT_SAMPLE_SIZE&  \
                                             HCI_CFG_AIR_CODING_FORMAT& \
                                             (HCI_CFG_LINEAR_PCM_BITPOS<<2))

#define HCI_CFG_SNIFF_MAX_INTERVAL           0x0320
#define HCI_CFG_SNIFF_MIN_INTERVAL           0x0300
#define HCI_CFG_SNIFF_ATTEMPT                0x08
#define HCI_CFG_SNIFF_TIMEOUT                0x00

#define HCI_CFG_LINK_POLICY_LOWPOWER         HCI_LP_ENABLE_ROLE_SWITCH_MASK \
                                             |HCI_LP_ENABLE_SNIFF_MODE_MASK


/* 10*1.28 seconds */
#define HCI_CFG_INQUIRY_TIMEOUT              10
/* unlimited */
#define HCI_CFG_INQUIRY_RESPONSE_NUM         0

#define CFG_DEFAULT_PAGE_TIMEOUT             0x2000


// COD Masks - isolate components of the class of device
#define CFG_COD_SERVICE_MASK              0x00ffC000     /* Less LIAC bit */
#define CFG_COD_LIMITED_DISC_MASK         0x00002000     /* LIAC bit */
#define CFG_COD_MAJOR_MASK                0x00001F00
#define CFG_COD_MINOR_MASK                0x000000FC

// Major service classes (can be OR together)
#define CFG_COD_LIMITED_DISCOVERABLE_MODE    0x00002000
#define CFG_COD_MAJOR_LE_AUDIO               0x00004000
#define CFG_COD_POSITIONING                  0x00010000
#define CFG_COD_NETWORKING                   0x00020000
#define CFG_COD_RENDERING                    0x00040000
#define CFG_COD_CAPTURING                    0x00080000
#define CFG_COD_OBJECT_TRANSFER              0x00100000
#define CFG_COD_AUDIO                        0x00200000
#define CFG_COD_TELEPHONY                    0x00400000
#define CFG_COD_INFORMATION                  0x00800000

// Major device classes (select one)
#define CFG_COD_MAJOR_MISCELLANEOUS          0x00000000
#define CFG_COD_MAJOR_COMPUTER               0x00000100
#define CFG_COD_MAJOR_PHONE                  0x00000200
#define CFG_COD_MAJOR_LAN_ACCESS_POINT       0x00000300
#define CFG_COD_MAJOR_AUDIO                  0x00000400
#define CFG_COD_MAJOR_PERIPHERAL             0x00000500
#define CFG_COD_MAJOR_IMAGING                0x00000600
#define CFG_COD_MAJOR_WEARABLE               0x00000700
#define CFG_COD_MAJOR_TOY                    0x00000800
#define CFG_COD_MAJOR_HEALTH                 0x00000900
#define CFG_COD_MAJOR_UNCLASSIFIED           0x00001F00
#define CFG_COD_MAJOR_LE_AUDIO               0x00004000


// Minor device class for CFG_COD_MAJOR_COMPUTER
#define CFG_COD_MINOR_COMP_UNCLASSIFIED     0x00000000
#define CFG_COD_MINOR_COMP_DESKTOP          0x00000004
#define CFG_COD_MINOR_COMP_SERVER           0x00000008
#define CFG_COD_MINOR_COMP_LAPTOP           0x0000000C
#define CFG_COD_MINOR_COMP_HANDHELD         0x00000010
#define CFG_COD_MINOR_COMP_PALM             0x00000014
#define CFG_COD_MINOR_COMP_WEARABLE         0x00000018

// Minor device class for CFG_COD_MAJOR_PHONE
#define CFG_COD_MINOR_PHONE_UNCLASSIFIED    0x00000000
#define CFG_COD_MINOR_PHONE_CELLULAR        0x00000004
#define CFG_COD_MINOR_PHONE_CORDLESS        0x00000008
#define CFG_COD_MINOR_PHONE_SMART           0x0000000C
#define CFG_COD_MINOR_PHONE_MODEM           0x00000010
#define CFG_COD_MINOR_PHONE_ISDN            0x00000014

// Minor device class for CFG_COD_MAJOR_AUDIO
#define CFG_COD_MINOR_AUDIO_UNCLASSIFIED     0x00000000
#define CFG_COD_MINOR_AUDIO_HEADSET          0x00000004
#define CFG_COD_MINOR_AUDIO_HANDSFREE        0x00000008
#define CFG_COD_MINOR_AUDIO_MICROPHONE       0x00000010
#define CFG_COD_MINOR_AUDIO_LOUDSPEAKER      0x00000014
#define CFG_COD_MINOR_AUDIO_HEADPHONES       0x00000018
#define CFG_COD_MINOR_AUDIO_PORTABLEAUDIO    0x0000001C
#define CFG_COD_MINOR_AUDIO_CARAUDIO         0x00000020
#define CFG_COD_MINOR_AUDIO_SETTOPBOX        0x00000024
#define CFG_COD_MINOR_AUDIO_HIFIAUDIO        0x00000028
#define CFG_COD_MINOR_AUDIO_VCR              0x0000002C
#define CFG_COD_MINOR_AUDIO_VIDEOCAMERA      0x00000030
#define CFG_COD_MINOR_AUDIO_CAMCORDER        0x00000034
#define CFG_COD_MINOR_AUDIO_VIDEOMONITOR     0x00000038
#define CFG_COD_MINOR_AUDIO_VIDEOSPEAKER     0x0000003C
#define CFG_COD_MINOR_AUDIO_CONFERENCING     0x00000040
#define CFG_COD_MINOR_AUDIO_GAMING           0x00000048

// Minor device class for CFG_COD_MAJOR_PERIPHERAL
#define CFG_COD_MINOR_PERIPH_NOT_KEYBOARD_NOT_POINTING_DEVICE   0x00
#define CFG_COD_MINOR_PERIPH_KEYBOARD                           0x40
#define CFG_COD_MINOR_PERIPH_POINTING_DEVICE                    0x80
#define CFG_COD_MINOR_PERIPH_COMBO_KEYBOARD_POINTING_DEVICE     0xC0

// Minor device class can be ORed with Peripheral Minor Device class
#define CFG_COD_MINOR_PERIPH_UNCATEGORIZED_DEVICE           0x00
#define CFG_COD_MINOR_PERIPH_JOYSTICK                       0x04
#define CFG_COD_MINOR_PERIPH_GAMEPAD                        0x08
#define CFG_COD_MINOR_PERIPH_REMOTE_CONTROL                 0x0C
#define CFG_COD_MINOR_PERIPH_SENSING_DEVICE                 0x10
#define CFG_COD_MINOR_PERIPH_DIGITIZER_TABLET               0x14
#define CFG_COD_MINOR_PERIPH_CARD_READER                    0x18
#define CFG_COD_MINOR_PERIPH_DIGITAL_PEN                    0x1C
#define CFG_COD_MINOR_PERIPH_HANDHELD_SCANNER               0x20
#define CFG_COD_MINOR_PERIPH_HANDHELD_GESTURAL_INPUT_DEVICE 0x24

// Minor device class for CFG_COD_MAJOR_IMAGING
#define CFG_COD_MINOR_IMAGE_UNCLASSIFIED  0x00000000
#define CFG_COD_MINOR_IMAGE_DISPLAY       0x00000010
#define CFG_COD_MINOR_IMAGE_CAMERA        0x00000020
#define CFG_COD_MINOR_IMAGE_SCANNER       0x00000040
#define CFG_COD_MINOR_IMAGE_PRINTER       0x00000080

// Minor device class for CFG_COD_MAJOR_WEARABLE
#define CFG_COD_MINOR_WEARABLE_WRISTWATCH    0x00000004
#define CFG_COD_MINOR_WEARABLE_PAGER         0x00000008
#define CFG_COD_MINOR_WEARABLE_JACKET        0x0000000C
#define CFG_COD_MINOR_WEARABLE_HELMET        0x00000010
#define CFG_COD_MINOR_WEARABLE_GLASSES       0x00000014
#define CFG_COD_MINOR_WEARABLE_PIN           0x00000018

// Minor device class for CFG_COD_MAJOR_TOY
#define CFG_COD_MINOR_TOY_ROBOT        0x00000004
#define CFG_COD_MINOR_TOY_VEHICLE      0x00000008
#define CFG_COD_MINOR_TOY_DOLL         0x0000000C
#define CFG_COD_MINOR_TOY_CONTROLLER   0x00000010
#define CFG_COD_MINOR_TOY_GAME         0x00000014

// Minor device class for CFG_COD_MAJOR_HEALTH
#define CFG_COD_MINOR_HEALTH_UNDEFINED                   0x00000000
#define CFG_COD_MINOR_HEALTH_BLOOD_PRESSURE_MONITOR      0x00000004
#define CFG_COD_MINOR_HEALTH_THERMOMETER                 0x00000008
#define CFG_COD_MINOR_HEALTH_WEIGHING_SCALE              0x0000000C
#define CFG_COD_MINOR_HEALTH_GLUCOSE_METER               0x00000010
#define CFG_COD_MINOR_HEALTH_PULSE_OXIMETER              0x00000014
#define CFG_COD_MINOR_HEALTH_PULSE_RATE_MONITOR          0x00000018
#define CFG_COD_MINOR_HEALTH_HEALTH_DATA_DISPLAY         0x0000001C
#define CFG_COD_MINOR_HEALTH_STEP_COUNTER                0x00000020
#define CFG_COD_MINOR_HEALTH_BODY_COMPOSITION_ANALYZER   0x00000024
#define CFG_COD_MINOR_HEALTH_PEAK_FLOW_MONITOR           0x00000028
#define CFG_COD_MINOR_HEALTH_MEDICATION_MONITOR          0x0000002C
#define CFG_COD_MINOR_HEALTH_KNEE_PROSTHESIS             0x00000030
#define CFG_COD_MINOR_HEALTH_ANKLE_PROSTHESIS            0x00000034
#define CFG_COD_MINOR_HEALTH_GENERIC_HEALTH_MANAGER      0x00000038
#define CFG_COD_MINOR_HEALTH_PERSONAL_MOBILITY_DEVICE    0x0000003C

#ifdef _SCO_BTPCM_CHANNEL_
#define CFG_SYNC_CONFIG_PATH                 (0<<8|1<<4|1<<0) /* all links use hci */
#else
#define CFG_SYNC_CONFIG_PATH                 (0<<8|0<<4|0<<0) /* all links use hci */
#endif
#define CFG_SYNC_CONFIG_MAX_BUFFER           (0) /* (e)sco use Packet size */
#ifdef CVSD_BYPASS
#define CFG_SYNC_CONFIG_CVSD_BYPASS          (1) /* use pcm over hci */
#else
#define CFG_SYNC_CONFIG_CVSD_BYPASS          (0) /* use pcm over hci */
#endif

#define CFG_LP_SNIFF_MODE                    0x0000

#define IS_EDR_ENABLED 0

#define CFG_BT_DEFAULT_PAGE_SCAN_WINDOW      0x12
#define CFG_BT_DEFAULT_PAGE_SCAN_INTERVAL    0x800
#define CFG_BT_DEFAULT_INQ_SCAN_WINDOW       0x12
#define CFG_BT_DEFAULT_INQ_SCAN_INTERVAL     0x800

#define CFG_INQ_TYPE    1
#define CFG_PAGE_TYPE   1

/* l2cap */
#define SSP_RECONNECT              1
#define L2CAP_AUTH_NEED_ENCRYPTION 1/* default value in spec */

#define L2CAP_LE_MIN_MTU                                            23
#define L2CAP_MIN_MTU                                               48
#define L2CAP_ENFC_MIN_MTU                                          64

#define L2CAP_MHDT_CFG_MTU                                          2820
#define L2CAP_LE_CFG_MTU                                            251 // shall equal to HCI_LE_ACL_TX_DATA_SIZE
#if defined(__3M_PACK__)
#define L2CAP_CFG_MTU                                               1021
#else
#define L2CAP_CFG_MTU                                               679
#endif

#define RFCOMM_PPB_HEAD_RESERVE 8
#define RFCOMM_PPB_TAIL_RESERVE 2
#define RFCOMM_PPB_RESERVE (RFCOMM_PPB_HEAD_RESERVE + RFCOMM_PPB_TAIL_RESERVE)
#define RFCOMM_DEFAULT_MTU (L2CAP_CFG_MTU - RFCOMM_PPB_RESERVE)

#define L2CAP_DEFAULT_FLUSH_TO                                      0xFFFF

#define L2CAP_DEFAULT_QOS_SERVICE_TYPE                              L2CAP_QOS_BEST_EFFORT
#define L2CAP_DEFAULT_QOS_TOKEN_RATE                                0x00000000
#define L2CAP_DEFAULT_QOS_TOKEN_BUCKET_SIZE                         0x00000000
#define L2CAP_DEFAULT_QOS_PEEK_BANDWIDTH                            0x00000000
#define L2CAP_DEFAULT_QOS_LATENCY                                   0xffffffff
#define L2CAP_DEFAULT_QOS_DELAY_VARIATION                           0xffffffff


#define L2CAP_DEFAULT_RFC_MODE                                      L2CAP_MODE_BASE
#define L2CAP_DEFAULT_RFC_TXWINDOW                                  32      /*1 to 32*/
#define L2CAP_DEFAULT_RFC_MAXTRANSMIT                               32
#define L2CAP_DEFAULT_RFC_RETRANSMISSION_TIMEOUT                    1000
#define L2CAP_DEFAULT_RFC_MONITOR_TIMEOUT                           1000
#define L2CAP_DEFAULT_RFC_MPS                                       0xFFFF

#define L2CAP_DEFAULT_FCS_TYPE      L2CAP_FCS_TYPE_16_BIT

/* used for pp_buff */


#define L2CAP_CFG_FLUSH_TO                                           L2CAP_DEFAULT_FLUSH_TO

#define L2CAP_CFG_QOS_SERVICE_TYPE                                   L2CAP_DEFAULT_QOS_SERVICE_TYPE
#define L2CAP_CFG_QOS_TOKEN_RATE                                     0x00000000
#define L2CAP_CFG_QOS_TOKEN_BUCKET_SIZE                              0x00000000
#define L2CAP_CFG_QOS_PEEK_BANDWIDTH                                 0x00000000
#define L2CAP_CFG_QOS_LATENCY                                        0xffffffff
#define L2CAP_CFG_QOS_DELAY_VARIATION                                0xffffffff

#define L2CAP_CFG_RFC_MODE                                           L2CAP_DEFAULT_RFC_MODE                   /*L2CAP_MODE_BASE*/
/* below is only available when rfc mode is not base mode*/
#define L2CAP_CFG_RFC_TXWINDOW                                       32      /*1 to 32*/
#define L2CAP_CFG_RFC_MAXTRANSMIT                                    32
#define L2CAP_CFG_RFC_RETRANSMISSION_TIMEOUT                         1000
#define L2CAP_CFG_RFC_MONITOR_TIMEOUT                                1000
#define L2CAP_CFG_RFC_MPS                                            0xFFFF

/* L2CAP_RTX: The Responsive Timeout eXpired timer is used to terminate
   the channel when the remote endpoint is unresponsive to signalling
   requests (min 1s, max 60s) */
#define L2CAP_CFG_RTX 60
/* L2CAP_RTX_MAXN: Maximum number of Request retransmissions before
   terminating the channel identified by the request. The decision
   should be based on the flush timeout of the signalling link. If the
   flush timeout is infinite, no retransmissions should be performed */
#define L2CAP_CFG_RTX_MAXN 0
/* L2CAP_ERTX: The Extended Response Timeout eXpired timer is used in
   place of the RTC timer when a L2CAP_ConnectRspPnd event is received
   (min 60s, max 300s) */
#define L2CAP_CFG_ERTX 300

#if !defined(BLE_ONLY_ENABLED)
#define  __BTIF_BT_RECONNECT__
#endif

#define __BTIF_SNIFF__

#ifdef __cplusplus
}
#endif
#endif /* __BLUETOOTH_SYS_CONFIG_H__ */
