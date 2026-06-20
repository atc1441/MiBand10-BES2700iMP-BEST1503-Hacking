/*
 * lhdcv5_util_dec.h
 *
 */

#ifndef LHDCV5_UTIL_DEC_H
#define LHDCV5_UTIL_DEC_H

#include <stdbool.h>
#include <stdint.h>
#include "bluetooth_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  VERSION_5 = 550
}lhdcv5_ver_t;

#ifdef PLAT_BES
#include "bluetooth.h"
typedef struct bes_bt_local_info_t{
    uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
    const char *bt_name;
    uint8_t bt_len;
    uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
    const char *ble_name;
    uint8_t ble_len;
}bes_bt_local_info;
typedef int (*LHDC_GET_BT_INFO)(bes_bt_local_info * bt_info);
#else

#define BD_ADDR_SIZE 6

typedef struct bt_local_info_t{
    uint8_t bt_addr[BD_ADDR_SIZE];
    const char *bt_name;
    uint8_t bt_len;
    uint8_t ble_addr[BD_ADDR_SIZE];
    const char *ble_name;
    uint8_t ble_len;
}bt_local_info;

typedef int (*LHDC_GET_BT_INFO)(bt_local_info * bt_info);

#endif

typedef struct _lhdcv5_frame_Info
{
    uint32_t frame_len;
    uint32_t isSplit;
    uint32_t isLeft;

} lhdcv5_frame_Info_t;

typedef enum {
    LHDCV5_OUTPUT_STEREO = 0,
    LHDCV5_OUTPUT_LEFT_CAHNNEL,
    LHDCV5_OUTPUT_RIGHT_CAHNNEL,
} lhdcv5_channel_t;

#define A2DP_LHDC_HDR_FRAME_NO_MASK 0xfc

//audio_fmt
#define LHDCV5_AUDIO_FMT_OUT_PCM            0   //2 channel input, 2 channel output (interleaved)
#define LHDCV5_AUDIO_FMT_OUT_PCM_16BIT      1   //2 channel input, 2 channel output (interleaved)
#define LHDCV5_AUDIO_FMT_INOUT_ONE_CHANNEL  2   //1 channel input, 1 channel output (not interleaved)
#define LHDCV5_AUDIO_FMT_OUT_DAT            3   //2 channel input, 2 channel output (not interleaved)

// return value
#define LHDCV5_UTIL_DEC_SUCCESS 0 
#define LHDCV5_UTIL_DEC_ERROR_NO_INIT -1
#define LHDCV5_UTIL_DEC_ERROR_PARAM -2
#define LHDCV5_UTIL_DEC_ERROR -3
#define LHDCV5_UTIL_DEC_ERROR_WRONG_DEC -10

typedef void (*print_log_fp)(char*  msg);






/**
 * @brief Initial lhdcv5 decoder. Must call this function before lhdcv5 decode process
 * 
 * @param[in] ptr, memory pointer for lhdcv5 decoder
 * @param[in] bitPerSample, bits per sample. Support 16bits, 24bits
 * @param[in] sampleRate, sample rate. Support 44100/48000/96000/192000 Hz
 * @param[in] audioFormat, audio_fmt. This version support LHDCV5_AUDIO_FMT_OUT_PCM, LHDCV5_AUDIO_FMT_INOUT_ONE_CHANNEL and LHDCV5_AUDIO_FMT_OUT_DAT.
 *                         This setting will effect input and output data format
 * @param[in] frm_duration, frame duration. This version only support 50
 * @param[in] lossless_enable, enable lossless or not. 1: enable. 0: disable. This version not support lossless
 * @param[in] version, Must be VERSION_5
 * @return return value
 */
int32_t lhdcv5_util_init_decoder(void *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t audioFormat, uint32_t frm_duration, uint32_t lossless_enable, lhdcv5_ver_t version);

