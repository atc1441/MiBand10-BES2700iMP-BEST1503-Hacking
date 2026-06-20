/***************************************************************************
 *
 * Copyright (c) 2015-2023 BES Technic
 *
 * Authored by BES CD team (Blueelf Prj).
 *
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
#ifndef __ACC_OTS__
#define __ACC_OTS__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_prf.h"

#include "ot_common.h"

/*DEFINE*/
#define OTS_CONNECTION_MAX      (GAF_CONNECTION_MAX)

/*ENUMERATION*/

/// List of OTS_CMD command codes
enum ots_cmd_codes
{
    OTS_COC_DISCONNECT = 0x0000,
    OTS_COC_SEND = 0x0001,
    OTS_COC_RELEASE = 0x0002,
};

/// List of OTS_REQ_IND request indication codes
enum ots_msg_req_ind_codes
{
    OTS_GET_NAME = 0x0000,
    OTS_SET_NAME = 0x0001,
    OTS_OBJECT_CREATE = 0x0002,
    OTS_OBJECT_EXECUTE = 0x0003,
    OTS_OBJECT_MANIPULATE = 0x0004,
    OTS_OBJECT_CONTROL = 0x0005,
    OTS_FILTER_GET = 0x0006,
    OTS_LIST_CONTROL = 0x0007,
    OTS_LIST_GOTO = 0x0008,
    OTS_FILTER_SET = 0x0009,
    OTS_FILTER_SET_TIME = 0x000A,
    OTS_FILTER_SET_SIZE = 0x000B,
    OTS_FILTER_SET_NAME = 0x000C,
    OTS_FILTER_SET_TYPE = 0x000D,
    OTS_COC_CONNECT = 0x000E,
};

/// Object Transfer Service Set type values
enum ots_set_type
{
    /// Set Current Size value
    OTS_SET_TYPE_CURRENT_SIZE = 0,
    /// Set Allocated Size value
    OTS_SET_TYPE_ALLOCATED_SIZE,
    /// Set number of objects
    OTS_SET_TYPE_NUM_OBJECTS,
    /// Set Properties value
    OTS_SET_TYPE_PROPERTIES,
};

/*TYPEDEF*/
/// Configuration structure
typedef struct ots_instant_cfg
{
    /// Preferred MTU size
    uint16_t pref_mtu;
    /// Primary Service instant
    bool is_primary_service;
    /// OACP features
    uint32_t oacp_features;
    /// OLCP features
    uint32_t olcp_features;
} ots_inst_cfg_t;

typedef struct ots_init_cfg
{
    /// Number of Object Transfer Services the
    /// device may belongs to supported max
    uint8_t nb_transfers_supp_max;
} ots_init_cfg_t;

/// Object Changed Information
typedef struct ots_changed_info
{
    /// Object ID
    ot_object_id_t object_id;
    /// Flags field (see enum #otp_changed_flags_bf)
    uint8_t flags;
} ots_changed_info_t;

/// Callback function called when client configuration for an instance of
///  the Object Transfer Service has been updated
typedef void (*ots_cb_bond_data)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint16_t cli_cfg_bf);
/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been established for an instance of the Object Transfer Service
typedef void (*ots_cb_coc_connected)(uint8_t con_lid, uint16_t rx_mtu, uint16_t tx_mtu);
/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been disconnected for an instance of the Object Transfer Service
typedef void (*ots_cb_coc_disconnected)(uint8_t con_lid, uint16_t reason);
/// Callback function called when data is received through LE Credit Based Connection
/// Oriented Link for an instance of the Object Transfer Service
typedef void (*ots_cb_coc_data)(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu);
/// Callback function called in order to retrieve name of current object
/// for an instance of the Object Transfer Service
typedef void (*ots_cb_get_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t offset);
/// Callback function called when a peer Client requests to set the object name
/// for an instance of the Object Transfer Service
typedef void (*ots_cb_set_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t name_len, const uint8_t *p_name);
/// Callback function called when a peer Client requests to create a new object
/// for an instance of the Object Transfer Service
typedef void (*ots_cb_object_create)(uint8_t con_lid, uint8_t transfer_lid, uint32_t size, uint8_t uuid_type, const void *p_uuid);
/// Callback function called when a peer Client requests to manipulate
/// (read, write, calculate checksum) current object for an instance of
/// the Object Transfer Service
typedef void (*ots_cb_object_execute)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint16_t param_len, const uint8_t *p_param);
/// Callback function called when a peer Client requests to manipulate
/// (read, write, calculate checksum) current object for an instance of
/// the Object Transfer Service
typedef void (*ots_cb_object_manipulate)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t opcode, uint32_t offset, uint32_t length, uint8_t mode);
/// Callback function called when a peer Client requests an operation on the
/// current object for an instance of the Object Transfer Service
typedef void (*ots_cb_object_control)(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid, uint8_t opcode);
/// Callback function called when a peer Client requests to get one of the
/// current filter rules for an instance of the Object Transfer Service
typedef void (*ots_cb_filter_get)(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint32_t ots_token, uint16_t offset);
/// Callback function called when a peer Client requests information about
/// the list of objects or request to change current object for an instance of
/// the Object Transfer Service
typedef void (*ots_cb_list)(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, const void *p_value);
/// Callback function called when a peer Client requests to set one of the
/// current filter rules for an instance of the Object Transfer Service
typedef void (*ots_cb_filter_set)(uint16_t req_ind_code, uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val, const void *p_value1, const void *p_value2);
/// Callback function called when a peer Client requests to establish a LE Credit
/// Based Connection Oriented Link for an instance of the Object Transfer Service
typedef void (*ots_cb_coc_connect)(uint8_t con_lid, uint16_t token, uint16_t peer_mtu);
/// Callback function called when a command has been handled
typedef void (*ots_cb_cmp_evt)(uint16_t cmd_code, uint16_t status, uint8_t con_lid);


