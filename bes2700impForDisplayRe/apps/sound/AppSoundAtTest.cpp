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
#include "AppSoundAtTest.h"

#include "SoundApi.h"
#include "SoundDebug.h"
#include "app_trace_rx.h"
#include "cmsis_os.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#ifdef APP_SOUND_UI_ENABLE
#include "app_sound.h"
#endif

#define BD_ADDR_LEN         6
#define STR_ROLE_LEN        1

/**
 * @brief Sound AT test module.
 *
 * The API of this file will start with "st_" that means sound test. And
 * these API should only be used in this module,
 * except "void soundAtTestInit(void)".
 *
 * @note The uart command fomat is "[ST,cmd] or [ST,cmd,para]"
 */
typedef void (*soundAtTestFunc)(uint8_t cmdLen, uint8_t *cmdParam);

typedef struct
{
    const char *string;
    soundAtTestFunc function;
} soundUartHandle_t;

static const soundUartHandle_t sound_uart_handle[] = {
    {"test",                    stDemo},
#ifndef APP_SOUND_UI_ENABLE
    {"btEnable",                stBtEnable},
    {"btPairing",               stBtPairing},
    {"btDisable",               stBtDisable},

    {"enableMultipoint",        stEnableMultipoint},
    {"disableMultipoint",       stDisableMultipoint},
    {"showRecord",              stShowPairedDeviceList},
    {"cleanAllRecord",          stCleanPairedDeviceList},
    {"cleanRecord",             stCleanPairedDeviceRecord},
    {"autoConnect",             stAutoConnect},
    {"connect",                 stConnectDevice},
    {"connectTws",              stConnectTws},
    {"disconnect",              stDisconnectDevice},
    {"disconnectAll",           stDisconnectAll},
    {"getBtName",               stGetRemoteBtName},
    {"getAudioChannel",         stGetAudioChannel},

    {"twsPageCancel",           stTwsCancelConnection},
    {"getTwsNvRole",            stTwsNvRoleGet},
    {"getTwsRole",              stTwsRoleGet},
    {"twsRoleSwitch",           stTwsRoleSwitch},
    {"twsAudchnlSwitch",        stTwsAudchnlSwitch},
    {"twsSendData",             stTwsSendData},
    {"dumpInfo",                stDumpInfo},

    {"enterFreeman",            stEnterFreeman},
    {"enterTws",                stEnterTws},
    {"enterBis",                stEnterBis},

    {"shutdown",                stShutdown},
#else
    {"power_click",             stPowerClick},
    {"power_double_click",      stPowerDoubleClick},
    {"power_long_press",        stPowerLongPress},
    {"bt_click",                stBtClick},
    {"bt_double_click",         stBtDoubleClick},
    {"bt_three_click",          stBtThreeClick},
    {"bt_long_press",           stBtLongPress},
    {"wifi_click",              stWifiClick},
    {"wifi_double_click",       stWifiDoubleClick},
    {"wifi_long_press",         stWifiLongPress},
    {"circle_click",            stCircleClick},
    {"circle_double_click",     stCircleDoubleClick},
    {"circle_left",             stCircleForward},
    {"circle_right",            stCircleBackward},
    {"circle_up",               stCircleUp},
    {"circle_down",             stCircleDown},
    {"aux_in_insert",           stAuxInInsert},
    {"aux_in_extract",          stAuxInExtract},
    {"bt_and_power_key",        stBtAndPowerKey},
    {"circle_and_power_key",    stCircleAndPowerKey},
#endif
    /// Add AT command
};

soundAtTestFunc sound_test_find_uart_handle(unsigned char *buf)
{
    soundAtTestFunc p = NULL;

    for (unsigned int i = 0; i < ARRAY_SIZE(sound_uart_handle); i++) {
        const char *str = sound_uart_handle[i].string;

        if (strlen(str) != strlen((char *)buf)) {
            continue;
        }

        if (strncmp((char *)buf, str, strlen(str)) == 0 || strstr(str, (char *)buf)) {
            LOG_I("AT:%s", str);
            p = sound_uart_handle[i].function;
            break;
        }
    }

    return p;
}

