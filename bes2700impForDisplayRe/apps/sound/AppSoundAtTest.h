/***************************************************************************
 *
 * @copyright 2015-2023 BES.
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
 * @author wangzhuli
 * @date 2023.02.20         v1.0
 *
 ****************************************************************************/
#ifndef __APP_SOUND_AT_TEST_H__
#define __APP_SOUND_AT_TEST_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void soundAtTestInit(void);

void stDemo(uint8_t cmdLen, uint8_t *cmdParam);
void stBtEnable(uint8_t cmdLen, uint8_t *cmdParam);
void stBtPairing(uint8_t cmdLen, uint8_t *cmdParam);
void stBtDisable(uint8_t cmdLen, uint8_t *cmdParam);
void stEnableMultipoint(uint8_t cmdLen, uint8_t *cmdParam);
void stDisableMultipoint(uint8_t cmdLen, uint8_t *cmdParam);
void stShowPairedDeviceList(uint8_t cmdLen, uint8_t *cmdParam);
void stCleanPairedDeviceList(uint8_t cmdLen, uint8_t *cmdParam);
void stCleanPairedDeviceRecord(uint8_t cmdLen, uint8_t *cmdParam);
void stAutoConnect(uint8_t cmdLen, uint8_t *cmdParam);
void stConnectTws(uint8_t cmdLen, uint8_t *cmdParam);
void stDisconnectAll(uint8_t cmdLen, uint8_t *cmdParam);
void stConnectDevice(uint8_t cmdLen, uint8_t *cmdParam);
void stDisconnectDevice(uint8_t cmdLen, uint8_t *cmdParam);
void stGetRemoteBtName(uint8_t cmdLen, uint8_t *cmdParam);
void stGetAudioChannel(uint8_t cmdLen, uint8_t *cmdParam);

void stTwsCancelConnection(uint8_t cmdLen, uint8_t *cmdParam);
void stTwsNvRoleGet(uint8_t cmdLen, uint8_t *cmdParam);
void stTwsRoleGet(uint8_t cmdLen, uint8_t *cmdParam);
void stTwsRoleSwitch(uint8_t cmdLen, uint8_t *cmdParam);
void stTwsAudchnlSwitch(uint8_t cmdLen, uint8_t *cmdParam);
void stTwsSendData(uint8_t cmdLen, uint8_t *cmdParam);
void stDumpInfo(uint8_t cmdLen, uint8_t *cmdParam);

void stEnterFreeman(uint8_t cmdLen, uint8_t *cmdParam);
void stEnterTws(uint8_t cmdLen, uint8_t *cmdParam);
void stEnterBis(uint8_t cmdLen, uint8_t *cmdParam);

void stShutdown(uint8_t cmdLen, uint8_t *cmdParam);

#ifdef APP_SOUND_UI_ENABLE
void stPowerClick(uint8_t cmdLen, uint8_t *cmdParam);
void stPowerDoubleClick(uint8_t cmdLen, uint8_t *cmdParam);
void stPowerLongPress(uint8_t cmdLen, uint8_t *cmdParam);
void stBtClick(uint8_t cmdLen, uint8_t *cmdParam);
void stBtDoubleClick(uint8_t cmdLen, uint8_t *cmdParam);
void stBtThreeClick(uint8_t cmdLen, uint8_t *cmdParam);
void stBtLongPress(uint8_t cmdLen, uint8_t *cmdParam);
void stWifiClick(uint8_t cmdLen, uint8_t *cmdParam);
void stWifiDoubleClick(uint8_t cmdLen, uint8_t *cmdParam);
void stWifiLongPress(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleClick(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleDoubleClick(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleForward(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleBackward(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleUp(uint8_t cmdLen, uint8_t *cmdParam);
void stCircleDown(uint8_t cmdLen, uint8_t *cmdParam);
void stAuxInInsert(uint8_t cmdLen, uint8_t *cmdParam);
void stAuxInExtract(uint8_t cmdLen, uint8_t *cmdParam);
void stBtAndPowerKey    (uint8_t cmdLen, uint8_t *cmdParam);
void stCircleAndPowerKey     (uint8_t cmdLen, uint8_t *cmdParam);
#endif

#ifdef __cplusplus
}
#endif

#endif /*__APP_SOUND_AT_TEST_H__ */
