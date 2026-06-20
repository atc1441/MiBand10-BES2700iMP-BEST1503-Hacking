#pragma once
#include <stdint.h>
namespace smf {
	struct XOther {
		static constexpr inline char proc(char c) { return c; };
	};
	struct XLower {
		static constexpr inline char proc(char c) { return (c <= 'Z' && c >= 'A') ? (c + ('a' - 'A')) : c; };
	};
	struct XUpper {
		static constexpr inline char proc(char c) { return (c <= 'z' && c >= 'a') ? (c + ('A' - 'a')) : c; };
	};
	///
	constexpr inline bool IsChar(char c) {
		return (c <= 'Z' && c >= 'A') || (c <= 'z' && c >= 'a') || (c <= '9' && c >= '0') || (c == '_') || (c == '.') || (c == ' ');
	}
	constexpr inline char Lower(char c) { return XLower::proc(c); }//{ return (c <= 'Z' && c >= 'A') ? (c + ('a' - 'A')) : c; };
	constexpr inline char Upper(char c) { return XUpper::proc(c); }//{ return (c <= 'z' && c >= 'a') ? (c + ('A' - 'a')) : c; };
	///
	constexpr inline uint16_t Swap16(const uint16_t v) {
		return ((((v) & 0xff) << 8) | (((v) >> 8) & 0xff));
	}
	constexpr inline uint32_t Swap32(const uint32_t v) {
		return (((v) << 24) | ((v) >> 24) | ((v) >> 8 << 24 >> 8) | ((v) << 8 >> 24 << 8));
	}
	constexpr inline uint64_t Swap64(const uint64_t v) {
		return Swap32((uint32_t)v) | ((uint64_t)Swap32(v >> 32) << 32);
	}
	///
	constexpr inline uint16_t Swap(const uint16_t v) {
		return Swap16(v);
	}
	constexpr inline uint32_t Swap(const uint32_t v) {
		return Swap32(v);
	}
	constexpr inline uint64_t Swap(const uint64_t v) {
		return Swap64(v);
	}
	///
	constexpr inline uint8_t Read8(const void* ptr) {
		return ((uint8_t)((uint8_t*)ptr)[0]);
	}
	constexpr inline uint16_t Read16(const void* ptr) {
		return ((uint16_t)((uint8_t*)ptr)[0] | ((uint16_t)((uint8_t*)ptr)[1] << 8));
	}
	constexpr inline uint32_t Read32(const void* ptr) {
		return ((uint32_t)((uint8_t*)ptr)[0] | ((uint32_t)((uint8_t*)ptr)[1] << 8) | ((uint32_t)((uint8_t*)ptr)[2] << 16) | ((uint32_t)((uint8_t*)ptr)[3] << 24));
	}
	constexpr inline uint64_t Read64(const void* ptr) {
		return Read32(ptr) | ((uint64_t)Read32((uint32_t*)ptr + 1) << 32);
	}
	///
	constexpr inline void Write8(void* ptr, uint8_t val) {
		((uint8_t*)ptr)[0] = val;
	}
	constexpr inline void Write16(void* ptr, uint16_t val) {
		((uint8_t*)ptr)[0] = val & 0xff;
		((uint8_t*)ptr)[1] = val >> 8;
	}
	constexpr inline void Write32(void* ptr, uint32_t val) {
		Write16(ptr, val & 0xffff);
		Write16((uint16_t*)ptr + 1, val >> 16);
	}
	constexpr inline void Write64(void* ptr, uint64_t val) {
		Write32(ptr, val & 0xffffffff);
		Write32((uint32_t*)ptr + 1, val >> 32);
	}

	constexpr inline uint32_t Align2(uint32_t x) {return (((x) + 1) & ~1);}
	constexpr inline uint32_t Align4(uint32_t x) {return (((x) + 3) & ~3);}
	constexpr inline uint32_t Align8(uint32_t x) {return (((x) + 7) & ~7);}
	constexpr inline uint32_t Align16(uint32_t x){return (((x) + 15) & ~15);}
	constexpr inline uint32_t Align32(uint32_t x){return (((x) + 31) & ~31);}
	constexpr inline uint32_t Align64(uint32_t x){return (((x) + 63) & ~63);}

	constexpr inline uint32_t Align2Lower(uint32_t x) {return ((x) & ~1);}
	constexpr inline uint32_t Align4Lower(uint32_t x) {return ((x) & ~3);}
	constexpr inline uint32_t Align8Lower(uint32_t x) {return ((x) & ~7);}
	constexpr inline uint32_t Align16Lower(uint32_t x){return ((x) & ~15);}
	constexpr inline uint32_t Align32Lower(uint32_t x){return ((x) & ~31);}
	constexpr inline uint32_t Align64Lower(uint32_t x){return ((x) & ~63);}
}