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
extern int disk_init(uint32_t);
extern int disk_io(int, void *, uint32_t, off_t);
#endif


#define DISK_OFFSET_DATA	(512*128)
#define DISK_OFFSET_MAP		(512*126)
#define DISK_SCRAPER_SIZE	64	/* 64 sectors, 32Kbytes */

#define DISK_RD		0
#define DISK_WR		1

#define DISK_INT	0x13

#define	INT13_RESET	0x00	/* Reset disk */
#define INT13_STSCHK	0x01	/* Check disk status */
#define INT13_RD	0x02	/* Read sectors */
#define INT13_WR	0x03	/* Write sectors */
#define INT13_RDPARM	0x08	/* Read disk parameters */
#define INT13_EXTCHK	0x41	/* Check it extentions are present */
#define INT13_EXTRD	0x42	/* Extended read sectors */
#define INT13_EXTWR	0x43	/* Extended write sectors */
#define INT13_EXTRDPARM	0x48	/* Extended read disk parameters */

#define BOOT_SIG	0xAA55
#define EXT_SIG		0x55AA

#define INT13_EXTCHK_PACKET	0x0001	/* Supports packet mode */
#define INT13_EXTCHK_LOCKEJECT	0x0002	/* Supports lock/eject */
#define INT13_EXTCHK_EDD	0x0004	/* Supports Enhanced Disk Drive */

#define INT13_EXTPARMS_SIZE	0x1A
#define INT13_EXTRW_SIZE	0x10

#define INT13_EXTPARMFLAGS_TRDMA	0x0001	/* DMA errors handled */
#define INT13_EXTPARMFLAGS_VALIDGEOM	0x0002	/* c/h/s info valid */
#define INT13_EXTPARMFLAGS_REMOVABLE	0x0004	/* removable drive */
#define INT13_EXTPARMFLAGS_VERIFYSUP	0x0008	/* write/verify supported */
#define INT13_EXTPARMFLAGS_CHGLINE	0x0010	/* has change-line support */
#define INT13_EXTPARMFLAGS_LOCKABLE	0x0020	/* drive can be locked */
#define INT13_EXTPARMFLAGS_CHSMAX	0x0040	/* c/h/s set to max */

#define	DOSBBSECTOR	0	/* DOS boot block relative sector number */
#define	DOSPARTOFF	446
#define	DOSPARTSIZE	16

#ifndef _ASM
struct int13_extparms {
	uint16_t		ep_size;	/* result buffer size */
	uint16_t		ep_flags;	/* flags */
	uint32_t		ep_physcyls;	/* physical # of tracks */
	uint32_t		ep_physheads;	/* physical # of heads */
	uint32_t		ep_physst;	/* physical # of secs/track */
	uint64_t		ep_physsecs;	/* total # of sectors */
	uint16_t		ep_secsize;	/* bytes per sector */
	uint32_t		ep_eddaddr;	/* EDD info pointer */
} __attribute__((packed));

struct int13_packet {
	uint8_t			pkt_size;	/* Size of structure */
	uint8_t			pkt_mbo0;	/* Must be zero */
	uint8_t			pkt_secs;	/* # of sectors, 0-127 */
	uint8_t			pkt_mbo1;	/* Must be zero */
	uint16_t		pkt_buf_off;	/* offset of buffer */
	uint16_t		pkt_buf_seg;	/* segment address of buffer */
	uint64_t		pkt_sect_off;	/* sector offset to access */
} __attribute__((packed));

struct dos_partition {
	unsigned char	dp_flag;	/* bootstrap flags */
	unsigned char	dp_shd;		/* starting head */
	unsigned char	dp_ssect;	/* starting sector */
	unsigned char	dp_scyl;	/* starting cylinder */
	unsigned char	dp_typ;		/* partition type */
	unsigned char	dp_ehd;		/* end head */
	unsigned char	dp_esect;	/* end sector */
	unsigned char	dp_ecyl;	/* end cylinder */
	uint32_t	dp_start;	/* absolute starting sector number */
	uint32_t	dp_size;	/* partition size in sectors */
};
#endif
