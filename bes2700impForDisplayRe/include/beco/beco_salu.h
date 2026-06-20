// Instructions with BECO-REG + BECO-REG

// Function: beco_salu_zero       - Type: bits        - Operation: ZERO(Clear all bits)
// Beco Logic ZERO operation
//
// Description:
//
// The beco_salu_zero instruction clear all bits and store the result
// in the destination register, rd.
//
// Operation:
//
// beco_salu_zero(rd):
//     rd = 0
//
// Where:
//     rd - destination Beco scalar register
//
BECO_INLINE void beco_salu_zero(
    beco_reg_t       rd)
{
    BECO_CDP2(OPC_SALU_LZERO, rd, 0, 0);
}

// Function: beco_salu_and        - Type: bits        - Operation: AND(Logical AND)
// Beco Logic AND operation
//
// Description:
//
// The beco_salu_and instruction perform logical AND between source
// registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_and(rd, rs, rt):
//     rd = rs & rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_and(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LAND, rd, rs, rt);
}

// Function: beco_salu_nand       - Type: bits        - Operation: NAND(Logical NAND)
// Beco Logic NAND operation
//
// Description:
//
// The beco_salu_nand instruction perform logical NAND between source
// registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_nand(rd, rs, rt):
//     rd = ~(rs & rt)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_nand(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LNAND, rd, rs, rt);
}

// Function: beco_salu_andn       - Type: bits        - Operation: ANDN(Logical ANDN (MASK))
// Beco Logic ANDN operation
//
// Description:
//
// The beco_salu_andn instruction perform logical ANDN (MASK) between
// source registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_andn(rd, rs, rt):
//     rd = ~rs & rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_andn(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LANDN, rd, rs, rt);
}

// Function: beco_salu_xor        - Type: bits        - Operation: XOR(Logical XOR)
// Beco Logic XOR operation
//
// Description:
//
// The beco_salu_xor instruction perform logical XOR between source
// registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_xor(rd, rs, rt):
//     rd = rs ^ rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_xor(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LXOR, rd, rs, rt);
}

// Function: beco_salu_xnor       - Type: bits        - Operation: XNOR(Logical XNOR)
// Beco Logic XNOR operation
//
// Description:
//
// The beco_salu_xnor instruction perform logical XNOR between source
// registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_xnor(rd, rs, rt):
//     rd = ~(rs ^ rt)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_xnor(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LXNOR, rd, rs, rt);
}

// Function: beco_salu_or         - Type: bits        - Operation: OR(Logical OR)
// Beco Logic OR operation
//
// Description:
//
// The beco_salu_or instruction perform logical OR between source registers
// and save result in the destination register.
//
//
// Operation:
//
// beco_salu_or(rd, rs, rt):
//     rd = rs | rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_or(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LOR, rd, rs, rt);
}

// Function: beco_salu_nor        - Type: bits        - Operation: NOR(Logical NOR)
// Beco Logic NOR operation
//
// Description:
//
// The beco_salu_nor instruction perform logical NOR between source
// registers and save result in the destination register.
//
//
// Operation:
//
// beco_salu_nor(rd, rs, rt):
//     rd = ~(rs | rt)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_nor(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LNOR, rd, rs, rt);
}

// Function: beco_salu_not        - Type: bits        - Operation: NOT(Logical NOT)
// Beco Logic NOT operation
//
// Description:
//
// The beco_salu_not instruction perform logical NOT of the (first)
// source register and store the result in the destination register,
// rd.
//
// Operation:
//
// beco_salu_not(rd, rs):
//     rd = ~rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_not(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_LNOT, rd, rs, 0);
}

// Function: beco_salu_ones       - Type: bits        - Operation: ONES(Set all bits)
// Beco Logic ONES operation
//
// Description:
//
// The beco_salu_ones instruction set all bits and store the result
// in the destination register, rd.
//
// Operation:
//
// beco_salu_ones(rd):
//     rd = ~0
//
// Where:
//     rd - destination Beco scalar register
//
BECO_INLINE void beco_salu_ones(
    beco_reg_t       rd)
{
    BECO_CDP2(OPC_SALU_LONES, rd, 0, 0);
}

// Function: beco_salu_add_32     - Type: int2x32_t   - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 32bit elements
//
// Description:
//
// The beco_salu_add_32 instruction operate on the source operands as
// vectors of 32 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_32(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_add_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ADD(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_add_16     - Type: int4x16_t   - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 16bit elements
//
// Description:
//
// The beco_salu_add_16 instruction operate on the source operands as
// vectors of 16 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_16(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_add_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ADD(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_add_8      - Type: int8x8_t    - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 8bit elements
//
// Description:
//
// The beco_salu_add_8 instruction operate on the source operands as
// vectors of 8 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_8(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_add_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ADD(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_sub_32     - Type: int2x32_t   - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 32bit elements
//
// Description:
//
// The beco_salu_sub_32 instruction operate on the source operands as
// vectors of 32 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_32(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_sub_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SUB(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_sub_16     - Type: int4x16_t   - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 16bit elements
//
// Description:
//
// The beco_salu_sub_16 instruction operate on the source operands as
// vectors of 16 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_16(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_sub_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SUB(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_sub_8      - Type: int8x8_t    - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 8bit elements
//
// Description:
//
// The beco_salu_sub_8 instruction operate on the source operands as
// vectors of 8 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_8(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_sub_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SUB(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_rsub_32    - Type: int2x32_t   - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 32bit elements
//
// Description:
//
// The beco_salu_rsub_32 instruction operate on the source operands
// as vectors of 32 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_32(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_rsub_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_RSUB(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_rsub_16    - Type: int4x16_t   - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 16bit elements
//
// Description:
//
// The beco_salu_rsub_16 instruction operate on the source operands
// as vectors of 16 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_16(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_rsub_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_RSUB(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_rsub_8     - Type: int8x8_t    - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 8bit elements
//
// Description:
//
// The beco_salu_rsub_8 instruction operate on the source operands as
// vectors of 8 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_8(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_rsub_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_RSUB(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_gabs_32    - Type: int2x32_t   - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 32bit elements
//
// Description:
//
// The beco_salu_gabs_32 instruction operate on the source operands
// as vectors of 32 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_32(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_gabs_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ABS(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_gabs_16    - Type: int4x16_t   - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 16bit elements
//
// Description:
//
// The beco_salu_gabs_16 instruction operate on the source operands
// as vectors of 16 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_16(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_gabs_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ABS(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_gabs_8     - Type: int8x8_t    - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 8bit elements
//
// Description:
//
// The beco_salu_gabs_8 instruction operate on the source operands as
// vectors of 8 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_8(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_gabs_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ABS(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_mask_32    - Type: int2x32_t   - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 32bit elements
//
// Description:
//
// The beco_salu_mask_32 instruction generate bit mask from size. If
// the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_32(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={0, 1, 2, 3, -4, -5, -6, -7}};
//     beco_write_reg(BECO_REG0, A);
//
//     beco_salu_mask_8(BECO_REG2, BECO_REG0);
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//
// /* Result in v[]:
//     mask_vec64 = 0103070ff8fcfeff
//     mask(0) = ff
//     mask(1) = fe
//     mask(2) = fc
//     mask(3) = f8
//     mask(-4) = 0f
//     mask(-5) = 07
//     mask(-6) = 03
//     mask(-7) = 01
// */
//
//
BECO_INLINE void beco_salu_mask_32(
    beco_reg_t       rd,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MASK(VECSZ32), rd, 0, rt);
}

// Function: beco_salu_mask_16    - Type: int4x16_t   - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 16bit elements
//
// Description:
//
// The beco_salu_mask_16 instruction generate bit mask from size. If
// the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_16(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_mask_16(
    beco_reg_t       rd,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MASK(VECSZ16), rd, 0, rt);
}

// Function: beco_salu_mask_8     - Type: int8x8_t    - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 8bit elements
//
// Description:
//
// The beco_salu_mask_8 instruction generate bit mask from size. If
// the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_8(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_mask_8(
    beco_reg_t       rd,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MASK(VECSZ8), rd, 0, rt);
}

