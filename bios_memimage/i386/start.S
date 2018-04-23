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
 * Bootstrap code, enter protected mode.
 */

	.globl	boot
	.text

boot:
	.code16
	cli
	cld

	/* Clear segment registers */

        ljmp    $0, $setcs
setcs:
        xor     %ax, %ax
        mov     %ax, %ds
        mov     %ax, %ss

        /* Set up a temporary stack. */

        mov     $boot, %sp

#ifdef notdef
	/*
         * See if BIOS supports 'fast' A20 method.
         * Note: this BIOS call, at least on my laptop, has been
	 * seen to cause a small amount of memory corruption.
	 * Until I can figure out why, we'll skip the shortcut
	 * method and always use the keyboard controller
	 * method.
	 */

	mov	$0x2403, %ax
	int	$0x15
	jc	no_fast
	cmp	$0, %ah
	jne	no_fast
	and	$2, %bx
	je	no_fast

	/* Attempt to set A20 line. */

	inb	$0x92, %al
	testb	$2, %al
	jnz	A20alreadyset
	orb	$2, %al
	and	$0xFE, %al
	outb	%al, $0x92
	jmp	A20alreadyset

no_fast:
#endif

	/* Issue command to read output port. */

	call	kbd_wait
	mov	$0xD0, %al
	outb	%al, $0x64

	/* Wait until output data ready. */
no_output:
	inb	$0x64, %al
	test	$1, %al
	jz	no_output

	/* Read data */

	inb	$0x60, %al

	/* Set A20 enable bit */

	or	$2, %al
	mov	%al, %ah

	/* Issue command to write output port */

	call	kbd_wait
	mov	$0xD1, %al
	outb	%al, $0x64

	/* Write command to enable A20. */

	call	kbd_wait
	mov	%ah, %al
	outb	%al, $0x60

	/* Wait for completion. */

	call	kbd_wait

A20alreadyset:

	/* Clear segment registers */
	xor	%ax, %ax
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs

	/* Save BIOS disk device info */

	mov	%dl, EXT(bios_disk)

	/* Load GDT */

	lgdtw	%fs:EXT(GDESC)

	/* Set PE bit in CR0 and enable the cache */
	mov	%cr0, %eax
	or	$CR0_PE, %eax
	and	$~CR0_CD, %eax
	mov	%eax, %cr0

	/* Do a jump to flush the instruction pipe */
	jmp	s1
s1:

	/* Load segments for protected mode */
	mov	$GDT_PDATA, %ax
	mov	%ax, %ds
	mov	%ax, %es
	mov	%ax, %fs
	mov	%ax, %gs
	mov	%ax, %ss

	/* Do a long jump to enter protected mode (sets %cs). */

	ljmp	$GDT_PCODE, $s2
s2:
	/* Now we're in protected mode */

	.code32
	/* Initialize the stack */

	mov	$STACK_PROT, %esp

	/* Jump to C entry point. */

	call	FUNC(_main)

#ifndef SILENT
	/* If the main routine returns, reboot the system. */

	push	$reboot
	call	FUNC(printf)
#endif

	/* Try to do a powerdown first. */

	call	FUNC(bios_apm_shutdown)

	/*
	 * If that didn't work, triple fault the CPU to reset.
	 * Load an empty IDT and then trigger an illegal instruction
         * interrupt.
	 */

	sti
	lidt	EXT(IDESC)
	int	$0x6

	/* If that didn't work, just hang. */

stop:
	hlt
	jmp	stop

/*
 * Wait until i8042 keyboard controller is
 * ready to accept a command or data byte
 */

	.code16
delay0:
	jmp	d1
d1:
	inb	$0x60, %al
kbd_wait:
	jmp	d2
d2:
	inb	$0x64, %al
	test	$1, %al
	jnz	delay0
	test	$2, %al
	jnz	kbd_wait
	ret
	.code32

#ifndef SILENT
	.data
reboot:
	.string "Rebooting...\n"
#endif
