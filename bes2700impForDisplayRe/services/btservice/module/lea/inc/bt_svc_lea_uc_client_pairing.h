/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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
 * @brief xxx.
 *
 ****************************************************************************/

/****************************** header include ********************************/
#include <stdint.h>

/***************************** external declaration *****************************/

/***************************** macro defination *******************************/

/*****************************  type defination ********************************/

/*****************************  variable defination *****************************/

/*****************************  function declaration ****************************/

/**
 ***************************************************************************
 * @brief Pairing start
 * @param[in]  : dev_num, pairing device number
 * @param[in]  : dev, pairing device info, see@bt_svc_lea_uc_cli_pairing_t
 * @param[out] : none
 * @return: error code, see@bt_status_t
 ***************************************************************************
 */
void bt_svc_lea_uc_cli_pairing_start(uint8_t dev_num, void* dev_info);

/**
 ***************************************************************************
 * @brief Pairing stop
 * @param[in] : none
 * @param[out] : none
 * @return: error code, see@bt_status_t
 ***************************************************************************
 */
void bt_svc_lea_uc_cli_pairing_stop(void);
