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

#include <sys/types.h>
#include <sys/socket.h>

#include <netinet/in.h>
#include <netinet/udp.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __FreeBSD__
#include <err.h>
#else
#define err(x,y) { perror(y); exit(x); }
#endif

#define CHUNK 4096

#include "protocol.h"
#include "data.h"

static char buf[65536 + sizeof(SCRAPE_MSG)];
static char mem[65536];

static int
get_block(s, sin, start, size, sendsize)
	int			s;
	struct sockaddr_in *	sin;
	uint32_t		start;
	uint32_t		size;
	int			sendsize;
{
	SCRAPE_MSG *		m;
	fd_set			fdset;
	struct timeval		tv;
	int			r;
	int			b;
	int			cur;
	int			i;
	uint32_t		total;
	struct sockaddr_in	rsin;
	int			rlen;

	cur = start;
	total = size;

	m = (SCRAPE_MSG *)buf;

	while (1) {

		m->sm_cmd = htonl(SM_CMD_REQ);
		m->sm_len = htonl(CHUNK);
		m->sm_seq = htonl(cur);

		if (sendto(s, buf, sizeof(SCRAPE_MSG), 0,
		   (struct sockaddr *)sin, sizeof(struct sockaddr_in)) < 0)
		    err(1, "sending request failed");

		r = 0;

		for (i = 0; i < CHUNK / sendsize; i++) {
			FD_ZERO(&fdset);
			FD_SET(s, &fdset);
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			b = select (FD_SETSIZE, &fdset, NULL, NULL, &tv);
			if (b <= 0) {
				fprintf(stderr, "timeout!!!\n");
				break;
			}
			b = read (s, m, sendsize + sizeof(SCRAPE_MSG));
			bcopy(buf + sizeof(SCRAPE_MSG), mem + (sendsize * i),
			    sendsize);
			r += b - sizeof(SCRAPE_MSG);
		}

		if (r != CHUNK) {
			fprintf(stderr, "only got %d\n", r);
			continue;
		}

		fwrite(mem, CHUNK, 1, stdout);
		fflush (stdout);
		cur += CHUNK;
		total -= CHUNK;
		if (total < CHUNK)
			break;
	}

	fflush (stdout);
	return (0);
}

int
main (argc, argv)
	int			argc;
	char			*argv[];
{
	struct sockaddr_in	sin;
	struct sockaddr_in	rsin;
	int			rlen;
	int			s;
	int			opt;
	SCRAPE_MSG *		m;
	fd_set			fdset;
	int			b;
	int			r;
	int			i;
	uint32_t		sendsize;
	uint32_t		start;
	uint32_t		total;
	struct bios_smap	bmap[32];

	if (argc != 2) {
		fprintf(stderr, "usage: pxedump <target ipaddr>\n");
		exit(1);
	}

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (s == -1)
		err(1, "socket()");

	/* Allow broadcasts */

	opt = 1;
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
		err(1, "setsockopt: broadcast");

	bzero ((char *)&sin, sizeof(sin));
	bzero (buf, sizeof(buf));

	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	sin.sin_port = htons(SM_PORT);
	sin.sin_family = AF_INET;

	bind(s, (struct sockaddr *)&sin, sizeof(sin));

	sin.sin_addr.s_addr = inet_addr(argv[1]);
	sin.sin_port = htons(SM_PORT);
	sin.sin_family = AF_INET;

	FD_ZERO(&fdset);
	FD_SET(s, &fdset);

	m = (SCRAPE_MSG *)buf;

	m->sm_cmd = htonl(SM_CMD_PROBE);
	if (sendto(s, buf, sizeof(SCRAPE_MSG), 0,
	    (struct sockaddr *)&sin, sizeof(sin)) < 0)
	    err(1, "sending probe failed");

	/* Wait for reply */

	select (FD_SETSIZE, &fdset, NULL, NULL, NULL);

	b = recvfrom (s, m, sizeof (bmap) + sizeof(SCRAPE_MSG),
	    0, (struct sockaddr *)&rsin, (socklen_t *)&rlen);

	if (b != sizeof(SCRAPE_MSG) + sizeof (bmap))
		err(1, "failed to receive reply");

	start = ntohl(m->sm_seq);
	total = ntohl(m->sm_len);
	sendsize = ntohl(m->sm_off);
	bcopy (buf + sizeof(SCRAPE_MSG), bmap, sizeof(bmap));

	i = 0;

	while (i < 32 && bmap[i].lenlo) {
		fprintf(stderr, "request segment %p size %lu\n",
		    ntohl(bmap[i].baselo), ntohl(bmap[i].lenlo));
		get_block(s, &sin, ntohl(bmap[i].baselo),
		    ntohl(bmap[i].lenlo), sendsize);
		i++;
	}

	m->sm_cmd = htonl(SM_CMD_QUIT);
	m->sm_len = 0;
	m->sm_seq = 0;

	if (sendto(s, buf, sizeof(SCRAPE_MSG), 0,
	   (struct sockaddr *)&sin, sizeof(sin)) < 0)
	    err(1, "sending quit failed");

	close (s);

	exit(0);
}
