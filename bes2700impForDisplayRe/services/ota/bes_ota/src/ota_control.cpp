#include "hal_iomux.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#include "hal_bootmode.h"
#include "hal_norflash.h"
#include "hal_cmu.h"
#include "pmu.h"
#include "crc32_c.h"
#include "cmsis_nvic.h"
#include "string.h"
#include "ota_control.h"
#include "hal_cache.h"
#include "factory_section.h"
#include "bt_drv_interface.h"
#include "ota_spp.h"
#include "ota_dbg.h"
#include "hal_wdt.h"
#include "norflash_api.h"
#include "app_flash_api.h"
#include "nvrecord_bt.h"
#include "factory_section.h"
#include "bluetooth_bt_api.h"
#include "apps.h"
#include "app_utils.h"
#include "app_bt.h"
#include "hal_aud.h"
#include "btapp.h"
#include "watchdog.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#include "app_ble.h"
#ifdef OTA_OVER_TOTA_ENABLED
#include "app_tota.h"
#endif
#endif

#ifdef IBRT
#include "app_tws_ctrl_thread.h"
#include "app_ibrt_ota_cmd.h"
#include "app_tws_ibrt_cmd_handler.h"
#include "app_ibrt_internal.h"
#endif

#ifdef COMBO_CUSBIN_IN_FLASH
#include "nvrecord_combo.h"
#endif

#ifdef AI_OTA
#include "ota_common.h"
#endif

#include "ota_basic.h"

#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#ifdef OTA_OVER_TOTA_ENABLED
#include "app_tota.h"
#endif
extern "C" uint8_t tota_ble_get_conidx(void);
extern "C" uint8_t ota_ble_get_conidx(void);
#endif

#define TO_STR_A(s)                     # s
#define TO_STR(s)                       TO_STR_A(s)

#define DelayMs(a)      hal_sys_timer_delay(MS_TO_TICKS(a))

#define OTA_CONTROL_NORFLASH_BUFFER_LEN    (FLASH_SECTOR_SIZE_IN_BYTES*2)
#define OTA_ROLE_INVALID 0xFF

#define CASES(prefix, item) \
    case prefix##item:      \
        str = #item;        \
        break;

#if PUYA_FLASH_ERASE_LIMIT
static uint8_t upsize_segment_cnt = 0;
static uint8_t upsize_erase_cnt = 0;
#define UPSIZE_ELEMENT_LIMIT    4           // No more than UPSIZE_ELEMENT_LIMIT+1 Writes after a Erase
#define UPSIZE_WRITE_FREQ       10          // Write once every UPSIZE_WRITE_FREQ usual upgrade
#endif

#define PING_WD_PERIOD_TICKS (5000)
#define OTA_APP_PACKET_INTERVAL_FOR_I6_BELOW    (80)    //app default packet sending interval(ms)
#define OTA_I6_BELOW_DEFAULT_MTU                (185)

extern uint32_t __ota_upgrade_log_start[];

extern OtaContext ota;
#ifdef COMBO_CUSBIN_IN_FLASH
#define COMBO_BIN_HEADER_TOTAL_LEN      256
#define COMBO_BIN_HEADER_OFFSET_OF_INFO (sizeof(uint32_t)*3)    //NV_COMBO_IMAGE_INFO_T:sizeof(mainInfo)+sizeof(version)+sizeof(contentNum)
void _combo_bin_erase_segment(uint32_t addr, uint32_t length);
#endif
static uint8_t ota_role_switch_req_timer_num = 1;
static bool ota_get_pkt_in_role_switch = false;
#define LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC    512
static const char* image_info_sanity_crc_key_word = "CRC32_OF_IMAGE=0x";

FLASH_OTA_UPGRADE_LOG_FLASH_T *otaUpgradeLogInFlash = (FLASH_OTA_UPGRADE_LOG_FLASH_T *)__ota_upgrade_log_start;
#define otaUpgradeLog   (*otaUpgradeLogInFlash)
FLASH_OTA_BOOT_INFO_T otaBootInfoInFlash = { NORMAL_BOOT, 0, 0, 0, BOOT_WORD_A};
#define HW_VERSION_LENTH        4    //sizeof(FIRMWARE_REV_INFO_T)
#define WHOLE_CRC_DEFAULT_INITIAL_VALUE  0xffffffff
uint8_t peerFwRevInfo[HW_VERSION_LENTH] = {0};

static uint8_t otaVersion[OTA_VERSION_INFO_LEN] = {0,0,0,1}; //!< current OTA version is 0.0.0.1

#ifdef IBRT
static OTA_BASIC_TWS_ROLE_E ibrt_tws_mode = OTA_BASIC_TWS_INVALID_ROLE;
#endif

OTA_IBRT_TWS_CMD_EXECUTED_RESULT_FROM_SLAVE_T receivedResultAlreadyProcessedBySlave = {0xFF, 0, 0, 0};
static CQueue otaCmdQueue;
static uint8_t accumulatedOtaBuf[BES_OTA_BT_DATA_PACKET_MAX_SIZE * 2];    //use BufSPP to store otaBuf_Data
static uint8_t isInBesOtaState = false;
extern "C" void system_get_info(uint8_t *fw_rev_0, uint8_t *fw_rev_1,  uint8_t *fw_rev_2, uint8_t *fw_rev_3);
void ota_status_change(bool status);
OTA_CONTROL_ENV_T ota_control_env __attribute__((aligned(4)));
static bool ota_control_check_image_crc(void);
static void app_update_ota_boot_info(void);
static void app_update_magic_number_of_app_image(uint32_t newMagicNumber);

static bool _tws_is_master(void);
static bool _tws_is_slave(void);
static void ota_control_opera_callback(void* param);

static uint8_t ota_role_switch_flag = 0;
static bool ota_role_switch_initiator = false;
osTimerId ota_role_switch_req_timer_id = NULL;
static int app_forbidden_roleswitch_handling_timer_handler(void const *param);
osTimerDef (APP_POST_ROLESWITCH_FORRIDEN_TIMER, (void (*)(void const *))app_forbidden_roleswitch_handling_timer_handler);

static const char *typeCode2Str(uint8_t typeCode) {
    #define CASE_M(s) \
        case s: return "["#s"]";

    switch(typeCode) {
    CASE_M(OTA_COMMAND_START)
    CASE_M(OTA_RSP_START)
    CASE_M(OTA_COMMAND_SEGMENT_VERIFY)
    CASE_M(OTA_RSP_SEGMENT_VERIFY)
    CASE_M(OTA_RSP_RESULT)
    CASE_M(OTA_DATA_PACKET)
    CASE_M(OTA_COMMAND_CONFIG_OTA)
    CASE_M(OTA_RSP_CONFIG)
    CASE_M(OTA_COMMAND_GET_OTA_RESULT)
    CASE_M(OTA_READ_FLASH_CONTENT)
    CASE_M(OTA_FLASH_CONTENT_DATA)
    CASE_M(OTA_DATA_ACK)
    CASE_M(OTA_COMMAND_RESUME_VERIFY)
    CASE_M(OTA_RSP_RESUME_VERIFY)
    CASE_M(OTA_COMMAND_GET_VERSION)
    CASE_M(OTA_RSP_VERSION)
    CASE_M(OTA_COMMAND_SIDE_SELECTION)
    CASE_M(OTA_RSP_SIDE_SELECTION)
    CASE_M(OTA_COMMAND_IMAGE_APPLY)
    CASE_M(OTA_RSP_IMAGE_APPLY)
    CASE_M(OTA_CMD_SET_USER)
    CASE_M(OTA_RSP_SET_USER)
    CASE_M(OTA_CMD_GET_OTA_VERSION)
    CASE_M(OTA_RSP_GET_OTA_VERSION)
    CASE_M(OTA_RSP_START_ROLE_SWITCH)
    }

    return "[]";
}

static char *_user2str(uint8_t user)
{
    const char *str = NULL;

    switch (user)
    {
        CASES(BES_OTA_USER_, FIRMWARE);
        CASES(BES_OTA_USER_, LANGUAGE_PACKAGE);
        CASES(BES_OTA_USER_, COMBOFIRMWARE);
        // CASES(BES_OTA_USER_, );

    default:
        str = "INVALID";
        break;
    }

    return (char *)str;
}

static void ota_env_var_init(void)
{
    isInBesOtaState = true;
    ota_role_switch_flag = 0;
    ota_get_pkt_in_role_switch = false;
    app_set_ota_role_switch_initiator(false);
}

static int app_forbidden_roleswitch_handling_timer_handler(void const *param)
{
    osTimerStop(ota_role_switch_req_timer_id);
    if(ota_role_switch_req_timer_num < 2)
    {
        ota_control_send_start_role_switch();
        osTimerStart(ota_role_switch_req_timer_id,200);
        ota_role_switch_req_timer_num++;
    }
    else if (ota_role_switch_req_timer_num == 2)
    {
        app_set_ota_role_switch_initiator(false);
    }
    return 0;
}

void bes_ota_send_role_switch_req(void)
{
#ifdef IBRT
    if(ota_role_switch_req_timer_id == NULL)
    {
        ota_role_switch_req_timer_id = osTimerCreate(osTimer(APP_POST_ROLESWITCH_FORRIDEN_TIMER), osTimerOnce, NULL);
    }

    if (!app_ibrt_conn_any_role_switch_running())
    {
        osTimerStart(ota_role_switch_req_timer_id, 200);
        ota_control_send_start_role_switch();
        ota_role_switch_req_timer_num = 1;
    }
#endif
}


#ifndef __IAG_BLE_INCLUDE__
#define VOICEPATH_COMMON_OTA_BUFF_SIZE 4096

uint8_t* app_voicepath_get_common_ota_databuf(void)
{
#ifndef AI_OTA
    static uint8_t voicepath_common_ota_buf[VOICEPATH_COMMON_OTA_BUFF_SIZE];
    return voicepath_common_ota_buf;
#else
    return ota_common_get_buff_cache();
#endif
}
#endif

/**
 * @brief Update the user of OTA progress.
 *
 * @param user          OTA user, @see BES_OTA_USER enum to get more detailed info
 */
static void _update_user(uint8_t user)
{
    LOG_I("try to update user:%s->%s", _user2str(ota_control_env.currentUser), _user2str(user));

    ota_control_env.currentUser = user;
}

// TODO: make an API decouple with IBRT version
static POSSIBLY_UNUSED void Bes_enter_ota_state(void)
{
    LOG_D("%s state %d", __func__, isInBesOtaState);

    if (isInBesOtaState)
    {
        return;
    }

    // 1. switch to the highest freq
    app_sysfreq_req(APP_SYSFREQ_USER_OTA, APP_SYSFREQ_104M);

    // 2. exit bt sniff mode
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
    if(DATA_PATH_SPP != ota_control_get_datapath_type()){
        app_ibrt_if_request_modify_tws_bandwidth(TWS_TIMING_CONTROL_USER_OTA, true);
    }
    app_tws_ibrt_exit_sniff_with_tws();
    uint8_t* mobileAddr = ota_basic_get_mac_addr();
    app_ibrt_if_exit_sniff_with_mobile(mobileAddr);

    if (DATA_PATH_SPP == app_ota_get_connected_type())
    {
        app_ibrt_if_prevent_sniff_set(mobileAddr, OTA_ONGOING);
    }

#else
    app_bt_active_mode_set(BT_ACTIVE_MODE_KEEP_USER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif

#ifdef __IAG_BLE_INCLUDE__
    // 3. update BLE conn parameter
    if (DATA_PATH_BLE == ota_control_get_datapath_type())
    {
        bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, true);
    }
#endif

    ota_env_var_init();
}

void Bes_exit_ota_state(void)
{
    LOG_D("%s state %d", __func__, isInBesOtaState);
    if (!isInBesOtaState)
    {
        return;
    }

    app_sysfreq_req(APP_SYSFREQ_USER_OTA, APP_SYSFREQ_32K);

#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
    app_ibrt_if_prevent_sniff_clear(ota_basic_get_mac_addr(), OTA_ONGOING);

    app_ibrt_if_request_modify_tws_bandwidth(TWS_TIMING_CONTROL_USER_OTA, false);

    tws_ctrl_free_cmd(APP_TWS_CMD_OTA_IMAGE_BUFF);
    tws_ctrl_free_cmd(APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);
#else
    app_bt_active_mode_clear(BT_ACTIVE_MODE_KEEP_USER_OTA, UPDATE_ACTIVE_MODE_FOR_ALL_LINKS);
#endif

#ifdef __IAG_BLE_INCLUDE__
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA, false);
    bes_ble_gap_update_conn_param_mode(BLE_CONN_PARAM_MODE_OTA_SLOWER, false);
#endif

    /// reset ota env
    ota_control_reset_env(true);

    isInBesOtaState = false;
}

void ota_control_send_start_role_switch(void)
{
#ifdef IBRT
    uint32_t len = 0;
    uint8_t ibrt_role_switch_parm = OTA_RS_INFO_MASTER_SEND_RS_REQ_CMD;

    if(!app_get_bes_ota_state())
    {
        return;
    }

    if ((OTA_BASIC_TWS_MASTER == ota_basic_get_tws_role()) &&
        (ota_control_env.transmitHander != NULL))
    {
        LOG_D("tws cmd : send 0x95 cmd complete");
        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_START_ROLE_SWITCH,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_START_ROLE_SWITCH,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
        }
    }
    else if ((OTA_BASIC_TWS_SLAVE == ota_basic_get_tws_role()) &&
        (app_tws_ibrt_tws_link_connected()))
    {
        osTimerStop(ota_role_switch_req_timer_id);
        LOG_D("[OTA] tws cmd : info send 0x95 cmd complete");
        tws_ctrl_send_cmd(APP_TWS_CMD_OTA_ROLE_SWITCH_CMD,&ibrt_role_switch_parm,1);
    }
#endif
}

void ota_control_send_role_switch_complete(void)
{
#ifdef IBRT
    uint8_t ibrt_role_switch_parm = OTA_RS_INFO_MASTER_DISCONECT_CMD;

    if(!app_get_bes_ota_state())
    {
        LOG_W("[OTA] not in ota");
        return;
    }

    if(ota_control_get_datapath_type() == DATA_PATH_SPP)
    {
        uint32_t len = 0;
        LOG_D("[OTA] send 0x96 cmd complete");
#if defined(IBRT_UI)
        if((OTA_BASIC_TWS_MASTER == ota_basic_get_tws_role()) &&
            (ota_control_env.transmitHander != NULL))
#endif
        {
            if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
            {
                OTA_PACKET_T rsp = {
                    .packetType = OTA_RSP_ROLE_SWITCH_COMPLETE,
                    .length = 0,
                    .rfu = 0,
                };
                len += OTA_TLV_PACKET_HEADER_LEN;

                ota_control_env.transmitHander((uint8_t *)&rsp, len);
            }
            else
            {
                OTA_LEGACY_PACKET_T rsp = {
                    .packetType = OTA_RSP_ROLE_SWITCH_COMPLETE,
                };
                len += OTA_LEGACY_PACKET_HEADER_LEN;

                ota_control_env.transmitHander((uint8_t *)&rsp, len);
            }
        }
#if defined(IBRT_UI)
        else if ((OTA_BASIC_TWS_SLAVE == ota_basic_get_tws_role()) && \
                 (app_tws_ibrt_tws_link_connected()) && \
                  ota_get_pkt_in_role_switch)
#endif
        {
            LOG_D("[OTA]info master disconnect");
            tws_ctrl_send_cmd(APP_TWS_CMD_OTA_ROLE_SWITCH_CMD,&ibrt_role_switch_parm,1);
            ota_get_pkt_in_role_switch = false;
        }
    }
    ota_role_switch_flag = 0;
#endif
}

/**
 * @brief Register the data transmission handler. Called from SPP or BLE layer
 *
 * @param transmit_handle    Handle of the transmission handler
 *
 */
void ota_control_register_transmitter(ota_transmit_data_t transmit_handle)
{
    ota_control_env.transmitHander = transmit_handle;
}

void ota_control_set_datapath_type(uint8_t datapathType)
{
    ota_control_env.dataPathType = datapathType;
}

uint8_t ota_control_get_datapath_type(void)
{
    return ota_control_env.dataPathType;
}

static NORFLASH_API_MODULE_ID_T _get_user_flash_module(uint8_t user)
{
    uint8_t module = NORFLASH_API_MODULE_ID_COUNT;

    for (uint8_t i = 0; i < ota_control_env.userNum; i++)
    {
        if (ota_control_env.userInfo[i].user == user)
        {
            module = ota_control_env.userInfo[i].norflashId;
            break;
        }
    }

    return (NORFLASH_API_MODULE_ID_T)module;
}

