/***************************************************************************
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
 ***************************************************************************/
#ifndef __USB_MSD_H__
#define __USB_MSD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

enum USB_MSD_API_MODE {
    USB_MSD_API_NONBLOCKING = 0,
    USB_MSD_API_BLOCKING,
};

enum USB_MSD_STATUS {
    USB_MSD_CONFIGURED = 0,
};

typedef void (*USB_MSD_XFER_CALLBACK)(uint8_t *data, uint32_t start_block, uint32_t block_count);
typedef void (*USB_MSD_DEVICE_STATUS_CALLBACK)(uint32_t status, uint32_t param);

int usb_msd_start(void);
int usb_msd_open(USB_MSD_XFER_CALLBACK read_disk, USB_MSD_XFER_CALLBACK write_disk, USB_MSD_DEVICE_STATUS_CALLBACK device_status,
        enum USB_MSD_API_MODE m, uint32_t block_count, uint32_t block_size);

#ifdef __cplusplus
}
#endif

#endif

