/*
 * ARM assembly optimized color format conversion functions
 * (YV12 -> YUY2)
 *
 * Copyright (C) 2007 Siarhei Siamashka <ssvb@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA
 */
        .text

/*******************************************************/

        .align
        .global yv12_to_yuy2_line_arm
        .func yv12_to_yuy2_line_arm
yv12_to_yuy2_line_arm:

#define DST     r0
#define SRC_Y   r1
#define SRC_U   r2
#define SRC_V   r3
#define WIDTH   ip

        ldr     ip, [sp], #0
        stmfd   sp!, {r4-r8, r10, lr}

#define TMP1    r8
#define TMP2    r10
#define TMP3    lr

        bic     WIDTH, #1

        subs    WIDTH, #8
        blt     2f
1:
        ldrb    r4, [SRC_Y], #1
        ldrb    TMP1, [SRC_U], #1
        ldrb    TMP2, [SRC_Y], #1
        ldrb    TMP3, [SRC_V], #1
        add     r4, r4, TMP1, lsl #8
        add     r4, r4, TMP2, lsl #16
        add     r4, r4, TMP3, lsl #24

        ldrb    r5, [SRC_Y], #1
        ldrb    TMP1, [SRC_U], #1
        ldrb    TMP2, [SRC_Y], #1
        ldrb    TMP3, [SRC_V], #1
        add     r5, r5, TMP1, lsl #8
        add     r5, r5, TMP2, lsl #16
        add     r5, r5, TMP3, lsl #24

        ldrb    r6, [SRC_Y], #1
        ldrb    TMP1, [SRC_U], #1
        ldrb    TMP2, [SRC_Y], #1
        ldrb    TMP3, [SRC_V], #1
        add     r6, r6, TMP1, lsl #8
        add     r6, r6, TMP2, lsl #16
        add     r6, r6, TMP3, lsl #24

        ldrb    r7, [SRC_Y], #1
        ldrb    TMP1, [SRC_U], #1
        ldrb    TMP2, [SRC_Y], #1
        ldrb    TMP3, [SRC_V], #1
        add     r7, r7, TMP1, lsl #8
        add     r7, r7, TMP2, lsl #16
        add     r7, r7, TMP3, lsl #24

        stmia   DST!, {r4-r7}
        subs    WIDTH, WIDTH, #8
        bge     1b
2:
        adds    WIDTH, WIDTH, #8
        ble     4f
3:
        ldrb    r4, [SRC_Y], #1
        ldrb    TMP1, [SRC_U], #1
        ldrb    TMP2, [SRC_Y], #1
        ldrb    TMP3, [SRC_V], #1
        add     r4, r4, TMP1, lsl #8
        add     r4, r4, TMP2, lsl #16
        add     r4, r4, TMP3, lsl #24
        str     r4, [DST], #4
        subs    WIDTH, WIDTH, #2
        bgt     3b
4:
        ldmfd  sp!, {r4-r8, r10, pc}

#undef  DST
#undef  SRC_Y
#undef  SRC_U
#undef  SRC_V
#undef  WIDTH
#undef  TMP1
#undef  TMP2
#undef  TMP3

        .endfunc

/*******************************************************/

.endm
