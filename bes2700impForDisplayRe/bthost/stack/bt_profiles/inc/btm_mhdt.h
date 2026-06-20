/***************************************************************************
 *
 * Copyright 2015-2020 BES.
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
#ifndef __BTM_MHDT__H__
#define __BTM_MHDT__H__
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

#define HCI_DBG_MHDT_EVENT                                          0xe9
#define TCI_SUBEVENT_LE_READ_REMOTE_PROPRIETARY_FEATURE_COMPLETE    0x04
#define TCI_SUBEVENT_LE_READ_LOCAL_PROPRIETARY_FEATURE_COMPLETE     0x05

/*
 *  mHDT_Core_v1.05 specification
 */

//Vendor HCI command
#define HCI_DBG_MHDT_COMMAND_OPCODE             0xfcdf
#define TCI_SUBCODE_READ_LOCAL_MHDT_FEATURE     0x01
#define TCI_SUBCODE_READ_REMOTE_MHDT_FEATURE    0x02
#define TCI_SUBCODE_MHDT_MODE                   0x03
#define TCI_SUBCODE_EXIT_MHDT_MODE              0x04
#define TCI_SUBCODE_WRITE_MHDT_TIMEOUT          0x05
#define TCI_SUBCODE_READ_MHDT_TIMEOUT           0x06
#define TCI_SUBCODE_LE_READ_LOCAL_PROPRIETARY_FEATURE    0x07
#define TCI_SUBCODE_LE_READ_REMOTE_PROPRIETARY_FEATURE   0x08

///Vendor HCI event(others are command complete event)
#define TCI_SUBEVENT_READ_REMOTE_MHDT_FEATURE_COMPLETE       0x01
#define TCI_SUBEVENT_MHDT_DATA_RATE_CHANGE                   0x02
#define TCI_SUBEVENT_MHDT_TIMEOUT_CHANGE                     0x03
#define TCI_SUBEVENT_READ_LOCAL_MHDT_FEATURE_COMPLETE        0x06


//bt data rate bits
#define  MHDT_BT_RATE_1MBPS_BIT       0x01
#define  MHDT_BT_RATE_2MBPS_BIT       0x02
#define  MHDT_BT_RATE_3MBPS_BIT       0x04
#define  MHDT_BT_RATE_4MBPS_BIT       0x08
#define  MHDT_BT_RATE_6MBPS_BIT       0x10
#define  MHDT_BT_RATE_8MBPS_BIT       0x20

//le proprietary bits
#define  MHDT_LE_RATE_4MBPS_BIT       0x0001
#define  MHDT_LE_AUDIO_ABR_BIT        0x0100

//local and peer remote mhdt support bits
#define L_BTC_MHDT_SPT    0X01
#define L_BTH_MHDT_SPT    0X02
#define P_BTC_MHDT_SPT    0X04
#define P_BTH_MHDT_SPT    0X08

//max mHDT payload
//4DH5 --> 1392 bytes
//6DH5 --> 2089 bytes
//8DH5 --> 2820 bytes

//mhdt state status
typedef enum mhdt_state_enum {
    MHDT_MODE_DISABLED = 0,
    MHDT_MODE_ENTERING,
    MHDT_MODE_ENABLED,
    MHDT_MODE_EXITING,
} mhdt_state_e;

struct bt_mhdt_data_rate
{
    uint8 tx_rates;
    uint8 rx_rates;
}__attribute__ ((packed));

//command/event paramtetes structure
struct hci_read_local_mhdt_feture_type
{
    uint8  subcode;
    uint8   param[4];
} __attribute__ ((packed));

//command complete parameters of TCI_SUBCODE_READ_LOCAL_MHDT_FEATURE
 struct hci_ev_cmd_complete_read_local_mhdt_feature
 {
     uint8 subcode;  //TCI_SUBCODE_READ_LOCAL_MHDT_FEATURE
     uint8 status;
     uint8 mhdt_feature; //bit0(EDR4),bit1(EDR6),bit2(EDR8),bit3(different mode in mHDT)
 }__attribute__ ((packed));

struct hci_read_remote_mhdt_feture_type
{
    uint8  subcode;
    uint16 conn_handle;
    uint8   param[2];
} __attribute__ ((packed));

//command status parameters of TCI_SUBCODE_READ_REMOTE_MHDT_FEATURE:
//status,numofcompleted,opcode,subcode

struct hci_ev_read_remote_mhdt_feature_complete
 {
     uint8 subevent;  //TCI_SUBEVENT_READ_REMOTE_MHDT_FEATURE_COMPLETE
     uint8 status;
     uint16 conn_handle;
     uint8 mhdt_feature; //bit0(EDR4),bit1(EDR6),bit2(EDR8),bit3(different mode in mHDT)
}__attribute__ ((packed));

struct hci_enter_mhdt_mode_type
{
    uint8  subcode;
    uint16 conn_handle;
    struct bt_mhdt_data_rate data_rate;
} __attribute__ ((packed));

//command status parameters of TCI_SUBCODE_MHDT_MODE:
//status,numofcompleted,opcode,subcode

struct hci_ev_mhdt_data_rate_change
 {
     uint8 subevent;  //TCI_SUBEVENT_MHDT_DATA_RATE_CHANGE
     uint8 status;
     uint16 conn_handle;
     uint8 mhdt_state; //0,disabled;1,enabled
     struct bt_mhdt_data_rate data_rate;
}__attribute__ ((packed));

