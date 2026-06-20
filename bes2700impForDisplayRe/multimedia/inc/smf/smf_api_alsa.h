#pragma once
#ifndef __SMF_API_ALSA_H__
#define __SMF_API_ALSA_H__
#include "smf_common.h"
EXTERNC void smf_msg_service_open();
EXTERNC void smf_msg_service_close();

EXTERNC bool smf_alsa_player_stop(uint64_t id);
EXTERNC bool smf_alsa_player_close(uint64_t id);
EXTERNC uint64_t smf_alsa_player_open(const char* codec, uint32_t rate, uint8_t ch, uint8_t bits, uint32_t shm);
EXTERNC bool smf_alsa_player_start(uint64_t id);
EXTERNC bool smf_alsa_player_receive_data(uint64_t id, void* buffer_s, int len, uint32_t seq, uint32_t flag);


EXTERNC bool smf_alsa_recorder_stop(uint64_t id);
EXTERNC bool smf_alsa_recorder_close(uint64_t id);
EXTERNC uint64_t smf_alsa_recorder_open(uint32_t rate, uint8_t ch, uint8_t bits, uint32_t shm);
EXTERNC bool smf_alsa_recorder_start(uint64_t id);
EXTERNC bool smf_alsa_recorder_get_data(uint64_t id, void* buffer, int len);

#endif
