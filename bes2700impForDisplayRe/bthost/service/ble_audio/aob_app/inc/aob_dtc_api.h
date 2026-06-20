/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#ifndef __AOB_DTC_API_H__
#define __AOB_DTC_API_H__

/*****************************header include********************************/

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Establish a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] local_max_sdu Maximum SDU size that the local device can receive
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void aob_dtc_connect(uint8_t con_lid, uint16_t local_max_sdu, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief Transfer data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 * @param[in] length        SDU length
 * @param[in] p_sdu         Pointer to SDU to be transferred to the peer device
 *
 * @return An error status
 * ****************************************************************************************
 */
void aob_dtc_send_data(uint8_t con_lid, uint16_t spsm, uint16_t length, uint8_t *sdu);

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 *
 * @return An error status
 ****************************************************************************************
 */
void aob_dtc_disconnect(uint8_t con_lid, uint16_t spsm);

void aob_dtc_api_init(void);

#ifdef __cplusplus
}
#endif

#endif