int soundUartCmdHandler(unsigned char *buf, unsigned int length)
{
    int ret = 0;
    unsigned char *buf_param = NULL;
    uint8_t len = 0;
    buf_param = (unsigned char *)strstr((char *)buf, (char *)",");

    if (buf_param != NULL) {
        *buf_param = '\0';
        len = length - (buf_param - buf) - 1;
        buf_param++;
    }

    soundAtTestFunc handl_function = sound_test_find_uart_handle(buf);
    if (handl_function) {
        handl_function(len, buf_param);
    } else {
        ret = -1;
        LOG_I("can not find handle function");
    }

    return ret;
}

unsigned int soundUartCmdCallback(unsigned char *cmd, unsigned int cmd_length)
{
    POSSIBLY_UNUSED int param_len = 0;
    char *cmd_param = NULL;
    char *cmd_end = (char *)cmd + cmd_length;

    cmd_param = strstr((char *)cmd, (char *)"|");

    if (cmd_param) {
        *cmd_param = '\0';
        cmd_length = cmd_param - (char *)cmd;
        cmd_param += 1;

        param_len = cmd_end - cmd_param;
    }

    if (soundUartCmdHandler((unsigned char *)cmd, strlen((char *)cmd)) == 0) {
    }
    return 0;
}

void soundAtTestInit(void)
{
#ifdef UTILS_ESHELL_EN
    LOG_I("AT by eshell...");
    return;
#endif
#ifdef APP_TRACE_RX_ENABLE
    LOG_I("AT Test Init...");
    app_trace_rx_register("ST", soundUartCmdCallback);
#endif
}

static bool char2hex(char ch, uint8_t *hex)
{
    if ((ch >= '0') && (ch <= '9')) {
        *hex = ch - '0';
    } else if ((ch >= 'a') && (ch <= 'f')) {
        *hex = ch - 'a' + 10;
    } else if ((ch >= 'A') && (ch <= 'F')) {
        *hex = ch - 'A' + 10;
    } else {
        return false;
    }
    return true;
}

static bool str2addr(char *str, uint8_t *addr)
{
    if (strlen(str) != BD_ADDR_LEN*2) {
        return false;
    }

    for (int i = 0; i < BD_ADDR_LEN; i++) {
        uint8_t h, l;
        if (!char2hex(str[i * 2], &h)) {
            return false;
        }
        if (!char2hex(str[i * 2 + 1], &l)) {
            return false;
        }
        addr[i] = (h << 4) | l;
    }

    return true;
}

static bool str2role(char *str, SoundTWSRole *role)
{
    if (strlen(str) != STR_ROLE_LEN) {
        return false;
    }

    if (str[0] == 'm' || str[0] == 'M') {
        *role = RoleMaster;
    } else if (str[0] == 's' || str[0] == 'S') {
        *role = RoleSlave;
    } else {
        return false;
    }

    return true;
}

static bool str2side(char *str, SoundTWSSide *side)
{
    if (strlen(str) != STR_ROLE_LEN) {
        return false;
    }

    if (str[0] == 'r' || str[0] == 'R') {
        *side = SideRight;
    } else if (str[0] == 'l' || str[0] == 'L') {
        *side = SideLeft;
    } else {
        return false;
    }

    return true;
}

static bool twsInfoPhrase(char *buf, uint8_t *addr, SoundTWSRole *role, SoundTWSSide *side, uint8_t *name)
{
    const char s[2] = ",";
    char *token;
    LOG_I("len=%d, str:%s", strlen(buf), buf);

    token = strtok(buf, s);
    LOG_I("Token:[0]->%s", token);
    if (token == NULL || !str2addr(token, addr)) {
        return false;
    }

    token = strtok(NULL, s);
    LOG_I("Token:[1]->%s", token);
    if (token == NULL || !str2role(token, role)) {
        return false;
    }

    token = strtok(NULL, s);
    LOG_I("Token:[2]->%s", token);
    if (token == NULL || !str2side(token, side)) {
        return false;
    }

    token = strtok(NULL, s);
    LOG_I("Token:[3]->%s", token);
    if (token == NULL) {
        return false;
    }
    strcpy((char *)name, token);

    return true;
}

