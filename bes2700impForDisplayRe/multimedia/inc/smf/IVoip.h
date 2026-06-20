#pragma once
#include <functional>
#include <stdint.h>
#include "IMvcs.h"
namespace smf {
	class IVoip {
	protected:
		virtual ~IVoip() {}
	public:
		virtual bool Start(void* p0) = 0;///start with param
	public:
		struct VoipUp_t{
			uint32_t strm_id;
			uint16_t payload_type;
			uint16_t dtmf_payload_type;
			int32_t sample_rate;
			uint8_t bitrate_mode;
			uint8_t octet_align;
		};
		struct VoipDn_t{
			uint32_t strm_id;
			uint16_t payload_type;
			uint8_t octet_align;
		};

		static IVoip* GetVoip();
		virtual bool downStart(const char *codec, VoipDn_t* dn) = 0;
		virtual bool downStop() = 0;
		virtual bool processRtpData(uint8_t *data, uint32_t len, bool is_rtcp=false) = 0;
		virtual bool upStart(const char *codec, VoipUp_t* up) = 0;
		virtual bool upStop() = 0;
		virtual bool dtmfPlay(uint32_t num, uint32_t len, uint32_t index) = 0;
		virtual bool setEncRateMode(uint32_t rate_mode) = 0;
		virtual bool mixConfig(uint32_t rate, uint32_t ch, uint32_t bits, uint32_t framesize) = 0;
		virtual bool micConfig(uint32_t rate, uint32_t ch, uint32_t bits, bool is_monopoly) = 0;
		virtual bool spkConfig(uint32_t rate, uint32_t ch, uint32_t bits, uint32_t framesize, bool is_monopoly) = 0;
		virtual bool setRtpTimeout(uint32_t timeout_ms) = 0;
		virtual bool setMicMute(bool en) = 0;
	public:
		IMvcs* _imvcs;
};
}