/*CALLBACKS*/
/// Set of callback functions for Object Transfer Server
typedef struct ots_cb
{
    /// Callback function called when client configuration for an instance of
    ///  the Object Transfer Service has been updated
    ots_cb_bond_data cb_bond_data;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    ots_cb_coc_connected cb_coc_connected;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been disconnected for an instance of the Object Transfer Service
    ots_cb_coc_disconnected cb_coc_disconnected;
    /// Callback function called when data is received through LE Credit Based Connection
    /// Oriented Link for an instance of the Object Transfer Service
    ots_cb_coc_data cb_coc_data;
    /// Callback function called in order to retrieve name of current object
    /// for an instance of the Object Transfer Service
    ots_cb_get_name cb_get_name;
    /// Callback function called when a peer Client requests to set the object name
    /// for an instance of the Object Transfer Service
    ots_cb_set_name cb_set_name;
    /// Callback function called when a peer Client requests to create a new object
    /// for an instance of the Object Transfer Service
    ots_cb_object_create cb_object_create;
    /// Callback function called when a peer Client requests execution of current object
    /// for an instance of the Object Transfer Service
    ots_cb_object_execute cb_object_execute;
    /// Callback function called when a peer Client requests to manipulate
    /// (read, write, calculate checksum) current object for an instance of
    /// the Object Transfer Service
    ots_cb_object_manipulate cb_object_manipulate;
    /// Callback function called when a peer Client requests an operation on the
    /// current object for an instance of the Object Transfer Service
    ots_cb_object_control cb_object_control;
    /// Callback function called when a peer Client requests to get one of the
    /// current filter rules for an instance of the Object Transfer Service
    ots_cb_filter_get cb_filter_get;
    /// Callback function called when a peer Client requests information about
    /// the list of objects or request to change current object for an instance of
    /// the Object Transfer Service
    ots_cb_list cb_list;
    /// Callback function called when a peer Client requests to set one of the
    /// current filter rules for an instance of the Object Transfer Service
    ots_cb_filter_set cb_filter_set;
    /// Callback function called when a peer Client requests to establish a LE Credit
    /// Based Connection Oriented Link for an instance of the Object Transfer Service
    ots_cb_coc_connect cb_coc_connect;
    /// Callback function called when a command has been handled
    ots_cb_cmp_evt cb_cmp_evt;
} ots_evt_cb_t;

/*FUNCTIONS*/
int ots_init(const ots_init_cfg_t *p_init_cfg, const ots_evt_cb_t *p_evt_cb);
int ots_deinit(void);
int ots_add_ots_instant(const ots_inst_cfg_t *p_ots_inst_cfg, uint8_t *p_transfer_lid_ret);
#if (GAF_USE_CACHE_GATT_CCCD)
int ots_restore_cli_cfg_cache(uint8_t con_lid, uint8_t transfer_lid, uint8_t cli_cfg_bf);
#endif /// (GAF_USE_CACHE_GATT_CCCD)

int ots_coc_disconnect(uint8_t con_lid);
int ots_coc_release(uint8_t con_lid);
int ots_coc_send(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu);

int ots_object_add(const ot_object_id_t *p_object_id, uint32_t current_size, uint32_t allocated_size,
                   const gaf_prf_date_time_t *p_first_created_time, const gaf_prf_date_time_t *p_last_modified_time,
                   uint32_t properties, uint8_t uuid_type, const void *p_uuid, uint8_t *p_object_lid);
int ots_object_remove(uint8_t object_lid);
int ots_object_change(uint8_t con_lid, uint8_t transfer_lid, uint8_t object_lid);
int ots_object_changed(uint8_t flags, const ot_object_id_t *p_object_id);
int ots_set(uint8_t object_lid, uint8_t set_type, uint32_t value);
int ots_set_time(uint8_t object_lid, const gaf_prf_date_time_t *p_time);

int ots_cfm_get_name(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                     uint8_t name_len, const uint8_t *p_name);
int ots_cfm_set_name(uint16_t status, uint8_t con_lid, uint8_t transfer_lid);
int ots_cfm_object_control(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                           uint8_t result_code, uint32_t checksum);
int ots_cfm_object_execute(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                           uint8_t result_code, uint16_t rsp_len, const uint8_t *p_rsp);
int ots_cfm_filter_get(uint16_t status, uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_val);
int ots_cfm_filter_get_time(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                            uint8_t filter_val, const gaf_prf_date_time_t *p_time_start,
                            const gaf_prf_date_time_t *p_time_end);
int ots_cfm_filter_get_size(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                            uint8_t filter_val, uint32_t size_min, uint32_t size_max);
int ots_cfm_filter_get_name(uint16_t status, uint8_t con_lid, uint8_t transfer_lid, uint32_t ots_token,
                            uint8_t filter_val, uint8_t name_len, const uint8_t *p_name);
int ots_cfm_filter_get_type(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                            uint8_t uuid_type, const void *p_uuid);
int ots_cfm_list_control(uint16_t status, uint8_t con_lid, uint8_t transfer_lid,
                         uint8_t result_code, uint32_t nb_object);
int ots_cfm_filter_set(uint16_t status, uint8_t con_lid, uint8_t transfer_lid);
void ots_cfm_coc_connect(uint16_t status, uint8_t con_lid, uint16_t token);

#endif /// __ACC_OTS__