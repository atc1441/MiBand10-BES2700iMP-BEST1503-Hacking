#ifndef __AUDIO_TRIGGER_COMMON_H__
#define __AUDIO_TRIGGER_COMMON_H__

enum BT_STREAM_TRIGGER_STATUS_T {
    BT_STREAM_TRIGGER_STATUS_NULL = 0,
    BT_STREAM_TRIGGER_STATUS_INIT,
    BT_STREAM_TRIGGER_STATUS_WAIT,
    BT_STREAM_TRIGGER_STATUS_OK,
};

#ifdef __cplusplus
extern "C" {
#endif

void app_bt_stream_trigger_stauts_set(enum BT_STREAM_TRIGGER_STATUS_T stauts);
enum BT_STREAM_TRIGGER_STATUS_T app_bt_stream_trigger_stauts_get(void);

#ifdef __cplusplus
}
#endif

#endif
