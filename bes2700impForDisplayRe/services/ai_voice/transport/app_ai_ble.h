#ifndef APP_AI_BLE_H_
#define APP_AI_BLE_H_

/**
 ****************************************************************************************
 * @addtogroup APP
 * @ingroup RICOW
 *
 * @brief Smart Voice Application entry point.
 *
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include "app_ai_if_ble.h"     // SW configuration

#include "ai_thread.h"
/*
 * TYPE DEFINITIONS
 ****************************************************************************************
 */
#ifdef __cplusplus
extern "C" {
#endif

#define SET_BLE_FLAG(conidx)            (conidx | 0x80)
#define GET_DEST_FLAG(dest_id)          (dest_id & 0x80)
#define GET_BLE_CONIDX(dest_id)         (dest_id & 0x7F)

extern uint32_t __ai_ble_handler_table_start[];
extern uint32_t __ai_ble_handler_table_end[];

#define INVALID_AI_BLE_HANDLER_ENTRY_INDEX 0xFFFF

#define AI_BLE_CONNECTION_INTERVAL_MIN_IN_MS            50
#define AI_BLE_CONNECTION_INTERVAL_MAX_IN_MS            60
#define AI_BLE_CONNECTION_SUPERVISOR_TIMEOUT_IN_MS      20000
#define AI_BLE_CONNECTION_SLAVELATENCY                  0

#ifndef BLE_CONNECTION_MAX
#define BLE_CONNECTION_MAX (1)
#endif

/// ama application environment structure
struct app_ai_env_tag
{
    uint8_t connectionIndex;
    uint8_t isCmdNotificationEnabled;
    uint8_t isDataNotificationEnabled;
    uint16_t connhdl;
    uint16_t mtu;
}__attribute__ ((__packed__));


/*
 * GLOBAL VARIABLES DECLARATIONS
 ****************************************************************************************
 */

/// Health Thermomter Application environment
extern struct app_ai_env_tag app_ai_env[2];

/// Table of message handlers
//extern const ai_ble_ke_state_handler_t *app_ai_table_handler[MUTLI_AI_NUM];
extern const ai_ble_ke_state_handler_t *app_ai_table_handler;

/**
 * @brief AI voice ble_table_handler definition data structure
 *
 */
typedef struct
{
    uint32_t                    ai_code;
    const ai_ble_ke_state_handler_t *ai_table_handler;   /**< ai ble handler table */
} AI_BLE_HANDLER_INSTANCE_T;


#define AI_BLE_HANDLER_TO_ADD(ai_code, ai_table_handler)    \
    static const AI_BLE_HANDLER_INSTANCE_T ai_code##_ble_handler __attribute__((used, section(".ai_ble_handler_table"))) =    \
        {(ai_code), (ai_table_handler)};

#define AI_BLE_HANDLER_PTR_FROM_ENTRY_INDEX(index)  \
    ((AI_BLE_HANDLER_INSTANCE_T *)((uint32_t)__ai_ble_handler_table_start + (index)*sizeof(AI_BLE_HANDLER_INSTANCE_T)))

/*
 * FUNCTIONS DECLARATION
 ****************************************************************************************
 */
/**
 ****************************************************************************************
 * @brief Initialize DataPath Server Application
 ****************************************************************************************
 */
void app_ai_ble_init(uint32_t ai_spec);

/**
 ****************************************************************************************
 * @brief Add a DataPath Server instance in the DB
 ****************************************************************************************
 */
void app_ai_ble_add_ai(void);
void app_ai_ble_add_dma(void);
void app_ai_ble_add_ama(void);
void app_ai_ble_add_gma(void);
void app_ai_ble_add_smartvoice(void);
void app_ai_ble_add_tencent(void);
void app_ai_ble_add_mma(void);

void app_ai_ble_add_rec(void);
void app_ai_connected_evt_handler(uint8_t conidx, uint8_t ai_index, uint16_t connhdl);
void app_ai_disconnected_evt_handler(uint8_t conidx);
bool app_ai_ble_is_connected(uint8_t ai_index);
void app_ai_disconnect_ble(uint8_t ai_index);
uint16_t app_ai_ble_get_conhdl(uint8_t ai_index);
uint8_t app_ai_ble_get_connection_index(uint8_t ai_index);
bool app_ai_ble_notification_processing_is_busy(uint8_t ai_index);
void app_ai_ble_update_conn_param_mode(bool isEnabled, uint8_t ai_index, uint8_t connect_index);
void app_ai_mtu_exchanged_handler(uint8_t conidx, uint16_t MTU);

#ifdef __cplusplus
    }
#endif

/// @} APP

#endif // APP_AI_BLE_H_
