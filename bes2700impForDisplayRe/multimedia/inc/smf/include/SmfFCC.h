#pragma once
#include "SmfString.h"
//#define fcc4(c0,c1,c2,c3) (((unsigned)c0)|((unsigned)c1<<8)|((unsigned)c2<<16)|((unsigned)c3<<24))
//#define fcc4x(c) fcc4(c[0],c[1],c[2],c[3])
namespace smf {
	template<class T, class X = XOther>
	struct TFcc {
		static constexpr inline T fcc(T v, char c) {
			return (v << 8) | X::proc(c);
		}
		static constexpr inline T fcc(const char* s, T v = 0, uint32_t i = 0) {
			return (s && (i < sizeof(T)) && IsChar(*s)) ? fcc(fcc(s + 1, v, i + 1), *s) : 0;
		}
		static constexpr inline T fccall(const char* s, T v = 0, uint32_t i = 0) {
			return (s && (i < sizeof(T))) ? fcc(fccall(s + 1, v, i + 1), *s) : 0;
		}

		//static T fccx(const char*& s, T v = 0, int i = 0) {
		//	char c;
		//	return (s && (i < sizeof(T)) && (c = *s) && IsChar(c)) ? fcc(fccx(++s, v, i + 1), c) : 0;
		//}
		static T fccx(const char*& s) {
			T v = 0;
			if (s) {
				for (uint32_t i = 0; i < sizeof(T); i++) {
					if (!IsChar(*s))break;
					v |= ((T)X::proc(*s)) << (i << 3);
					s++;
				}
			}
			return v;
		}
		static int fccx(const char*& str, T* vals, int max, char splitchr = '-') {
			for (uint32_t i = 0; i < max; i++) {
				vals[i] = fccx(str);
				if (!str || !*str || *str != splitchr)
					return i + 1;
				str++;
			}
			return max;
		}
	};
	//
	constexpr uint32_t fcc32(const char*s) {
		return TFcc<uint32_t, XOther>::fcc(s);
	}
	constexpr uint32_t fcc32up(const char*s) {
		return TFcc<uint32_t, XUpper>::fcc(s);
	}
	constexpr uint32_t fcc32low(const char*s) {
		return TFcc<uint32_t, XLower>::fcc(s);
	}
	constexpr uint64_t fcc64(const char* s) {
		return TFcc<uint64_t, XOther>::fcc(s);
	}
	constexpr uint64_t fcc64up(const char*s) {
		return TFcc<uint64_t, XUpper>::fcc(s);
	}
	constexpr uint64_t fcc64low(const char*s) {
		return TFcc<uint64_t, XLower>::fcc(s);
	}
	
	static inline uint32_t fcc32x(const char*& s) {
		return TFcc<uint32_t, XOther>::fccx(s);
	}
	static inline uint64_t fcc64x(const char*& s) {
		return TFcc<uint64_t, XOther>::fccx(s);
	}

	constexpr uint32_t fccall32(const char* s) {
		return TFcc<uint32_t, XOther>::fccall(s);
	}
	constexpr uint32_t fccall32up(const char* s) {
		return TFcc<uint32_t, XUpper>::fccall(s);
	}
	constexpr uint32_t fccall32low(const char* s) {
		return TFcc<uint32_t, XLower>::fccall(s);
	}
	constexpr uint64_t fccall64(const char* s) {
		return TFcc<uint64_t, XOther>::fccall(s);
	}
	constexpr uint64_t fccall64up(const char* s) {
		return TFcc<uint64_t, XUpper>::fccall(s);
	}
	constexpr uint64_t fccall64low(const char* s) {
		return TFcc<uint64_t, XLower>::fccall(s);
	}
#if 0
	constexpr uint32_t fcc32(uint32_t v, const char s) {
		return (v << 8) | s;
	}
	constexpr uint32_t fcc32(uint32_t v, const char*s, int i, int c) {
		return (*s && i < c && IsChar(*s)) ? fcc32(fcc32(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint32_t fcc32(const char*s, int c = 4) {
		return s ? fcc32(0, s, 0, c) : 0;
	}
	//
	constexpr uint32_t fcc32up(uint32_t v, const char s) {
		return (v << 8) | Upper(s);
	}
	constexpr uint32_t fcc32up(uint32_t v, const char*s, int i, int c) {
		return (*s && i < c) ? fcc32up(fcc32up(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint32_t fcc32up(const char*s, int c = 4) {
		return s ? fcc32up(0, s, 0, c) : 0;
	}
	//
	constexpr uint32_t fcc32low(uint32_t v, const char s) {
		return (v << 8) | Lower(s);
	}
	constexpr uint32_t fcc32low(uint32_t v, const char*s, int i, int c) {
		return (*s && i < c && IsChar(*s)) ? fcc32low(fcc32low(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint32_t fcc32low(const char*s, int c = 4) {
		return s ? fcc32low(0, s, 0, c) : 0;
	}
	//
	constexpr uint64_t fcc64(uint64_t v, const char s) {
		return (v << 8) | s;
	}
	constexpr uint64_t fcc64(uint64_t v, const char*s, int i, int c) {
		return (*s && i < c  && IsChar(*s)) ? fcc64(fcc64(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint64_t fcc64(const char*s, int c = 8) {
		return s ? fcc64(0, s, 0, c) : 0;
	}
	//
	constexpr uint64_t fcc64up(uint64_t v, const char s) {
		return (v << 8) | Upper(s);
	}
	constexpr uint64_t fcc64up(uint64_t v, const char*s, int i, int c) {
		return (*s && i < c) ? fcc64up(fcc64up(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint64_t fcc64up(const char*s, int c = 8) {
		return s ? fcc64up(0, s, 0, c) : 0;
	}
	//
	constexpr uint64_t fcc64low(uint64_t v, const char s) {
		return (v << 8) | Lower(s);
	}
	constexpr uint64_t fcc64low(uint64_t v, const char*s, int i, int c) {
		return (*s && i < c) ? fcc64low(fcc64low(v, s + 1, i + 1, c), *s) : 0;
	}
	constexpr uint64_t fcc64low(const char*s, int c = 8) {
		return s ? fcc64low(0, s, 0, c) : 0;
	}
#endif
#if 0
	constexpr inline unsigned fcc32(const char* s, int c=0) {
		unsigned v = *s;
		if (!c || c > 4)c = 4;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned)s[i] << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned fcc32up(const char* s, int c = 0) {
		unsigned v = Upper(*s);
		if (!c || c > 4)c = 4;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned)Upper(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned fcc32low(const char* s, int c=0) {
		unsigned v = Lower(*s);
		if (!c || c > 4)c = 4;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned)Lower(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}

	constexpr inline unsigned long long fcc64(const char* s, int c=0) {
		unsigned long long v = *s;
		if (!c || c > 8)c = 8;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned long long)s[i] << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned long long fcc64up(const char* s, int c = 0) {return (unsigned long long)s[0]|s[1];
		unsigned long long v = Upper(*s);
		if (!c || c > 8)c = 8;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned long long)Upper(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
	constexpr inline unsigned long long fcc64low(const char* s, int c=0) {return (unsigned long long)s[0]|s[1];
		unsigned long long v = Lower(*s);
		if (!c || c > 8)c = 8;
		for (int i = 1; i < c; i++) {
			if (s[i]) {
				v |= (unsigned long long)Lower(s[i]) << (8 * i);
			}
			else {
				break;
			}
		}
		return v;
	}
#endif
}