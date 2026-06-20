#ifndef __AI_THREAD_H__
#define __AI_THREAD_H__

#include "bluetooth_bt_api.h"
#include "bluetooth_ble_api.h"
#include "app_dip.h"
#include "app_ai_if_config.h"
#include "app_ai_if.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern uint32_t __ai_handler_function_table_start[];
extern uint32_t __ai_handler_function_table_end[];

typedef void (*AI_UI_GLOBAL_HANDLER_IND)(uint32_t code_id, void *param1, uint32_t param2, uint8_t ai_index, uint8_t dest_id);

#define INVALID_AI_HANDLER_FUNCTION_ENTRY_INDEX 0xFFFF
#ifdef __BIXBY_VOICE__
#define APP_AI_VOICE_MIC_TIMEOUT_INTERVEL           (60000)
#else
#define APP_AI_VOICE_MIC_TIMEOUT_INTERVEL           (15000)
#endif
#define APP_AI_SART_SPEECH_MUTE_A2DP_STREAMING_LASTING_TIME_IN_MS  (15000)
#define APP_AI_VOICE_MUTE_A2DP_TIMEOUT_IN_MS (APP_AI_SART_SPEECH_MUTE_A2DP_STREAMING_LASTING_TIME_IN_MS+1000)
#define AI_MAILBOX_MAX (40)

#ifndef MAX_TX_CREDIT
#define MAX_TX_CREDIT 5
#endif

#define MUTLI_AI_NUM AI_SPEC_COUNT
#define BD_CNT_MAX 2

#if defined(IS_MULTI_AI_ENABLED)
#define AI_CONNECT_NUM_MAX 2
#else
#define AI_CONNECT_NUM_MAX 1
#endif

typedef enum  {
    AI_CONNECT_1 = 0,
    AI_CONNECT_2 = 1,
    AI_CONNECT_COUNT =2,
    AI_CONNECT_INVALID = 0xFF,
} AI_CONNECT_NUM;

typedef enum  {
    SIGN_AI_NONE,
    SIGN_AI_CONNECT,
    SIGN_AI_WAKEUP,
    SIGN_AI_CMD_COME,
    SIGN_AI_TRANSPORT,
} AI_SIGNAL_E;

typedef struct {
    AI_SIGNAL_E signal;
    uint32_t    len;
    uint8_t     ai_index;
    uint8_t     dest_id;
} AI_MESSAGE_BLOCK;

typedef enum
{
    AI_IS_DISCONNECT = 0,
    AI_IS_CONNECTED,
    AI_IS_DISCONNECTING,
}AI_CONNECT_STATE;

//the ai transport type
typedef enum {
    AI_TRANSPORT_IDLE,
    AI_TRANSPORT_BLE,
    AI_TRANSPORT_SPP,
} AI_TRANS_TYPE_E;

//the ai speech type
typedef enum {
    TYPE__NORMAL_SPEECH,
    TYPE__PROVIDE_SPEECH,
} AI_SPEECH_TYPE_E;

typedef enum _AiSpeechState {
    AI_SPEECH_STATE__IDLE = 0,
    AI_SPEECH_STATE__LISTENING = 1,
    AI_SPEECH_STATE__PROCESSING = 2,
    AI_SPEECH_STATE__SPEAKING = 3
} APP_AI_SPEECH_STATE_E;

typedef struct
{
    uint32_t    sentDataSizeWithInFrame;
    uint8_t     seqNumWithInFrame;
    volatile uint8_t    ai_stream_opened    :1;
    volatile uint8_t    ai_stream_init      :1;
    volatile uint8_t    ai_stream_mic_open  :1;
    volatile uint8_t    ai_stream_running   :1;
    volatile uint8_t    audio_algorithm_engine_reset   :1;
    volatile uint8_t    ai_setup_complete   :1;
    volatile uint8_t    reserve             :2;

} APP_AI_STREAM_ENV_T;

typedef struct
{
    bool                    used;
    bool                    connectWithAi;
    MOBILE_CONN_TYPE_E      mobile_connect_type;
    uint8_t                 addr[6];

} APP_AI_MOBILE_INFO_T;

