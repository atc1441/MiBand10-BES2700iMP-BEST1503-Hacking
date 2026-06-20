/**
 ****************************************************************************************
 *
 * @file app_tota_data_handler.c
 *
 * @date 24th April 2018
 *
 * @brief The framework of the tota data handler
 *
 * Copyright (C) 2017
 *
 *
 ****************************************************************************************
 */
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "apps.h"
#include "stdbool.h"
#include "app_tota.h" 
#include "app_tota_cmd_code.h"
#include "app_tota_cmd_handler.h"
#include "app_tota_data_handler.h"
#include "bluetooth_ble_api.h"
#include "crc32_c.h"
#include "app_spp_tota.h"


/**
 * @brief tota data handling environment
 *
 */
typedef struct
{
    uint8_t		isDataXferInProgress; 	
	uint8_t		isCrcCheckEnabled;
	uint32_t	wholeDataXferLen;
	uint32_t	dataXferLenUntilLastSegVerification;
	uint32_t	currentWholeCrc32;
	uint32_t 	wholeCrc32UntilLastSeg;
	uint32_t	crc32OfCurrentSeg;											
} APP_TOTA_DATA_HANDLER_ENV_T;

#define APP_TOTA_TX_BUF_SIZE 2046
static uint8_t app_tota_tmpDataXferBuf[APP_TOTA_TX_BUF_SIZE];	
static APP_TOTA_DATA_HANDLER_ENV_T app_tota_data_rec_handler_env;
static APP_TOTA_DATA_HANDLER_ENV_T app_tota_data_trans_handler_env;

void app_tota_data_reset_env(void)
{
	app_tota_data_rec_handler_env.isDataXferInProgress = false;
	app_tota_data_rec_handler_env.isCrcCheckEnabled = false;
	app_tota_data_rec_handler_env.wholeDataXferLen = 0;
	app_tota_data_rec_handler_env.dataXferLenUntilLastSegVerification = 0;
	app_tota_data_rec_handler_env.currentWholeCrc32 = 0;
	app_tota_data_rec_handler_env.wholeCrc32UntilLastSeg = 0;	
	app_tota_data_rec_handler_env.crc32OfCurrentSeg = 0;	

	app_tota_data_trans_handler_env.isDataXferInProgress = false;
	app_tota_data_trans_handler_env.isCrcCheckEnabled = false;
	app_tota_data_trans_handler_env.wholeDataXferLen = 0;
	app_tota_data_trans_handler_env.dataXferLenUntilLastSegVerification = 0;
	app_tota_data_trans_handler_env.currentWholeCrc32 = 0;
	app_tota_data_trans_handler_env.wholeCrc32UntilLastSeg = 0;	
	app_tota_data_trans_handler_env.crc32OfCurrentSeg = 0;		
}

bool app_tota_data_is_data_transmission_started(void)
{
    return app_tota_data_trans_handler_env.isDataXferInProgress;
}

APP_TOTA_CMD_RET_STATUS_E app_tota_data_xfer_stop_handler(APP_TOTA_STOP_DATA_XFER_T* pStopXfer)
{
	APP_TOTA_CMD_RET_STATUS_E retStatus = TOTA_NO_ERROR;
	
	if (pStopXfer->isHasWholeCrcCheck)
	{
		if (pStopXfer->wholeDataLenToCheck != app_tota_data_rec_handler_env.wholeDataXferLen)
		{
			retStatus = TOTA_DATA_XFER_LEN_NOT_MATCHED;
		}

		if (pStopXfer->crc32OfWholeData != app_tota_data_rec_handler_env.currentWholeCrc32)
		{
			retStatus = TOTA_WHOLE_DATA_CRC_CHECK_FAILED;
		}
	}

	app_tota_data_reset_env();
	app_tota_data_rec_handler_env.isDataXferInProgress = false;

	return retStatus;
}

