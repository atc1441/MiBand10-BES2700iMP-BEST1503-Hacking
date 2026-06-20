#ifndef __LHDC_ENC_UTIL_H__
#define __LHDC_ENC_UTIL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lhdc_cfg.h"
	
typedef enum {
  LHDCV3_BLOCK_SIZE = 256,
  LHDCV2_BLOCK_SIZE = 512, //unsupported!!!!
} lhdc_block_size_t;

typedef enum {
  LHDC_PARA_ERR = -5,
  LHDC_ERR 		= -4,
  LHDC_NO_ERR = 0,
  LHDC_NEED_MORE,
  LHDC_CONTINUE,

} lhdc_enc_result_t;

typedef struct bes_bt_local_info_t{
    uint8_t bt_addr[BTIF_BD_ADDR_SIZE];
    const char *bt_name;
    uint8_t bt_len;
    uint8_t ble_addr[BTIF_BD_ADDR_SIZE];
    const char *ble_name;
    uint8_t ble_len;
}bes_bt_local_info;

typedef lhdc_enc_result_t (*LHDC_GET_BT_INFO)(bes_bt_local_info * bt_info);

lhdc_enc_result_t lhdc_enc_init(int32_t sr, int32_t bps, int32_t bit_rate, int32_t mtu, int32_t interval, bool onc_channel_per_frame);

lhdc_enc_result_t lhdc_enc_encode(uint8_t* p_pcm, uint8_t* out_put, uint32_t * written, uint32_t * out_fraems);

lhdc_enc_result_t lhdc_enc_set_bitrate(int32_t bitrate);

lhdc_enc_result_t lhdc_enc_deinit(void);

lhdc_enc_result_t lhdc_enc_verify_license(void);

const char * lhdc_enc_get_version(void);

#ifdef __cplusplus
}
#endif

#endif //end of __LHDC_ENC_UTIL_H__
