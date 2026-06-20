#include "utils.h"
#include "aes.h"
#include "cmac.h"

#define PASSORFAIL "\033[1;32mpassed\033[0m" : "\033[1;31mfailed\033[0m"

void print_bytes(unsigned char* buf, const size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02x ", buf[i]);
    }
    printf("\n");
}

void block_xor(unsigned char* dst, unsigned char* a, const unsigned char* b)
{
    for (unsigned char j = 0; j < 16; j++) {
        dst[j] = a[j] ^ b[j];
    }
}

void block_leftshift(unsigned char* dst,const unsigned char* src)
{
    unsigned char ovf = 0x00;
    for (int i = 15; i >= 0; i--) {
        dst[i] = src[i] << 1;
        dst[i] |= ovf;
        ovf = (src[i] & 0x80) ? 1 : 0;
    }
}

bool aes_test(void)
{
    // Known Plain Text and Cipher Text
    unsigned char key[] = {
        0x00, 0x01, 0x02, 0x03,
        0x04, 0x05, 0x06, 0x07,
        0x08, 0x09, 0x0a, 0x0b,
        0x0c, 0x0d, 0x0e, 0x0f,
        0x10, 0x11, 0x12, 0x13,
        0x14, 0x15, 0x16, 0x17,
        0x18, 0x19, 0x1a, 0x1b,
        0x1c, 0x1d, 0x1e, 0x1f
    };

    unsigned char in[] = {
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb,
        0xcc, 0xdd, 0xee, 0xff
    };

    unsigned char aes_128_out[] = {
        0x69, 0xc4, 0xe0, 0xd8,
        0x6a, 0x7b, 0x04, 0x30,
        0xd8, 0xcd, 0xb7, 0x80,
        0x70, 0xb4, 0xc5, 0x5a
    };

    unsigned char aes_192_out[] = {
        0xdd, 0xa9, 0x7c, 0xa4,
        0x86, 0x4c, 0xdf, 0xe0,
        0x6e, 0xaf, 0x70, 0xa0,
        0xec, 0x0d, 0x71, 0x91
    };

    unsigned char aes_256_out[] = {
        0x8e, 0xa2, 0xb7, 0xca,
        0x51, 0x67, 0x45, 0xbf,
        0xea, 0xfc, 0x49, 0x90,
        0x4b, 0x49, 0x60, 0x89
    };

    unsigned char aes_cmac_out[] = {
        0x38, 0x7b, 0x36, 0x22,
        0x8b, 0xa7, 0x77, 0x44,
        0x5b, 0xaf, 0xa0, 0x36,
        0x45, 0xb9, 0x40, 0x10
    };

    // Calculate the aes-128 result
    unsigned char out_128[16], out_192[16], out_256[16];
    unsigned char in_128[16], in_192[16], in_256[16];
    aes_128_encrypt(in, out_128, key);
    aes_192_encrypt(in, out_192, key);
    aes_256_encrypt(in, out_256, key);
    aes_128_decrypt(out_128, in_128, key);
    aes_192_decrypt(out_192, in_192, key);
    aes_256_decrypt(out_256, in_256, key);

    // Calculate the cmac result
    unsigned char cmac_out[16];
    aes_cal_cmac(in, 16, cmac_out, key);

    // Verify the AES output
    bool aes_128_de = !strncmp((char*)in_128, (char*)in, 16);
    bool aes_192_de = !strncmp((char*)in_192, (char*)in, 16);
    bool aes_256_de = !strncmp((char*)in_256, (char*)in, 16);
    bool aes_128_en = !strncmp((char*)out_128, (char*)aes_128_out, 16);
    bool aes_192_en = !strncmp((char*)out_192, (char*)aes_192_out, 16);
    bool aes_256_en = !strncmp((char*)out_256, (char*)aes_256_out, 16);
    bool aes_mac_co = !strncmp((char*)cmac_out, (char*)aes_cmac_out, 16);

    // Verify the aes-cmac output
    bool cmac_correct = !strncmp((char*)cmac_out, (char*)aes_cmac_out, 16);

    // output
    printf("AES-128 encrypt test %s.\n", aes_128_en ? PASSORFAIL);
    printf("AES-128 decrypt test %s.\n", aes_128_de ? PASSORFAIL);
    printf("AES-192 encrypt test %s.\n", aes_192_en ? PASSORFAIL);
    printf("AES-192 decrypt test %s.\n", aes_192_de ? PASSORFAIL);
    printf("AES-256 encrypt test %s.\n", aes_256_en ? PASSORFAIL);
    printf("AES-256 decrypt test %s.\n", aes_256_de ? PASSORFAIL);
    printf("AES-CMAC test %s.\n", aes_mac_co ? PASSORFAIL);

    return cmac_correct && aes_128_en && aes_128_de && aes_192_en && aes_192_de && aes_256_en && aes_256_de;
}
