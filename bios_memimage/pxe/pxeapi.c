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
#include "pxe.h"
#include "pxeapi.h"
#include "data.h"
#include "bios.h"
#include "stand.h"

/*
 * Since we don't have a heap, we use a static data buffer for
 * all our interaction with the PXE UDP API. It's important that
 * this buffer reside entirely in segment 0, otherwise we won't
 * be able to address it properly in real mode.
 */

static uint8_t pxebuf[1024];
static BOOTPLAYER * bootp;

/*
 * Initialize PXE: make an int 0x1a:5650 BIOS call to fetch the
 * PXENV+ structure, and then obtain the !PXE structure from that.
 * Once we have it, we can use that to obtain the PXE API entry point.
 */

int
pxe_init(IP4_t * myaddr, int * sendsize)
{
	pxenv_t * pxe;
	pxe_t * pxe2;
	t_PXENV_GET_CACHED_INFO * pxe_cache;
	t_PXENV_UNDI_GET_INFORMATION * pxe_info;

	bios_pxe();

	if (pxe_op != PXE_VN)
		return(1);

	/* Find PXENV+ structure */

	pxe = (pxenv_t *)((uintptr_t)((pxe_seg << 4) + pxe_off));

	/* Find !PXE structure */

	pxe2 = (pxe_t *)((uintptr_t)((pxe->PXEPtr.segment << 4) +
            pxe->PXEPtr.offset));

#ifndef SILENT
	printf("PXE version: %d.%d entry: %x:%x\n", pxe->Version >> 8,
	    pxe->Version & 0xFF, pxe2->EntryPointSP.segment,
	    pxe2->EntryPointSP.offset);
#endif

        /* Save the PXE entry point */

	pxe_seg = pxe2->EntryPointSP.segment;
	pxe_off = pxe2->EntryPointSP.offset;

	/* Get UNDI info */

	bzero (pxebuf, sizeof(pxebuf));
	pxe_info = (t_PXENV_UNDI_GET_INFORMATION *)pxebuf;
 
	pxe_op = (uint16_t)PXENV_UNDI_GET_INFORMATION;
        pxe_addr = (uint16_t *)pxe_info;

        if (pxe_call() != 0) {
#ifndef SILENT
                printf("PXE get UNDI info failed\n");
#endif
                return(1);
        }

#ifndef SILENT
	printf("pxe0 at 0x%x irq %d\n", pxe_info->BaseIo, pxe_info->IntNumber);
#endif

	*sendsize = pxe_info->TxBufCt;

	/* Now find our IP address */

	bzero (pxebuf, sizeof(pxebuf));
	pxe_cache = (t_PXENV_GET_CACHED_INFO *)pxebuf;
	pxe_cache->PacketType = PXENV_PACKET_TYPE_BINL_REPLY;
 
	pxe_op = (uint16_t)PXENV_GET_CACHED_INFO;
        pxe_addr = (uint16_t *)pxe_cache;

        pxe_call();

        if (pxe_call() != 0) {
#ifndef SILENT
                printf("PXE get cache failed\n");
#endif
                return(1);
        }

        bootp = (BOOTPLAYER *)((uintptr_t)((pxe_cache->Buffer.segment << 4) +
            pxe_cache->Buffer.offset));

#ifndef SILENT
	printf("My IP address: %d.%d.%d.%d\n", IP_ARGS(ntohl(bootp->yip)));
	printf("My eth address: %02x:%02x:%02x:%02x:%02x:%02x\n",
	    bootp->CAddr[0], bootp->CAddr[1], bootp->CAddr[2], bootp->CAddr[3],
	    bootp->CAddr[4], bootp->CAddr[5]);
#endif

	*myaddr = ntohl(bootp->yip);

	return (0);
}

/*
 * Do a UDP open. This initializes the UNDI layer for UDP service.
 * Only one UDP connection can be active at a time.
 */

int
pxe_open(void)
{
	t_PXENV_UDP_OPEN * pxe_open;

	bzero (pxebuf, sizeof(t_PXENV_UDP_OPEN));
	pxe_open = (t_PXENV_UDP_OPEN *)pxebuf;

        pxe_op = (uint16_t)PXENV_UDP_OPEN;
        pxe_addr = (uint16_t *)pxe_open;

	pxe_open->src_ip = bootp->yip;

        if (pxe_call() != 0) {
#ifndef SILENT
                printf("PXE UDP open failed\n");
#endif
                return(1);
        }

	return(0);
}

/*
 * Close an existing UDP session.
 */

int pxe_close(void)
{
	t_PXENV_UDP_CLOSE * pxe_close;

	bzero (pxebuf, sizeof(t_PXENV_UDP_CLOSE));
	pxe_close = (t_PXENV_UDP_CLOSE *)pxebuf;

        pxe_op = (uint16_t)PXENV_UDP_CLOSE;
        pxe_addr = (uint16_t *)pxe_close;

        if (pxe_call() != 0) {
#ifndef SILENT
                printf("PXE UDP close failed\n");
#endif
                return(1);
        }

	return(0);
}

/*
 * Transmit a UDP datagram. Note that the PXE ROM doesn't support
 * fragmentation, so we can't send more data than will fit in a
 * single ethernet frame.
 */

int
pxe_write(IP4_t dst, UDP_PORT_t port,
    void * buf, int len)
{
	t_PXENV_UDP_WRITE * pxe_write;
	int i = 0;

	bzero (pxebuf, sizeof(t_PXENV_UDP_WRITE));
	pxe_write = (t_PXENV_UDP_WRITE *)pxebuf;

retry:

	pxe_write->ip = htonl(dst);
        pxe_write->dst_port = htons(port);
        pxe_write->src_port = htons(port);
        pxe_write->buffer_size = len;
        pxe_write->buffer.segment = SEG(buf);
        pxe_write->buffer.offset = OFF(buf);

        pxe_op = (uint16_t)PXENV_UDP_WRITE;
        pxe_addr = (uint16_t *)pxe_write;

        if (pxe_call() != 0) {
		i++;
		if (i < 10)
			goto retry;
                return(1);
        }

	return(0);
}

/*
 * Receive a UDP datagram. Will return failure if there's no packet
 * waiting.
 */

int
pxe_read(IP4_t * src, UDP_PORT_t * port,
    void * buf, int len, int block)
{
	t_PXENV_UDP_READ * pxe_read;

	bzero (pxebuf, sizeof(t_PXENV_UDP_READ));
	pxe_read = (t_PXENV_UDP_READ *)pxebuf;

retry:

	pxe_read->dest_ip = htonl(*src);
        pxe_read->d_port = htons(*port);
        pxe_read->buffer_size = len;
        pxe_read->buffer.segment = SEG(buf);
        pxe_read->buffer.offset = OFF(buf);

        pxe_op = (uint16_t)PXENV_UDP_READ;
        pxe_addr = (uint16_t *)pxe_read;

        if (pxe_call() != 0) {
		if (block)
			goto retry;
                return(1);
	}

	*src = ntohl(pxe_read->src_ip);
	*port = ntohs(pxe_read->s_port);
	
	return(0);
}
