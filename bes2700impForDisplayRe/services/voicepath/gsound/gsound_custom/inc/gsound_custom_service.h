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

#ifndef __GSOUND_CUSTOM_SERVICE_H__
#define __GSOUND_CUSTOM_SERVICE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*****************************header include********************************/

/******************************macro defination*****************************/

/******************************type defination******************************/

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            gsound_service_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    initialize the gsound service
 *
 * Parameters:
 *    enable - true to enable the gsound service
 *             false to disable the gsound service
 *
 * Return:
 *    void
 */
void gsound_service_init(bool enable);

/*---------------------------------------------------------------------------
 *            gsound_service_enable_switch
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    switch on/off the gsound service
 *
 * Parameters:
 *    isEnable - true to switch on the gsound service
 *               false to switch off the gsound service
 *
 * Return:
 *    void
 */
void gsound_service_enable_switch(bool isEnable);

/*---------------------------------------------------------------------------
 *            gsound_sn_conv
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    Convert BES Serial Number to GSound Serial Number
 *
 * Parameters:
 *    bes_sn - sn pointer for plateform
 *    bes_len - sn length
 *    gsound_sn - gsound sn pointer
 *    gsound_len - gsound sn length
 *
 * Return:
 *    void
 */
void gsound_util_sn_conv(uint8_t const *const bes_sn,
                         uint32_t bes_len,
                         uint8_t *const gsound_sn,
                         uint32_t gsound_len);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_SERVICE_H__ */