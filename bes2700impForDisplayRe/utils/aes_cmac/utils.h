#include <string.h>
#include <stdbool.h>
// Print functions

void print_bytes(unsigned char* buf, const size_t len);
void block_xor(unsigned char* dst, unsigned char* a,const unsigned char* b);
void block_leftshift(unsigned char* dst, const unsigned char* src);

bool aes_test();
