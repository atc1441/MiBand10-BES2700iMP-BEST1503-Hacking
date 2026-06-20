#ifndef __APP_TENCENT_SV_CMD_CODE_H__
#define __APP_TENCENT_SV_CMD_CODE_H__

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t __custom_handler_table_start[];
extern uint32_t __custom_handler_table_end[];

#define INVALID_CUSTOM_ENTRY_INDEX      0xFFFF
#define TENCENT_SMARTVOICE_CMD_CODE_SIZE        sizeof(uint8_t)


/**
 * @brief The command code
 *
 */ 

typedef enum {

    APP_TENCENT_START_DATA_XFER=0,
    APP_TENCENT_VERIFY_DATA_SEGMENT,
    APP_TENCENT_INFORM_THROUGHPUT_TEST_CONFIG,
    APP_TENCENT_THROUGHPUT_TEST_DATA,
    APP_TENCENT_THROUGHPUT_TEST_DATA_ACK,
    APP_TENCENT_THROUGHPUT_TEST_DONE,

    APP_TENCENT_RESPONSE_TO_CMD=100,
	APP_TENCENT_SMARTVOICE_WAKEUP_APP,  //101
	APP_TENCENT_SMARTVOICE_STOP_VOICE_STREAM,   //102
	APP_TENCENT_SMARTVOICE_GET_PID_MAC,   //103
	APP_TENCENT_SMARTVOICE_START_VOICE_STREAM,  //104
	APP_TENCENT_SMARTVOICE_INFORM_ALGORITHM_TYPE,   //105
	APP_TENCENT_SMARTVOICE_PHONEID,   
	APP_TENCENT_SMARTVOICE_STOP_APP_PLAYING,  //the earphone and app both can stop the play
	APP_TENCENT_SMARTVOICE_REQUEST_FOR_STOP,
	APP_TENCENT_SMARTVOICE_CLEAN_FLAG_VALUE =110,
	APP_TENCENT_SMARTVOICE_PLAY_CONTROL =111,
	APP_TENCENT_SMARTVOICE_HEARTBEAT =112,

    APP_TENCENT_SMARTVOICE_REQUEST_DISCONNECT = 113,
    APP_TENCENT_SMARTVOICE_GENERAL_CONFIG = 114,
    APP_TENCENT_SMARTVOICE_CUSTOM_SKILLS = 115,
    APP_TENCENT_OTA_CANCEL=247,
    APP_TENCENT_OTA_RESULT = 248,
    APP_TENCENT_OTA_FIRMWARE_VERIFY,  //249
    APP_TENCENT_OTA_SEGMENT_VERIFY,   //250
    APP_TENCENT_OTA_DATA_PACKET,   //251
    APP_TENCENT_OTA_RESUME_VERIFY, //252
    APP_TENCENT_OTA_CONFIG,   //253
    APP_TENCENT_OTA_START,   //254
    // TODO: add new command code here
    APP_TENCENT_COMMAND_COUNT,

    APP_TENCENT_DATA_XFER = 0xFF,  // to mark that it's a data packet

} APP_TENCENT_SV_CMD_CODE_E;

/**
 * @brief The smart voice command handling return status
 *
 */
 
typedef enum {
    TENCENT_NO_ERROR,
	TENCENT_INVALID_COMMAND_CODE,
    TENCENT_PARAM_LEN_OUT_OF_RANGE,
	TENCENT_PARAM_LEN_TOO_SHORT,
	TENCENT_COMMAND_HANDLING_FAILED,
	TENCENT_WAKEUP_FAIL_NEED_SIGN,
	TENCENT_WAITING_RESPONSE_TIMEOUT,
	TENCENT_DATA_TRANSFER_HAS_ALREADY_BEEN_STARTED,
	TENCENT_DATA_TRANSFER_HAS_NOT_BEEN_STARTED,
	TENCENT_DATA_SEGMENT_CRC_CHECK_FAILED,
	TENCENT_WHOLE_DATA_CRC_CHECK_FAILED,
	TENCENT_DATA_TRANSFER_LENGTH_DOSENOT_MATCH, 
} APP_TENCENT_SV_CMD_RET_STATUS_E;


/**
 * @brief Format of the smart voice command handler function, called when the command is received
 *
 * @param cmdCode   Custom command code
 * @param ptrParam  Pointer of the received parameter
 * @param paramLen  Length of the recevied parameter
 *
 */

typedef void (*app_tencent_sv_cmd_handler_t)(APP_TENCENT_SV_CMD_CODE_E cmdCode, uint8_t* ptrParam, uint32_t paramLen,uint8_t device_id);

/**
 * @brief Format of the smart voice command response handler function,
 *  called when the response to formerly sent command is received
 *
 * @param retStatus Handling return status of the command
 * @param ptrParam  Pointer of the received parameter
 * @param paramLen  Length of the recevied parameter
 *
 */
typedef void (*app_tencent_sv_cmd_response_handler_t)(APP_TENCENT_SV_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen,uint8_t device_id);

/**
 * @brief Smart voice command definition data structure
 *
 */
typedef struct {
    uint32_t                cmdCode;
    app_tencent_sv_cmd_handler_t    cmdHandler;             /**< command handler function */
    uint16_t                isNeedResponse;         /**< true if needs the response from the peer device */
    uint16_t                timeoutWaitingRspInMs;  /**< time-out of waiting for response in milli-seconds */
    app_tencent_sv_cmd_response_handler_t   cmdRspHandler;  /**< command response handler function */
} APP_TENCENT_SV_CMD_INSTANCE_T;


#define CUSTOM_COMMAND_TO_ADD(cmdCode, cmdHandler, isNeedResponse, timeoutWaitingRspInMs, cmdRspHandler)    \
    static const APP_TENCENT_SV_CMD_INSTANCE_T cmdCode##_entry __attribute__((used, section(".custom_handler_table"))) =  \
        {(cmdCode), (cmdHandler), (isNeedResponse), (timeoutWaitingRspInMs), (cmdRspHandler)};

#define CUSTOM_COMMAND_PTR_FROM_ENTRY_INDEX(index)  \
    ((APP_TENCENT_SV_CMD_INSTANCE_T *)((uint32_t)__custom_handler_table_start + (index)*sizeof(APP_TENCENT_SV_CMD_INSTANCE_T)))



#ifdef __cplusplus
}
#endif


#endif // #ifndef __APP_TENCENT_SV_CMD_CODE_H__

