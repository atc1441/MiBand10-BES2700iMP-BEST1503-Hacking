#ifndef _GFPS_CRYPTO_H_
#define _GFPS_CRYPTO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define Gfps_CheckParm(code, exp) if (!(exp)) return (code)

enum {
    GFPS_SUCCESS,
    GFPS_ERROR_EXEC_FAIL,
    GFPS_ERROR_NULL,
    GFPS_ERROR_INVALID_PARAM,
    GFPS_ERROR_DATA_SIZE,

    GFPS_ERROR_UNAUTHENTICATED = 0x80,
    GFPS_ERROR_INVALID_VALUE = 0x81,
    GFPS_ERROR_NO_USER_CONSENT = 0x82
};

void gfps_crypto_init(void);

uint32_t gfps_crypto_get_secret_decrypt(const uint8_t* in_encryptdata ,const uint8_t *in_public_key,uint8_t * out_key,uint8_t *out_decryptdata );
uint32_t gfps_crypto_encrypt(const uint8_t *in_data,uint8_t len,const uint8_t *AESkey,uint8_t *out_encrypt);
uint32_t gfps_crypto_decrypt(const uint8_t *in_data,uint8_t len,const uint8_t *AESkey,uint8_t *out_encrypt);
uint32_t gfps_crypto_gen_DHKey(const uint8_t *in_PubKey,const uint8_t *in_PrivateKey,uint8_t *out_DHKey);

uint32_t gfps_crypto_make_P256_key(uint8_t * out_public_key,uint8_t * out_private_key);
uint32_t gfps_crypto_set_p256_key(const uint8_t* in_public_key,const uint8_t* in_private_key);
uint32_t gfps_SHA256_hash(const void* in_data, int len, void* out_data);
void gfps_encrypt_name(uint8_t* aesKey, uint8_t* inputRawName, uint32_t inputLen, 
    uint8_t* outputEncryptedName, uint8_t* hmacFirst8Bytes, uint8_t* nonce);
bool gfps_decrypt_name(uint8_t* aesKey, uint8_t* hmacFirst8Bytes, 
    uint8_t* nonce, uint8_t* encryptedName, uint8_t* rawName, uint32_t nameLen);

int rand_generator(uint8_t *dest, unsigned size);
void gfps_encrypt_messasge(uint8_t* accKey, uint8_t* nonce,
    uint8_t* inputData, uint32_t inputDataLen, uint8_t* outputHmacFirst8Bytes);
void gfps_hdkf(uint8_t *salt, uint8_t saltLen, uint8_t* ikm, uint8_t ikmLen,
    uint8_t*inputData, uint32_t inputDataLen, uint8_t* output, uint8_t outputLen);
void gfps_beacon_encrpt_data(uint8_t* accKey,
    uint8_t* inputData, uint32_t inputDataLen, uint8_t* outputHmacFirst8Bytes);

#ifdef __cplusplus
}
#endif

#endif

