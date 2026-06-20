/**
 ****************************************************************************************
 *
 * @file app_smartvoice_ble.h
 *
 * @brief Smart Voice Application entry point
 *
 * Copyright (C) BES
 *
 *
 ****************************************************************************************
 */

#ifndef APP_VOICE_BLE_H_
#define APP_VOICE_BLE_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#ifdef DUAL_MIC_RECORDING

#include <stdint.h>          // Standard Integer Definition

/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif


/**
 ****************************************************************************************
 * @brief Add a DataPath Server instance in the DB
 ****************************************************************************************
 */

bool app_recording_send_cmd_via_notification(uint8_t* ptrData, uint32_t length);

void app_recording_send_cmd_via_indication(uint8_t* ptrData, uint32_t length);

bool app_recording_send_data_via_notification(uint8_t* ptrData, uint32_t length);

void app_recording_send_data_via_indication(uint8_t* ptrData, uint32_t length);

#ifdef __cplusplus
    }
#endif


#endif //__CUSTOMIZE_VOICE__

/// @} APP

#endif // APP_SMARTVOICE_BLE_H_
