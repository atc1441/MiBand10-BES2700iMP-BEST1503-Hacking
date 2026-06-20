#ifndef __ATOMIC_CM_H__
#define __ATOMIC_CM_H__

//lint ++flb "Library Begin" [MISRA Note 12]

/// Atomic Access Operation: Write (8-bit)
/// \param[in]  mem             Memory address
/// \param[in]  val             Value to write
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint8_t atomic_wr8 (uint8_t *mem, uint8_t val) {
  mov    r2,r0
1
  ldrexb r0,[r2]
  strexb r3,r1,[r2]
  cbz    r3,%F2
  b      %B1
2
  bx     lr
}
#else
__STATIC_INLINE uint8_t atomic_wr8 (uint8_t *mem, uint8_t val) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint8_t  ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrexb %[ret],[%[mem]]\n\t"
    "strexb %[res],%[val],[%[mem]]\n\t"
    "cbz    %[res],2f\n\t"
    "b       1b\n"
  "2:"
  : [ret] "=&l" (ret),
    [res] "=&l" (res)
  : [mem] "l"   (mem),
    [val] "l"   (val)
  : "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Set bits (32-bit)
/// \param[in]  mem             Memory address
/// \param[in]  bits            Bit mask
/// \return                     New value
#if defined(__CC_ARM)
static __asm    uint32_t atomic_set32 (uint32_t *mem, uint32_t bits) {
  mov   r2,r0
1
  ldrex r0,[r2]
  orr   r0,r0,r1
  strex r3,r0,[r2]
  cbz   r3,%F2
  b     %B1
2
  bx     lr
}
#else
__STATIC_INLINE uint32_t atomic_set32 (uint32_t *mem, uint32_t bits) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[val],[%[mem]]\n\t"
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
    "mov   %[ret],%[val]\n\t"
    "orrs  %[ret],%[bits]\n\t"
#else
    "orr   %[ret],%[val],%[bits]\n\t"
#endif
    "strex %[res],%[ret],[%[mem]]\n\t"
    "cbz   %[res],2f\n\t"
    "b     1b\n"
  "2:"
  : [ret]  "=&l" (ret),
    [val]  "=&l" (val),
    [res]  "=&l" (res)
  : [mem]  "l"   (mem),
    [bits] "l"   (bits)
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
  : "memory", "cc"
#else
  : "memory"
#endif
  );

  return ret;
}
#endif

/// Atomic Access Operation: Clear bits (32-bit)
/// \param[in]  mem             Memory address
/// \param[in]  bits            Bit mask
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint32_t atomic_clr32 (uint32_t *mem, uint32_t bits) {
  push  {r4,lr}
  mov   r2,r0
1
  ldrex r0,[r2]
  bic   r4,r0,r1
  strex r3,r4,[r2]
  cbz   r3,%F2
  b     %B1
2
  pop   {r4,pc}
}
#else
__STATIC_INLINE uint32_t atomic_clr32 (uint32_t *mem, uint32_t bits) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
    "mov   %[val],%[ret]\n\t"
    "bics  %[val],%[bits]\n\t"
#else
    "bic   %[val],%[ret],%[bits]\n\t"
#endif
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],2f\n\t"
    "b     1b\n"
  "2:"
  : [ret]  "=&l" (ret),
    [val]  "=&l" (val),
    [res]  "=&l" (res)
  : [mem]  "l"   (mem),
    [bits] "l"   (bits)
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
  : "memory", "cc"
#else
  : "memory"
#endif
  );

  return ret;
}
#endif

/// Atomic Access Operation: Check if all specified bits (32-bit) are active and clear them
/// \param[in]  mem             Memory address
/// \param[in]  bits            Bit mask
/// \return                     Active bits before clearing or 0 if not active
#if defined(__CC_ARM)
static __asm    uint32_t atomic_chk32_all (uint32_t *mem, uint32_t bits) {
  push  {r4,lr}
  mov   r2,r0
1
  ldrex r0,[r2]
  and   r4,r0,r1
  cmp   r4,r1
  beq   %F2
  clrex
  movs  r0,#0
  pop   {r4,pc}
2
  bic   r4,r0,r1
  strex r3,r4,[r2]
  cbz   r3,%F3
  b     %B1
3
  pop   {r4,pc}
}
#else
__STATIC_INLINE uint32_t atomic_chk32_all (uint32_t *mem, uint32_t bits) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
    "mov   %[val],%[ret]\n\t"
    "ands  %[val],%[bits]\n\t"
#else
    "and   %[val],%[ret],%[bits]\n\t"
#endif
    "cmp   %[val],%[bits]\n\t"
    "beq   2f\n\t"
    "clrex\n\t"
    "movs  %[ret],#0\n\t"
    "b     3f\n"
  "2:\n\t"
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
    "mov   %[val],%[ret]\n\t"
    "bics  %[val],%[bits]\n\t"
#else
    "bic   %[val],%[ret],%[bits]\n\t"
