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
#ifndef __ME_COMMON_DEFINE_H__
#define __ME_COMMON_DEFINE_H__
#include "bt_common_define.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t btif_event_type_t;

#define BTIF_BTEVENT_INQUIRY_RESULT                     1
#define BTIF_BTEVENT_INQUIRY_COMPLETE                   2
#define BTIF_BTEVENT_INQUIRY_CANCELED                   3
#define BTIF_BTEVENT_LINK_CONNECT_IND                   4
#define BTIF_BTEVENT_SCO_CONNECT_IND                    5
#define BTIF_BTEVENT_LINK_DISCONNECT                    6
#define BTIF_BTEVENT_LINK_CONNECT_CNF                   7
#define BTIF_BTEVENT_ROLE_DISCOVERED                    8
#define BTIF_BTEVENT_MODE_CHANGE                        9
#define BTIF_BTEVENT_ACCESSIBLE_CHANGE                  10
#define BTIF_BTEVENT_AUTHENTICATED                      11
#define BTIF_BTEVENT_ENCRYPTION_CHANGE                  12
#define BTIF_BTEVENT_SECURITY_CHANGE                    13
#define BTIF_BTEVENT_ROLE_CHANGE                        14
#define BTIF_BTEVENT_SCO_DISCONNECT                     15
#define BTIF_BTEVENT_SCO_CONNECT_CNF                    16
#define BTIF_BTEVENT_SIMPLE_PAIRING_COMPLETE            17
#define BTIF_BTEVENT_REMOTE_FEATURES                    18
#define BTIF_BTEVENT_REM_HOST_FEATURES                  19
#define BTIF_BTEVENT_LINK_SUPERV_TIMEOUT_CHANGED        20
#define BTIF_BTEVENT_SET_SNIFF_SUBRATING_PARMS_CNF      21
#define BTIF_BTEVENT_SNIFF_SUBRATE_INFO                 22
#define BTIF_BTEVENT_SET_INQUIRY_MODE_CNF               23
#define BTIF_BTEVENT_SET_INQ_TX_PWR_LVL_CNF             24
#define BTIF_BTEVENT_SET_EXT_INQUIRY_RESP_CNF           25
#define BTIF_BTEVENT_SET_ERR_DATA_REPORTING_CNF         26
#define BTIF_BTEVENT_KEY_PRESSED                        27
#define BTIF_BTEVENT_QOS_SETUP_COMPLETE                 28

#ifdef __TWS_RECONNECT_USE_BLE__
#define BTIF_BTEVENT_TWS_BLE_ADV_REPORT_EVENT           29
#endif /* __TWS_RECONNECT_USE_BLE__ */

/* Indicates the name of a remote device or cancellation of a name request. */
#define BTIF_BTEVENT_NAME_RESULT                        30
#define BTIF_BTEVENT_SCO_DATA_IND                       31
/**
 * Outgoing SCO data has been sent and the packet is free for re-use by
 *  the application.
 */
#define BTIF_BTEVENT_SCO_DATA_CNF                       32
#define BTIF_BTEVENT_LINK_CONNECT_REQ                   33
/* Incoming link accept complete.  */
#define BTIF_BTEVENT_LINK_ACCEPT_RSP                    34
/* Incoming link reject complete. . */
#define BTEVENT_LINK_REJECT_RSP                         35
#define BTIF_BTEVENT_COMMAND_COMPLETE                   36
#define BTIF_BTEVENT_SCO_CONNECT_REQ                    37
/* Set Audio/Voice settings complete.  */
#define BTIF_BTEVENT_SCO_VSET_COMPLETE                  38
/* SCO link connection process started. */
#define BTIF_BTEVENT_SCO_STARTED                        39
/* Select Device operation complete, "p.select" is valid. */
#define BTIF_BTEVENT_DEVICE_SELECTED                    40
/* The eSCO connection has changed. "p.scoConnect" is valid. */
#define BTIF_BTEVENT_SCO_CONN_CHNG                      41

/* Group: Security-related events. */

/* Indicates access request is successful. "p.secToken" is valid. */
#define BTIF_BTEVENT_ACCESS_APPROVED                    50

/* Indicates access request failed. "p.secToken" is valid. */
#define BTIF_BTEVENT_ACCESS_DENIED                      51
/**
 * Request authorization when "errCode" is BEC_NO_ERROR.
 * "p.remDev" is valid.
 */
#define BTIF_BTEVENT_AUTHORIZATION_REQ                  52
/**
 * Request a Pin for pairing when "errCode" is BEC_NO_ERROR.
 * "p.pinReq" is valid. If p.pinReq.pinLen is > 0 then SEC_SetPin()
 * must be called in response to this event with a pin length >=
 * p.pinReq.pinLen.
 */
