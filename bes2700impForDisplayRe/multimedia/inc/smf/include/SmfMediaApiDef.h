#pragma once
#include "stdint.h"
#include "stdbool.h"
#include "smf_msg.h"

namespace smf {
	namespace api {
		enum class eMsg {
			AudioPlayerDone = 0x50e,
			AudioPlayerDoneResponse,
			AudioRecordDone = 0x54e,
			AudioRecordDoneResponse,
			VideoPlayerDone = 0x58e,
			VideoPlayerDoneResponse,
			VideoRecordDone = 0x5ce,
			VideoRecordDoneResponse,
			AudioPlayerEos = 0x510,
			AudioPlayerEosResponse,
			AudioPlayerError=0x518,
			AudioPlayerErrorResponse,
			AudioRecordEos = 0x550,
			AudioRecordEosResponse,
			VideoPlayerEos = 0x590,
			VideoPlayerEosResponse,
			VideoRecordEos = 0x5d0,
			VideoRecordEosResponse,
			AudioPlayerAdd = 0x502,
			AudioPlayerAddResponse,
			AudioPlayerRemove = 0x504,
			AudioPlayerRemoveResponse,
			AudioPlayerPause = 0x514,
			AudioPlayerPauseResponse,
			AudioPlayerResume = 0x516,
			AudioPlayerResumeResponse,
		};
		enum class eSts {
			Null,
			Running = 2,
			Pause
		};
		//
		struct Msg_t : public SmfMsg_t {
			uint64_t id;
		};
		//Done
		using MsgAudioPlayerDone_t = Msg_t;
		using MsgAudioRecordDone_t = Msg_t;
		using MsgVideoPlayerDone_t = Msg_t;
		using MsgVideoRecordDone_t = Msg_t;
		//Eos
		using MsgAudioPlayerEos_t = Msg_t;
		using MsgAudioRecordEos_t = Msg_t;
		using MsgVideoPlayerEos_t = Msg_t;
		using MsgVideoRecordEos_t = Msg_t;
	}
}

