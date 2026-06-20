#include "cmsis_os.h"
#include "hal_trace.h"
#include <stdlib.h>
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "app_ai_manager_api.h"
#include "app_ai_if_ble.h"
#include "app_ai_if_config.h"
#include "app_ai_if_thirdparty.h"
#include "app_ai_if_thirdparty.h"
#include "app_ai_if_custom_ui.h"
#include "app_through_put.h"
#include "ai_manager.h"
#include "ai_control.h"
#include "ai_thread.h"
#include "app_ai_voice.h"
#include "app_utils.h"

//SPP mode: Throughput MTU = RFCOMM_MTU - 7 = 665
#define THROUGHPUT_MTU_BASED_ON_SPP (BT_SPP_MAX_TX_MTU - 7)
//BLE mode: Throughput MTU = LinkLayer_Payload_Max_Length(251 octets) * 2 - L2CAP_CID_Length(4 octets) - ATT_HANDLE_VALUE_NTF-header length(3 octets)
#define THROUGHPUT_MTU_BASED_ON_BLE 495

static THROUGHPUT_TEST_ENV_T throughputTestEnv;
static THROUGHPUT_TEST_CONFIG_T throughputTestConfig;
static uint8_t app_throughput_datapattern[THROUGHPUT_DATA_MAX_SIZE];

static APP_THROUGHPUT_CMD_INSTANCE_T *find_throughput_instance_by_code(uint16_t cmdCode)
{
    for (uint32_t index = 0;
        index < APP_THROUGHPUT_CMD_INSTANCE_NUMBER;
        index++)
    {
        if (through_put_table[index].cmdCode == cmdCode)
        {
            return &through_put_table[index];
        }
    }

    return NULL;
}

APP_THROUGHPUT_CMD_PAYLOAD_T app_through_put_payload;
bool app_throughput_send_command(APP_THROUGHPUT_CMD_CODE_E cmdCode, 
    uint8_t *ptrParam, uint32_t paramLen)
{
    uint16_t cmdLength = 0;

    if ((throughputTestConfig.dataPacketSize - THROUGHPUT_CMD_PAYLOAD_HEADER_LEN) < paramLen)
    {
        TRACE(1,"%s error ", __func__);
        return false;
    }

    app_through_put_payload.cmdCode = cmdCode;
    app_through_put_payload.paramLen = paramLen;
    memcpy(app_through_put_payload.param, ptrParam, paramLen);

    cmdLength = (uint32_t)THROUGHPUT_CMD_PAYLOAD_HEADER_LEN + app_through_put_payload.paramLen;
    bool isCmdSentSuccessfully = app_ai_if_custom_ui_send_cmd_generic((uint8_t *)&app_through_put_payload, cmdLength, true);

    return isCmdSentSuccessfully;
}

void app_throughput_test_continuously_send_command(void)
{
    bool isTestCmdSentSuccessfully = true;
    uint8_t txCredits = app_ai_read_tx_credit(AI_SPEC_BES);
    while (throughputTestEnv.isThroughputTestOn)
    {
        if (DOWN_STREAM == throughputTestConfig.direction)
        {
            break;   //Under DOWN_STREAM moed, it doesn't need keep sending data.
        }

        if (isTestCmdSentSuccessfully && txCredits)
        {
            isTestCmdSentSuccessfully = app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DATA,
                                       app_throughput_datapattern, throughputTestConfig.dataPacketSize - 4);
            txCredits = app_ai_read_tx_credit(AI_SPEC_BES);
        }
        else
        {
            break;
        }

    }
}