#define BTIF_BTEVENT_PIN_REQ                            53
/* Pairing operation is complete. */
#define BTIF_BTEVENT_PAIRING_COMPLETE                   54
/* Authentication operation complete. "p.remDev" is valid. */
#define BTIF_BTEVENT_AUTHENTICATE_CNF                   55
/* Encryption operation complete. "p.remDev" is valid. */
#define BTIF_BTEVENT_ENCRYPT_COMPLETE                   56
/* Security mode 3 operation complete. "p.secMode" is valid. */
#define BTIF_BTIF_BTEVENT_SECURITY3_COMPLETE            57
/* A link key is returned. "p.bdLinkKey" is valid.  */
#define BTIF_BTEVENT_RETURN_LINK_KEYS                   58
/* Out of Band data has been received from the host controller. */
#define BTIF_BTEVENT_LOCAL_OOB_DATA                     59
/**
 * Request a Pass Key for simple pairing when "errCode" is BEC_NO_ERROR. The
 * application should call SEC_SetPassKey() to provide the passkey or reject
 * the request, and optionally save the link key.
 */
#define BTIF_BTEVENT_PASS_KEY_REQ                       60
/**
 * Request a User Confirmation for simple pairing when "errCode" is
 * BEC_NO_ERROR.
 */
#define BTIF_BTEVENT_CONFIRM_NUMERIC_REQ                61
#define BTIF_BTEVENT_DISPLAY_NUMERIC_IND                62
#define BTIF_BTEVENT_CONN_PACKET_TYPE_CHNG              63
#define BTIF_SDEVENT_QUERY_RSP                          70
#define BTIF_SDEVENT_QUERY_ERR                          71
#define BTIF_SDEVENT_QUERY_FAILED                       72
#define BTIF_BTEVENT_SELECT_DEVICE_REQ                  80
#define BTIF_BTEVENT_DEVICE_ADDED                       81
#define BTIF_BTEVENT_DEVICE_DELETED                     82
#define BTIF_BTEVENT_MAX_SLOT_CHANGED                   83
#define BTIF_BTEVENT_SNIFFER_CONTROL_DONE               84
#define BTIF_BTEVENT_LINK_POLICY_CHANGED                85
#define BTIF_BTEVENT_DEFAULT_LINK_POLICY_CHANGED        86
#define BTIF_BTEVENT_USER_PASSKEY_NOTIFY_EVENT          89
#define BTIF_BTEVENT_USER_PASSKEY_REQUEST_EVENT         90
#define BTIF_BTEVENT_IBRT_CONNECT_EVENT                 91
#define BTIF_BTEVENT_IBRT_DISCONNECT_EVENT              92
#define BTIF_BTEVENT_SET_BT_BLE_ACTIVE_LINK_COMPLETE    93
/**
 * an ACL connection has received an internal data transmit
 * request while it is in hold, park or sniff mode. The data will still be
 * passed to the radio in park and sniff modes. However, hold mode will
 * block data transmit. It may be necessary to return the ACL to active
 * mode to restore normal data transfer.
 */
#define BTIF_BTEVENT_ACL_DATA_NOT_ACTIVE                99
/**
 * Indicate that an ACL connection is sending or receiving data
 * while it is in active mode. Then, keep resetting the sniff timer.
 */
#define BTIF_BTEVENT_ACL_DATA_ACTIVE                    98
/* Indicates that the HCI failed to initialize. */
#define BTIF_BTEVENT_HCI_INIT_ERROR                     100
#define BTIF_BTEVENT_HCI_INITIALIZED                    101
/* Indicates that a fatal error has occurred in the radio or the HCI transport. */
#define BTIF_BTEVENT_HCI_FATAL_ERROR                    102
/* Indicates that the HCI has been deinitialized. */
#define BTIF_BTEVENT_HCI_DEINITIALIZED                  103
/* Indicates that the HCI cannot be initialized. */
#define BTIF_BTEVENT_HCI_FAILED                         104
#define BTIF_BTEVENT_HCI_COMMAND_SENT                   105

#define BTIF_BTEVENT_BES_AUD_CONNECTED                  200
#define BTIF_BTEVENT_BES_AUD_DISCONNECTED               201
#define BTIF_STACK_LINK_DISCONNECT_COMPLETE             220


/*---------------------------------------------------------------------------
 * btif_class_of_device_t type
 *
 *     Bit pattern representing the class of device along with the
 *     supported services. There can be more than one supported service.
 *     Service classes can be ORed together. The Device Class is composed
 *     of a major device class plus a minor device class. ORing together
 *     each service class plus one major device class plus one minor device
 *     class creates the class of device value. The minor device class is
 *     interpreted in the context of the major device class.
 */

typedef uint32_t btif_class_of_device_t;

