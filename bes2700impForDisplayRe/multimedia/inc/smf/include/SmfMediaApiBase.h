#pragma once
#include "SmfMediaParams.h"
#include "SmfMediaApiDef.h"
#include "smf_frame.h"
#include <functional>
namespace smf {
	namespace api {
		class MediaApiBase {
		public:
			using AddParams_t = api::AddParams_t;
			using AddSource_t = api::AddParams_t;
			using AddSink_t = api::AddParams_t;
			using FuncMsgCb = std::function<bool(Msg_t*)>;
			using ePropsTypes = api::ePropsTypes;
		public:
			MediaApiBase(const char* path);
		protected:
			void* _handle = 0;
		public:
			static bool Config(const char*);
		public:
			uint64_t Add(AddParams_t& param);
			uint64_t AddSource(AddSource_t& param);
			uint64_t AddSink(AddSink_t& param);
			bool Remove(uint64_t id, int timeout = 0);
		public:
			bool Set(const char* keys, void* vals);
			bool Get(const char* keys, void* vals);
			bool Set(uint64_t id, SetParam_t& para);
			bool Get(uint64_t id, GetParam_t& para);
		public:
			bool SetVolume(uint64_t id,uint16_t volume);
			bool SetMute(uint64_t id,bool);
			uint32_t GetPosition(uint64_t id);
			bool IsRunning(uint64_t id);
			bool GetStatus(uint64_t id, eSts& sts);
			bool Push(uint64_t id, smf_frame_t& frm);
			bool Pull(uint64_t id, smf_frame_t& frm);
			bool Register(uint64_t id, const std::function<bool(smf_frame_t*)>&);
			bool RegisterMsgCb(uint64_t id, const FuncMsgCb& func);
			bool Pause(uint64_t id);
			bool Resume(uint64_t id);
		};
		using AudioApiBase = MediaApiBase;
		using FuncFreqCb = std::function<void(smf_mips_t&)>;
		using FuncWakeupCb = std::function<void(smf_wakeup_t&)>;
        typedef void (*CbHook)(uint32_t pointer, void* data, int size);
        //
        bool FreqCallbackRegister(FuncFreqCb& cb);
        bool ReportMips(int delay);
		bool WakeUpResCallback(FuncWakeupCb& cb);
        //
        bool HookStart(uint32_t pointer);
        bool HookStop(uint32_t pointer);
        bool HookRegister(CbHook cbhook);
        bool HookDeinit();
        bool AutoHookConfig(uint64_t mask);
	}
}