// Function: beco_salu_clipz_32   - Type: int2x32_t   - Operation: CLIPZ(Signed Saturate to positive N bits)
// Beco SIMD CLIPZ operation for 32bit elements
//
// Description:
//
// The beco_salu_clipz_32 instruction perform signed saturate to positive
// N bits on each lane of 32 bits in the source register and save result
// in the destination register.
// (also known as ReLU operation).
//
//
// Operation:
//
// beco_salu_clipz_32(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = 0,                    when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={0,   0,  2,  2,  4,  4,  6,  6}};
//     beco_vec64_in_t B = {.i8={10,-10, 20,-20, 40,-40, 60,-60}};
//
//     beco_write_reg(BECO_REG0, A);
//     beco_write_reg(BECO_REG1, B);
//
//     beco_salu_clipz_8(BECO_REG2, BECO_REG1, BECO_REG0);
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//
// /* Result in v[]:
//     clipz(0, 10) = 0
//     clipz(0, -10) = 0
//     clipz(2, 20) = 3
//     clipz(2, -20) = 0
//     clipz(4, 40) = 15
//     clipz(4, -40) = 0
//     clipz(6, 60) = 60
//     clipz(6, -60) = 0
//
//
BECO_INLINE void beco_salu_clipz_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPZ(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_clipz_16   - Type: int4x16_t   - Operation: CLIPZ(Signed Saturate to positive N bits)
// Beco SIMD CLIPZ operation for 16bit elements
//
// Description:
//
// The beco_salu_clipz_16 instruction perform signed saturate to positive
// N bits on each lane of 16 bits in the source register and save result
// in the destination register.
// (also known as ReLU operation).
//
//
// Operation:
//
// beco_salu_clipz_16(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = 0,                    when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clipz_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPZ(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_clipz_8    - Type: int8x8_t    - Operation: CLIPZ(Signed Saturate to positive N bits)
// Beco SIMD CLIPZ operation for 8bit elements
//
// Description:
//
// The beco_salu_clipz_8 instruction perform signed saturate to positive
// N bits on each lane of 8 bits in the source register and save result
// in the destination register.
// (also known as ReLU operation).
//
//
// Operation:
//
// beco_salu_clipz_8(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = 0,                    when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clipz_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPZ(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_clip_32    - Type: int2x32_t   - Operation: CLIP(Signed saturate to N bits)
// Beco SIMD CLIP operation for 32bit elements
//
// Description:
//
// The beco_salu_clip_32 instruction perform signed saturate to N bits
// on each lane of 32 bits in the source register and save result in
// the destination register.
//
// Operation:
//
// beco_salu_clip_32(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = max(rs, ~max),   when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={0,   0,  2,  2,  4,  4,  6,  6}};
//     beco_vec64_in_t B = {.i8={10,-10, 20,-20, 40,-40, 60,-60}};
//
//     beco_write_reg(BECO_REG0, A);
//     beco_write_reg(BECO_REG1, B);
//
//     beco_salu_clip_8(BECO_REG2, BECO_REG1, BECO_REG0);
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//
// /* Result in v[]:
//     clip(0, 10) = 0
//     clip(0, -10) = -1
//     clip(2, 20) = 3
//     clip(2, -20) = -4
//     clip(4, 40) = 15
//     clip(4, -40) = -16
//     clip(6, 60) = 60
//     clip(6, -60) = -60 */
//
//
BECO_INLINE void beco_salu_clip_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIP(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_clip_16    - Type: int4x16_t   - Operation: CLIP(Signed saturate to N bits)
// Beco SIMD CLIP operation for 16bit elements
//
// Description:
//
// The beco_salu_clip_16 instruction perform signed saturate to N bits
// on each lane of 16 bits in the source register and save result in
// the destination register.
//
// Operation:
//
// beco_salu_clip_16(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = max(rs, ~max),   when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clip_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIP(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_clip_8     - Type: int8x8_t    - Operation: CLIP(Signed saturate to N bits)
// Beco SIMD CLIP operation for 8bit elements
//
// Description:
//
// The beco_salu_clip_8 instruction perform signed saturate to N bits
// on each lane of 8 bits in the source register and save result in
// the destination register.
//
// Operation:
//
// beco_salu_clip_8(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = max(rs, ~max),   when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clip_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIP(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_clipu_32   - Type: int2x32_t   - Operation: CLIPU(Unsigned Saturate to N bits)
// Beco SIMD CLIPU operation for 32bit elements
//
// Description:
//
// The beco_salu_clipu_32 instruction perform unsigned saturate to N
// bits on each lane of 32 bits in the source register and save result
// in the destination register.
//
// Operation:
//
// beco_salu_clipu_32(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={0,1,2,3,4,5,6,8}};
//     beco_vec64_in_t B = {.u8={1,20,40, 60, 80, 0xa0, 55, 0xfe}};
//
//     beco_write_reg(BECO_REG0, A);
//     beco_write_reg(BECO_REG1, B);
//
//     beco_salu_clipu_8(BECO_REG2, BECO_REG1, BECO_REG0);
//
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//
// /* Result in v[]:
//     clipu(0, 1) = 0
//     clipu(1, 20) = 1
//     clipu(2, 40) = 3
//     clipu(3, 60) = 7
//     clipu(4, 80) = 15
//     clipu(5, 160) = 31
//     clipu(6, 55) = 55
//     clipu(7, 254) = 127  */
//
//
BECO_INLINE void beco_salu_clipu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPU(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_clipu_16   - Type: int4x16_t   - Operation: CLIPU(Unsigned Saturate to N bits)
// Beco SIMD CLIPU operation for 16bit elements
//
// Description:
//
// The beco_salu_clipu_16 instruction perform unsigned saturate to N
// bits on each lane of 16 bits in the source register and save result
// in the destination register.
//
// Operation:
//
// beco_salu_clipu_16(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clipu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPU(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_clipu_8    - Type: int8x8_t    - Operation: CLIPU(Unsigned Saturate to N bits)
// Beco SIMD CLIPU operation for 8bit elements
//
// Description:
//
// The beco_salu_clipu_8 instruction perform unsigned saturate to N
// bits on each lane of 8 bits in the source register and save result
// in the destination register.
//
// Operation:
//
// beco_salu_clipu_8(rd, rs, rt):
//     for each lane:
//         max = (1 << rt) - 1
//         val = min(rs, max)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_clipu_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_CLIPU(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_min_32     - Type: int2x32_t   - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 32bit elements
//
// Description:
//
// The beco_salu_min_32 instruction operate on the source operands as
// vectors of 32 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_32(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_min_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MIN(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_min_16     - Type: int4x16_t   - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 16bit elements
//
// Description:
//
// The beco_salu_min_16 instruction operate on the source operands as
// vectors of 16 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_16(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_min_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MIN(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_min_8      - Type: int8x8_t    - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 8bit elements
//
// Description:
//
// The beco_salu_min_8 instruction operate on the source operands as
// vectors of 8 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_8(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_min_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MIN(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_minu_32    - Type: int2x32_t   - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 32bit elements
//
// Description:
//
// The beco_salu_minu_32 instruction operate on the source operands
// as vectors of 32 bit elements, unsigned MIN is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_minu_32(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_minu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MINU(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_minu_16    - Type: int4x16_t   - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 16bit elements
//
// Description:
//
// The beco_salu_minu_16 instruction operate on the source operands
// as vectors of 16 bit elements, unsigned MIN is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_minu_16(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_minu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MINU(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_minu_8     - Type: int8x8_t    - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 8bit elements
//
// Description:
//
// The beco_salu_minu_8 instruction operate on the source operands as
// vectors of 8 bit elements, unsigned MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_minu_8(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_minu_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MINU(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_max_32     - Type: int2x32_t   - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 32bit elements
//
// Description:
//
// The beco_salu_max_32 instruction operate on the source operands as
// vectors of 32 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_32(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_max_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAX(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_max_16     - Type: int4x16_t   - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 16bit elements
//
// Description:
//
// The beco_salu_max_16 instruction operate on the source operands as
// vectors of 16 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_16(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_max_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAX(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_max_8      - Type: int8x8_t    - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 8bit elements
//
// Description:
//
// The beco_salu_max_8 instruction operate on the source operands as
// vectors of 8 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_8(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_max_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAX(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_maxu_32    - Type: int2x32_t   - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 32bit elements
//
// Description:
//
// The beco_salu_maxu_32 instruction operate on the source operands
// as vectors of 32 bit elements, unsigned MAX is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_32(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_maxu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAXU(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_maxu_16    - Type: int4x16_t   - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 16bit elements
//
// Description:
//
// The beco_salu_maxu_16 instruction operate on the source operands
// as vectors of 16 bit elements, unsigned MAX is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_16(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_maxu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAXU(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_maxu_8     - Type: int8x8_t    - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 8bit elements
//
// Description:
//
// The beco_salu_maxu_8 instruction operate on the source operands as
// vectors of 8 bit elements, unsigned MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_8(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_maxu_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_MAXU(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_sextb_32   - Type: int2x32_t   - Operation: SEXT(Sign-extend low 8 bit to 32 bit)
// Beco SIMD SEXT operation for 32bit elements
//
// Description:
//
// The beco_salu_sextb_32 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements sign-extend low 8 bit to 32 bit and store the result in
// destination register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_ashl_32_g(rd, rs, -24)` to extend top 8 bit part of
// the input to 32 bit. To select and extend the two middle 8 bit elements
// require two instructions, a shift and a beco_salu_sextb_32.
//
// Operation:
//
// beco_salu_sextb_32(rd, rs):
//     for each lane (= 2):
//         rd = (int8_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_sextb_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SEXT(VECSZ32), rd, rs, 0);
}

// Function: beco_salu_sexth_32   - Type: int4x16_t   - Operation: SEXT(Sign-extend low 16 bit to 32 bit)
// Beco SIMD SEXT operation for 16bit elements
//
// Description:
//
// The beco_salu_sexth_32 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements sign-extend low 16 bit to 32 bit and store the result in
// destination register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_ashl_32_g(rd, rs, -16)` to extend other part of the
// input to 32 bit.
//
// Operation:
//
// beco_salu_sexth_32(rd, rs):
//     for each lane (= 2):
//         rd = (int16_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_sexth_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SEXT(VECSZ16), rd, rs, 0);
}

// Function: beco_salu_sextb_16   - Type: int8x8_t    - Operation: SEXT(Sign-extend low 8 bit to 16 bit)
// Beco SIMD SEXT operation for 8bit elements
//
// Description:
//
// The beco_salu_sextb_16 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements sign-extend low 8 bit to 16 bit and store the result in
// destination register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_ashl_16_g(rd, rs, -8)` to extend the other part of
// the input to 16 bit.
//
// Operation:
//
// beco_salu_sextb_16(rd, rs):
//     for each lane (= 4):
//         rd = (int8_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_sextb_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SEXT(VECSZ8), rd, rs, 0);
}

// Function: beco_salu_zextb_32   - Type: int2x32_t   - Operation: ZEXT(Zero-extend low 8 bit to 32 bit)
// Beco SIMD ZEXT operation for 32bit elements
//
// Description:
//
// The beco_salu_zextb_32 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements zero-extend low 8 bit to 32 bit and store the result in
// destination register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_shl_32_g(rd, rs, -24)` to extend top 8 bit to 32 bit.
// To select and extend the other 8 bit elements require a shift and
// a beco_salu_zextb_32.
//
// Operation:
//
// beco_salu_zextb_32(rd, rs):
//     for each lane (= 2):
//         rd = (uint8_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
// Example:
//
//     // REG0.u8 = {1,2,3,4,5,6,7,8}
//     for (int i = 0; i < 32; i += 8) {
//         beco_salu_shl_32_g(BECO_REG2, BECO_REG0, -i);
//         beco_salu_sextb_32(BECO_REG2,  BECO_REG2);
//         //  i  REG2.u32[1]  REG2.u32[0]
//         //  0  0x00000004  0x00000001
//         //  8  0x00000005  0x00000002
//         // 16  0x00000006  0x00000003
//         // 24  0x00000007  0x00000004
//     }
//
//
BECO_INLINE void beco_salu_zextb_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_ZEXT(VECSZ32), rd, rs, 0);
}

// Function: beco_salu_zexth_32   - Type: int4x16_t   - Operation: ZEXT(Zero-extend low 16 bit to 32 bit)
// Beco SIMD ZEXT operation for 16bit elements
//
// Description:
//
// The beco_salu_zexth_32 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements zero-extend low 16 bit to 32 bit and store the result in
// destination register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_shl_32_g(rd, rs, -16)` to extend the other part of
// the input to 32 bit
//
// Operation:
//
// beco_salu_zexth_32(rd, rs):
//     for each lane (= 2):
//         rd = (uint32_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
// Example:
//
//     // Sign extend vector of 8 bit to 16 bits
//     beco_vec64_in_t B = {.u8={254,127,128,64,32,16,8,4}};
//     beco_write_reg(BECO_REG0, B);
//
//     beco_salu_sextb_16(BECO_REG1,  BECO_REG0);
//     beco_bshift(BECO_REG2, BECO_REG0, BECO_REG0, 1);
//     beco_salu_sextb_16(BECO_REG2,  BECO_REG2);
//
//     beco_vec64_out_t veven = beco_read_reg(BECO_REG1);
//     beco_vec64_out_t vodd = beco_read_reg(BECO_REG2);
//
//     for (int i = 0; i < 8/2; i++)
//         printf("sext8->16({x%02x,x%02x}) = {x%04x,x%04x}\n",
//             B.u8[i*2+0], B.u8[i*2+1],
//             veven.u16[i], vodd.u16[i]);
// /*  Result in veven[], vodd[]:
//     sext8->16({xfe,x7f}) = {xfffe,x007f}
//     sext8->16({x80,x40}) = {xff80,x0040}
//     sext8->16({x20,x10}) = {x0020,x0010}
//     sext8->16({x08,x04}) = {x0008,x0004}  */
//
//
BECO_INLINE void beco_salu_zexth_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_ZEXT(VECSZ16), rd, rs, 0);
}

// Function: beco_salu_zextb_16   - Type: int8x8_t    - Operation: ZEXT(Zero-extend 8 bit to 16 bit)
// Beco SIMD ZEXT operation for 8bit elements
//
// Description:
//
// The beco_salu_zextb_16 instruction treat the source operand, rs,
// as a vector of the destination sized elements, for each of these
// elements zero-extend 8 bit to 16 bit and store the result in destination
// register rd. The high bits of the source is ignored.
//
// Note:
//
// Use `beco_salu_shl_16_g(rd, rs, -8)` to extend other part of the
// input to 16 bit
//
// Operation:
//
// beco_salu_zextb_16(rd, rs):
//     for each lane (= 4):
//         rd = (uint8_t)rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_zextb_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_ZEXT(VECSZ8), rd, rs, 0);
}


// Compare elements to mask

