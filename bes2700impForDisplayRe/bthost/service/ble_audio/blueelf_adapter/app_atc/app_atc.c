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
/**
 ****************************************************************************************
 * @addtogroup APP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "ble_app_dbg.h"
#include "app_atc.h"
#include "app_atc_csisc_msg.h"
#include "app_atc_csism_msg.h"

/*EXTERNAL FUNCTIONS*/
/*CSIS COORDINATOR*/
extern int app_atc_csisc_init(void);
extern int app_atc_csisc_deinit(void);
/*CSIS MEMBER*/
extern int app_atc_csism_init(void);
extern int app_atc_csism_deinit(void);

/*FUNCTIONS*/
#ifdef AOB_MOBILE_ENABLED
int app_atc_client_init(void)
{
    return app_atc_csisc_init();
}

int app_atc_client_deinit(void)
{
    return app_atc_csisc_deinit();
}

int app_atc_start(uint8_t con_lid)
{
    return app_atc_csisc_start(con_lid);
}
#endif

int app_atc_server_init(void)
{
    return app_atc_csism_init();
}

int app_atc_server_deinit(void)
{
    return app_atc_csism_deinit();
}

#endif

/// @} APP

