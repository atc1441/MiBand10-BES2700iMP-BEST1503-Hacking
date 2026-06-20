#ifndef __APP_VOICE_HANDLE_H
#define __APP_VOICE_HANDLE_H
#include "cqueue.h"

/***************************************************/
/*Marco                                            */
/***************************************************/
#define SIGNAL_VOICE_SHARE                          (0x01)
#define NEW_DATA_FORMAT      1

#ifdef RECORDING_USE_SCALABLE
#define CUMULATE_PACKET_THRESHOLD                   (1)
#define LOCAL_RESERVER_BUFFER_NUM                   (90)
#define SLAVE_RESERVER_BUFFER_NUM                   (90)
#define FRAME_LENGTH_MAX                            (166) //!< this value should be determained by encode spec
#define ENCODED_DATA_SIZE                           (2 * FRAME_LENGTH_MAX) //Gather 2 frames
#define FRAME_LENGTH_WITH_TWS                       (HEADER_SIZE + ENCODED_DATA_SIZE) //Real size. Exclude garbage
#define FRAME_LENGTH_WITH_APP                       (FRAME_LENGTH_WITH_TWS*2) //Include garbage data
#define RECORD_DATA_PACKET_GATHER_LIMIT             (2)
#define PLC_FLAG_SIZE                               (1)
#else
#ifdef RECORDING_USE_OPUS_LOW_BANDWIDTH
#define CUMULATE_PACKET_THRESHOLD                   (2) //Collect two packets and send them to the phone to avoid wasting bandwidth
#define LOCAL_RESERVER_BUFFER_NUM                   (90)
#define SLAVE_RESERVER_BUFFER_NUM                   (90)
#define ENCODED_DATA_SIZE                           (80)
#else
#define CUMULATE_PACKET_THRESHOLD                   (2)
#define LOCAL_RESERVER_BUFFER_NUM                   (50)  //50 *20ms = 1s
#define SLAVE_RESERVER_BUFFER_NUM                   (50)
#define ENCODED_DATA_SIZE                           (160)
#endif
#define FRAME_LENGTH_WITH_TWS                       (ENCODED_DATA_SIZE + HEADER_SIZE)                 //Real size. Exclude garbage
#ifndef NEW_DATA_FORMAT
#define FRAME_LENGTH_WITH_APP                       ((FRAME_LENGTH_WITH_TWS + ENCODED_DATA_SIZE) * 2) //Include garbage data
#else
#define FRAME_LENGTH_WITH_APP                       ((FRAME_LENGTH_WITH_TWS) * 2) //Include garbage data
#endif
#define RECORD_DATA_PACKET_GATHER_LIMIT             (1)
#endif

#define HEADER_SIZE                                 (4)
#define VOB_VOICE_BIT_NUMBER                        (AUD_BITS_16)
#define VOB_PCM_SIZE_TO_SAMPLE_CNT(size)            ((size)*8/VOB_VOICE_BIT_NUMBER)
#define MAX_DATA_INDEX                              (0xFFFFFFFF)
#define STARTING_DATA_INDEX                         (0x01)    //It will make confusing if starting value is 0. Because the default value is 0

#define ENABLE_RECORDING                            (0x1)
#define ENABLE_TRANSFER                             (0x2)
#define ENABLE_GET_ENCODED                          (0x4)

// #define RECORD_DUMP_AUDIO
// #define RECORD_DUMP_DATA

typedef enum{
    BLE_CONNECT = 0,
    BLE_DISCONNECT,
    SPP_CONNECT,
    SPP_DISCONNECT,
    BT_DEFAULT,
    BT_STATE_MAX,
}bt_connect_e;

typedef enum {
    RECORDING_CMD_STEREO_START  = 0x8004,
    RECORDING_CMD_STOP          = 0x8005,
    RECORDING_CMD_MONO_START    = 0x8006
} RECORDING_CMD_E;

typedef struct
{
    uint16_t    cmdCode;        /**< command code, from APP_SV_CMD_CODE_E */
    uint16_t    paramLen;       /**< length of the following parameter */
    uint8_t     param[1];
} APP_VOICE_CMD_PAYLOAD_T, APP_VOICE_RSP_PAYLOAD_T;

#ifdef RECORDING_USE_SCALABLE
#define CACHE_BUF_LVL_DEF(s)  _ENUM_DEF(CACHE_BUF_LVL_, s)
#define CACHE_BUF_LVL_LIST \
        CACHE_BUF_LVL_DEF(MORE_THAN_HALF_LEFT), \
        CACHE_BUF_LVL_DEF(MORE_THAN_QUARTER_LEFT), \
        CACHE_BUF_LVL_DEF(LESS_THAN_QUARTER_LEFT)