typedef struct
{
    bool (*ai_spp_data_transport_func)(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t device_id);
    bool (*ai_spp_cmd_transport_func)(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t device_id);
    bool (*ai_ble_data_transport_func)(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t device_id);
    bool (*ai_ble_cmd_transport_func)(uint8_t* ptrData, uint32_t length, uint8_t ai_index, uint8_t device_id);
} APP_AI_TRANSPORT_HANDLER;

typedef struct
{
    bool                    is_role_switching;
    bool                    can_role_switch;
    bool                    initiative_switch;  //current earphone initiative to role switch
    bool                    role_switch_direct;
    bool                    role_switch_need_ai_param;
    bool                    can_wake_up;
    bool                    transfer_voice_start;
    bool                    ai_voice_timeout_timer_need;
    bool                    peer_ai_set_complete;

    bool                    use_ai_32kbps_voice;
    bool                    local_start_tone;
    bool                    is_in_tws_mode;
    bool                    want_to_use_thirdparty;
    bool                    is_use_thirdparty;
    bool                    enabled_kws;
    bool                    opus_in_overlay;

    uint8_t                 send_num;
    uint8_t                 tx_credit;
    uint8_t                 encode_type;
    uint8_t                 isDoEncodingOnDetecting;
    uint32_t                currentAiSpec;              //current ai_type
    uint32_t                data_trans_size_per_trans;  //ai data transport size every transmission
    // some ai may have header for each data xfer frame
    uint32_t                header_size_per_xfer_frame;
    uint32_t                mtu;    //AI maximum transmission unit
    uint32_t                wake_word_start_index;
    uint32_t                wake_word_end_index;
    uint32_t                ignoredPcmDataRounds; // use to record the pcm data that ai voice ignore
    AI_TRANS_TYPE_E         transport_type[AI_CONNECT_COUNT];
    uint8_t                 wake_up_type;
    APP_AI_STREAM_ENV_T     ai_stream;
    APP_AI_SPEECH_STATE_E   speechState;
    AI_SPEECH_TYPE_E        ai_speech_type;
    APP_AI_MOBILE_INFO_T    ai_mobile_info[BD_CNT_MAX];
} AI_struct ;

typedef struct
{
    uint8_t     ai_connect_state;
    uint8_t     ai_connect_type;
    uint8_t     device_id;
    uint8_t     conidx;
    uint32_t    ai_spec;
    uint16_t    connhdl;
    uint8_t     addr[6];
}AI_connect_info;
/**
 * @brief AI voice ble_table_handler definition data structure
 *
 */
typedef struct
{
    uint32_t                            ai_code;
    const struct ai_handler_func_table* ai_handler_function_table;      /**< ai handler function table */
} AI_HANDLER_FUNCTION_TABLE_INSTANCE_T;


#define AI_HANDLER_FUNCTION_TABLE_TO_ADD(ai_code, ai_handler_function_table)    \
    static const AI_HANDLER_FUNCTION_TABLE_INSTANCE_T ai_code##_handler_function_table __attribute__((used, section(".ai_handler_function_table"))) =     \
        {(ai_code), (ai_handler_function_table)};

#define AI_HANDLER_FUNCTION_TABLE_PTR_FROM_ENTRY_INDEX(index)   \
    ((AI_HANDLER_FUNCTION_TABLE_INSTANCE_T *)((uint32_t)__ai_handler_function_table_start + (index)*sizeof(AI_HANDLER_FUNCTION_TABLE_INSTANCE_T)))

extern AI_struct ai_struct[MUTLI_AI_NUM];
extern APP_AI_TRANSPORT_HANDLER ai_trans_handler[MUTLI_AI_NUM];
extern osTimerId app_ai_voice_timeout_timer_id;
const char *mobile_connect_type2str(MOBILE_CONN_TYPE_E connect_type);
const char *ai_trans_type2str(AI_TRANS_TYPE_E trans_type);
const char *ai_speech_state2str(APP_AI_SPEECH_STATE_E speech_state);
const char *ai_speech_type2str(AI_SPEECH_TYPE_E speech_state);

void app_ai_register_ui_global_handler_ind(AI_UI_GLOBAL_HANDLER_IND handler);
void app_ai_ui_global_handler_ind(uint32_t code_id, void *param1, uint32_t param2, uint8_t ai_index, uint8_t dest_id);

// bool app_ai_is_to_mute_a2dp_during_ai_starting_speech(void);
// void app_ai_control_mute_a2dp_state(bool isEnabled);

