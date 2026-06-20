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
#ifndef __GAF_PRF__
#define __GAF_PRF__

#include "bluetooth.h"

#include "gatt_service.h"
#include "prf_types.h"

/// Possible values for setting client configuration characteristics
enum gatt_prf_cli_cfg
{
    /// Stop notification/indication
    GATT_PRF_CLI_STOP_NTF_IND = 0x0000,
    /// Start notification
    GATT_PRF_CLI_START_NTF   = 0x0001,
    /// Start indication
    GATT_PRF_CLI_START_IND   = 0x0002,
};

enum gatt_prf_cli_cfg_packed
{
    /// Start notification
    GATT_PRF_CLI_START_NTF_PACKED   = 0x0100,
    /// Start indication
    GATT_PRF_CLI_START_IND_PACKED   = 0x0200,
};

enum gatt_prf_ntf_policy
{
    /// Send ntf to all connection
    GATT_PRF_NTFIND_POLICY_SEND_TO_ALL = 0,
    /// Send ntf to all but specific
    GATT_PRF_NTFIND_POLICY_SEND_ALL_BUT,
    /// Send ntf to all specific
    GATT_PRF_NTFIND_POLICY_SEND_SPECIFIC,
};

typedef enum gatt_prf_event
{
    /// Profile event min
    GATT_PRF_STATUS_EVENT_MIN = 0,
    /// Prfoile opened
    GATT_PRF_STATUS_EVENT_OPENED,
    /// Profile closed
    GATT_PRF_STATUS_EVENT_CLOSED,
    /// Profile receive service changed
    GATT_PRF_STATUS_EVENT_SVC_CHANGED,
    /// Profile event max
    GATT_PRF_STATUS_EVENT_MAX,
} gatt_prf_status_event_e;

typedef void (*gatt_prf_status_event_callback)(uint8_t con_lid, bool is_central, gatt_prf_status_event_e event);

typedef union gatt_read_write_rsp_ctx
{
    void *p_ctx;
    /// GATT read ctx
    gatt_server_read_ctx_t *p_rd_ctx;
    /// GATT write ctx
    gatt_server_write_ctx_t *p_wr_ctx;
} gatt_rd_wr_rsp_ctx_u;

typedef struct gatt_disc_additional_param
{
    /// UUID list size
    uint8_t uuid_list_size;
    /// UUID list poniter
    const uint16_t *p_uuid_list;
} gatt_disc_add_param_t;

typedef struct gaf_prf_init_cfg
{
    /// Perform a timeout to refresh state
    /// Should more than @see GAF_RX_NTF_IND_TIMEOUT_MS
    uint8_t rx_ntf_ind_timeout_s;
    /// Support BTGATT of gaf
    bool gaf_btgatt_support;
} gaf_prf_cfg_t;

/// Service information structure
typedef prf_svc_t gatt_prf_svc_range_t;

/// Characteristic information structure
typedef prf_char_t gatt_prf_char_t;

/// Descriptor information structure
typedef prf_desc_t gatt_prf_desc_t;

/// Time profile information
typedef prf_date_time_t gaf_prf_date_time_t;

/// Date profile information - 4 bytes
typedef prf_date_t gaf_prf_date_t;

/*FUNCTIONS*/
/**
 * @brief GAF profile initilization
 *
 * @param  p_init_cfg  Initilization configuration
 *
 * @return int         status
 */
int gaf_prf_init(const gaf_prf_cfg_t *p_init_cfg);

/**
 * @brief GAF profile deinitilization
 *
 *
 * @return int         status
 */
int gaf_prf_deinit(void);

/**
 * @brief Get gaf timeout ms that waiting for a result from peer
 *
 *
 * @return uint32_t    Waiting timeout in ms
 */
uint32_t gaf_prf_get_rx_ntf_ind_timeout_in_ms(void);

/**
 * @brief Get or alloc gaf layer conlid by conidx from btgatt
 *
 * @param  conidx      Connection index from gap layer
 *
 * @return uint8_t     Connection local index from gaf layer
 */
uint8_t gaf_prf_alloc_con_lid_for_btgatt(uint8_t conidx, uint16_t event);

/**
 * @brief Get gap layer conidx by con_lid from gaf layer
 *
 * @param  con_lid     Connection local index from gaf layer
 *
 * @return uint8_t     Connection index from gap layer
 */
uint8_t gaf_prf_get_gap_conidx_by_con_lid(uint8_t con_lid);

/**
 * @brief Get connection local index from gaf layer by gap conidx
 *
 * @param  conidx      Connection index from gap layer
 *
 * @return uint8_t     Connection local index from gaf layer
 */
uint8_t gaf_prf_get_con_lid_by_gap_conidx(uint8_t conidx);

/**
 * @brief Get prfoile att connection handle
 *
 * @param  con_lid     Connection local index
 *
 * @return uint16_t    ATT connection handle
 */
uint16_t gaf_prf_get_att_conn_hdl(uint8_t con_lid);

/**
 * @brief Control all gaf service visibility in gatt database
 *
 * @param  connhdl     GAP Connection Handle
 * @param  visible     Service visibility in gatt discovery
 *
 */
void gaf_prf_control_all_service(uint16_t connhdl, bool visible);

/**
 * @brief Pack data time
 *
 * @param  p_buf       Buffer to return
 * @param  p_date_time Date time to be packed
 *
 */
uint8_t gaf_prf_pack_date_time(uint8_t *p_buf, const gaf_prf_date_time_t *p_date_time);

/**
 * @brief Pack date
 *
 * @param  p_buf       Buffer to return
 * @param  p_date      Date to be packed
 *
 */
uint8_t gaf_prf_pack_date(uint8_t *p_buf, const gaf_prf_date_t *p_date);

/**
 * @brief Unpack date time
 *
 * @param  p_buf       Buffer to return
 * @param  p_date_time Date time to be unpacked
 *
 */
uint8_t gaf_prf_unpack_date_time(const uint8_t *p_buf, gaf_prf_date_time_t *p_date_time);

/**
 * @brief Unpack date
 *
 * @param  p_buf       Buffer to return
 * @param  p_date      Date time to be unpacked
 *
 */
uint8_t gaf_prf_unpack_date(const uint8_t *p_buf, gaf_prf_date_t *p_date);

#endif /// __GAF_PRF__