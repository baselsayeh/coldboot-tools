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

#define MSR_EFER	0xc0000080
#define EFER_SCE	0x00000001
#define EFER_LME	0x00000100
#define EFER_LMA	0x00000400
#define EFER_NXE	0x00000800

#define CR4_PAE		0x00000020
#define CR4_PSE		0x00000010

#define CR0_PG		0x80000000	/* Enable paging */
#define	CR0_CD		0x40000000	/* Cache Disable */
#define CR0_PE		0x00000001	/* Enable protected mode */

#define SEGFLAGLO_PRESENT       0x80    /* segment is present */
#define SEGFLAGLO_PRIVLVL       0x60    /* privlevel needed for this seg */
#define SEGFLAGLO_CDS           0x10    /* 1 = code/data, 0 = system */
#define SEGFLAGLO_EXEC          0x08    /* Executable */
#define SEGFLAGLO_EXPANDDOWN    0x04    /* limit expands down */
#define SEGFLAGLO_WRITEABLE     0x02    /* segment is writeable */
#define SEGGLAGLO_ACCESSED      0x01    /* segment has been accessed */

#define SEGFLAGHI_GRAN          0x80    /* granularity, 1 = byte, 0 = page */
#define SEGFLAGHI_BIG           0x40    /* 1 = 32 bit stack, 0 = 16 bit */
#define SEGFLAGHI_LONG		0x20	/* long mode (cs only) */

/* Code flags */
#define GDT_CFLAGS	\
	SEGFLAGLO_PRESENT|SEGFLAGLO_CDS|SEGFLAGLO_EXEC|SEGFLAGLO_WRITEABLE
#define GDT_DFLAGS	\
	SEGFLAGLO_PRESENT|SEGFLAGLO_CDS|SEGFLAGLO_WRITEABLE
#define GDT_GRAN	\
	SEGFLAGHI_GRAN|SEGFLAGHI_BIG
#define GDT_LONG	\
	SEGFLAGHI_LONG

#define GDT_PCODE	0x08
#define GDT_PDATA	0x10
#define GDT_RCODE	0x18
#define GDT_RDATA	0x20
#define GDT_LCODE	0x28
#define GDT_LDATA	0x30

/*
 * Real mode stack: 0x7C00
 * Protected/long mode stack: 0x6C00
 * Real mode data segment: 0x4000
 * Note that this gives only one page of real mode stack space,
 * but we don't use the real mode stack for much, so this
 * should be ok.
 */
 
#define STACK_PROT	0x6C00
#define STACK_REAL_OFF	0x7C00
#define STACK_REAL_SEG	0x0000
#define DATA_REAL	0x0400

#define PAGE_TABLES	((void *)0x40000)

/*
 * Page-directory and page-table entries follow this format, with a few
 * of the fields not present here and there, depending on a lot of things.
 */
                                /* ---- Intel Nomenclature ---- */
#define PG_V            0x001   /* P    Valid                   */
#define PG_RW           0x002   /* R/W  Read/Write              */
#define PG_U            0x004   /* U/S  User/Supervisor         */
#define PG_NC_PWT       0x008   /* PWT  Write through           */
#define PG_NC_PCD       0x010   /* PCD  Cache disable           */
#define PG_A            0x020   /* A    Accessed                */
#define PG_M            0x040   /* D    Dirty                   */
#define PG_PS           0x080   /* PS   Page size (0=4k,1=2M)   */
#define PG_PTE_PAT      0x080   /* PAT  PAT index               */
#define PG_G            0x100   /* G    Global                  */
#define PG_AVAIL1       0x200   /*    / Available for system    */
#define PG_AVAIL2       0x400   /*   <  programmers use         */
#define PG_AVAIL3       0x800   /*    \                         */
#define PG_PDE_PAT      0x1000  /* PAT  PAT index               */
#define PG_NX           (1ul<<63) /* No-execute */

#define PAGE_SIZE	4096

#ifndef _ASM
struct bios_smap {
	uint64_t		base;
	uint64_t		len;
	uint32_t		type;
} __attribute__((__packed__));
#endif

#define SMAP_MEM		1
#define SMAP_RSVD		2
#define SMAP_SIG 0x534D4150

#ifndef _ASM
extern struct bios_smap bios_smap;
extern uint32_t bios_arg;
extern uint16_t bios_seg;
extern uint16_t bios_off;
extern uint32_t bios_next;
extern uint32_t bios_ext1;
extern uint32_t bios_ext2;
extern uint16_t bios_cylsec;
extern uint8_t bios_hds;
extern uint8_t bios_disk;
extern uint8_t bios_secs;

extern uint16_t pxe_off;
extern uint16_t pxe_seg;
extern uint16_t pxe_op;
extern uint16_t *pxe_addr;
extern void bcopy_long (uintptr_t *, uintptr_t *, int);
extern void mapmem (void);
#endif

#define PKT_SIZE	8192

#define SEG(x)		(((uint32_t)((uintptr_t)(x) & 0xFFFFFFFF) & 0xF0000) >> 4)
#define OFF(x)		((uint32_t)((uintptr_t)(x) & 0xFFFFFFFF) & 0xFFFF)

/*
 * Cygwin/MinGW version of gcc insists on using leading underscores
 * for C function names, but FreeBSD/Linux do not.
 */

#if defined (__CYGWIN__) || defined (__MINGW32__)
#define FUNC(x)		_ ## x
#define EXT(x)		_ ## x
#else
#define FUNC(x)		x
#define EXT(x)		x
#endif

#define BOOT_BASE	0x7C00
#define SCRAPER_BASE	0x7E00
