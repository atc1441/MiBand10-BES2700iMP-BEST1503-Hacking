/*
 * lhdcUtil.h
 *
 *  Created on: 2001/1/3
 *      Author: eric.lee
 */

#ifndef LHDC_UTIL_H
#define LHDC_UTIL_H

#include <stdbool.h>
#include "bluetooth_bt_api.h"

#ifdef __cplusplus
extern "C" {
#endif

// Define for LHDC stream type.
typedef enum {
    LHDC_STRM_TYPE_COMBINE,
    LHDC_STRM_TYPE_SPLIT
}LHDC_STRM_TYPE;

typedef enum {
  VERSION_2 = 200,
  VERSION_3 = 300,
  VERSION_4 = 400,
  VERSION_LLAC = 500
}lhdc_ver_t;

typedef enum {
  LHDCV2_BLOCK_SIZE = 512,
  LHDCV3_BLOCK_SIZE = 256,
}lhdc_block_size_t;

typedef struct bes_bt_local_info_t{
    uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
    const char *bt_name;
    uint8_t bt_len;
    uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
    const char *ble_name;
    uint8_t ble_len;
}bes_bt_local_info;
 
typedef struct _lhdc_frame_Info
{
    uint32_t frame_len; // 该 frame 的长处，若是分离压缩，则表示单一声道的 frame 长度。
    uint32_t isSplit;   // 是否为分离方式压缩
    uint32_t isLeft;    // 左声道 == true， 右声道 == false

} lhdc_frame_Info_t;


typedef enum {
    LHDC_OUTPUT_STEREO = 0,
    LHDC_OUTPUT_LEFT_CAHNNEL,
    LHDC_OUTPUT_RIGHT_CAHNNEL,
} lhdc_channel_t;

typedef int LHDCSample;

typedef void (*print_log_fp)(char*  msg);
typedef int (*LHDC_GET_BT_INFO)(bes_bt_local_info * bt_info);

#define A2DP_LHDC_HDR_LATENCY_LOW   0x00
#define A2DP_LHDC_HDR_LATENCY_MID   0x01
#define A2DP_LHDC_HDR_LATENCY_HIGH  0x02
#define A2DP_LHDC_HDR_LATENCY_MASK  (A2DP_LHDC_HDR_LATENCY_MID | A2DP_LHDC_HDR_LATENCY_HIGH)

#define A2DP_LHDC_HDR_FRAME_NO_MASK 0xfc


void lhdcInit(uint32_t *ptr, uint32_t bitPerSample, uint32_t sampleRate, uint32_t scaleTo16Bits, lhdc_ver_t version);
bool lhdcReadyForInput(void);
uint32_t lhdcPutData(uint8_t * pInpBuf, uint32_t NumBytes);
//uint32_t lhdcDecodeProcess(uint8_t * pOutBuf);
uint32_t lhdcDecodeProcess(uint32_t *ptr, uint8_t * pOutBuf, uint8_t * pInput, uint32_t len);
bool lhdcSetLicenseKeyTable(uint32_t *ptr, uint8_t * licTable, LHDC_GET_BT_INFO pFunc);
void lhdcSetLicenseKeyChkPeriod (uint32_t *ptr, uint8_t period);
bool larcIsEnabled();
char * getVersionCode();

void lhdcDestroy();

void lhdc_register_log_cb(print_log_fp cb);

uint32_t lhdcGetSampleSize(uint32_t *ptr);
bool lhdcFetchFrameInfo(uint32_t *ptr, uint8_t * frameData, lhdc_frame_Info_t * frameInfo);

uint32_t lhdcChannelSelsect(uint32_t *ptr, lhdc_channel_t channel_type);
uint32_t lhdc_util_dec_get_mem_req(lhdc_ver_t version, uint32_t *mem_req_bytes);

#ifdef __cplusplus
}
#endif
#endif /* End of LHDC_UTIL_H */
