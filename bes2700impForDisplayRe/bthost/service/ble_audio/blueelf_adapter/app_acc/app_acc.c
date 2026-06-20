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

#include "app_acc.h"
#include "app_acc_tbc_msg.h"
#include "app_acc_mcc_msg.h"
#include "app_acc_otc_msg.h"
#include "app_acc_dtc_msg.h"
#include "app_acc_ots_msg.h"
#include "app_acc_tbs_msg.h"
#include "app_acc_dts_msg.h"

/*EXTERNAL FUNCTIONS*/
/*OTC*/
extern int app_acc_otc_init(void);
extern int app_acc_otc_deinit(void);
/*OTS*/
extern int app_acc_ots_init(void);
extern int app_acc_ots_deinit(void);
extern int app_acc_ots_start(uint8_t con_lid);
/*MCC*/
extern int app_acc_mcc_init(void);
extern int app_acc_mcc_deinit(void);
/*MCC*/
extern int app_acc_mcs_init(void);
extern int app_acc_mcs_deinit(void);
/*TBC*/
extern int app_acc_tbc_init(void);
extern int app_acc_tbc_deinit(void);
/*TBS*/
extern int app_acc_tbs_init(void);
extern int app_acc_tbs_deinit(void);

/*FUCNTIONS*/
int app_acc_client_init(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_acc_otc_init();

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_mcc_init();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_tbc_init();
    }

    return status;
}

int app_acc_client_deinit(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_acc_otc_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_mcc_deinit();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_tbc_deinit();
    }

    return status;
}

#ifdef AOB_MOBILE_ENABLED
int app_acc_server_init(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_acc_ots_init();

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_mcs_init();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_tbs_init();
    }

    return status;
}

int app_acc_server_deinit(void)
{
    uint16_t status = BT_STS_SUCCESS;

    status = app_acc_ots_deinit();

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_mcs_deinit();
    }

    if (status == BT_STS_SUCCESS)
    {
        status = app_acc_tbs_deinit();
    }

    return status;
}
#endif

int app_acc_start(uint8_t con_lid, bool is_mobile)
{
    uint16_t status = BT_STS_SUCCESS;

#ifdef AOB_MOBILE_ENABLED
    if (is_mobile)
    {
        status = app_acc_ots_start(con_lid);
    }
    else
#endif
    {
        status = app_acc_mcc_start(con_lid);

        if (status == BT_STS_SUCCESS)
        {
            status = app_acc_tbc_start(con_lid);
        }
    }

    return status;
}

#endif

/// @} APP

