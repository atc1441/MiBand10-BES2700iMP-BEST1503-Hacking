/**
 ****************************************************************************************
 *
 * @file ecc_p192.h
 *
 * @brief  ECC functions for P192
 *
 * Copyright (C) RivieraWaves 2009-2015
 *
 ****************************************************************************************
 */

#ifndef ECC_P192_H_
#define ECC_P192_H_


/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>
#ifdef __cplusplus
extern "C" {
#endif

/*
 * DEFINES
 ****************************************************************************************
 */


/*
 * STRUCTURE DEFINITIONS
 ****************************************************************************************
 */

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */

void ecc_gen_new_secret_key_192(uint8_t* secret_key192);
void ecc_gen_new_public_key_192(uint8_t* secret_key,uint8_t* out_public_key);

#ifdef __cplusplus
}
#endif


#endif /* ECC_P192_H_ */
