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
#ifndef __AOB_DTS_API_H__
#define __AOB_DTS_API_H__

/*****************************header include********************************/
#include "aob_mgr_gaf_evt.h"
/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/
#ifdef __cplusplus
extern "C" {
#endif

/**
 ****************************************************************************************
 * @brief Transfer data through a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 * @param[in] length        SDU length
 * @param[in] p_sdu         Pointer to SDU to be transferred to the peer device
 *
 * Rule for this API: only when the tx done event is received(#see APP_DTS_COC_SEND_IND) could the upper side releases the buffer
 *
 * @return An error status
 * ****************************************************************************************
 */
void aob_dts_send_data(uint8_t con_lid, uint16_t spsm, uint16_t length, const uint8_t *sdu);

/**
 ****************************************************************************************
 * @brief Disconnect a LE Credit Based Connection Oriented Link
 *
 * @param[in] con_lid       Connection local index
 * @param[in] spsm          Simplified Protocol/Service Multiplexer
 *
 * @return An error status
 ****************************************************************************************
 */
void aob_dts_disconnect(uint8_t con_lid, uint16_t spsm);

/**
 ****************************************************************************************
 * @brief register a spsm in order to accept L2CAP connection oriented channel (COC) from a peer device
 *
 * @param[in] spsm             Simplified Protocol/Service Multiplexer
 * @param[in] initial_credits  Initial credits
 *
 * @return An error status
 ****************************************************************************************
 */
void aob_dts_register_spsm(uint16_t spsm, uint16_t initial_credits);

void aob_dts_api_init(dts_coc_event_handler_t *dts_coc_event_cb);

#ifdef __cplusplus
}
#endif

#endif
