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
#ifndef __APP_TOTA_CMD_CODE_H__
#define __APP_TOTA_CMD_CODE_H__

#include "hal_trace.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_TAG "[TOTA_CONTROL] "

#define TOTA_CONTROL_DEBUG

#ifdef TOTA_CONTROL_DEBUG
#define TOTA_LOG_DBG(num,str,...)   TRACE(num,LOG_TAG"" str, ##__VA_ARGS__)             // DEBUG OUTPUT
#define TOTA_LOG_MSG(num,str,...)   TRACE(num,LOG_TAG"" str, ##__VA_ARGS__)             // MESSAGE OUTPUT
#define TOTA_LOG_ERR(num,str,...)   TRACE(num,LOG_TAG"err:""" str, ##__VA_ARGS__)       // ERROR OUTPUT

#define TOTA_LOG_FUNC_LINE()    TRACE(2,LOG_TAG"%s:%d\n", __FUNCTION__, __LINE__)
#define TOTA_LOG_FUNC_IN()      TRACE(1,LOG_TAG"%s ++++\n", __FUNCTION__)
#define TOTA_LOG_FUNC_OUT()     TRACE(1,LOG_TAG"%s ----\n", __FUNCTION__)

#define TOTA_LOG_DUMP		   DUMP8
#else
#define TOTA_LOG_DBG(str,...)
#define TOTA_LOG_MSG(num,str,...)   TRACE(num,LOG_TAG"" str, ##__VA_ARGS__)
#define TOTA_LOG_ERR(num,str,...)   TRACE(num,LOG_TAG"err:""" str, ##__VA_ARGS__)

#define TOTA_LOG_FUNC_LINE()
#define TOTA_LOG_FUNC_IN()
#define TOTA_LOG_FUNC_OUT()

#define TOTA_LOG_DUMP
#endif

extern uint32_t __tota_handler_table_start[];
extern uint32_t __tota_handler_table_end[];

#define INVALID_TOTA_ENTRY_INDEX		0xFFFF
#define TOTA_CMD_CODE_SIZE				sizeof(uint16_t)

/**
 * @brief Type of the tota transmission path.
 *
 */
typedef enum
{
	APP_TOTA_VIA_SPP = 0,
	APP_TOTA_VIA_NOTIFICATION,
	APP_TOTA_VIA_INDICATION,
	APP_TOTA_TRANSMISSION_PATH_COUNT,

	APP_TOTA_PATH_IDLE = 0xff
} APP_TOTA_TRANSMISSION_PATH_E;
	
/**
 * @brief The command code
 *
 */
typedef enum
{
    /* 0 */ OP_TOTA_RESPONSE_TO_CMD = 0x8000, /**< the payload is: OP_TOTA_RESPONSE_TO_CMD + paramLen + BLE_TOTA_CMD_RSP_T */
    /* 1 */ OP_TOTA_START_DATA_XFER = 0x8001,
    /* 2 */ OP_TOTA_VERIFY_DATA_SEGMENT = 0x8002,
    /* 3 */ OP_TOTA_STOP_TOTA_DATA_XFER = 0x8003,
    /* 4 */ OP_TOTA_SPP_DATA_ACK = 0x8004,

    /* 5 */ OP_TOTA_DEMO_CMD = 0x8005,
    /* 6 */ OP_TOTA_GENERAL_CMD = 0x8006,

#if defined(TOTA_CRASH_DUMP_TOOL_ENABLE)
    OP_TOTA_CRASH_DUMP_REQ = 0x8050,
    OP_TOTA_CRASH_DUMP_PARAM_REQ = 0x8051,
    OP_TOTA_CRASH_DUMP_START_REQ = 0x8052,
    OP_TOTA_CRASH_DUMP_RECEIVED_ACK = 0x8053,
    OP_TOTA_CRASH_DUMP_END = 0x8054,
    OP_TOTA_CRASH_DUMP_ERASE_FLASH = 0x8055,
    OP_TOTA_CRASH_DUMP_HEART_BEAT = 0x8056,
#endif

    // TODO: add new command code here
    OP_TOTA_BATTERY_STATUS_CMD = 0x9001,
    OP_TOTA_MERIDIAN_EFFECT_CMD = 0x9002,
    OP_TOTA_EQ_SELECT_CMD = 0x9003,
    OP_TOTA_VOLUME_PLUS_CMD = 0x9004,
    OP_TOTA_VOLUME_DEC_CMD = 0x9005,
    OP_TOTA_VOLUME_SET_CMD,
    OP_TOTA_VOLUME_GET_CMD,
    OP_TOTA_EQ_SET_CMD,
    OP_TOTA_EQ_GET_CMD,
    OP_TOTA_FWVERSION_GET_CMD,
    OP_TOTA_RSSI_GET_CMD,
    OP_TOTA_ANC_STATUS_SYNC_CMD=0x900C,
    OP_TOTA_ANC_ONOFF_WRITE_CMD=0x900D,
    OP_TOTA_ANC_LEVEL_WRITE_CMD=0x900E,
    OP_TOTA_ANC_STATUS_REPORT_CMD=0x900F,
    OP_TOTA_ANC_TOTAL_GAIN_WRITE_CMD=0x9020,

    OP_TOTA_AUDIO_DUMP_CMD=0x9030,
    OP_TOTA_RAW_DATA_SET_CMD = 0x9100,
    OP_TOTA_COMMAND_COUNT,
    
    OP_TOTA_DATA_XFER = 0xFFFF,	// to mark that it's a data packet
} APP_TOTA_CMD_CODE_E;

/**
 * @brief The tota command handling return status
 *
 */
typedef enum
{
	TOTA_NO_ERROR = 0,
	TOTA_INVALID_CMD = 1,
	TOTA_INVALID_DATA_PACKET = 2,
	TOTA_PARAM_LEN_OUT_OF_RANGE = 3,
	TOTA_PARAMETER_LENGTH_TOO_SHORT = 4,
	TOTA_PARAM_LEN_TOO_SHORT = 5,	
	TOTA_CMD_HANDLING_FAILED = 6,
	TOTA_WAITING_RSP_TIMEOUT = 7,
	TOTA_DATA_XFER_ALREADY_STARTED = 8,
	TOTA_DATA_XFER_NOT_STARTED_YET = 9,
	TOTA_DATA_SEGMENT_CRC_CHECK_FAILED = 10,
	TOTA_WHOLE_DATA_CRC_CHECK_FAILED = 11,
	TOTA_DATA_XFER_LEN_NOT_MATCHED = 12,	
	
	// TO ADD: new return status
} APP_TOTA_CMD_RET_STATUS_E;

/**
 * @brief Format of the tota command handler function, called when the command is received
 *
 * @param cmdCode	Custom command code
 * @param ptrParam 	Pointer of the received parameter
 * @param paramLen 	Length of the recevied parameter
 * 
 */
typedef void (*app_tota_cmd_handler_t)(APP_TOTA_CMD_CODE_E cmdCode, uint8_t* ptrParam, uint32_t paramLen);

/**
 * @brief Format of the tota command response handler function, 
 *	called when the response to formerly sent command is received
 *
 * @param retStatus	Handling return status of the command
 * @param ptrParam 	Pointer of the received parameter
 * @param paramLen 	Length of the recevied parameter
 * 
 */
typedef void (*app_tota_cmd_response_handler_t)(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen);

/**
 * @brief Smart voice command definition data structure
 *
 */
typedef struct
{
	uint32_t				cmdCode;
    app_tota_cmd_handler_t 	cmdHandler;  			/**< command handler function */
    uint16_t 				isNeedResponse;     	/**< true if needs the response from the peer device */
	uint16_t				timeoutWaitingRspInMs;	/**< time-out of waiting for response in milli-seconds */
	app_tota_cmd_response_handler_t	cmdRspHandler;	/**< command response handler function */
} APP_TOTA_CMD_INSTANCE_T;


#define TOTA_COMMAND_TO_ADD(cmdCode, cmdHandler, isNeedResponse, timeoutWaitingRspInMs, cmdRspHandler)	\
	static const APP_TOTA_CMD_INSTANCE_T cmdCode##_entry __attribute__((used, section(".tota_handler_table"))) = 	\
		{(cmdCode), (cmdHandler), (isNeedResponse), (timeoutWaitingRspInMs), (cmdRspHandler)};

#define TOTA_COMMAND_PTR_FROM_ENTRY_INDEX(index)	\
	((APP_TOTA_CMD_INSTANCE_T *)((uint32_t)__tota_handler_table_start + (index)*sizeof(APP_TOTA_CMD_INSTANCE_T)))

#ifdef __cplusplus
	}
#endif


#endif // #ifndef __APP_TOTA_CMD_CODE_H__

