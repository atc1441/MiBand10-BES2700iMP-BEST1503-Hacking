/***************************************************************************
*
*Copyright 2015-2019 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/


#ifndef __GSOUND_CUSTOM_ACTIONS_H__
#define __GSOUND_CUSTOM_ACTIONS_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "app_key.h"
#include "gsound_target.h"

/******************************macro defination*****************************/
#define BOOL_TO_BIT(b) ((b) ? 1 : 0)
#define BIT_TO_BOOL(b) (( bool )((b) != 0))

/******************************type defination******************************/

/****************************function declearation**************************/
void gsound_custom_actions_store_lib_interface(const GSoundActionInterface *handlers);

void gsound_custom_actions_bes_handle_last_key(void);

GSoundStatus gsound_util_google_key_to_action(uint8_t event,
                                              GSoundActionMask *result);

GSoundStatus gsound_util_key_to_action(const APP_KEY_STATUS *status,
                                       GSoundActionEvents *result);

bool gsound_util_key_is_shared(APP_KEY_STATUS *status);

void gsound_custom_actions_handle_key(APP_KEY_STATUS *status, void *param);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* #ifndef __GSOUND_CUSTOM_ACTIONS_H__ */
