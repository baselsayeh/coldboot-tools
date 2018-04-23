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
	.globl FUNC(bios_apm_shutdown)
        .text

/*
 * Make a call to int0x15:E820 to read the memory map
 */

FUNC(bios_memmap):
	push	%ebp
	mov	%esp, %ebp
	call	prot_to_real
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
	call	real_to_prot
	.code32
	mov	EXT(bios_next), %eax
	leave
	ret

/*
 * Make a call to int0x15:E801 to read old style memory map
 */

FUNC(bios_memmap2):
	push	%ebp
	mov	%esp, %ebp
	call	prot_to_real
	.code16
	mov	$0xE801, %eax
	int	$0x15
	mov	%ax, %fs:EXT(bios_ext1)
	mov	%bx, %fs:EXT(bios_ext2)
	call	real_to_prot
	.code32
	leave
	ret

/*
 * Write a character to the console.
 */

FUNC(bios_putchar):
	push	%ebp
	mov	%esp, %ebp
	mov	8(%esp), %eax
	push	%edi
	mov	%eax, %edi
	call	prot_to_real
	.code16
	mov	%di, %ax
	mov	$0x0E, %ah
	int	$0x10
	call	real_to_prot
	.code32
	pop	%edi
	leave
	ret

/*
 * Attempt to perform an APM poweroff.
 *
 * Return values:
 * 1: APM BIOS not present
 * 2: connect failed
 * 3: register failed
 * 4: enable failed
 * 5: shutdown failed
 */

FUNC(bios_apm_shutdown):
	push	%ebp
	mov	%esp, %ebp
	call	prot_to_real
	.code16


	/* See if APM is available. */
	mov	$(APM_BIOS << 8 | APM_INSTCHECK), %ax
	mov	$PMDV_APMBIOS, %bx
	int	$APM_INT
	mov	$0x1, %ax	/* error code 1 */
	jc	error
	cmp	$0x50, %bh	/* P */
	jne	error
	cmp	$0x4D, %bl	/* M */
	jne	error
	mov	%ax, %fs:apm_ver /* save APM version */

	/* Connect APM BIOS real mode interface */ 
	mov	$(APM_BIOS << 8 | APM_REALCONNECT), %ax
	mov	$PMDV_APMBIOS, %bx
	int	$APM_INT
	mov	$0x2, %ax	/* error code 2 */
	jc	error

	/*
	 * For APM BIOS version 1.1 or higher, we must indicate
 	 * our APM support. If we don't do this, power off support
	 * won't work.
	 */

	mov	%fs:apm_ver, %cx
	cmp	$0x1, %cl	/* If minor version is less than 1, */
	jl	skip		/* skip the next step. */

	/* Register with power management */
	mov	$(APM_BIOS << 8 | APM_DRVVERSION), %ax
	mov	$PMDV_APMBIOS, %bx
	int	$APM_INT
	mov	$0x3, %ax	/* error code 3 */
	jc	error

skip:

	/* Enable power management */
	mov	$(APM_BIOS << 8 | APM_ENABLEDISABLEPM), %ax
	mov	$PMDV_ALLDEV, %bx
	mov	$0x1, %cx	/* 1 == enable, 0 == disable */
	int	$APM_INT
	mov	$0x4, %ax	/* error code 4 */
	jc	error

        /* set APM power state */
	mov	$(APM_BIOS << 8 | APM_SETPWSTATE), %ax
	mov	$PMDV_ALLDEV, %bx	/* all devices */
	mov	$PMST_OFF, %cx	/* off */
	int	$APM_INT
	mov	$0x5, %ax	/* error code 5 */
	jc	error

	/* Success -- should never be reached */

	mov	$0x0, %ax

error:
	mov	%ax, %fs:apm_result
	call	real_to_prot
	.code32
	mov	apm_result, %eax
	leave
	ret

	.data

apm_result:
	.long	0x00000000
apm_ver:
	.word	0x0000
