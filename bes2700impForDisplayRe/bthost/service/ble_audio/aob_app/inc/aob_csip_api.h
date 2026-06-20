/**
 *
 * @copyright Copyright (c) 2015-2021 BES Technic.
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
 */

#ifndef __AOB_CSIP_API_H__
#define __AOB_CSIP_API_H__

/*****************************header include********************************/
#include "app_atc_csism_msg.h"
#include "app_atc_csisc_msg.h"

/******************************macro defination*****************************/
#define AOB_CSIP_ADV_INTERVAL_MS        (160)       //millisecond unit
#define AOB_CSIP_RSI_ADV_AD_TYPE        (0x2E)
#define AOB_CSIP_LOCK_TIMEOUT           (60)           //second unit

/******************************type defination******************************/
typedef struct
{
    uint8_t len;
    uint8_t type;
    uint8_t value[APP_ATC_CSIS_RSI_LEN];
} CSIP_PSIR_ADV_T;

typedef struct
{
    uint8_t                     rsi[APP_ATC_CSIS_RSI_LEN];
    APP_ATC_CSISM_ENV_T        *gaf_csism_scb;
    uint8_t                     size;
} AOB_CSIP_ENV_T;


/****************************function declaration***************************/
#ifdef __cplusplus
extern "C" {
#endif


// TODO: store it into nv_record section, manufacturer should write it into factory section before production
#define AOB_CSIP_SIRK_TEST               "1234567891234567"

void aob_csip_if_update_rsi_data();
bool aob_csip_if_get_rsi_data(uint8_t *p_rsi_get);
void aob_csip_if_update_sirk(uint8_t *sirk, uint8_t sirk_len);
void aob_csip_if_user_parameters_init(uint8_t size);
void aob_csip_if_use_temporary_sirk();
void aob_csip_if_refresh_sirk(uint8_t *sirk);
void aob_csip_config_is_use_custom_sirk(bool use_custom_sirk);
bool aob_csip_is_use_custom_sirk(void);
void aob_csip_set_custom_sirk(uint8_t *sirk);
bool aob_csip_if_get_sirk(uint8_t *sirk);
bool aob_csip_if_set_device_numbers(uint8_t dev_num);
uint8_t aob_csip_if_get_device_numbers(void);

/******************************************************************************
 * Function         aob_csip_if_delete_nv_sirk nv
 * Description      delete the sirk in nv
 * Returns          void.
******************************************************************************/
void aob_csip_if_delete_nv_sirk(void);

bool aob_csip_sirk_already_refreshed(void);
void aob_csip_erase_sirk_record(void);


/**
 ****************************************************************************************
 * @brief Restore bond data after reconnection with a trusted device
 *
 * @param[in] con_lid       Connection local index
 * @param[in] set_lid       Coordinated Set local index
 * @param[in] locked        Indicate if the peer device is the device for which lock has been granted
 *
 * @return none
 ****************************************************************************************
 */
void aob_csip_if_restore_bond(uint8_t con_lid, uint8_t set_lid, bool locked);

void aob_csip_if_init(void);


///////////////////////////////////////////////////////////////////////////////////////////
#ifdef AOB_MOBILE_ENABLED
/******************************macro defination*****************************/
#define AOB_CSIP_MOBILE_DISCOVER_MEMBERS_TIMEOUT_VALUE          (10000) //10s

/******************************type defination******************************/
typedef struct
{
    bool            inprogress_flag;
    ble_bdaddr_t    addr2connect;
    uint8_t         rsi[6];
    uint8_t         result;
} AOB_CSIP_RSI_RESOLVE_CMD_T;

typedef enum
{
    AOB_CSIP_SIRK_OPCODE_ADD = 0,
    AOB_CSIP_SIRK_OPCODE_REMOVE,
    AOB_CSIP_SIRK_OPCODE_MAX,
} AOB_CSIP_SIRK_OPCODE_T;

// sirk add/remove manager data strcut
typedef struct
{
    bool            inprogress_flag;
    uint8_t         result;
    uint8_t         key_lid;
    uint8_t         sirk[16];
} AOB_CSIP_SIRK_MANAGER_CMD_T;

typedef union
{
    AOB_CSIP_RSI_RESOLVE_CMD_T          rsi_resolve_cmd;
    AOB_CSIP_SIRK_MANAGER_CMD_T         sirk_opcode_cmd;
} AOB_CSIP_CMD_RESULT_T;

typedef struct
{
    bool            inuse;
    uint8_t         con_lid;
    uint8_t         addr[6];
    uint8_t         addr_type;
} AOB_CSIP_DEV_T;

typedef struct
{
    bool            inuse;
    uint8_t         sirk[16];
    uint8_t         member_numbers;
    AOB_CSIP_DEV_T  dev_info[AOB_COMMON_MOBILE_CONNECTION_MAX];
    //list_t*       dev_list;
} AOB_CSIP_GROUP_INFO_T;

typedef struct
{
    bool                    discover_done;
    bool                    start_discover_memebers;
    osTimerId               discover_members_timer;
    AOB_CSIP_CMD_RESULT_T   cmd_result_info;    //TODO: it is better to move it into handler on the gaf core layer.
    AOB_CSIP_GROUP_INFO_T   group_info;
    APP_ATC_CSISC_ENV_T     *p_csisc_env;
} AOB_CSIP_MOBILE_ENV_T;

/****************************function declaration***************************/
bool aob_csip_mobile_if_get_group_device_numbers(uint8_t con_lid, uint8_t *dev_nbs_out);
bool aob_csip_mobile_if_get_device_rank_index(uint8_t con_lid, uint8_t *rank_index_out);
bool aob_csip_mobile_if_get_device_lock_status(uint8_t con_lid, uint8_t *lock_status_out);
bool aob_csip_mobile_if_get_sirk_value(uint8_t con_lid, uint8_t *sirk_value_out);
bool aob_csip_mobile_if_add_sirk(uint8_t *sirk);
bool aob_csip_mobile_if_remove_sirk(uint8_t *sirk);
void aob_csip_mobile_if_resolve_rsi_data(uint8_t *data, uint8_t data_len, ble_bdaddr_t *bdaddr_to_connect);
bool aob_csip_mobile_if_has_connected_device();
void aob_csip_if_discovery_service(uint8_t conlid);
void aob_csip_if_lock(uint8_t conlid, uint8_t set_lid, uint8_t lock);
void aob_csip_mobile_if_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __AOB_CSIP_API_H__ */

/// @} AOB_APP

