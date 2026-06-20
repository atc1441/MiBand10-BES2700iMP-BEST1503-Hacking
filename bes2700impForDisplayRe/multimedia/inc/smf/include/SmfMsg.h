#pragma once
#include "smf_msg.h"
#include "smf_msg_def.h"

namespace smf {
	namespace msg {
		enum class EXT {
			null = 0,
			callback,
			signal,
		};
		template<class ...TS>
		class TClass :public TS...{};
		//
		using MsgExt = SmfMsgResponse_t;
		template<class ...T>
		using TMsgExt = TClass<MsgExt, T...>;
		//
		template<class ...T>
		using TMsg = TClass<SmfMsg_t, T...>;
		template<class ...T>
		using TMsgResponse = TClass<SmfMsgResponse_t, T...>;
		////////////////////////////////////
		//msg ext with callback
		struct ExtCallBack_t {
			bool (*callback)(TMsgExt<ExtCallBack_t>* msg);
		};
		using MsgExtCallback = TMsgExt<ExtCallBack_t>;
		template<class ...T>
		using TMsgExtCallback = TClass<MsgExtCallback,T...>;
		//////////////////////////////////
		//msg ext with signal
		struct ExtSignal_t {
			uint32_t taskid;
			TMsgExt<ExtSignal_t>* msg;
		};
		using MsgExtSignal = TMsgExt<ExtSignal_t>;
		template<class ...T>
		using TMsgExtSignal = TClass<MsgExtSignal,T...>;
		//msg ext with local&remote
		struct ExtLocalRemote_t {
			void* local;
			void* remote;
		};
		//
		bool MsgIsResponse(SmfMsg_t* msg);
		void MsgToResponse(SmfMsg_t* msg);
		ExtCallBack_t* MsgGetExtCallback(SmfMsg_t* msg);
		ExtSignal_t* MsgGetExtSignal(SmfMsg_t* msg);
		ExtLocalRemote_t* MsgGetExtLocalRemote(SmfMsg_t* msg);
		int MsgGetBodyOffset(SmfMsg_t* msg);
		template<class T>
		T* MsgGetBody(SmfMsg_t* msg) {
			return (T*)((char*)msg + MsgGetBodyOffset(msg));
		}		
	}
}
