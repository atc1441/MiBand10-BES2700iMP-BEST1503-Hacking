/**
 * @file gsound_custom_hotword_common.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-08-05
 * 
 * @copyright Copyright (c) 2015-2020 BES Technic.
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

#ifndef __GSOUND_CUSTOM_HOTWORD_COMMON_H__
#define __GSOUND_CUSTOM_HOTWORD_COMMON_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "gsound_target_hotword_common.h"

/******************************macro defination*****************************/
#define STRING_TERMINATOR_NULL "\0"
#define STRING_TERMINATOR_SIZE 1

#define GSOUND_MAX_SUPPORTED_MODELS 8
#define GSOUND_HOTWORD_SUPPORTED_MODEL_ID_BYTES \
  ((GSOUND_HOTWORD_MODEL_ID_BYTES) + (STRING_TERMINATOR_SIZE))
#define GSOUND_MAX_SUPPORTED_HOTWORD_MODELS_BYTES \
  ((GSOUND_MAX_SUPPORTED_MODELS) * (GSOUND_HOTWORD_SUPPORTED_MODEL_ID_BYTES))

/******************************type defination******************************/

/****************************function declearation**************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_HOTWORD_COMMON_H__ */