/* Group: Major Service Classes. Can be ORed together */
#define BTIF_COD_LIMITED_DISCOVERABLE_MODE 0x00002000
#define BTIF_COD_POSITIONING               0x00010000
#define BTIF_COD_NETWORKING                0x00020000
#define BTIF_COD_RENDERING                 0x00040000
#define BTIF_COD_CAPTURING                 0x00080000
#define BTIF_COD_OBJECT_TRANSFER           0x00100000
#define BTIF_COD_AUDIO                     0x00200000
#define BTIF_COD_TELEPHONY                 0x00400000
#define BTIF_COD_INFORMATION               0x00800000

/* Group: Major Device Classes (Select one) */
#define BTIF_COD_MAJOR_MISCELLANEOUS       0x00000000
#define BTIF_COD_MAJOR_COMPUTER            0x00000100
#define BTIF_COD_MAJOR_PHONE               0x00000200
#define BTIF_COD_MAJOR_LAN_ACCESS_POINT    0x00000300
#define BTIF_COD_MAJOR_AUDIO               0x00000400
#define BTIF_COD_MAJOR_PERIPHERAL          0x00000500
#define BTIF_COD_MAJOR_IMAGING             0x00000600
#define BTIF_COD_MAJOR_UNCLASSIFIED        0x00001F00

/* Group: Minor Device Class - Computer Major class */
#define BTIF_COD_MINOR_COMP_UNCLASSIFIED   0x00000000
#define BTIF_COD_MINOR_COMP_DESKTOP        0x00000004
#define BTIF_COD_MINOR_COMP_SERVER         0x00000008
#define BTIF_COD_MINOR_COMP_LAPTOP         0x0000000C
#define BTIF_COD_MINOR_COMP_HANDHELD       0x00000010
#define BTIF_COD_MINOR_COMP_PALM           0x00000014
#define BTIF_COD_MINOR_COMP_WEARABLE       0x00000018
#define BTIF_COD_MINOR_COMP_TABLET         0x0000001C

/* Group: Minor Device Class - Phone Major class */
#define BTIF_COD_MINOR_PHONE_UNCLASSIFIED  0x00000000
#define BTIF_COD_MINOR_PHONE_CELLULAR      0x00000004
#define BTIF_COD_MINOR_PHONE_CORDLESS      0x00000008
#define BTIF_COD_MINOR_PHONE_SMART         0x0000000C
#define BTIF_COD_MINOR_PHONE_MODEM         0x00000010
#define BTIF_COD_MINOR_PHONE_ISDN          0x00000014

/* Group: Minor Device Class - LAN Access Point Major class */
#define BTIF_COD_MINOR_LAN_0               0x00000000   /* fully available */
#define BTIF_COD_MINOR_LAN_17              0x00000020   /* 1-17% utilized */
#define BTIF_COD_MINOR_LAN_33              0x00000040   /* 17-33% utilized */
#define BTIF_COD_MINOR_LAN_50              0x00000060   /* 33-50% utilized */
#define BTIF_COD_MINOR_LAN_67              0x00000080   /* 50-67% utilized */
#define BTIF_COD_MINOR_LAN_83              0x000000A0   /* 67-83% utilized */
#define BTIF_COD_MINOR_LAN_99              0x000000C0   /* 83-99% utilized */
#define BTIF_COD_MINOR_LAN_NO_SERVICE      0x000000E0   /* 100% utilized */

/* Group: Minor Device Class - Audio Major class */
#define BTIF_COD_MINOR_AUDIO_UNCLASSIFIED  0x00000000
#define BTIF_COD_MINOR_AUDIO_HEADSET       0x00000004
#define BTIF_COD_MINOR_AUDIO_HANDSFREE     0x00000008
#define BTIF_COD_MINOR_AUDIO_MICROPHONE    0x00000010
#define BTIF_COD_MINOR_AUDIO_LOUDSPEAKER   0x00000014
#define BTIF_COD_MINOR_AUDIO_HEADPHONES    0x00000018
#define BTIF_COD_MINOR_AUDIO_PORTABLEAUDIO 0x0000001C
#define BTIF_COD_MINOR_AUDIO_CARAUDIO      0x00000020
#define BTIF_COD_MINOR_AUDIO_SETTOPBOX     0x00000024
#define BTIF_COD_MINOR_AUDIO_HIFIAUDIO     0x00000028
#define BTIF_COD_MINOR_AUDIO_VCR           0x0000002C
#define BTIF_COD_MINOR_AUDIO_VIDEOCAMERA   0x00000030
#define BTIF_COD_MINOR_AUDIO_CAMCORDER     0x00000034
#define BTIF_COD_MINOR_AUDIO_VIDEOMONITOR  0x00000038
#define BTIF_COD_MINOR_AUDIO_VIDEOSPEAKER  0x0000003C
#define BTIF_COD_MINOR_AUDIO_CONFERENCING  0x00000040
#define BTIF_COD_MINOR_AUDIO_GAMING        0x00000048

