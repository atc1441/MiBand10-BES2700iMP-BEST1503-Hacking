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
#ifndef __FLASH_PROGRAMMER_H__
#define __FLASH_PROGRAMMER_H__

#ifdef __cplusplus
extern "C" {
#endif

int get_burn_data_buf(unsigned int *index);

int handle_data_buf(unsigned int index);

int free_data_buf(void);

int data_buf_in_use(void);

int data_buf_in_burn(void);

void set_data_buf_abort_flag(int abort);

void get_burn_verif_buf(unsigned char **buf, unsigned int *size);

void programmer_loop(void);

int send_notif_msg(const void *buf, unsigned char len);

int send_reply(const void *payload, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif

