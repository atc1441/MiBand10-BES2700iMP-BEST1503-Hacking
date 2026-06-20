// Copyright 2020 BES Technic
//
// Description: Bias functions.
//

// #define XSTR(x) STR(x)
// #define STR(x) #x
// #pragma message "The value of ABC: " XSTR(__cplusplus)

#include "beco.h"
#include "beco_rotate.h"
#include "beco_bias.hpp"

template <typename bias_t>
void beco_add_bias_8x8(const bias_t *bias, p7_t q)
{
    uint32_t config;
    uint32_t a0, a1, a2, a3;
    const uint32_t *p_bias = (const uint32_t *)bias;

    // Save current config:
    config = beco_config();

    beco_write_config(
                  BECO_CONF_ATYPE_UINT8
                | (config & BECO_CONF_BTYPE_MASK)
                | BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 );

    a0 = p_bias[0]; // stride?
    a1 = p_bias[1];
    a2 = p_bias[2];
    a3 = p_bias[3];

    if (config & BECO_CONF_LOCALROTATE) {
        BECO_ROTX(a0,a1, 0x00ff00ff,  8);
        BECO_ROTX(a2,a3, 0x00ff00ff,  8);
    }
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a2}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u8 = {q, 0}}), BECO_REG2);

    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a1, a3}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u8 = {0, q}}), BECO_REG2);

    // Restore original config:
    beco_write_config(config);
}


template <typename bias_t>
void beco_add_bias_8x16(const bias_t *bias, p7_t q)
{
    uint32_t config;
    uint32_t a0, a1, a2, a3;
    const uint32_t *p_bias = (const uint32_t *)bias;

    // Save current config:
    config = beco_config();

    beco_write_config(
                  BECO_CONF_ATYPE_UINT8
                | (config & BECO_CONF_BTYPE_MASK)
                | BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 );

    a0 = p_bias[0]; // stride?
    a1 = p_bias[1];
    a2 = p_bias[2];
    a3 = p_bias[3];
    BECO_ROT(a0,a1, 0x0000ffff, 16);
    BECO_ROT(a2,a3, 0x0000ffff, 16);

    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a2}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u8 = {q, 0}}), BECO_REG2);

    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a1, a3}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u8 = {0, q}}), BECO_REG2);

    // Restore original config:
    beco_write_config(config);
}


template <typename bias_t>
void beco_add_bias_16x8(const bias_t *bias, p7_t q)
{
    uint32_t config;
    uint32_t a0, a1, a2, a3;
    const uint32_t *p_bias = (const uint32_t *)bias;

    // Save current config:
    config = beco_config();

    beco_write_config(
                 (config & BECO_CONF_ATYPE_MASK)
                | BECO_CONF_BTYPE_UINT8
                | BECO_CONF_AMODE_REP32 | BECO_CONF_BMODE_REP32 );

    a0 = p_bias[0]; // stride?
    a1 = p_bias[1];
    a2 = p_bias[2];
    a3 = p_bias[3];

    //beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));

    beco_mmac(BECO_ACC0, ((beco_vec32_in_t){.u32 = a0}), ((beco_vec32_in_t){.u8 = {q, 0, 0, 0}}));
    beco_mmac(BECO_ACC0, ((beco_vec32_in_t){.u32 = a1}), ((beco_vec32_in_t){.u8 = {0, q, 0, 0}}));
    beco_mmac(BECO_ACC0, ((beco_vec32_in_t){.u32 = a2}), ((beco_vec32_in_t){.u8 = {0, 0, q, 0}}));
    beco_mmac(BECO_ACC0, ((beco_vec32_in_t){.u32 = a3}), ((beco_vec32_in_t){.u8 = {0, 0, 0, q}}));
//beco_mmacrr(
    // Restore original config:
    beco_write_config(config);
}


template <typename bias_t>
void beco_add_bias_16x16(const bias_t *bias, p15_t q)
{
    uint32_t config;
    uint32_t a0, a1;
    const uint32_t *p_bias = (const uint32_t *)bias;

    // Save current config:
    config = beco_config();

    beco_write_config(
                  BECO_CONF_ATYPE_UINT16
                | (config & BECO_CONF_BTYPE_MASK)
                | BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 );

    a0 = p_bias[0]; // stride?
    a1 = p_bias[1];
    
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u16 = {q, 0}}), BECO_REG2);

    // Restore original config:
    beco_write_config(config);
}

template <typename bias_t>
void beco_add_bias_4x4_16x16(const bias_t *bias, p15_t q)
{
    uint32_t config;
    uint32_t a0, a1;
    const uint32_t *p_bias = (const uint32_t *)bias;

    // Save current config:
    config = beco_config();

    beco_write_config(
                  BECO_CONF_ATYPE_UINT16
                | (config & BECO_CONF_BTYPE_MASK)
                | BECO_CONF_AMODE_REP16 | BECO_CONF_BMODE_REP64 );

    a0 = p_bias[0]; // stride?
    a1 = p_bias[1];
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));
    beco_mmacgr(BECO_ACC0, ((beco_vec32_in_t){.u16 = {q, 0}}), BECO_REG2);

    a0 = p_bias[2]; // stride?
    a1 = p_bias[3];
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));
    beco_mmacgr(BECO_ACC1, ((beco_vec32_in_t){.u16 = {q, 0}}), BECO_REG2);
    
    a0 = p_bias[4]; // stride?
    a1 = p_bias[5];
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));
    beco_mmacgr(BECO_ACC2, ((beco_vec32_in_t){.u16 = {q, 0}}), BECO_REG2);

    a0 = p_bias[6]; // stride?
    a1 = p_bias[7];
    beco_write_reg(BECO_REG2, ((beco_vec64_in_t){.u32 = {a0, a1}}));
    beco_mmacgr(BECO_ACC3, ((beco_vec32_in_t){.u16 = {q, 0}}), BECO_REG2);

    // Restore original config:
    beco_write_config(config);
}


template void beco_add_bias_8x8(const int8_t *bias, p7_t q);
template void beco_add_bias_8x8(const uint8_t *bias, p7_t q);
template void beco_add_bias_8x16<int16_t>(const int16_t *bias, p7_t q);
template void beco_add_bias_8x16<uint16_t>(const uint16_t *bias, p7_t q);
template void beco_add_bias_16x8<int16_t>(const int16_t *bias, p7_t q);
template void beco_add_bias_16x8<uint16_t>(const uint16_t *bias, p7_t q);
template void beco_add_bias_16x16<int16_t>(const int16_t *bias, p15_t q);
template void beco_add_bias_16x16<uint16_t>(const uint16_t *bias, p15_t q);

template void beco_add_bias_4x4_16x16<int16_t>(const int16_t *bias, p15_t q);
template void beco_add_bias_4x4_16x16<uint16_t>(const uint16_t *bias, p15_t q);

