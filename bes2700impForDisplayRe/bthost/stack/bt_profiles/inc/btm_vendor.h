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
#ifndef __BTM_VENDOR__H__
#define __BTM_VENDOR__H__
#include "bluetooth.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************************
 *Macro definitions
******************************************************************************************/

/*
 * IBRT related HCI opcode macro definitions
 */
#define HCI_DBG_START_TWS_EXCHANGE_CMD_OPCODE                            0xFC91

#define HCI_DBG_BTADDR_EXCHANGE_CMD_OPCODE                               0xFC92

#define HCI_DBG_SET_LINK_LBRT_CMD_OPCODE                                 0xFC97

#define HCI_DBG_ENABLE_IBRT_MODE_CMD_OPCODE                              0xFCA2

#define HCI_DBG_START_IBRT_CMD_OPCODE                                    0xFCA3

#define HCI_DBG_GET_TWS_SLAVE_OF_MOBILE_RSSI_CMD_OPCODE                  0xFCA4

#define HCI_DBG_STOP_IBRT_CMD_OPCODE                                     0xFCA8

#define HCI_DBG_SET_FIX_TWS_INTERVAL_PARAM_CMD_OPCODE                    0xFCAB

#define HCI_DBG_RESUME_IBRT_CMD_OPCODE                                   0xFCAC

#define HCI_DBG_SET_ECC_DATA_TEST_CMD_OPCODE                             0xFCBB

#define HCI_DBG_IBRT_SWITCH_CMD_OPCODE                                   0xFCBE

#define HCI_DBG_STOP_MULTI_POINT_IBRT_CMD_OPCODE                         0xFCBF

#define HCI_LE_SET_TX_PWR_CMD_OPCODE                                  0xFC1A

#define HCI_DBG_IBRT_UPDATE_TIME_SLICE_CMD_OPCODE                        0xFD00

/*
 * Walkie talkie related HCI opcode macro definitions
 */

#define HCI_DBG_RD_MESH_LIST_SIZE_CMD_OPCODE                             0xFDE0

#define HCI_DBG_CLEAR_MESH_LIST_CMD_OPCODE                               0xFDE1

#define HCI_DBG_ADD_DEV_TO_MESH_LIST_CMD_OPCODE                          0xFDE2

#define HCI_DBG_RMV_DEV_FROM_MESH_LIST_CMD_OPCODE                        0xFDE3

/* vendor event */
#define HCI_EV_VENDOR_EVENT                                              (0xFF)

//vendor event sub code
#define HCI_DBG_TRACE_WARNING_EVT_CODE                                   0x01
#define HCI_ACL_SNIFFER_STATUS_EVT_CODE                                  0x03
#define HCI_TWS_EXCHANGE_CMP_EVT_CODE                                    0x04
#define HCI_NOTIFY_CURRENT_ADDR_EVT_CODE                                 0x05
#define HCI_NOTIFY_DATA_XFER_EVT_CODE                                    0x06
#define HCI_START_SWITCH_EVT_CODE                                        0x09
#define HCI_LL_MONITOR_EVT_CODE                                          0x0A
#define HCI_DBG_LMP_MESSAGE_RECORD_EVT_SUBCODE                           0x0B
#define HCI_GET_TWS_SLAVE_MOBILE_RSSI_CODE                               0x0C
#define HCI_DBG_IBRT_SWITCH_COMPLETE_EVT_SUBCODE                         0x0D
#define HCI_DBG_IBRT_CONNECTED_EVT_SUBCODE                               0x0E
#define HCI_DBG_IBRT_DISCONNECTED_EVT_SUBCODE                            0x0f


#define HCI_SCO_SNIFFER_STATUS_EVT_CODE                                  0x02
#define SNIFFER_SCO_STOP                                                 0
#define SNIFFER_SCO_START                                                1
#define SNIFFER_SCO_RESTART                                              2
#define SNIFFER_SCO_RESUME_AFTER_TO                                      3
#define SNIFFER_ACL_DISCONNECT_STOP_SCO                                  4

/******************************************************************************************
 *Data structure declaration
******************************************************************************************/

/*
 * IBRT hci cmd related data structures
 */
