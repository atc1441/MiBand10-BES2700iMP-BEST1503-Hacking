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

#include "app_cap.h"
#include "app_cap_cac_msg.h"
#include "app_cap_cas_msg.h"

/*EXTERNAL FUNCTIONS*/
/*CAC*/
extern int app_cap_cac_init(void);
extern int app_cap_cac_deinit(void);
extern int app_cap_cac_start(uint8_t con_lid);
/*CAS*/
extern int app_cap_cas_init(void);
extern int app_cap_cas_deinit(void);

/*FUNCTIONS*/
#ifdef AOB_MOBILE_ENABLED
int app_cap_client_init(void)
{
    return app_cap_cac_init();
}

int app_cap_client_deinit(void)
{
    return app_cap_cac_deinit();
}
#endif

int app_cap_server_init(void)
{
    return app_cap_cas_init();
}

int app_cap_server_deinit(void)
{
    return app_cap_cas_deinit();
}

#ifdef AOB_MOBILE_ENABLED
int app_cap_start(uint8_t con_lid)
{
    return app_cap_cac_start(con_lid);
}
#endif

#endif

/// @} APP

