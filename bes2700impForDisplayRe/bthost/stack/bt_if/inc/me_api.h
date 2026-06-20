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
#ifndef __ME_API__H__
#define __ME_API__H__
#include "bluetooth.h"
#include "me_common_define.h"
#include "bt_callback_func.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t btif_event_mask_t;

#define BTIF_BEM_NO_EVENTS                    0x00000000
#define BTIF_BEM_INQUIRY_RESULT               0x00000001
#define BTIF_BEM_INQUIRY_COMPLETE             0x00000002
#define BTIF_BEM_INQUIRY_CANCELED             0x00000004
#define BTIF_BEM_LINK_CONNECT_IND             0x00000008
#define BTIF_BEM_SCO_CONNECT_IND              0x00000010
#define BTIF_BEM_LINK_DISCONNECT              0x00000020
#define BTIF_BEM_LINK_CONNECT_CNF             0x00000040
#define BTIF_BEM_ROLE_DISCOVERED              0x00000080
#define BTIF_BEM_MODE_CHANGE                  0x00000100
#define BTIF_BEM_ACCESSIBLE_CHANGE            0x00000200
#define BTIF_BEM_AUTHENTICATED                0x00000400
#define BTIF_BEM_ENCRYPTION_CHANGE            0x00000800
#define BTIF_BEM_SECURITY_CHANGE              0x00001000
#define BTIF_BEM_ROLE_CHANGE                  0x00002000
#define BTIF_BEM_SCO_DISCONNECT               0x00004000
#define BTIF_BEM_SCO_CONNECT_CNF              0x00008000
#define BTIF_BEM_SIMPLE_PAIRING_COMPLETE      0x00010000
#define BTIF_BEM_REMOTE_FEATURES              0x00020000
#define BTIF_BEM_REM_HOST_FEATURES            0x00040000
#define BTIF_BEM_LINK_SUPERV_TIMEOUT_CHANGED  0x00080000
#define BTIF_BEM_SET_SNIFF_SUBR_PARMS         0x00100000
#define BTIF_BEM_SNIFF_SUBRATE_INFO           0x00200000
#define BTIF_BEM_SET_INQ_MODE                 0x00400000
#define BTIF_BEM_SET_INQ_RSP_TX_PWR           0x00800000
#define BTIF_BEM_SET_EXT_INQ_RESP             0x01000000
#define BTIF_BEM_SET_ERR_DATA_REP             0x02000000
#define BTIF_BEM_KEY_PRESSED                  0x04000000
#define BTIF_BEM_CONN_PACKET_TYPE_CHNG        0x08000000
#define BTIF_BEM_QOS_SETUP_COMPLETE           0x10000000
#define BTIF_BEM_MAX_SLOT_CHANGED             0x20000000
#define BTIF_BEM_SNIFFER_CONTROL_DONE         0x40000000
#define BTIF_BEM_LINK_POLICY_CHANGED             0x80000000
#define BTIF_BEM_ALL_EVENTS                   0xffffffff

typedef uint32_t btif_iac_t;

#define BTIF_BT_IAC_GIAC 0x9E8B33   /* General/Unlimited Inquiry Access Code */
#define BTIF_BT_IAC_LIAC 0x9E8B00   /* Limited Dedicated Inquiry Access Code */

typedef uint8_t btif_link_type_t;

#define BTIF_BLT_SCO   0x00
#define BTIF_BLT_ACL   0x01
#define BTIF_BLT_ESCO  0x02

typedef U16 btif_acl_packet;

#define BTIF_BAPT_NO_2_DH1  0x0002
#define BTIF_BAPT_NO_3_DH1  0x0004
#define BTIF_BAPT_DM1       0x0008
#define BTIF_BAPT_DH1       0x0010
#define BTIF_BAPT_NO_2_DH3  0x0100
#define BTIF_BAPT_NO_3_DH3  0x0200
#define BTIF_BAPT_DM3       0x0400
#define BTIF_BAPT_DH3       0x0800
#define BTIF_BAPT_NO_2_DH5  0x1000
#define BTIF_BAPT_NO_3_DH5  0x2000
#define BTIF_BAPT_DM5       0x4000
#define BTIF_BAPT_DH5       0x8000

#define BTIF_2M_PACKET     (BTIF_BAPT_DM1|BTIF_BAPT_DH1|BTIF_BAPT_NO_3_DH1|BTIF_BAPT_NO_3_DH3|BTIF_BAPT_DM3|BTIF_BAPT_DH3|BTIF_BAPT_NO_3_DH5)
#define BTIF_3M_PACKET     (BTIF_BAPT_DM1|BTIF_BAPT_DH1|BTIF_BAPT_DM3|BTIF_BAPT_DH3|BTIF_BAPT_DM5|BTIF_BAPT_DH5)
#define BTIF_1_SLOT_PACKET (BTIF_BAPT_DM1|BTIF_BAPT_DH1|BTIF_BAPT_NO_3_DH1|BTIF_BAPT_NO_2_DH3|BTIF_BAPT_NO_3_DH3|BTIF_BAPT_NO_2_DH5|BTIF_BAPT_NO_3_DH5)
#define BTIF_3_SLOT_PACKET (BTIF_BAPT_DM1|BTIF_BAPT_NO_3_DH3|BTIF_BAPT_NO_2_DH5|BTIF_BAPT_NO_3_DH5)

/* Mask must be updated if new policy values are added */
#define BLP_MASK                0xfff0  /* Disables ScatterNet bit */
#define BLP_SCATTER_MASK        0xffe0  /* Enables ScatterNet bit */

/* End of BtAccessibleMode */

typedef uint8_t btif_oob_data_present_t;

#define BTIF_OOB_DATA_NOT_PRESENT  0    /* No Out of Band Data is present */
#define BTIF_OOB_DATA_PRESENT      1    /* Out of Band Data is present    */

typedef uint8_t btif_auth_requirements_t;

#define BTIF_MITM_PROTECT_NOT_REQUIRED  0x00    /* No Man in the Middle protection  */
#define BTIF_MITM_PROTECT_REQUIRED      0x01    /* Man in the Middle protection req */

