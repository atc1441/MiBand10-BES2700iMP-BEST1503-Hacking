/***************************************************************************
*
*Copyright 2015-2023 BES.
*All rights reserved. All unpublished rights reserved.
*
*No part of this work may be used or reproduced in any form or by any
*means, or stored in a database or retrieval system, without prior written
*permission of BES.
*
*Use of this work is governed by a license granted by BES.
*This work contains confidential and proprietary information of
*BES. which is protected by copyright, trade secret,
*trademark and other intellectual property rights.
*
****************************************************************************/
#include "string.h"
#include "gsound_dbg.h"
#include "cmsis_os.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "current_device_state.pb.h"
#include "device_control_request.pb.h"
#include "gsound_custom_device_actions.h"


/*********************external function declearation************************/

/************************private macro defination***************************/

/************************private type defination****************************/

/**********************private function declearation************************/

/************************private variable defination************************/


/****************************function defination****************************/

// Turn these on for debugging.
#define PRINT_MESSAGE_CONTENTSx

static void DeviceActionsSendBatteryState (int32_t current_battery_percentage);
static void DeviceActionsSendNcState (int32_t current_noise_cancellation);
static void DeviceActionsSendModelId (void);
static GSoundStatus ErrorCommandHandler(const google_assistant_v1_Execution *msg);
static GSoundStatus FirstExampleCommandHandler(const google_assistant_v1_Execution *msg);
static GSoundStatus SecondExampleCommandHandler(const google_assistant_v1_Execution *msg);
static GSoundStatus CommandHandlerOnOff(const google_assistant_v1_Execution *msg);
static GSoundStatus CommandHandlerNoiseCancellation(const google_assistant_v1_Execution *msg);
static GSoundStatus CommandHandlerAmbientMode(const google_assistant_v1_Execution *msg);
static GSoundStatus CommandHandlerQueryBatteryLevel(const google_assistant_v1_Execution *msg);
static GSoundStatus CommandHandlerUpdateTouchControl(const google_assistant_v1_Execution *msg);

static bool DeserializeMessage(uint32_t size, const uint8_t *buf, void *obj,
                               const pb_msgdesc_t *fields);
static void SerializeMessage(const pb_msgdesc_t fields[], const void *msg,
                             int *output_size, int buffer_number);

/*
 * Important capability notes in order to successfully call the nano pb
 * functions that are already built into libgsound:
 *
 * 1) Use nanopb-0.3.9 (2017-09-23) to compile .proto files to .pb.c and .pb.h.
 * 2) PB_FIELD_16BIT must be defined in target makefile for all source that
 *    include any nanopb header files.
 */

/*
 *  TODO: Note that it may be possible to re-use one encoding buffer
 *  but, if that is done, it is necessary to make sure no other encoding is done
 *  until the consumed API is called.
 */

#define ENCODING_BUFFER_SIZE (256)
#define BUFFER_BATTERY (0)
#define BUFFER_NC (1)
#define NUM_BUFFERS (2)
static const char device_action_model_id[] = "google-bisto-dev-v1";
static const uint32_t device_action_model_id_len = sizeof(device_action_model_id) - 1;
// arbitrary but must be unique
static const uint8_t action_state_key1[] = {5, 2, 3, 0, 4};
static const uint32_t action_state_key1_len = sizeof(action_state_key1);
static const uint8_t action_state_key2[] = {1};
static const uint32_t action_state_key2_len = sizeof(action_state_key2);

static int32_t battery_level = 1;
static int32_t nc_level = 1;
#define GSOUND_DEVICE_ACTIONS_BATTERY_POLLING_INTERVAL_MSEC (20000) // 20 seconds
#define GSOUND_DEVICE_ACTIONS_NC_POLLING_INTERVAL_MSEC (50000) // 50 seconds
#define GSOUND_DEVICE_ACTIONS_DELAYED_INIT_TIME_MSEC (500)
#define GSOUND_DEVICE_ACTIONS_RESPONSE_TO_STATE_UPDATE_REQUEST_TIME_MSEC (250)

typedef struct {
    const char * const command; // null terminated string
    GSoundStatus (*execution_handler)(const google_assistant_v1_Execution *msg);
} ExecutionEntry;
static const ExecutionEntry commands[] = {
    {"google error command", &ErrorCommandHandler},
    {"1st example command", &FirstExampleCommandHandler},
    {"2nd example command", &SecondExampleCommandHandler},
    {"action.devices.commands.OnOff", &CommandHandlerOnOff},
    {"action.devices.commands.headphone.UpdateNoiseCancellation", &CommandHandlerNoiseCancellation},
    {"action.devices.commands.headphone.UpdateAmbientMode", &CommandHandlerAmbientMode},
    {"action.devices.commands.QueryBatteryLevel", &CommandHandlerQueryBatteryLevel},
    {"action.devices.commands.UpdateTouchControl", &CommandHandlerUpdateTouchControl},
};
#define NUMBER_OF_COMMANDS (sizeof(commands)/sizeof(ExecutionEntry))

static const GSoundDeviceActionsInterface *gsound_device_actions_interface = NULL;
static uint8_t encoded_buffer[NUM_BUFFERS][ENCODING_BUFFER_SIZE];


static void GSoundDeviceActionsBatteryTimerCb(void const *n);
osTimerDef(GSOUND_DEVICE_ACTIONS_BATTERY_TIMER, GSoundDeviceActionsBatteryTimerCb);
static osTimerId gsound_device_actions_battery_timer;

static void GSoundDeviceActionsNcTimerCb(void const *n);
osTimerDef(GSOUND_DEVICE_ACTIONS_NC_TIMER, GSoundDeviceActionsNcTimerCb);
static osTimerId gsound_device_actions_nc_timer;

static void GSoundDeviceActionsDelayedInitTimerCb(void const *n);
osTimerDef(GSOUND_DEVICE_ACTIONS_DELAYED_INIT_TIMER, GSoundDeviceActionsDelayedInitTimerCb);
static osTimerId gsound_device_actions_delayed_init_timer;

static void GSoundDeviceActionsStateUpdateRequestCb(void const *n);
osTimerDef(GSOUND_DEVICE_ACTIONS_STATE_REQUESTED_TIMER, GSoundDeviceActionsStateUpdateRequestCb);
static osTimerId gsound_device_actions_state_update_request_timer;

static void SerializeMessage(const pb_msgdesc_t fields[], const void *msg,
                                int *output_size, int buffer_number) {
    GLOG_D("devAction SerializeMessage Entry.");
    size_t encoded_size = 0;

    // Compute the encoded length and make sure it will fit
    if(!pb_get_encoded_size(&encoded_size, fields, msg))
    {
        // error
        GLOG_E("devAction SerializeMessage compute length bad. encoded_size=%d", encoded_size);
        return;
    }
    *output_size = encoded_size;

    // Encode the message into the buffer
    pb_ostream_t stream = pb_ostream_from_buffer(encoded_buffer[buffer_number],
                                               ENCODING_BUFFER_SIZE);
    if (!pb_encode(&stream, fields, msg))
    {
        // error
        GLOG_E("devAction SerializeMessage encode bad.");
        return;
    }

#ifdef TRACE_PRINT_MESSAGE_CONTENTS
    uint8_t *ptr = encoded_buffer[buffer_number];
    int len = encoded_size;
    GLOG_I("devAction:");
    DUMP8("0x%02x ", ptr, len);
#endif /* TWS_TRACE_PRINT_MESSAGE_CONTENTS */

    GLOG_D("devAction SerializeMessage Normal exit.");
    return;
}

static bool DeserializeMessage (uint32_t size, const uint8_t *buf, void *obj, const pb_msgdesc_t *fields)
{
    pb_istream_t stream = pb_istream_from_buffer(buf, size);
    bool rc = pb_decode(&stream, fields, obj);
    if(!rc)
    {
        GLOG_E( "devAction %s err", __func__);
    }
    return rc;
}

static void DeviceActionsSendModelId (void)
{
    if(gsound_device_actions_interface == NULL)
    {
        // Error
        GLOG_E("devAction DeviceActionsSendModelId no interface.");
        return;
    }

    gsound_device_actions_interface->gsound_target_device_actions_model_id(
        device_action_model_id, device_action_model_id_len);
        
    GLOG_D("devAction DeviceActionsSendModelId complete.");
}

