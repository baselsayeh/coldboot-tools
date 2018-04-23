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

#include "types.h"
#include "data.h"
#include "bios.h"
#include "disk.h"
#include "stand.h"

#define CHUNK_SIZE 32768

static char page[] = { 0 };
static int checkA20 (void);
static int dump_seg (off_t, off_t, size_t);
static struct bios_smap bmap[32];

static int
checkA20 (void)
{
	uint32_t	m0, m1;
	volatile uint32_t *	p0 = (uint32_t *)0x000000;
	volatile uint32_t *	p1 = (uint32_t *)0x100000;

	/* Save original value */
	m0 = *p0;

	/* Write some new data */
	*p0 = 0xcafebabe;

	/* See if it shows up at the next 1MB offset */
	m1 = *p1;

	/* Restore original value */
	*p0 = m0;

	/* If m1 changed when we updated m0, A20 line isn't set. */

	return (m1 == 0xcafebabe);
}

static int
dump_seg (off_t off, off_t daddr, size_t len)
{
	int		i;
        char *		s;
	off_t		o;
	uintptr_t *	paddr;
	uint32_t	plen;
	uint32_t	r;
#ifndef SILENT
	uint32_t	cur = 0, prev = 0;
	uint32_t	fact;

	fact = len / 100;
#endif

	s = (char *)(uintptr_t)daddr;

#ifndef SILENT
	printf ("Dumping 0x%016llx bytes:  00%%", len, 0);
#endif

	paddr = (uintptr_t *)page;
	plen = CHUNK_SIZE;
	o = off;

	for (i = 0; i < ((len / CHUNK_SIZE) +
            ((len % CHUNK_SIZE) ? 1 : 0)); i++) {

		/* Pick up residual data. */

		r = len - (i * CHUNK_SIZE);
		if (r && r < CHUNK_SIZE)
			plen = r;

		if (s < (char *)0x100000)
			paddr = (uintptr_t *)s;
		else
			bcopy_long ((uintptr_t *)s, paddr, plen);
		r = disk_io(DISK_WR, paddr, plen, o);
		if (r) {
#ifndef SILENT
			printf("Writing page %p failed\n", s);
#endif
			return(-1);
		}
		o += plen;
		s += plen;
#ifndef SILENT
		cur += plen;
		r = cur / fact;
		/* Only generate output when % of completion changes. */
		if (prev != r) {
			prev = r;
			printf("");
			if (r == 100)
				printf("");
			printf("%02d%%", r);
		}
#endif
	}

#ifndef SILENT
	printf (" Done.\n");
#endif

	return (0);
}

/*
 * The Cygwin version of gcc insists on putting special runtime startup
 * code in any module that contains a real main() routine. As a standalone
 * app, we don't need any of that, so we deliberately use _main() as our
 * C entry point instead.
 */

void
_main(void)
{
	uint64_t total;
	int segs;
	off_t start = 0;
	int i;
	uint32_t crc;
#ifndef SILENT
	uint16_t * c;

	printf("USB memory scraper, written by Bill Paul (%s %s)\n",
	    __DATE__, __TIME__);
#endif

	if (checkA20() != 0) {
#ifndef SILENT
		printf("Failed to set A20 line, aborting.\n");
#endif
		return;
	}

	/* Figure out how much RAM is available  */

	total = 0;
	segs = 0;
	bios_next = 0;
	bzero((char *)bmap, sizeof(bmap));

	do {
		bios_memmap();
		if (bios_smap.type != SMAP_MEM)
			continue;
#ifndef SILENT
		printf("Memory segment %d: base: 0x%016llx: size: %llu (0x%llx)\n",
		    segs, bios_smap.base, bios_smap.len, bios_smap.len);
#endif
		bmap[segs].base = bios_smap.base;
		bmap[segs].len = bios_smap.len;
		segs++;
		total += bios_smap.len;
	} while (bios_next != 0);

	/* New style memory map access failed, try old style. */

	if (total == 0) {
		bios_memmap2();
		total = 0x100000;
		bmap[segs].base = 0;
		bmap[segs].len = total;
		segs++;
		bmap[segs].base =total;
		bmap[segs].len = bios_ext1 * 1024;
#ifndef SILENT
		printf("Memory segment %d: base: 0x%016llx: size: %llu (0x%llx)\n",
		    segs, total, bios_ext1 * 1024, bios_ext1 * 1024);
#endif
		segs++;
		total += bios_ext1 * 1024;
		bmap[segs].base = total;
		bmap[segs].len = bios_ext2 * 64 * 1024;
#ifndef SILENT
		printf("Memory segment %d: base: 0x%016llx: size: %llu (0x%llx)\n",
		    segs, total, bios_ext2 * 64 * 1024, bios_ext2 * 64 * 1024);
#endif
		total = (bios_ext1 * 1024) + (bios_ext2 * 64 * 1024);
		segs++;
	}

#ifndef SILENT
	printf("Total memory: %llu bytes\n", total);
#endif

	/*
	 * Dump keyboard buffer. This is already going to be
	 * dumped to disk, but printing it here may prove
	 * illuminating. Some disk encryption utilities
	 * use BIOS keyboard input to read the user's password
	 * and some OSes don't blank the keyboard buffer region.
	 * This means that in some scenarios, it may be possible
	 * to immediately recover a user's plaintext password
	 * (or at least the first 16 bytes of it).
	 */

#ifndef SILENT
	printf("Keyboard buffer: [");
	c = (uint16_t *)(BIOS_KEYBDBUF_BASE);
	for (i = 0; i < BIOS_KEYBDBUF_SIZE / 2; i++) {
                uint8_t ch;
                ch = c[i] & 0xFF;
                if (ch == '\r' || ch == '\n')
			printf(" ");
		else
			printf("%c", ch);
	}
	printf("]\n");
#endif

	/* Initialize our disk subsystem */

	disk_init(total);

	/* Write the dump map */

	crc = ether_crc32_le((const uint8_t *)bmap, sizeof(bmap));
	bmap[31].base = crc;

	if (disk_io(DISK_WR, bmap, 1024, DISK_OFFSET_MAP)) {
#ifndef SILENT
		printf ("Writing dump map failed!\n");
#endif
		return;
	}

	start = DISK_OFFSET_DATA;
	for (i = 0; i < segs; i++) {
		dump_seg(start, bmap[i].base, bmap[i].len);
		start += bmap[i].len;
		/*
		 * Once we dump the first 640K, expand our memory
	 	 * mappings. (Applies to x86-64 only.)
		 */
		if (i == 0)
			mapmem();
	}

	/* That's all for now. */

#ifndef SILENT
	printf("Dump complete\n");
#endif

	return;
}