// Function: beco_salu_lt_32      - Type: int2x32_t   - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 32bit elements
//
// Description:
//
// The beco_salu_lt_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_32(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_lt_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LT(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_lt_16      - Type: int4x16_t   - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 16bit elements
//
// Description:
//
// The beco_salu_lt_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_16(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_lt_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LT(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_lt_8       - Type: int8x8_t    - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 8bit elements
//
// Description:
//
// The beco_salu_lt_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_8(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_lt_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LT(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_ltu_32     - Type: int2x32_t   - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 32bit elements
//
// Description:
//
// The beco_salu_ltu_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if unsigned less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_32(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ltu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LTU(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_ltu_16     - Type: int4x16_t   - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 16bit elements
//
// Description:
//
// The beco_salu_ltu_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if unsigned less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_16(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ltu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LTU(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_ltu_8      - Type: int8x8_t    - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 8bit elements
//
// Description:
//
// The beco_salu_ltu_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if unsigned less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_8(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ltu_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_LTU(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_ge_32      - Type: int2x32_t   - Operation: GE(Set mask if Signed Greater-Equal )
// Beco SIMD GE operation for 32bit elements
//
// Description:
//
// The beco_salu_ge_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if signed greater-equal  is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if signed greater-equal .
//
// Operation:
//
// beco_salu_ge_32(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ge_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GE(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_ge_16      - Type: int4x16_t   - Operation: GE(Set mask if Signed Greater-Equal )
// Beco SIMD GE operation for 16bit elements
//
// Description:
//
// The beco_salu_ge_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if signed greater-equal  is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if signed greater-equal .
//
// Operation:
//
// beco_salu_ge_16(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ge_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GE(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_ge_8       - Type: int8x8_t    - Operation: GE(Set mask if Signed Greater-Equal )
// Beco SIMD GE operation for 8bit elements
//
// Description:
//
// The beco_salu_ge_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if signed greater-equal  is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed greater-equal .
//
// Operation:
//
// beco_salu_ge_8(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ge_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GE(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_geu_32     - Type: int2x32_t   - Operation: GEU(Set mask if Unsigned Greater-Equal )
// Beco SIMD GEU operation for 32bit elements
//
// Description:
//
// The beco_salu_geu_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if unsigned greater-equal  is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if unsigned greater-equal .
//
// Operation:
//
// beco_salu_geu_32(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_geu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GEU(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_geu_16     - Type: int4x16_t   - Operation: GEU(Set mask if Unsigned Greater-Equal )
// Beco SIMD GEU operation for 16bit elements
//
// Description:
//
// The beco_salu_geu_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if unsigned greater-equal  is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if unsigned greater-equal .
//
// Operation:
//
// beco_salu_geu_16(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_geu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GEU(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_geu_8      - Type: int8x8_t    - Operation: GEU(Set mask if Unsigned Greater-Equal )
// Beco SIMD GEU operation for 8bit elements
//
// Description:
//
// The beco_salu_geu_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if unsigned greater-equal  is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if unsigned greater-equal .
//
// Operation:
//
// beco_salu_geu_8(rd, rs, rt):
//     for each lane:
//        rd = (rs >= rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_geu_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_GEU(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_ne_32      - Type: int2x32_t   - Operation: NE(Set mask if not-equal)
// Beco SIMD NE operation for 32bit elements
//
// Description:
//
// The beco_salu_ne_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if not-equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// not-equal.
//
// Operation:
//
// beco_salu_ne_32(rd, rs, rt):
//     for each lane:
//        rd = (rs != rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ne_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_NE(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_ne_16      - Type: int4x16_t   - Operation: NE(Set mask if not-equal)
// Beco SIMD NE operation for 16bit elements
//
// Description:
//
// The beco_salu_ne_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if not-equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// not-equal.
//
// Operation:
//
// beco_salu_ne_16(rd, rs, rt):
//     for each lane:
//        rd = (rs != rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ne_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_NE(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_ne_8       - Type: int8x8_t    - Operation: NE(Set mask if not-equal)
// Beco SIMD NE operation for 8bit elements
//
// Description:
//
// The beco_salu_ne_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if not-equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// not-equal.
//
// Operation:
//
// beco_salu_ne_8(rd, rs, rt):
//     for each lane:
//        rd = (rs != rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ne_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_NE(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_eq_32      - Type: int2x32_t   - Operation: EQ(Set mask if equal)
// Beco SIMD EQ operation for 32bit elements
//
// Description:
//
// The beco_salu_eq_32 instruction operate on the source operands as
// vectors of 32 bit elements, set mask if equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// equal.
//
// Operation:
//
// beco_salu_eq_32(rd, rs, rt):
//     for each lane:
//        rd = (rs == rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_eq_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_EQ(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_eq_16      - Type: int4x16_t   - Operation: EQ(Set mask if equal)
// Beco SIMD EQ operation for 16bit elements
//
// Description:
//
// The beco_salu_eq_16 instruction operate on the source operands as
// vectors of 16 bit elements, set mask if equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// equal.
//
// Operation:
//
// beco_salu_eq_16(rd, rs, rt):
//     for each lane:
//        rd = (rs == rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_eq_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_EQ(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_eq_8       - Type: int8x8_t    - Operation: EQ(Set mask if equal)
// Beco SIMD EQ operation for 8bit elements
//
// Description:
//
// The beco_salu_eq_8 instruction operate on the source operands as
// vectors of 8 bit elements, set mask if equal is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd, lane rd is set mask if
// equal.
//
// Operation:
//
// beco_salu_eq_8(rd, rs, rt):
//     for each lane:
//        rd = (rs == rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_eq_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_EQ(VECSZ8), rd, rs, rt);
}


// Shifts

// Function: beco_salu_shl_8      - Type: int8x8_t    - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 8bit elements
//
// Description:
//
// The beco_salu_shl_8 instruction treat the source operands as vectors
// of 8 bit elements. The logical shift left shift amount in rt is signed,
// with positive values shift Left and negative values shift Right.
// The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_8(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_shl_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SHL(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_shl_16     - Type: int4x16_t   - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 16bit elements
//
// Description:
//
// The beco_salu_shl_16 instruction treat the source operands as vectors
// of 16 bit elements. The logical shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_16(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_shl_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SHL(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_shl_32     - Type: int2x32_t   - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 32bit elements
//
// Description:
//
// The beco_salu_shl_32 instruction treat the source operands as vectors
// of 32 bit elements. The logical shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_32(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_shl_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_SHL(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_ashl_8     - Type: int8x8_t    - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 8bit elements
//
// Description:
//
// The beco_salu_ashl_8 instruction treat the source operands as vectors
// of 8 bit elements. The arithmetic shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_8(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={1,1,1, -128,-128,85,85,  127}};
//     beco_vec64_in_t B = {.i8={1,7,8,   -1,  -8, 0,104, 1}};
//
//     beco_write_reg(BECO_REG0, A);
//     beco_write_reg(BECO_REG1, B);
//
//     beco_salu_ashl_8(BECO_REG2,  BECO_REG0, BECO_REG1);
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//
// /* Result in v[]:
//     (   1 s<<   1) =   2
//     (   1 s<<   7) = 128 (overflow)
//     (   1 s<<   8) = 0   (overflow)
//     (-128 s<<  -1) = 192 (== -64)
//     (-128 s<<  -8) = 255 (== -1)
//     (  85 s<<   0) = 85
//     (  85 s<< 104) = 0   (overflow)
//     ( 127 s<<   1) = 254 (overflow) */
//
//
BECO_INLINE void beco_salu_ashl_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ASHL(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_ashl_16    - Type: int4x16_t   - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 16bit elements
//
// Description:
//
// The beco_salu_ashl_16 instruction treat the source operands as vectors
// of 16 bit elements. The arithmetic shift left shift amount in rt
// is signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_16(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ashl_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ASHL(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_ashl_32    - Type: int2x32_t   - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 32bit elements
//
// Description:
//
// The beco_salu_ashl_32 instruction treat the source operands as vectors
// of 32 bit elements. The arithmetic shift left shift amount in rt
// is signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_32(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_ashl_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ASHL(VECSZ32), rd, rs, rt);
}

