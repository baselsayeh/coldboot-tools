#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>

#include "common.h"

#define TFTP_PORT 69
static int udpsock;
extern IPaddr_t NetServerIP;
extern IPaddr_t NetOurIP;
char * BootFile;
char * load_addr;
extern int NetState;
extern void TftpStart (void);

void
sockcreate(void)
{
        int                     s;
        int                     opt;
        struct sockaddr_in      sin;

	bzero((char *)&sin, sizeof(sin));

        s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (s == -1) {
                perror("socket create failed");
		return;
	}

        opt = 1;
        if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt)) < 0)
                perror("setsockopt: broadcast");

        bzero((char *)&sin, sizeof(sin));

        sin.sin_addr.s_addr = htonl(INADDR_ANY);
        sin.sin_port = htons(TFTP_PORT);
        sin.sin_family = AF_INET;

        bind(s, (struct sockaddr *)&sin, sizeof(sin));

	udpsock = s;

        return;
}

extern char NetTxPacket[];

void
udpsend (ip, port, len)
	IPaddr_t		ip;
	int			port;
	int			len;
{
        struct sockaddr_in      sin;
	int			r;

	bzero((char *)&sin, sizeof(sin));

        sin.sin_addr.s_addr = ip;
        sin.sin_port = htons(port);
        sin.sin_family = AF_INET;

	r = sendto (udpsock, NetTxPacket, len, 0,
	    (struct sockaddr *)&sin, sizeof(sin));

	if (r == -1)
		perror("sendto failed");

	return;
}

extern void
TftpHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len);

int
tftpget (filename, ourip, serverip, addr)
	char *			filename;
	char *			ourip;
	char *			serverip;
	char *			addr;
{
        fd_set                  fdset;
        struct timeval          tv;
	int			r;
	char			buf[2048];
	struct sockaddr_in	sin;
	int			len;

	load_addr = addr;
	BootFile = filename;
	NetServerIP = inet_addr(serverip);
	NetOurIP = inet_addr(ourip);
	
	sockcreate();
	TftpStart();

	while (1) {
		FD_ZERO(&fdset);
		FD_SET(udpsock, &fdset);
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		r = select (FD_SETSIZE, &fdset, NULL, NULL, &tv);

		bzero ((char *)&sin, sizeof(sin));
		len = sizeof(sin);
		r = recvfrom (udpsock, buf, 1500, 0,
		    (struct sockaddr *)&sin, &len);

		TftpHandler (buf, 0, ntohs(sin.sin_port), r);
		if (NetState == NETLOOP_SUCCESS)
			break;
	}
	return (0);
}
