
#ifndef __BLE_AUDIO_TEST_H__
#define __BLE_AUDIO_TEST_H__

#ifdef __cplusplus
extern "C"{
#endif

void ble_audio_test_key_init(void);

void ble_audio_dflt_update_role(void);

void ble_audio_dflt_config(void);

bool ble_audio_dflt_check_device_is_master(uint8_t *address);

void ble_audio_uart_cmd_init(void);

void ble_audio_start_advertising();

const unsigned char* ble_audio_dflt_read_peer_tws_addr(void);

void ble_audio_test_config_dynamic_audio_sharing_master(void);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef __BLE_TEST_H__ */