void stDemo(uint8_t cmdLen, uint8_t *cmdParam)
{
    char str[64];
    LOG_I("this is uart test handle");
    memcpy(str, cmdParam, cmdLen);
    str[cmdLen] = '\0';
    LOG_I("len=%d, str:%s", cmdLen, str);
}

void stBtEnable(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtEnable();
}

void stBtPairing(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtPairing();
}

void stBtDisable(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtDisable();
}

void stEnableMultipoint(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundEnableMultipoint();
}

void stDisableMultipoint(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundDisableMultipoint();
}

void stShowPairedDeviceList(uint8_t cmdLen, uint8_t *cmdParam)
{
    bt_bdaddr_t device_list[10];
    uint8_t count = 0;
    soundBtGetPairedDeviceList(device_list, 10, &count);

    LOG_I("record count=%d", count);

    for (int i = 0; i < count; i++) {
        uint8_t *p = device_list[i].address;
        LOG_I("device[%d]: %02x:%02x:%02x:%02x:%02x:%02x", i,
              p[0], p[1], p[2], p[3], p[4], p[5]);
    }
}

void stCleanPairedDeviceList(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtCleanPairedDeviceList();
}

void stCleanPairedDeviceRecord(uint8_t cmdLen, uint8_t *cmdParam)
{
    char str[64];
    char s[2] = ",";
    char *token;
    bt_bdaddr_t addr;
    memcpy(str, cmdParam, cmdLen);
    str[cmdLen] = '\0';

    token = strtok(str, s);
    LOG_I("Token:[0]->%s", token);
    if (token == NULL || !str2addr(token, addr.address)) {
        LOG_W("param error, please check");
        return ;
    }

    soundBtCleanPairedDeviceRecord(&addr);
}

void stAutoConnect(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtAutoConnect();
}

void stConnectTws(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtConnectTws();
}

void stDisconnectAll(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundBtDisconnectAll();
}

void stConnectDevice(uint8_t cmdLen, uint8_t *cmdParam)
{
    char str[64];
    char s[2] = ",";
    char *token;
    bt_bdaddr_t addr;
    memcpy(str, cmdParam, cmdLen);
    str[cmdLen] = '\0';

    token = strtok(str, s);
    LOG_I("Token:[0]->%s", token);
    if (token == NULL || !str2addr(token, addr.address)) {
        LOG_W("param error, please check");
        return ;
    }

    soundBtConnectDevice(&addr);
}

void stDisconnectDevice(uint8_t cmdLen, uint8_t *cmdParam)
{
    char str[64];
    char s[2] = ",";
    char *token;
    bt_bdaddr_t addr;
    memcpy(str, cmdParam, cmdLen);
    str[cmdLen] = '\0';

    token = strtok(str, s);
    LOG_I("Token:[0]->%s", token);
    if (token == NULL || !str2addr(token, addr.address)) {
        LOG_W("param error, please check");
        return ;
    }
    soundBtDisconnectDevice(&addr);
}

void stGetRemoteBtName(uint8_t cmdLen, uint8_t *cmdParam)
{
    bt_bdaddr_t list[10];
    char nameStr[32];
    uint8_t num = 0;
    uint8_t nameLen = 0;
    bool ret = false;

    ret = soundBtGetPairedDeviceList(list, 10, &num);

    if (!ret) {
        LOG_I("AtTest:Failed to get paired list");
        return;
    }

    for (int i = 0; i < num; i++) {
        if (soundGetRemoteBtName(&list[i], nameStr, &nameLen)) {
            uint8_t *p = list[i].address;
            LOG_I("AtTest:Device[%d]:", i);
            LOG_I("AtTest:BtAddr=%02x:%02x:%02x:%02x:%02x:%02x",
                  p[0], p[1], p[2], p[3], p[4], p[5]);
            LOG_I("AtTest:BtName=%s", nameStr);
        }
    }
}

