#ifndef __APP_TENCENT_SV_CMD_HANDLER_H__
#define __APP_TENCENT_SV_CMD_HANDLER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "hal_timer.h"
#include "app_tencent_sv_cmd_code.h"

#define GET_CURRENT_TICKS()         hal_sys_timer_get()
#define GET_CURRENT_MS()            TICKS_TO_MS(GET_CURRENT_TICKS())


/**< maximum payload size of one smart voice command */
#define MAXIMUM_DATA_PACKET_LEN     672
#define APP_TENCENT_SV_CMD_MAXIMUM_PAYLOAD_SIZE MAXIMUM_DATA_PACKET_LEN

/**
 * @brief BLE custom command playload
 *
 */
typedef struct {
    uint8_t     cmdCode;
    uint8_t     reqOrRsp;
    uint8_t     seqNum;
    uint16_t    paramLen;
    uint8_t     param[APP_TENCENT_SV_CMD_MAXIMUM_PAYLOAD_SIZE - 5 * sizeof(uint8_t)];
}__attribute__((packed)) APP_TENCENT_SV_CMD_PAYLOAD_T ;

/**
 * @brief Command response parameter structure
 *
 */
typedef struct {
    uint8_t     cmdCode;
    uint8_t     reqOrRsp;
    uint8_t     seqNum;
    uint8_t     ResponseCode;
    uint16_t    rspDataLen;
    uint8_t     rspData[APP_TENCENT_SV_CMD_MAXIMUM_PAYLOAD_SIZE - 3 * sizeof(uint16_t)];
} __attribute__((packed))APP_TENCENT_SV_CMD_RSP_T;

/* --- enums --- */
enum {
    TENCENT_COMMAND_REQ = 1,
    TENCENT_COMMAND_RSP = 2,
};

/*
 * //////////////////////////////////////////////////////////////////////////////
 * 传输层类型。当前还不支持iAP。
 */
typedef enum _TransportMode {
    TRANSPORT__BLUETOOTH_NONE = 0,
    TRANSPORT__BLUETOOTH_BLE = 1,
    TRANSPORT__BLUETOOTH_SPP = 2,
    TRANSPORT__BLUETOOTH_IAP = 3
} TransportMode;
typedef enum _AudioEncodeFormat {
    AUDIO_FORMAT__NONE = 0,
    AUDIO_FORMAT__OPUS = 1,
    AUDIO_FORMAT__SBC = 2,
} AudioEncodeFormat;
typedef enum _AudioComparessRation {
    Audio_Comparess_Ration_16 = 16,
} AudioComparessRation;
typedef enum _AudioSampleRate {
    Audio_Sample_Rate_16_K = 1,
    Audio_Sample_Rate_44_K = 2,
} AudioSampleRate;
typedef enum _AudioMicChannelCount {
    Audio_Mic_Channel_Count_1 = 1,
    Audio_Mic_Channel_Count_2 = 2,
} AudioMicChannelCount;
typedef enum _AudioBitNumber {
    Audio_Bit_Number_16 = 1,
    Audio_Bit_Number_8 = 2,
} AudioBitNumber;
typedef enum _RecordMethod {
  SPEECH_INITIATOR__TYPE__NONE = 0,
  SPEECH_INITIATOR__TYPE__PRESS_AND_HOLD = 1,
  SPEECH_INITIATOR__TYPE__TAP = 2,
} RecordMethod;
  
typedef enum _SpeechState {
  SPEECH_STATE__IDLE = 0,
  SPEECH_STATE__LISTENING = 1,
  SPEECH_STATE__PROCESSING = 2,
  SPEECH_STATE__SPEAKING = 3
} SpeechState;

void                        app_tencent_sv_get_cmd_response_handler(APP_TENCENT_SV_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);
void                        app_tencent_sv_data_xfer_control_handler(APP_TENCENT_SV_CMD_CODE_E funcCode, uint8_t* ptrParam, uint32_t paramLen);
void                        app_tencent_sv_control_data_xfer(bool isStartXfer);
void                        app_tencent_sv_start_data_xfer_control_rsp_handler(APP_TENCENT_SV_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen);
void                        app_tencent_sv_stAPP_TENCENT_DATA_XFER_control_rsp_handler(APP_TENCENT_SV_CMD_RET_STATUS_E retStatus, uint8_t* ptrParam, uint32_t paramLen);
APP_TENCENT_SV_CMD_RET_STATUS_E     app_tencent_sv_send_response_to_command(APP_TENCENT_SV_CMD_CODE_E responsedCmdCode, APP_TENCENT_SV_CMD_RET_STATUS_E returnStatus,
        uint8_t* rspData, uint32_t rspDataLen,uint8_t device_id);
APP_TENCENT_SV_CMD_RET_STATUS_E     app_tencent_sv_send_command(APP_TENCENT_SV_CMD_CODE_E cmdCode, uint8_t* ptrParam, uint32_t paramLen,uint8_t device_id);
void                        app_tencent_sv_cmd_handler_init(void);
APP_TENCENT_SV_CMD_INSTANCE_T*      app_tencent_sv_cmd_handler_get_entry_pointer_from_cmd_code(APP_TENCENT_SV_CMD_CODE_E cmdCode);
uint16_t                    app_tencent_sv_cmd_handler_get_entry_index_from_cmd_code(APP_TENCENT_SV_CMD_CODE_E cmdCode);
uint32_t app_tencent_rcv_cmd(void *param1, uint32_t param2,uint8_t device_id);
uint32_t app_tencent_parse_cmd(void *param1, uint32_t param2,uint8_t param3);


#ifdef __cplusplus
}
#endif

#endif // #ifndef __APP_TENCENT_SV_CMD_HANDLER_H__

