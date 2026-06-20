#pragma once
#ifndef __SMF_API_ABOX_H__
#define __SMF_API_ABOX_H__
#include "smf_common.h"
EXTERNC bool smf_abox_prompt_play(bool onoff, void* status);
EXTERNC bool smf_abox_add_lineinsouce(void);
EXTERNC bool smf_abox_remove_lineinsouce(void);
EXTERNC uint64_t smf_abox_add_filesouce(const char* fname, uint32_t position);
EXTERNC bool smf_abox_remove_filesouce(void);
EXTERNC bool smf_abox_add_btsouce(const char* codec);
EXTERNC bool smf_abox_remove_btsouce(void);
EXTERNC bool smf_abox_add_blesouce(uint8_t con_lid, uint32_t context_type);
EXTERNC bool smf_abox_remove_blesouce(void);

EXTERNC bool smf_abox_add_afsink(uint8_t sync_type);
EXTERNC bool smf_abox_remove_afsink(void);
EXTERNC bool smf_abox_add_blesink(void* event);
EXTERNC bool smf_abox_remove_blesink(void);
#endif