struct hci_ibrt_mode_op
{
    uint8 enable;
    uint8 switchOp;
} __attribute__ ((packed));

struct hci_ibrt_switch
{
    uint16 conn_handle;
} __attribute__ ((packed));

struct hci_start_ibrt
{
    uint16 slaveConnHandle;
    uint16 mobilConnHandle;
} __attribute__ ((packed));

struct hci_stop_ibrt
{
    uint8 enable;
    uint8 reason;
} __attribute__ ((packed));

struct hci_resume_ibrt
{
    uint8 enable;
} __attribute__ ((packed));

struct hci_stop_multi_point_ibrt
{
    uint16 mobile_conhdl;
    uint8 reason;
} __attribute__ ((packed));


struct hci_set_fix_tws_interval_param
{
    uint16_t duration;
    uint16_t interval;
}__attribute__ ((packed));

struct hci_start_tws_exchange
{
    uint16   tws_slave_conn_handle;
    uint16   mobile_conn_handle;
} __attribute__ ((packed));

struct hci_tws_bdaddr_exchange
{
    uint16 conn_handle;
} __attribute__ ((packed));

struct hci_get_tws_slave_of_mobile_rssi
{
    uint16 conn_handle;
} __attribute__ ((packed));

struct hci_set_link_lbrt_enable
{
    uint16 conn_handle;
    uint8  enable;
} __attribute__ ((packed));

struct link_env
{
    uint8_t link_id;
    uint32_t timeslice;
}__attribute__((packed));

struct hci_dbg_ibrt_update_time_slice_struct
{
    uint8_t update;
    uint8_t num_link_env;
    struct link_env link_env[0];
} __attribute__((packed));

struct hci_le_set_tx_power
{
    uint16_t connhdl;
    int8_t   tx_pwr;
} __attribute__ ((packed));

/*
 * IBRT hci evt related data structures
 */

struct hci_ev_ibrt_snoop_sco {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t sco_status;
    uint8_t airmode;
    uint32_t bandwidth;
} __attribute__ ((packed));


struct hci_ev_ibrt_snoop_conn {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t status;
    bt_bdaddr_t mobile_addr;
    uint8_t role;
} __attribute__ ((packed));


struct hci_ev_ibrt_snoop_disc {
    uint8_t subcode;
    uint16_t connhdl;
    uint8_t reason;
    bt_bdaddr_t mobile_addr;
    uint8_t role;
} __attribute__ ((packed));

struct btm_ctx_input {
    struct bdaddr_t *remote;
    struct ctx_content ctx;
    uint16 conn_handle;
};

struct btm_ctx_output {
    uint16 conn_handle;
    struct btm_conn_item_t *btm_conn;
};

/*
 * btm performance statistics process related data structure
 */

#define BTM_TX_DATA_RECORD_BUFFER_SIZE                    (10)
#define BTM_TX_DATA_INVALID_DELAY                         (0xFFFF)

typedef struct
{
    /* tx timing */
    struct {
        uint32_t tick_buf[HCI_NUM_BT_ACL_TX_CONTROLLER_BUFFERS];
        uint8_t head;
        uint8_t tail;
    } tx_record;

    struct {
        struct {
            uint32_t record_tick;
            uint32_t delay_value_ms;
        } list[BTM_TX_DATA_RECORD_BUFFER_SIZE];

        uint8_t head;
        uint8_t tail;
    } delay_record;
} btm_tx_data_delay_monitor_t;


/*
 * Walkie talkie related data structure
 */

struct hci_change_device_to_ml
{
    uint8 addr_type;
    struct bdaddr_t bdaddr;
} __attribute__ ((packed));


/******************************************************************************************
 *HCI cmd apis
******************************************************************************************/

/*
 * IBRT related commands API
 */
int8 btlib_hcicmd_ibrt_mode_init(uint8_t enable);

int8 btlib_hcicmd_ibrt_role_switch(uint8_t switch_op);

int8 btlib_hcicmd_start_ibrt(uint16 slaveConnHandle, uint16 mobileConnHandle);

int8 btlib_hcicmd_stop_ibrt(uint8_t enable,uint8_t reason);