struct hci_exit_mhdt_mode_type
{
    uint8  subcode;
    uint16 conn_handle;
} __attribute__ ((packed));

struct hci_exit_mhdt_mode_type2
{
    uint8  subcode;
    uint16 conn_handle;
    struct bt_mhdt_data_rate data_rate;
} __attribute__ ((packed));

//command status parameters of TCI_SUBCODE_EXIT_MHDT_MODE:
//status,numofcompleted,opcode,subcode

struct hci_write_mhdt_timeout_type
{
    uint8  subcode;
    uint16 conn_handle;
    uint16 mhdt_timeout;
} __attribute__ ((packed));


//master command complete parameters of TCI_SUBCODE_WRITE_MHDT_TIMEOUT
struct hci_ev_cmd_complete_write_mhdt_timeout
{
    uint8 subcode;  //TCI_SUBCODE_WRITE_MHDT_TIMEOUT
    uint8 status;
    uint16 conn_handle;
}__attribute__ ((packed));

//slave report to its host when mhdt timeout time changed
struct hci_ev_mhdt_timeout_change
 {
     uint8 subevent;  //TCI_SUBEVENT_MHDT_TIMEOUT_CHANGE
     uint16 conn_handle;
     uint16 mhdt_timeout;
}__attribute__ ((packed));

struct hci_read_mhdt_timeout_type
{
    uint8  subcode;
    uint16 conn_handle;
} __attribute__ ((packed));

//command complete parameters of TCI_SUBCODE_READ_MHDT_TIMEOUT
struct hci_ev_cmd_complete_read_mhdt_timeout
{
    uint8 subcode;  //TCI_SUBCODE_READ_MHDT_TIMEOUT
    uint8 status;
    uint16 conn_handle;
    uint16 mhdt_timeout;
}__attribute__ ((packed));

struct hci_le_read_local_proprietary_feture_type
{
    uint8  subcode;
} __attribute__ ((packed));

//command complete parameters of TCI_SUBCODE_LE_READ_LOCAL_PROPRIETARY_FEATURE
 struct hci_ev_cmd_complete_le_read_local_proprietary_feature
 {
     uint8 subcode;  //TCI_SUBCODE_LE_READ_LOCAL_PROPRIETARY_FEATURE
     uint8 status;
     uint32 le_mhdt_feature; //bit0(BLE4M),bit8(BLE Audio ABR)
 }__attribute__ ((packed));


struct hci_le_read_remote_proprietary_feture_type
{
    uint8  subcode;
    uint16 conn_handle;
} __attribute__ ((packed));

//command status parameters of TCI_SUBCODE_LE_READ_REMOTE_PROPRIETARY_FEATURE:
//status,numofcompleted,opcode,subcode

struct hci_ev_le_read_remote_proprietary_feature_complete
 {
     uint8 subevent;  //TCI_SUBEVENT_LE_READ_REMOTE_PROPRIETARY_FEATURE_COMPLETE
     uint8 status;
     uint16 conn_handle;
     uint32 le_mhdt_feature; //bit0(BLE4M),bit8(BLE Audio ABR)
}__attribute__ ((packed));

/*
 *  LE PHY parameter need add BLE4M in CMD/event:
 *     bit7 as BLE4M in bit form
 *     0x08 as BLE4M in value form
 *
 *  PDU max value change to 0x03FB in CMD/event
 */
//HCI_LE_BIGinfo_Adertising_Report
//HCI_LE_Create_Big
//HCI_LE_Create_BIG_Complete

//HCI_LE_Set_CIG_Parameters
//HCI_LE_CIS_Established

//HCI_LE_Set_PHY
//HCI_LE_PHY_Update_Complete
#if mHDT_SUPPORT
//hci commands functions declaration
int8 btlib_hcicmd_mhdt_read_local_feature(void);
int8 btlib_hcicmd_mhdt_read_remote_feature(uint16 conn_handle);
int8 btlib_hcicmd_mhdt_enter_mhdt_mode(uint16 conn_handle,struct bt_mhdt_data_rate data_rate);
int8 btlib_hcicmd_mhdt_exit_mhdt_mode(uint16 conn_handle);
int8 btlib_hcicmd_mhdt_exit_mhdt_mode_v2(uint16 conn_handle);
int8 btlib_hcicmd_mhdt_write_mhdt_timeout(uint16 conn_handle,uint16 mhdt_timeout);
int8 btlib_hcicmd_mhdt_read_mhdt_timeout(uint16 conn_handle);
int8 btlib_hcicmd_mhdt_le_read_local_feature(void);
int8 btlib_hcicmd_mhdt_le_read_remote_feature(uint16 conn_handle);
#endif /* mHDT_SUPPORT */
void btm_register_mhdt_mode_change_callback(void (*modechange_cb)(struct bdaddr_t remote, bool isIn_mhdt_mode));
void btm_set_bt_mhdt_host_remote_feature(uint16_t conn_handle,uint8_t remote_support);
bool btm_is_bt_mhdt_feature_support(uint16_t conn_handle,uint8_t check_masks);
uint8_t btm_get_bt_mhdt_state(uint16_t conn_handle);
int8 btm_get_bt_mhdt_data_rate(uint16_t conn_handle,uint8 *tx_rates,uint8 *rx_rates);
void btm_process_dbg_mhdt_event(struct hci_evt_packet_t *pkt);
void btm_process_cmd_complete_dbg_mhdt(struct hci_evt_packet_t *pkt);
#ifdef __cplusplus
}
#endif

#endif              /* __BTM_MHDT__H__ */