#define  BTIF_BAS_NOT_AUTHENTICATED  0x00
#define  BTIF_BAS_START_AUTHENTICATE 0x01
#define  BTIF_BAS_WAITING_KEY_REQ    0x02
#define  BTIF_BAS_SENDING_KEY        0x03
#define  BTIF_BAS_WAITING_FOR_IO     0x04
#define  BTIF_BAS_WAITING_FOR_IO_R   0x05
#define  BTIF_BAS_WAITING_FOR_KEY    0x06
#define  BTIF_BAS_WAITING_FOR_KEY_R  0x07
#define  BTIF_BAS_AUTHENTICATED      0x08

typedef uint8_t btif_stack_state_t;

/* The stack has completed initialization of the radio hardware. */
#define BTIF_BTSS_NOT_INITIALIZED 0

/* The stack is initialized. */
#define BTIF_BTSS_INITIALIZED     1

/* The stack has encountered an error while initializing the radio hardware. */
#define BTIF_BTSS_INITIALIZE_ERR  2

/* The stack is deinitializing. */
#define BTIF_BTSS_DEINITIALIZE    3

typedef void (*btif_callback) (const btif_event_t *);
typedef void (*ibrt_cmd_status_callback)(const uint8_t *para);
typedef uint8_t (*btif_callback_ext2) (void);
typedef void (*btif_callback_ext3) (void);

typedef uint8_t btif_inquiry_mode_t;

#define BTIF_INQ_MODE_NORMAL    0   /* Normal Inquiry Response format           */
#define BTIF_INQ_MODE_RSSI      1   /* RSSI Inquiry Response format             */
#define BTIF_INQ_INVALID_RSSI   127   /* RSSI Inquiry Response format             */
#define BTIF_INQ_MODE_EXTENDED  2   /* Extended or RSSI Inquiry Response format */

typedef uint8_t btif_rem_dev_state_t;

#define BTIF_BDS_DISCONNECTED  0x00
#define BTIF_BDS_OUT_CON       0x01 /* Starting an out going connection */
#define BTIF_BDS_IN_CON        0x02 /* In process of incoming connection */
#define BTIF_BDS_CONNECTED     0x03 /* Connected */
#define BTIF_BDS_OUT_DISC      0x04 /* Starting an out going disconnect */
#define BTIF_BDS_OUT_DISC2     0x05 /* Disconnect status received */
#define BTIF_BDS_OUT_CON2      0x06 /* In SCO, used when connection request has
                                       been sent */

typedef uint8_t btif_eir_data_type_t;

#define BTIF_EIR_FLAGS                0x01
#define BTIF_EIR_SRV_CLASS_16_PART    0x02
#define BTIF_EIR_SRV_CLASS_16_WHOLE   0x03
#define BTIF_EIR_SRV_CLASS_32_PART    0x04
#define BTIF_EIR_SRV_CLASS_32_WHOLE   0x05
#define BTIF_EIR_SRV_CLASS_128_PART   0x06
#define BTIF_EIR_SRV_CLASS_128_WHOLE  0x07
#define BTIF_EIR_REM_NAME_PART        0x08
#define BTIF_EIR_REM_NAME_WHOLE       0x09
#define BTIF_EIR_TX_POWER             0x0A
#define BTIF_EIR_MAN_SPECIFIC         0xFF


#define BTIF_BR_USE_FEC              (0<<0)
#define BTIF_BR_NOT_USE_FEC          (1<<0)
#define BTIF_BR_NO_PKT_PRFER_AVALB   (0<<1)
#define BTIF_BR_USE_1_SLOT_PKT       (1<<1)
#define BTIF_BR_USE_3_SLOT_PKT       (2<<1)
#define BTIF_BR_USE_5_SLOT_PKT       (3<<1)
#define BTIF_EDR_USE_DM1_PKT         (0<<3)
#define BTIF_EDR_USE_2MB_PKT         (1<<3)
#define BTIF_EDR_USE_3MB_PKT         (2<<3)
#define BTIF_EDR_NO_PKT_PRFER_AVALB  (0<<5)
#define BTIF_EDR_USE_1_SLOT_PTK      (1<<5)
#define BTIF_EDR_USE_3_SLOT_PTK      (2<<5)
#define BTIF_EDR_USE_5_SLOT_PTK      (3<<5)


#define CONN_AUTO_ACCEPT_DISABLE                      0x01

#define CONN_AUTO_ACCEPT_ENABLE_WITH_MSS_DISABLE      0x02

#define CONN_AUTO_ACCEPT_ENABLE_WITH_MSS_ENABLE       0x03


/* Group: BES vendore debug CMD sub opcode*/
#define BTIF_DBG_ENABLE_BTPCM  9
#define BTIF_DBG_SET_TWS_LINK  10

void btif_add_bt_event_callback(bt_event_callback_t cb, uint32_t masks);

bt_status_t btif_me_set_clear_all_filters(void);

bt_status_t btif_me_set_inquiry_no_filter(void);

bt_status_t btif_me_set_inquiry_cod_filter(uint32_t class_of_device_value, uint32_t class_of_device_mask_value);

bt_status_t btif_me_set_inquiry_address_filter(uint32_t bdaddr_high_4_byte, uint32_t bdaddr_low_2_byte);

bt_status_t btif_me_set_connect_no_filter(uint32_t auto_accept_flag);

bt_status_t btif_me_set_connect_cod_filter(uint32_t class_of_device_value, uint32_t class_of_device_mask_value, uint32_t auto_accept_flag);

bt_status_t btif_me_set_connect_address_filter(uint32_t bdaddr_high_4_byte, uint32_t bdaddr_low_2_byte, uint32_t auto_accept_flag);

#define BTIF_BTM_INVALID_HANDLE             (0xFFFF)

typedef struct {
    list_entry_t node;
    uint8_t opType;
} btif_operation_t;

typedef struct {

    /*  May be any number between 0x0000 and 0xFFFE.  The value is expressed in 0.625 ms
     * increments.
     */
    uint16_t maxLatency;

    /*  any number between 0x0000 and 0xFFFE.  The value is expressed in
     * 0.625 ms increments.
     */
    uint16_t minRemoteTimeout;

    /* Minimum base sniff subrate timeout that the local device may use.
     * May be any number between 0x0000 and 0xFFFE.  The value is expressed in
     * 0.625 ms increments.
     */
    uint16_t minLocalTimeout;
} btif_sniff_subrate_parms_t;