// current_battery_percentage 0 is not permitted due to proto3
static void DeviceActionsSendBatteryState (int32_t current_battery_percentage)
{
    if(current_battery_percentage == 0)
    {
        // Error
        GLOG_E("devAction DeviceActionsSendBatteryState current battery percent bad.");
        return;
    }

    google_assistant_devices_headphone_v1_CurrentDeviceStatus msg;
    memset (&msg, 0, sizeof(msg));
    msg.battery_details[0].connected = true;
    msg.battery_details[0].device_type = google_assistant_devices_headphone_v1_BatteryDetails_DeviceType_LEFT_EAR_BUD;
    msg.battery_details[0].remaining_battery_percentage = current_battery_percentage;
    msg.battery_details[1].connected = true;
    msg.battery_details[1].device_type = google_assistant_devices_headphone_v1_BatteryDetails_DeviceType_RIGHT_EAR_BUD;
    msg.battery_details[1].remaining_battery_percentage = current_battery_percentage + 2;
    msg.battery_details[2].connected = true;
    msg.battery_details[2].device_type = google_assistant_devices_headphone_v1_BatteryDetails_DeviceType_EAR_BUD_CASE;
    msg.battery_details[2].remaining_battery_percentage = current_battery_percentage + 4;
    msg.battery_details_count = 3;

    int output_size = 0;
    SerializeMessage (google_assistant_devices_headphone_v1_CurrentDeviceStatus_fields,
        &msg, &output_size, BUFFER_BATTERY);

    // Compute the encoded length and make sure it will fit
    if((output_size == 0) || (output_size > ENCODING_BUFFER_SIZE))
    {
        // Error
        GLOG_E("devAction DeviceActionsSendBatteryState compute length bad.output_size=%d", output_size);
        return;
    }

    if(gsound_device_actions_interface == NULL)
    {
        // Error
        GLOG_E("devAction DeviceActionsSendBatteryState no interface.");
        return;
    }

    gsound_device_actions_interface->gsound_target_device_actions_state_update(
        action_state_key2, action_state_key2_len, encoded_buffer[BUFFER_BATTERY], output_size);
        
    GLOG_D("devAction DeviceActionsSendBatteryState complete.");
}

// NC level of 0 is not permitted due to proto3.  Start with OFF == 1 probably.
static void DeviceActionsSendNcState(int32_t current_noise_cancellation)
{
    google_assistant_devices_headphone_v1_CurrentDeviceStatus msg;
    memset(&msg, 0, sizeof(msg));
    msg.noise_cancellation_level = current_noise_cancellation;

    int output_size = 0;
    SerializeMessage(google_assistant_devices_headphone_v1_CurrentDeviceStatus_fields,
                        &msg, &output_size, BUFFER_NC);

    // Compute the encoded length and make sure it will fit
    if((output_size == 0) || (output_size > ENCODING_BUFFER_SIZE))
    {
    // Error
        GLOG_E("devAction DeviceActionsSendNcState compute length bad. output_size=%d", output_size);
        return;
    }

    if(gsound_device_actions_interface == NULL)
    {
        // Error
        GLOG_E("devAction DeviceActionsSendNcState no interface.");
        return;
    }

    gsound_device_actions_interface->gsound_target_device_actions_state_update (
        action_state_key1, action_state_key1_len, encoded_buffer[BUFFER_NC], output_size);
    GLOG_D("devAction DeviceActionsSendNcState complete.");
}



// Called by one-shot timer after every request for state update.
static void GSoundDeviceActionsStateUpdateRequestCb(void const *n)
{
    GLOG_D("devAction GSoundDeviceActionsStateUpdateRequestCb Entry.");

    DeviceActionsSendModelId();
    // Note that battery and NC may have already been modified before arriving
    // here the first time.
    DeviceActionsSendBatteryState(battery_level);
    DeviceActionsSendNcState(nc_level);
}

// Emulate battery by incrementing battery level on every timer interval.
static void GSoundDeviceActionsBatteryTimerCb(void const *n)
{
    GLOG_D("devAction GSoundDeviceActionsBatteryTimerCb Entry.");

    DeviceActionsSendBatteryState(battery_level++);

    if(battery_level > 95)
    {
        battery_level = 1;
    }
    osTimerStart(gsound_device_actions_battery_timer,
        GSOUND_DEVICE_ACTIONS_BATTERY_POLLING_INTERVAL_MSEC);
}