#endif
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],3f\n\t"
    "b     1b\n"
  "3:"
  : [ret]  "=&l" (ret),
    [val]  "=&l" (val),
    [res]  "=&l" (res)
  : [mem]  "l"   (mem),
    [bits] "l"   (bits)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Check if any specified bits (32-bit) are active and clear them
/// \param[in]  mem             Memory address
/// \param[in]  bits            Bit mask
/// \return                     Active bits before clearing or 0 if not active
#if defined(__CC_ARM)
static __asm    uint32_t atomic_chk32_any (uint32_t *mem, uint32_t bits) {
  push  {r4,lr}
  mov   r2,r0
1
  ldrex r0,[r2]
  tst   r0,r1
  bne   %F2
  clrex
  movs  r0,#0
  pop   {r4,pc}
2
  bic   r4,r0,r1
  strex r3,r4,[r2]
  cbz   r3,%F3
  b     %B1
3
  pop   {r4,pc}
}
#else
__STATIC_INLINE uint32_t atomic_chk32_any (uint32_t *mem, uint32_t bits) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
    "tst   %[ret],%[bits]\n\t"
    "bne   2f\n\t"
    "clrex\n\t"
    "movs  %[ret],#0\n\t"
    "b     3f\n"
  "2:\n\t"
#if (defined(__ARM_ARCH_8M_BASE__) && (__ARM_ARCH_8M_BASE__ != 0))
    "mov   %[val],%[ret]\n\t"
    "bics  %[val],%[bits]\n\t"
#else
    "bic   %[val],%[ret],%[bits]\n\t"
#endif
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],3f\n\t"
    "b     1b\n"
  "3:"
  : [ret]  "=&l" (ret),
    [val]  "=&l" (val),
    [res]  "=&l" (res)
  : [mem]  "l"   (mem),
    [bits] "l"   (bits)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Increment (32-bit)
/// \param[in]  mem             Memory address
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint32_t atomic_inc32 (uint32_t *mem) {
  mov   r2,r0
1
  ldrex r0,[r2]
  adds  r1,r0,#1
  strex r3,r1,[r2]
  cbz   r3,%F2
  b     %B1
2
  bx     lr
}
#else
__STATIC_INLINE uint32_t atomic_inc32 (uint32_t *mem) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
    "adds  %[val],%[ret],#1\n\t"
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],2f\n\t"
    "b     1b\n"
  "2:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Increment (16-bit) if Less Than
/// \param[in]  mem             Memory address
/// \param[in]  max             Maximum value
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint16_t atomic_inc16_lt (uint16_t *mem, uint16_t max) {
  push   {r4,lr}
  mov    r2,r0
1
  ldrexh r0,[r2]
  cmp    r1,r0
  bhi    %F2
  clrex
  pop    {r4,pc}
2
  adds   r4,r0,#1
  strexh r3,r4,[r2]
  cbz    r3,%F3
  b      %B1
3
  pop    {r4,pc}
}
#else
__STATIC_INLINE uint16_t atomic_inc16_lt (uint16_t *mem, uint16_t max) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint16_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrexh %[ret],[%[mem]]\n\t"
    "cmp    %[max],%[ret]\n\t"
    "bhi    2f\n\t"
    "clrex\n\t"
    "b      3f\n"
  "2:\n\t"
    "adds   %[val],%[ret],#1\n\t"
    "strexh %[res],%[val],[%[mem]]\n\t"
    "cbz    %[res],3f\n\t"
    "b      1b\n"
  "3:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem),
    [max] "l"   (max)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Increment (16-bit) and clear on Limit
/// \param[in]  mem             Memory address
/// \param[in]  max             Maximum value
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint16_t atomic_inc16_lim (uint16_t *mem, uint16_t lim) {
  push   {r4,lr}
  mov    r2,r0
1
  ldrexh r0,[r2]
  adds   r4,r0,#1
  cmp    r1,r4
  bhi    %F2
  movs   r4,#0
2
  strexh r3,r4,[r2]
  cbz    r3,%F3
  b      %B1
3
  pop    {r4,pc}
}
#else
__STATIC_INLINE uint16_t atomic_inc16_lim (uint16_t *mem, uint16_t lim) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint16_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrexh %[ret],[%[mem]]\n\t"
    "adds   %[val],%[ret],#1\n\t"
    "cmp    %[lim],%[val]\n\t"
    "bhi    2f\n\t"
    "movs   %[val],#0\n"
  "2:\n\t"
    "strexh %[res],%[val],[%[mem]]\n\t"
    "cbz    %[res],3f\n\t"
    "b      1b\n"
  "3:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem),
    [lim] "l"   (lim)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Decrement (32-bit)
/// \param[in]  mem             Memory address
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint32_t atomic_dec32 (uint32_t *mem) {
  mov   r2,r0
1
  ldrex r0,[r2]
  subs  r1,r0,#1
  strex r3,r1,[r2]
  cbz   r3,%F2
  b     %B1
2
  bx     lr
}
#else
__STATIC_INLINE uint32_t atomic_dec32 (uint32_t *mem) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
    "subs  %[val],%[ret],#1\n\t"
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],2f\n\t"
    "b     1b\n"
  "2:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Decrement (32-bit) if Not Zero
