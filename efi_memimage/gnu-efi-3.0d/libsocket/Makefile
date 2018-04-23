OBJS = EfiSocketInit.o	getnetbydns.o	inet_addr.o	ns_addr.o	res_init.o
OBJS += EfiSocketIo.o	getnetbyht.o	inet_lnaof.o	ns_name.o	res_mkquery.o
OBJS += accept.o	getnetbynis.o	inet_makeaddr.o	ns_netint.o	res_mkupdate.o
OBJS += addr2ascii.o	getnetnamadr.o	inet_net_ntop.o	ns_ntoa.o	res_query.o
OBJS += ascii2addr.o	getpeername.o	inet_net_pton.o	ns_parse.o	res_send.o
OBJS += base64.o	getproto.o	inet_neta.o	ns_print.o	res_update.o
OBJS += bind.o		getprotoent.o	inet_netof.o	ns_ttl.o	send.o
OBJS += connect.o	getprotoname.o	inet_network.o	nsap_addr.o	sendto.o
OBJS += ether_addr.o	getservbyname.o	inet_ntoa.o	pollsocket.o	sethostname.o
OBJS += gethostbydns.o	getservbyport.o	inet_ntop.o	recv.o		setsockopt.o
OBJS += gethostbyht.o	getservent.o	inet_pton.o	recvfrom.o	shutdown.o
OBJS += gethostbynis.o	getsockname.o	linkaddr.o	res_comp.o	socket.o
OBJS += gethostnamadr.o	getsockopt.o	listen.o	res_data.o
OBJS += gethostname.o	herror.o	map_v4v6.o	res_debug.o

CFLAGS= -I. -I../inc -I../inc/ia32 -I../include/bsd -O2 -fpic -Wall -fshort-wchar -ffreestanding -fno-strict-aliasing -fno-merge-constants

libsocket.a: $(OBJS)
	ar rcs libsocket.a $(OBJS)

clean:
	rm -f *.o libsocket.a