// Function: beco_salu_rol_8      - Type: int8x8_t    - Operation: ROL(Rotate Left)
// Beco SIMD ROL operation for 8bit elements
//
// Description:
//
// The beco_salu_rol_8 instruction treat the source operands as vectors
// of 8 bit elements. The rotate left shift amount in rt is 5 bit unsigned,
// the lower bits of which control the rotation within each element,
// and the next higher bits allow *shuffle* elements lanes within the
// same 32 bit word. The result is saved in the destination register,
// rd.
//
// Note:
//
// The *shuffle* only allow for rearranging the elements within the
// two halfs of the 64 bit Beco register. This means that there are
// no shuffle for 32 bit elements, 16 bit elements only support swapping
// high and low elements. 8 bit elements support 8 different shuffle
// modes.
//
// Operation:
//
// beco_salu_rol_8(rd, rs, rt):
//     rs = shuffle(rs, rt[0].b4, rt[0].b3, rt[2].b3)
//     for each lane:
//         shift = rt % 8
//         rd = (rs << shift)  |  (rs >> -shift)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
// Example:
//
//     beco_vec64_in_t A = {.i8={ 1, 2, 3, 4,  5, 6, 7, 8}};
//     beco_vec64_in_t B = {.i8={24, 0, 8, 0, 24, 0, 8, 0}};  // rearrange
// elements
//     beco_write_reg(BECO_REG0, A);
//     beco_write_reg(BECO_REG1, B);
//
//     beco_salu_rol_8(BECO_REG2,  BECO_REG0, BECO_REG1);
//     beco_vec64_out_t v = beco_read_reg(BECO_REG2);
//     // input  08 07 06 05  04 03 02 01 (byte 7-0)
//     // expect 05 06 07 08  01 02 03 04 (byte 7-0)
//
//
BECO_INLINE void beco_salu_rol_8(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ROL(VECSZ8), rd, rs, rt);
}

// Function: beco_salu_rol_16     - Type: int4x16_t   - Operation: ROL(Rotate Left)
// Beco SIMD ROL operation for 16bit elements
//
// Description:
//
// The beco_salu_rol_16 instruction treat the source operands as vectors
// of 16 bit elements. The rotate left shift amount in rt is 5 bit unsigned,
// the lower bits of which control the rotation within each element,
// and the next higher bits allow *shuffle* elements lanes within the
// same 32 bit word. The result is saved in the destination register,
// rd.
//
// Note:
//
// The *shuffle* only allow for rearranging the elements within the
// two halfs of the 64 bit Beco register. This means that there are
// no shuffle for 32 bit elements, 16 bit elements only support swapping
// high and low elements. 8 bit elements support 8 different shuffle
// modes.
//
// Operation:
//
// beco_salu_rol_16(rd, rs, rt):
//     rs = shuffle(rs, rt[0].b4, rt[0].b3, rt[2].b3)
//     for each lane:
//         shift = rt % 16
//         rd = (rs << shift)  |  (rs >> -shift)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_rol_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ROL(VECSZ16), rd, rs, rt);
}

// Function: beco_salu_rol_32     - Type: int2x32_t   - Operation: ROL(Rotate Left)
// Beco SIMD ROL operation for 32bit elements
//
// Description:
//
// The beco_salu_rol_32 instruction treat the source operands as vectors
// of 32 bit elements. The rotate left shift amount in rt is 5 bit unsigned,
// the lower bits of which control the rotation within each element,
// and the next higher bits allow *shuffle* elements lanes within the
// same 32 bit word. The result is saved in the destination register,
// rd.
//
// Note:
//
// The *shuffle* only allow for rearranging the elements within the
// two halfs of the 64 bit Beco register. This means that there are
// no shuffle for 32 bit elements, 16 bit elements only support swapping
// high and low elements. 8 bit elements support 8 different shuffle
// modes.
//
// Operation:
//
// beco_salu_rol_32(rd, rs, rt):
//     rs = shuffle(rs, rt[0].b4, rt[0].b3, rt[2].b3)
//     for each lane:
//         shift = rt % 32
//         rd = (rs << shift)  |  (rs >> -shift)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source Beco scalar register
//
BECO_INLINE void beco_salu_rol_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    beco_reg_t       rt)
{
    BECO_CDP2(OPC_SALU_ROL(VECSZ32), rd, rs, rt);
}


//
// Instructions with BECO-REG + GPR
//

// Adder

// Function: beco_salu_add_32_g   - Type: int2x32_t   - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 32bit elements
//
// Description:
//
// The beco_salu_add_32_g instruction operate on the source operands
// as vectors of 32 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_32_g(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_add_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ADD(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_add_16_g   - Type: int4x16_t   - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 16bit elements
//
// Description:
//
// The beco_salu_add_16_g instruction operate on the source operands
// as vectors of 16 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_16_g(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_add_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ADD(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_add_8_g    - Type: int8x8_t    - Operation: ADD(Arithmetic Addition)
// Beco SIMD ADD operation for 8bit elements
//
// Description:
//
// The beco_salu_add_8_g instruction operate on the source operands
// as vectors of 8 bit elements, arithmetic addition is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_add_8_g(rd, rs, rt):
//     for each lane:
//         rd = rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_add_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ADD(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_sub_32_g   - Type: int2x32_t   - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 32bit elements
//
// Description:
//
// The beco_salu_sub_32_g instruction operate on the source operands
// as vectors of 32 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_32_g(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_sub_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SUB(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_sub_16_g   - Type: int4x16_t   - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 16bit elements
//
// Description:
//
// The beco_salu_sub_16_g instruction operate on the source operands
// as vectors of 16 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_16_g(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_sub_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SUB(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_sub_8_g    - Type: int8x8_t    - Operation: SUB(Arithmetic Subtract)
// Beco SIMD SUB operation for 8bit elements
//
// Description:
//
// The beco_salu_sub_8_g instruction operate on the source operands
// as vectors of 8 bit elements, arithmetic subtract is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_sub_8_g(rd, rs, rt):
//     for each lane:
//          rd = rs - rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_sub_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SUB(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_rsub_32_g  - Type: int2x32_t   - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 32bit elements
//
// Description:
//
// The beco_salu_rsub_32_g instruction operate on the source operands
// as vectors of 32 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_32_g(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_rsub_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_RSUB(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_rsub_16_g  - Type: int4x16_t   - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 16bit elements
//
// Description:
//
// The beco_salu_rsub_16_g instruction operate on the source operands
// as vectors of 16 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_16_g(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_rsub_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_RSUB(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_rsub_8_g   - Type: int8x8_t    - Operation: RSUB(Arithmetic Reverse Subtract)
// Beco SIMD RSUB operation for 8bit elements
//
// Description:
//
// The beco_salu_rsub_8_g instruction operate on the source operands
// as vectors of 8 bit elements, arithmetic reverse subtract is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_rsub_8_g(rd, rs, rt):
//     for each lane:
//          rd = -rs + rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_rsub_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_RSUB(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_gabs_32_g  - Type: int2x32_t   - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 32bit elements
//
// Description:
//
// The beco_salu_gabs_32_g instruction operate on the source operands
// as vectors of 32 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_32_g(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_gabs_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ABS(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_gabs_16_g  - Type: int4x16_t   - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 16bit elements
//
// Description:
//
// The beco_salu_gabs_16_g instruction operate on the source operands
// as vectors of 16 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_16_g(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_gabs_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ABS(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_gabs_8_g   - Type: int8x8_t    - Operation: ABS(General Absolute value)
// Beco SIMD ABS operation for 8bit elements
//
// Description:
//
// The beco_salu_gabs_8_g instruction operate on the source operands
// as vectors of 8 bit elements, perform general absolute value between
// corresponding lanes of the sources and save result in the destination
// register.
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Note2:
//
// When when second parameter is equal to zero,  a normal ABS is calculated,
// i.e.  `rd = ((rs > 0)?  rs : -rs)`.
//
//
// Operation:
//
// beco_salu_gabs_8_g(rd, rs, rt):
//     for each lane:
//         temp = rt - rs
//         rd = (temp < 0)? rs : temp)
//
//         // For rt == 0 this reduce to:
//         // rd = ((rs > 0)?  rs : -rs)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_gabs_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ABS(VECSZ8), (uint32_t)rt, rd, rs);
}


// Mask

// Function: beco_salu_mask_32_g  - Type: int2x32_t   - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 32bit elements
//
// Description:
//
// The beco_salu_mask_32_g instruction generate bit mask from size.
// If the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_32_g(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_mask_32_g(
    beco_reg_t       rd,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MASK(VECSZ32), (uint32_t)rt, rd, 0);
}

