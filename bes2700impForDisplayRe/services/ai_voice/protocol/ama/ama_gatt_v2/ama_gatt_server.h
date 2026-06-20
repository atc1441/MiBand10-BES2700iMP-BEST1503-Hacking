#ifndef _AMA_GATT_SERVER_H_
#define _AMA_GATT_SERVER_H_

#ifdef __AI_VOICE_BLE_ENABLE__

/**
 ****************************************************************************************
 * @addtogroup AMA Datapath Profile Server
 * @ingroup AMA
 * @brief ama Profile Server
 *
 * Datapath Profile Server provides functionalities to upper layer module
 * application. The device using this profile takes the role of Datapath Server.
 *
 * The interface of this role to the Application is:
 *  - Enable the profile role (from APP)
 *  - Disable the profile role (from APP)
 *  - Send data to peer device via notifications  (from APP)
 *  - Receive data from peer device via write no response (from APP)
 *
 *
 * @{
 ****************************************************************************************
 */


/*
 * INCLUDE FILES
 ****************************************************************************************
 */



#include "ke_task.h"



///Attributes State Machine
enum
{
    AMA_IDX_SVC,

    AMA_IDX_TX_CHAR,
    AMA_IDX_TX_VAL,
    AMA_IDX_TX_NTF_CFG,
    AMA_IDX_RX_CHAR,
    AMA_IDX_RX_VAL,

    AMA_IDX_NB,
};


/// Messages for Smart Voice Profile 
enum ama_msg_id
{
    AMA_CCC_CHANGED = TASK_FIRST_MSG(TASK_ID_AI),
    AMA_TX_DATA_SENT,
    AMA_RECEIVED,
    AMA_SEND_VIA_NOTIFICATION,
    AMA_SEND_VIA_INDICATION,
};


/**
 ****************************************************************************************
 * Initialize task handler
 *
 * @param task_desc Task descriptor to fill
 ****************************************************************************************
 */
void ama_task_init(struct ke_task_desc *task_desc);

void app_ai_ble_add_ama(void);

#endif //__AI_VOICE_BLE_ENABLE__
#endif //_AMA_GATT_SERVER_H_