static uint8_t _get_user_index(uint8_t user)
{
    uint8_t index = BES_OTA_USER_INDEX_INVALID;

    for (uint8_t i = 0; i < ota_control_env.userNum; i++)
    {
        if (ota_control_env.userInfo[i].user == user)
        {
            index = i;
            break;
        }
    }

    return index;
}

static POSSIBLY_UNUSED uint32_t _get_user_start_addr(uint8_t user)
{
    uint32_t addr = 0;

    for (uint8_t i = 0; i < ota_control_env.userNum; i++)
    {
        if (ota_control_env.userInfo[i].user == user)
        {
            addr = ota_control_env.userInfo[i].startAddr;
            break;
        }
    }

    return addr;
}

/**
 * @brief Reset the environment of the OTA handling
 *
 */
void ota_control_reset_env(bool resetUser)
{
    LOG_D("%s resetUser %d", __func__, resetUser);

    InitCQueue(&otaCmdQueue, sizeof(accumulatedOtaBuf), accumulatedOtaBuf);
    ota_control_env.configuration.isToClearUserData = true;
    ota_control_env.configuration.isToRenameBLE = false;
    ota_control_env.configuration.isToRenameBT = false;
    ota_control_env.configuration.isToUpdateBLEAddr = false;
    ota_control_env.configuration.isToUpdateBTAddr = false;
    ota_control_env.configuration.lengthOfFollowingData = 0;
    ota_control_env.crc32OfSegment = 0;
    ota_control_env.crc32OfImage = 0;
    ota_control_env.offsetInDataBufferForBurning = 0;
    ota_control_env.alreadyReceivedDataSizeOfImage = 0;
    ota_control_env.offsetOfImageOfCurrentSegment = 0;
    ota_status_change(false);
    ota_control_env.isPendingForReboot = false;
    ota_control_env.leftSizeOfFlashContentToRead = 0;

#ifdef COMBO_CUSBIN_IN_FLASH
    ota_control_env.is_crossregion = false;
    ota_control_env.blockid = 0;
    ota_control_env.currentlengthlimit = 0;
    ota_control_env.lastdstFlashOffsetForNewImage = 0;
    ota_control_env.combinedSize = 0;
#endif

    if(ota_control_env.resume_at_breakpoint == false)
    {
        ota_control_env.breakPoint = 0;
        ota_control_env.i_log = -1;
    }

#ifndef __IAG_BLE_INCLUDE__
    ota_control_env.dataBufferForBurning = app_voicepath_get_common_ota_databuf();
#endif

    if (resetUser)
    {
        ota_control_env.packetStructureType = OTA_PACKET_STRUCTURE_LEGACY;
        ota_control_env.isOneBudOTA = 0;
        _update_user(BES_OTA_USER_INVALID);
    }
}

/**
 * @brief Set the packet interval for app
 *
 *
 */
static void ota_control_set_packet_interval()
{
    // Suitable for i6 and below
    if (ota_basic_mtu_get() > OTA_I6_BELOW_DEFAULT_MTU)
    {
        return;
    }

    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T cmd = {
                .packetType = OTA_SET_PACKET_INTERVAL,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            cmd.payload.cmdOtaInterval.interval = OTA_APP_PACKET_INTERVAL_FOR_I6_BELOW;
            len += sizeof(cmd.payload.cmdOtaInterval.interval);

            cmd.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&cmd, len);

            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&cmd, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T cmd = {
                .packetType = OTA_SET_PACKET_INTERVAL,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            cmd.payload.cmdOtaInterval.interval = OTA_APP_PACKET_INTERVAL_FOR_I6_BELOW;
            len += sizeof(cmd.payload.cmdOtaInterval.interval);

            ota_control_env.transmitHander((uint8_t *)&cmd, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&cmd, len);
        }
    }
}

/**
 * @brief Send the response to start OTA control packet
 *
 *
 */
static void ota_control_send_start_response(bool isViaBle)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_START,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            rsp.payload.rspOtaStart.magicCode = BES_OTA_START_MAGIC_CODE;
            len += 4;

            rsp.payload.rspOtaStart.swVersion = OTA_SW_VERSION;
            len += 2;

            rsp.payload.rspOtaStart.hwVersion = OTA_HW_VERSION;
            len += 2;

            rsp.payload.rspOtaStart.MTU = ota_basic_mtu_get();
            len += 2;

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);

            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_START,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            rsp.payload.rspOtaStart.magicCode = BES_OTA_START_MAGIC_CODE;
            len += 4;

            rsp.payload.rspOtaStart.swVersion = OTA_SW_VERSION;
            len += 2;

            rsp.payload.rspOtaStart.hwVersion = OTA_HW_VERSION;
            len += 2;

            rsp.payload.rspOtaStart.MTU = ota_basic_mtu_get();
            len += 2;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ibrt_ota_send_start_response(bool isViaBle)
{
    ota_control_send_start_response(isViaBle);
}

static void ota_control_send_configuration_response(bool isDone)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_CONFIG,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspFlowControl.isConfigurationDone = isDone;
            len += sizeof(isDone);

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_CONFIG,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspFlowControl.isConfigurationDone = isDone;
            len += sizeof(isDone);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ibrt_ota_send_configuration_response(bool isDone)
{
    ota_control_send_configuration_response(isDone);
}

#if DATA_ACK_FOR_SPP_DATAPATH_ENABLED
static void ota_control_send_data_ack_response(void)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_DATA_ACK,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_DATA_ACK,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}
#endif

static void ota_control_send_version_rsp(void)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;
        uint8_t fw_rev[4];
        system_get_info(&fw_rev[0], &fw_rev[1], &fw_rev[2], &fw_rev[3]);
        char git_version[255] = TO_STR(GIT_VERSION);

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_VERSION,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            rsp.payload.rspGetVersion.magicCode = BES_OTA_START_MAGIC_CODE;
            len += 4;

            rsp.payload.rspGetVersion.deviceType = 0x00;
            len += 2;

            for (int i = 0; i < HW_VERSION_LENTH; i++)
            {
                rsp.payload.rspGetVersion.leftVersion[i] = fw_rev[i];
                rsp.payload.rspGetVersion.rightVersion[i] = 0;
            }
            len += HW_VERSION_LENTH * 2;

            uint8_t length = strlen(git_version);
            for (uint8_t i = 0; i < length; i++)
            {
                rsp.payload.rspGetVersion.gitVersion[i] = git_version[i];
            }
            len += length;

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_VERSION,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            rsp.payload.rspGetVersion.magicCode = BES_OTA_START_MAGIC_CODE;
            len += 4;

            rsp.payload.rspGetVersion.deviceType = 0x00;
            len += 1;

            for (int i = 0; i < HW_VERSION_LENTH; i++)
            {
                rsp.payload.rspGetVersion.leftVersion[i] = fw_rev[i];
                rsp.payload.rspGetVersion.rightVersion[i] = 0;
            }
            len += HW_VERSION_LENTH * 2;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ibrt_ota_send_version_rsp(void)
{
    ota_control_send_version_rsp();
}

void ota_control_side_selection_rsp(uint8_t success)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_SIDE_SELECTION,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspSideSelect.success = success;
            len += sizeof(success);

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_SIDE_SELECTION,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspSideSelect.success = success;
            len += sizeof(success);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ota_control_single_bud_upgrade_rsp(uint8_t success)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        /// only support tlv packet
        uint32_t len = 0;
        OTA_PACKET_T rsp = {
            .packetType = OTA_SET_ONE_BUD_UPGRADE_RSP,
            .length = 0,
            .rfu = 0,
        };
        len += OTA_TLV_PACKET_HEADER_LEN;

        /// update execution result
        rsp.payload.rspOneBudSelect.success = success;
        len += sizeof(success);

        rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
        ota_control_env.transmitHander((uint8_t *)&rsp, len);
        LOG_D("%s transmit data to APP(tlv):", __func__);
        DUMP8("%x ", (uint8_t *)&rsp, len);
    }
}

void ota_control_image_apply_rsp(uint8_t success)
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_IMAGE_APPLY,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspApply.success = success;
            len += sizeof(success);

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_IMAGE_APPLY,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspApply.success = success;
            len += sizeof(success);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

/**
 * @brief Send the response to segement verification request
 *
 * @param isPass    false if failed and central will retransmit the former segment.
 *
 */
static void ota_control_send_segment_verification_response(bool isPass)
{
    LOG_D("Segment of image's verification pass status is %d (1:pass 0:failed)", isPass);

    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_SEGMENT_VERIFY,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspSegmentVerify.isVerificationPassed = isPass;
            rsp.payload.rspSegmentVerify.crc32OfSegment = ota_control_env.receivedCrc32OfSegment;
            len += (sizeof(isPass)+sizeof(ota_control_env.receivedCrc32OfSegment));

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_SEGMENT_VERIFY,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspSegmentVerify.isVerificationPassed = isPass;
            len += sizeof(isPass);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ibrt_ota_send_segment_verification_response(bool isPass)
{
    tws_ctrl_free_cmd(APP_TWS_CMD_OTA_IMAGE_BUFF);
    tws_ctrl_free_cmd(APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);

    ota_control_send_segment_verification_response(isPass);
}

/**
 * @brief Send the response to inform central that the OTA has been successful or not
 *
 */
void ota_control_send_result_response(uint8_t isSuccessful)
{
#if !defined(FREEMAN_OTA_ENABLED)
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
#endif
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_RESULT,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspOtaResult.isVerificationPassed = isSuccessful;
            len += sizeof(isSuccessful);

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_RESULT,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            /// update execution result
            rsp.payload.rspOtaResult.isVerificationPassed = isSuccessful;
            len += sizeof(isSuccessful);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
    }
}

void ibrt_ota_send_result_response(uint8_t isSuccessful)
{
    ota_control_send_result_response(isSuccessful);
}

void ota_control_send_get_random_response(uint32_t ota_rv)
{
#if !defined(FREEMAN_OTA_ENABLED)
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
#endif
    {
        uint32_t len = 0;
        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_GET_LOCAL_RD_RSP,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            rsp.payload.rspLocalRandom.localRandom[0] = (uint8_t)((ota_rv>>8)&0xff);
            rsp.payload.rspLocalRandom.localRandom[1] = (uint8_t)(ota_rv & 0xff);
            len += 2;

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(tlv):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_GET_LOCAL_RD_RSP,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            rsp.payload.rspLocalRandom.localRandom[0] = (uint8_t)((ota_rv>>8)&0xff);
            rsp.payload.rspLocalRandom.localRandom[1] = (uint8_t)(ota_rv & 0xff);
            len += 2;

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
            LOG_D("%s transmit data to APP(legacy):", __func__);
            DUMP8("%x ", (uint8_t *)&rsp, len);

        }
    }
}

/**
 * @brief Send response to OTA set user command
 *
 * NOTE: set_user command is only supported with TLV packet structure
 *
 * @param isSuccessful  The result of set_user command execution, true for success; false for fail
 */
static void _send_set_user_response(uint8_t isSuccessful)
{
#if !defined(FREEMAN_OTA_ENABLED)
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        // do nothing
    }
    else
#endif
    {
        uint32_t len = 0;

        OTA_PACKET_T rsp = {
            .packetType = OTA_RSP_SET_USER,
            .length = 0,
            .rfu = 0,
        };
        len += OTA_TLV_PACKET_HEADER_LEN;

        rsp.payload.rspSetUser.result = isSuccessful;
        len += sizeof(isSuccessful);

        rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
        ota_control_env.transmitHander((uint8_t *)&rsp, len);
        LOG_D("%s transmit data to APP(tlv):", __func__);
        DUMP8("%x ", (uint8_t *)&rsp, len);
    }
}

/**
 * @brief Send response to get OTA version command
 *
 * NOTE: get_ota_version command is only supported with TLV packet structure
 *
 * @param buf           pointer of peer OTA version info
 * @param len           length of peer OTA version info
 */
static void _send_ota_version_rsp(uint8_t *buf, uint16_t length)
{
    ASSERT(OTA_VERSION_INFO_LEN == length, "INVALID OTA version info len");
    ASSERT(buf, "NULL pointer received in %s", __func__);

    // tws mode and isn't master
    if (!_tws_is_master())
    {
        // do nothing
    }
    else
    {
        uint32_t len = 0;
        int8_t ret = 0;

        if (buf)
        {
            ret = memcmp(otaVersion, buf, length);
        }

        OTA_PACKET_T rsp = {
            .packetType = OTA_RSP_GET_OTA_VERSION,
            .length = 0,
            .rfu = 0,
        };
        len += OTA_TLV_PACKET_HEADER_LEN;

        if (ret > 0)
        {
            memcpy(rsp.payload.rspOtaVersion.version, buf, OTA_VERSION_INFO_LEN);
        }
        else
        {
            memcpy(rsp.payload.rspOtaVersion.version, otaVersion, OTA_VERSION_INFO_LEN);
        }
        len += OTA_VERSION_INFO_LEN;

        rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
        ota_control_env.transmitHander((uint8_t *)&rsp, OTA_TLV_PACKET_HEADER_LEN + rsp.length);
        LOG_D("%s transmit data to APP(tlv):", __func__);
        DUMP8("%x ", (uint8_t *)&rsp, len);
    }
}

void ibrt_ota_send_set_user_response(uint8_t isSuccessful)
{
    _send_set_user_response(isSuccessful);
}

void ibrt_ota_send_ota_version_rsp(uint8_t *buf, uint16_t len)
{
    _send_ota_version_rsp(buf, len);
}

/**
 * @brief Program the data in the data buffer to flash.
 *
 * @param ptrSource    Pointer of the source data buffer to program.
 * @param lengthToBurn    Length of the data to program.
 * @param offsetInFlashToProgram    Offset in bytes in flash to program
 *
 */