void app_tota_data_xfer_control_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
	APP_TOTA_CMD_RET_STATUS_E retStatus = TOTA_NO_ERROR;
	APP_TOTA_TRANSMISSION_PATH_E dataPath = app_tota_get_datapath();
    TOTA_LOG_DBG(3,"[%s] funcCode = 0x%x, datapath = %d",__func__, funcCode,dataPath);
	if (OP_TOTA_START_DATA_XFER == funcCode)
	{
		if (app_tota_data_rec_handler_env.isDataXferInProgress)
		{
			TOTA_LOG_DBG(1,"[%s]error TOTA_DATA_XFER_ALREADY_STARTED",__func__);
			retStatus = TOTA_DATA_XFER_ALREADY_STARTED;
		}
		else
		{
			if (paramLen < sizeof(APP_TOTA_START_DATA_XFER_T))
			{
				TOTA_LOG_DBG(1,"[%s]error TOTA_PARAMETER_LENGTH_TOO_SHORT",__func__);
				retStatus = TOTA_PARAMETER_LENGTH_TOO_SHORT;
			}
			else if (paramLen > sizeof(APP_TOTA_START_DATA_XFER_T))
			{
				TOTA_LOG_DBG(1,"[%s]error TOTA_PARAM_LEN_OUT_OF_RANGE",__func__);
				retStatus = TOTA_PARAM_LEN_OUT_OF_RANGE;
			}
			else
			{
				app_tota_data_reset_env();
				APP_TOTA_START_DATA_XFER_T* pStartXfer = (APP_TOTA_START_DATA_XFER_T *)ptrParam;
				app_tota_data_rec_handler_env.isDataXferInProgress = true;
				app_tota_data_rec_handler_env.isCrcCheckEnabled = pStartXfer->isHasCrcCheck;
			}
		}
        APP_TOTA_START_DATA_XFER_RSP_T rsp = {0};
        app_tota_send_response_to_command(funcCode, retStatus, (uint8_t *)&rsp, sizeof(rsp), dataPath);
	}
	else if (OP_TOTA_STOP_TOTA_DATA_XFER == funcCode)
	{
		if (!app_tota_data_rec_handler_env.isDataXferInProgress)
		{
			retStatus = TOTA_DATA_XFER_NOT_STARTED_YET;
		}
		else
		{
			if (paramLen < sizeof(APP_TOTA_STOP_DATA_XFER_T))
			{
				retStatus = TOTA_PARAMETER_LENGTH_TOO_SHORT;
			}
			else if (paramLen > sizeof(APP_TOTA_STOP_DATA_XFER_T))
			{
				retStatus = TOTA_PARAM_LEN_OUT_OF_RANGE;
			}
			else
			{
				retStatus = app_tota_data_xfer_stop_handler((APP_TOTA_STOP_DATA_XFER_T *)ptrParam);
			}
		}
        APP_TOTA_STOP_DATA_XFER_RSP_T rsp = {0};
        app_tota_send_response_to_command(funcCode, retStatus, (uint8_t *)&rsp, sizeof(rsp), dataPath);
	}
	else
	{
		retStatus = TOTA_INVALID_CMD;
        app_tota_send_response_to_command(funcCode, retStatus, NULL, 0, dataPath);
	}		
}

__attribute__((weak)) void app_tota_data_xfer_started(APP_TOTA_CMD_RET_STATUS_E retStatus)
{

}

void app_tota_kickoff_dataxfer(void)
{
	app_tota_data_reset_env();
	app_tota_data_trans_handler_env.isDataXferInProgress = true;    
}

void app_tota_start_data_xfer_control_rsp_handler(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen)
{
    if (TOTA_NO_ERROR == retStatus)
	{
		app_tota_kickoff_dataxfer();
	}
	app_tota_data_xfer_started(retStatus);
}

__attribute__((weak)) void app_tota_data_xfer_stopped(APP_TOTA_CMD_RET_STATUS_E retStatus)
{

}

void app_tota_stop_dataxfer(void)
{
	app_tota_data_reset_env();
	app_tota_data_trans_handler_env.isDataXferInProgress = false;    
}

void app_tota_stop_data_xfer_control_rsp_handler(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen)
{
    if ((TOTA_NO_ERROR == retStatus) || (TOTA_WAITING_RSP_TIMEOUT == retStatus))
	{
		app_tota_stop_dataxfer();
	}
	app_tota_data_xfer_stopped(retStatus);
}

