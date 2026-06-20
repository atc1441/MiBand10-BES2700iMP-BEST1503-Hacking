#include "cmsis_os.h"
#include "hal_trace.h"
#include "bluetooth_bt_api.h"
#include "app_through_put.h"
#include "app_ai_if.h"
#include "app_ai_tws.h"
#include "app_ai_if_config.h"
#include "app_ai_if_custom_ui.h"

#ifdef __AI_VOICE__
#ifdef __IAG_BLE_INCLUDE__
#include "bluetooth_ble_api.h"
#endif
#include "ai_control.h"
#include "ai_thread.h"
#include "ai_transport.h"
#include "ai_manager.h"
#endif

#ifdef __AI_VOICE__
static void app_ai_if_custom_ui_global_handler_ind(uint32_t cmd, void *param1, uint32_t param2, uint8_t ai_index, uint8_t dest_id)
{
    switch (cmd)
    {
        case AI_CUSTOM_CODE_AI_ROLE_SWITCH:
        break;

        case AI_CUSTOM_CODE_CMD_RECEIVE:
#ifdef __THROUGH_PUT__
            app_throughput_receive_cmd(param1, param2, ai_index, dest_id);
#endif
        break;

        case AI_CUSTOM_CODE_DATA_RECEIVE:
        break;

        case AI_CUSTOM_CODE_AI_CONNECT:
        break;

        case AI_CUSTOM_CODE_AI_DISCONNECT:
#ifdef __THROUGH_PUT__
            app_stop_throughput_test();
#endif
        break;

        case AI_CUSTOM_CODE_UPDATE_MTU:
        break;

        case AI_CUSTOM_CODE_WAKE_UP:
        break;

        case AI_CUSTOM_CODE_START_SPEECH:
        break;

        case AI_CUSTOM_CODE_ENDPOINT_SPEECH:
        break;

        case AI_CUSTOM_CODE_STOP_SPEECH:
        break;
        case AI_CUSTOM_CODE_DATA_SEND_DONE:
#ifdef __THROUGH_PUT__
            app_throughput_cmd_send_done(param1, param2, ai_index, dest_id);
#endif
        break;

        case AI_CUSTOM_CODE_KEY_EVENT_HANDLE:
        break;

        case AI_CUSTOM_CODE_AI_ROLE_SWITCH_COMPLETE:
        break;

        case AI_CUSTOM_CODE_MOBILE_CONNECT:
        break;

        case AI_CUSTOM_CODE_MOBILE_DISCONNECT:
        break;

        case AI_CUSTOM_CODE_SETUP_COMPLETE:
        break;

        default:
        break;
    }
}
#endif

uint32_t app_ai_if_custom_get_ai_cmd_transport_fifo_length(void)
{
    return ai_get_ai_cmd_transport_fifo_length();
}

void app_ai_if_custom_send_rest_cmd_in_ai_cmd_transport_fifo(void)
{
    AI_connect_info *ai_info = app_ai_get_connect_info(ai_manager_get_foreground_ai_conidx());
    if (!ai_info)   //BES intentional code. ai_info has been checked.
    {
        TRACE(0,"NULL pointer in %s", __func__);
        return;
    }
    uint8_t dest_id = app_ai_get_dest_id(ai_info);
    ai_mailbox_put(SIGN_AI_TRANSPORT, 0, ai_info->ai_spec, dest_id);
}

bool app_ai_if_custom_ui_send_cmd(uint8_t *cmd_buf, uint16_t cmd_len)
{
    bool isCmdSentSuccessfully = app_ai_if_custom_ui_send_cmd_generic(cmd_buf, cmd_len, false);
    return isCmdSentSuccessfully;
}

bool app_ai_if_custom_ui_send_cmd_generic(uint8_t *cmd_buf, uint16_t cmd_len, bool isSkipWhenFifoOverflow)
{
#ifdef __AI_VOICE__
    TRACE(1, "%s", __func__);
    AI_connect_info *ai_info = app_ai_get_connect_info(ai_manager_get_foreground_ai_conidx());
    if(NULL == ai_info) {
        return false;
    }
    uint8_t dest_id = app_ai_get_dest_id(ai_info);
    uint8_t send_buf[BT_SPP_MAX_TX_MTU] = {0};
    uint16_t *output_size_p = NULL;
    uint8_t *output_buf_p = NULL;

    output_size_p = (uint16_t *)send_buf;
    output_buf_p = &send_buf[AI_CMD_CODE_SIZE];

    memcpy(output_buf_p, cmd_buf, cmd_len);
    *output_size_p = cmd_len;

    bool isCmdPutSuccessfully = ai_transport_cmd_put_generic(send_buf, (cmd_len+AI_CMD_CODE_SIZE), isSkipWhenFifoOverflow);
    if (isCmdPutSuccessfully)
    {
        ai_mailbox_put(SIGN_AI_TRANSPORT, cmd_len, ai_info->ai_spec, dest_id);
    }

    return isCmdPutSuccessfully;
#endif
}

void app_ai_if_custom_init(void)
{
#ifdef __AI_VOICE__
    app_ai_register_ui_global_handler_ind(app_ai_if_custom_ui_global_handler_ind);
#endif
}

