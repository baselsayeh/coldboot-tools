/*-
 * Copyright (c) 2007-2008
 *      Bill Paul <wpaul@windriver.com>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
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

        .globl FUNC(bios_memmap)
        .globl FUNC(bios_memmap2)
        .globl FUNC(bios_putchar)
        .text

/*
 * Make a call to int0x15:E820 to read the memory map
 */

FUNC(bios_memmap):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	xorw	%ax, %ax
	mov	%ax, %es	/* Need to set %es to segment 0 for this call */
	mov	$EXT(bios_smap), %di
	mov	$0xE820, %ax
	mov	%fs:EXT(bios_next), %ebx
	mov	$SMAP_SIG, %edx
	mov	$20, %cx
	int	$0x15
	mov	%ebx, %fs:EXT(bios_next)
fail:
	call	real_to_long
	.code64
	xor	%rax, %rax
	mov	EXT(bios_next), %eax
	leaveq
	retq

/*
 * For x86-64, this routine is just a stub, because all x86-64 BIOSes
 * support the standard memmap routine above.
 */

FUNC(bios_memmap2):
	xor	%rax, %rax
	retq

/*
 * Write a character to the console.
 */

FUNC(bios_putchar):
	push	%rbp
	mov	%rsp, %rbp
	call	long_to_real
	.code16
	mov	%di, %ax
	mov	$0x0E, %ah
	int	$0x10
	call	real_to_long
	.code64
	leave
	ret