// Function: beco_salu_mask_16_g  - Type: int4x16_t   - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 16bit elements
//
// Description:
//
// The beco_salu_mask_16_g instruction generate bit mask from size.
// If the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_16_g(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_mask_16_g(
    beco_reg_t       rd,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MASK(VECSZ16), (uint32_t)rt, rd, 0);
}

// Function: beco_salu_mask_8_g   - Type: int8x8_t    - Operation: MASK(Generate bit mask from size)
// Beco SIMD MASK operation for 8bit elements
//
// Description:
//
// The beco_salu_mask_8_g instruction generate bit mask from size. If
// the count is positive, then a mask with `count` zeros in the lsb
// is generated.  If the count is negative, then a mask with `-count`
// zeros in the msb is generated and result is save result in the destination
// register.
//
// Operation:
//
// beco_salu_mask_8_g(rd, rt):
//     for each lane:
//         n_shift = rt
//         rd = -1 << n_shift,    if (n_shift >= 0)
//         rd = -1 >> -n_shift,   if (n_shift < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_mask_8_g(
    beco_reg_t       rd,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MASK(VECSZ8), (uint32_t)rt, rd, 0);
}


// Shifts

// Function: beco_salu_shl_8_g    - Type: int8x8_t    - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 8bit elements
//
// Description:
//
// The beco_salu_shl_8_g instruction treat the source operands as vectors
// of 8 bit elements. The logical shift left shift amount in rt is signed,
// with positive values shift Left and negative values shift Right.
// The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_8_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_shl_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SHL(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_shl_16_g   - Type: int4x16_t   - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 16bit elements
//
// Description:
//
// The beco_salu_shl_16_g instruction treat the source operands as vectors
// of 16 bit elements. The logical shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_16_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_shl_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SHL(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_shl_32_g   - Type: int2x32_t   - Operation: SHL(Logical Shift Left)
// Beco SIMD SHL operation for 32bit elements
//
// Description:
//
// The beco_salu_shl_32_g instruction treat the source operands as vectors
// of 32 bit elements. The logical shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
//
// Note:
//
// The logical shift left shift argument is large enough to allow the
// all the bits of the source to be shifted out. For example for 8 bits
// `255/256 = (255 >> 8) = 0`, likewise for 32 bit `(0xffffffff << 39)
// = 0`.
//
// Operation:
//
// beco_salu_shl_32_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs >> -rt),   when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_shl_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_SHL(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_ashl_8_g   - Type: int8x8_t    - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 8bit elements
//
// Description:
//
// The beco_salu_ashl_8_g instruction treat the source operands as vectors
// of 8 bit elements. The arithmetic shift left shift amount in rt is
// signed, with positive values shift Left and negative values shift
// Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_8_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ashl_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ASHL(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_ashl_16_g  - Type: int4x16_t   - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 16bit elements
//
// Description:
//
// The beco_salu_ashl_16_g instruction treat the source operands as
// vectors of 16 bit elements. The arithmetic shift left shift amount
// in rt is signed, with positive values shift Left and negative values
// shift Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_16_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ashl_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ASHL(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_ashl_32_g  - Type: int2x32_t   - Operation: ASHL(Arithmetic Shift Left)
// Beco SIMD ASHL operation for 32bit elements
//
// Description:
//
// The beco_salu_ashl_32_g instruction treat the source operands as
// vectors of 32 bit elements. The arithmetic shift left shift amount
// in rt is signed, with positive values shift Left and negative values
// shift Right. The result is saved in the destination register, rd.
// Right shifts will zero-extend the result (shift zeros into the top
// bits).
// Regardless of the element size, the shift-amount in rt is always
// signed with 8 significant bits.
// Right shifts will sign-extend the result  (shift copies of the sign-bit
// into the top bits).
//
// Note:
//
// The arithmetic shift left shift argument is large enough to allow
// the all the bits of the source to be shifted out. For example for
// 8 bit `127/256 = (127 >> 8) = 0`, `-55/256 = (-55 >> 8) = -1`, likewise
// for 32 bit `(0xffffffff << 39) = 0`.
//
// Operation:
//
// beco_salu_ashl_32_g(rd, rs, rt):
//     for each lane:
//          rd = (rs << rt),    when (rt >= 0)
//          rd = (rs s>> -rt),  when (rt < 0)
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ashl_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_ASHL(VECSZ32), (uint32_t)rt, rd, rs);
}


// Min/Max

// Function: beco_salu_min_32_g   - Type: int2x32_t   - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 32bit elements
//
// Description:
//
// The beco_salu_min_32_g instruction operate on the source operands
// as vectors of 32 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_32_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_min_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MIN(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_min_16_g   - Type: int4x16_t   - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 16bit elements
//
// Description:
//
// The beco_salu_min_16_g instruction operate on the source operands
// as vectors of 16 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_16_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_min_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MIN(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_min_8_g    - Type: int8x8_t    - Operation: MIN(Signed MIN)
// Beco SIMD MIN operation for 8bit elements
//
// Description:
//
// The beco_salu_min_8_g instruction operate on the source operands
// as vectors of 8 bit elements, signed MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_min_8_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_min_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MIN(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_minu_32_g  - Type: int2x32_t   - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 32bit elements
//
// Description:
//
// The beco_salu_minu_32_g instruction operate on the source operands
// as vectors of 32 bit elements, unsigned MIN is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_minu_32_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_minu_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MINU(VECSZ32), rt, rd, rs);
}

// Function: beco_salu_minu_16_g  - Type: int4x16_t   - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 16bit elements
//
// Description:
//
// The beco_salu_minu_16_g instruction operate on the source operands
// as vectors of 16 bit elements, unsigned MIN is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_minu_16_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_minu_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MINU(VECSZ16), rt, rd, rs);
}

// Function: beco_salu_minu_8_g   - Type: int8x8_t    - Operation: MINU(Unsigned MIN)
// Beco SIMD MINU operation for 8bit elements
//
// Description:
//
// The beco_salu_minu_8_g instruction operate on the source operands
// as vectors of 8 bit elements, unsigned MIN is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_minu_8_g(rd, rs, rt):
//     for each lane:
//         rd = (rs <= rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_minu_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MINU(VECSZ8), rt, rd, rs);
}

// Function: beco_salu_max_32_g   - Type: int2x32_t   - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 32bit elements
//
// Description:
//
// The beco_salu_max_32_g instruction operate on the source operands
// as vectors of 32 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_32_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_max_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MAX(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_max_16_g   - Type: int4x16_t   - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 16bit elements
//
// Description:
//
// The beco_salu_max_16_g instruction operate on the source operands
// as vectors of 16 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_16_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_max_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MAX(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_max_8_g    - Type: int8x8_t    - Operation: MAX(Signed MAX)
// Beco SIMD MAX operation for 8bit elements
//
// Description:
//
// The beco_salu_max_8_g instruction operate on the source operands
// as vectors of 8 bit elements, signed MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_max_8_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_max_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_MAX(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_maxu_32_g  - Type: int2x32_t   - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 32bit elements
//
// Description:
//
// The beco_salu_maxu_32_g instruction operate on the source operands
// as vectors of 32 bit elements, unsigned MAX is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_32_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_maxu_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MAXU(VECSZ32), rt, rd, rs);
}

// Function: beco_salu_maxu_16_g  - Type: int4x16_t   - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 16bit elements
//
// Description:
//
// The beco_salu_maxu_16_g instruction operate on the source operands
// as vectors of 16 bit elements, unsigned MAX is performed between
// corresponding lanes of the source registers, rs and rt. The result
// is saved in the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_16_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_maxu_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MAXU(VECSZ16), rt, rd, rs);
}

// Function: beco_salu_maxu_8_g   - Type: int8x8_t    - Operation: MAXU(Unsigned MAX)
// Beco SIMD MAXU operation for 8bit elements
//
// Description:
//
// The beco_salu_maxu_8_g instruction operate on the source operands
// as vectors of 8 bit elements, unsigned MAX is performed between corresponding
// lanes of the source registers, rs and rt. The result is saved in
// the destination register, rd.
//
// Operation:
//
// beco_salu_maxu_8_g(rd, rs, rt):
//     for each lane:
//         rd = (rs > rt) ? rs : rt
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_maxu_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_MAXU(VECSZ8), rt, rd, rs);
}