// Emulate noise cancellation level by incrementing nc level on every timer
// interval.
static void GSoundDeviceActionsNcTimerCb(void const *n)
{
    GLOG_D("devAction GSoundDeviceActionsNcTimerCb Entry.");

    DeviceActionsSendNcState(nc_level++);

    if (nc_level > 9)
    {
        nc_level = 1;
    }
    osTimerStart(gsound_device_actions_nc_timer,
        GSOUND_DEVICE_ACTIONS_NC_POLLING_INTERVAL_MSEC);
}

static void GSoundDeviceActionsDelayedInitTimerCb(void const *n)
{
    GLOG_D("devAction GSoundDeviceActionsDelayedInitTimerCb Entry.");

    gsound_device_actions_battery_timer = osTimerCreate(
        osTimer(GSOUND_DEVICE_ACTIONS_BATTERY_TIMER), osTimerOnce, NULL);
    if(gsound_device_actions_battery_timer != NULL)
    {
        GLOG_I("devAction GSoundDeviceActionsDelayedInitTimerCb created battery timer.");
        osTimerStart(gsound_device_actions_battery_timer,
            GSOUND_DEVICE_ACTIONS_BATTERY_POLLING_INTERVAL_MSEC);
    }

    gsound_device_actions_nc_timer = osTimerCreate(
        osTimer(GSOUND_DEVICE_ACTIONS_NC_TIMER), osTimerOnce, NULL);
    if(gsound_device_actions_nc_timer != NULL)
    {
        GLOG_I("devAction GSoundDeviceActionsDelayedInitTimerCb created NC timer.");
        osTimerStart(gsound_device_actions_nc_timer,
            GSOUND_DEVICE_ACTIONS_NC_POLLING_INTERVAL_MSEC);
    }

    // create but do not start the send model id timer.
    gsound_device_actions_state_update_request_timer = osTimerCreate(
        osTimer(GSOUND_DEVICE_ACTIONS_STATE_REQUESTED_TIMER), osTimerOnce, NULL);
    if(gsound_device_actions_state_update_request_timer != NULL)
    {
        GLOG_I("devAction GSoundDeviceActionsDelayedInitTimerCb created state update request timer.");
    }
    GLOG_D("devAction GSoundDeviceActionsDelayedInitTimerCb complete.");
}



static GSoundStatus ErrorCommandHandler (const google_assistant_v1_Execution *msg) 
{
    GLOG_D("devAction received an error command.");
    
    return GSOUND_STATUS_ERROR;
}

/*
 * These are command handlers placeholders.
 *
 * The first example command expects exactly 2 params. The first param must be a
 * number (type double). The second param must be a string.
 *
 * The second and third example commands are not implemented.
 */
#define PARAM1_NAME_LEN (sizeof(msg->params[0].key))
#define PARAM2_NAME_LEN (sizeof(msg->params[0].key))
static GSoundStatus
FirstExampleCommandHandler (const google_assistant_v1_Execution *msg)
{
    // Check all of the expected parameters.
    if(msg->params_count != 2)
    {
        GLOG_D("devAction FirstExampleCommandHandler invalid number of params %d.",
            msg->params_count);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[0].key, "param name 1", PARAM1_NAME_LEN) != 0) ||
        (msg->params[0].value.which_kind != google_protobuf_Value_number_value_tag))
    {
        GLOG_D("devAction FirstExampleCommandHandler invalid param %s or invalid value"
            "type %d.", msg->params[0].key, msg->params[0].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[1].key, "param name 2", PARAM2_NAME_LEN) != 0) ||
        (msg->params[1].value.which_kind != google_protobuf_Value_string_value_tag))
    {
        GLOG_D("devAction FirstExampleCommandHandler invalid param %s or invalid value"
            "type %d.", msg->params[1].key, msg->params[1].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }

    // Now process command
    // msg->params[0].value.kind.number_value contains a double value
    // msg->params[1].value.kind.string_value contains a string value
    return GSOUND_STATUS_NOT_SUPPORTED;
}

