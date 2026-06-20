#pragma once
#ifndef __SMF_LOCAL_PLAYER_H__
#define __SMF_LOCAL_PLAYER_H__
// #include "smf_api.h"
// #include "smf_debug.h"
#include "smf_common.h"
#ifndef EXTERNC
#ifndef __cplusplus
#define EXTERNC
#else
#define EXTERNC extern "C"
#endif
#endif
struct local_play_meida_info{
	unsigned char channels;
	unsigned int  sample_bits;
};
enum playerMode{
	Sequence = 1,
	SingleLoop,
	Random
};

enum switchMode{
	Null,
	Next,
	Prev
};

struct smfLocalPlayParam{
	const char* musicPath;
	const char* configPath;
	const char* playListPath;
	enum playerMode mode;
	const char * assignMusic;
	bool retainLst;
	uint32_t startVolume;
	bool bis_mode;
};

struct local_play_device_info{
	unsigned int channel_num;
	unsigned int device_id;
	unsigned int device_map;
};

typedef void (*notify)(uint32_t,const char *);//   0:SWITCH_MUSIC 1:MUSIC_LIST_NUULL,2:UKNOW_MUSIC 
typedef void (*algo_open)();
typedef void (*algo_close)();
typedef void (*algo_filter)(uint8_t*,uint32_t);
typedef void (*cb_filter)(void* odata,void* idata,uint32_t isize,uint32_t chscale);

typedef enum{
	BisReady
} smf_local_play_notify_def;

EXTERNC bool smf_msg_service_open();

EXTERNC bool smf_msg_service_close();

/*-----------------LOCAL PLAY API------------------*/
EXTERNC bool smf_local_player_start(const char* musicPath,const char* configPath,const char* playListPath,enum playerMode mode,const char * assignMusic, \
									bool retainLst,uint32_t startVolume, struct local_play_device_info *devInfo, bool BisMode);
EXTERNC void smf_local_player_config();

EXTERNC WEAK bool smf_local_player_simple(const char* musicPath, uint32_t pos);

EXTERNC WEAK bool smf_local_player_simple_pcm(const char* musicPath, uint32_t pos, uint32_t rate, uint8_t ch, uint8_t bits);

EXTERNC bool smf_local_player_stop();

EXTERNC bool smf_local_player_pause();

EXTERNC bool smf_local_player_getMediaFromPath(const char* path,smf_meta_info_t* pMeta, local_play_meida_info *pMedia);

EXTERNC bool smf_local_player_getMusicPosition(uint32_t* pos);

EXTERNC bool smf_local_player_next();

EXTERNC bool smf_local_player_previous();

EXTERNC bool smf_local_player_setMode(uint32_t mode);

/*------------------TWS API---------------------*/
EXTERNC bool smf_tws_start(uint32_t mode);

EXTERNC bool smf_tws_stop(uint32_t mode);

EXTERNC void smf_tws_config();
/*-----------------LOCAL PLAY DEMO----------------*/
EXTERNC bool smf_local_player_start_test();
/*-----------------LOCA PLAY EXTRA API----------------*/
EXTERNC bool smf_local_player_start_with_param();

EXTERNC bool smf_local_player_start_set_param(struct smfLocalPlayParam*, struct local_play_device_info*);

EXTERNC WEAK bool smf_local_player_set_volume(uint32_t vol);

EXTERNC bool smf_local_player_register_algo_callback(algo_open, algo_close, algo_filter);

EXTERNC WEAK bool smf_local_player_register_notify(notify cb);

EXTERNC WEAK bool smf_local_player_unregister_notify();
#endif

