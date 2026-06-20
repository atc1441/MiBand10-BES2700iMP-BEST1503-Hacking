
#ifndef __APP_AMA_HANDLE_H__
#define __APP_AMA_HANDLE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "plat_types.h"

#define APP_AMA_SPEECH_STATE_TIMEOUT_INTERVEL           (90000)
#define AMA_VOICE_ONESHOT_PROCESS_LEN    (1280)

#ifdef AMA_ENCODE_USE_OPUS
    #ifdef AQE_KWS_ALEXA
    #define SPP_TRANS_SIZE  80
    #define SPP_TRANS_TIMES 1
    #define BLE_TRANS_SIZE  80
    #define BLE_TRANS_TINES 1
    #else
    #define SPP_TRANS_SIZE  320
    #define SPP_TRANS_TIMES 1
    #define BLE_TRANS_SIZE  160
    #define BLE_TRANS_TINES 1
    #endif
#elif defined(AMA_ENCODE_USE_SBC)
    #define SPP_TRANS_SIZE  342
    #define SPP_TRANS_TIMES 3
    #define BLE_TRANS_SIZE  171
    #define BLE_TRANS_TINES 6
#else
    #define SPP_TRANS_SIZE  80
    #define SPP_TRANS_TIMES 1
    #define BLE_TRANS_SIZE  80
    #define BLE_TRANS_TINES 1
#endif
/// Table of ama handlers function
extern const struct ai_handler_func_table ama_handler_function_table;

int32_t app_ama_event_notify(uint32_t msgID,uint32_t msgPtr, uint32_t param0, uint32_t param1);
void app_ama_manual_disconnect(uint8_t deviceID);

uint32_t _get_read_offset_in_ai_voice_buf(void);

void _set_read_offset_in_ai_voice_buf(uint32_t offset);

#ifdef __cplusplus
}
#endif
#endif
