#pragma once
#if 0
constexpr unsigned dbgFileNameHash(const char* fn) {
#define HASH_FNV_offset_basis 2166136261U//0x811c9dc5
#define HASH_FNV_prime 16777619U//0x01000193
	const char* ptr = fn;
	const char* lst = 0;
	//get filename without path
	while (*ptr) {
		if (*ptr == '/' || *ptr == '\\') {
			lst = ptr;
		}
		ptr++;
	}
	if (!lst)lst = fn;
	//compute the hash value
	ptr = lst + 1;
	unsigned v = HASH_FNV_offset_basis;
	while (*ptr) {
		v = (v ^ (unsigned)*ptr++) * HASH_FNV_prime;
	}
	//return v;
	return (v & 0xffff) ^ (v >> 16);
}
#endif
#if 0
constexpr unsigned dbgFileNameHash(const char* fn) {
#define HASH_FNV_offset_basis 2166136261U//0x811c9dc5
#define HASH_FNV_prime 16777619U//0x01000193
	const char* ptr = fn;
	//const char* lst = 0;
	//get filename without path
	//while (*ptr) {
	//	if (*ptr == '/' || *ptr == '\\') {
	//		lst = ptr;
	//	}
	//	ptr++;
	//}
	//if (!lst)lst = fn;
	//compute the hash value
	//ptr = lst + 1;
	unsigned v = HASH_FNV_offset_basis;
	while (*ptr) {
		v = (v ^ (unsigned)*ptr++) * HASH_FNV_prime;
	}
	//return v;
	return (v & 0xffff) ^ (v >> 16);
}
#endif
