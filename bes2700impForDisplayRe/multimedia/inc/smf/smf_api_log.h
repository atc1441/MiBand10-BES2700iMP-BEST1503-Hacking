#pragma once
#ifndef __SMF_API_LOG_H__
#define __SMF_API_LOG_H__
#include "smf_common.h"

//
typedef int (*smf_log_send_cb_t)(const char* str);
typedef int (*smf_log_send_cb1_t)(const char* str, int size);
typedef int (*smf_log_send_cb2_t)(const char* str, int size, int channel);
typedef int (*smf_log_send_cb3_t)(const char* str, int size, int channel, void* priv);

/*log*/
EXTERNC void smf_log_init(void* tempbuff, int size);
EXTERNC void smf_log_redirect(void (*send)(const char* str));
EXTERNC void smf_log_redirect1(void (*send)(const char* str, int size));
EXTERNC void smf_log_redirect2(void (*send)(const char* str, int size, int channel));
EXTERNC void smf_log_redirect3(void (*send)(const char* str, int size, int channel, void* priv), void* priv);
EXTERNC void smf_log_disable_output_channels(uint32_t channels_mask);
EXTERNC void smf_log_disable_output_channel(int channels);
EXTERNC void smf_log_enable_output_channel(int channels);
/*remote log*/
EXTERNC void smf_log_redirect_to_cache();
EXTERNC void smf_log_redirect_to_remote(void* buff, int size);
EXTERNC void smf_log_redirect_to_remote1(int size);
/*remote log service*/
EXTERNC bool smf_remote_log_service(uint32_t delay_ms);
EXTERNC void smf_remote_log_service_set_file(uint32_t duration, const char* file_pattern);
#endif