int8 btlib_hcicmd_resume_ibrt(uint8_t enable);

int8 btlib_hcicmd_multi_point_ibrt_switch_cmd(uint16 mobile_conhdl);

int8 btlib_hcicmd_stop_multi_point_ibrt(uint16 mobile_conhdl,uint8_t reason);

int8 btlib_hcicmd_set_tws_fix_interval_param(uint16 duration, uint16 interval);

int8 btlib_hcicmd_set_ecc_ibrt_data_test(uint8  ecc_data_test_en, uint8 ecc_data_len, uint16 ecc_count, uint32 data_pattern);

int8 btlib_hcicmd_start_tws_exchange(uint16_t tws_slave_conn_handle, uint16_t mobile_conn_handle);

int8 btlib_hcicmd_tws_bdaddr_exchange(uint16 conn_handle);

int8 btlib_hcicmd_get_slave_mobile_rssi(uint16_t conn_handle);

int8 btlib_hcicmd_set_link_lbrt_enable(uint16 conn_handle, uint8 enable);

int8 btlib_hcicmd_dbg_ibrt_update_time_slice(uint8_t nb, struct link_env* multi_ibrt);

int8 btlib_hcicmd_set_le_tx_pwr(uint16_t connhdl, int8_t tx_pwr);

/*
 * Walkie talkie related commands API
 */
int8 btlib_hcicmd_read_mesh_list(void);

int8 btlib_hcicmd_clear_mesh_list(void);

int8 btlib_hcicmd_add_mesh_list(uint8 addr_type, struct bdaddr_t *bdaddr);

int8 btlib_hcicmd_remove_mesh_list(uint8 addr_type, struct bdaddr_t *bdaddr);

/******************************************************************************************
 *btm process apis
******************************************************************************************/

/*
 * IBRT related APIs
 */
void btm_register_notify_save_creadit_report(void (*cb)(const bt_bdaddr_t *remote));

void btm_conn_ibrt_disconnected_handle(struct btm_conn_item_t *btm_conn);

bool btm_ibrt_master_wait_remote_new_master_ready(const bt_bdaddr_t *remote);

bool btm_ibrt_master_tws_switch_start(const bt_bdaddr_t *remote);

bool btm_ibrt_slave_tws_switch_start(const bt_bdaddr_t *remote);

bool btm_ibrt_master_become_slave(const bt_bdaddr_t *remote);

bool btm_ibrt_slave_become_master(const bt_bdaddr_t *remote);

bool btm_ibrt_clear_side_save_credit_bit(const bt_bdaddr_t *remote);

void btm_ibrt_old_master_receive_ready_req(const bt_bdaddr_t *remote);

void btm_ibrt_new_master_receive_ready_rsp(const bt_bdaddr_t *remote);

void btm_register_snoop_acl_connection_callback(
        void (*conn)(uint8_t device_id, void* remote, void* btm_conn),
        void (*disc)(uint8_t device_id, void* remote));

void btm_ibrt_report_snoop_acl_disconnected(uint8_t device_id, bt_bdaddr_t* remote);

void btm_set_conn_tws_link(void *conn, uint8_t is_tws_link);

uint8_t btm_get_conn_tws_link(void *conn);

struct btm_conn_item_t *btm_ibrt_create_new_snoop_link(struct bdaddr_t *remote, uint16 conn_handle);

uint32 btm_save_ctx(struct bdaddr_t *remote, uint8_t *buf, uint32_t buf_len);

uint32 btm_restore_ctx(struct btm_ctx_input *input, struct btm_ctx_output *output);

/*
 * btm performance statistics process related APIs
 */
void btm_tx_data_start_time_record(btm_tx_data_delay_monitor_t *monitor);

void btm_tx_data_complete_time_ind(btm_tx_data_delay_monitor_t *monitor, int n);

/*
 * Walkie talkie related commands API
 */

void btm_register_mesh_list_report(void (*cb)(uint16_t opcode, uint8_t status, uint8_t size));

void btm_process_cmd_complete_mesh_list(uint16_t opcode, uint8_t* data);
#ifdef __cplusplus
}
#endif

#endif              /* __BTM_VENDOR__H__ */
