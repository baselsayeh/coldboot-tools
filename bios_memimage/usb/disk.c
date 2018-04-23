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
#include "stand.h"
#include "disk.h"

static int disk_extended = 0;
static uint32_t disk_offset;
static uint64_t disk_size;
static uint32_t chs_heads, chs_cyls, chs_secs;
static uint32_t pkt_heads, pkt_cyls, pkt_secs;
static struct int13_extparms exparms;
static struct int13_packet pkt;

int
disk_init (uint32_t memsize)
{
	struct dos_partition * p;

	/* Get disk geometry */
	bios_getgeom(bios_disk);

 	chs_heads = bios_hds;
 	chs_cyls = (bios_cylsec & 0xFF00) >> 8;
 	chs_cyls += (bios_cylsec & 0x00C0) << 2;
 	chs_secs = bios_cylsec & 0x3F;
 	chs_cyls++;  
 	chs_heads++; 

	disk_size = chs_cyls * chs_heads * chs_secs * 512;

	/* Check for INT13 extentions */

	bios_arg = 0;

	bios_extchk(bios_disk);

	if (bios_arg != 0xFFFF && bios_arg & INT13_EXTCHK_PACKET)
		disk_extended = 1;
	else
		goto done;

	bios_reset(bios_disk);

	bios_seg = SEG(&exparms);
	bios_off = OFF(&exparms);

	bzero((char *)&exparms, sizeof(exparms));
        exparms.ep_size = sizeof(exparms);

	bios_getextgeom(bios_disk);

	pkt_heads = exparms.ep_physheads;
	pkt_cyls = exparms.ep_physcyls;
	pkt_secs = exparms.ep_physst;

        /*
         * If reading the disk geometry using packet mode
	 * failed, stick with CHS mode.
	 */

        if (pkt_heads == 0 && pkt_cyls == 0 && pkt_secs == 0) {
		disk_extended = 0;
		goto done;
	}

	disk_size = exparms.ep_physsecs * exparms.ep_secsize;

done:

	p = (struct dos_partition *)(BOOT_BASE + DOSPARTOFF);
	disk_offset = p[3].dp_start;

#ifndef SILENT
        printf("Disk size: %lu bytes\n", disk_size);
	if (memsize > disk_size)
		printf("Warning: disk is too small, dump will be truncated.\n");

#endif

	return(0);
}

int
disk_io (int op, void * buf,
    uint32_t size, off_t off)
{
	uint32_t		blk, cyl, hd, sec;

	if (off > disk_size)
		goto error;

	blk = (off / 512) + disk_offset;
	cyl = blk / (chs_secs * chs_heads);

	if (disk_extended && cyl > 1023) {
		pkt.pkt_size = INT13_EXTRW_SIZE;
		pkt.pkt_mbo0 = pkt.pkt_mbo1 = 0;
		pkt.pkt_secs = (size / 512);
		pkt.pkt_buf_seg = SEG(buf);
		pkt.pkt_buf_off = OFF(buf);
		pkt.pkt_sect_off = (off / 512) + disk_offset;
		bios_seg = SEG(&pkt);
		bios_off = OFF(&pkt);
		if (op == DISK_WR)
			bios_extwr(bios_disk);
		else
			bios_extrd(bios_disk);
	} else {
		blk %= (chs_secs * chs_heads);
		hd = blk / chs_secs;
		sec = blk % chs_secs;
		sec++;
		bios_hds = hd;
		bios_cylsec = ((cyl & 0xFF) << 8) | ((cyl & 0x300) >> 2) | sec;
		bios_seg = SEG(buf);
		bios_off = OFF(buf);
		bios_secs = size / 512;
		if (op == DISK_WR)
			bios_wr(bios_disk);
		else
			bios_rd(bios_disk);
	}

	if (bios_arg) {
error:
#ifndef SILENT
		printf("\n%s error at block %d\n",
		    op == DISK_WR ? "Write" : "Read", off / 512, buf);
#endif
		return(-1);
	}

	return (0);
}
