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


#ifndef __GSOUND_CUSTOM_TWS_H__
#define __GSOUND_CUSTOM_TWS_H__

#ifdef __cplusplus
extern "C"{
#endif

/*****************************header include********************************/

/******************************macro defination*****************************/
#define WITH_GSOUND_ROLE_SWITCH_INTEGRATEDx

/******************************type defination******************************/
typedef enum
{
    GS_ROLE_PRIMARY = 0,
    GS_ROLE_SECONDARY = 1,
    GS_ROLE_NUM,
} GS_ROLE_E;

/****************************function declearation**************************/
/*---------------------------------------------------------------------------
 *            gsound_tws_register_target_handle
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    register target tws handle
 *
 * Parameters:
 *    handle - pointer of target tws handle
 *
 * Return:
 *    void
 */
void gsound_tws_register_target_handle(const void *handle);

/*---------------------------------------------------------------------------
 *            gsound_tws_get_target_handle
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    get gsound tws target handle
 *
 * Parameters:
 *    void
 *
 * Return:
 *    pointer of gsound tws target handle
 */
const void *gsound_tws_get_target_handle(void);

/*---------------------------------------------------------------------------
 *            gsound_tws_init
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    init tws related parameters
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void gsound_tws_init(void);

/*---------------------------------------------------------------------------
 *            gsound_tws_request_roleswitch
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    gsound request role switch handler
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
bool gsound_tws_request_roleswitch(void);

/*---------------------------------------------------------------------------
 *            gsound_tws_role_update
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update the role of gsound tws
 *
 * Parameters:
 *    newRole - the role to set
 *
 * Return:
 *    void
 */
void gsound_tws_role_update(uint8_t newRole);

/*---------------------------------------------------------------------------
 *            gsound_tws_on_roleswitch_accepted_by_lib
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    role switch request is accepted by libgsound handler
 *
 * Parameters:
 *    en - accept role switch or not
 *
 * Return:
 *    void
 */
void gsound_tws_on_roleswitch_accepted_by_lib(bool en);

/*---------------------------------------------------------------------------
 *            gsound_tws_roleswitch_lib_complete
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    role switch of the libgsound is complete handler
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void gsound_tws_roleswitch_lib_complete(void);

/*---------------------------------------------------------------------------
 *            gsound_tws_inform_roleswitch_done
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    inform libgsound that system role switch is done
 *
 * Parameters:
 *    void
 *
 * Return:
 *    void
 */
void gsound_tws_inform_roleswitch_done(void);
#ifdef IBRT
/*---------------------------------------------------------------------------
 *            gsound_tws_update_roleswitch_initiator
 *---------------------------------------------------------------------------
 *
 *Synopsis:
 *    update role switch initiator for gsound
 *
 * Parameters:
 *    role - role of the initiator
 *
 * Return:
 *    void
 */
void gsound_tws_update_roleswitch_initiator(uint8_t role);
#endif

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __GSOUND_CUSTOM_TWS_H__ */