typedef struct {
    list_entry_t node;          /* For internal stack use only. */
    btif_callback callback;  /* Pointer to callback function */
    btif_event_mask_t emask; /* For internal stack use only. */
} btif_handler;

struct conn_handler {
    list_entry_t        node;
    struct bdaddr_t     remote;
    bool                use;
    btif_cmgr_callback  callback;
    btif_sniff_info_t   sniff_info;             /*record the sniff infomation               */
    btif_handler        btHandler;
    uint8_t             sniff_timer;            /*record the timer                          */
    uint32              sniff_timeout;          /* Timeout value of the sniff timer         */
    uint32              timer_start_time_tick;  /* record the time tick that sniff timer start  */
};

/* Internal types required for BtRemoteDevice structure */
typedef uint8_t btif_auth_state_t;

typedef uint8_t btif_encrypt_state_t;

typedef uint8_t btif_authorize_state_t;

typedef uint8_t btif_sec_access_state_t;

typedef uint8_t btif_link_rx_state_t;

typedef uint8_t btif_op_type_t;

typedef struct {

    /* Reserved */
    uint8_t flags;

    /* Service Type: 0 = No Traffic, 1 = Best Effort, 2 Guaranteed */
    uint8_t serviceType;

    /* Token Rate in octets per second */
    uint32_t tokenRate;

    /* Peak Bandwidth in octets per second */
    uint32_t peakBandwidth;

    /* Latency in microseconds */
    uint32_t latency;

    /* Delay Variation in microseconds */
    uint32_t delayVariation;
} btif_qos_info_t;

typedef struct {
    btif_operation_t op;
    uint8_t len;
    uint8_t data[31];
} btif_write_adv_data_op_t;

typedef struct {
    btif_operation_t op;
    btif_adv_para_struct_t adv_para;
} btif_write_adv_para_op_t;

typedef struct {
    btif_operation_t op;
    uint8_t en;
} btif_write_adv_en_op_t;

typedef struct {
    btif_operation_t op;
    btif_scan_para_struct_t scan_para;
} btif_write_scan_para_op_t;

typedef struct {
    btif_operation_t op;
    uint8_t scan_en;
    uint32_t filter_duplicat;
} btif_write_scan_en_op_t;

typedef struct
{
    U8   psRepMode;
    U8   psMode;
    U16  clockOffset;
} bt_page_scanInfo_t;

struct dbg_send_prefer_rate
{
    uint16_t conhdl;
    uint8_t rate;
};

typedef struct
{
    bt_bdaddr_t bdAddr;           /* Device Address */
    bt_page_scanInfo_t  psi;              /* Page scan info used for connecting */
    U8              psPeriodMode;
    U32     classOfDevice;

    /* RSSI in dBm (-127 to +20). Only valid when controller reports RSSI with
     * in inquiry results (also see ME_SetInquiryMode). Otherwise it will be
     * set to BT_INVALID_RSSI.
     */
    S8              rssi;

    /* Extended Inquiry response.  Only valid when controller reports an
     * extended inquiry (also see ME_SetInquiryMode).  Otherwise it will be
     * set to all 0's.
     */
    U8              extInqResp[240];

    /* Describes the format of the current inquiry result */
    U8   inqMode;

} bt_Inquiry_result_t;

