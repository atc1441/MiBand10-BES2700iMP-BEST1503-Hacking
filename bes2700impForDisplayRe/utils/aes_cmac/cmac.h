#ifndef CMAC_H
#define CMAC_H

#define const_Bsize 16

void aes_cal_cmac(const unsigned char* in, unsigned int length, unsigned char* out, const unsigned char* key);
//bool verify_mac(unsigned char* in, unsigned int length, unsigned char* out, unsigned char* key);

void GenerateSubkey(const unsigned char* key, unsigned char* K1, unsigned char* K2);

#endif // !CMAC_H
