#pragma once
#include "stdint.h"

namespace smf {
	namespace op {
		using int08_t = int8_t;
		struct int24_t {
			int24_t(int v) { *this = v; }
			uint8_t _vs[3];
			inline int operator+(int24_t& v) { return (int)*this + (int)v; }
			inline int operator<<(int shf) { return (int)*this << shf; }
			inline int operator>>(int shf) { return (int)*this >> shf; }
			inline int operator<<=(int shf) { return *this=(int)*this << shf; }
			inline int operator>>=(int shf) { return *this=(int)*this >> shf; }
			inline int operator=(int v) { _vs[0] = ((uint8_t*)&v)[0]; _vs[1] = ((uint8_t*)&v)[1]; _vs[2] = ((uint8_t*)&v)[2]; return v; }
			inline int operator=(int24_t& v) { _vs[0] = v._vs[0]; _vs[1] = v._vs[1]; _vs[2] = v._vs[2]; return v; }
			inline operator int()const { return (int32_t)((_vs[2] << 24) | (_vs[1] << 16) | (_vs[0] << 8)) >> 8; }
		};

		///
		template<int __bit>
		struct sat_t;

		template<>
		struct sat_t<0> {
			template<class Ti>
			static inline int sat(Ti v) {
				return v;
			}
		};
		
		template<>
		struct sat_t<8> {
			template<class Ti>
			static inline int sat(Ti v) {
				return (v > 0x7f) ? 0x7f : ((v < -0x80) ? -0x80 : v);
			}
		};

		template<>
		struct sat_t<16> {
			template<class Ti>
			static inline int sat(Ti v) {
				return (int)((v > 0x7fff) ? 0x7fff : ((v < -0x8000) ? -0x8000 : v));
			}
		};

		template<>
		struct sat_t<24> {
			template<class Ti>
			static inline int sat(Ti v) {
				return (int)((v > 0x007fffff) ? 0x007fffff : ((v < -0x00800000l) ? -0x00800000l : v));
			}
		};

		template<>
		struct sat_t<32> {
			template<class Ti>
			static inline int sat(Ti v) {
				return (int)((v > 0x7fffffff) ? 0x7fffffff : ((v < -0x80000000ll) ? -0x80000000ll : v));
			}
		};

		///
		template<int __shift_type,class To,class Tx = int>
		struct shift_t;

		template<class To, class Tx>
		struct shift_t<0,To,Tx> {//no shift
			template<class Ti>
			static inline To shift(Ti x, int8_t shf) { return (To)x; }
		};

		template<class To, class Tx>
		struct shift_t<1, To, Tx> {//left shift
			template<class Ti>
			static inline To shift(Ti x, int8_t shf) { return (To)((Tx)x << shf); }
		};

		template<class To, class Tx>
		struct shift_t<-1, To, Tx> {//right shift
			template<class Ti>
			static inline To shift(Ti x, int8_t shf) { return (To)((Tx)x >> shf); }
		};

		template<class T>
		static inline T max(T x, T y) { return (((x) > (y)) ? (x) : (y)); }

		template<class T, class...Ts>
		static inline T max(T x, Ts...ts) { auto y = max(ts...);  return (((x) > (y))) ? (x) : (y); }

		template<class T>
		static inline T min(T x, T y) { return (((x) > (y)) ? (x) : (y)); }

		template<class T, class...Ts>
		static inline T min(T x, Ts...ts) { auto y = min(ts...);  return (((x) < (y))) ? (x) : (y); }
	}
}