typedef struct
{
    /* Event causing callback. Always valid.*/
    uint8_t   eType;

    /* Error code. See BtEventType for guidance on whether errCode is valid. */
    uint8_t   errCode;

    /* Pointer to handler. Only valid for events directed to BtHandlers. */
    btif_handler   *handler;

    bt_bdaddr_t bdAddr;     /* Device Address */
    uint16_t conn_handle;   /* acl link conn handle */
    void *btm_conn;

    /* Parameters */
    union
    {
        btif_accessible_mode_t   aMode;      /* New access mode */
        void    *meToken;    /* Me command token */
        U8                 pMode;      /* Simple Pairing Mode */

#ifdef __TWS_RECONNECT_USE_BLE__
        struct
        {
            U8          *data;
            U16         len;
        } twsBleReport;
#endif
        bt_Inquiry_result_t *inqResult;

        void  *secToken;   /* Security Token */
        void    *token;

        /* Information for BTEVENT_PAIRING_COMPLETE */
        struct
        {
            btif_link_key_type_t   keyType;
        } pairingInfo;

        /* Information for BTEVENT_PIN_REQ */
        struct
        {
            /* If > 0 then the pin length returned in SEC_SetPin must be >= pinLen */
            U8              pinLen;
        } pinReq;

        /* Information for BTEVENT_SET_INQUIRY_MODE_CNF */
        btif_inquiry_mode_t      inqMode;

        /* Information for BTEVENT_SET_INQ_TX_PWR_LVL_CNF */
        S8       inqTxPwr;

        /* Information for BTEVENT_REMOTE_FEATURES */
        struct
        {
            U8      features[8];
        } remoteFeatures;

        /* Information for BTEVENT_REM_HOST_FEATURES */
        struct
        {
            U8      features[8];
        } remHostFeatures;

        /* Information for BTEVENT_LINK_SUPERV_TIMEOUT_CHANGED */
        struct
        {
            U16             timeout;
        } linkSupervision;

        /* Information for BTEVENT_MAX_SLOT_CHANGED */
        struct
        {
            U16             connHandle;
            U8              maxSlot;
        } maxSlotChanged;

        /* Information for BTEVENT_CONN_PACKET_TYPE_CHNG */
        struct
        {
            U16 connHandle;
            U16 packetType;
        } packetTypeChanged;

        /* Information for BTEVENT_QOS_SETUP_COMPLETE */
        struct
        {

            /* Reserved */
            U8 flags;

            /* Service Type: 0 = No Traffic, 1 = Best Effort, 2 Guaranteed */
            U8  serviceType;

            /* Token Rate in octets per second */
            U32 tokenRate;

            /* Peak Bandwidth in octets per second */
            U32 peakBandwidth;

            /* Latency in microseconds */
            U32 latency;

            /* Delay Variation in microseconds */
            U32 delayVariation;
        } qos;

        /* Result for BTEVENT_SET_SNIFF_SUBRATING_PARMS_CNF */
        btif_sniff_subrate_parms_t *sniffSubrateParms;

        /* Information for BTEVENT_SNIFF_SUBRATE_INFO */
        struct
        {
            /* Maximum latency for data being transmitted from the local
             * device to the remote device.
             */
            U16 maxTxLatency;

            /* Maximum latency for data being received by the local
             * device from the remote device.
             */
            U16 maxRxLatency;

            /* The base sniff subrate timeout in baseband slots that the
             * remote device shall use.
             */
            U16 minRemoteTimeout;

            /* The base sniff subrate timeout in baseband slots that the
             * local device will use.
             */
            U16 minLocalTimeout;
        } sniffSubrateInfo;

        /* Information for BTEVENT_CONFIRM_NUMERIC_REQ, BTEVENT_PASS_KEY_REQ,
         * and BTEVENT_DISPLAY_NUMERIC_IND
         */
        struct
        {
            U32             numeric;     /* Numeric value received from Secure
                                          * Simple Pairing (not valid for
                                          * BTEVENT_PASS_KEY_REQ
                                          */

            U8   bondingMode;  /* Bonding has been requested */
        } userIoReq;

        /* Result for BTEVENT_ENCRYPTION_CHANGE event. */
        struct
        {
            U8              mode; /* New encryption mode (uses the
                                   * BtEncryptMode type) */
        } encrypt;

        /* Result for BTEVENT_KEY_PRESSED */
        struct
        {
            U8  parm;   /* The value of the keypress parameter */
        } keyPress;

        struct
        {
            /* If disconnection was successful, contains BEC_NO_ERROR.
             * errCode will contain the disconnect reason.
             *
             * Unsuccessful disconnections will contain an error code
             * as generated by the radio. In this case, errCode can be
             * ignored.
             */
            btif_error_code_t status;
            U8 device_id;
        } disconnect;

        /* Result for the BTEVENT_SCO_DATA_CNF event */
        struct
        {
            void   *scoCon;     /* SCO connection */
            void       *scoPacket;  /* SCO Packet Handled */
        } scoPacketHandled;

        /* Result for the BTEVENT_SCO_CONNECT_CNF && BTEVENT_SCO_CONNECT_IND
           events.
         */
        struct
        {
            U16             scoHandle;  /* SCO Connection handle for HCI */
            void   *scoCon;     /* SCO connection */
            U8      scoLinkType;/* SCO link type */
            U8      device_id;  /* Belong device id */
            U8      interval;
            U8      window;
            void   *scoTxParms; /* Pointer to eSCO TX parameters */
            void   *scoRxParms; /* Pointer to eSCO RX parameters */
        } scoConnect;

        /* Result for the BTEVENT_SCO_DATA_IND event */
        struct
        {
            U16     scoHandle;  /* SCO Connection handle for HCI */
            void   *scoCon;     /* SCO connection. */
            U8              len;        /* SCO data len */
            U8             *ptr;        /* SCO data ptr */
            U8   errFlags;   /* Erroneous Data Reporting */
        } scoDataInd;

        /* Result for the BTEVENT_SECURITY_CHANGE and
         * BTEVENT_SECURITY3_COMPLETE events
         */
        struct
        {
            U8    mode;    /* New security mode (uses the BtSecurityMode
                            * type). */
            BOOL  encrypt; /* Indicate if encryption set or not */
        } secMode;

        /* Results for the BTEVENT_MODE_CHANGE event */
        struct
        {
            uint8_t      curMode;
            U16             interval;
        } modeChange;

        /* Results for BTEVENT_ROLE_CHANGE */
        struct
        {
            uint8_t   newRole;    /* New role */
        } roleChange;

        /* Results for BTEVENT_ACL_DATA_ACTIVE */
        struct
        {
            uint16_t   dataLen;    /* ACL data length */
        } aclDataActive;

        /* Results for BTEVENT_ACL_DATA_NOT_ACTIVE */
        struct
        {
            uint16_t   dataLen;    /* ACL data length */
        } aclDataNotActive;
        struct
        {
            uint8_t status;
            uint16_t name_len;
            const uint8_t *name; // 248 bytes in utf-8
        } name_rsp;
        struct
        {
            uint32_t passkey;
        } userPasskeyNotify;
    } edata;
} event_t;

typedef struct
{
    event_t  *evt;
    btif_event_mask_t mask;
} me_event_t;

typedef struct
{
    uint8_t link_id;
    uint32_t timeslice;
}__attribute__((packed)) me_link_env_t;

typedef enum
{
    A2DP_MODE = 0,
    ESCO_MODE,
    CIS_MODE,

    INVALID_MODE,
} link_traffic_mode_t;

enum btif_le_phy_rate
{
    /// 1 Mbits/s Rate
    LE_RATE_1MBPS   = 0,
    /// 2 Mbits/s Rate
    LE_RATE_2MBPS   = 1,
    /// 125 Kbits/s Rate
    LE_RATE_125KBPS = 2,
    /// 500 Kbits/s Rate
    LE_RATE_500KBPS = 3,
    /// Undefined rate (used for reporting when no packet is received)
    LE_RATE_UNDEF   = 4,

    LE_RATE_MAX     = 4,
};   //must be synced with @enum phy_rate

typedef void (*ibrt_disconnect_callback)(const btif_event_t *event);

bt_status_t btif_me_get_tws_slave_mobile_rssi(uint16_t ConnHandle);
void btif_me_set_sniffer_env(uint8_t sniffer_acitve, uint8_t sniffer_role,
                             uint8_t * monitored_addr, uint8_t * sniffer_addr);
