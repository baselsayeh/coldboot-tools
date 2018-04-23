
#define _ASM
#include "data.h"

/*
 * Note: all code in this module should be position independent.
 */

	.globl prot_to_real
	.globl real_entry
	.globl x86_getgdt
	.globl x86_setgdt
	.text

x86_getgdt:
        movl    4(%esp),%eax
        sgdtl   (%eax)
        xor     %eax,%eax
        ret

x86_setgdt:
        movl    4(%esp),%eax
        lgdt    (%eax)
        js     1f
        nop
1:
        xor     %eax,%eax
        ret

/*
 * Switch from 32 bit segment to 16 bit segment
 */

prot_to_real:
	ljmpw	$GDT_APPLERCODE, $BIOS_ENTRY


/*
 * This code is manually copied by the loader to address 0x7C00,
 * which is the usual place where PC BIOSes load primary bootstrap
 * code. When we enter it, we'll be running in a 16 bit segment,
 * but the PE bit in CR0 will still be set. This is actually
 * the second part of the two stage 'protected mode to real mode'
 * transition process. We have to be running in 16 bit mode before
 * we can clear the PE bit, and then once we've done that we can
 * do an inter-segment jump to segment 0, which will return us
 * to real mode.
 */

real_entry:
	/* Set segments for real mode */
	.code16
	mov	$GDT_APPLERDATA, %ax
        mov	%ax, %ds
        mov	%ax, %es
        mov	%ax, %fs
        mov	%ax, %gs
        mov	%ax, %ss

	/* Clear PE bit in CR0 */

	mov	%cr0, %eax
	and	$0xFFFFFFFE, %eax
	mov	%eax, %cr0

	/* Switch to real mode */

	ljmp	$0, $(BIOS_ENTRY + (real1 - real_entry))

real1:
	xor	%ax, %ax
        mov	%ax, %ds
        mov	%ax, %es
        mov	%ax, %fs
        mov	%ax, %gs
        mov	%ax, %ss

	/* Set up a dummy stack */

	mov	$0x7B00, %sp

	/* Jump to bootrom */

	mov	$VXWORKS_ENTRY, %ax
	jmp	*%ax