// Compare elements to mask

// Function: beco_salu_lt_32_g    - Type: int2x32_t   - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 32bit elements
//
// Description:
//
// The beco_salu_lt_32_g instruction operate on the source operands
// as vectors of 32 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_32_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_lt_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_LT(VECSZ32), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_lt_16_g    - Type: int4x16_t   - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 16bit elements
//
// Description:
//
// The beco_salu_lt_16_g instruction operate on the source operands
// as vectors of 16 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_16_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_lt_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_LT(VECSZ16), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_lt_8_g     - Type: int8x8_t    - Operation: LT(Set mask if Signed Less-Than)
// Beco SIMD LT operation for 8bit elements
//
// Description:
//
// The beco_salu_lt_8_g instruction operate on the source operands as
// vectors of 8 bit elements, set mask if signed less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if signed less-than.
//
// Operation:
//
// beco_salu_lt_8_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_lt_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    int32_t          rt)
{
    SALU_MCR2(OPC_SALU_LT(VECSZ8), (uint32_t)rt, rd, rs);
}

// Function: beco_salu_ltu_32_g   - Type: int2x32_t   - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 32bit elements
//
// Description:
//
// The beco_salu_ltu_32_g instruction operate on the source operands
// as vectors of 32 bit elements, set mask if unsigned less-than is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_32_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ltu_32_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_LTU(VECSZ32), rt, rd, rs);
}

// Function: beco_salu_ltu_16_g   - Type: int4x16_t   - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 16bit elements
//
// Description:
//
// The beco_salu_ltu_16_g instruction operate on the source operands
// as vectors of 16 bit elements, set mask if unsigned less-than is
// performed between corresponding lanes of the source registers, rs
// and rt. The result is saved in the destination register, rd, lane
// rd is set mask if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_16_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ltu_16_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_LTU(VECSZ16), rt, rd, rs);
}

// Function: beco_salu_ltu_8_g    - Type: int8x8_t    - Operation: LTU(Set mask if Unsigned Less-Than)
// Beco SIMD LTU operation for 8bit elements
//
// Description:
//
// The beco_salu_ltu_8_g instruction operate on the source operands
// as vectors of 8 bit elements, set mask if unsigned less-than is performed
// between corresponding lanes of the source registers, rs and rt. The
// result is saved in the destination register, rd, lane rd is set mask
// if unsigned less-than.
//
// Operation:
//
// beco_salu_ltu_8_g(rd, rs, rt):
//     for each lane:
//        rd = (rs < rt) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     rt - source ARM-GPR
//
BECO_INLINE void beco_salu_ltu_8_g(
    beco_reg_t       rd,
    beco_reg_t       rs,
    uint32_t         rt)
{
    SALU_MCR2(OPC_SALU_LTU(VECSZ8), rt, rd, rs);
}


//
// Instructions with BECO-REG + small constant
//

// Instructions with small int {0}

// Function: beco_salu_neg_32     - Type: int2x32_t   - Operation: RSUB(Negate)
// Beco SIMD RSUB operation for 32bit elements
//
// Description:
//
// The beco_salu_neg_32 instruction perform negate of the (first) source
// register and store the result in the destination register, rd.
//
//
// Operation:
//
// beco_salu_neg_32(rd, rs):
//     for each lane:
//         rd = -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_neg_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ32), rd, rs, FNC_SALU_SHIMM_RSUB(0));
}

// Function: beco_salu_neg_16     - Type: int4x16_t   - Operation: RSUB(Negate)
// Beco SIMD RSUB operation for 16bit elements
//
// Description:
//
// The beco_salu_neg_16 instruction perform negate of the (first) source
// register and store the result in the destination register, rd.
//
//
// Operation:
//
// beco_salu_neg_16(rd, rs):
//     for each lane:
//         rd = -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_neg_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ16), rd, rs, FNC_SALU_SHIMM_RSUB(0));
}

// Function: beco_salu_neg_8      - Type: int8x8_t    - Operation: RSUB(Negate)
// Beco SIMD RSUB operation for 8bit elements
//
// Description:
//
// The beco_salu_neg_8 instruction perform negate of the (first) source
// register and store the result in the destination register, rd.
//
//
// Operation:
//
// beco_salu_neg_8(rd, rs):
//     for each lane:
//         rd = -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_neg_8(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ8), rd, rs, FNC_SALU_SHIMM_RSUB(0));
}

// Function: beco_salu_abs_32     - Type: int2x32_t   - Operation: ABS(Absolute value)
// Beco SIMD ABS operation for 32bit elements
//
// Description:
//
// The beco_salu_abs_32 instruction operate on the source operands as
// vectors of 32 bit elements, perform absolute value between corresponding
// lanes of the sources and save result in the destination register.
//
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Operation:
//
// beco_salu_abs_32(rd, rs):
//     for each lane:
//         rd = (rs >= 0)? rs : -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_abs_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ32), rd, rs, FNC_SALU_SHIMM_ABS(0));
}

// Function: beco_salu_abs_16     - Type: int4x16_t   - Operation: ABS(Absolute value)
// Beco SIMD ABS operation for 16bit elements
//
// Description:
//
// The beco_salu_abs_16 instruction operate on the source operands as
// vectors of 16 bit elements, perform absolute value between corresponding
// lanes of the sources and save result in the destination register.
//
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Operation:
//
// beco_salu_abs_16(rd, rs):
//     for each lane:
//         rd = (rs >= 0)? rs : -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_abs_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ16), rd, rs, FNC_SALU_SHIMM_ABS(0));
}

// Function: beco_salu_abs_8      - Type: int8x8_t    - Operation: ABS(Absolute value)
// Beco SIMD ABS operation for 8bit elements
//
// Description:
//
// The beco_salu_abs_8 instruction operate on the source operands as
// vectors of 8 bit elements, perform absolute value between corresponding
// lanes of the sources and save result in the destination register.
//
//
// Note:
//
// The output must be unsigned for correct interpretation of the the
// smallest negative integer e.g. abs(-128) = 128u. Both values have
// the same encoding in 8-bit integers (0x80).
//
//
// Operation:
//
// beco_salu_abs_8(rd, rs):
//     for each lane:
//         rd = (rs >= 0)? rs : -rs
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_abs_8(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ8), rd, rs, FNC_SALU_SHIMM_ABS(0));
}

// Function: beco_salu_maxz_32    - Type: int2x32_t   - Operation: MAX(Clamp Negative to Zero)
// Beco SIMD MAX operation for 32bit elements
//
// Description:
//
// The beco_salu_maxz_32 instruction perform clamp negative to zero
// of the (first) source register and store the result in the destination
// register, rd.
//
// Operation:
//
// beco_salu_maxz_32(rd, rs):
//     for each lane:
//         rd = (rs > 0) ? rs : %T
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_maxz_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ32), rd, rs, FNC_SALU_SHIMM_MAX(0));
}

// Function: beco_salu_maxz_16    - Type: int4x16_t   - Operation: MAX(Clamp Negative to Zero)
// Beco SIMD MAX operation for 16bit elements
//
// Description:
//
// The beco_salu_maxz_16 instruction perform clamp negative to zero
// of the (first) source register and store the result in the destination
// register, rd.
//
// Operation:
//
// beco_salu_maxz_16(rd, rs):
//     for each lane:
//         rd = (rs > 0) ? rs : %T
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_maxz_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ16), rd, rs, FNC_SALU_SHIMM_MAX(0));
}

// Function: beco_salu_maxz_8     - Type: int8x8_t    - Operation: MAX(Clamp Negative to Zero)
// Beco SIMD MAX operation for 8bit elements
//
// Description:
//
// The beco_salu_maxz_8 instruction perform clamp negative to zero of
// the (first) source register and store the result in the destination
// register, rd.
//
// Operation:
//
// beco_salu_maxz_8(rd, rs):
//     for each lane:
//         rd = (rs > 0) ? rs : %T
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_maxz_8(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ8), rd, rs, FNC_SALU_SHIMM_MAX(0));
}

