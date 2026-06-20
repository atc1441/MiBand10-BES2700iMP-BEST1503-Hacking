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
#ifndef __ACC_OTC__
#define __ACC_OTC__

#include "bluetooth.h"

#include "gaf_cfg.h"
#include "gaf_prf.h"

#include "ot_common.h"

/*DEFINE*/
#define OTC_CONNECTION_MAX                  (GAF_CONNECTION_MAX)

#define OTC_CP_IND_RX_TIMEOUT_DUR_MS        (gaf_prf_get_rx_ntf_ind_timeout_in_ms())

/*ENUMERATION*/

/*TYPEDEF*/
typedef struct otc_prf_svc_info
{
    /// Service handler range
    gatt_prf_svc_range_t svc_range;
    /// Service UUID
    uint16_t uuid;
} otc_prf_svc_info_t;

typedef struct otc_init_cfg
{
    /// Max supported ots instant found
    uint8_t supp_ots_found_max;
    /// Preferred MTU size
    uint16_t pref_mtu;
} otc_init_cfg_t;

/// Callback function called when a discovery is complete
typedef void (*otc_cb_discovery_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback function called when an instance of the Object Transfer Service
/// has been found in Server device database
typedef void (*otc_cb_bond_data)(uint8_t con_lid, uint8_t transfer_lid, const otc_prf_svc_info_t *param);
/// Callback for otc gatt set cfg cmp evt
typedef void (*otc_cb_set_cfg_cmp_evt)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint16_t err_code);
/// Callback function called when non-empty execution response for current object of
/// an instance of the Object Transfer Service has been received
typedef void (*otc_cb_execute_rsp)(uint8_t con_lid, uint8_t transfer_lid, uint16_t rsp_len,
                                   const uint8_t *p_rsp);
/// Callback function called when information about current object have been received
typedef void (*otc_cb_value)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                             uint32_t value1, uint32_t value2);
/// Callback function called when either time of creation or time of last update
/// for current object for an instance of the Object Transfer Service has been received
typedef void (*otc_cb_time)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type,
                            const gaf_prf_date_time_t *p_time);
/// Callback function called when a alcp command has been handled
typedef void (*otc_cb_olcp_cmp_evt)(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t result_code, uint32_t checksum, uint16_t err_code);
/// Callback function called when a oacp command has been handled
typedef void (*otc_cb_oacp_cmp_evt)(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t result_code, uint8_t nb_obj, uint16_t err_code);
/// Callback function called when a command has been handled
typedef void (*otc_cb_set_cmp_evt)(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, uint8_t char_inst_id, uint16_t err_code);
/// Callback function called when Object ID of current object for an instance of
/// the Object Transfer Service has been received
typedef void (*otc_cb_object_id)(uint8_t con_lid, uint8_t transfer_lid, const ot_object_id_t *p_object_id);
/// Callback function called when UUID of current object for an instance of the
/// Object Transfer Service has been received
typedef void (*otc_cb_type)(uint8_t con_lid, uint8_t transfer_lid, uint8_t uuid_type, const void *p_uuid);
/// Callback function called when name of current object for an instance of the
/// Object Transfer Service has been received
typedef void (*otc_cb_name)(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t *p_name);
/// Callback function called when a filter rule has been received
typedef void (*otc_cb_filter)(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                              uint8_t filter_val, const void *p_value1, const void *p_value2);
/// Callback function called when peer Server notifies that current object for
/// an instance of the Object Transfer Service has been updated
typedef void (*otc_cb_changed)(uint8_t con_lid, uint8_t transfer_lid, uint8_t flags,
                               const ot_object_id_t *p_object_id);
/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been established
typedef void (*otc_cb_coc_connected)(uint8_t con_lid, uint16_t peer_mtu, uint16_t local_mtu);
/// Callback function called when an LE Credit Based Connection Oriented Link has
/// been disconnected for an instance of the Object Transfer Service
typedef void (*otc_cb_coc_disconnected)(uint8_t con_lid, uint16_t reason);
/// Callback function called when data is received through LE Credit Based Connection
/// Oriented Link for an instance of the Object Transfer Service
typedef void (*otc_cb_coc_data)(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu);
/// Callback function called when a coc send command has been handled
typedef void (*otc_cb_coc_send_cmp_evt)(uint8_t con_lid, uint16_t err_code);
/// Callback for otc prf event
typedef gatt_prf_status_event_callback otc_cb_prf_status_evt;

