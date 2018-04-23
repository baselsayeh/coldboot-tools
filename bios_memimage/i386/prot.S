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

/*
 * This module handles switching back and forth between real mode and
 * protected mode. The assumption is that we'll be running in protected
 * mode most of the time, and will thunk into real mode only long enough
 * to call a BIOS or PXE routine. During the thunk, we'll preserve most
 * of the registers on the protected mode stack (except for %eax, which
 * is generally assumed to be volatile).
 *
 * When entering real mode, we set up a special real mode stack and
 * data segment. This is so that the real mode code (primarily PXE)
 * can use stack and data space without potentially colliding with
 * the stack and data used in protected mode. The protected mode stack
 * is located at 0x6C00. The real mode stack is placed at 0x7C00.
 * The real mode data segment is placed at 0x4000. We leave the %fs
 * segment register set to segment 0 so that we can use it from
 * real mode to access storage used from protected mode. The assumption
 * The assumption is that the real mode code won't use this segment 
 * register. I don't know if this is guaranteed, but there haven't 
 * been any problems with it so far.
 */

#define _ASM
#include "data.h"

	.globl prot_to_real
	.globl real_to_prot
	.text


/*
 * Switch to real mode from protected mode.
 * %eax is destroyed, all other registers are preserved.
 */

prot_to_real:
	/* Grab the return address off the stack. */
	pop	%eax
	/* Save all registers */
	pushal
	/* Save protected mode stack pointer. */
	mov	%esp, EXT(saved_stack)
	mov	%eax, %esi /* Save return address here */
	ljmpw	$GDT_RCODE, $real1
real1:
	/*
	 * Set segments for real mode.
	 * Note: this is required, because the CPU caches the
	 * segment/selector register values and will retain them
	 * even after we've switched back to running in a 16-bit
	 * code segment. (This is known as "unreal mode," and can
	 * be used to allow real mode code to access 4GB of
	 * address space.)
	 */
	.code16
	mov	$GDT_RDATA, %ax
        mov	%ax, %ds
        mov	%ax, %es
        mov	%ax, %fs
        mov	%ax, %gs
        mov	%ax, %ss

	/* Clear PE bit in CR0 */
	mov	%cr0, %eax
	and	$~CR0_PE, %eax
	mov	%eax, %cr0

	/* Switch to real mode */
	ljmp	$0, $real2
real2:
	/* Give real mode its own data segment */
	mov	$DATA_REAL, %ax
        mov	%ax, %ds
        mov	%ax, %es
        mov	%ax, %gs

	/* Leave %fs alone so we can use it. */

	xorw	%ax, %ax
        mov	%ax, %fs

	/* Set up the real mode stack. */
	mov	$STACK_REAL_SEG, %ax
        mov	%ax, %ss
	mov	$STACK_REAL_OFF, %sp

	/* Put the return address onto the real mode stack. */

	push	%si
	ret

	/*
 	 * The following two NOP instructions are here to work
	 * around what appears to be a bug in the Cygwin assembler
	 * and/or linker. For some reason, code in the bios.S and
	 * pxecall.S modules that calls the real_to_prot() function
	 * below ends up branching to an address two bytes _before_
         * the start of real_to_prot(). This corresponds to the
	 * 'push %si' instruction above. It's unclear why this
	 * happens, but the two NOP instructions mitigate the
	 * problem (even though the branch destination is still
         * wrong, the code falls through to the real_to_prot()
	 * entry below).
	 *
	 * When building the code with an assembler/linker that
	 * calculates the branch offset correctly, these two NOPs
	 * are never executed.
	 */

	nop
	nop

/*
 * Switch from protected mode to real mode.
 * %eax is destroyed, all other registers preserved.
 */

real_to_prot:
	cli
	/* Grab return address off the stack */
	pop	%si

	/* Reload GDT, in case it's been modified. */

	lgdtw	%fs:EXT(GDESC)

        /* Set PE bit in CR0 */

        mov     %cr0, %eax
        or      $CR0_PE, %eax
        mov     %eax, %cr0

        /* Do a jump to flush the instruction pipe */
        jmp     prot1
prot1:

	/* Load segments for protected mode */
	mov	$GDT_PDATA, %ax
        mov	%ax, %ds
        mov	%ax, %es
        mov	%ax, %fs
        mov	%ax, %gs
        mov	%ax, %ss

	/* Return to protected mode (sets %cs) */
	ljmp	$GDT_PCODE, $prot2

prot2:
	.code32
	/* Restore the protexted mode stack */
	mov	EXT(saved_stack), %esp
	/* Temporarily save return address. */
	mov	%esi, EXT(saved_stack)
	/* Restore all registers */
	popal
	mov	EXT(saved_stack), %eax
	push	%eax
	ret
