/***************************************************************************
 *
 * Copyright 2015-2021 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/

//
// Description: Optimized ARM memmove and version of memcpy
//              that can copy overlapping data down
//

// =========================================================
// Local Configuration

#define EXPORT_MEMMOVE_NAME          memmove
#define EXPORT_MEMCPY_NAME           memcpy

#define MEMCPY_IS_MEMMOVE    // if defined: memcpy and memmove both works like memmove
#define HAVE_FAST_MEMCPY       // if defined: dedicated (fast) non-overlap memcpy
#define MEMCPY_UNROLL_64BYTES  // ... else 32 bytes.


// =========================================================
// Exports

// =========================================================
// memcpy - assume non-overlapping src and dest.
//          copy from low memory to high (incrementing)
// =========================================================
        .global   EXPORT_MEMCPY_NAME
        .type     EXPORT_MEMCPY_NAME, %function

// =========================================================
// memmove - test for overlapping src and dest. call fastest
//           memory copy that work for overlap/no-overlap
// =========================================================
        .global   EXPORT_MEMMOVE_NAME
        .type     EXPORT_MEMMOVE_NAME, %function


// =========================================================
// Internal:

#ifdef MEMCPY_UNROLL_64BYTES
#define BLOCK_SIZE 16
#else
#define BLOCK_SIZE 8
#endif

#define CPY_STEP(rdst, rs2d, rtmp, inc)         \
        ldr    rtmp, [rdst, rs2d] ;             \
        str    rtmp, [rdst], #(inc)


// =========================================================
// MACROS

.macro BLOCK_COPY r_dest, r_end, r_dest2src, r_temp, inc, block_size
        and    \r_temp, #\block_size-1
        tbb    [pc, \r_temp]
1:
        .byte    (20f - 1b)/2
        .byte    (1f - 1b)/2
        .byte    (2f - 1b)/2
        .byte    (3f - 1b)/2
        .byte    (4f - 1b)/2
        .byte    (5f - 1b)/2
        .byte    (6f - 1b)/2
        .byte    (7f - 1b)/2
.if \block_size == 16
        .byte    (8f - 1b)/2
        .byte    (9f - 1b)/2
        .byte    (10f - 1b)/2
        .byte    (11f - 1b)/2
        .byte    (12f - 1b)/2
        .byte    (13f - 1b)/2
        .byte    (14f - 1b)/2
        .byte    (15f - 1b)/2
.endif
        @ BODY
        .p2align 2
20:
.if \block_size == 16
16:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
15:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
14:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
13:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
12:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
11:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
10:     CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
9:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
.endif
8:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
7:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
6:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
5:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
4:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
3:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
2:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
1:      CPY_STEP(\r_dest, \r_dest2src, \r_temp, \inc)
        cmp    \r_end, \r_dest
        bne    20b
.endm


// FUNCTIONS

// MEMMOVE Entry:

// =========================================================
        .text
        .align    1
        .p2align  2,,3
        .arch     armv8-m.main
        .syntax   unified
        .thumb
        .thumb_func
// =========================================================
// memmove - test for overlapping src and dest. call fastest
//           memory copy that work for overlap/no-overlap
// =========================================================
#ifdef MEMCPY_IS_MEMMOVE
EXPORT_MEMCPY_NAME:
#endif
EXPORT_MEMMOVE_NAME:
// =========================================================
        @ Check if overlapping copy, moving up or down
#if 1   // Optimized
	    subs     r3, r1, r0       @ t = src - dest
	    bls     .L_src_lower      @ src <= dest?

        cmp     r3, r2            @ src - dest < len?
	    blo	    .L_memmove_inc
//        b	    .L_memcpy         @ no overlap (r3>=len , +len, still >= len)
.L_src_lower:
        @ if there is no dedicated fast memcpy, then use memmove
#ifdef HAVE_FAST_MEMCPY
        adds    r3, r2            @ (src - dest) + len <= 0  ( => len  <= dest -src)
	    bls	    .L_memcpy         @ no overlap
#endif

#else   // Non-Optimized
        cmp     r1, r0            @ src <= dest?
	    bls     .L_src_lower

.L_src_higher:
	    adds	r3, r0, r2        @ dend = dest + len
	    cmp     r3, r1            @ dend >= src
	    bhs	    .L_memmove_inc

.L_src_lower:
#ifdef HAVE_FAST_MEMCPY
        @ if there is no dedicated fast memcpy, then use memmove
	    adds    r3, r1, r2        @ send = src + len
	    cmp     r3, r0            @ send <= dest
	    bls	    .L_memcpy         @ no overlap
#endif
#endif
// Fall through...

// =========================================================
// Copy to higher address, addresses here will decrement
.L_memmove_dec:
		add    r3, r0, r2      @ copy end of dest pointer
        and    ip, r3, #3      @ r3 = adjustment bytes (0,1,2,3)

        sub    r3, #1          @ pre-decrement destination-end
        sub    r1, r1, r0      @ Calculate source to dest distance

        cmp    r2, #3          @ Small block?
        it     ls
        movls  ip, r2          @  then move small block
        subs   r2, ip          @ update remaing bytes (len -= align)

        tbb    [pc, ip]
1:
        .byte (10f - 1b)/2
        .byte (11f - 1b)/2
        .byte (12f - 1b)/2
        .byte (13f - 1b)/2
        .align    1
13:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #-1   @ store src byte to dest
12:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #-1   @ store src byte to dest
11:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #-1   @ store src byte to dest
10:
        @ r0: dest unchanged
        @ r1: dest to source distance
        @ r2: (len) remaining bytes
        @ r3: aligned end of destination, or len == 0

		beq    99f             @ return if done

        @ Copy aligned blocks

        bic    r3, #3          @ r3 = (dest & ~3)
        cmp    r2, #4          @ Finished, if size is less than or equal to 4
        blo    .Lcopy_dec_tail

        @ Calculate remaining block and end_p

        lsr    r2, r2, #2          @ r2 = r2/4 = number of word remaining
        sub    ip, r3, r2, LSL #2

        @ r0: dest unchanged
        @ r1: dest to source distance
        @ r2: n_words
        @ r3: dest_end_align
        @ ip: dest_start_p  (like r0, but aligned)

        BLOCK_COPY r3, ip, r1, r2, -4, BLOCK_SIZE

		tst   r0, #3          @ if dest is aligned, we are finished
		beq   99f

.Lcopy_dec_tail:
		rsb    r2, r0, #0     @ dest alignment (...0 -> 0, ...1 -> 3, ...2 -> 2, ...)
		and    r2, #3         @ calculate dest alignment

        @ need:
        @ r0: unchanged dest
        @ r1: s2d difference
        @ r2: remaing bytes
        @ r3: aligned dest, next to write

        mvn    ip, #0         @ 0xffffffff
        lsls   r2, #3         @ dest_align * 8
        lsr    ip, r2         @ ip = -1 >> (8 * n) Create mask 0x00ffffff

        @ ip: mask - 00 source bytes, ff dest bytes
        ldr    r1, [r3, r1]   @ load remaing source bytes
        ldr    r2, [r3]       @ load dest word
        eor    r2, r1         @ merge source into dest word
        and    r2, ip
        eor    r2, r1
        str    r2, [r3]
99:
        bx     lr


        .align    1
        .p2align  2,,3

// =========================================================
// Copy to lower address, addresses here will increment
.L_memmove_inc:
        mov    r3, r0          @ copy dest pointer
        rsb    ip, r0, #0      @ r3 = adjustment bytes (0,3,2,1)
        and    ip, #3          @ r3 = (dest & 3)

        sub    r1, r1, r0      @ Calculate source to dest distance

        cmp    r2, #3          @ Small block?
        it     ls
        movls  ip, r2          @  then move small block
        subs   r2, ip          @ update remaing bytes (len -= align)

        tbb [pc, ip]
1:
        .byte (10f - 1b)/2
        .byte (11f - 1b)/2
        .byte (12f - 1b)/2
        .byte (13f - 1b)/2
        .align    1
13:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #1    @ store src byte to dest
12:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #1    @ store src byte to dest
11:
        ldrb   ip, [r3, r1]    @ next src byte
        strb   ip, [r3], #1    @ store src byte to dest
10:

        @ r0: dest unchanged
        @ r1: dest to source distance
        @ r2: (len) remaining bytes
        @ r3: aligned destination, or len == 0

		beq    99b             @ return if done

        @ Copy aligned blocks

        cmp    r2, #4          @ Finished, if size is less than or equal to 4
        blo    .Lcopy_inc_tail

        push   {r2}            @ Save original remaining length

        @ Calculate remaining block and end_p

        lsr    r2, r2, #2      @ r2 = r2/4 = number of word remaining
        add    ip, r3, r2, LSL #2  @ ip = p + 4*n_words = end_word_p

        @ r0: dest unchanged
        @ r1: dest to source distance
        @ r2: n_words
        @ r3: dest_align   (like r0, but aligned)
        @ ip: dest_end_p   (dest_align + n_words)

        BLOCK_COPY r3, ip, r1, r2, 4, BLOCK_SIZE

        pop    {r2}          @ get original length
        ands   r2,#3         @ if remaining length is aligned, we are finished
		beq   99f

.Lcopy_inc_tail:
        @ need:
        @ r0: unchanged dest
        @ r1: s2d difference
        @ r2: remaing bytes
        @ r3: aligned dest, next to write

        mvn    ip, #0        @ 0xffffffff
        lsls   r2, #3        @ n * 8
        lsl    ip, r2        @ ip = -1 << (8 * n) Create mask 0xffffff00

        @ ip: mask - 00 source bytes, ff dest bytes
        ldr    r1, [r3, r1]   @ load remaing source bytes
        ldr    r2, [r3]       @ load dest word
        eor    r2, r1         @ merge source into dest word
        and    r2, ip
        eor    r2, r1
        str    r2, [r3]
99:
        bx     lr
        .size  EXPORT_MEMMOVE_NAME, .-EXPORT_MEMMOVE_NAME
// =========================================================




#ifdef HAVE_FAST_MEMCPY
        .p2align  2
// =========================================================
// memcpy - assume non-overlapping src and dest.
//          copy from low memory to high (incrementing)
// =========================================================
#ifndef MEMCPY_IS_MEMMOVE
EXPORT_MEMCPY_NAME:
#endif
// =========================================================
.L_memcpy:
        cmp    r2, #4
        bhi    .Large_block

// SMALL HEAD BLOCK
        cbz    r2, 99f

        mov    r3, r0        @ copy dest pointer
        add    ip, r2, r0    @ calculate end of dest
        ldr    r1, [r1]      @ load 4 bytes from src

        @ Now we know we have between 1 and 4 bytes
        .p2align 2
1:
        strb   r1, [r3], #1 @ store src byte to dest
        lsr    r1, #8       @ next src byte
        cmp    r3, ip
        bne    1b

99:     bx     lr

// =========================================================
.Large_block:

        @ We know we have 4 bytes or more

        ldr    r3, [r1]        @ Copy head
        str    r3, [r0]

        sub    ip, r2, #4
        ldr    r3, [r1, ip]    @ Copy tail
        str    r3, [r0, ip]

        cmp    r2, #8          @ Finished, if size is less than or equal to 8
        bls    99b

        sub    r1, r0          @ Calculate source to dest distance

        @ Calculate remaining block and end_p

        and    r3, r0, #3      @ r3 = (dest & 3)
        rsb    r3, #4          @ r3 = adjustment bytes (4,3,2,1)
        sub    r2, r3          @ r2 = bytes remain = n - align

        lsr    r2, #2          @ r2 = r2/4 = number of word remaining

        add    r3, r0          @ Align dest pointer
        add    ip, r3, r2, LSL #2  @ ip = p + 4*n_words = end_word_p

        @ r0: dest unchanged
        @ r1: dest to source distance
        @ r2: n_words
        @ r3: dest_align   (like r0, but aligned)
        @ ip: dest_end_p   (dest_align + n_words)

        BLOCK_COPY r3, ip, r1, r2, 4, BLOCK_SIZE

        bx     lr
        .size  EXPORT_MEMCPY_NAME, .-EXPORT_MEMCPY_NAME
// =========================================================
#endif