/* Group: Minor Device Class - Peripheral Major class */
#define BTIF_COD_MINOR_PERIPH_KEYBOARD     0x00000040
#define BTIF_COD_MINOR_PERIPH_POINTING     0x00000080
#define BTIF_COD_MINOR_PERIPH_COMBOKEY     0x000000C0
/* Group: Minor Device Class - ORed with Peripheral Minor Device class */
#define BTIF_COD_MINOR_PERIPH_UNCLASSIFIED 0x00000000
#define BTIF_COD_MINOR_PERIPH_JOYSTICK     0x00000004
#define BTIF_COD_MINOR_PERIPH_GAMEPAD      0x00000008
#define BTIF_COD_MINOR_PERIPH_REMOTECTRL   0x0000000C
#define BTIF_COD_MINOR_PERIPH_SENSING      0x00000010
#define BTIF_COD_MINOR_PERIPH_DIGITIZER    0x00000014
#define BTIF_COD_MINOR_PERIPH_CARD_RDR     0x00000018

/* Group: Minor Device Class - Imaging Major class */
#define BTIF_COD_MINOR_IMAGE_UNCLASSIFIED  0x00000000
#define BTIF_COD_MINOR_IMAGE_DISPLAY       0x00000010
#define BTIF_COD_MINOR_IMAGE_CAMERA        0x00000020
#define BTIF_COD_MINOR_IMAGE_SCANNER       0x00000040
#define BTIF_COD_MINOR_IMAGE_PRINTER       0x00000080

/* Group: Masks used to isolate the class of device components */
#define BTIF_COD_SERVICE_MASK              0x00ffC000   /* Less LIAC bit */
#define BTIF_COD_MAJOR_MASK                0x00001F00
#define BTIF_COD_MINOR_MASK                0x000000FC
#define BTIF_COD_LIMITED_DISC_MASK         0x00002000   /* LIAC bit */

#define BTIF_CMGR_SNIFF_DISABLED                   (0xFF)
#define BTIF_CMGR_SNIFF_DONT_CARE                  (0)
#define BTIF_CMGR_SNIFF_TIMER                      (10000)
#define BTIF_CMGR_BT_MASTER_SNIFF_TIMER            (22000)
#define BTIF_CMGR_BT_SLAVE_SNIFF_TIMER             (24000)
#define BTIF_CMGR_MOBILE_SNIFF_TIMER               (20000)
#define BTIF_CMGR_SNIFF_TIMER_AFTER_CONFLICT       (2000)
#define BTIF_CMGR_MOBILE_SNIFF_TIMEOUT             (0)

typedef uint8_t (*btif_callback_ext) (const bt_bdaddr_t *);

typedef void btif_cmgr_handler_t;

#ifndef BTIF_CMGR_DEFAULT_SNIFF_EXIT_POLICY
#define BTIF_CMGR_DEFAULT_SNIFF_EXIT_POLICY CMGR_SNIFF_EXIT_ON_AUDIO
#endif /*  */

#ifndef BTIF_CMGR_DEFAULT_SNIFF_TIMER
#define BTIF_CMGR_DEFAULT_SNIFF_TIMER CMGR_SNIFF_DONT_CARE
#endif /*  */

#ifndef BTIF_CMGR_AUDIO_DEFAULT_PARMS
//#define CMGR_AUDIO_DEFAULT_PARMS CMGR_AUDIO_PARMS_S3
#define BTIF_CMGR_AUDIO_DEFAULT_PARMS CMGR_AUDIO_PARMS_S4
#endif /*  */

#ifndef BTIF_CMGR_SNIFF_MIN_INTERVAL
#ifdef BISTO_ENABLED
#define BTIF_CMGR_SNIFF_MIN_INTERVAL 160
#else
#define BTIF_CMGR_SNIFF_MIN_INTERVAL (796)
#endif
#endif /*  */

#ifndef BTIF_CMGR_SNIFF_MAX_INTERVAL
#ifdef BISTO_ENABLED
#define BTIF_CMGR_SNIFF_MAX_INTERVAL (160)
#else
#define BTIF_CMGR_SNIFF_MAX_INTERVAL (796)
#endif
#endif /*  */

#ifndef BTIF_CMGR_SNIFF_ATTEMPT
#define BTIF_CMGR_SNIFF_ATTEMPT 3
#endif /*  */

#ifndef BTIF_CMGR_SNIFF_TIMEOUT
#define BTIF_CMGR_SNIFF_TIMEOUT 1
#endif /*  */

typedef uint8_t cmgr_event_t;