void stGetAudioChannel(uint8_t cmdLen, uint8_t *cmdParam)
{
    SoundAudioChnl chnl = soundAudioChannelGet();
    LOG_I("AtTest:Audio Channel:%d", chnl);
}

void stTwsCancelConnection(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundTwsCancelConnection();
}

void stTwsNvRoleGet(uint8_t cmdLen, uint8_t *cmdParam)
{
    SoundTWSNvRole nvRole = soundTwsNvRoleGet();
    LOG_I("AtTest:TWS NV ROLE:%d", nvRole);
}

void stTwsRoleGet(uint8_t cmdLen, uint8_t *cmdParam)
{
    SoundTWSRole twsRole = soundTwsRoleGet();
    LOG_I("AtTest:TWS ROLE:%d", twsRole);
}

void stTwsRoleSwitch(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundTwsRoleSwitch();
}

void stTwsAudchnlSwitch(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundTwsAudioChannelSwitch();
}

void stTwsSendData(uint8_t cmdLen, uint8_t *cmdParam)
{
    char name[] = "*HELLO-WORLD*";
    soundTwsSendData((uint8_t *)name, strlen(name) + 1);
}

void stDumpInfo(uint8_t cmdLen, uint8_t *cmdParam)
{
    SoundMode m = soundGetCurrMode();
    SoundBisRole br = soundGetCurrBisRole();

    LOG_I("AT:INFO:MODE=%s", soundMode2str(m));
    LOG_I("AT:INFO:BIS_ROLE=%s", soundBisRole2str(br));
}

void stEnterFreeman(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundEnterFreeman();
}

void stEnterTws(uint8_t cmdLen, uint8_t *cmdParam)
{
    char str[64];
    memcpy(str, cmdParam, cmdLen);
    str[cmdLen] = '\0';

    SoundTWSInfo info;

    if (twsInfoPhrase(str, info.btaddr.address, &info.role, &info.side, info.nameStr)) {
        LOG_I("AT:Update tws info");
        info.nameLen = strlen((char *)info.nameStr);
        soundEnterTws(&info);
    }
}

void stEnterBis(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundEnterBis();
}

void stShutdown(uint8_t cmdLen, uint8_t *cmdParam)
{
    soundShutdown();
}

#ifdef APP_SOUND_UI_ENABLE
void stPowerClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_POWER_KEY_CLICK);
}

void stPowerDoubleClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_POWER_KEY_DOUBLE_CLICK);
}

void stPowerLongPress(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_POWER_KEY_LONG_PRESS);
}

void stBtClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_BT_KEY_CLICK);
}

void stBtDoubleClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_BT_KEY_DOUBLE_CLICK);
}

void stBtThreeClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_BT_KEY_THREE_CLICK);
}

void stBtLongPress(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_BT_KEY_LONG_PRESS);
}

void stWifiClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_WIFI_KEY_CLICK);
}

void stWifiDoubleClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_WIFI_KEY_DOUBLE_CLICK);
}

void stWifiLongPress(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_WIFI_KEY_LONG_PRESS);
}

void stCircleClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    LOG_I("app_sound_key_handle: stCircleClick !");
    app_sound_key_handle(SOUND_CIRCLE_KEY_CLICK);
}

void stCircleDoubleClick(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_KEY_DOUBLE_CLICK);
}

void stCircleForward(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_LEFT_KEY_CLICK);
}

void stCircleBackward(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_RIGHT_KEY_CLICK);
}

void stCircleUp(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_UP_KEY_CLICK);
}

void stCircleDown(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_DOWN_KEY_CLICK);
}

void stAuxInInsert(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_AUX_IN_INSERT);
}

void stAuxInExtract(uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_AUX_IN_EXTRACT);
}

void stBtAndPowerKey    (uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_BT_AND_POWER);
}

void stCircleAndPowerKey     (uint8_t cmdLen, uint8_t *cmdParam)
{
    app_sound_key_handle(SOUND_CIRCLE_AND_POWER);
}

#endif

#ifdef UTILS_ESHELL_EN
#include "eshell.h"
#ifndef APP_SOUND_UI_ENABLE
static void esh_bt_enable(int argc, char *argv[])
{
    eshell_putstring("receive bt enable cmd");
    stBtEnable(argc,(uint8_t *)argv);
}

