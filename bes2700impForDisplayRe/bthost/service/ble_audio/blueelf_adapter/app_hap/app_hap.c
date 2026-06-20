/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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
#if BLE_AUDIO_ENABLED
#include "ble_app_dbg.h"
#include "app_hap.h"
#include "app_hap_has_msg.h"
#include "app_hap_hac_msg.h"

/*EXTERNAL FUNCTIONS*/
/*HAC*/
extern int app_hap_hac_init(void);
extern int app_hap_hac_deinit(void);
extern int app_hap_hac_start(uint8_t con_lid);
/*HAS*/
extern int app_hap_has_init(void);
extern int app_hap_has_deinit(void);

/*FUNCTIONS*/
#ifdef AOB_MOBILE_ENABLED
int app_hap_client_init(void)
{
    return app_hap_hac_init();
}

int app_hap_client_deinit(void)
{
    return app_hap_hac_deinit();
}
#endif

int app_hap_server_init(void)
{
    return app_hap_has_init();
}

int app_hap_server_deinit(void)
{
    return app_hap_has_deinit();
}

int app_hap_start(uint8_t con_lid)
{
#ifdef AOB_MOBILE_ENABLED
    return app_hap_hac_start(con_lid);
#endif
    return 0;
}

int app_hap_msg_configure_req(uint8_t cfg)
{
    LOG_I("%s does not support", __func__);
    return 0;
}

#endif