#define BTIF_CMEVENT_DATA_LINK_CON_CNF     1

#define BTIF_CMEVENT_DATA_LINK_CON_IND     2

#define BTIF_CMEVENT_DATA_LINK_DIS         3

#define BTIF_CMEVENT_AUDIO_LINK_REQ        4

#define BTIF_CMEVENT_AUDIO_LINK_CON        5

#define BTIF_CMEVENT_AUDIO_LINK_DIS        6

#define BTIF_CMEVENT_AUDIO_DATA            7

#define BTIF_CMEVENT_AUDIO_DATA_SENT       8

#define BTIF_CMEVENT_ENTER_SNIFF_MODE      9

#define BTIF_CMEVENT_EXIT_SNIFF_PARK_MODE  10


//when report_format=0,data_format will set the interval(unit:slot)
//when report_format=1,data_format will set the packet total num
//when report_format=2,data_format has no meaning
#define FLAG_RESET_DATA    2
#define FLAG_START_DATA    1
#define FLAG_GET_DATA        0

#define REP_FORMAT_TIME     0
#define REP_FORMAT_PACKET  1
#define REP_FORMAT_STOP_GET  2

#define  BTIF_EVT_ERROR_SCO_UNKNOWN       (0xE8)
#define  BTIF_EVT_ERROR_ACL_UNKNOWN       (0xE9)

#define BTIF_BTM_RX_FILTER_HCI_TYPE   (0x01)
#define BTIF_BTM_RX_FILTER_L2C_TYPE   (0x02)

#define BTIF_STANDARD_SCAN           0x00
#define BTIF_INTERLACED_SCAN         0x01

#define BTM_NAME_MAX_LEN                         248  /*include '\0'*/
#define BTM_SHORT_NAME_MAX_LEN                   32   /*include '\0'*/

#define MAX_ACTIVE_MODE_MANAGED_LINKS       3
#define UPDATE_ACTIVE_MODE_FOR_ALL_LINKS    MAX_ACTIVE_MODE_MANAGED_LINKS

typedef void btif_cmgr_handler_t;

typedef void (*btif_confirmation_req_callback_t)(struct bdaddr_t *bdaddr, uint32 numeric_value);

typedef struct {
    uint16_t inqInterval;            /* Inquiry scan interval */
    uint16_t inqWindow;              /* Inquiry scan Window */
    uint16_t pageInterval;           /* Page scan interval */
    uint16_t pageWindow;             /* Page scan window */
} btif_access_mode_info_t;

typedef uint8_t btif_link_key_type_t;
struct btm_conn_item_t;

#define BTIF_COMBINATION_KEY            0x00
#define BTIF_LOCAL_UNIT_KEY             0x01
#define BTIF_REMOTE_UNIT_KEY            0x02
#define BTIF_DEBUG_COMBINATION_KEY      0x03
#define BTIF_UNAUTH_COMBINATION_KEY     0x04
#define BTIF_AUTH_COMBINATION_KEY       0x05
#define BTIF_CHANGED_COMBINATION_KEY    0x06
#define BTIF_UNAUTH_SC_COMBINATION_KEY  0x07
#define BTIF_AUTH_SC_COMBINATION_KEY    0x08
#define BTIF_IBRT_STORE_ADDR_ONLY       0x10

typedef struct
{
    bt_bdaddr_t bdAddr;           /* Device Address */
    uint8_t     linkKey[16];
} btif_dev_linkkey;

typedef struct {
    bt_bdaddr_t bdAddr;
    bool trusted;
    bool for_bt_source;
    uint8_t linkKey[16];
    btif_link_key_type_t keyType;
    uint8_t pinLen;
    uint8_t cod[3];
    char remote_dev_name[BTM_SHORT_NAME_MAX_LEN];
} btif_device_record_t;

typedef struct {
    uint16_t interval_min;
    uint16_t interval_max;
    uint8_t adv_type;
    uint8_t own_addr_type;
    uint8_t peer_addr_type;
    bt_bdaddr_t bd_addr;
    uint8_t adv_chanmap;
    uint8_t adv_filter_policy;
} btif_adv_para_struct_t;

#define BLE_ADV_REPORT_MAX_LEN 31

typedef struct {
    uint8_t type;
    uint8_t addr_type;
    bt_bdaddr_t addr;
    uint8_t data_len;
    uint8_t data[BLE_ADV_REPORT_MAX_LEN];
    int8_t rssi;
} btif_ble_adv_report;

typedef struct {
    uint8_t scan_type;
    uint16_t scan_interval;
    uint16_t scan_window;
    uint8_t own_addr_type;
    uint8_t scan_filter_policy;
} btif_scan_para_struct_t;

