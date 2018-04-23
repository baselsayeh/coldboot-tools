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

#ifndef _ASM
struct x86desc {
        uint16_t		x_lolimit;
        uint16_t		x_base0;
        uint8_t			x_base1;
        uint8_t			x_flags;
        uint8_t			x_hilimit;
        uint8_t			x_base2;
};

struct gdt {
        uint16_t		limit;
        void *			base;
} __attribute__((__packed__));
#endif

#define SEGFLAGLO_PRESENT       0x80    /* segment is present */
#define SEGFLAGLO_PRIVLVL       0x60    /* privlevel needed for this seg */
#define SEGFLAGLO_CDS           0x10    /* 1 = code/data, 0 = system */
#define SEGFLAGLO_EXEC          0x08    /* Executable */
#define SEGFLAGLO_EXPANDDOWN    0x04    /* limit expands down */
#define SEGFLAGLO_WRITEABLE     0x02    /* segment is writeable */
#define SEGGLAGLO_ACCESSED      0x01    /* segment has been accessed */

#define SEGFLAGHI_GRAN          0x80    /* granularity, 1 = byte, 0 = page */
#define SEGFLAGHI_BIG           0x40    /* 1 = 32 bit stack, 0 = 16 bit */

/* Code flags */
#define GDT_CFLAGS	\
	SEGFLAGLO_PRESENT|SEGFLAGLO_CDS|SEGFLAGLO_EXEC|SEGFLAGLO_WRITEABLE
#define GDT_DFLAGS	\
	SEGFLAGLO_PRESENT|SEGFLAGLO_CDS|SEGFLAGLO_WRITEABLE
#define GDT_GRAN	\
	SEGFLAGHI_GRAN|SEGFLAGHI_BIG

#define GDT_PCODE	0x08
#define GDT_PDATA	0x10
#define GDT_RCODE	0x18
#define GDT_RDATA	0x20

#define STACK_PROT	0x20000
#define STACK_REAL_OFF	0xFFE
#define STACK_REAL_SEG	0x3000
#define DATA_REAL	0x6000

#ifndef _ASM
struct bios_smap {
	uint32_t		baselo;
	uint32_t		basehi;
	uint32_t		lenlo;
	uint32_t		lenhi;
	uint32_t		type;
};
#endif

#define SMAP_MEM		1
#define SMAP_RSVD		2
#define SMAP_SIG 0x534D4150

#ifndef _ASM
extern struct bios_smap bios_smap;
extern uint32_t bios_next;
extern uint32_t bios_ext1;
extern uint32_t bios_ext2;

extern uint16_t pxe_off;
extern uint16_t pxe_seg;
extern uint16_t pxe_op;
extern uint16_t *pxe_addr;
extern void bcopy_long (uint32_t *, uint32_t *, int);
#endif

#define PKT_SIZE	8096
