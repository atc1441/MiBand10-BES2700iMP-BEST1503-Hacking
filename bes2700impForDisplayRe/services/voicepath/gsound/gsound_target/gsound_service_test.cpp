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
#include <cstdarg>
#include <cstdio>
#include <string>
#include <vector>
#include "app_key.h"
#include "gtest/gtest.h"
#include "hal_trace.h"
#include "gsound_custom.h"

class GSoundUtilsTest : public ::testing::Test {
protected:
    GSoundUtilsTest()
    {
    }

    virtual ~GSoundUtilsTest()
    {
    }

    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }
};

TEST_F(GSoundUtilsTest, BtBdAddr)
{
    const uint8_t test_flag = 0x55;
    uint8_t dst_buffer[32];
    uint8_t test_buffer[32];
    uint8_t src_buffer[BD_ADDR_SIZE];
    BT_BD_ADDR src_bes_addr = {0};
    BT_BD_ADDR *dst_bes_addr;

    GSoundBTAddr src_gsound_addr = {0};
    GSoundBTAddr *dst_gsound_addr;

    uint8_t test_addr[] = {// don't use "test_flag" value
                           0x12,
                           0x23,
                           0x69,
                           0xFA,
                           0xA9,
                           0x7B};

    memset(test_buffer, test_flag, sizeof(test_buffer));
    // Setup BES structure with test address
    // Set known values in destination GSound structure
    memcpy(src_bes_addr.addr, test_addr, sizeof(test_addr));
    memset(dst_buffer, test_flag, sizeof(dst_buffer));
    dst_gsound_addr = ( GSoundBTAddr * )dst_buffer;

    gsound_convert_bdaddr_to_gsound(src_bes_addr.addr, dst_gsound_addr);

    EXPECT_EQ(src_bes_addr.addr[0], ((dst_gsound_addr->nap) >> 0) & 0xFF);
    EXPECT_EQ(src_bes_addr.addr[1], ((dst_gsound_addr->nap) >> 8) & 0xFF);
    EXPECT_EQ(src_bes_addr.addr[2], (dst_gsound_addr->uap));
    EXPECT_EQ(src_bes_addr.addr[3], ((dst_gsound_addr->lap >> 0)) & 0xFF);
    EXPECT_EQ(src_bes_addr.addr[4], ((dst_gsound_addr->lap >> 8)) & 0xFF);
    EXPECT_EQ(src_bes_addr.addr[5], ((dst_gsound_addr->lap >> 16)) & 0xFF);
    EXPECT_EQ(memcmp(( const char * )&dst_buffer[sizeof(GSoundBTAddr)],
                     ( const char * )&test_buffer[0],
                     sizeof(dst_buffer) - sizeof(GSoundBTAddr)),
              0);

    src_gsound_addr = *dst_gsound_addr;

    memset(dst_buffer, test_flag, sizeof(dst_buffer));
    dst_bes_addr = ( BT_BD_ADDR * )dst_buffer;

    gsound_convert_bdaddr_to_plateform(&src_gsound_addr, dst_bes_addr->addr);

    EXPECT_EQ(dst_bes_addr->addr[0], ((src_gsound_addr.nap) >> 0) & 0xFF);
    EXPECT_EQ(dst_bes_addr->addr[1], ((src_gsound_addr.nap) >> 8) & 0xFF);
    EXPECT_EQ(dst_bes_addr->addr[2], (src_gsound_addr.uap));
    EXPECT_EQ(dst_bes_addr->addr[3], ((src_gsound_addr.lap >> 0)) & 0xFF);
    EXPECT_EQ(dst_bes_addr->addr[4], ((src_gsound_addr.lap >> 8)) & 0xFF);
    EXPECT_EQ(dst_bes_addr->addr[5], ((src_gsound_addr.lap >> 16)) & 0xFF);
    EXPECT_EQ(memcmp(( const char * )&dst_buffer[sizeof(BT_BD_ADDR)],
                     ( const char * )&test_buffer[0],
                     sizeof(dst_buffer) - sizeof(BT_BD_ADDR)),
              0);
}

