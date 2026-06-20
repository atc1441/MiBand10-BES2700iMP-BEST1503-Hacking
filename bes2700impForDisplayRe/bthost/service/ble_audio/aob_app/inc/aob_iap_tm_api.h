/***************************************************************************
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
/**
 ****************************************************************************************
 * @addtogroup AOB_APP
 * @{
 ****************************************************************************************
 */

#ifndef _AOB_IAP_TM_API_H_
#define _AOB_IAP_TM_API_H_

#ifdef __cplusplus
extern "C" {
#endif
/*****************************header include********************************/

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declaration***************************/

void aob_iap_msg_test_mode_start(uint8_t stream_lid, uint8_t transmit, uint8_t payload_type);

void aob_iap_msg_read_iso_tx_sync_cmd(uint8_t stream_lid);

void aob_iap_msg_tm_cnt_get(uint8_t stream_lid);

void aob_iap_msg_test_mode_stop(uint8_t stream_lid);

#ifdef __cplusplus
}
#endif
#endif