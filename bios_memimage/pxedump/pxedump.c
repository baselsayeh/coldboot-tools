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


#ifdef __MINGW32__
#include <stdint.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

#include <netinet/in.h>
#include <netinet/udp.h>

#include <arpa/inet.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifdef __FreeBSD__
#include <err.h>
#else
#ifdef __MINGW32__
#define err(x,y) { perror(y); if (s != -1) closesocket (s); exit(x); }
#else
#define err(x,y) { perror(y); exit(x); }
#endif
#endif

#if _BYTE_ORDER == _LITTLE_ENDIAN
#define htonll(x) ((((uint64_t)htonl(x)) << 32) + htonl(x >> 32))
#define ntohll(x) ((((uint64_t)ntohl(x)) << 32) + ntohl(x >> 32))
#else
#define htonll(x) (x)
#define ntohll(x) (x)
#endif

#define CHUNK 65536

#include "protocol.h"
#include "data.h"

static char buf[4096 + sizeof(SCRAPE_MSG)];
static char mem[65536];

static int
get_block(int s, struct sockaddr_in * sin, off_t start,
    uint64_t size, int sendsize)
{
	SCRAPE_MSG *		m;
	fd_set			fdset;
	struct timeval		tv;
	int			r;
	int			b;
	uint64_t		cur;
	int			i;
	uint64_t		total;
	uint64_t		chunk;

	cur = start;
	total = size;
	chunk = CHUNK;

	m = (SCRAPE_MSG *)buf;

	while (1) {

		m->sm_cmd = htonl(SM_CMD_REQ);
		m->sm_len = htonll(chunk);
		m->sm_seq = htonll(cur);

		if (sendto(s, buf, sizeof(SCRAPE_MSG), 0,
		   (struct sockaddr *)sin, sizeof(struct sockaddr_in)) < 0)
		    err(1, "sending request failed");

		r = 0;

		for (i = 0; i < chunk / sendsize; i++) {
			FD_ZERO(&fdset);
			FD_SET(s, &fdset);
			tv.tv_sec = 0;
			tv.tv_usec = 50000;
			b = select (FD_SETSIZE, &fdset, NULL, NULL, &tv);
			if (b <= 0)
				break;
			b = recv (s, (void *)m,
			    sendsize + sizeof(SCRAPE_MSG), 0);
			memcpy(mem + (sendsize * i), buf + sizeof(SCRAPE_MSG),
			    sendsize);
			r += b - sizeof(SCRAPE_MSG);
		}

		if (r != chunk) {
			fprintf(stderr,
			    "warning, chunksize %llu too large -- ", chunk);
			fprintf(stderr, "reducing to %llu\n", chunk >> 1);
			chunk >>= 1;
			continue;
		}

		fwrite(mem, chunk, 1, stdout);
		fflush(stdout);
		cur += chunk;
		total -= chunk;
		if (total == 0)
			break;
		if (total < chunk)
			chunk = total;
	}

	fflush (stdout);
	return (0);
}

int
main (int argc, char * argv[])
{
	struct sockaddr_in	sin;
	struct sockaddr_in	rsin;
	int			rlen;
	int			s = -1;
	int			opt;
	SCRAPE_MSG *		m;
	fd_set			fdset;
	int			b;
	int			i;
	uint32_t		sendsize;
	uint64_t		start;
	uint64_t		total;
	struct bios_smap	bmap[32];
#ifdef __MINGW32__
	uint16_t		winsock_version;
	WSADATA			winsock_data;
#endif

	if (argc != 2) {
		fprintf(stderr, "usage: pxedump <target ipaddr>\n");
		exit(1);
	}

#ifdef __MINGW32__
	winsock_version = MAKEWORD(2,2);
	if (WSAStartup (winsock_version, &winsock_data))
		err(1, "winsock init failed");
#endif

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (s == -1)
		err(1, "socket()");

	/* Allow broadcasts */

	opt = 1;
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST,
            (void *)&opt, sizeof(opt)) < 0)
		err(1, "setsockopt: broadcast");

	/*
	 * Increase default RX socket buffer size. Cygwin seems to
	 * need this in order to handle 64K chunks properly.
	 */

        opt = CHUNK * 2;
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF,
            (void *)&opt, sizeof(opt)) < 0)
                err(1, "setsockopt: SO_RCVBUF");

	memset ((char *)&sin, 0, sizeof(sin));
	memset (buf, 0, sizeof(buf));

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

	rlen = sizeof(struct sockaddr_in);

	b = recvfrom (s, (void *)m, sizeof (bmap) + sizeof(SCRAPE_MSG),
	    0, (struct sockaddr *)&rsin, (socklen_t *)&rlen);

	if (b != sizeof(SCRAPE_MSG) + sizeof (bmap))
		err(1, "failed to receive reply");

	start = ntohll(m->sm_seq);
	total = ntohll(m->sm_len);
	sendsize = ntohl(m->sm_off);
	memcpy (bmap, buf + sizeof(SCRAPE_MSG), sizeof(bmap));

	i = 0;
	while (i < 32 && bmap[i].len) {
		fprintf(stderr,
                   "request segment%d [base: 0x%llx size: %llu]\n", i,
		    ntohll(bmap[i].base), ntohll(bmap[i].len));
		fflush(stderr);
		get_block(s, &sin, ntohll(bmap[i].base),
		    (uint64_t)ntohll(bmap[i].len), sendsize);
		i++;
	}

	m->sm_cmd = htonl(SM_CMD_QUIT);
	m->sm_len = 0;
	m->sm_seq = 0;

	if (sendto(s, buf, sizeof(SCRAPE_MSG), 0,
	   (struct sockaddr *)&sin, sizeof(sin)) < 0)
	    err(1, "sending quit failed");

#ifdef __MINGW32__
	closesocket (s);
#else
	close (s);
#endif

	exit(0);
}
