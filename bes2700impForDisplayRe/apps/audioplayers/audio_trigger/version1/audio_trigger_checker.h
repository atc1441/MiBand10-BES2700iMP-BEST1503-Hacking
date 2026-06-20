#ifndef __AUDIO_TRIGGER_CHECKER_H__
#define __AUDIO_TRIGGER_CHECKER_H__

#define TRIGGER_CHECKER_A2DP_PLAYERBLACK        (1<<0)
#define TRIGGER_CHECKER_A2DP_DONE               (TRIGGER_CHECKER_A2DP_PLAYERBLACK)

#define TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK   (1<<1)
#define TRIGGER_CHECKER_HFP_BTPCM_CAPTURE       (1<<2)
#define TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK  (1<<3)
#define TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE      (1<<4)
#define TRIGGER_CHECKER_HFP_DONE                (TRIGGER_CHECKER_HFP_BTPCM_PLAYERBLACK|TRIGGER_CHECKER_HFP_BTPCM_CAPTURE|TRIGGER_CHECKER_HFP_AUDPCM_PLAYERBLACK|TRIGGER_CHECKER_HFP_AUDPCM_CAPTURE)

typedef int (*APP_BT_STREAM_TRIGGER_CHECKER_TRIGOK_CB)(uint32_t);


#ifdef __cplusplus
extern "C" {
#endif

int app_bt_stream_trigger_checker_handler(uint32_t trigger_checker);
int app_bt_stream_trigger_checker_set_cb(APP_BT_STREAM_TRIGGER_CHECKER_TRIGOK_CB trigok_cb_fn);
int app_bt_stream_trigger_checker_init(void);
int app_bt_stream_trigger_checker_start(void);
int app_bt_stream_trigger_checker_stop(void);

#ifdef __cplusplus
}
#endif

#endif