static GSoundStatus SecondExampleCommandHandler (const google_assistant_v1_Execution *msg)
{
    // fields available for use.
    //      msg.params_count;
    //      msg.params[n].key; // strncmp to ensure valid
    //      msg.params[n].value.which_kind;
    //      msg.params[n].value.kind.bool_value;
    //      msg.params[n].value.kind.null_value;
    //      msg.params[n].value.kind.number_value;
    //      msg.params[n].value.kind.string_value;

    return GSOUND_STATUS_NOT_SUPPORTED;
}

static GSoundStatus CommandHandlerOnOff(const google_assistant_v1_Execution *msg)
{
    // Check all of the expected parameters.
    if (msg->params_count != 1)
    {
        GLOG_D("devAction CommandHandlerOnOff invalid number of params %d.",
            msg->params_count);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[0].key, "on", PARAM1_NAME_LEN) != 0) ||
        (msg->params[0].value.which_kind != google_protobuf_Value_bool_value_tag))
    {
        GLOG_D("devAction CommandHandlerOnOff invalid param %s or invalid value type %d.", 
            msg->params[0].key, msg->params[0].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }

    // Now process command.
    if(msg->params[0].value.kind.bool_value)
    {
        // On a real headset, "on" isn't useful since the headset can't receive
        // a device action command when off.  However, it's useful on dev board.
        GLOG_I( "devAction CommandHandlerOnOff tuning on power.");
    }
    else
    {
        GLOG_I( "devAction CommandHandlerOnOff tuning off power.");
    }
    
    return GSOUND_STATUS_OK;
}
static GSoundStatus CommandHandlerNoiseCancellation(const google_assistant_v1_Execution *msg)
{
    // Check all of the expected parameters.
    if (msg->params_count != 1)
    {
        GLOG_D("devAction CommandHandlerNoiseCancellation invalid number of params %d.",
            msg->params_count);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[0].key, "level", PARAM1_NAME_LEN) != 0) ||
        (msg->params[0].value.which_kind != google_protobuf_Value_number_value_tag))
    {
        GLOG_D("devAction NoiseCancellation invalid param %s or invalid value type %d.", 
            msg->params[0].key, msg->params[0].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }

    // Now process command.
    if(-1 == msg->params[0].value.kind.number_value)
    {
        // On a real headset, "on" isn't useful since the headset can't receive
        // a device action command when off.  However, it's useful on dev board.
        GLOG_I( "devAction NoiseCancellation tuning on.");
    }
    else if(0 == msg->params[0].value.kind.number_value)
    {
        GLOG_I( "devAction NoiseCancellation tuning off.");
    }
    else
    {
        GLOG_W("devAction CommandHandlerAmbientMode number_value %d.", 
            (int)(msg->params[0].value.kind.number_value));
    }

    return GSOUND_STATUS_OK;
}
static GSoundStatus CommandHandlerAmbientMode(const google_assistant_v1_Execution *msg)
{
    // Check all of the expected parameters.
    if (msg->params_count != 1)
    {
        GLOG_D("devAction CommandHandlerAmbientMode invalid number of params %d.",
            msg->params_count);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[0].key, "level", PARAM1_NAME_LEN) != 0) ||
        (msg->params[0].value.which_kind != google_protobuf_Value_number_value_tag))
    {
        GLOG_D("devAction CommandHandlerAmbientMode invalid param %s or invalid value type %d.", 
            msg->params[0].key, msg->params[0].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }
    

    // Now process command.
    if(-1 == msg->params[0].value.kind.number_value)
    {
        // On a real headset, "on" isn't useful since the headset can't receive
        // a device action command when off.  However, it's useful on dev board.
        GLOG_I( "devAction CommandHandlerAmbientMode tuning on.");
    }
    else if(0 == msg->params[0].value.kind.number_value)
    {
        GLOG_I( "devAction CommandHandlerAmbientMode tuning off.");
    }
    else
    {
        GLOG_I("devAction CommandHandlerAmbientMode number_value %d.", 
            (int)(msg->params[0].value.kind.number_value));
    }
    
    return GSOUND_STATUS_OK;
}
static GSoundStatus CommandHandlerQueryBatteryLevel(const google_assistant_v1_Execution *msg)
{
    GLOG_D("devAction CommandHandlerQueryBatteryLevel command.");

    return GSOUND_STATUS_OK;

}
static GSoundStatus CommandHandlerUpdateTouchControl(const google_assistant_v1_Execution *msg)
{
    // Check all of the expected parameters.
    if (msg->params_count != 1)
    {
        GLOG_D("devAction UpdateTouchControl invalid number of params %d.",
            msg->params_count);
        return GSOUND_STATUS_ERROR;
    }
    if((strncmp(msg->params[0].key, "on", PARAM1_NAME_LEN) != 0) ||
        (msg->params[0].value.which_kind != google_protobuf_Value_bool_value_tag))
    {
        GLOG_D("devAction UpdateTouchControl invalid param %s or invalid value type %d.", 
            msg->params[0].key, msg->params[0].value.which_kind);
        return GSOUND_STATUS_ERROR;
    }

    // Now process command.
    //Whether touch controls are enabled (true) or disabled (false)
    if(msg->params[0].value.kind.bool_value)
    {
        // On a real headset, "on" isn't useful since the headset can't receive
        // a device action command when off.  However, it's useful on dev board.
        GLOG_I( "devAction UpdateTouchControl tuning on.");
    }
    else
    {
        GLOG_I( "devAction UpdateTouchControl tuning off.");
    }
    
    return GSOUND_STATUS_OK;
}