BOOL btif_me_get_remote_device_initiator(btif_remote_device_t * rdev);
uint16_t btif_me_get_remote_device_hci_handle(btif_remote_device_t * rdev);
btif_remote_device_t* btif_me_get_remote_device_by_handle(uint16_t hci_handle);
uint16_t btif_me_get_conhandle_by_remote_address(bt_bdaddr_t *addr);
btif_remote_device_t* btif_me_get_remote_device_by_addr(const bt_bdaddr_t *remote);
uint16_t btif_me_get_hci_handle_by_remote_dev(btif_remote_device_t *p_remote_dev);
btif_remote_device_t* btif_me_get_remote_device_by_bdaddr(bt_bdaddr_t *bdaddr);
uint8_t btif_me_get_remote_device_op_optype(btif_remote_device_t * rdev);
btif_connection_role_t btif_me_get_remote_device_role(btif_remote_device_t * rdev);
void btif_me_set_remote_device_role(uint16_t conn_handle, uint8_t role);
BOOL *btif_me_get_remote_device_new_link_key(btif_remote_device_t * rdev);
void *btif_me_get_remote_device_parms(btif_remote_device_t * rdev);
bt_status_t btif_me_exchange_bt_addr(uint16_t connHandle);
bool btif_me_role_switch_pending(uint16_t handle);
bool btif_me_is_tws_role_switch_pending();
bool btif_me_is_exechange_bt_addr_pending();
bool is_btif_me_current_role_bcr_master(btif_remote_device_t * device);
bt_status_t btif_me_inquiry(uint32_t lap, uint8_t len, uint8_t maxResp);
bt_status_t btif_me_ble_add_dev_to_whitelist(uint8_t addr_type, bt_bdaddr_t * addr);
bt_status_t btif_me_ble_clear_whitelist(void);
bt_status_t btif_me_ble_set_private_address(bt_bdaddr_t * addr);
bt_status_t btif_me_ble_read_mesh_list(void);
bt_status_t btif_me_ble_clear_mesh_list(void);
bt_status_t btif_me_ble_add_dev_to_mesh_list(uint8_t addr_type, bt_bdaddr_t * addr);
bt_status_t btif_me_ble_remove_dev_to_mesh_list(uint8_t addr_type, bt_bdaddr_t * addr);
void btif_me_set_mesh_list_callback(void (*cb)(uint16_t opcode, uint8_t status, uint8_t size));
bt_status_t btif_me_ble_set_adv_data(uint8_t len, uint8_t * data);
bt_status_t btif_me_ble_set_scan_rsp_data(U8 len, U8 * data);
bt_status_t btif_me_ble_set_adv_parameters(btif_adv_para_struct_t * para);
bt_status_t btif_me_ble_set_adv_en(uint8_t en);
bt_status_t btif_me_ble_set_scan_parameter(btif_scan_para_struct_t * para);
bt_status_t btif_me_ble_set_scan_en(uint8_t scan_en, uint8_t filter_duplicate);
bt_status_t btif_me_ble_receive_adv_report(void (*cb)(const btif_ble_adv_report* report));
bt_status_t btif_me_ble_read_chnle_map(uint16_t conn_handle);
bt_status_t btif_me_ble_set_host_chnl_classification(uint8_t *chnl_map);

bt_status_t btif_sec_find_device_record(const bt_bdaddr_t * bdAddr,
                                        btif_device_record_t * record);

uint8_t btif_sec_set_io_capabilities(uint8_t ioCap);
uint8_t btif_sec_set_authrequirements(uint8_t authRequirements);
uint8_t btif_sec_set_general_bonding(bool general_bonding);
void btif_sec_allow_responder_trigger_auth(bool responder_trigger_auth);
void btif_sec_set_min_enc_key_size(uint8_t min_enc_key_size);
uint8_t btif_me_get_callback_event_type(const btif_event_t * event);
bt_bdaddr_t *btif_me_get_callback_event_address(const btif_event_t *event);
uint16_t btif_me_get_callback_event_handle(const btif_event_t *event);
btif_remote_device_t *btif_me_get_callback_event_rem_dev(const btif_event_t *event);
uint8_t btif_me_get_callback_event_rem_dev_role(const btif_event_t * event);
bt_bdaddr_t *btif_me_get_callback_event_rem_dev_bd_addr(const btif_event_t * event);
bt_bdaddr_t *btif_me_get_callback_event_disconnect_rem_dev_bd_addr(const btif_event_t *
        event);
btif_remote_device_t *btif_me_get_callback_event_disconnect_rem_dev(const btif_event_t *
        event);
btif_remote_device_t *btif_me_get_callback_event_role_change_rem_dev(const btif_event_t *event);
uint8_t btif_me_get_callback_event_disconnect_rem_dev_disc_reason_saved(const btif_event_t *
        event);
uint8_t btif_me_get_callback_event_disconnect_rem_dev_disc_reason(const btif_event_t *
        event);
uint8_t btif_me_reset_this_device_to_source(uint8_t device_id);

uint8_t btif_me_get_pendCons(void);

uint8_t btif_me_get_source_activeCons(void);

void btif_me_set_pendCons(uint8_t pendCons);

void btif_me_reset_l2cap_sigid(const bt_bdaddr_t *addr);

uint8_t btif_me_get_callback_event_max_slot(const btif_event_t *event);
uint16_t btif_me_get_callback_event_packet_type(const btif_event_t * event);

btif_remote_device_t *btif_me_get_callback_event_sco_connect_rem_dev(const btif_event_t *event);
uint8_t btif_me_get_callback_event_role_change_new_role(const btif_event_t * event);
bt_bdaddr_t *btif_me_get_callback_event_inq_result_bd_addr(const btif_event_t * event);
bt_bdaddr_t *btif_me_get_callback_event_name_rsp_bd_addr(const btif_event_t * event);
uint8_t *btif_me_get_callback_event_inq_result_bd_addr_addr(const btif_event_t * event);
uint8_t btif_me_get_callback_event_inq_result_inq_mode(const btif_event_t * event);
uint8_t btif_me_get_callback_event_rssi(const btif_event_t *event);
uint8_t *btif_me_get_callback_event_inq_result_ext_inq_resp(const btif_event_t * event);
uint32_t btif_me_get_callback_event_inq_result_classofdevice(const btif_event_t *event);
uint8_t btif_me_get_callback_event_err_code(const btif_event_t * event);
uint8_t btif_me_get_callback_event_a_mode(const btif_event_t * event);
uint16_t btif_me_get_callback_event_max_slot_changed_connHandle(const btif_event_t * event);
uint8_t btif_me_get_callback_event_max_slot_changed_max_slot(const btif_event_t * event);
uint8_t btif_me_get_callback_event_mode_change_curMode(const btif_event_t * event);
uint16_t btif_me_get_callback_event_mode_change_interval(const btif_event_t * event);

