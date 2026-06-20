#pragma once
#include <functional>
#include <stdint.h>
#include "IMvds.h"
namespace smf {
	class IVoipVideo {
	protected:
		~IVoipVideo() {}
	public:
		struct IVoipVideoUp_t{
			uint32_t strm_id;
			uint16_t pltype;
			uint32_t width;
			uint32_t height;
			uint32_t level;
			uint32_t bit_rate;
			uint32_t frame_rate;
			bool support_nack;
		};
		struct IVoipVideoDn_t{
			uint32_t strm_id;
			uint16_t pltype;
			uint64_t codec_type;
			uint64_t format;
			uint32_t width;
			uint32_t height;
			uint32_t bit_rate;
			uint32_t frame_rate;
			bool is_voip;
			bool support_nack;
		};

		static IVoipVideo* GetVoipVideo();
		virtual bool downStartVideo(IVoipVideoDn_t* dn) = 0;
		virtual bool downStopVideo() = 0;
		virtual bool upStartVideo(IVoipVideoUp_t* up) = 0;
		virtual bool upStopVideo() = 0;
		virtual bool processRtpDataVideo(uint8_t *data, uint32_t len, bool is_rtcp = false) = 0;
		virtual bool addExtraParaSet(char *s) = 0;
		virtual bool setCamera(bool on) = 0;
	public:
		IMvds* _imvds = 0;
		char * _para_sets = 0;
		bool _permit_send = true;
		bool _permite_recv = true;
};
}