#define BITRATE_UPDATE_STATE_DEF(s) _ENUM_DEF(BITRATE_UPDATE_STATE_, s)
#define BITRATE_UPDATE_STATE_LIST \
        BITRATE_UPDATE_STATE_DEF(IDLE), \
        BITRATE_UPDATE_STATE_DEF(DEC_SEND_WAIT_RSP), \
        BITRATE_UPDATE_STATE_DEF(INC_SEND_WAIT_RSP)

typedef enum
{
    CACHE_BUF_LVL_LIST,

    CACHE_BUF_LVL_CNT,
} CACHE_BUF_LVL_E;

typedef enum
{
    BITRATE_UPDATE_STATE_LIST,

    BITRATE_UPDATE_STATE_CNT,
} BITRATE_UPDATE_STATE_E;

typedef enum
{
    FRAME_BOTH_NORMAL   = 0,
    FRAME_BOTH_PLC      = 1,
    FRAME_LEFT_PLC      = 2,
    FRAME_RIGHT_PLC     = 3,
} FRAME_PLC_STATE_E;

typedef struct
{
    uint8_t bitrate;
    uint32_t cursor;
} SCALABLE_CONFIG_BITRATE_T;

typedef struct
{
    uint32_t    dataIndex;
    uint8_t     dataBuf[FRAME_LENGTH_WITH_APP - HEADER_SIZE];  //dataIndex+ data + dataIndex + data
}__attribute__ ((packed)) APP_RECORDING_DATA_T;
#else
#ifdef RECORDING_USE_OPUS_LOW_BANDWIDTH
typedef struct
{
    uint32_t    dataIndex;
    uint8_t     dataBuf[FRAME_LENGTH_WITH_APP - HEADER_SIZE];  //dataIndex+ data + garbage + dataIndex + data + garbage
    uint8_t     dataBuf1[FRAME_LENGTH_WITH_APP];  //dataIndex+ data + garbage + dataIndex + data + garbage
    uint8_t     dataBuf2[FRAME_LENGTH_WITH_APP];  //dataIndex+ data + garbage + dataIndex + data + garbage
}__attribute__ ((packed)) APP_RECORDING_DATA_T;
#else
typedef struct
{
    uint32_t    dataIndex;
    uint8_t     dataBuf[FRAME_LENGTH_WITH_APP - HEADER_SIZE];  //dataIndex+ data + dataIndex + data
}__attribute__ ((packed)) APP_RECORDING_DATA_T;
#endif /// RECORDING_USE_OPUS_LOW_BANDWIDTH
#endif

uint32_t app_recording_get_out_cursor(void);
void app_recording_init(void);
void app_recording_start_voice_stream(void);
void app_recording_stop_voice_stream(void);
void app_recording_bt_connect_cb(bt_connect_e bt_state);
bool app_recording_spp_send_data(uint8_t *buf, uint32_t len);
uint32_t app_recording_connected(void *param1, uint32_t param2);
void app_recording_disconnect(void);
uint32_t app_recording_disconnected(void *param1, uint32_t param2);
void app_recording_audio_send_done(void);
void app_recording_send_data_to_master(void);
void app_recording_role_switch_done_handle(void);
void app_recording_info_slave_received(uint16_t rsp_seq, uint8_t* data);
void app_recording_info_master_received(uint8_t* data);
bool app_recording_ble_send_data(uint8_t *buf, uint32_t len);
bool app_recording_send_data(uint8_t *buf, uint32_t len);
void app_recording_role_switch_done_handle(void);
void app_recording_info_slave_received(uint16_t rsp_seq, uint8_t* data);
void app_recording_info_master_received(uint8_t* data);

/**
 * @brief Start the capture stream on a specific BT tick
 * 
 * @param triggerTicks  The BT trigger to start the capture stream
 */
void app_recording_start_capture_stream_on_specific_ticks(uint32_t triggerTicks);

#ifdef RECORDING_USE_SCALABLE
/**
 * @brief Update scalable handler
 * 
 * @param bitrate       bitrate to update
 * @param cursor        cursor to update bitrate
 */
void app_recording_update_scalable_bitrate(uint8_t bitrate, uint32_t cursor);

/**
 * @brief Check if need to update the bitrate
 * 
 * @param cursor        the compression count to update the cursor
 * @return uint8_t      current bitrate
 */
uint8_t app_recording_scalable_update_bitrate_check(uint32_t cursor);

/**
 * @brief Peer buffer level received handler
 * 
 * @param lvl           peer buffer level
 */
void app_recording_on_peer_buf_lvl_received(uint8_t lvl);
#endif

#endif