static void esh_bt_pairing(int argc, char *argv[])
{
    eshell_putstring("receive be pair cmd");
    stBtPairing(argc,(uint8_t *)argv);
}

static void esh_bt_disable(int argc, char *argv[])
{
    eshell_putstring("receive bt disable cmd");
    stBtDisable(argc,(uint8_t *)argv);
}

static void esh_bt_enterFreeman(int argc, char *argv[])
{
    eshell_putstring("receive enter freeman cmd");
    stEnterFreeman(argc,(uint8_t *)argv);
}

static void esh_bt_enterTws(int argc, char *argv[])
{
    SoundTWSInfo info;
    if (argc >= 4) {
        eshell_putstring("receive enter tws cmd");
        str2addr(argv[1], info.btaddr.address);
        if (strcmp(argv[2], "M") == 0) {
            info.role = RoleMaster;
        } else {
            info.role = RoleSlave;
        }
        if (strcmp(argv[3], "R") == 0) {
            info.side = SideRight;
        } else {
            info.side = SideLeft;
        }
        eshell_putstring("nv role=%d, side=%d", info.role, info.side);
        strcpy((char *)info.nameStr, argv[4]);
        info.nameLen = strlen((char *)info.nameStr);
        soundEnterTws(&info);
    } else {
        eshell_putstring("cmd fail: need more args");
    }
}

static void esh_bt_enterBis(int argc, char *argv[])
{
    eshell_putstring("receive enter bis cmd");
    stEnterBis(argc,(uint8_t *)argv);
}

static void esh_bt_multipoint(int argc, char *argv[])
{
    if (argc < 1) {
        eshell_putstring("cmd fail: need more args");
        return;
    }

    if (strcmp(argv[1], "on") == 0) {
        stEnableMultipoint(argc, (uint8_t *)argv);
    } else if (strcmp(argv[1], "off") == 0) {
        stDisableMultipoint(argc, (uint8_t *)argv);
    } else {
        eshell_putstring("cmd fail: para error please input [on] or [off]");
    }
}

static void esh_show_paired_list(int argc, char *argv[])
{
    stShowPairedDeviceList(argc, (uint8_t *)argv);
}

static void esh_clean_paired_list(int argc, char *argv[])
{
    stCleanPairedDeviceList(argc, (uint8_t *)argv);
}

static void esh_clean_paired_record(int argc, char *argv[])
{
    bt_bdaddr_t addr;

    if (argc >= 1) {
        eshell_putstring("receive enter tws cmd");
        if (str2addr(argv[1], addr.address)) {
            soundBtCleanPairedDeviceRecord(&addr);
        } else {
            eshell_putstring("cmd fail: address is invalid");
        }
    } else {
        eshell_putstring("cmd fail: need more args");
    }
}

static void esh_bt_autoConnect(int argc, char *argv[])
{
    stAutoConnect(argc,(uint8_t *)argv);
}

static void esh_bt_connectTws(int argc, char *argv[])
{
    stConnectTws(argc,(uint8_t *)argv);
}

static void esh_bt_connectMob(int argc, char *argv[])
{
    bt_bdaddr_t addr;

    if (argc >= 1) {
        eshell_putstring("receive enter tws cmd");
        if (str2addr(argv[1], addr.address)) {
            soundBtConnectDevice(&addr);
        } else {
            eshell_putstring("cmd fail: address is invalid");
        }
    } else {
        eshell_putstring("cmd fail: need more args");
    }
}

static void esh_bt_disconnectMob(int argc, char *argv[])
{
    bt_bdaddr_t addr;

    if (argc >= 1) {
        eshell_putstring("receive enter tws cmd");
        if (str2addr(argv[1], addr.address)) {
            soundBtDisconnectDevice(&addr);
        } else {
            eshell_putstring("cmd fail: address is invalid");
        }
    } else {
        eshell_putstring("cmd fail: need more args");
    }
}

static void esh_bt_disconnectAll(int argc, char *argv[])
{
    stDisconnectAll(argc,(uint8_t *)argv);
}

