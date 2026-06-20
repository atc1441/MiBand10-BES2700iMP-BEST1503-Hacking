/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

/*****************************header include********************************/

/*********************external function declaration*************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declaration*************************/

/************************private variable defination************************/

#ifndef __APP_AUDIO_ROUTE_MANAGER_H__
#define __APP_AUDIO_ROUTE_MANAGER_H__

typedef uint32_t (*app_audio_arm_stream_read_callback)(uint8_t *data_ptr, uint32_t data_len);

typedef struct
{
    // 0: player send,  1:device read
    uint8_t data_transfer_mode;
    void (*output_start)(uint32_t sample_rate, uint8_t bits_depth, uint8_t num_channels, uint32_t frame_samples, uint8_t vol);
    void (*output_stop)();
    void (*output_set_volume)(uint8_t vol);
    // data_transfer_mode = 0
    void (*output_data)(uint8_t *data_ptr, uint32_t data_len);
    // data_transfer_mode = 1
    void (*output_receive_ntf)(void);
    void (*output_reg_read_cb)(app_audio_arm_stream_read_callback cb);
} app_audio_arm_output_device_t;

void app_audio_arm_set_output(app_audio_arm_output_device_t *route);

app_audio_arm_output_device_t *app_audio_arm_get_output();

#endif /* __APP_AUDIO_ROUTE_MANAGER_H__ */