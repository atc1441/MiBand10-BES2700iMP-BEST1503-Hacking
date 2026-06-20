#ifndef __APP_AI_VOICE_H
#define __APP_AI_VOICE_H

#ifdef __cplusplus
extern "C" {
#endif

#define UPSTREAM_INVALID_READ_OFFSET        (0xFFFFFFFF)

/// currently, max_len requestor is DUAL_MIC_RECORDING using OPUS
#define AI_VOICE_ONESHOT_PROCESS_MAX_LEN    (1920)

#define AIV_DEFAULT_FREQ_VALUE                          (APP_SYSFREQ_52M)
#define AIV_DETECTING_BOOST_FREQ_VALUE                  (APP_SYSFREQ_104M)
#define AIV_UPLOADING_BOOST_FREQ_VALUE                  (APP_SYSFREQ_208M)

/**************************************************************************************************
 *                          self-defined strcutres                                                *
 * ************************************************************************************************/
#define AIV_WORK_MODE_DEF(s)  _ENUM_DEF(AIV_WORK_MODE_, s)
#define AIV_WORK_MODE_LIST \
        AIV_WORK_MODE_DEF(IDLE), \
        AIV_WORK_MODE_DEF(HW_DETECTING), \
        AIV_WORK_MODE_DEF(UPSTREAMING)

#define AIV_USER_DEF(s)  _ENUM_DEF(AIV_USER_, s)
#define AIV_USER_LIST \
        AIV_USER_DEF(INVALID), \
        AIV_USER_DEF(BES), \
        AIV_USER_DEF(GVA), \
        AIV_USER_DEF(AMA), \
        AIV_USER_DEF(GMA), \
        AIV_USER_DEF(DMA), \
        AIV_USER_DEF(PENGUIN), \
        AIV_USER_DEF(REC), \
        AIV_USER_DEF(SSB), \
        AIV_USER_DEF(COMM)

#define AIV_MIC_STATE_DEF(s)  _ENUM_DEF(AIV_MIC_STATE_, s)
#define AIV_MIC_STATE_LIST \
        AIV_MIC_STATE_DEF(CLOSED), \
        AIV_MIC_STATE_DEF(CLOSING), \
        AIV_MIC_STATE_DEF(OPENED), \
        AIV_MIC_STATE_DEF(OPENING)

#define AIV_OP_DEF(s)   _ENUM_DEF(AIV_OP_, s)
#define AIV_OP_LIST \
        AIV_OP_DEF(START_UPSTREAM), \
        AIV_OP_DEF(STOP_UPSTREAM), \
        AIV_OP_DEF(START_CAPTURE), \
        AIV_OP_DEF(STOP_CAPTURE), \
        AIV_OP_DEF(HANDLE_DATA)

#define AIV_ENCODE_STATE_DEF(s) _ENUM_DEF(AIV_ENCODE_STATE_, s)
#define AIV_ENCODE_STATE_LIST \
        AIV_ENCODE_STATE_DEF(IDLE), \
        AIV_ENCODE_STATE_DEF(RUNNING)

#define AIV_WAKEUP_TRIGGER_DEF(s) _ENUM_DEF(AIV_WAKEUP_TRIGGER_, s)
#define AIV_WAKEUP_TRIGGER_LIST \
        AIV_WAKEUP_TRIGGER_DEF(NONE), \
        AIV_WAKEUP_TRIGGER_DEF(PRESS_AND_HOLD), \
        AIV_WAKEUP_TRIGGER_DEF(TAP), \
        AIV_WAKEUP_TRIGGER_DEF(KEYWORD)

typedef enum
{
    AIV_WORK_MODE_LIST,

    AIV_WORK_MODE_CNT,
} AIV_WORK_MODE_E;

typedef enum
{
    AIV_USER_LIST,

    AIV_USER_CNT,
} AIV_USER_E;

typedef enum
{
    AIV_MIC_STATE_LIST,

    AIV_MIC_STATE_CNT,
} AIV_MIC_STATE_E;

typedef enum
{
    AIV_OP_LIST,

    AIV_OP_CNT,
} AIV_OP_E;

typedef enum
{
    AIV_ENCODE_STATE_LIST,

    AIV_ENCODE_STATE_CNT,
} AIV_ENCODE_STATE_E;

typedef enum
{
    AIV_WAKEUP_TRIGGER_LIST,

    AIV_WAKEUP_TRIGGER_CNT,
} AIV_WAKEUP_TRIGGER_E;

typedef struct
{
    /// to mark if slave need to open mic
    uint32_t slaveOpenMic   : 1;
    /// to mark if AI is active(connected with APP)
    uint32_t active         : 1;
    /// to mark if AI is a super user which could:
    /// 1. force stop cpature stream when other AI is active
    /// 2. do not allow other AI open stream when super works
    /// to add more
    uint32_t superUser      : 1;
    /// reserved for future use
    uint32_t rfu            : 30;
} AIV_FLAG_BUDLE_T;

typedef int (*TRIGGER_INIT_T)(void);
typedef void (*UPSTREAM_STARTED_T)(void);
typedef void (*UPSTREAM_STOPPED_T)(uint32_t readOffset);
typedef int (*STREAM_STARTED_T)(void);
typedef void (*STREAM_STOPPED_T)(void);
typedef void (*UPSTREAM_DATA_HANDLER_T)(void);
typedef void (*HW_DETECT_HANDLER_T)(uint8_t user);
typedef uint32_t (*READ_OFFSET_GETTER_T)(void);
typedef void (*READ_OFFSET_SETTER_T)(uint32_t offset);

typedef struct
{
    UPSTREAM_STARTED_T      upstreamStarted;
    UPSTREAM_STOPPED_T      upstreamStopped;
    STREAM_STARTED_T        streamStarted;
    STREAM_STOPPED_T        streamStopped;
    UPSTREAM_DATA_HANDLER_T upstreamDataHandler;
    HW_DETECT_HANDLER_T     hwDetectHandler;
    READ_OFFSET_GETTER_T    readOffsetGetter;
    READ_OFFSET_SETTER_T    readOffsetSetter;
} AIV_HANDLER_BUNDLE_T;

typedef struct
{
    uint32_t op;
    uint32_t param0;
    uint32_t param1;
} AIV_MESSAGE_BLOCK;

typedef struct
{
    int         score;
    uint32_t    sIdx;
    uint32_t    eIdx;
    /// must in type of @see AIV_WAKEUP_TRIGGER_TYPE_E
    uint8_t     triggerType;
    /// must in type of @see AIV_USER_E
    uint8_t     user;
    uint32_t    hotword_detect_offset_of_encode_queue;
#ifdef VOICE_DETECTOR_SENS_EN
    uint32_t    kwsHistoryPcmDataSize;
#endif
} AIV_WAKEUP_INFO_T;

/**************************************************************************************************
 *                          APIs declearation                                                     *
 * ************************************************************************************************/
/**
 * @brief Init stream
 * 
 * @param user          AI user @see AIV_USER_E
 * @return int          0 - init sucessfully
 *                      0ther - already init
 */
int app_ai_voice_stream_init(uint8_t user);

int app_ai_voice_deinit(uint8_t ai_index, uint8_t ai_connect_index);

/**
 * @brief Start AI capture stream, only used for AUDIO MANAGER
 * 
 * @return int          0 - start sucessfully
 *                      other - start failed
 */
int app_ai_voice_start_mic_stream(void);

/**
 * @brief Stop AI capture stream, only used for AUDIO MANAGER
 * 
 * @return int          0 - start sucessfully
 *                      other - start failed
 */
int app_ai_voice_stop_mic_stream(void);

/**
 * @brief Update work mode for specific AI
 * 
 * @param user          @see AIV_USER_E to get more info 
 * @param mode          @see AIV_WORK_MODE_E to get more info
 */
void app_ai_voice_update_work_mode(uint8_t user, uint8_t mode);

/**
 * @brief Default upstream handler
 * 
 */
void app_ai_voice_default_upstream_data_handler(void);

/**
 * @brief Default hotword detection handler
 * 
 * @param user          AI voice user, @see AIV_USER_E to get more info
 */
void app_ai_voice_default_hotword_detect_handler(uint8_t user);

/**
 * @brief Start/stop the upstream.
 * 
 * @param on            true - start the upstream
 *                      false - stop the upstream
 * @param user          AI voice user, @see AIV_USER_E
 * @param offset        Read offset in PCM data cache buffer queue
 * @return int          0 - Open mic(start the capture stream)
 *                      other - will not open mic
 */
int app_ai_voice_upstream_control(bool on, uint8_t user, uint32_t offset);

/**
 * @brief Start/stop the capture stream
 * 
 * @param on            true - start the capture stream
 *                      flase - stop the capture stream
 * @param user          AI voice user, @see AIV_USER_E
 * @return int          0 - start capture stream
 *                      other - will not start capture stream
 */
int app_ai_voice_stream_control(bool on, uint8_t user);

/**
 * @brief Callback function of AI voice module when thirdparty detected the hotword
 * 
 * @param param1        parameter 1
 * @param param2        parameter 2
 * @return uint32_t     Excute result
 */
uint32_t app_ai_voice_thirdparty_wake_up_callback(void* param1, uint32_t param2);

int app_ai_voice_uart_audio_init();

/**
 * @brief Lock ai_voice pcm cache buffer
 * 
 */
void app_ai_voice_lock_buf(void);

/**
 * @brief Unlock ai_voice pcm cache buffer
 * 
 */
void app_ai_voice_release_buf(void);

/**
 * @brief Get ai_voice pcm buffer length
 * 
 * @return uint32_t     length of pcm buffer
 */
uint32_t app_ai_voice_get_ai_voice_buf_len(void);

/**
 * @brief Get ai_voice pcm buffer read offset(queue->read offset)
 * 
 * @return uint32_t     read offset of pcm buffer queue
 */
uint32_t app_ai_voice_get_buf_read_offset(void);

/**
 * @brief Get ai_voice pcm buffer write offset(queue->write offset)
 * 
 * @return uint32_t     write offset of pcm buffer queue
 */
uint32_t app_ai_voice_get_buf_write_offset(void);

/**
 * @brief Get ai_voice pcm queue length
 * 
 * @return uint32_t     length of pcm buffer queue
 */
uint32_t app_ai_voice_get_pcm_queue_len(void);

/**
 * @brief Decrease read offset in ai_voice buffer queue
 * 
 * @param readOffset    Current read offset
 * @param dec           Shift to decrease
 * @return uint32_t     Result after drease
 */
uint32_t app_ai_voice_pcm_queue_read_dec(uint32_t readOffset, uint32_t dec);

/**
 * @brief Increase read offset in ai_voice buffer queue
 * 
 * @param readOffset    Current read offset
 * @param inc           Shift to increase
 * @return uint32_t     Result after increase
 */
uint32_t app_ai_voice_pcm_queue_read_inc(uint32_t readOffset, uint32_t inc);

/**
 * @brief Get cached data length in ai_voice buffer queue
 * 
 * @param offset        Read offset that start to calculate
 * @return uint32_t     Cached data length
 */
uint32_t app_ai_voice_get_cached_len_according_to_read_offset(uint32_t offset);

/**
 * @brief Get cached pcm data in ai_voice pcm buffer according to offset
 * 
 * @param offset        Offset of cahced data to read
 * @param buf           Buffer to copy cached data
 * @param len           Length of cached to get
 */
bool app_ai_voice_get_pcm_data(uint32_t offset, uint8_t *buf, uint32_t len);

// void app_ai_voice_update_upstream_data_read_offset(uint32_t offset);

/**
 * @brief Get mic open status
 *
 * @return true         mic opened
 * @return false        mic closed
 */
bool app_ai_voice_is_mic_open(void);

/**
 * @brief Initialize the AI voice thread
 *
 */
void app_ai_voice_thread_init(void);

/**
 * @brief Get if need to block the A2DP stream
 *
 * @return true         AI need to block the A2DP stream
 * @return false        AI don't need to block the A2DP stream
 */
bool app_ai_voice_is_need2block_a2dp(void);

/**
 * @brief AI voice block A2DP stream
 *
 */
void app_ai_voice_block_a2dp(void);

/**
 * @brief Resume the AI block A2DP stream
 *
 */
void app_ai_voice_resume_blocked_a2dp(void);

/**
 * @brief Specific AI request to monopolize the stream
 *
 * @param requestor     The AI spec request to monopolize the AI stream
 * @return true         Request accepted
 * @return false        Request denied(stream already monopolized by other user)
 */
bool app_ai_voice_request_monopolize_stream(uint8_t requestor);

/**
 * @brief Specific AI request to share the AI stream when AI stream is monopolized by it
 *
 * @param requestor     The AI spec request to share AI stream, NOTE: must be the monopolist of AI stream
 * @param offset        The current read offset of requestor
 */

void app_ai_voice_request_share_stream(uint8_t requestor, uint32_t offset);

/**
 * @brief Register callback functions for specific AI
 *
 * @param user          specific AI spec
 * @param cb            pointer of callback functions
 */
void app_ai_voice_register_ai_handler(uint8_t user, AIV_HANDLER_BUNDLE_T* cb);

/**
 * @brief Update the one-shot process data length
 * 
 * @param len           The length to update
 */
void app_ai_voice_update_handle_frame_len(uint32_t len);

/**
 * @brief Update the handler of init trigger
 * 
 * @param handler       handler for trigger init
 */
void app_ai_voice_register_trigger_init_handler(TRIGGER_INIT_T handler);

/**
 * @brief Get stream configuration pointer
 * 
 * @return void*        Pointer of stream configuration
 */
void *app_ai_voice_get_stream_config(void);

/**
 * @brief Get AIV user type from AI spec
 * 
 * @param spec          AI spec
 * @return uint8_t      AIV user, @see AIV_USER_E to get more info
 */
uint8_t app_ai_voice_get_user_from_spec(uint8_t spec);

/**
 * @brief API to purchace pcm data and encode them 
 * 
 * @return bool         TRUE if any PCM data are purchased, otherwise FALSE
 */
bool app_ai_voice_pcm_data_encoding_handler(void);

/**
 * @brief API to get the current PCM buffer offset to be encoded
 * 
 * @return uint32_t
 */
uint32_t app_ai_voice_get_encoding_pcm_buf_offset(void);

#ifdef VOC_ENCODE_ENABLE
/**
 * @brief API to initialize the AI encoder
 * 
 * @param user          AI vioce user
 * @param code          encoder used codec
 * @return void
 */
void app_ai_voice_init_encoder(uint8_t user, uint8_t codec);

/**
 * @brief API to de-initialize the AI encoder
 * 
 * @param aiIndex          AI spec index
 * @return void
 */
void app_ai_voice_deinit_encoder(uint8_t aiIndex);
#endif

/**
 * @brief Get the interface handler for raw input pcm data
 * 
 * @param spec          void
 * @return uint8_t      void * pointer
 */
void * app_ai_voice_interface_of_raw_input_handler_get(void);

/**
 * @brief API to restart the AI voice stream
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @return void
 */
void app_ai_voice_stream_restart(uint8_t user);

/**
 * @brief Get active flag for concrete AI
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @return true         AI is active
 * @return false        AI is inactive
 */
bool app_ai_voice_get_active_flag(uint8_t user);

/**
 * @brief Set active flag for concrete AI
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @param active        active flag to set
 */
void app_ai_voice_set_active_flag(uint8_t user, bool active);

/**
 * @brief Get slaveOpenMic( @see AIV_FLAG_BUDLE_T) flag
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @return true         slave should open mic
 * @return false        slave shouldn't open mic
 */
bool app_ai_voice_get_slave_open_mic_flag(uint8_t user);

/**
 * @brief Set slaveOpenMic( @see AIV_FLAG_BUDLE_T) flag
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @param slaveOpenMic  slaveOpenMic flag to set
 */
void app_ai_voice_set_slave_open_mic_flag(uint8_t user, bool slaveOpenMic);

/**
 * @brief Set is-super-user flag( @see AIV_FLAG_BUDLE_T)
 * 
 * @param user          AI voice user, @see AIV_USER_E
 * @param isSuperUser   Set AI voice user as super user or not
 */
void app_ai_voice_set_super_user_flag(uint8_t user, bool isSuperUser);

/**
 * @brief is AI voice upstreaming
 * 
 * @return true         AI voice upstreaming
 * @return false        AI voice not upstreaming
 */
bool app_ai_voice_is_upstreaming(void);

/**
 * @brief Update read offset of AI voice buffer
 * 
 * @param offset        read offset to update
 */
void app_ai_voice_set_buf_read_offset(uint32_t offset);

/**
 * @brief Restore forcely closed cpature stream by super user
 * 
 */
void app_ai_voice_restore_capture_stream_after_super_user_stream_closed(void);

bool app_ai_voice_is_in_upstream_state(void);

/**
 * @brief Read the upstream voice start offset.
 *
 */
uint32_t app_ai_voice_upstream_read_offset_update(uint32_t pre_roll_len, uint32_t one_frame_len, uint32_t detect_offset);

/**
 * @brief Callback function for mic data comes
 * 
 * @param ptrBuf        Pointer of incoming data
 * @param length        Length of incoming data
 * @return uint32_t     Length of incoming data
 */
uint32_t _mic_data_come(uint8_t *ptrBuf, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif

