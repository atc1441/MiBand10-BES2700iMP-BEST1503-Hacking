#include "cmsis_os.h"
#include "bluetooth_bt_api.h"
#include "app_bt_audio.h"
#include "audio_trigger_common.h"

static enum BT_STREAM_TRIGGER_STATUS_T bt_stream_trigger_status = BT_STREAM_TRIGGER_STATUS_NULL;

void app_bt_stream_trigger_stauts_set(enum BT_STREAM_TRIGGER_STATUS_T stauts)
{
    TRACE_AUD_STREAM_I("[STRM_TRIG] stauts_set %d->%d", bt_stream_trigger_status,stauts);
    bt_stream_trigger_status = stauts;
}

enum BT_STREAM_TRIGGER_STATUS_T app_bt_stream_trigger_stauts_get(void)
{
    //TRACE(1,"bt_stream_trigger_stauts_get:%d", bt_stream_trigger_status);
    return bt_stream_trigger_status;
}

