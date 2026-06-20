/**
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
#ifndef __APP_WALKIE_TALKIE_KEY_HANDLER_H__
#define __APP_WALKIE_TALKIE_KEY_HANDLER_H__

typedef enum
{
    WT_TALK_BTN_DOWN,
    WT_TALK_BTN_UP,   
}APP_W_T_KEY_EVENT_E;

void app_walkie_talkie_key_init(void);

#endif /* __APP_WALKIE_TALKIE_KEY_HANDLER_H__ */