void app_tota_data_segment_verifying_handler(APP_TOTA_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen)
{
	APP_TOTA_CMD_RET_STATUS_E retStatus = TOTA_NO_ERROR;
	APP_TOTA_TRANSMISSION_PATH_E dataPath = app_tota_get_datapath();
	if (paramLen < sizeof(APP_TOTA_VERIFY_DATA_SEGMENT_T))
	{
		retStatus = TOTA_PARAMETER_LENGTH_TOO_SHORT;
	}
	else if (paramLen > sizeof(APP_TOTA_VERIFY_DATA_SEGMENT_T))
	{
		retStatus = TOTA_PARAM_LEN_OUT_OF_RANGE;
	}	
	else
	{
		APP_TOTA_VERIFY_DATA_SEGMENT_T* pVerifyData = (APP_TOTA_VERIFY_DATA_SEGMENT_T *)ptrParam;
		if (pVerifyData->segmentDataLen != 
			(app_tota_data_rec_handler_env.wholeDataXferLen - 
			app_tota_data_rec_handler_env.dataXferLenUntilLastSegVerification))
		{
			retStatus = TOTA_DATA_XFER_LEN_NOT_MATCHED;
		}
		else if (pVerifyData->crc32OfSegment != app_tota_data_rec_handler_env.crc32OfCurrentSeg)
		{	
			app_tota_data_rec_handler_env.wholeDataXferLen = app_tota_data_rec_handler_env.dataXferLenUntilLastSegVerification;
			app_tota_data_rec_handler_env.currentWholeCrc32 = app_tota_data_rec_handler_env.wholeCrc32UntilLastSeg;
			retStatus = TOTA_DATA_SEGMENT_CRC_CHECK_FAILED;
		}
		else
		{
			app_tota_data_rec_handler_env.dataXferLenUntilLastSegVerification = app_tota_data_rec_handler_env.wholeDataXferLen;
			app_tota_data_rec_handler_env.wholeCrc32UntilLastSeg = app_tota_data_rec_handler_env.currentWholeCrc32;		
		}

		app_tota_data_rec_handler_env.crc32OfCurrentSeg = 0;
	}

    APP_TOTA_VERIFY_DATA_SEGMENT_RSP_T rsp = {0};
	app_tota_send_response_to_command(funcCode, retStatus, (uint8_t *)&rsp, sizeof(rsp), dataPath);
}

__attribute__((weak)) void app_tota_data_segment_verification_result_report(APP_TOTA_CMD_RET_STATUS_E retStatus)
{
	// should re-send this segment
}

void app_tota_verify_data_segment_rsp_handler(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen)
{
	if (TOTA_NO_ERROR != retStatus)
	{	
		app_tota_data_trans_handler_env.wholeDataXferLen = app_tota_data_rec_handler_env.dataXferLenUntilLastSegVerification;
		app_tota_data_trans_handler_env.currentWholeCrc32 = app_tota_data_trans_handler_env.wholeCrc32UntilLastSeg;
	}
	else
	{
		app_tota_data_trans_handler_env.dataXferLenUntilLastSegVerification = app_tota_data_trans_handler_env.wholeDataXferLen;
		app_tota_data_trans_handler_env.wholeCrc32UntilLastSeg = app_tota_data_trans_handler_env.currentWholeCrc32;		
	}

	app_tota_data_trans_handler_env.crc32OfCurrentSeg = 0;

	app_tota_data_segment_verification_result_report(retStatus);
}
static receive_data_callback recervice_data_cb = NULL;
void app_tota_data_received_callback_handler_register(receive_data_callback cb )
{
    recervice_data_cb = cb;
}
__attribute__((weak)) void app_tota_data_received_callback(uint8_t* ptrData, uint32_t dataLength)
{
	if(recervice_data_cb != NULL){
        recervice_data_cb(ptrData,dataLength);
    }
}

#if 1
/**
 * @brief Receive the data from the peer device and parse them
 *
 * @param ptrData 		Pointer of the received data
 * @param dataLength	Length of the received data
 * 
 * @return APP_TOTA_CMD_RET_STATUS_E
 */
