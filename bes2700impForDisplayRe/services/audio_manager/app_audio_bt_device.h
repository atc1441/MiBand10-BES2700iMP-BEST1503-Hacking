/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
 #ifndef APP_AUDIO_BT_DEVICE_H__
 #define APP_AUDIO_BT_DEVICE_H__

 typedef enum
 {
     UNKNOWN_DEVICE = 0,
     AUDIO_TYPE_BT,
     AUDIO_TYPE_LE_AUDIO,
     AUDIO_TYPE_WALKIE_TALKIE,
     AUDIO_TYPE_BIS_TRANSPORT,
     AUDIO_TYPE_LOCAL_AUDIO,
     AUDIO_TYPE_UNSPECIFIED,
     AUDIO_TYPE_NUM,
 } AUDIO_DEVICE_TYPE_E;

#define AUDIO_TYPE_WALKIE_TALKIE       3

typedef struct
{
    uint8_t device_type;
    uint8_t device_id;
}BT_AUDIO_DEVICE_T;

 #endif /* APP_AUDIO_BT_DEVICE_H__ */

