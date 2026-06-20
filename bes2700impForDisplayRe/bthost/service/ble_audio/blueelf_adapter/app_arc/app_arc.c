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
#include "app_arc.h"
#include "app_arc_vcc_msg.h"
#include "app_arc_vcs_msg.h"
#include "app_arc_micc_msg.h"
#include "app_arc_mics_msg.h"
#include "app_arc_aicc_msg.h"
#include "app_arc_aics_msg.h"
#include "app_arc_vocc_msg.h"
#include "app_arc_vocs_msg.h"

/*EXTERNAL FUNCTIONS*/
/*VOCC*/
extern int app_arc_vocc_init(void);
extern int app_arc_vocc_deinit(void);
/*AICC*/
extern int app_arc_aicc_init(void);
extern int app_arc_aicc_deinit(void);
/*AICS*/
extern int app_arc_aics_init(void);
extern int app_arc_aics_deinit(void);
/*VOCS*/
extern int app_arc_vocs_init(void);
extern int app_arc_vocs_deinit(void);
/*MICC*/
extern int app_arc_micc_start(uint8_t con_lid);
extern int app_arc_micc_init(void);
extern int app_arc_micc_deinit(void);
/*MICS*/
extern int app_arc_mics_init(void);
extern int app_arc_mics_deinit(void);
/*VCC*/
extern int app_arc_vcc_start(uint8_t con_lid);
extern int app_arc_vcc_init(void);
extern int app_arc_vcc_deinit(void);
/*VCS*/
extern int app_arc_vcs_init(void);
extern int app_arc_vcs_deinit(void);

/*FUNCTIONS*/
#ifdef AOB_MOBILE_ENABLED
int app_arc_client_init(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_arc_vocc_init();

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_aicc_init();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_micc_init();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_vcc_init();
    }

    return status;
}

int app_arc_client_deinit(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_arc_vocc_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_aicc_deinit();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_micc_deinit();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_vcc_deinit();
    }

    return status;
}

int app_arc_start(uint8_t con_lid)
{
    uint16_t status = app_arc_micc_start(con_lid);

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_vcc_start(con_lid);
    }

    return status;
}
#endif

int app_arc_server_init(void)
{
    uint16_t status = app_arc_mics_init();

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_vcs_init();
    }

    return status;
}

int app_arc_server_deinit(void)
{
    uint16_t status = app_arc_mics_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = app_arc_vcs_deinit();
    }

    return status;
}

#endif

/// @} APP