void app_ai_connect_info_init(void);
#ifdef __AI_VOICE_BLE_ENABLE__
uint8_t app_ai_set_ble_connect_info(uint8_t type, uint8_t AiSpec, bes_ble_ai_event_param_t *param);
#endif
void app_ai_update_connect_state(uint8_t state, uint8_t ai_connect_index);
uint8_t app_ai_get_connect_index_from_device_id(uint8_t device_id, uint8_t ai_spec);
uint8_t app_ai_get_connect_index_from_ai_spec( uint8_t ai_spec);
uint8_t app_ai_get_connect_index_from_ble_conidx(uint8_t conidx, uint8_t ai_spec);
uint16_t app_ai_get_conn_handle_from_ble_conidx(uint8_t conidx, uint8_t ai_spec);
uint8_t app_ai_set_spp_connect_info(uint8_t *_addr, uint8_t device_id, uint8_t type, uint8_t AiSpec);
bool app_ai_spec_have_other_connection(uint8_t ai_spec);
AI_connect_info *app_ai_get_connect_info(uint8_t ai_connect_index);
uint8_t app_ai_get_connected_num(void);
uint8_t app_ai_get_connect_state_index(uint8_t state);
bool app_ai_check_device_is_connected(uint8_t *_addr);
uint8_t app_ai_get_dest_id(AI_connect_info* ai_info);
uint8_t app_ai_get_dest_id_by_ai_spec(uint8_t ai_spec);
void app_ai_clear_connect_info(uint8_t ai_connect_index);
uint8_t app_ai_get_connect_index(uint8_t device_id, uint8_t ai_spec);
uint8_t app_ai_get_connect_index_from_addr(uint8_t *_addr, uint8_t AiSpec);
uint8_t app_ai_get_device_id_from_index(uint8_t index);
uint8_t app_ai_get_ai_index_from_connect_index(uint8_t connect_index);
void app_ai_set_ai_spec(uint8_t AiSpec, uint8_t ai_index);
uint8_t app_ai_get_ai_spec(uint8_t ai_index);
void app_ai_set_transport_type(AI_TRANS_TYPE_E type, uint8_t ai_index, uint8_t ai_connect_index);
AI_TRANS_TYPE_E app_ai_get_transport_type(uint8_t ai_index, uint8_t ai_connect_index);
void app_ai_set_data_trans_size(uint32_t trans_size, uint8_t ai_index);
uint32_t app_ai_get_data_trans_size(uint8_t ai_index);
void app_ai_set_header_size_per_frame(uint32_t header_size_per_frame, uint8_t ai_index);
uint32_t app_ai_get_header_size_per_frame(uint8_t ai_index);
uint8_t app_ai_read_tx_credit(uint8_t ai_index);
bool app_ai_take_tx_credit(uint8_t ai_index);
bool app_ai_release_tx_credit(uint8_t ai_index);
void app_ai_set_mtu(uint32_t mtu, uint8_t ai_index);
uint32_t app_ai_get_mtu(uint8_t ai_index);
bool app_is_ai_voice_connected(uint8_t ai_index);
void app_ai_set_wake_up_type(uint8_t type, uint8_t ai_index);
uint8_t app_ai_get_wake_up_type(uint8_t ai_index);
void app_ai_set_speech_state(APP_AI_SPEECH_STATE_E speech_state, uint8_t ai_index);
APP_AI_SPEECH_STATE_E app_ai_get_speech_state(uint8_t ai_index);
void app_ai_disconnect_all_mobile_link(void);
bool app_ai_dont_have_mobile_connect(uint8_t ai_index);
bool app_ai_connect_mobile_has_ios(uint8_t ai_index);
void app_ai_set_speech_type(AI_SPEECH_TYPE_E type, uint8_t ai_index);
AI_SPEECH_TYPE_E app_ai_get_speech_type(uint8_t ai_index);
void app_ai_set_encode_type(uint8_t encode_type, uint8_t ai_index);
uint8_t app_ai_get_encode_type(uint8_t ai_index);

