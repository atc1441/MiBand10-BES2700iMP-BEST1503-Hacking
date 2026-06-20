/* apps/doom/audio_stubs_cpp.cpp - C++ link stubs for the DOOM build (no audio HW).
 *
 * These few audio functions have C++ linkage (mangled names). They are referenced
 * only by never-executed BT/app code (main() runs hw_selftest()->doom which never
 * returns), so empty bodies just satisfy the linker. Parameter types must match
 * the originals so the mangled names line up; return type is irrelevant to
 * mangling and to runtime (never called). See audio_stubs.c for the C symbols. */
#ifdef DOOM

struct APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T;   /* opaque - only used by pointer */

void app_bt_audio_a2dp_stream_recheck_timer_callback(void const *) {}
void app_bt_audio_avrcp_play_status_wait_timer_callback(void const *) {}
void app_bt_audio_avrcp_status_quick_switch_filter_timer_callback(void const *) {}
void app_bt_audio_check_a2dp_restreaming_timer_handler(void const *) {}
void app_bt_audio_delay_play_a2dp_timer_handler(void const *) {}
void app_bt_stream_ibrt_mobile_link_playback_info_receive(unsigned char, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *) {}
void app_bt_stream_ibrt_set_trigger_time(unsigned char, APP_TWS_IBRT_AUDIO_SYNC_TRIGGER_T *) {}
void bt_sco_chain_set_master_role(bool) {}

#endif