/// \param[in]  mem             Memory address
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint32_t atomic_dec32_nz (uint32_t *mem) {
  mov   r2,r0
1
  ldrex r0,[r2]
  cbnz  r0,%F2
  clrex
  bx    lr
2
  subs  r1,r0,#1
  strex r3,r1,[r2]
  cbz   r3,%F3
  b     %B1
3
  bx     lr
}
#else
__STATIC_INLINE uint32_t atomic_dec32_nz (uint32_t *mem) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint32_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[mem]]\n\t"
    "cbnz  %[ret],2f\n\t"
    "clrex\n\t"
    "b     3f\n"
  "2:\n\t"
    "subs  %[val],%[ret],#1\n\t"
    "strex %[res],%[val],[%[mem]]\n\t"
    "cbz   %[res],3f\n\t"
    "b     1b\n"
  "3:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Decrement (16-bit) if Not Zero
/// \param[in]  mem             Memory address
/// \return                     Previous value
#if defined(__CC_ARM)
static __asm    uint16_t atomic_dec16_nz (uint16_t *mem) {
  mov    r2,r0
1
  ldrexh r0,[r2]
  cbnz   r0,%F2
  clrex
  bx     lr
2
  subs   r1,r0,#1
  strexh r3,r1,[r2]
  cbz    r3,%F3
  b      %B1
3
  bx      lr
}
#else
__STATIC_INLINE uint16_t atomic_dec16_nz (uint16_t *mem) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register uint16_t ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrexh %[ret],[%[mem]]\n\t"
    "cbnz   %[ret],2f\n\t"
    "clrex\n\t"
    "b      3f\n"
  "2:\n\t"
    "subs   %[val],%[ret],#1\n\t"
    "strexh %[res],%[val],[%[mem]]\n\t"
    "cbz    %[res],3f\n\t"
    "b      1b\n"
  "3:"
  : [ret] "=&l" (ret),
    [val] "=&l" (val),
    [res] "=&l" (res)
  : [mem] "l"   (mem)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Link Get
/// \param[in]  root            Root address
/// \return                     Link
#if defined(__CC_ARM)
static __asm    void *atomic_link_get (void **root) {
  mov   r2,r0
1
  ldrex r0,[r2]
  cbnz  r0,%F2
  clrex
  bx    lr
2
  ldr   r1,[r0]
  strex r3,r1,[r2]
  cbz   r3,%F3
  b     %B1
3
  bx     lr
}
#else
__STATIC_INLINE void *atomic_link_get (void **root) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif
  register void    *ret;

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldrex %[ret],[%[root]]\n\t"
    "cbnz  %[ret],2f\n\t"
    "clrex\n\t"
    "b     3f\n"
  "2:\n\t"
    "ldr   %[val],[%[ret]]\n\t"
    "strex %[res],%[val],[%[root]]\n\t"
    "cbz   %[res],3f\n\t"
    "b     1b\n"
  "3:"
  : [ret]  "=&l" (ret),
    [val]  "=&l" (val),
    [res]  "=&l" (res)
  : [root] "l"   (root)
  : "cc", "memory"
  );

  return ret;
}
#endif

/// Atomic Access Operation: Link Put
/// \param[in]  root            Root address
/// \param[in]  lnk             Link
#if defined(__CC_ARM)
static __asm    void atomic_link_put (void **root, void *link) {
1
  ldr   r2,[r0]
  str   r2,[r1]
  dmb
  ldrex r2,[r0]
  ldr   r3,[r1]
  cmp   r3,r2
  bne   %B1
  strex r3,r1,[r0]
  cbz   r3,%F2
  b     %B1
2
  bx    lr
}
#else
__STATIC_INLINE void atomic_link_put (void **root, void *link) {
#ifdef  __ICCARM__
#pragma diag_suppress=Pe550
#endif
  register uint32_t val1, val2, res;
#ifdef  __ICCARM__
#pragma diag_default=Pe550
#endif

  __ASM volatile (
#ifndef __ICCARM__
  ".syntax unified\n\t"
#endif
  "1:\n\t"
    "ldr   %[val1],[%[root]]\n\t"
    "str   %[val1],[%[link]]\n\t"
    "dmb\n\t"
    "ldrex %[val1],[%[root]]\n\t"
    "ldr   %[val2],[%[link]]\n\t"
    "cmp   %[val2],%[val1]\n\t"
    "bne   1b\n\t"
    "strex %[res],%[link],[%[root]]\n\t"
    "cbz   %[res],2f\n\t"
    "b     1b\n"
  "2:"
  : [val1] "=&l" (val1),
    [val2] "=&l" (val2),
    [res]  "=&l" (res)
  : [root] "l"   (root),
    [link] "l"   (link)
  : "cc", "memory"
  );
}
#endif

//lint --flb "Library End" [MISRA Note 12]
#endif /*__ATOMIC_CM_H__*/