uint16_t btif_me_get_callback_event_remote_dev_name(const btif_event_t * event, const uint8_t** ppName);
uint32_t btif_me_get_callback_event_passkey_notify_passkey(const btif_event_t *event);

bt_status_t btif_me_get_remote_device_name(const bt_bdaddr_t * bdAddr, btif_global_handle handler);

uint8_t btif_me_get_ext_inq_data(uint8_t * eir, btif_eir_data_type_t type,
                                 uint8_t * outBuffer, uint8_t Length);
bt_status_t btif_me_cancel_inquiry(void);
bt_status_t btif_sec_delete_device_record(const bt_bdaddr_t * bdAddr);
bt_status_t btif_me_cancel_create_link(btif_handler * handler,
                                       btif_remote_device_t * rdev);
void btif_me_set_handler(void *handler, btif_callback cb);
bool btif_me_current_bt_role_is_master(btif_remote_device_t *rem_dev_ptr);
bool btif_me_current_bt_role_is_slave(btif_remote_device_t *rem_dev_ptr);
bool btif_me_is_conn_preferred_as_slave(btif_remote_device_t *rem_dev_ptr);
bool btif_me_is_conn_preferred_as_master(btif_remote_device_t *rem_dev_ptr);
bt_status_t btif_me_disconnect_link(btif_handler * handler,
                                    btif_remote_device_t * rdev);
bt_status_t btif_me_fault_free_link(uint16 conn_handle);
bt_status_t btif_me_set_link_policy(btif_remote_device_t *rdev, btif_link_policy_t policy);
bt_status_t btif_me_set_link_lowlayer_monitor(btif_remote_device_t * rdev, uint8_t control_flag,uint8_t report_format,
        uint32_t data_format,uint8_t report_unit);
bt_status_t btif_me_set_link_lowlayer_monitor_by_handle(uint16_t connhdl, uint8_t control_flag,uint8_t report_format,
        uint32_t data_format,uint8_t report_unit);
void btif_me_fake_tws_disconnect(uint16_t hci_handle, uint8_t reason);
void btif_me_fake_mobile_disconnect(uint16_t hci_handle, uint8_t reason);
void btif_me_fake_tws_connect(uint8_t status, bt_bdaddr_t * bdAddr);
void btif_me_reset_bt_controller(void);
void btif_me_fake_mobile_connect(uint8_t status, uint16_t hci_handle, bt_bdaddr_t *bdAddr);

bt_status_t btif_me_set_lbrt_enable(uint16_t connHandle, uint8_t enable);
bt_status_t btif_me_set_accessible_mode(btif_accessible_mode_t mode,
                                        const btif_access_mode_info_t * info);
bt_status_t btif_me_write_page_timeout(uint16_t timeout);
bt_status_t btif_me_switch_role(uint16_t conn_handle);
void btif_me_set_bt_source_event_handler(btif_handler *handler);
bt_status_t btif_me_register_global_handler(void *handler);
void *btif_me_register_accept_handler(void *handler);
bt_status_t btif_me_set_event_mask(void *handler, btif_event_mask_t mask);
void *btif_me_get_bt_handler(void);
bt_status_t btif_me_set_bt_address(const uint8_t * btAddr);
bt_status_t btif_me_set_local_device_name(const U8 *name, U8 length);
bt_status_t btif_me_set_ble_bd_address(const uint8_t * btAddr);
bt_status_t btif_me_set_ble_tx_pwr(uint16_t connhdl, int8_t tx_pwr);
bt_status_t btif_me_write_automatic_flush_timeout(uint16 connhandle, uint16 flush_timeout);
bt_status_t btif_sec_add_device_record(btif_device_record_t * record);
bt_status_t btif_enum_device_record(U16 dev_id, btif_device_record_t *record);
bt_bdaddr_t *btif_me_get_remote_device_bdaddr(const btif_remote_device_t * rdev);
btif_rem_dev_state_t btif_me_get_remote_device_state(btif_remote_device_t * rdev);
bool btif_me_get_remote_device_cod(btif_remote_device_t *rdev, uint8_t *cod);
btif_link_mode_t btif_me_get_remote_device_mode(btif_remote_device_t * rdev);
btif_authorize_state_t btif_me_get_remote_device_auth_state(btif_remote_device_t * rdev);
bt_status_t btif_me_write_link_superv_timeout(uint16_t handle, uint16_t slots);
btif_link_mode_t btif_me_get_current_mode(btif_remote_device_t * rdev);
btif_connection_role_t btif_me_get_current_role(btif_remote_device_t * rdev);

bt_status_t btif_me_start_sniff(uint16_t conn_handle, btif_sniff_info_t* info);

bt_status_t btif_me_stop_sniff(uint16_t conn_handle);

bt_status_t btif_me_accept_incoming_link(const bt_bdaddr_t *remote, btif_connection_role_t role);

bt_status_t btif_me_reject_incoming_link(const bt_bdaddr_t *remote, btif_error_code_t reason);

void btif_me_response_acl_conn_req(bt_bdaddr_t *remote, bool accept, uint8_t reason);

bt_status_t btif_me_start_tws_role_switch(uint16_t slaveConnHandle, uint16_t mobileConnHandle);
bt_status_t btif_me_set_sco_tx_silence(uint16_t connHandle, uint8_t silence_on);
uint16_t btif_me_get_sco_hcihandle_by_addr(const bt_bdaddr_t *addr);
bool btif_me_is_sending_data_to_peer_dev_pending(void);
btif_handler *btif_me_get_me_handler(void);
bt_status_t btif_me_force_disconnect_link_with_reason(uint16_t connhdl, uint8_t reason, bool forceDisconnect);
void btif_me_write_bt_sleep_enable(uint8_t sleep_en);
void btif_me_write_bt_page_scan_type(uint8_t scan_type);
void btif_me_write_bt_inquiry_scan_type(uint8_t scan_type);