APP_TOTA_CMD_RET_STATUS_E app_tota_data_received(uint8_t* ptrData, uint32_t dataLength)
{
	if ((OP_TOTA_DATA_XFER != (APP_TOTA_CMD_CODE_E)(((uint16_t *)ptrData)[0])) || 
		(dataLength < TOTA_CMD_CODE_SIZE))
	{
		return TOTA_INVALID_DATA_PACKET;
	}

	uint8_t* rawDataPtr = ptrData + TOTA_CMD_CODE_SIZE;
	uint32_t rawDataLen = dataLength - TOTA_CMD_CODE_SIZE;
	APP_TOTA_TRANSMISSION_PATH_E dataPath = app_tota_get_datapath();
	app_tota_data_received_callback(rawDataPtr, rawDataLen);

	app_tota_data_rec_handler_env.wholeDataXferLen += rawDataLen;

	if (app_tota_data_rec_handler_env.isCrcCheckEnabled)
	{
		// calculate the CRC
		app_tota_data_rec_handler_env.currentWholeCrc32 = 
		crc32_c(app_tota_data_rec_handler_env.currentWholeCrc32, rawDataPtr, rawDataLen);
		app_tota_data_rec_handler_env.crc32OfCurrentSeg = 
			crc32_c(app_tota_data_rec_handler_env.crc32OfCurrentSeg, rawDataPtr, rawDataLen);
	}

    APP_TOTA_DATA_ACK_T tAck = {rawDataLen};
    app_tota_send_command(OP_TOTA_SPP_DATA_ACK, (uint8_t *)&tAck, sizeof(tAck), dataPath);	
	return TOTA_NO_ERROR;
}

void app_tota_send_data(APP_TOTA_TRANSMISSION_PATH_E path, uint8_t* ptrData, uint32_t dataLength)
{
	if (path < APP_TOTA_TRANSMISSION_PATH_COUNT)
	{
		((uint16_t *)app_tota_tmpDataXferBuf)[0] = OP_TOTA_DATA_XFER;
		memcpy(app_tota_tmpDataXferBuf + TOTA_CMD_CODE_SIZE, 
			ptrData, dataLength);
		
		app_tota_data_trans_handler_env.wholeDataXferLen += dataLength;
		if (app_tota_data_trans_handler_env.isCrcCheckEnabled)
		{
			app_tota_data_trans_handler_env.currentWholeCrc32 = 
				crc32_c(app_tota_data_trans_handler_env.currentWholeCrc32, ptrData, dataLength);
			app_tota_data_trans_handler_env.crc32OfCurrentSeg = 
				crc32_c(app_tota_data_trans_handler_env.crc32OfCurrentSeg, ptrData, dataLength);
		}
		switch (path)
		{
			case APP_TOTA_VIA_SPP:
                app_tota_send_data_via_spp(app_tota_tmpDataXferBuf, dataLength + TOTA_CMD_CODE_SIZE);
				break;
			default:
				break;
		}	
	}
}
#endif

#if defined(APP_ANC_TEST)
void app_anc_tota_send_data(APP_TOTA_TRANSMISSION_PATH_E path, uint8_t* ptrData, uint32_t dataLength)
{
	if (path < APP_TOTA_TRANSMISSION_PATH_COUNT)
	{
		
		//memcpy(app_tota_tmpDataXferBuf,ptrData, dataLength);
		
		app_tota_data_trans_handler_env.wholeDataXferLen += dataLength;
		if (app_tota_data_trans_handler_env.isCrcCheckEnabled)
		{
			app_tota_data_trans_handler_env.currentWholeCrc32 = 
				crc32_c(app_tota_data_trans_handler_env.currentWholeCrc32, ptrData, dataLength);
			app_tota_data_trans_handler_env.crc32OfCurrentSeg = 
				crc32_c(app_tota_data_trans_handler_env.crc32OfCurrentSeg, ptrData, dataLength);
		}
		switch (path)
		{
			case APP_TOTA_VIA_SPP:
                //app_tota_send_data_via_spp(app_tota_tmpDataXferBuf, dataLength);
                app_tota_send_data_via_spp(ptrData, dataLength);
				break;
			default:
				break;
		}
	}
}
#endif

void app_tota_start_data_xfer(APP_TOTA_TRANSMISSION_PATH_E path, APP_TOTA_START_DATA_XFER_T* req)
{
	app_tota_data_trans_handler_env.isCrcCheckEnabled = req->isHasCrcCheck;
    app_tota_send_command(OP_TOTA_START_DATA_XFER, (uint8_t *)req, sizeof(APP_TOTA_START_DATA_XFER_T), path);	
}

