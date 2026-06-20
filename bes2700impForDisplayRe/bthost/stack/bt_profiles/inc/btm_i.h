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
#ifndef __BTM_I_H__
#define __BTM_I_H__
#include "bluetooth.h"
#include "co_ppbuff.h"
#include "hci_i.h"
#include "btm_vendor.h"
#include "btm_mhdt.h"
#if defined(__cplusplus)
extern "C" {
#endif

#define BT_ECODE_NO_ERROR                   0x00
#define BT_ECODE_UNKNOWN_HCI_CMD            0x01
#define BT_ECODE_NO_CONNECTION              0x02
#define BT_ECODE_HARDWARE_FAILURE           0x03
#define BT_ECODE_PAGE_TIMEOUT               0x04
#define BT_ECODE_AUTHENTICATE_FAILURE       0x05
#define BT_ECODE_MISSING_KEY                0x06
#define BT_ECODE_MEMORY_FULL                0x07
#define BT_ECODE_CONNECTION_TIMEOUT         0x08
#define BT_ECODE_MAX_CONNECTIONS            0x09
#define BT_ECODE_MAX_SCO_CONNECTIONS        0x0a
#define BT_ECODE_ACL_ALREADY_EXISTS         0x0b
#define BT_ECODE_COMMAND_DISALLOWED         0x0c
#define BT_ECODE_LIMITED_RESOURCE           0x0d
#define BT_ECODE_SECURITY_ERROR             0x0e
#define BT_ECODE_PERSONAL_DEVICE            0x0f
#define BT_ECODE_CONN_ACCEPT_TIMEOUT        0x10
#define BT_ECODE_UNSUPPORTED_FEATURE        0x11
#define BT_ECODE_INVALID_HCI_PARM           0x12
#define BT_ECODE_REMOTE_USER_TERMINATED     0x13
#define BT_ECODE_LOW_RESOURCES              0x14
#define BT_ECODE_POWER_OFF                  0x15
#define BT_ECODE_LOCAL_TERMINATED           0x16
#define BT_ECODE_REPEATED_ATTEMPTS          0x17
#define BT_ECODE_PAIRING_NOT_ALLOWED        0x18
#define BT_ECODE_UNKNOWN_LMP_PDU            0x19
#define BT_ECODE_UNSUPPORTED_REMOTE         0x1a
#define BT_ECODE_SCO_OFFSET_REJECT          0x1b
#define BT_ECODE_SCO_INTERVAL_REJECT        0x1c
#define BT_ECODE_SCO_AIR_MODE_REJECT        0x1d
#define BT_ECODE_INVALID_LMP_PARM           0x1e
#define BT_ECODE_UNSPECIFIED_ERR            0x1f
#define BT_ECODE_UNSUPPORTED_LMP_PARM       0x20
#define BT_ECODE_ROLE_CHG_NOT_ALLOWED       0x21
#define BT_ECODE_LMP_RESPONSE_TIMEOUT       0x22
#define BT_ECODE_LMP_TRANS_COLLISION        0x23
#define BT_ECODE_LMP_PDU_NOT_ALLOWED        0x24
#define BT_ECODE_ENCRYP_MODE_NOT_ACC        0x25
#define BT_ECODE_UNIT_KEY_USED              0x26
#define BT_ECODE_QOS_NOT_SUPPORTED          0x27
#define BT_ECODE_INSTANT_PASSED             0x28
#define BT_ECODE_PAIR_UNITKEY_NO_SUPP       0x29
#define BT_ECODE_NOT_FOUND                  0xf1
#define BT_ECODE_REQUEST_CANCELLED          0xf2

enum device_mode_dis_enum{
    DEVICE_MODE_DISCOVERABLE = 0x01,
    DEVICE_MODE_NON_DISCOVERABLE = 0x0
};
enum device_mode_conn_enum{
    DEVICE_MODE_CONNECTABLE = 0x01,
    DEVICE_MODE_NON_CONNECTABLE = 0x0,
    DEVICE_MODE_NO_CHANGE = 0x04
};

enum acl_pkt_boundary_enum{
    ACL_START = 0x02,
    ACL_CONTINUE = 0x01
};
enum btm_security_event_enum {
    BTM_SECURITY_AUTORITY_SUCCESS=1,
    BTM_SECURITY_AUTORITY_FAILURE
};

enum btm_l2cap_event_enum {
    BTM_EV_CONN_ACL_OPENED=1,
    BTM_EV_CONN_ACL_CLOSED,
    BTM_EV_SECURITY_AUTORITY_SUCCESS,
    BTM_EV_SECURITY_AUTORITY_FAILURE
};

enum conn_sco_stat_enum{
    BTM_CONN_SCO_CLOSED,
    BTM_CONN_SCO_CONNECTING,
    BTM_CONN_SCO_OPENED,
    BTM_CONN_SCO_CONN_FAIL,
    BTM_CONN_SCO_WAIT      /*the sco connection is waiting for the acl connection to be connected first*/
};

enum btm_pairing_event
{
	PAIRING_OK,
	PAIRING_TIMEOUT,
	PAIRING_FAILED,
	UNPAIR_OK
};	// change

/* enum for inquiry events */
enum btm_inquiry_event {
	INQUIRY_DONE,			// inquiry is done
	NEW_REMOTE_DEV_IND,		// found a new remote device
	INQUIRY_CANCEL_OK,      // cancel inquiry
	INQUIRY_CANCEL_FAIL
};

enum btm_name_event
{
	NAME_DONE,
	NAME_FAIL,
	NAME_CANCEL_OK,
	NAME_CANCEL_FAIL
};

enum btm_check_hfp_codec_enable_type
{
    BTM_CHECK_MSBC_ENABLE,
    BTM_CHECK_CVSD_ENABLE,
    BTM_CHECK_VENDER_ENABLE,
    BTM_CHECK_LC3_ENABLE,
};

typedef void (*btm_pairing_callback_t)(enum btm_pairing_event event,void *pdata);

typedef void (*btm_chip_init_ready_callback_t)(int status);

/**
 * bt event definition to application layer
 */
#define BTEVENT_INQUIRY_RESULT                  1
#define BTEVENT_INQUIRY_COMPLETE                2
#define BTEVENT_INQUIRY_CANCELED                3
#define BTEVENT_LINK_CONNECT_IND                4
#define BTEVENT_SCO_CONNECT_IND                 5
#define BTEVENT_LINK_DISCONNECT                 6
#define BTEVENT_LINK_CONNECT_CNF                7
#define BTEVENT_ROLE_DISCOVERED                 8
#define BTEVENT_MODE_CHANGE                     9
#define BTEVENT_ACCESSIBLE_CHANGE               10
#define BTEVENT_AUTHENTICATED                   11
#define BTEVENT_ENCRYPTION_CHANGE               12
#define BTEVENT_SECURITY_CHANGE                 13
#define BTEVENT_ROLE_CHANGE                     14
#define BTEVENT_SCO_DISCONNECT                  15
#define BTEVENT_SCO_CONNECT_CNF                 16
#define BTEVENT_SIMPLE_PAIRING_COMPLETE         17
#define BTEVENT_REMOTE_FEATURES                 18
#define BTEVENT_REM_HOST_FEATURES               19
#define BTEVENT_LINK_SUPERV_TIMEOUT_CHANGED     20
#define BTEVENT_SET_SNIFF_SUBRATING_PARMS_CNF   21
#define BTEVENT_SNIFF_SUBRATE_INFO              22
#define BTEVENT_SET_INQUIRY_MODE_CNF            23
#define BTEVENT_SET_INQ_TX_PWR_LVL_CNF          24
#define BTEVENT_SET_EXT_INQUIRY_RESP_CNF        25
#define BTEVENT_SET_ERR_DATA_REPORTING_CNF      26
#define BTEVENT_KEY_PRESSED                     27
#define BTEVENT_QOS_SETUP_COMPLETE              28

#ifdef __TWS_RECONNECT_USE_BLE__
#define BTEVENT_TWS_BLE_ADV_REPORT_EVENT        29
#endif /* __TWS_RECONNECT_USE_BLE__ */

/* Indicates the name of a remote device or cancellation of a name request. */
#define BTEVENT_NAME_RESULT                     30
#define BTEVENT_SCO_DATA_IND                    31
/**
 * Outgoing SCO data has been sent and the packet is free for re-use by
 * the application.
 */
#define BTEVENT_SCO_DATA_CNF                    32
#define BTEVENT_LINK_CONNECT_REQ                33
/* Incoming link accept complete. */
#define BTEVENT_LINK_ACCEPT_RSP                 34
/* Incoming link reject complete. */
#define BTEVENT_LINK_REJECT_RSP                 35
#define BTEVENT_COMMAND_COMPLETE                36
#define BTEVENT_SCO_CONNECT_REQ                 37
/* Set Audio/Voice settings complete. */
#define BTEVENT_SCO_VSET_COMPLETE               38
/* SCO link connection process started. */
#define BTEVENT_SCO_STARTED                     39
/* Select Device operation complete, "p.select" is valid. */
#define BTEVENT_DEVICE_SELECTED                 40
/* The eSCO connection has changed. "p.scoConnect" is valid. */
#define BTEVENT_SCO_CONN_CHNG                   41

/* Group: Security-related events. */

/* Indicates access request is successful. "p.secToken" is valid. */
#define BTEVENT_ACCESS_APPROVED                 50
/* Indicates access request failed. "p.secToken" is valid. */
#define BTEVENT_ACCESS_DENIED                   51
/**
 * Request authorization when "errCode" is BEC_NO_ERROR.
 * "p.remDev" is valid.
 */
#define BTEVENT_AUTHORIZATION_REQ               52
/**
 * Request a Pin for pairing when "errCode" is BEC_NO_ERROR.
 * "p.pinReq" is valid. If p.pinReq.pinLen is > 0 then SEC_SetPin()
 * must be called in response to this event with a pin length >=
 * p.pinReq.pinLen.
 */
#define BTEVENT_PIN_REQ                         53
/* Pairing operation is complete. */
#define BTEVENT_PAIRING_COMPLETE                54
/* Authentication operation complete. "p.remDev" is valid. */
#define BTEVENT_AUTHENTICATE_CNF                55
/* Encryption operation complete. "p.remDev" is valid. */
#define BTEVENT_ENCRYPT_COMPLETE                56
/* Security mode 3 operation complete. "p.secMode" is valid. */
#define BTEVENT_SECURITY3_COMPLETE              57
/* A link key is returned. "p.bdLinkKey" is valid. */
#define BTEVENT_RETURN_LINK_KEYS                58
/* Out of Band data has been received from the host controller. */
#define BTEVENT_LOCAL_OOB_DATA                  59
/**
 * Request a Pass Key for simple pairing when "errCode" is BEC_NO_ERROR. The
 * application should call SEC_SetPassKey() to provide the passkey or reject
 * the request, and optionally save the link key.
 */
#define BTEVENT_PASS_KEY_REQ                    60
/**
 * Request a User Confirmation for simple pairing when "errCode" is
 * BEC_NO_ERROR.
 */
#define BTEVENT_CONFIRM_NUMERIC_REQ             61
#define BTEVENT_DISPLAY_NUMERIC_IND             62
#define BTEVENT_CONN_PACKET_TYPE_CHNG           63
#define SDEVENT_QUERY_RSP                       70
#define SDEVENT_QUERY_ERR                       71
#define SDEVENT_QUERY_FAILED                    72
#define BTEVENT_SELECT_DEVICE_REQ               80
#define BTEVENT_DEVICE_ADDED                    81
#define BTEVENT_DEVICE_DELETED      	        82
#define BTEVENT_MAX_SLOT_CHANGED		        83
#define BTEVENT_SNIFFER_CONTROL_DONE 	        84
#define BTEVENT_LINK_POLICY_CHANGED		        85
/* added for pass command status to up level */
#define BTEVENT_COMMAND_STATUS	    	        86
#define BTEVENT_VENDOR_EVENT	    	        87
#define BTEVENT_COMMAND_CMP_EVENT	            88
#define BTEVENT_USER_PASSKEY_NOTIFY_EVENT	    89
#define BTEVENT_USER_PASSKEY_REQUEST_EVENT	    90
#define BTEVENT_SET_BT_BLE_ACTIVE_LINK_COMPLETE 93
/**
 * Indicate that an ACL connection is sending or receiving data
 * while it is in active mode. Then, keep resetting the sniff timer.
 */
#define BTEVENT_ACL_DATA_ACTIVE                 98
/**
 * an ACL connection has received an internal data transmit
 * request while it is in hold, park or sniff mode. The data will still be
 * passed to the radio in park and sniff modes. However, hold mode will
 * block data transmit. It may be necessary to return the ACL to active
 * mode to restore normal data transfer.
 */
#define BTEVENT_ACL_DATA_NOT_ACTIVE             99
/* Indicates that the HCI failed to initialize. */
#define BTEVENT_HCI_INIT_ERROR                  100
#define BTEVENT_HCI_INITIALIZED                 101
/* Indicates that a fatal error has occurred in the radio or the HCI transport. */
#define BTEVENT_HCI_FATAL_ERROR                 102
/* Indicates that the HCI has been deinitialized. */
#define BTEVENT_HCI_DEINITIALIZED               103
/* Indicates that the HCI cannot be initialized. */
#define BTEVENT_HCI_FAILED                      104
#define BTEVENT_HCI_COMMAND_SENT                105


typedef uint32_t event_mask_t;

#define BTM_EVTMASK_NO_EVENTS                    0x00000000
#define BTM_EVTMASK_INQUIRY_RESULT               0x00000001
#define BTM_EVTMASK_INQUIRY_COMPLETE             0x00000002
#define BTM_EVTMASK_INQUIRY_CANCELED             0x00000004
#define BTM_EVTMASK_LINK_CONNECT_IND             0x00000008
#define BTM_EVTMASK_SCO_CONNECT_IND              0x00000010
#define BTM_EVTMASK_LINK_DISCONNECT              0x00000020
#define BTM_EVTMASK_LINK_CONNECT_CNF             0x00000040
#define BTM_EVTMASK_ROLE_DISCOVERED              0x00000080
#define BTM_EVTMASK_MODE_CHANGE                  0x00000100
#define BTM_EVTMASK_ACCESSIBLE_CHANGE            0x00000200
#define BTM_EVTMASK_AUTHENTICATED                0x00000400
#define BTM_EVTMASK_ENCRYPTION_CHANGE            0x00000800
#define BTM_EVTMASK_SECURITY_CHANGE              0x00001000
#define BTM_EVTMASK_ROLE_CHANGE                  0x00002000
#define BTM_EVTMASK_SCO_DISCONNECT               0x00004000
#define BTM_EVTMASK_SCO_CONNECT_CNF              0x00008000
#define BTM_EVTMASK_SIMPLE_PAIRING_COMPLETE      0x00010000
#define BTM_EVTMASK_REMOTE_FEATURES              0x00020000
#define BTM_EVTMASK_REM_HOST_FEATURES            0x00040000
#define BTM_EVTMASK_LINK_SUPERV_TIMEOUT_CHANGED  0x00080000
#define BTM_EVTMASK_SET_SNIFF_SUBR_PARMS         0x00100000
#define BTM_EVTMASK_SNIFF_SUBRATE_INFO           0x00200000
#define BTM_EVTMASK_SET_INQ_MODE                 0x00400000
#define BTM_EVTMASK_SET_INQ_RSP_TX_PWR           0x00800000
#define BTM_EVTMASK_SET_EXT_INQ_RESP             0x01000000
#define BTM_EVTMASK_SET_ERR_DATA_REP             0x02000000
#define BTM_EVTMASK_KEY_PRESSED                  0x04000000
#define BTM_EVTMASK_CONN_PACKET_TYPE_CHNG        0x08000000
#define BTM_EVTMASK_QOS_SETUP_COMPLETE           0x10000000
#define BTM_EVTMASK_MAX_SLOT_CHANGED             0x20000000
#define BTM_EVTMASK_SNIFFER_CONTROL_DONE         0x40000000
#define BTM_EVTMASK_LINK_POLICY_CHANGED	         0x80000000
#define BTM_EVTMASK_ALL_EVENTS                   0xffffffff

#define BTM_ACL_ST_DISCONNECTED  0x00
#define BTM_ACL_ST_OUT_CON       0x01
#define BTM_ACL_ST_IN_CON1       0x02  /*income but not accept*/
#define BTM_ACL_ST_CONNECTED     0x03
#define BTM_ACL_ST_OUT_DISC      0x04
#define BTM_ACL_ST_OUT_DISC2     0x05
#define BTM_ACL_ST_OUT_CON2      0x06
#define BTM_ACL_ST_IN_CON2       0x07 /*accept but not complete*/

#define BTM_CMGR_INIT_REQ       0x00
#define BTM_CMGR_DEINIT_REQ     0x01

/*  event param of BTEVENT_LINK_CONNECT_IND*/
struct acl_open_data{
    struct btm_conn_item_t* conn; //acl connection
    uint8 err_code;
    uint32 emask;
};
struct conn_req_data{
    struct btm_conn_item_t *conn;
    uint32   emask;
};
struct acl_close_data{
    struct btm_conn_item_t *conn;
    struct hci_ev_disconn_complete *param;
    uint8 status;
    uint8 reason;
    uint32 emask;
};

struct sco_open_data{
    struct btm_sco_conn_item_t* conn;
    uint8 err_code;
    uint8 device_id;
    uint8 sco_codec;
    uint8 interval;
    uint8 window;
    uint32 emask;
};

struct sco_close_data{
    struct btm_sco_conn_item_t *conn;
    struct hci_ev_disconn_complete *param;
    uint8 status;
    uint8 reason;
    uint8 device_id;
    uint32 emask;
};

struct accessible_change_data{
    enum device_mode_dis_enum discoverable;
    enum device_mode_conn_enum connectable;
    uint8 err_code;
    uint32 emask;
};

struct inquiry_cancel_data{
    uint8 err_code;
    uint32 emask;
};

struct command_status_data{
	uint8  status;
	uint8  num_hci_cmd_packets;
	uint16 cmd_opcode;
	struct bdaddr_t bdaddr;
    uint32 emask;
};

struct bt_ibrt_event{
    uint8  status;
    uint16 cmd_opcode;
    bt_bdaddr_t addr;
    uint32 emask;
};

struct role_change_data{
    struct btm_conn_item_t *rem_dev;
	uint8 new_role;
    uint8 err_code;
    uint32 emask;
};

struct mode_change_data{
    struct btm_conn_item_t *rem_dev;
	uint8 cur_mode;
    uint16 interval;
    uint8 err_code;
    uint32 emask;
};

struct acl_data_active{
    struct btm_conn_item_t *rem_dev;
    uint16 data_len;
    uint8 err_code;
    uint32 emask;
};

struct acl_data_not_active{
    struct btm_conn_item_t *rem_dev;
    uint16 data_len;
    uint8 err_code;
    uint32 emask;
};

struct max_slot_change_data {
    struct btm_conn_item_t *rem_dev;
    uint32 emask;
    uint8 err_code;
    uint16 conn_handle;
    uint8 max_slot;
};

struct packet_type_changed_data {
    struct btm_conn_item_t *rem_dev;
    uint32 emask;
    uint8 err_code;
    uint16 conn_handle;
    uint16 packet_type;
};

struct authenticaion_complete_data{
    struct btm_conn_item_t *rem_dev;
    uint32 emask;
    uint8 err_code;
};

struct simple_pairing_complete_data{
    struct btm_conn_item_t *rem_dev;
    uint32 emask;
    uint8 err_code;
};

struct encryption_change_data{
    struct btm_conn_item_t *rem_dev;
	uint8 mode;
    uint32 emask;
    uint8 err_code;
};

struct inquiry_result_data{
    uint8 *data;
    uint8 is_rssi;
    uint8 is_extinq;
    uint8 err_code;
    uint32 emask;
};

struct name_rsp_data{
    uint8    status;
    struct   bdaddr_t bdaddr;
    const char* name;
    uint32   emask;
};
struct inquiry_complete_data{
    uint8 status;
    uint8 err_code;
    uint32 emask;
};

struct user_passkey_notify_data{
    struct btm_conn_item_t *rem_dev;
    uint32 passkey;
    uint8 err_code;
    uint32 emask;
};

struct user_passkey_request_data{
    struct btm_conn_item_t *rem_dev;
    uint8 err_code;
    uint32 emask;
};

struct set_bt_ble_active_link_complete_data{
    uint8 err_code;
    uint32 emask;
};

struct btm_security_auth_pending_item_t {
    struct list_node list;
};

#ifndef BOOL_DEFINED
typedef unsigned int BOOL;      /* IGNORESTYLE */
#endif

struct btm_device_mode_t
{
    enum device_mode_dis_enum discoverable;
    enum device_mode_conn_enum connectable;
};

struct btm_inquiry_result_item_t
{
    struct list_node list;

    struct bdaddr_t remote;

    uint16 timeout_count;  /*=0: disable , count down to 1: timeout happen*/

    /* received in inquiry result */
    uint8 page_scan_repetition_mode;
    uint8 page_scan_period_mode;
    uint8 class_dev[3];
    uint16 clk_off;
};

typedef uint8  connection_role;

#define BCR_MASTER   0x00
#define BCR_SLAVE    0x01
#define BCR_ANY      0x02
#define BCR_UNKNOWN  0x03

#define BTM_MAX_xSCO_NUMS BT_ACL_MAX_LINK_NUMS

#define BTM_FEAT_3SLOT_PACKETS      0,0,0x01
#define BTM_FEAT_5SLOT_PACKETS      0,0,0x02
#define BTM_FEAT_ENCRYPTION         0,0,0x04
#define BTM_FEAT_SLOT_OFFSET        0,0,0x08
#define BTM_FEAT_TIMING_ACC         0,0,0x10
#define BTM_FEAT_ROLE_SWITCH        0,0,0x20
#define BTM_FEAT_HOLD_MODE          0,0,0x40
#define BTM_FEAT_SNIFF_MODE         0,0,0x80
#define BTM_FEAT_PWR_CTRL_REQ       0,1,0x02
#define BTM_FEAT_CQDDR              0,1,0x04
#define BTM_FEAT_SCO_LINK           0,1,0x08
#define BTM_FEAT_HV2_PACKETS        0,1,0x10
#define BTM_FEAT_HV3_PACKETS        0,1,0x20
#define BTM_FEAT_ULAW_SYNC_DATA     0,1,0x40
#define BTM_FEAT_ALAW_SYNC_DATA     0,1,0x80
#define BTM_FEAT_CVSD_SYNC_DATA     0,2,0x01
#define BTM_FEAT_PAGE_PARA_NEGO     0,2,0x02
#define BTM_FEAT_PWR_CTRL           0,2,0x04
#define BTM_FEAT_TRANS_SYNC_DATA    0,2,0x08
#define BTM_FEAT_FLOW_CTRL_LST_BIT  0,2,0x10
#define BTM_FEAT_FLOW_CTRL_MID_BIT  0,2,0x20
#define BTM_FEAT_FLOW_CTRL_MST_BIT  0,2,0x40
#define BTM_FEAT_BROADCAST_ENCRYT   0,2,0x80
#define BTM_FEAT_EDR_2M_MODE        0,3,0x02
#define BTM_FEAT_EDR_3M_MODE        0,3,0x04
#define BTM_FEAT_ENHANCED_ISCAN     0,3,0x08
#define BTM_FEAT_INTERLACED_ISCAN   0,3,0x10
#define BTM_FEAT_INTERLACED_PSCAN   0,3,0x20
#define BTM_FEAT_RSSI_WITH_INQRES   0,3,0x40
#define BTM_FEAT_ESCO_LINK          0,3,0x80
#define BTM_FEAT_EV4_PACKETS        0,4,0x01
#define BTM_FEAT_EV5_PACKETS        0,4,0x02
#define BTM_FEAT_AFH_CAPAB_SLAVE    0,4,0x08
#define BTM_FEAT_AFH_CLASS_SLAVE    0,4,0x10
#define BTM_FEAT_BREDR_NOT_SUPP     0,4,0x20
#define BTM_FEAT_LE_CTRL_SUPP       0,4,0x40
#define BTM_FEAT_3SLOT_EDR_ACL      0,4,0x80
#define BTM_FEAT_5SLOT_EDR_ACL      0,5,0x01
#define BTM_FEAT_SNIFF_SUBRATING    0,5,0x02
#define BTM_FEAT_PAUSE_ENCRYPT      0,5,0x04
#define BTM_FEAT_AFH_CAPAB_MASTER   0,5,0x08
#define BTM_FEAT_AFH_CLASS_MASTER   0,5,0x10
#define BTM_FEAT_EDR_ESCO_2M_MODE   0,5,0x20
#define BTM_FEAT_EDR_ESCO_3M_MODE   0,5,0x40
#define BTM_FEAT_3SLOT_EDR_ESCO     0,5,0x80
#define BTM_FEAT_EXTENDED_INQRES    0,6,0x01
#define BTM_FEAT_SIMU_LE_BREDR_CTRL 0,6,0x02
#define BTM_FEAT_SECURE_SIMPLE_PAIR 0,6,0x08
#define BTM_FEAT_ENCAPSULATED_PDU   0,6,0x10
#define BTM_FEAT_ERR_DATA_REPORT    0,6,0x20
#define BTM_FEAT_NONFLUSH_PBF       0,6,0x40
#define BTM_FEAT_LINKSUPTO_CHANGE   0,7,0x01
#define BTM_FEAT_INQ_TX_PWR_LEVEL   0,7,0x02
#define BTM_FEAT_ENHANCED_PWR_CTRL  0,7,0x04
#define BTM_FEAT_EXTENDED_FEATURES  0,7,0x80
#define BTM_FEAT_SSP_HOST_SUPP      1,0,0x01
#define BTM_FEAT_LE_HOST_SUPP       1,0,0x02
#define BTM_FEAT_SIMU_LE_BREDR_HOST 1,0,0x04
#define BTM_FEAT_SEC_CONN_HOST_SUPP 1,0,0x08
#define BTM_FEAT_SLAVE_BROAD_MSTOP  2,0,0x01
#define BTM_FEAT_SLAVE_BROAD_SLVOP  2,0,0x02
#define BTM_FEAT_SYNCHRON_TRAIN     2,0,0x04
#define BTM_FEAT_SYNCHRON_SCAN      2,0,0x08
#define BTM_FEAT_INQRES_NOTIFY      2,0,0x10
#define BTM_FEAT_GENERAL_INTERSCAN  2,0,0x20
#define BTM_FEAT_COARSE_CLOCK_ADJ   2,0,0x40
#define BTM_FEAT_SEC_CONN_CTRL_SUPP 2,1,0x01
#define BTM_FEAT_PING               2,1,0x02
#define BTM_FEAT_TRAIN_NUDGING      2,1,0x08
#define BTM_FEAT_SLOT_AVAIL_MASK    2,1,0x10

#define BTM_MAX_FEATURE_PAGE        (3)
#define BTM_MAX_REMOTE_NAME_LEN     (32)    /* The value must be the same as BTM_SHORT_NAME_MAX_LEN */

struct btm_feature_t
{
    uint8 max_page;
    uint8 feature[8];
};

#define BTM_AUTH_WAIT_CMPL 0x01
#define BTM_AUTH_WAIT_MSS  0x02

#define BTM_DEVICE_INVALID_ID 0xff
#define BT_TWS_DEVICE_ID 0x0f
#define BT_SOURCE_DEVICE_ID_BASE 0x10

struct btm_acl_open_report
{
    struct acl_open_data edata;
    uint8 event;
    bool report;
};

struct btm_dip_info
{
    uint16 spec_id;
    uint16 vend_id;
    uint16 prod_id;
    uint16 prod_ver;
    uint8  prim_rec;
    uint16 vend_id_source;
};

struct btm_conn_item_t
{
    struct list_node list;
    struct list_node sco_conn_list;

    struct bdaddr_t remote;
    void *cmgr_handler;
    uint16 conn_handle;
    bool marked_as_source;
    bool dip_is_in_query;
    bool dip_is_queried;
    bool is_tws_link;
    uint8 sco_req_type;
    bool conn_perferred_as_slave;
    bool conn_perferred_as_master;
    bool ibrt_both_side_save_credit_before_danger;
    bool ibrt_master_wait_remote_new_master_ready;
    struct btm_dip_info dip_info;

    uint8 used;
    /*1: positive connet to the remote or 0: negtive be connected*/
    uint8 positive;

    /* received in inquiry result */
    uint8 page_scan_repetition_mode;
    uint16 clk_off;

    uint8 discReason_saved;
    uint8 discReason;
    uint8 lowpower_flag;  /*if is lowpower. decided to buffer tx data*/
    uint8 lowpower_count; /*to count when to enter lowpower*/
    uint8 sniff_count;  /*to count how many sniff req sent*/
    bool ext_sniff_in_process;
    bool start_sniff_pending;
    uint16 sniff_interval;
    uint8 role_switch_pending;
    uint8 authen_enable_flag;
    uint8 authen_pending;
    uint8 auth_req_send_count;
    uint8 encry_enable_flag;/*tell if the entryption is enabled in this acl conn*/
    uint8 encry_need_flag;/*tell if the entryption is need*/
    uint8 encry_pending;
    connection_role btm_bt_role;
    uint8 state;
    uint8 mode;
    uint8 authState;
    bool iocap_rsp_received;
    uint8 remote_io_cap;
    uint8 remote_oob_present;
    uint8 remote_authen_requirement;
    uint8 acl_array_idx;
    uint8 device_id;
    uint16 rx_complete_count; /* Host HCI RX packets complete number, controller to host flow control */
    uint8 remote_version[5];
    struct btm_feature_t remote_feature[BTM_MAX_FEATURE_PAGE];
    uint8 *btm_acl_inplace_buff;
    struct btm_acl_open_report acl_open_report;
    uint8 cod[3];
    uint32 passkey;  //passkey notify and save notified passkey
    uint8 passkey_input_timer; //passkey request and waiting for user input

    bool isSimplePairingCompleted;

    uint8_t l2cap_wait_enc_timer;
    uint8_t enc_key_size;

    btm_tx_data_delay_monitor_t tx_data_delay_monitor;

#if mHDT_SUPPORT
    //move to le ACL link later
    uint32  le_mhdt_remote_feature;//bit0(BLE4M),bit8(BLE Audio ABR),get from TC hci command

    uint8  bt_mhdt_remote_feature;//get from TC hci command
    uint8  bt_mhdt_remote_host_support;//get from l2cap config res/req
    mhdt_state_e  mhdt_state; //0,disabled;1,entering;2,enabled;3,existing
    struct bt_mhdt_data_rate mhdt_data_rate;
    uint16 mhdt_timeout;
#endif

    char remote_name[BTM_MAX_REMOTE_NAME_LEN];
};

struct remote_version_t
{
    uint8    lmp_version;
    uint16   manufacturer_name;
    uint16   lmp_subversion;
} __attribute__ ((packed));

#define IS_REMOTE_FEAT_SUPPORT(conn, FEAT_MASK) \
    btm_is_remote_feature_support(conn, FEAT_MASK)

struct btm_sco_conn_item_t
{
    struct list_node list;
    struct btm_conn_item_t *conn;  /*acl connection*/
    uint16 conn_handle;
    uint8 link_type;  /* HCI_LINK_TYPE_ESCO or HCI_LINK_TYPE_SCO */
    enum conn_sco_stat_enum status;
    connection_role role;
    uint8 index;
    uint8 used;
};

enum btm_stack_state {
    BTM_STACK_Initializing = 0,
    BTM_STACK_Ready = 1,
};

enum btm_stack_init_sub_state {
    BTM_INIT_ST_RESET = 0,
    BTM_INIT_ST_SET_VOICE_SETTTING,
    BTM_INIT_ST_READ_BUFFER_SIZE,
    BTM_INIT_ST_LE_READ_BUFFER_SIZE,
    BTM_INIT_ST_LE_READ_BUFFER_SIZE_V2,
    BTM_INIT_ST_HOST_BUFFER_SIZE,
    BTM_INIT_ST_SET_HCITOHOST_FLOW_CONTROL,
    BTM_INIT_ST_WRITE_PAGE_TIEMOUT,
    BTM_INIT_ST_READ_PAGE_TIMEOUT,
    BTM_INIT_ST_SET_BLE_ADDRESS,
    BTM_INIT_ST_SET_BD_ADDRESS,
    BTM_INIT_ST_SET_EVENT_MASK,
    BTM_INIT_ST_SET_BLE_EVENT_MASK,
    BTM_INIT_ST_READ_LOCAL_VER_INFO,
    BTM_INIT_ST_READ_LOCAL_SUP_COMMANDS,
    BTM_INIT_ST_READ_LOCAL_FEATURES,
    BTM_INIT_ST_READ_LOCAL_EXT_FEATURES,
    BTM_INIT_ST_READ_LOCAL_EXT_FEATURES_1,
    BTM_INIT_ST_READ_LOCAL_EXT_FEATURES_2,
    BTM_INIT_ST_READ_BD_ADDRESS,
    BTM_INIT_ST_READ_INQUIRY_MODE,
    BTM_INIT_ST_READ_DEF_ERR_DATA_REPORTING,
    BTM_INIT_ST_WRITE_SAMPLE_PAIRING_MODE,
    BTM_INIT_ST_WRITE_CLASS_OF_DEVICE,
    BTM_INIT_ST_WRITE_LOCAL_NAME,
    BTM_INIT_ST_WRITE_SYNC_CONFIG,
    BTM_INIT_ST_WRITE_DEF_ERR_DATA_REPORTING,
    BTM_INIT_ST_WRITE_DEFAULT_LP_SETTINGS,
    BTM_INIT_ST_WRITE_PAGESCAN_ACTIVITY,
    BTM_INIT_ST_WRITE_INQUIRYSCAN_ACTIVITY,
    BTM_INIT_ST_WRITE_INQUIRYSCAN_TYPE,
    BTM_INIT_ST_WRITE_PAGESCAN_TYPE,
    BTM_INIT_ST_WRITE_SEC_CON_HOST_SUPP,
    BTM_INIT_ST_WRITE_SCAN_EN,
    BTM_INIT_ST_WRITE_LE_HOST_SUPPORT,
#if mHDT_SUPPORT
    BTM_INIT_ST_READ_BT_LOCAL_MHDT_FEATURE,
    BTM_INIT_ST_READ_LE_LOCAL_MHDT_FEATURE,
#endif

    BTM_INIT_ST_NUM,
};

typedef void (*btm_init_start_callback_t)(void);

struct btm_sync_conn_param {
    uint16 max_latency;
    uint16 packet_type;
    uint16 voice_setting;
    uint8  retrans_effort;
    uint32 receive_bandwidth;
    uint32 transmit_bandwidth;
};

struct btm_ctrl_t {
    enum btm_stack_state stack_state;
    enum btm_stack_init_sub_state init_sub_state;
    uint16 init_sub_state_opcode;
    uint8 pairing_flag;  /*tell whether the device in pairing state, 1:yes, 0:no*/
    uint32 pairing_timeout;
    void (*btm_pairing_notify_callback)(enum btm_pairing_event event,void *pdata);
    void (*btm_confirmation_req_callback)(struct bdaddr_t *bdaddr, uint32 numeric_value);
    uint8 security_waitfor_linkkey_reply;
    struct bdaddr_t security_waitfor_linkkey_reply_bdaddr;

    // local
    uint8 bt_version;
    uint8 bt_features[8];
    uint8 bt_ext_features[2][8];

    // esco
    uint8 sco_param_select;
    uint8 sco_default_param_select;
    struct btm_sync_conn_param sco_custom_param;

    // security
    uint8 security_bonding_mode;
    uint8 security_auth_requirements;
    uint8 security_io_capability;
    uint8 security_oob_present;
    uint8 pin_code[16];
    uint8 pin_len;
    uint8 host_ssp_mode;

    void (*btm_event_report)(uint16 evt_id, void* pdata);
    void (*btm_user_event_report)(const void* pdata);
    void (*btm_cmgr_event_report)(uint16 evt_id, void* conn);
    void (*btm_debug_event_report)(uint8_t* p_buf, uint16_t buf_len);
    void (*btm_get_local_address_cb)(void *remote);
    bool (*btm_is_creating_source_link_cb)(const void *remote);
    void (*btm_ibrt_snoop_acl_connected_report)(uint8_t device_id, void* remote, void* btm_conn);
    void (*btm_ibrt_snoop_acl_disconnected_report)(uint8_t device_id, void* remote);
    void (*btm_notify_save_creadit_cb)(const bt_bdaddr_t *remote);
    void (*btm_mesh_list_func)(uint16_t opcode, uint8_t status, uint8_t size);
    void (*btm_read_avg_rssi_cb)(uint8_t status, uint16_t conhdl, int8_t rssi);
    bool conn_req_cb_enable;
    uint8 con_num;

    uint16 init_sub_state_opcode_normal_test_switch;
    uint8_t testmode_enable;

    uint8_t allow_resp_trigger_auth;
    uint8_t min_enc_key_size;

#if mHDT_SUPPORT
    void    (*btm_mhdt_mode_change_cb)(struct bdaddr_t remote, bool isIn_mhdt_mode);
    uint8_t bt_mhdt_host_support;//local bt host mhdt support,used for enable/disable bt mHDT feature
    uint8_t bt_mhdt_feature;//bit0-bit3,local bt controller mhdt feature

    uint8_t le_mhdt_host_support;//local le host mhdt support,used for enable/disable le mHDT feature
    uint32_t le_mhdt_feature;//bit0(BLE4M),bit8(BLE Audio ABR),local le controller mhdt feature
#endif
};

extern struct btm_ctrl_t btm_ctrl;
struct btm_conn_env_t {
    struct btm_conn_item_t *conn_item;
};

#ifdef BQB_PROFILE_TEST
struct btm_pts_ctrl_t{
    uint8_t pts_mode;
    uint8_t pts_accept;
    uint8_t pts_reject_code;
};
extern struct btm_pts_ctrl_t btm_pts_ctrl;
#endif

void btm_event_handle(struct hci_evt_packet_t *pkt);
void btm_acl_handle(uint16_t conn_handle, struct pp_buff *ppb);
void btm_acl_tx_done(uint16_t connhdl, struct pp_buff *ppb);
void btm_debug_trace_handle(struct hci_evt_packet_t *pkt);

typedef bool (*ibrt_io_capbility_callback)(void *bdaddr);

void btm_security_link_key_notify(struct bdaddr_t *remote, uint8 *linkkey, uint8 key_type);
void btm_security_pin_code_req(struct bdaddr_t *remote);
void btm_security_link_key_req(struct bdaddr_t *remote);
bool btm_security_authen_complete(uint8 status, uint16 conn_handle);
void btm_security_encryption_change(uint8 status, uint16 conn_handle, uint8 encrypt_en);
void btm_security_user_passkey_req(struct bdaddr_t *remote);
void btm_security_user_passkey_notify(struct bdaddr_t *remote, uint32 passkey);
void btm_security_user_passkey_input_timer_release(struct bdaddr_t *remote);
void btm_security_user_passkey_input_timeout_ms(uint32_t timeout_ms);

void btm_process_conn_complete_evt(struct hci_evt_packet_t *pkt);
int8 btm_create_acl_connection_fail_process(struct btm_conn_item_t *conn, uint8 status, struct bdaddr_t *bdaddr);

//void btm_process_conn_req_evt(struct hci_evt_packet_t *pkt);
void btm_process_pin_code_req_evt(struct hci_evt_packet_t *pkt);
void btm_process_link_key_req_evt(struct hci_evt_packet_t *pkt);
void btm_process_link_key_notify_evt(struct hci_evt_packet_t *pkt);
void btm_process_authentication_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_simple_pairing_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_encryption_change_evt(struct hci_evt_packet_t *pkt);
void btm_process_user_passkey_req_evt(struct hci_evt_packet_t *pkt);
void btm_process_user_passkey_notify_evt(struct hci_evt_packet_t *pkt);
void btm_process_remote_name_req_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_inquiry_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_inquiry_result_evt(struct hci_evt_packet_t *pkt, uint8 rssi, uint8 extinq);
void btm_process_mode_change_evt(struct hci_evt_packet_t *pkt);
void btm_process_acl_data_active_evt(void *conn, uint16_t len);
void btm_process_acl_data_not_active_evt(void *conn, uint16_t len);
void btm_process_read_remote_version_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_read_remote_supported_feature_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_read_remote_extended_feature_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_cmd_complete_inquiry_cancel (uint8 *data);
void btm_process_cmd_complete_remote_name_cancel(uint8 *data);
void btm_process_cmd_complete_evt(struct hci_evt_packet_t *pkt);
void btm_process_return_linkkeys_evt (struct hci_evt_packet_t *pkt);
void btm_process_vendor_evt(uint8_t* pbuf, uint32_t length);
void btm_acl_handle_nocopy(uint16 conn_handle, uint16 data_len, uint8 *data_p);

struct btm_ctrl_t *btm_get_ctrl_ptr(void);
struct list_node *btm_get_conn_list_ptr(void);

struct l2cap_channel;

int8 btm_init(void);

void btm_start_ble_stack_init(void);

int8 btm_device_mode_set(enum device_mode_dis_enum discoverable, enum device_mode_conn_enum connectable);
int8 btm_device_mode_recover(void);
int8 btm_device_write_iac(uint8 num);
int8 btm_device_write_page_scan_activity(uint16 interval, uint16 window);
int8 btm_device_write_inquiry_scan_activity(uint16 interval, uint16 window);

uint16 btm_conn_acl_get_conn_handle ( struct bdaddr_t *remote_bdaddr);

uint32_t btm_get_class_of_device(void);

int8 btm_conn_acl_req ( struct bdaddr_t *remote_bdaddr);
bool btm_conn_acl_is_open( struct bdaddr_t *remote_bdaddr);

int8 btm_conn_acl_senddata (struct bdaddr_t *remote_bdaddr, uint8 *data_buf_p, uint16 data_buf_len);

int8 btm_conn_acl_send_ppb (uint16 conn_handle, struct pp_buff *ppb);

int8 btm_conn_acl_send_continue_ppb (uint16 conn_handle, struct pp_buff *ppb);

int8 btm_conn_acl_close(struct bdaddr_t *bdaddr, uint8 reason);

void btm_conn_acl_fake_connection_complete(uint8_t status, struct bdaddr_t *bdAddr);

int8 btm_security_askfor_authority(uint8_t device_id, uint16 conn_handle, uint16 psm, struct l2cap_channel *l2cap_channel);

void btm_pairing_enter(uint32 pairing_timeout, btm_pairing_callback_t callback);

void btm_register_is_creating_source_link(bool (*cb)(const void* remote));

/* for pairing that needs remote device address */
void btm_start_pairing(	struct bdaddr_t remote_addr, btm_pairing_callback_t callback);

void btm_unpair_reomte(struct bdaddr_t remote_addr, btm_pairing_callback_t callback);

void btm_pairing_register_callback(btm_pairing_callback_t callback);
void btm_confirmation_register_callback(void (*cb)(struct bdaddr_t *bdaddr, uint32 numeric_value));
int8 btm_confirmation_resp(struct bdaddr_t *bdaddr, bool accept);

void btm_pairing_exit(void);

void btm_direct_set_audio_param(uint8 param);
uint8 btm_get_audio_default_param(void);
void btm_set_audio_default_param(uint8 param);

/* inquiry functions */
void btm_start_inquiry(void (*btm_start_inquiry_notify_callback)(enum btm_inquiry_event event, void *pData), uint8 inq_period, uint8 num_rsp);
void btm_cancel_inquiry(void (*btm_start_inquiry_notify_callback)(enum btm_inquiry_event event, void *pData));
void btm_remote_name_request(struct hci_cp_remote_name_request *req, void (*btm_name_notify_callback)(enum btm_name_event event,void *pData));
void btm_remote_name_cancel(struct bdaddr_t *remote, void (*btm_name_notify_callback)(enum btm_name_event event,void *pData));

void btm_hcicmd_sniff_mode(struct bdaddr_t *remote_bdaddr, uint16 sniff_max_interval,uint16 sniff_min_interval, uint16 sniff_attempt,uint16 sniff_timeout);
int8 btm_exit_sniff (struct bdaddr_t *remote_bdaddr);

void register_hci_read_avg_rssi_callback(void (*cb)(uint8_t status, uint16_t conhdl, int8_t rssi));

int8 btm_acl_conn_connected(struct bdaddr_t *bdAddr, uint16 conn_handle);
int8 btm_acl_conn_disconnected(struct bdaddr_t *bdAddr, uint16 conn_handle, uint8_t status, uint8_t reason);

void btm_conn_reset_as_source_device_id(struct btm_conn_item_t *conn);
struct btm_conn_item_t *btm_conn_reset_this_device_to_source_device_id(uint8_t device_id);

bool btm_conn_accept_extra_acl_req_check(struct bdaddr_t *remote, bool connected);

const char *btm_state2str(uint8 state);
const char *btm_errorcode2str(uint32 errorcode);
const char *btm_cmd_opcode2str(uint16 opcode);

struct btstack_chip_config_t
{
    uint16_t hci_dbg_set_sync_config_cmd_opcode;
    uint16_t hci_dbg_set_sco_switch_cmd_opcode;
};

struct btstack_chip_config_t* btm_get_btstack_chip_config(void);
void btm_set_btstack_chip_config(void* config);
int8 btm_device_send_prefer_rate(uint16 connhdl, uint8 rate);
void btm_security_user_passkey_input(struct bdaddr_t *remote,uint32 passkey);
void btm_security_set_host_pin_code(uint8 *pin, uint8 pinlen);
void btm_security_write_host_ssp_mode(uint8 enable);
uint8_t btm_conn_get_mobile_link_num(void);
void btm_conn_set_device_id(struct btm_conn_item_t *curr_conn, bool is_snoop_link);
void btm_get_local_device(struct bdaddr_t *remote);
void btm_register_get_local_device_callback(void (*cb)(void *remote));
void *btm_get_con_cmgr_handler(void);
void btm_chip_init_ready_callback(int status);

void btif_report_bt_event(const bt_bdaddr_t *bd_addr, BT_EVENT_T event, void *param);

struct btm_inquiry_result_item_t *btm_inquiry_result_search ( struct bdaddr_t *bdaddr );
struct btm_inquiry_result_item_t *btm_inquiry_result_find_or_add ( struct bdaddr_t *bdaddr );
struct btm_conn_item_t *btm_conn_add_new ( struct bdaddr_t *bdaddr );
struct btm_conn_item_t *btm_conn_search ( struct bdaddr_t *bdaddr );
struct btm_conn_item_t *btm_conn_search_by_device_id(uint8 device_id);
struct btm_conn_item_t *btm_conn_search_linkup ( struct bdaddr_t *bdaddr );
struct btm_sco_conn_item_t *btm_conn_sco_find_or_add( struct btm_conn_item_t *conn);
struct btm_conn_item_t *btm_conn_acl_search_by_handle( uint16 conn_handle);
int btm_get_bt_role_by_device_id(uint8 device_id);
void btm_set_conn_preferred_as_slave_role(uint8 device_id);
void btm_set_conn_preferred_as_master_role(uint8 device_id);
bool btm_current_bt_role_is_master(uint8 device_id);
bool btm_current_bt_role_is_slave(uint8 device_id);
bool btm_is_conn_preferred_as_slave(uint8 device_id);
bool btm_is_conn_preferred_as_master(uint8 device_id);
bool btm_is_remote_feature_support(struct btm_conn_item_t* conn, uint8 page, uint8 i, uint8 mask);
void btm_conn_disconnect_process(uint16 handle, uint8 status, uint8 reason);
int8 btlib_hcicmd_read_remote_name(struct bdaddr_t *bdaddr, uint8 page_scan_repetition_mode, uint16 clk_off);
int8 btm_conn_acl_send_ppb_done(uint8 device_id, uint16 conn_handle, struct pp_buff *ppb);
void btm_register_event_report( void (*evt_cb)(uint16 evt_id, void *pdata));
void btm_register_user_event_report( void (*evt_cb)(const void *pdata));
void btm_register_cmgr_event_report(void (*evt_cb)(uint16 evt_id, void* conn));
void btm_register_check_hfp_codec_enable_callback(bool (*cb)(uint8_t codec_type, uint8_t device_id, struct bdaddr_t *bdaddr));
extern uint8 gapc_get_conidx(uint16 conhdl);
extern void gapc_inc_rx_packet_count(uint8 idx,uint8 inc_count);
extern uint8 gapc_get_rx_packet_counnt(uint8 idx);
extern uint8 gapc_get_conn_handle(uint8 idx);
extern void gapc_rx_packet_count_reset(uint8 idx);
bool btm_conn_need_authentication(struct btm_conn_item_t *conn);
uint8 btm_conn_allocate_dev_idx(void);
void btm_conn_free_dev_idx(uint8 idx);
void btm_conn_set_item_by_idx(struct btm_conn_item_t *p_conn_item, uint8 dev_idx);
struct bdaddr_t *btm_get_address_from_rem_dev(struct btm_conn_item_t *rem_dev);
struct btm_conn_item_t * btm_conn_get_item_by_idx(uint8 dev_idx);
uint8 btm_get_bt_version(void);
uint8 btm_get_bt_features(uint8 index);
void btm_conn_delete_free(struct btm_conn_item_t *conn);
bool btm_acl_role_switch_pending(uint16_t conn_handle);
uint8_t btm_conn_get_device_id(struct bdaddr_t* remote);
void btm_chip_init_noraml_test_mode_switch(void);
bool btm_is_stack_ready(void);
bt_status_t hci_send_cmd_packet_immediately(struct pp_buff *ppb);
int8 btlib_hcicmd_acl_connect(struct bdaddr_t *bdaddr,
                              uint16 pkt_type,
                              uint8 page_scan_repetition_mode,
                              uint16 clk_off,
                              uint8 allow_role_switch);
int8 btlib_hcicmd_addsyc_conn(struct btm_conn_item_t *conn);
int8 btlib_hcicmd_addsco_conn(struct btm_conn_item_t *conn, uint16 pkt_type);
int8 btlib_hcicmd_write_scan_enable(int8 scan_enable);
int8 btlib_hcicmd_write_automatic_flush_timeout(uint16 connhandle, uint16 flush_timeout);
int8 btlib_hcicmd_write_current_iac_lap(uint8 num);
int8 btlib_hcicmd_reject_conn_req(struct bdaddr_t *bdaddr, uint8 reason);
int8 btlib_hcicmd_accept_conn_req(struct bdaddr_t *bdaddr, uint8 role);
int8 btlib_hcicmd_switch_role(struct bdaddr_t *bdaddr, uint8 role);
int8 btlib_hcicmd_write_page_timeout(uint16_t timeout);
int8 btlib_hcicmd_write_wait_connection_accept_timeout(uint16_t timeout);
int8 btlib_hcicmd_write_superv_timeout(uint16 connhandle, uint16 superv_timeout);
int8 btlib_hcicmd_create_connection_cancel(const bt_bdaddr_t *bdaddr);
int8 btlib_hcicmd_set_extended_inquiry_response(uint8 fec, uint8 *buff, uint32 len);
int8 btlib_hcicmd_enable_fastack(uint16_t conhdl, uint8_t direction, uint8_t enable);
int8 btlib_hcicmd_set_sniffer_env(uint8 sniffer_active, uint8 sniffer_role, struct bdaddr_t *monitor_bdaddr, struct bdaddr_t *sniffer_bdaddr);

int8 btlib_hcicmd_qos_setup(uint16 conn_handle);
int8 btlib_hcicmd_qos_setup_with_tpoll(uint16 conn_handle, uint32_t tpoll_slot, uint8_t service_type);
int8 btlib_hcicmd_ble_write_random_addr(struct bdaddr_t *bdaddr);
int8 btlib_hcicmd_ble_write_adv_param(struct hci_write_adv_param *para);
int8 btlib_hcicmd_ble_write_adv_data(U8 len, U8 *data);
int8 btlib_hcicmd_ble_write_scan_rsp_data(U8 len, U8 *data);
int8 btlib_hcicmd_ble_write_adv_en(uint8 en);
int8 btlib_hcicmd_ble_write_scan_param(struct hci_write_ble_scan_param *para);
int8 btlib_hcicmd_ble_write_scan_en(uint8 scan_en, uint8 filter_duplicate);
int8 btlib_hcicmd_ble_clear_wl(void);
int8 btlib_hcicmd_ble_add_dev_to_wl(uint8 addr_type, struct bdaddr_t *bdaddr);

int8 btlib_hcicmd_pincode_reply(struct bdaddr_t *bdaddr, uint8 *pin, int8 pinlen);
int8 btlib_hcicmd_pincode_neg_reply(struct bdaddr_t *bdaddr);
int8 btlib_hcicmd_linkkey_reply(struct bdaddr_t *bdaddr, uint8 *linkkey);
int8 btlib_hcicmd_linkkey_neg_reply(struct bdaddr_t *bdaddr);

int8 btlib_hcicmd_authentication_req (uint16 conn_handle);
int8 btlib_hcicmd_write_auth_enable(uint8 flag);
int8 btlib_hcicmd_set_conn_encryption (uint16 conn_handle, uint8 encry_enable);
int8 btlib_hcicmd_disconnect (uint16 conn_handle, uint8 reason);
int8 btlib_hcicmd_fault_free_link(uint16 conn_handle, uint8 reason);
int8 btlib_hcicmd_write_class_of_device(uint8 *cod);
int8 btlib_hcicmd_write_localname (uint8 *local_name);
int8 btlib_hcicmd_set_bdaddr (uint8 *address);
int8 btlib_hcicmd_set_ble_bdaddr (const uint8 *address);
int8 btlib_hcicmd_write_memory(uint32 addr, uint32 value, uint8 bytelen);

int8 btlib_hcicmd_sniff_mode(uint16 conn_handle,
                            uint16 sniff_max_interval,
                            uint16 sniff_min_interval,
                            uint16 sniff_attempt,
                            uint16 sniff_timeout);

int8 btlib_hcicmd_exit_sniff_mode(uint16 conn_handle);
int8 btlib_hcicmd_bt_role_discovery(uint16 conn_handle);
int8 btlib_hcicmd_read_remote_version_info(uint16 conn_handle);
int8 btlib_hcicmd_read_remote_supported_feat(uint16 conn_handle);
int8 btlib_hcicmd_read_remote_extended_feat(uint16 conn_handle, uint8 page_n);
int8 btlib_hcicmd_write_link_policy(uint16 conn_handle, uint16 link_policy_settings);
int8 btlib_hcicmd_lowlayer_monitor(uint16 conn_handle, uint8 control_flag, uint8 report_format, uint32 data_format, uint8 report_unit);
int8 btlib_hcicmd_read_stored_linkkey(struct bdaddr_t *bdaddr, uint8 read_all_flag);
int8 btlib_hcicmd_write_stored_linkkey(struct bdaddr_t *bdaddr, uint8 *linkkey);
int8 btlib_hcicmd_delete_stored_linkkey(struct bdaddr_t *bdaddr, uint8 delete_all_flag);

int8 btlib_hcicmd_inquiry(uint32 lap, uint8 inq_period, uint8 num_rs);
int8 btlib_hcicmd_inquiry_cancel(void);
int8 btlib_hcicmd_change_packet_type(uint16 conn_handle,uint16 packet_type);
int8 btlib_hcicmd_remote_name_request(struct hci_cp_remote_name_request *req);
int8 btlib_hcicmd_remote_name_cancel(struct bdaddr_t *bdaddr);
int8 btlib_hcicmd_write_pagescan_type (const uint8 pagescan_type);
int8 btlib_hcicmd_write_inqscan_type (const uint8 inqscan_type);
int8 btlib_hcicmd_write_sleep_enable(uint8 sleep_en);
int8 btlib_hcicmd_write_inquiry_mode(uint8 mode);
int8 btlib_hcicmd_dbg_sniffer_interface (const uint8 subcode, const uint16 connhandle);

int8 btlib_hcicmd_set_clear_all_filters(void);
int8 btlib_hcicmd_set_inquiry_no_filter(void);
int8 btlib_hcicmd_set_inquiry_cod_filter(uint8 *class_of_device, uint8 *class_of_device_mask);
int8 btlib_hcicmd_set_inquiry_address_filter(uint8 *bt_bdaddr);
int8 btlib_hcicmd_set_connect_no_filter(uint8 auto_accept_flag);
int8 btlib_hcicmd_set_connect_cod_filter(uint8 *class_of_device, uint8 *class_of_device_mask, uint8 auto_accept_flag);
int8 btlib_hcicmd_set_connect_address_filter(uint8 *bt_bdaddr, uint8 auto_accept_flag);

int8 btlib_hcicmd_dbg_set_txpwr_link_thd(uint8 index, uint8 enable,uint8 link_id,
    uint16 rssi_avg_nb_pkt, int8 rssi_high_thd, int8 rssi_low_thd, int8 rssi_below_low_thd, int8 rssi_interf_thd);
int8 btlib_hcicmd_dbg_send_prefer_rate(uint16 conn_handle, uint8 rate);
int8 btlib_hcicmd_write_controller_memory(uint32_t addr, uint32_t val,uint8_t type);
int8 btlib_hcicmd_read_controller_memory(uint32_t addr, uint32_t len,uint8_t type);
int8 btlib_hcicmd_set_local_clock(uint32_t clock);
int8 btlib_hcicmd_set_afh_chnl_classification(uint8 *chnl_map);
int btlib_hcicmd_ble_set_host_chnl_classification(uint8_t *chnl_map);
int btlib_hcicmd_ble_read_chnl_map(uint16_t conn_handle);
int8 btlib_hcicmd_read_avg_rssi(uint16 conn_handle);
int8 btlib_hcicmd_user_passkey_request_reply(struct bdaddr_t *bdaddr, uint32_t passkey);
int8 btlib_hcicmd_user_passkey_request_neg_reply(struct bdaddr_t *bdaddr);
int8 btlib_hcicmd_write_simple_pairing_mode(uint8 enable);
int8 btlib_hcicmd_dbg_le_tx_power_request(uint16 conn_handle,  uint8 enable, int8 delta, uint8 rx_rate);
int8 btlib_hcicmd_ble_audio_dbg_trc_enable_cmd(uint32_t trc_enable_opcode);
int8 btlib_hcicmd_dbg_set_bt_ble_active_link(uint8_t traffic_mode, uint16_t link_handle);
void register_hci_debug_trace_callback(void (*cb)(uint8_t* p_buf, uint16_t buf_len));
#if BLE_AUDIO_ENABLED
int8 btlib_hcicmd_dbg_set_iso_quality_rep_thr(uint16 conn_handle,                   uint16 qlty_rep_evt_cnt_thr,
    uint16 tx_unack_pkts_thr, uint16 tx_flush_pkts_thr, uint16 tx_last_subevent_pkts_thr, uint16 retrans_pkts_thr,
    uint16 crc_err_pkts_thr, uint16 rx_unreceived_pkts_thr, uint16 duplicate_pkts_thr);
#endif

int8 btlib_hcicmd_set_local_class_of_device(uint32_t class_of_device);
int8 btlib_hcicmd_set_host_secure_connections_support(bool support);
int8 btlib_hcicmd_set_ble_host_support(bool support);

#ifdef __cplusplus
}
#endif
#endif /* __BTM_I_H__ */