#if IS_USE_INTERNAL_ACL_DATA_PATH
bt_status_t btif_me_send_data_to_peer_dev(uint16_t connHandle, uint8_t dataLen, uint8_t * data);
#endif                          /*  */
void btif_me_init_handler(btif_handler * handler);
bt_status_t btif_me_dbg_sniffer_interface(uint16_t connHandle, uint8_t subCode);
uint8_t *btif_me_get_remote_device_version(btif_remote_device_t * rdev);
void *btif_me_get_cmgr_handler();
bt_status_t btif_bind_cmgr_handler(void *cmgr_handler, bt_bdaddr_t * bdAddr,btif_cmgr_callback Callback);
bt_status_t btif_create_acl_to_slave(const bt_bdaddr_t * bdAddr);
bt_status_t btif_register_cmgr_handle(void *cmgr_handler,btif_cmgr_callback Callback);
void btif_me_unregister_globa_handler(btif_handler * handler);
void btif_me_set_inquiry_mode(uint8_t mode);
void btif_me_inquiry_result_setup(uint8_t *inquiry_buff, bool rssi,
                                  bool extended_mode);
btif_remote_device_t *btif_me_enumerate_remote_devices(uint32_t devid);

bool btif_me_get_remote_dip_queried(btif_remote_device_t *rdev);
void btif_me_set_remote_dip_queried(const bt_bdaddr_t *remote, bool queried);
bool btif_me_is_remote_dip_query_on_going(btif_remote_device_t *rdev);
bool btif_me_is_connection_simple_pairing_completed(const bt_bdaddr_t *remote);
void btif_me_set_remote_dip_query_state(const bt_bdaddr_t *remote, bool isInQuery);

uint8_t btif_me_get_remote_sevice_encrypt_state(btif_remote_device_t* rdev);

uint8_t btif_me_get_remote_device_disc_reason_saved(btif_remote_device_t * device);

void btif_me_register_snoop_acl_connection_callback(void (*conn)(uint8_t device_id, void* remote, void* btm_conn), void (*disc)(uint8_t device_id, void* remote));

void btif_me_register_pending_too_many_rx_acl_packets_callback(void (*cb)(void));

uint8_t btif_me_get_remote_device_disc_reason(btif_remote_device_t * device);
bool btif_me_wait_acl_complete(void* remote, void (*cb)(uint8_t device_id, void* remote, bool succ, uint8_t errcode));

uint8_t btif_me_get_device_id_from_addr(const bt_bdaddr_t *addr);
uint8_t btif_me_get_device_id_from_rdev(btif_remote_device_t *rdev);
bool btif_me_stop_pending_page_activity(const bt_bdaddr_t* addr);

void  btif_me_event_report(me_event_t *event);

void btif_me_set_creating_source_link(const bt_bdaddr_t *remote, bool mark_as_source);
bool btif_me_is_creating_source_link(const void *remote);

void btif_me_set_accepting_source_link(bool set_source_link);
bool btif_me_is_accepting_source_link(const bt_bdaddr_t *remote);

void btif_me_register_is_creating_source_link(bool (*cb)(const void *remote));

void btif_me_init_peer_headset_addr(uint8_t *p_remote_addr);
bt_bdaddr_t * btif_me_get_peer_headset_addr(void);
uint8_t btif_me_get_remote_device_link_mode(btif_remote_device_t* rdev);
uint8_t btif_me_get_remote_device_bt_role(btif_remote_device_t* rdev);
bt_status_t btif_me_change_packet_type(btif_remote_device_t *rdev, btif_acl_packet packetTypes);
bt_status_t btif_me_read_controller_memory(uint32_t addr, uint32_t len,uint8_t type);
bt_status_t btif_me_write_controller_memory(uint32_t addr,uint32_t val,uint8_t type);
bool btif_me_is_in_sniff_mode(bt_bdaddr_t *remote);
bool btif_me_is_in_active_mode(bt_bdaddr_t *remote);

void btif_me_set_conn_tws_link(uint16_t conn_handle, uint8_t is_tws_link);
bool btif_me_get_conn_tws_link(void *conn);
bt_status_t btif_me_bt_dbg_send_prefer_rate(uint16_t conhdl,uint8_t rate);
bt_status_t btif_me_bt_dbg_set_txpwr_link_thd(uint8_t index, uint8_t enable,uint8_t link_id,
    uint16_t rssi_avg_nb_pkt, int8_t rssi_high_thd, int8_t rssi_low_thd, int8_t rssi_below_low_thd, int8_t rssi_interf_thd);
bt_status_t btif_me_read_avg_rssi(uint16_t connhld);

bt_status_t btif_me_qos_set_up(uint16_t conn_handle);
bt_status_t btif_me_qos_setup_with_tpoll(uint16_t conn_handle, uint32_t tpoll_slot);

bt_status_t btif_me_qos_setup_with_tpoll_generic(uint16_t conn_handle, uint32_t tpoll_slot, uint8_t service_type);

void btif_report_bt_event(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, void *param);

/**
 ****************************************************************************************
 * @brief bt set channel classification map which related Set AFH Host Channel Classification command
 * AFH_Host_Channel_Classification: Size: 10 octets (79 bits meaningful)
 * This parameter contains 80 1-bit fields
 * The nth such field (in the range 0 to 78) contains the value for channel n:
 *    0: channel n is bad
 *    1: channel n is unknown
 *
 * The most significant bit (bit 79) is reserved for future use
 * At least (Nmin == 20) channels shall be marked as unknown.
 *
 * default all bits value is 1
 ****************************************************************************************
 */
