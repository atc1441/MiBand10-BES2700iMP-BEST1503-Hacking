#ifndef _TENCENT_GATT_SERVER_H_
#define _TENCENT_GATT_SERVER_H_

/**
 ****************************************************************************************
 * @addtogroup TENCENT_SMARTVOICETASK Task
 * @ingroup TENCENT_SMARTVOICE
 * @brief Smart Voice Profile Task.
 *
 * The TENCENT_SMARTVOICETASK is responsible for handling the messages coming in and out of the
 * @ref TENCENT_SMARTVOICE collector block of the BLE Host.
 *
 * @{
 ****************************************************************************************
 */



/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include "bluetooth_ble_api.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 0
///Attributes State Machine
enum {
    TENCENT_IDX_SVC,

    TENCENT_IDX_TX_CHAR,
    TENCENT_IDX_TX_VAL,
    TENCENT_IDX_TX_NTF_CFG,

    TENCENT_IDX_RX_CHAR,
    TENCENT_IDX_RX_VAL,

    TENCENT_IDX_NB,
};

/// Messages for Smart Voice Profile
enum tencent_sv_msg_id {
    TENCENT_CCC_CHANGED = TASK_FIRST_MSG(TASK_ID_AI),

    TENCENT_TX_DATA_SENT,

    TENCENT_RECEIVED,

    TENCENT_SEND_VIA_NOTIFICATION,

    TENCENT_SEND_VIA_INDICATION,

};

/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void tencent_task_init(struct ke_task_desc *task_desc);

bool app_tencent_send_cmd_via_notification(uint8_t* ptrData, uint32_t length,uint8_t ai_index, uint8_t device_id);

void app_ai_ble_add_tencent(void);
#endif

bool app_tencent_send_cmd_via_notification(uint8_t* ptrData, uint32_t length,uint8_t ai_index, uint8_t device_id);

#ifdef __cplusplus
}
#endif


/// @} 

#endif /* _TENCENT_SMARTVOICE_TASK_H_ */

