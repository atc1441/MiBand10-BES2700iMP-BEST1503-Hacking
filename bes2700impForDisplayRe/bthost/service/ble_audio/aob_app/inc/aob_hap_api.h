/***************************************************************************
 *
 * @copyright Copyright (c) 2015-2022 BES Technic.
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

#ifndef _AOB_HAP_API_H_
#define _AOB_HAP_API_H_

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declaration***************************/
void aob_hap_hac_restore_bond_data(uint8_t con_lid);

void aob_hap_hac_msg_get(uint8_t con_lid, uint8_t char_type, uint8_t presets_instance_idx);

void aob_hap_hac_msg_get_cfg(uint8_t con_lid, uint8_t char_type, uint8_t presets_instance_idx);

void aob_hap_hac_msg_set_cfg(uint8_t con_lid, uint8_t char_type,
                             uint8_t presets_instance_idx, uint8_t enable);

void aob_hap_hac_msg_set_preset_name(uint8_t con_lid, uint8_t preset_idx,
                                     uint8_t length, char *name);

void aob_hap_hac_msg_set_active_preset(uint8_t con_lid, uint8_t set_type,
                                       uint8_t coordinated, uint8_t preset_idx);

void aob_hap_has_restore_bond_data(uint8_t con_lid, uint8_t cli_cfg_bf,
                                   uint8_t presets_cli_cfg_bf, uint8_t evt_cfg_bf, uint8_t presets_evt_cfg_bf);

void aob_hap_has_msg_cfg_preset(uint8_t preset_lid, uint8_t presets_instance_idx,
                                uint8_t read_only, uint8_t length, char *name);

void aob_hap_has_msg_set_active_preset(uint8_t preset_lid);

void aob_hap_has_msg_set_coordination_sup(uint8_t supported);

void aob_hap_has_msg_set_preset_avtive_cfm(uint16_t status);



#ifdef __cplusplus
}
#endif
#endif