static void esh_show_audchnl(int argc, char *argv[])
{
    stGetAudioChannel(argc,(uint8_t *)argv);
}

static void esh_bt_cancel_tws_connection(int argc, char *argv[])
{
    stTwsCancelConnection(argc,(uint8_t *)argv);
}

static void esh_show_tws_nv_role(int argc, char *argv[])
{
    stTwsNvRoleGet(argc,(uint8_t *)argv);
}

static void esh_show_tws_role(int argc, char *argv[])
{
    stTwsRoleGet(argc,(uint8_t *)argv);
}

static void esh_tws_role_switch(int argc, char *argv[])
{
    stTwsRoleSwitch(argc,(uint8_t *)argv);
}

static void esh_tws_audchnl_switch(int argc, char *argv[])
{
    stTwsAudchnlSwitch(argc,(uint8_t *)argv);
}

static void esh_tws_send_data(int argc, char *argv[])
{
    stTwsSendData(argc,(uint8_t *)argv);
}

static void esh_dump_info(int argc, char *argv[])
{
    stDumpInfo(argc,(uint8_t *)argv);
}

static void esh_shutdown(int argc, char *argv[])
{
    stShutdown(argc, (uint8_t *)argv);
}

#else
static void esh_stPowerClick(int argc, char *argv[])
{
    stPowerClick(argc, (uint8_t *)argv);
}

static void esh_stPowerDoubleClick(int argc, char *argv[])
{
    stPowerDoubleClick(argc, (uint8_t *)argv);
}

static void esh_stPowerLongPress(int argc, char *argv[])
{
    stPowerLongPress(argc, (uint8_t *)argv);
}

static void esh_stBtClick(int argc, char *argv[])
{
    stBtClick(argc, (uint8_t *)argv);
}

static void esh_stBtDoubleClick(int argc, char *argv[])
{
    stBtDoubleClick(argc, (uint8_t *)argv);
}

static void esh_stBtThreeClick(int argc, char *argv[])
{
    stBtThreeClick(argc, (uint8_t *)argv);
}

static void esh_stBtLongPress(int argc, char *argv[])
{
    stBtLongPress(argc, (uint8_t *)argv);
}

static void esh_stWifiClick(int argc, char *argv[])
{
    stWifiClick(argc, (uint8_t *)argv);
}

static void esh_stWifiDoubleClick(int argc, char *argv[])
{
    stWifiDoubleClick(argc, (uint8_t *)argv);
}

static void esh_stWifiLongPress(int argc, char *argv[])
{
    stWifiLongPress(argc, (uint8_t *)argv);
}

static void esh_stCircleClick(int argc, char *argv[])
{
    stCircleClick(argc, (uint8_t *)argv);
}

static void esh_stCircleDoubleClick(int argc, char *argv[])
{
    stCircleDoubleClick(argc, (uint8_t *)argv);
}

static void esh_stCircleForward(int argc, char *argv[])
{
    stCircleForward(argc, (uint8_t *)argv);
}

static void esh_stCircleBackward(int argc, char *argv[])
{
    stCircleBackward(argc, (uint8_t *)argv);
}

static void esh_stCircleUp(int argc, char *argv[])
{
    stCircleUp(argc, (uint8_t *)argv);
}

static void esh_stCircleDown(int argc, char *argv[])
{
    stCircleDown(argc, (uint8_t *)argv);
}

static void esh_stAuxInInsert(int argc, char *argv[])
{
    stAuxInInsert(argc, (uint8_t *)argv);
}

static void esh_stAuxInExtract(int argc, char *argv[])
{
    stAuxInExtract(argc, (uint8_t *)argv);
}

static void esh_stBtAndPowerKey(int argc, char *argv[])
{
    stBtAndPowerKey(argc, (uint8_t *)argv);
}

static void esh_stCircleAndPowerKey(int argc, char *argv[])
{
    stCircleAndPowerKey(argc, (uint8_t *)argv);
}
#endif