bt_status_t  btif_me_set_afh_chnl_classification(uint8_t *chnl_map);
bt_status_t btif_me_input_user_passkey(btif_remote_device_t *rdev,uint32_t passkey);
uint32_t btif_me_get_user_passkey(btif_remote_device_t *rdev);
void btif_me_user_passkey_input_timeout_ms(uint32_t timeout_ms);
void btif_me_set_host_pin_code(uint8_t *pin, uint8_t pinlen);
void btif_me_write_host_ssp_mode(uint8_t enable);
uint8_t btif_me_get_testmode_enable(void);
#if mHDT_SUPPORT
bt_status_t btif_me_mhdt_enter_mhdt_mode(btif_remote_device_t *rdev,uint8 tx_rates,uint8 rx_rates);
bt_status_t btif_me_mhdt_exit_mhdt_mode(btif_remote_device_t *rdev);
bt_status_t btif_me_mhdt_write_supervision_timeout(btif_remote_device_t *rdev,uint16 mhdt_timeout);
bt_status_t btif_me_mhdt_read_supervision_timeout(btif_remote_device_t *rdev);
bt_status_t btif_me_mhdt_get_bt_data_rate(btif_remote_device_t *rdev,uint8 *tx_rates,uint8 *rx_rates);
#endif
void btif_me_write_dbg_sniffer(const uint8_t subcode, const uint16_t connhandle);
void btif_me_register_get_local_device_callback(void (*cb)(void *remote));
void btif_me_register_notify_save_creadit_callback(void (*cb)(const bt_bdaddr_t *remote));
void btif_me_write_tws_link(uint8_t link_id);
void btif_me_write_btpcm_en(bool en);
bt_remver_t btif_me_get_remote_version_by_handle(uint16_t conn_handle);
uint8_t btif_me_get_remote_class_of_device(btif_remote_device_t *rdev);
bt_status_t btif_dbg_ibrt_update_time_slice(uint8_t nb, me_link_env_t* multi_ibrt);
bt_status_t btif_dbg_set_bt_ble_active_link(link_traffic_mode_t traffic_mode, uint16_t link_handle);
#ifdef IS_BLE_AUDIO_DEBUG_INFO_COLLECTOR_ENABLED
bt_status_t btif_me_enable_ble_audio_dbg_trc_report(bool isEnabled);
#endif

#if defined(IBRT)
bt_status_t btif_me_enable_fastack(uint16_t conhdl, uint8_t direction, uint8_t enable);
bt_status_t btif_me_start_ibrt(U16 slaveConnHandle, U16 mobileConnHandle);
bt_status_t btif_me_stop_ibrt(uint16_t mobile_conhdl,uint8_t reason);
bt_status_t btif_me_suspend_ibrt(void);
bt_status_t btif_me_ibrt_mode_init(bool enable);
bt_status_t btif_me_ibrt_role_switch(uint16_t mobile_conhdl);
void btif_me_set_devctx_state(uint8_t acl_array_idx, uint16_t state);
void btif_me_set_devctx_link(uint8_t acl_array_idx, btif_remote_device_t * rm_dev);
bt_bdaddr_t*  btif_me_get_devctx_btaddr(uint8_t acl_array_idx);
btif_remote_device_t* btif_me_get_remote_device(uint8_t acl_array_idx);
void btif_me_free_tws_outgoing_dev(uint8_t *peer_tws_addr);
btif_remote_device_t*  btif_me_get_devctx_remote_device(uint8_t acl_array_idx);

uint32_t btif_me_save_record_ctx(uint8_t *ctx_buffer, uint8_t *addr);
uint32_t btif_me_set_record_ctx(uint8_t *ctx_buffer,  uint8_t *addr);
uint32_t btif_me_save_me_ctx(uint8_t *ctx_buffer, uint16_t dev_id);
uint32_t btif_me_set_me_ctx(uint8_t *ctx_buffer, uint16_t dev_id);
uint8_t btif_me_get_mobile_avdev_index(uint16_t mobile_handle);
uint8_t btif_me_get_free_avdev_index(void);
uint32_t btif_me_save_avdev_ctx(uint8_t*  ctx_buffer, uint16_t dev_id);
uint32_t btif_me_set_avdev_ctx(uint8_t *ctx_buffer, uint16_t dev_id, uint8_t rm_devtbl_idx);
void btif_me_conn_auto_accept_enable(uint8_t *condition);

bt_status_t btif_me_resume_ibrt(uint8_t enable);
void btif_me_ibrt_simu_hci_event_disallow(uint8_t opcode1, uint8_t opcode2);
void btif_me_register_get_ibrt_handle_callback(uint16_t (*cb)(void* remote));
void btif_me_set_ecc_ibrt_data_test(uint8_t  ecc_data_test_en, uint8_t ecc_data_len, uint16_t ecc_count, uint32_t data_pattern);
bt_status_t btif_me_ibrt_conn_connected(bt_bdaddr_t *bt_addr, uint16_t conhdl);
bt_status_t btif_me_ibrt_conn_disconnected(bt_bdaddr_t *bt_addr, uint16_t conhdl, uint8_t status, uint8_t reason);
void btif_me_send_prefer_rate(uint16_t connhdl, uint8_t rate);
uint16_t btif_me_get_ibrt_hci_handle(bt_bdaddr_t *bd_addr);
void btif_me_configure_keeping_both_scan(bool isToKeepBothScan);
uint8_t btif_me_get_mobile_link_num(void);
btif_remote_device_t *btif_me_conn_acl_search_by_handle(uint16 conn_handle);
#endif

void btif_me_set_testmode_enable(uint8_t enable);
void btif_me_chip_init_noraml_test_mode_switch(void);

uint8_t btif_me_get_callback_event_encrypt_mode(const btif_event_t * event);
btif_dev_linkkey *btif_me_get_callback_link_key(const btif_event_t *event);
bt_status_t btif_me_auth_req(uint16_t conn_handle);
void btif_me_write_scan_activity_specific(uint16_t opcode, uint16_t scan_interval, uint16_t scan_window);
uint16_t btif_me_get_connhandle_by_addr(const bt_bdaddr_t *addr);
bt_status_t btif_me_bt_dbg_set_iso_quality_rep_thr(uint16_t conn_handle, uint16_t qlty_rep_evt_cnt_thr,
    uint16_t tx_unack_pkts_thr, uint16_t tx_flush_pkts_thr, uint16_t tx_last_subevent_pkts_thr, uint16_t retrans_pkts_thr,
    uint16_t crc_err_pkts_thr, uint16_t rx_unreceived_pkts_thr, uint16_t duplicate_pkts_thr);
bt_status_t btif_me_bt_dbg_le_tx_power_request(uint16_t conn_handle, uint8_t enable, int8_t delta, enum btif_le_phy_rate rx_rate);
void btif_register_debug_trace_callback(void (*cb)(uint8_t* p_buf, uint16_t buf_len));
#ifdef __cplusplus
}
#endif                          /*  */
#endif                          /* __ME_H */
