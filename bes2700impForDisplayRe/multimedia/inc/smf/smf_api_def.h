#pragma once
#ifndef __SMF_API_DEF_H__
#define __SMF_API_DEF_H__

#define FCC1(a) ((a))
#define FCC2(a,b) ((a)|((b)<<8))
#define FCC3(a,b,c) ((a)|((b)<<8)|((c)<<16))
#define FCC4(a,b,c,d) ((a)|((b)<<8)|((c)<<16)|((d)<<24))
#define FCC5(a,b,c,d,e) (FCC4(a,b,c,d)|(((uint64_t)FCC1(e))<<32))
#define FCC6(a,b,c,d,e,f) (FCC4(a,b,c,d)|(((uint64_t)FCC2(e,f))<<32))
#define FCC7(a,b,c,d,e,f,g) (FCC4(a,b,c,d)|(((uint64_t)FCC3(e,f,g))<<32))
#define FCC8(a,b,c,d,e,f,g,h) (FCC4(a,b,c,d)|(((uint64_t)FCC4(e,f,g,h))<<32))

enum smf_frame_flags_e{
	SMF_FRAME_FLAGS_NONCACHE = 1<<23,
};
/// <summary>
/// create by vs/tools/SmfParamCreate.cpp
/// </summary>
enum smf_param_e{
	SMF_PARAMs = 0,	
	SMF_PARAM_Name					=0x8D39BDE6,//Hash("Name")
	SMF_PARAM_ID					=0x37386AE0,//Hash("ID")
	SMF_PARAM_Status                =0xBA4B77EF,//Hash("Status")
	SMF_PARAM_OpenParam             =0x2FF08426,//Hash("OpenParam")
	SMF_PARAM_OpenParamSize         =0x03CA5927,//Hash("OpenParamSize")
	SMF_PARAM_Timeout               =0x97F68388,//Hash("Timeout")
	SMF_PARAM_InputCallback         =0x495C96A4,//Hash("InputCallback")
	SMF_PARAM_InputCallbackPriv     =0xCC8CACBD,//Hash("InputCallbackPriv")
	SMF_PARAM_InputStatus           =0x27FBB60D,//Hash("InputStatus")
	SMF_PARAM_InputMin              =0x2E3F356D,//Hash("InputMin")
	SMF_PARAM_InputMax              =0x1829F813,//Hash("InputMax")
	SMF_PARAM_InputMediaInfo        =0x2B99F58D,//Hash("InputMediaInfo")
	SMF_PARAM_InputResidue          =0x1E839054,//Hash("InputResidue")
	SMF_PARAM_OutputCallback        =0x8A8C0A0F,//Hash("OutputCallback")
	SMF_PARAM_OutputCallbackPriv    =0x1506002E,//Hash("OutputCallbackPriv")
	SMF_PARAM_OutputStatus          =0x5651080E,//Hash("OutputStatus")
	SMF_PARAM_OutputMin             =0x1911AE38,//Hash("OutputMin")
	SMF_PARAM_OutputMax             =0x2F255892,//Hash("OutputMax")
	SMF_PARAM_OutputMediaInfo       =0x83C8EED4,//Hash("OutputMediaInfo")
	SMF_PARAM_SupportSuperFrame     =0xAFA720B8,//Hash("SupportSuperFrame")
	SMF_PARAM_DynamicMemorySize     =0xC088C7B8,//Hash("DynamicMemorySize")
	SMF_PARAM_DynamicMemorySizeMax  =0xECD82856,//Hash("DynamicMemorySizeMax")
	SMF_PARAM_MediaInfo             =0x2C045957,//Hash("MediaInfo")
	SMF_PARAM_VBR					=0x7C2E2079,//Hash("VBR")
	SMF_PARAM_ChannelSelect         =0x09ABD2DC,//Hash("ChannelSelect")
	SMF_PARAM_IsInterlaced          =0x3CEEE28A,//Hash("IsInterlaced")
};
enum smf_param_old_e {
	SMF_GET_MEDIA_INFO				= SMF_PARAM_MediaInfo,
};
enum smf_channels_e{
	SMF_CHANNEL_0,
	SMF_CHANNEL_1,
	SMF_CHANNEL_2,
	SMF_CHANNEL_3,
	SMF_CHANNEL_4,
	SMF_CHANNEL_5,
	SMF_CHANNEL_6,
	SMF_CHANNEL_7,
	SMF_CHANNEL_8,
	SMF_CHANNEL_9,

	SMF_CHANNEL_L = SMF_CHANNEL_0,
	SMF_CHANNEL_R = SMF_CHANNEL_1,
	
	SMF_CHANNEL_STEREO = 10,	
	SMF_CHANNEL_LRMERGE,

	SMF_CHANNEL_NULL=99,
};
enum smf_status_e {
	SMF_STS_Null = 0,
	SMF_STS_Ready,
	SMF_STS_Play,
	SMF_STS_Pause,
	SMF_STS_Max,
};

enum smf_message_e: uint64_t {
	SMF_MESSAGE_ERROR = FCC5('e','r','r','o','r'),
	SMF_MESSAGE_EOS = FCC3('e','o','s'),
	SMF_MESSAGE_DONE = FCC4('d', 'o', 'n', 'e'),
};
#endif