static GSoundStatus HandleExecution(uint16_t len, const uint8_t *data)
{
    unsigned int cmd_idx;
    GLOG_D("devAction HandleExecution Entry.");
    google_assistant_v1_Execution msg;
    memset (&msg, 0, sizeof(msg));

    if(DeserializeMessage(len, data, &msg, google_assistant_v1_Execution_fields))
    {
        // assume that only one command will match.
        for(cmd_idx=0; cmd_idx<NUMBER_OF_COMMANDS; cmd_idx++)
        {
            if(strncmp(commands[cmd_idx].command, msg.command, sizeof(msg.command)) == 0)
            {
                GLOG_I( "devAction received command %s", commands[cmd_idx].command);
                return commands[cmd_idx].execution_handler(&msg);
            }
        }
        GLOG_W("devAction HandleExecution no command found for %s.", msg.command);
        
        return GSOUND_STATUS_NOT_SUPPORTED;
    }
    GLOG_E("devAction HandleExecution deserialize error.");
    return GSOUND_STATUS_ERROR;
}

GSoundStatus gsound_custom_device_actions_request_state(void)
{
    GLOG_D("%s", __func__);

   if (gsound_device_actions_state_update_request_timer != NULL)
   {
      GLOG_D("devAction GSoundTargetDeviceActionsRequestState starting timer.");
      osTimerStart(gsound_device_actions_state_update_request_timer,
                   GSOUND_DEVICE_ACTIONS_RESPONSE_TO_STATE_UPDATE_REQUEST_TIME_MSEC);
  }

  return GSOUND_STATUS_OK;
}

void gsound_custom_device_actions_state_update_consumed(const uint8_t *const key)
{
    GLOG_I("%s", __func__);
}

GSoundStatus gsound_custom_device_actions_excute(const uint8_t *const action,
                                                 uint32_t actionLen)
{
    GLOG_D("%s", __func__);

    GSoundStatus status = HandleExecution(actionLen, action);
   
    return status;
}

void gsound_custom_device_actions_store_lib_interface(const GSoundDeviceActionsInterface *handlers)
{
    GLOG_D("%s", __func__);

    gsound_device_actions_interface = handlers;
}

void gsound_custom_deivce_acionts_update_init(void)
{
    GLOG_D("%s", __func__);

    gsound_device_actions_delayed_init_timer = osTimerCreate(
    osTimer(GSOUND_DEVICE_ACTIONS_DELAYED_INIT_TIMER), osTimerOnce, NULL);
    if(gsound_device_actions_delayed_init_timer != NULL)
    {
        GLOG_I("devAction GSoundTargetDeviceActionsInit created startup timer.");
        osTimerStart(gsound_device_actions_delayed_init_timer,
            GSOUND_DEVICE_ACTIONS_DELAYED_INIT_TIME_MSEC);
    }

}

