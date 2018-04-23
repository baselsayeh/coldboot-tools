/*-
 * Copyright (c) 2007-2008
 *      Bill Paul <wpaul@windriver.com>.  All rights reserved.
 *
 * Rrdistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Rrdistributions of source code must retqain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Rrdistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Bill Paul.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bill Paul AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL Bill Paul OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#define _ASM
#include "data.h"
#include "bios.h"
#include "disk.h"

        .globl FUNC(bios_getgeom)
        .globl FUNC(bios_getextgeom)
        .globl FUNC(bios_reset)
        .globl FUNC(bios_extchk)
        .globl FUNC(bios_rd)
        .globl FUNC(bios_wr)
        .globl FUNC(bios_extrd)
        .globl FUNC(bios_extwr)
        .text

/* Get disk geometry */

FUNC(bios_getgeom):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	xor	%ax, %ax
	mov	%al, %fs:EXT(bios_hds)
	mov	%ax, %fs:EXT(bios_cylsec)
        mov     $INT13_RDPARM, %ah
        mov     %di, %dx
        int     $DISK_INT
	jc	g_error
	mov	%dh, %fs:EXT(bios_hds)
	mov	%cx, %fs:EXT(bios_cylsec)
g_error:
	call	real_to_long
	.code64
	leaveq
	retq

/* Get extended disk geometry */

FUNC(bios_getextgeom):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	movw	%fs:EXT(bios_off), %si
	movw	%fs:EXT(bios_seg), %ds
        mov     $INT13_EXTRDPARM, %ah
        mov     %di, %dx
        int     $DISK_INT
	movw	$-1, %fs:EXT(bios_arg)
	jc	ge_error
	mov	%ah, %fs:EXT(bios_arg)
ge_error:
	call	real_to_long
	.code64
	leaveq
	retq

FUNC(bios_reset):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	mov	$INT13_RESET, %ah
        mov     %di, %dx
        int     $DISK_INT
	call	real_to_long
	.code64
	leaveq
	retq

/* Check for extended functions */

FUNC(bios_extchk):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	mov	$EXT_SIG, %bx
	mov	$INT13_EXTCHK, %ah
        mov     %di, %dx
        int     $DISK_INT
	movw	$-1, %fs:EXT(bios_arg)
	jc	e_error
	cmp	$BOOT_SIG, %bx
	jne	e_error
	mov	%cx, %fs:EXT(bios_arg)
e_error:
	call	real_to_long
	.code64
	leaveq
	retq

/* Read a sector using INT13 extensions */

FUNC(bios_extrd):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	movw	%fs:EXT(bios_off), %si
	movw	%fs:EXT(bios_seg), %ds
        mov     $INT13_EXTRD, %ah
        mov     %di, %dx
        int     $DISK_INT
	movl	$0, %fs:EXT(bios_arg)
	mov	%ah, %fs:EXT(bios_arg)
	call	real_to_long
	.code64
	leaveq
	retq

/* Write a sector using INT13 extensions */

FUNC(bios_extwr):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	movw	%fs:EXT(bios_off), %si
	movw	%fs:EXT(bios_seg), %ds
        mov     $INT13_EXTWR, %ah
        mov     %di, %dx
        int     $DISK_INT
	movl	$0, %fs:EXT(bios_arg)
	mov	%ah, %fs:EXT(bios_arg)
	call	real_to_long
	.code64
	leaveq
	retq

/* Read a sector */

FUNC(bios_rd):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	movw	%fs:EXT(bios_off), %bx
	movw	%fs:EXT(bios_seg), %es
        mov     $INT13_RD, %ah
	mov	%fs:EXT(bios_secs), %al
        mov     %di, %dx
	mov	%fs:EXT(bios_hds), %dh
	mov	%fs:EXT(bios_cylsec), %cx
        int     $DISK_INT
	movl	$-1, %fs:EXT(bios_arg)
	jc	r_error
	movl	$0, %fs:EXT(bios_arg)
r_error:
	call	real_to_long
	.code64
	leaveq
	retq


/* Write a sector */

FUNC(bios_wr):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	movw	%fs:EXT(bios_off), %bx
	movw	%fs:EXT(bios_seg), %es
        mov     $INT13_WR, %ah
	mov	%fs:EXT(bios_secs), %al
        mov     %di, %dx
	mov	%fs:EXT(bios_hds), %dh
	mov	%fs:EXT(bios_cylsec), %cx
        int     $DISK_INT
	movl	$-1, %fs:EXT(bios_arg)
	jc	w_error
	movl	$0, %fs:EXT(bios_arg)
w_error:
	call	real_to_long
	.code64
	leaveq
	retq