typedef struct btif_sniff_info_t {

    /* the mandatory sniff interval range for controllers is between 0x0006 and 0x0540.
     * The value is expressed in 0.625 ms increments (0x0006 = 3.75 ms).
     *
     * The actual interval selected by the radio will be returned in
     * a BTEVENT_MODE_CHANGE event.
     */

    uint16_t maxInterval;

    /* Minimum acceptable interval between each consecutive sniff period.
     * Must be an even number between 0x0002 and 0xFFFE, and be less than
     * "maxInterval". Like maxInterval this value is expressed in
     * 0.625 ms increments.
     */
    uint16_t minInterval;

    /* The number of master-to-slave transmission slots during which
     * a device should listen for traffic (sniff attempt).
     * Expressed in 0.625 ms increments. May be between 0x0001 and 0x7FFF.
     */
    uint16_t attempt;

    /* The amount of time before a sniff timeout occurs. Expressed in
     * 1.25 ms increments. May be between 0x0000 and 0x7FFF, but the mandatory
     * range for controllers is 0x0000 to 0x0028.
     */
    uint16_t timeout;
} btif_sniff_info_t;

typedef struct btif_sniff_timer_t {
    TimeT       timeout;                /* Sniff timer manager timeout param not sniff_mode's timeout*/
    bool        updata_sniff_timer;     /* Updata sniff timer manager timeout param*/
} btif_sniff_timer_t;

struct _evm_timer {
    list_entry_t node;          /* Used internally by the Event Manager */
    void *context;              /* Context area for use by callers */
    evm_timer_notify func;      /* Function to call when timer fires */

    /* === Internal use only === */
    TimeT time;                 /* Amount of time to wait */
    TimeT startTime;            /* System time when the timer started */
};

enum app_bt_audio_event_t {
    APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED                  = 0x1,
    APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_NOTIFY_RSP               = 0x2,
    APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS                          = 0x3,
    APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_MOCK                     = 0x4,
    APP_BT_AUDIO_EVENT_AVRCP_IS_REALLY_PAUSED                     = 0x5,
    APP_BT_AUDIO_EVENT_AVRCP_MEDIA_PLAYING                        = 0x6,
    APP_BT_AUDIO_EVENT_AVRCP_MEDIA_PAUSED                         = 0x7,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_OPEN                           = 0x8,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_CLOSE                          = 0x9,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_START                          = 0xA,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_MOCK_START                     = 0xB,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_SUSPEND                        = 0xC,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_SWITCH                         = 0xD,
    APP_BT_AUDIO_EVENT_A2DP_STREAM_RESELECT                       = 0xE,
    APP_BT_AUDIO_EVENT_AVRCP_CONNECTED                            = 0xF,
    APP_BT_AUDIO_EVENT_HFP_SERVICE_CONNECTED                      = 0x10,
    APP_BT_AUDIO_EVENT_HFP_SERVICE_MOCK_CONNECTED                 = 0x11,
    APP_BT_AUDIO_EVENT_HFP_SERVICE_DISCONNECTED                   = 0x12,
    APP_BT_AUDIO_EVENT_HFP_SCO_CONNECT_REQ                        = 0x13,
    APP_BT_AUDIO_EVENT_HFP_SCO_CONNECTED                          = 0x14,
    APP_BT_AUDIO_EVENT_HFP_SCO_DISCONNECTED                       = 0x15,
    APP_BT_AUDIO_EVENT_HFP_CCWA_IND                               = 0x16,
    APP_BT_AUDIO_EVENT_HFP_RING_IND                               = 0x17,
    APP_BT_AUDIO_EVENT_HFP_CLCC_IND                               = 0x18,
    APP_BT_AUDIO_EVENT_HFP_CALL_IND                               = 0x19,  //call hang up or active indication
    APP_BT_AUDIO_EVENT_HFP_CALLSETUP_IND                          = 0x1A,  //call setup
    APP_BT_AUDIO_EVENT_HFP_CALLHELD_IND                           = 0x1B,
    APP_BT_AUDIO_EVENT_PROFILE_EXCHANGED                          = 0x1C,
    APP_BT_AUDIO_EVENT_IBRT_STARTED                               = 0x1D,
    APP_BT_AUDIO_EVENT_REPLAY_WHEN_PAUSE_COME_AFTER_SCO_DISC      = 0x1E,
    APP_BT_AUDIO_EVENT_REPLAY_WHEN_SUSPEND_COME_AFTER_SCO_DISC    = 0x1F,
    APP_BT_AUDIO_EVENT_ROUTE_CALL_TO_LE                           = 0x20,
    APP_BT_AUDIO_EVENT_ACL_CONNECTED                              = 0x21,
    APP_BT_AUDIO_EVENT_ACL_DISC                                   = 0x22,
    APP_BT_AUDIO_EVENT_AUTHENTICATED                              = 0x23,
    APP_BT_AUDIO_EVENT_HFP_CALL_MOCK                              = 0x24,
    APP_BT_AUDIO_EVENT_HFP_CALLSETUP_MOCK                         = 0x25,
    APP_BT_AUDIO_EVENT_HFP_CALLHOLD_MOCK                          = 0x26,
    APP_BT_AUDIO_EVENT_HUNGUP_INCOMING_CALL                       = 0x27,
    APP_BT_AUDIO_EVENT_HUNGUP_ACTIVE_CALL                         = 0x28,
    APP_BT_AUDIO_HOLD_SWICTH_BACKGROUND                           = 0x29,
    APP_BT_AUDIO_BG_TO_FG_COMPLETE                                = 0x2A,
    APP_BT_AUDIO_EVENT_AVRCP_PLAY_STATUS_CHANGED_MOCK             = 0x2B,
    APP_BT_AUDIO_EVENT_A2DP_SYNC_STREAMING_COMPLETE               = 0x2C,
    APP_BT_AUDIO_EVENT_A2DP_MUTE_FRAME                            = 0x2D,
    APP_BT_AUDIO_EVENT_HFP_SCO_CON_FAIL_IND                       = 0x2E,
    APP_BT_AUDIO_EVENT_MUTE_STREAM_DETECT_NOTIFY                  = 0x2F,
    APP_BT_AUDIO_EVENT_SET_BT_BLE_ACTIVE_LINK_COMPLETE            = 0x30,
};