uint32_t app_throughput_cmd_send_done(void *param1, uint32_t param2, uint8_t ai_index, uint8_t dest_id)
{
    if (throughputTestEnv.isThroughputTestOn)   //throughput is going on.
    {
        uint8_t throughputTransportType = app_ai_get_transport_type(AI_SPEC_BES, throughputTestEnv.testAiIndex);
        if (AI_TRANSPORT_BLE == throughputTransportType)   //BLE transport link.
        {
            //Due to throughput test has start, constantly send cmd to phone.
            app_throughput_test_continuously_send_command();
        }
        else                                               //SPP transport link.
        {
            if (WITHOUT_RSP == throughputTestConfig.responseModel)
            {
                //Under without ack modle: Due to throughput test has start, constantly send cmd to phone.
                app_throughput_test_continuously_send_command();
            }
            else
            {
                //Under with ack modle: earphone need wait for phone's ack, so there is no need to send cmd constantly.
            }
        }
    }
    else                                         //throughput has stopped.
    {
        uint32_t restCmdLengthInAiCmdTransportFifo = app_ai_if_custom_get_ai_cmd_transport_fifo_length();
        if (restCmdLengthInAiCmdTransportFifo)
        {
            //it is possible for ai_thread fail to send cmd due to lack of tx_credits, 
            //so we must send the rest of cmd even though throughput has ended.
            TRACE(1,"Throughput send left data in ai_cmd_transport_fifo");
            app_ai_if_custom_send_rest_cmd_in_ai_cmd_transport_fifo();
        }
        else
        {
            //do nothing.
        }
    }

    return 0;
}

uint32_t app_throughput_receive_cmd(void *param1, uint32_t param2, uint8_t ai_index, uint8_t dest_id)
{
    uint8_t *ptrbuf = NULL;
    uint32_t data_length = 0;
    APP_THROUGHPUT_CMD_PAYLOAD_T* pPayload = NULL;

    TRACE(2,"%s data len %d", __func__, param2);
    //DUMP8("0x%02x ", param1, param2);

    data_length = param2;
    while(data_length) {
        ptrbuf = (uint8_t *)((uint32_t)param1 + param2 - data_length);

        pPayload = (APP_THROUGHPUT_CMD_PAYLOAD_T *)ptrbuf;
        data_length -= THROUGHPUT_CMD_PAYLOAD_HEADER_LEN;
        if(data_length < pPayload->paramLen) {
            TRACE(3,"%s error data_length %d paramLen %d", __func__, data_length, pPayload->paramLen);
            return 2;
        }
        TRACE(3,"%s data_length %d  paramLen %d", __func__, data_length, pPayload->paramLen);
        data_length -= pPayload->paramLen;

        // check parameter length
        if (pPayload->paramLen > sizeof(pPayload->param)) {
            TRACE(0,"SV COMMAND PARAM ERROR LENGTH");
            return 4;
        }

        APP_THROUGHPUT_CMD_INSTANCE_T *pInstance = find_throughput_instance_by_code(pPayload->cmdCode);

        // execute the command handler
        if (pInstance)
        {
            pInstance->cmdHandler((APP_THROUGHPUT_CMD_CODE_E)(pPayload->cmdCode), pPayload->param, pPayload->paramLen, ai_index);
        }
    }
    
    return 0;
}

#define APP_THROUGHPUT_PRE_CONFIG_PENDING_TIME_IN_MS    2000
static void app_throughput_pre_config_pending_timer_cb(void const *n);
osTimerDef (APP_THROUGHPUT_PRE_CONFIG_PENDING_TIMER, app_throughput_pre_config_pending_timer_cb);
osTimerId   app_throughput_pre_config_pending_timer_id = NULL;

static void app_throughput_test_data_xfer_lasting_timer_cb(void const *n);
osTimerDef (APP_THROUGHPUT_TEST_DATA_XFER_LASTING_TIMER, app_throughput_test_data_xfer_lasting_timer_cb);
osTimerId   app_throughput_test_data_xfer_lasting_timer_id = NULL;


static void app_throughput_test_data_xfer_lasting_timer_cb(void const *n)
{
    uint8_t maxNumberOfTimesToSendCmd = 5;
    for (uint8_t i = 0; i < maxNumberOfTimesToSendCmd; i++)
    {
        bool isTestDoneCmdSentSuccessfully = app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DONE, NULL, 0);
        if (isTestDoneCmdSentSuccessfully)
        {
            break;
        }
        else
        {
            osDelay(5);
            continue;
        }
    }

    //app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DONE, NULL, 0);
    app_stop_throughput_test();
}

static void app_throughput_pre_config_pending_timer_cb(void const *n)
{
    // inform the configuration
    uint8_t maxNumberOfTimesToSendCmd = 5;
    for (uint8_t i = 0; i < maxNumberOfTimesToSendCmd; i++)
    {
        bool isTestConfigCmdSentSuccessfully = app_throughput_send_command(THROUGHPUT_OP_INFORM_THROUGHPUT_TEST_CONFIG,
                                                   (uint8_t *)&throughputTestConfig, sizeof(throughputTestConfig));
        if (isTestConfigCmdSentSuccessfully)
        {
            break;
        }
        else
        {
            osDelay(5);
            continue;
        }
    }

    app_start_throughput_test();
}