/// Set of callback functions for Object Transfer Client
typedef struct otc_cb
{
    /// Callback function called when a discovery is complete
    otc_cb_discovery_cmp_evt cb_discovery_cmp;
    /// Callback function called when an instance of the Object Transfer Service has been discovered
    otc_cb_bond_data cb_bond_data;
    /// Callback for otc gatt set cfg cmp evt
    otc_cb_set_cfg_cmp_evt cb_set_cfg_cmp;
    /// Callback function called when non-empty execution response for current object of
    /// an instance of the Object Transfer Service has been receive
    otc_cb_execute_rsp cb_execute_rsp;
    /// Callback function called when information about current object have been received
    otc_cb_value cb_value;
    /// Callback function called when either time of creation or time of last update for
    /// current object for an instance of the Object Transfer Service has been received
    otc_cb_time cb_time;
    /// Callback function called when Object ID of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_object_id cb_object_id;
    /// Callback function called when UUID of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_type cb_type;
    /// Callback function called when name of current object for an instance of
    /// the Object Transfer Service has been received
    otc_cb_name cb_name;
    /// Callback function called when a filter rule has been received
    otc_cb_filter cb_filter;
    /// Callback function called when a oacp command has been handled
    otc_cb_oacp_cmp_evt cb_oacp_cmp_evt;
    /// Callback function called when a olcp command has been handled
    otc_cb_olcp_cmp_evt cb_olcp_cmp_evt;
    /// Callback function called when a set val command has been handled
    otc_cb_set_cmp_evt cb_set_cmp_evt;
    /// Callback function called when peer Server notifies that current object
    /// for an instance of the Object Transfer Service has been updated
    otc_cb_changed cb_changed;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    otc_cb_coc_connected cb_coc_connected;
    /// Callback function called when an LE Credit Based Connection Oriented Link has
    /// been established for an instance of the Object Transfer Service
    otc_cb_coc_disconnected cb_coc_disconnected;
    /// Callback function called when data is received through LE Credit Based Connection
    /// Oriented Link for an instance of the Object Transfer Service
    otc_cb_coc_data cb_coc_data;
    /// Callback function called when a coc send command has been handled
    otc_cb_coc_send_cmp_evt cb_coc_send_cmp;
    /// Callback function called when prf status event generated
    otc_cb_prf_status_evt cb_prf_status_event;
} otc_evt_cb_t;

/*FUNCTIONS*/
int otc_init(const otc_init_cfg_t *p_init_cfg, const otc_evt_cb_t *p_evt_cb);
int otc_deinit(void);

int otc_service_discovery(uint8_t con_lid);
int otc_character_value_read(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type);
int otc_character_cccd_write(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, bool enable_ind);

int otc_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t name_len, const uint8_t *p_name);
int otc_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t char_type, const gaf_prf_date_time_t *p_time);
int otc_set_properties(uint8_t con_lid, uint8_t transfer_lid, uint32_t properties);
int otc_object_create(uint8_t con_lid, uint8_t transfer_lid, uint32_t size, uint8_t uuid_type, const void *p_uuid);
int otc_object_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode);
int otc_object_manipulate(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint32_t offset, uint32_t length, uint8_t mode);
int otc_object_execute(uint8_t con_lid, uint8_t transfer_lid, uint16_t param_len, const uint8_t *p_param);
int otc_list_control(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, uint8_t order);
int otc_list_goto(uint8_t con_lid, uint8_t transfer_lid, uint8_t opcode, const ot_object_id_t *p_object_id);

int otc_filter_set(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val);
int otc_filter_set_time(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                        const gaf_prf_date_time_t *p_time_start, const gaf_prf_date_time_t *p_time_end);
int otc_filter_set_size(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                        uint32_t size_min, uint32_t size_max);
int otc_filter_set_name(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid, uint8_t filter_val,
                        uint8_t name_len, const uint8_t *p_name);
int otc_filter_set_type(uint8_t con_lid, uint8_t transfer_lid, uint8_t filter_lid,
                        uint8_t uuid_type, const void *p_uuid);

int otc_coc_connect(uint8_t con_lid, uint16_t local_max_sdu);
int otc_coc_disconnect(uint8_t con_lid);
int otc_coc_send(uint8_t con_lid, uint16_t length, const uint8_t *p_sdu);

#endif /// __ACC_OTC__