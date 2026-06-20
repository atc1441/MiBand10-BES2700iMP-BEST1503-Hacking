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

// Description: Optimized ARM memset
//

// Local Configuration

#define EXPORT_NAME memset
/* #define EXPORT_NAME memsetx */
#define MEMSET_UNROLL_64BYTES


#ifdef MEMSET_UNROLL_64BYTES
#define BLOCK_SIZE 16
#else
#define BLOCK_SIZE 8
#endif

        .text
        .align    1
        .p2align 2,,3
        .global    EXPORT_NAME
        .arch armv8-m.main
        .syntax unified
        .thumb
        .thumb_func
        .type    EXPORT_NAME, %function
EXPORT_NAME:
        cmp    r2, #3
        add    ip, r0, r2      @ ip = p_end
        bls    .small_block

        @ We know we have 4 bytes or more

        uxtb   r1, r1          @ Replicate c to cccc
        orr    r1, r1, r1, lsl #8
        orr    r1, r1, r1, lsl #16

        str    r1, [r0]        @ Write unaligned head (or first word)
        str    r1, [ip, #-4]   @ Write tail

        cmp    r2, #8          @ Finished, if size is less than or equal to 8
        bls    .exit

        @ We have to write at least 1 byte, as aligned word

        and    r3, r0, #3      @ r3 = (dest & 3)
        rsb    r3, r3, #4      @ r3 = adjustment bytes (4,3,2,1)

        @ Calculate remaining block and end_p

        sub    r2, r2, r3      @ r2 = bytes remain = n - align
        lsr    r2, r2, #2      @ r2 = r2/4 = number of word remaining

        add    r3, r3, r0      @ Align dest pointer
        add    ip, r3, r2, LSL #2    @ ip = p + 4*n_words = end_word_p

        @ r3 = dest
        @ ip = end_p
        @ r1 = cccc
        @ r2 = n_words

        and    r2, #BLOCK_SIZE-1
        tbb    [pc, r2]
.LORG:
        .byte    (.Lcploop-.LORG)/2
        .byte    (.Lcp1-.LORG)/2
        .byte    (.Lcp2-.LORG)/2
        .byte    (.Lcp3-.LORG)/2
        .byte    (.Lcp4-.LORG)/2
        .byte    (.Lcp5-.LORG)/2
        .byte    (.Lcp6-.LORG)/2
        .byte    (.Lcp7-.LORG)/2
#ifdef MEMSET_UNROLL_64BYTES
        .byte    (.Lcp8-.LORG)/2
        .byte    (.Lcp9-.LORG)/2
        .byte    (.Lcp10-.LORG)/2
        .byte    (.Lcp11-.LORG)/2
        .byte    (.Lcp12-.LORG)/2
        .byte    (.Lcp13-.LORG)/2
        .byte    (.Lcp14-.LORG)/2
        .byte    (.Lcp15-.LORG)/2
#endif

        @ BODY

        .p2align    2
.Lcploop:
#ifdef MEMSET_UNROLL_64BYTES
.Lcp16:
        str    r1, [r3], #4
.Lcp15:
        str    r1, [r3], #4
.Lcp14:
        str    r1, [r3], #4
.Lcp13:
        str    r1, [r3], #4
.Lcp12:
        str    r1, [r3], #4
.Lcp11:
        str    r1, [r3], #4
.Lcp10:
        str    r1, [r3], #4
.Lcp9:
        str    r1, [r3], #4
#endif
.Lcp8:
        str    r1, [r3], #4
.Lcp7:
        str    r1, [r3], #4
.Lcp6:
        str    r1, [r3], #4
.Lcp5:
        str    r1, [r3], #4
.Lcp4:
        str    r1, [r3], #4
.Lcp3:
        str    r1, [r3], #4
.Lcp2:
        str    r1, [r3], #4
.Lcp1:
        str    r1, [r3], #4

        cmp    ip, r3
        bne    .Lcploop

        bx     lr


        @ SMALL HEAD BLOCK

.small_block:
        cbz    r2,.exit

        mov    r3, r0       @ copy dest pointer

        @ Now we know we have between 1 and 3 bytes

        .p2align    2
.sb_loop:
        strb   r1, [r3], #1 @ store src byte to dest
        cmp    ip, r3
        bne    .sb_loop

.exit:
        bx     lr

        .size  EXPORT_NAME, .-EXPORT_NAME