// Function: beco_salu_eqz_32     - Type: int2x32_t   - Operation: EQ(Set Mask if Zero)
// Beco SIMD EQ operation for 32bit elements
//
// Description:
//
// The beco_salu_eqz_32 instruction operate on the source operand as
// vectors of 32 bit elements, set mask if zero is performed on source
// register rs. Register lane rd is set mask if zero.
//
// Operation:
//
// beco_salu_eqz_32(rd, rs):
//     for each lane:
//        rd = (rs == 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_eqz_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ32), rd, rs, FNC_SALU_SHIMM_EQ(0));
}

// Function: beco_salu_eqz_16     - Type: int4x16_t   - Operation: EQ(Set Mask if Zero)
// Beco SIMD EQ operation for 16bit elements
//
// Description:
//
// The beco_salu_eqz_16 instruction operate on the source operand as
// vectors of 16 bit elements, set mask if zero is performed on source
// register rs. Register lane rd is set mask if zero.
//
// Operation:
//
// beco_salu_eqz_16(rd, rs):
//     for each lane:
//        rd = (rs == 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_eqz_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ16), rd, rs, FNC_SALU_SHIMM_EQ(0));
}

// Function: beco_salu_eqz_8      - Type: int8x8_t    - Operation: EQ(Set Mask if Zero)
// Beco SIMD EQ operation for 8bit elements
//
// Description:
//
// The beco_salu_eqz_8 instruction operate on the source operand as
// vectors of 8 bit elements, set mask if zero is performed on source
// register rs. Register lane rd is set mask if zero.
//
// Operation:
//
// beco_salu_eqz_8(rd, rs):
//     for each lane:
//        rd = (rs == 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_eqz_8(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ8), rd, rs, FNC_SALU_SHIMM_EQ(0));
}

// Function: beco_salu_gez_32     - Type: int2x32_t   - Operation: GE(Set Mask if Positive)
// Beco SIMD GE operation for 32bit elements
//
// Description:
//
// The beco_salu_gez_32 instruction operate on the source operand as
// vectors of 32 bit elements, set mask if positive is performed on
// source register rs. Register lane rd is set mask if positive.
//
// Operation:
//
// beco_salu_gez_32(rd, rs):
//     for each lane:
//        rd = (rs >= 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_gez_32(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ32), rd, rs, FNC_SALU_SHIMM_GE(0));
}

// Function: beco_salu_gez_16     - Type: int4x16_t   - Operation: GE(Set Mask if Positive)
// Beco SIMD GE operation for 16bit elements
//
// Description:
//
// The beco_salu_gez_16 instruction operate on the source operand as
// vectors of 16 bit elements, set mask if positive is performed on
// source register rs. Register lane rd is set mask if positive.
//
// Operation:
//
// beco_salu_gez_16(rd, rs):
//     for each lane:
//        rd = (rs >= 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_gez_16(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ16), rd, rs, FNC_SALU_SHIMM_GE(0));
}

// Function: beco_salu_gez_8      - Type: int8x8_t    - Operation: GE(Set Mask if Positive)
// Beco SIMD GE operation for 8bit elements
//
// Description:
//
// The beco_salu_gez_8 instruction operate on the source operand as
// vectors of 8 bit elements, set mask if positive is performed on source
// register rs. Register lane rd is set mask if positive.
//
// Operation:
//
// beco_salu_gez_8(rd, rs):
//     for each lane:
//        rd = (rs >= 0) ? -1 : 0
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//
BECO_INLINE void beco_salu_gez_8(
    beco_reg_t       rd,
    beco_reg_t       rs)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ8), rd, rs, FNC_SALU_SHIMM_GE(0));
}


// Instructions with small int {7, 15, 23}
//
// These instructions are not useful:
//    beco_salu_bclip_8(...)               - is a NOP
//    beco_salu_bclip_16(..., 15)          - is a NOP
//    beco_salu_bclip_16(..., 23)          - is a NOP
//    beco_salu_bclip_16(value, 7)         - is OK
//    beco_salu_bclip_32(value, [7,15,23]) - are OK

// Function: beco_salu_bclip_32   - Type: int2x32_t   - Operation: CLIP(Signed Saturate)
// Beco SIMD CLIP operation for 32bit elements
//
// Description:
//
// The beco_salu_bclip_32 instruction perform signed saturate on each
// lane of 32 bits in the source register and save result in the destination
// register.
//
// Operation:
//
// beco_salu_bclip_32(rd, rs, n):
//     for each lane:
//         max = (1 << N) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = max(rs, ~max),   when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     n - source literal constant  SALU_CONST_[7,15,23]
//
BECO_INLINE void beco_salu_bclip_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    salu_const2s_t   n)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ32), rd, rs, FNC_SALU_SHIMM_CLIP(n));
}

// Function: beco_salu_bclip_16   - Type: int4x16_t   - Operation: CLIP(Signed Saturate)
// Beco SIMD CLIP operation for 16bit elements
//
// Description:
//
// The beco_salu_bclip_16 instruction perform signed saturate on each
// lane of 16 bits in the source register and save result in the destination
// register.
//
// Operation:
//
// beco_salu_bclip_16(rd, rs, n):
//     for each lane:
//         max = (1 << N) - 1
//         val = min(rs, max),    when (rs >= 0)
//         val = max(rs, ~max),   when (rs < 0)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     n - source literal constant  SALU_CONST_[7,15,23]
//
BECO_INLINE void beco_salu_bclip_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    salu_const2s_t   n)
{
    BECO_CDP2(OPC_SALU_SHIMM(VECSZ16), rd, rs, FNC_SALU_SHIMM_CLIP(n));
}


// Instructions with small int {8, 16, 24}
//
// These instructions are not useful:
//    beco_salu_bclipu_8()         - is a NOP
//    beco_salu_bclipu_16(..., 16) - is a NOP
//    beco_salu_bclipu_16(..., 24) - is a NOP
//    beco_salu_bclipu_16(value, 8)         - is OK
//    beco_salu_bclipu_32(value, [8,16,24]) - are OK

// Function: beco_salu_bclipu_32  - Type: int2x32_t   - Operation: CLIP(Unsigned Saturate)
// Beco SIMD CLIP operation for 32bit elements
//
// Description:
//
// The beco_salu_bclipu_32 instruction perform unsigned saturate on
// each lane of 32 bits in the source register and save result in the
// destination register.
//
// Operation:
//
// beco_salu_bclipu_32(rd, rs, n):
//     for each lane:
//         max = (1 << N) - 1
//         val = min(rs, max)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     n - source literal constant  SALU_CONST_[8,16,24]
//
BECO_INLINE void beco_salu_bclipu_32(
    beco_reg_t       rd,
    beco_reg_t       rs,
    salu_const2u_t   n)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ32), rd, rs, FNC_SALU_SHIMM_CLIP(n));
}

// Function: beco_salu_bclipu_16  - Type: int4x16_t   - Operation: CLIP(Unsigned Saturate)
// Beco SIMD CLIP operation for 16bit elements
//
// Description:
//
// The beco_salu_bclipu_16 instruction perform unsigned saturate on
// each lane of 16 bits in the source register and save result in the
// destination register.
//
// Operation:
//
// beco_salu_bclipu_16(rd, rs, n):
//     for each lane:
//         max = (1 << N) - 1
//         val = min(rs, max)
//         rd = val
//
// Where:
//     rd - destination Beco scalar register
//     rs - source Beco scalar register
//     n - source literal constant  SALU_CONST_[8,16,24]
//
BECO_INLINE void beco_salu_bclipu_16(
    beco_reg_t       rd,
    beco_reg_t       rs,
    salu_const2u_t   n)
{
    BECO_CDP2(OPC_SALU_SHIMMU(VECSZ16), rd, rs, FNC_SALU_SHIMM_CLIP(n));
}

