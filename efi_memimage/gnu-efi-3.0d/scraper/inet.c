#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/errno.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <arpa/inet.h>
#include <net/if.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int
ifconfig (ifname, ifaddr, ifmask)
	char *			ifname;
	char *			ifaddr;
	char *			ifmask;
{
	int			s;
	struct sockaddr_in	*sp;
	struct ifreq		ifr;
	uint32_t		bcast;
	uint32_t		flags;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
	if (s == -1)
		return (EBADF);

	/* First bring interface up */

	bzero((char *)&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	if (ioctl (s, SIOCGIFFLAGS, &ifr) < 0) {
		printf("get flags failed... %d\n", errno);
		return (errno);
	}
	flags = ifr.ifr_flags;
	ifr.ifr_flags |= IFF_UP;
	if (ioctl (s, SIOCSIFFLAGS, &ifr) < 0) {
		printf("set flags failed... %d\n", errno);
		return (errno);
	}

	/* Set IP address */

	bzero((char *)&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	sp = (struct sockaddr_in *)&(ifr.ifr_addr);
	sp->sin_addr.s_addr = inet_addr(ifaddr);
	sp->sin_len = sizeof(struct sockaddr_in);
	sp->sin_family = AF_INET;
	if (ioctl (s, SIOCSIFADDR, &ifr) < 0) {
		printf("set address failed... %d\n", errno);
		return (errno);
	}

	/* Set netmask */

	bzero((char *)&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	sp = (struct sockaddr_in *)&(ifr.ifr_addr);
	sp->sin_addr.s_addr = inet_addr(ifmask);
	sp->sin_len = sizeof(struct sockaddr_in);
	sp->sin_family = AF_INET;
	if (ioctl (s, SIOCSIFNETMASK, &ifr) < 0) {
		printf("set netmask failed... %d\n", errno);
		return (errno);
	}

	/* Set broadcast address, if this is a broadcast capable iface. */

	if (!(flags & IFF_BROADCAST))
		return (0);

	bcast = inet_addr(ifaddr) & inet_addr(ifmask);
	bcast |= INADDR_BROADCAST & ~inet_addr(ifmask);

	bzero((char *)&ifr, sizeof(ifr));
	strcpy(ifr.ifr_name, ifname);
	ifr.ifr_addr.sa_family = AF_INET;
	sp = (struct sockaddr_in *)&(ifr.ifr_broadaddr);
	sp->sin_addr.s_addr = bcast;
	sp->sin_len = sizeof(struct sockaddr_in);
	sp->sin_family = AF_INET;
	if (ioctl (s, SIOCSIFBRDADDR, &ifr) < 0) {
		printf("set broadcast failed... %d\n", errno);
		return (errno);
	}

	return (0);
}

