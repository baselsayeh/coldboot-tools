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

/*
 * This is an optimized bcopy() routine which only copies data in
 * 32 bit chunks. When we snarf a page of memory, we always do it in
 * at least 1024 byte chunks, and the source and destination addresses
 * are always 32-bit aligned. We may as well take advantage of this.
 * We can realize a significant performance gain if we use this
 * compared to the C version of bcopy().
 */

	.globl FUNC(bcopy_long)
	.globl FUNC(bswap16)
	.globl FUNC(bswap32)
	.globl FUNC(bswap64)
	.globl FUNC(mapmem)
	.text

FUNC(bcopy_long):

	push	%ebp
	mov	%esp, %ebp
	push	%esi	
	push	%edi
	mov	16(%esp), %esi
	mov	20(%esp), %edi
	mov	24(%esp), %ecx
	sar	$0x2, %ecx
	rep
	movsl
	pop	%edi
	pop	%esi
	leave
	ret

FUNC(bswap16):
	mov	4(%esp), %ax
	xchgb	%ah, %al
	ret

FUNC(bswap32):
	mov	4(%esp), %eax
	bswap	%eax
	ret

FUNC(bswap64):
	mov     4(%esp), %edx
	mov	8(%esp), %eax
	bswap	%eax
	bswap	%edx
	ret

/*
 * Not needed for i386 arch
 */

FUNC(mapmem):
	ret
