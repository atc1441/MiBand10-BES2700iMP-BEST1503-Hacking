/**
 * @file gsound_dbg.h
 * @author BES AI team
 * @version 0.1
 * @date 2020-04-22
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

#ifndef __GSOUND_DBG_H__
#define __GSOUND_DBG_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/
#include "hal_trace.h"

/******************************macro defination*****************************/
#define GLOG_V(str, ...) TR_VERBOSE(TR_MOD(AI_GVA), str, ##__VA_ARGS__)
#define GLOG_D(str, ...) TR_DEBUG(TR_MOD(AI_GVA), str, ##__VA_ARGS__)
#define GLOG_I(str, ...) TR_INFO(TR_MOD(AI_GVA), str, ##__VA_ARGS__)
#define GLOG_W(str, ...) TR_WARN(TR_MOD(AI_GVA), str, ##__VA_ARGS__)
#define GLOG_E(str, ...) TR_ERROR(TR_MOD(AI_GVA), str, ##__VA_ARGS__)

/******************************type defination******************************/

/****************************function declearation**************************/

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_DBG_H__ */