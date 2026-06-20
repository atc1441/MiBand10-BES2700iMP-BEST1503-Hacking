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

#ifndef __GSOUND_CUSTOM_RESET_H__
#define __GSOUND_CUSTOM_RESET_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/
#include "gsound_target_reset.h"

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            gsound_get_plateform_reset
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the pointer of plateform reset info
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void const *gsound_get_plateform_reset(void);

/**
 * Convert Platform specific crash-dump to
 * GSound crash-dump format
 *
 * Must use gsound_util_get_reset_information() to
 * Retrieve pointer to platform specific information
 */
/*---------------------------------------------------------------------------
 *            gsound_crash_convert
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    Convert Platform specific crash-dump to GSound crash-dump format
 *
 * Parameters:
 *    gsound_crash - pointer to the data send to libgsound after convert process
 *    plateform_reset - BES internal crash dump info pointer, see @CRASH_OUTPUT_BUF_T
 *                      to get more info
 *
 * Return:
 *    void
 */
void gsound_crash_convert(GSoundCrashFatalDump *gsound_crash,
                          void const *plateform_reset);

/**
 * Converts plaform specific reset reason
 * to GSound reset reason
 *
 * Must use gsound_util_get_reset_information() to
 * Retrieve pointer to platform specific information
 *
 */
/*---------------------------------------------------------------------------
 *            gsound_reset_convert
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    
 *
 * Parameters:
 *    gsound_reset - pointer to the data send to libgsound after convert process
 *    plateform_reset - BES internal crash dump info pointer, see @CRASH_OUTPUT_BUF_T
 *                      to get more info
 *
 * Return:
 *    void
 */
void gsound_reset_convert(GSoundResetReason *gsound_reset,
                          void const *plateform_reset);

/*---------------------------------------------------------------------------
 *            gsound_crash_dump_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    init gsound crash dump module
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void gsound_crash_dump_init(void);

/*---------------------------------------------------------------------------
 *            gsound_dump_set_flag
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    set the gsound crash happened flag @gsound_dump_is_happend
 *
 * Parameters:
 *    is_happened - false: crash not happened
 *                  true: crash happened
 *
 * Return:
 *    void
 */
void gsound_dump_set_flag(uint8_t is_happened);

/*---------------------------------------------------------------------------
 *            gsound_is_crash_happened
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    is crash happened or not
 *
 * Parameters:
 *    void
 *
 * Return:
 *    true - crash happened
 *    false - crash not happened
 */
bool gsound_is_crash_happened(void);

/*---------------------------------------------------------------------------
 *            gsound_get_latest_crash_dump_flash_addr
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get the latest crash info pointer
 *
 * Parameters:
 *    void
 *
 * Return:
 *    pointer of latest crash info
 */
uint32_t gsound_get_latest_crash_dump_flash_addr(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_RESET_H__ */