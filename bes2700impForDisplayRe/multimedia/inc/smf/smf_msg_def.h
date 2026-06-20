#pragma once

enum smf_msg_e {
	SMF_MSG_SHAOPIAN = 0x000,//000~03f
	SMF_MSG_USER = 0x040,//040~0ff
	SMF_MSG_API = 0xa00,//a00~aff
	SMF_MSG_MSVC = 0xf00,//f00~fff
	//
	SMF_MSG_POOL = 0x210,
	SMF_MSG_CMD = 0x220,
	SMF_MSG_FS = 0x230,
	SMF_MSG_VOIP = 0x240,
	SMF_MSG_SHM = 0x250,
	SMF_MSG_PROXY = 0x260,
	SMF_MSG_FRAME = 0x270,
	SMF_MSG_STREAM = 0x280,
	SMF_MSG_VIDEO = 0x290,
	SMF_MSG_LCDC = 0x2a0,
	//
	SMF_MSG_TWSPB = 0x4d0,
	//
	SMF_MSG_AudioPlayer = 0x500,
	SMF_MSG_AudioRecord = 0X540,
	SMF_MSG_VideoPlayer = 0x580,
	SMF_MSG_VideoRecord = 0X5c0,
	//
	SMF_MSG_HOOK = 0x838,
	SMF_MSG_LocalPlayer = 0x840,
	SMF_MSG_LocalRecord = 0x850,
	SMF_MSG_LocalSco = 0x860,
	SMF_MSG_APPlayer = 0x870,
	SMF_MSG_APRecord = 0x878,
	//SMF_MSG_APSco = 0x880,
	SMF_MSG_ESIMSco = 0x8e0,
	SMF_MSG_LocalKws = 0x8f0,
	SMF_MSG_Jpeg = 0xe00,
	//
	SMF_MSG_Device = 0xf0a,
};