TEST_F(GSoundUtilsTest, ResetCrash)
{
    struct CRASH_OUTPUT_BUF_T reset_info;
    GSoundCrashFatalDump gsound_crash_dump;
    GSoundResetReason reset_reason;

    reset_info.hdr.magicCode      = CRASH_DUMP_MAGIC_CODE;
    reset_info.hdr.liteDumpOffset = 10;

    /**********************/
    reset_info.hdr.bufpos = reset_info.hdr.liteDumpOffset +
                            CRASH_DUMP_REGISTERS_NUM_BYTES +
                            CRASH_DUMP_STACK_NUM_BYTES;
    reset_info.buf[reset_info.hdr.liteDumpOffset] =
        0x69;  // write to register location
    reset_info
        .buf[reset_info.hdr.liteDumpOffset + CRASH_DUMP_REGISTERS_NUM_BYTES] =
        0xAB;  // write to stack location

    reset_info.hdr.crashCode = CRASH_DUMP_EXCEPTION_CODE;
    gsound_crash_convert(&gsound_crash_dump, &reset_info);
    EXPECT_EQ(gsound_crash_dump.reason, GSOUND_RESET_TYPE_EXCEPTION);
    EXPECT_EQ(gsound_crash_dump.registers.len, CRASH_DUMP_REGISTERS_NUM_BYTES);
    EXPECT_EQ(gsound_crash_dump.registers.data[0], 0x69);
    EXPECT_EQ(gsound_crash_dump.stack.len, CRASH_DUMP_STACK_NUM_BYTES);
    EXPECT_EQ(gsound_crash_dump.stack.data[0], 0xAB);
    EXPECT_EQ(gsound_crash_dump.panic_file.len, 0);
    EXPECT_EQ(gsound_crash_dump.custom.len, 0);

    gsound_reset_convert(&reset_reason, &reset_info);
    EXPECT_EQ(reset_reason, GSOUND_RESET_TYPE_EXCEPTION);

    /**********************/
    const char *test_string = "hello_crash_dump";
    reset_info.hdr.bufpos =
        reset_info.hdr.liteDumpOffset + strlen(test_string) + 1;
    strcpy(( char * )&reset_info.buf[reset_info.hdr.liteDumpOffset], test_string);

    reset_info.hdr.crashCode = CRASH_DUMP_ASSERT_CODE;
    gsound_crash_convert(&gsound_crash_dump, &reset_info);
    EXPECT_EQ(gsound_crash_dump.reason, GSOUND_RESET_TYPE_ASSERT);
    EXPECT_STREQ(( const char * )gsound_crash_dump.panic_file.data, test_string);
    EXPECT_EQ(gsound_crash_dump.panic_file.len, strlen(test_string) + 1);
    EXPECT_EQ(gsound_crash_dump.panic_file.data[strlen(test_string)], 0);

    gsound_reset_convert(&reset_reason, &reset_info);
    EXPECT_EQ(reset_reason, GSOUND_RESET_TYPE_ASSERT);
}

TEST_F(GSoundUtilsTest, SerialNumber)
{
    uint8_t bes_buffer[30];
    uint8_t gsd_buffer[30];

    bes_buffer[0] = 0x69;
    bes_buffer[1] = 0xA5;
    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsound_util_sn_conv(bes_buffer, 2, gsd_buffer, 5);
    EXPECT_EQ(strcmp(( const char * )gsd_buffer, ( const char * )"69A5"), 0);

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsound_util_sn_conv(bes_buffer, 1, gsd_buffer, 5);
    EXPECT_EQ(strcmp(( const char * )gsd_buffer, ( const char * )"69"), 0);

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsound_util_sn_conv(bes_buffer, 2, gsd_buffer, 4);
    EXPECT_EQ(strcmp(( const char * )gsd_buffer, ( const char * )"69"), 0);

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsd_buffer[0] = 0x95;
    gsound_util_sn_conv(bes_buffer, 2, gsd_buffer, 2);
    EXPECT_EQ(gsd_buffer[0], 0);

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsd_buffer[0] = 0x57;
    gsound_util_sn_conv(bes_buffer, 2, gsd_buffer, 0);
    EXPECT_EQ(gsd_buffer[0], 0x57);
}

TEST_F(GSoundUtilsTest, SerialNumber_BadInputPointers)
{
    uint8_t bes_buffer[30];
    uint8_t gsd_buffer[30];
    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsd_buffer[0] = 0xA6;
    EXPECT_DEATH(gsound_util_sn_conv(nullptr, 2, gsd_buffer, 5), "");

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsd_buffer[0] = 0xA6;
    EXPECT_DEATH(gsound_util_sn_conv(bes_buffer, 2, nullptr, 5), "");

    memset(gsd_buffer, 0, sizeof(gsd_buffer));
    gsd_buffer[0] = 0xA6;
    EXPECT_DEATH(gsound_util_sn_conv(nullptr, 0, nullptr, 0), "");
}

TEST_F(GSoundUtilsTest, KeyToAction_None)
{
    APP_KEY_STATUS status;
    GSoundActionEvents result_action;
    GSoundButtonMask result_button;
    status.event = APP_KEY_EVENT_NONE;
    status.code  = APP_KEY_CODE_GOOGLE;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_ERROR);

    EXPECT_EQ(gsound_util_key_to_action(&status, nullptr),
              GSOUND_STATUS_ERROR);

    EXPECT_EQ(gsound_util_key_to_action(nullptr, &result_action),
              GSOUND_STATUS_ERROR);

    EXPECT_EQ(gsound_util_google_key_to_action(APP_KEY_EVENT_NONE, &result_button),
              GSOUND_STATUS_ERROR);

    EXPECT_EQ(gsound_util_google_key_to_action(APP_KEY_EVENT_NONE, nullptr),
              GSOUND_STATUS_ERROR);

    status.code = APP_KEY_CODE_PWR;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_ERROR);
}

