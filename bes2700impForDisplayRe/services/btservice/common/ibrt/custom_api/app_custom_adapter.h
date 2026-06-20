#ifndef __APP_CUSTOM_ADAPTER__
#define __APP_CUSTOM_ADAPTER__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined(IBRT_UI)

typedef enum
{
    SYSTEM_TX_RSSI_TYPE_BUD,
    SYSTEM_TX_RSSI_TYPE_PHONE,
} SYSTEM_TX_RSSI_TYPE_T;

typedef struct {
    uint8_t initiator_cnt;
    uint8_t responsor_cnt;
} ADPT_TWS_SHARE_INFO_T;

bool app_ctm_adpt_tx_pwr_rssi_read(SYSTEM_TX_RSSI_TYPE_T type, int *t_val, int *r_val);

uint32_t app_ctm_adpt_get_profiles_sync_time(const uint8_t *uuid_data_ptr, uint8_t uuid_len);

void app_ctm_adpt_tws_share_info_cmd_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

void app_ctm_adpt_tws_share_info_rsp_cb(uint16_t rsp_seq, uint8_t *buf, uint16_t len);

void app_ctm_adpt_init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
