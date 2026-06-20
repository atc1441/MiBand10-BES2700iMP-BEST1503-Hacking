#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "smf_media_api_def.h"

#ifdef __cplusplus
extern "C" {
#endif

bool smf_media_service_init(void* buffer, uint32_t size);

bool smf_media_service_deinit(void);

bool smf_media_open(void);

bool smf_media_close(void);

/*config*/
bool smf_audio_player_mix_config(const smf_audio_mix_config_t* mix_cfg);

bool smf_audio_player_load_output(const smf_audio_output_config_t* config);

bool smf_audio_player_unload_output(const smf_audio_output_config_t* config);

bool smf_audio_recorder_load_input(const smf_audio_input_config_t* config);

bool smf_audio_recorder_unload_input(const smf_audio_input_config_t* config);

bool smf_audio_set_algo_param(const smf_audio_algo_set_param_t* param);

bool smf_audio_get_algo_param(const smf_audio_algo_get_param_t* param);

/*player*/
uint64_t smf_audio_player_start(const char* name, SMF_AUDIO_PLAYER_TYPE type, void* params);

bool smf_audio_player_set_volume(uint64_t id, uint16_t volume);

bool smf_audio_player_set_mute(uint64_t id, bool mute);

bool smf_audio_player_stop(uint64_t id);

bool smf_audio_player_pause(uint64_t id);

bool smf_audio_player_resume(uint64_t id);

uint32_t smf_audio_player_get_status(uint64_t id);

uint32_t smf_audio_player_get_currrent_postion(uint64_t id);

bool smf_audio_player_get_meta_info(const char* url, smf_audio_player_meta_info_t* info);

bool smf_audio_player_frame_push(uint64_t id, void* buff, int size);

bool smf_audio_player_button_tone(uint32_t num, uint32_t len);

bool smf_audio_device_register_status_callback(SmfAudioDeviceCallback cb);

bool smf_audio_device_remove_status_callback();

/*recoder*/
uint64_t smf_audio_recorder_start(const char* name, SMF_AUDIO_RECORDER_TYPE type, void* params);

bool smf_audio_recorder_stop(uint64_t id);

/*btsco*/
uint64_t smf_audio_btsco_start(const char* codec_type);

bool smf_audio_btsco_stop(uint64_t id);

bool smf_audio_btsco_set_down_volume(uint64_t id, uint16_t volume);

bool smf_audio_btsco_set_down_mute(uint64_t id, bool mute);

bool smf_audio_btsco_set_up_mute(uint64_t id, bool mute);

/*hook*/
bool smf_media_hook_register(SmfMediaHookCallback cbhook);

bool smf_media_hook_start(uint32_t id);

bool smf_media_hook_stop(uint32_t id);

/*kws*/
uint64_t smf_audio_kws_algo_start(const char* name, const smf_audio_algo_kws_t* params);

bool smf_audio_kws_algo_stop(uint64_t id);

uint64_t smf_audio_wakeup_start(const char* name, const smf_audio_algo_wakeup_t* params);

bool smf_audio_wakeup_stop(uint64_t id);

void smf_audio_register_wakeup_cb(SmfAudioWakeupCallback cb);

/*usb audio*/
uint64_t smf_audio_usb_start(smf_audio_usb_t params);

bool smf_audio_usb_stop(uint64_t id);

bool smf_audio_usb_promt_play(SMF_AUDIO_USB_PROMPT_TYPE prompt_type);

/*video*/
uint64_t smf_video_player_stream_start(const smf_video_player_stream_param_t* param);

bool smf_video_player_push_frame(uint64_t id, uint32_t type, const smf_media_push_frame_t* frame);

bool smf_video_player_stop(uint64_t id);

uint64_t smf_video_player_file_start(const smf_video_player_file_param_t* param_t);

uint32_t smf_video_player_file_get_position(uint64_t id);

bool smf_video_player_file_stop(uint64_t id);

/*mp4*/
uint64_t smf_media_file_muxer_start(const char* type, const smf_media_file_mux_param_t* param);

bool smf_media_file_muxer_stop(uint64_t id);

bool smf_media_file_muxer_push_frame(uint64_t id, SMF_MEDIA_MUX_STREAM_TYPE stream, const smf_media_push_frame_t* frame);

#ifdef __cplusplus
}
#endif
