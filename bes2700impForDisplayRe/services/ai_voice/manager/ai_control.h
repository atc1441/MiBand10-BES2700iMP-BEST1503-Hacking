#ifndef __AI_CONTROL_H__
#define __AI_CONTROL_H__


#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

#define AI_CMD_HEAD_SIZE 4
#define ERROR_RETURN 0xFFFF

// cmd code for AI Voice Profile 
typedef enum
{
    API_HANDLE_INIT = 0,
    API_START_ADV,
    API_DATA_HANDLE,
    API_DATA_SEND,
    API_DATA_INIT,
    API_DATA_DEINIT,
    API_AI_RESET,
    API_AI_ROLE_SWITCH,
    API_AI_EXC_SECRET_KEY,
    API_AI_EXC_FLAG,
    CALLBACK_CMD_RECEIVE,
    CALLBACK_DATA_RECEIVE,
    CALLBACK_DATA_PARSE,
    CALLBACK_PEER_OTA_DATA_REV,
    CALLBACK_PEER_OTA_CMD_REV,
    CALLBACK_PEER_OTA_CMD_RSP,
    CALLBACK_REV_PEER_OTA_RSP_OUT,
    CALLBACK_AI_CONNECT,
    CALLBACK_AI_DISCONNECT,
    CALLBACK_UPDATE_MTU,
    CALLBACK_WAKE_UP,
    CALLBACK_AI_ENABLE,
    CALLBACK_START_SPEECH,
    CALLBACK_ENDPOINT_SPEECH,
    CALLBACK_STOP_SPEECH,
    CALLBACK_DATA_SEND_DONE,
    CALLBACK_AI_PARAM_SEND,
    CALLBACK_AI_PARAM_RECEIVE,
    CALLBACK_KEY_EVENT_HANDLE,
    CALLBACK_STOP_REPLYING,
    CALLBACK_AI_APP_KEEPALIVE_POST_HANDLER,  // modifyed
    CALLBACK_STORE_SLAVE_DATA,
#ifdef __TENCENT_VOICE__
	CALLBACK_DISCONNECT_APP,
#endif
    AI_CMD_CODE_NUM,
} AI_FUNC_CMD_M;

typedef enum  {
    AI_CUSTOM_CODE_VOICE_STREAM_INIT    = API_DATA_INIT,
    AI_CUSTOM_CODE_VOICE_STREAM_DEINIT  = API_DATA_DEINIT,
    AI_CUSTOM_CODE_AI_ROLE_SWITCH   = API_AI_ROLE_SWITCH,
    AI_CUSTOM_CODE_CMD_RECEIVE      = CALLBACK_CMD_RECEIVE,
    AI_CUSTOM_CODE_DATA_RECEIVE     = CALLBACK_DATA_RECEIVE,
    AI_CUSTOM_CODE_AI_CONNECT       = CALLBACK_AI_CONNECT,
    AI_CUSTOM_CODE_AI_DISCONNECT    = CALLBACK_AI_DISCONNECT,
    AI_CUSTOM_CODE_UPDATE_MTU       = CALLBACK_UPDATE_MTU,
    AI_CUSTOM_CODE_WAKE_UP          = CALLBACK_WAKE_UP,
    AI_CUSTOM_CODE_START_SPEECH     = CALLBACK_START_SPEECH,
    AI_CUSTOM_CODE_ENDPOINT_SPEECH  = CALLBACK_ENDPOINT_SPEECH,
    AI_CUSTOM_CODE_STOP_SPEECH      = CALLBACK_STOP_SPEECH,
    AI_CUSTOM_CODE_DATA_SEND_DONE   = CALLBACK_DATA_SEND_DONE,
    AI_CUSTOM_CODE_KEY_EVENT_HANDLE = CALLBACK_KEY_EVENT_HANDLE,
    AI_CUSTOM_CODE_AI_ROLE_SWITCH_COMPLETE = AI_CMD_CODE_NUM,
    AI_CUSTOM_CODE_MOBILE_CONNECT,
    AI_CUSTOM_CODE_MOBILE_DISCONNECT,
    AI_CUSTOM_CODE_SETUP_COMPLETE,
} AI_CUSTOM_CODE_E;

typedef enum
{
    AI_CMD_PEER_MIC_OPEN    = 0x9000,
    AI_CMD_PEER_MIC_CLOSE   = 0x9001,
    AI_CMD_GMA_SEND_SECRET  = 0x9002,
    AI_CMD_GMA_OTA_DATA     = 0x9003,
    AI_CMD_GMA_OTA_CMD      = 0x9004,
    AI_CMD_GMA_OTA_CMD_RSP  = 0x9005,
    AI_CMD_DUAL_MIC_OPEN    = 0x9006,
    AI_CMD_DUAL_MIC_CLOSE   = 0x9007,
    AI_CMD_TENCENT_SEND_FLAG = 0x9008,
} AI_FUNC_CMD_TO_PEER_M;

typedef struct
{
    // code of the cmd
    AI_FUNC_CMD_TO_PEER_M cmdcode;
    // length of parameter
    uint16_t param_length;
    // Pointer to the param
    uint8_t *param;
} ai_cmd_instance_t;

// function Identifier. The number of function is limited to 0xFF.
// The function ID parts:
// bits[7~0]: function index(no more than 255 function per AI)
typedef uint8_t ai_func_id_t;

// Format of a AI handler function
typedef uint32_t (*ai_handler_func_t)(void *param1, uint32_t param2, uint8_t param3);

// Element of a AI handler table.
struct ai_func_handler
{
    // Id of the handled function.
    ai_func_id_t id;
    // Pointer to the handler function.
    ai_handler_func_t func;
};

// Element of a state handler table.
struct ai_handler_func_table
{
    // Pointer to the handler function table
    const struct ai_func_handler *func_table;
    // Number of handled function
    uint16_t func_cnt;
};

uint32_t ai_function_handle(AI_FUNC_CMD_M func_id, void *param1, uint32_t param2, uint8_t param3, uint8_t param4);
void app_ai_send_cmd_to_peer(ai_cmd_instance_t *ai_cmd);
void app_ai_send_cmd_to_peer_with_rsp(ai_cmd_instance_t *ai_cmd);
void app_ai_send_cmd_rsp_to_peer(ai_cmd_instance_t *ai_cmd, uint16_t rsp_seq);
void app_ai_rev_peer_cmd_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void app_ai_rev_peer_cmd_rsp_timeout_hanlder(uint16_t rsp_seq, uint8_t *p_buff, uint16_t length);
void ai_control_init(const struct ai_handler_func_table *func_table, uint8_t ai_index);

#ifdef __cplusplus
}
#endif

#endif

