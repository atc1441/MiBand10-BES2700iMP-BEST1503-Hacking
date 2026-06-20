#ifndef ENCRYPT_H

#include "aes.h"

unsigned char* ecb_encrypt(unsigned char* in, unsigned char* key, unsigned char* (*some_encrypt)(unsigned char* in, unsigned char* out, unsigned char* key), unsigned int* n);

unsigned char* ecb_decrypt(unsigned char* in, unsigned char* key, unsigned char* (*some_decrypt)(unsigned char* in, unsigned char* out, unsigned char* key), unsigned int* n);

#endif // !ENCRYPT_H