#ifndef APP_SOUND_UI_ENABLE
#define BT_ENABLE_HELP          "enable bt, no arg"
#define BT_PAIRING_HELP         "enter bt pair, no arg"
#define BT_DISABLE_HELP         "disable bt, no arg"
#define BT_ENTER_FREEMAN_HELP   "enter freeman mode, no arg"
#define BT_ENTER_TWS_HELP       "args:bt_addr bt_role(M or S) tws_side bt_name, example: bt_enterTws 6c3333227c6d M R ali_master"
#define BT_ENTER_BIS_HELP       "enter bis mode, no arg"
#define BT_MULTIPOINT_HELP      "args:on/off, example: bt_multipoint on"
#define BT_SHOW_PAIRED_HELP     "show paired record"
#define BT_CLEAN_ALL_PAIRED_HELP "clean all paired record, no bt connection is needed"
#define BT_CLEAN_PAIRED_HELP    "args:bt_addr, example: bt_cleanRecord 62baebe357a0"
#define BT_AUTOCONNECT_HELP     "auto connect TWS and mobiles"
#define BT_CONNECTTWS_HELP      "connect TWS"
#define BT_CONNECT_HELP         "args:bt_addr, example: bt_connect 62baebe357a0"
#define BT_DISCONNECT_HELP      "args:bt_addr, example: bt_disconnect 62baebe357a0"
#define BT_DISCONNECTALL_HELP   "disconnect all connections"
#define BT_SHOW_AUDCHNL_HELP    "show current audio channel"
#define BT_CANCEL_TWS_HELP      "cancel tws connection"
#define BT_SHOW_NV_ROLE_HELP    "show tws nv role"
#define BT_SHOW_TWS_ROLE_HELP   "show tws role"
#define BT_TWS_ROLE_SWITCH_HELP "tws role switch"
#define BT_TWS_AUDCHNL_SWITCH_HELP "tws audio channel switch"
#define BT_TWS_SEND_DATA_HELP   "tws send data"
#define BT_DUMP_INFO_HELP       "dump sound info"
#define BT_SHUTDOWN_HELP        "disconnect links in turn then call system shutdown"
#else
#define POWER_KEY_CLICK_HELP           "click power key"
#define POWER_KEY_DOUBLE_CLICK_HELP    "double click power key"
#define POWER_KEY_LONG_PRESS_HELP      "long press power key"
#define BT_KEY_CLICK_HELP              "click bt key"
#define BT_KEY_DOUBLE_CLICK_HELP       "double click bt key"
#define BT_KEY_THREE_CLICK_HELP        "three click bt key"
#define BT_KEY_LONG_PRESS_HELP         "long press bt key"
#define WIFI_KEY_CLICK_HELP            "click wifi key"
#define WIFI_KEY_DOUBLE_CLICK_HELP     "double click wifi key"
#define WIFI_KEY_LONG_PRESS_HELP       "long press wifi key"
#define CIRCLE_KEY_CLICK_HELP          "click circle key"
#define CIRCLE_KEY_DOUBLE_CLICK_HELP   "double click circle key"
#define CIRCLE_KEY_FORWARD_HELP        "click forward of circle  key"
#define CIRCLE_KEY_BACKWARD_HELP       "click backward of circle key"
#define CIRCLE_KEY_UP_HELP             "click up of circle key"
#define CIRCLE_KEY_DOWN_HELP           "click down of circle key"
#define AUX_IN_INSERT_HELP             "aux input linein or usb insert"
#define AUX_IN_EXTRACT_HELP            "aux input linein or usb extract"
#define BT_AND_POWER_HELP              "bt and power key press"
#define CIRCLE_AND_POWER_HELP          "circle and power key press"
#endif

