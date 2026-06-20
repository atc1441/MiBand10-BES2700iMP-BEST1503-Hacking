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
#ifndef __SENSOR_HUB_MINIMA_CORE_H__
#define __SENSOR_HUB_MINIMA_CORE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* app_sensor_hub_minima_init - initialize minima hardware */
void app_sensor_hub_minima_init(void);

/* app_sensor_hub_minima_exit - deinitialize minima hardware */
void app_sensor_hub_minima_exit(void);

/* app_sensor_hub_minima_enable - enable minima hardware;
 * When minima is initialized done, the minima is enabled as default setting;
 * The minima will regulate vcore voltage when system freq is changed;
 */
void app_sensor_hub_minima_enable(void);

/* app_sensor_hub_minima_disable - disable minima hardware;
 * When minima is disabled, it will not regulate vcore voltage when system freq is changed;
 */
void app_sensor_hub_minima_disable(void);

/* app_sensor_hub_minima_set - set system frequency for minima
 * The minima searches the voltage OP(operation point) by freq,
 * then to regulate the voltage.
 * parameter:
 *            freq - system frequency value
 *
 * return: true(success) or false(failed)
 */
bool app_sensor_hub_minima_set_freq(uint8_t freq);

#ifdef __cplusplus
extern "C" {
#endif

#endif /* __SENSOR_HUB_MINIMA_CORE_H__ */