/**
 * @brief lhdcv5 decoder processing
 * 
 * @param[in]  ptr, memory pointer for lhdcv5 decoder
 * @param[out] pOutBuf, pointer of output buffer (decode data)
 * @param[in]  pInput, pointer of input buffer (encoded data)
 * @param[in]  InLen, input buffer bytes length (pInput)
 * @param[out] OutLen, decode data bytes length (pOutBuf)
 * @return return value
 */
int32_t lhdcv5_util_dec_process(void *ptr, uint8_t * pOutBuf, uint8_t * pInput, uint32_t InLen, uint32_t *OutLen);

/**
 * @brief lhdc license key
 * 
 * @param[in] ptr, memory pointer for lhdcv5 decoder
 * @param[in] licTable, pointer of license key
 * @param[in] pFunc, callback function to get license information
 * @return true: success; false: fail.
 */
bool lhdcv5_util_set_license(void *ptr, uint8_t * licTable, LHDC_GET_BT_INFO pFunc);

/**
 * @brief lhdc license key period
 * 
 * @param[in] ptr, memory pointer for lhdcv5 decoder
 * @param[in] period, license check period (minutes), max is 10, min is 0
 * @return return value
 */
int32_t lhdcv5_util_set_license_check_period (void *ptr, uint8_t period);

/**
 * @brief Get string of LHDC-V version
 * 
 * @param none
 * @return version string information
 */
char *lhdcv5_util_dec_get_version();

/**
 * @brief Destory lhdcv5 decoder
 * 
 * @param[in] ptr, memory pointer for lhdcv5 decoder
 * @return return value
 */
int32_t lhdcv5_util_dec_destroy(void *ptr);

/**
 * @brief Register log callback function
 * 
 * @param[in] cb, callback function
 * @return none
 */
void lhdcv5_util_dec_register_log_cb(print_log_fp cb);

/**
 * @brief get sample size
 * 
 * @param ptr, memory pointer for lhdcv5 decode process
 * @return the sample size (will > 0). if <= 0, it is error, please check return value
 */
int32_t lhdcv5_util_dec_get_sample_size (void *ptr);

/**
 * @brief get frame size information
 * 
 * @param[in]  ptr, memory pointer for lhdcv5 decoder
 * @param[in]  frameData, pointer of encoded data frame 
 * @param[in]  frameDataLen, encdoed data length
 * @param[out] frameInfo, return frame information
 * @return return value
 */
int32_t lhdcv5_util_dec_fetch_frame_info(void *ptr, uint8_t *frameData, uint32_t frameDataLen, lhdcv5_frame_Info_t *frameInfo);

/**
 * @brief Set channel selection
 * 
 * @param[in] ptr, memory pointer for lhdcv5 decode process
 * @param channel_type, set lhdcv5_channel_t value. thise value will effect decode process (not format).
 * @return return value 
 */
int32_t lhdcv5_util_dec_channel_selsect(void *ptr, lhdcv5_channel_t channel_type);

/**
 * @brief Get the memory size required for LHDC-V decoding.
 * 
 * @param version, The version of LHDC decoder. 550: LHDC-V.
 * @param sampleRate, Sample rate. Support 44100Hz, 48000Hz, 96000Hz and 192000Hz
 * @param audioFormat, The audioFormat type of input and output buffer.
 * @param lossless_enable, enable lossless or not. 1: enable. 0: disable.
 * @param mem_req_bytes, return the memory size required of LHDC-V Decoder.
 * @return return value
 */
int32_t lhdcv5_util_dec_get_mem_req(lhdcv5_ver_t version, uint32_t sampleRate, uint32_t audioFormat, uint32_t lossless_enable, uint32_t *mem_req_bytes);

/**
 *  @brief Get lhdcdata before set license
 *  @param none
 *  @return lhdcdata
 */
uint8_t lhdcv5_util_get_lhdcdata(void);

#ifdef __cplusplus
}
#endif
#endif /* End of LHDCV5_UTIL_DEC_H */