void app_tota_stop_data_xfer(APP_TOTA_TRANSMISSION_PATH_E path, APP_TOTA_STOP_DATA_XFER_T* req)
{
	if (req->isHasWholeCrcCheck)
	{
		req->crc32OfWholeData = app_tota_data_trans_handler_env.currentWholeCrc32;
		req->wholeDataLenToCheck = app_tota_data_trans_handler_env.wholeDataXferLen;
	}
	
	app_tota_send_command(OP_TOTA_STOP_TOTA_DATA_XFER, (uint8_t *)req, sizeof(APP_TOTA_STOP_DATA_XFER_T), path);	
}

void app_tota_verify_data_segment(APP_TOTA_TRANSMISSION_PATH_E path)
{
	APP_TOTA_VERIFY_DATA_SEGMENT_T req;
	req.crc32OfSegment = app_tota_data_trans_handler_env.crc32OfCurrentSeg;
	req.segmentDataLen = (app_tota_data_trans_handler_env.wholeDataXferLen - 
			app_tota_data_trans_handler_env.dataXferLenUntilLastSegVerification);
	
	app_tota_send_command(OP_TOTA_VERIFY_DATA_SEGMENT, (uint8_t *)&req, sizeof(APP_TOTA_VERIFY_DATA_SEGMENT_T), path);	
}

__attribute__((weak)) void app_tota_data_ack_received(uint32_t dataLength)
{

}

#if 0
void app_tota_data_xfer_ack_handler(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen)
{
    APP_TOTA_CMD_RET_STATUS_E ret = TOTA_NO_ERROR;
    
    if (paramLen < sizeof(APP_TOTA_DATA_ACK_T))
	{
		ret = TOTA_PARAMETER_LENGTH_TOO_SHORT;
	}
	else if (paramLen > sizeof(APP_TOTA_DATA_ACK_T))
	{
		ret = TOTA_PARAM_LEN_OUT_OF_RANGE;
	}
    else
    {
        app_tota_data_ack_received(((APP_TOTA_DATA_ACK_T *)ptrParam)->dataLenReceivedByPeerDev);
    }     
}
#else
void app_tota_data_xfer_ack_handler(APP_TOTA_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen)
{
    TOTA_LOG_DBG(2,"retStatus 0x%x, param len %d", retStatus, paramLen);
    app_tota_data_ack_received(((APP_TOTA_DATA_ACK_T *)ptrParam)->dataLenReceivedByPeerDev);
}
#endif

void app_tota_handle_received_data(uint8_t* buffer, uint16_t maxBytes)
{
    TOTA_LOG_DBG(2,"[%s]data receive data length = %d",__func__,maxBytes);
    TOTA_LOG_DUMP("[0x%x]",buffer,(maxBytes>20 ? 20 : maxBytes));
    if (OP_TOTA_DATA_XFER == *(uint16_t *)buffer)
        {
            TRACE(0,"APP TOTA DATA RECEIVED");
            app_tota_data_received(buffer, (uint32_t)maxBytes);
        }
        // otherwise it will be a command packet
        else    
        {
            app_tota_cmd_received(buffer, (uint32_t)maxBytes);
        }
}

TOTA_COMMAND_TO_ADD(OP_TOTA_START_DATA_XFER, 	app_tota_data_xfer_control_handler, true,  APP_TOTA_DATA_CMD_TIME_OUT_IN_MS,  	app_tota_start_data_xfer_control_rsp_handler );
TOTA_COMMAND_TO_ADD(OP_TOTA_STOP_TOTA_DATA_XFER, 	app_tota_data_xfer_control_handler, true,  APP_TOTA_DATA_CMD_TIME_OUT_IN_MS,  	app_tota_stop_data_xfer_control_rsp_handler );
TOTA_COMMAND_TO_ADD(OP_TOTA_VERIFY_DATA_SEGMENT, app_tota_data_segment_verifying_handler, true,  APP_TOTA_DATA_CMD_TIME_OUT_IN_MS,  	app_tota_verify_data_segment_rsp_handler );
TOTA_COMMAND_TO_ADD(OP_TOTA_SPP_DATA_ACK, 	app_tota_data_xfer_control_handler, false,  0,  	app_tota_data_xfer_ack_handler );