void app_ai_set_role_switching(bool switching, uint8_t ai_index);
bool app_ai_is_role_switching(uint8_t ai_index);
void app_ai_set_can_role_switch(bool can_switch, uint8_t ai_index);
bool app_ai_can_role_switch(uint8_t ai_index);
void app_ai_set_initiative_switch(bool initiative_switch, uint8_t ai_index);
bool app_ai_is_initiative_switch(uint8_t ai_index);
void app_ai_set_role_switch_direct(bool role_switch_direct, uint8_t ai_index);
bool app_ai_is_role_switch_direct(uint8_t ai_index);
void app_ai_set_need_ai_param(bool need, uint8_t ai_index);
bool app_ai_is_need_ai_param(uint8_t ai_index);
bool is_ai_can_wake_up(uint8_t ai_index);
void ai_set_can_wake_up(bool wake_up, uint8_t ai_index);
void ai_audio_stream_allowed_to_send_set(bool isEnabled, uint8_t ai_index);
bool is_ai_audio_stream_allowed_to_send(uint8_t ai_index);
void app_ai_set_timer_need(bool need, uint8_t ai_index);
bool app_ai_is_timer_need(uint8_t ai_index);
void app_ai_set_stream_opened(bool opened, uint8_t ai_index);
bool app_ai_is_stream_opened(uint8_t ai_index);
void app_ai_set_stream_init(bool init, uint8_t ai_index);
bool app_ai_is_stream_init(uint8_t ai_index);
void app_ai_set_stream_running(bool running, uint8_t ai_index);
bool app_ai_is_stream_running(uint8_t ai_index);
void app_ai_set_algorithm_engine_reset(bool reset, uint8_t ai_index);
bool app_ai_is_algorithm_engine_reset(uint8_t ai_index);
void app_ai_set_setup_complete(bool complete, uint8_t ai_index);
bool app_ai_is_setup_complete(uint8_t ai_index);
void app_ai_set_peer_setup_complete(bool complete, uint8_t ai_index);
bool app_ai_is_peer_setup_complete(uint8_t ai_index);
void app_ai_set_use_ai_32kbps_voice(bool use_ai_32kbps_voice, uint8_t ai_index);
bool app_ai_is_use_ai_32kbps_voice(uint8_t ai_index);
void app_ai_set_local_start_tone(bool local_start_tone, uint8_t ai_index);
bool app_ai_is_local_start_tone(uint8_t ai_index);
void app_ai_set_in_tws_mode(bool is_in_tws_mode, uint8_t ai_index);
bool app_ai_is_in_tws_mode(uint8_t ai_index);
bool app_ai_is_in_multi_ai_mode(void);
void app_ai_set_use_thirdparty(bool is_use_thirdparty, uint8_t ai_index);
bool app_ai_is_use_thirdparty(uint8_t ai_index);
void app_ai_check_if_need_set_thirdparty(uint8_t ai_index);
void app_ai_set_opus_in_overlay(bool opus_in_overlay, uint8_t ai_index);
bool app_ai_is_opus_in_overlay(uint8_t ai_index);
bool app_ai_is_sco_mode(void);
void app_ai_set_is_do_encoding_during_detecting(bool isEnable, uint8_t ai_index);
bool app_ai_is_do_encoding_during_detecting(uint8_t ai_index);

void app_ai_mobile_connect_handle(MOBILE_CONN_TYPE_E type, uint8_t *_addr);
void app_ai_mobile_disconnect_handle(uint8_t *_addr);
uint8_t app_ai_connect_handle(AI_TRANS_TYPE_E ai_trans_type, uint8_t ai_index, uint8_t device_id, uint8_t *addr);
void app_ai_disconnect_handle(AI_TRANS_TYPE_E ai_trans_type, uint8_t ai_index);
void app_ai_setup_complete_handle(uint8_t ai_index);

void ai_parse_data_init(void (*cb)(void));
int ai_mailbox_put(AI_SIGNAL_E signal, uint32_t len, uint8_t ai_index, uint8_t device_id);
void ai_open_specific_ai(uint32_t ai_spec);
void ai_open_all_ai(uint32_t ai_spec);
uint32_t ai_save_ctx(uint8_t *buf, uint32_t buf_len);
uint32_t ai_restore_ctx(uint8_t *buf, uint32_t buf_len);
bool app_ai_is_enabled_wakeword(uint8_t ai_index);
void app_ai_enable_wakeword(bool enable, uint8_t ai_index);

#ifdef __cplusplus
}
#endif

#endif