typedef enum {
    APP_BT_A2DP_STREAM_DETECT_STOP           = 0x00,    // Final test result: stop
    APP_BT_A2DP_STREAM_DETECT_PLAYING        = 0x01,    // Final test result: playing
    APP_BT_A2DP_STREAM_DETECT_INVALID        = 0xFF,
} app_bt_mute_detect_t;

typedef struct {
    uint16_t id;
    uint16_t status;

    uint16_t aud_type;
    uint32_t aud_id;

    uint8_t freq;
    uint8_t device_id;
} APP_AUDIO_STATUS;

enum APP_BT_AUDIO_Q_POS {
    APP_BT_SETTING_Q_POS_HEAD = 0,
    APP_BT_SETTING_Q_POS_TAIL = 1,
};

typedef enum {
    BT_ACTIVE_MODE_KEEP_USER_A2DP_STREAMING = 1, // always in active mode during a2dp streaming
    BT_ACTIVE_MODE_KEEP_USER_SCO_STREAMING,     // always in active mode during ai voice up-streaming
    BT_ACTIVE_MODE_KEEP_USER_AI_VOICE_STREAM,   // always in active mode during OTA
    BT_ACTIVE_MODE_KEEP_USER_OTA,               // stay in active mode for 15 seconds after role switch
    BT_ACTIVE_MODE_KEEP_USER_ROLE_SWITCH,       // stay in active mode during synchronous voice prompt playing
    BT_ACTIVE_MODE_KEEP_USER_SYNC_VOICE_PROMPT, // always in active mode during phone call
    BT_ACTIVE_MODE_KEEP_USER_SPP,               // stay in active mode for speed
} BT_ACTIVE_MODE_KEEP_USER_T;

typedef enum {
    BTIF_BLM_ACTIVE_MODE    = 0x00,
    BTIF_BLM_HOLD_MODE      = 0x01,
    BTIF_BLM_SNIFF_MODE     = 0x02,
} btif_link_mode_t;

typedef struct bt_remver_t
{
    uint8_t     vers;
    uint16_t    compid;
    uint16_t    subvers;
} __attribute__((packed)) bt_remver_t;

typedef uint16_t btif_link_policy_t;

#define BTIF_BLP_DISABLE_ALL         0x0000
#define BTIF_BLP_MASTER_SLAVE_SWITCH 0x0001
#define BTIF_BLP_HOLD_MODE           0x0002
#define BTIF_BLP_SNIFF_MODE          0x0004
#define BTIF_BLP_PARK_MODE           0x0008
#define BTIF_BLP_SCATTER_MODE        0x0010

typedef uint8_t btif_connection_role_t;

#define BTIF_BCR_MASTER   0x00
#define BTIF_BCR_SLAVE    0x01
#define BTIF_BCR_ANY      0x02
#define BTIF_BCR_UNKNOWN  0x03
#define BTIF_BCR_PSLAVE   0x04
#define BTIF_BCR_PMASTER  0x05

typedef uint8_t btif_accessible_mode_t;