void app_throughput_test_init(void)
{
    memset(&throughputTestEnv, 0x00, sizeof(throughputTestEnv));
    memset(&throughputTestConfig, 0x00, sizeof(throughputTestConfig));

    app_throughput_pre_config_pending_timer_id =
        osTimerCreate(osTimer(APP_THROUGHPUT_PRE_CONFIG_PENDING_TIMER),
        osTimerOnce, NULL);

    app_throughput_test_data_xfer_lasting_timer_id =
        osTimerCreate(osTimer(APP_THROUGHPUT_TEST_DATA_XFER_LASTING_TIMER),
        osTimerOnce, NULL);
}

static void app_throughput_test_data_handler(APP_THROUGHPUT_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen, uint8_t ai_index)
{
    if ((WITH_RSP == throughputTestConfig.responseModel) &&
        (AI_TRANSPORT_SPP == app_ai_get_transport_type(ai_index, throughputTestEnv.testAiIndex)))
    {
        app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DATA_ACK, NULL, 0);
    }
}

static void app_throughput_test_data_ack_handler(APP_THROUGHPUT_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen, uint8_t ai_index)
{
    if (throughputTestEnv.isThroughputTestOn &&
        (AI_TRANSPORT_SPP == app_ai_get_transport_type(ai_index, throughputTestEnv.testAiIndex)))
    {
        app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DATA,
            app_throughput_datapattern, throughputTestConfig.dataPacketSize - 4);
    }
}

static void app_throughput_test_done_signal_handler(APP_THROUGHPUT_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen, uint8_t ai_index)
{
    app_stop_throughput_test();
}

void app_start_throughput_test(void)
{
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, APP_SYSFREQ_78M);
    if (UP_STREAM == throughputTestConfig.direction)
    {
#ifndef SLAVE_ADV_BLE
        osTimerStart(app_throughput_test_data_xfer_lasting_timer_id,
            throughputTestConfig.lastTimeInSecond*1000);
#endif

        uint8_t throughputTransportType = app_ai_get_transport_type(AI_SPEC_BES, throughputTestEnv.testAiIndex);
        if (AI_TRANSPORT_BLE == throughputTransportType)   //BLE transport link.
        {
            //Due to throughput test has start, constantly send cmd to phone.
            app_throughput_test_continuously_send_command();
        }
        else                                               //SPP transport link.
        {
            if (WITHOUT_RSP == throughputTestConfig.responseModel)
            {
                //Under without ack modle: Due to throughput test has start, constantly send cmd to phone.
                app_throughput_test_continuously_send_command();
            }
            else
            {
                //Under with ack modle: earphone need wait for phone's ack, so there is no need to send cmd constantly.
                app_throughput_send_command(THROUGHPUT_OP_THROUGHPUT_TEST_DATA,
                    app_throughput_datapattern, throughputTestConfig.dataPacketSize - 4);
            }
        }
    }
    else
    {
        //do nothing!
    }
}

void app_stop_throughput_test(void)
{
    throughputTestEnv.isThroughputTestOn = false;
    osTimerStop(app_throughput_pre_config_pending_timer_id);
    osTimerStop(app_throughput_test_data_xfer_lasting_timer_id);
    app_sysfreq_req(APP_SYSFREQ_USER_AI_VOICE, APP_SYSFREQ_32K);
}