static void ota_control_flush_data_to_flash(uint8_t* ptrSource, uint32_t lengthToBurn, uint32_t offsetInFlashToProgram)
{
    LOG_D("flush %d bytes to flash offset 0x%x", lengthToBurn, offsetInFlashToProgram);
    #ifdef ARM_CMNS
    if(BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser &&
        nv_record_combo_bin_get_fw_id(ota_control_env.blockid) == COMBO_TZ_SB)
    {
        app_flash_tz_ota_program(offsetInFlashToProgram, ptrSource, lengthToBurn);
    }
    else
    #endif
    {
        NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

        uint8_t i = _get_user_index(ota_control_env.currentUser);

        offsetInFlashToProgram = ota_control_env.dstFlashOffsetForNewImage - 
                                (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + 
                                offsetInFlashToProgram;

        uint32_t preBytes = (FLASH_SECTOR_SIZE_IN_BYTES - (offsetInFlashToProgram % FLASH_SECTOR_SIZE_IN_BYTES)) % FLASH_SECTOR_SIZE_IN_BYTES;
        if (lengthToBurn < preBytes)
        {
            preBytes = lengthToBurn;
        }

        uint32_t middleBytes = 0;
        if (lengthToBurn > preBytes)
        {
            middleBytes = ((lengthToBurn - preBytes) / FLASH_SECTOR_SIZE_IN_BYTES * FLASH_SECTOR_SIZE_IN_BYTES);
        }

        uint32_t postBytes = 0;
        if (lengthToBurn > (preBytes + middleBytes))
        {
            postBytes = (offsetInFlashToProgram + lengthToBurn) % FLASH_SECTOR_SIZE_IN_BYTES;
        }

        LOG_D("Prebytes is %d middlebytes is %d postbytes is %d", preBytes, middleBytes, postBytes);

        if (preBytes > 0)
        {
            app_flash_program(mod,
                            offsetInFlashToProgram,
                            ptrSource,
                            preBytes,
                            false);

            ptrSource += preBytes;
            offsetInFlashToProgram += preBytes;
        }

        uint32_t sectorIndexInFlash = offsetInFlashToProgram/FLASH_SECTOR_SIZE_IN_BYTES;

        if (middleBytes > 0)
        {
            uint32_t sectorCntToProgram = middleBytes/FLASH_SECTOR_SIZE_IN_BYTES;
            for (uint32_t sector = 0;sector < sectorCntToProgram;sector++)
            {
                app_flash_sector_erase(mod,
                                    sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES);
                app_flash_program(mod,
                                sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES,
                                ptrSource + sector * FLASH_SECTOR_SIZE_IN_BYTES, FLASH_SECTOR_SIZE_IN_BYTES,
                                false);

                sectorIndexInFlash++;
            }

            ptrSource += middleBytes;
        }

        if (postBytes > 0)
        {
            app_flash_sector_erase(mod,
                                sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES);
            app_flash_program(mod,
                            sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES,
                            ptrSource,
                            postBytes,
                            false);
        }

        app_flash_flush_pending_op(mod,
                                NORFLASH_API_ALL);
    }
}

/**
 * ota_control_flush_data_to_flash() doesn't erase sector firstly on non sector boundary addresses,
 * so the erase_segment() function is supplemented.
 */
void erase_segment(uint32_t addr, uint32_t length)
{
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    uint32_t bytes = 0, sector_num = 0;

    /// The number of bytes current address minus the previous sector boundary address.
    bytes = addr & (FLASH_SECTOR_SIZE_IN_BYTES - 1);
    ///The number of sectors to be erased
    sector_num = (bytes + length)/FLASH_SECTOR_SIZE_IN_BYTES + 1;

    if(bytes)
    {
        //save original data-----first, read
        app_flash_read(mod, addr - bytes, ota_control_env.dataBufferForBurning, FLASH_SECTOR_SIZE_IN_BYTES);
    }

    if(bytes)
    {
        for(uint8_t i = 0; i < sector_num; i++)
        {
            app_flash_sector_erase(mod, (addr - bytes + i * FLASH_SECTOR_SIZE_IN_BYTES));
        }
    }
    else
    {
        for(uint8_t i = 0; i < sector_num; i++)
        {
            app_flash_sector_erase(mod, (addr + i * FLASH_SECTOR_SIZE_IN_BYTES));
        }
    }

    if(bytes)
    {
        //save original data ---- second write back
        app_flash_program(mod, addr - bytes, ota_control_env.dataBufferForBurning, bytes, true);
    }
}

void ota_control_erase_segment(uint32_t offset, uint32_t length)
{
    uint8_t i = _get_user_index(ota_control_env.currentUser);

    uint32_t addr = ota_control_env.dstFlashOffsetForNewImage - (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + offset;
    erase_segment(addr, length);
}

uint32_t ota_get_current_start_addr(void)
{
    uint32_t startFlashAddr = ota_control_env.dstFlashOffsetForNewImage +
                              ota_control_env.offsetOfImageOfCurrentSegment;

    return startFlashAddr;
}

uint32_t ota_get_current_length_to_check(void)
{
    uint32_t lengthToDoCrcCheck = ota_control_env.alreadyReceivedDataSizeOfImage -
                                  ota_control_env.offsetOfImageOfCurrentSegment;

    return lengthToDoCrcCheck;
}

void ota_restore_offset(void)
{
    // restore the offset
    ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
    ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.offsetOfImageOfCurrentSegment;
}

/**
 * @brief CRC check on the whole image
 *
 * @return true if the CRC check passes, otherwise, false.
 */

static bool ota_control_check_image_crc(void)
{
    uint32_t verifiedDataSize = 0;
    uint32_t crc32Value = 0;
    uint32_t verifiedBytes = 0;
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    app_flash_read(mod,
                   0,
                   ota_control_env.dataBufferForBurning,
                   OTA_DATA_BUFFER_SIZE_FOR_BURNING);

    uint32_t *pData = (uint32_t *)ota_control_env.dataBufferForBurning;

    LOG_D("%s:start word: 0x%x", __func__, *pData);

    if (*pData != NORMAL_BOOT)
    {
        LOG_D("%s@%d,Wrong Boot = %p.",
              __func__, __LINE__, ota_control_env.dataBufferForBurning);

        return false;
    }
    LOG_D("ota_control_env.totalImageSize:0x%x", ota_control_env.totalImageSize);

    while (verifiedDataSize < ota_control_env.totalImageSize)
    {
        if (ota_control_env.totalImageSize - verifiedDataSize > OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            verifiedBytes = OTA_DATA_BUFFER_SIZE_FOR_BURNING;
        }
        else
        {
            verifiedBytes = ota_control_env.totalImageSize - verifiedDataSize;
        }

        app_flash_read(mod,
                       verifiedDataSize,
                       ota_control_env.dataBufferForBurning,
                       OTA_DATA_BUFFER_SIZE_FOR_BURNING);

        if (0 == verifiedDataSize)
        {
            if (*(uint32_t *)ota_control_env.dataBufferForBurning != NORMAL_BOOT)
            {
                LOG_D("%s:%d,Wrong Boot = 0x%p.",
                      __func__, __LINE__, ota_control_env.dataBufferForBurning);
                return false;
            }
            else
            {
                *(uint32_t *)ota_control_env.dataBufferForBurning = 0xFFFFFFFF;
                LOG_D("first 50 byte in flash:");
                DUMP8("0x%02x ", ota_control_env.dataBufferForBurning, 50);
            }
        }

        verifiedDataSize += verifiedBytes;

        crc32Value = crc32_c(crc32Value,
                             (uint8_t *)ota_control_env.dataBufferForBurning,
                             verifiedBytes);
    }

    LOG_D("Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", ota_control_env.crc32OfImage, crc32Value);
    if (crc32Value == ota_control_env.crc32OfImage)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ota_update_info(void)
{
#ifdef ARM_CMNS
#ifdef FLASH_REMAP
    app_flash_tz_ota_apply(OTA_CODE_OFFSET, OTA_REMAP_OFFSET, ota_control_env.crc32OfImage);
#else
    app_flash_tz_ota_apply(ota_control_env.dstFlashOffsetForNewImage, 
            ota_control_env.totalImageSize, ota_control_env.crc32OfImage);
#endif
#else
    app_update_ota_boot_info();
#endif
}

void ota_check_and_reboot_to_use_new_image(void)
{
    if (ota.permissionToApply && ota_control_env.isPendingForReboot == true)
    {
        app_start_postponed_reset();
    }
}

bool ota_is_in_progress(void)
{
    return ota_control_env.isOTAInProgress;
}

void ota_status_change(bool status)
{
    ota_control_env.isOTAInProgress = status;
}

static int32_t find_key_word(uint8_t *targetArray, uint32_t targetArrayLen,
                             uint8_t *keyWordArray, uint32_t keyWordArrayLen)
{
    if ((keyWordArrayLen > 0) && (targetArrayLen >= keyWordArrayLen))
    {
        uint32_t index = 0, targetIndex = 0;
        for (targetIndex = 0;targetIndex < targetArrayLen;targetIndex++)
        {
            for (index = 0;index < keyWordArrayLen;index++)
            {
                if (targetArray[targetIndex + index] != keyWordArray[index])
                {
                    break;
                }
            }

            if (index == keyWordArrayLen)
            {
                return targetIndex;
            }
        }

        return -1;
    }
    else
    {
        return -1;
    }
}

static uint8_t asciiToHex(uint8_t asciiCode)
{
    if ((asciiCode >= '0') && (asciiCode <= '9'))
    {
        return asciiCode - '0';
    }
    else if ((asciiCode >= 'a') && (asciiCode <= 'f'))
    {
        return asciiCode - 'a' + 10;
    }
    else if ((asciiCode >= 'A') && (asciiCode <= 'F'))
    {
        return asciiCode - 'A' + 10;
    }
    else
    {
        return 0xff;
    }
}

static bool ota_check_image_data_sanity_crc(void)
{
    // TODO: obsolete when the known apk issue is fixed

    // find the location of the CRC key word string
    uint8_t *ptrOfTheLast4KImage = NULL;
    uint32_t sanityCrc32 = 0;
    uint32_t lengthToDoCrcCheck = 0;
    uint32_t lengthCrcCheckDone = 0;
    uint32_t lengthCrcCheckNow = 0;
    uint32_t calculatedCrc32 = 0;

    /// get the OTA module address
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    app_flash_read(mod,
                   (ota_control_env.totalImageSize - LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC),
                   ota_control_env.dataBufferForBurning,
                   LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC);
    ptrOfTheLast4KImage = ota_control_env.dataBufferForBurning;

    int32_t sanity_crc_location = find_key_word(ptrOfTheLast4KImage,
                                                LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC,
                                                (uint8_t *)image_info_sanity_crc_key_word,
                                                strlen(image_info_sanity_crc_key_word));
    if (-1 == sanity_crc_location)
    {
        // if no sanity crc, the image has the old format, just ignore such a check
        return true;
    }

    LOG_D("sanity_crc_location is %d", sanity_crc_location);

    uint8_t *crcString = (uint8_t *)(ptrOfTheLast4KImage + sanity_crc_location + strlen(image_info_sanity_crc_key_word));

    for (uint8_t index = 0; index < 8; index++)
    {
        sanityCrc32 |= (asciiToHex(crcString[index]) << (28 - 4 * index));
    }

    LOG_D("sanityCrc32 is 0x%x", sanityCrc32);

    // generate the CRC from image data
    lengthToDoCrcCheck = sanity_crc_location + ota_control_env.totalImageSize -
                         LEN_OF_IMAGE_TAIL_TO_FIND_SANITY_CRC + strlen(image_info_sanity_crc_key_word);
    LOG_D("Bytes to generate crc32 is %d", lengthToDoCrcCheck);

    uint32_t last_ping_time_ms = 0;
    uint32_t current_time_ms  = 0;
    last_ping_time_ms = hal_sys_timer_get();

    while (lengthToDoCrcCheck > lengthCrcCheckDone)
    {
        lengthCrcCheckNow = (lengthToDoCrcCheck - lengthCrcCheckDone) > OTA_DATA_BUFFER_SIZE_FOR_BURNING
                                ? OTA_DATA_BUFFER_SIZE_FOR_BURNING
                                : (lengthToDoCrcCheck - lengthCrcCheckDone);
                                // LOG_D(2,"lengthCrcCheckNow 0x%x", lengthCrcCheckNow);

        app_flash_read(mod,
                       lengthCrcCheckDone,
                       ota_control_env.dataBufferForBurning,
                       lengthCrcCheckNow);

        current_time_ms = hal_sys_timer_get();
        if (current_time_ms - last_ping_time_ms > PING_WD_PERIOD_TICKS)
        {
    #ifdef __WATCHER_DOG_RESET__
            watchdog_ping();
    #endif
            last_ping_time_ms = current_time_ms;
        }

        lengthCrcCheckDone += lengthCrcCheckNow;
        calculatedCrc32 = crc32_c(calculatedCrc32, ota_control_env.dataBufferForBurning, lengthCrcCheckNow);
    }

    LOG_D("calculatedCrc32 is 0x%x", calculatedCrc32);

    return (sanityCrc32 == calculatedCrc32);
}

static unsigned int seed = 1;

static void set_rand_seed(unsigned int init)
{
    seed = init;
}

static int get_rand(void)
{
    //Based on Knuth "The Art of Computer Programming"
    seed = seed * 1103515245 + 12345;
    return ( (unsigned int) (seed / 65536) % (32767+1) );
}

void ota_randomCode_log(uint8_t randomCode[])
{
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_UPGRADE_LOG;

    app_flash_sector_erase(mod,
                         0);

    app_flash_program(mod,
                      0,
                      randomCode,
                      sizeof(otaUpgradeLog.randomCode),
                      false);
}

void ota_update_start_message(void)
{
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_UPGRADE_LOG;

    /// update upgrade basic info if needed
    if ((otaUpgradeLog.totalImageSize != ota_control_env.totalImageSize) ||
        (otaUpgradeLog.crc32OfImage != ota_control_env.crc32OfImage))
    {
        app_flash_program(mod,
                          OFFSETOF(FLASH_OTA_UPGRADE_LOG_FLASH_T, totalImageSize),
                          (uint8_t *)&ota_control_env.totalImageSize,
                          sizeof(ota_control_env.totalImageSize),
                          false);
        app_flash_program(mod,
                          OFFSETOF(FLASH_OTA_UPGRADE_LOG_FLASH_T, crc32OfImage),
                          (uint8_t *)&ota_control_env.crc32OfImage,
                          sizeof(ota_control_env.crc32OfImage),
                          true); //!< force flush the flash op

        LOG_I("update upgrade info: totalImageSize->0x%08x, crc32OfImage->0x%08x",
              otaUpgradeLog.totalImageSize, otaUpgradeLog.crc32OfImage);
    }
}

void ota_get_start_message(void)
{
    ota_control_env.totalImageSize = otaUpgradeLog.totalImageSize;
    ota_control_env.crc32OfImage = otaUpgradeLog.crc32OfImage;
    LOG_D("[%s] totalImageSize = 0x%x, crc32OfImage = 0x%x",
          __func__, ota_control_env.totalImageSize, ota_control_env.crc32OfImage);
}


uint32_t ota_common_get_boot_info_flash_offset(void)
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    #ifndef OTA_SEC_BOOT_INFO_OFFSET
    uint32_t otaBootInfoAddr = OTA_INFO_IN_OTA_BOOT_SEC;
    #else
    uint32_t otaBootInfoAddr = OTA_SEC_BOOT_INFO_OFFSET;
    #endif

    memcpy((uint8_t *)&otaBootInfo, (uint8_t *)otaBootInfoAddr, sizeof(FLASH_OTA_BOOT_INFO_T));
    LOG_D("[%s],otaBootInfo.magicNumber = 0x%04X", __func__, otaBootInfo.magicNumber);
    LOG_D("0x%02x ",otaBootInfoAddr);
    if ((NORMAL_BOOT == otaBootInfo.magicNumber) || (COPY_NEW_IMAGE == otaBootInfo.magicNumber))
    {
        return OTA_INFO_IN_OTA_BOOT_SEC;
    }
    else
    {
        ASSERT(0, "OTA bootinfo addr is wrong");
        return 0;
    }
}
void ota_upgradeSize_log(void)
{
    LOG_D("[%s] totalImageSize = 0x%x, crc32OfImage = 0x%x",
          __func__, ota_control_env.totalImageSize, ota_control_env.crc32OfImage);

    #ifdef ARM_CMNS
    if(BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser &&
        nv_record_combo_bin_get_fw_id(ota_control_env.blockid) == COMBO_TZ_SB)
    {
        return;
    }
    #endif
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_UPGRADE_LOG;

    if(++ota_control_env.i_log >= sizeof(otaUpgradeLog.upgradeSize)/(sizeof(uint32_t)/sizeof(uint8_t)))
    {
        ota_control_env.i_log = 0;

        app_flash_read(mod,
                       0,
                       ota_control_env.dataBufferForBurning,
                       OTAUPLOG_HEADSIZE);
        app_flash_sector_erase(mod,
                             0);
        app_flash_program(mod,
                          0,
                          ota_control_env.dataBufferForBurning,
                          OTAUPLOG_HEADSIZE,
                          false);
    }

    app_flash_program(mod,
                      OFFSETOF(FLASH_OTA_UPGRADE_LOG_FLASH_T, upgradeSize) + (sizeof(uint32_t) * (ota_control_env.i_log)),
                      (uint8_t *)&ota_control_env.alreadyReceivedDataSizeOfImage,
                      sizeof(ota_control_env.alreadyReceivedDataSizeOfImage),
                      true);

    LOG_D("{i_log: %d, RecSize: 0x%x, FlashWrSize: 0x%x}",
          ota_control_env.i_log, ota_control_env.alreadyReceivedDataSizeOfImage,
          otaUpgradeLog.upgradeSize[ota_control_env.i_log]);
}

void ota_upgradeLog_destroy(void)
{
    LOG_D("Destroyed upgrade log in flash.");
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_UPGRADE_LOG;

    ota_control_env.resume_at_breakpoint = false;
    app_flash_sector_erase(mod,
                         0);

    app_flash_flush_pending_op(mod,
                               NORFLASH_API_ERASING);
}

uint32_t get_upgradeSize_log(void)
{
    int32_t *p = (int32_t*)otaUpgradeLog.upgradeSize,
            left = 0, right = sizeof(otaUpgradeLog.upgradeSize)/4 - 1, mid;

    if(p[0] != -1)
    {
        while(left < right)
        {
            mid = (left + right) / 2;
            if(p[mid] == -1)
                right = mid - 1;
            else
                left = mid + 1;
        }
    }
    if(p[left]==-1)
        left--;

    ota_control_env.i_log = left;
    ota_control_env.breakPoint = left!=-1 ? p[left] : 0;
    ota_control_env.resume_at_breakpoint = ota_control_env.breakPoint?true:false;

    LOG_D("ota_control_env.i_log: %d", ota_control_env.i_log);
    return ota_control_env.breakPoint;
}

void ota_control_send_resume_response(uint32_t breakPoint, uint8_t randomCode[])
{
    // tws mode and isn't master
    if (!_tws_is_master())
    {
        /// do nothing
    }
    else
    {
        uint32_t len = 0;

        if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
        {
            OTA_PACKET_T rsp = {
                .packetType = OTA_RSP_RESUME_VERIFY,
                .length = 0,
                .rfu = 0,
            };
            len += OTA_TLV_PACKET_HEADER_LEN;

            rsp.payload.rspResumeBreakpoint.breakPoint = breakPoint;
            len += sizeof(breakPoint);

            for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
            {
                rsp.payload.rspResumeBreakpoint.randomCode[i] = randomCode[i];
            }
            len += sizeof(otaUpgradeLog.randomCode);

            rsp.payload.rspResumeBreakpoint.crc32 =
                crc32_c(0, (uint8_t *)&rsp.payload.rspResumeBreakpoint.breakPoint,
                      sizeof(rsp.payload.rspResumeBreakpoint.breakPoint) +
                          sizeof(rsp.payload.rspResumeBreakpoint.randomCode));
            len += 4;

            rsp.length = len - OTA_TLV_PACKET_HEADER_LEN;
            ota_control_env.transmitHander((uint8_t *)&rsp, len);
        }
        else
        {
            OTA_LEGACY_PACKET_T rsp = {
                .packetType = OTA_RSP_RESUME_VERIFY,
            };
            len += OTA_LEGACY_PACKET_HEADER_LEN;

            rsp.payload.rspResumeBreakpoint.breakPoint = breakPoint;
            len += sizeof(breakPoint);

            for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
            {
                rsp.payload.rspResumeBreakpoint.randomCode[i] = randomCode[i];
            }
            len += sizeof(otaUpgradeLog.randomCode);

            rsp.payload.rspResumeBreakpoint.crc32 =
                crc32_c(0, (uint8_t *)&rsp.payload.rspResumeBreakpoint.breakPoint,
                      sizeof(rsp.payload.rspResumeBreakpoint.breakPoint) +
                          sizeof(rsp.payload.rspResumeBreakpoint.randomCode));
            len += sizeof(rsp.payload.rspResumeBreakpoint.crc32);

            ota_control_env.transmitHander((uint8_t *)&rsp, len);
        }
    }
}

void ota_update_flash_offset_after_segment_crc(bool flag)
{
    if(flag)
    {
        ota_upgradeSize_log();
        ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;
        ota_control_env.offsetOfImageOfCurrentSegment = ota_control_env.alreadyReceivedDataSizeOfImage;
    }
    else
    {
        uint32_t lengthToDoCrcCheck = ota_control_env.alreadyReceivedDataSizeOfImage -
                                        ota_control_env.offsetOfImageOfCurrentSegment;
#if COMBO_CUSBIN_IN_FLASH
        if(BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
        {
            _combo_bin_erase_segment(ota_control_env.offsetInFlashOfCurrentSegment, lengthToDoCrcCheck);
        }
#else
        ota_control_erase_segment(ota_control_env.offsetInFlashOfCurrentSegment, lengthToDoCrcCheck);
#endif

        // restore the offset
        ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
        ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.offsetOfImageOfCurrentSegment;
    }
}

#ifdef IBRT
void ibrt_tws_ota_breakPoint_resume(uint32_t breakPoint, uint8_t randomCode[])
{
    ASSERT(randomCode, "NULL pointer received in %s", __func__);
    uint32_t len = 0;

    OTA_LEGACY_PACKET_T packet = {
        .packetType = OTA_RSP_RESUME_VERIFY,
    };
    len += OTA_LEGACY_PACKET_HEADER_LEN;

    /// load breakpoint
    packet.payload.rspResumeBreakpoint.breakPoint = breakPoint;
    len += sizeof(packet.payload.rspResumeBreakpoint.breakPoint);

    /// load random code
    memcpy(packet.payload.rspResumeBreakpoint.randomCode,
           randomCode,
           sizeof(otaUpgradeLog.randomCode));
    len += ARRAY_SIZE(packet.payload.rspResumeBreakpoint.randomCode);

    /// calculate and load crc
    packet.payload.rspResumeBreakpoint.crc32 =
        crc32_c(0, (uint8_t *)&packet.payload.rspResumeBreakpoint.breakPoint,
              sizeof(packet.payload.rspResumeBreakpoint.breakPoint) +
                  sizeof(packet.payload.rspResumeBreakpoint.randomCode));
    len += sizeof(packet.payload.rspResumeBreakpoint.crc32);

    tws_ctrl_send_cmd(APP_TWS_CMD_OTA_BP_CHECK_CMD, (uint8_t*)&packet, len);
}

static void _tws_get_role(void)
{
    ibrt_tws_mode = ota_basic_get_tws_role();
    LOG_D("Role is update to %d", ibrt_tws_mode);
}

// not tws mode or is master
static bool _tws_is_master(void)
{
#if defined(FREEMAN_ENABLED_STERO) || defined(FREEMAN_OTA_ENABLED)
    return true;
#endif
    return (OTA_BASIC_TWS_MASTER == ibrt_tws_mode || OTA_BASIC_TWS_UNKNOWN == ibrt_tws_mode);
}

// not tws mode or is slave
static POSSIBLY_UNUSED bool _tws_is_slave(void)
{
    return (OTA_BASIC_TWS_SLAVE == ibrt_tws_mode);
}

#else //IBRT
static void _tws_get_role(void)
{
    // not in tws mode
}

// not tws mode or is master
static bool _tws_is_master(void)
{
    return true;
}

// not tws mode or is slave
static bool _tws_is_slave(void)
{
    return true;
}

#endif //IBRT

void ota_send_rsp_handle(uint8_t typeCode, uint8_t *buff, uint16_t length, uint32_t cmd_dode)
{
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)

    if (_tws_is_master())
    {
        if (ota_control_env.isOneBudOTA || OTA_SET_ONE_BUD_UPGRADE_RSP == typeCode) {
            goto single_device_process;
        }

        LOG_D("master send ota info to slave");
        tws_ctrl_send_cmd(cmd_dode, (uint8_t *)buff, length);
        return;
    }
    else if (_tws_is_slave())
    {
        LOG_D("slave wait for ota info from master");
        ibrt_ota_cmd_type = typeCode;

        if ((receivedResultAlreadyProcessedBySlave.typeCode == 0xFF)
            || (receivedResultAlreadyProcessedBySlave.typeCode != typeCode))
        {
            LOG_D("slave wait for ota info from master!");
        }
        else
        {
            switch (typeCode)
            {
            case OTA_RSP_SEGMENT_VERIFY:
            {
                app_ibrt_ota_segment_crc_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                            receivedResultAlreadyProcessedBySlave.p_buff,
                                                            receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_START:
            {
                app_ibrt_ota_start_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                    receivedResultAlreadyProcessedBySlave.p_buff,
                                                    receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_CONFIG:
            {
                app_ibrt_ota_config_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                        receivedResultAlreadyProcessedBySlave.p_buff,
                                                        receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_RESULT:
            {
                app_ibrt_ota_image_crc_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                        receivedResultAlreadyProcessedBySlave.p_buff,
                                                        receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_VERSION:
            {
                app_ibrt_ota_get_version_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                            receivedResultAlreadyProcessedBySlave.p_buff,
                                                            receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_SIDE_SELECTION:
            {
                app_ibrt_ota_select_side_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                            receivedResultAlreadyProcessedBySlave.p_buff,
                                                            receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_IMAGE_APPLY:
            {
                app_ibrt_ota_image_overwrite_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                                receivedResultAlreadyProcessedBySlave.p_buff,
                                                                receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_GET_OTA_VERSION:
            {
                app_ibrt_ota_get_ota_version_cmd_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                            receivedResultAlreadyProcessedBySlave.p_buff,
                                                            receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            case OTA_RSP_SET_USER:
            {
                app_ibrt_ota_set_user_cmd_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                    receivedResultAlreadyProcessedBySlave.p_buff,
                                                    receivedResultAlreadyProcessedBySlave.length);
            }
            break;
            default:
            break;
            }
            receivedResultAlreadyProcessedBySlave.typeCode = 0XFF;
        }
    }
single_device_process:
    if (!ota_control_env.isOneBudOTA) {
        return;
    }
#endif

    switch (typeCode)
    {
    case OTA_RSP_SEGMENT_VERIFY:
    {
        OTA_REQ_RESULT_E ota_segment_crc = (OTA_REQ_RESULT_E)*buff;
        if (OTA_RESULT_PASSED == ota_segment_crc ||
            OTA_RESULT_ERR == ota_segment_crc)
        {
            ota_update_flash_offset_after_segment_crc((bool)ota_segment_crc);
            ota_control_send_segment_verification_response((bool)ota_segment_crc);
        }
        else
        {
            ota_control_send_result_response(ota_segment_crc);
        }
    }
    break;
    case OTA_RSP_START:
    {
        bool isViaBle = (bool)(*buff);
        ota_control_send_start_response(isViaBle);
    }
    break;
    case OTA_RSP_CONFIG:
    {
        OTA_REQ_RESULT_E ota_config = (OTA_REQ_RESULT_E)*buff;
        if (OTA_RESULT_PASSED == ota_config ||
            OTA_RESULT_ERR == ota_config)
        {
            ota_control_send_configuration_response((bool)ota_config);
        }
        else
        {
            ota_control_send_result_response(ota_config);
        }
    }
    break;
    case OTA_RSP_RESULT:
    {
        OTA_REQ_RESULT_E whole_image_crc = (OTA_REQ_RESULT_E)*buff;
        if (OTA_RESULT_ERR == whole_image_crc)
        {
            ota_control_reset_env(true);
        }
        ota_upgradeLog_destroy();
        ota_control_send_result_response(whole_image_crc);
    }
    break;
    case OTA_RSP_VERSION:
    {
        ota_control_send_version_rsp();
    }
    break;
    case OTA_RSP_SIDE_SELECTION:
    {
        ota_control_side_selection_rsp(true);
    }
    break;
    case OTA_RSP_IMAGE_APPLY:
    {
        OTA_REQ_RESULT_E overWrite = (OTA_REQ_RESULT_E)*buff;
        ota_control_image_apply_rsp((bool)overWrite);
        if (OTA_RESULT_PASSED == overWrite)
        {
            ota_apply();
        }
        else
        {
            Bes_exit_ota_state();
        }
    }
    break;
    case OTA_RSP_GET_OTA_VERSION:
    {
        _send_ota_version_rsp(otaVersion, ARRAY_SIZE(otaVersion));
    }
    break;
    case OTA_RSP_SET_USER:
    {
        _send_set_user_response(true);
    }
    break;
    case OTA_SET_ONE_BUD_UPGRADE_RSP:
    {
        ota_control_single_bud_upgrade_rsp(true);
    }
    break;
    default:
    break;
    }
}

void breakPoint_confirm(uint32_t breakPoint, uint8_t randomCode[])
{
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
    if (_tws_is_slave())
    {
        ibrt_ota_cmd_type = OTA_RSP_RESUME_VERIFY;
        twsBreakPoint = breakPoint;
        if(receivedResultAlreadyProcessedBySlave.typeCode == OTA_RSP_RESUME_VERIFY)
        {
            app_ibrt_ota_segment_crc_cmd_send_handler(receivedResultAlreadyProcessedBySlave.rsp_seq,
                                                receivedResultAlreadyProcessedBySlave.p_buff,
                                                receivedResultAlreadyProcessedBySlave.length);
        }
        else
        {
            LOG_D("slave wait for ota info from master");
        }
    }
    else if (_tws_is_master() && !ota_control_env.isOneBudOTA)
    {
        ibrt_tws_ota_breakPoint_resume(breakPoint, randomCode);
    }
    else
#endif
    {
        ota_control_send_resume_response(breakPoint, randomCode);
    }
}

static void ota_control_opera_callback(void* param)
{
    NORFLASH_API_OPERA_RESULT *opera_result;

    opera_result = (NORFLASH_API_OPERA_RESULT*)param;

    LOG_D("%s type:%d, addr:0x%x",
          __func__,
          opera_result->type,
          opera_result->addr);
    LOG_D("len:0x%x, result:%d.",
          opera_result->len,
          opera_result->result);
}

static void _init_flash_offset(void)
{
    LOG_D("[%s].",__func__);

    /// use firmware OTA as default user for compatibility
    if (BES_OTA_USER_INVALID == ota_control_env.currentUser)
    {
#ifdef COMBO_CUSBIN_IN_FLASH
        _update_user(BES_OTA_USER_COMBOFIRMWARE);
#else
        _update_user(BES_OTA_USER_FIRMWARE);
#endif
    }

    LOG_D("current OTA user:%s", _user2str(ota_control_env.currentUser));

    uint8_t i = _get_user_index(ota_control_env.currentUser);

    if (BES_OTA_USER_INDEX_INVALID != i)
    {
        /// set default value of flash related ota_control_env context
        ota_control_env.offsetInFlashToProgram = 0;
        ota_control_env.offsetInFlashOfCurrentSegment = 0;
        ota_control_env.dstFlashOffsetForNewImage = ota_control_env.userInfo[i].startAddr&0xFFFFFF;
    }
}

static void _calcute_segment_crc32(uint32_t offset)
{
    uint32_t lengthCrcCheckDone = 0;
    uint32_t lengthCrcCheckNow = 0;
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    uint8_t i = _get_user_index(ota_control_env.currentUser);

    uint32_t startFlashAddr = ota_control_env.dstFlashOffsetForNewImage - 
                            (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + 
                            ota_control_env.offsetInFlashOfCurrentSegment - offset;
    LOG_D("Calculate the crc32 of the segment, startAddr:0x%x, curSeg:0x%x, offset:0x%x",
            startFlashAddr, ota_control_env.offsetInFlashOfCurrentSegment, offset);

    uint32_t lengthToDoCrcCheck = ota_control_env.alreadyReceivedDataSizeOfImage -
                                  ota_control_env.offsetOfImageOfCurrentSegment;

    while (lengthToDoCrcCheck > lengthCrcCheckDone)
    {
        lengthCrcCheckNow = (lengthToDoCrcCheck - lengthCrcCheckDone) > OTA_DATA_BUFFER_SIZE_FOR_BURNING
                                ? OTA_DATA_BUFFER_SIZE_FOR_BURNING
                                : lengthToDoCrcCheck - lengthCrcCheckDone;
        // LOG_D("lengthCrcCheckNow 0x%x", lengthCrcCheckNow);

        app_flash_read(mod,
                       (startFlashAddr + lengthCrcCheckDone),
                       ota_control_env.dataBufferForBurning,
                       lengthCrcCheckNow);
        lengthCrcCheckDone += lengthCrcCheckNow;
        ota_control_env.crc32OfSegment = crc32_c(ota_control_env.crc32OfSegment,
                                                 ota_control_env.dataBufferForBurning,
                                                 lengthCrcCheckNow);
    }
}

static void _handle_received_data(uint8_t *otaBuf, bool isViaBle,uint16_t dataLenth)
{
    #define MAX_SET_VERIFY_RETRY    3
    static uint32_t seg_verify_retry = MAX_SET_VERIFY_RETRY;
    /// use the legacy manner parsing the received data on this layer
    OTA_LEGACY_PACKET_T *packet = (OTA_LEGACY_PACKET_T *)otaBuf;

    LOG_D("[%s],case = 0x%x%s", __func__,
          packet->packetType,
          typeCode2Str(packet->packetType));

    switch (packet->packetType)
    {
        case OTA_DATA_PACKET:
        {
            if (!ota_is_in_progress()
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
                || (OTA_BASIC_TWS_MASTER == ota_basic_get_tws_role() &&
                 !app_tws_ibrt_tws_link_connected())
#endif
                 )
            {
                ota_control_send_result_response(false);
                return;
            }

#ifdef COMBO_CUSBIN_IN_FLASH
            if (BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
            {
                _combo_bin_package_handler(otaBuf,isViaBle,dataLenth);
                return;
            }
#endif

            uint8_t* rawDataPtr = &otaBuf[1];
            uint32_t rawDataSize = dataLenth - 1;

            LOG_D("Received image data size %d", rawDataSize);

            uint32_t leftDataSize = rawDataSize;
            uint32_t offsetInReceivedRawData = 0;
            do
            {
                uint32_t bytesToCopy;
                // copy to data buffer
                if ((ota_control_env.offsetInDataBufferForBurning + leftDataSize) >
                    OTA_DATA_BUFFER_SIZE_FOR_BURNING)
                {
                    bytesToCopy = OTA_DATA_BUFFER_SIZE_FOR_BURNING - ota_control_env.offsetInDataBufferForBurning;
                }
                else
                {
                    bytesToCopy = leftDataSize;
                }

                leftDataSize -= bytesToCopy;

                memcpy(&ota_control_env.dataBufferForBurning[ota_control_env.offsetInDataBufferForBurning],
                        &rawDataPtr[offsetInReceivedRawData], bytesToCopy);
                offsetInReceivedRawData += bytesToCopy;
                ota_control_env.offsetInDataBufferForBurning += bytesToCopy;
                LOG_D("offsetInDataBufferForBurning is %d", ota_control_env.offsetInDataBufferForBurning);
                if (OTA_DATA_BUFFER_SIZE_FOR_BURNING <= ota_control_env.offsetInDataBufferForBurning)
                {
                    LOG_D("Program the image to flash.");

                    #if (IMAGE_RECV_FLASH_CHECK == 1)
                    if((ota_control_env.offsetInFlashToProgram > ota_control_env.totalImageSize) ||
                        (ota_control_env.totalImageSize > MAX_IMAGE_SIZE) ||
                        (ota_control_env.offsetInFlashToProgram & (MIN_SEG_ALIGN - 1)))
                    {
                        LOG_D("ERROR: IMAGE_RECV_FLASH_CHECK");
                        LOG_D(" ota_control_env(.offsetInFlashToProgram >= .totalImageSize)");
                        LOG_D(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                        LOG_D(" or .offsetInFlashToProgram isn't segment aligned");
                        LOG_D(".offsetInFlashToProgram:0x%x, .dstFlashOffsetForNewImage:0x%x, .totalImageSize:%d",
                              ota_control_env.offsetInFlashToProgram,
                              ota_control_env.dstFlashOffsetForNewImage,
                              ota_control_env.totalImageSize);

                        /// In order to reduce unnecessary erasures and retransmissions we don't imeediately destory
                        /// the log but reset ota, because a boundary check is performed before flashing and if there
                        ///  is really wrong we'll catch when an image CRC32 check finally.
                        // ota_upgradeLog_destroy();
                        ota_control_send_result_response(OTA_RESULT_ERR_FLASH_OFFSET);
                        return;
                    }
                    #endif
                    if (ota_control_env.offsetInDataBufferForBurning)
                    {
                        ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                                        OTA_DATA_BUFFER_SIZE_FOR_BURNING,
                                                        ota_control_env.offsetInFlashToProgram);
                        ota_control_env.offsetInFlashToProgram += OTA_DATA_BUFFER_SIZE_FOR_BURNING;
                        ota_control_env.offsetInDataBufferForBurning = 0;
                    }
                }
            } while (offsetInReceivedRawData < rawDataSize);

            ota_control_env.alreadyReceivedDataSizeOfImage += rawDataSize;
            LOG_D("Image already received %d", ota_control_env.alreadyReceivedDataSizeOfImage);

            #if (IMAGE_RECV_FLASH_CHECK == 1)
            if((ota_control_env.alreadyReceivedDataSizeOfImage > ota_control_env.totalImageSize) ||
                (ota_control_env.totalImageSize > MAX_IMAGE_SIZE))
            {
                LOG_D("ERROR: IMAGE_RECV_FLASH_CHECK");
                LOG_D(" ota_control_env(.alreadyReceivedDataSizeOfImage > .totalImageSize)");
                LOG_D(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                LOG_D(".alreadyReceivedDataSizeOfImage:%d, .totalImageSize:%d",
                      ota_control_env.alreadyReceivedDataSizeOfImage,
                      ota_control_env.totalImageSize);

                /// In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log
                /// but reset ota, because a boundary check is performed before flashing and if there is really wrong
                /// we'll catch when an image CRC32 check finally.
                //ota_upgradeLog_destroy();
                ota_control_send_result_response(OTA_RESULT_ERR_RECV_SIZE);
                return;
            }
#endif

#if DATA_ACK_FOR_SPP_DATAPATH_ENABLED
            if (DATA_PATH_SPP == ota_control_env.dataPathType)
            {
                ota_control_send_data_ack_response();
            }
#endif
            break;
        }
        case OTA_COMMAND_SEGMENT_VERIFY:
        {
            if(!app_get_bes_ota_state())
            {
                break;
            }
            ota_control_env.receivedCrc32OfSegment = packet->payload.cmdSegmentVerify.crc32OfSegment;

            if((ota_control_env.offsetInDataBufferForBurning == 0) &&
                (ota_control_env.alreadyReceivedDataSizeOfImage == ota_control_env.offsetOfImageOfCurrentSegment))
            {
                //there is no new data, it is app resend 82 cmd
                LOG_D("resend 82 cmd, CRC32 of the segement:0x%x, received num:0x%x",
                    ota_control_env.crc32OfSegment,
                    ota_control_env.receivedCrc32OfSegment);
                if((BES_OTA_START_MAGIC_CODE == packet->payload.cmdSegmentVerify.magicCode) &&
                    (ota_control_env.receivedCrc32OfSegment == ota_control_env.crc32OfSegment))
                {
                    ota_control_send_segment_verification_response(true);
                }
                else
                {
                    ota_control_send_segment_verification_response(false);
                }
                break;
            }
#if (IMAGE_RECV_FLASH_CHECK == 1)
            if((ota_control_env.offsetInFlashToProgram > ota_control_env.totalImageSize) ||
                (ota_control_env.totalImageSize > MAX_IMAGE_SIZE) ||
                (ota_control_env.offsetInFlashToProgram & (MIN_SEG_ALIGN - 1)))
            {
                LOG_D("ERROR: IMAGE_RECV_FLASH_CHECK");
                LOG_D(" ota_control_env(.offsetInFlashToProgram - .dstFlashOffsetForNewImage >= .totalImageSize)");
                LOG_D(" or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                LOG_D(" or .offsetInFlashToProgram isn't segment aligned");
                LOG_D(".offsetInFlashToProgram:0x%x, .dstFlashOffsetForNewImage:0x%x, .totalImageSize:%d",
                      ota_control_env.offsetInFlashToProgram,
                      ota_control_env.dstFlashOffsetForNewImage,
                      ota_control_env.totalImageSize);

                /// In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log
                /// but reset ota, because a boundary check is performed before flashing and if there is really wrong
                /// we'll catch when an image CRC32 check finally.
                errOtaCode = OTA_RESULT_ERR_FLASH_OFFSET;
                ota_send_rsp_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&errOtaCode, 1, APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);
                return;
            }
#endif
            if (ota_control_env.offsetInDataBufferForBurning)
            {
                ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                                ota_control_env.offsetInDataBufferForBurning,
                                                ota_control_env.offsetInFlashToProgram);
                ota_control_env.offsetInFlashToProgram += ota_control_env.offsetInDataBufferForBurning;
                ota_control_env.offsetInDataBufferForBurning = 0;
            }
            ota_control_env.crc32OfSegment = 0;
#ifdef COMBO_CUSBIN_IN_FLASH
            if(BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
            {
                _combo_bin_calc_crc32();
            }
            else
#endif
            {
                _calcute_segment_crc32(0);
            }

            LOG_D("CRC32 of the segement:0x%x, received num:0x%x",
                  ota_control_env.crc32OfSegment,
                  ota_control_env.receivedCrc32OfSegment);
            if ((BES_OTA_START_MAGIC_CODE == packet->payload.cmdSegmentVerify.magicCode) &&
                (ota_control_env.receivedCrc32OfSegment == ota_control_env.crc32OfSegment))
            {
                uint8_t ota_segment_crc = OTA_RESULT_PASSED;
                errOtaCode = 1;
                ota_send_rsp_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);
                ota_status_change(true);
                seg_verify_retry = MAX_SET_VERIFY_RETRY;
            }
            else
            {
                if(--seg_verify_retry == 0)
                {
                    seg_verify_retry = MAX_SET_VERIFY_RETRY;

                    LOG_D("ERROR: segment verification retry too much!");
                    ota_upgradeLog_destroy();  // Yes, destory it and retransmit the entire image.
                    errOtaCode = OTA_RESULT_ERR_SEG_VERIFY;
                    ota_send_rsp_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&errOtaCode, 1, APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);
                    return;
                }

                uint8_t ota_segment_crc = OTA_RESULT_ERR;
                ota_send_rsp_handle(OTA_RSP_SEGMENT_VERIFY, (uint8_t *)&ota_segment_crc, 1, APP_TWS_CMD_OTA_SEGMENT_CRC_CMD);
            }

            // reset the data buffer
            LOG_D("total size is %d already received %d",
                  ota_control_env.totalImageSize,
                  ota_control_env.alreadyReceivedDataSizeOfImage);

            break;
        }
        case OTA_COMMAND_START:
        {
#ifdef IBRT
            ibrt_ota_cmd_type = 0;
#endif
            if (BES_OTA_START_MAGIC_CODE == packet->payload.cmdOtaStart.magicCode)
            {
                LOG_D("Receive command start request:");
                ota_control_reset_env(false);
                /// init flash offset to write data
                _init_flash_offset();

                ota_control_env.totalImageSize = packet->payload.cmdOtaStart.imageSize;
                ota_control_env.crc32OfImage = packet->payload.cmdOtaStart.crc32OfImage;
                ota_status_change(true);

                seg_verify_retry = MAX_SET_VERIFY_RETRY;
                LOG_D("Image size is 0x%x, crc32 of image is 0x%x",
                      ota_control_env.totalImageSize,
                      ota_control_env.crc32OfImage);

                ota_update_start_message();

                ota_send_rsp_handle(OTA_RSP_START, (uint8_t *)&isViaBle, 1, APP_TWS_CMD_OTA_START_OTA_CMD);
                ota_start();
            }
            break;
        }
        case OTA_COMMAND_RESUME_VERIFY:
        {
            /// init flash offset to write data
            _init_flash_offset();
            ota_control_set_packet_interval();
            Bes_enter_ota_state();

            if (BES_OTA_START_MAGIC_CODE == packet->payload.cmdResumeBreakpoint.magicCode)
            {
                uint32_t breakPoint;
                uint8_t *randomCode = NULL;
                uint8_t zeroCode[32] = {0};

                if (ota_control_env.dataBufferForBurning == NULL) {
                    LOG_D("%s:randomCode buffer is NULL,reset buffer here!", __func__);

                    ota_control_env.dataBufferForBurning = app_ota_get_common_databuf();
                }

                randomCode = ota_control_env.dataBufferForBurning;
                memset(randomCode, 0, sizeof(otaUpgradeLog.randomCode));

                LOG_D("Receive command resuming verification:");
                if (packet->payload.cmdResumeBreakpoint.crc32 !=
                    crc32_c(0, packet->payload.cmdResumeBreakpoint.randomCode,
                          sizeof(packet->payload.cmdResumeBreakpoint.randomCode) +
                              sizeof(packet->payload.cmdResumeBreakpoint.segmentSize)))
                {
                    LOG_D("Received crc32 0x%x, cal 0x%x, segSize %d",
                          packet->payload.cmdResumeBreakpoint.crc32,
                          (uint32_t)crc32_c(0, packet->payload.cmdResumeBreakpoint.randomCode,
                                            sizeof(packet->payload.cmdResumeBreakpoint.randomCode) +
                                                sizeof(packet->payload.cmdResumeBreakpoint.segmentSize)),
                          (uint32_t)packet->payload.cmdResumeBreakpoint.segmentSize);
                    breakPoint = -1;
                    LOG_D("Resuming verification crc32 check fail.");
                    goto resume_response;
                }

                LOG_D("Resuming verification crc32 check pass.");

                LOG_D("Receive segment size 0x%x.", packet->payload.cmdResumeBreakpoint.segmentSize);

                LOG_D("Receive random code:");
                DUMP8("%02x ", packet->payload.cmdResumeBreakpoint.randomCode, sizeof(otaUpgradeLog.randomCode));

                LOG_D("Device's random code:");
                DUMP8("%02x ", otaUpgradeLog.randomCode, sizeof(otaUpgradeLog.randomCode));

                breakPoint = get_upgradeSize_log();

#ifdef COMBO_CUSBIN_IN_FLASH
                if(breakPoint)
                    breakPoint = ota_bin_breakpoint_update(breakPoint);
#endif
                if(breakPoint & (MIN_SEG_ALIGN - 1))  // Minimum segment alignment.
                {
                    LOG_D("Breakpoint:0x%x isn't minimum segment alignment!", breakPoint);
                    ota_upgradeLog_destroy();  // Error in log, we'd better try to retransmit the entire image.
                    #if 1
                    breakPoint = get_upgradeSize_log();
                    #else
                    ota_control_send_result_response(OTA_RESULT_ERR_BREAKPOINT);
                    return;
                    #endif
                }

                if (breakPoint)
                {
                    if (!memcmp(otaUpgradeLog.randomCode, packet->payload.cmdResumeBreakpoint.randomCode,
                    sizeof(otaUpgradeLog.randomCode)) && memcmp(otaUpgradeLog.randomCode, zeroCode, sizeof(zeroCode)))
                    {
                        ota_control_reset_env(false);
                        ota_status_change(true);
                        ota_get_start_message();
                        LOG_D("OTA can resume. Resuming from the breakpoint at: 0x%x.", breakPoint);
                    }
                    else
                    {
                        LOG_D("OTA can't resume because the randomCode is inconsistent. [breakPoint: 0x%x]", breakPoint);

                        breakPoint = ota_control_env.breakPoint = ota_control_env.resume_at_breakpoint = 0;
                    }
                }
                // not in tws mode or is slave
                if (breakPoint == 0 && _tws_is_master())
                {
                    LOG_D("OTA resume none. Generate new random code for the new transmisson now. [breakPoint: 0x%x]", breakPoint);
                    for(uint32_t i = 0; i < sizeof(otaUpgradeLog.randomCode); i++)
                    {
                        set_rand_seed(hal_sys_timer_get());
                        randomCode[i] = get_rand();
                        DelayMs(1);
                    }

                    ota_randomCode_log(randomCode);

                    LOG_D("New random code:");
                    DUMP8("%02x ", randomCode, sizeof(otaUpgradeLog.randomCode));
                }

            resume_response:
                if(breakPoint == 0)
                {
                    breakPoint_confirm(breakPoint, randomCode);
                }
                else
                {
                    breakPoint_confirm(breakPoint, packet->payload.cmdResumeBreakpoint.randomCode);
                }
            }
            break;
        }
        case OTA_COMMAND_CONFIG_OTA:
        {
            uint32_t cfg_len = 0;
            OTA_FLOW_CONFIGURATION_T* ptConfig = (OTA_FLOW_CONFIGURATION_T *)&(ota_control_env.configuration);
            cfg_len = dataLenth - 1;
            ASSERT(cfg_len==sizeof(OTA_FLOW_CONFIGURATION_T), "plaese check config len. %d != %d",\
                    cfg_len, sizeof(OTA_FLOW_CONFIGURATION_T));
            memcpy((uint8_t *)ptConfig, &packet->payload.cmdFlowControl.config, cfg_len);

            if ((ptConfig->lengthOfFollowingData + 4) <= cfg_len)
            {
                uint32_t localCrc = crc32_c(0, (uint8_t *)ptConfig, sizeof(OTA_FLOW_CONFIGURATION_T) - sizeof(uint32_t));

                LOG_D("lengthOfFollowingData 0x%x", ptConfig->lengthOfFollowingData);
                LOG_D("startLocationToWriteImage 0x%x", ptConfig->startLocationToWriteImage);
                LOG_D("isToClearUserData %d", ptConfig->isToClearUserData);
                LOG_D("isToRenameBT %d", ptConfig->isToRenameBT);
                LOG_D("isToRenameBLE %d", ptConfig->isToRenameBLE);
                LOG_D("isToUpdateBTAddr %d", ptConfig->isToUpdateBTAddr);
                LOG_D("isToUpdateBLEAddr %d", ptConfig->isToUpdateBLEAddr);
                LOG_D("New BT name:");
                DUMP8("0x%02x ", ptConfig->newBTName, BES_OTA_NAME_LENGTH);
                LOG_D("New BLE name:");
                DUMP8("0x%02x ", ptConfig->newBLEName, BES_OTA_NAME_LENGTH);
                LOG_D("New BT addr:");
                DUMP8("0x%02x ", ptConfig->newBTAddr, BT_ADDR_OUTPUT_PRINT_NUM);
                LOG_D("New BLE addr:");
                DUMP8("0x%02x ", ptConfig->newBLEAddr, BT_ADDR_OUTPUT_PRINT_NUM);
                LOG_D("crcOfConfiguration:0x%x", ptConfig->crcOfConfiguration);
                LOG_D("local calculated crc:0x%x", localCrc);

                if(ota_control_env.configuration.startLocationToWriteImage != ota_control_env.offsetInFlashToProgram)
                {
                    LOG_D("APP Start Location is unused");
                }

                // check CRC
                if (ptConfig->crcOfConfiguration == localCrc)
                {
                    if(ota_control_env.totalImageSize > MAX_IMAGE_SIZE)
                    {
                        LOG_D("ImageSize 0x%x greater than 0x%x! Terminate the upgrade.",
                              ota_control_env.totalImageSize, MAX_IMAGE_SIZE);
                        errOtaCode = OTA_RESULT_ERR_IMAGE_SIZE;
                        ota_send_rsp_handle(OTA_RSP_CONFIG, (uint8_t *)&errOtaCode, 1, APP_TWS_CMD_OTA_OTA_CONFIG_CMD);
                        return;
                    }

                    if(ota_control_env.breakPoint > ota_control_env.totalImageSize)
                    {
                        ota_upgradeLog_destroy(); // Error in log, we'd better try to retransmit the entire image.
                        errOtaCode = OTA_RESULT_ERR_BREAKPOINT;
                        ota_send_rsp_handle(OTA_RSP_CONFIG, (uint8_t *)&errOtaCode, 1, APP_TWS_CMD_OTA_OTA_CONFIG_CMD);
                        return;
                    }

                    if(ota_control_env.resume_at_breakpoint == true)
                    {
                        ota_control_env.alreadyReceivedDataSizeOfImage = ota_control_env.breakPoint;
                        ota_control_env.offsetOfImageOfCurrentSegment = ota_control_env.alreadyReceivedDataSizeOfImage;
                        ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetOfImageOfCurrentSegment;
                        ota_control_env.offsetInFlashToProgram = ota_control_env.offsetInFlashOfCurrentSegment;
                        #ifdef COMBO_CUSBIN_IN_FLASH
                        _combo_bin_breakpoint_recovery(ota_control_env.alreadyReceivedDataSizeOfImage);
                        #endif
                    }

                    LOG_D("OTA config pass.");
                    LOG_D("Start writing the received image to flash offset 0x%x", ota_control_env.offsetInFlashToProgram);

                    errOtaCode = 1;
                    bool ota_config = OTA_RESULT_PASSED;
                    ota_send_rsp_handle(OTA_RSP_CONFIG, (uint8_t *)&ota_config, 1, APP_TWS_CMD_OTA_OTA_CONFIG_CMD);
                }
                else
                {
                    LOG_D("OTA config failed.");

                    /// inform APP OTA configure failed, APP should abort the OTA progress immediately
                    bool ota_config = OTA_RESULT_ERR;
                    ota_send_rsp_handle(OTA_RSP_CONFIG, (uint8_t *)&ota_config, 1, APP_TWS_CMD_OTA_OTA_CONFIG_CMD);
                }
            }
            break;
        }
        case OTA_COMMAND_GET_OTA_RESULT:
        {
            // check whether all image data have been received
            if (ota_control_env.alreadyReceivedDataSizeOfImage == ota_control_env.totalImageSize)
            {
                ota_data_reception_done();
            }
            else
            {
                bool whole_image_crc = OTA_RESULT_ERR;
                ota_send_rsp_handle(OTA_RSP_RESULT, (uint8_t *)&whole_image_crc, 1, APP_TWS_CMD_OTA_IMAGE_CRC_CMD);
            }
            break;
        }
        case OTA_COMMAND_GET_VERSION:
        {
            hal_norflash_disable_protection(HAL_FLASH_ID_0);

            if (BES_OTA_START_MAGIC_CODE == packet->payload.cmdGetVision.magicCode)
            {
                uint8_t fw_rev[4];
                #ifdef FIRMWARE_REV
                system_get_info(&fw_rev[0], &fw_rev[1], &fw_rev[2], &fw_rev[3]);
                #endif
                DUMP8("%02x ", fw_rev, ARRAY_SIZE(fw_rev));
                ota_send_rsp_handle(OTA_RSP_VERSION, fw_rev, ARRAY_SIZE(fw_rev), APP_TWS_CMD_OTA_GET_VERSION_CMD);
            }
            else
            {
                LOG_D("invalid magic code:%x", packet->payload.cmdGetVision.magicCode);
            }
            break;
        }
        case OTA_COMMAND_SIDE_SELECTION:
        {
            uint8_t selectSide[2] = {0x91, 0x01};
            ota_send_rsp_handle(OTA_RSP_SIDE_SELECTION, selectSide, ARRAY_SIZE(selectSide), APP_TWS_CMD_OTA_SELECT_SIDE_CMD);
            break;
        }
        case OTA_COMMAND_IMAGE_APPLY:
        {
            if (BES_OTA_START_MAGIC_CODE == packet->payload.cmdApply.magicCode && ota_control_env.isPendingForReboot)
            {
                ota.permissionToApply = 1;
                bool overWrite = OTA_RESULT_PASSED;
                errOtaCode = 1;
                ota_send_rsp_handle(OTA_RSP_IMAGE_APPLY, (uint8_t *)&overWrite, 1, APP_TWS_CMD_OTA_IMAGE_OVERWRITE_CMD);
            }
            else
            {
                ota.permissionToApply = 0;
                bool overWrite = OTA_RESULT_ERR;
                ota_send_rsp_handle(OTA_RSP_IMAGE_APPLY, (uint8_t *)&overWrite, 1, APP_TWS_CMD_OTA_IMAGE_OVERWRITE_CMD);
            }
            break;
        }

        /*****************************************************************************/
        //                      OTA protocol version: 0.0.0.1                        //
        /*****************************************************************************/
        case OTA_CMD_GET_OTA_VERSION:
            /// update local error code to no_error
            errOtaCode = 1;

            /// TWS sync command from phone
            ota_send_rsp_handle(OTA_RSP_GET_OTA_VERSION,
                                otaVersion,
                                ARRAY_SIZE(otaVersion),
                                APP_TWS_CMD_OTA_GET_OTA_VERSION_CMD);
            break;
        case OTA_CMD_SET_USER:
            /// update OTA user
            _update_user(packet->payload.cmdSetUser.user);

            /// update local error code to no_error
            errOtaCode = 1;

            /// TWS sync command from phone
            ota_send_rsp_handle(OTA_RSP_SET_USER,
                                &(packet->payload.cmdSetUser.user),
                                sizeof(packet->payload.cmdSetUser.user),
                                APP_TWS_CMD_OTA_SET_USER_CMD);
            break;
        case OTA_SET_ONE_BUD_UPGRADE_CMD:
        {
            ota_control_env.isOneBudOTA = packet->payload.cmdOneBudSelect.selectOneBud;
            ota_send_rsp_handle(OTA_SET_ONE_BUD_UPGRADE_RSP, NULL, 0, 0);   // this cmd needn't send to tws-slave
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
            if (app_tws_ibrt_tws_link_connected() && ota_control_env.isOneBudOTA)
            {
                ota_control_send_result_response(false);
                ota_control_env.isOneBudOTA = 0;
                return;
            }
#endif
            break;
        }
        default:
            if (packet->packetType < 0x70 || packet->packetType > 0x9F)
            {
                TRACE(0,"[%s] UNKNOWN CMD, EXIT OTA", __func__);

                // if (_tws_is_master())
                // {
                //     ota_control_send_result_response(OTA_RESULT_ERR_RECV_SIZE);
                // }
                // Bes_exit_ota_state();
            }
    }
}

#if defined(IBRT) && defined(__IAG_BLE_INCLUDE__) && !defined(FREEMAN_OTA_ENABLED)
static void _relay_ota_data(uint8_t *otaBuf, uint16_t len)
{
    // TODO: enable gatt over br/edr for new ble stack

        tws_ctrl_send_cmd(APP_TWS_CMD_OTA_IMAGE_BUFF, otaBuf, len);
}
#endif

static void _packet_structure_convert(uint8_t *otaBuf, bool isViaBle, uint16_t len)
{
    /// remove the length field in the packet to fit the legacy command parser
    if (OTA_PACKET_STRUCTURE_TLV == ota_control_env.packetStructureType)
    {
        static uint8_t extractedOtaRequestBuf[BES_OTA_BT_DATA_PACKET_MAX_SIZE];
        static bool isOnceInitOtaCmdQueue = false;
        if (!isOnceInitOtaCmdQueue)
        {
            InitCQueue(&otaCmdQueue, sizeof(accumulatedOtaBuf), accumulatedOtaBuf);
            isOnceInitOtaCmdQueue = true;
        }
        uint16_t cmdLength = len;
        EnCQueue(&otaCmdQueue, otaBuf, cmdLength);
        uint8_t cmdHeader[OTA_PACKET_LENGTH_HEADER_LEN];
        uint16_t queueLeftDataLen = LengthOfCQueue(&otaCmdQueue);
        while (queueLeftDataLen >= OTA_PACKET_LENGTH_HEADER_LEN)
        {
            PeekCQueueToBuf(&otaCmdQueue, cmdHeader, OTA_PACKET_LENGTH_HEADER_LEN);
            OTA_PACKET_T *packet = (OTA_PACKET_T *)cmdHeader;
            uint16_t payloadLen = packet->length;
            if((payloadLen + OTA_PACKET_LENGTH_HEADER_LEN) > BES_OTA_BT_DATA_PACKET_MAX_SIZE)
            {
                //packet error,  discard all the msg.
                InitCQueue(&otaCmdQueue, sizeof(accumulatedOtaBuf), accumulatedOtaBuf);
                break;
            }
            if ((queueLeftDataLen - OTA_PACKET_LENGTH_HEADER_LEN) >= payloadLen)
            {
                DeCQueue(&otaCmdQueue, extractedOtaRequestBuf, payloadLen + OTA_PACKET_LENGTH_HEADER_LEN);
                queueLeftDataLen = LengthOfCQueue(&otaCmdQueue);
                extractedOtaRequestBuf[OTA_PACKET_LENGTH_FIELD_LEN] = extractedOtaRequestBuf[0];
                cmdLength = payloadLen + sizeof(packet->packetType);
                //LOG_D("received len:%d, data:", cmdLength);
                //DUMP8("0x%02x ", extractedOtaRequestBuf, (cmdLength > 20 ? 20: cmdLength));
                _handle_received_data(&extractedOtaRequestBuf[OTA_PACKET_LENGTH_FIELD_LEN], isViaBle, cmdLength);
            }
            else
            {
                break;
            }
        }
    }
    else if (OTA_PACKET_STRUCTURE_LEGACY == ota_control_env.packetStructureType)
    {
        /// do nothing
    }
    else
    {
        /// sanity check for pacekt_structure
        ASSERT(0, "INVALID packet structure");
    }
}

void ota_control_handle_received_data(uint8_t *otaBuf, bool isViaBle, uint16_t len)
{
    uint16_t length = len;
    LOG_D("[%s]type:0x%x, len:%d", __func__, otaBuf[0], length);

    /// get role for OTA progress
    _tws_get_role();

    /// NOTE: if APP support OTA_CMD_GET_OTA_VERSION or OTA_SET_ONE_BUD_UPGRADE_CMD, then it must use TLV packet structure
    if (OTA_CMD_GET_OTA_VERSION == otaBuf[0] || OTA_SET_ONE_BUD_UPGRADE_CMD == otaBuf[0])
    {
        ota_control_env.packetStructureType = OTA_PACKET_STRUCTURE_TLV;
        LOG_D("OTA packet type update to tlv");
    }

    if (_tws_is_master())
    {
#ifdef __IAG_BLE_INCLUDE__
#if defined(IBRT) && !defined(FREEMAN_OTA_ENABLED)
        /// relay receviced cmd/data to peer
        if (isViaBle)
        {
            _relay_ota_data(otaBuf, len);
        }
#endif
#endif
    }

    /// convert packet type into legacy manner
    if (ota_control_env.packetStructureType == OTA_PACKET_STRUCTURE_TLV)
    {
        _packet_structure_convert(otaBuf, isViaBle, length);
    }
    else
    {
        /// handle the received cmd/data locally
        _handle_received_data(otaBuf, isViaBle, length);
    }

}

static void app_update_magic_number_of_app_image(uint32_t newMagicNumber)
{
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);
    uint8_t i = _get_user_index(ota_control_env.currentUser);
    uint32_t flashOffset = ota_control_env.dstFlashOffsetForNewImage - (ota_control_env.userInfo[i].startAddr&0xFFFFFF);

    app_flash_read(mod,
                   flashOffset,
                   ota_control_env.dataBufferForBurning,
                   FLASH_SECTOR_SIZE_IN_BYTES);

    *(uint32_t *)ota_control_env.dataBufferForBurning = newMagicNumber;

    app_flash_sector_erase(mod, flashOffset);
    app_flash_program(mod,
                      flashOffset,
                      ota_control_env.dataBufferForBurning,
                      FLASH_SECTOR_SIZE_IN_BYTES,
                      true);

    LOG_D("%s flash_offset:0x%x, data:0x%x",
            __func__, flashOffset, *(uint32_t *)ota_control_env.dataBufferForBurning);
}

uint32_t app_get_boot_word(void)
{
    if (ota_basic_is_remap_enabled())
    {
        FLASH_OTA_BOOT_INFO_T otaBootInfo;
        app_flash_read(NORFLASH_API_MODULE_ID_BOOTUP_INFO, 0, (uint8_t*)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));
        LOG_D("%s boot word 0x%x", __func__, otaBootInfo.boot_word);
        return otaBootInfo.boot_word;
    }
    else
    {
        return 0;
    }
}

POSSIBLY_UNUSED static void app_update_ota_boot_info(void)
{
    FLASH_OTA_BOOT_INFO_T otaBootInfo;
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_BOOTUP_INFO;

    app_flash_read(NORFLASH_API_MODULE_ID_BOOTUP_INFO, 0, (uint8_t*)&otaBootInfo, sizeof(FLASH_OTA_BOOT_INFO_T));

    otaBootInfo.magicNumber = COPY_NEW_IMAGE;
    otaBootInfo.imageCrc = ota_control_env.crc32OfImage;
    otaBootInfo.imageSize = ota_control_env.totalImageSize;

    if (ota_basic_is_remap_enabled())
    {
        LOG_D("%s boot_word 0x%x", __func__, otaBootInfo.boot_word);
        if (otaBootInfo.boot_word == BOOT_WORD_B)
        {
            otaBootInfo.boot_word = BOOT_WORD_A;
            otaBootInfo.newImageFlashOffset = ota_control_env.dstFlashOffsetForNewImage;
        }
        else
        {
            otaBootInfo.boot_word = BOOT_WORD_B;
            otaBootInfo.newImageFlashOffset = ota_basic_get_remap_offset();
        }
    }
    else
    {
        otaBootInfo.newImageFlashOffset = ota_control_env.dstFlashOffsetForNewImage;
    }

    TRACE(0, "new image flash offset 0x%x", otaBootInfo.newImageFlashOffset);

    // LOG_D("%s magic 0x%x addr 0x%x", __func__, otaBootInfo->magicNumber, OTA_INFO_IN_OTA_BOOT_SEC);
    app_flash_sector_erase(mod,0);
    app_flash_program(mod,
                      0,
                      (uint8_t *)&otaBootInfo,
                      sizeof(FLASH_OTA_BOOT_INFO_T),
                      true);

}

void ota_control_register_ota_user(BES_OTA_USER_INFO_T info)
{
    bool alreadyExist = false;

    for (uint8_t i = 0; i < ota_control_env.userNum; i++)
    {
        if (ota_control_env.userInfo[i].user == info.user)
        {
            alreadyExist = true;
            ota_control_env.userInfo[i] = info;
            break;
        }
    }

    if (!alreadyExist)
    {
        ASSERT(BES_OTA_USER_NUM > ota_control_env.userNum, "OTA user number exceeds");
        ota_control_env.userInfo[ota_control_env.userNum] = info;
        ota_control_env.userNum++;
        LOG_D("current supported OTA user number:%d", ota_control_env.userNum);
        LOG_D("info.norflashId:%d", info.norflashId);
        LOG_D("app_flash_get_dev_id_by_addr(info.startAddr):%d", app_flash_get_dev_id_by_addr(info.startAddr));
#if defined(PROMPT_IN_FLASH)
        if(BES_OTA_USER_LANGUAGE_PACKAGE != info.user)
#endif
        {
            app_flash_register_module(info.norflashId, app_flash_get_dev_id_by_addr(info.startAddr), info.startAddr, info.length, (uint32_t)ota_control_opera_callback);
        }
    }
}

void ota_start(void)
{
    uint8_t i = _get_user_index(ota_control_env.currentUser);

    if (BES_OTA_USER_INDEX_INVALID != i && ota_control_env.userInfo[i].start)
    {
        ota_control_env.userInfo[i].start();
    }
    else
    {
        LOG_W("START callback for %s is null",
              _user2str(ota_control_env.currentUser));
    }
}

void ota_data_check_image_crc_handler(bool ret)
{
    errOtaCode = ret;
    if (ret)
    {
        LOG_D("Whole image verification pass.");
        ota_control_env.isPendingForReboot = true;
        ota_send_rsp_handle(OTA_RSP_RESULT, (uint8_t *)&ret, 1, APP_TWS_CMD_OTA_IMAGE_CRC_CMD);
    }
    else
    {
        LOG_D("Whole image verification failed.");
        ibrt_ota_send_result_response(false);
        Bes_exit_ota_state();
        ota_send_rsp_handle(OTA_RSP_RESULT, (uint8_t *)&ret, 1, APP_TWS_CMD_OTA_IMAGE_CRC_CMD);
    }
}

void ota_data_reception_done(void)
{
    LOG_D("%s", __func__);

    // flush the remaining data to flash
    if (0 != ota_control_env.offsetInDataBufferForBurning)
    {
        ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                        ota_control_env.offsetInDataBufferForBurning,
                                        ota_control_env.offsetInFlashToProgram);
    }

    if (BES_OTA_USER_FIRMWARE == ota_control_env.currentUser)
    {
        bool Ret = ota_check_image_data_sanity_crc();
        if (Ret)
        {
            // update the magic code of the application image
            app_update_magic_number_of_app_image(NORMAL_BOOT);
        }
        else
        {
            LOG_D("data sanity crc failed.");
            ota_data_check_image_crc_handler(Ret);
            return;
        }

        // check the crc32 of the image
        Ret = ota_control_check_image_crc();
        ota_data_check_image_crc_handler(Ret);
    }
    else
    {
#ifdef IBRT
            errOtaCode = 1;
#endif
        /// set pending for apply flag
        ota_control_env.isPendingForReboot = true;

        uint8_t i = _get_user_index(ota_control_env.currentUser);
        if (BES_OTA_USER_INDEX_INVALID != i && ota_control_env.userInfo[i].reception_done)
        {
            ota_control_env.userInfo[i].reception_done();
        }
    }
}

void ota_apply(void)
{
    // uint32_t _boot_word = app_get_boot_word();
    // LOG_D("[%s] _boot_word 0x%x",__func__, _boot_word);

    uint8_t i = _get_user_index(ota_control_env.currentUser);

#if defined(CHIP_BEST1501) && defined(COMBO_CUSBIN_IN_FLASH)
    if(BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
    {
        ota_control_env.dstFlashOffsetForNewImage = NEW_IMAGE_FLASH_OFFSET;
    }
#endif

    if (BES_OTA_USER_INDEX_INVALID != i && ota_control_env.userInfo[i].apply)
    {
        ota_control_env.userInfo[i].apply(ota_control_env.dstFlashOffsetForNewImage,
                                          ota_control_env.totalImageSize);
    }
    else
    {
        LOG_W("APPLY callback for %s is null",
              _user2str(ota_control_env.currentUser));
    }
}

void ota_control_update_flash_offset(uint32_t addr)
{
    /// set default value of flash related ota_control_env context
    ota_control_env.offsetInFlashToProgram = 0;
    ota_control_env.offsetInFlashOfCurrentSegment = 0;
    ota_control_env.dstFlashOffsetForNewImage = addr;
    LOG_D("[%s]start address 0x%x",__func__, addr);
}

uint8_t app_get_bes_ota_state(void)
{
    return isInBesOtaState;
}

bool app_check_is_ota_role_switch_initiator(void)
{
    return ota_role_switch_initiator;
}

void app_set_ota_role_switch_initiator(bool is_initiate)
{
    ota_role_switch_initiator = is_initiate;
}

void app_statistics_get_ota_pkt_in_role_switch(void)
{
#ifdef IBRT
    if (app_ibrt_conn_any_role_switch_running() && \
         (OTA_BASIC_TWS_MASTER == ota_basic_get_tws_role()) && \
          (ota_control_get_datapath_type() == DATA_PATH_SPP))
    {
        LOG_D("GET PKT IN OTA");
        ota_get_pkt_in_role_switch = true;
    }
#endif
}

bool app_check_user_can_role_switch_in_ota(void)
{
#ifdef IBRT
    return (app_get_bes_ota_state() && ota_role_switch_flag &&
            app_ibrt_conn_any_role_switch_running());
#else
    return false;
#endif
}

#ifdef PROMPT_IN_FLASH
void ota_language_copy_prompt(uint32_t prompt_start, uint32_t prompt_end, uint32_t offset, uint32_t packetSize)
{
    NORFLASH_API_MODULE_ID_T mod = NORFLASH_API_MODULE_ID_PROMPT;
    uint32_t offsetInFlashToProgram = offset;
    uint32_t lengthToBurn = packetSize;
    uint8_t* ptrSource = (uint8_t*)(OTA_FLASH_LOGIC_ADDR + NEW_IMAGE_FLASH_OFFSET);

    app_flash_register_module(mod, app_flash_get_dev_id_by_addr(prompt_start), prompt_start, prompt_end-prompt_start, (uint32_t)ota_control_opera_callback);

    uint32_t preBytes = (FLASH_SECTOR_SIZE_IN_BYTES - (offsetInFlashToProgram % FLASH_SECTOR_SIZE_IN_BYTES)) % FLASH_SECTOR_SIZE_IN_BYTES;
    if (lengthToBurn < preBytes)
    {
        preBytes = lengthToBurn;
    }

    uint32_t middleBytes = 0;
    if (lengthToBurn > preBytes)
    {
        middleBytes = ((lengthToBurn - preBytes) / FLASH_SECTOR_SIZE_IN_BYTES * FLASH_SECTOR_SIZE_IN_BYTES);
    }

    uint32_t postBytes = 0;
    if (lengthToBurn > (preBytes + middleBytes))
    {
        postBytes = (offsetInFlashToProgram + lengthToBurn) % FLASH_SECTOR_SIZE_IN_BYTES;
    }

    LOG_D("Prebytes is %d middlebytes is %d postbytes is %d", preBytes, middleBytes, postBytes);

    if (preBytes > 0)
    {
        app_flash_program(mod,
                          offsetInFlashToProgram,
                          ptrSource,
                          preBytes,
                          false);

        ptrSource += preBytes;
        offsetInFlashToProgram += preBytes;
    }

    uint32_t sectorIndexInFlash = offsetInFlashToProgram/FLASH_SECTOR_SIZE_IN_BYTES;

    if (middleBytes > 0)
    {
        uint32_t sectorCntToProgram = middleBytes/FLASH_SECTOR_SIZE_IN_BYTES;
        for (uint32_t sector = 0;sector < sectorCntToProgram;sector++)
        {
            app_flash_sector_erase(mod,
                                 sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES);
            app_flash_program(mod,
                              sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES,
                              ptrSource + sector * FLASH_SECTOR_SIZE_IN_BYTES, FLASH_SECTOR_SIZE_IN_BYTES,
                              false);

            sectorIndexInFlash++;
        }

        ptrSource += middleBytes;
        app_flash_flush_pending_op(mod, NORFLASH_API_ALL);      
    }

    if (postBytes > 0)
    {
        app_flash_sector_erase(mod,
                             sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES);
        app_flash_program(mod,
                          sectorIndexInFlash * FLASH_SECTOR_SIZE_IN_BYTES,
                          ptrSource,
                          postBytes,
                          false);
    }

    app_flash_flush_pending_op(mod,
                               NORFLASH_API_ALL);

}

static bool ota_control_check_language_crc(void)
{
    uint32_t verifiedDataSize = 0;
    uint32_t crc32Value = 0;
    uint32_t verifiedBytes = 0;
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    LOG_D("ota_control_env.totalImageSize:0x%x", ota_control_env.totalImageSize);

    while (verifiedDataSize < ota_control_env.totalImageSize)
    {
        if (ota_control_env.totalImageSize - verifiedDataSize > OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            verifiedBytes = OTA_DATA_BUFFER_SIZE_FOR_BURNING;
        }
        else
        {
            verifiedBytes = ota_control_env.totalImageSize - verifiedDataSize;
        }

        app_flash_read(mod,
                       verifiedDataSize,
                       ota_control_env.dataBufferForBurning,
                       OTA_DATA_BUFFER_SIZE_FOR_BURNING);

        verifiedDataSize += verifiedBytes;

        crc32Value = crc32_c(crc32Value,
                             (uint8_t *)ota_control_env.dataBufferForBurning,
                             verifiedBytes);
    }

    LOG_D("Original CRC32 is 0x%x Confirmed CRC32 is 0x%x.", ota_control_env.crc32OfImage, crc32Value);
    if (crc32Value == ota_control_env.crc32OfImage)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void ota_data_reception_languange_check(void)
{
    //language bin check
    bool Ret = ota_control_check_language_crc();

    if(Ret)
    {
        //internal crc check

    }

#ifdef IBRT
    errOtaCode = Ret ? 1 : 0;
    ota_send_rsp_handle(OTA_RSP_RESULT, (uint8_t *)&errOtaCode, 1, APP_TWS_CMD_OTA_IMAGE_CRC_CMD);
#else
    ota_control_send_result_response(Ret);
#endif
}


void ota_check_and_reboot_to_copy_language(void)
{
    if (ota.permissionToApply && ota_control_env.isPendingForReboot == true)
    {
        app_start_ota_language_reset();
    }    
}

#endif


#ifdef COMBO_CUSBIN_IN_FLASH
void _combo_bin_exception_disconnect(void)
{
#ifdef IBRT
    if (OTA_BASIC_TWS_MASTER == ota_basic_get_tws_role())
#endif
    {
        //ota disconnect
        if (DATA_PATH_SPP == ota_control_env.dataPathType)
        {
            ota_disconnect();
        }
#ifdef __IAG_BLE_INCLUDE__
        else if (DATA_PATH_BLE == ota_control_env.dataPathType)
        {
            bes_ble_gap_disconnect_all();
        }
#endif
    }
}

uint8_t _combo_bin_start_load(uint8_t* buf,uint32_t* len)
{
    uint8_t index = 0;
    OTA_COMBO_STATUS status = OTA_COMBO_PROCESS_FAILED;

    if ((BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser) &&\
        (0 == ota_control_env.alreadyReceivedDataSizeOfImage))
    {
        index = _get_user_index(BES_OTA_USER_COMBOFIRMWARE);
        status = ota_control_env.userInfo[index].process(COMBIN_PARASE_HEADER_EVT,buf,0);
        // calculate combo bin header crc
        ota_control_env.crc32OfHeader = crc32_c(ota_control_env.crc32OfSegment,
                                                (uint8_t *)buf,
                                                COMBO_BIN_HEADER_TOTAL_LEN);
        if(!status)
        {
            LOG_D("[COMBO]0x%x",nv_record_combo_bin_get_hearlen());
            //memcpy(tmp_buf,buf,nv_record_combo_bin_get_hearlen());
            //DUMP8("%02x ",tmp_buf,0x28);
            if (0 == ota_control_env.blockid)
            {
                ota_control_env.totalImageSize -= nv_record_combo_bin_get_hearlen();
            }
            ota_control_env.crc32OfImage = nv_record_combo_bin_get_whole_crc32();
            ota_control_env.blockid = 1;
            ota_control_env.is_started = true;
            ota_control_env.currentlengthlimit = nv_record_combo_bin_get_datalen(ota_control_env.blockid);

            LOG_D("%s: Image size is 0x%x, crc32 of image is 0x%x start 0x%x",
                  __func__,
                  ota_control_env.totalImageSize,
                  ota_control_env.crc32OfImage,
                  ota_control_env.offsetInFlashToProgram);
        }
        else
        {
            TRACE(0,"[%s] status:%d",__func__, status);
            status = OTA_COMBO_PROCESS_EXPCEPTION;
            _combo_bin_exception_disconnect();
        }
    }else{
        TRACE(0,"[%s] alreadyReceivedData:%d %d user:%d",__func__, \
            ota_control_env.alreadyReceivedDataSizeOfImage, nv_record_combo_bin_get_hearlen(),
            BES_OTA_USER_COMBOFIRMWARE);
    }

    return status;
}

static uint32_t _combo_bin_common_crc32(uint32_t start_addr, uint32_t need_check_len,uint32_t crc_src)
{
    uint32_t tmp = 0;
    uint32_t crc = crc_src;
    uint32_t need_check_len_done = 0;
    NORFLASH_API_MODULE_ID_T mod = _get_user_flash_module(ota_control_env.currentUser);

    TRACE(3, "%s start addr 0x%x len 0x%x", __func__, start_addr, need_check_len);
    while (need_check_len > need_check_len_done)
    {
        tmp = (need_check_len - need_check_len_done) > OTA_DATA_BUFFER_SIZE_FOR_BURNING
                  ? OTA_DATA_BUFFER_SIZE_FOR_BURNING
                  : need_check_len - need_check_len_done;
        app_flash_read(mod,
                       start_addr + need_check_len_done,
                       ota_control_env.dataBufferForBurning,
                       tmp);
        need_check_len_done += tmp;
        crc = crc32_c(crc, ota_control_env.dataBufferForBurning, tmp);
    }

    return crc;
}

#ifdef ARM_CMNS
static void app_cmse_initial_crc_set(void)
{
    uint8_t index  = 0;
    uint32_t startFlashAddr = 0;
    uint32_t total_len = 0;
    uint32_t tmp,need_check_len = 0;
    uint32_t crc32 = 0;
    uint8_t i = _get_user_index(ota_control_env.currentUser);

    if (ota_control_env.is_crossregion)
    {
        LOG_D("[COMBO]cross regional crc32 calcuate");
        // get previous region data crc
        for(index = 1; index <= ota_control_env.blockid-1; index++)
        {
            total_len += nv_record_combo_bin_get_datalen(index);
            LOG_D("[COMBO]%x %x",total_len,index);
        }
        LOG_D("[COMBO]region crc32 %x %x",total_len,ota_control_env.offsetOfImageOfCurrentSegment);
        need_check_len = total_len - ota_control_env.offsetOfImageOfCurrentSegment;
        tmp = nv_record_combo_bin_get_datalen(ota_control_env.blockid-1) - need_check_len;
        LOG_D("[COMBO] %x %x",nv_record_combo_bin_get_datalen(ota_control_env.blockid-1),(ota_control_env.blockid-1));
        LOG_D("n:%x t:%x",need_check_len,tmp);
        startFlashAddr = ota_control_env.lastdstFlashOffsetForNewImage - (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + tmp;
        LOG_D("[COMBO]start address %x %x",startFlashAddr,ota_control_env.lastdstFlashOffsetForNewImage);
        crc32 = _combo_bin_common_crc32(startFlashAddr,need_check_len, 0);
        app_flash_tz_ota_set_segment_crc(crc32);
    }
}
#endif

void _combo_bin_calc_crc32(void)
{
    uint8_t index  = 0;
    uint32_t startFlashAddr = 0;
    uint32_t total_len = 0;
    uint32_t tmp,need_check_len = 0;
    uint8_t i = _get_user_index(ota_control_env.currentUser);

    if (ota_control_env.is_crossregion)
    {
        LOG_D("[COMBO]cross regional crc32 calcuate");
        // get previous region data crc
        for(index = 1; index <= ota_control_env.blockid-1; index++)
        {
            total_len += nv_record_combo_bin_get_datalen(index);
            LOG_D("[COMBO]%x %x",total_len,index);
        }
        LOG_D("[COMBO]region crc32 %x %x",total_len,ota_control_env.offsetOfImageOfCurrentSegment);
        need_check_len = total_len - ota_control_env.offsetOfImageOfCurrentSegment;
        tmp = nv_record_combo_bin_get_datalen(ota_control_env.blockid-1) - need_check_len;
        LOG_D("[COMBO] %x %x",nv_record_combo_bin_get_datalen(ota_control_env.blockid-1),(ota_control_env.blockid-1));
        LOG_D("n:%x t:%x",need_check_len,tmp);
        startFlashAddr = ota_control_env.lastdstFlashOffsetForNewImage - (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + tmp;
        LOG_D("[COMBO]start address %x %x",startFlashAddr,ota_control_env.lastdstFlashOffsetForNewImage);
        ota_control_env.crc32OfSegment = _combo_bin_common_crc32(startFlashAddr,need_check_len,ota_control_env.crc32OfSegment);

        // get current region data crc
        need_check_len = ota_control_env.alreadyReceivedDataSizeOfImage - total_len;
        startFlashAddr = ota_control_env.dstFlashOffsetForNewImage - (ota_control_env.userInfo[i].startAddr&0xFFFFFF);
        #ifdef ARM_CMNS
        if(nv_record_combo_bin_get_fw_id(ota_control_env.blockid) == COMBO_TZ_SB)
        {
            ota_control_env.crc32OfSegment = app_flash_tz_ota_get_segment_crc();
        }
        else
        #endif
        {
            ota_control_env.crc32OfSegment = _combo_bin_common_crc32(startFlashAddr,need_check_len,ota_control_env.crc32OfSegment);
        }

        LOG_D("[COMBO]region crc32 val %x",ota_control_env.crc32OfSegment);
        ota_control_env.is_crossregion = false;
    }
    else
    {
        LOG_D("[COMBO]cross common crc32 calcuate");
        startFlashAddr = ota_control_env.dstFlashOffsetForNewImage - 
                        (ota_control_env.userInfo[i].startAddr&0xFFFFFF) + 
                        ota_control_env.offsetInFlashOfCurrentSegment;
        LOG_D("Calculate the crc32 of the segment, startAddr:0x%x, curSeg:0x%x",
                startFlashAddr, ota_control_env.offsetInFlashOfCurrentSegment);
        need_check_len = ota_control_env.alreadyReceivedDataSizeOfImage - ota_control_env.offsetOfImageOfCurrentSegment;
        if (ota_control_env.offsetOfImageOfCurrentSegment == 0)
        {
            ota_control_env.crc32OfSegment = ota_control_env.crc32OfHeader;
        }

        #ifdef ARM_CMNS
        if(nv_record_combo_bin_get_fw_id(ota_control_env.blockid) == COMBO_TZ_SB)
        {
            ota_control_env.crc32OfSegment = app_flash_tz_ota_get_segment_crc();
        }
        else
        #endif
        {
            ota_control_env.crc32OfSegment = _combo_bin_common_crc32(startFlashAddr,need_check_len,ota_control_env.crc32OfSegment);
        }
    }
}

void _combo_bin_erase_segment(uint32_t addr, uint32_t length)
{
    LOG_D("%s addr 0x%x length 0x%x", __func__, addr, length);
    if(ota_control_env.offsetOfImageOfCurrentSegment == 0)
    {
        length -= nv_record_combo_bin_get_hearlen();
    }

    if(ota_control_env.offsetInFlashToProgram >= length)
    {
        LOG_D("erase start - common calcuate");
    }
    else
    {
        uint32_t total_len = 0;
        LOG_D("erase start - region calcuate");
        for(uint8_t index = 1; index <= ota_control_env.blockid-1; index++)
        {
            total_len += nv_record_combo_bin_get_datalen(index);
        }

        uint32_t need_check_len = total_len - ota_control_env.offsetOfImageOfCurrentSegment;
        uint32_t offsetInFlashOfpreSegment = nv_record_combo_bin_get_datalen(ota_control_env.blockid-1) - need_check_len;
        LOG_D("n:0x%x t:0x%x",need_check_len,offsetInFlashOfpreSegment);

        //restore param
        ota_control_env.blockid--;
        ota_control_env.currentlengthlimit = total_len;
        ota_control_env.dstFlashOffsetForNewImage = ota_control_env.lastdstFlashOffsetForNewImage;
        ota_control_env.offsetInFlashOfCurrentSegment = offsetInFlashOfpreSegment;
    }
}

uint8_t  _combo_bin_update_data_block(void)
{
    uint8_t status,i;
    uint8_t index= 0;
    uint32_t num = nv_record_combo_bin_get_content_num();
    uint32_t id = 0;
    LOG_D("block num %d",num);
    if (num > ota_control_env.blockid)
    {
        ota_control_env.blockid += 1;
        ota_control_env.currentlengthlimit = 0;

        for(i = 1; i <= ota_control_env.blockid; i++)
        {
            ota_control_env.currentlengthlimit += nv_record_combo_bin_get_datalen(i);
            //LOG_D(1,"[OTA]current limit %d",ota_control_env.currentlengthlimit);
        }
        ota_control_env.lastdstFlashOffsetForNewImage = ota_control_env.dstFlashOffsetForNewImage;
        id = nv_record_combo_bin_get_fw_id(ota_control_env.blockid);

        index = _get_user_index(BES_OTA_USER_COMBOFIRMWARE);
        status = ota_control_env.userInfo[index].process(COMBIN_SET_START_ADDRESS,NULL,&id);
        if(status)
        {
            LOG_D("[OTA]exception!!");
            _combo_bin_exception_disconnect();
            return  OTA_COMBO_PROCESS_FAILED;
        }
    }
    return OTA_COMBO_PROCESS_SUCCESS;
}

void _combo_bin_breakpoint_recovery(uint32_t breakpoint)
{
    uint8_t i = 0;
    uint8_t num = nv_record_combo_bin_get_content_num();
    uint32_t length = 0;
    uint32_t offset = 0;
    bool is_update = false;
    bool is_find = false;

    LOG_D("%s %d %d %d",__func__,num,breakpoint,ota_control_env.offsetInFlashOfCurrentSegment);

    ota_control_env.is_started = true;
    for (i = 0; i < num; i++)
    {
        offset = length;
        length += nv_record_combo_bin_get_datalen(i+1);

        if ((breakpoint <= length)&&(is_find == false))
        {
            is_find = true;
            ota_control_env.blockid = i+1;
            ota_control_env.currentlengthlimit = length;

            if(breakpoint == length)
            {
                is_update = true;
            }
            else
            {
                uint32_t id = nv_record_combo_bin_get_fw_id(ota_control_env.blockid);
                uint8_t index = _get_user_index(BES_OTA_USER_COMBOFIRMWARE);
                ota_control_env.userInfo[index].process(COMBIN_SET_START_ADDRESS,NULL,&id);
                ota_control_env.offsetInFlashToProgram = breakpoint - offset;
                ota_control_env.offsetInFlashOfCurrentSegment = ota_control_env.offsetInFlashToProgram;
                TRACE(4, "%s offset %d id %d dst %x",
                        __func__,ota_control_env.offsetInFlashToProgram, ota_control_env.blockid, ota_control_env.dstFlashOffsetForNewImage);
            }
            break;
        }
    }

    ota_control_env.totalImageSize -= nv_record_combo_bin_get_hearlen();

    if(is_update)
    {
        _combo_bin_update_data_block();
    }
}

void _combo_bin_package_handler(uint8_t *buf, bool isViaBle,uint16_t length)
{
    uint8_t status = 1;
    bool detech_cross_region = false;
    uint32_t offset = 0;
    uint8_t* rawDataPtr = &buf[1];
    uint32_t rawDataSize = length - 1;
    uint32_t bytesToCopy = 0;
    uint32_t leftDataSize = rawDataSize;
    uint32_t offsetInReceivedRawData = 0;

    LOG_D("Received image data size %d %d %d",
          rawDataSize,
          ota_control_env.currentlengthlimit,
          ota_control_env.resume_at_breakpoint);

    if(ota_control_env.resume_at_breakpoint)
    {
        ota_control_env.resume_at_breakpoint = false;
    }
    else
    {
        if (COMBO_BIN_HEADER_TOTAL_LEN < rawDataSize) {
            status = _combo_bin_start_load(rawDataPtr,&rawDataSize);
            if(OTA_COMBO_PROCESS_SUCCESS == status)
            {
                offset = nv_record_combo_bin_get_hearlen();
                rawDataPtr = &buf[offset+1];
                rawDataSize = rawDataSize-offset;
            }
        } else if(!ota_control_env.is_started){

            if (0 == ota_control_env.alreadyReceivedDataSizeOfImage)
            {
                memcpy(&ota_control_env.dataBufferForBurning[ota_control_env.combinedSize],
                    &rawDataPtr[offsetInReceivedRawData], rawDataSize);
                ota_control_env.combinedSize += rawDataSize;
            }
            if (COMBO_BIN_HEADER_TOTAL_LEN < ota_control_env.combinedSize)
            {
                status = _combo_bin_start_load(ota_control_env.dataBufferForBurning, &ota_control_env.combinedSize);
                if(OTA_COMBO_PROCESS_SUCCESS == status)
                {
                    offset = nv_record_combo_bin_get_hearlen();
                    rawDataSize = ota_control_env.combinedSize-offset;
                    rawDataPtr = &buf[length-rawDataSize];
                }
            }else{
                return;
            }
        }
    }

    leftDataSize = rawDataSize;
    offsetInReceivedRawData = 0;

    do
    {
        if ((ota_control_env.offsetInDataBufferForBurning + leftDataSize) >OTA_DATA_BUFFER_SIZE_FOR_BURNING)
        {
            bytesToCopy = OTA_DATA_BUFFER_SIZE_FOR_BURNING - ota_control_env.offsetInDataBufferForBurning;
        }
        else
        {
            bytesToCopy = leftDataSize;
        }
        // LOG_D("byte cpy 1 %d %d %d %d",bytesToCopy,ota_control_env.alreadyReceivedDataSizeOfImage, ota_control_env.blockid, nv_record_combo_bin_get_content_num());
        // LOG_D("curret buffer offset %d %d %d",offsetInReceivedRawData, ota_control_env.offsetInDataBufferForBurning,ota_control_env.currentlengthlimit);
        if ((ota_control_env.alreadyReceivedDataSizeOfImage + bytesToCopy >= ota_control_env.currentlengthlimit)&&\
            ota_control_env.blockid && (ota_control_env.blockid != nv_record_combo_bin_get_content_num()))
        {
            bytesToCopy = ota_control_env.currentlengthlimit - ota_control_env.alreadyReceivedDataSizeOfImage;
            detech_cross_region = true;
            ota_control_env.is_crossregion = true;
        }

        leftDataSize -= bytesToCopy;
        // LOG_D("byte cpy %d,%d",bytesToCopy,offsetInReceivedRawData);
        memcpy(&ota_control_env.dataBufferForBurning[ota_control_env.offsetInDataBufferForBurning],
               &rawDataPtr[offsetInReceivedRawData], bytesToCopy);
        offsetInReceivedRawData += bytesToCopy;
        ota_control_env.offsetInDataBufferForBurning += bytesToCopy;
        ota_control_env.alreadyReceivedDataSizeOfImage += bytesToCopy;

        // LOG_D("offsetInDataBufferForBurning is %d %d",ota_control_env.offsetInDataBufferForBurning,offsetInReceivedRawData);
        if (detech_cross_region)
        {
            LOG_D("detech cross region");
            detech_cross_region = false;
            ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                            ota_control_env.offsetInDataBufferForBurning,
                                            ota_control_env.offsetInFlashToProgram);
            ota_control_env.offsetInFlashToProgram += ota_control_env.offsetInDataBufferForBurning;
            ota_control_env.offsetInDataBufferForBurning = 0;
            _combo_bin_update_data_block();
            #ifdef ARM_CMNS
            if(nv_record_combo_bin_get_fw_id(ota_control_env.blockid) == COMBO_TZ_SB)
            {
                app_cmse_initial_crc_set();
            }
            #endif
        }
        else if (OTA_DATA_BUFFER_SIZE_FOR_BURNING <= ota_control_env.offsetInDataBufferForBurning)
        {
            LOG_D("Program the image to flash.");
#if (IMAGE_RECV_FLASH_CHECK == 1)
            if ((ota_control_env.offsetInFlashToProgram > ota_control_env.totalImageSize) ||
                (ota_control_env.totalImageSize > MAX_IMAGE_SIZE) ||
                (ota_control_env.offsetInFlashToProgram & (MIN_SEG_ALIGN - 1)))
            {
                LOG_D("ERROR: IMAGE_RECV_FLASH_CHECK");
                LOG_D("ota_control_env(.offsetInFlashToProgram >= .totalImageSize)");
                LOG_D("or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
                LOG_D("or .offsetInFlashToProgram isn't segment aligned");
                LOG_D(".offsetInFlashToProgram:0x%x  .dstFlashOffsetForNewImage:0x%x  .totalImageSize:%d",
                      ota_control_env.offsetInFlashToProgram,
                      ota_control_env.dstFlashOffsetForNewImage,
                      ota_control_env.totalImageSize);
                /// In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log
                /// but reset ota, because a boundary check is performed before flashing and if there is really wrong
                /// we'll catch when an image CRC32 check finally.
                ota_control_send_result_response(OTA_RESULT_ERR_FLASH_OFFSET);
                return;
            }
#endif
            //LOG_D(0,"Program start address %x",ota_control_env.offsetInFlashToProgram);
            ota_control_flush_data_to_flash(ota_control_env.dataBufferForBurning,
                                            OTA_DATA_BUFFER_SIZE_FOR_BURNING,
                                            ota_control_env.offsetInFlashToProgram);
            ota_control_env.offsetInFlashToProgram += OTA_DATA_BUFFER_SIZE_FOR_BURNING;
            ota_control_env.offsetInDataBufferForBurning = 0;
        }
    } while (offsetInReceivedRawData < rawDataSize);

    LOG_D("Image already received %d", ota_control_env.alreadyReceivedDataSizeOfImage);

#if (IMAGE_RECV_FLASH_CHECK == 1)
    if((ota_control_env.alreadyReceivedDataSizeOfImage > ota_control_env.totalImageSize) ||
        (ota_control_env.totalImageSize > MAX_IMAGE_SIZE))
    {
        LOG_D("ERROR: IMAGE_RECV_FLASH_CHECK");
        LOG_D("ota_control_env(.alreadyReceivedDataSizeOfImage > .totalImageSize)");
        LOG_D("or (ota_control_env.totalImageSize > %d)", MAX_IMAGE_SIZE);
        LOG_D(".alreadyReceivedDataSizeOfImage:%d. totalImageSize:%d",
              ota_control_env.alreadyReceivedDataSizeOfImage,
              ota_control_env.totalImageSize);
        /// In order to reduce unnecessary erasures and retransmissions we don't imeediately destory the log but reset
        /// ota, because a boundary check is performed before flashing and if there is really wrong we'll catch when
        /// an image CRC32 check finally.
        //ota_upgradeLog_destroy();
        ota_control_send_result_response(OTA_RESULT_ERR_RECV_SIZE);
        return;
    }
#endif

#if DATA_ACK_FOR_SPP_DATAPATH_ENABLED
    if (DATA_PATH_SPP == ota_control_env.dataPathType)
    {
        ota_control_send_data_ack_response();
    }
#endif
}

void  ota_combo_bin_data_check_image_crc(void)
{
    bool ret = false;
    uint8_t num ,i ,index = 0;
    uint32_t startaddr,crc32 = 0;

    if (BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
    {
        // check the crc32 of the image
        num = nv_record_combo_bin_get_content_num();

        index = _get_user_index(BES_OTA_USER_COMBOFIRMWARE);

        //sanity crc check
        for(i = 1; i <= num; i++)
        {
            ota_control_env.userInfo[index].process(COMBIN_GET_START_ADDRESS,&i,&startaddr);
            #ifdef ARM_CMNS
            if (nv_record_combo_bin_get_fw_id(i) == COMBO_TZ_SB)
            {
                crc32 = app_flash_tz_ota_get_whole_crc();
            }
            else
            #endif
            {
                crc32 = _combo_bin_common_crc32((startaddr-ota_control_env.userInfo[index].startAddr)&0xFFFFFF,\
                                                nv_record_combo_bin_get_datalen(i),0);
            }

            // TRACE(0,"[%s] %x == %x? len:%x offset:%x",__func__, crc32, nv_record_combo_bin_get_id_crc32(i),
            //         nv_record_combo_bin_get_datalen(i), (startaddr-ota_control_env.userInfo[index].startAddr)&0xFFFFFF);
            ret = (crc32==nv_record_combo_bin_get_id_crc32(i)) ? true : false;
            if (!ret)
            {
                break;
            }

            if (nv_record_combo_bin_get_fw_id(i) == COMBO_SOC_FW || nv_record_combo_bin_get_fw_id(i) == COMBO_TZ_NSB)
            {
                ota_control_env.crc32OfImage = crc32;
                ota_control_env.totalImageSize = nv_record_combo_bin_get_datalen(i);
                ota_control_env.dstFlashOffsetForNewImage = startaddr&0xFFFFFF;
                app_update_magic_number_of_app_image(NORMAL_BOOT);
            }
        }

        ota_data_check_image_crc_handler(ret);
    }
}

uint32_t ota_bin_breakpoint_update(uint32_t breakpoint)
{
    if (BES_OTA_USER_COMBOFIRMWARE == ota_control_env.currentUser)
    {
        return (breakpoint+nv_record_combo_bin_get_hearlen());
    }
    else
    {
        return breakpoint;
    }
}
#endif