#ifndef APP_SOUND_UI_ENABLE
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_enable",       BT_ENABLE_HELP,         esh_bt_enable);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_pairing",      BT_PAIRING_HELP,        esh_bt_pairing);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_disable",      BT_DISABLE_HELP,        esh_bt_disable);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_enterFreeman", BT_ENTER_FREEMAN_HELP,  esh_bt_enterFreeman);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_enterTws",     BT_ENTER_TWS_HELP,      esh_bt_enterTws);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_enterBis",     BT_ENTER_BIS_HELP,      esh_bt_enterBis);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_multipoint",   BT_MULTIPOINT_HELP,     esh_bt_multipoint);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_showRecord",   BT_SHOW_PAIRED_HELP,    esh_show_paired_list);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_cleanAllRecord",  BT_CLEAN_ALL_PAIRED_HELP,   esh_clean_paired_list);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_cleanRecord",  BT_CLEAN_PAIRED_HELP,   esh_clean_paired_record);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_autoConnect",  BT_AUTOCONNECT_HELP,    esh_bt_autoConnect);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connectTws",   BT_CONNECTTWS_HELP,     esh_bt_connectTws);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_connectMob",   BT_CONNECT_HELP,        esh_bt_connectMob);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_disconnectMob",BT_DISCONNECT_HELP,     esh_bt_disconnectMob);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_disconnectAll",BT_DISCONNECTALL_HELP,  esh_bt_disconnectAll);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "show_audchnl",    BT_SHOW_AUDCHNL_HELP,   esh_show_audchnl);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "cancel_tws",      BT_CANCEL_TWS_HELP,     esh_bt_cancel_tws_connection);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "show_nvrole",     BT_SHOW_NV_ROLE_HELP,   esh_show_tws_nv_role);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "show_twsrole",    BT_SHOW_TWS_ROLE_HELP,  esh_show_tws_role);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "twsrole_switch",  BT_TWS_ROLE_SWITCH_HELP,esh_tws_role_switch);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "twsaudchnl_switch",BT_TWS_AUDCHNL_SWITCH_HELP,esh_tws_audchnl_switch);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "tws_send_data",   BT_TWS_SEND_DATA_HELP,  esh_tws_send_data);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_dump_info",    BT_DUMP_INFO_HELP,      esh_dump_info);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_shutdown",     BT_SHUTDOWN_HELP,       esh_shutdown);
#else
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "power_click",     POWER_KEY_CLICK_HELP,                     esh_stPowerClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "power_double_click",     POWER_KEY_DOUBLE_CLICK_HELP,       esh_stPowerDoubleClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "power_long_press",     POWER_KEY_LONG_PRESS_HELP,           esh_stPowerLongPress);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_click",     BT_KEY_CLICK_HELP,                           esh_stBtClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_double_click",     BT_KEY_DOUBLE_CLICK_HELP,             esh_stBtDoubleClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_three_click",     BT_KEY_THREE_CLICK_HELP,               esh_stBtThreeClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_long_press",     BT_KEY_LONG_PRESS_HELP,                 esh_stBtLongPress);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "wifi_click",     WIFI_KEY_CLICK_HELP,                       esh_stWifiClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "wifi_double_click",     WIFI_KEY_DOUBLE_CLICK_HELP,         esh_stWifiDoubleClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "wifi_long_press",     WIFI_KEY_LONG_PRESS_HELP,             esh_stWifiLongPress);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_click",     CIRCLE_KEY_CLICK_HELP,                   esh_stCircleClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_double_click",     CIRCLE_KEY_DOUBLE_CLICK_HELP,     esh_stCircleDoubleClick);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_forward",     CIRCLE_KEY_FORWARD_HELP,               esh_stCircleForward);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_backward",     CIRCLE_KEY_BACKWARD_HELP,             esh_stCircleBackward);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_up",     CIRCLE_KEY_UP_HELP,                         esh_stCircleUp);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_down",     CIRCLE_KEY_DOWN_HELP,                     esh_stCircleDown);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "aux_in_insert",     AUX_IN_INSERT_HELP,                     esh_stAuxInInsert);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "aux_in_extract",     AUX_IN_EXTRACT_HELP,                   esh_stAuxInExtract);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "bt_and_power_key",     BT_AND_POWER_HELP,                   esh_stBtAndPowerKey);
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_BT_COMMON, "circle_and_power_key",     CIRCLE_AND_POWER_HELP,           esh_stCircleAndPowerKey);
#endif
#endif

INIT_APP_SOUND_EXPORT(soundAtTestInit);
