#ifndef __AUDIO_TRIGGER_IBRT_H__
#define __AUDIO_TRIGGER_IBRT_H__


#ifdef __cplusplus
extern "C" {
#endif

void app_bt_stream_ibrt_auto_synchronize_stop(void);
int app_bt_stream_ibrt_audio_master_detect_next_packet_start(void);
int app_bt_stream_ibrt_audio_slave_detect_next_packet_start(int need_autotrigger);
void app_bt_stream_ibrt_reset_synchronize_status(void);

#ifdef __cplusplus
}
#endif

#endif
