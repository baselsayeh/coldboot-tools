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
#include "disk.h"

#define _SEG(x)          (((x) & 0xF0000) >> 4)
#define _OFF(x)          ((x) & 0xFFFF)

	.code16

	.set PRT_OFF,		DOSPARTOFF              # Partition table

/* Information about the 4th entry in the partition table. */

	.set PRT4_START,	0x7DEE
	.set PRT4_FLAGS,	0x7DEE
	.set PRT4_STARTHD,	0x7DEF
	.set PRT4_STARTSEC,	0x7DF0
	.set PRT4_STARTCYL,	0x7DF1
	.set PRT4_TYPE,		0x7DF2
	.set PRT4_ENDHD,	0x7DF3
	.set PRT4_ENDSEC,	0x7DF4
	.set PRT4_ENDCYL,	0x7DF5
	.set PRT4_START,	0x7DF6
	.set PRT4_SIZE,		0x7DFA

/*
 * Bootstrap code.
 * This code resides at sector 0 on a disk and is loaded by the BIOS
 * to 0x7c00 during bootstrap. Once we're running, we use the BIOS
 * to load an additional group of sectors (up to 32) from the disk
 * to 0x7e00, which is where the real memory dumper code will live.
 * As soon as the code is loaded, we branch to 0x7e00 to launch it.
 * When we're called, the BIOS will pass us the boot disk number
 * (0x0 for floppies/USB, 0x80 for hard disk) in %dl. We save this
 * value and insure it's still present in %dl when we invoke the
 * memory dump code.
 *
 * Everything here runs in real mode. We don't switch to protected
 * mode until we hit the startup code in the memory dumper.
 */

	.globl	boot
	.text

boot:
	cli

	/* Clear segment registers */

	ljmp	$0, $setcs
setcs:
	xor	%ax, %ax
	mov	%ax, %ds
	mov	%ax, %ss

	/* Set up a temporary stack. */

	mov	$boot, %sp

	cld

	mov	PRT4_STARTSEC, %al

	/* Save the BIOS disk number */

	mov	%dl, bios_disk

	mov	$str1, %si
	call	printf

	/* Reset the drive */

	mov	$INT13_RESET, %ah
	mov	bios_disk, %dl
	int	$DISK_INT
	jc	stop

	/* Check disk status */

	mov	$INT13_STSCHK, %ah
	mov	bios_disk, %dl
	int	$DISK_INT
	jc	stop

	cmp	$0x0, %al
	jne	stop

	/* Check for extended INT13 services. */

	mov	$EXT_SIG, %bx
	mov	$INT13_EXTCHK, %ah
	mov	bios_disk, %dl
	int	$DISK_INT
	jc	use_chs
        cmp     $BOOT_SIG, %bx
	jne	use_chs
	mov	bios_disk, %dl
	testb	$0x80, %dl
	jz	use_chs	

	/*
	 * Try loading the scraper code into RAM. We look at the
	 * partition table entry for partition 4 in the master
	 * boot record (i.e. us) and use its 'absolute starting sector
	 * offset' as the location on the disk from which we will
	 * read the scraper. If the scraper.bin image has been
	 * written directly to a disk starting at sector 0, the entire
	 * partition table will be empty, and the offset for partition
	 * 4 will be zero. In effect, no offset will be applied.
	 * However, if the partition table has been edited such that
	 * partition 4 exists, we assume that the scraper.bin image
	 * has been written there instead, and we use the start of
	 * partition 4 as the start offset.
	 */

	/* Try using packet mode */

	mov	$packet, %si
	call	printf

	mov	PRT4_START, %eax
	inc	%eax		/* skip over the boot sector */
	mov	%eax, pkt_soff1
        mov	$pkt, %si
        mov	$INT13_EXTRD, %ah
	mov	bios_disk, %dl
        int     $DISK_INT
	jc	use_chs
	jmp	startup

	/* Try using CHS mode */
use_chs:
	mov	$chs, %si
	call	printf

	xor	%ax, %ax
	mov	%ax, %es
	mov	$SCRAPER_BASE, %bx
	mov	$INT13_RD, %ah
	mov	$DISK_SCRAPER_SIZE, %al
	mov	PRT4_STARTCYL, %ch
	mov	PRT4_STARTSEC, %cl
	mov	PRT4_STARTHD, %dh
	cmp	$0x0, %ch
	jne	read
	cmp	$0x0, %cl
	jne	read
	cmp	$0x0, %dh
	jne	read
	add	$1, %cl
read:
	add	$1, %cl
	mov	bios_disk, %dl
	int	$DISK_INT
	jc	stop

startup:
	mov	$str2, %si
	call	printf

	mov	bios_disk, %dl

	/*
         * Do a long jump here instead of a jump. This prevents the
	 * assembler from possibly calculating a relative offset which,
	 * with the Cygwin and MinGW assembler, always seems to be
	 * calculated wrong.
	 */

	ljmp	$0, $SCRAPER_BASE

stop:
	mov	$fail, %si
	call	printf
halt:
	hlt
	jmp	halt

	/*
	 * Stripped down printf implementation.
	 */
printf:
	lodsb
	testb	%al, %al
	jnz	putc
	ret
putc:
	movb	$0xE, %ah
	movw	$0x7, %bx
	int	$0x10
	jmp	printf

str1:
	.asciz	"Bootstrap loaded... "
str2:
	.asciz	"starting.\r\n"
chs:
	.asciz	"trying c/h/s mode... "
packet:
	.asciz	"trying packet mode... "
fail:
	.asciz	"epic fail!\r\n"

bios_disk:
	.word	0x00

pkt:
	.byte	0x10			/* structure size (16 bytes) */
	.byte	0x00			/* Must be 0 */
	.byte	DISK_SCRAPER_SIZE	/* 16 sectors */	
	.byte	0x00			/* Must also be 0 */
	.word	_OFF(SCRAPER_BASE)	/* Destination buffer offset */
	.word	_SEG(SCRAPER_BASE)	/* Destination buffer segment */
pkt_soff1:
	.long	0x00000000		/* Sector offset */
pkt_soff2:
	.long	0x00000000		/* Sector offset */

	/* Fake partition table */

	.org	PRT_OFF, 0x00
partbl:
	.fill	0x40,0x1,0x0		/* Partition table */
	.word	BOOT_SIG		/* Magic number */

