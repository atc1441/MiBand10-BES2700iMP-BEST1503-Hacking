#ifndef APP_AI_IF_BLE_H_
#define APP_AI_IF_BLE_H_

#define APP_AI_IF_BLE_INVALID_CONNECTION_INDEX 0xFF

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_trace.h"
#ifndef BLE_INVALID_CONNECTION_INDEX
#define BLE_INVALID_CONNECTION_INDEX 0xFF
#endif

#define AI_BLE_ADVERTISING_INTERVAL (160)

#if 0
#include "bluetooth_ble_api.h"
typedef ke_state_t ai_ble_ke_state_t;
typedef ke_task_id_t ai_ble_ke_task_id_t;
typedef ke_msg_id_t ai_ble_ke_msg_id_t;
typedef ke_msg_func_t ai_ble_ke_msg_func_t;
typedef struct prf_task_cbs ai_ble_prf_task_cbs_t;
typedef struct ke_msg_handler ai_ble_ke_msg_handler_t;
typedef prf_hdr_t ai_ble_prf_env_t;
typedef struct app_subtask_handlers ai_ble_ke_state_handler_t;
#endif

#define AI_BLE_ATT_UUID_128_LEN 0x0010
/// Task State
typedef uint8_t ai_ble_ke_state_t;

/// Task Identifier. Composed by the task type and the task index.
typedef uint16_t ai_ble_ke_task_id_t;

/// Message Identifier. The number of messages is limited to 0xFFFF.
/// The message ID is divided in two parts:
/// bits[15~8]: task index (no more than 255 tasks support)
/// bits[7~0]: message index(no more than 255 messages per task)
/*@TRACE*/
typedef uint16_t ai_ble_ke_msg_id_t;

/// Format of a task message handler function
typedef int (*ai_ble_ke_msg_func_t)(ai_ble_ke_msg_id_t const msgid, void const *param,
                             ai_ble_ke_task_id_t const dest_id, ai_ble_ke_task_id_t const src_id);

typedef struct
{
    /// Initialization callback
    void *init;
    /// Destroy profile callback
    void *destroy;
    /// Connection callback
    void *create;
    /// Disconnection callback
    void *cleanup;
    /// Updated callback
    void *updated;
} ai_ble_prf_task_cbs_t;

/// Element of a message handler table.
typedef struct
{
    /// Id of the handled message.
    ai_ble_ke_msg_id_t id;
    /// Pointer to the handler function for the msgid above.
    ai_ble_ke_msg_func_t func;
} ai_ble_ke_msg_handler_t;

/// Element of a state handler table.
typedef struct
{
    /// Pointer to the message handler table of this state.
    const ai_ble_ke_msg_handler_t *msg_table;
    /// Number of messages handled in this state.
    uint16_t msg_cnt;
} ai_ble_ke_state_handler_t;

/// Profile Environment Data
typedef struct
{
    /// Callback API - meaning specific to the profile
    const void*             p_cb;
    /// Application Task Number - if MSB bit set, Multi-Instantiated task
    ai_ble_ke_task_id_t     app_task;
    /// Profile Task  Number    - if MSB bit set, Multi-Instantiated task
    ai_ble_ke_task_id_t     prf_task;
} ai_ble_prf_env_t;

/// Internal 16bits UUID service description
typedef struct
{
    /// 16 bits UUID LSB First
    uint16_t uuid;
    /// Attribute Permissions (@see enum attm_perm_mask)
    uint16_t perm;
    /// Attribute Extended Permissions (@see enum attm_value_perm_mask)
    uint16_t ext_perm;
    /// Attribute Max Size
    /// note: for characteristic declaration contains handle offset
    /// note: for included service, contains target service handle
    uint16_t max_size;
} ai_ble_attm_desc_t;

/// Internal 128bits UUID service description
typedef struct
{
    /// 128 bits UUID LSB First
    uint8_t uuid[AI_BLE_ATT_UUID_128_LEN];
    /// Attribute Permissions (@see enum attm_perm_mask)
    uint16_t perm;
    /// Attribute Extended Permissions (@see enum attm_value_perm_mask)
    uint16_t ext_perm;
    /// Attribute Max Size
    /// note: for characteristic declaration contains handle offset
    /// note: for included service, contains target service handle
    uint16_t max_size;
} ai_ble_attm_desc_128_t;


bool app_ai_if_ble_check_if_notification_processing_is_busy(uint8_t conidx);
uint16_t app_ai_if_ble_get_conhdl_from_conidx(uint8_t conidx);
void app_ai_if_ble_disconnect_ble(uint8_t conidx);
void app_ai_if_ble_set_adv(uint16_t advInterval);
void app_ai_if_ble_update_conn_param(uint8_t  conidx,
                                    uint32_t min_interval_in_ms,
                                    uint32_t max_interval_in_ms,
                                    uint32_t supervision_timeout_in_ms,
                                    uint8_t  slaveLantency);
void app_ai_if_ble_update_conn_param_mode(bool isEnabled);
void app_ai_if_ble_register_data_fill_handle(void *func, bool enable);
void app_ai_if_ble_data_fill_enable(bool enable);
void app_ai_if_ble_disconnect_all(void);
void app_ai_if_ble_force_switch_adv(bool onOff);

#ifdef __cplusplus
    }
#endif


#endif //APP_AI_IF_BLE_H_