#define BTIF_BAM_NOT_ACCESSIBLE     0x00    /* Non-discoverable or connectable */
#define BTIF_BAM_GENERAL_ACCESSIBLE 0x03    /* General discoverable and connectable */
#define BTIF_BAM_LIMITED_ACCESSIBLE 0x13    /* Limited discoverable and connectable */
#define BTIF_BAM_CONNECTABLE_ONLY   0x02    /* Connectable but not discoverable */
#define BTIF_BAM_DISCOVERABLE_ONLY  0x01    /* Discoverable but not connectable */
#define BTIF_BT_DEFAULT_ACCESS_MODE_PAIR       BTIF_BAM_GENERAL_ACCESSIBLE
#define BTIF_BAM_INVALID_ACCESS_MODE    0xFF

#define QOS_SETUP_SERVICE_TYPE_NO_TRAFFIC       0x00
#define QOS_SETUP_SERVICE_TYPE_BEST_EFFORT      0x01
#define QOS_SETUP_SERVICE_TYPE_GUARANTEED       0x02

typedef uint8_t voice_report_role_t;
#define VOICE_REPORT_MASTER   0x00
#define VOICE_REPORT_SLAVE    0x01
#define VOICE_REPORT_LOCAL    0x02

typedef bool (*ibrt_io_capbility_callback)(void *bdaddr);
typedef void (*btif_cmgr_callback) (void *handler, uint8_t event, int status);
typedef void (*btif_global_handle)(const btif_event_t *Event);

typedef int32_t (*bt_gather_global_srv_uuids)(uint8_t in_uuid_size, uint8_t *out_buff,
    uint32_t out_buff_len, uint32_t *out_len, uint32_t *out_real_len);

typedef struct bt_acl_state_t {
    bt_bdaddr_t remote;
    uint16_t acl_conn_hdl;
    bool acl_is_connected;
    btif_link_mode_t acl_link_mode;
    uint8_t acl_bt_role;
    uint8_t device_id;
    uint16_t sniff_interval;
} bt_acl_state_t;

typedef enum {
    BT_PROFILE_HFP = 1,
    BT_PROFILE_A2DP,
    BT_PROFILE_AVRCP,
} BT_PROFILE_ID_ENUM_T;

typedef enum {
    APP_IBRT_HFP_PROFILE_ID = 1,
    APP_IBRT_A2DP_PROFILE_ID = 2,
    APP_IBRT_AVRCP_PROFILE_ID = 3,
    APP_IBRT_SDP_PROFILE_ID = 4,
    APP_IBRT_HID_PROFILE_ID = 5,
    APP_IBRT_MAX_PROFILE_ID,
} app_ibrt_profile_id_enum;

typedef enum {
    BT_PAGE_SCAN = 1,
    BT_INQUIRY_SCAN,
} BT_SCAN_ENUM_T;

typedef enum {
    BT_COUNT_MOBILE_LINK = 1,
    BT_COUNT_MOBILE_TWS_LINK,
    BT_COUNT_SOURCE_LINK,
    BT_COUNT_TOTAL_ACL_LINK,
    BT_COUNT_CONNECTED_SCO, // app_bt_audio_count_connected_sco
    BT_COUNT_STREAMING_A2DP, // app_bt_audio_count_streaming_a2dp
    BT_COUNT_STREAMING_LINK, // app_bt_audio_count_straming_mobile_links a2dp+sco
} BT_COUNT_LINK_ENUM_T;

typedef enum {
    BT_SELECT_CALL_ACTIVE_HFP_DEVICE = 1,
    BT_SELECT_CURR_A2DP_DEVICE,
    BT_SELECT_CURR_HFP_DEVICE, // app_bt_audio_get_curr_hfp_device
    BT_SELECT_CURR_PLAYING_SCO, // app_bt_audio_get_curr_playing_sco
    BT_SELECT_CURR_PLAYING_A2DP,
    BT_SELECT_CONNECTED_DEVICE, // app_bt_audio_select_connected_device
    BT_SELECT_USER_ACTION_DEVICE, // app_bt_audio_get_device_for_user_action
} BT_SELECT_DEVICE_ENUM_T;

typedef enum {
    BT_SELECT_ANOTHER_CREATE_SCO_DEVICE = 1,
    BT_SELECT_ANOTHER_STREAMING_A2DP_DEVICE, // app_bt_audio_select_another_streaming_a2dp
} BT_SELECT_ANOTHER_DEVICE_ENUM_T;

#ifdef FPGA
void app_bt_accessmode_set_for_test(btif_accessible_mode_t mode);
void app_bt_adv_mode_set_for_test(uint8_t en);
void app_start_ble_adv_for_test(void);
void app_bt_write_controller_memory_for_test(uint32_t addr,uint32_t val,uint8_t type);
void app_bt_read_controller_memory_for_test(uint32_t addr,uint32_t len,uint8_t type);
#endif

#ifdef __cplusplus
}
#endif
#endif /* __ME_COMMON_DEFINE_H__ */
