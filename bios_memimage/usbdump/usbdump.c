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

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#ifdef __FreeBSD__
#include <err.h>
#else
#define err(x,y) { perror(y); exit(x); }
#endif

#include "data.h"
#include "disk.h"
#include "stand.h"

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))
#else
#define htonll(x) (x)
#define ntohll(x) (x)
#endif

#define CHUNK	32768

static uint8_t	map[1024];
static uint8_t	block[CHUNK];

static int
recover_seg(FILE * fp, off_t off, uint64_t size)
{
	int			i;

#ifdef __MINGW32__
	if (fseek(fp, off, SEEK_SET) == -1)
#else
	if (fseeko(fp, off, SEEK_SET) == -1)
#endif
		err(1, "failed to seek to data");

	for (i = 0; i < (size / CHUNK); i++) {
		if (fread (block, CHUNK, 1, fp) <= 0)
			return (-1);
		fwrite(block, CHUNK, 1, stdout);
	}

	if (size % CHUNK) {
		if (fread (block, size % CHUNK, 1, fp) <= 0)
			return (-1);
		fwrite(block, size % CHUNK, 1, stdout);
	}

	return (0);
}

int
main (int argc, char * argv[])
{
	FILE *			fp;
	struct bios_smap *	bmap;
	int			r;
	int			i;
	off_t			off;
	uint32_t		crc1, crc2;

	if (argc != 2) {
		fprintf(stderr, "usage: usbdump <device>\n");
		exit(1);
	}

	fp = fopen (argv[1], "rb");

	if (fp == NULL)
		err(1, "opening input device failed");

	if (fseek(fp, DISK_OFFSET_MAP, SEEK_SET) == -1)
		err(1, "failed to seek to dump map");

	r = fread(map, sizeof(map), 1, fp);

	if (r == -1)
		err(1, "failed to read to dump map");

	bmap = (struct bios_smap *)map;
	crc1 = bmap[31].base & 0xFFFFFFFF;
	bmap[31].base = 0;
	crc2 = ether_crc32_le((const uint8_t *)bmap,
            sizeof(struct bios_smap) * 32);

	if (crc1 != crc2) {
		fprintf (stderr, "bad checksum on dump map "
		    "(%x != %x), aborting\n", crc1, crc2);
		exit(1);
	}

	off = DISK_OFFSET_DATA;

	for (i = 0; i < 32; i++) {
		if (bmap[i].len == 0)
			continue;
		fprintf(stderr, "recover segment%d [base: 0x%llx size: %llu]\n",
		    i, bmap[i].base, bmap[i].len);
		fflush(stderr);
		if (recover_seg (fp, off, bmap[i].len) == -1) {
			fprintf (stderr, "error dumping segment %d\n", i);
			exit(1);
		}
		off += bmap[i].len;
	}

	fclose (fp);

	exit(0);
}