TEST_F(GSoundUtilsTest, KeyClickEvent)
{
    APP_KEY_STATUS status;
    GSoundActionEvents result_action;
    GSoundButtonMask result_button;
    status.event = APP_KEY_EVENT_CLICK;
    status.code  = APP_KEY_CODE_PWR;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_action, GSOUND_TARGET_ACTION_EVENT_TOGGLE_PLAY_PAUSE);

    status.event = APP_KEY_EVENT_UP;
    status.code  = APP_KEY_CODE_PWR;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_action, GSOUND_TARGET_ACTION_EVENT_TOGGLE_PLAY_PAUSE);

    status.event = APP_KEY_EVENT_DOUBLECLICK;
    status.code  = APP_KEY_CODE_PWR;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_action, GSOUND_TARGET_ACTION_EVENT_NEXT_TRACK);

    status.event = APP_KEY_EVENT_LONGPRESS;
    status.code  = APP_KEY_CODE_FN2;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_action, GSOUND_TARGET_ACTION_EVENT_PREV_TRACK);

    status.event = APP_KEY_EVENT_UP;
    status.code  = APP_KEY_CODE_FN2;
    EXPECT_EQ(gsound_util_key_to_action(&status, &result_action),
              GSOUND_STATUS_ERROR);
    EXPECT_EQ(
        gsound_util_google_key_to_action(APP_KEY_EVENT_DOWN, &result_button),
        GSOUND_STATUS_OK);
    EXPECT_EQ(result_button, GSOUND_TARGET_BUTTON_EVENT_DOWN);

    EXPECT_EQ(gsound_util_google_key_to_action(APP_KEY_EVENT_UP, &result_button),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_button, GSOUND_TARGET_BUTTON_EVENT_UP);

    EXPECT_EQ(
        gsound_util_google_key_to_action(APP_KEY_EVENT_LONGPRESS, &result_button),
        GSOUND_STATUS_OK);
    EXPECT_EQ(result_button, GSOUND_TARGET_BUTTON_EVENT_LONG_CLICK);

    EXPECT_EQ(
        gsound_util_google_key_to_action(APP_KEY_EVENT_CLICK, &result_button),
        GSOUND_STATUS_OK);
    EXPECT_EQ(result_button, GSOUND_TARGET_BUTTON_EVENT_CLICK);

    EXPECT_EQ(gsound_util_google_key_to_action(APP_KEY_EVENT_DOUBLECLICK,
                                               &result_button),
              GSOUND_STATUS_OK);
    EXPECT_EQ(result_button, GSOUND_TARGET_BUTTON_EVENT_DOUBLE_CLICK);

    EXPECT_EQ(
        gsound_util_google_key_to_action(APP_KEY_EVENT_NONE, &result_button),
        GSOUND_STATUS_ERROR);
}

TEST_F(GSoundUtilsTest, Percentage)
{
    BesBatteryStatus bbs;
    GSoundBatteryInfo gbs;

    bbs.status = APP_BATTERY_STATUS_NORMAL;
    bbs.level  = APP_BATTERY_LEVEL_MIN;

    gsound_battery_conv(&bbs, &gbs);
    EXPECT_EQ(gbs.percent_full, 10);
    EXPECT_EQ(gbs.charging, false);

    bbs.status = APP_BATTERY_STATUS_CHARGING;
    bbs.level  = APP_BATTERY_LEVEL_MAX;

    gsound_battery_conv(&bbs, &gbs);
    EXPECT_EQ(gbs.percent_full, 100);
    EXPECT_EQ(gbs.charging, true);

    bbs.status = APP_BATTERY_STATUS_OVERVOLT;
    bbs.level  = 3;

    gsound_battery_conv(&bbs, &gbs);
    EXPECT_EQ(gbs.percent_full, 40);
    EXPECT_EQ(gbs.charging, false);

    bbs.status = APP_BATTERY_STATUS_CHARGING;
    bbs.level  = 7;

    gsound_battery_conv(&bbs, &gbs);
    EXPECT_EQ(gbs.percent_full, 80);
    EXPECT_EQ(gbs.charging, true);
}

TEST_F(GSoundUtilsTest, BatteryCompare)
{
    EXPECT_TRUE(gsound_battery_equals_values(
        5, APP_BATTERY_STATUS_NORMAL, 5, APP_BATTERY_STATUS_NORMAL));

    EXPECT_FALSE(gsound_battery_equals_values(
        0, APP_BATTERY_STATUS_CHARGING, 2, APP_BATTERY_STATUS_CHARGING));

    EXPECT_FALSE(gsound_battery_equals_values(
        8, APP_BATTERY_STATUS_NORMAL, 8, APP_BATTERY_STATUS_CHARGING));
}

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
