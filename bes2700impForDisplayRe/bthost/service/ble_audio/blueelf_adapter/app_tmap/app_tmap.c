/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
 * @addtogroup APP_TMAP
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#if BLE_AUDIO_ENABLED
#include "ble_app_dbg.h"

#include "app_tmap_tmac_msg.h"
#include "app_tmap_tmas_msg.h"

/*EXTERNAL FUNCTIONS*/
/*TMAC*/
extern int app_tmap_tmac_init(void);
extern int app_tmap_tmac_deinit(void);
/*TMAS*/
extern int app_tmap_tmas_init(uint16_t);
extern int app_tmap_tmas_deinit(void);

/*FUNCTIONS*/
int app_tmap_client_init(void)
{
    return app_tmap_tmac_init();
}

int app_tmap_client_deinit(void)
{
    return app_tmap_tmac_deinit();
}

int app_tmap_server_init(uint16_t role_bf)
{
    return app_tmap_tmas_init(role_bf);
}

int app_tmap_server_deinit(void)
{
    return app_tmap_tmas_deinit();
}

int app_tmap_start(uint8_t con_lid)
{
    return app_tmap_tmac_start(con_lid);
}

#endif
/// @} APP_TMAP