static void app_throughput_test_config_handler(APP_THROUGHPUT_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen, uint8_t ai_index)
{
    throughputTestConfig = *(THROUGHPUT_TEST_CONFIG_T *)ptrParam;
    throughputTestEnv.testAiIndex = ai_manager_get_foreground_ai_conidx();
    uint32_t ai_mtu = app_ai_get_mtu(ai_index);

    TRACE(4,"%s patter %d time %d response %d", __func__, \
            throughputTestConfig.dataPattern, \
            throughputTestConfig.lastTimeInSecond, \
            throughputTestConfig.responseModel);

    if (ai_mtu < throughputTestConfig.dataPacketSize)
    {
        throughputTestConfig.dataPacketSize = ai_mtu;
        TRACE(4,"throughput_test_config packet_size %d %d", ai_mtu, throughputTestConfig.dataPacketSize);
    }

    //set appropriate MTU based on different transport link: BLE or SPP.
    uint8_t throughputTransportType = app_ai_get_transport_type(AI_SPEC_BES, throughputTestEnv.testAiIndex);
    if (AI_TRANSPORT_BLE == throughputTransportType)
    {
        throughputTestConfig.dataPacketSize = THROUGHPUT_MTU_BASED_ON_BLE;
    }
    else
    {
        throughputTestConfig.dataPacketSize = THROUGHPUT_MTU_BASED_ON_SPP;
    }
    TRACE(1,"throughputTestConfig.dataPacketSize = %d", throughputTestConfig.dataPacketSize);

    //the length of data need to be generated by designative way
    uint16_t throughputParamLen = throughputTestConfig.dataPacketSize - THROUGHPUT_CMD_PAYLOAD_HEADER_LEN;
    // generate the data pattern
    switch (throughputTestConfig.dataPattern)
    {
        case PATTERN_RANDOM:
        {
            for (uint32_t index = 0;index < throughputParamLen;index++)
            {
                app_throughput_datapattern[index] = (uint8_t)rand();
            }
            break;
        }
        case PATTERN_11110000:
        {
            memset(app_throughput_datapattern, 0xF0, throughputParamLen);
            break;
        }
        case PATTERN_10101010:
        {
            memset(app_throughput_datapattern, 0xAA, throughputParamLen);
            break;
        }
        case PATTERN_ALL_1:
        {
            memset(app_throughput_datapattern, 0xFF, throughputParamLen);
            break;
        }
        case PATTERN_ALL_0:
        {
            memset(app_throughput_datapattern, 0, throughputParamLen);
            break;
        }
        case PATTERN_00001111:
        {
            memset(app_throughput_datapattern, 0x0F, throughputParamLen);
            break;
        }
        case PATTERN_0101:
        {
            memset(app_throughput_datapattern, 0x55, throughputParamLen);
            break;
        }
        default:
            throughputTestConfig.dataPattern = 0;
            break;
    }

    throughputTestEnv.isThroughputTestOn = true;
    throughputTestEnv.conidx = app_ai_if_get_ble_connection_index();
    TRACE(2, "conidx 0x%x useSpecificConnParameter %d", throughputTestEnv.conidx, throughputTestConfig.isToUseSpecificConnParameter);

    // check whether need to use the new conn parameter
    if (AI_TRANSPORT_BLE == app_ai_get_transport_type(ai_index, throughputTestEnv.testAiIndex)
        && throughputTestConfig.isToUseSpecificConnParameter)
    {
        if (throughputTestEnv.conidx != 0xFF)
        {
            app_ai_if_ble_update_conn_param(throughputTestEnv.conidx, 
                throughputTestConfig.minConnIntervalInMs,
                throughputTestConfig.maxConnIntervalInMs,
                THROUGHPUT_BLE_CONNECTION_SUPERVISOR_TIMEOUT_IN_MS,
                THROUGHPUT_BLE_CONNECTION_SLAVELATENCY);
        }

        osTimerStart(app_throughput_pre_config_pending_timer_id,
            APP_THROUGHPUT_PRE_CONFIG_PENDING_TIME_IN_MS);
    }
    else
    {
        osTimerStart(app_throughput_pre_config_pending_timer_id,
            APP_THROUGHPUT_PRE_CONFIG_PENDING_TIME_IN_MS);
    }
}


APP_THROUGHPUT_CMD_INSTANCE_T through_put_table[APP_THROUGHPUT_CMD_INSTANCE_NUMBER] =
{
    {THROUGHPUT_OP_INFORM_THROUGHPUT_TEST_CONFIG, app_throughput_test_config_handler},
    {THROUGHPUT_OP_THROUGHPUT_TEST_DATA, app_throughput_test_data_handler},
    {THROUGHPUT_OP_THROUGHPUT_TEST_DATA_ACK, app_throughput_test_data_ack_handler},
    {THROUGHPUT_OP_THROUGHPUT_TEST_DONE, app_throughput_test_done_signal_handler}
};

